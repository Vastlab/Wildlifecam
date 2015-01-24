#ifndef _NMS_H
#define _NMS_H

#include <vector>
#include <string.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "../utils/Model.h"
#include "../utils/define.h"

using namespace std;
using namespace cv;


class CNMS {
public:
    CNMS();
    ~CNMS();
    
    double ComputeGroundTruthOverlap(RESSTRUCT box1, RESSTRUCT box2);
    Mat ComputeSelfOverlap(vector<RESSTRUCT> boxes, int n);
    
    vector<RESSTRUCT> NonMaximumSuppression(vector<RESSTRUCT> res, double osThreshold=DETECT_NMS_OS_THRESHOLD);
    

};

#endif
