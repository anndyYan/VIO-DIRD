# project
cmake_minimum_required (VERSION 2.6)
project (libDird)

# directories
set (LIBDIRD_SRC_DIR src)

# include directory
include_directories("${LIBDIRD_SRC_DIR}")
IF(MSVC)
	include_directories(win32)
	SET(FreeImageLib "${CMAKE_CURRENT_SOURCE_DIR}/win32/FreeImage.lib")
ELSE(MSVC)
	SET(FreeImageLib freeimage)
ENDIF(MSVC)

IF(MSVC)
  ADD_DEFINITIONS(/arch:SSE2)
ELSE(MSVC)
  SET(CMAKE_CXX_FLAGS "-O3 -msse3")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ENDIF(MSVC)

# sources
SET(DIRD_SRC_FILES 
  "src/compute_features.cpp"
  "src/cDird.cpp"
  "src/cImage.cpp"
  )

# sources
SET(DEBUG_SRC_FILES 
  "src/create_debug_output.cpp"
  "src/cPlaceRecognizer.cpp"
  "src/cImage.cpp"
  )

# sources
SET(LOOP_SRC_FILES 
  "src/compute_loops.cpp"
  "src/cDird.cpp"
  "src/cPlaceRecognizer.cpp"
  "src/cImage.cpp"
  )

# make release version
set(CMAKE_BUILD_TYPE Release)

# demo programs
add_executable(compute_features ${DIRD_SRC_FILES})
add_executable(compute_loops ${LOOP_SRC_FILES})
add_executable(create_debug_output ${DEBUG_SRC_FILES})
target_link_libraries(compute_features ${FreeImageLib})
target_link_libraries(compute_loops ${FreeImageLib})
target_link_libraries(create_debug_output ${FreeImageLib})

IF(MSVC)
	if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	  set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install" CACHE PATH
		"Install path prefix, prepended onto install directories." FORCE)
	endif() 

	INSTALL(TARGETS "compute_features" "compute_loops" "create_debug_output" RUNTIME DESTINATION debug CONFIGURATIONS Debug)
	INSTALL(TARGETS "compute_features" "compute_loops" "create_debug_output" RUNTIME DESTINATION release CONFIGURATIONS Release)
	
	INSTALL(FILES "./win32/FreeImage.dll" DESTINATION debug CONFIGURATIONS Debug)
	INSTALL(FILES "./win32/FreeImage.dll" DESTINATION release CONFIGURATIONS Release)
ENDIF(MSVC)
