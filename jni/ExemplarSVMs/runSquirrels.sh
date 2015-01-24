#!/bin/bash

rm -rf "*Output.txt"

counter=1
for myFile in /home/brian/Desktop/yuiExemplar/ExemplarSVMs/camFiles/*
do
    while read line
    do
	echo "$line"
	./ExemplarSVMs -Test Squirrel17 $line | tee tempOutput.txt
	while read new_line
	do
	    echo "$new_line $line" >> "cam"$counter"Output.txt"
	done < "tempOutput.txt"
    done < $myFile

counter=$((counter+1))

if [ $counter -eq "8" ]; then
    counter=9
fi

done

matlab -nodisplay -r "try, scoreExemplars, catch, end, quit"
