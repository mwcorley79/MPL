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
        TCPConnector(TCPSocketOptions *sc = nullptr);
        virtual ~TCPConnector();

        // void Stop();
        bool Close(std::thread* listener=nullptr);
        bool IsConnected() const;
        bool IsSending() const;
        bool IsReceiving() const;
        void UseSendReceiveQueues(bool use_qs);
        void UseSendQueue(bool use_q);
        bool UseSendQueue();
        void UseReceiveQueue(bool use_q);
        bool UseReceiveQueue();
        void PostMessage(const Message &m);
        void SendMessage(const Message &m);
        Message GetMessage();
        Message ReceiveMessage();

        TCPClientSocket &GetClientSocket();

        void Connect(const EndPoint &ep);
        int ConnectPersist(const EndPoint &ep, unsigned retries,
            unsigned wtime_secs, unsigned vlevel);

        // prevent users from making copies of TCPConnector objects
        TCPConnector(const TCPConnector &) = delete;
        TCPConnector &operator=(TCPConnector &) = delete;

    protected:
        TCPClientSocket socket;

    private:
        TCPSocketOptions *sc_;

        // can redefine socket level processing (if you wish)
        virtual void SendSocketMessage(const Message &msg);
        virtual Message RecvSocketMessage();

        virtual void sendProc();
        virtual void RecvProc();

        std::atomic<bool> isSending_;
        std::atomic<bool> isReceiving_;

        std::atomic<bool> useSendQueue_;
        std::atomic<bool> useRecvQueue_;

        BlockingQueue<Message> recv_queue_;
        BlockingQueue<Message> send_bq_;
        std::thread send_thread_;
        std::thread recvThread;

        void StartReceiving();
        void StopReceiving();
        void IsReceiving(bool receiving);

        bool Start();

        void IsSending(bool issending);
        void StartSending();
        void StopSending();

        #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64)
        // On windows the Winsock needs to initialized SocketSystem manages this with a ref count
        SocketSystem s;
        #endif
    };

    inline void TCPConnector::PostMessage(const Message &m)
    {
        send_bq_.enQ(m);
    }

    inline void TCPConnector::SendMessage(const Message &m)
    {
        SendSocketMessage(m);
    }

    inline void TCPConnector::IsSending(bool issending)
    {
        isSending_.store(issending);
    }

    inline Message TCPConnector::GetMessage()
    {
        return recv_queue_.deQ();
    }

    inline Message TCPConnector::ReceiveMessage()
    {
        return RecvSocketMessage();
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

    inline TCPClientSocket &TCPConnector::GetClientSocket()
    {
        return socket;
    }

    inline void TCPConnector::UseSendReceiveQueues(bool use_qs)
    {
        UseReceiveQueue(use_qs);
        UseSendQueue(use_qs);
    }

    inline bool TCPConnector::UseReceiveQueue()
    {
        return useRecvQueue_.load();
    }

    inline void TCPConnector::UseReceiveQueue(bool use_q)
    {
        useRecvQueue_.store(use_q);
    }

    inline bool TCPConnector::UseSendQueue()
    {
        return useSendQueue_.load();
    }

    inline void TCPConnector::UseSendQueue(bool use_q)
    {
        useSendQueue_.store(use_q);
    }

    ////////////////////////////////////////
    //  fix size message connector       //
    ///////////////////////////////////////
    class FixedSizeMsgConnector : public TCPConnector
    {
    public:
        FixedSizeMsgConnector(int msg_size, TCPSocketOptions *sc = nullptr);
        int GetMessageSize() const;
    private:
        // redefine socket level processing for fixed message handling 
        // only one send and recv system call
        virtual void SendSocketMessage(const Message &msg);
        virtual Message RecvSocketMessage();
        int msg_size_;
    };

    inline int FixedSizeMsgConnector::GetMessageSize() const
    {
        return msg_size_;
    }
    
}; // namespace CSE384

#endif
