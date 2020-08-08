
#include <string>
#include <vector>
#include <iostream>
#include <mpl.h>

using namespace CSE384;

void SenderProc(TCPConnector *connector)
{
   MessagePtr msg;
   while ((msg = connector->GetMessage())->GetType() != MessageType::DISCONNECT)
   {
      std::cout << msg->ToString() << std::endl;
   }
}

int main()
{
   // specify the server Endpoint we wish to connect
   EndPoint serverEp("127.0.0.1", 6060);
   //EndPoint serverEp("::1", 6060);

   const int MSG_SIZE = 1024;

   // instantiate a sender
   FixedSizeMsgConnector perfConnector(MSG_SIZE);

   int wait_secs = 1;         // wait time between connection attempts
   int verbose_level = 1;     // verbose (display connect attempts)
   int connect_attempts = 10; // make 10 connection attempts

   if (perfConnector.ConnectPersist(serverEp, connect_attempts, wait_secs, verbose_level) < connect_attempts)
   {
      //handle messages received in separate thread
      std::thread receiverProc(SenderProc, &perfConnector);

      int num_messages = 10;
      // loop to send 10 messages
      for (int j = 0; j < num_messages; j++)
      {
         // build an and print each string message
         std::string strMsg = "[ Message #: " + std::to_string(j + 1) + " ]";
       
         MessagePtr msg = Message::CreateFixedSizeMessage(MSG_SIZE, strMsg);
         std::cout << "Message is: " << *msg << std::endl;

         // post message into the send queue
         perfConnector.PostMessage(msg);
         //std::cout << sender.GetMessage() << std::endl;

         // sleep for a second so we can see what's going on
         std::this_thread::sleep_for(std::chrono::seconds(1));
      }

      perfConnector.Close();
      receiverProc.join();
   }
   else
      std::cerr << " failed to connect in " << connect_attempts << " attempts " << std::endl;

   return 0;
}