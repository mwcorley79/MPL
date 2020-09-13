cmake_minimum_required(VERSION 3.10)
project(MPL_unit_tests)
 
#######################  Build the unit test targets ################################

# set to c++ 17 (standard) 
set(CMAKE_CXX_STANDARD 17)

# include all the headers from the MPL include folder, and Performance test folder
include_directories(../include)

# add the sources (.cpp) files for the MPL library and test targets
set (TEST_SOURCES ../src/Message.cpp ./Message_unit_test.cpp )

# add the sources (.cpp) files for the MPL library and test targets
# set (TEST_INCLUDES include/Message.h)  

# *** This section is for compiling the unit test targets ***
# 1. generate the Message class test stub target (executable test))
add_executable(MessageUnitTest ${TEST_SOURCES} )
