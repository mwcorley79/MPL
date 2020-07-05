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

#include<exception>
#include <string>
#include <cstring>

#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


namespace CSE384 
{
  class ReceiverException : public std::exception
  {
     public:
       ReceiverException()
       {}

       ReceiverException(int errnum): errnum_(errnum)
       {}

       virtual const char* what() const throw()
       {
         return strerror(errnum_);
       }

      virtual ~ReceiverException() throw() {}
    protected:
       int errnum_;
  };


 class ReceiverTransmitMessageHeaderException : public ReceiverException
 {
   public: 
     ReceiverTransmitMessageHeaderException(): ReceiverException(errno)
     {}
 };

 class ReceiverTransmitMessageDataException : public ReceiverException
 {
   public:
     ReceiverTransmitMessageDataException(): ReceiverException(errno)
     {}
 };

 class ReceiverSendResponseException : public ReceiverException
 {
   public:
     ReceiverSendResponseException(): ReceiverException(errno)
     {}
 };

 class ReceiverReceiveMessageHeaderException : public ReceiverException
 { 
   public:
     ReceiverReceiveMessageHeaderException(): ReceiverException(errno)
     {}
 };

 class ReceiverReceiveMessageDataException : public ReceiverException
 {
   public:
     ReceiverReceiveMessageDataException(): ReceiverException(errno)
     {}
 };
 
 class ReceiverShutDownReadException : public ReceiverException
 {
   public:
     ReceiverShutDownReadException(): ReceiverException(errno)
     {}
 };

class ReceiverShutDownWriteException : public ReceiverException
{
  public:
    ReceiverShutDownWriteException(): ReceiverException(errno)
    {}
};

class ReceiverCloseException : public ReceiverException
{
  public:
    ReceiverCloseException(): ReceiverException(errno)
    {}
};


class ReceiverAcceptException : public ReceiverException
{
  public:
    ReceiverAcceptException(): ReceiverException(errno)
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
