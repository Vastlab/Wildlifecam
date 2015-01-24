#pragma once
#ifdef _CH_
#pragma package <opencv>
#endif


#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fstream>

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#ifndef EXEMPLAR_H
#define EXEMPLAR_H

#include "utils/ImageSet.h"
#include "utils/ColorDetection.h"
#include "internal/TrainModels.h"
#include "internal/TestModels.h"

#define LOG_TAG "ExemplarSVMs"

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

char* runExemplar(int argc, char **argv);


#endif // EXEMPLAR_H
