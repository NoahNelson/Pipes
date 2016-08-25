#!/bin/bash

FILESTEMS="TestSet/futurefreaksmeout TestSet/angels TestSet/forriver"

echo "Master    Recording   matches"
for f in TestSet/*{1,2,3}.csv
do
    for g in TestSet/*FULL.csv
    do
        python PrintMatcher.py $f $g
    done
done
