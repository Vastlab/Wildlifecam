clc, clear all
imtool close all

%run the crops thru ExemplarSVM

myStr = (['/home/brian/Desktop/yuiExemplar/ExemplarSVMs/Bushnell_Crop_Out/cam1/EK005649.JPG'])

myCommand = system('./ExemplarSVMs -Test Squirrel /home/brian/Desktop/yuiExemplar/ExemplarSVMs/Bushnell_Crop_Out/cam1/EK005649.JPG');