#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fstream>
#include <time.h> 
#include "ImageSet.h"

#define SCALE_SIZE 600

int getdir(string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if ((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error in opening " << dir << endl;
        return -1;
    }
    
    while ((dirp = readdir(dp)) != NULL) {
        // avoid . files
        if (dirp->d_name[0] != '.')
            files.push_back(string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}

Mat RotateImg(Mat img, int idx)
{
    Mat flip = cv::Mat::zeros(img.rows, img.cols, CV_8UC3);
    img.copyTo(flip);
    
    // angle [1 2 3] -> (90, 180 270)
    for(int i = 0; i<idx; ++i){
        cv::transpose(flip, flip);
        cv::flip(flip, flip, 1);
    }

    return flip;
}


CImageSet::CImageSet(string folderName, string name, bool bAddFlip)
{
    m_data.imgs.clear();
    m_data.bboxes.clear();
    
    m_clsName = name;
    ReadDataFromDir(folderName, bAddFlip);
}

CImageSet::CImageSet(char *filename, string name, bool bAddFlip)
{
    m_data.imgs.clear();
    m_data.bboxes.clear();
    
    m_clsName = name;
    ReadDataFromFile(filename, bAddFlip);
}

CImageSet::~CImageSet()
{
    for (int i=0; i<(int)m_data.imgs.size(); i++) {
        Mat img = m_data.imgs.at(i);
        img.release();
    }
}

void CImageSet::ReadDataFromDir(string folderName, bool bAddFlip)
{
    srand((unsigned)time(NULL)); 
    
    vector<string> files = vector<string>();
    if (0 != getdir(folderName, files)) {
        cout << folderName << " cannot be opened\n";
        exit(0);
    }
    
    cout << "Reading data from " << folderName << endl;
    for (int i=0; i<(int)files.size(); i++) {
        string filename = folderName + files[i];

        try {
            Mat img = imread(filename.c_str());
            
            if (!img.data ) {
                cout <<  "Could not open " << filename.c_str() << std::endl ;
                exit(0);
            }
            
#ifdef _SCALE_IMAGES
            double ratio=1;
            if (max(img.cols, img.rows) > SCALE_SIZE) {
                
                ratio = max(img.cols, img.rows) / (double)SCALE_SIZE;
                double scaler = 1.0 / ratio;
                
                int w = (int) (scaler * img.cols + .5);
                int h = (int) (scaler * img.rows + .5);
                
                Mat dst = cv::Mat::zeros(h, w, CV_8UC3);
                
                cv::resize(img, dst, dst.size()); 
                
                img = dst;
            }
#endif
            
            AssignData(img);
            
            if (bAddFlip) {
                Mat flip1 = cv::Mat::zeros(img.rows, img.cols, CV_8UC3);
                cv::flip(img, flip1, 1);
                
                AssignData(flip1);
            }
        } catch(...) {
            cout << "ERROR: fail to open " << filename.c_str() << "\n";
        }
    }
}

void CImageSet::ReadDataFromFile(char *infile, bool bAddFlip)
{
    srand((unsigned)time(NULL)); 
    
    char fname[256];
    vector<string> files = vector<string>();

    ifstream datafile(infile);
    string line;
    
    if (datafile) {
        while (getline(datafile, line)) {
            if (!line.empty()) {
                files.push_back(line);
            }
        }
        datafile.close();
    }
    else {
        cout << "Could not open " << infile << endl;
        exit(0);
    }
    
    int *order = new int [files.size()];
    for(int i=0; i<files.size(); i++) order[i] = i;
    random_shuffle(order, order+files.size());
    
    
    cout << "Reading data ... (" << m_clsName << ")" << endl;
    for (int i=0; i<(int)files.size(); i++) {
        int idn = order[i];
        string filename = files[idn];
        
        try {
            Mat img = imread(filename.c_str());
            
            if (!img.data ) {
                cout <<  "Could not open " << filename.c_str() << std::endl ;
                exit(0);
            }
            
#ifdef _SCALE_IMAGES
            double ratio=1;
            if (max(img.cols, img.rows) > SCALE_SIZE) {
                
                ratio = max(img.cols, img.rows) / (double)SCALE_SIZE;
                double scaler = 1.0 / ratio;
                
                int w = (int) (scaler * img.cols + .5);
                int h = (int) (scaler * img.rows + .5);
                
                Mat dst = cv::Mat::zeros(h, w, CV_8UC3);
                
                cv::resize(img, dst, dst.size()); 
                
                img = dst;
            }
#endif
            
            AssignData(img);
            
            if (bAddFlip) {
                Mat flip1 = cv::Mat::zeros(img.rows, img.cols, CV_8UC3);
                cv::flip(img, flip1, 1);
                
                AssignData(flip1);
            }
        } catch(...) {
            cout << "ERROR: fail to open " << filename.c_str() << "\n";
        }
    }
    
    delete [] order;
}


Mat CImageSet::GetImage(int k)
{
    if (k < 0 || k > (int)m_data.imgs.size()) {
        cout << "ERROR: Invalid Index ....\n";
        exit(0);
    }
    else {
        return m_data.imgs.at(k);
    }
}

void CImageSet::ShowImage(int k)
{
    if (k < 0 || k > (int)m_data.imgs.size()) {
        cout << "ERROR: Invalid Index ....\n";
        return;
    }
    else {
        Mat img = m_data.imgs.at(k);
        imshow("Image",img);
        cvWaitKey(0);
    }
    
}
