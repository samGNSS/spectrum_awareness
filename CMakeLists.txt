cmake_minimum_required(VERSION 2.6)
project(plot_spectrum)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O4 -std=c++11 -Wall -pg")
file(GLOB_RECURSE sources ./src/*.cpp ./src/*.c ./src/*.h)
 
add_executable(plot_spectrum main_plot_spectrum.cpp ${sources})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
MESSAGE( STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})

###############################################################################
## dependencies ###############################################################
###############################################################################

#########################################################
#Boost: program options (still the best)   
######################################################### 
find_package(Boost 1.5.4 COMPONENTS program_options REQUIRED)

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
#libusb bullshit    
#########################################################
IF (NOT WIN32)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  find_package(PkgConfig)
  pkg_check_modules(PC_LIBUSB libusb-1.0)
ENDIF(NOT WIN32)

set(LIBUSB_LIBRARY_NAME usb-1.0)
IF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  set(LIBUSB_LIBRARY_NAME usb)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

FIND_PATH(LIBUSB_INCLUDE_DIR libusb.h
  PATHS ${PC_LIBUSB_INCLUDEDIR} ${PC_LIBUSB_INCLUDE_DIRS})

FIND_LIBRARY(LIBUSB_LIBRARIES NAMES ${LIBUSB_LIBRARY_NAME}
  PATHS ${PC_LIBUSB_LIBDIR} ${PC_LIBUSB_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBUSB DEFAULT_MSG LIBUSB_LIBRARIES LIBUSB_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBUSB_INCLUDE_DIR LIBUSB_LIBRARIES)

#########################################################
#qwt    
#########################################################

# - try to find Qwt libraries and include files
# QWT_INCLUDE_DIR where to find qwt_global.h, etc.
# QWT_LIBRARIES libraries to link against
# QWT_FOUND If false, do not try to use Qwt
# qwt_global.h holds a string with the QWT version;
#   test to make sure it's at least 5.2

find_path(QWT_INCLUDE_DIRS
  NAMES qwt_global.h
  HINTS
  ${CMAKE_INSTALL_PREFIX}/include/qwt
  ${CMAKE_PREFIX_PATH}/include/qwt
  PATHS
  /usr/local/include/qwt-qt4
  /usr/local/include/qwt
  /usr/include/qwt6
  /usr/include/qwt-qt4
  /usr/include/qwt
  /usr/include/qwt5
  /opt/local/include/qwt
  /sw/include/qwt
  /usr/local/lib/qwt.framework/Headers
)

find_library (QWT_LIBRARIES
  NAMES qwt6 qwt6-qt4 qwt qwt-qt4 qwt5 qwtd5
  HINTS
  ${CMAKE_INSTALL_PREFIX}/lib
  ${CMAKE_INSTALL_PREFIX}/lib64
  ${CMAKE_PREFIX_PATH}/lib 
  PATHS
  /usr/local/lib
  /usr/lib
  /opt/local/lib
  /sw/lib
  /usr/local/lib/qwt.framework 
)

set(QWT_FOUND FALSE)
if(QWT_INCLUDE_DIRS)
  file(STRINGS "${QWT_INCLUDE_DIRS}/qwt_global.h"
    QWT_STRING_VERSION REGEX "QWT_VERSION_STR")
  set(QWT_WRONG_VERSION True)
  set(QWT_VERSION "No Version")
  string(REGEX MATCH "[0-9]+.[0-9]+.[0-9]+" QWT_VERSION ${QWT_STRING_VERSION})
  string(COMPARE LESS ${QWT_VERSION} "5.2.0" QWT_WRONG_VERSION)
  string(COMPARE GREATER ${QWT_VERSION} "6.2.0" QWT_WRONG_VERSION)

  message(STATUS "QWT Version: ${QWT_VERSION}")
  if(NOT QWT_WRONG_VERSION)
    set(QWT_FOUND TRUE)
  else(NOT QWT_WRONG_VERSION)
    message(STATUS "QWT Version must be >= 5.2 and <= 6.2.0, Found ${QWT_VERSION}")
  endif(NOT QWT_WRONG_VERSION)

endif(QWT_INCLUDE_DIRS)

if(QWT_FOUND)
  # handle the QUIETLY and REQUIRED arguments and set QWT_FOUND to TRUE if
  # all listed variables are TRUE
  include ( FindPackageHandleStandardArgs )
  find_package_handle_standard_args( Qwt DEFAULT_MSG QWT_LIBRARIES QWT_INCLUDE_DIRS )
  MARK_AS_ADVANCED(QWT_LIBRARIES QWT_INCLUDE_DIRS)
endif(QWT_FOUND)
 
#########################################################
#qt4    
#########################################################

find_package(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

#########################################################
## Target the libraries
#########################################################
target_link_libraries(plot_spectrum PUBLIC
  ${Boost_LIBRARIES}
  ${VOLK_LIBRARIES}
  ${LIBUSB_LIBRARIES}
  ${FFTW3F_LIBRARIES}
  ${QWT_LIBRARIES}
  ${QT_LIBRARIES}
  -lpthread
 ) 
 
#  target_link_libraries(filter_timing_test PUBLIC
#   ${Boost_LIBRARIES}
#   ${VOLK_LIBRARIES}
#   ${LIBUSB_LIBRARIES}
#   ${FFTW3F_LIBRARIES}
#   -lpthread
#   -lpython2.7
#  ) 
