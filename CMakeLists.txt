# Copyright 2008 28msec, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
PROJECT(libaws)

# ctest support
ENABLE_TESTING()
INCLUDE(CTest)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)


# prohibit in-source-builds
IF (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    MESSAGE(STATUS "In-source-builds are not allowed")
    MESSAGE(STATUS "Clean your source directory (e.g. delete the CMakeCache.txt file)")
    MESSAGE(FATAL_ERROR "Please create a separate build directory and call CMake again")
ENDIF (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

# set the default build type to debug
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

# Look for required libraries
SET(requiredlibs)

FIND_PACKAGE(CURL)
IF(CURL_FOUND)
  INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
  SET(requiredlibs ${requiredlibs} ${CURL_LIBRARIES} )
ELSE(CURL_FOUND)
  MESSAGE(FATAL_ERROR "Could not find the CURL library and development files.")
ENDIF(CURL_FOUND)

FIND_PACKAGE(OpenSSL)
IF(OPENSSL_FOUND)
  INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
  SET(requiredlibs ${requiredlibs} ${OPENSSL_LIBRARIES} crypto) # bugfix, because libcrypto is not found automatically
ELSE(OPENSSL_FOUND)
  MESSAGE(FATAL_ERROR "Could not find the OpenSSL library and development files.")
ENDIF(OPENSSL_FOUND)
  
FIND_PACKAGE(LibXml2)
IF(LIBXML2_FOUND)
    INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
    SET(requiredlibs ${requiredlibs} ${LIBXML2_LIBRARIES}) 
    SET(CXXFLAGS ${CXXFLAGS} ${LIBXML2_DEFINITONS}) # needed?
ELSE(LIBXML2_FOUND)
  MESSAGE(FATAL_ERROR "Could not find the libxml2 library and development files.")
ENDIF(LIBXML2_FOUND)

INCLUDE (CheckIncludeFiles)
SET(CMAKE_REQUIRED_LIBRARIES pthread)
CHECK_INCLUDE_FILES(pthread.h LIBAWS_HAVE_PTHREAD_H)

if(NOT LIBAWS_HAVE_PTHREAD_H)
  MESSAGE(FATAL_ERROR "Could not find pthread development headers.")
ELSEIF(NOT LIBAWS_HAVE_PTHREAD_H)
    SET(requiredlibs ${requiredlibs} pthread) 
ENDIF(NOT LIBAWS_HAVE_PTHREAD_H)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

INCLUDE(CMakeConfiguration.txt)
INCLUDE(CMakeCompiler.txt)

SET(LIBAWS_MAJOR_VERSION "0")
SET(LIBAWS_MINOR_VERSION "9")
SET(LIBAWS_PATCH_VERSION "2")

# set the include path to the API headers
# note that there also some generated headers files in the build directory
INCLUDE_DIRECTORIES(AFTER ${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(AFTER ${CMAKE_CURRENT_BINARY_DIR}/include)

FIND_PACKAGE(Fuse)
IF(FUSE_FOUND)
  MESSAGE(STATUS "Found Fuse. Building the s3fs module.")
  INCLUDE_DIRECTORIES(${FUSE_INCLUDE_DIR})
ELSE(FUSE_FOUND)
  MESSAGE(STATUS "Fuse not found. Can't build the s3fs module.")
ENDIF(FUSE_FOUND)


# Tell CMake to process the sub-directories
ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(bin)
IF(FUSE_FOUND)
  ADD_SUBDIRECTORY(fuse)
ENDIF(FUSE_FOUND)
ADD_SUBDIRECTORY(doc)

# add the uninstall support
CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeUninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake"
    @ONLY)
MESSAGE(STATUS "configured ${CMAKE_CURRENT_SOURCE_DIR}/CMakeUninstall.cmake.in --> ${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake")
ADD_CUSTOM_TARGET(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake")

# configure the cmake script to create a source package
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/CPackSourceConfig.cmake.in 
               ${CMAKE_BINARY_DIR}/CPackSourceConfig.cmake)
MESSAGE(STATUS "configured ${CMAKE_SOURCE_DIR}/CPackSourceConfig.cmake.in --> ${CMAKE_BINARY_DIR}/CPackSourceConfig.cmake")

# export stuff
INCLUDE(CMakeExportBuildSettings)
CMAKE_EXPORT_BUILD_SETTINGS(${CMAKE_BINARY_DIR}/LIBAWSBuildSettings.cmake)
EXPORT_LIBRARY_DEPENDENCIES(${CMAKE_BINARY_DIR}/LIBAWSDepends.cmake)

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/LIBAWSConfig.cmake.in ${CMAKE_BINARY_DIR}/LIBAWSConfig.cmake)

INCLUDE(CMakeCPack.cmake)
