cmake_minimum_required(VERSION 3.10)
project(MPL)
 
####################### Build MPL library targets ################################

# set to c++ 20 (standard) 
set(CMAKE_CXX_STANDARD 20)

# include all the headers from the MPL include folder, and Performance test folder
include_directories(./include ./MPLPerformanceTests/include)

# add the sources (.cpp) files for the MPL library and test stub targets
set (SOURCES src/Cpp11-BlockingQueue.cpp
             src/ClientHandler.cpp
             src/Logger.cpp
             src/TCPConnector.cpp 
             src/ThreadPool.cpp
             src/Message.cpp
             src/Task.cpp
             src/Utilities.cpp
             src/EndPoint.cpp         
             src/TCPResponder.cpp
             src/TCPSocket.cpp
             src/Platform.cpp)

set (INCLUDES include/ClientHandler.h
              include/Cpp11-BlockingQueue.h
              include/EndPoint.h
              include/Logger.h
              include/Message.h
              include/mpl.h
              include/Platform.h
              include/ReceiverExceptions.h
              include/TCPResponder.h
              include/SenderExceptions.h
              include/TCPConnector.h
              include/Task.h
              include/TCPSocketExceptions.h
              include/TCPSocket.h
              include/ThreadPool.h
              include/Utilities.h)  

# generate the MPL (shared) library target (.so / .dll) from the SOURCES
# add_library(MPLshared SHARED ${SOURCES} )

# generate the MPL (static) library target (.a / .lib) from the SOURCES
add_library(MPL STATIC ${SOURCES} )

# install the lib(s) and includes into the respeective the install directories
install(TARGETS MPL DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)    
         

# *** This section is for compiling test stub targets *****

# 1. generate the TCP socket test stub target (executable test) 
add_executable(TCPSocketsTest src/TCPSocket.cpp src/EndPoint.cpp src/Platform.cpp)
target_compile_definitions(TCPSocketsTest PUBLIC TEST_SOCKETS)  

# 2. generate the Message class test stub target (executable test)
add_executable(MessageTest src/Message.cpp src/Platform.cpp)
target_compile_definitions(MessageTest PUBLIC TEST_MESSAGE) 

# 3. generate the BloclingQueue class test stub target (executable test)
add_executable(BQueueTest src/Cpp11-BlockingQueue.cpp)
target_compile_definitions(BQueueTest PUBLIC TEST_BLOCKING_QUEUE) 

# 4. generate the TCPConnector class test stub target (executable test)
add_executable(TCPConnectorTest  src/TCPConnector.cpp 
                           src/Message.cpp
                           src/EndPoint.cpp         
                           src/TCPSocket.cpp
                           src/Platform.cpp)
target_compile_definitions(TCPConnectorTest PUBLIC TEST_CONNECTOR) 

# 5. generate the TCPResponder class test stub target (executable test)
add_executable(TCPResponderTest src/TCPResponder.cpp 
                            src/ClientHandler.cpp
                            src/Logger.cpp
                            src/Task.cpp
                            src/ThreadPool.cpp
                            src/Message.cpp
                            src/EndPoint.cpp         
                            src/TCPSocket.cpp
                            src/Platform.cpp)
target_compile_definitions(TCPResponderTest PUBLIC TEST_RESPONDER) 


#  6. Performance Test Targets
# ******************************************************************
link_directories( ./debug ./release ./Debug ./Release ./install)

# generate the TCPResponderPerfTest test stub target (executable test) from the SOURCES
add_executable(TCPResponderPerfTest ./MPLPerformanceTests/src/PerformanceResponder.cpp)
# TCPResponderPerfTest -- depends on MPL target
add_dependencies(TCPResponderPerfTest MPL)

# generate the TCPConnector test stub target (executable test) from the SOURCES
add_executable(TCPConnectorPerfTest ./MPLPerformanceTests/src/PerformanceConnector.cpp)
add_dependencies(TCPConnectorPerfTest MPL)

# generate the PerfTestCombinedFixedSizeMsg test stub target (executable test) from the SOURCES
add_executable(PerfTestCombinedFixedSizeMsg ./MPLPerformanceTests/src/PerfTestCombinedFixedSizeMsg.cpp)
add_dependencies(PerfTestCombinedFixedSizeMsg MPL)

# generate the PerfTestCombinedVariableSizeMsg test stub target (executable test) from the SOURCES
add_executable(PerfTestCombinedVariableSizeMsg ./MPLPerformanceTests/src/PerfTestCombinedVariableSizeMsg.cpp)
add_dependencies(PerfTestCombinedVariableSizeMsg MPL)

if (UNIX)
    # link target to pthread library for LINUX
    target_link_libraries (TCPSocketsTest pthread)
    target_link_libraries (BQueueTest pthread)
    target_link_libraries (TCPConnectorTest pthread)
    target_link_libraries (TCPResponderTest pthread)
   
    # Performance test targets
    # link libMPL and pthread to the targets for LINUX
    target_link_libraries (TCPResponderPerfTest MPL.a pthread)
    target_link_libraries (TCPConnectorPerfTest MPL.a pthread)
    target_link_libraries (PerfTestCombinedFixedSizeMsg MPL.a pthread)
    target_link_libraries (PerfTestCombinedVariableSizeMsg MPL.a pthread)

else (NOT UNIX) 
     # no need to link the others targets to pthread on Windows
     # here we link MPL.lib to TCPResponderTest , TCPConnectorTest, combined targets for WINDOWS
    target_link_libraries (TCPResponderPerfTest MPL.lib)
    target_link_libraries (TCPConnectorPerfTest MPL.lib)
    target_link_libraries (PerfTestCombinedFixedSizeMsg MPL.lib)
    target_link_libraries (PerfTestCombinedVariableSizeMsg MPL.lib)
endif (UNIX)

# ***  End test stub target section ***
