1) Type "make" to compile the code


2) To train the ExemplarSVMs (stage 1)

>> ./ExemplarSVMs -Train Butterfly positive_training_files negative_training_files
>> ./ExemplarSVMs -Train Squirrel positive_training_files negative_training_files
where the positive_training_files and negative_training_files consists of a set of filenames 
e.g. 
./ExemplarSVMs -Train Squirrel sq_pos_training.txt sq_neg_training.txt

## train all other exemplarSVMs
.
.
.
 

3) To Test the exemplar
>> ./ExemplarSVM -Test Squirrel test_image
e.g. 
>> ./ExemplarSVMs -Test Squirrel Test/squirrel_test.jpg


4) To perform a batch test
>> ./ExemplarSVM -Test Squirrel test_files output-file
e.g.
>> ./ExemplarSVM -Test Squirrel sq_testing.txt output.txt
where sq_testing.txt consists of a set of testing filenames


5) To train color models in MATLAB
>> run matlab
>> Train('red_train.txt','red_model.mat','red',10);
>> SaveModelToText('red_model.mat','red_model.txt');
## train all other colors
.
.
.
## move *_model.txt to Models/Color/


6) To test a specific ExemplarSVMs with a specific color
>> ./ExemplarSVMs -Test Test/butterfly_test3.jpg Butterfly Green


#################
To change the calibration method {MR, PLATT, CO_OCCURRENCE}, one must the variable gCalibrationMethod in utils/define.h

To change the detection threshold, one must change DETECTION_THRESHOLD in utils/define.h

   


