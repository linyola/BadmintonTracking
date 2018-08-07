#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "Strategy.h"

namespace analysis {
	void StrokeStatistics(std::vector<int> strokes) {
		std::vector<int> cf(6, 0); // 6 types of strokes

		for (int j = 0; j < cf.size(); j++) {
			int found = 0;
			for (size_t i = 0; i < strokes.size() - 1; i++) {
				if (j == strokes[i]) {
					found += 1;
				}
				else {
					found += 0;
				}
			}
			cf[j] = found;
		}

		// Data size
		float ds = 0;
		for (size_t i = 0; i < cf.size(); i++) {
			ds += cf[i];
		}

		// The data for the pie chart
		double cfp[6]; // 6 types of strokes
		for (size_t i = 0; i < sizeof(cfp) / sizeof(*cfp); i++) {
			cfp[i] = (double)(cf[i] * 100) / ds;
		}

		/************************************** Uncomment to See the Result **************************************/
		//double total = 0;
		//for (size_t i = 0; i < sizeof(cfp)/sizeof(*cfp); i++) {
		//	std::cout << i << ":" << cfp[i] << std::endl;
		//	total += cfp[i];
		//}
		//std::cout << "Total : " << total << std::endl;
		/*********************************************************************************************************/

		// The labels for the pie chart
		const char *labels[] = { "Clear", "Drive", "Drop", "Lob", "Smash", "Other" };

		// The depths for the sectors
		double depths[] = { 20, 20, 20, 20, 20, 20 };

		// Create a PieChart object of size 360 x 300 pixels, with a light blue (DDDDFF) background and
		// a 1 pixel 3D border
		PieChart *c = new PieChart(360, 300, 0xddddff, -1, 1);

		// Set the center of the pie at (180, 175) and the radius to 100 pixels
		c->setPieSize(180, 175, 100);

		// Add a title box using 15pt Times Bold Italic font and blue (AAAAFF) as background color
		c->addTitle("Stroke Evaluation Statistics", "timesbi.ttf", 15)->setBackground(0xaaaaff);

		// Set the pie data and the pie labels
		c->setData(DoubleArray(cfp, (int)(sizeof(cfp) / sizeof(cfp[0]))), StringArray(labels, (int)(sizeof(labels) / sizeof(labels[0]))));

		// Draw the pie in 3D with variable 3D depths
		c->set3D(DoubleArray(depths, (int)(sizeof(depths) / sizeof(depths[0]))));

		// Set the start angle to 225 degrees may improve layout when the depths of the sector are
		// sorted in descending order, because it ensures the tallest sector is at the back.
		c->setStartAngle(225);

		// Output the chart
		c->makeChart("results/strokestatistics.png");

		//free up resources
		delete c;

		// show the chart
		cv::Mat image_chart = cv::imread("results/strokestatistics.png");

		int startX = 0, startY = 0, width = 360, height = 290;

		cv::Mat ROI(image_chart, cv::Rect(startX, startY, width, height));

		cv::Mat croppedImage;

		// Copy the data into new matrix
		ROI.copyTo(croppedImage);

		imwrite("results/strokestatistics.png", croppedImage);

		cv::imshow("Stroke Statistics", croppedImage);
	}

	void StrategyStatistics(std::vector<int> strategy) {
		std::vector<int> cf(2, 0); // 2 types of strategies

		for (int j = 0; j < cf.size(); j++) {
			int found = 0;
			for (size_t i = 0; i < strategy.size() - 1; i++) {
				if (j == strategy[i]) {
					found += 1;
				}
				else {
					found += 0;
				}
			}
			cf[j] = found;
		}

		/************************************** Uncomment to See the Result **************************************/
		//double total = 0;
		//for (size_t i = 0; i < cf.size(); i++) {
		//	std::cout << i << ":" << cf[i] << std::endl;
		//}
		/*********************************************************************************************************/

		// Data size
		float ds = 0;
		for (size_t i = 0; i < cf.size(); i++) {
			ds += cf[i];
		}

		// The data for the pie chart
		double cfp[2]; // 2 types of strategies
		for (size_t i = 0; i < sizeof(cfp) / sizeof(*cfp); i++) {
			cfp[i] = (double)(cf[i] * 100) / ds;
		}

		/************************************** Uncomment to See the Result **************************************/
		//double total = 0;
		//for (size_t i = 0; i < sizeof(cfp)/sizeof(*cfp); i++) {
		//	std::cout << i << ":" << cfp[i] << std::endl;
		//	total += cfp[i];
		//}
		//std::cout << "Total : " << total << std::endl;
		/*********************************************************************************************************/

		// The labels for the pie chart
		const char *labels[] = { "Offensive", "Defensive" };

		// The depths for the sectors
		double depths[] = { 20, 20 };

		// Create a PieChart object of size 360 x 300 pixels, with a light blue (DDDDFF) background and
		// a 1 pixel 3D border
		PieChart *c = new PieChart(360, 300, 0xddddff, -1, 1);

		// Set the center of the pie at (180, 175) and the radius to 100 pixels
		c->setPieSize(180, 175, 100);

		// Add a title box using 15pt Times Bold Italic font and blue (AAAAFF) as background color
		c->addTitle("Strategy Evaluation Statistics", "timesbi.ttf", 15)->setBackground(0xaaaaff);

		// Set the pie data and the pie labels
		c->setData(DoubleArray(cfp, (int)(sizeof(cfp) / sizeof(cfp[0]))), StringArray(labels, (int)(sizeof(labels) / sizeof(labels[0]))));

		// Draw the pie in 3D with variable 3D depths
		c->set3D(DoubleArray(depths, (int)(sizeof(depths) / sizeof(depths[0]))));

		// Set the start angle to 225 degrees may improve layout when the depths of the sector are
		// sorted in descending order, because it ensures the tallest sector is at the back.
		c->setStartAngle(225);

		// Output the chart
		c->makeChart("results/strategystatistics.png");

		//free up resources
		delete c;

		// show the chart
		cv::Mat image_chart = cv::imread("results/strategystatistics.png");

		int startX = 0, startY = 0, width = 360, height = 290;

		cv::Mat ROI(image_chart, cv::Rect(startX, startY, width, height));

		cv::Mat croppedImage;

		// Copy the data into new matrix
		ROI.copyTo(croppedImage);

		imwrite("results/strategystatistics.png", croppedImage);

		cv::imshow("Strategy Statistics", croppedImage);
	}

	void ExecuteStatistics(std::string filename, std::vector<cv::Vec4i> court_model, std::vector<cv::Vec8i> playerpositions, std::vector<int> strokes, std::vector<int> strategies) {
		cv::VideoCapture stream(filename);
		if (!stream.isOpened()) {
			std::cout << "Cannot read the file!" << std::endl;
			std::exit(-1);
		}

		// Read badminton court model
		cv::Mat bcourt = cv::imread("model/Badminton Court.bmp");

		// Show the stroke
		cv::Mat canvas = cv::imread("model/Canvas.bmp");

		// Show the strategy
		cv::Mat canvas2 = cv::imread("model/Canvas 2.bmp");

		int cnt = 0, tmp_keyframes, space = 70, space2 = 80;
		cv::Mat frame;
		cv::Rect toplayerrect, bottomplayerrect;
		std::vector<std::string> str_strategy;

		std::vector<int> keyframes = strategy::FindKeyframes(filename);

		for (size_t i = 0; i < strategies.size(); i++) {
			if (strategies[i] == 0) {
				str_strategy.push_back("Offensive");
			}
			else {
				str_strategy.push_back("Defensive");
			}
		}

		// Homography matrix
		cv::Mat H;
		cv::FileStorage storage("model/homography.yml", cv::FileStorage::READ);
		storage["H"] >> H;
		storage.release();

		while (true) {
			stream >> frame;
			if (frame.empty() || strokes[cnt] == strokes.size()) {
				//std::cout << "End...\n" << std::endl;
				break;
			}

			toplayerrect.x = playerpositions[cnt][0];
			toplayerrect.y = playerpositions[cnt][1];
			toplayerrect.width = playerpositions[cnt][2];
			toplayerrect.height = playerpositions[cnt][3];
			bottomplayerrect.x = playerpositions[cnt][4];
			bottomplayerrect.y = playerpositions[cnt][5];
			bottomplayerrect.width = playerpositions[cnt][6];
			bottomplayerrect.height = playerpositions[cnt][7];

			std::string predicted_label;
			if (strokes[cnt] == 0)
				predicted_label = "CLEAR";
			else if (strokes[cnt] == 1)
				predicted_label = "DRIVE";
			else if (strokes[cnt] == 2)
				predicted_label = "DROP";
			else if (strokes[cnt] == 3)
				predicted_label = "LOB";
			else if (strokes[cnt] == 4)
				predicted_label = "SMASH";
			else if (strokes[cnt] == 5)
				predicted_label = "OTHER";
			cnt++;

			// Draw players rectangle
			cv::rectangle(frame, toplayerrect, cv::Scalar(255, 255, 255), 1, 8, 0);
			cv::rectangle(frame, bottomplayerrect, cv::Scalar(255, 255, 255), 1, 8, 0);

			// Draw the player's trajectory
			cv::Mat invH = H.inv(cv::DECOMP_SVD);
			int upx = toplayerrect.x + (toplayerrect.width / 2);
			int upy = toplayerrect.y + toplayerrect.height;
			int bpx = bottomplayerrect.x + (bottomplayerrect.width / 2);
			int bpy = bottomplayerrect.y + bottomplayerrect.height;

			int upx2 = (invH.at<double>(0, 0)*upx + invH.at<double>(0, 1)*upy + invH.at<double>(0, 2)) / (invH.at<double>(2, 0)*upx + invH.at<double>(2, 1)*upy + invH.at<double>(2, 2));
			int upy2 = (invH.at<double>(1, 0)*upx + invH.at<double>(1, 1)*upy + invH.at<double>(1, 2)) / (invH.at<double>(2, 0)*upx + invH.at<double>(2, 1)*upy + invH.at<double>(2, 2));
			int bpx2 = (invH.at<double>(0, 0)*bpx + invH.at<double>(0, 1)*bpy + invH.at<double>(0, 2)) / (invH.at<double>(2, 0)*bpx + invH.at<double>(2, 1)*bpy + invH.at<double>(2, 2));
			int bpy2 = (invH.at<double>(1, 0)*bpx + invH.at<double>(1, 1)*bpy + invH.at<double>(1, 2)) / (invH.at<double>(2, 0)*bpx + invH.at<double>(2, 1)*bpy + invH.at<double>(2, 2));

			cv::circle(bcourt, cv::Point(upx2, upy2), 2, cv::Scalar(255, 0, 255), 2, 8, 0);
			cv::circle(bcourt, cv::Point(bpx2, bpy2), 2, cv::Scalar(0, 0, 255), 2, 8, 0);

			// Draw the court line
			for (size_t i = 0; i < court_model.size(); i++) {
				cv::line(frame, cv::Point(court_model[i][0], court_model[i][1]), cv::Point(court_model[i][2], court_model[i][3]), cv::Scalar(255, 255, 255), 3, CV_AA);
			}

			cv::putText(canvas, predicted_label, cv::Point(35, space), cv::FONT_HERSHEY_PLAIN, 0.9, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
			space += 15;
			if (cnt % 28 == 0) {
				canvas = cv::imread("model/Canvas.bmp");
				space = 70;
			}

			for (size_t i = 0; i < keyframes.size(); i++) {
				if (cnt == keyframes[i] && cnt != 0) {
					std::string goal = "Play " + std::to_string(i + 1) + " - " + str_strategy[i];
					cv::putText(canvas2, goal, cv::Point(20, space2), cv::FONT_HERSHEY_PLAIN, 0.85, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
					space2 += 15;
					bcourt = cv::imread("model/Badminton Court.bmp");
				}
			}

			cv::Mat bcourt2;
			cv::flip(bcourt, bcourt2, 0);

			cv::imshow("1", frame);
			cv::imshow("2", bcourt2);
			cv::imshow("3", canvas);
			cv::imshow("4", canvas2);
			if (cv::waitKey(27) == 'q') break;
		}

		StrokeStatistics(strokes);
		StrategyStatistics(strategies);
		cv::waitKey(0);
	}
} // namespace analysis

#endif ANALYSIS_H