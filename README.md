# Message Passing Library (MPL) 
Reusable message passing library (MPL) framework for developing network (client/server) applications by wrapping full duplex TCP sockets.  <br> <br>
One goal is to enable cross-platform portability (TCP socket level compatibility between Linux and Windows) by providing a TCPSocket class that wraps the (basic) differences between the low-level (C-based) Windows and Linux socket APIs.  (note: this not a complete, but minimal port to support the TCP communication required to enable message passing.) <br> <br>
<b> The See TCPSocket.h/.cpp for details on the TCP socket wrapper code </b> <br><br>
This project uses CMake to manage the build process for Linux and Windows.  <br> <br>
The current objective (purpose) for this repo is to participate in a joint effort with Dr. James Fawcett (https://github.com/JimFawcett) to compare various (primarily performance related) aspects of the C++ and the (newer) Rust programming language. <br><br>

<b> Prerequisites for Linux </b>
<ol>
  <li> install cmake 3.10 or greater (debian based:  sudo apt-get install cmake) </li>
  <li> toolchain: e.g. gcc/g++ etc. </li>
</ol>
<b> Build and install the MPL library on Linux: MPL.a (static library installed in MPL/install/lib) </b>
<ol> 
  <li> open a terminal/console window and type the following commands in the shell ... </li>
  <li> git clone https://github.com/mwcorley79/MPL.git </li>
  <li> cd MPL && mkdir build && cd build </li>
  <li> cmake .. -DCMAKE_INSTALL_PREFIX=../install </li>
  <li> make install </li>
</ol>
<b> Testing the MPL library on Linux </b>
<ol>
  <li>  ... in the same terminal from the previous step, type the following commands </li>
  <li> cd ../MPLtests && mkdir build && cd build </li>
  <li> cmake .. </li>
  <li> make all </li>
  The previous step should have produced two executable files: SenderTest and ReceiverTest. </br></br>
  <li> Open two (separate terminal windows and type ./ReceiverTest in one terminal, and ./SenderTest in the other trerminal
       This should produce output such as the following: </i>
  <img src="./png/output.PNG"/>  
</ol> 
<b> Prerequisites for Windows <b>
  <ol>
    <li> Install CMake for Windows: https://cmake.org/download/ </li>
    <li> Visual Studio 2019 (installed, using as default generator)</li>
  </ol>
 <b> Build and install the MPL library on Windows: MPL.lib (static library installed in MPL/install/lib) </b>
  <ol> 
    <li> open a terminal/command window and type the following commands in the shell ... </li>
    <li> git clone https://github.com/mwcorley79/MPL.git </li>
    <li> cd MPL && mkdir build && cd build </li>
    <li> cmake .. -DCMAKE_INSTALL_PREFIX=../install
      <ol><li> this generate the Visual Studio solution and projects in "MPL/build" folder <br>
        and set install mpl library install folder to "MPL/INSTALL") </li></ol>
    </li>   
    <li>  Start Visual Studio 2019, choose "Open and Project or solution", <br>
          navigate to the "MPL/build" folder and open solution file (MPL.sln) 
    </li>
    <li> right click on the "INSTALL" and click "Build" <br>
         Note: Leave Visual Studio and terminal (from step 3) running
    </li>
    <b> Testing the MPL library on Windows </b>
   
       
  
    
</ol>
  
  
  <H2>Description:</H2>
  Three (3) primary classes comprise the framework: <b> Sender - </b> encapsulates client-side message passing. <b> Receiver </b> and <b> ClientHandler - </b>         encapsulate server-side message passing. Full-duplex infers bidirectional communication in which both the Sender and (derived) ClientHandler instances support GetMessage() and PostMessage() operations simultaneously.  The following picture illustrates the high-level concept: 
  <img src="./png/concept.PNG"/>  
