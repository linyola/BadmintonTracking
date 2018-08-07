/// C++
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>

/// OpenCV
#include <opencv2\opencv.hpp>

/// Additional
#include "CvHMM.h"
#include "CourtModel.h"
#include "Player.h"
#include "Stroke.h"
#include "Strategy.h"
#include "Analysis.h"

int main() {
	std::cout << "                                                                             " << std::endl;
	std::cout << "=============================================================================" << std::endl;
	std::cout << "                                                                             " << std::endl;
	std::cout << "                           Badminton Video Analysis                          " << std::endl;
	std::cout << "                                                                             " << std::endl;
	std::cout << "=============================================================================" << std::endl;
	std::cout << "                                                                             " << std::endl;

	// open a video file
	std::string filename = "videos/1/1.wmv";
	struct stat buffer;
	if (stat(filename.c_str(), &buffer) != 0) {
		std::cerr << "Unable to open video file: " << filename << " !" << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "Load " << filename << std::endl;

	// court detection
	std::vector<cv::Vec4i> courtmodel;
	std::cout << "Court detection is in progress..." << std::endl;
	courtmodel = courtmodel::CreateCourtModel(filename);
	std::cout << "Court detection is finished!" << std::endl << std::endl;

	// player detection and tracking
	std::vector<cv::Vec8i> playerpositions;
	std::cout << "Player detection and tracking is in progress..." << std::endl;
	playerpositions = player::PositionsExtraction(filename);
	std::cout << "Player detection and tracking is finished!" << std::endl << std::endl;

	// stroke classification
	std::vector<int> strokes;
	std::cout << "Stroke classification is in progress..." << std::endl;
	strokes = stroke::Classification(filename, playerpositions);
	std::cout << "Stroke classification is finished!" << std::endl << std::endl;

	// strategy classification
	std::vector<int> strategies;
	std::cout << "Strategy classification is in progress..." << std::endl;
	strategies = strategy::Classification(filename, playerpositions, strokes);
	std::cout << "Strategy classification is finished!" << std::endl << std::endl;

	/*********** Uncomment to Skip player detection and tracking - stroke classification - strategy ***********/
	//std::ifstream ifs;
	//int a, b, c, d, e, f, g, h, i, j;
	//ifs.open("results/playerpositions.txt");
	//while (ifs >> a >> b >> c >> d >> e >> f >> g >> h) {
	//	playerpositions.push_back(cv::Vec8i(a, b, c, d, e, f, g, h));
	//}
	//ifs.clear(); ifs.close();
	//ifs.open("results/strokes.txt");
	//while (ifs >> i) {
	//	strokes.push_back(a);
	//}
	//ifs.clear(); ifs.close();
	//ifs.open("results/strategy.txt");
	//while (ifs >> j) {
	//	strategies.push_back(a);
	//}
	//ifs.clear(); ifs.close();
	/*********************************************************************************************************/

	// analysis
	std::cout << "Analysis is in progress..." << std::endl;
	std::cout << "Press any key to proceed...";
	std::cin.get();
	std::cout << std::endl;
	analysis::ExecuteStatistics(filename, courtmodel, playerpositions, strokes, strategies);
	std::cout << "Analysis is finished!" << std::endl << std::endl;

	return 0;
}