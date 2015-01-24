1) Type "make -f Makefile2" to compile the code


2) To train the Picodes (stage 2)

>> ./RunPicodes -Train training_files
where the training_files consists of a set of className, positive_training_files, and Model_prefix 

e.g. 
./RunPicodes -Train training_2.txt

 
3) To Test the Picodes
>> ./RunPicodes -Test testing_files output
where the testing_files consists of a set of className, testing files, and Model_prefix; the output gives class_labels, detected_labels, and scores

e.g. 
>> ./RunPicodes -Test testing_files.txt output.txt


#################
To change the calibration method {MR, PLATT}, one must the variable gCalibrationMethod in utils/define.h
