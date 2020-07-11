#include "Sender.h"
#include "SenderExceptions.h"
#include "ReceiverExceptions.h"
#include "Platform.h"

namespace CSE384 
{
   Sender::Sender(TCPSocketOptions* sc):    sc_(sc),
                                            isSending_(false),
                                            isReceiving_(false)
   {}

   void Sender::PostMessage(const Message& m)
   {  
      send_bq_.enQ(m);  
   }

   void Sender::StartSending()
   {
      if(!IsSending())
      {    
         //start the send the send thread
         IsSending(true);
         send_thread_ = std::thread(&Sender::sendProc, this);
      }
   }

   bool Sender::Start()
   {
      if(IsConnected())
      {
        StartSending();
        StartReceiving();
        return true;
      }
      return false;
   }

   // dedicated thread function that deques messages 
   // from the blocking queue and writes them into the socket
   void Sender::sendProc()
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

   // serialize the message header and message and write them into the socket
   void Sender::SendSocketMessage(const Message& msg)
   {
       // convert the wire protocol (message header) to big endian (network byte order)
       struct MSGHEADER mhdr =  *(const_cast<Message&>(msg).GetHeader());
       mhdr.ToNetworkByteOrder();

      // send message header
      if(socket.Send( (const char*) &mhdr, sizeof(struct MSGHEADER)) == -1)
        throw SenderTransmitMessageHeaderException(getlasterror_portable());

      // send message data
      if(socket.Send(msg.GetData(), msg.Length()) == -1)
         throw SenderTransmitMessageDataException(getlasterror_portable());
   }

   Message Sender::GetMessage()
   {
       return this->recv_queue_.deQ();
   }

   void Sender::StartReceiving()
   {
      if(!IsReceiving())
      {    
         //start the send the send thread
         IsReceiving(true);
         recvThread = std::thread(&Sender::RecvProc, this);
      }
   }

    // RecvProc is a dedicated thread for servicing the socket
    // by pulling out messasges and enQing in the recv blocking queue
    void Sender::RecvProc()
    {
        try
        {
          //IsReceiving(true);

          while(IsReceiving())
          {
             Message msg = RecvSocketMessage();
             recv_queue_.enQ(msg);
             if(msg.GetType() == MessageType::DISCONNECT)
            // if(msg.GetType() == MessageType::STOP_RECEIVING)
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
    Message Sender::RecvSocketMessage()
    {   
        struct MSGHEADER mhdr;
        int recv_bytes;
        // receive fixed size message header (see wire protocol in Message.h)
        if(  (recv_bytes = socket.Recv((const char*) &mhdr, sizeof(MSGHEADER))) == sizeof(MSGHEADER))
        {
          // *** MUST convert message header to host byte order (e.g. Intel CPU == little endian)
          mhdr.ToHostByteOrder();
        
          //construct a Message using the Message header read from the socket channel
          // *** critical that mhdr is host byte order ****
          Message msg(mhdr);
        
          // send message data
          if(socket.Recv(msg.GetData(), msg.Length()) == -1)
            throw ReceiverReceiveMessageDataException(getlasterror_portable());
        
          return msg;
        }

        // if read zero bytes, then this is the zero length message signaling client shutdown
        if(recv_bytes == 0)
        {
          return Message(NULL,0,DISCONNECT);
        }
        else
        {
          throw ReceiverReceiveMessageHeaderException(getlasterror_portable());
        }
    }   


   void Sender::StopSending()
   {
      if(IsSending())
      {
        //note: only gets deposited into queue if IsSending is true
        Message stopMsg(nullptr,0,STOP_SENDING);
        send_bq_.enQ(stopMsg);
      }
   }

   //void Sender::Stop()
   // {
    //    StopSending();
   // }

   /* 
   void Sender::StopReceiving()
   {
      if(IsReceiving())
      {
        //note: only gets deposited into queue if IsSending is true
        Message stopMsg(nullptr,0,STOP_RECEIVING);
        recv_queue_.enQ(stopMsg);
      }
   }
   */

 
   bool Sender::Close()
   {
      bool ret = false;
      if(IsConnected())
      {
         StopSending();
         
         if(send_thread_.joinable())
           send_thread_.join();

         socket.ShutdownSend();
         
         
         //if(ret)
        // {
           if(recvThread.joinable())
             recvThread.join();
        // }

         socket.ShutdownRecv();
         ret = (socket.Close() == 0);

      }

      return ret; 
   }

   Sender::~Sender()
   {
      // call close in case the client forgot
      Close();
   }

   void Sender::Connect(const EndPoint& ep)
   {
	    socket.Connect(ep, sc_);

	    // start send and receive threads
	    Start();
   }



   int Sender::ConnectPersist(const EndPoint& ep,  unsigned retries,
                             unsigned wtime_secs, unsigned vlevel)
   {
     unsigned int runAttempts = 0;
     while(runAttempts++ < retries)
     {
    	 try
    	 {
           if(vlevel)
           {
             std::cout << "Connection attempt # "<< runAttempts <<" of "
                       << retries << " to "<< ep << std::endl;
           }

           Connect(ep);
           break;
    	 }
    	 catch(SenderException)
    	 {
           if(vlevel)
           {
             std::cout << "Failed Attempt: " << runAttempts << std::endl;
           }
           std::this_thread::sleep_for(std::chrono::seconds(wtime_secs));
    	 }
     }
     return runAttempts;
   }

};

// *** SENDER TEST STUB ****
// ** For testing: run against "Receiver test stub **
#ifdef TEST_SENDER
#include <string>
#include <vector>
#include<iostream>
#include <mpl.h>

using namespace CSE384;

void SenderProc(Sender* sender)
{
   Message msg;
   while( (msg = sender->GetMessage()).GetType() != MessageType::DISCONNECT)
   {
      std::cout << msg.ToString() << std::endl;
   }   
}

void TestSenderAsync(const std::string& name)
{
   // announce the sender
   std::cout << name << " started" << std::endl;
   
   // specify the server Endpoint we wish to connect
   EndPoint serverEp("127.0.0.1", 6060);
   //EndPoint serverEp("::1", 6060);
   
   // instantiate a sender 
   Sender sender;

   int wait_secs = 1; // wait time between connection attempts
   int verbose_level = 1;  // verbose (display connect attempts)
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

   if (sender.ConnectPersist(serverEp, connect_attempts, wait_secs, verbose_level) < 10)
   {
       //handle messages received in separate thread
       std::thread receiverProc(SenderProc, &sender);

       int num_messages = 10;
       // loop to send 10 messages 
       for (int j = 0; j < num_messages; j++)
       {
           // build an and print each string message 
           std::string strMsg = name + " [ Message #: " + std::to_string(j + 1) + " ]";
           Message msg(strMsg.c_str(), (int) strMsg.length(),  MessageType::STRING);
           std::cout << "Message is: " << msg << std::endl;

           // post message into the send queue
           sender.PostMessage(msg);
           //std::cout << sender.GetMessage() << std::endl;

           // sleep for a second so we can see what's going on
          std::this_thread::sleep_for(std::chrono::seconds(1));
       }

       sender.Close();
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


