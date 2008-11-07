# Copyright 2006-2008 28msec, Inc.
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
# - Try to find libMemcached
# Once done this will define
#
#  MEMCACHED_FOUND - system has libmemcache
#  MEMCACHED_INCLUDE_DIR - the libmemcache include directory
#  MEMCACHED_LIBRARY - Link these to use libmemcache
#

IF (MEMCACHED_INCLUDE_DIR AND MEMCACHED_LIBRARY)
   # in cache already
   SET(Memcached_FIND_QUIETLY TRUE)
ENDIF (MEMCACHED_INCLUDE_DIR AND MEMCACHED_LIBRARY)

FIND_PATH(MEMCACHED_INCLUDE_DIR memcached.h
  /usr/include
  /usr/local/include
  /usr/include/libmemcached
  /usr/local/include/libmemcached
)

FIND_LIBRARY(MEMCACHED_LIBRARY NAMES memcached
  PATHS
  /usr/${LIB_DESTINATION}
  /usr/local/${LIB_DESTINATION})

IF (MEMCACHED_INCLUDE_DIR AND MEMCACHED_LIBRARY)
   SET(MEMCACHED_FOUND TRUE)
ELSE (MEMCACHED_INCLUDE_DIR AND MEMCACHED_LIBRARY)
   SET(MEMCACHED_FOUND FALSE)
ENDIF (MEMCACHED_INCLUDE_DIR AND MEMCACHED_LIBRARY)

IF(MEMCACHED_FOUND)
  IF(NOT Memcached_FIND_QUIETLY)
    MESSAGE(STATUS "Found libMemcached: ${MEMCACHED_LIBRARY}")
    MESSAGE(STATUS "Found libMemcached include dir: ${MEMCACHED_INCLUDE_DIR}")
  ENDIF(NOT Memcached_FIND_QUIETLY)
ELSE(MEMCACHED_FOUND)
  IF(Memcached_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find libMemcached")
  ENDIF(Memcached_FIND_REQUIRED)
ENDIF(MEMCACHED_FOUND)

MARK_AS_ADVANCED(MEMCACHED_INCLUDE_DIR MEMCACHED_LIBRARY)
