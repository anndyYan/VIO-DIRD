#!/bin/bash

# This script will convert all images in a folder into the 
# required format (png) and naming scheme.
# Input images need to be gray scale.

if [ -z "$1" ]
then
  echo "Usage: $0 <IMAGE_FOLDER>"
  exit 1
fi

dir=$1
files=$(ls -1 $dir)
count=0

for file in $files
do
  file_in=$dir/$file
  file_out=$dir/$(printf "%06d\n" $count).png
  echo Processing file $file_in
  #mv $file_in $file_out
  convert $file_in $file_out
  count=`expr $count + 1`
done
