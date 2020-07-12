cmake_minimum_required(VERSION 3.10)
project(TransferService)

# set to c++ 17 (standard) 
set(CMAKE_CXX_STANDARD 17)

# include all the headers from the include folder
include_directories(../../MPL/install/include ./include)

# specify directories for linker/loader to search
link_directories( ../../MPL/install/lib)

# pick the correct version of Dr. Fawcett's Filesystem class (Linux versus Windows)
if(UNIX)
  set (FILESYSTEM_SOURCE src/FileSystemLin.cpp)
else (NOT UNIX)
  set (FILESYSTEM_SOURCE src/FileSystemWin.cpp)
endif (UNIX)

 
# generate the FileTransfer Server target (executable test) from the SOURCES  
add_executable(FileTransferServer src/FileTransferService.cpp ${FILESYSTEM_SOURCE})
target_compile_definitions(FileTransferServer PUBLIC TEST_SERVER)

# generate the FileTransfer client target (executable test) from the SOURCES
add_executable(FileTransferClient src/FileTransferClient.cpp ${FILESYSTEM_SOURCE})
target_compile_definitions(FileTransferClient PUBLIC TEST_CLIENT)

# link the MPL library according to platform: MPL.a (static lib) on Linux,  MPL.lib (static lib on Windows)
if (UNIX)
    # link MPL.so and pthread to the ReceiverTest and SenderTest targets for LINUX
    target_link_libraries (FileTransferServer MPL.a pthread)
    target_link_libraries (FileTransferClient MPL.a pthread)
else (NOT UNIX) 
    # link MPL.dll to ReceiverTest and SenderTest targets for WINDOWS
    target_link_libraries (FileTransferServer MPL.lib)
    target_link_libraries (FileTransferClient MPL.lib)
endif (UNIX)


