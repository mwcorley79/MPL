#include "ClientHandler.h"

#include "ReceiverExceptions.h"
#include "SenderExceptions.h"

namespace CSE384
{
    // RecvProc is a dedicated thread for servicing the socket
    // by pulling out messages and enQing in the recv blocking queue
    void ClientHandler::RecvProc()
    {
        try
        {
          IsReceiving(true);

          while(IsReceiving())
          {
             Message msg = RecvSocketMessage();
             recv_queue_.enQ(msg);
             if(msg.GetType() == MessageType::DISCONNECT)
             {
                IsReceiving(false);
             }
          }
        }
        catch(const std::exception& ex)
        {
           std::cerr << ex.what() << std::endl;
           IsReceiving(false);
        }
    }

    // serialize the message header and message and write them into the socket
    Message ClientHandler::RecvSocketMessage()
    {   
        struct MSGHEADER mhdr;
        int recv_bytes;
        // receive fixed size message header (see wire protocol in Message.h)
        if((recv_bytes = data_socket.Recv((const char*) &mhdr, sizeof(MSGHEADER))) == sizeof(MSGHEADER))
        {
          // *** MUST convert message header to host byte order (e.g. Intel CPU == little endian)
          mhdr.ToHostByteOrder();
        
          //construct a Message using the Message header read from the socket channel
          // *** critical that mhdr is host byte order ****
          Message msg(mhdr);
        
          // send message data
          if(data_socket.Recv(msg.GetData(), msg.Length()) == -1)
            throw ReceiverReceiveMessageDataException(getlasterror_portable());
        
          return msg;
        }

        // if read zero bytes, thyen this is the zero length message signaling client shutdown
        if(recv_bytes == 0)
        {
          return Message(NULL,0,DISCONNECT);
        }
        else
        {
          throw ReceiverReceiveMessageHeaderException(getlasterror_portable());
        }
    }

    // serialize the message header and message and write them into the socket
   void ClientHandler::SendSocketMessage(const Message& msg)
   {
       // convert the wire protocol (message header) to big endian (network byte order)
       struct MSGHEADER mhdr =  *(const_cast<Message&>(msg).GetHeader());
       mhdr.ToNetworkByteOrder();

      // send message header
      if(data_socket.Send((const char*) &mhdr, sizeof(struct MSGHEADER)) == -1)
        throw SenderTransmitMessageHeaderException(getlasterror_portable());

      // send message data
      if(data_socket.Send(msg.GetData(), msg.Length()) == -1)
         throw SenderTransmitMessageDataException(getlasterror_portable());
   }

   void ClientHandler::StartSending()
   {
      if(!IsSending())
      {    
         //start the send the send thread
         IsSending(true);
         sendThread = std::thread(&ClientHandler::sendProc, this);
      }
   }

   void ClientHandler::StopSending()
   {
      if(IsSending())
      {
        //note: only gets deposited into queue if IsSending is true
        Message stopMsg(nullptr,0,STOP_SENDING);
        send_bq_.enQ(stopMsg);

        // make the calling thread wait for the send thread to finish
        if(sendThread.joinable())
          sendThread.join();
      }
   }

   // dedicated thread function that deques messages 
   // from the blocking queue and writes them into the socket
   void ClientHandler::sendProc()
   {
      try
      {
         //IsSending(true);

         while(IsSending())
         {
            // deque the next message
            Message msg = send_bq_.deQ();
          
            // if this is the stop sending message, signal 
            // the send thread to shutdown
            if(msg.GetType() == STOP_SENDING)
            {
              IsSending(false);
            }
            else
            {
               // serialize the message into the socket
               SendSocketMessage(msg);
            }     
         }
      }
      catch(...)
      {
         IsSending(false);
      }
   }
}
