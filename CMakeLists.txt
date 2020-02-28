cmake_minimum_required(VERSION 3.15)
project(ResEneAwareOpt)

set(CMAKE_CXX_STANDARD 14)

SET(CMAKE_CXX_FLAGS -pthread)

add_executable(ResEneAwareOpt main.cpp ResourceAware/resInfoCollection.cpp ResourceAware/resMonitor.cpp ResourceAware/resStateSet.cpp ResourceAware/test1/linuxserverstate.h ResourceAware/test1/linuxserverstate.cpp ResourceAware/test1/test.cpp)