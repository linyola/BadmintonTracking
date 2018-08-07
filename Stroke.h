#ifndef STROKE_H
#define STROKE_H

namespace stroke {
	std::vector<int> Classification(std::string filename, std::vector<cv::Vec8i> playerpositions) {
		cv::VideoCapture stream(filename);
		
		// first frame
		cv::Mat frame_rgb;

		std::vector<cv::Rect> rtopplayerrect;
		std::vector<cv::Rect> rbottomplayerrect;

		for (size_t i = 0; i < playerpositions.size(); i++) {
			int tx = playerpositions[i][0];
			int ty = playerpositions[i][1];
			int tw = playerpositions[i][2];
			int th = playerpositions[i][3];
			int bx = playerpositions[i][4];
			int by = playerpositions[i][5];
			int bw = playerpositions[i][6];
			int bh = playerpositions[i][7];
			rtopplayerrect.push_back(cv::Rect(tx, ty, tw, th));
			rbottomplayerrect.push_back(cv::Rect(bx, by, bw, bh));
		}
		
		// variables for the following loop
		int count = 0;
		cv::Mat testingmat;
		std::vector<int> strokes;

		while (true) {
			stream >> frame_rgb;
			if (frame_rgb.empty()) {
				//std::cout << "End...\n" << std::endl;
				break;
			}

			cv::Mat bottomplayer = cv::Mat(frame_rgb, rbottomplayerrect[count]);
			cv::Size size(120, 150);
			cv::resize(bottomplayer, bottomplayer, size);

			// load stroke model
			cv::Ptr<cv::ml::SVM> msvm = cv::ml::SVM::create();
			msvm = cv::Algorithm::load<cv::ml::SVM>("model/stroke_model.xml");

			// classify the stroke (stroke feature)
			cv::HOGDescriptor test_hog;
			std::vector<float> test_descriptors_values;
			std::vector<cv::Point> test_locations;
			test_hog.compute(bottomplayer, test_descriptors_values, cv::Size(32, 32), cv::Size(0, 0), test_locations);

			std::vector<std::vector<float>> test_v_descriptors_values;
			test_v_descriptors_values.push_back(test_descriptors_values);

			int row2 = test_v_descriptors_values.size(), col2 = test_v_descriptors_values[0].size();

			testingmat = cv::Mat::zeros(row2, col2, CV_32FC1);
			for (size_t j = 0; j < testingmat.rows; j++) {
				for (size_t i = 0; i < testingmat.cols; i++) {
					testingmat.at<float>(j, i) = test_v_descriptors_values[j][i];
				}
			}

			int stroke = msvm->predict(testingmat);

			strokes.push_back(stroke);

			count++;
			bottomplayer.release();
		}

		// save the stroke classification result to a file
		std::ofstream ofs;
		ofs.open("results/strokes.txt");
		for (size_t i = 0; i < strokes.size(); i++) {
			ofs << strokes[i] << " " << std::endl;
		}
		ofs.clear(); ofs.close();

		return strokes;
	}
} // namespace strokeclassification

#endif STROKE
