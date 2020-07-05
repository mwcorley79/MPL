/*
 * TCPSocketExceptions.h
 *
 *  Created on: Jul 4, 2020
 *      Author: mwcorley79
 */

#ifndef TCPSOCKETEXCEPTIONS_H_
#define TCPSOCKETEXCEPTIONS_H_

#include <cstring>
#include <exception>

  class SocketException : public std::exception
  {
     public:
       SocketException(int errnum): errnum_(errnum)
       {}

       virtual const char* what() const throw()
       {
         return (const char*) strerror(errnum_);
       }

       virtual ~SocketException() throw()
       {}
    protected:
       int errnum_;
  };

  class SocketCreateException : SocketException
  {
    public:
      SocketCreateException(): SocketException(errno)
      {}
  };

    class SocketOptionsException : SocketException
    {
        public:
          SocketOptionsException(): SocketException(errno)
          {}
    };

    class GetAddrInfoException : public SocketException
    {
       public:
         GetAddrInfoException(int errnum): SocketException(errnum)
         {}
         virtual const char* what() const throw()
         {
        	 return gai_strerror(errnum_);
         }
    };


#endif /* TCPSOCKETEXCEPTIONS_H_ */
