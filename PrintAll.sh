#!/bin/bash

rm -f TestSet/*{1,2,3}.csv

for f in TestSet/*{1,2,3}.wav
do
    echo "Fingerprinting $f"
    ./FingerPrinter $f > "${f%.wav}.csv"
done
