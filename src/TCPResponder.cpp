#include "TCPResponder.h"
#include "ReceiverExceptions.h"

namespace CSE384
{
   TCPResponder::TCPResponder(const EndPoint &ep, TCPSocketOptions *sc) : ServiceEP(ep),
                                                                          sc_(sc),
                                                                          ch_(nullptr),
                                                                          islistening_(false),
                                                                          useClientRecvQueue_(true),
                                                                          useClientSendQueue_(true),
                                                                          num_clients_(-1)
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
         int client_count = 0;
         // set socket listening
         listenSocket_.Listen(backlog);
         IsListening(true);

         // std::vector<std::thread> serviceQ_;
         
         //Dr. Fawcett's thread pool hard wired to 8 threads (I think?)
         ThreadPool<8> threadPool_;

         // loop around accepting)
         while (IsListening() && (client_count++ < NumClients() || NumClients() == -1))
         {
             // ---accept a connection (creating a data pipe)---
             TCPSocket client_socket = listenSocket_.Accept();

             if (client_socket.IsValid())
             {
                 if (ch_ != nullptr)
                 {
                     // clone the registered client hander, and hand the socket to the client handler
                     ClientHandler* ch = ch_->Clone();

                     // give service end point the client handler instance
                     ch->SetServiceEndPoint(ServiceEP);

                     // give the TCPSocket to the client handler instance
                     ch->SetSocket(client_socket);

                     // service the current client request using a threadPool thread
                     ThreadPool<8>::CallObj service_client = [this, ch]() -> bool
                     {
                         ServiceClient(ch);
                         return true;
                     };
                     threadPool_.workItem(service_client);
                    


                     // spawn a thread to service the current client request
                     // serviceQ_.push_back(std::thread(&TCPResponder::ServiceClient, this, ch));
                 }
                 else
                 {
                     throw ReceiverNoRegisteredClientHandlerException();
                 }
             }
         }

         /* for(int i = 0; i < serviceQ_.size(); ++i)
         {
             if (serviceQ_[i].joinable())
                 serviceQ_[i].join();
         }
         */

          ThreadPool<8>::CallObj exit = []() ->bool { return false; };
          threadPool_.workItem(exit);
          threadPool_.wait();
         
      }
      catch (const std::exception)
      {
         IsListening(false);
      }
   }

   void TCPResponder::ServiceClient(ClientHandler* ch)
   {
       std::thread clientThread;
       try
       {
           //start the client processing thread, if use specifies to 
           if (UseClientReceiveQueue())
               ch->StartReceiving();

           // start the send thread
           if (UseClientSendQueue())
               ch->StartSending();

           // start the user defined AppProc() on a new thread
           // std::thread app_thread_ = std::thread(&ClientHandler::AppProc, ch);

           // recycle this (current) thread to run the user define AppProc
           ch->AppProc();
       }
       catch (std::exception& ex)
       {
           std::cerr << ex.what() << std::endl;
       }

       try
       {
           //wait for the receive thread to shutdown
           if (UseClientReceiveQueue())
               ch->StopReceiving();
           ch->ShutdownRecv();

           // stop the send thread
           if (UseClientSendQueue())
               ch->StopSending();
           ch->ShutdownSend();

           // signal client to shutdown
           ch->Close();
       }
       catch (const std::exception& e)
       {
           std::cerr << e.what() << std::endl;
       }

       delete ch;
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
            if(listenThread_.joinable())
               listenThread_.join();

            listenSocket_.Close(); 
            IsListening(false);
         }
         catch(...)
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

      Message msg;
      // when UseReceiveQueue == false, then we must ReceiveMessage instead of GetMessage()
      while ((msg = GetMessage()).get_type() != MessageType::DISCONNECT)
      //while ((msg = ReceiveMessage())->GetType() != MessageType::DISCONNECT)
      {
         std::cout << "Got a message:" << msg << "from:" << RemoteEP() << std::endl;      
         //SendMessage(Message::CreateMessage(std::string("Reply from server: ") + GetServiceEndPoint().ToString(), MessageType::DEFAULT));
         Message msg = Message::create_msg_str_fit( std::string("Reply from server: ") + GetServiceEndPoint().ToString());
         //std::cout << msg.get_content_str() << std::endl;
         PostMessage(msg);
      } 

     /* while ((msg = GetMessage())->GetType() != MessageType::DISCONNECT)
      {
         std::cout << "Got a message:" << *msg << "from:" << RemoteEP() << std::endl;
         PostMessage(MessagePtr(new Message(std::string("Reply from server: ") + GetServiceEndPoint().ToString())));
      }
      */  
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

   // false: does not start the receive thread, and does not enQ  messages into the receive blocking Queue
   // note: when false, user must call ReceiveMessage() in the ClientHandler AppProc(), not GetMessage()
   //responder.UseClientReceiveQueue(false);
   
   // false: does not start the send thread, and does not deQ messages and send via the send blocking Queue
   // note: when false, user must call SendMessage() in the ClientHandler AppProc(), not PostMessage() which enQ into the send blocking Queue
   //responder.UseClientSendQueue(false);
   // responder.UseClientSendReceiveQueues(false);

   // shutdown the listener after one client connection
   responder.NumClients(5);  

   // start the server listening thread
   responder.Start();
   std::cout << "Server listening on: " << serverEP << std::endl;
   std::cout << "Press any key to quit" << std::endl;
  // ::getchar();

   // stop the listener and quit
   responder.Stop();

  // exit(0);
}
#endif
