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

SET(requiredlibs)

FIND_PACKAGE(CURL)
IF(CURL_FOUND)
    INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
    SET(requiredlibs ${requiredlibs} ${CURL_LIBRARIES} )
ENDIF(CURL_FOUND)

FIND_PACKAGE(OpenSSL)
IF(OPENSSL_FOUND)
    INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
    SET(requiredlibs ${requiredlibs} ${OPENSSL_LIBRARIES} crypto) # bugfix, because libcrypto is not found automatically
ENDIF(OPENSSL_FOUND)
  
FIND_PACKAGE(LibXml2)
IF(LIBXML2_FOUND)
    INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
    SET(requiredlibs ${requiredlibs} ${LIBXML2_LIBRARIES}) 
    SET(CXXFLAGS ${CXXFLAGS} ${LIBXML2_DEFINITONS}) # needed?
ENDIF(LIBXML2_FOUND)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)
INCLUDE(cmake_modules/FindIconv.cmake)
FIND_PACKAGE(Iconv)
IF (Iconv_FOUND)
    INCLUDE_DIRECTORIES(${ICONV_INCLUDE_DIR})
    SET(requiredlibs ${requiredlibs} ${ICONV_LIBRARY})
ENDIF (Iconv_FOUND)

INCLUDE(CMakeConfiguration.txt)
INCLUDE(CMakeCompiler.txt)

SET(LIBAWS_MAJOR_VERSION "0")
SET(LIBAWS_MINOR_VERSION "5")
SET(LIBAWS_PATCH_VERSION "0")

# set the include path to the API headers
INCLUDE_DIRECTORIES(AFTER ${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(AFTER ${CMAKE_CURRENT_BINARY_DIR}/include)


# Tell CMake to process the sub-directories
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(bin)
add_subdirectory(fuse)
add_subdirectory(doc)
