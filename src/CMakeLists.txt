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
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/awsversion.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/awsversion.cpp)
MESSAGE(STATUS "configured ${CMAKE_CURRENT_SOURCE_DIR}/awsversion.cpp.in --> ${CMAKE_CURRENT_BINARY_DIR}/awsversion.cpp")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(AWS_SRCS awsconnection.cpp 
             response.cpp
             requestheadermap.cpp 
             callingformat.cpp 
             canonizer.cpp
             awstime.cpp
             exception.cpp
             curlstreambuf.cpp
             ${CMAKE_CURRENT_BINARY_DIR}/awsversion.cpp)

INCLUDE(api/CMakeLists.txt)
FOREACH (SRCS ${API_SRCS})
   LIST(APPEND AWS_SRCS api/${SRCS})
ENDFOREACH (SRCS)

INCLUDE(s3/CMakeLists.txt)
FOREACH (SRCS ${S3_SRCS})
   LIST(APPEND AWS_SRCS s3/${SRCS})
ENDFOREACH (SRCS)


ADD_LIBRARY(aws SHARED ${AWS_SRCS})
TARGET_LINK_LIBRARIES(aws ${requiredlibs})

SET_TARGET_PROPERTIES(aws PROPERTIES VERSION ${LIBAWS_MAJOR_VERSION}.${LIBAWS_MINOR_VERSION}.${LIBAWS_PATCH_VERSION})
SET_TARGET_PROPERTIES(aws PROPERTIES CLEAN_DIRECT_OUTPUT 1)


# install the library
INSTALL(TARGETS aws
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

