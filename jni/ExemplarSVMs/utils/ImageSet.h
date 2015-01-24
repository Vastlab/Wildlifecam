#ifndef _IMAGESET_H
#define _IMAGESET_H

#include <vector>
#include <string.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

using namespace std;
using namespace cv;

typedef struct _DATASET {
    vector<Mat> imgs; // image raw data
    vector<Rect> bboxes; // bounding box
} DATASET;

class CImageSet {
public:
    CImageSet(string folderName, string name, bool bAddFlip=false);
    CImageSet(char *filename, string name, bool bAddFlip=false);
    ~CImageSet();
    
    void ShowImage(int k);
    Mat GetImage(int k);
    
    string GetClsName() {return m_clsName;}
    int size() {return m_data.imgs.size();}
    
    void SetModelName(string name) {m_modelName = name;}
    string GetModelName() {return m_modelName;}
    
private:
    DATASET m_data;
    string m_clsName;
    string m_modelName;
    
    void ReadDataFromDir(string folderName, bool bAddFlip);
    void ReadDataFromFile(char *infile, bool bAddFlip);
    
    void AssignData(Mat img) {
        Rect r(0, 0, img.cols, img.rows);
        m_data.imgs.push_back(img);
        m_data.bboxes.push_back(r);
    }
    
};

#endif
