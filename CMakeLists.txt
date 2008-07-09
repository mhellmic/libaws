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

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)
#INCLUDE(cmake_modules/FindIconv.cmake)
#FIND_PACKAGE(Iconv)
#IF (Iconv_FOUND)
#  INCLUDE_DIRECTORIES(${ICONV_INCLUDE_DIR})
#  SET(requiredlibs ${requiredlibs} ${ICONV_LIBRARY})
#ELSE (Iconv_FOUND)
#  MESSAGE(FATAL_ERROR "Could not find the iconv library and development files (required by libxml2).")
#ENDIF (Iconv_FOUND)

INCLUDE(CMakeConfiguration.txt)
INCLUDE(CMakeCompiler.txt)

SET(LIBAWS_MAJOR_VERSION "0")
SET(LIBAWS_MINOR_VERSION "5")
SET(LIBAWS_PATCH_VERSION "0")

# set the include path to the API headers
# note that there also some generated headers files in the build directory
INCLUDE_DIRECTORIES(AFTER ${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(AFTER ${CMAKE_CURRENT_BINARY_DIR}/include)


# Tell CMake to process the sub-directories
ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(bin)
ADD_SUBDIRECTORY(fuse)
ADD_SUBDIRECTORY(doc)

# add the uninstall support
CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeUninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake"
    @ONLY)
MESSAGE(STATUS "configured ${CMAKE_CURRENT_SOURCE_DIR}/CMakeUninstall.cmake.in --> ${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake")
ADD_CUSTOM_TARGET(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake")

INCLUDE(InstallRequiredSystemLibraries)

# configure the cmake script to create a source package
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/CPackSourceConfig.cmake.in 
               ${CMAKE_BINARY_DIR}/CPackSourceConfig.cmake)
MESSAGE(STATUS "configured ${CMAKE_SOURCE_DIR}/CPackSourceConfig.cmake.in --> ${CMAKE_BINARY_DIR}/CPackSourceConfig.cmake")

