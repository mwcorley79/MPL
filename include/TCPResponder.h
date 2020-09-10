/////////////////////////////////////////////////////////////////////////////
// TCPResponder.h -   multithreaded (concurrent) server framework              //
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
 
 *  USAGE:  See TCPResponder test stub

 * Required Files:
 * ==============
 * 
 *
 * Build Process: (for test stub)
 * ==============
 *
 * Maintenance:
 * ===========
 *  ver 1.2 : 26 July 2020
 *  -- first release 
*/

#ifndef _TCPRESPONDER_H
#define _TCPRESPONDER_H

#include <thread>
#include <atomic>

#include "EndPoint.h"
#include "TCPSocket.h"
#include "ClientHandler.h"
#include "ThreadPool.h"

namespace CSE384
{
   class TCPResponder
   {
      public:
        TCPResponder(const EndPoint& ep, TCPSocketOptions* sc = nullptr);
        virtual ~TCPResponder();
        void RegisterClientHandler(ClientHandler* ch);
        void Start(int backlog=20);
        void Stop();
        bool UseClientReceiveQueue();
        void UseClientSendReceiveQueues(bool use_qs);
        void UseClientReceiveQueue(bool use_q);
        bool UseClientSendQueue();
        void UseClientSendQueue(bool use_q);
        bool IsListening();
        int NumClients();
        void NumClients(int client_count);

        // prevent users from making copies of TCPResponder objects
        TCPResponder(const TCPResponder&) = delete;
        TCPResponder& operator=(TCPResponder&) = delete;

      private:
        virtual void ServiceClient(ClientHandler* ch);

        void ListenThreadProc(int backlog);
        void Initialize(const char* ip, unsigned int port);
        void IsListening(bool listening);  

        EndPoint ServiceEP;
        TCPSocketOptions* sc_;
        TCPServerSocket listenSocket_;
        ClientHandler* ch_;
        std::atomic<bool> islistening_;
        std::thread listenThread_;

        std::atomic<bool> useClientRecvQueue_;
        std::atomic<bool> useClientSendQueue_;
        std::atomic<int>  num_clients_;

        #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64)
        SocketSystem s;
        #endif
   };

   inline bool TCPResponder::IsListening()
   {
      return islistening_.load();
   }

   inline void TCPResponder::IsListening(bool listening)
   {
      islistening_.store(listening); 
   }

   inline void TCPResponder::UseClientSendReceiveQueues(bool use_qs)
   {
      UseClientReceiveQueue(use_qs);
      UseClientSendQueue(use_qs);
   }

   inline bool TCPResponder::UseClientReceiveQueue()
   {
      return useClientRecvQueue_.load();
   }

   inline void TCPResponder::UseClientReceiveQueue(bool use_q)
   {
      useClientRecvQueue_.store(use_q);
   }

   inline bool TCPResponder::UseClientSendQueue()
   {
      return useClientSendQueue_.load();
   }

   inline void TCPResponder::UseClientSendQueue(bool use_q)
   {
      useClientSendQueue_.store(use_q);
   }

   inline int TCPResponder::NumClients()
   {
      return  num_clients_.load();
   }

   inline void TCPResponder::NumClients(int client_count)
   {
      num_clients_.store(client_count);
   }

}
#endif 

