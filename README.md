# Message Passing Library (MPL) 
Reusable message passing library (MPL) framework for developing network (client/server) applications.  Wraps full duplex TCP sockets On Linux and Windows. <br> <br>
One goal is to enable cross-platform portability (TCP socket level compatibility between Linux and Windows) by providing a TCPSocket class that encapsulates the (basic) differences between the low-level (C-based) Windows and Linux APIs. <br> <br>
<b> The See TCPSocket.h/.cpp for details on the TCP socket wrapper code. </b> <br><br>
This project uses CMake to manage the build process for Linux and Windows.  <br> <br>
The current objective of this repo is to provide an effcient C++ network communication facility for a joint effort with Dr. James Fawcett (https://github.com/JimFawcett) to compare various (initially performance related) aspects of modern C++ and the Rust programming language. <hr>
<b>Prerequisites for getting started on Linux:</b>
<ol>
  <li> Download and install CMake (Debian-based distros): <b> sudo apt-get install cmake </b> </li>
  <li> Download and install git:   <b> sudo apt-get install git </b> </li>
  <li> Download and install Visual Studio Code editor (VScode): <b> https://code.visualstudio.com/download </b>
     <ol>
       <li> Start VScode and install Microsoft C/C++ VScode extension (ms-vscode.cpptools) </li>
       <li> Install Microsoft CMake Tools VScode extension (ms-vscode.cmake-tools) </li>
     </ol>
  </li>
  <li> Download and install gcc/g++ (compiler toolchain): <b> sudo apt-get install g++ </b> </li>
  <li> Open a terminal/console and type command: </b> git clone https://github.com/mwcorley79/MPL.git </b> </li>
</ol>
<br>
<b>Prerequisites for getting started on Windows:</b>
<ol>
  <li> Download and install CMake: https://cmake.org/download/</b> </li>
  <li> Download and install git:   <b> https://git-scm.com/download/win </b> </li>
  <li> Download and install Visual Studio Code editor (VScode): <b> https://code.visualstudio.com/download </b>
     <ol>
       <li> Start VScode and install Microsoft C/C++ VScode extension (ms-vscode.cpptools) </li>
       <li> Install Microsoft CMake Tools VScode extension (ms-vscode.cmake-tools) </li>
     </ol> 
  </li>
  <li> Download and install Microsoft C++ Build Tools  (MSVC compiler tools): <b> https://visualstudio.microsoft.com/visual-cpp-build-tools/ </b> 
     <ol>
       <li> Run vs_buildtools (Visual Studio) installer. Check the workload for C++ build tools 
           <img src="./png/build-tools.PNG"/>  
       </li>
     </ol>
  </li>  
  <li> open a windows command prompt (cmd.exe) and the command: <b> git clone https://github.com/mwcorley79/MPL.git </b> </li>
</ol>

<br>

<b>Setup the MPL CMake project in VScode (these steps apply to both Windows and Linux) </b>
<ol> 
  <li> Start VSCode: click File -> Open Folder and Select the project MPL folder
    <img src="./png/open_project.PNG"/>  
  </li>
  <li> As illusrated below: Select a compiler kit: (gcc/g++ for Linux), (Visual Studio Build Tools 2019 Release for Windows)  
    <img src="./png/no-kit2.PNG"/>  
    <img src="./png/vscode-windows-kit.PNG"/>  
  </li>
   <li> As illustrated below: Build the MPL (static library) by selecting the MPL STATIC_LIBARY target and clicking Build.
        On Windows this generates MPL.lib, and Linux: libMPL.a
    <img src="./png/build-MPL-target.png"/>  
  </li> 
  <li> In similar fashion, Select and Build the TCPConnectorPerfTest EXECUTABLE target, and TCPResponderTest EXECULABLE target </li>
  <li>  As illustrated below: run the MPL performace test on Windows by clicking the TERMINAL tab, and splitting the terminal into two   
        seperate panes as shown in the output. <br> Run MPL performance exemplar by using the commands as shown in the output. <br>
        The exemplar provides throughput/latency measurements for 1024 byte message sizes, running Windows 10 (VMWARE). 
   <img src="./png/perf_test_windows.PNG"/> 
  </li>
   <li>  As illustrated below: run the MPL performace test on Lunux by clicking the TERMINAL tab, and splitting the terminal into two   
        seperate panes as shown in the output. <br> Run MPL performance exemplar by using the commands as shown in the output. <br>
        The exemplar provides throughput/latency measurements for 1024 byte message sizes, running Linux Mint (VMWARE). 
   <img src="./png/perf_test_linux.PNG"/> 
  </li>

</ol>

  
