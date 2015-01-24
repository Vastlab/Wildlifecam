#ifndef _PYRAMID_H
#define _PYRAMID_H

#include <vector>
#include <string.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "../utils/define.h"
#include "../utils/Model.h"

using namespace std;
using namespace cv;

#define PADSIZE 0 //2 

typedef struct _PYRLEVEL {
    double *mfeat[SHIFT_SIZE];
    double *feat;
    double scaler;
    double scaler_x, scaler_y;
    int w,h,d;
    int padsize;
} PYRLEVEL;

vector<PYRLEVEL*> GetPyramid(Mat *img, int *targetlevel);
vector<PYRLEVEL*> GetPyramid1(Mat *img, int *targetlevel);
vector<PYRLEVEL*> GetPyramidPadZero(Mat *img, int ww, int hh, int *targetlevel);

double** Convolve(PYRLEVEL *pyr, CModel *m, int *ww, int *hh);

double Convolve1(PYRLEVEL *pyr, CModel *m);

double* getROI(PYRLEVEL *data, int dx, int dy, int w, int h);


#endif
