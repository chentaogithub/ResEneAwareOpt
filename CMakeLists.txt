cmake_minimum_required(VERSION 3.15)
project(ResEneAwareOpt)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -rdynamic -ldl")

add_executable(ResEneAwareOpt main.cpp ResourceAware/resStateSet.cpp ResourceAware/test/linuxserverstate.h ResourceAware/test/linuxserverstate.cpp ResourceAware/test/test.cpp ResourceAware/resInfoCollection.h ResourceAware/resInfoCollection.cpp ResourceAware/resInfoMonitor.cpp PerfEneAnalyse/ProFeatureCollection.cpp schedOptimize/schedInterface.cpp)

TARGET_LINK_LIBRARIES(ResEneAwareOpt ${CMAKE_DL_LIBS})