#ifndef ANN_CLASSIFIER_HPP
#define ANN_CLASSIFIER_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/ml/ml.hpp>

#include <cstring>

const int NUM_OF_LAYERS = 3;
const int NUM_OF_INPUTS = 180;

const int NUM_OF_HAND_GESTURES = 4;
enum HandGestures {
	OPEN_HAND		= 0,
	INDEX			= 1,
	V_SIGN			= 2,
	THREE_FINGERS 	= 3,
	FOUR_FINGERS 	= 4,
	FIVE_FINGERS	= 5,
	FIST 			= 6
};

class GestureClassifier
{
public:
	GestureClassifier();
	int Train(const cv::Mat inputs, const cv::Mat outputs); // Returns number of iterations
	void Classify(const cv::Mat& input, HandGestures& gesture);
	void Save(const std::string filename);
	void Load(const std::string filename);
	void LoadTrainingData(std::string training_data_file, cv::Mat& inputs, cv::Mat& outputs);
	int TrainOnFileData(std::string training_data_file);
	~GestureClassifier();

private:
	CvANN_MLP ann;
	CvANN_MLP_TrainParams train_params;

};

#endif