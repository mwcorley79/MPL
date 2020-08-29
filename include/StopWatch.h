//////////////////////////////////////////////////////////////////////////////////////////////////
// A high resolution timer based on std::chrono, but wrapped in class to                        //
// resemble the Rust StopWatch interface.                                                       //
// inspired by example:  https://gist.github.com/mcleary/b0bf4fa88830ff7c882d                   // 
// Clocks reference: https://solarianprogrammer.com/2012/10/14/cpp-11-timing-code-performance/  //
// Mike Corley, 23 Aug 2020                                                                     //
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _STOPWATCH_
#define _STOPWATCH_

#include <chrono>

namespace CSE384
{
    class StopWatch
    {
    public:
        StopWatch();
        void start();
        void stop();
        int64_t elapsed_micros();

    private: 
         std::chrono::time_point<std::chrono::steady_clock> tp_start_;
         std::chrono::time_point<std::chrono::steady_clock> tp_stop_;  

      // std::chrono::time_point<std::chrono::high_resolution_clock> tp_start_;
      // std::chrono::time_point<std::chrono::high_resolution_clock> tp_stop_;  
    };
        
                                            // tp_start_(std::chrono::high_resolution_clock::now()),
    inline StopWatch::StopWatch():  tp_start_(std::chrono::steady_clock::now()),  
                                    tp_stop_(tp_start_)
    {}
                                     
    inline void StopWatch::start()
    {
        tp_start_ = tp_stop_ = std::chrono::steady_clock::now();  // std::chrono::high_resolution_clock::now();
    }


    inline void StopWatch::stop()
    {
        tp_stop_ = std::chrono::steady_clock::now(); //   std::chrono::high_resolution_clock::now();
    }

    inline int64_t StopWatch::elapsed_micros()
    {
         return std::chrono::duration_cast<std::chrono::microseconds>(tp_stop_ - tp_start_).count();
    }
} // namespace CSE384

#endif