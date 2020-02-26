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

function [ LOOPS, DYN_PROG, SIMILARITY ] = compute_loops( path_to_feature_folder )
%COMPUTE_LOOPS Loads holistic image features from text files and computes pairs of image indicies
%              that show the same place.
%
%   path_to_feature_folder: The folder containing the image features. It should have been
%                           filled by compute_features.m. See that file for details.
%
%   Output: Three (sparse) matrices of size num_images x num_images
%           are returned. LOOPS(i,j) specifies the likelihood of images i and
%           j showing the same place. This is the final result of the detector.
%           All non-zeros entries in LOOPS are accepted as loop closures.
%           DYN_PROG is an inermediate value and output only for the curious.
%           SIMILARITY is an inermediate value and output only for the curious.
%
%   Example: compute_loops( '~/storage/sequences/threefold/features' )                           
%

  file_list = dir(path_to_feature_folder);

  % loop over data directory
  for i = 1:length(file_list)

    f = file_list(i);

    if (f.isdir())
      continue;
    end

    % read file from disk
    file_name = [path_to_feature_folder, '/',  f.name];
    try
      feature = uint8(importdata(file_name));
    catch exception
      display(['Couldnt read file ', file_name ]);
      continue;
    end

    if (~exist('all_featuers'))
      all_featuers = uint8(zeros(length(feature), length(file_list)));
      num_features = 0;
    end

    num_features = num_features + 1;
    all_featuers(:,num_features) = feature;

    display(['Loading file', file_name]);

  end

  % trim to correct size
  all_featuers = all_featuers(:,1:num_features);
  [loops, dyn_prog, similarity] = placeRecognizerMex( all_featuers );

  LOOPS = toSparse(loops, num_features);
  DYN_PROG = toSparse(dyn_prog, num_features);
  SIMILARITY = toSparse(similarity, num_features);


end

