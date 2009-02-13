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
SET(FUSE_SRCS 
  s3fs.cpp
  properties.cpp
)

SET(s3fs_required_libs aws ${FUSE_LIBRARY})

FIND_PACKAGE(Memcached)
IF(MEMCACHED_FOUND)
  SET(FUSE_SRCS ${FUSE_SRCS} awscache.cpp)
  INCLUDE_DIRECTORIES(${MEMCACHED_INCLUDE_DIR})
  SET(S3FS_USE_MEMCACHED "1")
  SET(s3fs_required_libs ${s3fs_required_libs} ${MEMCACHED_LIBRARY})
ELSE(MEMCACHED_FOUND)
  MESSAGE(STATUS "Could not find the MEMCACHED library and development files.")
ENDIF(MEMCACHED_FOUND)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_EXECUTABLE(s3fs ${FUSE_SRCS})
TARGET_LINK_LIBRARIES(s3fs ${s3fs_required_libs})
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/s3fs DESTINATION bin)

