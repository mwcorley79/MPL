# Message Passing Library (MPL) 
Reusable message passing framework (repo) for developing network (client/server) applications by wrapping full duplex TCP sockets.  <br> <br>
 One goal is to enable cross-platform portability (TCP socket level compatibility between Linux and Windows) by providing a TCPSocket class that wraps the (basic) differences between low-level (C-based) Window and Linux socket APIs  -- note: this not a complete, but minimal port to support the TCP communication required to enable message passing). <br> <br>
<b> The Windows port is not yet functional (as of 07/05/2020). See TCPSocket.h/.cpp for details on socket wrapping </b> <br><br>
This project uses CMake to manage the build process for Linux and Windows.  <br> <br>
The current objective (purpose) for this repo is to participate in a joint effort with Dr. James Fawcett to compare various (primarily performance related) aspects of the C++ and the (newer) Rust programming language. <br><br>

Steps to Build MPL (shared library: MPL.so -- Linux and MPL.dll -- on Windows) <br> <br>
(Note: only Linux vesrion is functional as of 7/6/20)
Prerequisites for Linux
<ol>
  <li> install cmake 3.10 or greater (debian based:  sudo apt-get install cmake) </li>
  <li> toolchain: e.g. gcc/g++ etc. </li>
</ol>
Steps for testing on Linux: build the MPL.so (shared object library installed ./MPL/install/lib)
<ol> 
  <li> open a terminal console windows and type the following commands in the shell ... 
  <li> git clone https://github.com/mwcorley79/MPL.git </li>
  <li> cd MPL</li>
  <li> mkdir build </li>
  <li> cd build </li>
  <li> cmake .. -DCMAKE_INSTALL_PREFIX=../install </li>
  <li> make install </li>
 </ol>
 Steps for testing the MPL
 <ol>
  <li>  ... in the same terminal from the previous step, type the following commands </li>
  <li> cd ../MPLtest </li>
  <li> mkdir build </li>
  <li> cd build </li>
  <li> cmake .. </li>
  <li> make all </li>
  The previous step should have produced two executable files: SenderTest and ReceiberTest. </br></br>
  <li> Open two (separate terminal windows and type ./ReceiverTest in one terminal, and ./SenderTest in the other trerminal
       This should produce input such as the following </i>
  

Prerequisites for Linux(Windows)
  <ol>
    <li> Install CMake for Windows: https://cmake.org/download/ </li>
    <li> Visual Studio 2019 (installed, using as default generator)</li>
  </ol>
  Steps for testing on Windows <br><br>
  Will update when the Windows version is complete

