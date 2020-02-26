dbclear all;

% compile matlab wrappers
disp('Building wrappers ...');
mex dirdMex.cpp ../src/cDird.cpp CXXFLAGS="\$CXXFLAGS -O3 -msse3";
mex placeRecognizerMex.cpp ../src/cDird.cpp ../src/cPlaceRecognizer.cpp CXXFLAGS="\$CXXFLAGS -O3 -msse3";
disp('...done!');
