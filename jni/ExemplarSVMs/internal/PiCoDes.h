#ifndef _PICODES_H
#define _PICODES_H

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <cv.h>
#include <ml.h>
#include <cxcore.h>
#include <highgui.h>
#include "Detect.h"
#include "../utils/ImageSet.h"
#include "../utils/Model.h"
#include "../utils/define.h"
#include "../libsvm/svmtrain.h"


using namespace std;
using namespace cv;


class CPicodes : CDetect {
public:
    CPicodes();
    CPicodes(const char *filename);
    ~CPicodes();
    
    void Train(vector<CImageSet*> dataset, int nExemplarCls);
    void Test(vector<CImageSet*> dataset, int nExemplarCls);
   
    int SingleTest(char *filename, float *scores);
    
    void Recognition(const char *filename, vector<RESSTRUCT> res, const char *clsLabels[]);
    
    void ReadModels(const char *modelNames[], int n);
    
    void SetModelFile(const char *filename) {
        strcpy(m_modelFile, filename);
    }
    
private:
    vector<CModel*> m_models;
    vector<RESSTRUCT> m_grid;
    CDetect m_detect;
    char m_modelFile[80];
    
    void RunDetection(CImageSet *data, int clsIdx, int idx1, int idx2);
    
   
    void Calibration(Mat &feat);
    
    void TrainSVM(int numClasses);
    void TrainRandomForest(int numClasses);
    
    void ReadModels(vector<CImageSet*> dataset, int n);
    
    Mat DrawResults(Mat display, RESSTRUCT hn, int classCode);
};



class CvRTreesMultiClass : public CvRTrees
{
public:
    CvRTreesMultiClass() {};
    ~CvRTreesMultiClass() {};
    
    int getNclass() {return nclasses;} 
    
    
    int predict_multi_class(Mat sample, int *votes, int *total_votes) 
    {
        int result = -1;
        
        if( nclasses > 0 ) //classification
        {
            int max_nvotes = 0;
            memset( votes, 0, sizeof(*votes)*nclasses );
            for(int k = 0; k < ntrees; k++ )
            {
                CvDTreeNode* predicted_node = trees[k]->predict( sample );
                int class_idx = predicted_node->class_idx;
                CV_Assert( 0 <= class_idx && class_idx < nclasses );
                
                ++votes[class_idx];
                if (votes[class_idx] > max_nvotes) {
                    max_nvotes = votes[class_idx]; 
                    result = class_idx;
                }
                
            }
            
            *total_votes = 0;
            for(int k = 0; k < nclasses; k++ ) {
                *total_votes += votes[k]; 
            }
            
        }
        else // regression
        {
            throw std::runtime_error("can only be used classification");
        }
        
        return result;
    }
};





#endif
