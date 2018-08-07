#ifndef STRATEGY_H
#define STRATEGY_H

namespace strategy {
	struct myclass {
		bool operator() (int i, int j) { return (i < j); }
	} myobject;

	std::vector<int> FindKeyframes(std::string filename) {
		cv::VideoCapture stream(filename);

		// variables to check total frames
		int counter = 0, play = 0, y = 20;
		cv::Mat frame, pframe, rframe;

		//std::vector<int> diff;

		//while (true) {
		//	stream >> frame;
		//	if (frame.empty()) {
		//		//std::cout << "End...\n" << std::endl;
		//		break;
		//	}

		//	if (counter == 0) {
		//		pframe = frame.clone();
		//		cv::absdiff(frame, pframe, rframe);
		//	}
		//	else {
		//		cv::absdiff(frame, pframe, rframe);
		//		pframe = frame.clone();
		//	}

		//	int a = cv::sum(rframe)[0];
		//	diff.push_back(a);
		//	counter++;
		//}

		//std::sort(diff.begin(), diff.end(), myobject);

		//float toptenaverage = (diff[diff.size() - 11] + diff[diff.size() - 12] +
		//	diff[diff.size() - 13] + diff[diff.size() - 14] + 
		//	diff[diff.size() - 15] + diff[diff.size() - 16] +
		//	diff[diff.size() - 17] + diff[diff.size() - 18] +
		//	diff[diff.size() - 19] + diff[diff.size() - 20]) / 10;

		//float median = 0;
		//float toptenaverage = (std::accumulate(diff.begin(), diff.end(), 0.0 / diff.size())) / 1000;
		//if (diff.size() % 2 == 0) {
		//	median = (diff[diff.size() / 2 - 1] + diff[diff.size() / 2]) / 2;
		//}
		//else {
		//	median = diff[diff.size() / 2];
		//}

		//std::cout << median << std::endl;

		counter = 0;
		std::vector<int> keyframes, newkeyframes;

		while (true) {
			stream >> frame;
			if (frame.empty()) {
				//std::cout << "End...\n" << std::endl;
				break;
			}

			if (counter == 0) {
				pframe = frame.clone();
				cv::absdiff(frame, pframe, rframe);
				//std::cout << cv::sum(rimage)[0] << std::endl;
			}
			else {
				cv::absdiff(frame, pframe, rframe);
				//std::cout << cv::sum(rimage)[0] << std::endl;
				pframe = frame.clone();
			}

			if (cv::sum(rframe)[0] > 2500000) {
				play++;
				keyframes.push_back(counter);
				//std::cout << counter;
			}
			counter++;
		}

		//for (size_t i = 0; i < keyframes.size() - 2; i++) {
		//	if (keyframes[i + 1] - keyframes[i] < 20) {
		//		keyframes.erase(std::remove(keyframes.begin(), keyframes.end(), keyframes[i + 1]), keyframes.end());
		//	}
		//	if (keyframes[i + 2] - keyframes[i] < 20) {
		//		keyframes.erase(std::remove(keyframes.begin(), keyframes.end(), keyframes[i + 2]), keyframes.end());
		//	}
		//}
		for (size_t i = 0; i < keyframes.size() - 1; i++) {
			if (i == 0) {
				newkeyframes.push_back(keyframes[i]);
			}
			else if (keyframes[i + 1] - keyframes[i] > 20 && i > 0) {
				newkeyframes.push_back(keyframes[i + 1]);
			}
		}

		return newkeyframes;
	}

	double CalculateOffensiveProbability(cv::Mat fs, cv::Mat fx, cv::Mat fy, std::string filename) {
		CvHMM hmm;
		double oflogpseq;
		double slogpseq, xylogpseq;
		cv::Mat spstates, sforward, sbackward, xypstates, xyforward, xybackward;
		cv::Mat resfxy = cv::Mat(1, fx.cols, CV_64F);
		cv::FileStorage storage;

		cv::Mat TRANStso;
		storage.open("model/TRANStso.yml", cv::FileStorage::READ);
		storage["TRANStso"] >> TRANStso;
		storage.release();

		cv::Mat TRANStxyo;
		storage.open("model/TRANStxyo.yml", cv::FileStorage::READ);
		storage["TRANStxyo"] >> TRANStxyo;
		storage.release();

		cv::Mat EMIStso;
		storage.open("model/EMIStso.yml", cv::FileStorage::READ);
		storage["EMIStso"] >> EMIStso;
		storage.release();

		cv::Mat EMIStxyo;
		storage.open("model/EMIStxyo.yml", cv::FileStorage::READ);
		storage["EMIStxyo"] >> EMIStxyo;
		storage.release();

		cv::Mat INITtso;
		storage.open("model/INITtso.yml", cv::FileStorage::READ);
		storage["INITtso"] >> INITtso;
		storage.release();

		cv::Mat INITtxyo;
		storage.open("model/INITtxyo.yml", cv::FileStorage::READ);
		storage["INITtxyo"] >> INITtxyo;
		storage.release();

		cv::VideoCapture stream(filename);
		// grab the first frame
		cv::Mat frame_rgb;
		stream.read(frame_rgb);

		/************************************************** Stroke **************************************************/

		// Stroke
		for (int i = 0; i < fs.rows; i++) {
			hmm.decode(fs.row(i), TRANStso, EMIStso, INITtso, slogpseq, spstates, sforward, sbackward);
			if (std::isnan(slogpseq) == 1) {
				slogpseq = 0;
			}
			//std::cout << slogpseq << std::endl;
		}

		/*********************************************************************************************************/
		
		/************************************************** XY ***************************************************/

		int binsize = 3;
		int xrange[] = { (int)19 * frame_rgb.cols / 100, (int)53 * frame_rgb.cols / 100 };
		int yrange[] = { (int)49.2 * frame_rgb.rows / 100, (int)92.7 * frame_rgb.rows / 100 };
		std::vector<cv::Point2i> pos = {};

		for (size_t j = 0; j < binsize; j++) {
			if (j == 0) {
				int x = xrange[0] + ((xrange[1] - xrange[0]) / binsize);
				int y = yrange[0] + ((yrange[1] - yrange[0]) / binsize);
				pos.push_back(cv::Point2i(x, y));
			}
			else {
				int x = pos[j - 1].x + ((xrange[1] - xrange[0]) / binsize);
				int y = pos[j - 1].y + ((yrange[1] - yrange[0]) / binsize);
				pos.push_back(cv::Point2i(x, y));
			}
		}

		// indexing each int value to the nearest bin
		for (size_t j = 0; j < resfxy.rows; j++) {
			for (size_t i = 0; i < resfxy.cols; i++) {
				for (size_t h = 0; h < binsize; h++) {
					if (fx.at<int>(j, i) >= pos[h].x && fx.at<int>(j, i) < pos[h + 1].x) {
						for (size_t g = 0; g < binsize; g++) {
							if (fy.at<int>(j, i) >= pos[g].y && fy.at<int>(j, i) < pos[g + 1].y) {
								resfxy.at<int>(j, i) = (g + (2 * g) + 1) + h;
							}
						}
					}
				}
			}
		}
		
		for (size_t j = 0; j < resfxy.rows; j++) {
			for (size_t i = 0; i < resfxy.cols; i++) {
				if (resfxy.at<int>(j, i) > 5 || resfxy.at<int>(j, i) < 0) {
					resfxy.at<int>(j, i) = resfxy.at<int>(j, i - 1);
				}
			}
		}

		/************************************************************************************************************/

		for (int i = 0; i < resfxy.rows; i++) {
			hmm.decode(resfxy.row(i), TRANStxyo, EMIStxyo, INITtxyo, xylogpseq, xypstates, xyforward, xybackward);
			if (std::isnan(xylogpseq) == 1) {
				xylogpseq = 0;
			}
			//std::cout << dlogpseq << std::endl;
		}

		oflogpseq = slogpseq + xylogpseq;

		return oflogpseq;
	}

	double CalculateDefensiveProbability(cv::Mat fs, cv::Mat fx, cv::Mat fy, std::string filename) {
		CvHMM hmm;
		double delogpseq;
		double slogpseq, xylogpseq;
		cv::Mat spstates, sforward, sbackward, xypstates, xyforward, xybackward;
		cv::Mat resfxy = cv::Mat(1, fx.cols, CV_64F);
		cv::FileStorage storage;

		cv::Mat TRANStsd;
		storage.open("model/TRANStsd.yml", cv::FileStorage::READ);
		storage["TRANStsd"] >> TRANStsd;
		storage.release();

		cv::Mat TRANStxyd;
		storage.open("model/TRANStxyd.yml", cv::FileStorage::READ);
		storage["TRANStxyd"] >> TRANStxyd;
		storage.release();

		cv::Mat EMIStsd;
		storage.open("model/EMIStsd.yml", cv::FileStorage::READ);
		storage["EMIStsd"] >> EMIStsd;
		storage.release();

		cv::Mat EMIStxyd;
		storage.open("model/EMIStxyd.yml", cv::FileStorage::READ);
		storage["EMIStxyd"] >> EMIStxyd;
		storage.release();

		cv::Mat INITtsd;
		storage.open("model/INITtsd.yml", cv::FileStorage::READ);
		storage["INITtsd"] >> INITtsd;
		storage.release();

		cv::Mat INITtxyd;
		storage.open("model/INITtxyd.yml", cv::FileStorage::READ);
		storage["INITtxyd"] >> INITtxyd;
		storage.release();

		cv::VideoCapture stream(filename);
		// grab the first frame
		cv::Mat frame_rgb;
		stream.read(frame_rgb);

		/************************************************** Stroke **************************************************/

		// Stroke
		for (int i = 0; i < fs.rows; i++) {
			hmm.decode(fs.row(i), TRANStsd, EMIStsd, INITtsd, slogpseq, spstates, sforward, sbackward);
			if (std::isnan(slogpseq) == 1) {
				slogpseq = 0;
			}
			//std::cout << slogpseq << std::endl;
		}

		/*********************************************************************************************************/

		/************************************************** XY ***************************************************/

		int binsize = 3;
		int xrange[] = { (int)19 * frame_rgb.cols / 100, (int)53 * frame_rgb.cols / 100 };
		int yrange[] = { (int)49.2 * frame_rgb.rows / 100, (int)92.7 * frame_rgb.rows / 100 };
		std::vector<cv::Point2i> pos = {};

		for (size_t j = 0; j < binsize; j++) {
			if (j == 0) {
				int x = xrange[0] + ((xrange[1] - xrange[0]) / binsize);
				int y = yrange[0] + ((yrange[1] - yrange[0]) / binsize);
				pos.push_back(cv::Point2i(x, y));
			}
			else {
				int x = pos[j - 1].x + ((xrange[1] - xrange[0]) / binsize);
				int y = pos[j - 1].y + ((yrange[1] - yrange[0]) / binsize);
				pos.push_back(cv::Point2i(x, y));
			}
		}

		// indexing each int value to the nearest bin
		for (size_t j = 0; j < resfxy.rows; j++) {
			for (size_t i = 0; i < resfxy.cols; i++) {
				for (size_t h = 0; h < binsize; h++) {
					if (fx.at<int>(j, i) >= pos[h].x && fx.at<int>(j, i) < pos[h + 1].x) {
						for (size_t g = 0; g < binsize; g++) {
							if (fy.at<int>(j, i) >= pos[g].y && fy.at<int>(j, i) < pos[g + 1].y) {
								resfxy.at<int>(j, i) = (g + (2 * g) + 1) + h;
							}
						}
					}
				}
			}
		}

		for (size_t j = 0; j < resfxy.rows; j++) {
			for (size_t i = 0; i < resfxy.cols; i++) {
				if (resfxy.at<int>(j, i) > 5 || resfxy.at<int>(j, i) < 0) {
					resfxy.at<int>(j, i) = resfxy.at<int>(j, i - 1);
				}
			}
		}

		/************************************************************************************************************/

		for (int i = 0; i < resfxy.rows; i++) {
			hmm.decode(resfxy.row(i), TRANStxyd, EMIStxyd, INITtxyd, xylogpseq, xypstates, xyforward, xybackward);
			if (std::isnan(xylogpseq) == 1) {
				xylogpseq = 0;
			}
			//std::cout << dlogpseq << std::endl;
		}

		delogpseq = slogpseq + xylogpseq;

		return delogpseq;
	}

	std::vector<int> Classification(std::string filename, std::vector<cv::Vec8i> playerpositions, std::vector<int> strokes) {
		std::vector<double> all_logpseq;
		std::vector<int> strategy;

		std::vector<int> keyframes = FindKeyframes(filename);

		// calculate bottom player position (x and y feature)
		std::vector<int> xpos;
		std::vector<int> ypos;
		for (size_t i = 0; i < playerpositions.size(); i++) {
			int x = playerpositions[i][4] + (playerpositions[i][6] / 2);
			int y = playerpositions[i][5] + playerpositions[i][7];

			xpos.push_back(x);
			ypos.push_back(y);
		}

		// Process every play, use the keyframe as starter of each play
		for (size_t i = 0; i < keyframes.size(); i++) {
			cv::Mat mpstrokes, mpx, mpy;
			double cpo = 0; // counter for probability offensive
			double cpd = 0; // counter for probability defensive
			if (i <= keyframes.size() - 2) { // first keyframes to very last keyframes
				// extract every stroke within each play using keyframes vector
				std::vector<int> subs(&strokes[keyframes[i]], &strokes[keyframes[i + 1]]);
				// extract every x within each play using keyframes vector
				std::vector<int> subx(&xpos[keyframes[i]], &xpos[keyframes[i + 1]]);
				// extract every y within each play using keyframes vector
				std::vector<int> suby(&ypos[keyframes[i]], &ypos[keyframes[i + 1]]);

				/************************************** Uncomment to See the Result **************************************/
				//for (size_t i = 0; i < subs.size(); i++) {
				//	std::cout << subs[i] << " ";
				//}
				//std::cout << std::endl << subs.size() << std::endl;
				/*********************************************************************************************************/

				mpstrokes = cv::Mat(1, subs.size(), CV_64F);
				memcpy(mpstrokes.data, subs.data(), subs.size() * sizeof(int));
				mpx = cv::Mat(1, subx.size(), CV_64F);
				memcpy(mpx.data, subx.data(), subx.size() * sizeof(int));
				mpy = cv::Mat(1, suby.size(), CV_64F);
				memcpy(mpy.data, suby.data(), suby.size() * sizeof(int));
			}
			else { // very last keyframes to the last frames
				// extract every stroke within each play using keyframes vector
				std::vector<int> subs(&strokes[keyframes[i]], &strokes[strokes.size()]);
				// extract every x within each play using keyframes vector
				std::vector<int> subx(&xpos[keyframes[i]], &xpos[xpos.size()]);
				// extract every y within each play using keyframes vector
				std::vector<int> suby(&ypos[keyframes[i]], &ypos[ypos.size()]);

				/************************************** Uncomment to See the Result **************************************/
				//for (size_t i = 0; i < subs.size(); i++) {
				//	std::cout << subs[i] << " ";
				//}
				//std::cout << std::endl << subs.size() << std::endl;
				/*********************************************************************************************************/

				mpstrokes = cv::Mat(1, subs.size(), CV_64F);
				memcpy(mpstrokes.data, subs.data(), subs.size() * sizeof(int));
				mpx = cv::Mat(1, subx.size(), CV_64F);
				memcpy(mpx.data, subx.data(), subx.size() * sizeof(int));
				mpy = cv::Mat(1, suby.size(), CV_64F);
				memcpy(mpy.data, suby.data(), suby.size() * sizeof(int));
			}
			/************************************** Uncomment to See the Result **************************************/
			//for (size_t j = 0; j < mpstrokes.rows; j++) {
			//	for (size_t i = 0; i < mpstrokes.cols; i++) {
			//		std::cout << mpstrokes.at<int>(j, i) << " ";
			//	}
			//	std::cout << std::endl;
			//}
			//std::cout << std::endl << std::endl;
			//std::cin.get();
			/*********************************************************************************************************/

			// HMM Offensive
			cpo = CalculateOffensiveProbability(mpstrokes, mpx, mpy, filename);
			//cpo = cpo * -1;

			// HMM Defensive
			cpd = CalculateDefensiveProbability(mpstrokes, mpx, mpy, filename);
			//cpd = cpd * -1;

			if (cpo < (float)-30) {
				all_logpseq.push_back(cpo);
				strategy.push_back(0); // offensive
			}
			else {
				all_logpseq.push_back(cpd);
				strategy.push_back(1); // defensive
			}
		}

		std::ofstream fout;
		fout.open("results/strategy.txt");
		for (size_t i = 0; i < all_logpseq.size(); i++) {
			fout << all_logpseq[i] << "\t" << strategy[i] << std::endl;
		}
		fout << std::flush;
		fout.close();

		return strategy;
	}
} // namespace strategy

#endif STRATEGY_H