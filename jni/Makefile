#OS=OSX
OS=LINUX

SHELL=/bin/sh
OBJS = squirrelPipeline
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)
INCLUDE = include

ifeq ($(OS), OSX)
	CVINCLUDE = /opt/local/include/opencv2/nonfree
	CV2INCLUDE = /opt/local/include/opencv
	CV2INCLUDE2 = /opt/locao/include/opencv2
	LOCALLIB = /opt/local/lib
	LOCALINCLUDE = /opt/local/include
else 
	CVINCLUDE = /usr/local/include/opencv2/nonfree
	CV2INCLUDE = /usr/local/include/opencv
	CV2INCLUDE2 = /usr/local/include/opencv2
	LOCALLIB = /usr/local/lib
	LOCALINCLUDE = /usr/local/include
endif

LIBS = -lopencv_core -lopencv_highgui  -lopencv_objdetect -lopencv_imgproc -lopencv_features2d -lopencv_nonfree -lm -lopencv_ml -lpthread 

SRCFILE = 	Squirrel_pipeline.cpp \
			ExemplarSVMs/main.cpp \
			ExemplarSVMs/utils/ImageSet.cpp \
			ExemplarSVMs/utils/ColorDetection.cpp \
			ExemplarSVMs/internal/TrainModels.cpp \
			ExemplarSVMs/internal/TestModels.cpp \
			ExemplarSVMs/internal/Detect.cpp \
			ExemplarSVMs/internal/NMS.cpp \
			ExemplarSVMs/internal/PiCoDes.cpp \
			ExemplarSVMs/internal/Pyramid.cpp \
			ExemplarSVMs/libsvm/svmtrain.cpp \
			ExemplarSVMs/libsvm/svm.cpp \
			ExemplarSVMs/libMR/MetaRecognition.cpp \
			ExemplarSVMs/libMR/weibull.cpp 
			
TARGET =  squirrelPipeline

#Exemplars:
#	@echo "***** NOTE *****"
#	@echo "You might have to modify the MakeFile in ExemplarSVMs to adjust to Linux vs OSX"
#	@echo "***** END NOTE ******"
#	cd ExemplarSVMs; make; cd ..;

$(TARGET):	$(SRCFILE)
	$(CXX) $(LFLAGS) $(SRCFILE) -I$(INCLUDE) -I$(CVINCLUDE) -I$(CV2INCLUDE) -I$(CV2INCLUDE2) -L$(LOCALLIB) -I$(LOCALINCLUDE) $(LIBS) -o $(OBJS)

all: $(TARGET) #Exemplars



clean:
	rm -f $(OBJS)

