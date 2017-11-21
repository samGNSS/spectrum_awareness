cmake_minimum_required(VERSION 2.6)
project(plot_spectrum)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O4 -std=c++11 -Wall -pg")
file(GLOB_RECURSE sources ../src/*.cpp ../src/*.c ../src/*.h)

add_executable(test_fft main_fft_test.cpp ${sources})
add_executable(test_cfar main_cfar_unittest.cpp ${sources})



set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
MESSAGE( STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})

#add_definitions(-DFILE_DUMP)

###############################################################################
## dependencies ###############################################################
###############################################################################

#########################################################
#Volk
#########################################################
find_package(Volk REQUIRED)

#########################################################
#fftw
#########################################################
FIND_PATH(
    FFTW3F_INCLUDE_DIRS
    NAMES fftw3.h
    HINTS $ENV{FFTW3_DIR}/include
        ${PC_FFTW3F_INCLUDE_DIR}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    FFTW3F_LIBRARIES
    NAMES fftw3f libfftw3f
    HINTS $ENV{FFTW3_DIR}/lib
        ${PC_FFTW3F_LIBDIR}
    PATHS /usr/local/lib
          /usr/lib
          /usr/lib64
)

FIND_LIBRARY(
    FFTW3F_THREADS_LIBRARIES
    NAMES fftw3f_threads libfftw3f_threads
    HINTS $ENV{FFTW3_DIR}/lib
        ${PC_FFTW3F_LIBDIR}
    PATHS /usr/local/lib
          /usr/lib
          /usr/lib64
)

#########################################################
## Target the libraries
#########################################################

 target_link_libraries(test_fft PUBLIC
  ${Boost_LIBRARIES}
  ${VOLK_LIBRARIES}
  ${LIBUSB_LIBRARIES}
  ${FFTW3F_LIBRARIES}
  -lpthread
  -lsqlite3
 )

 target_link_libraries(test_cfar PUBLIC
  ${Boost_LIBRARIES}
  ${VOLK_LIBRARIES}
  ${LIBUSB_LIBRARIES}
  ${FFTW3F_LIBRARIES}
  -lpthread
  -lsqlite3
 )
