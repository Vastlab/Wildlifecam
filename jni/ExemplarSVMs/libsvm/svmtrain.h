#ifndef _SVM_TRAIN_H
#define _SVM_TRAIN_H

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "svm.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace cv;
using namespace std;

struct svm_model* svmtrain(const char *cmd, int *labels, double *instances, int n, int p);
void svmfree(struct svm_model *model);

vector<double> svmGetRho();
vector<Mat> svmGetSVCoef();
Mat svmGetSVs(int num_features);


#ifdef __cplusplus
}
#endif

#endif


