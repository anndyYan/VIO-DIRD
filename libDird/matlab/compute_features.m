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

function [ ] = compute_features( path_to_image_sequence, path_to_feature_folder )
%COMPUTE_FEATURES Traverses an image folder and computes DIRD features for every image.
%                 Image features are saved to disk.
% 
%   path_to_image_sequence: The folder containing the images. Images need
%                           to be gray scale images. The folder must not contain 
%                           any other files except image files.
%
%   path_to_feature_folder: An initially empty folder. For every image of 
%                           the image folder one text file with the image feature
%                           will be stored for further proecessing. 
%                           Files are ASCII files.
% 
%   Example: compute_features( '~/storage/sequences/threefold/image_0', '~/storage/sequences/threefold/features' )
%   

  
  % IMPORTANT: if you change the values below then the parameters
  % of the logistic function in cPlaceRecognizer.cpp need to be adjusted!

  % number of pixels for one tile
  tile_size = 48;
  % number of tiles horizontally
  num_tiles_hor = 4;
  % number of tiles vertically
  num_tiles_ver = 4;
  % dimension of down-sampled image
  width_down = tile_size * num_tiles_hor;
  height_down = tile_size * num_tiles_ver;

  file_list = dir(path_to_image_sequence);
  % loop over data directory
  for i = 1:length(file_list)

    f = file_list(i);

    if (f.isdir())
      continue;
    end

    % read file from disk
    file_name = [path_to_image_sequence, '/',  f.name];
    try
      img = imread(file_name);
    catch exception
      display(['Couldnt read file ', file_name ]);
      continue;
    end

    display(['Processing file', file_name]);

    % downsample (CAVEAT: Note the transpose img' !)
    img_down = imresize(img', [width_down, height_down], 'nearest') ;

    % compute DIRD features
    dirdMex( 'process', img_down );
    
    % loop over tiling and extract feature
    % for the middle pixel position of each tile
    dird_feature = [];
    for x = 0:num_tiles_hor-1
      for y = 0:num_tiles_ver-1
        u = x * tile_size + tile_size/2;
        v = y * tile_size + tile_size/2;
        dird_feature = [dird_feature, dirdMex('get', u, v )']; 
      end
    end

    % write feature to text file 
    base_name = f.name(1:end-4);
    file_name_out = [ path_to_feature_folder, '/', base_name, '.txt'];
    dlmwrite(file_name_out, dird_feature, 'delimiter', ' ', 'precision', '%d');
       
  end




end

