// main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>

/// OpenCV
//#include <opencv2\opencv.hpp>
//#include <opencv2\xfeatures2d.hpp>

/// Additional
#include "CvHMM.h"
#include "CourtModel.h"
#include "Player.h"
#include "Stroke.h"
#include "Strategy.h"
//#include "Analysis.h"
#include "detectline/LineDetector.h"

#include <fstream>
#include <bitset>
#include "detectline/SimpleLineIdentifier.h"
#include "detectline/CSVExporter.h"
#include "BallCVPoint.h"
/// Additional
using namespace std;
using namespace cv;

//General Configuration
int hMin = 0;
int hMax = 180;
int sMin = 0;
int sMax = 80;
int vMin = 80;
int vMax = 255;


//滑动条回调
void on_trackbar(int, void*) {
	//HSV二值化
	//inRange(frame1HSV, Scalar(hMin, sMin, vMin), Scalar(hMax, sMax, vMax), thresholded);
	//imshow("frame", thresholded);

	//灰度二值化
	//threshold(fgMaskMOG2, fgMaskMOG2, thresholdMin, thresholdMax, THRESH_BINARY);
	//imshow("frame", fgMaskMOG2);
}

double GetDistance(Point a, Point b) {
	double xx = (a.x - b.x)*(a.x - b.x);
	double yy = (a.y - b.y)*(a.y - b.y);
	return sqrt(xx + yy);
}
/***
	判断合理性，三种曲线（递增 递减 和凸波峰），各两个方向。匹配模型
*/
bool checkValid(BallCVPoint ball, list<BallCVPoint> balls) {
	for (list<BallCVPoint>::iterator ballIt = balls.begin(); ballIt != balls.end(); ++ballIt) {
		if ((*ballIt).getDistance() * 2<ball.getDistance() || ball.getSlope()*(*ballIt).getSlope()<0) {
			return false;
		}
	}
	return true;
}

int main() {
	//载入视频E:\1badminton\github\tracking\BadmintonTracking-master\BadmintonTracking
	//VideoCapture cap("E:\1badminton\github\tracking\BadmintonTracking-master\BadmintonTracking\shanghai3.
	std::string filename = "E:\\1badminton\\github\\tracking\\BadmintonTracking-master\\BadmintonTracking\\output_file.mp4";
	
	/*std::vector<cv::Vec4i> courtmodel;
	std::cout << "Court detection is in progress..." << std::endl;
	courtmodel = courtmodel::CreateCourtModel(filename);
	std::cout << "Court detection is finished!" << std::endl << std::endl;*/
	/*
	// player detection and tracking
	std::vector<cv::Vec8i> playerpositions;
	std::cout << "Player detection and tracking is in progress..." << std::endl;
	playerpositions = player::PositionsExtraction(filename);
	std::cout << "Player detection and tracking is finished!" << std::endl << std::endl;
	*/
	cv::VideoCapture cap(filename);
	/*
	LineDetector detector;
	std::string exportPath = "E:\\1badminton\\\\github\\tracking\\BadmintonTracking-master\\BadmintonTracking\\";
	Mat image1, image;
	cap >> image1;
	cvtColor(image1, image, CV_RGB2GRAY);
	//imshow("image11", image);
	//waitKey(0); 
	int totalFrames = (int)cap.get(CV_CAP_PROP_FRAME_COUNT);
	vector<Line> lines = detector.run(image, exportPath);

	SimpleLineIdentifier identifier;
	vector<pair<string, Line>> labeledLines = identifier.run(lines, image, exportPath);
	*/
	/// Grab the first frame
	//cv::Mat frame_rgb;
	//stream.read(frame_rgb);
	//if (!cap.isOpened())
	//	return -1;
	int fps = (int)cap.get(CV_CAP_PROP_FPS);

	int frameCount = 0;//帧计数
	int ballCount = 0;
	Point curPos, lastPos;
	int lastFrame = 0;//上一次出现球的帧数
	Mat frame1, frame2, diff, overlay, fgMaskMOG2;


	namedWindow("trackbar", WINDOW_NORMAL);
	namedWindow("frame");

	createTrackbar("hMin", "trackbar", &hMin, 180, on_trackbar);
	createTrackbar("hMax", "trackbar", &hMax, 180, on_trackbar);
	createTrackbar("sMin", "trackbar", &sMin, 255, on_trackbar);
	createTrackbar("sMax", "trackbar", &sMax, 255, on_trackbar);
	createTrackbar("vMin", "trackbar", &vMin, 255, on_trackbar);
	createTrackbar("vMax", "trackbar", &vMax, 255, on_trackbar);

	//构造自适应混合高斯背景提取器
	Ptr<BackgroundSubtractor> pMOG2;
	pMOG2 = createBackgroundSubtractorMOG2();
	//轨迹白板图像
	Mat track(Size(1280, 720), CV_8UC3, cv::Scalar(0, 0, 0));
	Mat track1(Size(1280, 720), CV_8UC3, cv::Scalar(0, 0, 0));
	//Mat track(Size(960, 540), CV_8UC3, cv::Scalar(0, 0, 0));

	double aveTime = 0;
	bool ballExist = false;
	int pixelAveValThre = 500; 
	VideoWriter record("tennis.mov", CV_FOURCC('D', 'I', 'V', 'X'), fps, track.size(), true);
	//帧循环开始
	bool isFirst = true;
	int totalFrames = (int)cap.get(CV_CAP_PROP_FRAME_COUNT);
	int i = 0;
	/*while (++i < 440) {
		cap >> frame1;
	}*/
	cap >> frame1;
	list<BallCVPoint> ballList;
	while (frameCount < totalFrames-50) {
		list<cv::Point> ballpos;
		double t = (double)getTickCount();
		//读取视频帧
		if (isFirst) {
			cap >> frame2;
			isFirst = false;
		}
		else {
			frame2.copyTo(frame1);
			cap >> frame2;
		}
		
		++frameCount;

		//更新背景并输出前景图像
		pMOG2->apply(frame1, fgMaskMOG2);
		medianBlur(fgMaskMOG2, fgMaskMOG2, 11);
		threshold(fgMaskMOG2, fgMaskMOG2, 180, 255, THRESH_BINARY);
		//imshow("mog", fgMaskMOG2);
		//waitKey(1000/fps);

		//进行差分
		diff = frame1 - frame2;
		blur(diff, diff, Size(7, 7));
		threshold(diff, diff, 30, 255, THRESH_BINARY);
		morphologyEx(diff, diff, MORPH_CLOSE, Mat());
		//imshow("diff", diff);
		//waitKey(1000/fps);

		//创建轮廓数组
		vector<vector<cv::Point>> contours;
		//获取轮廓
		findContours(fgMaskMOG2,
			contours,
			CV_RETR_EXTERNAL, //只取外部轮廓
			CV_CHAIN_APPROX_NONE);//存储所有轮廓点

		Mat contoursResult(Size(1280, 720), CV_8UC3, cv::Scalar(255, 255, 10));
		drawContours(contoursResult, contours, -1, Scalar(0, 0, 0), 2);
		/*if (frameCount>400) {  
		}*/

		//imshow("contoursResult", contoursResult);
		//waitKey(1000/fps);
		int maxPixelAveVal = 0;
		double minDistance = 1280;
		RotatedRect rrs[10];
		Point2f center;
		float radius;
		int contoursNum = 0;
		//创建轮廓的迭代器
		vector<vector<cv::Point>>::const_iterator itc = contours.begin();
		while (itc != contours.end()) {
			//按size()过滤
			if (itc->size() < 6 || itc->size() > 100)
				itc = contours.erase(itc);
			else {
				RotatedRect rr = minAreaRect(Mat(*itc));//计算最小包围可旋转矩形
														//求最短边长（最小包围可旋转矩形）
				int rrMinSize = rr.size.height;
				if (rr.size.width < rrMinSize)
					rrMinSize = rr.size.width;
				//按最小包围矩形的宽度过滤 太大的去掉
				if (rrMinSize > 15)
					itc = contours.erase(itc);
				else {
					double pointDistance = 0;
					if (ballExist) {
						//pointDistance = GetDistance(rr.center, lastPos) / (frameCount - lastFrame);
						pointDistance = GetDistance(rr.center, lastPos);
					}
					if (ballExist && pointDistance > 210)
						itc = contours.erase(itc);
					else {
						//计算该轮廓在diff中包含区域内的像素均值来过滤轮廓
						int pointNumber = 0;	//轮廓中的点数
						int pixelValueSum = 0;	//轮廓范围中像素值之和
						int pixelAveVal = 0;	//平均的像素值
						Rect r0 = boundingRect(Mat(*itc));//计算最小包围矩形
														  //以下四行为包围矩形的位置的尺寸
						int xROI = r0.x;
						int yROI = r0.y;
						int widthROI = r0.width;
						int heightROI = r0.height;

						double area = contourArea(Mat(*itc));
						int length = arcLength(Mat(*itc), true);
						if (area > 200) {
							double radio = length*length / area;
							if (radio<18 && radio>21) {
								itc = contours.erase(itc);
								continue;
							}
						}
						for (int i = xROI - 1; i < xROI + widthROI; i++)
							for (int j = yROI - 1; j < yROI + heightROI; j++) {
								//如果该点在轮廓内
								if (pointPolygonTest(*itc, Point2f(i + 1, j + 1), false) >0) {
									++pointNumber;
									pixelValueSum += diff.at<Vec3b>(j, i)[0];
									pixelValueSum += diff.at<Vec3b>(j, i)[1];
									pixelValueSum += diff.at<Vec3b>(j, i)[2];
								}
							}
						if (pointNumber != 0)
							pixelAveVal = pixelValueSum / pointNumber;
						if (pixelAveVal < 450) {
							//cout << "1-x:" << rr.center.x << ", y:" << rr.center.y << ", pixelAveVal:" << pixelAveVal <<", pixelValueSum:" << pixelValueSum << ", pointNumber:" << pointNumber << ", area:" << area << ", length:" << length << endl;
							itc = contours.erase(itc);
						}
						else {
							cout << "2-x:" << rr.center.x << ", y:" << rr.center.y << ", pixelAveVal:" << pixelAveVal <<  ", pointNumber:" << pointNumber << ", area:" << area << ", length:" << length << endl;

							/*if (area > 200) {
								circle(track1, rr.center, 2, Scalar(255, 252, 60), 2);
								cv::imshow("area", frame1 + track1);
								cv::imshow("fgMaskMOG2", fgMaskMOG2);
								waitKey(0);
							}*/
							//下面四行 绘制该旋转矩形
							Point2f vertices[4];
							rr.points(vertices);
							for (int i = 0; i < 4; i++)
								line(frame1, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 255), 2);
							/*
							if ((ballCount<10 && pixelAveVal > maxPixelAveVal) {
							maxPixelAveVal = pixelAveVal;
							curPos = rr.center;
							*/
							if (ballCount >= 10 && pointDistance < minDistance) {
								minDistance = pointDistance;
							}
							//curPos = rr.center;
							//cout << "2-pixelAveVal:" << pixelAveVal << ", x:" << rr.center.x << ", y:" << rr.center.y << endl;
							ballExist = true;
							ballpos.push_back(rr.center);
							++itc;
						}
					}
				}
			}
		}
		if (ballExist) {
			++ballCount;
			if (ballpos.size() == 1) {
				curPos = ballpos.front();
				BallCVPoint ballCvPoint(curPos, frameCount);
				if (ballList.size() > 0){
					ballCvPoint.setSlope(ballList.back());
					ballCvPoint.setDistance(ballList.back());
				}
				ballList.push_back(ballCvPoint);
				cout << "frame:" << frameCount << ", x:" << curPos.x << ", y:" << curPos.y << endl;

			}
			else {
				//计算斜率相似度
				list<BallCVPoint> validBall;
				for (list<cv::Point>::iterator ballIt = ballpos.begin(); ballIt != ballpos.end(); ++ballIt) {
					BallCVPoint ballCvPoint(*ballIt, frameCount);
					ballCvPoint.setSlope(ballList.back());
					ballCvPoint.setDistance(ballList.back());
					if (checkValid(ballCvPoint, ballList)) {
						validBall.push_back(ballCvPoint);
					}
				}
				if (validBall.size() > 1 || validBall.size()<1) {
					continue;
				}
				else {
					curPos = validBall.front().getPoint();
					ballList.push_back(validBall.front());
					cout << "frame:" << frameCount << ", x:" << curPos.x << ", y:" << curPos.y << endl;
				}
			}

			//circle(track, curPos, 2, Scalar(255, 252, 60), 2);
			if (ballCount >= 2) {
				line(track, curPos, lastPos, Scalar(40, 255, 255), 1);
			}
			lastPos = curPos;
			lastFrame = frameCount;
			if (ballList.size() > 3) {
				ballList.pop_front();
			}
		}
		ballpos.clear();
		overlay = frame1 + track;
		record << overlay;
		//cv::imshow("Frame", overlay);
		//waitKey(0);
		//double time = ((double)getTickCount() - t) / getTickFrequency();
		//if (frameCount>10)
		//	aveTime = (aveTime*(frameCount - 11) + time) / (frameCount - 10);
		//cout << "------------------------当前帧序：" << frameCount << "耗时" << aveTime << endl;
		//按空格暂停 再按继续
		if (waitKey(30) == 32) {
			if (waitKey() == 32) {
				continue;
			}
		}
	}
	return 0;
}



