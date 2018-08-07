#include "stdafx.h"
#include "BallCVPoint.h"
#include <algorithm>
#include <numeric>

BallCVPoint::BallCVPoint(cv::Point p, int f) {
	point = p;
	frame = f;
}

double BallCVPoint::calcSlope(BallCVPoint p) {
	return (p.getPoint().y - point.y) / ((p.getPoint().x - point.x) == 0 ? 1 : ((p.getPoint().x - point.x)));
}

void BallCVPoint::setSlope(BallCVPoint p) {
	slope = (p.getPoint().y - point.y) / ((p.getPoint().x - point.x)==0?1:((p.getPoint().x - point.x)));
}

cv::Point BallCVPoint::getPoint() {
	return point;
}


int BallCVPoint::getFrame() {
	return frame;
}


void BallCVPoint::setDistance(BallCVPoint p) {
	Point a = point;
	Point b = p.getPoint();
	double xx = (a.x - b.x)*(a.x - b.x);
	double yy = (a.y - b.y)*(a.y - b.y);
	distance = sqrt(xx + yy)/(frame-p.getFrame());
}

double BallCVPoint::getDistance() {
	return distance;
}

double BallCVPoint::getSlope() {
	return slope;
}

BallCVPoint::~BallCVPoint() {
}