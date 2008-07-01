MESSAGE(STATUS "configured ${CMAKE_CURRENT_SOURCE_DIR}/awsversion.cpp.in --> ${CMAKE_CURRENT_BINARY_DIR}/awsversion.cpp")
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/awsversion.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/awsversion.cpp)

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


# install the library
INSTALL(TARGETS aws
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

