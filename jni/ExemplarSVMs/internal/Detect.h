#ifndef _DETECT_H
#define _DETECT_H

#include <vector>
#include <string.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "NMS.h"
#include "../utils/Model.h"
#include "../utils/define.h"

using namespace std;
using namespace cv;

struct INFO {
    int i, j;
    double score;
};



class CDetect : public CNMS {
public:
    CDetect();
    ~CDetect();
    
    vector<RESSTRUCT> RunDetection(Mat *img, vector<CModel*> *m, double osThreshold=DETECT_NMS_OS_THRESHOLD);
 
    Mat RunROIDetection(Mat *img, vector<CModel*> *m);
};

#endif
