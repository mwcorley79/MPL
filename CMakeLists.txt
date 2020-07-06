cmake_minimum_required(VERSION 3.10)
project(MPL)
 
####################### Build MPL library targets ################################

# set to c++ 17 (standard) 
set(CMAKE_CXX_STANDARD 17)

# include all the headers from the include folder
include_directories(include)

# add the sources (.cpp) files for the MPL library and test stub targets
set (SOURCES src/Cpp11-BlockingQueue.cpp
             src/ClientHandler.cpp
             src/Logger.cpp
             src/Sender.cpp 
             src/ThreadPool.cpp
             src/Message.cpp
             src/Task.cpp
             src/Utilities.cpp
             src/EndPoint.cpp         
             src/Receiver.cpp
             src/TCPSocket.cpp)

# generate the MPL (shared) library target (.so / .dll) from the SOURCES
add_library(MPL SHARED ${SOURCES} )

# generate the MPL (static) library target (.a / .lib) from the SOURCES
add_library(MPLstatic STATIC ${SOURCES} )

# install the statis and shared libs into the bin directory of the install directory
# install(TARGETS MPL MPLstatic  DESTINATION lib)
