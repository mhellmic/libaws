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
# - Try to find Fuse
# Once done this will define
#
#  FUSE_FOUND - system has Fuse
#  FUSE_INCLUDE_DIR - the Fuse include directory
#  FUSE_LIBRARY - Link these to use Fuse
#

IF (FUSE_INCLUDE_DIR AND FUSE_LIBRARY)
   # in cache already
   SET(Fuse_FIND_QUIETLY TRUE)
ENDIF (FUSE_INCLUDE_DIR AND FUSE_LIBRARY)

FIND_PATH(FUSE_INCLUDE_DIR fuse.h
  /usr/include
  /usr/local/include
  /usr/include/fuse
  /usr/local/include/fuse
)

FIND_LIBRARY(FUSE_LIBRARY NAMES fuse
  PATHS
  /usr/${LIB_DESTINATION}
  /usr/local/${LIB_DESTINATION})

IF (FUSE_INCLUDE_DIR AND FUSE_LIBRARY)
   SET(FUSE_FOUND TRUE)
ELSE (FUSE_INCLUDE_DIR AND FUSE_LIBRARY)
   SET(FUSE_FOUND FALSE)
ENDIF (FUSE_INCLUDE_DIR AND FUSE_LIBRARY)

IF(FUSE_FOUND)
  IF(NOT Fuse_FIND_QUIETLY)
    MESSAGE(STATUS "Found Fuse: ${FUSE_LIBRARY}")
  ENDIF(NOT Fuse_FIND_QUIETLY)
ELSE(Fuse_FOUND)
  IF(Fuse_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find Fuse")
  ENDIF(Fuse_FIND_REQUIRED)
ENDIF(FUSE_FOUND)

MARK_AS_ADVANCED(FUSE_INCLUDE_DIR FUSE_LIBRARY)
