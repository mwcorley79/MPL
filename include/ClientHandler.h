#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H



#include <string>
#include <thread>
#include <atomic>

#include "EndPoint.h"
#include "TCPSocket.h"
#include "Cpp11-BlockingQueue.h"
#include "Message.h"

////////////////////////////////////////////////////////////////////////////
// ClientHandler.h - Defines customizable server side processing          //
// Language:    Standard C++ 11 (g++ 7.4),                                //
// Platform:    Dell Precision m7720, MINT Linux 19.3 (64-bit)            //
// Application: Project 2 helpers                                         //
// Author:      Mike Corley, Syracuse University                          //
//              mwcorley@syr.edu                                          //
////////////////////////////////////////////////////////////////////////////

/*
 * Module Operations
 * =================
 * This module provides the ClientHandler protocol used define custom server side
 * processing.    Receiver (server) hosts the client handler by registering the 
 * it through a base class pointer.  This a allows to easily change the processing.
 * Client users must implement a class that derives from ClientHandler get application 
 * specific server behavior. The client must (though public inheritance), derive 
 * from ClientHandler and provide an override for "Clone()" and "AppProc()". 
 * "Clone()" provides the creation function for creating the ClientHandler instances 
 * used to service client requests.  The clients implements AppProc()" 
 * to define the application specific processing code.
 * 
 * See Receiver test stub for build information and testing
 */

namespace CSE384
{
    // ClientHandler Interface: subclass this to get custom behavior
    class ClientHandler
    {
       public: 
         friend class TCPResponder; // let TCPResponder access private state
         ClientHandler();
         void SetSocket(TCPSocket& sock);
         int Close();
         bool IsReceiving() const;
         bool IsSending() const;
         TCPSocket&  GetDataSocket(); 
         Message GetMessage();
         Message ReceiveMessage();
         void PostMessage(const Message& m);
         void SendMessage(const Message& m);

         EndPoint& GetServiceEndPoint();

         EndPoint RemoteEP();
         
         // pure virtual function: must implement 
         virtual void AppProc() = 0;
         virtual ClientHandler* Clone() = 0;
         
         virtual ~ClientHandler();

         // prevent users from making explicit copies of ClientHandler ojects  
        ClientHandler(const ClientHandler&) = delete;
        ClientHandler& operator=(ClientHandler&) = delete;
        
       private:
         void StartSending();
         void StartReceiving();

         void StopSending();
         void StopReceiving();

         // can redefine socket level processing (if you wish)
         virtual void RecvProc();
         virtual void SendProc();  
         virtual Message RecvSocketMessage();
         virtual void SendSocketMessage(const Message& msg);
        
         void IsReceiving(bool receiving);  
         void IsSending(bool issending);

         void ShutdownRecv();
         void ShutdownSend();
        
         void SetServiceEndPoint(const EndPoint& ep);

         std::thread recvThread;
         std::thread sendThread;
         
         TCPSocket data_socket;
         std::atomic<bool> isReceiving_;
         std::atomic<bool> isSending_;
         
         BlockingQueue<Message> recv_queue_;
         BlockingQueue<Message> send_bq_;
         EndPoint ServiceEP;
    };
    
    
    inline ClientHandler::ClientHandler(): isReceiving_(false),
                                           isSending_(false)                         
    {}

    inline int ClientHandler::Close()
    {   
       //data_socket.ShutdownSend();
       //data_socket.ShutdownRecv();
       return data_socket.Close();
    }

    inline TCPSocket& ClientHandler::GetDataSocket()
    {
       return data_socket;
    }

    inline void ClientHandler::SetServiceEndPoint(const EndPoint& ep)
    {
      ServiceEP = ep;
    }

    inline EndPoint ClientHandler::RemoteEP()
    {
       return data_socket.RemoteEP();
    }

    inline EndPoint& ClientHandler::GetServiceEndPoint()
    {
       return ServiceEP;
    }

    inline ClientHandler::~ClientHandler()
    {
       try
       {
          StopSending();
       }
       catch(const std::exception& e)
       {
          std::cerr << e.what() << '\n';
       }
    }

    inline Message ClientHandler::GetMessage()
    {
       return recv_queue_.deQ();
    }

    inline Message ClientHandler::ReceiveMessage()
    {
       return RecvSocketMessage();
    }

    inline void ClientHandler::PostMessage(const Message& msg)
    {
       return send_bq_.enQ(msg);
    }

    inline void ClientHandler::SendMessage(const Message& msg)
    {
       SendSocketMessage(msg);
    }
    
    inline void ClientHandler::SetSocket(TCPSocket& sock)
    {
      data_socket = std::move(sock);
    }

    inline bool ClientHandler::IsReceiving() const
    {
       return isReceiving_.load();
    }

    inline void ClientHandler::IsReceiving(bool receiving)
    {
       isReceiving_.store(receiving);
    }

    inline void ClientHandler::IsSending(bool sending)
    {
       isSending_.store(sending);
    }

    inline bool ClientHandler::IsSending() const
    {
       return isSending_.load();
    }

    ////////////////////////////////////////
    //  fix size message client handler   //
    ///////////////////////////////////////
    class FixedSizeMsgClientHander : public ClientHandler
    {
       public:
          FixedSizeMsgClientHander(int msg_size);
          int GetMessageSize() const;
       private: 
          int msg_size_;
          // redefine socket level processing for fixed message handling 
          // only one send and recv system call
          virtual Message RecvSocketMessage();
          virtual void SendSocketMessage(const Message& msg);
    };

    inline int FixedSizeMsgClientHander::GetMessageSize() const
    {
       return msg_size_;
    }
}

#endif 
