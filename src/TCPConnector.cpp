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
            IsSending(true);

            Message msg = send_bq_.deQ();
            // if this is the stop sending message, signal
            // the send thread to shutdown
            while (msg.get_type() != STOP_SENDING)
            {
                // serialize the message into the socket
                SendSocketMessage(msg);
                // deque the next message
                msg = send_bq_.deQ();            
            }
        }
        catch (...)
        {
            // IsSending(false);
        }
    }

    // serialize the message header and message and write them into the socket
    void TCPConnector::SendSocketMessage(const Message &msg)
    {
        // send message header
        if(socket.Send( (const char*) msg.get_raw_ref(), HEADER_SIZE, 0,1) == -1)
        {
           throw SenderTransmitMessageHeaderException(getlasterror_portable());
        }
    
        if(socket.Send( (const char*) msg.get_content_bytes(), msg.get_content_len() ,0,1) == -1)
            throw SenderTransmitMessageDataException(getlasterror_portable());
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
            Message msg;
            IsReceiving(true);
            do
            {
                msg = RecvSocketMessage();
                recv_queue_.enQ(msg);
            } 
            while(msg.get_type() != MessageType::DISCONNECT);
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
            // IsReceiving(false);
        }
    }


    // serialize the message header and message and write them into the socket
    Message TCPConnector::RecvSocketMessage()
    {
        int recv_bytes;
        //char header_buf[HEADER_SIZE];

        Message msg;

        // receive fixed size message header (see wire protocol in Message.h)
        if ((recv_bytes = socket.Recv( (const char*) msg.get_raw_ref(), HEADER_SIZE, MSG_WAITALL, 1)) == HEADER_SIZE)
        {
            // build message directly from the header
            // Message msg( (u8*) header_buf);
         
            // receive the message data
            if (socket.Recv( (const char *) msg.get_content_bytes(), msg.get_content_len(), MSG_WAITALL, 1) == -1)
                throw ReceiverReceiveMessageDataException(getlasterror_portable());

            return msg;
        }

        // if read zero bytes, then this is the zero length message signaling client shutdown
        if (recv_bytes == 0)
        {
            return Message(DISCONNECT);
        }
        else
        {
            throw ReceiverReceiveMessageHeaderException(getlasterror_portable());
        }
    }

    void TCPConnector::StopSending()
    {
        try
        {
            if (IsSending())
            {
                //note: only gets deposited into queue if IsSending is true
                Message StopMsg;
                StopMsg.set_type(STOP_SENDING);
                PostMessage(StopMsg);

                if (send_thread_.joinable())
                    send_thread_.join();

                IsSending(false);
            }
        }
        catch (...)
        {
            IsSending(false);
        }
    }

    void TCPConnector::StopReceiving()
    {
        try
        {
            if (IsReceiving())
            {
                if (recvThread.joinable())
                    recvThread.join();

                IsReceiving(false);
            }
        }
        catch (...)
        {
            IsReceiving(false);
        }
    }
    

    bool TCPConnector::Close(std::thread* listener)
    {
        bool ret = false;
        if (IsConnected())
        {
            if (UseSendQueue())
                StopSending();
            socket.ShutdownSend();

            if (listener)
                if (listener->joinable())
                    listener->join();

            if (UseReceiveQueue())
                StopReceiving();
            socket.ShutdownRecv();

            ret = (socket.Close() == 0);
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

    void FixedSizeMsgConnector::SendSocketMessage(const Message &msg)
    {   
        if (socket.Send( (const char*) msg.get_raw_ref(), msg_size_ ,0,1) == -1)
        {
            throw SenderTransmitMessageDataException(getlasterror_portable());
        }
    }
    
    Message FixedSizeMsgConnector::RecvSocketMessage()
    {
        Message msg(msg_size_);
        int recv_bytes;

        if((recv_bytes = socket.Recv( (const char*) msg.get_raw_ref(), msg_size_, MSG_WAITALL,1 )) == msg_size_)
        {
           return msg;
        }
        
        // if read zero bytes, then this is the zero length message signaling client shutdown
        if (recv_bytes == 0)
        {
            return Message(DISCONNECT);
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


using namespace CSE384;

void ReceiveProc(TCPConnector *connector)
{
    Message msg;
    //while ((msg = connector->GetMessage())->GetType() != MessageType::DISCONNECT)
    while ((msg = connector->ReceiveMessage()).get_type() != MessageType::DISCONNECT)
    {
       std::cout << msg.get_content_str() << std::endl;
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
            
            Message msg = Message::create_msg_str_fit(strMsg);
            //Message msg = Message(strMsg, MessageType::DEFAULT);

            connector.SendMessage(msg);
          
            // post message into the send queue
            //connector.PostMessage(msg);
            //std::cout << sender.GetMessage() << std::endl;

            // sleep for a second so we can see what's going on
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        connector.Close(&receiverProc);
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
