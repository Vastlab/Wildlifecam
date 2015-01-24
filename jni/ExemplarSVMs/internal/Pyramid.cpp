#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fstream>
#include <time.h>
#include <pthread.h>
#include "Pyramid.h"

// small value, used to avoid division by zero
#define eps 0.0001
#define MIN_IMAGE_SIZE 320

// unit vectors used to compute gradient orientation
double uu[9] = {1.0000, 
    0.9397, 
    0.7660, 
    0.500, 
    0.1736, 
    -0.1736, 
    -0.5000, 
    -0.7660, 
    -0.9397};
double vv[9] = {0.0000, 
    0.3420, 
    0.6428, 
    0.8660, 
    0.9848, 
    0.9848, 
    0.8660, 
    0.6428, 
    0.3420};



double* ComputeHOGFeature(std::vector<Mat> *rgb, int sbin, int *d1, int *d2, int *d3) 
{
    Mat R = rgb->at(0);
    Mat G = rgb->at(1);
    Mat B = rgb->at(2);
    
   
    int dims[2];
    dims[0] = R.cols;
    dims[1] = R.rows;
    
    // memory for caching orientation histograms & their norms
    int blocks[2];
    blocks[0] = (int)round((double)dims[0]/(double)sbin);
    blocks[1] = (int)round((double)dims[1]/(double)sbin);
    
    double *hist = (double *)calloc(blocks[0]*blocks[1]*18, sizeof(double));
    double *norm = (double *)calloc(blocks[0]*blocks[1], sizeof(double));
    
    // memory for HOG features
    int out[3];
    *d1 = out[0] = max(blocks[0]-2, 0);
    *d2 = out[1] = max(blocks[1]-2, 0);
    *d3 = out[2] = 27+4;
    
    double *feat = new double [out[0]*out[1]*out[2]];  
    
    int visible[2];
    visible[0] = blocks[0]*sbin;
    visible[1] = blocks[1]*sbin;
    
    for (int x = 1; x < visible[1]-1; x++) {
        for (int y = 1; y < visible[0]-1; y++) {

            // first color channel
            uchar *s = R.data + min(x, dims[1]-2)*dims[0] + min(y, dims[0]-2);
            double dy = (double)*(s+1) - (double)*(s-1);
            double dx = (double)*(s+dims[0]) - (double)*(s-dims[0]);
            double v = dx*dx + dy*dy;
            
            
            // second color channel
            s = G.data + min(x, dims[1]-2)*dims[0] + min(y, dims[0]-2);
            double dy2 = (double)*(s+1) - (double)*(s-1);
            double dx2 = (double)*(s+dims[0]) - (double)*(s-dims[0]);
            double v2 = dx2*dx2 + dy2*dy2;
            
                
            // third color channel
            s = B.data + min(x, dims[1]-2)*dims[0] + min(y, dims[0]-2);
            double dy3 = (double)*(s+1) - (double)*(s-1);
            double dx3 = (double)*(s+dims[0]) - (double)*(s-dims[0]);
            double v3 = dx3*dx3 + dy3*dy3;
            
            
            // pick channel with strongest gradient
            if (v2 > v) {
                v = v2;
                dx = dx2;
                dy = dy2;
            } 
            if (v3 > v) {
                v = v3;
                dx = dx3;
                dy = dy3;
            }
            
            // snap to one of 18 orientations
            double best_dot = 0;
            int best_o = 0;
            for (int o = 0; o < 9; o++) {
                double dot = uu[o]*dx + vv[o]*dy;
                if (dot > best_dot) {
                    best_dot = dot;
                    best_o = o;
                } else if (-dot > best_dot) {
                    best_dot = -dot;
                    best_o = o+9;
                }
            }
            
            // add to 4 histograms around pixel using linear interpolation
            double xp = ((double)x+0.5)/(double)sbin - 0.5;
            double yp = ((double)y+0.5)/(double)sbin - 0.5;
            int ixp = (int)floor(xp);
            int iyp = (int)floor(yp);
            double vx0 = xp-ixp;
            double vy0 = yp-iyp;
            double vx1 = 1.0-vx0;
            double vy1 = 1.0-vy0;
            v = sqrt(v);
            
            if (ixp >= 0 && iyp >= 0) {
                *(hist + ixp*blocks[0] + iyp + best_o*blocks[0]*blocks[1]) += 
                vx1*vy1*v;
            }
            
            if (ixp+1 < blocks[1] && iyp >= 0) {
                *(hist + (ixp+1)*blocks[0] + iyp + best_o*blocks[0]*blocks[1]) += 
                vx0*vy1*v;
            }
            
            if (ixp >= 0 && iyp+1 < blocks[0]) {
                *(hist + ixp*blocks[0] + (iyp+1) + best_o*blocks[0]*blocks[1]) += 
                vx1*vy0*v;
            }
            
            if (ixp+1 < blocks[1] && iyp+1 < blocks[0]) {
                *(hist + (ixp+1)*blocks[0] + (iyp+1) + best_o*blocks[0]*blocks[1]) += 
                vx0*vy0*v;
            }
        }
    }
    
    
    // compute energy in each block by summing over orientations
    for (int o = 0; o < 9; o++) {
        double *src1 = hist + o*blocks[0]*blocks[1];
        double *src2 = hist + (o+9)*blocks[0]*blocks[1];
        double *dst = norm;
        double *end = norm + blocks[1]*blocks[0];
        while (dst < end) {
            *(dst++) += (*src1 + *src2) * (*src1 + *src2);
            src1++;
            src2++;
        }
    }
    
    // compute features
    for (int x = 0; x < out[1]; x++) {
        for (int y = 0; y < out[0]; y++) {
            double *dst = feat + x*out[0] + y;      
            double *src, *p, n1, n2, n3, n4;
            
            p = norm + (x+1)*blocks[0] + y+1;
            n1 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
            p = norm + (x+1)*blocks[0] + y;
            n2 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
            p = norm + x*blocks[0] + y+1;
            n3 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
            p = norm + x*blocks[0] + y;      
            n4 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
            
            double t1 = 0;
            double t2 = 0;
            double t3 = 0;
            double t4 = 0;
            
            // contrast-sensitive features
            src = hist + (x+1)*blocks[0] + (y+1);
            for (int o = 0; o < 18; o++) {
                double h1 = min(*src * n1, 0.2);
                double h2 = min(*src * n2, 0.2);
                double h3 = min(*src * n3, 0.2);
                double h4 = min(*src * n4, 0.2);
                *dst = 0.5 * (h1 + h2 + h3 + h4);
                t1 += h1;
                t2 += h2;
                t3 += h3;
                t4 += h4;
                dst += out[0]*out[1];
                src += blocks[0]*blocks[1];
            }
            
            // contrast-insensitive features
            src = hist + (x+1)*blocks[0] + (y+1);
            for (int o = 0; o < 9; o++) {
                double sum = *src + *(src + 9*blocks[0]*blocks[1]);
                double h1 = min(sum * n1, 0.2);
                double h2 = min(sum * n2, 0.2);
                double h3 = min(sum * n3, 0.2);
                double h4 = min(sum * n4, 0.2);
                *dst = 0.5 * (h1 + h2 + h3 + h4);
                dst += out[0]*out[1];
                src += blocks[0]*blocks[1];
            }
            
            // texture features
            *dst = 0.2357 * t1;
            dst += out[0]*out[1];
            *dst = 0.2357 * t2;
            dst += out[0]*out[1];
            *dst = 0.2357 * t3;
            dst += out[0]*out[1];
            *dst = 0.2357 * t4;
        }
    }
    
    free(hist);
    free(norm);

    return feat;     
}


Mat GetTranslatedImg(Mat M, int dx, int dy)
{
    int r = M.rows;
    int c = M.cols;
    
    if (dx == 0 && dy == 0) return M;
    
    Mat MM = M.clone();
    
    if (dx != 0) {
                
        if (dx > 0) {
            for (int i=0; i<dx; i++) {
                Mat c1 = M.col(c-i-1);
                Mat c2 = M.col(i);
                
                c2.col(0).copyTo(MM.col(c-i-1));
                c1.col(0).copyTo(MM.col(i));
            }
            for (int i=0; i<c-dx; i++) {
                M.col(i).copyTo(MM.col(i+dx));
            }
        }
        else if (dx < 0) {
            for (int i=0; i<abs(dx); i++) {
                Mat c1 = M.col(i);
                Mat c2 = M.col(c-i-1);
                
                c2.col(0).copyTo(MM.col(i));
                c1.col(0).copyTo(MM.col(c-i-1));
            }
            for (int i=0; i<c-abs(dx); i++) {
                M.col(i-dx).copyTo(MM.col(i));
            }
        }
        
        M = MM.clone();
    }
    
    if (dy != 0) {
        if (dy > 0) {
            for (int i=0; i<dy; i++) {
                Mat r1 = M.row(r-i-1);
                Mat r2 = M.row(i);
                
                r2.row(0).copyTo(MM.row(r-i-1));
                r1.row(0).copyTo(MM.row(i));

            }
            for (int i=0; i<r-dy; i++) {
                M.row(i).copyTo(MM.row(i+dy));
            }
        }
        else if (dy < 0) {
            for (int i=0; i<abs(dy); i++) {
                Mat r1 = M.row(i);
                Mat r2 = M.row(r-i-1);
                
                r2.row(0).copyTo(MM.row(i));
                r1.row(0).copyTo(MM.row(r-i-1));
            }
            for (int i=0; i<r-abs(dy); i++) {
                M.row(i-dy).copyTo(MM.row(i));
            }
        }

    }
    
    return MM;
}


// no need to divide the sigma since it is just a scalar where SVM would rescale the output anyway
static double* ZeroMean(double *f, long lsize)
{
    double sumv=0;
    for (long k=0; k<lsize; k++) sumv += f[k];
    double u = sumv / (double) lsize;
    
    double *ff = new double [lsize];
    for (long k=0; k<lsize; k++) 
        ff[k] = f[k] - u;
    
    return ff;
}



vector<PYRLEVEL*> GetPyramid(Mat *img, int *targetlevel)
{
    if (!img->data) {    
        cout << "ERROR: Invalid data ...\n";
        exit(0);
    }
    *targetlevel = 0;
    
    float sc = (float) pow(2.0, 1.0/(float)DETECT_LEVELS_PER_OCTAVE);
    
    
    int d1,d2,d3;
    std::vector<Mat> rgb, rgb2;
    vector<PYRLEVEL*> pyramid; 
    
    for (int i=0; i<PYRAMID_MAXLEVELS; i++) {
        float scaler = (float)DETECT_MAX_SCALE / pow(sc,i);
        
        if (scaler < DETECT_MIN_SCALE) return pyramid;
        
        int w = (int) (scaler * img->cols + .5);
        int h = (int) (scaler * img->rows + .5);
        if (min(w,h) <= PYRAMID_MINDIMENSION) return pyramid;
    
        
        Mat dst = cv::Mat::zeros(h, w, CV_8UC3);
        
        cv::resize(*img, dst, dst.size());
        
        cv::split(dst, rgb);
        cv::split(dst, rgb2);
        
        PYRLEVEL *pyrLevel = new PYRLEVEL;
        
        int nx=SHIFTX, ny=SHIFTY, k=0;
        for (int dx=-nx; dx<=nx; dx++) {
            for (int dy=-ny; dy<=ny; dy++) {
            
                Mat R = rgb.at(0).clone();
                Mat G = rgb.at(1).clone();
                Mat B = rgb.at(2).clone();
      
                rgb2.at(0) = GetTranslatedImg(R, SHIFT_STEP*dx, SHIFT_STEP*dy);
                rgb2.at(1) = GetTranslatedImg(G, SHIFT_STEP*dx, SHIFT_STEP*dy);
                rgb2.at(2) = GetTranslatedImg(B, SHIFT_STEP*dx, SHIFT_STEP*dy);
        
                double *feat = ComputeHOGFeature(&rgb2, HOG_SBIN, &d1, &d2, &d3);
                
                pyrLevel->scaler = scaler;
                pyrLevel->padsize = 0;
                pyrLevel->w = d1;
                pyrLevel->h = d2;
                pyrLevel->d = d3;
                pyrLevel->scaler_x = (w/(double)d1)/scaler;
                pyrLevel->scaler_y = (h/(double)d2)/scaler;
                
                
                pyrLevel->mfeat[k] = new double [d1*d2*d3];
                //memcpy(pyrLevel->mfeat[k], feat, sizeof(double)*d1*d2*d3);
                double *ptr = ZeroMean(feat,d1*d2*d3);
                memcpy(pyrLevel->mfeat[k], ptr, sizeof(double)*d1*d2*d3);
                k++;
                
                delete [] feat;
                delete [] ptr;
            }
        }
                
        
        pyramid.push_back(pyrLevel);
        
        if (d1 <= HOG_MAXDIM || d2 <= HOG_MAXDIM) {
            *targetlevel = i;
            return pyramid;
        }
    }
    
    return pyramid;
}

vector<PYRLEVEL*> GetPyramid1(Mat *img, int *targetlevel)
{
    if (!img->data) {    
        cout << "ERROR: Invalid data ...\n";
        exit(0);
    }
    *targetlevel = 0;
    
    float sc = (float) pow(2.0, 1.0/(float)DETECT_LEVELS_PER_OCTAVE);
    
    
    int d1,d2,d3;
    std::vector<Mat> rgb, rgb2;
    vector<PYRLEVEL*> pyramid; 
    
    for (int i=0; i<PYRAMID_MAXLEVELS; i++) {
        float scaler = (float)DETECT_MAX_SCALE / pow(sc,i);
        
        if (scaler < DETECT_MIN_SCALE) return pyramid;
        
        int w = (int) (scaler * img->cols + .5);
        int h = (int) (scaler * img->rows + .5);
        if (min(w,h) <= PYRAMID_MINDIMENSION) return pyramid;
        
        
        Mat dst = cv::Mat::zeros(h, w, CV_8UC3);
        
        cv::resize(*img, dst, dst.size());
        
        cv::split(dst, rgb);
        
        double *feat = ComputeHOGFeature(&rgb, HOG_SBIN, &d1, &d2, &d3);
        
        PYRLEVEL *pyrLevel = new PYRLEVEL;
        pyrLevel->scaler = scaler;
        pyrLevel->padsize = 0;
        pyrLevel->w = d1;
        pyrLevel->h = d2;
        pyrLevel->d = d3;
        pyrLevel->scaler_x = (w/(double)d1)/scaler;
        pyrLevel->scaler_y = (h/(double)d2)/scaler;
        
        
        pyrLevel->feat = new double [d1*d2*d3];
        //memcpy(pyrLevel->feat, feat, sizeof(double)*d1*d2*d3);
        double *ptr = ZeroMean(feat,d1*d2*d3);
        memcpy(pyrLevel->feat, ptr, sizeof(double)*d1*d2*d3);
        
        pyramid.push_back(pyrLevel);
        
        delete [] feat;
        delete [] ptr;
        
        if (d1 <= HOG_MAXDIM || d2 <= HOG_MAXDIM) {
            *targetlevel = i;
            return pyramid;
        }
    }
    
    return pyramid;
}


vector<PYRLEVEL*> GetPyramidPadZero(Mat *img, int ww, int hh, int *targetlevel)
{
    if (!img->data) {
        cout << "ERROR: Invalid data ...\n";
        exit(0);
    }
    *targetlevel = 0;
    
    float sc = (float) pow(2.0, 1.0/(float)DETECT_LEVELS_PER_OCTAVE);
    
    int d1,d2,d3;
    std::vector<Mat> rgb;
    vector<PYRLEVEL*> pyramid; 
    
    
    for (int i=0; i<PYRAMID_MAXLEVELS; i++) {
        
        float scaler = (float)DETECT_MAX_SCALE / pow(sc,i);
                
        if (scaler < DETECT_MIN_SCALE) return pyramid;
        
        int w = (int) (scaler * img->cols + .5);
        int h = (int) (scaler * img->rows + .5);

        if (min(w,h) <= PYRAMID_MINDIMENSION) return pyramid;
        
        PYRLEVEL *pyrLevel = new PYRLEVEL;
        
        Mat dst = cv::Mat::zeros(h, w, CV_8UC3);
        cv::resize(*img, dst, dst.size());
        
        cv::split(dst, rgb);
        
        //double *feat = ComputeHOGFeature(&rgb, HOG_SBIN, &d1, &d2, &d3);
        double *hog = ComputeHOGFeature(&rgb, HOG_SBIN, &d1, &d2, &d3);
        
        
        pyrLevel->padsize = (min(img->cols,img->rows) < MIN_IMAGE_SIZE ? 0 : PADSIZE);
        pyrLevel->scaler = scaler;
        pyrLevel->w = d1+2*pyrLevel->padsize;
        pyrLevel->h = d2+2*pyrLevel->padsize;
        pyrLevel->d = d3;
        pyrLevel->scaler_x = (w/(double)d1)/scaler;
        pyrLevel->scaler_y = (h/(double)d2)/scaler;
        
        double *feat = ZeroMean(hog, d1*d2*d3);
        
        
        // pad zero outsize the image
        pyrLevel->feat = new double [(d1+2*pyrLevel->padsize)*(d2+2*pyrLevel->padsize)*d3];        
        memset(pyrLevel->feat, 0, sizeof(double)*(d1+2*pyrLevel->padsize)*(d2+2*pyrLevel->padsize)*d3);
        
        for (int ii=0; ii<d3; ii++) {
            for (int j1=0,j2=pyrLevel->padsize; j1<d2; j1++, j2++) {
                for (int k1=0,k2=pyrLevel->padsize; k1<d1; k1++, k2++) {
                    
                    pyrLevel->feat[ii*(d2+2*pyrLevel->padsize)*(d1+2*pyrLevel->padsize)+j2*(d1+2*pyrLevel->padsize)+k2] = feat[ii*d2*d1+j1*d1+k1];
                }
            }
        }
                
        pyramid.push_back(pyrLevel);
        
        if (d1 <= ww || d2 <= hh) {
            // negative level means that target is smaller than the template
            *targetlevel = i-1;
            
            delete [] feat;
            delete [] hog;
            return pyramid;
        }
        
        
        delete [] feat;
        delete [] hog;
    }
    
    return pyramid;
}


#ifdef _MULTITHREADED
struct thread_data {
    double *target, *src;
    double *ptr;
    int tW, tH, sW, sH, sD, ww, hh;
    int i, j;
};

void* ProductSum(void *threadarg)
{
    struct thread_data *data;
    data = (struct thread_data *) threadarg;

    
    if (data->i >= data->ww) {
        pthread_exit(NULL);
    }
    else {
        long offset1 = data->tH*data->tW;
        long offset2 = data->sH*data->sW;
        
        
        for (int j=0; j<data->hh; j++) {
            
            double sumv = 0;    
            for (int ii=0; ii<data->sD; ii++) {
                long a1 = ii * offset1;
                long a2 = ii * offset2;
                
                for (int jj=0,j2=j; jj<data->sH; jj++, j2++) {
                    long b1 = j2*data->tW + a1;
                    long b2 = jj*data->sW + a2;

                    for (int kk=b2,k2=data->i+b1; kk<(b2+data->sW); kk++, k2++) {
                        sumv += *(data->target+k2) * *(data->src+kk);
                    }                    
                } 
            }
            
            data->ptr[j] = sumv;
        }
        
        pthread_exit(NULL);
    }
    
}


double** Convolve(PYRLEVEL *pyr, CModel *m, int *ww, int *hh)
{
    int tW = pyr->w;
    int tH = pyr->h;
    int tD = pyr->d;
    int sW = m->getSizeW();
    int sH = m->getSizeH();
    int sD = m->getSizeD();
    
    if (tD != sD || tW < sW || tH < sH) {
        cout << "ERROR: Invalid convolution size ...\n";
        cout << tW << " " << tH << " " << tD <<  endl;
        cout << sW << " " << sH << " " << sD <<  endl;
        exit(0);
    }
    else {
        *ww = (tW - sW + 1);
        *hh = (tH - sH + 1);
        
        // make the res the same for all models
        double **res = new double* [tW];
        for (int i=0; i<tW; i++) {
            res[i] = new double [tH];
            memset(res[i], 0, sizeof(double)*tH);
        }    
        
        
        pthread_t threads[NUM_THREADS];
        struct thread_data td[NUM_THREADS];
        void *status;
    
        double *target = pyr->feat;
        double *src = m->getWS();
        
        
        for (int ii=0; ii<NUM_THREADS; ii++) {
            td[ii].tW = tW;
            td[ii].tH = tH;
            td[ii].sW = sW;
            td[ii].sH = sH;
            td[ii].sD = sD;
            td[ii].ww = *ww;
            td[ii].hh = *hh;
            td[ii].target = target;
            td[ii].src = src;
            //td[ii].ptr = new double [*hh];
        }
        
        
        for (int i=0; i<(*ww); i+=NUM_THREADS) {
                    
            // run the threads
            for (int ii=0; ii<NUM_THREADS; ii++) {
            
                // setup offsets and result pointers
                td[ii].i = i+ii; td[ii].ptr = res[i+ii];
                
                int ret = pthread_create(&threads[ii], NULL, ProductSum, (void *)&td[ii]);
                if (ret) {
                    cout << "Error: unable to create thread, " << ret << endl;
                    exit(-1);
                }
            }
            
            // release threads
            for (int ii=0; ii<NUM_THREADS; ii++) {
                pthread_join(threads[ii], (void**)&status);
            }  
            
            // collect results
        //    for (int ii=0; ii<NUM_THREADS; ii++) {
        //        if (i+ii < *ww) {
        //            memcpy(res[i+ii], td[ii].ptr, sizeof(double)*(*hh));
        //        }
        //    }
            
        }
        
      //  for (int ii=0; ii<NUM_THREADS; ii++) {
      //      delete [] td[ii].ptr;    
      //  }
        
        return res;
    }
}
#else
double** Convolve(PYRLEVEL *pyr, CModel *m, int *ww, int *hh)
{
    int tW = pyr->w;
    int tH = pyr->h;
    int tD = pyr->d;
    int sW = m->getSizeW();
    int sH = m->getSizeH();
    int sD = m->getSizeD();

    if (tD != sD || tW < sW || tH < sH) {
        cout << "ERROR: Invalid convolution size ...\n";
        cout << tW << " " << tH << " " << tD <<  endl;
        cout << sW << " " << sH << " " << sD <<  endl;
        exit(0);
    }
    else {
        *ww = (tW - sW + 1);
        *hh = (tH - sH + 1);
        
        double **res = new double* [*ww];
        for (int i=0; i<(*ww); i++) {
            res[i] = new double [*hh];
            memset(res[i], 0, sizeof(double)*(*hh));
        }
        
        
        double *target = pyr->feat;
        double *src = m->getWS();
        
        
        for (int j=0; j<(*hh); j++) {
            for (int i=0; i<(*ww); i++) {
                
                double sumv = 0;
                for (int ii=0; ii<sD; ii++) {
                    long a1 = ii*tH*tW;
                    long a2 = ii*sH*sW;

                    for (int jj=0,j2=j; jj<sH; jj++, j2++) {
                        long b1 = j2*tW + a1;
                        long b2 = jj*sW + a2;
                        for (int kk=b2,k2=i+b1; kk<(b2+sW); kk++, k2++) {
                            sumv += *(target+k2) * *(src+kk);
                        }
                    }

                }
                res[i][j] = sumv;            
            }
        }
    
        return res;
    }
}
#endif

double Convolve1(PYRLEVEL *pyr, CModel *m)
{
    int tW = pyr->w;
    int tH = pyr->h;
    int tD = pyr->d;
    int sW = m->getSizeW();
    int sH = m->getSizeH();
    int sD = m->getSizeD();
    
    if (tD != sD || tW != sW || tH != sH) {
        cout << "ERROR: Invalid convolution size ...\n";
        cout << tW << " " << tH << " " << tD <<  endl;
        cout << sW << " " << sH << " " << sD <<  endl;
        exit(0);
    }
    else {
        double *target = pyr->feat;
        double *src = m->getWS();
        
        double sumv = 0;
        for (int ii=0; ii<sD; ii++) {
            long a1 = ii*tH*tW;
            long a2 = ii*sH*sW;
            
            for (int jj=0,j2=0; jj<sH; jj++, j2++) {
                long b1 = j2*tW + a1;
                long b2 = jj*sW + a2;
                for (int kk=b2,k2=0+b1; kk<(b2+sW); kk++, k2++) {
                    sumv += *(target+k2) * *(src+kk);
                }
            }
            
        }  
        
        return sumv;
    }
}


double* getROI(PYRLEVEL *data, int dx, int dy, int w, int h)
{
    double *ptr = new double [w*h*data->d];
    memset(ptr, 0, sizeof(double)*w*h*data->d);
    
    for (int ii=0; ii<data->d; ii++) {
        long a1 = ii*w*h;
        long a2 = ii*(data->h)*(data->w);
        for (int jj=0,j2=dy; jj<h; jj++, j2++) {
            long b1 = jj*w + a1;
            long b2 = j2*(data->w) + a2;
            for (int kk=0,k2=dx; kk<w; kk++, k2++) {
                ptr[b1 + kk] = *(data->feat + b2 + k2);
            }
        }
    }
    
    return ptr;    
}



