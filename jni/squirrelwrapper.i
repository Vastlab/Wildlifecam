/* File : squirrelwrapper.i */
%module squirrelwrapper

%{
#include "squirrelwrapper.h"
%}

/* Let's just grab the original header file here */
%include "Squirrel_pipeline.h"