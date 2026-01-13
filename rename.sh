#!/bin/bash

prefix="../001/"

for file in *; do

    newfile="${prefix}${file}"

    echo "Previous filename: $file"
    echo "New filename: $newfile"

    mv $file $newfile
done