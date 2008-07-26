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
ENABLE_TESTING()
INCLUDE(CTest)

# S3
CREATE_TEST_SOURCELIST(s3tests
    s3tests.cpp
    s3buckettest.cpp  
    s3objecttest.cpp  
  )

# add the executable
ADD_EXECUTABLE(s3tests ${s3tests})
TARGET_LINK_LIBRARIES(s3tests aws ${requiredlibs})

SET (TestsToRun ${s3tests})
REMOVE (TestsToRun s3tests.cpp)

# add all the ADD_TEST for each testing
FOREACH (test ${TestsToRun})
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  MESSAGE(STATUS ${TName})
  ADD_TEST(${TName} s3tests ${TName})
ENDFOREACH(test)

# SQS
CREATE_TEST_SOURCELIST(sqstests
  sqstests.cpp
  sqstest.cpp
  )

ADD_EXECUTABLE(sqstests ${sqstests})
TARGET_LINK_LIBRARIES(sqstests aws ${requiredlibs})

SET (TestsToRun ${sqstests})
REMOVE (TestsToRun sqstests.cpp)

# add all the ADD_TEST for each testing
FOREACH (test ${TestsToRun})
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  MESSAGE(STATUS ${TName})
  ADD_TEST(${TName} sqstests ${TName})
ENDFOREACH(test)

# SDB
CREATE_TEST_SOURCELIST(sdbtests
  sdbtests.cpp
  sdbtest.cpp
  )

ADD_EXECUTABLE(sdbtests ${sdbtests})
TARGET_LINK_LIBRARIES(sdbtests aws ${requiredlibs})

# remove the test driver source file
SET (TestsToRun ${sdbtests})
REMOVE (TestsToRun sdbtests.cpp)

# add all the ADD_TEST for each testing
FOREACH (test ${TestsToRun})
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  MESSAGE(STATUS ${TName})
  ADD_TEST(${TName} sdbtests ${TName})
ENDFOREACH(test)
