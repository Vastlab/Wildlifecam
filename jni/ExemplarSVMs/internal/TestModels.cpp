#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fstream>
#include "TestModels.h"
#include "Pyramid.h"


#define COLOR_COUNT_THRESHOLD 0.05


int DISPLAY_MODE = 0;


extern SVM_TASK gSVMsTask; 


static bool mysort(const RESSTRUCT &a, const RESSTRUCT &b)
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



CTestModels::CTestModels()
{
    m_bUseColor = false;
    m_bOccReady = false;
}

CTestModels::~CTestModels()
{
    for (int k=0; k<(int)m_models.size(); k++) {
        CModel *ptr = m_models.at(k);
        delete ptr;
    }
}


vector<RESSTRUCT> CTestModels::Test(Mat rgbImg, Mat colorSegment, float threshold, int bDisplay)
{
    gSVMsTask = SVM_TEST;
    
    m_bUseColor = true;
    m_colorSegment = colorSegment;
    DISPLAY_MODE = bDisplay;
    
    if (gCalibrationMethod == PLATT) {
        return RunPlatt(rgbImg, threshold);
    }
    else if (gCalibrationMethod == CO_OCCURRENCE) {
        return RunOccurrence(rgbImg, threshold);
    }
    else if (gCalibrationMethod == MR) {
        return RunMetaRecognition(rgbImg, threshold);
    }
    else if (gCalibrationMethod == BASELINE) {
        return RunBaseline(rgbImg, threshold);
    }
    else {
        cout << "ERROR: Undefined validation task ...\n";
        exit(0);
    }
}


vector<RESSTRUCT> CTestModels::Test(Mat rgbImg, float threshold, int bDisplay)
{
    gSVMsTask = SVM_TEST;
    
    m_bUseColor = false;
    DISPLAY_MODE = bDisplay;

    if (gCalibrationMethod == PLATT) {
        return RunPlatt(rgbImg, threshold);
    }
    else if (gCalibrationMethod == CO_OCCURRENCE) {
        return RunOccurrence(rgbImg, threshold);
    }
    else if (gCalibrationMethod == MR) {
        return RunMetaRecognition(rgbImg, threshold);
    }
    else if (gCalibrationMethod == BASELINE) {
        return RunBaseline(rgbImg, threshold);
    }
    else {
        cout << "ERROR: Undefined validation task ...\n";
        exit(0);
    }

}

void CTestModels::DetectionFilter(vector<RESSTRUCT> &res, float threshold)
{
    if (false == m_bUseColor) {
        for (int j=0; j<(int)res.size(); j++) {
            RESSTRUCT *hn = &res.at(j);
            if (hn->score > threshold)
                hn->label = 1;
        }
    }
    else {
        for (int j=0; j<(int)res.size(); j++) {
            RESSTRUCT *hn = &res.at(j);

            if (hn->score > threshold) {
                double sumv=0;
                for (int x=hn->x1; x<hn->x2; x++)
                    for (int y=hn->y1; y<hn->y2; y++) 
                        sumv += (int)m_colorSegment.at<float>(y,x);
                
                sumv /= (255*abs(hn->x2-hn->x1)*abs(hn->y2-hn->y1));
                hn->label = (sumv > COLOR_COUNT_THRESHOLD ? 1 : 0);
            }
            else 
                hn->label = 0;
        }
    }
}


vector<RESSTRUCT> CTestModels::RunOccurrence(Mat display, float threshold)
{
    if (false == m_bOccReady) {
        cout << "WARNING: No Co-Occurrence Matrix ....\n";
        vector<RESSTRUCT> ret;
        return ret;
    }
    
    double ratio=1.0;
    //Mat img = ScaleImage(display, &ratio);
    Mat img = display;
    
    vector<RESSTRUCT> res = m_detect.RunDetection(&img, &m_models);
    
    vector<int> exids;
    vector<RESSTRUCT> detection;
    
    if ((int)res.size() > 0) {
        std::sort(res.begin(), res.end(), mysort);
        
        vector<RESSTRUCT> boxes;
        for (int j=0; j<(int)res.size(); j++) {
            RESSTRUCT hn = res.at(j);
            
            hn.x1 *= ratio;
            hn.x2 *= ratio;
            hn.y1 *= ratio;
            hn.y2 *= ratio;
            
            
            hn.score += 1.0;
            if (hn.score > CALIBRATION_THRESHOLD+1) {
                boxes.push_back(hn);
                exids.push_back(hn.exemplarID); 
            }
        }
        Mat xraw = m_detect.ComputeSelfOverlap(boxes, m_models.size());

    
        // apply M
        Mat colsum = Mat::zeros(1, xraw.cols, CV_64FC1);
        cv::reduce(xraw, colsum, 0, CV_REDUCE_SUM);
        
        for (int j=0; j<(int)boxes.size(); j++) {
            
            int id = exids.at(j); 
            Mat r = m_M.col(id).t() * xraw.col(j) + colsum.col(j); 
            
            RESSTRUCT *hn = &boxes.at(j);
            hn->score = r.at<double>(0,0);
            
            hn->x1 = max(0.0, hn->x1);
            hn->y1 = max(0.0, hn->y1);
            hn->x2 = min(display.cols-1.0, hn->x2);
            hn->y2 = min(display.rows-1.0, hn->y2);
        }
        
        detection = m_detect.NonMaximumSuppression(boxes, 0.3);

        DetectionFilter(detection, threshold);

        if (DISPLAY_MODE) {
            // show results
            std::sort(detection.begin(), detection.end(), mysort);
            
            for (int j=0; j<min(MAX_DETECTED_RETURNS,(int)detection.size()); j++) {
                RESSTRUCT hn = detection.at(j);
                
                if (1 == hn.label) {
                    cout << "Detected M score = " << hn.score <<  " " << hn.level <<endl;
                    
                    Mat display2 = display.clone();
                    CvPoint pt1 = cvPoint(hn.x1, hn.y1);
                    CvPoint pt2 = cvPoint(hn.x2, hn.y2);
                    cv::rectangle(display2, pt1, pt2, cvScalar(0, 255, 0), 2, 4, 0);
                    imshow("Result",display2);
                    cvWaitKey(0);
                }
            }
        }
        ReleaseRes(boxes);
    }

    
    ReleaseRes(res);
    
    return detection;
}

vector<RESSTRUCT> CTestModels::RunPlatt(Mat display, float threshold)
{
    double ratio=1.0;
    //Mat img = ScaleImage(display, &ratio);
    Mat img = display;
    
    vector<RESSTRUCT> res = m_detect.RunDetection(&img, &m_models);
    
    vector<int> exids;
    vector<RESSTRUCT> detection;
    
    if ((int)res.size() > 0) {
        std::sort(res.begin(), res.end(), mysort);
        
        for (int j=0; j<(int)res.size(); j++) {
            RESSTRUCT *hn = &res.at(j);
          
            int exid = hn->exemplarID;
            CModel *m = m_models.at(exid);
            
            double a,b;
            m->getBeta(&a, &b);
            
            // sigmoid predict
            double score = hn->score*a+b;
            if (score >= 0)
                hn->score = exp(-score)/(1.0+exp(-score));
            else
                hn->score = 1.0/(1+exp(score));
          
            
            hn->x1 *= ratio; hn->y1 *= ratio;
            hn->x2 *= ratio; hn->y2 *= ratio;
            
            hn->x1 = max(0.0, hn->x1);
            hn->y1 = max(0.0, hn->y1);
            hn->x2 = min(display.cols-1.0, hn->x2);
            hn->y2 = min(display.rows-1.0, hn->y2);
        }
        
        detection = m_detect.NonMaximumSuppression(res, 0.3);
        std::sort(detection.begin(), detection.end(), mysort);
        
        DetectionFilter(detection, threshold);
        
        if (DISPLAY_MODE) {
            // show results
            for (int j=0; j<min(MAX_DETECTED_RETURNS,(int)detection.size()); j++) {
                RESSTRUCT hn = detection.at(j);
                
                if (1 == hn.label) {
                    cout << "Detected Platt score = " << hn.score << endl;
                    
                    Mat display2 = display.clone();
                    CvPoint pt1 = cvPoint(hn.x1, hn.y1);
                    CvPoint pt2 = cvPoint(hn.x2, hn.y2);
                    cv::rectangle(display2, pt1, pt2, cvScalar(0, 255, 0), 2, 4, 0);
                    imshow("Result",display2);
                    cvWaitKey(0);
                }
            }
        }
    }
    
    ReleaseRes(res);

    return detection;
}


vector<RESSTRUCT> CTestModels::RunMetaRecognition(Mat display, float threshold)
{
    double ratio=1.0;
    //Mat img = ScaleImage(display, &ratio);
    Mat img = display;
    
    vector<RESSTRUCT> res = m_detect.RunDetection(&img, &m_models);
    
    vector<int> exids;
    vector<RESSTRUCT> detection;
    
    if ((int)res.size() > 0) {
        std::sort(res.begin(), res.end(), mysort);
        
        for (int k=0; k<(int)m_models.size(); k++) {
            
            vector<double> data;
            for (int j=0; j<(int)res.size(); j++) {
                RESSTRUCT *hn = &res.at(j);
             
                if (hn->exemplarID == k) {
                    data.push_back(hn->score);
                }
            }
            double *fdata = new double [(int)data.size()];
            for (int j=0; j<(int)data.size(); j++) fdata[j] = data.at(j); 
              
            double *out = m_models.at(k)->getMRScores(fdata, (int)data.size());
            
            for (int j=0, count=0; j<(int)res.size(); j++) {
                RESSTRUCT *hn = &res.at(j);

                if (hn->exemplarID == k) hn->score = out[count++];
            }
        
            
            delete [] fdata;
            delete [] out;
            
            data.clear();
        }
        
        for (int j=0; j<(int)res.size(); j++) {
            RESSTRUCT *hn = &res.at(j);
         
            hn->x1 *= ratio; hn->y1 *= ratio;
            hn->x2 *= ratio; hn->y2 *= ratio;
            
            hn->x1 = max(0.0, hn->x1);
            hn->y1 = max(0.0, hn->y1);
            hn->x2 = min(display.cols-1.0, hn->x2);
            hn->y2 = min(display.rows-1.0, hn->y2);
        }
        
        
        detection = m_detect.NonMaximumSuppression(res, 0.3);
        std::sort(detection.begin(), detection.end(), mysort);
        
        DetectionFilter(detection, threshold);
        
        if (DISPLAY_MODE) {
            // show results
            for (int j=0; j<min(MAX_DETECTED_RETURNS,(int)detection.size()); j++) {
                RESSTRUCT hn = detection.at(j);
                
                if (1 == hn.label) {
                    cout << "Detected MR score = " << hn.score << endl;
                    
                    Mat display2 = display.clone();
                    CvPoint pt1 = cvPoint(hn.x1, hn.y1);
                    CvPoint pt2 = cvPoint(hn.x2, hn.y2);
                    cv::rectangle(display2, pt1, pt2, cvScalar(0, 255, 0), 2, 4, 0);
                    imshow("Result",display2);
                    cvWaitKey(0);
                }
            }         
        }
    }
    
    ReleaseRes(res);
    
    return detection;
}


vector<RESSTRUCT> CTestModels::RunBaseline(Mat display, float threshold)
{
    double ratio;
    Mat img = ScaleImage(display, &ratio);

    vector<RESSTRUCT> detection;
    vector<RESSTRUCT> res = m_detect.RunDetection(&img, &m_models);
    
    if ((int)res.size() > 0) {
        std::sort(res.begin(), res.end(), mysort);
        
        detection = m_detect.NonMaximumSuppression(res, 0.3);
        
        std::sort(detection.begin(), detection.end(), mysort);
    }
    
    DetectionFilter(detection, threshold);
    
    if (DISPLAY_MODE) {
        // show results
        for (int j=0; j<min(MAX_DETECTED_RETURNS,(int)detection.size()); j++) {
            RESSTRUCT hn = detection.at(j);
            
            if (1 == hn.label) {
                cout << "Detected score = " << hn.score << endl;
                
                Mat display2 = display.clone();
                CvPoint pt1 = cvPoint(hn.x1, hn.y1);
                CvPoint pt2 = cvPoint(hn.x2, hn.y2);
                cv::rectangle(display2, pt1, pt2, cvScalar(0, 255, 0), 2, 4, 0);
                imshow("Result",display2);
                cvWaitKey(0);

            }
        }         
    }
    
    ReleaseRes(res);
    
    return detection;
}

void CTestModels::ReadModels(const char *prefix)
{
	//LOGI("Reading models from %s",prefix);

    for (int k=0; k<(int)m_models.size(); k++) {
        CModel *ptr = m_models.at(k);
        delete ptr;
    }
    m_models.clear();

    int size, w,h;
    char filename[80];
    sprintf(filename,"%s_info.txt",prefix);
//LOGI("Filename: %s",filename);
    FILE *fp = fopen(filename,"rt");
    fscanf(fp,"%d",&size);
    for (int k=0; k<size; k++) {
        fscanf(fp,"%s %d %d",filename,&w,&h);
        char modelFile[128];
        sprintf(modelFile,"%s_%s",prefix,filename);
//LOGI("Loading: %s",modelFile);
        CModel *m = new CModel;
        m->loadModel(modelFile);
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
        m_bOccReady = false;
    }
    else {
        fs["M"] >> m_M;
        fs.release();
        
        m_bOccReady = true;
    }
    
    
    
    for (int k=0; k<(int)m_models.size(); k++) {
        sprintf(filename,"%s_MR%d.txt",prefix,k+1);
        m_models.at(k)->loadMRParams(filename);
    }
  
}

