// Note: the purpose of the this is really just for managing the platform differences 
// between Windows and Linux socket libraries

#ifndef _MPL_PLATFORM_H
#define _MPL_PLATFORM_H

const int PORTABLE_SOCK_ERR_BUF_SIZE = 512;

#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
  #include <netdb.h>
  #include <unistd.h>
  #include <sys/types.h> 
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include<errno.h>

  // for strerror_s on Linux: source: https://en.cppreference.com/w/c/string/byte/strerror
  #ifndef __STDC_WANT_LIB_EXT1__
    #define __STDC_WANT_LIB_EXT1__  
  #endif 

  // helper function to get socket errors in a portable way (windows and linux)
  inline const char* strerror_portable(const char* errbuf, int buflen, int error)
  {
#ifdef __STDC_LIB_EXT1__
	strerror_s(errbuf, buflen, errno, error);
	return errbuf;
#endif   
  }

  // helper function to make error handling portable:  on Linux return errno
  inline int getlasterror_portable() { return errno; }

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

  #include<atomic>

  #pragma warning(disable:4522)
  #pragma comment(lib, "Ws2_32.lib")

  // helper function to make error handling portable:  on Windows return WSAGetLastError()
  inline int getlasterror_portable() { return  WSAGetLastError(); }

  // helper function to get socket errors in a portable way (windows and linux)
  const char* strerror_portable(const char* errbuf, int buflen, int error);

  /////////////////////////////////////////////////////////////////////////////
  // SocketSystem class - manages loading and unloading Winsock library
  // Sender and Receiver define an instance of SocketSystem as private member
  // SocketSystem hold a static counter as reference counter or loading
  // and unloading the Windows socket system.  The first Sender or Receiver
  // instantiated will load the windows sockets (WSA startup), and the 
  // last sSender or Receiver to go out of scope will will release (unload WAS cleanup)
  // the socket system
  class SocketSystem
  {
    public: 
	  
	   SocketSystem()
	   { 
		   rcount++;
		   if (rcount.load() == 1)
		   {
			   iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		   }
	   }

	   ~SocketSystem()
	   {
		   rcount--;
		   if (rcount.load() == 0)
		   {
			   int error = WSACleanup();
		   }
	   }	 
     private:
		
		static std::atomic<int> rcount;
		inline static int iResult;
        inline static WSADATA wsaData;
  };
#endif

#include<thread>
  
#endif 
