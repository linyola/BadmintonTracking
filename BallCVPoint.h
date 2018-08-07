#pragma once

#include "stdafx.h"
#include <sstream>

using namespace std;
using namespace cv;

class BallCVPoint {
public:
	BallCVPoint(cv::Point, int);
	double calcSlope(BallCVPoint);
	void setSlope(BallCVPoint);
	void setDistance(BallCVPoint);
	double getDistance();
	cv::Point getPoint();
	int getFrame();
	double getSlope();
	~BallCVPoint();
private:
	cv::Point point;
	int frame;
	double slope;
	double distance;
};
