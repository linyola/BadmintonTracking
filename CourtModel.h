#ifndef COURTMODEL_H
#define COURTMODEL_H

namespace courtmodel {
	/* Comparing for equality */
	bool Approx(float A, float B) {
		float diff = A - B;
		// an error (epsilon) of 0.00001 is appropriate for numbers around one
		float epsilon = pow(10, -4);
		return (diff < epsilon) && (-diff < epsilon);
	}

	/* White Pixel Extraction */
	cv::Mat WhitePixelExtraction(cv::Mat src) {
		double thetal = 160;
		double thetad = 20;
		cv::Mat dst = src.clone();

		// Assume that the court lines are typically not wider than 2 * 0.7% of frame height  
		int dist = (0.7 * src.rows) / 100;

		for (size_t j = 0; j < src.rows - 0; j++) {
			for (size_t i = 0; i < src.cols - 0; i++) {
				uchar intensity = src.at<uchar>(j, i);

				// Corresponds to the test if darker pixels can be found at some horizontal distance,
				// assuming that the court line is mostly vertical
				if (intensity >= thetal && i>=dist && i-dist<src.cols && intensity - src.at<uchar>(j, i - dist) > thetad 
					&& i+dist<src.cols &&intensity - src.at<uchar>(j, i + dist) > thetad) {
					dst.at<uchar>(j, i) = 255;
				}

				// Corresponds to the test if darker pixels can be found at some vertical distance,
				// assuming that the court line is mostly horizontal
				else if (intensity >= thetal && j>=dist && j-dist<src.rows && intensity - src.at<uchar>(j - dist, i) > thetad 
					&& j+dist<src.rows && intensity - src.at<uchar>(j + dist, i) > thetad) {
					dst.at<uchar>(j, i) = 255;
				}
				else {
					dst.at<uchar>(j, i) = 0;
				}
			}
		}

		return dst;
	}

	/* Textured Area Exclusion */
	cv::Mat CalculateGradients(cv::Mat src) {
		cv::Mat lamdas = cv::Mat::zeros(src.size(), CV_32FC(6));
		cv::Mat Mc = cv::Mat::zeros(src.size(), CV_32FC1);
		cv::Mat dst = cv::Mat::zeros(src.size(), src.type());

		//// find the eigenvalues and eigenvectors to determine if a pixel is a corner
		cv::cornerEigenValsAndVecs(src, lamdas, 3, 3);

		// additional constrains
		int left = floor((21 * src.cols) / 100);
		//std::cout << left << std::endl;
		int right = floor((79 * src.cols) / 100);
		//std::cout << right << std::endl;
		int top = floor((35 * src.rows) / 100);
		//std::cout << top << std::endl;
		int bottom = floor((93 * src.rows) / 100);
		//std::cout << bottom << std::endl;

		// calculate Mc (the response of the detector at each pixel)
		for (size_t j = 0; j < src.rows - 0; j++) {
			for (size_t i = 0; i < src.cols - 0; i++) {
				uchar intensity = src.at<uchar>(j, i);
				float lambda_1 = lamdas.at<cv::Vec6f>(j, i)[0];
				float lambda_2 = lamdas.at<cv::Vec6f>(j, i)[1];

				// 0.04f is tunable sensitivity parameter, in the literature values in the range 0.04¡V0.15 have been reported as feasible
				Mc.at<float>(j, i) = lambda_1*lambda_2 - 0.04f*pow((lambda_1 + lambda_2), 2);

				// if Mc (almost equal) 0 and Mc (almost equal) 0 then this pixel (i, j) has no features of interest, modify the pixel to 0
				if (Approx(Mc.at<float>(j, i), 0) == 1 && Approx(Mc.at<float>(j, i), 0) == 1) {
					dst.at<uchar>(j, i) = 0;
				}
				else {
					dst.at<uchar>(j, i) = 255;
				}

				if (j < top || j > bottom)
					dst.at<uchar>(j, i) = 0;

				if (i < left || i > right) {
					dst.at<uchar>(j, i) = 0;
				}
			}
		}

		return dst;
	}

	/* Compute Intersection between Lines */
	cv::Point ComputeIntersection(cv::Point l1pt1, cv::Point l1pt2, cv::Point l2pt1, cv::Point l2pt2) {
		int x1 = l1pt1.x, y1 = l1pt1.y, x2 = l1pt2.x, y2 = l1pt2.y;
		int x3 = l2pt1.x, y3 = l2pt1.y, x4 = l2pt2.x, y4 = l2pt2.y;

		if (float d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4))) {
			cv::Point pt;
			pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
			pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
			return pt;
		}
		else
			return cv::Point(-1, -1);
	}

	/* Line Detection */
	std::vector<cv::Vec4i> LineDetection(cv::Mat src, cv::Mat src2) {
		cv::Vec<double, 7> output;

		cv::Mat cdst;
		cvtColor(src, cdst, CV_GRAY2BGR);
		std::vector<cv::Vec4i> lines;
		HoughLinesP(src, lines, 1, CV_PI / 180, 80, 60, 25);

		std::vector<cv::Vec<double, 7>> vecs = std::vector<cv::Vec<double, 7> >();

		/************************************** Uncomment to See the Result **************************************/
		//// Draw the lines
		//for (size_t i = 0; i < lines.size(); i++) {
		//	cv::Vec4i l = lines[i];
		//	cv::line(src2, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(255, 255, 255), 2, CV_AA);
		//}
		//cv::imwrite("1.png", src2);
		//cv::imshow("1", src2);
		//cv::waitKey(0);
		/*********************************************************************************************************/

		for (size_t i = 0; i < lines.size(); i++) {

			double xdist = -(lines[i][3] - lines[i][1]);
			double ydist = lines[i][2] - lines[i][0];

			double norm = sqrt(xdist*xdist + ydist*ydist);

			double d = abs(lines[i][2] * lines[i][1] - lines[i][0] * lines[i][3]) / norm;

			cv::Vec<double, 7> v(lines[i][0], lines[i][1], lines[i][2], lines[i][3], xdist / norm, ydist / norm, d);
			vecs.push_back(v);
		}

		// Line Refinement
		for (size_t j = 0; j < vecs.size(); j++) {

			CvPoint tt, ttt;
			tt.x = vecs[j][0];
			tt.y = vecs[j][1];
			ttt.x = vecs[j][2];
			ttt.y = vecs[j][3];

			double x_diff = abs(tt.x - ttt.x);
			double y_diff = abs(tt.y - ttt.y);
			int horizontal = 0;

			if (x_diff > y_diff)
				horizontal = 1;
			else
				horizontal = 0;

			for (size_t i = j + 1; i < vecs.size() - 1; i++) {

				double dprod = vecs[i][4] * vecs[j][4] + vecs[i][5] * vecs[j][5];
				double angle = acos(dprod);

				double pt1x = vecs[i][0];
				double pt1y = vecs[i][1];
				double pt2x = vecs[i][2];
				double pt2y = vecs[i][3];

				double pt3x = vecs[j][0];
				double pt3y = vecs[j][1];
				double pt4x = vecs[j][2];
				double pt4y = vecs[j][3];

				//double d1 = sqrt(pow(pt1x - pt3x, 2) + pow(pt1y - pt3y, 2));
				//double d2 = sqrt(pow(pt1x - pt4x, 2) + pow(pt1y - pt4y, 2));
				//double d3 = sqrt(pow(pt2x - pt3x, 2) + pow(pt2y - pt3y, 2));
				//double d4 = sqrt(pow(pt2x - pt4x, 2) + pow(pt2y - pt4y, 2));

				int close = 0;
				int distThresh = floor((1.5 * src.rows) / 100);

				if (horizontal)
					close = (abs(pt1y - pt3y) < distThresh) || (abs(pt1y - pt4y) < distThresh) || (abs(pt2y - pt3y) < distThresh) || (abs(pt2y - pt4y) < distThresh);
				else
					close = (abs(pt1x - pt3x) < distThresh) || (abs(pt1x - pt4x) < distThresh) || (abs(pt2x - pt3x) < distThresh) || (abs(pt2x - pt4x) < distThresh);

				// remove similiar lines in term of lines distances and lines angles
				if (angle < cos(0.75) && close) {
					vecs.erase(vecs.begin() + i);
					i--;
				}
			}
		}

		for (size_t j = 0; j < vecs.size()-1; j++) {
			double pt1x = vecs[j][0];
			double pt1y = vecs[j][1];
			double pt2x = vecs[j][2];
			double pt2y = vecs[j][3];

			double pt3x = vecs[j + 1][0];
			double pt3y = vecs[j + 1][1];
			double pt4x = vecs[j + 1][2];
			double pt4y = vecs[j + 1][3];

			int length = 0;
			int lineThresh = floor((41 * src.rows) / 100);

			length = sqrt(pow((pt1x - pt2x), 2) + pow((pt1y - pt2y), 2));

			// remove short lines
			if (length < lineThresh) {
				vecs.erase(vecs.begin() + j);
				j--;
			}
		}

		/************************************** Uncomment to See the Result **************************************/
		//// Draw the lines
		//for (size_t i = 0; i < vecs.size(); i++) {
		//	cv::line(src2, cv::Point(vecs[i][0], vecs[i][1]), cv::Point(vecs[i][2], vecs[i][3]), cv::Scalar(255, 255, 255), 2, CV_AA);
		//	std::cout << cv::Point(vecs[i][0], vecs[i][1]) << " and " << cv::Point(vecs[i][2], vecs[i][3]) << std::endl;
		//}
		//cv::imwrite("2.png", src2);
		//cv::imshow("2", src2);
		//cv::waitKey(0);
		/*********************************************************************************************************/

		// Choose top line, bottom line, right line and left line
		std::vector<int> pt1x;
		std::vector<int> pt1y;
		for (size_t i = 0; i < vecs.size(); i++) {
			double x_diff = abs(vecs[i][2] - vecs[i][0]);
			double y_diff = abs(vecs[i][3] - vecs[i][1]);

			if (x_diff > y_diff) { // horizontal case
				pt1y.push_back(vecs[i][1]);
			}
			else { // vertical case
				pt1x.push_back(vecs[i][0]);
			}
		}

		auto xresult = minmax_element(pt1x.begin(), pt1x.end());
		auto yresult = minmax_element(pt1y.begin(), pt1y.end());

		std::vector<cv::Vec4i> newlines;
		for (size_t i = 0; i < vecs.size(); i++) {
			double x_diff = abs(vecs[i][2] - vecs[i][0]);
			double y_diff = abs(vecs[i][3] - vecs[i][1]);

			if (x_diff > y_diff) { // horizontal case
				if (vecs[i][1] == *(yresult.first) || vecs[i][1] == *(yresult.second)) {
					newlines.push_back(cv::Vec4i(vecs[i][0], vecs[i][1], vecs[i][2], vecs[i][3]));
				}
			}
			else { // vertical case
				if (vecs[i][0] == *(xresult.first) || vecs[i][0] == *(xresult.second)) {
					newlines.push_back(cv::Vec4i(vecs[i][0], vecs[i][1], vecs[i][2], vecs[i][3]));
				}
			}
		}

		/************************************** Uncomment to See the Result **************************************/
		//// Draw the lines
		//for (size_t i = 0; i < newlines.size(); i++) {
		//	cv::line(cdst, cv::Point(newlines[i][0], newlines[i][1]), cv::Point(newlines[i][2], newlines[i][3]), cv::Scalar(0, 0, 255), 1, CV_AA);
		//	std::cout << cv::Point(newlines[i][0], newlines[i][1]) << " and " << cv::Point(newlines[i][2], newlines[i][3]) << std::endl;
		//}

		//cv::imshow("3", cdst);
		//cv::waitKey(0);
		/*********************************************************************************************************/

		// Line extension
		std::vector<cv::Vec4i> finallines;
		int tmp = 0;
		for (size_t j = 0; j < newlines.size(); j++) {
			newlines[j][0] = newlines[j][0] + ((newlines[j][0] - newlines[j][2]) / floor((1.2 * src.rows) / 100));
			newlines[j][1] = newlines[j][1] + ((newlines[j][1] - newlines[j][3]) / floor((1.2 * src.rows) / 100));
			newlines[j][2] = newlines[j][2] + ((newlines[j][2] - newlines[j][0]) / floor((1.2 * src.rows) / 100));
			newlines[j][3] = newlines[j][3] + ((newlines[j][3] - newlines[j][1]) / floor((1.2 * src.rows) / 100));
			finallines.push_back(cv::Vec4i(newlines[j][0], newlines[j][1], newlines[j][2], newlines[j][3]));
		}

		/************************************** Uncomment to See the Result **************************************/
		//// Draw the lines
		//for (size_t i = 0; i < finallines.size(); i++) {
		//	cv::line(cdst, cv::Point(finallines[i][0], finallines[i][1]), cv::Point(finallines[i][2], finallines[i][3]), cv::Scalar(0, 0, 255), 1, CV_AA);
		//}

		//cv::imshow("3", cdst);
		//cv::waitKey(0);
		/*********************************************************************************************************/

		return finallines;
	}

	std::vector<cv::Point2i> CourtCorners(std::vector<cv::Vec4i> lines, int r, int c) {
		std::vector<cv::Point> lpoints;

		for (size_t j = 0; j < lines.size(); j++) {
			for (size_t i = j + 1; i < lines.size(); i++) {

				cv::Point pt = ComputeIntersection(cv::Point(lines[j][0], lines[j][1]),
					cv::Point(lines[j][2], lines[j][3]),
					cv::Point(lines[i][0], lines[i][1]),
					cv::Point(lines[i][2], lines[i][3]));

				// Erase every points that is higher than (100, 100) and lower than (height, width)
				if (pt.x > floor((20 * c) / 100) && pt.y > floor((20 * c) / 100)) {
					lpoints.push_back(pt);
				}
			}
		}

		std::vector<cv::Point2i> pts_src;
		pts_src.push_back(cv::Point(lpoints[0].x, lpoints[0].y));
		pts_src.push_back(cv::Point(lpoints[1].x, lpoints[1].y));
		pts_src.push_back(cv::Point(lpoints[2].x, lpoints[2].y));
		pts_src.push_back(cv::Point(lpoints[3].x, lpoints[3].y));

		return pts_src;
	}

	/* Create the Court Model */
	std::vector<cv::Vec4i> CreateCourtModel(std::string filename) {
		cv::VideoCapture stream(filename);

		/// Grab the first frame
		cv::Mat frame_rgb;
		stream.read(frame_rgb);

		/// Create a new matrix to hold the gray image
		cv::Mat frame_gray;

		/// Convert the frame from RGB to Grayscale
		cv::cvtColor(frame_rgb, frame_gray, CV_BGR2GRAY);

		/// Extract white pixel from the frame
		frame_gray = WhitePixelExtraction(frame_gray);

		/************************************** Uncomment to See the Result **************************************/
		cv::imshow("a", frame_gray);
		cv::waitKey(10);
		/*********************************************************************************************************/

		/// Calculate the false detection area and exclude the area
		frame_gray = CalculateGradients(frame_gray);

		/************************************** Uncomment to See the Result **************************************/
		//cv::imshow("b", frame_gray);
		//cv::waitKey(0);
		/*********************************************************************************************************/

		/// Detect the lines using hough transform
		std::vector<cv::Vec4i> lines;
		lines = LineDetection(frame_gray, frame_rgb);

		/************************************** Uncomment to See the Result **************************************/
		//// Draw the lines
		//for (size_t i = 0; i < lines.size(); i++) {
		//	cv::line(frame_rgb, cv::Point(lines[i][0], lines[i][1]), cv::Point(lines[i][2], lines[i][3]), cv::Scalar(0, 0, 255), 1, CV_AA);
		//}

		//cv::imshow("c", frame_rgb);
		//cv::waitKey(0);
		/*********************************************************************************************************/

		std::vector<cv::Point> corners;
		corners = CourtCorners(lines, frame_rgb.cols, frame_rgb.rows);

		/// Four corners of the badminton court model
		std::vector<cv::Point2i> pts_dst;
		pts_dst.push_back(cv::Point2i(14, 29)); // top left
		pts_dst.push_back(cv::Point2i(205, 29)); // top right
		pts_dst.push_back(cv::Point2i(14, 450)); // bottom left
		pts_dst.push_back(cv::Point2i(205, 450)); // bottom right

												  /// Calculate the homography matrix
		cv::Mat H = findHomography(pts_dst, corners, CV_RANSAC, 3);

		/// Write the homography matrix to a file
		cv::FileStorage storage("model/homography.yml", cv::FileStorage::WRITE);
		storage << "H" << H;
		storage.release();

		/// Insert the coordinates of model court lines
		std::vector<cv::Vec4i> mcourtline;
		mcourtline.push_back(cv::Vec4i(14, 29, 205, 29));
		mcourtline.push_back(cv::Vec4i(14, 53, 205, 53));
		mcourtline.push_back(cv::Vec4i(14, 177, 205, 177));
		mcourtline.push_back(cv::Vec4i(14, 303, 205, 303));
		mcourtline.push_back(cv::Vec4i(14, 426, 205, 426));
		mcourtline.push_back(cv::Vec4i(14, 450, 205, 450));
		mcourtline.push_back(cv::Vec4i(14, 29, 14, 450));
		mcourtline.push_back(cv::Vec4i(29, 29, 29, 450));
		mcourtline.push_back(cv::Vec4i(110, 29, 110, 177));
		mcourtline.push_back(cv::Vec4i(110, 303, 110, 450));
		mcourtline.push_back(cv::Vec4i(191, 29, 191, 450));
		mcourtline.push_back(cv::Vec4i(205, 29, 205, 450));

		/// Use the homography matrix to approximate the court lines
		std::vector<cv::Vec4i> courtline;
		for (size_t j = 0; j < 12; j++) {
			int x1 = mcourtline[j][0];
			int y1 = mcourtline[j][1];
			int x2 = mcourtline[j][2];
			int y2 = mcourtline[j][3];
			int u1 = (H.at<double>(0, 0)*x1 + H.at<double>(0, 1)*y1 + H.at<double>(0, 2)) / (H.at<double>(2, 0)*x1 + H.at<double>(2, 1)*y1 + H.at<double>(2, 2));
			int v1 = (H.at<double>(1, 0)*x1 + H.at<double>(1, 1)*y1 + H.at<double>(1, 2)) / (H.at<double>(2, 0)*x1 + H.at<double>(2, 1)*y1 + H.at<double>(2, 2));
			int u2 = (H.at<double>(0, 0)*x2 + H.at<double>(0, 1)*y2 + H.at<double>(0, 2)) / (H.at<double>(2, 0)*x2 + H.at<double>(2, 1)*y2 + H.at<double>(2, 2));
			int v2 = (H.at<double>(1, 0)*x2 + H.at<double>(1, 1)*y2 + H.at<double>(1, 2)) / (H.at<double>(2, 0)*x2 + H.at<double>(2, 1)*y2 + H.at<double>(2, 2));
			courtline.push_back(cv::Vec4i(u1, v1, u2, v2));
		}

		/****************************************************************************************************/
		// Draw the lines
		for (size_t i = 0; i < courtline.size(); i++) {
			cv::line(frame_rgb, cv::Point(courtline[i][0], courtline[i][1]), cv::Point(courtline[i][2], courtline[i][3]), cv::Scalar(255, 255, 255), 3, CV_AA);
		}

		cv::imshow("d", frame_rgb);
		cv::waitKey(10);
		/****************************************************************************************************/

		return courtline;
	}
} // namespace courtmodel

#endif COURTMODEL_H