/////////////////////////////////////////////////////////////////////////////
// Receiver.h -   multithreaded (concurrent) server framework              //
//                framework protoype                                       //
// ver 1.0                                                                 //
// Language:    Standard C++ (gcc/g++ 7.4)                                 //
// Platform:    Dell Precision M7720, Linux Mint 19.3 (64-bit)             //
// Application: CSE 384, MPL (Message passing Layer)                       //
// Author:      Mike Corley, Syracuse University                           //
//              mwcorley@syr.edu                                           //
/////////////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * ===================
 *  This package serves as the basis for providing efficient 
 *  light-weight customizable TCP/IP server framework, allowing
 *  the client (executive) to build custom TCP Servers
 *  This package assumes the target operating system (Linux) is POSIX standard 
 *  compliant. i.e. standard I/O, sockets (IPv4 - old interface) I/O, and 
 *  Threading model. 
 
 *  USAGE:  See Receiver test stub

 * Required Files:
 * ==============
 * Receiver.h, MPUtils.h, Receiver.cpp, Message.cpp, MPUtils.cpp
 *
 * Build Process: (for test stub)
 * =============
 * Linux:  g++ -DTEST_RECEIVER -o ReceiverTest Receiver.cpp Message.cpp MPUtils.cpp ClientHandler.cpp -lpthread 
 *
 * Maintenance:
 * ===========
 *  ver 1.0 : 29 March 2019
 *  -- first release 
*/

#ifndef _Receiver_h_
#define _Receiver_h_

#include <thread>
#include <atomic>

#include "EndPoint.h"
#include "TCPSocket.h"
#include "ClientHandler.h"
#include "ThreadPool.h"

namespace CSE384
{
   class Receiver
   {
      public:
        Receiver(const EndPoint& ep, TCPSocketOptions* sc = nullptr);
        virtual ~Receiver();
        void RegisterClientHandler(ClientHandler* ch);
        void Start(int backlog=20);
        void Stop();
        bool IsListening();

        // prevent users from making copies of Receiver objects
        Receiver(const Receiver&) = delete;
        Receiver& operator=(Receiver&) = delete;

      private:
        virtual void ServiceClient(ClientHandler* ch);

        void ListenThreadProc(int backlog);
        void Initialize(const char* ip, unsigned int port);
        void IsListening(bool listening);  

        EndPoint ServiceEP;
        TCPSocketOptions* sc_;
        TCPSocket listenSocket_;
        ClientHandler* ch_;
        std::atomic<bool> islistening_;
        std::thread listenThread_;

        ThreadPool<8> threadPool_;

        #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64)
        SocketSystem s;
        #endif
   };

   inline bool Receiver::IsListening()
   {
      return islistening_.load();
   }

   inline void Receiver::IsListening(bool listening)
   {
      islistening_.store(listening); 
   }
}
#endif 

