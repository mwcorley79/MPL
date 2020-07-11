#include "TCPSocket.h"
#include "TCPSocketExceptions.h"
#include "SenderExceptions.h"
#include "ReceiverExceptions.h"

#include <string>
#include <sstream>
#include <thread>

namespace CSE384
{
  template<typename T>
  static std::string ToString(const T& t)
  {
    std::ostringstream convert;   // stream used for the conversion
    convert << t;
    return convert.str();
  }

  //get sockaddr, IPv4 or IPv6:
  static void* get_in_addr(struct sockaddr *sa)
  {
     if(sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);

     return &(((struct sockaddr_in6*)sa)->sin6_addr);
  }
  

  // reference: Beej's guide to network programming: http://beej.us/guide/bgnet/
  static int GetAddressInfo(const char* node_name, const char* serv_name, int ai_family, struct addrinfo** servinfo)
  {
      struct addrinfo hints;
      memset(&hints, 0, sizeof(hints));
      hints.ai_family = ai_family;      // use IPv6 or IPv4
      hints.ai_socktype = SOCK_STREAM;  // for now this will always be TCP
      return getaddrinfo(node_name, serv_name, &hints, servinfo);
  }

  TCPSocket::TCPSocket() : sock_fd(INVALID_SOCKET)
  {}


  TCPSocket::TCPSocket(SOCKET socket): sock_fd(socket)
  {}

  TCPSocket::TCPSocket(TCPSocket&& s) noexcept
  {
	  sock_fd = s.sock_fd;
	  s.sock_fd = INVALID_SOCKET;
  }

  TCPSocket& TCPSocket::operator=(TCPSocket&& s) noexcept
  {
     if (this != &s)
     {
    	sock_fd = s.sock_fd;
    	s.sock_fd = INVALID_SOCKET;
     }
     return *this;
  }

  void TCPSocket::Connect(const EndPoint& ep, TCPSocketOptions* sc)
  {
	  struct addrinfo* p;
	  struct addrinfo* servinfo;
      int error = 0;

	  int addr_info_result;
	  if((addr_info_result = GetAddressInfo(ep.IP_STR(), ToString(ep.Port()).c_str(), AF_UNSPEC, &servinfo)) != 0)
	 	 throw GetAddrInfoException(addr_info_result);

	  //iterate over all the results and connect to the first we can
	  for(p = servinfo; p != 0; p = p->ai_next)
	  {
          if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
          {
              error = getlasterror_portable();
              continue;
          }

	  	 // if socket options are specified by the application, then apply before connect
	  	 if(sc != 0)
             if (sc->SetSocketOptions(this) == -1)
             {
                 error = getlasterror_portable();
                 freeaddrinfo(servinfo);
                 throw SocketOptionsException(error);
             }

	  	 // attempt current connection result, and get out with first successful attempt
	  	 if(connect(sock_fd, p->ai_addr, (int) p->ai_addrlen) == INVALID_SOCKET)
	  	 {
             error = getlasterror_portable();
             closesocket(sock_fd);
	  	     continue;
	  	 }
	  	 break; //if we get here we must have connected successfully
	 }

	 // if couldn't connect to any of the addrinfo results, then bail with -1 indicator
	 if(p == 0 || sock_fd == INVALID_SOCKET)
	 {
        sock_fd = INVALID_SOCKET;
        freeaddrinfo(servinfo);
	    throw SenderConnectException(error);
	 }

	 // fill textual address information to return
	 //inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), address, INET6_ADDRSTRLEN);
     freeaddrinfo(servinfo);
  }

  void TCPSocket::Bind(const EndPoint& ep, TCPSocketOptions* sc)
  {
      struct addrinfo* p;
      struct addrinfo* servinfo;
      int addr_info_result;
      int error = 0;

      if ((addr_info_result = GetAddressInfo(ep.IP_STR(), ToString(ep.Port()).c_str(), AF_UNSPEC, &servinfo)) != 0)
          throw GetAddrInfoException(addr_info_result);

      // iterate over all the results and connect to the first we can
      for (p = servinfo; p != 0; p = p->ai_next)
      {
          if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
          {
              error = getlasterror_portable();
              continue;
          }

          // if socket options are specified by the application, then apply before bind
          if (sc != 0)
              if (sc->SetSocketOptions(this) == -1)
              {
                  error = getlasterror_portable();
                  freeaddrinfo(servinfo);
                  throw SocketOptionsException(error);
              }

          // attempt current connection result, and get out with first successful attempt
          if (bind(sock_fd, p->ai_addr, (int) p->ai_addrlen) == INVALID_SOCKET)
          {
              error = getlasterror_portable();
              closesocket(sock_fd);
              continue;
          }
          break; //if we get here we must have connected successfully
      }

      // if couldn't bind to any of the addrinfo results, then bail with -1 indicator
      if (p == 0 || sock_fd == INVALID_SOCKET)
      {
          freeaddrinfo(servinfo);
          throw ReceiverBindException(error);
      }

      freeaddrinfo(servinfo);
  }


  int TCPSocket::Send(const char *block, unsigned int blockLen, int sendRetries, unsigned int wait_time)
  {
     int bytesSent;
     int bytesLeft = blockLen;
     int blockIndx = 0;
     int count = 0;

     while(bytesLeft > 0)
     {
       bytesSent = send(sock_fd,&block[blockIndx],bytesLeft,0);
       if(bytesSent > 0)
       {
    	   bytesLeft -= bytesSent;
    	   blockIndx += bytesSent;
       }
       else if(bytesSent == -1 )
       {
         ++count;
         if(count > sendRetries)
            return -1;
         std::this_thread::sleep_for(std::chrono::microseconds(wait_time));
         //usleep(wait_time);
       }
      }
      return blockLen;
   }


   int TCPSocket::Recv(const char *block, unsigned int blockLen, int recvRetries, unsigned int wait_time)
   {
     int bytesRecvd, bytesLeft = blockLen;
     int blockIndx = 0;
     int count = 0;

     while(bytesLeft > 0)
     {
       bytesRecvd = recv(sock_fd, (char*) &block[blockIndx],bytesLeft,0);

       if(bytesRecvd > 0)
       {
    	   bytesLeft -= bytesRecvd;
    	   blockIndx += bytesRecvd;
       }
       else if(bytesRecvd == 0)
	      return 0;
       else
       {
         ++count;
         if(count > recvRetries)
           return -1;
         std::this_thread::sleep_for(std::chrono::microseconds(wait_time));
         //usleep(wait_time);
       }
     }
     return blockLen;
   }

   void TCPSocket::Listen(int backlog)
   {
     if(listen(sock_fd, backlog) != 0)
    	throw ReceiverListenException(getlasterror_portable());
   }

   TCPSocket TCPSocket::Accept()
   {
	   struct sockaddr_in client_addr;
	   socklen_t addrlen= sizeof(client_addr);
	   // ---accept a connection (creating a data pipe)---

	   // std::cout << "Hello New Client From: " << inet_ntoa(client_addr.sin_addr) << " : "
	                    //           << ntohs(client_addr.sin_port) << std::endl;

	   return TCPSocket(accept(sock_fd, (struct sockaddr *)&client_addr, &addrlen));
   }
};


#ifdef TEST_SOCKET
#include<iostream>
#include <thread>
using namespace CSE384;

// dedicated server thread
void ServiceProc(const EndPoint& service_ep)
{
  int backlog = 10;

  TCPSocketOptions so(SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR));
  TCPSocket sock;
  sock.Bind(service_ep, &so);

  // set socket socket listening
  sock.Listen(backlog);

  // ---accept a connection (creating a data pipe)---
  TCPSocket client_sock = sock.Accept();
  char block[11];
  int n = client_sock.Recv(block, 10);
  block[10] = '\0';
  std::cout << "Received: " << block << " : " << n << " bytes" << std::endl;
}

// start a dedicated server thread for testing
std::thread ServerTest(const EndPoint& service_ep)
{
	std::thread listenThread = std::thread(ServiceProc, service_ep);
	return listenThread;
}

int main()
{
    // test: construct a socket for Bind, Listen, Receive test (on dedicated thread)
    EndPoint server_ep("::1", 5050);
    std::thread serverThread = ServerTest(server_ep);

    // give server thread a change to start
    ::sleep(3);

    // create client socket for Connect, Send test
    TCPSocket send_sock;
    send_sock.Connect(server_ep);
    if(send_sock.IsConnected())
    {
      int n = send_sock.Send((char*)"send test!",10);
      closesocket( (SOCKET) send_sock);
      std::cout << "Sent: "  << n << " bytes" << std::endl;
    }
    // wait until the server thread finishes
    serverThread.join();

	return 0;
}

#endif





