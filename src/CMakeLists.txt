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

# set the include path for all files in the src directory
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# add files to compile
SET(AWS_SRCS awsconnection.cpp 
             awsquerycallback.cpp             
             awsqueryconnection.cpp
             response.cpp
             requestheadermap.cpp 
             callingformat.cpp 
             canonizer.cpp
             awstime.cpp
             exception.cpp
             curlstreambuf.cpp
             ${CMAKE_CURRENT_BINARY_DIR}/awsversion.cpp
             )

INCLUDE(api/CMakeLists.txt)
FOREACH (SRCS ${API_SRCS})
   LIST(APPEND AWS_SRCS api/${SRCS})
ENDFOREACH (SRCS)

INCLUDE(s3/CMakeLists.txt)
FOREACH (SRCS ${S3_SRCS})
   LIST(APPEND AWS_SRCS s3/${SRCS})
ENDFOREACH (SRCS)

INCLUDE(sqs/CMakeLists.txt)
FOREACH (SRCS ${SQS_SRCS})
   LIST(APPEND AWS_SRCS sqs/${SRCS})
ENDFOREACH (SRCS)

INCLUDE(sdb/CMakeLists.txt)
FOREACH (SRCS ${SDB_SRCS})
   LIST(APPEND AWS_SRCS sdb/${SRCS})
ENDFOREACH (SRCS)

INCLUDE(logging/CMakeLists.txt)
FOREACH (SRCS ${LOGGING_SRCS})
   LIST(APPEND AWS_SRCS logging/${SRCS})
ENDFOREACH (SRCS)


# which libraries should be installed
SET(AWS_INSTALL_LIST)

# generate library
ADD_LIBRARY(aws SHARED ${AWS_SRCS})
TARGET_LINK_LIBRARIES(aws ${requiredlibs})
SET(AWS_INSTALL_LIST ${AWS_INSTALL_LIST} aws)

SET_TARGET_PROPERTIES(aws PROPERTIES VERSION ${LIBAWS_MAJOR_VERSION}.${LIBAWS_MINOR_VERSION}.${LIBAWS_PATCH_VERSION})
SET_TARGET_PROPERTIES(aws PROPERTIES CLEAN_DIRECT_OUTPUT 1)

#generate static library
IF (AWS_BUILD_STATIC_LIBRARY)
  ADD_LIBRARY(aws_static STATIC ${AWS_SRCS})
  TARGET_LINK_LIBRARIES(aws_static ${requiredlibs})
  SET_TARGET_PROPERTIES(aws_static PROPERTIES OUTPUT_NAME "aws")
  SET_TARGET_PROPERTIES(aws_static PROPERTIES VERSION ${LIBAWS_MAJOR_VERSION}.${LIBAWS_MINOR_VERSION}.${LIBAWS_PATCH_VERSION})
  SET_TARGET_PROPERTIES(aws_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
  SET(AWS_INSTALL_LIST ${AWS_INSTALL_LIST} aws_static)
ENDIF (AWS_BUILD_STATIC_LIBRARY)

# install the library
INSTALL(TARGETS ${AWS_INSTALL_LIST}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

