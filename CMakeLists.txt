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

set (INCLUDES include/ClientHandler.h
              include/Cpp11-BlockingQueue.h
              include/EndPoint.h
              include/Logger.h
              include/Message.h
              include/mpl.h
              include/Platform.h
              include/ReceiverExceptions.h
              include/Receiver.h
              include/SenderExceptions.h
              include/Sender.h
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

