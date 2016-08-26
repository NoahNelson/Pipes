#!/bin/bash

#echo "Master    Recording   matches"
#for f in TestSet/*{1,2,3}.csv
#do
    #for g in TestSet/*FULL.csv
    #do
        #python3 PrintMatcher.py $f $g
    #done
#done

echo "Testing all snippet csv files against database $1"
for f in TestSet/*{1,2,3}.csv
do
    python3 PrintMatcher.py $f $1
done
