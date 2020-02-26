function [ index ] = getFileNames( path_to_image_sequence )
%GETFILENAME Summary of this function goes here
%   Detailed explanation goes here

  file_list = dir(path_to_image_sequence);
  
  % loop over data directory
  counter = 1;
  for i = 1:length(file_list)

    f = file_list(i);

    if (f.isdir())
      continue;
    end

    index{counter} = [path_to_image_sequence, '/',  f.name];
    counter = counter + 1;

  end

end

