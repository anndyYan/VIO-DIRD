% Copyright 2012. All rights reserved.
% Institute of Measurement and Control Systems
% Karlsruhe Institute of Technology, Germany
% 
% This file is part of libDird.
% Authors: Henning Lategahn
% Website: http://www.mrt.kit.edu/libDird.php
% 
% libDird is free software; you can redistribute it and/or modify it under the
% terms of the GNU General Public License as published by the Free Software
% Foundation; either version 3 of the License, or any later version.
% 
% libDird is distributed in the hope that it will be useful, but WITHOUT ANY
% WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
% PARTICULAR PURPOSE. See the GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License along with
% libDird; if not, write to the Free Software Foundation, Inc., 51 Franklin
% Street, Fifth Floor, Boston, MA 02110-1301, USA 


function [ output_args ] = runDemo( path_to_folder )
%RUNDEMO runs the loop closure detection on the sample data of 
%        the website.
%
%   path_to_folder: path to either "threefold" or "twofold"
%                   these datasets can be downloaded from the DIRD
%                   website: 
%                   http://www.mrt.kit.edu/libDird.php
%
%   example: runDemo('~/storage/sequences/threefold')
%

  path_to_images = [path_to_folder, '/image_0'];
  path_to_features = [path_to_folder, '/features'];

  % extract features first
  compute_features( path_to_images, path_to_features );

  % load features again and detect loops.
  % Three (sparse) matrices of size num_images x num_images
  % are returned. LOOPS(i,j) specifies the likelihood of images i and
  % j showing the same place. This is the final result of the detector.
  % All non-zeros entries in LOOPS are accepted as loop closures.
  % DYN_PROG is an inermediate value and output only for the curious.
  % SIMILARITY is an inermediate value and output only for the curious.
  [ LOOPS, DYN_PROG, SIMILARITY ] = compute_loops( path_to_features );
  
  % One may want to view these matrices. However due to their size 
  % they are downsampled here to something managable.
  down_size = 3000;
  loops = downSizeMatrix( LOOPS, down_size );
  dyn_prog = downSizeMatrix( DYN_PROG, down_size );
  similarity = downSizeMatrix( SIMILARITY, down_size );

  figure, imagesc( loops )
  title('All detected loops');
  colormap( hot );

  figure, imagesc( dyn_prog )
  title('Some intermediate value (after dynamic programming)');
  colormap( hot );

  figure, imagesc( similarity )
  colormap( hot );
  title('Pairwise similarity. First step of processing chain.')
  
  display('Similarity matrices are shown. Press any key to view images showing the same place.')
  pause
  figure


  % Here we loop over all detected loops and show the image pairs
  % This may take a long time. 
  index_to_filename = getFileNames( path_to_images );
  [i,j,v] = find(LOOPS);

  for k = 1:100:length(i)

    assert( i(k) <= length(index_to_filename), 'Something went wrong with mapping image indicies to filenames');
    assert( j(k) <= length(index_to_filename), 'Something went wrong with mapping image indicies to filenames');

    file_name_1 = index_to_filename{ i(k) };
    file_name_2 = index_to_filename{ j(k) };

    i1 = imread( file_name_1 );
    i2 = imread( file_name_2 );

    image = [i1;i2];
    imshow(image);

    display('Loop Closure detected for images');
    disp( file_name_1 );
    disp( file_name_2 );
    display(['This is loop closure number ', num2str(k), ' of ', num2str(length(i)), '.']);
    display( ' ' );

    pause

  end


end

