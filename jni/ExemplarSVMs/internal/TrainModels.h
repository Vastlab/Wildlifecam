#ifndef _TRAINMODELS_H
#define _TRAINMODELS_H

#include <vector>
#include <string.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "../utils/ImageSet.h"
#include "../utils/Model.h"
#include "../utils/define.h"
#include "../libsvm/svmtrain.h"
#include "Detect.h"

using namespace std;
using namespace cv;



class CTrainModels {
public:
    CTrainModels();
    ~CTrainModels();
    
    void InitializeExemplars(CImageSet *dataset, int k1, int k2);
    void Train(CImageSet *dataset, int neg_idx1, int neg_idx2);
    void Validate(CImageSet *posSet, CImageSet *negSet, int neg_idx1, int neg_idx);
    void Calibrate(const char *task);
    
    void SaveModels(const char *prefix);
    void ReadModels(const char *prefix);
    
private:
    vector<CModel*> m_models;
    vector<RESSTRUCT> m_grid;
    CDetect m_detect;
    int m_posValSize, m_negValSize;
    bool m_bPlattCalibrated, m_bMRCalibrated, m_bOccMatrixCalibrated;
    Mat m_M;
    
    
    void MineTrainingData(CModel *m, CImageSet *trainingset, vector<int> *miningQueue, int *total_mines);
    vector<RESSTRUCT> MineNegative(CModel *m, CImageSet *trainingset, vector<int> *miningQueue, int *total_mines);
    
    void AddSupportVectors(CModel *m, vector<RESSTRUCT> hn);
    void UpdateModel(CModel *m);
    
    struct svm_model* TrainSVMs(CModel *m);
    void UpdateSVMsParams(CModel *m, struct svm_model *svmModel);
    
    
    void PlattCalibration(vector<RESSTRUCT> grid);
    void MetaCalibration(vector<RESSTRUCT> grid);
    void EstimateOccurrenceMatrix(vector<RESSTRUCT> grid);
    
    void FitSigmoid(vector<RESSTRUCT> grid);
    void FitWeibull(vector<RESSTRUCT> grid, int posSize, int negSize);
                         
    vector<RESSTRUCT> RunValidation(CImageSet *posSet, CImageSet *negSet, int neg_idx1, int neg_idx2);
    
};

#endif
