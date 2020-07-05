#include "Message.h"

namespace CSE384
{
    std::ostream& operator << (std::ostream& outs, Message& msg)
    {
      for(int i = 0; i < msg.Length(); ++i)
          outs << msg.GetData()[i];
      return outs;
    }

    std::ostream& operator << (std::ostream& outs, const Message& msg)
    {
      for(int i = 0; i < msg.Length(); ++i)
          outs << msg.GetData()[i];
      return outs;
    }


    Message::Message(const std::string& str, int type)
    {
      // initialize fix size header, and set network byte order
      msg_hdr_.type_ = type;
      msg_hdr_.len_ =  str.length();
       
      // set the data envelope if message len > 0 
      if(Length())
      {
        msg_data_ = new char[Length()];
        memcpy(msg_data_, str.c_str(), Length());
      }
      else
      {
        msg_data_ = NULL;
      }
    }

    Message::Message(const char* data, int length, int type) 
    {
      // initialize fix size header, and set network byte order
      msg_hdr_.type_ = type;
      msg_hdr_.len_ =  length;
       
      // set the data envelope if message len > 0 
      if(Length())
      {
        msg_data_ = new char[Length()];
        memcpy(msg_data_, data, Length());
      }
      else
      {
        msg_data_ = NULL;
      }
    } 

    Message::Message(Message&& msg): msg_hdr_(msg.msg_hdr_), 
                                     msg_data_(msg.msg_data_)
    {
      msg.msg_data_ = nullptr;
      msg.msg_hdr_.len_ = 0;
      msg.msg_hdr_.type_ = 0;
    }  
    

    Message::Message(const Message& msg)
    {
      // initialize fix size header, and set network byte order
      msg_hdr_ = msg.msg_hdr_;

      // set the data envelope if message len > 0 
      if(Length())
      {
        msg_data_ = new char[Length()];
        memcpy(msg_data_, const_cast<Message&>(msg).GetData(), Length()); 
      }
      else
      {
        msg_data_ = NULL;
      } 
    }

    Message::Message(struct MSGHEADER& mhdr, char* data): msg_hdr_(mhdr)
    {
       msg_data_ = new char[msg_hdr_.len_];
       if(data != nullptr)
       {
           memcpy(msg_data_, data, msg_hdr_.len_);
       }
    }

    Message& Message::operator=(Message&& msg)
    {
      if(&msg != this)
      {
        delete[] msg_data_; 

        msg_hdr_ = msg.msg_hdr_;
        msg_data_ = msg.msg_data_;

        msg.msg_data_ = nullptr;
        msg.msg_hdr_.len_ = 0;
      }
      return *this;

    }

    
    Message& Message::operator=(const Message& msg)
    {
      if (&msg != this)
      {
        msg_hdr_ = msg.msg_hdr_;
        delete[] msg_data_;

        // set the data envelope if message len > 0
        if (Length())
        {
          msg_data_ = new char[Length()];
          memcpy(msg_data_, const_cast<Message &>(msg).GetData(), Length());
        }
        else
        {
          msg_data_ = NULL;
        }
      }

      return *this;
    }

    std::string Message::ToString() const
    {
      return std::string(GetData(), Length());
    }
};

#ifdef TEST_MESSAGE
#include <iostream>
using namespace CSE384;

void test(const Message& msg)
{
  std::cout << msg << std::endl;
  std::cout << msg.GetData() << std::endl;
  std::cout << msg.Length() << std::endl;
  std::cout << msg.ToString() << std::endl;
}

void test(Message& msg)
{
  std::cout << msg << std::endl;
  std::cout << msg.GetData() << std::endl;
  std::cout << msg.Length() << std::endl;
  std::cout << msg.ToString() << std::endl;
}

int main()
{
  std::cout << "Test accessors on non const message " << std::endl;
  Message non_const_msg(std::string("message test"));
  std::cout << non_const_msg << std::endl;
  std::cout << non_const_msg.GetData() << std::endl;
  std::cout << non_const_msg.Length() << std::endl;
  std::cout << non_const_msg.ToString() << std::endl;


  std::cout << "Test accessors on copy of non const message " << std::endl;
  Message m = non_const_msg;
  std::cout << m << std::endl;
  std::cout << m.GetData() << std::endl;
  std::cout << m.Length() << std::endl;
  std::cout << m.ToString() << std::endl;

  std::cout << "Test accessors on const message " << std::endl;
  const Message  const_msg(std::string("message test"));
  std::cout << const_msg << std::endl;
  std::cout << const_msg.GetData() << std::endl;
  std::cout << const_msg.Length() << std::endl;
  std::cout << const_msg.ToString() << std::endl;

  std::cout << "Test pass \"non const message\" by const reference:" << std::endl;
  test(m);

  std::cout << "Test pass \"const message\" by const reference:" << std::endl;
  test(const_msg);

}

#endif
