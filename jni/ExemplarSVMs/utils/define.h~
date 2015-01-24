#ifndef _DEFINE_H
#define _DEFINE_H

//#define _SCALE_IMAGES 1
//#define _ADD_TRANSLATION 1

#ifdef _ADD_TRANSLATION
#define SHIFTX 1
#define SHIFTY 1
#define SHIFT_STEP 3
#define MAX_DETECTED_WINDOWS 500 
#define TRAIN_MAX_MINE_ITERATIONS 1 
#else
#define SHIFTX 0
#define SHIFTY 0
#define SHIFT_STEP 1
#define MAX_DETECTED_WINDOWS 10000 //100
#define TRAIN_MAX_MINE_ITERATIONS 100
#endif

#define SHIFT_SIZE ((2*SHIFTX+1)*(2*SHIFTY+1))

#define _MULTITHREADED 1
#define NUM_THREADS 4

#define USE_FLIP 1

#define MIN_IMG_DIM 64
#define MAX_DETECTED_RETURNS 5
#define DETECT_LEVELS_PER_OCTAVE 10
#define DETECT_MAX_SCALE 1.0
#define DETECT_MIN_SCALE 0.01
#define PYRAMID_MAXLEVELS 200
#define PYRAMID_MINDIMENSION 5
#define HOG_SBIN 8
#define HOG_MAXDIM 16 
#define NEGATIVE_SAMPLE_SIZE 400
#define MAX_MINED_WINDOWS_PER_ITERATION 10000 //200
#define MIN_MINED_IMAGES_PER_ITERATION 1 //10
#define MAX_DETECTION_FOR_CALIBRATION 100
#define DETECT_KEEP_THRESHOLD -1
#define DETECT_NMS_OS_THRESHOLD 0.5
#define SVM_TRAIN_C 0.1 //0.01
#define SVM_TRAIN_POSITIVES_CONSTANT 50
#define SVM_TRAIN_KEEP_NSV_MULTIPLER 3.0 //.25
#define CALIBRATION_NEIGHBOR_THRESHOLD 0.5
#define CALIBRATION_COUNT_THRESHOLD 0.5 
#define CALIBRATION_THRESHOLD -1 
#define MAX_VALIDATE_PER_IMAGE 1000 //400
#define MAX_VALIDATE_NEGATIVE 200000
#define MAX_MR_TAILSIZE 10
#define DETECTION_THRESHOLD 0.0

static const char *picodes_model_file = "Models/rf_mr.txt";

typedef enum  {MR, PLATT, CO_OCCURRENCE, BASELINE} CALIBRATION_METHOD;
static CALIBRATION_METHOD gCalibrationMethod = CO_OCCURRENCE;

enum SVM_TASK {SVM_TRAIN, SVM_VALIDATE, SVM_TEST};

typedef struct _RESSTRUCT {
    double score;
    double scale;
    double *xs;
    double x1, y1, x2, y2;
    double os;
    double area;
    long lsize;
    int exemplarID;
    int imgID;
    int label;
    int level;
    char strLabel[80];
} RESSTRUCT;

typedef struct _OCCURRENCE {
    int exid, boxid;
    double score;
    double os;
} OCCURRENCE;


#endif

