#include "Platform.h"

 #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64)
std::atomic<int> SocketSystem::rcount = 0;
#endif 