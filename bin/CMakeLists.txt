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
ADD_EXECUTABLE(s3 s3.cpp)
ADD_EXECUTABLE(sqs sqs.cpp)
ADD_EXECUTABLE(sdb sdb.cpp)

SET(S3_LIBS ${requiredlibs})
TARGET_LINK_LIBRARIES(s3 aws ${S3_LIBS})
TARGET_LINK_LIBRARIES(sqs aws ${S3_LIBS})
TARGET_LINK_LIBRARIES(sdb aws ${S3_LIBS})

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/s3 DESTINATION bin)
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/sqs DESTINATION bin)
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/sdb DESTINATION bin)