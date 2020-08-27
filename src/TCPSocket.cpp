#include "TCPSocket.h"
#include "TCPSocketExceptions.h"
#include "SenderExceptions.h"
#include "ReceiverExceptions.h"

#include <string>
#include <sstream>
#include <thread>

namespace CSE384
{

  TCPSocket::TCPSocket() : sock_fd(INVALID_SOCKET)
  {
  }

  TCPSocket::TCPSocket(SOCKET socket) : sock_fd(socket)
  {
  }

  TCPSocket::TCPSocket(TCPSocket &&s) noexcept
  {
    sock_fd = s.sock_fd;
    s.sock_fd = INVALID_SOCKET;
  }

  TCPSocket &TCPSocket::operator=(TCPSocket &&s) noexcept
  {
    if(this != &s)
    {
      sock_fd = s.sock_fd;
      s.sock_fd = INVALID_SOCKET;
    }
    return *this;
  }

  template <typename T>
  static std::string ToString(const T &t)
  {
    std::ostringstream convert; // stream used for the conversion
    convert << t;
    return convert.str();
  }

  //get sockaddr, IPv4 or IPv6:
  static void *get_in_addr(struct sockaddr *sa)
  {
    if (sa->sa_family == AF_INET)
      return &(((struct sockaddr_in *)sa)->sin_addr);

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
  }

  // reference: Beej's guide to network programming: http://beej.us/guide/bgnet/
  int TCPSocket::GetAddressInfo(const char *node_name, const char *serv_name, int ai_family, struct addrinfo **servinfo)
  {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = ai_family;     // use IPv6 or IPv4
    hints.ai_socktype = SOCK_STREAM; // for now this will always be TCP
    return getaddrinfo(node_name, serv_name, &hints, servinfo);
  }

  int TCPSocket::Send(const char *block, size_t blockLen, int flags, int sendRetries, unsigned int wait_time)
  {
    int bytesSent;

    int bytesLeft = (int) blockLen;
    int blockIndx = 0;
    int count = 0;

    while (bytesLeft > 0)
    {
      bytesSent = send(sock_fd, &block[blockIndx], bytesLeft, flags);
      if (bytesSent > 0)
      {
        bytesLeft -= bytesSent;
        blockIndx += bytesSent;
      }
      else if (bytesSent == -1)
      {
        ++count;
        if(count > sendRetries)
          return -1;
        std::this_thread::sleep_for(std::chrono::microseconds(wait_time));
        //usleep(wait_time);
      }
    }
    return (int) blockLen;
  }

  //test MSG_WAITALL flag
  int TCPSocket::Recv(const char *block, size_t blockLen, int flags, int recvRetries, unsigned int wait_time)
  {
    int bytesRecvd, bytesLeft = (int) blockLen;
    int blockIndx = 0;
    int count = 0;

    while (bytesLeft > 0)
    {
      bytesRecvd = recv(sock_fd, (char *)&block[blockIndx], bytesLeft, flags);

      if (bytesRecvd > 0)
      {
        bytesLeft -= bytesRecvd;
        blockIndx += bytesRecvd;
      }
      else if (bytesRecvd == 0)
        return 0;
      else
      {
        ++count;
        if (count > recvRetries)
          return -1;
        std::this_thread::sleep_for(std::chrono::microseconds(wait_time));
         //usleep(wait_time);
      }
    }
    return (int) blockLen;
  }

  static int GetPeerEndPoint(int sock_fd, char ipstr[], unsigned int &port)
  {
    socklen_t len;
    struct sockaddr_storage addr;
    len = sizeof(addr);

    if (getpeername(sock_fd, (struct sockaddr *)&addr, &len) == -1)
      return -1;

    // deal with both IPv4 and IPv6:
    if (addr.ss_family == AF_INET)
    {
      struct sockaddr_in *s = (struct sockaddr_in *)&addr;
      port = ntohs(s->sin_port);
      inet_ntop(AF_INET, &s->sin_addr, ipstr, INET6_ADDRSTRLEN);
    }
    else
    { // AF_INET6
      struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
      port = ntohs(s->sin6_port);
      inet_ntop(AF_INET6, &s->sin6_addr, ipstr, INET6_ADDRSTRLEN);
    }
    return 0;
  }

  EndPoint TCPSocket::RemoteEP()
  {
    unsigned int port;
    char ipstr[INET6_ADDRSTRLEN];
    if(GetPeerEndPoint((int) GetSockFd(), ipstr, port) == 0)
      return EndPoint(std::string(ipstr), port);
    return EndPoint();
  }

  int TCPSocket::Close()
  {
    int ret = closesocket(sock_fd);
    if (ret == 0)
      sock_fd = INVALID_SOCKET;
    return ret;
  }

  void TCPClientSocket::Connect(const EndPoint &ep, TCPSocketOptions *sc)
  {
    struct addrinfo *p;
    struct addrinfo *servinfo;
    int error = 0;
    int addr_info_result;

    if ((addr_info_result = GetAddressInfo(ep.IP_STR(), ToString(ep.Port()).c_str(), AF_UNSPEC, &servinfo)) != 0)
      throw GetAddrInfoException(addr_info_result);

    // iterate over all the results and connect to the first we can
    for (p = servinfo; p != 0; p = p->ai_next)
    {
    
      if (SetSockFd(socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET)
      {
        error = getlasterror_portable();
        continue;
      }

      // if socket options are specified by the application, then apply before connect
      if (sc != 0)
      {
        if (sc->SetSocketOptions(this) == -1)
        {
          error = getlasterror_portable();
          freeaddrinfo(servinfo);
          throw SocketOptionsException(error);
        }
      }

      // attempt current connection result, and get out with first successful attempt
      if (connect(GetSockFd(), p->ai_addr, (int)p->ai_addrlen) == INVALID_SOCKET)
      {
        error = getlasterror_portable();
        closesocket(GetSockFd());
        continue;
      }
      break; //if we get here we must have connected successfully
    }

    // if couldn't connect to any of the addrinfo results, then bail with -1 indicator
    if (p == 0 || GetSockFd() == INVALID_SOCKET)
    {
      SetSockFd(INVALID_SOCKET);
      freeaddrinfo(servinfo);
      throw SenderConnectException(error);
    }

    // fill textual address information to return
    //inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), address, INET6_ADDRSTRLEN);
    freeaddrinfo(servinfo);
  }

  void TCPServerSocket::Listen(int backlog)
  {
    if (listen(GetSockFd(), backlog) != 0)
      throw ReceiverListenException(getlasterror_portable());
  }

  void TCPServerSocket::Bind(const EndPoint &ep, TCPSocketOptions *sc)
  {
    struct addrinfo *p;
    struct addrinfo *servinfo;
    int addr_info_result;
    int error = 0;

    if((addr_info_result = GetAddressInfo(ep.IP_STR(), ToString(ep.Port()).c_str(), AF_UNSPEC, &servinfo)) != 0)
      throw GetAddrInfoException(addr_info_result);

    // iterate over all the results and bind to the first we can
    for (p = servinfo; p != 0; p = p->ai_next)
    {
      if(SetSockFd(socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET)
      {
        error = getlasterror_portable();
        continue;
      }

      // if socket options are specified by the application, then apply before bind
      if (sc != 0)
      {
        if (sc->SetSocketOptions(this) == INVALID_SOCKET)
        {
          error = getlasterror_portable();
          freeaddrinfo(servinfo);
          throw SocketOptionsException(error);
        }
      }

      // attempt current connection result, and get out with first successful attempt
      if (bind(GetSockFd(), p->ai_addr, (int)p->ai_addrlen) == INVALID_SOCKET)
      {
        error = getlasterror_portable();
        closesocket(GetSockFd());
        continue;
      }
      break; //if we get here we must successfully bind
    }

    // if couldn't bind to any of the addrinfo results, then bail with -1 indicator
    if (p == 0 || GetSockFd() == INVALID_SOCKET)
    {
      freeaddrinfo(servinfo);
      throw ReceiverBindException(error);
    }

    freeaddrinfo(servinfo);
  }

  TCPSocket TCPServerSocket::Accept()
  {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    // ---accept a connection (creating a data pipe)---

    // std::cout << "Hello New Client From: " << inet_ntoa(client_addr.sin_addr) << " : "
    //           << ntohs(client_addr.sin_port) << std::endl;

    return TCPSocket(accept(GetSockFd(), (struct sockaddr *)&client_addr, &addrlen));
  }

}; // namespace CSE384

#ifdef TEST_SOCKETS
#include <iostream>
#include <thread>
#include <chrono>

using namespace CSE384;

// dedicated server thread
void ServiceProc(const EndPoint &service_ep)
{
  int backlog = 10;
// TCPSocketOptions so(SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR));
  TCPSocketOptions so(SOL_SOCKET, (SO_REUSEADDR));
  TCPServerSocket sock;
  sock.Bind(service_ep, &so);

  // set socket socket listening
  sock.Listen(backlog);

  // ---accept a connection (creating a data pipe)---
  TCPSocket channel = sock.Accept();
  char block[11];
  int rn = channel.Recv(block, 10, MSG_WAITALL, 1);
  block[10] = '\0';
  int sn = channel.Send((char *)"rend test!", 10,0,1);

  int ret = channel.Close();
  if(ret != INVALID_SOCKET)
    std::cout << "server channel socket closed" << std::endl;

  std::cout << "Server Received: " << block << " : " << rn << " bytes" << std::endl;
  std::cout << "Server Sent: " << sn << " bytes" << std::endl;

  ret = sock.Close();
  if(ret != INVALID_SOCKET)
    std::cout << "server listening socket closed" << std::endl;
}

// start a dedicated server thread for testing
std::thread ServerTest(const EndPoint &service_ep)
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
   std::this_thread::sleep_for(std::chrono::seconds(3));

  // create client socket for Connect, Send test
  TCPClientSocket client_sock;
  client_sock.Connect(server_ep);
  if (client_sock.IsConnected())
  { 
    int sn = client_sock.Send((char *)"cend test!", 10,0,1);
    char block[10];
    int sr = client_sock.Recv(block, 10, MSG_WAITALL, 1);
    block[10] = '\0';

    int ret = client_sock.Close();
    if(ret != INVALID_SOCKET)
       std::cout << "client channel socket closed" << std::endl;

    std::cout << "Client Sent    : " << sn << " bytes" << std::endl;
    std::cout << "Client Received: " << block << " : " << sr << " bytes" << std::endl;
  }
  // wait until the server thread finishes
  serverThread.join();

  return 0;
}

#endif
