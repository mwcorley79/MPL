/////////////////////////////////////////////////////////////////////////////
// TCPConnector.h -   multithreaded (concurrent) TCP client                //
//                framework protoype                                       //
// ver 1.0                                                                 //
// Language:    Standard C++ (gcc/g++ 7.4)                                 //
// Platform:    Dell Precision M7720, Linux Mint 19.3 (64-bit)             //
// Application: CSE 384, MPL, Project 2 helpers                            //
// Author:      Mike Corley, Syracuse University                           //
//              mwcorley@syr.edu                                           //
/////////////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * ===================
 *  This package serves as the basis for efficient light-weight customizable TCP
 *  message passing client server framework.
 *  The intent is to provide the basic facilities enabling the executive (client)
 *  to build custom/resuable TCP message passing clients 
 *  This package assumes the target operating system is POSIX standard compliant 
 *  i.e. standard I/O, sockets (IPv4 - old interface) I/O, and Threading model.   
 
 * Required Files:
 * ==============
 * 
 *
 * USAGE: See TCPConnector Test stub
 * 
 * Build Process: (for test stub)
 * =============
 * 
 *
 * Maintenance:
 * ===========
 * ver 1.2 : 26 July 2019
 * -- first release 
*/

#ifndef _TCPCONNECTOR_H_
#define _TCPCONNECTOR_H_

#include "Cpp11-BlockingQueue.h"
#include "EndPoint.h"
#include "TCPSocket.h"
#include "Message.h"

#include <cstring>
#include <thread>
#include <atomic>
#include <memory>

namespace CSE384
{
  class TCPConnector
  {
    public: 
     TCPConnector(TCPSocketOptions* sc = nullptr);
     virtual ~TCPConnector();
   
    // void Stop();
     bool Close();
     bool IsConnected() const;
     bool IsSending() const;
     bool IsReceiving() const;
     void PostMessage(const MessagePtr& m);

     MessagePtr GetMessage();
     
     void Connect(const EndPoint& ep);
     int  ConnectPersist(const EndPoint& ep,  unsigned retries, 
                         unsigned wtime_secs, unsigned vlevel);   

     // prevent users from making copies of TCPConnector objects  
     TCPConnector(const TCPConnector&) = delete;
     TCPConnector& operator=(TCPConnector&) = delete;

    private:
        TCPClientSocket socket;
        TCPSocketOptions* sc_;

        std::atomic<bool> isSending_;
        std::atomic<bool> isReceiving_;

        BlockingQueue<MessagePtr> recv_queue_;
        BlockingQueue<MessagePtr> send_bq_;
        std::thread send_thread_;
        std::thread recvThread;
        
        void StartReceiving();
       
       // void StopReceiving();
        void IsReceiving(bool receiving);  

        bool Start();

        // can redefine socket level processing (if you wish)
        virtual void SendSocketMessage(const MessagePtr& msg);
        virtual void sendProc();  
        virtual void RecvProc();
        virtual MessagePtr RecvSocketMessage();

        void IsSending(bool issending);   
        void StartSending();
        void StopSending();

        
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64)
        // On windows the Winsock needs to initialized SocketSystem manages this with a ref count
        SocketSystem s;
#endif
        
  };

  inline void TCPConnector::IsSending(bool issending)
  {
     isSending_.store(issending);
  }

  inline bool TCPConnector::IsSending() const
  {
    return isSending_.load(); 
  }

  inline bool TCPConnector::IsReceiving() const
  {
    return isReceiving_.load();
  }

  inline void TCPConnector::IsReceiving(bool receiving)
  {
       isReceiving_.store(receiving);
  }

  inline bool TCPConnector::IsConnected() const
  {
     return socket.IsConnected();
  }
}; 

#endif 