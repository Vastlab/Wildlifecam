/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#pragma once

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <jni.h>
#include <malloc.h>
#include <time.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <string.h>

//This should probably clear up some of the errors.
#include "Squirrel_pipeline.h"
#include "com_securics_wildlifecapture_MainActivity.h"

#define ANDROID_LOG_VERBOSE ANDROID_LOG_DEBUG

#define LOG_TAG "CVJNI"

#define LOGV(...) __android_log_print(ANDROID_LOG_SILENT, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

//#include <opencv2/opencv.hpp>
#ifndef _CVJNI_CPP
#define _CVJNI_CPP

#ifdef __cplusplus
extern "C" {
#endif

cv::Mat Background1;
cv::Mat Background2;
vector<cv::Mat> Background3;

cv::Mat Thresholds;
cv::Mat dI_Hi;
cv::Mat dI_Lo;
cv::Mat classificationMap;

int imageLength;
int imageWidth;

static bool runSwitch = true;

static bool firstFrame = true;

static sConfig theConfig;

JNIEXPORT jint JNICALL Java_com_securics_wildlifecapture_MainActivity_giveConfig(
		JNIEnv *env, jclass, jstring key1, jstring value1) {

	const char * key = env->GetStringUTFChars(key1, 0);
	const char * value = env->GetStringUTFChars(value1, 0);

	LOGGING(1, "I just received %s -> %s", key, value);
	LOGI("I just received %s -> %s", key, value);

	if (!strcmp(key, "fudge")) {
		theConfig.fudge = atof(value);
	} else if (!strcmp(key, "scale")) {
		theConfig.scale = atoi(value);
	} else if (!strcmp(key, "UPDATE_NUMBER")) {
		theConfig.UPDATE_NUMBER = atoi(value);
	} else if (!strcmp(key, "HI_THRESHOLD_INC")) {
		theConfig.HI_THRESHOLD_INC = atoi(value);
	} else if (!strcmp(key, "GLOBAL_INCREMENT")) {
		theConfig.GLOBAL_INCREMENT = atoi(value);
	} else if (!strcmp(key, "GLOBAL_THRESHOLD_INIT")) {
		theConfig.GLOBAL_THRESHOLD_INIT = atoi(value);
	} else if (!strcmp(key, "THRESHOLD_DECREMENT_NUMBER")) {
		theConfig.THRESHOLD_DECREMENT_NUMBER = atoi(value);
	} else if (!strcmp(key, "THRESHOLD_DECREMENT_VALUE")) {
		theConfig.THRESHOLD_DECREMENT_VALUE = atoi(value);
	} else if (!strcmp(key, "THRESHOLD_UPDATE_NEG_EXEMPLAR")) {
		theConfig.THRESHOLD_UPDATE_NEG_EXEMPLAR = atoi(value);
	} else if (!strcmp(key, "THRESHOLD_UPDATE_POS_EXEMPLAR")) {
		theConfig.THRESHOLD_UPDATE_POS_EXEMPLAR = atoi(value);
	} else if (!strcmp(key, "THRESHOLD_UPDATE_TOO_BIG_SMALL")) {
		theConfig.THRESHOLD_UPDATE_TOO_BIG_SMALL = atoi(value);
	} else if (!strcmp(key, "UPDATE_BACKGROUND_INC")) {
		theConfig.UPDATE_BACKGROUND_INC = atoi(value);
	} else if (!strcmp(key, "MIN_AREA_QCC")) {
		theConfig.MIN_AREA_QCC = atoi(value);
	} else if (!strcmp(key, "PERCENT_PIXELS_NEEDED_HIGH_ONLY")) {
		theConfig.PERCENT_PIXELS_NEEDED_HIGH_ONLY = atof(value);
	} else if (!strcmp(key, "PERCENT_PIXELS_NEEDED_HIGH")) {
		theConfig.PERCENT_PIXELS_NEEDED_HIGH = atof(value);
	} else if (!strcmp(key, "PERCENT_PIXELS_NEEDED_LOW")) {
		theConfig.PERCENT_PIXELS_NEEDED_LOW = atof(value);
	} else if (!strcmp(key, "NUMBER_OLD_FRAMES")) {
		theConfig.NUMBER_OLD_FRAMES = atoi(value);
	} else if (!strcmp(key, "USE_MAX_AREA")) {
		theConfig.USE_MAX_AREA = atoi(value);
	} else if (!strcmp(key, "USE_MIN_AREA")) {
		theConfig.USE_MIN_AREA = atoi(value);
	} else if (!strcmp(key, "mMaxArea")) {
		theConfig.mMaxArea = atof(value);
	} else if (!strcmp(key, "bMaxArea")) {
		theConfig.bMaxArea = atof(value);
	} else if (!strcmp(key, "mMinArea")) {
		theConfig.mMinArea = atof(value);
	} else if (!strcmp(key, "bMinArea")) {
		theConfig.bMinArea = atof(value);
	} else if (!strcmp(key, "maxAreaFudge")) {
		theConfig.maxAreaFudge = atoi(value);
	} else if (!strcmp(key, "minAreaFudge")) {
		theConfig.minAreaFudge = atoi(value);
	} else if (!strcmp(key, "minROIWidth")) {
		theConfig.minROIWidth = atoi(value);
	} else if (!strcmp(key, "minROIHeight")) {
		theConfig.minROIHeight = atoi(value);
	} else if (!strcmp(key, "maxOverlapBeforeMerge")) {
		theConfig.maxOverlapBeforeMerge = atof(value);
	} else if (!strcmp(key, "USE_MAX_AREA_AFTER_MERGE")) {
		theConfig.USE_MAX_AREA_AFTER_MERGE = atoi(value);
	} else if (!strcmp(key, "mMaxAreaAfterMerge")) {
		theConfig.mMaxAreaAfterMerge = atof(value);
	} else if (!strcmp(key, "bMaxAreaAfterMerge")) {
		theConfig.bMaxAreaAfterMerge = atof(value);
	} else if (!strcmp(key, "maxAreaFudgeAfterMerge")) {
		theConfig.maxAreaFudgeAfterMerge = atoi(value);
	} else if (!strcmp(key, "maxRatio")) {
		theConfig.maxRatio = atof(value);
	} else if (!strcmp(key, "maxCorrVal")) {
		theConfig.maxCorrVal = atof(value);
	} else if (!strcmp(key, "minWidthOutputBox")) {
		theConfig.minWidthOutputBox = atoi(value);
	} else if (!strcmp(key, "minHeightOutputBox")) {
		theConfig.minHeightOutputBox = atoi(value);
	} else if (!strcmp(key, "cropFudgeFactorExemplar")) {
		theConfig.cropFudgeFactorExemplar = atoi(value);
	} else if (!strcmp(key, "minExemplarValue")) {
		theConfig.minExemplarValue = atof(value);
	} else if (!strcmp(key, "SAVE_NEGATIVE_EXEMPLARS")) {
		theConfig.SAVE_NEGATIVE_EXEMPLARS = atoi(value);
		//Strings
	} else if (!strcmp(key, "svmModelFiles")) {
		sprintf(theConfig.model_dir, "%s", value);
	} else if (!strcmp(key, "svmModelName")) {
		sprintf(theConfig.model_name, "%s", value);
	} else if (!strcmp(key, "processLog")) {
		sprintf(theConfig.log_file, "%s", value);
	} else if (!strcmp(key, "deviceID")) {
		sprintf(theConfig.myID, "%s", value);
	} else if (!strcmp(key, "negativeExemplarFolder")) {
		sprintf(theConfig.negativeExemplarFolder, "%s", value);
	} else if (!strcmp(key, "posExemplarMaxFolder")) {
		sprintf(theConfig.positiveExemplarMaxValFolder, "%s", value);
	} else if (!strcmp(key, "posExemplarMinFolder")) {
		sprintf(theConfig.positiveExemplarMinValFolder, "%s", value);
	} else if (!strcmp(key, "imageGallery")) {
		sprintf(theConfig.image_gallery, "%s", value);
	} else if (!strcmp(key, "processedImages")) {
		sprintf(theConfig.processed_images, "%s", value);
	} else if (!strcmp(key, "chipsGallery")) {
		sprintf(theConfig.image_chips, "%s", value);
	} else if (!strcmp(key, "outputFile")) {
		sprintf(theConfig.output_file, "%s", value);
	} else if (!strcasecmp(key, "posExemplars")) {
		sprintf(theConfig.positiveExemplars, "%s", value);
	} else
		return 1;

	updateConfig(theConfig);

	return 0;
}

JNIEXPORT jint JNICALL Java_com_securics_squirrelgpu_NativeInterface_getStartTime(
		JNIEnv *, jclass) {

	return 0;

}

JNIEXPORT jint JNICALL Java_com_securics_squirrelgpu_NativeInterface_getEndTime(
		JNIEnv *, jclass) {

	return 0;

}

static JNINativeMethod sMethods[] =
		{
				/* name, signature, funcPtr */
//The first item was originally named: "pushImage", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)[I
				{ "getStartTime", "()I",
						(void*) Java_com_securics_squirrelgpu_NativeInterface_getStartTime },
				{ "getEndTime", "()I",
						(void*) Java_com_securics_squirrelgpu_NativeInterface_getEndTime }, };

static JNINativeMethod mMethods[] = { { "giveConfig",
		"(Ljava/lang/String;Ljava/lang/String;)I",
		(void*) Java_com_securics_wildlifecapture_MainActivity_giveConfig }, {
		"doRevisedJNI",
		"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;DDDD)I",
		(void*) Java_com_securics_wildlifecapture_MainActivity_doRevisedJNI },

};

static int jniRegisterNativeMethods(JNIEnv *env, const char *className,
		JNINativeMethod* Methods, int numMethods) {

	LOGGING(1, "Registering Native Methods!!");
	jclass clazz = env->FindClass(className);

	if (clazz == NULL) {
		LOGE("Native registration unable to find class '%s'", className);
		return JNI_FALSE;
	}

	if (env->RegisterNatives(clazz, Methods, numMethods) < 0) {
		LOGE("RegisterNatives failed for '%s'", className);
		return JNI_FALSE;
	}
	return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	jint result = -1;

	if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
		return result;
	}

	jniRegisterNativeMethods(env, "com/securics/wildlifecapture/MainActivity",
			mMethods, 2);
	return JNI_VERSION_1_6;
}

JNIEXPORT jint JNICALL Java_com_securics_wildlifecapture_MainActivity_doRevisedJNI(
		JNIEnv *env, jobject obj, jstring rawfilename, jstring config,
		jstring filename, jdouble camVertDegRange, jdouble camHorizDegRange,
		jdouble picSizeX, jdouble picSizeY) {
	vector<int> boxes;
	//Modified code from the original push to JNI call:

	LOGGING(1, "Running revised image detector.");

	const char * _nativeStringName = env->GetStringUTFChars(filename, 0);
	const char * _nativeStringNameRaw = env->GetStringUTFChars(rawfilename, 0);
	const char * _nativeStringConfig = env->GetStringUTFChars(config, 0);

	Mat originalImage = imread(_nativeStringName, 0); //grayscale (,0)
	Mat colorFrame = imread(_nativeStringName);

	LOGGING(1, "Revised Image location: %s", _nativeStringName);
	LOGGING(1, "Revised Image RawName: %s", _nativeStringNameRaw);
	LOGI("Revised Image location: %s", _nativeStringName);
	LOGI("Revised Image RawName: %s", _nativeStringNameRaw);

	int gHeight = originalImage.rows;
	int gWidth = originalImage.cols;

	if (Background1.empty()) {
		Background1 = cv::Mat::ones(gHeight, gWidth, CV_8UC1) * 255;
		Background2 = cv::Mat::ones(gHeight, gWidth, CV_8UC1) * 255;
		//Background3 = cv::Mat::ones(gHeight, gWidth, CV_8UC1) * 255;

		Thresholds = cv::Mat::zeros(gHeight, gWidth, CV_8UC1);

		dI_Hi = cv::Mat::zeros(gHeight, gWidth, CV_8UC1);
		dI_Lo = cv::Mat::zeros(gHeight, gWidth, CV_8UC1);

		classificationMap = cv::Mat::zeros(gHeight, gWidth, CV_8UC1);
	}

	if (originalImage.empty()) {
		LOGE("Error loading pixel array.");
	}

	LOGI("Log File: %s", theConfig.log_file);
	LOGGING(1, "Log File: %s\n", theConfig.log_file);

	/*
	 * Here all these config files should be replaced with "theConfig" struct!
	 * Also, do a check to make sure the values of theConfig have been set first.
	 * Remove string config all together. Just use theConfig.
	 */

	clock_t begin = clock();

	//initalize background 3
	if (firstFrame == true) {
		//Background3 = colorFrame.clone();
		Background3.push_back(colorFrame);
	}

	if (originalImage.rows == Background1.rows
			&& originalImage.cols == Background1.cols) {
		LOGGING(1, "imageGallery: %s", theConfig.image_gallery);

		boxes = run_program(theConfig, firstFrame, originalImage, colorFrame,
				Background1, Background2, Background3, Thresholds, dI_Hi, dI_Lo,
				classificationMap, _nativeStringName, _nativeStringNameRaw,
				colorFrame);

	} else {
		LOGGING(1, "IMAGES NOT EQUAL: %s", _nativeStringNameRaw);
		boxes.push_back(-1);
	}
	/*
	 Mat ch1, ch2, ch3;
	 Mat ch1A, ch2A, ch3A;
	 // "channels" is a vector of 3 Mat arrays:
	 vector<Mat> channels(3);
	 // split img:
	 split(Background3, channels);

	 vector<Mat> channelsA(3);
	 // split img:
	 split(colorFrame, channelsA);

	 // get the channels (dont forget they follow BGR order in OpenCV)
	 ch1 = channels[0];
	 ch2 = channels[1];
	 ch3 = channels[2];

	 ch1A = channelsA[0];
	 ch2A = channelsA[1];
	 ch3A = channelsA[2];

	 for (int y = 0; y < Background3.rows; y++)
	 for (int x = 0; x < Background3.cols; x++) {

	 float temp1 = ((float) (ch1.at<uchar>(y, x))
	 * (float) ((7.0 / 8.0)));

	 float temp2 = ((float) (ch2.at<uchar>(y, x))
	 * (float) ((7.0 / 8.0)));

	 float temp3 = ((float) (ch3.at<uchar>(y, x))
	 * (float) ((7.0 / 8.0)));

	 float temp1A = ((float) (ch1A.at<uchar>(y, x))
	 * (float) ((1.0 / 8.0)));

	 float temp2A = ((float) (ch2A.at<uchar>(y, x))
	 * (float) ((1.0 / 8.0)));

	 float temp3A = ((float) (ch3A.at<uchar>(y, x))
	 * (float) ((1.0 / 8.0)));

	 ch1.at<uchar>(y, x) = (uchar) temp1 + (uchar) temp1A;
	 ch2.at<uchar>(y, x) = (uchar) temp2 + (uchar) temp2A;
	 ch3.at<uchar>(y, x) = (uchar) temp3 + (uchar) temp3A;
	 }

	 //merge the channels
	 cv::merge(channels, Background3);
	 */
	clock_t end = clock();

	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

	LOGGING(1, "Time Elapsed: %f\n", elapsed_secs);

	LOGGING(1, "Finished run_program!!!\n");

	firstFrame = false;

	jint length = boxes.size();
	LOGGING(1, "Length is %d", length);

	//jintArray result = (env)->NewIntArray(1);//length);
	//*result[0]=1;

	jint result;

	//Check for the runSwitch and act accordingly.
	if (runSwitch) {
		//Just some arbitrary number.
		result = 765;
		runSwitch = false;
	}

	else {
		result = 0;
		runSwitch = true;
	}

	jint temp[length];

	for (int i = 0; i < length; i++) {
		temp[i] = boxes[i];
	}

	//(env)->SetIntArrayRegion(result, 0, length, temp);

	if (length > 0) {
		jint count = boxes[0];
		LOGGING(1, "Count is %d.", count);
	}

	LOGGING(1, "Finished running jni. Returning result.");

	//return result;
	return result;

	return 0;
}

#ifdef __cplusplus
}
#endif

#endif /*_CVJNI_CPP*/
