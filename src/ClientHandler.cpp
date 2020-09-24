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
            while (msg.GetType() != MessageType::DISCONNECT);
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
            // IsReceiving(false);
        }
    }

   // serialize the message header and message and write them into the socket
   Message ClientHandler::RecvSocketMessage()
   {
      struct MSGHEADER mhdr;
      int recv_bytes;
      // receive fixed size message header (see wire protocol in Message.h)
      if ((recv_bytes = data_socket.Recv((const char *)&mhdr, sizeof(MSGHEADER),MSG_WAITALL,1)) == sizeof(MSGHEADER))
      {
         // *** MUST convert message header to host byte order (e.g. Intel CPU == little endian)
         mhdr.ToHostByteOrder();

         //construct a Message using the Message header read from the socket channel
         // *** critical that mhdr is host byte order ****
         Message msg(mhdr);

         // send message data
         if (data_socket.Recv(msg.GetData(), msg.Length(), MSG_WAITALL,1) == -1)
            throw ReceiverReceiveMessageDataException(getlasterror_portable());

         return msg;
      }

      // if read zero bytes, thyen this is the zero length message signaling client shutdown
      if (recv_bytes == 0)
      {
         return Message(nullptr, 0, DISCONNECT);
      }
      else
      {
         throw ReceiverReceiveMessageHeaderException(getlasterror_portable());
      }
   }

   // serialize the message header and message and write them into the socket
   void ClientHandler::SendSocketMessage(const Message &msg)
   {
      // Note: could do the send in one call (same as for the Fixed message), but choosing 
        // not to here. instead send the fixed size header, followed by the variable length data
        /* 
        msg->GetHeader()->ToNetorkByteOrder();
        if (data_socket.Send(msg->GetRawMsg(), msg->RawMsgLength(),0,1) == -1)
        {
            msg->GetHeader()->ToHostByteOrder();
            throw SenderTransmitMessageDataException(getlasterror_portable());
        }
        msg->GetHeader()->ToHostByteOrder();
        */

        // convert the wire protocol (message header) to big endian (network byte order)
        msg.GetHeader()->ToNetorkByteOrder();  

        // send message header
        if(data_socket.Send( (const char*) msg.GetHeader(), sizeof(struct MSGHEADER), 0,1) == -1)
        {
           msg.GetHeader()->ToHostByteOrder();
           throw SenderTransmitMessageHeaderException(getlasterror_portable());
        }
        msg.GetHeader()->ToHostByteOrder();

        // send message data
        if(data_socket.Send(msg.GetData(), msg.Length(),0,1) == -1)
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
           while (msg.GetType() != STOP_SENDING)
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
               Message stopMsg(nullptr, 0, STOP_SENDING);
               send_bq_.enQ(stopMsg);

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
      // create the fixed message recieve that message size from the socket
      Message msg = Message(msg_size_, msg_size_, MessageType::DEFAULT);
      int recv_bytes;

      if ((recv_bytes = GetDataSocket().Recv(msg.GetRawMsg(), msg.RawMsgLength(), MSG_WAITALL,1)) == msg.RawMsgLength())
      {
         // *** MUST convert message header to host byte order (e.g. Intel CPU == little endian)
         msg.GetHeader()->ToHostByteOrder();
         return msg;
      }

      // if read zero bytes, then this is the zero length message signaling client shutdown
      if (recv_bytes == 0)
      {
         return Message(nullptr, 0, DISCONNECT);
      }
      else
      {
         throw ReceiverReceiveMessageHeaderException(getlasterror_portable());
      }
   }

   // serialize the message header and message and write them into the socket
   void FixedSizeMsgClientHander::SendSocketMessage(const Message &msg)
   {
      msg.GetHeader()->ToNetorkByteOrder();
      if (GetDataSocket().Send(msg.GetRawMsg(), msg.RawMsgLength(),0,1) == -1)
      {
         msg.GetHeader()->ToHostByteOrder();
         throw SenderTransmitMessageDataException(getlasterror_portable());
      }
      msg.GetHeader()->ToHostByteOrder();
   }

} // namespace CSE384
