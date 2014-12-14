#ifndef MOVEMENT_HPP_
#define MOVEMENT_HPP_

#include <opencv2/core/core.hpp>

#define MOVEMENT_THERSHOLD  10
#define MOVEMENT_DURATION 	2

#define MOVEMENT_BACKGROUND 0
#define MOVEMENT_FOREGROUND 255

class MovementExtractor
{
public:
	MovementExtractor(const cv::Mat& init);
	~MovementExtractor() {}

	void UpdateModel(const cv::Mat& image, cv::Mat& movement);
	void RefineMovement(cv::Mat& movement);

private:
	cv::Mat background;
	cv::Mat movement_durations;

};

#endif // MOVEMENT_HPP_