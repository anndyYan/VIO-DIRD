*** DIRD - Dird is an Illumination Robust Descriptor ***
http://www.mrt.kit.edu/libDird.php

This is an open-source library for place recognition and loop closure
detection (as used in visual SLAM). It is based on the DIRD descriptor.
The library is written in C++ and includes MATALB wrappers. 
Datasets can be downloaded from the DIRD website (see above).


*** Paper Describing the Approach ***
How to Learn an Illumination Robust Image Feature for Place Recognition (submitted) 
Henning Lategahn and Johannes Beck and Bernd Kitt and Christoph Stiller 
IEEE Intelligent Vehicles Symposium (IV) 2013


*** Datasets  ***
Two ready-to-use data sets can be downloaded from the website.
They are named "threefold" and "twofold" and contain a readme each.
More data sets can be downloaded from the KITTI website (see DIRD website
for the link).


*** Installation ***
Assuming a linux box. Type into the terminal:
- sudo apt-get install libfreeimage-dev imagemagick cmake
- mkdir build
- cd build
- cmake ..
- make
- follow instructions "Running the Place Recognizer ... " below

Assuming MATLAB:
- start up matlab
- goto the matlab folder of this library
- run make.m
- run the demo runDemo.m (see that file for arguements)

Assuming a Windows box:
- run cmake-gui
- select libDird folder and build folder (do not use the same folders)
- configure and generate (change install path if needed)
- open created solution and build (debug or release)
- build project install
- now the compiled executables are now in the install folder
- follow instructions "Running the Place Recognizer ... " below


*** Running the Place Recognizer on the Test Sets ***
From the build / install directory:
1: ./compute_features path/to/threefold/image_0 path/to/threefold/features
2: ./compute_loops path/to/threefold/features path/to/threefold/matrices 2000
3: ./create_debug_output path/to/threefold/matrices/step3_loops.txt path/to/threefold/image_0 path/to/threefold/debug

Step 1 computes the DIRD features from the input sequence and stores them in
the specified destination folder (.../features). One text file is created for
every input image. Features are stored in human readable ASCII format.

Step 2 reads these DIRD features, computes pairwise similarities between
all images and computes loop closure hypothesis from them. Output is dumped
into text files (in .../matrices). Text files represent sparse (squared)
matrices of size num_images X num_images. The text files have the following format:
  
  number_of_rows_which_is_number_of_columns_of_matrix                        
  row_index_1 column_index_1 place_equality_measure_1                    
  row_index_2 column_index_2 place_equality_measure_2                    
  row_index_3 column_index_3 place_equality_measure_3                      
  row_index_4 column_index_4 place_equality_measure_4

Three matrices are dumped ("similarity","dyn_prog","loops"). Only the loops 
matrix stores detected loop closures. The others are mere intermediate values.
For easy inspection these matrices are also stored as images. Looking at 
step3_loops.png in the matrix folder will show all detected loops consicely.

Step 3 reads the loop closure matrix and loads the two associated input
images for every detected loop closure. These images are (down sampled a bit 
and) concatinated and saved to the specified output folder (.../debug). 
Thereafter the debug folder contains images of all detected loops. This step is
a mere convinience step and not nesseccary.


*** Running the Place Recognizer on KITTI (only Linux) ***
The KITTI data set is not designed for benchmarking loop closures. Nevertheless
it contains many high quality datasets some of which do contain loopy traversals.
First the dataset for visual odometry (the download link is on the DIRD website)
needs to be downloaded and extracted. Then the script "run_on_all_kitti_sequences.sh"
in the scripts folder runs the loop closure detection on all sequences of the
dataset. To launch run from the scripts folder:
./run_on_all_kitti_sequences.sh path/to/kitti/dataset/sequences

Thereafter the folder path/to/kitti/dataset/sequences will contain all loop
closure matrix images of all sequences. The sequences will contain a debug
folder (see above) for a more detailed inspection of the result.


*** Using your Own Datasets ***
The datasets need to have the following format:
  <image_sequence>                                                               
        ├── 000000.png                                                             
        ├── 000001.png                                                             
        ├── 000002.png                                                             
        ├── 000003.png                                                             
        ├── 000004.png                                                             
        ├── 000005.png                                                             
        └── ........
To convert a sequence of gray-scale images into the required format one may
use the script rename_images.sh (only Linux) in the scripts folder.


*** Contact Information ***
Henning Lategahn <henning.lategahn@kit.edu>
Johannes Beck <johannes.beck@mrt.uka.de>
Bernd Kitt <bernd.kitt@kit.edu>
