////////////////////////////////////////////////////////////////////////////////
// ReceiverExceptions.h - Defines a variety of exceptions thrown by the       //
//                        Receiver                                            //
// Language:    Standard C++11 (g++ 7.4)                                      //
// Platform:    Dell Precision m7720, Mint Linux 19.3 (64-bit)                //
// Application: CSE 384, Project 2 helpers                                    //
// Author:      Mike Corley, Syracuse University                              //
//              mwcorley@syr.edu                                              //
////////////////////////////////////////////////////////////////////////////////

#ifndef RECEIVEREXCEPTIONS_H_
#define RECEIVEREXCEPTIONS_H_

#include <exception>
#include <string>
#include <cstring>
#include <locale.h>

// for strerror_s on Linux: source: https://en.cppreference.com/w/c/string/byte/strerror
#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
  #ifndef __STDC_WANT_LIB_EXT1__
    #define __STDC_WANT_LIB_EXT1__
  #endif 
#endif

#include <errno.h>

#include "Platform.h"

namespace CSE384 
{
  class ReceiverException : public std::exception
  {
     public:
  #if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
       ReceiverException(): errnum_(errno)
  #else
       ReceiverException(): errnum_(WSAGetLastError())
  #endif
       {
           msgbuf[0] = '\0';
       }

       ReceiverException(int errnum): errnum_(errnum)
       {
           msgbuf[0] = '\0';
       }

       virtual const char* what() const throw()
       {
 #if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
         #ifdef __STDC_LIB_EXT1__
           strerror_s(msgbuf, 511, errno);
           return msgbuf;
         #endif
 #else
       // source: https://stackoverflow.com/questions/3400922/how-do-i-retrieve-an-error-string-from-wsagetlasterror
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
               NULL,                // lpsource
               errnum_,             // message id
               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
               (LPSTR) msgbuf,              // output buffer
               sizeof(msgbuf),     // size of msgbuf, bytes
               NULL);

        return msgbuf;
#endif                
       }

      virtual ~ReceiverException() throw() {}
    protected:
       int errnum_;
       char msgbuf[512];
  };


 class ReceiverTransmitMessageHeaderException : public ReceiverException
 {
   public: 
     ReceiverTransmitMessageHeaderException()
     {}
 };

 class ReceiverTransmitMessageDataException : public ReceiverException
 {
   public:
     ReceiverTransmitMessageDataException()
     {}
 };

 class ReceiverSendResponseException : public ReceiverException
 {
   public:
     ReceiverSendResponseException()
     {}
 };

 class ReceiverReceiveMessageHeaderException : public ReceiverException
 { 
   public:
     ReceiverReceiveMessageHeaderException()
     {}
 };

 class ReceiverReceiveMessageDataException : public ReceiverException
 {
   public:
     ReceiverReceiveMessageDataException()
     {}
 };
 
 class ReceiverShutDownReadException : public ReceiverException
 {
   public:
     ReceiverShutDownReadException()
     {}
 };

class ReceiverShutDownWriteException : public ReceiverException
{
  public:
    ReceiverShutDownWriteException()
    {}
};

class ReceiverCloseException : public ReceiverException
{
  public:
    ReceiverCloseException()
    {}
};


class ReceiverAcceptException : public ReceiverException
{
  public:
    ReceiverAcceptException()
    {}
};


class ReceiverListenException : public ReceiverException
{
  public:
    ReceiverListenException(): ReceiverException(errno)
    {}
};

class ReceiverCreateSocketException : public ReceiverException
{
   public:
     ReceiverCreateSocketException(): ReceiverException(errno)
     {}
};

class ReceiverBindException : public ReceiverException
{
   public:
     ReceiverBindException(): ReceiverException(errno)
     {}
};

class ReceiverGetAddrException : public ReceiverException
{
   public:
     ReceiverGetAddrException(): ReceiverException(errno)
     {}    
};

class ReceiverGetAddrInfoException : public ReceiverException
{
   public:
     ReceiverGetAddrInfoException(int errnum): ReceiverException(errnum)
     {}

     virtual const char* what() const throw()
     {
    	  return gai_strerror(errnum_);
     }
};



class ReceiverNoRegisteredClientHandlerException : public ReceiverException
{
  public:
    ReceiverNoRegisteredClientHandlerException() {}

};






}
#endif /* RECEIVEREXCEPTIONS_H_ */
