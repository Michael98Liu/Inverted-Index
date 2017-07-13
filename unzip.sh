#!/bin/bash
FILES=./gzfiles/*
for f in $FILES
do
    echo "Processing $f file..."
    gunzip f
done
