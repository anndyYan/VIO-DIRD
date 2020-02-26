function [ matrix_out ] = downSizeMatrix( sparse_matrix, new_size )
%DOWNSIZEMATRIX Summary of this function goes here
%   Detailed explanation goes here

  old_size = size(sparse_matrix,1);
  scale = new_size / old_size;
  matrix_out = sparse( new_size, new_size );

  [i,j,v] = find(sparse_matrix);

  for k = 1:length(i)
    ii = ceil( i(k) * scale );
    jj = ceil( j(k) * scale );
    matrix_out(ii, jj) = v(k);
  end


end

