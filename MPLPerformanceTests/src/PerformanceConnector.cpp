
#include <string>
#include <vector>
#include <iostream>
#include <mpl.h>
#include <chrono>

using namespace CSE384;
using namespace std::chrono;

const int MSG_SIZE = 1024;

inline void SyncPrint(const std::string& message)
{
   static std::mutex ioLock;
   std::lock_guard<std::mutex> l(ioLock);
   std::cout << message << std::endl;
}

void ReceiverProc(TCPConnector *connector)
{
   SyncPrint("Begin Receiving messages of size " + std::to_string(MSG_SIZE)); 
   high_resolution_clock::time_point t1 = high_resolution_clock::now();
   MessagePtr msg;
   int count = 0;
   while ((msg = connector->ReceiveMessage())->GetType() != MessageType::DISCONNECT)
   //while ((msg = connector->GetMessage())->GetType() != MessageType::DISCONNECT)
   {
     ++count; // std::cout << msg->ToString() << std::endl;
   }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    SyncPrint("Finished Receiving messages: " + std::to_string(count)  + " of size " + std::to_string(MSG_SIZE)); 
    SyncPrint("Receive Processing time    : " + std::to_string(time_span.count()) +  " seconds.");
    SyncPrint("Receive Thoughput          : " + std::to_string(((unsigned long long) count * (MSG_SIZE * 8)) / time_span.count()) +  " bits/sec");
}



int main()
{
   // specify the server Endpoint we wish to connect
   EndPoint serverEp("127.0.0.1", 6060);
   //EndPoint serverEp("::1", 6060);

   // instantiate a sender
   FixedSizeMsgConnector perfConnector(MSG_SIZE);

   int wait_secs = 1;         // wait time between connection attempts
   int verbose_level = 1;     // verbose (display connect attempts)
   int connect_attempts = 10; // make 10 connection attempts

   perfConnector.UseReceiveQueue(false);
   perfConnector.UseSendQueue(false);
   
   if (perfConnector.ConnectPersist(serverEp, connect_attempts, wait_secs, verbose_level) < connect_attempts)
   {
      //handle messages received in separate thread
      std::thread receiverProc(ReceiverProc, &perfConnector);

      int num_messages = 1000000;
      SyncPrint("Begin Sending: " + std::to_string(num_messages)  + " of size " + std::to_string(MSG_SIZE)); 
      high_resolution_clock::time_point t1 = high_resolution_clock::now();
      // loop to send messages
      for (int j = 0; j < num_messages; j++)
      {
         MessagePtr msg = Message::CreateFixedSizeMessage(MSG_SIZE, "[ Message #: " + std::to_string(j + 1) + " ]");
         // std::cout << "Message is: " << *msg << std::endl;

         // post message into the send queue
         // perfConnector.PostMessage(msg);

         //send message directly (no queue)
         perfConnector.SendMessage(msg);
         //std::cout << sender.GetMessage() << std::endl;

         // sleep for a second so we can see what's going on
         //std::this_thread::sleep_for(std::chrono::seconds(1));
      }
      high_resolution_clock::time_point t2 = high_resolution_clock::now();
      duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
      perfConnector.Close();
      SyncPrint("Finished Sending     : " + std::to_string(num_messages)  + " of size " + std::to_string(MSG_SIZE)); 
      SyncPrint("Send Processing time : " + std::to_string(time_span.count()) +  " seconds.");
      SyncPrint("Send Thoughput       : " + std::to_string(((unsigned long long) num_messages * (MSG_SIZE * 8)) / time_span.count()) +  " bits/sec");

      receiverProc.join();
   }
   else
      std::cerr << " failed to connect in " << connect_attempts << " attempts " << std::endl;

   return 0;
}