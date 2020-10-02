#include "ClientHandler.h"
#include "ReceiverExceptions.h"
#include "SenderExceptions.h"

namespace CSE384
{
    void ClientHandler::StartReceiving()
    {
        if (!IsReceiving())
        {
            //start the send the send thread
            IsReceiving(true);

            // RecvProc is a dedicated thread for servicing the socket
            // by pulling out messages and enQing in the recv blocking queue
            recvThread = std::thread(&ClientHandler::RecvProc, this);
        }
    }

    void ClientHandler::StopReceiving()
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

        // data_socket.ShutdownRecv();
    }

    void ClientHandler::ShutdownRecv()
    {
        data_socket.ShutdownRecv();
    }

    void ClientHandler::RecvProc()
    {
        try
        {
            IsReceiving(true);
            Message msg;
            do
            {
                msg = RecvSocketMessage();
                recv_queue_.enQ(msg);
            } 
            while (msg.get_type() != MessageType::DISCONNECT);
        }
        catch (const std::exception &ex)
        {
            std::cerr << ex.what() << std::endl;
            // IsReceiving(false);
        }
    }

    // serialize the message header and message and write them into the socket
    Message ClientHandler::RecvSocketMessage()
    {
        int recv_bytes;
        char header_buf[HEADER_SIZE];
      
        // receive fixed size message header (see wire protocol in Message.h)
        if((recv_bytes = data_socket.Recv(header_buf, HEADER_SIZE, MSG_WAITALL, 1)) == HEADER_SIZE)
        {
            // build message (and space for the message content) directly from the header
            Message msg((u8*) header_buf);

            // receive the message data
            if(data_socket.Recv((const char *) msg.get_content_bytes(), msg.get_content_len(), MSG_WAITALL, 1) == -1)
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

    // serialize the message header and message and write them into the socket
    void ClientHandler::SendSocketMessage(const Message &msg)
    {
        // send message header
        if(data_socket.Send( (const char*) msg.get_raw_ref(), HEADER_SIZE, 0, 1) == -1)
        {
           throw SenderTransmitMessageHeaderException(getlasterror_portable());
        }
    
        if(data_socket.Send( (const char*) msg.get_content_bytes(), msg.get_content_len(),0,1) == -1)
            throw SenderTransmitMessageDataException(getlasterror_portable());
    }

    void ClientHandler::StartSending()
    {
        if (!IsSending())
        {
            //start the send the send thread

            IsSending(true);

            // dedicated thread function that deques messages
            // from the blocking queue and writes them into the socket
            sendThread = std::thread(&ClientHandler::SendProc, this);
        }
    }

    void ClientHandler::SendProc()
    {
        try
        {
            IsSending(true);
            Message msg = send_bq_.deQ();

            // if this is the stop sending message, signal
            // the send thread to shutdown
            while (msg.get_type() != STOP_SENDING)
            {
                // deque the next message
                SendSocketMessage(msg);
                msg = send_bq_.deQ();
            }
        }
        catch (...)
        {
            // IsSending(false);
        }
    }

    void ClientHandler::StopSending()
    {
        try
        {
            if (IsSending())
            {
                //note: only gets deposited into queue if IsSending is true
                PostMessage( Message(STOP_SENDING));

                // make the calling thread wait for the send thread to finish
                if (sendThread.joinable())
                    sendThread.join();

                IsSending(false);
            }
        }
        catch (...)
        {
            IsSending(false);
        }

        // data_socket.ShutdownSend();
    }

    void ClientHandler::ShutdownSend()
    {
        data_socket.ShutdownSend();
    }

    FixedSizeMsgClientHander::FixedSizeMsgClientHander(int msg_size) : msg_size_(msg_size)
    {
    }

    // serialize the message header and message and write them into the socket
    Message FixedSizeMsgClientHander::RecvSocketMessage()
    {
        Message msg(msg_size_);
        int recv_bytes;

        if((recv_bytes = GetDataSocket().Recv( (const char*) msg.get_raw_ref(), msg_size_, MSG_WAITALL,1 )) == msg_size_)
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

    // serialize the message header and message and write them into the socket
    void FixedSizeMsgClientHander::SendSocketMessage(const Message &msg)
    {
        // create the fixed message recieve that message size from the socket
        if (GetDataSocket().Send( (const char*) msg.get_raw_ref(), msg_size_,0,1) == -1)
        {
            throw SenderTransmitMessageDataException(getlasterror_portable());
        }
    }

} // namespace CSE384
