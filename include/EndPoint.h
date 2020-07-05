////////////////////////////////////////////////////////////////////////
// EndPoint.h -  end point class that wraps IP address and ports      //
// ver 1.0                                                            //
// Language:    Standard C++ (gcc/g++ 7.4)                            //
// Platform:    Dell Precision M7720, Linux Mint 19.3 (64-bit)        //
// Application: CSE 384, MPL (Message passing Layer)                  //
// Author:      Mike Corley, Syracuse University                      //
//              mwcorley@syr.edu                                      //
////////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * ===================
 * 
 * end point class that wraps IP address and ports
 * for convenience in working the TCP (socket-based) messaging
 *
 * Required Files:
 * ==============
 * EndPoint.h, EndPoint.cpp
 *
 * For Test Stub Build also requires: (EndPoint.h/.cpp)
 *
 * Build Process: (for test stub)
 * =============
 * Linux:  g++ -DTEST_ENDPOINT -o EndPointTest EndPoint.cpp
 *
 * Maintenance:
 * ===========
 *  ver 1.0 : 29 March 2019
 *     -- first release
 *
*/

#ifndef ENDPOINT_H_
#define ENDPOINT_H_

#include <iostream>
#include <string>
#include <exception>


namespace CSE384
{
  class BadEndPoint : public std::exception
  {
      public:
        BadEndPoint(const char* str): str_(str)
        {}

        virtual const char* what() const throw()
        {
      	  return str_;
        }

       ~BadEndPoint() throw() {}
     protected:
        const char* str_;
  };


  class EndPoint
  {
    public:
      EndPoint();
      EndPoint(const std::string& IP, unsigned int port);
      EndPoint(const std::string& serialized_ep);
      void SetEndPoint(const std::string& ip, unsigned int port);
      bool IsValid() const;
      void Parse(const std::string& serialized_ep);
      bool operator == (const EndPoint& ep);
      std::string IP() const;
      const char* IP_STR() const;
      unsigned int Port() const;
      std::string ToString() const;
    private:
      std::string _ip;
      unsigned int _port;
  };


  std::ostream& operator << (std::ostream& os, const EndPoint& ep);
}

#endif /* MPUtils_H_ */
