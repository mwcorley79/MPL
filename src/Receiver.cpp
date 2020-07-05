#include "Receiver.h"
#include "ReceiverExceptions.h"



namespace CSE384
{ 
     Receiver::Receiver(const EndPoint& ep, TCPSocketOptions* sc): ServiceEP(ep),
                                                                      sc_(sc), 
                                                                      ch_(nullptr),
                                                                      islistening_(false)
     {               

    	 listenSocket_.Bind(ep, sc);
     }

     void Receiver::Start(int backlog)
     {
        if(!IsListening())
           listenThread_ = std::thread(&Receiver::ListenThreadProc, this, backlog);
     }

     void Receiver::ListenThreadProc(int backlog)
     {
        try
        {
           // set socket listening
        	listenSocket_.Listen(backlog);
            IsListening(true);

            // loop around accepting
            while (IsListening())
            {
            	// ---accept a connection (creating a data pipe)---
            	TCPSocket client_socket = listenSocket_.Accept();
            	if(client_socket.IsValid())
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
                    ThreadPool<8>::CallObj service_client = [this, ch]() -> bool
                    {
                       ServiceClient(ch);
                       return true;
                    };

                    threadPool_.workItem(service_client);

                    // spawn a thread to service the current client request
                    // std::thread clientThread = std::thread(&Receiver::ServiceClient, this, ch, clientfd);
                    // clientThread.detach();
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

      void Receiver::ServiceClient(ClientHandler* ch)
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

         // make sure the client handler resources are freed 
         if(ch != nullptr)
         { 
            if(clientThread.joinable())
               clientThread.join();

            //stop the send thread
            ch->StopSending();

            // signal client to shutdown
            ch->Close();
            delete ch;
         }
      }

      void Receiver::RegisterClientHandler(ClientHandler* ch)
      {
         ch_ = ch;
         ch_->SetServiceEndPoint(ServiceEP);
      }
 
      void Receiver::Stop()
      {
         // if user started the listener, then shut it all down
         if(IsListening())
         {
            try
            {
               IsListening(false);
               listenSocket_.Close();
               listenThread_.detach();
            }
            catch (...)
            {
               IsListening(false);
            }
         }
      }

      Receiver::~Receiver()
      {
         // call Stop on the listening thread in case the client forgot
         Stop();
      }
 };

// ***  RECEIVER TEST STUB ***
 #ifdef TEST_RECEIVER
  /*  Note: use either "SenderTest" for test */
   using namespace CSE384;
  
   // this is an example of custom server processing class
   class TestClientHandler : public ClientHandler
   {
       public:

       // this is a creational function: you must implement it
       // the Receiver creates an instance of this class (it's how the server polymorphism works) 
       virtual ClientHandler* Clone()
       {
          return new TestClientHandler();
       }

      
       // this is where you define the custom server processing: you must implement it
       virtual void AppProc()
       {
          // while there are messages in the blocking queue, and you have seen the disconnect
          // message, pull messages out and dispaly them.
          //StartSending();
          
          Message msg;
          while( (msg = GetMessage()).GetType() != MessageType::DISCONNECT)
          {
             std::cout << "Got a message from: " << msg << std::endl;
             PostMessage(Message( std::string("Reply from server: ") +  GetServiceEndPoint().ToString()));
          }

          // StopSending();
       }

       ~TestClientHandler()
       {
          std::cout << "Handler destroyed" << std::endl;
       }
   };


   int main(int argc, char* argv[])
   {   
      // SocketSystem ss;
      // define server endpoint ip address and port for listening
      // EndPoint serverEP("127.0.0.1", 6060);
      EndPoint serverEP("::1", 6060);

      // define instance of custom server processing (ccustom client handler) 
      TestClientHandler th;

      // set TCP socket options
      //TCPSocketOptions sock_opts(SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR));
      TCPSocketOptions sock_opts(SOL_SOCKET, (SO_REUSEADDR));

      // define instance of the receiver (server host)
      Receiver receiver(serverEP, &sock_opts);

      // register the custom client handler with receiver instance 
      receiver.RegisterClientHandler(&th);

      // start the server listening thread 
      receiver.Start();
      std::cout << "Server listening on: " << serverEP << std::endl;
      std::cout << "Press any key to quit" << std::endl;
      std::cin.get();

      // stop the listener and quit
      receiver.Stop();
      
      return 0;
   }
 #endif
