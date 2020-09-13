FROM ubuntu:bionic
RUN apt-get update && apt-get install -y g++ 
RUN apt-get install -y cmake
COPY .  MPL/
WORKDIR MPL
RUN mkdir release && cd release && cmake .. -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_BUILD_TYPE=Release
RUN cd release && cmake --build . --target all 
CMD ./release/PerfTestCombinedVariableSizeMsg ; ./release/PerfTestCombinedFixedSizeMsg
