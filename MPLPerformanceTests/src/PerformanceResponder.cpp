
#include <iostream>
#include <chrono>
#include <mpl.h>

using namespace CSE384;
using namespace std::chrono;

const int MSG_SIZE = 1024;

inline void SyncPrint(const std::string &message)
{
   static std::mutex ioLock;
   std::lock_guard<std::mutex> l(ioLock);
   std::cout << message << std::endl;
}

void SendProc(FixedSizeMsgClientHander *handler)
{
   int num_messages = 1000000;
   SyncPrint("Begin Sending: " + std::to_string(num_messages) + " of size " + std::to_string(MSG_SIZE));
   high_resolution_clock::time_point t1 = high_resolution_clock::now();
   // loop to send messages
   for (int j = 0; j < num_messages; j++)
   {
      handler->SendMessage(Message(MSG_SIZE, "[ Message #: " + std::to_string(j + 1) + " ]", MessageType::DEFAULT));
   }
   high_resolution_clock::time_point t2 = high_resolution_clock::now();
   duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
   SyncPrint("Finished Sending     : " + std::to_string(num_messages) + " of size " + std::to_string(MSG_SIZE));
   SyncPrint("Send Processing time : " + std::to_string(time_span.count()) + " seconds.");
   SyncPrint("Send Thoughput       : " + std::to_string(((unsigned long long)num_messages * (MSG_SIZE * 8)) / time_span.count()) + " bits/sec");
}

class ResponderClientHandler : public FixedSizeMsgClientHander
{
public:
   ResponderClientHandler(int msg_size) : FixedSizeMsgClientHander(msg_size)
   {
   }

   // this is a creational function: you must implement it
   // the TCPResponder creates an instance of this class (it's how the server polymorphism works)
   virtual ClientHandler *Clone()
   {
      return new ResponderClientHandler(GetMessageSize());
   }

   // this is where you define the custom server processing: you must implement it
   virtual void AppProc()
   {
      // handle messages sent to TCPConnector (client) in separate thread
      std::thread senderProc(::SendProc, this);

      // while there are messages in the blocking queue, and you have seen the disconnect
      // message, pull messages out and dispaly them.
      int count = 0;
      high_resolution_clock::time_point t1 = high_resolution_clock::now();


      Message msg;
      while ((msg = ReceiveMessage()).GetType() != MessageType::DISCONNECT)
      //while ((msg = GetMessage())->GetType() != MessageType::DISCONNECT)
      {
         // std::cout << "Got a message:" << *msg << "from: " << RemoteEP() << std::endl;
         ++count;
         Message msg(GetMessageSize(), std::string("Reply from server: ") + GetServiceEndPoint().ToString(), MessageType::DEFAULT);

         // queue reply message
         // PostMessage(msgPtr);

         // send reply message directly (no queue)
         //SendMessage(msgPtr);
      }

      high_resolution_clock::time_point t2 = high_resolution_clock::now();
      duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
      SyncPrint("Finished Receiving messages: " + std::to_string(count) + " of size " + std::to_string(MSG_SIZE));
      SyncPrint("Receive Processing time    : " + std::to_string(time_span.count()) + " seconds.");
      SyncPrint("Receive Thoughput          : " + std::to_string(((unsigned long long)count * (MSG_SIZE * 8)) / time_span.count()) + " bits/sec");

      senderProc.join();
   }

   ~ResponderClientHandler()
   {
      std::cout << "Handler destroyed" << std::endl;
   }
};

int main(int argc, char *argv[])
{
   // define server endpoint ip address and port for listening
   EndPoint serverEP("127.0.0.1", 8080);
   //EndPoint serverEP("::1", 6060);

   ResponderClientHandler rh(MSG_SIZE);

   // set TCP socket options
   //TCPSocketOptions sock_opts(SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR));
   TCPSocketOptions sock_opts(SOL_SOCKET, (SO_REUSEADDR));

   // define instance of the TCPResponder (server host)
   TCPResponder responder(serverEP, &sock_opts);

   responder.UseClientSendQueue(false);
   responder.UseClientReceiveQueue(false);

   // register the custom client handler with TCPResponder instance
   responder.RegisterClientHandler(&rh);

  // number of clients to service before exiting
   responder.NumClients(1);

   // start the server listening thread
   responder.Start();
   std::cout << "Server listening on: " << serverEP << std::endl;
   std::cout << "Press any key to quit" << std::endl;
   //std::cin.get();

   // stop the listener and quit
   responder.Stop(); 
}