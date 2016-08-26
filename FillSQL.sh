#!/bin/bash


echo "Printing all FULL wav files to sqlite file $1"

SONGID=1
for f in TestSet/*FULL.wav
do
    echo "Fingerprinting $f with songId $SONGID"
    ./Fingerprinter $f $SONGID > "${f%.wav}ID.csv"
    ((SONGID++))
done

for f in TestSet/*ID.csv
do
    echo "Inserting $f into sqlite database at $1"
    echo ".mode csv
.import $f fingerprints" | sqlite3 $1
done
