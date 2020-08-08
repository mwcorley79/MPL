#include "TCPResponder.h"
#include "ReceiverExceptions.h"

namespace CSE384
{
   TCPResponder::TCPResponder(const EndPoint &ep, TCPSocketOptions *sc) : ServiceEP(ep),
                                                                          sc_(sc),
                                                                          ch_(nullptr),
                                                                          islistening_(false)
   {
      listenSocket_.Bind(ep, sc);
   }

   void TCPResponder::Start(int backlog)
   {
      if (!IsListening())
      {
         IsListening(true);
         listenThread_ = std::thread(&TCPResponder::ListenThreadProc, this, backlog);
      }
   }

   void TCPResponder::ListenThreadProc(int backlog)
   {
      try
      {
         // set socket listening
         listenSocket_.Listen(backlog);

         // loop around accepting
         while (IsListening())
         {
            // ---accept a connection (creating a data pipe)---
            TCPSocket client_socket = listenSocket_.Accept();

            if (client_socket.IsValid())
            {
               if (ch_ != nullptr)
               {
                  // clone the registered client hander, and hand the socket to the client handler
                  ClientHandler *ch = ch_->Clone();

                  // give service end point the client handler instance
                  ch->SetServiceEndPoint(ServiceEP);

                  // give the TCPSocket to the client handler instance
                  ch->SetSocket(client_socket);

                  // service the current client request using a threadPool thread
                 //ThreadPool<8>::CallObj service_client = [this, ch]() -> bool {
                 //    ServiceClient(ch);
                 //    return true;
                 // };

                 // threadPool_.workItem(service_client);

                  // spawn a thread to service the current client request
                  std::thread clientThread = std::thread(&TCPResponder::ServiceClient, this, ch);
                  clientThread.detach();
               }
               else
               {
                  throw ReceiverNoRegisteredClientHandlerException();
               }
            }
         }
      }
      catch (const std::exception)
      {
         IsListening(false);
      }
   }

   void TCPResponder::ServiceClient(ClientHandler *ch)
   {
      // make sure the client handler resources are freed
      if (ch != nullptr)
      {
         std::thread clientThread;
         try
         {
            //start the client processing thread
            clientThread = std::thread(&ClientHandler::RecvProc, ch);

            // start the send thread
            ch->StartSending();

            // start the user defined AppProc() on a new thread
            // std::thread app_thread_ = std::thread(&ClientHandler::AppProc, ch);

            // recycle this (current) thread to run the user define AppProc
            ch->AppProc();
         }
         catch (std::exception &ex)
         {
            std::cerr << ex.what() << std::endl;
         }

         try
         {
            if (clientThread.joinable())
               clientThread.join();

            // stop the send thread
            ch->StopSending();

            // signal client to shutdown
            ch->Close();
         }
         catch (const std::exception &e)
         {
            std::cerr << e.what() << std::endl;
         }

         delete ch;
      }
   }

   void TCPResponder::RegisterClientHandler(ClientHandler *ch)
   {
      ch_ = ch;
      ch_->SetServiceEndPoint(ServiceEP);
   }

   void TCPResponder::Stop()
   {
      // if user started the listener, then shut it all down
      if (IsListening())
      {
         try
         {
            IsListening(false);
            listenSocket_.Close();

            // listenThread_.~thread();

            if (listenThread_.joinable())
               listenThread_.join();

            //listenThread_.detach();
         }
         catch (...)
         {
            IsListening(false);
         }
      }
   }

   TCPResponder::~TCPResponder()
   {
      // call Stop on the listening thread in case the client forgot
      Stop();
   }
}; // namespace CSE384

   // ***  TCPResponder TEST STUB ***
#ifdef TEST_RESPONDER
#include <iostream>
#ifdef LINK_MPL
#include <mpl.h>
#endif
/*  Note: use either "SenderTest" for test */
using namespace CSE384;

// this is an example of custom server processing class
class TestClientHandler : public ClientHandler
{
public:
   // this is a creational function: you must implement it
   // the TCPResponder creates an instance of this class (it's how the server polymorphism works)
   virtual ClientHandler *Clone()
   {
      return new TestClientHandler();
   }

   // this is where you define the custom server processing: you must implement it
   virtual void AppProc()
   {
      // while there are messages in the blocking queue, and you have seen the disconnect
      // message, pull messages out and dispaly them.

      MessagePtr msg;
      while ((msg = GetMessage())->GetType() != MessageType::DISCONNECT)
      {
         std::cout << "Got a message:" << *msg << "from:" << RemoteEP() << std::endl;
         PostMessage(MessagePtr(new Message(std::string("Reply from server: ") + GetServiceEndPoint().ToString())));
      }
   }

   ~TestClientHandler()
   {
      std::cout << "Handler destroyed" << std::endl;
   }
};

int main(int argc, char *argv[])
{
   // define server endpoint ip address and port for listening
   EndPoint serverEP("127.0.0.1", 6060);
   //EndPoint serverEP("::1", 6060);

   // define instance of custom server processing (ccustom client handler)
   TestClientHandler th;

   // set TCP socket options
   //TCPSocketOptions sock_opts(SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR));
   TCPSocketOptions sock_opts(SOL_SOCKET, (SO_REUSEADDR));

   // define instance of the TCPResponder (server host)
   TCPResponder responder(serverEP, &sock_opts);

   // register the custom client handler with TCPResponder instance
   responder.RegisterClientHandler(&th);

   // start the server listening thread
   responder.Start();
   std::cout << "Server listening on: " << serverEP << std::endl;
   std::cout << "Press any key to quit" << std::endl;
   ::getchar();

   // stop the listener and quit
   responder.Stop();

   exit(0);
}
#endif
