#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fstream>

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#include "utils/ImageSet.h"
#include "internal/TrainModels.h"
#include "internal/Detect.h"
#include "internal/PiCoDes.h"

using namespace std;
using namespace cv;

#define BUFFER_LENGTH 256

/*
const char *clsLabels[] = {"Butterfly", "Skull","Flower","Star","Dragon","Person"};
const char *modelFolders[] = {
    "Models/Butterfly1/Butterfly",
    "Models/Skulls1/Skulls",
    "Models/Flower1/Flower",
    "Models/Star1/Star",
    "Models/Dragon1/Dragon"
};
const char *trainingFolders[] = {
    "../../Data/Tattoos/data/butterfly-positive-training/",
    "../../Data/Tattoos/data/skulls-positive-training/",
    "../../Data/Tattoos/data/flower-positive-training/",
    "../../Data/Tattoos/data/star-positive-training/",
    "../../Data/Tattoos/data/dragon-positive-training/",
    "../../Data/Person/person-positive-training/"
};



int main(int argc, char **argv)
{     
    // training
    int nCls = sizeof(clsLabels)/sizeof(clsLabels[0]);
    
    vector<CImageSet*> dataset;
    for (int i=0; i<nCls; i++) {
        // read data
        CImageSet *data = new CImageSet(trainingFolders[i], clsLabels[i], false);
        
        // we do not have the last exemplarSVMs model
        if (i < nCls-1)
            data->SetModelName(modelFolders[i]);
        
        dataset.push_back(data);
    }
    
    CPicodes picodesSVMs("Models/rf_mr3.txt");

    cout << "Training ...\n";
    
    // use n-1 exemplarSVMs classes to train picodes
    picodesSVMs.Train(dataset,"MR", nCls-1);

    
    // testing  
    const char *clsName[] = {"butterfly","skulls","flower","star","dragon","person"};
    int clsSize[] = {50,50,46,50,42,50};
    
    
    // we only train n-1 classes
    picodesSVMs.ReadModels(modelFolders, nCls-1);
    
    
    // test all n classes where the last class does not 
    int cMatrix[nCls][nCls];
    for (int i=0; i<nCls; i++) 
        for (int j=0; j<nCls; j++) 
            cMatrix[i][j] = 0;
    
    char filename[80];
    for (int k=0; k<nCls; k++) {
        
        int TP=0,FP=0;
        for (int i=1; i<=clsSize[k]; i++) {
            sprintf(filename,"Test/%s_img%d.jpg",clsName[k],i);
            
            int cls = picodesSVMs.SingleTest(filename, "MR");
            
            cMatrix[k][cls]++;
            
            if (cls == k) TP++;
            else FP++;
            
            cout << TP << " " << FP << " " << TP / (double)(TP+FP) << endl;
        }
        cout << "\n-----------------------------------------\n";
    }
    
    
    for (int i=0; i<nCls; i++) {
        for (int j=0; j<nCls; j++) 
            cout << cMatrix[i][j] << "\t";
        cout << "\n";
    }

    
    for (int i=0; i<nCls; i++) {
        CImageSet *data = dataset.at(i);
        
        delete data;
    }

    
    return EXIT_SUCCESS;
}
*/


void RunBenchmark(const char *infname, const char *outfname)
{
    
    int c,nCls=0;
    FILE *fp = fopen(infname,"rt");
    if (NULL == fp) {
        cout << "Could not open " << infname << endl;
        exit(0);
    }
    while ( (c=fgetc(fp)) != EOF ) {
        if ( c == '\n' ) nCls++;
    }
    fclose(fp);
    
    char *clsName[nCls], *modelFile[nCls], *modelFolder[nCls];
    for (int i=0; i<nCls; i++) {
        clsName[i] = new char [BUFFER_LENGTH];
        modelFile[i] = new char [BUFFER_LENGTH];
        modelFolder[i] = new char [BUFFER_LENGTH];
    }
    
    int cMatrix[nCls][nCls];
    for (int i=0; i<nCls; i++)
        for (int j=0; j<nCls; j++)
            cMatrix[i][j] = 0;
    
  
    fp = fopen(infname,"rt");
    if (NULL == fp) {
        cout << "Could not open " << infname << endl;
        exit(0);
    }
    int k=0;
    while (!feof(fp)) {
        cout << "Reading " << k << endl;
        fscanf(fp,"%s %s %s\n",clsName[k], modelFile[k], modelFolder[k]);
        k++;
    }
    fclose(fp);
    
    CPicodes picodesSVMs(picodes_model_file);
    picodesSVMs.ReadModels((const char**)modelFolder, nCls);
    
    
    float *scores = new float [nCls];
    
    FILE *fout = fopen(outfname,"wt");
    
    char filename[BUFFER_LENGTH];
    for (int k=0; k<nCls; k++) {
        
        int count=0,TP=0,FP=0;
        fp = fopen(modelFile[k],"rt");
        if (NULL == fp) {
            cout << "Could not open " << modelFile[k] << endl;
            exit(0);
        }
        while (!feof(fp)) {
            fscanf(fp,"%s\n",filename);
            
            cout << "Running " << filename << " " << k << endl;
            int cls = picodesSVMs.SingleTest(filename, scores);
            
            cMatrix[k][cls]++;
            
            if (cls == k) TP++;
            else FP++;
            
            cout << TP << " " << FP << " " << TP / (double)(TP+FP) << " . . . " << cls << endl;
            
            for (int i=0; i<nCls; i++) {
                fprintf(fout,"%d %d %f\n",k,i,scores[i]);
                fflush(fout);
            }
     
            count++;
            // only test 10 images at most
            if (count >= 10) break;
        }
        cout << "\n----------------------------\n";
        
        fclose(fp);
    }
    
    fclose(fout);
    delete [] scores;
    
  
    fp = fopen("result.txt","wt");
    for (int i=0; i<nCls; i++) {
        for (int j=0; j<nCls; j++) {
            cout << cMatrix[i][j] << "\t";
            fprintf(fp,"%d ",cMatrix[i][j]);
        }
        cout << "\n";
        fprintf(fp,"\n");
    }
    fclose(fp);
  
}

void PrintUsage()
{
    cout << "****************************** Usage **************************************** \n";
    cout << " -Train training_files \n";
    cout << " -Test testing_files output_file  \n";
}

int main(int argc, char **argv)
{
    if (argc != 3 && argc != 4) {
        PrintUsage();
        exit(0);
    }
    else {
        if (!strcmp(argv[1],"-Train") && argc == 3) {
            int nCls=0;
            char modelName[BUFFER_LENGTH], trainingFile[BUFFER_LENGTH], modelFolder[BUFFER_LENGTH];
            vector<CImageSet*> dataset;
            
            
            FILE *fp = fopen(argv[2],"rt");
            if (NULL == fp) {
                cout << "Could not open " << argv[2] << endl;
                exit(0);
            }
            while (!feof(fp)) {
                cout << "Reading " << nCls << endl;
                fscanf(fp,"%s %s %s\n",modelName, trainingFile, modelFolder);
                
                CImageSet *data = new CImageSet(trainingFile, modelName, false);
                
                data->SetModelName(modelFolder);
                dataset.push_back(data);
                nCls++;
            }
            fclose(fp);
            
            
            CPicodes picodesSVMs(picodes_model_file);
            picodesSVMs.Train(dataset,dataset.size());
            
            
            for (int i=0; i<nCls; i++) {
                CImageSet *data = dataset.at(i);
                
                delete data;
            }

        }
        else if (!strcmp(argv[1],"-Test") && argc == 4) {
            RunBenchmark(argv[2], argv[3]);
        }
        else {
            PrintUsage();
            exit(0);
        }
    }
    
    return EXIT_SUCCESS;
}


