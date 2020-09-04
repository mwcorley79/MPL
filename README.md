# Message Passing Library (MPL) 
This repository provides a reusable C++ message passing library (MPL) framework for developing TCP/IP network (client/server) applications by wrapping full duplex TCP socket communucation on Linux and Windows. <br> <br>
One goal is to enable cross-platform portability (TCP socket level compatibility between Linux and Windows) by providing a TCPSocket class that encapsulates the (basic) differences between the low-level (C-based) Windows and Linux APIs. <br> <br>
<b> See TCPSocket.h/.cpp for details of the TCP socket wrapper code. </b> <br><br>
This project uses CMake to manage the build process for Linux and Windows.  <br> <br>
The current objective is to provide an efficient C++ network communication facility for a joint experiment with Dr. James Fawcett (https://github.com/JimFawcett) to compare various (initially performance related) aspects of modern C++ and the Rust programming language. <hr>
<b>Prerequisites for getting started on Linux:</b>
<ol>
  <li> Download and install CMake (Debian-based distros): <b> sudo apt-get install cmake </b> </li>
  <li> Download and install git:   <b> sudo apt-get install git </b> </li>
  <li> Download and install gcc/g++ (compiler toolchain): <b> sudo apt-get install g++ </b> </li>
  <li> Open a terminal/console and type command: <b> git clone https://github.com/mwcorley79/MPL.git </b> </li>
</ol>
<br>
<b>Prerequisites for getting started on Windows:</b>
<ol>
  <li> Download and install CMake: https://cmake.org/download/</b> </li>
  <li> Download and install git:   <b> https://git-scm.com/download/win </b> </li>
  <li> Note: If Visual Studio 2019 is not installed, then download and install Microsoft C++ Build Tools  (MSVC compiler tools): <b> https://visualstudio.microsoft.com/visual-cpp-build-tools/ </b> 
     <ol>
       <li> Run vs_buildtools (Visual Studio) installer. Check the workload for C++ build tools (as shown below) 
           <img src="./png/build-tools.PNG"/>  
       </li>
     </ol>
  </li>  
  <li>Open a windows command prompt (cmd.exe) and type command: <b> git clone https://github.com/mwcorley79/MPL.git </b> </li>
</ol>

<hr>

<b>Option 1: Building the CMake targets from the command line -- Debug and Release Mode  (these steps apply to both Windows and Linux) </b>
<ol> 
  <li> Open a command line terminal (i.e. bash in Linux) and (cmd.exe in Windows)
    <ul>
      <li> 
        <b> Building in Debug Mode </b> 
        <ol>
           <li> cd MPL </l>
           <li> mkdir debug </li>
           <li> cd debug </li>
           <li> cmake .. -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_BUILD_TYPE=Debug </li>  
        </ol>
      </li>
      <li>
        <b> Building in Release Mode </b> 
        <ol>
           <li> cd MPL </l>
           <li> mkdir release </li>
           <li> cd release </li>
           <li> cmake .. -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_BUILD_TYPE=Release </li>  
        </ol>  
     </li>
   </li>
   
   <li> 
     <b> Building CMake Project Targets </b>
         <li> <b> *** Important notes for Building on Windows only *** </b>
           <ol>
              <li> If you have the Visual Studio 2019 IDE installed, wish may stop at this point and open the MPL.sln solution in the IDE </li>
             <li> If you wish build from the command line in <b> <em> release mode </em> </b>, then you must explicity specify the Release configuration for the target like this: 
                <b> <em> cmake --build . --target  ALL_BUILD --config Release </em> </b>
              </li>  
           </ol>
          </li>
          <li> 
            <ul>
              <li> For Linux: <b> cmake --build . --target all </b> <em> <- build all project targets </em> </li>
              <li> For Windows: <b> cmake --build . --target  ALL_BUILD </b> <em> <- build all project targets </em> 
                  <br> <b> Note: ** </b>
                </li>
            </ul>
          </li> 
          <li> <b> cmake --build . --target MPL </b> <em> <- build the (static) Message Passing Library (MPL) </em> </li>
          <li> <b> cmake --build . --target PerfTestCombinedFixedSizeMsg </b> <em> <- builds the fixed size message performance test </em> </li>
          <li> <b> cmake --build . --target PerfTestCombinedVariableSizeMsg </b> <em> <- builds the variable size message performance test </em> </li>    
          <li> <b> cmake --build . --target BQueueTest  </b> <em> <- builds the BlockingQueue test </em> </li>
          <li> <b> cmake --build . --target MessageTest </b> <em> <-  builds the Message test </em> </li>
          <li> <b> cmake --build . --target TCPSocketsTest </b> <em> <- builds the TCPSocketsTest </em> </li>
        </ul>
    </li>
    <li> 
     <b> Running the Targets </b>
        <ul>
          <li> From a command terminal, type the name of target to run, followed by the Enter key </li>
          <li> e.g. On Windows:<b> cd Debug or cd Release (depends build configuration) and type PerfTestCombinedFixedSizeMsg.exe </b> </li>
          <li> e.g. On Linux: <b> ./PerfTestCombinedFixedSizeMsg </b> </li>
        </ul>
    </li>
 </ol>  
    

<b> Option 2: Building the CMake targets with Visual Studio Code (these steps apply to both Windows and Linux) </b>
<ol> 
  <li> Download and install Visual Studio Code editor (VScode): <b> https://code.visualstudio.com/download </b>
     <ol>
       <li> Start VScode and install "Microsoft C/C++ VScode" extension (ms-vscode.cpptools) </li>
       <li> Install "CMake Tools: Extended CMake support in Visual Studion Code" extension (ms-vscode.cmake-tools) </li>
     </ol> 
  </li>
  <li> Start VScode: click File -> Open Folder and Select the project MPL folder
    <img src="./png/open_project.PNG"/>  
  </li>
  <li> As illustrated (below): Select a compiler kit: (i.e. gcc/g++ for Linux, and Visual Studio Build Tools 2019 Release for Windows)  
     <b> ... on Linux ... </b>
    <img src="./png/no-kit2.PNG"/>  
    <b> ... and on Windows ... </b>
    <img src="./png/vscode-windows-kit.PNG"/>  
  </li>
   <li> As illustrated (below): Build the MPL (library) by selecting the MPL STATIC_LIBARY target and clicking "Build".
        On Windows this generates library: MPL.lib, and on Linux: libMPL.a
    <img src="./png/build-MPL-target.png"/>  
  </li> 
  <li> In similar fashion, build and run the included MPL performance test. Select and Build the TCPConnectorPerfTest EXECUTABLE target, and TCPResponderTest EXECULABLE target </li>
  <li>  As illustrated( below): run the MPL performace test on Windows by clicking the TERMINAL tab, and splitting the terminal into two   
        separate panes (as shown in the output). <br> Run the MPL performance exemplar by using the commands shown in the output. <br>
        The exemplar provides throughput/latency measurements for 1024 byte message sizes running on a Windows 10 virtual machine (VMWARE). 
   <img src="./png/perf_test_windows.PNG"/> 
  </li>
   <li> Run the MPL performace test on Linux by clicking the TERMINAL tab, and splitting the terminal into two   
        seperate panes (as shown in the output). <br> Run MPL performance exemplar by using the commands shown in the output. <br>
        The exemplar provides throughput/latency measurements for 1024 byte message sizes running on a Linux Mint virtual machine (VMWARE). 
   <img src="./png/perf_test_linux.PNG"/> 
  </li>
</ol>
 
 <b> Running the combined test target (PerfTestCombined) is complementary to Dr. Fawcett's testing of the same in Rust (located here: https://jimfawcett.github.io/RustCommWithThreadPool.html) </b>
<ol>
  <li> Start VScode: click File -> Open Folder and Select the project MPL folder
       <img src="./png/perf_test_combined0.PNG"/>  
  </li>
  <li> Build the MPL (library) by selecting the MPL STATIC_LIBARY target and clicking "Build".
       On Windows this generates library: MPL.lib, and on Linux: libMPL.a
       <img src="./png/perf_test_combined.PNG"/>  
  </li> 
  <li> Build the PerfTestCombined target by selecting the PerfTestCombined EXECUTABLE target and clicking "Build".
       <img src="./png/perf_test_combined2.PNG"/>  
  </li> 
  <li> Finally, Run the PerfTestCombined target by clicking Run (shown below) 
       <img src="./png/perf_test_combined3.PNG"/>  
  </li> 
</ol>



 <hr>
   <img src="./png/MPL.jpg"/> 
   MPL class diagram
  
