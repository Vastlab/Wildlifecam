#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include "ColorDetection.h"

#define H_RATIO 176.0
#define S_RATIO 255.0
#define V_RATIO 255.0
#define DETECTED 255
#define SKIN_THRESHOLD 0.1
#define BUFFER_LENGTH 1024

#define DISPLAY_MODE 0

using namespace cv;
using namespace std;

const char *COLORS[] = {"Red", "Green", "Blue", "Yellow", "Purple", "Orange", "Brown", "Gray", "Black", "White", "Multicolor"};


const char *colorModelFiles[] = {
    "Models/Colors/red_model.txt",
    "Models/Colors/green_model.txt",
    "Models/Colors/blue_model.txt",
    "Models/Colors/yellow_model.txt",
    "Models/Colors/purple_model.txt",
    "Models/Colors/orange_model.txt",
    "Models/Colors/brown_model.txt",
    "Models/Colors/gray_model.txt",
    "Models/Colors/black_model.txt",
    "Models/Colors/white_model.txt",
    "Models/Colors/skin_model.txt"
};

const int colorThresholds[] = {50, 10, 10, 50, 20, 50, 50, 50, 30, 50, 500};



CColorDetection::CColorDetection()
{
    m_numColors = sizeof(colorThresholds)/sizeof(colorThresholds[0]);
    m_colorModels = new COLORMODEL [m_numColors];
    
    for (int i=0; i<m_numColors; i++) {
        ReadModel(colorModelFiles[i], &m_colorModels[i]);
        m_colorModels[i].threshold = colorThresholds[i];
    }
}

CColorDetection::~CColorDetection()
{    
    delete [] m_colorModels;
}

const char** CColorDetection::GetColorNames()
{
    return COLORS;
}

void CColorDetection::ReadModel(const char *fname, COLORMODEL *model)
{
    ifstream reader;
    char buf[BUFFER_LENGTH], *p;
    float val1, val2, val3, val4, val5, val6, val7, val8, val9;
    int k;
    
    reader.open(fname);
    if (reader.is_open()) {
        reader.getline(buf, BUFFER_LENGTH);
        sscanf(buf, "%d",&k);
        
        model->k = k;
        
        // get weights
        reader.getline(buf, BUFFER_LENGTH);
        p = strtok(buf, " ");
        for (int i=0; i<k; i++) {
            model->wt[i] = atof(p);
            p = strtok(NULL, " ");
        }
        
        // get det
        reader.getline(buf, BUFFER_LENGTH);
        p = strtok(buf, " ");
        for (int i=0; i<k; i++) {
            model->det[i] = atof(p);
            p = strtok(NULL, " ");
        }
        
        // get mu
        for (int i=0; i<k; i++) {
            reader.getline(buf, BUFFER_LENGTH);
            sscanf(buf, "%f %f %f",&val1, &val2, &val3);
            model->mu[i][0] = val1;
            model->mu[i][1] = val2;
            model->mu[i][2] = val3;
        }
        
        // get inverse sigma
        for (int i=0; i<k; i++) {
            reader.getline(buf, BUFFER_LENGTH);
            sscanf(buf, "%f %f %f %f %f %f %f %f %f",&val1, &val2, &val3, &val4, &val5, &val6, &val7, &val8, &val9);
                        
            model->sigmaInv[i][0][0] = val1;
            model->sigmaInv[i][1][0] = val2;
            model->sigmaInv[i][2][0] = val3;
            model->sigmaInv[i][0][1] = val4;
            model->sigmaInv[i][1][1] = val5;
            model->sigmaInv[i][2][1] = val6;
            model->sigmaInv[i][0][2] = val7;
            model->sigmaInv[i][1][2] = val8;
            model->sigmaInv[i][2][2] = val9;
        }
    }
    else {
        cout << "ERROR: " << fname << " cannot be opened ...\n";
        exit(1);
    }
    reader.close();
    
}

Mat CColorDetection::Convert2HSV(Mat rgbImg)
{  
    
    Mat hsvImg;
    Mat hsvData(rgbImg.rows, rgbImg.cols, CV_32FC3);
    
    cvtColor(rgbImg, hsvImg, CV_BGR2HSV);
    
    for (int i=0; i<hsvImg.rows; i++) {
        for (int j=0; j<hsvImg.cols; j++) {
            hsvData.at<Vec3f>(i,j)[0] = (float) hsvImg.at<Vec3b>(i,j)[0] / H_RATIO;
            hsvData.at<Vec3f>(i,j)[1] = (float) hsvImg.at<Vec3b>(i,j)[1] / S_RATIO;
            hsvData.at<Vec3f>(i,j)[2] = (float) hsvImg.at<Vec3b>(i,j)[2] / V_RATIO;
        }
    }
    
    
    //Mat hsvData = RGB2HSV(rgbImg);
    
    return hsvData;
}

Mat CColorDetection::ComputeGMMPDF(Mat data, COLORMODEL *model)
{
    Mat diffval(3,1,CV_32F), temp(3,1,CV_32F);
    Mat score = Mat::zeros(data.rows, data.cols, CV_32F);

    for (int k=0; k<model->k; k++) {
        
        for (int i=0; i<data.rows; i++) {
            for (int j=0; j<data.cols; j++) {
                diffval.at<float>(0,0) = data.at<Vec3f>(i,j)[0] - model->mu[k][0];
                diffval.at<float>(0,1) = data.at<Vec3f>(i,j)[1] - model->mu[k][1];
                diffval.at<float>(0,2) = data.at<Vec3f>(i,j)[2] - model->mu[k][2];
                
                for (int c=0; c<3; c++)
                    temp.at<float>(0,c) = diffval.at<float>(0,0) * model->sigmaInv[k][0][c] +
                                          diffval.at<float>(0,1) * model->sigmaInv[k][1][c] +
                                          diffval.at<float>(0,2) * model->sigmaInv[k][2][c];
                
                
                float val = temp.at<float>(0,0) * diffval.at<float>(0,0) +
                            temp.at<float>(0,1) * diffval.at<float>(0,1) +
                            temp.at<float>(0,2) * diffval.at<float>(0,2);
                
                
                score.at<float>(i,j) += model->wt[k] * exp(-0.5*val) / (model->det[k]+0.0000001);
            }
        }
    }
    
    
    return score;
}

Mat CColorDetection::Thresholding(Mat score, Mat skin, Mat mask, float threshold, long *count)
{
    Mat res = Mat::zeros(score.rows, score.cols, CV_32F);
    
    *count=0;
    for (int i=0; i<score.rows; i++) {
        for (int j=0; j<score.cols; j++) {
            
            if (mask.at<uchar>(i,j) > 0) {
                res.at<float>(i,j) = (score.at<float>(i,j) > threshold &&
                                      score.at<float>(i,j) > skin.at<float>(i,j) ? DETECTED : 0);
            
                if (res.at<float>(i,j) > 0) (*count)++;
            }
        }
    }
    
    return res;
}

Mat CColorDetection::Thresholding(Mat score, Mat skin, float threshold)
{
    Mat res = Mat::zeros(score.rows, score.cols, CV_32F);
    
    for (int i=0; i<score.rows; i++) {
        for (int j=0; j<score.cols; j++) {
            
            res.at<float>(i,j) = (score.at<float>(i,j) > threshold &&
                                  score.at<float>(i,j) > skin.at<float>(i,j) ? DETECTED : 0);
            /*
            if (skin.at<float>(i,j) < SKIN_THRESHOLD && score.at<float>(i,j) > threshold)
                res.at<float>(i,j) = DETECTED;
            else
                res.at<float>(i,j) = 0;
            */ 
        }
    }
    
    return res;
}

void CColorDetection::GetMultiColorCount(Mat *matArray, long *count, long *total)
{
    Mat res = Mat::zeros(matArray[0].rows, matArray[0].cols, CV_32F);
    for (int i=0; i<NUM_COLORS; i++) { 
        res += matArray[i];
    }
    
    *total = 0;
    *count = 0;
    int threshold = 2 * DETECTED;
    
    for (int i=0; i<matArray[0].rows; i++) {
        for (int j=0; j<matArray[0].cols; j++) {
            if (res.at<float>(i,j) >= threshold) (*count)++;
            if (res.at<float>(i,j) > 0) (*total)++;
        }
    }
}

Mat CColorDetection::DetectColor(Mat hsvImg, Mat mask, int k, long *count)
{
    // the last color model is for skin 
    Mat score_skin = ComputeGMMPDF(hsvImg, &m_colorModels[NUM_COLORS]);
    Mat score = ComputeGMMPDF(hsvImg, &m_colorModels[k]);
    Mat res = Thresholding(score, score_skin, mask, m_colorModels[k].threshold, count);

    return res;
}

Mat CColorDetection::DetectColor(Mat hsvImg, int k)
{
    // the last color model is for skin 
    Mat score_skin = ComputeGMMPDF(hsvImg, &m_colorModels[NUM_COLORS]);
    Mat score = ComputeGMMPDF(hsvImg, &m_colorModels[k]);
    Mat res = Thresholding(score, score_skin, m_colorModels[k].threshold);
    
    return res;
}

float* CColorDetection::GetColorHistogram(Mat rgbImg)
{
    Mat hsvImg = Convert2HSV(rgbImg);
    
    // no masking
    Mat mask = Mat::ones(rgbImg.rows, rgbImg.cols, CV_8U) * 255;
    
    long pixelCount[NUM_COLORS+1], total;
    
    Mat *matArray = new Mat [NUM_COLORS];
    for (int i=0; i<NUM_COLORS; i++) {
        matArray[i] = DetectColor(hsvImg, mask, i, &pixelCount[i]);
    }
    GetMultiColorCount(matArray, &pixelCount[NUM_COLORS], &total);
    
    if (DISPLAY_MODE) {
        imshow("Original",rgbImg);
        cvMoveWindow("Original", 20, 20);
        cvWaitKey(0);
    }
    
    float *hist = new float [NUM_COLORS];
    memset(hist, 0, sizeof(float)*NUM_COLORS);
    
    for (int i=0; i<NUM_COLORS+1; i++) { 
        
        if (total > 0)
            hist[i] = pixelCount[i] / (float) total;
        
        if (DISPLAY_MODE && i < NUM_COLORS) {
            imshow(COLORS[i], matArray[i]);
            cvMoveWindow(COLORS[i], 20, 45+rgbImg.rows);
            cvWaitKey(0);
        }
    }
    

    delete [] matArray;
    
    return hist;
}

Mat CColorDetection::GetColorSegment(Mat rgbImg, const char *color)
{    
    Mat hsvImg = Convert2HSV(rgbImg);
    
    int colorIdx=-1;
    for (int i=0; i<NUM_COLORS; i++) {
        if (!strcmp(color, COLORS[i])) {
            colorIdx = i;
            break;
        }
    }
    
    if (-1 == colorIdx) {
        cout << "ERROR: " << color << " " << "undefined ..." << endl;
        exit(1);
    }

    
    return DetectColor(hsvImg, colorIdx);
}

Mat CColorDetection::RGB2HSV(Mat rgb) 
{
    std::vector<Mat> rgbChannels;
	cv::split(rgb, rgbChannels);
    
	Mat tempR(rgbChannels[2]);
	Mat tempG(rgbChannels[1]);
	Mat tempB(rgbChannels[0]);
    
    double minv=99999, maxv=-99999;
    double minVal, maxVal; 
    Point minLoc, maxLoc; 
    
    minMaxLoc(tempR, &minVal, &maxVal, &minLoc, &maxLoc);
    
    minv = (minVal < minv ? minVal : minv);
    maxv = (maxVal > maxv ? maxVal : maxv);
    
    minMaxLoc(tempG, &minVal, &maxVal, &minLoc, &maxLoc);
    
    minv = (minVal < minv ? minVal : minv);
    maxv = (maxVal > maxv ? maxVal : maxv);
    
    minMaxLoc(tempB, &minVal, &maxVal, &minLoc, &maxLoc);
    
    minv = (minVal < minv ? minVal : minv);
    maxv = (maxVal > maxv ? maxVal : maxv);
    float range = maxv - minv;
    
        
    Mat minM = Mat::ones(rgb.rows, rgb.cols, CV_32F) *  99999;
    Mat maxM = Mat::ones(rgb.rows, rgb.cols, CV_32F) * -99999;
    
    Mat R = Mat::zeros(rgb.rows, rgb.cols, CV_32F);
    Mat G = Mat::zeros(rgb.rows, rgb.cols, CV_32F);
    Mat B = Mat::zeros(rgb.rows, rgb.cols, CV_32F);
    
    for (int i=0; i<rgb.rows; i++) {
        for (int j=0; j<rgb.cols; j++) {
            R.at<float>(i,j) = (float)(tempR.at<uchar>(i,j) - minv) / range;
            G.at<float>(i,j) = (float)(tempG.at<uchar>(i,j) - minv) / range;
            B.at<float>(i,j) = (float)(tempB.at<uchar>(i,j) - minv) / range;
            
            minM.at<float>(i,j) = (minM.at<float>(i,j) > R.at<float>(i,j) ? R.at<float>(i,j) : minM.at<float>(i,j));
            minM.at<float>(i,j) = (minM.at<float>(i,j) > G.at<float>(i,j) ? G.at<float>(i,j) : minM.at<float>(i,j));
            minM.at<float>(i,j) = (minM.at<float>(i,j) > B.at<float>(i,j) ? B.at<float>(i,j) : minM.at<float>(i,j));
            
            maxM.at<float>(i,j) = (maxM.at<float>(i,j) < R.at<float>(i,j) ? R.at<float>(i,j) : maxM.at<float>(i,j));
            maxM.at<float>(i,j) = (maxM.at<float>(i,j) < G.at<float>(i,j) ? G.at<float>(i,j) : maxM.at<float>(i,j));
            maxM.at<float>(i,j) = (maxM.at<float>(i,j) < B.at<float>(i,j) ? B.at<float>(i,j) : maxM.at<float>(i,j));
        
        }
    }
    
    
    Mat hsv(rgb.rows, rgb.cols, CV_32FC3);
    Mat h = Mat::zeros(rgb.rows, rgb.cols, CV_32F);
    Mat s = minM;
    Mat v = maxM;
    
       
    for (int i=0; i<rgb.rows; i++) {
        for (int j=0; j<rgb.cols; j++) {
            if (s.at<float>(i,j) != v.at<float>(i,j)) {
            
                if (v.at<float>(i,j) == B.at<float>(i,j)) {
                    h.at<float>(i,j) = 2./3 + 1./6 * (R.at<float>(i,j)-G.at<float>(i,j)) / (v.at<float>(i,j)-s.at<float>(i,j));
                }
                else if (v.at<float>(i,j) == G.at<float>(i,j)) {
                    h.at<float>(i,j) = 1./3 + 1./6 * (B.at<float>(i,j)-R.at<float>(i,j)) / (v.at<float>(i,j)-s.at<float>(i,j));
                }
                else if (v.at<float>(i,j) == R.at<float>(i,j)) {
                    h.at<float>(i,j) = 1./6 * (G.at<float>(i,j)-B.at<float>(i,j)) / (v.at<float>(i,j)-s.at<float>(i,j));
                    
                } 
            }
            
            if (h.at<float>(i,j) < 0)
                h.at<float>(i,j) += 1;
            
            
            if (s.at<float>(i,j) == v.at<float>(i,j)) {
                s.at<float>(i,j) = 0;
            }
            else {
                s.at<float>(i,j) = 1.0 - s.at<float>(i,j) / v.at<float>(i,j);
            }
            
            
            hsv.at<Vec3f>(i,j)[0] = (float) h.at<float>(i,j);
            hsv.at<Vec3f>(i,j)[1] = (float) s.at<float>(i,j);
            hsv.at<Vec3f>(i,j)[2] = (float) v.at<float>(i,j);
            
        }    
    }
    
    return hsv;
}

