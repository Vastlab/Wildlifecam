#include <stdio.h>
#include <stdlib.h>
#include <ml.h>
#include "Pyramid.h"
#include "PiCoDes.h"

extern SVM_TASK gSVMsTask;

//#define USE_SVM 1


static bool mysort1(const RESSTRUCT &a, const RESSTRUCT &b)
{
    // decending order
    return (a.score > b.score);
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



CPicodes::CPicodes()
{
    
}

CPicodes::CPicodes(const char *filename)
{
    strcpy(m_modelFile, filename);
}

CPicodes::~CPicodes()
{
    for (int k=0; k<(int)m_models.size(); k++) {
        CModel *ptr = m_models.at(k);
        delete ptr;
    }

    ReleaseRes(m_grid);
}

void CPicodes::Train(vector<CImageSet*> dataset, int nExemplarCls)
{
    ReleaseRes(m_grid);
    m_grid.clear();
    
    int idx1=0, idx2=0;
    int numClasses = (int)dataset.size(); 
    
    // concat all models together
    ReadModels(dataset, nExemplarCls);
    
    for (int k=0; k<numClasses; k++) {
        CImageSet *data = dataset.at(k);
 
        int idx1 = m_grid.size();
        RunDetection(data, k, 0, data->size());
        
        cout << data->GetClsName() << " ... " << data->size() << endl;
    }
    
    cout << "Training a classifier ....\n";
    
#ifdef USE_SVM    
    TrainSVM(numClasses);
#else
    TrainRandomForest(numClasses);
#endif
}


void CPicodes::Recognition(const char *filename, vector<RESSTRUCT> res, const char *clsLabels[])
{
#ifdef USE_SVM  
    CvSVM *classifier = new CvSVM;
    CvFileStorage* fs = cvOpenFileStorage(m_modelFile, 0, CV_STORAGE_READ);
    classifier->read(fs, cvGetFileNodeByName(fs, 0, "SVM" )); 
    cvReleaseFileStorage(&fs);
#else
    CvRTrees *classifier = new CvRTrees;
    CvFileStorage* fs = cvOpenFileStorage(m_modelFile, 0, CV_STORAGE_READ);
    classifier->read(fs, cvGetFileNodeByName(fs, 0, "rTree" )); 
    cvReleaseFileStorage(&fs);
#endif    
    
    Mat img = imread(filename);
    if (!img.data ) {
        cout <<  "Could not open " << filename << std::endl ;
        return;
    }
    
    Mat display1 = img.clone();
    Mat display2 = img.clone();
    
    double in[1];
    for (int i=0; i<(int)res.size(); i++) {
        RESSTRUCT hn = res.at(i);
        
        cv::Rect myROI(hn.x1, hn.y1, hn.x2-hn.x1+1, hn.y2-hn.y1+1);
        Mat roi(img, myROI);
        
        cout << i << " " << hn.score << endl;
        
        Mat feature = m_detect.RunROIDetection(&roi, &m_models);
        Calibration(feature);
        
        double result = classifier->predict(feature);
        
        cout << "Class result = " << clsLabels[(int)result] << " ( " <<  clsLabels[hn.label] << " ) " << endl; 
        
        display1 = DrawResults(display1, hn, (int)result);
        display2 = DrawResults(display2, hn, hn.label);
    }
    
    delete classifier;
    
    imshow("Result",display1);
    cvWaitKey(0);
    
    imshow("Result",display2);
    cvWaitKey(0);
}


int CPicodes::SingleTest(char *filename, float *scores)
{
    Mat img = imread(filename);
    if (!img.data ) {
        cout <<  "Could not open " << filename << std::endl ;
        return 0;
    }
    
    Mat feat = m_detect.RunROIDetection(&img, &m_models);
    Calibration(feat);
    

#ifdef USE_SVM     
    CvSVM *classifier = new CvSVM;
    CvFileStorage* fs = cvOpenFileStorage(m_modelFile, 0, CV_STORAGE_READ);
    classifier->read(fs, cvGetFileNodeByName(fs, 0, "SVM" )); 
    cvReleaseFileStorage(&fs);
    
    double result = classifier->predict(feat);
#else    
    //CvRTrees *classifier = new CvRTrees;
    CvRTreesMultiClass *classifier = new CvRTreesMultiClass;
    CvFileStorage* fs = cvOpenFileStorage(m_modelFile, 0, CV_STORAGE_READ);
    classifier->read(fs, cvGetFileNodeByName(fs, 0, "rTree" )); 
    cvReleaseFileStorage(&fs);
    
    int total_votes;
    int *votes = new int [classifier->getNclass()];
    double result = classifier->predict_multi_class(feat, votes, &total_votes);
    
    memset(scores, 0, sizeof(*scores)*classifier->getNclass());
    for (int i=0; i<classifier->getNclass(); i++) {
        scores[i] = (float)votes[i] / (float)total_votes;
    }
    
    delete [] votes;
#endif
    
    delete classifier;
    
    return (int)result;
}

void CPicodes::Test(vector<CImageSet*> dataset, int nExemplarCls)
{
    ReleaseRes(m_grid);
    m_grid.clear();
    
    // concat all models together
    ReadModels(dataset, nExemplarCls);
    
    int numClasses = (int)dataset.size(); 
    
    for (int k=0; k<numClasses; k++) {
        CImageSet *data = dataset.at(k);
        
        RunDetection(data, k, 0, data->size());
    }
    
    
#ifdef USE_SVM  
    CvSVM *classifier = new CvSVM;
    CvFileStorage* fs = cvOpenFileStorage(m_modelFile, 0, CV_STORAGE_READ);
    classifier->read(fs, cvGetFileNodeByName(fs, 0, "SVM" )); 
    cvReleaseFileStorage(&fs);
#else  
    CvRTrees *classifier = new CvRTrees;
    CvFileStorage* fs = cvOpenFileStorage(m_modelFile, 0, CV_STORAGE_READ);
    classifier->read(fs, cvGetFileNodeByName(fs, 0, "rTree" )); 
    cvReleaseFileStorage(&fs);
#endif
    

    int TP=0, FP=0;
    int featureSize = m_models.size();
    Mat test_sample = Mat(1, featureSize, CV_32FC1);
    
    for (int i=0; i<(int)m_grid.size(); i++) {
        RESSTRUCT hn = m_grid.at(i);
        
        for (int j=0; j<featureSize; j++)
            test_sample.at<float>(0,j) = hn.xs[j];
        
        double result = classifier->predict(test_sample);
        
        printf("Testing Sample %i -> class result (%f) %d %f\n", i, result, hn.label, hn.os);
        
        if (result == hn.label) TP++;
        else FP++;
    }
    cout << TP << " " << FP << " " << TP/(double)(TP+FP) << endl;

    delete classifier;
}



void CPicodes::TrainRandomForest(int numClasses)
{
    int featureSize = m_models.size();
    int sampleSize = (int)m_grid.size();
    
    
    Mat data = Mat(sampleSize, featureSize, CV_32FC1);
    Mat labels = Mat(sampleSize, 1, CV_32FC1);
    
    for (int i=0; i<(int)m_grid.size(); i++) {
        RESSTRUCT hn = m_grid.at(i);
        
        labels.at<float>(i) = hn.label;
        for (int j=0; j<featureSize; j++)
            data.at<float>(i,j) = hn.xs[j];
    }
    
    float *priors = new float [numClasses];
    for (int i=0; i<numClasses; i++) priors[i] = 1;
    
    CvRTParams params = CvRTParams(15, // max depth
                                   10, // min sample count
                                   0, // regression accuracy: N/A here
                                   false, // compute surrogate split, no missing data
                                   15, // max number of categories (use sub-optimal algorithm for larger numbers)
                                   priors, // the array of priors
                                   false,  // calculate variable importance
                                   16,    // number of variables randomly selected at node and used to find the best split(s).
                                   100,	 // max number of trees in the forest
                                   0.01f, // forrest accuracy
                                   CV_TERMCRIT_ITER |	CV_TERMCRIT_EPS // termination cirteria
                                   );

    
    
    Mat var_type = Mat(featureSize+1, 1, CV_8U);
    var_type.setTo(Scalar(CV_VAR_NUMERICAL) ); // all inputs are numerical
    
    // this is a classification problem (i.e. predict a discrete number of class
    // outputs) so reset the last (+1) output var_type element to CV_VAR_CATEGORICAL
    var_type.at<uchar>(featureSize, 0) = CV_VAR_CATEGORICAL;
    
    CvRTrees* rtree = new CvRTrees;
    rtree->train(data, CV_ROW_SAMPLE, labels, Mat(), Mat(), var_type, Mat(), params);
    
    
    int TP=0, FP=0;
    Mat test_sample = Mat(1, featureSize, CV_32FC1);
    for (int i=0; i<(int)m_grid.size(); i++) {
        RESSTRUCT hn = m_grid.at(i);
        
        for (int j=0; j<featureSize; j++)
            test_sample.at<float>(0,j) = hn.xs[j];
        
        double result = rtree->predict(test_sample, Mat());
        
        printf("Testing Sample %i -> class result (%f) %d\n", i, result, hn.label);
        
        if (result == hn.label) TP++;
        else FP++;
    }
    cout << TP << " " << FP << " " << TP/(double)(TP+FP) << endl;


    CvFileStorage* fs = cvOpenFileStorage(m_modelFile, 0, CV_STORAGE_WRITE_TEXT);
    rtree->write(fs, "rTree");
    cvReleaseFileStorage(&fs);

    delete [] priors;
    delete rtree;
}

void CPicodes::TrainSVM(int numClasses)
{
    int featureSize = m_models.size();
    int sampleSize = (int)m_grid.size();
    
    
    Mat data = Mat(sampleSize, featureSize, CV_32FC1);
    Mat labels = Mat(sampleSize, 1, CV_32FC1);
    
    for (int i=0; i<(int)m_grid.size(); i++) {
        RESSTRUCT hn = m_grid.at(i);
        
        labels.at<float>(i) = hn.label;
        for (int j=0; j<featureSize; j++)
            data.at<float>(i,j) = hn.xs[j];
    }
    
    CvSVMParams params = CvSVMParams(
                                     CvSVM::C_SVC,   // Type of SVM; using N classes here
                                     CvSVM::LINEAR,     // Kernel type
                                     2.0,            // Param (degree) for poly kernel only
                                     1.0,            // Param (gamma) for poly/rbf kernel only
                                     1.0,            // Param (coef0) for poly/sigmoid kernel only
                                     100,            // SVM optimization param C
                                     0,              // SVM optimization param nu (not used for N class SVM)
                                     0,              // SVM optimization param p (not used for N class SVM)
                                     NULL,           // class weights (or priors)
                                     cvTermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 1000, 0.000001));
    
    CvSVM* svm = new CvSVM;
    
    svm->train(data, labels, Mat(), Mat(), params);
    

    int TP=0, FP=0;
    Mat test_sample = Mat(1, featureSize, CV_32FC1);
    for (int i=0; i<(int)m_grid.size(); i++) {
        RESSTRUCT hn = m_grid.at(i);
        
        for (int j=0; j<featureSize; j++)
            test_sample.at<float>(0,j) = hn.xs[j];
        
        double result = svm->predict(test_sample);
        
        printf("Testing Sample %i -> class result (%f) %d\n", i, result, hn.label);
        
        if (result == hn.label) TP++;
        else FP++;
    }
    cout << TP << " " << FP << " " << TP/(double)(TP+FP) << endl;
        
    
    
    CvFileStorage* fs = cvOpenFileStorage(m_modelFile, 0, CV_STORAGE_WRITE_TEXT);
    svm->write(fs, "SVM");
    cvReleaseFileStorage(&fs);
    
    delete svm;
}



void CPicodes::RunDetection(CImageSet *data, int clsIdx, int idx1, int idx2)
{
    string str = data->GetClsName();
    char *clsName = (char*) str.c_str();
    
    for (int i=idx1; i<idx2; i++) {
        
        cout << "Detecting " << i << " out of " << idx2 << endl;
        
        Mat img = data->GetImage(i);
        
        Mat f = m_detect.RunROIDetection(&img, &m_models);
        
        Calibration(f);

        RESSTRUCT hn;
        strcpy(hn.strLabel, clsName);
        hn.label = clsIdx;
        
        hn.xs = new double [(int)m_models.size()];
        for (int j=0; j<(int)m_models.size(); j++) {
            hn.xs[j] = f.at<float>(0,j);
        }
    
        m_grid.push_back(hn);
    }
}



void CPicodes::Calibration(Mat &feat)
{    
    double in[1];
    for (int j=0; j<(int)m_models.size(); j++) {
        in[0] = feat.at<float>(0,j);
        CModel *m = m_models.at(j);
            
        if (MR == gCalibrationMethod) {
            double *out = m->getMRScores(in, 1);
            feat.at<float>(0,j) = out[0];
        }
        else if (PLATT == gCalibrationMethod) {
            double a,b;
            m->getBeta(&a, &b);
            
            // sigmoid predict
            double score = in[0]*a+b;
            if (score >= 0)
                feat.at<float>(0,j) = exp(-score)/(1.0+exp(-score));
            else
                feat.at<float>(0,j) = 1.0/(1+exp(score));
        }
        else {
            cout << "ERROR: Invalid Calibration Method ...\n";
            exit(0);
        }
    }
}


void CPicodes::ReadModels(vector<CImageSet*> dataset, int n) 
{
    for (int k=0; k<(int)m_models.size(); k++) {
        CModel *ptr = m_models.at(k);
        delete ptr;
    }
    m_models.clear();
    
    int size,w,h;
    char filename[80];
    for (int i=0; i<n; i++) {

        CImageSet *data = dataset.at(i);
        string prefix = data->GetModelName();
        
        sprintf(filename,"%s_info.txt",prefix.c_str());
        FILE *fp = fopen(filename,"rt");
        fscanf(fp,"%d",&size);

        for (int k=0; k<size; k++) {
            fscanf(fp,"%s %d %d",filename,&w,&h);
            
            CModel *m = new CModel;
            m->loadModel(filename);
            m->setImgWidth(w);
            m->setImgHeight(h);
            
            sprintf(filename,"%s_MR%d.txt",prefix.c_str(),k+1);
            m->loadMRParams(filename);
            
            m_models.push_back(m);
        }
        fclose(fp);
        
    }
}

void CPicodes::ReadModels(const char *modelNames[], int n) 
{
    for (int k=0; k<(int)m_models.size(); k++) {
        CModel *ptr = m_models.at(k);
        delete ptr;
    }
    m_models.clear();
    
    int size,w,h;
    char filename[80];
    for (int i=0; i<n; i++) {
        
        string prefix = modelNames[i];
        
        sprintf(filename,"%s_info.txt",prefix.c_str());
        FILE *fp = fopen(filename,"rt");
        if (NULL == fp) {
            cout << "Could not open " << filename << endl;
            exit(0);
        }
        fscanf(fp,"%d",&size);

        for (int k=0; k<size; k++) {
            fscanf(fp,"%s %d %d",filename,&w,&h);
            
            CModel *m = new CModel;
            m->loadModel(filename);
            m->setImgWidth(w);
            m->setImgHeight(h);
            
            sprintf(filename,"%s_MR%d.txt",prefix.c_str(),k+1);
            m->loadMRParams(filename);
            
            m_models.push_back(m);
        }
        fclose(fp);        
    }
}


Mat CPicodes::DrawResults(Mat display, RESSTRUCT hn, int classCode)
{
    CvScalar colors[5];
    colors[0] = cvScalar(255, 0, 0);
    colors[1] = cvScalar(0, 255, 0);
    colors[2] = cvScalar(0, 0, 255);
    colors[3] = cvScalar(0, 0, 0);
    colors[4] = cvScalar(0, 255, 255);
    
    
    CvPoint pt1 = cvPoint(hn.x1, hn.y1);
    CvPoint pt2 = cvPoint(hn.x2, hn.y2);
    
    cv::rectangle(display, pt1, pt2, colors[classCode], 2, 4, 0);

    return display;
}

