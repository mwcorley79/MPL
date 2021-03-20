FROM ubuntu:latest
RUN apt-get update && apt-get install -y g++
RUN apt-get install -y cmake
RUN apt-get -y install git
RUN git clone https://github.com/mwcorley79/MPL.git
#COPY .  MPL/
WORKDIR MPL
RUN mkdir release && cd release && cmake .. -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_BUILD_TYPE=Release
RUN cd release && cmake --build . --target all 
CMD ./release/PerfTestCombinedVariableSizeMsg ; ./release/PerfTestCombinedFixedSizeMsg
