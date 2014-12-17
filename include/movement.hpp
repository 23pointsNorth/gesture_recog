#ifndef MOVEMENT_HPP
#define MOVEMENT_HPP

#include <opencv2/core/core.hpp>

#define MOVEMENT_THERSHOLD  10

#define MOVEMENT_BACKGROUND 0
#define MOVEMENT_FOREGROUND 255

class MovementExtractor
{
public:
	MovementExtractor(const cv::Mat& init);
	~MovementExtractor() {}

	void SetBackground(const cv::Mat& new_bg);
	void UpdateModel(const cv::Mat& image, cv::Mat& movement);
	void RefineMovement(cv::Mat& movement);

private:
	cv::Mat background;
};

#endif // MOVEMENT_HPP