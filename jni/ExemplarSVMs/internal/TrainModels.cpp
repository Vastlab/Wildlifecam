#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fstream>
#include "TrainModels.h"
#include "Pyramid.h"


bool myfun1 (double a, double b) { return (a > b); } // acending order
bool myfun2 (double a, double b) { return (a < b); } // decending order

struct node {
    double val;
    int idx;
};


SVM_TASK gSVMsTask; 


static bool mysort(const RESSTRUCT &a, const RESSTRUCT &b)
{
    // decending order
    return (a.score > b.score);
}


// decending order
static int mysort_struct(const void *arg1, const void *arg2) 
{        
    double v1, v2;
    v1 = *(double*)(arg1);
    v2 = *(double*)(arg2);	
    
    if (v1 < v2) return 1;
	
	if (v1 == v2) return 0;
    
	return -1; 
}

static void ReleaseRes(vector<RESSTRUCT> res)
{
    // release memory
    for (int i=0; i<(int)res.size(); i++) {
        RESSTRUCT bbs = res.at(i);
        if (NULL != bbs.xs) {delete [] bbs.xs; bbs.xs=NULL;}
    }
    res.clear();
}


CTrainModels::CTrainModels()
{
    m_bPlattCalibrated = false;
    m_bMRCalibrated = false;
    m_bOccMatrixCalibrated = false;
}

CTrainModels::~CTrainModels()
{
    for (int k=0; k<(int)m_models.size(); k++) {
        CModel *ptr = m_models.at(k);
        delete ptr;
    }
}


void CTrainModels::InitializeExemplars(CImageSet *dataset, int k1, int k2)
{
    CDetect detect;
    
    for (int k=k1; k<k2; k++) {
        cout << "Initializing exemplars " << k << " ....\n";
        Mat img = dataset->GetImage(k);
        
        if (min(img.cols, img.rows) < MIN_IMG_DIM) continue;
        
        int targetlevel;
        vector<PYRLEVEL*> pyramid = GetPyramid(&img, &targetlevel);
        
        CModel *m = new CModel;
        m->setSizeW(pyramid.at(targetlevel)->w);
        m->setSizeH(pyramid.at(targetlevel)->h);
        m->setSizeD(pyramid.at(targetlevel)->d);
        
        m->setScale(pyramid.at(targetlevel)->scaler);
        m->setScaleX(pyramid.at(targetlevel)->scaler_x);
        m->setScaleY(pyramid.at(targetlevel)->scaler_y);
        
        // initial template W and positive samples
        int idx = (int) floor(SHIFT_SIZE/2.0);
        m->setWS(pyramid.at(targetlevel)->mfeat[idx]);
        
        for (int i=0; i<SHIFT_SIZE; i++) {
            m->addXS(pyramid.at(targetlevel)->mfeat[i]);
        }

        m->setImgWidth(img.cols);
        m->setImgHeight(img.rows);
        
        m_models.push_back(m);
        
    
        for (int i=0; i<(int)pyramid.size(); i++) {
            PYRLEVEL *p = pyramid.at(i);
             
            for (int j=0; j<SHIFT_SIZE; j++) {
                double *f = p->mfeat[j];
                delete [] f;
            }
                
            delete p;
        }
    }
    cout << "Initialization Done ...\n";
}


vector<RESSTRUCT> CTrainModels::MineNegative(CModel *m, CImageSet *trainingset, vector<int> *miningQueue, int *total_mines)
{
    vector<RESSTRUCT> hn;
    if (miningQueue->size() == 0) return hn;
        
    vector<CModel*> models;
    models.push_back(m); // train each exemplar individually 
    
    vector<int> mining_queue;
    int num_violated_img = 0;
    int num_detected_windows = 0;
    
    
    for (int k=0; k<(int)miningQueue->size(); k++) {
        int idx = miningQueue->at(k);
        Mat img = trainingset->GetImage(idx);
        
        // if it is too small, do not mine this negative sample
        if (min(img.cols, img.rows) < 2*MIN_IMG_DIM) continue;
        
        //vector<RESSTRUCT> res = m_detect.RunDetection(&img, &models, 0.35);
        vector<RESSTRUCT> res = m_detect.RunDetection(&img, &models, 1);
        
        if ((int)res.size() > 0) {
            std::sort(res.begin(), res.end(), mysort);
            
            cout << "Max score = " << res.at(0).score << endl; 
            if (res.at(0).score > -1) {
                num_violated_img++;
                mining_queue.push_back(k);
            }
            
            num_detected_windows += (int)res.size();
        }
        else {
            num_violated_img++;
            mining_queue.push_back(k);
        }

        for (int i=0; i<(int)res.size(); i++) {
            RESSTRUCT f = res.at(i);
            hn.push_back(f);
        }
        res.clear();
        
        
        cout << "Image: (" << idx << "," << (int)miningQueue->size() << ")" << " " << num_detected_windows << endl; 
        
        if (num_detected_windows > MAX_MINED_WINDOWS_PER_ITERATION && (miningQueue->size()-k) > MIN_MINED_IMAGES_PER_ITERATION) break;
    }
    
    if (num_violated_img > 0) {
        //update mining queue
        int elementRemoved=0;
        for (int i=0; i<num_violated_img; i++) {
            int idx = mining_queue.at(i);
            miningQueue->erase(miningQueue->begin()+idx-elementRemoved);
            elementRemoved++;
        }
    }
        
    
    *total_mines += num_violated_img;

    
    // keep top detected windows and remove the rest
    std::sort(hn.begin(), hn.end(), mysort);
    int elementRemoved=0;
    int iter = (int)hn.size() - MAX_DETECTED_WINDOWS;
    for (int i=0; i<iter; i++) {
        RESSTRUCT *f = &hn.at(MAX_DETECTED_WINDOWS);
        if (NULL != f->xs) {delete [] f->xs; f->xs=NULL;}
        hn.erase(hn.begin()+MAX_DETECTED_WINDOWS);
        elementRemoved++;
    }
        
    return hn;
}


void CTrainModels::AddSupportVectors(CModel *m, vector<RESSTRUCT> hn)
{
    for (int k=0; k<(int)hn.size(); k++) {
        RESSTRUCT f = hn.at(k);
        m->addSVs(f.xs);
    }
     
    cout << "Num of SVs = " << m->getNumSVs() << endl;
}

struct svm_model* CTrainModels::TrainSVMs(CModel *m)
{
    // HOG dimensions
    long n = m->getSizeW()*m->getSizeH()*m->getSizeD();
    // number of negative samples
    int num_sv = m->getNumSVs();

    // number of positive samples
    int num_sx = m->getNumXS();
    
    int *supery = new int [num_sv+num_sx];
    memset(supery, -1, sizeof(int)*(num_sv+num_sx));
    
    double *superx = new double [n*(num_sv+num_sx)];
    
    // setup the positive sample
    for (int i=0; i<num_sx; i++) {
        supery[i] = 1;
        double *ptr = m->getXS(i);
        for (int j=0; j<n; j++) {
            superx[j*(num_sv+num_sx)+i] = ptr[j];
        }
    }
    
    // setup negative samples
    for (int i=num_sx; i<(num_sv+num_sx); i++) {
        double *ptr2 = m->getSV(i-num_sx);
        for (int j=0; j<n; j++) {
            superx[j*(num_sv+num_sx)+i] = ptr2[j];
        }
    }

    char cmd[80];
    sprintf(cmd,"-s 0 -t 0 -c %f -w1 %f -q",(float)SVM_TRAIN_C, (float)SVM_TRAIN_POSITIVES_CONSTANT);
    struct svm_model *svmModel = svmtrain(cmd, supery, superx, n, num_sv+num_sx);
     

    delete [] superx;
    delete [] supery;
    
    return svmModel;
}

void CTrainModels::UpdateSVMsParams(CModel *m, struct svm_model *svmModel)
{
    long n = m->getSizeW()*m->getSizeH()*m->getSizeD();
    int num_sv = m->getNumSVs();

    
    vector<Mat> svCoef = svmGetSVCoef();
    if (svCoef.size() != 1) {
        cout << "ERROR: Should be only one class for exemplarSVMs ...\n";
        
        svmfree(svmModel);
        exit(0);
    }
    else {
        Mat svcoef = svCoef.at(0);
        //check sv_coef for further update
        if (svcoef.rows == 0 || svcoef.cols == 0) {
            // get previous model and no update is needed
            cout << "GET OLD MODELS ..........\n";
        }
        else {
            Mat svData = svmGetSVs(n);
            
            vector<double> rho = svmGetRho();
            double b = rho.at(0);
            
            //for (int i=0; i<(int)rho.size(); i++)
            //    cout << rho.at(i) << endl;
            
            Mat wt = svData * svcoef;
          
            
            if (norm(wt) < 0.00001)
                cout << "Warning: learning broke down!\n";
            
            
            double maxv = -99999999;
            for (int i=0; i<m->getNumXS(); i++) {
                double *ptr = m->getXS(i);
            
                Mat x = Mat(n,1, CV_64F, ptr);
                Mat tmp = wt.t() * x - b;
                
                maxv = max(tmp.at<double>(0,0), maxv); 
            }
            cout << "Max positive: " << maxv << endl;
            
            
            struct node *r = new struct node [num_sv];
            
            int svCount=0;
            for (int i=0; i<num_sv; i++) {
                double *ptr = m->getSV(i);
                
                Mat x = Mat(n,1, CV_64F, ptr);
                Mat tmp = wt.t() * x - b;
                
                r[i].val = tmp.at<double>(0,0);
                r[i].idx = i;
                
                if (r[i].val >= DETECT_KEEP_THRESHOLD) svCount++;
                
            }
            
            if (svCount == 0) {
                cout << "ERROR: number of negative support vectors is 0 ....\n";
                
                svmfree(svmModel);
                exit(0);
            }
            else {
                // update model parameters
                m->setWS((double*)wt.data);
                m->setBias(b);
            }
            
            // update support vectors
            int total_keep = min((int)(SVM_TRAIN_KEEP_NSV_MULTIPLER*svCount), num_sv);
            if (total_keep > 0 && total_keep != m->getNumSVs()) {
                
                ::qsort(&r[0], num_sv, sizeof(struct node), mysort_struct);
                
                vector<double*> SVs;
                for (int i=0; i<total_keep; i++) {
                    double *ptr = new double [n];
                    memcpy(ptr, m->getSV(r[i].idx), sizeof(double)*n);
                    
                    SVs.push_back(ptr);
                }
                m->updateSVs(SVs);
                cout << "Keeping " << total_keep << " negatives ...\n";
                
                // release memory
                for (int i=0; i<(int)SVs.size(); i++) {
                    double *ptr = SVs.at(i);
                    delete [] ptr; 
                }
                SVs.clear();
                
                cout << "Updated Num of SVs: " << m->getNumSVs() << endl;
            }
            
            delete [] r;
        }
    }
}


void CTrainModels::UpdateModel(CModel *m)
{
    struct svm_model *svmModel = TrainSVMs(m);
    
    UpdateSVMsParams(m, svmModel);
    
    svmfree(svmModel);
    
    cout << "SVMs Updated ...\n";
}

void CTrainModels::MineTrainingData(CModel *m, CImageSet *trainingset, vector<int> *miningQueue, int *total_mines)
{
    int old_total_mines = *total_mines;
    vector<RESSTRUCT> hn = MineNegative(m, trainingset, miningQueue, total_mines);
    
    if (old_total_mines != *total_mines && hn.size() > 0) {
  
        AddSupportVectors(m, hn);
      
        UpdateModel(m);
    }
    else {
        // no more violated images for negative mining
        miningQueue->clear();
    }
         
    ReleaseRes(hn);
}


void CTrainModels::Train(CImageSet *trainingset, int neg_idx1, int neg_idx2)
{
    gSVMsTask = SVM_TRAIN;
    
    if (neg_idx1 < 0 || neg_idx1 > trainingset->size() ||
        neg_idx2 < 0 || neg_idx2 > trainingset->size() ||
        neg_idx1 >= neg_idx2)
    {
        cout << "ERROR: Wrong training index ....\n";
        exit(0);
    }
    
    for (int k=0; k<(int)m_models.size(); k++) {
        CModel *m = m_models.at(k);
        
        int count = 0;
        
        int total_mines = 0;
        
        // initialize mining queue - use all images
        vector<int> miningQueue;
        for (int i=neg_idx1; i<neg_idx2; i++) miningQueue.push_back(i);
        
        
        // train one exemplar at a time
        while (count < TRAIN_MAX_MINE_ITERATIONS && miningQueue.size() > MIN_MINED_IMAGES_PER_ITERATION) {
            
            cout << "Training " << k << " exemplar ... (Iternation " << count << ")\n";
            
            MineTrainingData(m, trainingset, &miningQueue, &total_mines);
            
            count++;           
        }
        
        m->ClearSVs();
    }    
}

void CTrainModels::Validate(CImageSet *posSet, CImageSet *negSet, int neg_idx1, int neg_idx2)
{
    gSVMsTask = SVM_VALIDATE;
    
    if (neg_idx1 < 0 || neg_idx1 > negSet->size() ||
        neg_idx2 < 0 || neg_idx2 > negSet->size() ||
        neg_idx1 >= neg_idx2)
    {
        cout << "ERROR: Wrong validation index ....\n";
        exit(0);
    }
    
    ReleaseRes(m_grid);
    m_grid.clear();
    
    m_grid = RunValidation(posSet, negSet, neg_idx1, neg_idx2);
    
    m_posValSize = posSet->size();
    m_negValSize = neg_idx2 - neg_idx1;
}


void CTrainModels::Calibrate(const char *task)
{
    if (!strcmp(task, "Platt")) {
        PlattCalibration(m_grid);
    }
    else if (!strcmp(task, "Occurrence")) {
        EstimateOccurrenceMatrix(m_grid);
    }
    else if (!strcmp(task, "MR")) {
        MetaCalibration(m_grid);
    }
    else {
        cout << "ERROR: Undefined validation task ...\n";
    }
}


vector<RESSTRUCT> CTrainModels::RunValidation(CImageSet *posSet, CImageSet *negSet, int neg_idx1, int neg_idx2)
{
    vector<RESSTRUCT> grid;
    
    for (int i=0; i<posSet->size(); i++) {
        
        double ratio=1;
        Mat img = posSet->GetImage(i);
        img = ScaleImage(img, &ratio);
        
        RESSTRUCT gt;
        gt.x1 = 0; 
        gt.y1 = 0;
        gt.x2 = img.cols-1;
        gt.y2 = img.rows-1;
        
        gSVMsTask = SVM_TRAIN;
        vector<RESSTRUCT> res = m_detect.RunDetection(&img, &m_models);
        
        if ((int)res.size() > 0) {
            std::sort(res.begin(), res.end(), mysort);
            
            cout << i << " Pos exemplars, " << res.size() << " windows, " << "maxscore = " << res.at(0).score << endl;
            
            for (int j=0; j<min(MAX_VALIDATE_PER_IMAGE,(int)res.size()); j++) {
                RESSTRUCT hn = res.at(j);
                
                if (hn.exemplarID != i) { // assocated with the training image index
                
                    // groud truth is the entire image (that is how we train)
                    hn.os = m_detect.ComputeGroundTruthOverlap(hn, gt);
                    hn.label = 1;
                    hn.imgID = i;
                    
                    hn.x1 *= ratio;
                    hn.x2 *= ratio;
                    hn.y1 *= ratio;
                    hn.y2 *= ratio;

                    grid.push_back(hn);
                }
            }
        }
        
        ReleaseRes(res);
    }
    
    
    int count=0;
    for (int i=neg_idx1; i<neg_idx2; i++) {
        
        double ratio=1;
        Mat img = negSet->GetImage(i);
        //img = ScaleImage(img, &ratio);
        
        vector<RESSTRUCT> res = m_detect.RunDetection(&img, &m_models);
        
        if ((int)res.size() > 0) {
            std::sort(res.begin(), res.end(), mysort);
            
            cout << i << " Neg exemplars, " << res.size() << " windows, " << "maxscore = " << res.at(0).score << endl;
            
            for (int j=0; j<min(MAX_VALIDATE_PER_IMAGE,(int)res.size()); j++) {
                RESSTRUCT hn = res.at(j);
                
                // negative sample (no ground truth)
                hn.os = 0;
                hn.label = -1;
                hn.imgID = i - neg_idx1;
                
                hn.x1 *= ratio;
                hn.x2 *= ratio;
                hn.y1 *= ratio;
                hn.y2 *= ratio;
                
                grid.push_back(hn);
                
                count++;
            }
        }
        
        ReleaseRes(res);
        
        
        if (count > MAX_VALIDATE_NEGATIVE) break;
    }
    
    return grid;
}

void CTrainModels::PlattCalibration(vector<RESSTRUCT> grid)
{
    FitSigmoid(grid);
    m_bPlattCalibrated = true;
}

void CTrainModels::MetaCalibration(vector<RESSTRUCT> grid)
{
    FitWeibull(grid, m_posValSize, m_negValSize);
    m_bMRCalibrated = true;
}


void CTrainModels::EstimateOccurrenceMatrix(vector<RESSTRUCT> grid)
{
    cout << "Computing Occurrence Matrix ...\n";
    
    if (m_models.size() <= 1) {
        cout << "WARNING: Number of Exemplars must be more than one for Co-occurrence ...\n";
        return;
    }
    
    vector<int> exids;
    vector<double> os;
    
    
    vector<RESSTRUCT> boxes;
    for (int i=0; i<(int)grid.size(); i++) {
        RESSTRUCT hn = grid.at(i);
        
        if (hn.label == 1) {
            
            hn.score += 1.0;
            if (hn.score > CALIBRATION_THRESHOLD) {
                boxes.push_back(hn);
                exids.push_back(hn.exemplarID);  
                os.push_back(hn.os);
            }
        }
    }
    Mat xraw1 = m_detect.ComputeSelfOverlap(boxes, m_models.size());
    
    boxes.clear();
    
    
    for (int i=0; i<(int)grid.size(); i++) {
        RESSTRUCT hn = grid.at(i);
        
        if (hn.label == -1) {
            
            hn.score += 1.0;
            if (hn.score > CALIBRATION_THRESHOLD) {
                boxes.push_back(hn);
                exids.push_back(hn.exemplarID); 
                os.push_back(hn.os);
            }  
        }
    }
    Mat xraw2 = m_detect.ComputeSelfOverlap(boxes, m_models.size());
    
    boxes.clear();
    
    
    // concat the xraw matrix
    Mat xx;
    if (xraw2.rows > 0 && xraw2.cols > 0) {
        hconcat(xraw1, xraw2, xx);
    }
    else xx = xraw1;
    
    
    // learning by counting
    m_M = Mat::zeros(xx.rows, xx.rows, CV_64F);
    int n = xx.cols;
    
    for (int j=0; j<n; j++) {
        
        int count=0;
        for (int i=0; i<xx.rows; i++) 
            if (xx.at<double>(i,j) > 0) count++;
        
        for (int i=0; i<xx.rows; i++) {
            if (xx.at<double>(i,j) > 0) {
                m_M.at<double>(i, exids.at(j)) += os.at(j) * (os.at(j) > CALIBRATION_COUNT_THRESHOLD) / (double)count;  
            }
        }
    }
    
    m_bOccMatrixCalibrated = true;
}

void CTrainModels::FitSigmoid(vector<RESSTRUCT> grid)
{    
    cout << "Performing Platt Calibration ....\n";
    
    double thresh1 = 0.7;
    double thresh2 = 0.2;
    for (int k=0; k<(int)m_models.size(); k++) {
        CModel *m = m_models.at(k);
        
        int pos=0, neg=0;
        double maxScore = -99999;
        double minScore = 99999;
        vector<RESSTRUCT> data;
        for (int i=0; i<(int)grid.size(); i++) {
            RESSTRUCT hn = grid.at(i); 
            
            if (hn.exemplarID == k) {
                
                if (hn.label == 1 && hn.os >= thresh1) {
                    hn.label = 1;
                    data.push_back(hn);
                    pos++;
                    
                    minScore = min(minScore, hn.score);
                }
                else if (hn.label == -1 && hn.os <= thresh2) {
                    hn.label = 0;
                    data.push_back(hn);
                    neg++;
                    
                    maxScore = max(maxScore, hn.score);     
                }
            }
        }
        
        // just in case
        if (pos == 0) {
            RESSTRUCT hn;
            hn.label = 1;
            hn.score = maxScore + 0.1;
            data.push_back(hn);
        }
        if (neg == 0) {
            RESSTRUCT hn;
            hn.label = 0;
            hn.score = minScore - 0.1;
            data.push_back(hn);
        }    
        ////
        
        
        
        if (data.size() > 1) {
            
            double beta[2];
            double *vals = new double [(int)data.size()];
            double *labels = new double [(int)data.size()];
            
            for (int i=0; i<(int)data.size(); i++) {
                RESSTRUCT hn = data.at(i);
                vals[i] = hn.score;
                labels[i] = hn.label;
            }
            
            sigmoid_train((int)data.size(), vals, labels, beta[0], beta[1]);
            
            m->setBeta(beta);
            
            delete [] vals;
            delete [] labels;
        }
        data.clear();
    }
    
}

void CTrainModels::FitWeibull(vector<RESSTRUCT> grid, int posSize, int negSize)
{    
    cout << "Performing Meta Calibration ....\n";
    
    double thresh1 = 0.5; 
    double thresh2 = 0.2;
    
    for (int k=0; k<(int)m_models.size(); k++) {
        
        vector<double> good, bad;
        
        for (int idx=0; idx<posSize; idx++) {
            double minScore = 9999999;
            for (int i=0; i<(int)grid.size(); i++) {
                RESSTRUCT hn = grid.at(i);
                
                // finding the minimum match score in an image
                if (hn.label == 1 && hn.imgID == idx && hn.exemplarID != k && hn.os >= thresh1) {
                    if (hn.score < minScore)
                        minScore = hn.score;
                }
            }
            
            if (minScore < 9999999) good.push_back(minScore);
        }
        
        for (int idx=0; idx<negSize; idx++) {
            double maxScore = -9999999;
            for (int i=0; i<(int)grid.size(); i++) {
                RESSTRUCT hn = grid.at(i);
                
                // finding the maximum non-match score in an image
                //if (hn.label == -1 && hn.imgID == idx && hn.exemplarID == k && hn.os < thresh2) {
                //    if (hn.score > maxScore)
                //        maxScore = hn.score;
                //}
                
                if (hn.label == -1 && hn.imgID == idx && hn.exemplarID == k && hn.os < thresh2) {
                    bad.push_back(hn.score);
                }
            }
            
            if (maxScore > -9999999) bad.push_back(maxScore);
        }
        
        std::sort(good.begin(), good.end(), myfun2);
        std::sort(bad.begin(),  bad.end(), myfun1);
        
        
        int m = 1; 
        int tailsize = m + min(MAX_MR_TAILSIZE,(int)bad.size());
        double *tail = new double [tailsize]; 
      
        
        bool bSkipFirst=false;
        if (good.size() == 0) {bSkipFirst = true; good.push_back(bad.at(0)+0.1);}
        else {
            // select the smallest larger than negatives
            tail[0] = good.at(good.size()-1);
            for (int i=0; i<(int)good.size(); i++) {
                if (good.at(i) > bad.at(0)) {
                    tail[0] = good.at(i);
                    break;
                }
            }
        }      
        
        
        for (int j=0; j<tailsize-m; j++) {
            tail[j+m] = bad.at(j);
        }
        
        m_models.at(k)->MRFit(tail, tailsize, bSkipFirst);
                
        delete [] tail;         
    }
}


void CTrainModels::ReadModels(const char *prefix)
{
    for (int k=0; k<(int)m_models.size(); k++) {
        CModel *ptr = m_models.at(k);
        delete ptr;
    }
    m_models.clear();

    int size, w,h;
    char filename[80];
    sprintf(filename,"%s_info.txt",prefix);
    FILE *fp = fopen(filename,"rt");
    fscanf(fp,"%d",&size);
    for (int k=0; k<size; k++) {
        fscanf(fp,"%s %d %d",filename,&w,&h);
        
        CModel *m = new CModel;
        m->loadModel(filename);
        m->setImgWidth(w);
        m->setImgHeight(h);
        
        m_models.push_back(m);
    }
    fclose(fp);
    
    

    FileStorage fs;
    sprintf(filename,"%s_occurrenceModel.txt",prefix);
    fs.open(filename, FileStorage::READ);
    if (!fs.isOpened()) {
        cout << "ERROR: failed to open occurrenceMatrix.txt ...\n";
        exit(0);
    }
    else {
        fs["M"] >> m_M;
        fs.release();
    }
    
    
    
    for (int k=0; k<(int)m_models.size(); k++) {
        sprintf(filename,"%s_MR%d.txt",prefix,k+1);
        m_models.at(k)->loadMRParams(filename);
    }
  
}


void CTrainModels::SaveModels(const char *prefix)
{
    cout << "Saving models ...\n";
    
    char filename[80];
    
    // exemplarSVM models 
    sprintf(filename,"%s_info.txt",prefix);
    FILE *fp = fopen(filename,"wt");
    fprintf(fp,"%d\n",(int)m_models.size());
    for (int k=0; k<(int)m_models.size(); k++) {
        sprintf(filename,"%s_model%d.txt",prefix,k+1);
        fprintf(fp,"%s %d %d\n",filename,m_models.at(k)->getImgWidth(),m_models.at(k)->getImgHeight());
    }
    fclose(fp);
    
    for (int k=0; k<(int)m_models.size(); k++) {
        sprintf(filename,"%s_model%d.txt",prefix,k+1);
        m_models.at(k)->saveModel(filename);
    }
    
    
    
    if (m_bOccMatrixCalibrated) {
        
        FileStorage fs;
        sprintf(filename,"%s_occurrenceModel.txt",prefix);
        fs.open(filename, FileStorage::WRITE);
        fs << "M" << m_M;
        fs.release();
    }
    
    
    if (m_bPlattCalibrated) {
        // parameters are saved in *_model*.txt
    }
    
    
    if (m_bMRCalibrated) {
        
        for (int k=0; k<(int)m_models.size(); k++) {
            sprintf(filename,"%s_MR%d.txt",prefix,k+1);
            m_models.at(k)->saveMRParams(filename);
        }
    } 
}

