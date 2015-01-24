#ifndef _COLOR_DETECTION_
#define _COLOR_DETECTION_

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#define NUM_COLORS 10
#define NUM_ELEMENTS 3

typedef struct _COLORMODEL {
    float mu[NUM_COLORS][NUM_ELEMENTS];
    float sigmaInv[NUM_COLORS][NUM_ELEMENTS][NUM_ELEMENTS];
    float det[NUM_COLORS];
    float wt[NUM_COLORS];
    float threshold;
    int k;
} COLORMODEL;


using namespace cv;
using namespace std;

class CColorDetection {
public:
    CColorDetection();
    ~CColorDetection();
    
    Mat GetColorSegment(Mat rgbImg, const char *color);
    float* GetColorHistogram(Mat rgbImg);
    int GetNumColors() {return NUM_COLORS+1;}
    const char** GetColorNames();
    
private:
    COLORMODEL *m_colorModels;
    int m_numColors;
    
    
    Mat Convert2HSV(Mat rgbImg);
    Mat RGB2HSV(Mat rgb); 
    Mat ComputeGMMPDF(Mat data, COLORMODEL *model);
    Mat Thresholding(Mat score, Mat skin, Mat mask, float threshold, long *count);
    Mat Thresholding(Mat score, Mat skin, float threshold);
    Mat DetectColor(Mat hsvImg, Mat mask, int k, long *count);
    Mat DetectColor(Mat hsvImg, int k);
    void GetMultiColorCount(Mat *matArray, long *count, long *total);
    void ReadModel(const char *fname, COLORMODEL *model);
    
};

#endif
