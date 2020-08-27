#ifndef TCPSOCKET_H_
#define TCPSOCKET_H_

#include "Platform.h"
#include "EndPoint.h"

namespace CSE384
{
  class TCPSocketOptions;
  class TCPSocket
  {
  public:
    TCPSocket();
    TCPSocket(SOCKET fd);
  
    bool IsValid() const;
   
    int Send(const char *block, size_t blockLen, int flags, int sendRetries, unsigned int wait_time = 1);
    int Recv(const char *block, size_t blockLen, int flags, int recvRetries, unsigned int wait_time = 1);
    operator SOCKET();
    SOCKET GetSockFd() const;
    SOCKET SetSockFd(SOCKET sock_fd);
    TCPSocket Accept();

    int  GetLastMsgSizeTransmitted() const;
    void SetLastMsgSizeTransmitted(int msg_size);
   
    int Shutdown();
    int ShutdownSend();
    int ShutdownRecv();
    int Close();
    EndPoint RemoteEP(); 

    TCPSocket(TCPSocket &&s) noexcept;
    TCPSocket &operator=(TCPSocket &&s) noexcept;

    // disable copy construction and assignment
    TCPSocket(const TCPSocket &s) = delete;
    TCPSocket &operator=(const TCPSocket &s) = delete;
  
  protected:
     int GetAddressInfo(const char *node_name, const char *serv_name, 
                         int ai_family, struct addrinfo **servinfo);
  private:
    SOCKET sock_fd;
    int last_msg_size_;
  };

  class TCPClientSocket : public TCPSocket
  {
     public:
        void Connect(const EndPoint &ep, TCPSocketOptions *sc = nullptr);
        bool IsConnected() const;
  };

  class TCPServerSocket : public TCPSocket
  {
    public:
      void Bind(const EndPoint &ep, TCPSocketOptions *sc = nullptr);
      TCPSocket Accept();
      void Listen(int backlog);
     
  };


  // class for deferring socket options to the application
  // (for client side and server side)

  class TCPSocketOptions
  {
  public:
    friend TCPSocket;
    friend TCPServerSocket;
    friend TCPClientSocket;
    TCPSocketOptions(int level, int option_names);

  private:
    int SetSocketOptions(TCPSocket *soc);
    int level_;
    int option_names_;
  };

  inline TCPSocketOptions::TCPSocketOptions(int level, int option_names) : level_(level),
                                                                           option_names_(option_names)
  {
  }

  inline int TCPSocketOptions::SetSocketOptions(TCPSocket *soc)
  {
    int optval = 1;
    return setsockopt(soc->GetSockFd(), level_, option_names_, (char *)&optval, sizeof(optval));
  }

  inline SOCKET TCPSocket::GetSockFd() const
  {
    return sock_fd;
  }

  inline SOCKET TCPSocket::SetSockFd(SOCKET sock)
  {
    return (sock_fd = sock);
  }

  inline TCPSocket::operator SOCKET()
  {
    return sock_fd;
  }

  inline int TCPSocket::GetLastMsgSizeTransmitted() const
  {
     return last_msg_size_;
  }
  
  inline void TCPSocket::SetLastMsgSizeTransmitted(int msg_size)
  {
    last_msg_size_ = msg_size;
  }

  inline bool TCPSocket::IsValid() const
  {
    return (sock_fd != INVALID_SOCKET);
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

  inline bool TCPClientSocket::IsConnected() const
  {
    return (GetSockFd() != INVALID_SOCKET);
  }

}; // namespace CSE384

#endif /* TCPSOCKET_H_ */
