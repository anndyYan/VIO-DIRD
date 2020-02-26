function SPARSE=toSparse( vectors, size )

  SPARSE = sparse(size, size);
  for l=vectors
    if (l(3) > 0.00000001)
      % vectors is zero based index!
      SPARSE(l(1)+1,l(2)+1) = l(3);
    end
  end

end