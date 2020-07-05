#ifndef TCPSOCKET_H_
#define TCPSOCKET_H_


#include <netdb.h>
#include <unistd.h>
#include "EndPoint.h"


namespace CSE384
{
  #if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
     using SOCKET = int;
     const int SD_SEND = SHUT_WR;
     const int SD_RECEIVE = ::SHUT_RD;
     const int SD_BOTH = ::SHUT_RDWR;
     const int INVALID_SOCKET = -1;

     inline int closesocket(SOCKET s)
     {
	   return close(s);
     }
  #endif

  class TCPSocketOptions;
  class TCPSocket
  {
     public:
	    TCPSocket();
	    TCPSocket(SOCKET fd);
	    void Connect(const EndPoint& ep, TCPSocketOptions* sc=nullptr);
	    bool IsConnected() const;
	    bool IsValid() const;
	    void Bind(const EndPoint& ep, TCPSocketOptions* sc=nullptr);
	    void Listen(int backlog);
	    int Send(const char *block, unsigned int blockLen, int sendRetries=1, unsigned int wait_time=1);
	    int Recv(const char *block, unsigned int blockLen, int recvRetries=1, unsigned int wait_time=1);
	    operator SOCKET ();
	    SOCKET GetSockFd();
	    TCPSocket Accept();
	    int Shutdown();
	    int ShutdownSend();
	    int ShutdownRecv();
	    int Close();

	    TCPSocket(TCPSocket&& s);
	    TCPSocket& operator=(TCPSocket&& s);

	    // disable copy construction and assignment
	    TCPSocket(const TCPSocket& s) = delete;
	    TCPSocket& operator=(const TCPSocket& s) = delete;
     private:
	   SOCKET sock_fd;
	   //char remote_addr[INET6_ADDRSTRLEN];
  };

  // class for deferring socket options to the application
  // (for client and server side)

  class TCPSocketOptions
  {
      public:
	     friend TCPSocket;
	     TCPSocketOptions(int level, int option_names);
      private:
	     int SetSocketOptions(TCPSocket* soc);
	     int level_;
         int option_names_;
  };

  inline TCPSocketOptions::TCPSocketOptions(int level, int option_names): level_(level),
                                                                           option_names_(option_names)
  {}

  inline int TCPSocketOptions::SetSocketOptions(TCPSocket* soc)
  {
	  int optval = 1;
	  return setsockopt(soc->GetSockFd(),level_, option_names_, (char*) &optval, sizeof(optval));
  }

  inline SOCKET TCPSocket::GetSockFd()
  {
  	 return sock_fd;
  }

  inline TCPSocket::operator SOCKET ()
  {
	 return sock_fd;
  }

  inline bool TCPSocket::IsValid() const
  {
	  return (sock_fd != INVALID_SOCKET);
  }

  inline bool TCPSocket::IsConnected() const
  {
	  return (sock_fd != -1);
  }

  inline int TCPSocket::Shutdown()
  {
	  return shutdown(sock_fd, SD_BOTH);
  }

  inline int TCPSocket::ShutdownSend()
  {
	  return shutdown(sock_fd, SD_SEND);
  }

  inline int TCPSocket::ShutdownRecv()
  {
	  return shutdown(sock_fd, SD_RECEIVE);
  }

  inline int TCPSocket::Close()
  {
	 int ret = closesocket(sock_fd);
	 if(ret == 0)
		 sock_fd = -1;
	 return ret;
  }

};

#endif /* TCPSOCKET_H_ */
