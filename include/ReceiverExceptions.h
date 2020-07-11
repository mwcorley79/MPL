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
#include <errno.h>
#include "Platform.h"

namespace CSE384 
{
  class ReceiverException : public std::exception
  {
     public: 
       ReceiverException(int errnum): errnum_(errnum)
       {
           msgbuf[0] = '\0';
       }

       virtual const char* what() const throw()
       {
           return strerror_portable(msgbuf, PORTABLE_SOCK_ERR_BUF_SIZE-1, errnum_);
       }

      virtual ~ReceiverException() throw() {}
    protected:
       int errnum_;
       char msgbuf[PORTABLE_SOCK_ERR_BUF_SIZE];
  };


 class ReceiverTransmitMessageHeaderException : public ReceiverException
 {
   public: 
     ReceiverTransmitMessageHeaderException(int errnum): ReceiverException(errnum)
     {}
 };

 class ReceiverTransmitMessageDataException : public ReceiverException
 {
   public:
     ReceiverTransmitMessageDataException(int errnum): ReceiverException(errnum)
     {}
 };

 class ReceiverSendResponseException : public ReceiverException
 {
   public:
     ReceiverSendResponseException(int errnum): ReceiverException(errnum)
     {}
 };

 class ReceiverReceiveMessageHeaderException : public ReceiverException
 { 
   public:
     ReceiverReceiveMessageHeaderException(int errnum): ReceiverException(errnum)
     {}
 };

 class ReceiverReceiveMessageDataException : public ReceiverException
 {
   public:
     ReceiverReceiveMessageDataException(int errnum): ReceiverException(errnum)
     {}
 };
 
 class ReceiverShutDownReadException : public ReceiverException
 {
   public:
     ReceiverShutDownReadException(int errnum): ReceiverException(errnum)
     {}
 };

class ReceiverShutDownWriteException : public ReceiverException
{
  public:
    ReceiverShutDownWriteException(int errnum): ReceiverException(errnum)
    {}
};

class ReceiverCloseException : public ReceiverException
{
  public:
    ReceiverCloseException(int errnum): ReceiverException(errnum)
    {}
};


class ReceiverAcceptException : public ReceiverException
{
  public:
    ReceiverAcceptException(int errnum): ReceiverException(errnum)
    {}
};


class ReceiverListenException : public ReceiverException
{
  public:
    ReceiverListenException(int errnum) : ReceiverException(errnum)
    {}
};

class ReceiverCreateSocketException : public ReceiverException
{
   public:
     ReceiverCreateSocketException(int errnum): ReceiverException(errnum)
     {}
};

class ReceiverBindException : public ReceiverException
{
   public:
     ReceiverBindException(int errnum): ReceiverException(errnum)
     {}
};

class ReceiverGetAddrException : public ReceiverException
{
   public:
     ReceiverGetAddrException(int errnum): ReceiverException(errnum)
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
    ReceiverNoRegisteredClientHandlerException() : ReceiverException(0) {}
    virtual const char* what() const throw()
    {
        return "No clientHandler Instance was registered";
    }
};

}
#endif /* RECEIVEREXCEPTIONS_H_ */
