//============================================================================
// Name        : Squirrel_Detection_pipeline.h
// Author      : RC Johnson
// Version     :
// Copyright   : Copyright Securics Inc
// Description : Squirrel Pipeline Header file
//============================================================================
#pragma once
#ifdef _CH_
#pragma package <opencv>
#endif

#ifndef _EiC
#include <opencv2/opencv.hpp>

#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>
#include <vector>
#include <complex>
#include <string.h>
#include <time.h>
#include <limits>

#ifndef SQUIRREL_PIPELINE_H
#define SQUIRREL_PIPELINE_H

//Exemplar
#include "ExemplarSVMs/ExemplarSVMs.h"

#if !defined(__APPLE__)
#include <malloc.h>
#endif
#include <vector>
#endif

using namespace std;
using namespace cv;

//Why have to use this rather than the include guard? Ask whoever designed the preprocessor!
#ifndef CONFIGDEF
struct sConfig {

	char myID[256];

	//Stuff
	float fudge;
	int scale;

	//Squirrel Size
	int squirrelMinArea;
	int squirrelMaxArea;

	int g_starttime;
	int g_endtime;

	bool USE_MAX_AREA;

	bool USE_MIN_AREA;

	double mMaxArea;
	double bMaxArea;

	double mMinArea;
	double bMinArea;

	double maxAreaFudge;
	double minAreaFudge;

	int minROIWidth;
	int minROIHeight;

    double maxOverlapBeforeMerge;

    bool USE_MAX_AREA_AFTER_MERGE;
    double mMaxAreaAfterMerge;
    double bMaxAreaAfterMerge;
    int maxAreaFudgeAfterMerge;

    double minWidthOutputBox;
    double minHeightOutputBox;

    double maxCorrVal;

    //Exemplar
    int cropFudgeFactorExemplar;
    double minExemplarValue;

    bool SAVE_NEGATIVE_EXEMPLARS;
    bool USE_EXEMPLAR;

    char positiveExemplarMaxValFolder [2048];
    char positiveExemplarMinValFolder [2048];
    char negativeExemplarFolder [2048];

	//Location of files that scored rank 1 from ExemplarSVMs code
	char positiveExemplars[2048];

	//SVM model files location
	char model_dir[2048];

	char model_name[2048];

	char positive_exemplar_dir[2048];

	//ImageChips Location
	char image_chips[2048];

	//Image Gallery Location
	char image_gallery[2048];

	//Processed Images Location
	char processed_images[2048];

	//Output File location
	char output_file[2048];

	//Log file location
	char log_file[2048];

	//ROI Coords Driectory
	char roi_coords_dir[2048];

	int CONDITION1_HI;
	int CONDITION1_LOW;
	int CONDITION2_LOW;

	int NUMBER_OLD_FRAMES;
	//
	int UPDATE_NUMBER;

	int UPDATE_BACKGROUND_INC;

	//
	int HI_THRESHOLD_INC;

	int THRESHOLD_UPDATE_NEG_EXEMPLAR;

	int THRESHOLD_UPDATE_POS_EXEMPLAR;

	int THRESHOLD_UPDATE_TOO_BIG_SMALL;

	//
	int GLOBAL_INCREMENT;

	//
	int GLOBAL_THRESHOLD_INIT;

	float MIN_AREA_QCC;

	double PERCENT_PIXELS_NEEDED_HIGH_ONLY;
	double PERCENT_PIXELS_NEEDED_HIGH;
	double PERCENT_PIXELS_NEEDED_LOW;

	double maxRatio;
	//
	int THRESHOLD_DECREMENT_NUMBER;
	int THRESHOLD_DECREMENT_VALUE;

	//Dimensions of image to process a subrect with the dimensions
	int iX; // X
	int iY; // Y
	int iW; // width
	int iH; // height

};
#define CONFIGDEF
#endif

void LOGGING(int level, const char *fmt, ...);

void Update_Threshold_Background(bool firstFrame, Mat inputFrame,
		Mat Background1, Mat Background2, Mat Thresholds, Mat Blobs,
		Mat Classifications, int* GLOBAL_THRESHOLD);

void Background_Subtraction(Mat& diffImage, Mat inputFrame, Mat Background1,
		Mat Background2, Mat Thresholds, Mat Detected_Image_HI,
		Mat Detected_Image_LOW, Mat classificationMap, int* GLOBAL_THRESHOLD,
		int HI_THRESHOLD_INC);

void QCC(Mat imgHi, Mat imgLow, Mat parentImage);

void FindBlobs(const cv::Mat &binary,
		std::vector<std::vector<cv::Point2i> > &blobs);

vector<int> run_program(sConfig &config, bool firstFrame, Mat grayImage,
		Mat colorFrame, Mat b1, Mat b2, vector<cv::Mat> oldFrames, Mat thresh,
		Mat Hi, Mat Lo, Mat classificationMap, const char *baseName,
		const char* rawName, Mat frame);

bool updateConfig(sConfig &config);

#endif /*SQUIRREL_PIPELINE_H*/
