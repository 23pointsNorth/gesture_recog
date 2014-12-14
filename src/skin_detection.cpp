#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "mouse.hpp"
#include "movement.hpp"
#include "classifier.hpp"
#include "virtualHID.hpp"

#include <iostream>

using namespace cv;
using namespace std;


void preprocessImage(const Mat& src, Mat& blur, Mat& hsv);
void teachClassifier(const Mat& src, Classifier& cls, MouseData& mouse);
void skinSegmentation(const Mat& src, Classifier& cls, Mat& seg, Mat& morph);
void shapeAnalysis(const Mat& morph, Mat& skin);

VirtualHID vhid;
bool move_mouse = false;

int main()
{
    Mat src, blur, hsv, movement, seg, morph, skin;
    Classifier seg_cls;
    MouseData mouse;

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
    MovementExtractor fg(src);

    char key;
    do
    {
        cam >> src;
        if (src.empty())
            return -1;

        // Preprocess the image
        preprocessImage(src, blur, hsv);

        // Teach classifier
        teachClassifier(hsv, seg_cls, mouse);

        // Skin detection
        skinSegmentation(hsv, seg_cls, seg, morph);

        // Shape analysis
        shapeAnalysis(morph, skin);

        // Movement
        fg.UpdateModel(src, movement);
        fg.RefineMovement(movement);
        
        // Mat joint;
        // bitwise_and(movement, skin, joint);
        // imshow("Joint Mat", joint);

        // Display degug info
        imshow("Image", src);
        imshow("Segmented", seg);
        imshow("Morphed", morph);
        imshow("Skin", skin);
        imshow("Movement", movement);

        key = waitKey(10);

        if(key == 'm')
        {
            move_mouse = !move_mouse;
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
    static int erosion_size = 1;
    static Mat element = 
        getStructuringElement(MORPH_RECT,
                              Size(2*erosion_size + 1, 2*erosion_size+1),
                              Point(erosion_size, erosion_size));

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

    erode(seg, morph, element);
    dilate(morph, morph, element);
    dilate(morph, morph, element);
    

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

void shapeAnalysis(const Mat& morph, Mat& skin)
{
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    skin = Mat::zeros(morph.rows, morph.cols, CV_8UC1);
    

    findContours(morph.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (contours.empty()) return;

    // Get fisrt and second largest contours (head & hand)
    int f = -1, s = -1;
    double maxArea = 0;
    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        if(area > maxArea)
        {
            maxArea = area;
            s = f;
            f = i;
        }
    }

    drawContours(skin, contours, f, Scalar(255), CV_FILLED, 8, hierarchy, 0, Point());

    Point cntr = Point(0, 0);    
    for (size_t i = 0; i < contours[f].size(); i++)
    {
        cntr += contours[f][i];
    }
    cntr *= 1.0 / contours[f].size();

    cout << "Mouse @ " << cntr << endl; 

    if(move_mouse)
    {
        vhid.MoveMouse(cntr.x, cntr.y);
    }
}