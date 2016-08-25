#!/bin/bash

rm -f TestSet/*.csv

for f in TestSet/*.wav
do
    echo "Fingerprinting $f"
    ./FingerPrinter $f > "${f%.wav}.csv"
done
