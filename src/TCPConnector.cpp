#include "TCPConnector.h"
#include "SenderExceptions.h"
#include "ReceiverExceptions.h"
#include "Platform.h"

namespace CSE384
{
    TCPConnector::TCPConnector(TCPSocketOptions *sc) : sc_(sc),
        isSending_(false),
        isReceiving_(false),
        useSendQueue_(true),
        useRecvQueue_(true)
    {
    }

    void TCPConnector::StartSending()
    {
        if (!IsSending())
        {
            //start the send the send thread
            IsSending(true);
            send_thread_ = std::thread(&TCPConnector::sendProc, this);
        }
    }

    bool TCPConnector::Start()
    {
        if (IsConnected())
        {
            if (UseSendQueue())
                StartSending();

            if (UseReceiveQueue())
                StartReceiving();

            return true;
        }

        return false;
    }

    // dedicated thread function that deques messages
    // from the blocking queue and writes them into the socket
    void TCPConnector::sendProc()
    {
        try
        {
            //IsSending(true);
            while (IsSending())
            {
                // deque the next message
                MessagePtr msgPtr = send_bq_.deQ();

                // if this is the stop sending message, signal
                // the send thread to shutdown
                if (msgPtr->GetType() == STOP_SENDING)
                {
                    IsSending(false);
                }
                else
                {
                    // serialize the message into the socket
                    SendSocketMessage(msgPtr);
                }
            }
        }
        catch (...)
        {
            IsSending(false);
        }
    }

    // serialize the message header and message and write them into the socket
    void TCPConnector::SendSocketMessage(const MessagePtr &msgPtr)
    {
        // convert the wire protocol (message header) to big endian (network byte order)
        //struct MSGHEADER mhdr =  *(const_cast<Message&>(msg).GetHeader());
        //mhdr.ToNetworkByteOrder();

        msgPtr->GetHeader()->ToNetorkByteOrder();
        if (socket.Send(msgPtr->GetRawMsg(), msgPtr->RawMsgLength()) == -1)
        {
            msgPtr->GetHeader()->ToHostByteOrder();
            throw SenderTransmitMessageDataException(getlasterror_portable());
        }
        msgPtr->GetHeader()->ToHostByteOrder();

        // send message header
        //if(socket.Send( (const char*) &mhdr, sizeof(struct MSGHEADER)) == -1)
        // throw SenderTransmitMessageHeaderException(getlasterror_portable());

        // send message data
        //if(socket.Send(msg.GetData(), msg.Length()) == -1)
        //  throw SenderTransmitMessageDataException(getlasterror_portable());
    }



    void TCPConnector::StartReceiving()
    {
        if (!IsReceiving())
        {
            //start the send the send thread
            IsReceiving(true);
            recvThread = std::thread(&TCPConnector::RecvProc, this);
        }
    }

    // RecvProc is a dedicated thread for servicing the socket
    // by pulling out messasges and enQing in the recv blocking queue
    void TCPConnector::RecvProc()
    {
        try
        {
            while (IsReceiving())
            {
                MessagePtr msgPtr = RecvSocketMessage();
                recv_queue_.enQ(msgPtr);
                if (msgPtr->GetType() == MessageType::DISCONNECT)
                {
                    IsReceiving(false);
                }
            }
        }
        catch (const std::exception &ex)
        {
            std::cerr << ex.what() << std::endl;
            IsReceiving(false);
        }
    }

    // serialize the message header and message and write them into the socket
    MessagePtr TCPConnector::RecvSocketMessage()
    {
        struct MSGHEADER mhdr;
        int recv_bytes;
        // receive fixed size message header (see wire protocol in Message.h)
        if ((recv_bytes = socket.Recv((const char *)&mhdr, sizeof(MSGHEADER))) == sizeof(MSGHEADER))
        {
            // *** MUST convert message header to host byte order (e.g. Intel CPU == little endian)
            mhdr.ToHostByteOrder();

            // construct a Message using the Message header read from the socket channel
            // *** critical that mhdr is host byte order ****
            MessagePtr msgPtr(new Message(mhdr));

            // send message data
            if (socket.Recv(msgPtr->GetData(), msgPtr->Length()) == -1)
                throw ReceiverReceiveMessageDataException(getlasterror_portable());

            return msgPtr;
        }

        // if read zero bytes, then this is the zero length message signaling client shutdown
        if (recv_bytes == 0)
        {
            return MessagePtr(new Message(nullptr, 0, DISCONNECT));
        }
        else
        {
            throw ReceiverReceiveMessageHeaderException(getlasterror_portable());
        }
    }

    void TCPConnector::StopSending()
    {
        if (IsSending())
        {
            //note: only gets deposited into queue if IsSending is true
            MessagePtr StopMsgPtr(new Message(nullptr, 0, STOP_SENDING));
            send_bq_.enQ(StopMsgPtr);
        }
    }

    //void TCPConnector::Stop()
    // {
    //    StopSending();
    // }

    /*
    void TCPConnector::StopReceiving()
    {
       if(IsReceiving())
       {
         //note: only gets deposited into queue if IsSending is true
         Message stopMsg(nullptr,0,STOP_RECEIVING);
         recv_queue_.enQ(stopMsg);
       }
    }
    */

    bool TCPConnector::Close()
    {
        bool ret = false;
        if (IsConnected())
        {
            if (UseSendQueue())
            {
                StopSending();

                if (send_thread_.joinable())
                    send_thread_.join();
            }

            socket.ShutdownSend();

            //if(ret)
            // {
            if (UseReceiveQueue())
                if (recvThread.joinable())
                    recvThread.join();
            // }

            // socket.ShutdownRecv();
            // ret = (socket.Close() == 0);
        }

        return ret;
    }

    TCPConnector::~TCPConnector()
    {
        // call close in case the client forgot
        Close();
    }

    void TCPConnector::Connect(const EndPoint &ep)
    {
        socket.Connect(ep, sc_);

        // start send and receive threads
        Start();
    }

    int TCPConnector::ConnectPersist(const EndPoint &ep, unsigned retries,
        unsigned wtime_secs, unsigned vlevel)
    {
        unsigned int runAttempts = 0;
        while (runAttempts++ < retries)
        {
            try
            {
                if (vlevel)
                {
                    std::cout << "Connection attempt # " << runAttempts << " of "
                        << retries << " to " << ep << std::endl;
                }

                Connect(ep);
                break;
            }
            catch (SenderException)
            {
                if (vlevel)
                {
                    std::cout << "Failed Attempt: " << runAttempts << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::seconds(wtime_secs));
            }
        }
        return runAttempts;
    }

    FixedSizeMsgConnector::FixedSizeMsgConnector(int msg_size, TCPSocketOptions *sc) : TCPConnector(sc),
        msg_size_(msg_size)
    {
    }

    /* void FixedSizeMsgConnector::SendSocketMessage(const MessagePtr &msg)
    {
        msg->GetHeader()->ToNetorkByteOrder();
        if (socket.Send(msg->GetRawMsg(), msg->RawMsgLength()) == -1)
            throw SenderTransmitMessageDataException(getlasterror_portable());
        msg->GetHeader()->ToHostByteOrder();
    }
    */

    MessagePtr FixedSizeMsgConnector::RecvSocketMessage()
    {
        MessagePtr msgPtr = Message::CreateEmptyFixedSizeMessage(msg_size_);
        int recv_bytes;

        if ((recv_bytes = socket.Recv(msgPtr->GetRawMsg(), msgPtr->RawMsgLength())) == msgPtr->RawMsgLength())
        {
            // *** MUST convert message header to host byte order (e.g. Intel CPU == little endian)
            msgPtr->GetHeader()->ToHostByteOrder();
            return msgPtr;
        }

        // if read zero bytes, then this is the zero length message signaling client shutdown
        if (recv_bytes == 0)
        {
            return MessagePtr(new Message(nullptr, 0, DISCONNECT));
        }
        else
        {
            throw ReceiverReceiveMessageHeaderException(getlasterror_portable());
        }
    }

}; // namespace CSE384

// *** TCPConnector TEST STUB ****
// ** For testing: run against "Receiver test stub **
#ifdef TEST_CONNECTOR
#include <string>
#include <vector>
#include <iostream>

#ifdef LINK_MPL
#include <mpl.h>
#endif

using namespace CSE384;

void ReceiveProc(TCPConnector *connector)
{
    MessagePtr msg;
    //while ((msg = connector->GetMessage())->GetType() != MessageType::DISCONNECT)
    while ((msg = connector->ReceiveMessage())->GetType() != MessageType::DISCONNECT)
    {
        std::cout << msg->ToString() << std::endl;
    }
}

void TestSenderAsync(const std::string &name)
{
    // announce the sender
    std::cout << name << " started" << std::endl;

    // specify the server Endpoint we wish to connect
    EndPoint serverEp("127.0.0.1", 6060);
    //EndPoint serverEp("::1", 6060);

    // instantiate a sender
    TCPConnector connector;

    // false: does not start the send thread, and does not deQ and send messages via the send blocking Queue
    // note: when false, user must call SendMessage(), not PostMessage() which enQ into the blocking Queue
    connector.UseSendQueue(false);
   
   // false: does not start the receive thread, and does not enQ  messages into the receive blocking Queue
   // note: when false, user must call ReceiveMessage, not GetMessage() which deQ from the blocking Qeueue
    connector.UseReceiveQueue(false);

    int wait_secs = 1;         // wait time between connection attempts
    int verbose_level = 1;     // verbose (display connect attempts)
    int connect_attempts = 10; // make 10 connection attempts

    /*
      uncomment to test/see socket exception handling
    try
    {
        sender.Connect(serverEp); // -- makes one attempt to connect
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }

    if(sender.IsConnected())
    */

    if (connector.ConnectPersist(serverEp, connect_attempts, wait_secs, verbose_level) < 10)
    {
        //handle messages received in separate thread
        std::thread receiverProc(ReceiveProc, &connector);

        int num_messages = 10;
        // loop to send 10 messages
        for (int j = 0; j < num_messages; j++)
        {
            // build an and print each string message
            std::string strMsg = name + " [ Message #: " + std::to_string(j + 1) + " ]";
            //MessagePtr msg(new Message(strMsg.c_str(), (int)strMsg.length(), MessageType::STRING));
            MessagePtr msg = Message::CreateMessage(strMsg); 
             
            connector.SendMessage(msg);
            std::cout << "Message is: " << *msg << std::endl;
           
            // post message into the send queue
            //connector.PostMessage(msg);
            //std::cout << sender.GetMessage() << std::endl;

            // sleep for a second so we can see what's going on
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        connector.Close();
        receiverProc.join();
    }
    else
        std::cerr << " failed to connect in " << connect_attempts << " attempts " << std::endl;
}

int main()
{
    TestSenderAsync("test 1");

    //int senderCount = 1;
    //std::vector<std::thread> thread_vec;  // store thread is in vector
    // start senderCount (5) concurrent sender threads to test
    /* for(int i = 1; i <= senderCount; ++i)
     {
         std::string name = "Sender: -> " + std::to_string(i);

         // start sender thread, and store it vector
         thread_vec.push_back(std::thread(TestSenderAsync, name));
     }
     */

     // wait for a send threads to complete: using iterator as an example
     /* for(auto iter = thread_vec.begin();  iter != thread_vec.end(); ++iter)
      {
         if(iter->joinable())
            iter->join();
      }
      */

    return 0;
}
#endif
