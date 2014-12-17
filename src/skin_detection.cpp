#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "mouse.hpp"
#include "movement.hpp"
#include "classifier.hpp"
#include "virtualHID.hpp"
#include "ann_classifier.hpp"

#include <iostream>

using namespace cv;
using namespace std;

const int ANN_INPUT_VECTOR_SIZE = 360;

void preprocessImage(const Mat& src, Mat& blur, Mat& hsv);
void teachClassifier(const Mat& src, Classifier& cls, MouseData& mouse);
void skinSegmentation(const Mat& src, Classifier& cls, Mat& seg, Mat& morph);
void skinAnalysis(const Mat& morph, Mat& skin);
void saveDescriptor(const vector<double> &descriptor, int label);
void saveTrainData(string filename);
vector<double> handAnalysis(const Mat& moving_skin);
vector<Point2d> getEigenVectors(vector<Point> &pts);

// Utility functions
int findLargestContour(vector< vector<Point> > contours);
Point findContourCenter(vector<Point> contour);

VirtualHID vhid;
bool move_mouse = false;

std::vector<Mat> train_data;
std::vector<int> train_label;

int main()
{
    Mat src, roi, blur, hsv, movement, seg, morph, skin, moving_skin;
    Classifier seg_cls;
    GestureClassifier gc;
    MouseData mouse;

    const Rect roi_rect(0, 0, 400, 300);


    namedWindow("Image");
    setMouseCallback("Image", onMouse, &mouse);

    VideoCapture cam(0);
    if (!cam.isOpened())
    {
        cout << "Cannot open camera." << endl;
        return 0;
    }

    // Read one frame
    cam >> src;

    // Initialize foreground
    MovementExtractor fg(src(roi_rect));

    char key;
    do
    {
        cam >> src;
        if (src.empty())
            return -1;

        // Get the ROI
        roi = src(roi_rect).clone();


        // Preprocess the image
        preprocessImage(roi, blur, hsv);

        // Teach classifier
        teachClassifier(hsv, seg_cls, mouse);

        // Skin detection
        skinSegmentation(hsv, seg_cls, seg, morph);

        // Analysis of the detected skin pixels
        skinAnalysis(morph, skin);

        // Movement
        fg.UpdateModel(roi, movement);
        fg.RefineMovement(movement);

        // Find moving skin
        bitwise_and(movement, skin, moving_skin);

        // Extract hand descriptor
        vector<double> hand_desc = handAnalysis(moving_skin);


        // Display degug info
        rectangle(src, roi_rect, Scalar(0, 0, 255), 2);
        imshow("Image", src);
        imshow("Segmented", seg);
        imshow("Morphed", morph);
        imshow("Skin", skin);
        imshow("Movement", movement);
        imshow("Joint Mat", moving_skin);

        key = waitKey(10);

        switch(key)
        {
            case 'm':
            {
                move_mouse = !move_mouse;
                break;
            }
            case 'b':
            {
                fg.SetBackground(roi);
                break;
            }
            case '1':
            case '2':
            case '3':
            case '0':
            {
                saveDescriptor(hand_desc, key - '0');
                break;
            }
            case 's':
            {
                saveTrainData("train_data.xml");
                cout << "Training" << endl;
                gc.TrainOnFileData("train_data.xml");
                cout << "Training ended." << endl;
            }
        }
    }
    while(key != 'q');

    return 0;
}

void preprocessImage(const Mat& src, Mat& blur, Mat& hsv)
{
    // Blur the image
    GaussianBlur(src, blur, Size(3, 3), 0, 0);

    // Convert
    cvtColor(blur, hsv, COLOR_BGR2HSV);

    // Offset the H component
    for(int i = 0; i < hsv.rows; ++i)
    {
        for(int j = 0; j < hsv.cols; ++j)
        {
            int idx = (i * hsv.cols + j) * 3;
            hsv.data[idx] = ((int)hsv.data[idx] + 90) % 180;
        }
    }
}

void skinSegmentation(const Mat& src, Classifier& cls, Mat& seg, Mat& morph)
{


    if(seg.empty())
    {
        seg = Mat(src.rows, src.cols, CV_8UC1);
    }

    for(int i = 0; i < src.rows; ++i)
    {
        for(int j = 0; j < src.cols; ++j)
        {
            int idx = (i * src.cols + j) * 3;

            Point3i pt = Point3i(src.data[idx],
                                 src.data[idx + 1],
                                 src.data[idx + 2]);

            PixelClass c = cls.Gaussian(pt);

            int clr = (c == SKIN) ? 255: 0;

            seg.data[i * src.cols + j] = clr;
        }
    }

    static Mat element = cv::getStructuringElement(MORPH_RECT, cv::Size(5, 5), Point(3, 3));
    cv::morphologyEx(seg, morph, MORPH_OPEN, element);

    // erode(seg, morph, element);
    // dilate(morph, morph, element);
    // dilate(morph, morph, element);


}

void teachClassifier(const Mat& src, Classifier& cls, MouseData& mouse)
{
    if(mouse.l_up)
    {
        int idx = mouse.y * src.cols * 3 + mouse.x * 3;
        Point3i pt = Point3i(src.data[idx],
                             src.data[idx + 1],
                             src.data[idx + 2]);

        cls.AddPoint(pt);
        mouse.l_up = false;

        cls.LearnGaussian();
    }
}

void skinAnalysis(const Mat& morph, Mat& skin)
{
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    skin = Mat::zeros(morph.rows, morph.cols, CV_8UC1);


    findContours(morph.clone(), contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (contours.empty()) return;

    int idx = findLargestContour(contours);

    drawContours(skin, contours, idx, Scalar(255), CV_FILLED, 8, hierarchy, 0, Point());

    // cout << " R:" << contourArea(contours[f]) / boundingRect(contours[f]).area() << endl;

    // if(move_mouse)
    // {
    //     vhid.MoveMouse(cntr.x, cntr.y);
    // }
}

vector<double> handAnalysis(const Mat& moving_skin)
{
    vector<double> descriptor;

    Mat hand_img = Mat::zeros(moving_skin.rows, moving_skin.cols, CV_8UC3);

    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;

    findContours(moving_skin.clone(), contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (contours.empty()) return descriptor;

    int idx = findLargestContour(contours);
    vector<Point>& hand = contours[idx];

    drawContours(hand_img, contours, idx, Scalar(255, 255, 255), 2, 8, hierarchy, 0, Point());

    vector<Point2d> vecs = getEigenVectors(contours[idx]);



    vector<double> dists;
    for(size_t i = 0; i < hand.size(); ++i)
    {
        double dist = (hand[i].x - vecs.back().x) * (hand[i].x - vecs.back().x);
        dist += (hand[i].y - vecs.back().y) * (hand[i].y - vecs.back().y);

        dists.push_back(dist);
    }

    circle(hand_img, vecs.back(), 3, CV_RGB(255, 0, 255), 2);
    line(hand_img, vecs.back(), vecs.back() + vecs[0], CV_RGB(255, 255, 0));
    line(hand_img, vecs.back(), vecs.back() + vecs[1], CV_RGB(0, 255, 255));
    imshow("Hand", hand_img);

    return dists; // descriptor
}

vector<Point2d> getEigenVectors(vector<Point> &pts)
{
    //Construct a buffer used by the pca analysis
    Mat data_pts = Mat(pts.size(), 2, CV_64FC1);
    for (int i = 0; i < data_pts.rows; ++i)
    {
        data_pts.at<double>(i, 0) = pts[i].x;
        data_pts.at<double>(i, 1) = pts[i].y;
    }

    //Perform PCA analysis
    PCA pca_analysis(data_pts, Mat(), CV_PCA_DATA_AS_ROW);

    //Store the position of the object
    Point2d pos = Point(pca_analysis.mean.at<double>(0, 0),
                        pca_analysis.mean.at<double>(0, 1));

    //Store the eigenvalues and eigenvectors
    vector<Point2d> eigen_vecs(2);
    vector<double> eigen_vals(2);
    for (int i = 0; i < 2; ++i)
    {
        eigen_vecs[i] = Point2d(pca_analysis.eigenvectors.at<double>(i, 0),
                                pca_analysis.eigenvectors.at<double>(i, 1));

        eigen_vals[i] = pca_analysis.eigenvalues.at<double>(0, i);

        eigen_vecs[i] *= sqrt(eigen_vals[i]);
    }

    eigen_vecs.push_back(pos);

    return eigen_vecs;
}


void saveDescriptor(const vector<double>& descriptor, int label)
{
    if (descriptor.empty()) return;

    double k = double(descriptor.size()) / ANN_INPUT_VECTOR_SIZE;
    Mat fdesc = Mat::zeros(Size(1, ANN_INPUT_VECTOR_SIZE), CV_64FC1);
    // double* fdesc_data = (double*)(fdesc.data);

    cout << "SIZE: " << descriptor.size() << endl;

    double max = -1;
    for (int i = 0; i < ANN_INPUT_VECTOR_SIZE; ++i)
    {
        double idx = k * i;
        double higher_weight = idx - floor(idx);
        double lower_weight = 1 - higher_weight;

        double interpulated = descriptor[floor(idx)] * lower_weight +
                              descriptor[ceil(idx)] * higher_weight;
        fdesc.at<double>(i, 0) = interpulated;

        if (interpulated > max) max = interpulated;
    }

    // Normalize values to 1.
    fdesc /= max;
    // cout << " " << fdesc << endl;
    // Mat desc = Mat(ANN_INPUT_VECTOR_SIZE, 1, CV_64FC1, fdesc);

    train_data.push_back(fdesc);
    train_label.push_back(label);

    cout << "Training data: " << train_data.size() << endl;
}

void saveTrainData(string filename)
{
    FileStorage fs(filename, FileStorage::WRITE);
    fs << "len" << int(train_data.size());
    for(size_t i = 0; i < train_data.size(); ++i)
    {
        ostringstream oss;
        oss << i;
        fs << "data" + oss.str() << train_data[i];
    }

    for(size_t i = 0; i < train_label.size(); ++i)
    {
        ostringstream oss;
        oss << i;
        fs << "label" + oss.str() << train_label[i];
    }
    fs.release();
}

int findLargestContour(vector< vector<Point> > contours)
{
    // Get the largest contour
    int idx = -1;
    double maxArea = 0;
    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        if(area > maxArea)
        {
            maxArea = area;
            idx = i;
        }
    }

    return idx;
}

Point findContourCenter(vector<Point> contour)
{
    Point cntr = Point(-1, -1);
    for (size_t i = 0; i < contour.size(); i++)
    {
        cntr += contour[i];
    }
    cntr *= 1.0 / contour.size();
    return cntr;
}