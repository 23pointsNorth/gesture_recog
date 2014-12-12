#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "movement.hpp"
#include <iostream>
using namespace cv;
using namespace std;

int findBiggestContour(vector<vector<Point> > contours);
void detect_skin(const Mat& frame, Mat& skin);

int main()
{
    VideoCapture cam(0);
    if (!cam.isOpened())
    {
        cout << "Cannot open camera." << endl;
        return 0;
    }

    Mat src, movement, skin;
    cam >> src;

    MovementExtractor fg(src);
    char key;
    do{
        cam >> src;
        if (src.empty())
            return -1;

        fg.UpdateModel(src, movement);
        fg.RefineMovement(movement);
        imshow("Movement", movement);

        detect_skin(src, skin);
        imshow("Skin", skin);

        Mat joint;
        bitwise_and(movement, skin, joint);

        imshow("Joint Mat", joint);

        key = waitKey(10);
    }while(key != 'q');
    return 0;
}

int findBiggestContour(vector<vector<Point> > contours){
    int maxIdx = -1;
    double maxArea = 0;
    for (unsigned int i = 0; i < contours.size(); i++){

        double area = contourArea(contours[i]);
        if(area > maxArea){
            maxArea = area;
            maxIdx = i;
        }
    }
    return maxIdx;
}


void detect_skin(const Mat& frame, Mat& skin)
{
    skin = Mat::zeros(frame.size(), CV_8UC1);
    blur(frame, frame, Size(3, 3));

    Mat hsv;
    cvtColor(frame, hsv, COLOR_BGR2HSV);

    Mat bw;
    inRange(hsv, Scalar(0, 10, 60), Scalar(20, 150, 255), bw);
    dilate(bw, bw, Mat());

    imshow("All Skin", bw);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    findContours(bw, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
    if (contours.empty()) return;

    int s = findBiggestContour(contours);
    vector<vector<Point> > hull(1);
    convexHull(Mat(contours[s]), hull[0], false);
    //RotatedRect el = fitElipse(hull[0]);

    Mat fskin = Mat::zeros(frame.size(), CV_8UC1);
    drawContours(fskin, contours, s, Scalar(255), FILLED, 8, hierarchy, 0, Point());
    drawContours(skin, contours, s, Scalar(255), FILLED, 8, hierarchy, 0, Point());
    Scalar color = Scalar(255, 128, 128);
    drawContours(fskin, hull, 0, color, 5); // 8, vector<Vec4i>(), 0, Point()

    imshow("Feature skin", fskin);
}
