# Message Passing Library (MPL) 
Reusable message passing library (MPL) framework for developing network (client/server) applications by wrapping full duplex TCP sockets.  <br> <br>
One goal is to enable cross-platform portability (TCP socket level compatibility between Linux and Windows) by providing a TCPSocket class that wraps the (basic) differences between the low-level (C-based) Windows and Linux socket APIs.  (note: this not a complete, but minimal port to support the TCP communication required to enable message passing.) <br> <br>
<b> The Windows port is not yet functional (as of 07/05/2020). See TCPSocket.h/.cpp for details on the TCP socket wrapper code </b> <br><br>
This project uses CMake to manage the build process for Linux and Windows.  <br> <br>
The current objective (purpose) for this repo is to participate in a joint effort with Dr. James Fawcett to compare various (primarily performance related) aspects of the C++ and the (newer) Rust programming language. <br><br>

<b> Steps to Build MPL (shared library: MPL.so -- Linux and MPL.dll -- on Windows) </b> <br> 
(Note: only Linux vesrion is functional as of 7/6/20)
Prerequisites for Linux
<ol>
  <li> install cmake 3.10 or greater (debian based:  sudo apt-get install cmake) </li>
  <li> toolchain: e.g. gcc/g++ etc. </li>
</ol>
Build MPL.so (shared object library installed in ./MPL/install/lib)
<ol> 
  <li> open a terminal console windows and type the following commands in the shell ... 
  <li> git clone https://github.com/mwcorley79/MPL.git </li>
  <li> cd MPL</li>
  <li> mkdir build </li>
  <li> cd build </li>
  <li> cmake .. -DCMAKE_INSTALL_PREFIX=../install </li>
  <li> make install </li>
 </ol>
 Testing the MPL library on Linux
 <ol>
  <li>  ... in the same terminal from the previous step, type the following commands </li>
  <li> cd ../MPLtest </li>
  <li> mkdir build </li>
  <li> cd build </li>
  <li> cmake .. </li>
  <li> make all </li>
  The previous step should have produced two executable files: SenderTest and ReceiverTest. </br></br>
  <li> Open two (separate terminal windows and type ./ReceiverTest in one terminal, and ./SenderTest in the other trerminal
       This should produce output such as the following: </i>
  <img src="./png/output.PNG"/>  
  
Prerequisites for Windows
  <ol>
    <li> Install CMake for Windows: https://cmake.org/download/ </li>
    <li> Visual Studio 2019 (installed, using as default generator)</li>
  </ol>
  Steps for testing on Windows <br><br>
  *** Will update when the Windows version is complete *** <br><br>
  
  <H2>Description:</H2>
  Three (3) primary classes comprise the framework: <b> Sender - </b> encapsulates client-side message passing. <b> Receiver </b> and <b> ClientHandler - </b>         encapsulate server-side message passing. Full-duplex infers bidirectional communication in which both the Sender and (derived) ClientHandler instances support GetMessage() and PostMessage() operations simultaneously.  The following picture illustrates the high-level concept: 
  <img src="./png/concept.PNG"/>  

