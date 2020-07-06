#ifndef _MPL_PLATFORM_H
#define _MPL_PLATFORM_H

#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
  #include <netdb.h>
  #include <unistd.h>
  #include <sys/types.h> 
  #include <sys/socket.h>
  #include <arpa/inet.h>

  using SOCKET = int;
  const int SD_SEND = SHUT_WR;
  const int SD_RECEIVE = ::SHUT_RD;
  const int SD_BOTH = ::SHUT_RDWR;
  const int INVALID_SOCKET = -1;

  inline int closesocket(SOCKET s)
  {
	 return close(s);
  }     
#else 
  #ifndef WIN32_LEAN_AND_MEAN  // prevents duplicate includes of core parts of windows.h in winsock2.h 
     #define WIN32_LEAN_AND_MEAN
  #endif
 
  #include <Windows.h>      // Windows API
  #include <winsock2.h>     // Windows sockets, ver 2
  #include <WS2tcpip.h>     // support for IPv6 and other things
  #include <IPHlpApi.h>     // ip helpers

  #pragma warning(disable:4522)
  #pragma comment(lib, "Ws2_32.lib")

  /////////////////////////////////////////////////////////////////////////////
  // SocketSystem class - manages loading and unloading Winsock library

  class SocketSystem
  {
     public:
	   SocketSystem()  
	   {
		 iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	   }

	   ~SocketSystem()
	   {
		  int error = WSACleanup();
	   }	 
     private:
  	  int iResult;
      WSADATA wsaData;
  };
#endif

#include<thread>
  
#endif 
