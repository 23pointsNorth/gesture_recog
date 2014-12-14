#include "classifier.hpp"
#include "utils.hpp"
#include <iostream>

using namespace std;
using namespace cv;
Classifier::Classifier()
{
	points.clear();
}

void Classifier::AddPoint(cv::Point3i& pnt)
{
	points.push_back(pnt);
	cout << "Added point: " << pnt << endl;
}

void Classifier::LearnGaussian()
{
	vector<double> data(points.size());
	for(size_t i = 0; i < points.size(); ++i)
	{
		data[i] = points[i].x; // Hue
	}

	getMeanVar(data, mean_h, var_h);
	
	double norm_factor = 1.0 / sqrt(var_h * 2 * M_PI);
	thresh_h =  0.2 * norm_factor;

	for(size_t i = 0; i < points.size(); ++i)
	{
		data[i] = points[i].y; // Saturation
	}

	getMeanVar(data, mean_s, var_s);
	
	norm_factor = 1.0 / sqrt(var_s * 2 * M_PI);
	thresh_s =  0.2 * norm_factor;

	for(size_t i = 0; i < points.size(); ++i)
	{
		data[i] = points[i].z; // Value
	}

	getMeanVar(data, mean_v, var_v);
	
	norm_factor = 1.0 / sqrt(var_v * 2 * M_PI);
	thresh_v =  0.2 * norm_factor;



	cout << "   Mean H: " << mean_h << "   Var H: " << var_h << "   Thresh H: " << thresh_h << endl;
	cout << "   Mean S: " << mean_s << "   Var S: " << var_s << "   Thresh S: " << thresh_s << endl;
	cout << "   Mean V: " << mean_v << "   Var V: " << var_v << "   Thresh V: " << thresh_v << endl;
}

PixelClass Classifier::Gaussian(cv::Point3i& pnt)
{
	double p_h = exp(-((pnt.x - mean_h) * (pnt.x - mean_h) / 2 / var_h)) / sqrt(var_h * 2 * M_PI);
	double p_s = exp(-((pnt.y - mean_s) * (pnt.y - mean_s) / 2 / var_s)) / sqrt(var_s * 2 * M_PI);
	
	return (p_h * p_s > thresh_h * thresh_s) ? SKIN : BACKGROUND;
}

void Classifier::getMeanVar(std::vector<double> data, double& mean, double& var)
{
	if (data.size() == 0) return;

	double sum = 0;
	for(size_t i = 0; i < data.size(); ++i)
	{
		sum += data[i];
	}
	mean = sum / data.size();

	var = 0;
	for(size_t i = 0; i < data.size(); ++i)
	{
		var += (data[i] - mean) * (data[i] - mean);
	}
	var /= data.size();
}