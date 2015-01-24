#include <stdio.h>
#include <stdlib.h>
#include "Detect.h"
#include "Pyramid.h"


extern SVM_TASK gSVMsTask; 


CDetect::CDetect()
{
    
}

CDetect::~CDetect()
{
}


vector<RESSTRUCT> CDetect::RunDetection(Mat *img, vector<CModel*> *m, double osThreshold)
{
    int targetlevel;
    vector<RESSTRUCT> resData;
    
    int inc = 1; 
    
    for (int i=0; i<(int)m->size(); i++) { 
        
        vector<PYRLEVEL*> pyramid = GetPyramidPadZero(img, m->at(i)->getSizeW(), m->at(i)->getSizeH(), &targetlevel);
                
        vector<RESSTRUCT> resRecord;
        
        double thresh = DETECT_KEEP_THRESHOLD;
        double newthresh = -999999;
        

        double maxScore=-99999;
        for (int k=targetlevel; k>=0; k-=inc) {
            PYRLEVEL *pyr = pyramid.at(k);
            
            int xsize, ysize;           
            double **res = Convolve(pyr, m->at(i), &xsize, &ysize);    
            
            double oldMaxScore = maxScore;
            int templateWidth =  m->at(i)->getSizeW();
            int templateHeight = m->at(i)->getSizeH();
            
             for (int jj=0; jj<ysize; jj++) {
                for (int ii=0; ii<xsize; ii++) {
                    if (res[ii][jj] > thresh) {    
                        
                        RESSTRUCT bbs;
                        bbs.scale = pyr->scaler;
                        bbs.x1 = max(0.0,(ii - pyr->padsize) * pyr->scaler_x);
                        bbs.y1 = max(0.0,(jj - pyr->padsize) * pyr->scaler_y);
                        bbs.x2 = min(img->cols-1.0, bbs.x1 + templateWidth  * pyr->scaler_x); 
                        bbs.y2 = min(img->rows-1.0, bbs.y1 + templateHeight * pyr->scaler_y); 
                        
                        bbs.level = k;
                        bbs.exemplarID = i;
                        bbs.score = res[ii][jj] - m->at(i)->getBias();
                        
                        // get ROI feature p->feat 
                        if (SVM_TRAIN == gSVMsTask)
                            bbs.xs = getROI(pyr, ii, jj, m->at(i)->getSizeW(), m->at(i)->getSizeH());
                        else
                            bbs.xs = NULL;
                        
                        bbs.lsize = m->at(i)->getSizeW() * m->at(i)->getSizeH() * m->at(i)->getSizeD();
                        
                        resRecord.push_back(bbs);
                        
                        
                        newthresh = max(newthresh, res[ii][jj]);
                    }
                }
            }
            if (SVM_TRAIN != gSVMsTask) thresh = newthresh;
            
            for (int j=0; j<xsize; j++) delete [] res[j];
            delete [] res;
        }
        
    
        // release memory 
        for (int k=0; k<(int)pyramid.size(); k++) {
            PYRLEVEL *pyr = pyramid.at(k);
            double *f = pyr->feat;
            
            delete [] f;
            delete pyr;
        }
        
        
        if (osThreshold < 1) {
            vector<RESSTRUCT> hn = NonMaximumSuppression(resRecord, osThreshold);
            
            resData.insert(resData.end(), hn.begin(), hn.end());
            
            for (int i=0; i<(int)resRecord.size(); i++) {
                RESSTRUCT hn = resRecord.at(i);
                delete [] hn.xs;
            }
            resRecord.clear();
        }
        else
            resData = resRecord;
    }
    
    return resData;
}


Mat CDetect::RunROIDetection(Mat *img, vector<CModel*> *m)
{
    Mat f = Mat(1, (int)m->size(), CV_32FC1);
    for (int i=0; i<(int)m->size(); i++) {
        
        int w = m->at(i)->getImgWidth();
        int h= m->at(i)->getImgHeight();
        Mat dst = cv::Mat::zeros(h, w, CV_8UC3);
        
        cv::resize(*img, dst, dst.size());
     
        int targetlevel;
        vector<PYRLEVEL*> pyramid = GetPyramid1(&dst, &targetlevel);
        PYRLEVEL *pyr = pyramid.at(targetlevel);
        
        double s = Convolve1(pyr, m->at(i));
        
        f.at<float>(0,i) = s - m->at(i)->getBias();  
        
        
        // release memory
        for (int k=0; k<(int)pyramid.size(); k++) {
            PYRLEVEL *pyr = pyramid.at(k);
            double *f = pyr->feat;
            
            delete [] f;
            
            delete pyr;
        }

    }
    
    return f;
}

