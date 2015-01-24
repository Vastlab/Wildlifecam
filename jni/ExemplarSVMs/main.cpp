#include "ExemplarSVMs.h"

using namespace std;
using namespace cv;

#define BUFFER_LENGTH 256

void BatchTest(char *modelName, char *infile, char *outfile, char *outDir)
{
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
    
    
    string model = "Models/" + (string)modelName + "/" + (string)modelName;
    
    CTestModels exemplarSVMs;
    exemplarSVMs.ReadModels(model.c_str());
    
    FILE *fp=fopen(outfile,"wt");
    for (int i=0; i<(int)files.size(); i++) {
        string fname = files.at(i);
        Mat rgbImg = imread(fname.c_str());
        
        if (!rgbImg.data ) {
            cout <<  "Could not open " << modelName << std::endl ;
            return;
        }
        cout << fname << endl;

	clock_t begin = clock();
        vector<RESSTRUCT> res = exemplarSVMs.Test(rgbImg, DETECTION_THRESHOLD, 0);
	clock_t end = clock();

	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	cout << "Processed in " << elapsed_secs << " secs." << endl;
	
        RESSTRUCT hn;
	char tmp[256], *pos;
	sprintf(tmp,"%s",fname.c_str());
	pos = strrchr(tmp,'/');
	for(unsigned int j=0; j< min(MAX_DETECTED_RETURNS,(int)res.size()); j++) {
	  hn = res.at(j);
	  if(j==0) fprintf(fp,"%s %f %f %f %f %f\n",fname.c_str(), hn.x1, hn.y1, hn.x2, hn.y2, hn.score);
	  if(!pos) sprintf(tmp,"%s/%f_%s",outDir,hn.score,fname.c_str());
	  else sprintf(tmp,"%s/%f_%s",outDir,hn.score,pos+1);
	  fprintf(stdout,"Saving: %s\n",tmp);
	  cv::Mat imgRoi = rgbImg(cv::Rect(hn.x1, hn.y1, (hn.x2-hn.x1), (hn.y2-hn.y1)));
	  cv::imwrite(tmp, imgRoi);
        }


        fflush(fp);
        
        res.clear();
    }
    fclose(fp);
}

void ShowResults()
{
    char fname[256];
    float x1,x2,y1,y2,score;
    
    FILE *fp=fopen("output.txt","rt");
    if (NULL == fp) {
        cout << "Could not open " << "output.txt" << endl;
        exit(0);
    }
    
    while (!feof(fp)) {
        fscanf(fp,"%s %f %f %f %f %f\n",fname, &x1,&y1,&x2,&y2,&score);
        
        Mat rgbImg = imread(fname);
        
        CvPoint pt1 = cvPoint(x1, y1);
        CvPoint pt2 = cvPoint(x2, y2);
        cv::rectangle(rgbImg, pt1, pt2, cvScalar(0, 255, 0), 2, 4, 0);
        imshow(fname,rgbImg);
        cvWaitKey(0);
        
        destroyWindow(fname);
    }
    fclose(fp);
}


char* runExemplar(int argc, char **argv)
{
	char myResults[1024];
	memset(myResults,0,1024);

	char model_dir[128];
	sprintf(model_dir,"%s",argv[5]);
	argc--;

    //if (argc != 4 && argc != 5) {
    if ( argc < 4 || argc > 6) {
        cout << "****************************** Usage **************************************** \n";
        cout << " -Train classname pos_file neg_file \n";
        cout << " -Test className image_file [ color | ../path/to/output/images ] \n";
        cout << " -BatchTest className input_files output_file output_dir\n";
        exit(0);
    }
    else {
        if (!strcmp(argv[1], "-Train")) {
            string cmd = "mkdir Models/" + (string)argv[2];
            system(cmd.c_str());

            string model = "Models/" + (string)argv[2] + "/" + (string)argv[2];

            CImageSet *posSet = new CImageSet(argv[3], argv[2], USE_FLIP);
            CImageSet *negSet = new CImageSet(argv[4], "Negative", false);

            int train_size_neg = min(round(NEGATIVE_SAMPLE_SIZE/2), round(negSet->size()/2));

            CTrainModels exemplarSVMs;
            exemplarSVMs.InitializeExemplars(posSet, 0, posSet->size());
            exemplarSVMs.Train(negSet, 0, train_size_neg);

            // use saved model for validation
            //exemplarSVMs.ReadModels(model.c_str());


            // validation and calibration
            exemplarSVMs.Validate(posSet, negSet, train_size_neg, 2*train_size_neg);
            exemplarSVMs.Calibrate("Occurrence");
            exemplarSVMs.Calibrate("Platt");
            exemplarSVMs.Calibrate("MR");

            exemplarSVMs.SaveModels(model.c_str());

            delete posSet;
            delete negSet;
        }
        else if (!strcmp(argv[1], "-Test")) {


            Mat segment;
            Mat rgbImg = imread(argv[3]);

            if (!rgbImg.data ) {
                cout <<  "Could not open " << argv[3] << std::endl ;
                sprintf(myResults,"ERROR: Could not open %s\n",argv[3]);
                return myResults;
            }

            if (argc == 5 && argv[4][0] != '.') {
                CColorDetection color;
                segment = color.GetColorSegment(rgbImg, argv[4]);
            }


            string model = (string)model_dir + "/" + (string)argv[2] + "/" + (string)argv[2];

            CTestModels exemplarSVMs;
            exemplarSVMs.ReadModels(model.c_str());


            vector<RESSTRUCT> res;
            if (argc == 5 && argv[4][0] != '.')
	        res = exemplarSVMs.Test(rgbImg, segment, DETECTION_THRESHOLD, 0);
            else
	        res = exemplarSVMs.Test(rgbImg, DETECTION_THRESHOLD, 0);


	    RESSTRUCT hn;
	    char tmp[256], *pos;
	    pos = strrchr(argv[3],'/');

			for (unsigned int i = 0; i < min(MAX_DETECTED_RETURNS, (int) res.size()); i++) {
				hn = res.at(i);
				if (!pos)
					sprintf(tmp, "%s/%f_%s", argv[4], hn.score, argv[3]);
				else
					sprintf(tmp, "%s/%f_%s", argv[4], hn.score, pos + 1);
				LOGI("Result: %s\n", tmp);
				if( (strlen(myResults)+strlen(tmp)) < 1024) {
					strcat(myResults," ");
					strcat(myResults,tmp);
				}

				cv::Mat imgRoi = rgbImg(
						cv::Rect(hn.x1, hn.y1, (hn.x2 - hn.x1),
								(hn.y2 - hn.y1)));
				if (i == 0)
					cv::imwrite(tmp, imgRoi);
			}

        }
        else if (!strcmp(argv[1], "-BatchTest")) {
        	BatchTest(argv[2], argv[3], argv[4], argv[5]);

        }
        else {
            cout << "ERROR : Test option not found ...\n";
            sprintf(myResults,"ERROR : Test option not found.");
            return myResults;
        }

    }

    LOGE("Return results length: %d",strlen(myResults));
    return myResults;
}


#ifdef STANDALONE
int main(int argc, char **argv)
{
    //ShowResults(); return 0;
    
    //if (argc != 4 && argc != 5) {
    if ( argc < 4 || argc > 6) {
        cout << "****************************** Usage **************************************** \n";
        cout << " -Train classname pos_file neg_file \n";
        cout << " -Test className image_file [ color | ../path/to/output/images ] \n";
        cout << " -BatchTest className input_files output_file output_dir\n";
        exit(0);
    }
    else {
        if (!strcmp(argv[1], "-Train")) {
            string cmd = "mkdir Models/" + (string)argv[2];
            system(cmd.c_str());
            
            string model = "Models/" + (string)argv[2] + "/" + (string)argv[2];
            
            CImageSet *posSet = new CImageSet(argv[3], argv[2], USE_FLIP);
            CImageSet *negSet = new CImageSet(argv[4], "Negative", false);
            
            int train_size_neg = min(round(NEGATIVE_SAMPLE_SIZE/2), round(negSet->size()/2));
            
            CTrainModels exemplarSVMs;
            exemplarSVMs.InitializeExemplars(posSet, 0, posSet->size());
            exemplarSVMs.Train(negSet, 0, train_size_neg);
            
            // use saved model for validation
            //exemplarSVMs.ReadModels(model.c_str());
            
            
            // validation and calibration
            exemplarSVMs.Validate(posSet, negSet, train_size_neg, 2*train_size_neg);
            exemplarSVMs.Calibrate("Occurrence");
            exemplarSVMs.Calibrate("Platt");
            exemplarSVMs.Calibrate("MR");
            
            exemplarSVMs.SaveModels(model.c_str());
            
            delete posSet;
            delete negSet;
        }
        else if (!strcmp(argv[1], "-Test")) {
            
            
            Mat segment;
            Mat rgbImg = imread(argv[3]);
            
            if (!rgbImg.data ) {
                cout <<  "Could not open " << argv[3] << std::endl ;
                return -1;
            }
            
            if (argc == 5 && argv[4][0] != '.') {
                CColorDetection color;
                segment = color.GetColorSegment(rgbImg, argv[4]);
            }
            
            
            string model = "Models/" + (string)argv[2] + "/" + (string)argv[2];
            
            CTestModels exemplarSVMs;
            exemplarSVMs.ReadModels(model.c_str());
            
	    vector<RESSTRUCT> res;
            if (argc == 5 && argv[4][0] != '.')
	        res = exemplarSVMs.Test(rgbImg, segment, DETECTION_THRESHOLD, 0);
            else
	        res = exemplarSVMs.Test(rgbImg, DETECTION_THRESHOLD, 0);
            
	    RESSTRUCT hn;
	    char tmp[256], *pos;
	    pos = strrchr(argv[3],'/');
	    for(unsigned int i=0; i< min(MAX_DETECTED_RETURNS,(int)res.size()); i++) {
	      hn = res.at(i);
	      if(argc ==5 && argv[4][0]=='.') {
		if(!pos) sprintf(tmp,"%s/%f_%s",argv[4],hn.score,argv[3]);
		else sprintf(tmp,"%s/%f_%s",argv[4],hn.score,pos+1);
	      }
	      else {
		if(!pos) sprintf(tmp,"%f_%s",hn.score,argv[3]);
		else sprintf(tmp,"%f_%s",hn.score,pos+1);
	      }
	      fprintf(stdout,"Result: %s\n",tmp);
	      cv::Mat imgRoi = rgbImg(cv::Rect(hn.x1, hn.y1, (hn.x2-hn.x1), (hn.y2-hn.y1)));
	      if(i==0) cv::imwrite(tmp, imgRoi);
	    }

        }
        else if (!strcmp(argv[1], "-BatchTest")) {
	  BatchTest(argv[2], argv[3], argv[4], argv[5]);
            
        }
        else {
            cout << "ERROR : Test option not found ...\n";
            exit(0);
        }

    }

    return EXIT_SUCCESS;
}
#endif
