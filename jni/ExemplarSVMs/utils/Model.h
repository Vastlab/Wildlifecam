#ifndef _MODEL_H
#define _MODEL_H

#include <vector>
#include <string.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "../libMR/MetaRecognition.h"

using namespace std;
using namespace cv;


class CModel {
public:
    CModel() {
        m_ws = NULL;  m_mr = NULL;
        m_width = m_height = m_d3 = 0;
        m_b = 0;
        
        m_beta[0] = 0.1;
        m_beta[1] = 100;
        m_scale = 1;
        m_scale_x = 1;
        m_scale_y = 1;
    }
    ~CModel() {
        if (NULL != m_ws) delete [] m_ws;
        if (NULL != m_mr) delete m_mr;

        ClearSVs();
        ClearXS();
    }
    
    int getSizeW() {return m_width;}
    int getSizeH() {return m_height;}
    int getSizeD() {return m_d3;}
    double getScale() {return m_scale;}
    double getScaleX() {return m_scale_x;}
    double getScaleY() {return m_scale_y;}
    
    int getImgWidth() {return m_imgWidth;}
    int getImgHeight() {return m_imgHeight;}
    
    
    void setSizeW(int w) {m_width = w;}
    void setSizeH(int h) {m_height = h;}
    void setSizeD(int d) {m_d3 = d;}
    void setScale(double s) {m_scale = s;}
    void setScaleX(double s) {m_scale_x = s;}
    void setScaleY(double s) {m_scale_y = s;}
    
    void setImgWidth(int w) {m_imgWidth = w;}
    void setImgHeight(int h) {m_imgHeight = h;}

    
    void setBias(double b) {m_b = b;}
    double getBias() {return m_b;}

    double* getWS() {return m_ws;}
    
    void setWS(double *ws) {
        if (NULL != m_ws) delete [] m_ws;
        
        long lsize = m_width*m_height*m_d3;
        m_ws = new double [lsize];
        memcpy(m_ws, ws, sizeof(double)*lsize);
    }

    
    void ClearXS() {
        for (int k=0; k<(int)m_xsSet.size(); k++) {
            double *ptr = m_xsSet.at(k);
            delete [] ptr;
        }
        m_xsSet.clear();
    }

    // positive samples
    void addXS(double *xs) {
        
        long lsize = m_width*m_height*m_d3;
        double *p = new double [lsize];
        memcpy(p, xs, sizeof(double)*lsize);
        
        m_xsSet.push_back(p);
    }
    double* getXS(int k) {
        if (k < 0 || k >= (int)m_xsSet.size()) {
            cout << "ERROR: Invalid XS ...\n";
            exit(0);
        }
        
        return m_xsSet.at(k);
    }
    int getNumXS() {return (int)m_xsSet.size();}
    
    ////////////
    
    void ClearSVs() {
        for (int k=0; k<(int)m_svxs.size(); k++) {
            double *ptr = m_svxs.at(k);
            delete [] ptr;
        }
        m_svxs.clear();
    }
    
    // negative samples
    void addSVs(double *ptr) {
        long lsize = m_width*m_height*m_d3;
        double *p = new double [lsize];
        memcpy(p, ptr, sizeof(double)*lsize);
        
        m_svxs.push_back(p);
    }
    int getNumSVs() {return (int)m_svxs.size();}
    
    double* getSV(int k) {
        if (k < 0 || k >= (int)m_svxs.size()) {
            cout << "ERROR: Invalid support vectors ...\n";
            exit(0);
        }
                
        return m_svxs.at(k);
    }
    
    void updateSVs(vector<double*> SVs) {
        ClearSVs();
        
        for (int k=0; k<(int)SVs.size(); k++) {
            double *ptr = SVs.at(k);
            addSVs(ptr);
        }
    }
    
    void setBeta(double *beta) {
        m_beta[0] = beta[0];
        m_beta[1] = beta[1];
    }
    
    void getBeta(double *b1, double *b2) {
        *b1 = m_beta[0];
        *b2 = m_beta[1];
    }
    
    void MRFit(double *tail, int tailsize, bool bSkipFirst) {
        if (NULL != m_mr) delete m_mr;
        
        m_mr = new MetaRecognition(bSkipFirst, tailsize);
        m_mr->FitHigh(tail, tailsize);
    }
    
    double* getMRScores(double *fdata, int fsize) {
        if (NULL == m_mr || NULL == fdata) return NULL;
        
        double *outdata = new double [fsize];
        m_mr->ReNormalize(fdata, outdata, fsize);
        
        return outdata;
    }
    
    
    void saveModel(char *filename) {
        
        FileStorage fs;
        fs.open(filename, FileStorage::WRITE);
        fs << "bias" << m_b;
        fs << "beta1" << m_beta[0]; // platt parameter
        fs << "beta2" << m_beta[1]; // platt parameter
        fs << "width" << m_width;
        fs << "height" << m_height;
        fs << "orientation" << m_d3;
        
        long lsize = m_width*m_height*m_d3;
        Mat x = Mat(lsize,1, CV_64F, m_ws);
        
        fs << "data" << x;
        fs.release();
    }
    
    void saveMRParams(char *filename) {
        if (NULL != m_mr)
            m_mr->Save(filename);
    }
    void loadMRParams(char *filename) {
        if (NULL != m_mr) delete m_mr;
        
        m_mr = new MetaRecognition;
        m_mr->Load(filename);
    }
    
    void loadModel(char *filename) {

        FileStorage fs;
        fs.open(filename, FileStorage::READ);
        if (!fs.isOpened()) {
            cout << "ERROR: failed to open " << filename << endl;
            exit(0);
        }
        else {
            if (NULL != m_ws) delete [] m_ws;
            
            fs["bias"] >> m_b;
            fs["beta1"] >> m_beta[0];
            fs["beta2"] >> m_beta[1];
            fs["width"] >> m_width;
            fs["height"] >> m_height;
            fs["orientation"] >> m_d3;
            
            long lsize = m_width*m_height*m_d3;
            m_ws = new double [lsize];

            Mat x;
            fs["data"] >> x;
            
            memcpy(m_ws, x.data, sizeof(double)*lsize);
            fs.release();
        }

    }
    
private:
    double *m_ws;
    vector<double*> m_xsSet;
    double m_b;
    double m_scale, m_scale_x, m_scale_y;
    double m_beta[2];
    vector<double*> m_svxs;   // support vectors
    MetaRecognition *m_mr;
    
    int m_width, m_height, m_d3;
    int m_imgWidth, m_imgHeight;
};




#define SCALE_SIZE1 600
#define SCALE_SIZE2 300

static Mat ScaleImage(Mat display, double *ratio)
{
    *ratio=1;
    Mat img;
    
    if (max(display.cols, display.rows) > SCALE_SIZE1) {
        
        *ratio = max(display.cols, display.rows) / (double)SCALE_SIZE1;
        double scaler = 1.0 / (*ratio);
        
        int w = (int) (scaler * display.cols + .5);
        int h = (int) (scaler * display.rows + .5);
        
        img = cv::Mat::zeros(h, w, CV_8UC3);
        
        cv::resize(display, img, img.size()); 
    }
    else if (min(display.cols, display.rows) < SCALE_SIZE2) {
        
        *ratio = min(display.cols, display.rows) / (double)SCALE_SIZE2;
        double scaler = 1 / (*ratio);
        
        int w = (int) (scaler * display.cols + .5);
        int h = (int) (scaler * display.rows + .5);
        
        img = cv::Mat::zeros(h, w, CV_8UC3);
        
        cv::resize(display, img, img.size()); 
    }
    else
        img = display;
    
    
    return img;
}



#endif
