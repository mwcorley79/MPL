#include "Message.h"

namespace CSE384
{
  Message &Message::operator=(Message &&msg)
  {
    if (&msg != this)
    {
      delete[] raw_msg_;
      raw_msg_ = msg.raw_msg_;
      raw_len_ = msg.raw_len_;
      msg.raw_msg_ = nullptr;
      msg.raw_len_ = 0;
      shadow_hdr = msg.shadow_hdr;
      msg.shadow_hdr.data = 0;
    }
    return *this;
  }

  Message &Message::operator=(const Message &msg)
  {
    if (&msg != this)
    {
      delete[] raw_msg_;
      raw_msg_ = new char[msg.RawMsgLength()];
      raw_len_ = msg.raw_len_;
      shadow_hdr = msg.shadow_hdr;
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
  MessagePtr non_const_msg = Message::CreateMessage(std::string("message test"));
  std::cout << "Test insertion << operator: " << *non_const_msg << std::endl;
  std::cout << "Test GetData: " << non_const_msg->GetData() << std::endl;
  std::cout << "Test Length() accessor: " << non_const_msg->Length() << std::endl;
  std::cout << "Test Message Type accessor:" << non_const_msg->GetType() << std::endl;
  std::cout << "Test ToString() method: " << non_const_msg->ToString() << std::endl;
  std::cout << "Test non const index operator " << (*non_const_msg)[0] << std::endl;

  std::cout << "Test accessors on copy (move) of non const message " << std::endl;
  Message m = std::move(*non_const_msg);
  std::cout << m.GetType() << std::endl;
  std::cout << m.GetData() << std::endl;
  std::cout << m.Length() << std::endl;
  std::cout << m.ToString() << std::endl;

  
  std::cout << "Test pass \"const message\" by const reference:" << std::endl;
  test(m);

  std::cout << "Test zero len message from MSGHEADER" << std::endl;
  Message End = *Message::CreateMessage(nullptr, 0, MessageType::STOP_SENDING);
  std::cout << "Data:" << End.GetData() << std::endl;
  std::cout << "Len :" << End.Length() << std::endl;
  std::cout << "Type:" << End.GetType() << std::endl;
  std::cout << "Str :" << End.ToString() << std::endl;

  std::cout << "Test  MSGHEADER" << std::endl;
  MSGHEADER msghdr(65000, 2);
  std::cout << "len : " << msghdr.len() << std::endl;
  std::cout << "type: " << msghdr.type() << std::endl;
  std::cout << "size: " << msghdr.SIZE() << std::endl;

  Message m2("tttu",4,0xA); // copy ctr
  std::cout << "Test copy constructor: " << m2[m2.Length()-1] << std::endl;

  m = m2; // assignment 
  std::cout << "Test assignment operator: " << (*(&m[0]) = 'l') << std::endl;
  std::cout << "Len :" << m.Length() << std::endl;
  std::cout << "Type:" << m.GetType() << std::endl;
  std::cout << "Str :" << m.ToString() << std::endl;
  std::cout << "Test GetRawLength() " << m.RawMsgLength() << std::endl;


  std::cout << "Test Fixed message" << std::endl;
  char buf[512];
  std::memset(buf, 's', 511);
  buf[511] = '\0';

  Message msg = Message(1024, buf, 512);
  std::cout << "Test insertion << operator: " << msg << std::endl;
  std::cout << "Test GetData: " << msg.GetData() << std::endl;
  std::cout << "Test Length() accessor: " << msg.Length() << std::endl;
  std::cout << "Test Message Type accessor:" << msg.GetType() << std::endl;
  std::cout << "Test ToString() method: " << msg.ToString() << std::endl;
  std::cout << "Test non const index operator " << msg[0] << std::endl;
  std::cout << "Test GetRawLength() " << msg.RawMsgLength() << std::endl;
 
  MessagePtr mptr = Message::CreateFixedSizeMessage(1024, std::string(buf), 0xA);
  std::cout << "Test insertion << operator: " << *mptr << std::endl;
  std::cout << "Test GetData: " << mptr->GetData() << std::endl;
  std::cout << "Test Length() accessor: " << mptr->Length() << std::endl;
  std::cout << "Test Message Type accessor:" << mptr->GetType() << std::endl;
  std::cout << "Test ToString() method: " << mptr->ToString() << std::endl;
  std::cout << "Test non const index operator " << (*mptr)[0] << std::endl;
  std::cout << "Test GetRawLength() " << mptr->RawMsgLength() << std::endl;

  MessagePtr mptr2 = Message::CreateEmptyFixedSizeMessage(1024);
  std::cout << "Test insertion << operator: " << *mptr2 << std::endl;
  std::cout << "Test GetData: " << mptr2->GetData() << std::endl;
  std::cout << "Test Length() accessor: " << mptr2->Length() << std::endl;
  std::cout << "Test Message Type accessor:" << mptr2->GetType() << std::endl;
  std::cout << "Test ToString() method: " << mptr2->ToString() << std::endl;
  std::cout << "Test non const index operator " << (*mptr2)[0] << std::endl;
  std::cout << "Test GetRawLength() " << mptr2->RawMsgLength() << std::endl;
}

#endif
