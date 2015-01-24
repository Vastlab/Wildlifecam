///============================================================================
/// Name        : Squirrel_Detection_pipeline.cpp
/// Authors      : Brian Heflin & RC Johnson
/// Version     :
/// Copyright   : Copyright Securics Inc
/// Description : Squirrel pipeline code. Can be run in OSX, Linux, and Windows.
///============================================================================
#pragma once

//#define WRITE_2_FILE
#define DEBUG
#define ADVANCED_DEBUG

#include "Squirrel_pipeline.h"

#define LOG_TAG "SQUIRREL_JNI"

#define SQUIRREL_PIPELINE_CPP

#ifdef __ANDROID__
#include <android/log.h>
#define ANDROID_LOG_VERBOSE ANDROID_LOG_DEBUG
#define LOGV(...) __android_log_print(ANDROID_LOG_SILENT, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#define LOGI( fmt, ...) fprintf( stderr, "\nI : %s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGV( fmt, ...) fprintf( stderr, "\nV : %s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGE( fmt, ...) fprintf( stderr, "\nE : %s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

static int DEBUG_LEVEL = 1;

using namespace std;
using namespace cv;

typedef unsigned int uint;

static int THRESHOLD_COUNTER = 0;

sConfig myConfig;

static int IMAGE_COUNTER = 1;

bool checkROIAssertion(cv::Mat m, cv::Rect roi) {

	if ((0 <= roi.x && 0 <= roi.width && roi.x + roi.width <= m.cols
			&& 0 <= roi.y && 0 <= roi.height && roi.y + roi.height <= m.rows
			&& roi.height > 0 && roi.width > 0))
		return true;
	else
		return false;
}

bool updateConfig(sConfig &config) {

	myConfig = config;
	return true;
}

void LOGGING(int level, const char *fmt, ...) {

	//LOGI("Log File: %s",myConfig.log_file);

	if (strlen(myConfig.log_file) < 1)
		return; //can't log, so just quit
	char formatted_string[256];

	FILE *fp = fopen(myConfig.log_file, "a");
	if (!fp) {
		LOGE("Couldn't open log file");
		return;
	}
	time_t rawtime;
	time(&rawtime);
	char mTime[256];
	sprintf(mTime, "%s", ctime(&rawtime));
	mTime[strlen(mTime) - 1] = '\0';

	va_list argptr;
	va_start(argptr, fmt);

	char myMessage[512];

	if (level >= DEBUG_LEVEL) {
		fprintf(fp, "[%s] CVJNI [Device: %s]  ", mTime, myConfig.myID);
		vfprintf(fp, fmt, argptr);
		fprintf(fp, "\n\n");

#ifdef DEBUG
		//fprintf(stdout, "[%s] CVJNI [Device: %s]  ", mTime, myConfig.myID);
		//vfprintf(stdout, fmt, argptr);
		//fprintf(stdout, "\n\n");
#endif
		//vsprintf(myMessage, fmt, argptr);
		//LOGI(myMessage);

	}
	va_end(argptr);

	fclose(fp);
}

/**
 * Find_Bounding_Boxes
 *
 * This module locates the bounding boxes around the blobs
 *
 * @param  Mat output - holds boxes found
 * @param  vector<vector<cv::Point2i> > blobs - the blob data that the boxes will be formed around
 * @param  vector<int> &minXArr - minimum X values for boxes
 * @param  vector<int> &minYArr - minimum Y values for boxes
 * @param  vector<int> &maxXArr - maximum X values for boxes
 * @param  vector<int> &maxYArr - maximum Y values for boxes
 * @param  int parentImageCols - columns of parent image
 * @param  int parentImageRows - rows of parent image
 * @return void
 */

void Find_Bounding_Boxes(Mat output, vector<vector<cv::Point2i> > blobs,
		vector<int> &minXArr, vector<int> &minYArr, vector<int> &maxXArr,
		vector<int> &maxYArr, int parentImageCols, int parentImageRows) {

	//Initialize the arrays
	for (unsigned int i = 0; i < blobs.size(); i++) {

		minXArr.push_back(parentImageCols);
		minYArr.push_back(parentImageRows);
		maxXArr.push_back(0);
		maxYArr.push_back(0);
	}

	// Randomly color the blobs
	for (int labels = 0; labels < (int) blobs.size(); labels++) {
		unsigned char r = 255 * (rand() / (1.0 + RAND_MAX));
		unsigned char g = 255 * (rand() / (1.0 + RAND_MAX));
		unsigned char b = 255 * (rand() / (1.0 + RAND_MAX));

		vector < cv::Point2i > blobVector = blobs[labels];

		int terminator = blobVector.size();

		for (int j = 0; j < terminator; j++) {

			int x = blobs[labels][j].x;
			int y = blobs[labels][j].y;

			int temp_min_x = minXArr[labels];
			int temp_min_y = minYArr[labels];
			int temp_max_x = maxXArr[labels];
			int temp_max_y = maxYArr[labels];

			if (y > temp_max_y)
				maxYArr[labels] = y;

			if (y < temp_min_y)
				minYArr[labels] = y;

			if (x > temp_max_x)
				maxXArr[labels] = x;

			if (x < temp_min_x)
				minXArr[labels] = x;

			//randomly color the output
			output.at < cv::Vec3b > (y, x)[0] = b;
			output.at < cv::Vec3b > (y, x)[1] = g;
			output.at < cv::Vec3b > (y, x)[2] = r;

		}
	}

	return;
}

/**
 * Find_Rectangles
 *
 * This module makes rectangles out of the bounding boxes
 *
 * @param  vector<cv::Rect> &boundingBoxes - the rectangles created from the bounding boxes
 * @param  Mat colorLabels - image that the randomly colored boxes will be placed on
 * @param  vector<vector<cv::Point2i> > blobs - REMOVE? number of blobs should equal number of items in min/max arrays
 * @param  vector<int> &minYArr - minimum Y values for boxes
 * @param  vector<int> &maxXArr - maximum X values for boxes
 * @param  vector<int> &maxYArr - maximum Y values for boxes
 * @return void
 */
void Find_Rectangles(vector<cv::Rect> &boundingBoxes, Mat colorLabels,
		vector<vector<cv::Point2i> > blobs, vector<int> minXArr,
		vector<int> minYArr, vector<int> maxXArr, vector<int> maxYArr,
		double minAreaQCC) {

	for (unsigned int i = 0; i < blobs.size(); i++) {

		int tempLabel = i;

		CvRect tempRect;

		tempRect.x = minXArr[tempLabel];
		tempRect.y = minYArr[tempLabel];
		tempRect.width = (maxXArr[tempLabel] - minXArr[tempLabel]) + 1;
		tempRect.height = (maxYArr[tempLabel] - minYArr[tempLabel]) + 1;

		//threshold the rectangle on size before putting into return vector
		//check to make sure myROI isn't too big or small
		double mArea = tempRect.height * tempRect.width;

		if (mArea >= minAreaQCC) {

			CvPoint pt1 = cvPoint(tempRect.x, tempRect.y);
			CvPoint pt2 = cvPoint(tempRect.x + tempRect.width,
					tempRect.y + tempRect.height);

			unsigned char r = 255 * (rand() / (1.0 + RAND_MAX));
			unsigned char g = 255 * (rand() / (1.0 + RAND_MAX));
			unsigned char b = 255 * (rand() / (1.0 + RAND_MAX));

			cv::rectangle(colorLabels, pt1, pt2, cvScalar(b, g, r), 0, 1, 0);

			boundingBoxes.push_back(
					cvRect(tempRect.x, tempRect.y, tempRect.width,
							tempRect.height));
		}
	}

	return;
}

void Update_Threshold_Lighting(Mat Thresholds, int i, int j, int scale,
		int GLOBAL_DIFF) {

	//update the threshold with the global difference
	//then GLOBAL THRESHOLD will be reset in state machine
	if (Thresholds.at < uchar > (i, j) <= (255 - GLOBAL_DIFF)) {
		Thresholds.at < uchar > (i, j) += (uchar) GLOBAL_DIFF;
	} else
		Thresholds.at < uchar > (i, j) = (uchar) 255;

}

/**
 * Update_Threshold
 *
 * This module updates the threshold of one pixel.
 * This is a helper function.
 *
 * @param  Mat classificationMap - image that holds the svm classification
 * @param  Mat Thresholds - the threshold image
 * @param  Mat Blobs - the blob filled image
 * @param  int i - image location
 * @param  int j - image location
 * @param  int scale - image scale
 * @param  int minValue - minimum value for threshold
 * @return void
 */

void Update_Threshold(Mat classificationMap, Mat Thresholds, Mat Blobs, int i,
		int j, int scale, int minValue, bool DECREMENT_THRESHOLDS,
		int THRESHOLD_DECREMENT_VALUE, int THRESHOLD_UPDATE_NEG_EXEMPLAR,
		int THRESHOLD_UPDATE_POS_EXEMPLAR, int THRESHOLD_UPDATE_TOO_BIG_SMALL) {

	if (DECREMENT_THRESHOLDS) {
		if (Thresholds.at < uchar > (i, j) > THRESHOLD_DECREMENT_VALUE) {
			Thresholds.at < uchar > (i, j) -= THRESHOLD_DECREMENT_VALUE;
		}
	} else {
		if (Blobs.at < uchar > (i / scale, j / scale) > 0) {

			if (classificationMap.at < uchar > (i, j) == (uchar) 0) { // -1
				if (Thresholds.at < uchar > (i, j)
						<= 255 - THRESHOLD_UPDATE_NEG_EXEMPLAR)
					Thresholds.at < uchar > (i, j) +=
							THRESHOLD_UPDATE_NEG_EXEMPLAR;
				else
					Thresholds.at < uchar > (i, j) = (uchar) 255;

			} else if (classificationMap.at < uchar > (i, j) == (uchar) 255) { // 1
				if (Thresholds.at < uchar > (i, j)
						> THRESHOLD_UPDATE_POS_EXEMPLAR - 1)
					Thresholds.at < uchar > (i, j) -=
							THRESHOLD_UPDATE_POS_EXEMPLAR;
			} else if (classificationMap.at < uchar > (i, j) == (uchar) 10) {
				if (Thresholds.at < uchar > (i, j)
						<= 255 - THRESHOLD_UPDATE_TOO_BIG_SMALL)
					Thresholds.at < uchar > (i, j) +=
							THRESHOLD_UPDATE_TOO_BIG_SMALL;
				else
					Thresholds.at < uchar > (i, j) = (uchar) 255;
			}

		} //if blobs

		else { //means weren't detected

			if (classificationMap.at < uchar > (i, j) == (uchar) 255) { // 1
				if (Thresholds.at < uchar > (i, j)
						> THRESHOLD_UPDATE_POS_EXEMPLAR - 1)
					Thresholds.at < uchar > (i, j) -=
							THRESHOLD_UPDATE_POS_EXEMPLAR;
			} else if (classificationMap.at < uchar > (i, j) == (uchar) 10) {
				if (Thresholds.at < uchar > (i, j)
						<= 255 - THRESHOLD_UPDATE_TOO_BIG_SMALL)
					Thresholds.at < uchar > (i, j) +=
							THRESHOLD_UPDATE_TOO_BIG_SMALL;
				else
					Thresholds.at < uchar > (i, j) = (uchar) 255;
			}
		} // no blobs
	}
}

/**
 * Update_Background_Incremental
 *
 * This module updates the background using the incremental model
 * This is a helper function.
 *
 * @param  Mat inputFrame - newly received image
 * @param  Mat Background1 - first background image
 * @param  int i - image location
 * @param  int j - image location
 * @return void
 */

void Update_Background_Incremental(Mat inputFrame, Mat Background1, int i,
		int j, int UPDATE_BACKGROUND_INC) {
	if (inputFrame.at < uchar > (i, j) < Background1.at < uchar > (i, j)
			&& (Background1.at < uchar > (i, j) > UPDATE_BACKGROUND_INC - 1)) {
		Background1.at < uchar > (i, j) = Background1.at < uchar > (i, j) - 1;
	} else if (inputFrame.at < uchar > (i, j) > Background1.at < uchar > (i, j)
			&& (Background1.at < uchar > (i, j) < 255 - UPDATE_BACKGROUND_INC)) {
		Background1.at < uchar > (i, j) = Background1.at < uchar > (i, j) + 1;
	}
	//else
	//do nothing they are equal

}

/**
 * Update_Background_Blend1IMAGE_COUNTER
 *
 * This module updates the background using the first blending method
 * This is a helper function.
 *
 * @param  Mat inputFrame - newly received image
 * @param  Mat Background1 - first background image
 * @param  Mat Background2 - second  background image
 * @param  int aplha - blending model
 * @param  int i - image location
 * @param  int j - image location
 * @return void
 */

void Update_Background_Blend1(Mat inputFrame, Mat Background1, Mat Background2,
		int alpha, int i, int j) {

	float tempPixel;

	if (alpha == 0) {
		tempPixel =
				((float) Background1.at < uchar > (i, j) * (float) (7.0 / 8.0))
						+ ((float) inputFrame.at < uchar
								> (i, j) * (float) (1.0 / 8.0));

		Background1.at < uchar > (i, j) = (uchar) tempPixel;
	}
	if (alpha == 1) {
		tempPixel =
				((float) Background1.at < uchar > (i, j) * (float) (3.0 / 4.0))
						+ ((float) inputFrame.at < uchar
								> (i, j) * (float) (1.0 / 4.0));

		Background1.at < uchar > (i, j) = (uchar) tempPixel;
	}
	if (alpha == 2) {
		tempPixel =
				((float) Background1.at < uchar > (i, j) * (float) (7.0 / 8.0))
						+ ((float) inputFrame.at < uchar
								> (i, j) * (float) (1.0 / 8.0));

		Background1.at < uchar > (i, j) = (uchar) tempPixel;

		tempPixel =
				((float) Background2.at < uchar > (i, j) * (float) (7.0 / 8.0))
						+ ((float) inputFrame.at < uchar
								> (i, j) * (float) (1.0 / 8.0));

		Background2.at < uchar > (i, j) = (uchar) tempPixel;
	}

}

void Update_Threshold_Background(int operationMode, Mat inputFrame,
		Mat Background1, Mat Background2, Mat Thresholds, Mat Blobs,
		Mat classificationMap, int *GLOBAL_THRESHOLD, int UPDATE_NUMBER,
		int GLOBAL_INCREMENT, int GLOBAL_THRESHOLD_INIT,
		int THRESHOLD_DECREMENT_NUMBER, int THRESHOLD_DECREMENT_VALUE,
		int THRESHOLD_UPDATE_NEG_EXEMPLAR, int THRESHOLD_UPDATE_POS_EXEMPLAR,
		int THRESHOLD_UPDATE_TOO_BIG_SMALL, int UPDATE_BACKGROUND_INC) {

	//If operationMode:
	// 0 = Normal operation
	// 1 = Lighting or Motion Change
	// 2 = Global Increment successful
	// 3 = Global Increment not successful
	// 4 = first frame or re-initializing

	//Threshold Initialization: The initial threshold is initialized by taking the MAX PIXEL VALUE-MIN PIXEL
	//VALUE in a 3x3 pixel grid of the current image frame.

	//Background Initialization: Both backgrounds are also initialized in the threshold initialization block where

	//Background1= MIN PIXEL VALUE in a 3x3 pixel grid with the current pixel at the center
	//Background2= MAX PIXEL VALUE in a 3x3 pixel grid with the current pixel at the center

	double maxValue;
	double minValue;
	double max_min_Value;

	static bool absolute_update;
	static bool non_target_update;
	static int alphaFactor;
	static bool first_frame;
	static bool DECREMENT_THRESHOLDS;

	int GLOBAL_DIFF = abs(*GLOBAL_THRESHOLD - GLOBAL_THRESHOLD_INIT);

	int Process_Counter = 0;		//updated when a pixel is updated

	int scale = inputFrame.rows / Blobs.rows;

	switch (operationMode) {
	// 0 = Normal operation
	case 0:
		*GLOBAL_THRESHOLD = GLOBAL_THRESHOLD_INIT;
		absolute_update = false;
		non_target_update = false;
		alphaFactor = 0;
		first_frame = false;
		break;

		// 1 = Lighting or Motion Change
	case 1:
		absolute_update = true;
		non_target_update = false;
		alphaFactor = 0;
		first_frame = false;
		*GLOBAL_THRESHOLD += GLOBAL_INCREMENT;
		break;

		// 2 = Global Increment successful
	case 2:
		absolute_update = false;
		non_target_update = true;
		alphaFactor = 1;
		first_frame = false;
		break;

		// 3 = Global Increment not successful
	case 3:
		absolute_update = true;
		non_target_update = false;
		alphaFactor = 2;
		first_frame = false;
		break;

		// 4 = first frame or re-initializing
	case 4:
		absolute_update = false;
		non_target_update = false;
		alphaFactor = 0;
		first_frame = true;
		break;

	default:
		*GLOBAL_THRESHOLD = GLOBAL_THRESHOLD_INIT;
		absolute_update = false;
		non_target_update = false;
		alphaFactor = 0;
		first_frame = false;
		break;
	}

	if (first_frame == true && operationMode == 4) { // (means first frame or re-initialization)

	//Initialize the background and thresholds
		for (int i = 1; i < inputFrame.rows - 1; i++)
			for (int j = 1; j < inputFrame.cols - 1; j++) {
				cv::Rect myROI(j - 1, i - 1, 2, 2);

				Mat crop(inputFrame, myROI); // NOTE: this will only give you a reference to the ROI of the original data

				//find the max/min and max-min value
				cv::minMaxLoc(crop, &minValue, &maxValue); //find minimum and maximum intensities
				max_min_Value = maxValue - minValue;

				//assign values to the backgrounds and threshold
				Background1.at < uchar > (i, j) = (uchar) minValue;
				Background2.at < uchar > (i, j) = (uchar) maxValue;
				Thresholds.at < uchar > (i, j) = (uchar) max_min_Value;
			}

	} //if first_frame == true
	else {
		THRESHOLD_COUNTER++;
		//Initialize the background and thresholds
		for (int i = 0; i < inputFrame.rows; i++)
			for (int j = 0; j < inputFrame.cols; j++) {

				Process_Counter++;

				Process_Counter %= UPDATE_NUMBER;

				if (Process_Counter == 0 || absolute_update == true
						|| non_target_update == true) {

					//normal operation
					if (operationMode == 0) {

						//Update Background using Incremental Model
						//Update_Background_Incremental(inputFrame, Background1,
						//	i, j, UPDATE_BACKGROUND_INC);
						//Alpha Blending model level 1: non-target
						Update_Background_Blend1(inputFrame, Background1,
								Background2, alphaFactor, i, j);

					} else if (operationMode == 1) { // lighting or motion change

						//Update Background using Incremental Model
						Update_Background_Incremental(inputFrame, Background1,
								i, j, UPDATE_BACKGROUND_INC);

					} else if (operationMode == 2) { // global increment successful
						//Alpha Blending model level 1: non-target
						Update_Background_Blend1(inputFrame, Background1,
								Background2, alphaFactor, i, j);

					} else if (operationMode == 3) { // global inc fail
						//Alpha Blending model level 2: all
						Update_Background_Blend1(inputFrame, Background1,
								Background2, alphaFactor, i, j);
					}

				} //process == fu

				//Update Thresholds - always the same
				if (operationMode == 2 || operationMode == 3) {
					// 2 = Global Increment successful
					Update_Threshold_Lighting(Thresholds, i, j, scale,
							GLOBAL_DIFF);

				} else {
					if (THRESHOLD_COUNTER == THRESHOLD_DECREMENT_NUMBER)
						DECREMENT_THRESHOLDS = true;
					else
						DECREMENT_THRESHOLDS = false;
				}

			} //j

		if (DECREMENT_THRESHOLDS == false || DECREMENT_THRESHOLDS == true) {
			//Initialize the background and thresholds
			for (int i = 1; i < inputFrame.rows - 1; i++)
				for (int j = 1; j < inputFrame.cols - 1; j++) {
					cv::Rect myROI(j - 1, i - 1, 2, 2);

					Mat crop(inputFrame, myROI); // NOTE: this will only give you a reference to the ROI of the original data

					//find the max/min and max-min value
					cv::minMaxLoc(crop, &minValue, &maxValue); //find minimum and maximum intensities
					max_min_Value = maxValue - minValue;

					//assign values to threshold
					Thresholds.at < uchar > (i, j) = (uchar) max_min_Value;
				}
		}

	} // not first frame

	if (THRESHOLD_COUNTER == THRESHOLD_DECREMENT_NUMBER)
		THRESHOLD_COUNTER = 0;

}
/**
 * Background_Subtraction
 *
 * This module does the background subtraction and lists all changes detected that are above certain thresholds.
 *
 * @param  Mat inputFrame - newly received image
 * @param  Mat Background1 - first background image
 * @param  Mat Background2 - second  background image
 * @param  Mat Thresholds - the threshold image
 * @param  Mat Detected_Image_HI - image contains all pixels that were detected and are above the high threshold
 * @param  Mat Detected_Image_LOW - image contains all pixels that were detected and are above the low threshold
 * @param  int GLOBAL_THRESHOLD - used to modify all thresholds
 * @param  int HI_THRESHOLD_INC - increses the high threshold
 * @return void
 */

void Background_Subtraction(Mat diffImage, Mat inputFrame, Mat Background1,
		Mat Background2, Mat Thresholds, Mat Detected_Image_HI,
		Mat Detected_Image_LOW, int* GLOBAL_THRESHOLD, int HI_THRESHOLD_INC) {

	//do the business
	uchar abs_b1, abs_b2, temp_b1, temp_b2;

	for (int i = 1; i < inputFrame.rows - 1; i++)
		for (int j = 1; j < inputFrame.cols - 1; j++) {

			abs_b1 = abs(
					inputFrame.at < uchar > (i, j) - Background1.at < uchar
							> (i, j));
			abs_b2 = abs(
					inputFrame.at < uchar > (i, j) - Background2.at < uchar
							> (i, j));

			if (abs_b2 < abs_b1) {
				//if (abs_b2 > abs_b1) {
				//switch the background pixels
				temp_b1 = Background1.at < uchar > (i, j);
				temp_b2 = Background2.at < uchar > (i, j);

				Background1.at < uchar > (i, j) = temp_b2;
				Background2.at < uchar > (i, j) = temp_b1;

				diffImage.at < uchar > (i, j) = abs_b1;
			}

			else {
				diffImage.at < uchar > (i, j) = abs_b2;

			}

			//threshold HI
			if (diffImage.at < uchar > (i, j)
					>= (Thresholds.at < uchar
							> (i, j) + (int) *GLOBAL_THRESHOLD
									+ (uchar) HI_THRESHOLD_INC)) {
				Detected_Image_HI.at < uchar > (i, j) = 255;
				Detected_Image_LOW.at < uchar > (i, j) = 0;
			}
			//threshold LOW
			else if (diffImage.at < uchar > (i, j) > Thresholds.at < uchar
					> (i, j) + (int) *GLOBAL_THRESHOLD) {
				Detected_Image_HI.at < uchar > (i, j) = 0;
				Detected_Image_LOW.at < uchar > (i, j) = 255;
			}
			//not detected
			else {
				Detected_Image_LOW.at < uchar > (i, j) = 0;
				Detected_Image_HI.at < uchar > (i, j) = 0;
			}
		}
}

/**
 * QCC
 *
 * This modiule performs QCC: if region has enough pixels above threshold, mark region
 *
 * @param  Mat imgHi - image contains pixels above high threshold
 * @param  Mat imgLow - image contains pixels above low threshold
 * @param  Mat parentImage - image QCC output
 * @param  int boxSize - size of region boxes
 * @return void
 */

void QCC(Mat imgHi, Mat imgLow, Mat parentImage, int boxSize,
		double percentPixelsNeededHighOnly, double percentPixelsNeededHigh,
		double percentPixelsNeededLow) {

	CvScalar hiCount, lowCount;
	CvRect r;

	int parentImageI = 0;
	int parentImageJ = 0;

	for (int i = 0; i < imgHi.rows; i += boxSize) {
		for (int j = 0; j < imgHi.cols; j += boxSize) {

			r = cvRect(j, i, boxSize, boxSize);

			if (i + boxSize > imgHi.rows)
				r.height = imgHi.rows - i;

			if (j + boxSize > imgHi.cols)
				r.width = imgHi.cols - j;

			cv::Rect myROI(r);

			Mat cropHi(imgHi, myROI);
			Mat cropLow(imgLow, myROI);

			//threshold on % of hi/low pixels in the ROI
			hiCount = cv::sum(cropHi);
			lowCount = cv::sum(cropLow);

			double boxArea = myROI.width * myROI.height;

			double numPixelsNeededHighOnly = boxArea
					* percentPixelsNeededHighOnly;
			double numPixelsNeededHigh = boxArea * percentPixelsNeededHigh;
			double numPixelsNeededLow = boxArea * percentPixelsNeededLow;

			//check conditions for detected region to form binary image
			//QCC Threshold Parameters
			if ((hiCount.val[0] >= numPixelsNeededHigh
					&& lowCount.val[0] >= numPixelsNeededLow)
					|| hiCount.val[0] >= numPixelsNeededHighOnly) {
				parentImage.at < uchar > (parentImageI, parentImageJ) =
						(uchar) 255;
			}

			else {
				parentImage.at < uchar > (parentImageI, parentImageJ) =
						(uchar) 0;
			}

			parentImageJ++;

			if (parentImageJ == parentImage.cols) {
				parentImageJ = 0;
				parentImageI++;
			}

		}
	}

#ifdef DEBUG
	imshow("parent", parentImage);
#endif
}

/**
 * FindBlobs
 *
 * This module locates blobs in an image
 *
 * @param  const cv::Mat &binary - binary image of data on pixels above threshold
 * @param  std::vector<std::vector<cv::Point2i> > &blobs - blobs that are found from binary image
 * @return void
 */

void FindBlobs(const cv::Mat &binary,
		std::vector<std::vector<cv::Point2i> > &blobs) {
	blobs.clear();

	// Fill the label_image with the blobs
	// 0  - background
	// 1  - Unlabeled foreground
	// 2+ - Labeled foreground

	cv::Mat label_image;
	binary.convertTo(label_image, CV_32FC1); // weird it doesn't support CV_32S!

	int label_count = 2;		// starts at 2 because 0,1 are used already

	for (int y = 0; y < binary.rows; y++) {
		for (int x = 0; x < binary.cols; x++) {
			if ((int) label_image.at<float>(y, x) != 1) {
				continue;
			}

			cv::Rect rect;
			cv::floodFill(label_image, cv::Point(x, y), cv::Scalar(label_count),
					&rect, cv::Scalar(0), cv::Scalar(0), 4);

			std::vector < cv::Point2i > blob;

			for (int i = rect.y; i < (rect.y + rect.height); i++) {
				for (int j = rect.x; j < (rect.x + rect.width); j++) {
					if ((int) label_image.at<float>(i, j) != label_count) {
						continue;
					}

					blob.push_back(cv::Point2i(j, i));
				}
			}

			blobs.push_back(blob);

			label_count++;
		}
	}

}

void runCommand(char *cmd, char *response = NULL) {

	LOGGING(1, "\nRunning Command:\n\"%s\"\n", cmd);
	FILE* pipe = popen(cmd, "r");
#ifndef __ANDROID__
	if (!pipe)
		exit(-1);
#else
	if(!pipe) LOGGING(1,"Error running command!!");
#endif
	char buffer[128];
	if (response)
		response[0] = '\0';

	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL) {
			if (response)
				strcat(response, buffer);
			else
				fprintf(stdout, "%s", buffer);
		}
	}
	pclose(pipe);

}

//Tests two boundary boxes for overlap (using the intersection over union metric) and returns the overlap if the objects
//defined by the two bounding boxes are considered to be matched according to the criterion outlined in
//the VOC documentation [namely intersection/union > some threshold] otherwise returns -1.0 (no match)
//----------------------------------------------------------------------------------------------------------
float testBoundingBoxesForOverlap(const Rect detection,
		const Rect ground_truth) {

	int detection_x2 = detection.x + detection.width;
	int detection_y2 = detection.y + detection.height;
	int ground_truth_x2 = ground_truth.x + ground_truth.width;
	int ground_truth_y2 = ground_truth.y + ground_truth.height;

	//first calculate the boundaries of the intersection of the rectangles
	int intersection_x = std::max(detection.x, ground_truth.x);	//rightmost left
	int intersection_y = std::max(detection.y, ground_truth.y);	//bottommost top
	int intersection_x2 = std::min(detection_x2, ground_truth_x2);//leftmost right
	int intersection_y2 = std::min(detection_y2, ground_truth_y2);//topmost bottom

	//check for total overlap
	if (intersection_x == detection.x && intersection_y == detection.y
			&& intersection_x2 == detection_x2
			&& intersection_y2 == detection_y2)
		return 1.0;

	if (intersection_x == ground_truth.x && intersection_y == ground_truth.y
			&& intersection_x2 == ground_truth_x2
			&& intersection_y2 == ground_truth_y2)
		return 1.0;

	//then calculate the width and height of the intersection rect
	int intersection_width = (intersection_x2 - intersection_x);
	int intersection_height = (intersection_y2 - intersection_y);

	//if there is no overlap then return false straight away
	if ((intersection_width <= 0) || (intersection_height <= 0))
		return -1.0;

	//otherwise calculate the intersection
	int intersection_area = intersection_width * intersection_height;

	//now calculate the union
	int union_x = std::min(detection.x, ground_truth.x);		//rightmost left
	int union_y = std::min(detection.y, ground_truth.y);		//bottommost top
	int union_x2 = std::max(detection_x2, ground_truth_x2);		//leftmost right
	int union_y2 = std::max(detection_y2, ground_truth_y2);		//topmost bottom
	//then calculate the width and height of the intersection rect

	int union_width = (union_x2 - (union_x));
	int union_height = (union_y2 - (union_y));

	int union_area = union_width * union_height;

	int SA = detection.width * detection.height;
	int SB = ground_truth.width * ground_truth.height;

	float overlap;

	//total overlap
	if ((SB == intersection_area && SA == union_area)
			|| (SA == intersection_area && SB == union_area))
		overlap = 1.0;
	else
		overlap = static_cast<float>(intersection_area)
				/ static_cast<float>(union_area);

	return overlap;
}

/**
 * run_program
 *
 * This module is the main entry point into the squirrel pipeline.
 * Any system using the pipeline will have to set up the appropriate files to be modified beforehand.
 *
 * @param  float fudge - padding around detected regions
 * @param  int scale - how will the image be scaled
 * @param  bool firstFrame - first frame or not
 * @param  Mat grayImage - grayscale input image
 * @param  Mat b1 - first background image
 * @param  Mat b2 - second background image
 * @param  Mat thresh - threshold image
 * @param  Mat Hi - high threshold image
 * @param  Mat Lo - low threshold image
 * @param  Mat classificationMap - classification map image
 * @param  const char *baseName - base name of image working with
 * @param int minArea - minimum area for detected squirrel regions
 * @param int maxArea - maximum area for detected squirrel regions
 * @param char* model_dir - directory that contains the SVM model files
 * @return vector<int>  - coordinates of the bounding boxes of objects detected
 */

vector<int> run_program(sConfig &config, bool firstFrame, Mat grayImage,
		Mat colorFrame, Mat b1, Mat b2, vector<cv::Mat> oldFrames, Mat thresh,
		Mat Hi, Mat Lo, Mat classificationMap, const char *baseName,
		const char* rawName, Mat frame) {

	vector<int> boxes;

	Mat saveFrameOut = frame.clone();

	//open the output file
#ifdef WRITE_2_FILE
	char fileBuffer[1024];
	sprintf(fileBuffer, "%s/%s.txt", config.image_chips, rawName);
	FILE *outFile = fopen(fileBuffer, "w");
#endif

	bool assertionPass = false;
	myConfig = config;
	static int imageCounter = 1;

	//debug
#ifdef DEBUG
	Mat debugSubtraction = colorFrame.clone();
	Mat debugSubtraction1 = colorFrame.clone();
#endif

	int boxSize = config.scale;

	//get image frame
	Mat inputFrame;
	Mat Background1 = b1;
	Mat Background2 = b2;
	Mat Thresholds = thresh;
	Mat Detected_Image_HI = Hi;
	Mat Detected_Image_LOW = Lo;
	Mat parentImage;

	static int GLOBAL_THRESHOLD = config.GLOBAL_THRESHOLD_INIT;

	Mat saveImage = colorFrame.clone();

	Mat targetDetect = cv::Mat::zeros(grayImage.rows / config.scale,
			grayImage.cols / config.scale, CV_8UC3);

	//Initialize memory for the background(1,2) and Thresholds
	if (firstFrame) {

#ifdef ERASE_FOLDERS
		char systemBuffer[2048];

		sprintf(systemBuffer, "rm -rf ./debugFolders/positiveFrame/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/colorOut/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/colorOutPositive/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/back1/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/back2/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/back3/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/classMap/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/current/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/diff/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/hilo/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/imageChips/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/negOutExemplar/*");
		system(systemBuffer);

		sprintf(systemBuffer,
				"rm -rf  ./debugFolders/positiveExemplarMaxVal/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/positiveExemplarLowVal/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/positiveExemplars/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/qcc/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/thresholds/*");
		system(systemBuffer);

		sprintf(systemBuffer, "rm -rf ./debugFolders/ebugBox/*");
		system(systemBuffer);

#endif

		Update_Threshold_Background(4, grayImage, Background1, Background2,
				Thresholds, Thresholds, Thresholds, &GLOBAL_THRESHOLD,
				config.UPDATE_NUMBER, config.GLOBAL_INCREMENT,
				config.GLOBAL_THRESHOLD_INIT, config.THRESHOLD_DECREMENT_NUMBER,
				config.THRESHOLD_DECREMENT_VALUE,
				config.THRESHOLD_UPDATE_NEG_EXEMPLAR,
				config.THRESHOLD_UPDATE_POS_EXEMPLAR,
				config.THRESHOLD_UPDATE_TOO_BIG_SMALL,
				config.UPDATE_BACKGROUND_INC);

		//return vector of 0
		vector<int> zeroVector;
		zeroVector.push_back(-6789);
		return zeroVector;
	}

	float parentImageRowsF = ceil((float) grayImage.rows / (float) boxSize);

	float parentImageColumnsF = ceil((float) grayImage.cols / (float) boxSize);

	parentImage = cv::Mat::zeros((int) parentImageRowsF,
			(int) parentImageColumnsF, CV_8UC1);

	Mat diffImage = cv::Mat::zeros(grayImage.size(), CV_8UC1);

	Background_Subtraction(diffImage, grayImage, Background1, Background2,
			Thresholds, Detected_Image_HI, Detected_Image_LOW,
			&GLOBAL_THRESHOLD, config.HI_THRESHOLD_INC);

	//QCC MAP
	cv::Mat binaryHi = cv::Mat::zeros(Detected_Image_HI.size(), CV_8UC1);
	cv::Mat binaryLow = cv::Mat::zeros(Detected_Image_LOW.size(), CV_8UC1);

	//Detected images will always have same dimensions
	for (int i = 0; i < Detected_Image_HI.rows; i++) {
		for (int j = 0; j < Detected_Image_HI.cols; j++) {
			if (Detected_Image_HI.at < uchar > (i, j) > 0)
				binaryHi.at < uchar > (i, j) = (uchar) 1;
			if (Detected_Image_LOW.at < uchar > (i, j) > 0)
				binaryLow.at < uchar > (i, j) = (uchar) 1;
		}
	}

	QCC(binaryHi, binaryLow, parentImage, boxSize,
			config.PERCENT_PIXELS_NEEDED_HIGH_ONLY,
			config.PERCENT_PIXELS_NEEDED_HIGH,
			config.PERCENT_PIXELS_NEEDED_LOW);

	cv::Mat binary = cv::Mat::zeros(parentImage.size(), CV_8UC1);

#ifdef DEBUG
	cv::Mat binaryDisp = cv::Mat::zeros(parentImage.size(), CV_8UC1);
#endif

	std::vector < std::vector<cv::Point2i> > blobs;
	cv::Mat output = cv::Mat::zeros(parentImage.size(), CV_8UC3);

	for (int i = 0; i < parentImage.rows; i++) {
		for (int j = 0; j < parentImage.cols; j++) {
			if (parentImage.at < uchar > (i, j) == 255) {
				binary.at < uchar > (i, j) = (uchar) 1;
#ifdef DEBUG
				binaryDisp.at < uchar > (i, j) = (uchar) 255;
#endif
			}
		}
	}

#ifdef DEBUG
	imshow("binary", binaryDisp);
	//cvWaitKey(0);
#endif

	cv::Scalar imgSum = cv::sum(binary);
	int top = imgSum[0];

	int bottom = (binary.rows * binary.cols);
	double detectedRatio = (double) top / (double) bottom;

#ifdef DEBUG
	cout << "detected ratio: " << detectedRatio << endl;
#endif

	vector<int> returnBoxes;
	std::vector < cv::Rect > boundingBoxes;
	std::vector < cv::Rect > boundingBoxesFinal;

	//if (detectedRatio < config.maxRatio) {
	FindBlobs(binary, blobs);

	//find bounding boxes
	std::vector<int> minXArr;
	std::vector<int> minYArr;
	std::vector<int> maxXArr;
	std::vector<int> maxYArr;

	Find_Bounding_Boxes(output, blobs, minXArr, minYArr, maxXArr, maxYArr,
			parentImage.cols, parentImage.rows);

#ifdef DEBUG
	cout << "boxes in: " << (int) blobs.size() << endl;
#endif
	//put the blob coordinates into a CvRect Structure
	Find_Rectangles(boundingBoxes, output, blobs, minXArr, minYArr, maxXArr,
			maxYArr, config.MIN_AREA_QCC);

	//This is the number of bounding boxes.
	returnBoxes.push_back((int) boundingBoxes.size());

#ifdef DEBUG
	cout << "returned boxes: " << (int) boundingBoxes.size() << endl;
#endif

	///////////////////THE BRAIN///////////////////////////////////////
	///////////////////to decide what to do and if to process the frame

	static int state = 0, next_state = 0;
	static int command;

	state = next_state;

	//simple logic for lighting changes/movement
	switch (state) {
	case 0:
		if (detectedRatio >= config.maxRatio)
			next_state = 1;
		else
			next_state = 0;
		break;

		//increase global by GLOBAL_INCREMENT, 2 times
	case 1:
		if (detectedRatio >= config.maxRatio)	//save frame to SDCard

			next_state = 4;
		else
			next_state = 3;
		break;

	case 2:
		if (detectedRatio >= config.maxRatio)
			next_state = 4;
		else
			next_state = 3;
		break;

		//increase global successful update non-target
	case 3:
		next_state = 0;
		break;

		//increase global unsuccessful (ABSOLUTE_UPDATE)
	case 4:
		next_state = 5;
		break;

		//re-initialize background and threshold
	case 5:
		next_state = 0;
		break;

	}

	if (state == 0)
		command = 0;
	else if (state == 1 || state == 2)
		command = 1;
	else if (state == 3)
		command = 2;
	else if (state == 4)
		command = 4;
	else if (state == 5)
		command = 4;

	cout << "State: " << state << " Ratio: " << detectedRatio << endl;
	///////////////////END  BRAIN////////////////////////////////

	//Mat saveFrame = colorFrame.clone();

	//extract the blobs
	vector < Rect > finalRects;

	//normal operation
	if (state == 0 && detectedRatio <= config.maxRatio) {

		//zero out the classification map
		for (int y = 0; y < classificationMap.rows; y++)
			for (int x = 0; x < classificationMap.cols; x++) {
				classificationMap.at < uchar > (y, x) = 128;
			}

		vector < cv::Rect > boundingBoxesBigROI;

		for (unsigned int i = 0; i < boundingBoxes.size(); i++) {

			cv::Rect myROI(boundingBoxes[i]);

			myROI.x *= (config.scale);
			myROI.y *= (config.scale);
			myROI.width += 1;
			myROI.width *= (config.scale);
			myROI.height += 1;
			myROI.height *= (config.scale);

			float fudgeWidth = myROI.width * config.fudge;
			float fudgeHeight = myROI.height * config.fudge;

			//make crop a bigger
			myROI.x -= (int) fudgeWidth;
			if (myROI.x < 0)
				myROI.x = 0;

			myROI.y -= (int) fudgeHeight;
			if (myROI.y < 0)
				myROI.y = 0;

			myROI.width += (int) fudgeWidth * 2;
			if (myROI.width + myROI.x > grayImage.cols)
				myROI.width = grayImage.cols - myROI.x;

			myROI.height += (int) fudgeHeight * 2;
			if (myROI.height + myROI.y > grayImage.rows)
				myROI.height = grayImage.rows - myROI.y;

			//check to make sure myROI isn't too big or small
			double mArea = myROI.height * myROI.width;
			double maxArea, minArea;

			if (config.USE_MAX_AREA) {
				//calculate the max/min area parameter for the chip given the y coord
				maxArea = (config.mMaxArea * (double) myROI.y)
						+ (config.bMaxArea);
				//detecting squirrel and it's shadow also need to make bigger
				maxArea += config.maxAreaFudge;
			} else
				maxArea = grayImage.rows * grayImage.cols;

			if (config.USE_MIN_AREA) {
				minArea = (config.mMinArea * (double) myROI.y)
						+ (config.bMinArea);
				minArea -= config.minAreaFudge;
			} else
				minArea = 0;

			if (mArea <= minArea || mArea >= maxArea
					|| myROI.width <= config.minROIWidth
					|| myROI.height <= config.minROIHeight) {
				returnBoxes[0]--;

				for (int boxY = myROI.y;
						boxY <= myROI.y + myROI.height
								&& boxY < classificationMap.rows; boxY++)
					for (int boxX = myROI.x;
							boxX <= myROI.x + myROI.width
									&& boxX < classificationMap.cols; boxX++) {
						//keep region positive
						if (classificationMap.at < uchar > (boxY, boxX) != 255)
							classificationMap.at < uchar > (boxY, boxX) = 10;
					}

#ifdef DEBUG
				//boxes too small or large
				Rect rr = myROI;
				if (mArea <= minArea || myROI.width < config.minROIWidth
						|| myROI.height < config.minROIHeight) {
					rectangle(debugSubtraction, rr.tl(), rr.br(),
							Scalar(255, 255, 0), 3);
				} else {
					rectangle(debugSubtraction, rr.tl(), rr.br(),
							Scalar(255, 128, 128), 3);
				}
#endif

			} else {
				//push data
				boundingBoxesBigROI.push_back(myROI);
			}

		}

		//merge overlapping rectangles
		cv::Rect inputRect;
		cv::Rect testRect;
		cv::Rect tempRect;

		float input_x_end, test_x_end, input_y_end, test_y_end;

		vector<bool> flagVector;
		flagVector.assign(boundingBoxesBigROI.size(), true);

		for (unsigned int outerLoopOuter = 0; outerLoopOuter <= 1;
				outerLoopOuter++) {

			for (unsigned int outerLoop = 0;
					outerLoop < boundingBoxesBigROI.size(); outerLoop++) {

				inputRect = boundingBoxesBigROI[outerLoop];

				if (inputRect.width == -99 && inputRect.height == -99)
					continue;

				for (unsigned int innerLoop = 0;
						innerLoop < boundingBoxesBigROI.size(); innerLoop++) {

					if (flagVector[innerLoop] == true
							&& innerLoop != outerLoop) {

						//get test rectangle
						testRect = boundingBoxesBigROI[innerLoop];

						//check for overlap
						float overlap = testBoundingBoxesForOverlap(inputRect,
								testRect);

						if (overlap >= config.maxOverlapBeforeMerge) {

							//merge the rectangles
							tempRect.x = std::min(inputRect.x, testRect.x);
							tempRect.y = std::min(inputRect.y, testRect.y);

							input_x_end = inputRect.x + inputRect.width;
							test_x_end = testRect.x + testRect.width;

							input_y_end = inputRect.y + inputRect.height;
							test_y_end = testRect.y + testRect.height;

							if (input_x_end > test_x_end)
								tempRect.width = input_x_end - tempRect.x;
							else
								tempRect.width = test_x_end - tempRect.x;

							if (input_y_end > test_y_end)
								tempRect.height = input_y_end - tempRect.y;
							else
								tempRect.height = test_y_end - tempRect.y;

							//invalidate the test rectangle
							flagVector[innerLoop] = false;
							boundingBoxesBigROI[innerLoop].width = -99;
							boundingBoxesBigROI[innerLoop].height = -99;

							//assign tempRect
							boundingBoxesBigROI[outerLoop].x = tempRect.x;
							boundingBoxesBigROI[outerLoop].y = tempRect.y;
							boundingBoxesBigROI[outerLoop].width =
									tempRect.width;
							boundingBoxesBigROI[outerLoop].height =
									tempRect.height;

							//put the merged rectangle into the testRect slot
							inputRect.x = tempRect.x;
							inputRect.y = tempRect.y;
							inputRect.width = tempRect.width;
							inputRect.height = tempRect.height;

						}		//if (overlap > )
					}
				}
			}
		}

		//get the final rectangles
		vector < cv::Rect > boundingBoxesBigROIFinal;
		cv::Rect tempRectFinalBig;

		for (unsigned int finalLoop = 0; finalLoop < boundingBoxesBigROI.size();
				finalLoop++) {

			if (flagVector[finalLoop] == true) {

				tempRectFinalBig = boundingBoxesBigROI[finalLoop];

				//check size again
				if (config.USE_MAX_AREA_AFTER_MERGE) {
					double tempArea = tempRectFinalBig.width
							* tempRectFinalBig.height;

					double maxArea = (config.mMaxAreaAfterMerge
							* (double) tempRectFinalBig.y)
							+ (config.bMaxAreaAfterMerge);

					//detecting squirrel and it's shadow also need to make bigger
					maxArea += config.maxAreaFudgeAfterMerge;

					if (tempArea <= maxArea)
						boundingBoxesBigROIFinal.push_back(tempRectFinalBig);
					else {
						for (int boxY = tempRectFinalBig.y;
								boxY
										<= tempRectFinalBig.y
												+ tempRectFinalBig.height
										&& boxY < classificationMap.rows;
								boxY++)
							for (int boxX = tempRectFinalBig.x;
									boxX
											<= tempRectFinalBig.x
													+ tempRectFinalBig.width
											&& boxX < classificationMap.cols;
									boxX++) {
								//keep region positive
								if (classificationMap.at < uchar > (boxY, boxX)
										!= 255)
									classificationMap.at < uchar > (boxY, boxX) =
											10;
							}

#ifdef DEBUG
						//boxes too small or large
						Rect rr = tempRectFinalBig;
						rectangle(debugSubtraction, rr.tl(), rr.br(),
								Scalar(255, 0, 255), 3);
#endif
					}
				} else {

					boundingBoxesBigROIFinal.push_back(tempRectFinalBig);
				}
			}
		}

		for (unsigned int i = 0; i < boundingBoxesBigROIFinal.size(); i++) {
			double minVal;
			double maxVal;
			double maxValTemp;
			Point minLoc;
			Point maxLoc;

			finalRects.clear();

			cv::Rect myROI(boundingBoxesBigROIFinal[i]);

			//crop the image
			//check assertion before cropping
			cv::Mat cropOut;
			cv::Mat oldCropOut;

			assertionPass = checkROIAssertion(saveImage, myROI);

			if (assertionPass) {
				cropOut = saveImage(myROI);

				Mat result;
				cv::Mat b3;

				for (unsigned int testLoop = 0; testLoop < oldFrames.size();
						testLoop++) {

					b3 = oldFrames[testLoop];

					oldCropOut = b3(myROI);

					/// Create the result matrix
					int result_cols = cropOut.cols - cropOut.cols + 1;
					int result_rows = cropOut.rows - cropOut.rows + 1;

					result.create(result_cols, result_rows, CV_32FC1);

					/// Do the Matching
					matchTemplate(cropOut, oldCropOut, result,
							CV_TM_CCOEFF_NORMED);

					Point matchLoc;
					minMaxLoc(result, &minVal, &maxValTemp, &minLoc, &maxLoc,
							Mat());

					if (testLoop == 0)
						maxVal = maxValTemp;
					else if (maxValTemp > maxVal)
						maxVal = maxValTemp;
				}

			} else {
				continue;
			}
			assertionPass = checkROIAssertion(grayImage, myROI);

			cv::Mat cropGray;
			if (assertionPass) {
				cropGray = grayImage(myROI);
			} else {
				continue;
			}

			/////////////////RUN EXEMPlAR SVM TO GET POSITIVE WINDOWS/////////////////

			if (config.USE_EXEMPLAR) {
				if (maxVal <= config.maxCorrVal && maxVal >= 0.0) {

					//get the number of hi and low pixels
					cv::Mat hi_crop = binaryHi(myROI);
					cv::Mat low_crop = binaryLow(myROI);

					CvScalar hiCount1 = cv::sum(hi_crop);
					CvScalar lowCount1 = cv::sum(low_crop);

					char outBufferExemplar[256];

					//make crop out bigger
					cv::Rect cropROI;
					cropROI.x = myROI.x;
					cropROI.y = myROI.y;
					cropROI.width = myROI.width;
					cropROI.height = myROI.height;

					float fudgeWidth = cropROI.width * config.fudge;
					float fudgeHeight = cropROI.height * config.fudge;

					//make crop a bigger
					cropROI.x -= (int) fudgeWidth
							* config.cropFudgeFactorExemplar;
					if (cropROI.x < 0)
						cropROI.x = 0;

					cropROI.y -= (int) fudgeHeight
							* config.cropFudgeFactorExemplar;
					if (cropROI.y < 0)
						cropROI.y = 0;

					cropROI.width += (int) fudgeWidth
							* (config.cropFudgeFactorExemplar * 2);
					if (cropROI.width + cropROI.x > grayImage.cols)
						cropROI.width = grayImage.cols - cropROI.x;

					cropROI.height += (int) fudgeHeight
							* (config.cropFudgeFactorExemplar * 2);
					if (cropROI.height + cropROI.y > grayImage.rows)
						cropROI.height = grayImage.rows - cropROI.y;

					cv::Mat cropOut1 = saveImage(cropROI);

					sprintf(outBufferExemplar, "%s/%s_%f.jpg",
							config.image_chips, rawName, maxVal);

					bool successExemplar = cv::imwrite(outBufferExemplar,
							cropOut1);

					cout << "Write Success: " << successExemplar << " "
							<< rawName << maxVal << endl;

					double myValue = -1.0;
					bool ranExemplar = false;

					if (successExemplar == true) {
						char exemplarBuf[2048];
						sprintf(exemplarBuf,
								"./ExemplarSVMs/ExemplarSVMs -Test Squirrel31 %s > tempExemplarOutput",
								outBufferExemplar);
						puts(exemplarBuf);

						system(exemplarBuf);

						//parse the response
						FILE *exemplarResponse = fopen("tempExemplarOutput",
								"r");
						char response[2048];

						int index = 0;
						if (fgets(response, sizeof response, exemplarResponse)
								!= NULL) {

							ranExemplar = true;

							char * pch;

							pch = strchr(response, '=');
							index = (pch - response + 1);

							if (index > 0) {
								string str(response);
								cout << response << endl;
								//fix error of out of range
								//Error unable to create thread, 11
								//TODO check for , or ERROR then run again
								std::string part(str.substr((index + 1), 8));
								myValue = atof(part.c_str());
							}

						}
						fclose(exemplarResponse);
					}

					if (myValue >= config.minExemplarValue) {

						//write to positive folder
						char outBuffer[2048];

						sprintf(outBuffer, "%s/%f_%f_%s.jpg",
								config.positiveExemplarMaxValFolder, myValue,
								maxVal, rawName);
						cout << "going to save max" << endl;
						bool maxSuccess = cv::imwrite(outBuffer, cropOut1);

						for (int boxY = myROI.y;
								boxY <= myROI.y + myROI.height
										&& boxY < classificationMap.rows;
								boxY++) {
							for (int boxX = myROI.x;
									boxX <= myROI.x + myROI.width
											&& boxX < classificationMap.cols;
									boxX++) {

								classificationMap.at < uchar > (boxY, boxX) =
										255;
							}
						}
						//run exemplar svm on crop
#ifdef DEBUG
						//draw on color frame green squares
						Rect r = myROI;
						rectangle(debugSubtraction, r.tl(), r.br(),
								Scalar(0, 255, 0), 3);

						if (config.SAVE_NEGATIVE_EXEMPLARS) {
							char outBuffer[256];
							sprintf(outBuffer, "%s/%d.jpg",
									config.negativeExemplarFolder,
									IMAGE_COUNTER++);
							cv::imwrite(outBuffer, cropGray);
						}
#endif
					} else {

						if (ranExemplar == true) {
							//write to positive folder
							char outBuffer[256];

							sprintf(outBuffer, "%s/%f_%f_%s.jpg",
									config.positiveExemplarMinValFolder,
									myValue, maxVal, rawName);

							bool success = cv::imwrite(outBuffer, cropOut1);

							for (int boxY = myROI.y;
									boxY <= myROI.y + myROI.height
											&& boxY < classificationMap.rows;
									boxY++) {
								for (int boxX = myROI.x;
										boxX <= myROI.x + myROI.width
												&& boxX < classificationMap.cols;
										boxX++) {

									//keep region positive
									if (classificationMap.at < uchar
											> (boxY, boxX) != 255)
										classificationMap.at < uchar
												> (boxY, boxX) = 0;
								}
							}

							//draw on color frame red squares
#ifdef DEBUG
							Rect r = myROI;
							rectangle(debugSubtraction, r.tl(), r.br(),
									Scalar(0, 128, 255), 3);

							char outBufferNeg[256];
							sprintf(outBufferNeg,
									"./debugFolders/negOut/%d.jpg",
									IMAGE_COUNTER++);
							cv::imwrite(outBufferNeg, cropGray);
#endif
						}
					}
				}

				//correlation value too high
				else {

					for (int boxY = myROI.y;
							boxY <= myROI.y + myROI.height
									&& boxY < classificationMap.rows; boxY++) {
						for (int boxX = myROI.x;
								boxX <= myROI.x + myROI.width
										&& boxX < classificationMap.cols;
								boxX++) {

							//keep region positive
							if (classificationMap.at < uchar > (boxY, boxX)
									!= 255)
								classificationMap.at < uchar > (boxY, boxX) = 0;
						}
					}

					//draw on color frame red squares
#ifdef DEBUG
					Rect r = myROI;
					rectangle(debugSubtraction, r.tl(), r.br(),
							Scalar(0, 0, 255), 3);
					if (config.SAVE_NEGATIVE_EXEMPLARS) {

						char outBuffer[2048];
						sprintf(outBuffer, "%s/%f_%s.jpg",
								config.negativeExemplarFolder, maxVal, rawName);
						cv::imwrite(outBuffer, cropGray);
					}
#endif
				} //end else
			} else {
				if (maxVal <= config.maxCorrVal) {
					Mat debugSubtraction2 = debugSubtraction1.clone();

					//get the number of hi and low pixels
					cv::Mat hi_crop = binaryHi(myROI);
					cv::Mat low_crop = binaryLow(myROI);

					CvScalar hiCount1 = cv::sum(hi_crop);
					CvScalar lowCount1 = cv::sum(low_crop);

					char outBufferExemplar[256];

					//make crop out bigger
					cv::Rect cropROI;
					cropROI.x = myROI.x;
					cropROI.y = myROI.y;
					cropROI.width = myROI.width;
					cropROI.height = myROI.height;

					float fudgeWidth = cropROI.width * config.fudge;
					float fudgeHeight = cropROI.height * config.fudge;

					//make crop a bigger
					cropROI.x -= (int) fudgeWidth
							* config.cropFudgeFactorExemplar;
					if (cropROI.x < 0)
						cropROI.x = 0;

					cropROI.y -= (int) fudgeHeight
							* config.cropFudgeFactorExemplar;
					if (cropROI.y < 0)
						cropROI.y = 0;

					cropROI.width += (int) fudgeWidth
							* (config.cropFudgeFactorExemplar * 2);
					if (cropROI.width + cropROI.x > grayImage.cols)
						cropROI.width = grayImage.cols - cropROI.x;

					cropROI.height += (int) fudgeHeight
							* (config.cropFudgeFactorExemplar * 2);
					if (cropROI.height + cropROI.y > grayImage.rows)
						cropROI.height = grayImage.rows - cropROI.y;

					if (cropROI.width >= config.minWidthOutputBox
							&& cropROI.height >= config.minHeightOutputBox) {

						cv::Mat cropOut1 = saveImage(cropROI);

						sprintf(outBufferExemplar, "%s/%f_%s.jpg",
								config.positiveExemplars, maxVal, rawName);

						bool successExemplar = cv::imwrite(outBufferExemplar,
								cropOut1);

#ifdef DEBUG
						//draw on color frame green squares
						Rect r = myROI;
						rectangle(debugSubtraction, r.tl(), r.br(),
								Scalar(0, 255, 0), 3);

						rectangle(debugSubtraction2, r.tl(), r.br(),
								Scalar(0, 255, 0), 3);
						char outBufferPos[2048];
						sprintf(outBufferPos,
								"./debugFolders/colorOutPositive/%s_%f.jpg",
								rawName, maxVal);
						cv::imwrite(outBufferPos, debugSubtraction2);

						//draw on original image

						r = myROI;
						r.x += config.iX - 20;
						r.y += config.iY - 20;
						r.width = myROI.width + 40;
						r.height = myROI.height + 40;

						rectangle(frame, r.tl(), r.br(), Scalar(0, 255, 0), 3);

						sprintf(outBufferPos,
								"./debugFolders/positiveFrame/%s_%f.jpg",
								rawName, maxVal);
						cv::imwrite(outBufferPos, frame);

#endif
					} else {
						for (int boxY = myROI.y;
								boxY <= myROI.y + myROI.height
										&& boxY < classificationMap.rows;
								boxY++) {
							for (int boxX = myROI.x;
									boxX <= myROI.x + myROI.width
											&& boxX < classificationMap.cols;
									boxX++) {

								if (classificationMap.at < uchar > (boxY, boxX)
										!= 255)
									classificationMap.at < uchar > (boxY, boxX) =
											0;
							}
						}

						//draw on color frame red squares
#ifdef DEBUG

						Rect r = myROI;
						rectangle(debugSubtraction, r.tl(), r.br(),
								Scalar(0, 0, 255), 3);

						//char outBuffer[2048];
						//sprintf(outBuffer, "%s/%f_%s.jpg",
						//		config.negativeExemplarFolder, maxVal, rawName);
						//cv::imwrite(outBuffer, cropGray);

#endif
					}
				}
				//correlation value too high
				else {

					for (int boxY = myROI.y;
							boxY <= myROI.y + myROI.height
									&& boxY < classificationMap.rows; boxY++) {
						for (int boxX = myROI.x;
								boxX <= myROI.x + myROI.width
										&& boxX < classificationMap.cols;
								boxX++) {

							if (classificationMap.at < uchar > (boxY, boxX)
									!= 255)
								classificationMap.at < uchar > (boxY, boxX) = 0;
						}
					}

					//draw on color frame red squares
#ifdef DEBUG

					Rect r = myROI;
					rectangle(debugSubtraction, r.tl(), r.br(),
							Scalar(0, 0, 255), 3);

					//char outBuffer[2048];
					//sprintf(outBuffer, "%s/%f_%s.jpg",
					//config.negativeExemplarFolder, maxVal, rawName);
				}

			}
		} //end for loop

	} //end if detect ratio

	//close the output file

#ifdef WRITE_2_FILE
	fclose(outFile);
#endif

	Update_Threshold_Background(command, grayImage, Background1, Background2,
			Thresholds, binary, classificationMap, &GLOBAL_THRESHOLD,
			config.UPDATE_NUMBER, config.GLOBAL_INCREMENT,
			config.GLOBAL_THRESHOLD_INIT, config.THRESHOLD_DECREMENT_NUMBER,
			config.THRESHOLD_DECREMENT_VALUE,
			config.THRESHOLD_UPDATE_NEG_EXEMPLAR,
			config.THRESHOLD_UPDATE_POS_EXEMPLAR,
			config.THRESHOLD_UPDATE_TOO_BIG_SMALL,
			config.UPDATE_BACKGROUND_INC);

#ifdef DEBUG

	char imageNumber[256];
	Size mSize = cv::Size(grayImage.cols / 1, grayImage.rows / 1);
	Mat mFrame = Mat(mSize, CV_8UC1);

	namedWindow("colorFrame", 0);
	imshow("colorFrame", debugSubtraction);

	sprintf(imageNumber, "./debugFolders/colorOut/%s.jpg", rawName);
	cv::imwrite(imageNumber, debugSubtraction);

	namedWindow("Current Frame", 0);
	cv::resize(saveFrameOut, mFrame, mFrame.size());
	imshow("Current Frame", mFrame);
	sprintf(imageNumber, "./debugFolders/current/%s.jpg", rawName);
	cv::imwrite(imageNumber, saveFrameOut);

#endif

#ifdef ADVANCED_DEBUG
	namedWindow("Background1", 0);
	cv::resize(Background1, mFrame, mFrame.size());
	imshow("Background1", mFrame);
	sprintf(imageNumber, "./debugFolders/back1/%s.png", rawName);
	cv::imwrite(imageNumber, Background1);

	namedWindow("Background2", 0);
	cv::resize(Background2, mFrame, mFrame.size());
	imshow("Background2", mFrame);
	sprintf(imageNumber, "./debugFolders/back2/%s.png", rawName);
	cv::imwrite(imageNumber, Background2);

	namedWindow("Diff", 0);
	cv::resize(diffImage, mFrame, mFrame.size());
	imshow("Diff", mFrame);
	sprintf(imageNumber, "./debugFolders/diff/%s.jpg", rawName);
	cv::imwrite(imageNumber, diffImage);

	//combine high and low threshold into one "displayImage"
	//if high = 255, then 255
	//if low  = 255, then 0
	//else           128
	Mat displayImage = Mat(Detected_Image_HI.size(), CV_8UC1);

	for (int i = 0; i < Detected_Image_HI.rows; i++) {
		for (int j = 0; j < Detected_Image_HI.cols; j++) {
			if (Detected_Image_HI.at < uchar > (i, j) == (uchar) 255)
				displayImage.at < uchar > (i, j) = (uchar) 255;
			else if (Detected_Image_LOW.at < uchar > (i, j) == (uchar) 255)
				displayImage.at < uchar > (i, j) = (uchar) 0;
			else
				displayImage.at < uchar > (i, j) = (uchar) 128;
		}
	}

	namedWindow("Hi Low", 0);
	cv::resize(displayImage, mFrame, mFrame.size());
	imshow("Hi Low", mFrame);
	sprintf(imageNumber, "./debugFolders/hilo/%s.jpg", rawName);
	cv::imwrite(imageNumber, mFrame);

	namedWindow("Thresholds", 0);
	cv::resize(Thresholds, mFrame, mFrame.size());
	imshow("Thresholds", mFrame);
	sprintf(imageNumber, "./debugFolders/thresholds/%s.jpg", rawName);
	cv::imwrite(imageNumber, Thresholds);

	namedWindow("QCC Labeled", 0);
	imshow("QCC Labeled", output);
	sprintf(imageNumber, "./debugFolders/qcc/%s.jpg", rawName);
	cv::imwrite(imageNumber, output);

	namedWindow("Classification Map", 0);
	imshow("Classification Map", classificationMap);
	sprintf(imageNumber, "./debugFolders/classMap/%s.jpg", rawName);
	cv::imwrite(imageNumber, classificationMap);

	cvWaitKey(1000);
#endif

	char sysBuffer[1024];
	sprintf(sysBuffer, "rm -rf ./imageChips/*.jpg");
	system(sysBuffer);

	imageCounter++;

	return boxes;
}

#ifndef __ANDROID__
/**
 * main
 *
 * This is the entry point for the c version of the squirrel pipeline.
 * In this we set up all the needed images and values for which to run the pipeline.
 * This is only called in the c version.
 * All other versions should call run_program directly
 *
 * @param  int argc - number of command line inputs
 * @param  char **argv - command line inputs
 * @return int
 */

bool readConfig(const char *fConfig, sConfig *c) {

	FILE *f = fopen(fConfig, "r");

	if (!f) {
		fprintf(stderr, "Could not open file: %s\n", fConfig);
		return false;
	}
	char line[256], *first, *key, *value;

	while (fgets(line, 256, f) != NULL) {
		//now parse the line
		first = line;
		while (first[0] == ' ')
			first++;
		if (first[0] == '#')
			continue;
		key = strtok(first, " \t");

		if (!strcmp(key, "fudge")) {
			value = strtok(NULL, " \t");
			c->fudge = atof(value);
		} else if (!strcmp(key, "scale")) {
			value = strtok(NULL, " \t");
			c->scale = atoi(value);
		} else if (!strcmp(key, "UPDATE_NUMBER")) {
			value = strtok(NULL, " \t");
			c->UPDATE_NUMBER = atoi(value);
		} else if (!strcmp(key, "starttime")) {
			value = strtok(NULL, " \t");
			c->g_starttime = atoi(value);
		} else if (!strcmp(key, "endtime")) {
			value = strtok(NULL, " \t");
			c->g_endtime = atoi(value);
		} else if (!strcmp(key, "HI_THRESHOLD_INC")) {
			value = strtok(NULL, " \t");
			c->HI_THRESHOLD_INC = atoi(value);
		} else if (!strcmp(key, "UPDATE_BACKGROUND_INC")) {
			value = strtok(NULL, " \t");
			c->UPDATE_BACKGROUND_INC = atoi(value);
		} else if (!strcmp(key, "GLOBAL_INCREMENT")) {
			value = strtok(NULL, " \t");
			c->GLOBAL_INCREMENT = atoi(value);
		} else if (!strcmp(key, "GLOBAL_THRESHOLD_INIT")) {
			value = strtok(NULL, " \t");
			c->GLOBAL_THRESHOLD_INIT = atoi(value);
		} else if (!strcmp(key, "THRESHOLD_DECREMENT_NUMBER")) {
			value = strtok(NULL, " \t");
			c->THRESHOLD_DECREMENT_NUMBER = atoi(value);
		} else if (!strcmp(key, "THRESHOLD_DECREMENT_VALUE")) {
			value = strtok(NULL, " \t");
			c->THRESHOLD_DECREMENT_VALUE = atoi(value);
		} else if (!strcmp(key, "NUMBER_OLD_FRAMES")) {
			value = strtok(NULL, " \t");
			c->NUMBER_OLD_FRAMES = atoi(value);
		} else if (!strcmp(key, "MIN_AREA_QCC")) {
			value = strtok(NULL, " \t");
			c->MIN_AREA_QCC = atof(value);
		} else if (!strcmp(key, "PERCENT_PIXELS_NEEDED_HIGH_ONLY")) {
			value = strtok(NULL, " \t");
			c->PERCENT_PIXELS_NEEDED_HIGH_ONLY = atof(value);
		} else if (!strcmp(key, "PERCENT_PIXELS_NEEDED_HIGH")) {
			value = strtok(NULL, " \t");
			c->PERCENT_PIXELS_NEEDED_HIGH = atof(value);
		} else if (!strcmp(key, "PERCENT_PIXELS_NEEDED_LOW")) {
			value = strtok(NULL, " \t");
			c->PERCENT_PIXELS_NEEDED_LOW = atof(value);
		} else if (!strcmp(key, "THRESHOLD_UPDATE_NEG_EXEMPLAR")) {
			value = strtok(NULL, " \t");
			c->THRESHOLD_UPDATE_NEG_EXEMPLAR = atoi(value);
		} else if (!strcmp(key, "THRESHOLD_UPDATE_POS_EXEMPLAR")) {
			value = strtok(NULL, " \t");
			c->THRESHOLD_UPDATE_POS_EXEMPLAR = atoi(value);
		} else if (!strcmp(key, "THRESHOLD_UPDATE_TOO_BIG_SMALL")) {
			value = strtok(NULL, " \t");
			c->THRESHOLD_UPDATE_TOO_BIG_SMALL = atoi(value);
		} else if (!strcmp(key, "maxRatio")) {
			value = strtok(NULL, " \t");
			c->maxRatio = atof(value);
		} else if (!strcmp(key, "USE_MAX_AREA")) {
			value = strtok(NULL, " \t");
			c->USE_MAX_AREA = atoi(value);
		} else if (!strcmp(key, "USE_MIN_AREA")) {
			value = strtok(NULL, " \t");
			c->USE_MIN_AREA = atoi(value);
		} else if (!strcmp(key, "mMaxArea")) {
			value = strtok(NULL, " \t");
			c->mMaxArea = atof(value);
		} else if (!strcmp(key, "bMaxArea")) {
			value = strtok(NULL, " \t");
			c->bMaxArea = atof(value);
		} else if (!strcmp(key, "mMinArea")) {
			value = strtok(NULL, " \t");
			c->mMinArea = atof(value);
		} else if (!strcmp(key, "bMinArea")) {
			value = strtok(NULL, " \t");
			c->bMinArea = atof(value);
		} else if (!strcmp(key, "maxAreaFudge")) {
			value = strtok(NULL, " \t");
			c->maxAreaFudge = atof(value);
		} else if (!strcmp(key, "minAreaFudge")) {
			value = strtok(NULL, " \t");
			c->minAreaFudge = atof(value);
		} else if (!strcmp(key, "minROIWidth")) {
			value = strtok(NULL, " \t");
			c->minROIWidth = atoi(value);
		} else if (!strcmp(key, "minROIHeight")) {
			value = strtok(NULL, " \t");
			c->minROIHeight = atoi(value);
		} else if (!strcmp(key, "maxOverlapBeforeMerge")) {
			value = strtok(NULL, " \t");
			c->maxOverlapBeforeMerge = atof(value);
		} else if (!strcmp(key, "minWidthOutputBox")) {
			value = strtok(NULL, " \t");
			c->minWidthOutputBox = atof(value);
		} else if (!strcmp(key, "minHeightOutputBox")) {
			value = strtok(NULL, " \t");
			c->minHeightOutputBox = atof(value);
		} else if (!strcmp(key, "USE_MAX_AREA_AFTER_MERGE")) {
			value = strtok(NULL, " \t");
			c->USE_MAX_AREA_AFTER_MERGE = atoi(value);
		} else if (!strcmp(key, "mMaxAreaAfterMerge")) {
			value = strtok(NULL, " \t");
			c->mMaxAreaAfterMerge = atof(value);
		} else if (!strcmp(key, "bMaxAreaAfterMerge")) {
			value = strtok(NULL, " \t");
			c->bMaxAreaAfterMerge = atof(value);
		} else if (!strcmp(key, "maxAreaFudgeAfterMerge")) {
			value = strtok(NULL, " \t");
			c->maxAreaFudgeAfterMerge = atoi(value);
		} else if (!strcmp(key, "maxCorrVal")) {
			value = strtok(NULL, " \t");
			c->maxCorrVal = atof(value);
		} else if (!strcmp(key, "cropFudgeFactorExemplar")) {
			value = strtok(NULL, " \t");
			c->cropFudgeFactorExemplar = atoi(value);
		} else if (!strcmp(key, "minExemplarValue")) {
			value = strtok(NULL, " \t");
			c->minExemplarValue = atof(value);
		} else if (!strcmp(key, "SAVE_NEGATIVE_EXEMPLARS")) {
			value = strtok(NULL, " \t");
			c->SAVE_NEGATIVE_EXEMPLARS = atoi(value);
		} else if (!strcmp(key, "USE_EXEMPLAR")) {
			value = strtok(NULL, " \t");
			c->USE_EXEMPLAR = atoi(value);
		} else if (!strcasecmp(key, "model_dir")) {
			value = strtok(NULL, " \t");
			sprintf(c->model_dir, "%s", value);
			if (c->model_dir[strlen(c->model_dir) - 1] == '\n')
				c->model_dir[strlen(c->model_dir) - 1] = '\0';
		} else if (!strcasecmp(key, "positiveExemplarMaxValFolder")) {
			value = strtok(NULL, " \t");
			sprintf(c->positiveExemplarMaxValFolder, "%s", value);
			if (c->positiveExemplarMaxValFolder[strlen(
					c->positiveExemplarMaxValFolder) - 1] == '\n')
				c->positiveExemplarMaxValFolder[strlen(
						c->positiveExemplarMaxValFolder) - 1] = '\0';
		} else if (!strcasecmp(key, "positiveExemplarMinValFolder")) {
			value = strtok(NULL, " \t");
			sprintf(c->positiveExemplarMinValFolder, "%s", value);
			if (c->positiveExemplarMinValFolder[strlen(
					c->positiveExemplarMinValFolder) - 1] == '\n')
				c->positiveExemplarMinValFolder[strlen(
						c->positiveExemplarMinValFolder) - 1] = '\0';
		} else if (!strcasecmp(key, "negativeExemplarFolder")) {
			value = strtok(NULL, " \t");
			sprintf(c->negativeExemplarFolder, "%s", value);
			if (c->negativeExemplarFolder[strlen(c->negativeExemplarFolder) - 1]
					== '\n')
				c->negativeExemplarFolder[strlen(c->negativeExemplarFolder) - 1] =
						'\0';
		} else if (!strcasecmp(key, "model_name")) {
			value = strtok(NULL, " \t");
			sprintf(c->model_name, "%s", value);
			if (c->model_name[strlen(c->model_name) - 1] == '\n')
				c->model_name[strlen(c->model_name) - 1] = '\0';
		} else if (!strcasecmp(key, "positive_exemplar_dir")) {
			value = strtok(NULL, " \t");
			sprintf(c->positive_exemplar_dir, "%s", value);
			if (c->positive_exemplar_dir[strlen(c->positive_exemplar_dir) - 1]
					== '\n')
				c->positive_exemplar_dir[strlen(c->positive_exemplar_dir) - 1] =
						'\0';
		} else if (!strcasecmp(key, "positiveExemplars")) {
			value = strtok(NULL, " \t");
			sprintf(c->positiveExemplars, "%s", value);
			if (c->positiveExemplars[strlen(c->positiveExemplars) - 1] == '\n')
				c->positiveExemplars[strlen(c->positiveExemplars) - 1] = '\0';
		} else if (!strcasecmp(key, "logFile")) {
			value = strtok(NULL, " \t");
			sprintf(c->log_file, "%s", value);
			if (c->log_file[strlen(c->log_file) - 1] == '\n')
				c->log_file[strlen(c->log_file) - 1] = '\0';
		} else if (!strcasecmp(key, "image_chips")) {
			value = strtok(NULL, " \t");
			sprintf(c->image_chips, "%s", value);
			if (c->image_chips[strlen(c->image_chips) - 1] == '\n')
				c->image_chips[strlen(c->image_chips) - 1] = '\0';
		} else if (!strcasecmp(key, "roi_coords_dir")) {
			value = strtok(NULL, " \t");
			sprintf(c->roi_coords_dir, "%s", value);
			if (c->roi_coords_dir[strlen(c->roi_coords_dir) - 1] == '\n')
				c->roi_coords_dir[strlen(c->roi_coords_dir) - 1] = '\0';
		}
	}

	fclose(f);
	return true;
}

int main(int argc, char **argv) {

	bool useAVI = false;

	//assign all parameters manually

	static int frameCounter;
	fprintf(stdout, "Getting Config Parameters\n");

	sConfig c;
	bool success = readConfig("squirrel_config.txt", &c);
	cout << "Config Read:  " << success << endl;

	VideoCapture _videoSource;
	FILE *f;

	fprintf(stdout, "Running Pipeline. Opening image list: %s\n", argv[1]);
	if (useAVI) {
		cout << "useAVI: " << endl;

		if (!_videoSource.open(argv[1])) {
			exit(1);         // Exit if fail
		}
		_videoSource.set(CV_CAP_PROP_CONVERT_RGB, 1);
	} else {
		cout << "opening: " << argv[1] << endl;
		f = fopen(argv[1], "r");

		if (!f) {
			fprintf(stderr, "Could not open %s\n", argv[1]);
			exit(-1);
		}

	}

	Mat frame;
	int posFrame;

	bool firstFrame = true;
	const char baseDir[] = "./";
	static Mat ch1, ch2, ch3;
	static Mat ch1A, ch2A, ch3A;

	char fileName[2048];
	char fileName1[2048];
	char rawName[2048];
	char tmpName[2048];
	char roiChar[10];

	char ROI_directory[2048];

	sprintf(ROI_directory, "%s/selectROI/roiCoords.txt", c.roi_coords_dir);
	FILE *inputROI = fopen(ROI_directory, "r");

	if (!inputROI) {
		c.iW = 0;
		c.iH = 0;
	} else {
		for (int i = 0; i < 4; i++) {

			fscanf(inputROI, "%s", roiChar);
			string str(roiChar);

			float myValue = atof(str.c_str());
			if (i == 0)
				c.iX = (int) myValue;
			else if (i == 1)
				c.iY = (int) myValue;
			else if (i == 2)
				c.iW = (int) myValue;
			else
				c.iH = (int) myValue;
		}
	}
	//get N old frames
	vector < cv::Mat > oldFrames;
	cv::Mat tempFrame;
	cv::Mat itempFrame;

	for (int i = 0; i < c.NUMBER_OLD_FRAMES; i++) {
		if (useAVI) {
			_videoSource >> itempFrame;
			posFrame = _videoSource.get(CV_CAP_PROP_POS_FRAMES);
		} else {
			if (fgets(fileName, sizeof fileName, f) != NULL) {

				int slen = strlen(fileName);
				if (fileName[slen - 1] == '\n')
					fileName[slen - 1] = 0;
				cout << fileName << endl;
				itempFrame = imread(fileName);
			}
		}

		if (c.iH > 0 && c.iW > 0) {
			tempFrame = itempFrame(cv::Rect(c.iX, c.iY, c.iW, c.iH));

		} else {
			tempFrame = itempFrame.clone();

		}

		oldFrames.push_back(tempFrame);

	}
	if (useAVI) {
		_videoSource >> frame;
		posFrame = _videoSource.get(CV_CAP_PROP_POS_FRAMES);
	} else {
		if (fgets(fileName, sizeof fileName, f) != NULL)
			puts(fileName);

		int slen = strlen(fileName);
		if (fileName[slen - 1] == '\n')
			fileName[slen - 1] = 0;
		frame = imread(fileName);
	}
	Mat inputFrameS_temp = frame.clone();         //gray
	Mat inputFrameS;
	cv::cvtColor(inputFrameS_temp, inputFrameS, CV_BGR2GRAY);

	Mat iFrame_temp = frame.clone();         //gray
	Mat iFrame;
	cv::cvtColor(iFrame_temp, iFrame, CV_BGR2GRAY);

	Mat colorFrameS = frame.clone();
	Mat cFrame = frame.clone();

	Mat inputFrame;
	Mat colorFrame;

	if (c.iH > 0 && c.iW > 0) {
		inputFrame = iFrame(cv::Rect(c.iX, c.iY, c.iW, c.iH));
		colorFrame = cFrame(cv::Rect(c.iX, c.iY, c.iW, c.iH));
	} else {
		inputFrame = iFrame;
		colorFrame = cFrame;
	}

	Mat grayImage = cv::Mat::zeros(inputFrame.size(), CV_8UC1);
	Mat b1 = cv::Mat::zeros(inputFrame.size(), CV_8UC1);
	Mat b2 = cv::Mat::zeros(inputFrame.size(), CV_8UC1);

	Mat thresh = cv::Mat::zeros(inputFrame.size(), CV_8UC1);
	Mat Hi = cv::Mat::zeros(inputFrame.size(), CV_8UC1);
	Mat Lo = cv::Mat::zeros(inputFrame.size(), CV_8UC1);

	//classification map sent to update background/threshold

	float parentImageRowsF = (float) grayImage.rows;
	float parentImageColumnsF = (float) grayImage.cols;

	cv::Mat classificationMap = cv::Mat::ones((int) parentImageRowsF,
			(int) parentImageColumnsF, CV_8UC1) * 128;

	if (useAVI) {
		sprintf(rawName, "%d", frameCounter++);
	} else {
		sprintf(tmpName, "%s", fileName);
		strrchr(tmpName, '.')[0] = '\0';
		sprintf(rawName, "%s", strrchr(tmpName, '/') + 1);
		if (strlen(rawName) < 1)
			sprintf(rawName, "%s", tmpName);

		cout << "filename First: " << fileName << endl;
		cout << "temp: " << tmpName << endl;
		cout << "raw: " << rawName << endl;
		cout << "imageChips: " << c.image_chips << endl;
	}

	//run the first image
	run_program(c, firstFrame, inputFrame, colorFrame, b1, b2, oldFrames,
			thresh, Hi, Lo, classificationMap, baseDir, (const char *) rawName,
			frame);

	firstFrame = false;

	b2 = inputFrame.clone();

	//run the rest of the file list
	while (1) {

		static int frameCtr = 0;

		Mat nextFrame;
		Mat colorFrame;
		Mat colorFramePrevious;
		if (useAVI) {
			_videoSource >> frame;
			posFrame = _videoSource.get(CV_CAP_PROP_POS_FRAMES);
		} else {
			if (fgets(fileName, sizeof fileName, f) != NULL) {
				if (fileName != NULL)
					puts(fileName);

				int slen = strlen(fileName);
				if (fileName[slen - 1] == '\n')
					fileName[slen - 1] = 0;
				frame = imread(fileName);
			}
		}
		if (frame.channels() == 3 && frame.data != NULL) {
			Mat nextFrameS_temp = frame.clone();
			Mat nextFrameS;
			cv::cvtColor(nextFrameS_temp, nextFrameS, CV_BGR2GRAY);
			Mat nFrame_temp = frame.clone();
			Mat nFrame;
			cv::cvtColor(nFrame_temp, nFrame, CV_BGR2GRAY);

			Mat colorFrameS = frame.clone();
			Mat cFrame = frame.clone();

			if (c.iH > 0 && c.iW > 0) {
				nextFrame = nFrame(cv::Rect(c.iX, c.iY, c.iW, c.iH));
				colorFrame = cFrame(cv::Rect(c.iX, c.iY, c.iW, c.iH));
			} else {
				nextFrame = nFrame;
				colorFrame = cFrame;
			}
			if (useAVI) {

				sprintf(rawName, "%d", frameCounter++);
			} else {
				sprintf(tmpName, "%s", fileName);
				strrchr(tmpName, '.')[0] = '\0';
				fprintf(stdout, "\nSending file: %s\n", tmpName);
				sprintf(rawName, "%s", strrchr(tmpName, '/') + 1);
				if (strlen(rawName) < 1)
					sprintf(rawName, "%s", tmpName);
			}
			clock_t begin = clock();
			cout << "Processing Frame: " << rawName << endl;

			run_program(c, firstFrame, nextFrame, colorFrame, b1, b2, oldFrames,
					thresh, Hi, Lo, classificationMap, baseDir,
					(const char*) rawName, frame);

			cv::Mat previousFrame = nextFrame.clone();

			for (unsigned int i = 0; i < oldFrames.size() - 1; i++) {
				oldFrames[i] = oldFrames[i + 1];
			}
			unsigned int endIdx = oldFrames.size();

			oldFrames[endIdx - 1] = colorFrame;

			clock_t end = clock();

			double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

			cout << "Time Elapsed: " << elapsed_secs << endl;
			cout << "-------------------------------------------" << endl;

			b2 = inputFrame.clone();
			frameCtr++;

		} else
			break;
	}

	fprintf(stdout, "\nFinished.\n");
	return 0;
}

#endif

#endif
