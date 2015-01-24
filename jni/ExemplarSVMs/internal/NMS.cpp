#include <stdio.h>
#include <stdlib.h>
#include "NMS.h"

#define eps 0.0001

extern SVM_TASK gSVMsTask; 

bool mysort(const RESSTRUCT &a, const RESSTRUCT &b)
{
    // ascending order
    return (a.score < b.score);
}


CNMS::CNMS()
{
}

CNMS::~CNMS()
{
}

double CNMS::ComputeGroundTruthOverlap(RESSTRUCT box1, RESSTRUCT box2)
{
    double os=0;
    
    double area1 = abs(box1.x2-box1.x1) * abs(box1.y2 - box1.y1);
    double area2 = abs(box2.x2-box2.x1) * abs(box2.y2 - box2.y1);
    
    double xx1 = max(box1.x1, box2.x1);
    double yy1 = max(box1.y1, box2.y1);
    double xx2 = min(box1.x2, box2.x2);
    double yy2 = min(box1.y2, box2.y2);
    
    double w = max(0.0, xx2-xx1);
    double h = max(0.0, yy2-yy1);
    
    if (w > 0 && h > 0)
        os = (w * h) / (double) (area1 + area2 - w*h + eps);
    
    return os;
}


Mat CNMS::ComputeSelfOverlap(vector<RESSTRUCT> boxes, int n)
{ 

    Mat x = Mat::zeros(n, boxes.size(), CV_64F);
    
    for (int j=0; j<(int)boxes.size(); j++) {
        RESSTRUCT box_j = boxes.at(j);
        for (int i=0; i<(int)boxes.size(); i++) {
            RESSTRUCT box_i = boxes.at(i);
            
            double os = ComputeGroundTruthOverlap(box_i, box_j);
         
            if (os > CALIBRATION_NEIGHBOR_THRESHOLD) {
                
                //neighbor[j] = i;
                
                x.at<double>(box_i.exemplarID, j) = box_i.score;
            }
        }
    }
    
    return x;
}


vector<RESSTRUCT> CNMS::NonMaximumSuppression(vector<RESSTRUCT> res, double osThreshold)
{
    if (res.size() == 0) return res;
    
    std::sort(res.begin(), res.end(), mysort);
    
    int *lst = new int [res.size()];
    memset(lst, 0, sizeof(int)*res.size());
    
    vector<int> pick;
    for (int k=(int)res.size()-1; k>=0; k--) {
        
        if (lst[k] == 1) continue;
        
        pick.push_back(k);
        
        RESSTRUCT box = res.at(k); 
        double x1 = box.x1;
        double y1 = box.y1;
        double x2 = box.x2;
        double y2 = box.y2;
        
        for (int i=0; i<k; i++) {
            RESSTRUCT bbs = res.at(i);
            double xx1 = max(x1, bbs.x1);
            double yy1 = max(y1, bbs.y1);
            double xx2 = min(x2, bbs.x2);
            double yy2 = min(y2, bbs.y2);
        
            double w = max(0.0, xx2-xx1);
            double h = max(0.0, yy2-yy1);
            
            double area = abs(bbs.x2-bbs.x1) * abs(bbs.y2 - bbs.y1);
        
            double overlap = w*h / (double)area; 
            if (overlap > osThreshold) lst[i] = 1;
        }     
    }
    
    vector<RESSTRUCT> res_nms;
    for (int i=0; i<(int)pick.size(); i++) {
        int idx = pick.at(i);
        RESSTRUCT bbs = res.at(idx);
        
        if (SVM_TRAIN == gSVMsTask && NULL != bbs.xs) {
            bbs.xs = new double [bbs.lsize];
            memcpy(bbs.xs, res.at(idx).xs, sizeof(double)*bbs.lsize);
        }
        res_nms.push_back(bbs);
    }
    
    delete [] lst;
    
    return res_nms;
}

