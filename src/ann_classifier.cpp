#include "ann_classifier.hpp"
#include <opencv2/ml/ml.hpp>
#include <cstring>

using namespace std;
using namespace cv;

GestureClassifier::GestureClassifier()
{
	cv::Mat layers(3, 1, CV_32S);
    layers.at<int>(0,0) = NUM_OF_INPUTS;			//input layer
    layers.at<int>(1,0) = 16;						//hidden layer
    layers.at<int>(2,0) = NUM_OF_HAND_GESTURES;		//output layer

    // Activation params
    const double alpha = 1;
    const double beta = 1;
    ann.create(layers, CvANN_MLP::SIGMOID_SYM, alpha, beta);

    const double learning_rate = 0.2;
    const double momentum = 0.1;

    train_params = CvANN_MLP_TrainParams(
    	cvTermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 1000, 0.01),
	    CvANN_MLP_TrainParams::BACKPROP,
	    learning_rate,
	    momentum);
}

int GestureClassifier::Train(const Mat inputs, const Mat outputs)
{
	// Give all data equal priority
	Mat const priority = Mat::ones(1, inputs.rows, CV_32FC1);

	//Train classifier
	return ann.train(inputs, outputs, priority, Mat(), train_params);
}

void GestureClassifier::Classify(const cv::Mat& input, HandGestures& gesture)
{
	Mat output;
	ann.predict(input, output);

	// Find the gesture that has the maximum probability
	double minVal, maxVal; Point minLoc, maxLoc;
	minMaxLoc(output, &minVal, &maxVal, &minLoc, &maxLoc);

	gesture = HandGestures(maxLoc.x);
}

void GestureClassifier::Load(const string filename)
{
	// Load model from file
	CvFileStorage* storage = cvOpenFileStorage(filename.c_str(), 0, CV_STORAGE_READ);
    CvFileNode *n = cvGetFileNodeByName(storage, 0, "GestureClassifier");
    ann.read(storage, n);
    cvReleaseFileStorage(&storage);
}

void GestureClassifier::Save(const string filename)
{
	// Save the model generated into an xml file.
    CvFileStorage* storage = cvOpenFileStorage(filename.c_str(), 0, CV_STORAGE_WRITE);
    ann.write(storage, "GestureClassifier");
    cvReleaseFileStorage(&storage);
}

void GestureClassifier::LoadTrainingData(string training_data_file, Mat& inputs, Mat& outputs)
{
	FileStorage fs(training_data_file, FileStorage::READ);
    fs["inputs"] >> inputs;
    fs["outputs"] >> outputs;
}

int GestureClassifier::TrainOnFileData(string training_data_file)
{
	Mat inputs, outputs;
	LoadTrainingData(training_data_file, inputs, outputs);
	return Train(inputs, outputs);
}