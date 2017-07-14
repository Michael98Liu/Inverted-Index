#!/bin/bash
FILES=./gzfiles/*
for f in $FILES
do
    STEM=$(basename "${f}" .gz)
    echo "Processing $f file..."
    gunzip -c $f > ./input_files/"${STEM}"
done
