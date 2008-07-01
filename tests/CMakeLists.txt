ENABLE_TESTING()
INCLUDE(CTest)

CREATE_TEST_SOURCELIST(s3tests
    s3tests.cpp
    s3buckettest.cpp  
    s3objecttest.cpp  
  )

# add the executable
ADD_EXECUTABLE(s3tests ${s3tests})
TARGET_LINK_LIBRARIES(s3tests aws ${requiredlibs})


# remove the test driver source file
SET (TestsToRun ${s3tests})
REMOVE (TestsToRun s3tests.cpp)

# add all the ADD_TEST for each testing
FOREACH (test ${TestsToRun})
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  MESSAGE(STATUS ${TName})
  ADD_TEST(${TName} UnitTests ${TName})
ENDFOREACH(test)
