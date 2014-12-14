#include "movement.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace cv;

MovementExtractor::MovementExtractor(const Mat& init)
{
	init.copyTo(background);
	movement_durations = Mat::zeros(background.size(), CV_8UC1);
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
			Vec3b old = background.at<Vec3b>(i, j);
			Vec3b new_pxl = image.at<Vec3b>(i, j);

			if(norm(old - new_pxl) > MOVEMENT_THERSHOLD)
			{
				movement.at<uchar>(i, j) = MOVEMENT_FOREGROUND;
				movement_durations.at<uchar>(i, j)++;
			}
			else
			{
				movement_durations.at<uchar>(i, j) = 0;
			}

			if(movement_durations.at<uchar>(i, j) > MOVEMENT_DURATION)
			{
				background.at<Vec3b>(i, j) = new_pxl * 0.95 + old * 0.05; //(new_pxl + old) * 0.5;
			}
		}
	}
}


void MovementExtractor::RefineMovement(cv::Mat& movement)
{
	Mat element = cv::getStructuringElement(MORPH_RECT, cv::Size(3, 3), Point(0, 0));

 	cv::morphologyEx(movement, movement, MORPH_OPEN, element, Point(-1, -1), 2); // 2 iternations
	cv::morphologyEx(movement, movement, MORPH_CLOSE, element, Point(-1, -1), 2);
}