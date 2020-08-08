
#include <iostream>
#include <chrono>
#include <mpl.h>

using namespace CSE384;
using namespace std::chrono;

class ResponderClientHandler : public FixedSizeMsgClientHander
{
public:
   
   ResponderClientHandler(int msg_size): FixedSizeMsgClientHander(msg_size)
   {}
   // this is a creational function: you must implement it
   // the TCPResponder creates an instance of this class (it's how the server polymorphism works)
   virtual ClientHandler *Clone()
   {
      return new ResponderClientHandler(GetMessageSize());
   }

   // this is where you define the custom server processing: you must implement it
   virtual void AppProc()
   {
      // while there are messages in the blocking queue, and you have seen the disconnect
      // message, pull messages out and dispaly them.
      
      high_resolution_clock::time_point t1 = high_resolution_clock::now();

      MessagePtr msg;
      while ((msg = GetMessage())->GetType() != MessageType::DISCONNECT)
      {   
         std::cout << "Got a message:" << *msg << "from: " << RemoteEP() << std::endl;
         MessagePtr msgPtr = Message::CreateFixedSizeMessage(GetMessageSize(),
                         std::string("Reply from server: ") + GetServiceEndPoint().ToString() 
         );

         // queue reply message
         // PostMessage(msgPtr);

         // send reply message directly (no queue)
         SendMessage(msgPtr);
      }

      high_resolution_clock::time_point t2 = high_resolution_clock::now();
      duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
      std::cout << "Processing time:" << time_span.count() << " seconds." << std::endl;
   }

   ~ResponderClientHandler()
   {
      std::cout << "Handler destroyed" << std::endl;
   }
};

int main(int argc, char *argv[])
{
   // define server endpoint ip address and port for listening
   EndPoint serverEP("127.0.0.1", 6060);
   //EndPoint serverEP("::1", 6060);

   const int MSG_SIZE = 1024;
   
   ResponderClientHandler rh(MSG_SIZE);

   // set TCP socket options
   //TCPSocketOptions sock_opts(SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR));
   TCPSocketOptions sock_opts(SOL_SOCKET, (SO_REUSEADDR));

   // define instance of the TCPResponder (server host)
   TCPResponder responder(serverEP, &sock_opts);

   // register the custom client handler with TCPResponder instance
   responder.RegisterClientHandler(&rh);

   // start the server listening thread
   responder.Start();
   std::cout << "Server listening on: " << serverEP << std::endl;
   std::cout << "Press any key to quit" << std::endl;
   ::getchar();

   // stop the listener and quit
   responder.Stop();

   exit(0);
}