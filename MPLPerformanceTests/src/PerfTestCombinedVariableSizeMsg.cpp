//////////////////////////////////////////////////////////////
// C++ (MPL) Comm - Test Communication library              //
//                                                          //
// Mike Corley, https://github.com/mwcorley79, 22 Aug 2020  //
//////////////////////////////////////////////////////////////

/*
   Demo:
   Test message rate and throughput
   - start Listener component
   - start Connector component
   - start post_message thread
   - start recv_message thread
   - send a fixed number of messages
   - send END message to exit client handler
   - eval elapsed time
   - send QUIT message to shut down Listener
*/

#include <string>
#include <vector>
#include <iostream>
#include <mpl.h>
#include <chrono>

using namespace CSE384;
using namespace std::chrono;

static std::mutex ioLock;


/*---------------------------------------------------------
  Display test data - used for individual tests
*/
void display_test_data(int64_t et, unsigned num_msgs, unsigned msg_size)
{
  /*  double elapsed_time_sec = 1.0e-6 * et;
   double num_msgs_f64 = num_msgs;
   double size_mb = 1.0e-6 * (msg_size + 4);
   double msg_rate = num_msgs_f64 / elapsed_time_sec;
   double byte_rate_mbpsec = num_msgs_f64 * size_mb / elapsed_time_sec;

   std::cout << "\n   elapsed microsec " << et;
   std::cout << "\n   messages/second  " << msg_rate;
   std::cout << "\n   thruput - MB/s   " << byte_rate_mbpsec;
   */
}


/*---------------------------------------------------------
  Perf test - client waits for reply before posting again
*/
void client_wait_for_reply(const EndPoint &addr,    // endpoint (address, port)
                           const std::string &name, // test name
                           unsigned num_msgs,       // number of mesages to send
                           unsigned sz_bytes        // body size in bytes
)
{

   std::cout << "\n --: " << name << "msgs, " << num_msgs << "bytes per msg" << sz_bytes;

   TCPConnector conn;
   conn.Connect(addr);

   // construct message of sz_bytes (pertains to message body, not including header)
   char* body = new char[sz_bytes];
   std::memset(body, '\0', sz_bytes);
   Message msg(body, sz_bytes, MessageType::DEFAULT);
   delete[] body;
   
   StopWatch tmr;

   //std::thread handle = std::thread( [&]() {
   tmr.start();
   for (unsigned _i = 0; _i < num_msgs; ++_i)
   {
      // std::cout << "\n posting msg " << name << " of size " << sz_bytes;
      conn.PostMessage(msg);
      msg = conn.GetMessage(); // changing the message, is this what we want?
      // std::cout << "\n received msg: " << msg->GetType();
   }

 
   tmr.stop();
   int64_t et = tmr.elapsed_micros();
   conn.Close(); // shutdown connection (end message etc.)
   display_test_data(et, num_msgs, sz_bytes);
   //});

   //return handle;
}

/*---------------------------------------------------------
  Perf test - client does not wait for reply before posting
*/
void client_no_wait_for_reply(const EndPoint &addr,    // endpoint (address, port)
                              const std::string &name, // test name
                              unsigned num_msgs,       // number of mesages to send
                              unsigned sz_bytes        // body size in bytes
)
{
   {
     std::lock_guard<std::mutex> l(ioLock);
     std::cout <<"\n -- " << name << ": " << num_msgs << " msgs," << sz_bytes + MSGHEADER::SIZE()  << " bytes per msg";
   }

   TCPConnector conn;
   conn.ConnectPersist(addr, 10, 1, 0);
   if (conn.IsConnected())
   {
      std::thread handle = std::thread([&]() {
         Message msg;
         while ((msg = conn.GetMessage()).GetType() != MessageType::DISCONNECT)
         {
            // std::cout << "\n received msg: " << msg->Length();
         }
      });

      // construct message of sz_bytes (pertains to message body, not including header)
      char* body = new char[sz_bytes];
      std::memset(body, '0', sz_bytes);
      Message msg(body, sz_bytes, MessageType::DEFAULT);
      delete [] body;
     
      for (unsigned _i = 0; _i < num_msgs; ++_i)
      {
         //std::cout << "\n posting msg " << name << " of size " << sz_bytes;
          conn.PostMessage(msg);
      }
      
      conn.Close(&handle);

   }
}


/*-------------------------------------------------------
   Multiple clients running client_no_wait_for_reply
*/
void multiple_clients(int nc,
                     const EndPoint &addr,    // endpoint (address, port)
                     const std::string &name, // test name
                     unsigned num_msgs,       // number of mesages to send
                     unsigned sz_bytes        // body size in bytes
)
{
   std::cout << "\n  number of clients:  ", nc;
   StopWatch tmr;
   tmr.start();

   std::vector<std::thread> handles;
   for (int _i = 0; _i < nc; ++_i)
   {
      handles.push_back(std::thread(client_no_wait_for_reply, addr, name, num_msgs, sz_bytes));
   }

   /*-- wait for all replies --*/
   for (auto handle = handles.begin(); handle != handles.end(); ++handle)
   {
      if (handle->joinable())
         handle->join();
   }
   tmr.stop();

   auto et = tmr.elapsed_micros();
   uint64_t nm = nc * num_msgs;
   uint64_t tp = (nm * sz_bytes) / et;

   std::cout << "\n elapsed microseconds: " << et;
   std::cout << "\n number messages: " << nm;
   std::cout << "\n throughput MB/S: " << tp
             << "\n";
}

class PerfClientHandler : public ClientHandler
{
public:
   // sz_bytes not a fixed size msg, its the chosen size to use for the given instance
   PerfClientHandler(int sz_bytes):  sz_bytes_(sz_bytes)
   {
   }

   // this is a creational function: you must implement it
   // the TCPResponder creates an instance of this class (it's how the server polymorphism works)
   virtual ClientHandler *Clone()
   {
      return new PerfClientHandler(sz_bytes_);
   }

   // this is where you define the custom server processing: you must implement it
   virtual void AppProc()
   {
      // construct message of sz_bytes (pertains to message body, not including header)
      char* body = new char[sz_bytes_];
      std::memset(body, '\0', sz_bytes_);
      Message msgSend(body, sz_bytes_, MessageType::DEFAULT);
      delete [] body;

      Message msg;
      // use of Queue
      // while ((msg = GetMessage())->GetType() != MessageType::DISCONNECT)
      
      //no use of queue
      while ((msg = ReceiveMessage()).GetType() != MessageType::DISCONNECT)
      {
         // PostMessage(msg); // post to send queue
         SendMessage(msgSend); //direct send 
         // std::cout << "RECEIVED" << std::endl;
      }   
   }

   ~PerfClientHandler()
   {
      // std::cout << "Handler destroyed" << std::endl;
   }
   private:
     int sz_bytes_;
};

int main(int argc, char* argv[])
{
   // specify the server Endpoint we wish to connect
   
   const int MSG_SIZE = 4096;
   const int NUM_CLIENTS = 16;
   const int NUM_MSGS = 1000;
   const int NUM_THREAD_POOL_THREADS = 8;
   const std::string TEST_NAME = "test4";
   
  EndPoint addr("127.0.0.1", 8080);
   PerfClientHandler ph(MSG_SIZE);

   // set TCP socket options
   //TCPSocketOptions sock_opts(SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR));
   TCPSocketOptions sock_opts(SOL_SOCKET, (SO_REUSEADDR));

   // define instance of the TCPResponder (server host)
   TCPResponder responder(addr, &sock_opts);

    // set number of clients for the server process to service before exiting (-1 runs indefinitely)
   responder.NumClients(NUM_CLIENTS);

   responder.UseClientSendReceiveQueues(false);  // uncomment if you use SendMessage/ReceiveMessage

   // register the custom client handler with TCPResponder instance
   responder.RegisterClientHandler(&ph);

   // start the server listening thread
   responder.Start();

   // give the server a 
   std::this_thread::sleep_for(std::chrono::milliseconds(100));

   // std::thread test3 = std::thread(client_wait_for_reply, addr, "test3", 1000, 1024);
   // test3.join();
  
   std::cout << "\n  -- test4 (variable size message): c++_comm --\n";
   int nt = 8;
   std::cout << "\n  num thrdpool thrds: " << nt;
   

   multiple_clients(NUM_CLIENTS, addr, TEST_NAME, NUM_MSGS, MSG_SIZE);

  // std::cin.get();

   // stop the listener and quit
  responder.Stop();

   return 0;
}