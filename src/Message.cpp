#include "Message.h"

namespace CSE384
{
  Message::Message(const char *data, uint16_t length, int16_t type) : raw_msg_(new char[sizeof(MSGHEADER) + length])
  {
    // use placement new to instantiate MSG_HDR in raw_msg_ memory space
    new (raw_msg_) MSGHEADER(length, type);

    // copy the message into the data portion of the raw message
    std::memcpy(raw_msg_ + sizeof(MSGHEADER), data, length);
  }

  Message::Message(const std::string &str, int type) : Message(str.c_str(), str.length(), type)
  {
  }

  Message::Message(const MSGHEADER &hdr) : raw_msg_(new char[sizeof(MSGHEADER) + hdr.len()])
  {
    // store fixed size header on the heap with the data envelope
    raw_msg_ = new char[sizeof(MSGHEADER) + hdr.len()];

    // use placement new to instantiate MSG_HDR in raw_msg_ memory space
    new (raw_msg_) MSGHEADER(hdr.len(), hdr.type());

    std::memset(GetData(), 0, Length());
  }

  Message::Message(Message &&msg) : raw_msg_(msg.raw_msg_)

  {
    msg.raw_msg_ = nullptr;
  }

  Message::Message(const Message &msg) : raw_msg_(new char[msg.RawMsgLength()])
  {
    std::memcpy(raw_msg_, msg.raw_msg_, msg.RawMsgLength());
  }

  Message &Message::operator=(Message &&msg)
  {
    if (&msg != this)
    {
      delete[] raw_msg_;
      raw_msg_ = msg.raw_msg_;
      msg.raw_msg_ = nullptr;
    }
    return *this;
  }

  Message &Message::operator=(const Message &msg)
  {
    if (&msg != this)
    {
      delete[] raw_msg_;
      raw_msg_ = new char[msg.RawMsgLength()];
      std::memcpy(raw_msg_, msg.raw_msg_, msg.RawMsgLength());
    }

    return *this;
  }

  std::string Message::ToString() const
  {
    return std::string(GetData(), Length());
  }

  std::ostream &operator<<(std::ostream &outs, Message &msg)
  {
    for (int i = 0; i < msg.Length(); ++i)
      outs << msg.GetData()[i];
    return outs;
  }

  std::ostream &operator<<(std::ostream &outs, const Message &msg)
  {
    for (int i = 0; i < msg.Length(); ++i)
      outs << msg.GetData()[i];
    return outs;
  }
}; // namespace CSE384

#ifdef TEST_MESSAGE
#include <iostream>
using namespace CSE384;

void test(const Message &msg)
{
  std::cout << msg << std::endl;
  std::cout << msg.GetData() << std::endl;
  std::cout << msg.Length() << std::endl;
  std::cout << msg.ToString() << std::endl;
}

void test(Message &msg)
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
  std::cout << "Test insertion << operator: " << non_const_msg << std::endl;
  std::cout << "Test GetData: " << non_const_msg.GetData() << std::endl;
  std::cout << "Test Length() accessor: " << non_const_msg.Length() << std::endl;
  std::cout << "Test Message Type accessor:" << non_const_msg.GetType() << std::endl;
  std::cout << "Test ToString() method: " << non_const_msg.ToString() << std::endl;
  std::cout << "Test non const index operator " << non_const_msg[0] << std::endl;

  std::cout << "Test accessors on copy (move) of non const message " << std::endl;
  Message m = std::move(non_const_msg);
  std::cout << m.GetType() << std::endl;
  std::cout << m.GetData() << std::endl;
  std::cout << m.Length() << std::endl;
  std::cout << m.ToString() << std::endl;

  std::cout << "Test accessors on const message " << std::endl;
  const Message const_msg(std::string("message test"), -1);
  std::cout << const_msg << std::endl;
  std::cout << const_msg.GetData() << std::endl;
  std::cout << const_msg.Length() << std::endl;
  std::cout << const_msg.GetType() << std::endl;
  std::cout << const_msg.ToString() << std::endl;
  std::cout << "Test const index operator " << const_msg[0] << std::endl;

  std::cout << "Test pass \"non const message\" by const reference:" << std::endl;
  test(m);

  std::cout << "Test pass \"const message\" by const reference:" << std::endl;
  test(const_msg);

  std::cout << "Test zero len message from MSGHEADER" << std::endl;
  Message End(nullptr, 0, MessageType::STOP_SENDING);
  std::cout << "Data:" << End.GetData() << std::endl;
  std::cout << "Len :" << End.Length() << std::endl;
  std::cout << "Type:" << End.GetType() << std::endl;
  std::cout << "Str :" << End.ToString() << std::endl;

  std::cout << "Test  MSGHEADER" << std::endl;
  MSGHEADER msghdr(65000, 2);
  std::cout << "len : " << msghdr.len() << std::endl;
  std::cout << "type: " << msghdr.type() << std::endl;
  std::cout << "size: " << msghdr.SIZE() << std::endl;

  Message m2 = const_msg; // copy ctr
  std::cout << "Test copy constructor: " << m2[m2.Length()-1] << std::endl;

  m = m2; // assignment 
  std::cout << "Test assignment operator: " << (*(&m[0]) = 'l') << std::endl;

}

#endif
