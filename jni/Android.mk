LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=off
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=STATIC

#OPENCV_MK_PATH:=${NDKROOT}/../OpenCV-2.4.3.2-android-sdk-tadp/sdk/native/jni/OpenCV-tegra3.mk
#include includeOpenCV.mk

include ${NDKROOT}/../OpenCV-2.4.8.2-Tegra-sdk/sdk/native/jni/OpenCV-tegra3.mk

LOCAL_MODULE	:=nativePipeline

LOCAL_CFLAGS := $(LOCAL_C_INCLUDES:%=-I%)
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -ldl -llog -landroid \
                -L$(TARGET_OUT) -ljnigraphics

LOCAL_SRC_FILES := \
	cvjni.cpp \
	Squirrel_pipeline.cpp \
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

include $(BUILD_SHARED_LIBRARY)

