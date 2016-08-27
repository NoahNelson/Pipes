#!/bin/bash


echo "Printing all FULL wav files to sqlite file $1"

SONGID=1
for f in TestSet/*FULL.wav
do
    echo "Fingerprinting $f with songId $SONGID"
    ./Fingerprinter $f -s $SONGID > "${f%.wav}ID.csv"
    ((SONGID++))
done

for f in TestSet/*ID.csv
do
    echo "Inserting $f into sqlite database at $1"
    python3 PrintToSQL.py $1 $f
done
