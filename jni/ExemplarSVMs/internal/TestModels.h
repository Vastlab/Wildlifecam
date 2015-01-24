#ifndef _TESTMODELS_H
#define _TESTMODELS_H

#include <vector>
#include <string.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "../utils/ImageSet.h"
#include "../utils/Model.h"
#include "../utils/define.h"
#include "Detect.h"

using namespace std;
using namespace cv;

#include "../ExemplarSVMs.h"

class CTestModels {
public:
    CTestModels();
    ~CTestModels();
    
    vector<RESSTRUCT> Test(Mat rgbImg, float threshold, int bDisplay=0);
    vector<RESSTRUCT> Test(Mat rgbImg, Mat colorSegment, float threshold, int bDisplay=0);
    
    void ReadModels(const char *prefix);
    
private:
    vector<CModel*> m_models;
    vector<RESSTRUCT> m_grid;
    CDetect m_detect;
    Mat m_M;
    Mat m_colorSegment;
    bool m_bUseColor, m_bOccReady;
    
    vector<RESSTRUCT> RunOccurrence(Mat img, float threshold);
    vector<RESSTRUCT> RunPlatt(Mat img, float threshold);
    vector<RESSTRUCT> RunMetaRecognition(Mat img, float threshold);
    vector<RESSTRUCT> RunBaseline(Mat img, float threshold);
    
    void DetectionFilter(vector<RESSTRUCT> &res, float threshold);

};

#endif
