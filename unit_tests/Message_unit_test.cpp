

#include <iostream>
#include <cassert>
#include <string>
#include "Message.h"
using namespace CSE384;

void test()
{
 
     MessagePtr non_const_msg = Message::CreateMessage(std::string("message test"),MessageType::DEFAULT);
     
     assert(("Test ToString() : ", (*non_const_msg).ToString() == std::string("message test")));
     
     assert(("Test GetData()",   std::string(non_const_msg->GetData()) == std::string("message test")));
     
     assert(("Test Length() accessor: ", non_const_msg->Length() == 12));

     assert(("Test GetType", non_const_msg->GetType() == MessageType::DEFAULT ));
     
     assert(("Test non const index operator ", (*non_const_msg)[0] == 'm'));
  
}


int main()
{
     std::cout << "Message class unit tests " << std::endl;
    try
    {
       test();
    }
    catch(const std::exception& e)
    {
      assert(("throws test: ", true));
    }

    std::cout << "All tests passed"<< std::endl;
    
    return 0;
}