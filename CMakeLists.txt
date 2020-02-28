cmake_minimum_required(VERSION 3.15)
project(ResEneAwareOpt)

set(CMAKE_CXX_STANDARD 14)

SET(CMAKE_CXX_FLAGS -pthread)

add_executable(ResEneAwareOpt main.cpp ResourceAware/resStateSet.cpp ResourceAware/test/linuxserverstate.h ResourceAware/test/linuxserverstate.cpp ResourceAware/test/test.cpp ResourceAware/resInfoCollection.h ResourceAware/resInfoCollection.cpp ResourceAware/resInfoMonitor.cpp)