#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "svmtrain.h"



#if MX_API_VER < 0x07030000
typedef int mwIndex;
#endif

#define CMD_LEN 2048
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

void print_null(const char *s) {}

void exit_with_help()
{
	printf(
	"Usage: model = svmtrain(training_label_vector, training_instance_matrix, 'libsvm_options');\n"
	"libsvm_options:\n"
	"-s svm_type : set type of SVM (default 0)\n"
	"	0 -- C-SVC\n"
	"	1 -- nu-SVC\n"
	"	2 -- one-class SVM\n"
	"	3 -- epsilon-SVR\n"
	"	4 -- nu-SVR\n"
	"-t kernel_type : set type of kernel function (default 2)\n"
	"	0 -- linear: u'*v\n"
	"	1 -- polynomial: (gamma*u'*v + coef0)^degree\n"
	"	2 -- radial basis function: exp(-gamma*|u-v|^2)\n"
	"	3 -- sigmoid: tanh(gamma*u'*v + coef0)\n"
	"	4 -- precomputed kernel (kernel values in training_instance_matrix)\n"
	"-d degree : set degree in kernel function (default 3)\n"
	"-g gamma : set gamma in kernel function (default 1/num_features)\n"
	"-r coef0 : set coef0 in kernel function (default 0)\n"
	"-c cost : set the parameter C of C-SVC, epsilon-SVR, and nu-SVR (default 1)\n"
	"-n nu : set the parameter nu of nu-SVC, one-class SVM, and nu-SVR (default 0.5)\n"
	"-p epsilon : set the epsilon in loss function of epsilon-SVR (default 0.1)\n"
	"-m cachesize : set cache memory size in MB (default 100)\n"
	"-e epsilon : set tolerance of termination criterion (default 0.001)\n"
	"-h shrinking : whether to use the shrinking heuristics, 0 or 1 (default 1)\n"
	"-b probability_estimates : whether to train a SVC or SVR model for probability estimates, 0 or 1 (default 0)\n"
	"-wi weight : set the parameter C of class i to weight*C, for C-SVC (default 1)\n"
	"-v n : n-fold cross validation mode\n"
	"-q : quiet mode (no outputs)\n"
	);
}

// svm arguments
struct svm_parameter param;		// set by parse_command_line
struct svm_problem prob;		// set by read_problem
struct svm_model *model;
struct svm_node *x_space;


// nrhs should be 3
int parse_command_line(char cmd[])
{
	int i, argc = 1;
	char *argv[CMD_LEN/2];
	void (*print_func)(const char *) = NULL;	// default printing to stdout

	// default values
	param.svm_type = C_SVC;
	param.kernel_type = LINEAR;
	param.degree = 3;
	param.gamma = 0;	// 1/num_features
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 100;
	param.C = 1;
	param.eps = 1e-3;
	param.p = 0.1;
	param.shrinking = 0;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;

	
    
    if((argv[argc] = strtok(cmd, " ")) != NULL)
        while((argv[++argc] = strtok(NULL, " ")) != NULL)
				;
	

	// parse options
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] != '-') break;
		++i;
		if(i>=argc && argv[i-1][1] != 'q')	// since option -q has no parameter
			return 1;
		switch(argv[i-1][1])
		{
			case 's':
				param.svm_type = atoi(argv[i]);
				break;
			case 't':
				param.kernel_type = atoi(argv[i]);
				break;
			case 'd':
				param.degree = atoi(argv[i]);
				break;
			case 'g':
				param.gamma = atof(argv[i]);
				break;
			case 'r':
				param.coef0 = atof(argv[i]);
				break;
			case 'n':
				param.nu = atof(argv[i]);
				break;
			case 'm':
				param.cache_size = atof(argv[i]);
				break;
			case 'c':
				param.C = atof(argv[i]);
				break;
			case 'e':
				param.eps = atof(argv[i]);
				break;
			case 'p':
				param.p = atof(argv[i]);
				break;
			case 'h':
				param.shrinking = atoi(argv[i]);
				break;
			case 'b':
				param.probability = atoi(argv[i]);
				break;
			case 'q':
				print_func = &print_null;
				i--;
				break;
            /*    
			case 'v':
				cross_validation = 1;
				nr_fold = atoi(argv[i]);
				if(nr_fold < 2)
				{
					printf("n-fold cross validation: n must >= 2\n");
					return 1;
				}
				break;
            */ 
			case 'w':
				++param.nr_weight;
				param.weight_label = (int *)realloc(param.weight_label,sizeof(int)*param.nr_weight);
				param.weight = (double *)realloc(param.weight,sizeof(double)*param.nr_weight);
				param.weight_label[param.nr_weight-1] = atoi(&argv[i-1][2]);
				param.weight[param.nr_weight-1] = atof(argv[i]);
				break;
			default:
				printf("Unknown option -%c\n", argv[i-1][1]);
				return 1;
		}
	}

	svm_set_print_string_function(print_func);

	return 0;
}

// read in a problem (in svmlight format)
int read_problem(int *labels, double *samples, int n, int p)
{
	int i, j, k;
	int elements, max_index, sc, label_vector_row_num;
	
	prob.x = NULL;
	prob.y = NULL;
	x_space = NULL;


	sc = n;

	elements = 0;
	// the number of instance
	prob.l = p;
	label_vector_row_num = p;

	if(label_vector_row_num!=prob.l)
	{
		printf("Length of label vector does not match # of instances.\n");
		return -1;
	}
    
	if(param.kernel_type == PRECOMPUTED)
		elements = prob.l * (sc + 1);
	else
	{
		for(i = 0; i < prob.l; i++)
		{
			for(k = 0; k < sc; k++) {
//printf("%d %d %f\n",i,k,samples[k * prob.l + i]);
                //if(samples[k * prob.l + i] != 0)
					elements++;
            }
            
			// count the '-1' element
			elements++;
		}
	}

	prob.y = Malloc(double,prob.l);
	prob.x = Malloc(struct svm_node *,prob.l);
	x_space = Malloc(struct svm_node, elements);

	max_index = sc;
	j = 0;
	for(i = 0; i < prob.l; i++)
	{
		prob.x[i] = &x_space[j];
		prob.y[i] = labels[i];

		for(k = 0; k < sc; k++)
		{
			//if(param.kernel_type == PRECOMPUTED || samples[k * prob.l + i] != 0)
			{
				x_space[j].index = k + 1;
				x_space[j].value = samples[k * prob.l + i];
				j++;
			}
		}
		x_space[j++].index = -1;
	}

	if(param.gamma == 0 && max_index > 0)
		param.gamma = 1.0/max_index;


	return 0;
}



struct svm_model* svmtrain(const char *cmd, int *labels, double *instances, int n, int p)
{
    parse_command_line((char *)cmd);
    read_problem(labels, instances, n, p);
    
    model = svm_train(&prob, &param);
    
    return model;
}

vector<double> svmGetRho()
{    
    vector<double> rho;
    for (int i=0; i < model->nr_class*(model->nr_class-1)/2; i++)
		rho.push_back(model->rho[i]);
    
    return rho;
}

vector<Mat> svmGetSVCoef()
{
    vector<Mat> svCoef;
    for(int i = 0; i < model->nr_class-1; i++) {
        Mat data = cv::Mat::zeros(model->l, 1, CV_64F);
        for(int j = 0; j < model->l; j++)
            data.at<double>(j,0) = model->sv_coef[i][j];
        svCoef.push_back(data);
    }
    
    return svCoef;
}

Mat svmGetSVs(int num_features)
{    
    double *ptr = new double [num_features*model->l];   
    
    int ir_index = 0;
    for (int i=0; i<model->l; i++) {
        int x_index=0;
        while (model->SV[i][x_index].index != -1)
        {
            ptr[ir_index] = model->SV[i][x_index].value;
            ir_index++, x_index++;
        }
    }
    Mat data = Mat(model->l, num_features, CV_64F, ptr);
    Mat M = data.clone();
    delete [] ptr;
    
    return M.t();
}




void svmfree(struct svm_model *model)
{
    svm_free_and_destroy_model(&model);
    svm_destroy_param(&param);
    free(prob.y);
    free(prob.x);
    free(x_space);
}

