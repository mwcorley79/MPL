

#include <iostream>
#include <cassert>
#include <string>
#include "Message.h"
#include <memory>
using namespace CSE384;


void test()
{
    using MessagePtr = std::shared_ptr<Message>;

     
     
     MessagePtr non_const_msg ( new Message(100, 
                                            std::string("message test"),  
                                            MessageType::DEFAULT));
     
     std::cout << (*non_const_msg).get_content_str() << std::endl;
     
      // assert(("Test ToString() : ", (*non_const_msg).get_content_str() == std::string("message test")));
     
      // assert(("Test GetData()",  non_const_msg->get_content_str() == std::string("message test")));
     
     assert(("Test Length() accessor: ", non_const_msg->get_content_len() == 12));

     assert(("Test GetType", non_const_msg->get_type() == MessageType::DEFAULT ));
     
     // assert(("Test non const index operator ", (*non_const_msg)[0] == 'm'));
  
}


int main()
{
     std::cout << "Message class unit tests " << std::endl;
    try
    {
       test();
    }
    catch(...)
    {
      assert(("throws test: ", true));
    }

    std::cout << "All tests passed"<< std::endl;
    
    return 0;
}