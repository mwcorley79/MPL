# Message Passing Library (MPL) 
Is a reusable message passing framework (repo) for developing network (client/server) applications by wrapping full duplex TCP sockets.  <br> <br>
 One goal is to enable cross-platform portability (TCP socket level compatibility between Linux and Windows) by providing a TCPSocket class that wraps the (basic) differences between low-level (C-based) Window and Linux socket APIs  -- note: this not a complete, but minimal port to support the TCP communication required to enable message passing). <br> <br>
This project uses CMake to manage the build process for Linux and Windows.  <br> <br>
The current objective (purpose) for this repo is to participate in a joint effort with Dr. James Fawcett to compare various (primarily performance related) aspects of the C++ and the (newer) Rust programming language.    
