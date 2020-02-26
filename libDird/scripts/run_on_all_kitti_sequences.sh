#!/bin/bash

# This script runs the place recognizer on all KITTI 
# visual odometry sequences. 
# The entire bunch of sequences can be downloaded on the KITTI 
# website: http://www.cvlibs.net/download.php?file=data_odometry_gray.zip

if [ -z "$1" ]
then
  echo "Usage: $0 <EXTRACTED_KITTI_FOLDER>"
  echo "Example: $0 ~/data/kitti/dataset/sequences"
  exit 1
fi

kitti=$1
sequences=$(ls -1 $kitti | grep -v png)

# compute loops first
for seq in $sequences
do
  echo Computing loop closures for sequence $seq
  dir=$kitti/$seq
  img_dir=$dir/image_0
  features_dir=$dir/features
  matrices_dir=$dir/matrices
  mkdir $features_dir
  mkdir $matrices_dir
  ../build/compute_features $img_dir $features_dir
  ../build/compute_loops $features_dir $matrices_dir
  cp $matrices_dir/step3_loops.png $kitti/loops_of_$seq.png
done

# create debug afterwards
for seq in $sequences
do
  echo Creating debug output for sequence $seq
  dir=$kitti/$seq
  img_dir=$dir/image_0
  matrices_dir=$dir/matrices
  debug_dir=$dir/debug
  mkdir $debug_dir
  ../build/create_debug_output $matrices_dir/step3_loops.txt $img_dir $debug_dir
done
