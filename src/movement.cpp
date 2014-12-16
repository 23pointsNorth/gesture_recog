#include "movement.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace cv;

MovementExtractor::MovementExtractor(const Mat& init)
{
	init.copyTo(background);
}

void MovementExtractor::SetBackground(const cv::Mat& new_bg)
{
	new_bg.copyTo(background);
}

void MovementExtractor::UpdateModel(const Mat& image, Mat& movement)
{
	CV_Assert(!image.empty());


	// imshow("img", image);
	// imshow("background", background);
	//waitKey();

	movement = Mat::zeros(image.size(), CV_8UC1);
	Scalar old_mean, new_entry;

	for(int i = 0; i < image.rows; ++i)
	{
		for(int j = 0; j < image.cols; ++j)
		{
			const Vec3b& old = background.at<Vec3b>(i, j);
			const Vec3b& new_pxl = image.at<Vec3b>(i, j);

			if(norm(old - new_pxl) > MOVEMENT_THERSHOLD)
			{
				movement.at<uchar>(i, j) = MOVEMENT_FOREGROUND;
			}
		}
	}
}


void MovementExtractor::RefineMovement(cv::Mat& movement)
{
	Mat element = cv::getStructuringElement(MORPH_RECT, cv::Size(3, 3), Point(1, 1));

 	cv::morphologyEx(movement, movement, MORPH_OPEN, element);
}