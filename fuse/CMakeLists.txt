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
SET(FUSE_SRCS s3fs.cpp) 

SET(CMAKE_REQUIRED_LIBRARIES pthread)
CHECK_INCLUDE_FILES(pthread.h LIBAWS_HAVE_PTHREAD_H)
CHECK_FUNCTION_EXISTS(pthread_spin_init HAVE_PTHREAD_SPINLOCK)
CHECK_FUNCTION_EXISTS(pthread_mutex_init HAVE_PTHREAD_MUTEX)

if(NOT LIBAWS_HAVE_PTHREAD_H)
  MESSAGE(FATAL_ERROR "Could not find the MEMCACHED library and development files.")
ENDIF(NOT LIBAWS_HAVE_PTHREAD_H)

ADD_EXECUTABLE(s3fs ${FUSE_SRCS})
TARGET_LINK_LIBRARIES(s3fs aws ${FUSE_LIBRARY} pthread)
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/s3fs DESTINATION bin)
