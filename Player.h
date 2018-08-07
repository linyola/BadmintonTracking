#ifndef PLAYER_H
#define PLAYER_H

namespace player {
	cv::Mat TopPlayerFrame(cv::Mat src) {
		cv::Mat dst = src.clone();

		// constrains
		int left = floor((21 * src.cols) / 100);
		//std::cout << left << std::endl;
		int right = floor((79 * src.cols) / 100);
		//std::cout << right << std::endl;
		int top = floor((18 * src.rows) / 100);
		//std::cout << top << std::endl;
		int bottom = floor((58 * src.rows) / 100);
		//std::cout << bottom << std::endl;

		for (size_t j = 0; j < src.rows; j++) {
			for (size_t i = 0; i < src.cols; i++) {
				if (j < top || j > bottom) {
					dst.at<uchar>(j, i) = 255;
				}
				if (i < left || i > right) {
					dst.at<uchar>(j, i) = 255;
				}
			}
		}

		return dst;
	}

	cv::Mat BottomPlayerFrame(cv::Mat src) {
		cv::Mat dst = src.clone();

		// constrains
		int left = floor((21 * src.cols) / 100);
		//std::cout << left << std::endl;
		int right = floor((79 * src.cols) / 100);
		//std::cout << right << std::endl;
		int top = floor((45 * src.rows) / 100);
		//std::cout << top << std::endl;
		int bottom = floor((94 * src.rows) / 100);
		//std::cout << bottom << std::endl;

		for (size_t j = 0; j < src.rows; j++) {
			for (size_t i = 0; i < src.cols; i++) {
				if (j < top || j > bottom) {
					dst.at<uchar>(j, i) = 255;
				}
				if (i < left || i > right) {
					dst.at<uchar>(j, i) = 255;
				}
			}
		}

		return dst;
	}

	cv::Rect NormalizeTopROI(cv::Mat frm, cv::Rect boundingBox) {
		cv::Point center = cv::Point(boundingBox.x + (boundingBox.width / 2), boundingBox.y + (boundingBox.height / 2));
		cv::Rect returnRect = cv::Rect(center.x - 30, center.y - 45, 60, 90);

		if (returnRect.x < 0)returnRect.x = 0;
		if (returnRect.y < 0)returnRect.y = 0;
		if (returnRect.x + returnRect.width >= frm.cols)returnRect.width = frm.cols - returnRect.x;
		if (returnRect.y + returnRect.height >= frm.rows)returnRect.height = frm.rows - returnRect.y;

		return returnRect;
	}

	cv::Rect NormalizeBottomROI(cv::Mat frm, cv::Rect boundingBox) {
		cv::Point center = cv::Point(boundingBox.x + (boundingBox.width / 2), boundingBox.y + (boundingBox.height / 2));
		cv::Rect returnRect = cv::Rect(center.x - 35, center.y - 63, 70, 126);

		if (returnRect.x < 0)returnRect.x = 0;
		if (returnRect.y < 0)returnRect.y = 0;
		if (returnRect.x + returnRect.width >= frm.cols)returnRect.width = frm.cols - returnRect.x;
		if (returnRect.y + returnRect.height >= frm.rows)returnRect.height = frm.rows - returnRect.y;

		return returnRect;
	}

	cv::Rect TopPlayerRectEstimation(cv::Mat frame, cv::Ptr<cv::BackgroundSubtractor> gmm) {
		cvtColor(frame, frame, CV_BGR2GRAY);

		cv::Mat tpimage = frame.clone();

		// >>>>> Noise smoothing
		cv::GaussianBlur(frame, tpimage, cv::Size(3, 3), 3.0, 3.0);
		// <<<<< Noise smoothing

		tpimage = TopPlayerFrame(tpimage);

		// Background Subtraction
		cv::Mat fgMaskGMM;
		gmm->apply(tpimage, fgMaskGMM);

		cv::Mat skinimage;
		// shapes for morphology operators kernel
		cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
		cv::dilate(fgMaskGMM, skinimage, element, cv::Point(-1, -1), 2, 1, 1);
		cv::erode(skinimage, skinimage, element, cv::Point(-1, -1), 2, 1, 1);

		// Find contours
		cv::Mat contourimage;
		contourimage = skinimage.clone();
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		findContours(contourimage, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

		cv::Rect up_bounding_rect, up_bounding_rect2(tpimage.cols, tpimage.rows, 0, 0);

		/// Iterate through each contour
		for (size_t i = 0; i < contours.size(); i++) {
			//  Find the area of contour
			up_bounding_rect = boundingRect(contours[i]);
			if (up_bounding_rect.y < up_bounding_rect2.y && up_bounding_rect.width > 15 && up_bounding_rect.height > 15) {
				up_bounding_rect2 = up_bounding_rect;
			}
		}

		cv::Scalar color(255, 255, 255);

		// Check the size of the bounding box
		if (up_bounding_rect2.width < 60 && up_bounding_rect2.height < 90) {
			up_bounding_rect2 = NormalizeTopROI(tpimage, up_bounding_rect2);
		}

		return up_bounding_rect2;
	}

	cv::Rect BottomPlayerRectEstimation(cv::Mat frame, cv::Ptr<cv::BackgroundSubtractor> gmm) {
		cvtColor(frame, frame, CV_BGR2GRAY);

		cv::Mat bpimage = frame.clone();

		// >>>>> Noise smoothing
		cv::GaussianBlur(frame, bpimage, cv::Size(3, 3), 3.0, 3.0);
		// <<<<< Noise smoothing

		bpimage = BottomPlayerFrame(bpimage);


		// Background Subtraction
		cv::Mat fgMaskGMM;
		gmm->apply(bpimage, fgMaskGMM);

		cv::Mat skinimage;
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
		cv::dilate(fgMaskGMM, skinimage, element, cv::Point(-1, -1), 2, 1, 1);
		cv::erode(skinimage, skinimage, element, cv::Point(-1, -1), 2, 1, 1);

		// Find contours
		cv::Mat contourimage;
		contourimage = skinimage.clone();
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> bpimage_hierarchy;
		findContours(contourimage, contours, bpimage_hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

		cv::Rect bp_bounding_rect, bp_bounding_rect2(0, 0, 0, 0);

		// Iterate through each contour
		for (size_t i = 0; i < contours.size(); i++) {
			//  Find the area of contour
			bp_bounding_rect = boundingRect(contours[i]);
			if (bp_bounding_rect.y > bp_bounding_rect2.y && bp_bounding_rect.width > 15 && bp_bounding_rect.height > 15) {
				bp_bounding_rect2 = bp_bounding_rect;
			}
		}

		cv::Scalar color(255, 255, 255);

		// Check the size of the bounding box
		if (bp_bounding_rect2.width < 70 && bp_bounding_rect2.height < 126) {
			bp_bounding_rect2 = NormalizeBottomROI(frame, bp_bounding_rect2);
		}

		// Draw the largest contour using previously stored index
		cv::rectangle(frame, bp_bounding_rect2, color, 1, 8, 0);

		return bp_bounding_rect2;
	}

	std::vector<cv::Rect> PlayerRectExtraction(std::string filename) {
		cv::VideoCapture stream(filename);
		if (!stream.isOpened()) {
			// error in opening the video input
			std::cerr << "Unable to open video file: " << filename << std::endl;
			exit(EXIT_FAILURE);
		}

		cv::Mat image, pimage, rimage, tpimage, bpimage, stpimage, sbpimage, tpimage2, bpimage2;
		cv::Mat tpfgmaskGMM, bpfgmaskGMM;
		cv::Rect tplayer_rect, bplayer_rect;
		cv::Ptr<cv::BackgroundSubtractor> GMM; // Gaussian Mixture Model

		std::vector<cv::Rect> playersposition;

		GMM = cv::createBackgroundSubtractorMOG2(500, 16, false);

		int count = 0;
		while (true) {
			stream >> image;
			if (image.empty()) {
				//std::cout << "End...";
				break;
			}

			cvtColor(image, tpimage, CV_BGR2GRAY);
			cvtColor(image, bpimage, CV_BGR2GRAY);

			tplayer_rect = TopPlayerRectEstimation(image, GMM);
			bplayer_rect = BottomPlayerRectEstimation(image, GMM);

			playersposition.push_back(tplayer_rect);
			playersposition.push_back(bplayer_rect);
			count++;
		}

		return playersposition;
	}

	std::vector<cv::Rect> TopPlayerRectRefinement(std::vector<cv::Rect> inputrectangle, std::string filename) {
		cv::VideoCapture stream(filename);
		if (!stream.isOpened()) {
			std::cerr << "Unable to open video file: " << filename << " !" << std::endl;
			exit(EXIT_FAILURE);
		}

		// grab the first frame
		cv::Mat frame_rgb;
		stream.read(frame_rgb);

		std::vector<cv::Rect> outputrectangle;

		for (size_t i = 0; i < inputrectangle.size(); i++) {
			outputrectangle.push_back(inputrectangle[i]);
		}

		//for (size_t i = 0; i < inputrectangle.size(); i++) {
		//	// check and correct the x value
		//	if (inputrectangle[i].x == 0) {
		//		outputrectangle[i].x = (int)(frame_rgb.cols / 2) - (int)(4.1 * frame_rgb.cols / 100);
		//	}

		//	// check and correct the y value
		//	if (inputrectangle[i].y == 0) {
		//		outputrectangle[i].y = (int)(frame_rgb.rows / 2) - (int)(18.75 * frame_rgb.rows / 100);
		//	}

		//	// check and correct the width value
		//	if (inputrectangle[i].width < (5.86 * frame_rgb.cols / 100) || inputrectangle[i].width >(19.91 * frame_rgb.cols / 100)) {
		//		outputrectangle[i].width = (int)(15 * frame_rgb.cols / 100);
		//	}

		//	// check and correct the height value
		//	if (inputrectangle[i].height < (10.42 * frame_rgb.rows / 100) || inputrectangle[i].height >(35.42 * frame_rgb.rows / 100)) {
		//		outputrectangle[i].height = (int)(26.25 * frame_rgb.rows / 100);
		//	}
		//}

		// correction so that the rectangle not exceed the frame size
		for (size_t i = 0; i < outputrectangle.size(); i++) {
			if (frame_rgb.cols < outputrectangle[i].x + outputrectangle[i].width) {
				outputrectangle[i].x = outputrectangle[i].x - ((outputrectangle[i].x + outputrectangle[i].width) - frame_rgb.cols);
			}
		}
		for (size_t i = 0; i < outputrectangle.size(); i++) {
			if (frame_rgb.rows < outputrectangle[i].y + outputrectangle[i].height) {
				outputrectangle[i].y = outputrectangle[i].y - ((outputrectangle[i].y + outputrectangle[i].height) - frame_rgb.rows);
			}
		}

		return outputrectangle;
	}

	std::vector<cv::Rect> BottomPlayerRectRefinement(std::vector<cv::Rect> inputrectangle, std::string filename) {
		cv::VideoCapture stream(filename);
		if (!stream.isOpened()) {
			std::cerr << "Unable to open video file: " << filename << " !" << std::endl;
			exit(EXIT_FAILURE);
		}

		// grab the first frame
		cv::Mat frame_rgb;
		stream.read(frame_rgb);

		std::vector<cv::Rect> outputrectangle;

		for (size_t i = 0; i < inputrectangle.size(); i++) {
			outputrectangle.push_back(inputrectangle[i]);
		}

		//for (size_t i = 0; i < inputrectangle.size(); i++) {
		//	// Check and correct the x value
		//	if (inputrectangle[i].x == 0) {
		//		outputrectangle[i].x = (int)(frame_rgb.cols / 2) - (int)(4.1 * frame_rgb.cols / 100);
		//	}

		//	// Check and correct the y value
		//	if (inputrectangle[i].y == 0) {
		//		outputrectangle[i].y = (int)(frame_rgb.rows / 2);
		//	}

		//	// Check and correct the width value
		//	if (inputrectangle[i].width < (5.86 * frame_rgb.cols / 100) || inputrectangle[i].width >(19.91 * frame_rgb.cols / 100)) {
		//		outputrectangle[i].width = (int)(15 * frame_rgb.cols / 100);
		//	}

		//	// Check and correct the height value
		//	if (inputrectangle[i].height < (10.42 * frame_rgb.rows / 100) || inputrectangle[i].height >(35.42 * frame_rgb.rows / 100)) {
		//		outputrectangle[i].height = (int)(26.25 * frame_rgb.rows / 100);
		//	}
		//}

		// Correction so that the rectangle not exceed the frame size
		for (size_t i = 0; i < outputrectangle.size(); i++) {
			if (frame_rgb.cols < outputrectangle[i].x + outputrectangle[i].width) {
				outputrectangle[i].x = outputrectangle[i].x - ((outputrectangle[i].x + outputrectangle[i].width) - frame_rgb.cols);
			}
		}
		for (size_t i = 0; i < outputrectangle.size(); i++) {
			if (frame_rgb.rows < outputrectangle[i].y + outputrectangle[i].height) {
				outputrectangle[i].y = outputrectangle[i].y - ((outputrectangle[i].y + outputrectangle[i].height) - frame_rgb.rows);
			}
		}

		return outputrectangle;
	}

	std::vector<cv::Vec8i> PositionsExtraction(std::string filename) {
		cv::VideoCapture stream(filename);
		if (!stream.isOpened()) {
			std::cerr << "Unable to open video file: " << filename << " !" << std::endl;
			exit(EXIT_FAILURE);
		}

		// first frame
		cv::Mat frame_rgb;

		std::vector<cv::Rect> playersrect = PlayerRectExtraction(filename);
		std::vector<cv::Rect> topplayerrect;
		std::vector<cv::Rect> bottomplayerrect;

		for (size_t i = 0; i < playersrect.size(); i += 2) {
			topplayerrect.push_back(playersrect[i]);
			bottomplayerrect.push_back(playersrect[i + 1]);
		}

		std::vector<cv::Rect> rtopplayerrect = TopPlayerRectRefinement(topplayerrect, filename);
		std::vector<cv::Rect> rbottomplayerrect = BottomPlayerRectRefinement(bottomplayerrect, filename);

		std::vector<cv::Vec8i> playerpositions;

		for (size_t i = 0; i < rtopplayerrect.size(); i++) {
			int tx = rtopplayerrect[i].x;
			int ty = rtopplayerrect[i].y;
			int tw = rtopplayerrect[i].width;
			int th = rtopplayerrect[i].height;
			int bx = rbottomplayerrect[i].x;
			int by = rbottomplayerrect[i].y;
			int bw = rbottomplayerrect[i].width;
			int bh = rbottomplayerrect[i].height;
			playerpositions.push_back(cv::Vec8i(tx, ty, tw, th, bx, by, bw, bh));
		}

		// save the player detection and tracking result to a file
		std::ofstream ofs;
		ofs.open("results/playerpositions.txt");
		for (size_t i = 0; i < playerpositions.size(); i++) {
			ofs << playerpositions[i][0] << " " << playerpositions[i][1] << " "
				<< playerpositions[i][2] << " " << playerpositions[i][3] << " "
				<< playerpositions[i][4] << " " << playerpositions[i][5] << " "
				<< playerpositions[i][6] << " " << playerpositions[i][7] << " "
				<< std::endl;

		}
		ofs.clear(); ofs.close();

		return playerpositions;
	}
} // namespace stroke

#endif PLAYER_H