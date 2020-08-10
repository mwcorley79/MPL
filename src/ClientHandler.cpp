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

         while (IsReceiving())
         {
            MessagePtr msg = RecvSocketMessage();
            recv_queue_.enQ(msg);
            if (msg->GetType() == MessageType::DISCONNECT)
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

      // std::cout << "recv thread shutdown" << std::endl;
   }

   // serialize the message header and message and write them into the socket
   MessagePtr ClientHandler::RecvSocketMessage()
   {
      struct MSGHEADER mhdr;
      int recv_bytes;
      // receive fixed size message header (see wire protocol in Message.h)
      if ((recv_bytes = data_socket.Recv((const char *)&mhdr, sizeof(MSGHEADER))) == sizeof(MSGHEADER))
      {
         // *** MUST convert message header to host byte order (e.g. Intel CPU == little endian)
         mhdr.ToHostByteOrder();

         //construct a Message using the Message header read from the socket channel
         // *** critical that mhdr is host byte order ****
         MessagePtr msgPtr(new Message(mhdr));

         // send message data
         if (data_socket.Recv(msgPtr->GetData(), msgPtr->Length()) == -1)
            throw ReceiverReceiveMessageDataException(getlasterror_portable());

         return msgPtr;
      }

      // if read zero bytes, thyen this is the zero length message signaling client shutdown
      if (recv_bytes == 0)
      {
         return MessagePtr(new Message(nullptr, 0, DISCONNECT));
      }
      else
      {
         throw ReceiverReceiveMessageHeaderException(getlasterror_portable());
      }
   }

   // serialize the message header and message and write them into the socket
   void ClientHandler::SendSocketMessage(const MessagePtr &msg)
   {
      // convert the wire protocol (message header) to big endian (network byte order)
      //struct MSGHEADER mhdr =  *(const_cast<Message&>(msg).GetHeader());
      //mhdr.ToNetworkByteOrder();
    
      msg->GetHeader()->ToNetorkByteOrder();
      if (data_socket.Send(msg->GetRawMsg(), msg->RawMsgLength()) == -1)
      {
         msg->GetHeader()->ToHostByteOrder();
         throw SenderTransmitMessageDataException(getlasterror_portable());
      }

      msg->GetHeader()->ToHostByteOrder();
      // send message header
      //if(data_socket.Send((const char*) &mhdr, sizeof(struct MSGHEADER)) == -1)
      // throw SenderTransmitMessageHeaderException(getlasterror_portable());

      // send message data
      //if(data_socket.Send(msg.GetData(), msg.Length()) == -1)
      //   throw SenderTransmitMessageDataException(getlasterror_portable());
   }

   void ClientHandler::StartSending()
   {
      if (!IsSending())
      {
         //start the send the send thread
         IsSending(true);
         sendThread = std::thread(&ClientHandler::sendProc, this);
      }
   }

   void ClientHandler::StopSending()
   {
      if (IsSending())
      {
         //note: only gets deposited into queue if IsSending is true
         MessagePtr stopMsg(new Message(nullptr, 0, STOP_SENDING));
         send_bq_.enQ(stopMsg);

         // make the calling thread wait for the send thread to finish
         if (sendThread.joinable())
            sendThread.join();
      }
   }

   // dedicated thread function that deques messages
   // from the blocking queue and writes them into the socket
   void ClientHandler::sendProc()
   {
      try
      {
         while (IsSending())
         {
            // deque the next message
            MessagePtr msg = send_bq_.deQ();

            // if this is the stop sending message, signal
            // the send thread to shutdown
            if (msg->GetType() == STOP_SENDING)
            {
               IsSending(false);
            }
            else
            {
               // serialize the message into the socket
               SendSocketMessage(msg);
            }
         }

         //std::cout << "send thread shutdown" << std::endl;
      }
      catch (...)
      {
         IsSending(false);
      }
   }

   FixedSizeMsgClientHander::FixedSizeMsgClientHander(int msg_size) : msg_size_(msg_size)
   {
   }

   // serialize the message header and message and write them into the socket
   MessagePtr FixedSizeMsgClientHander::RecvSocketMessage()
   {
      // create the fixed message recieve that message size from the socket
      MessagePtr msgPtr = Message::CreateEmptyFixedSizeMessage(msg_size_);
      int recv_bytes;
      if ((recv_bytes = GetDataSocket().Recv(msgPtr->GetRawMsg(), msgPtr->RawMsgLength())) == msgPtr->RawMsgLength())
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

   // serialize the message header and message and write them into the socket
   /* void FixedSizeMsgClientHander::SendSocketMessage(const MessagePtr &msg)
   {
      msg->GetHeader()->ToNetorkByteOrder();
      if (GetDataSocket().Send(msg->GetRawMsg(), msg->RawMsgLength()) == -1)
         throw SenderTransmitMessageDataException(getlasterror_portable());
   }
   */

} // namespace CSE384
