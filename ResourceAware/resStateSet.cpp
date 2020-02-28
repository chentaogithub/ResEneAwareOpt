//
// Created by lynne on 2/26/20.
//
#include <iostream>
#include "resInfoCollection.h"
using namespace std;
using namespace RESOURCE_INFO;

#define HIGH_CPU_USAGE 90
#define LOW_CPU_USAGE 40
#define HIGH_MEM_USAGE 70
#define LOW_MEM_USAGE 10

enum ResState {
    CPU_MEM_BOUND = 1,//cpu和内存资源都很紧张
    CPU_BOUND,
    MEM_BOUND,
    CPU_ENOUGH,
    MEM_ENOUGH,
    OTHER
};
int getResState(ResourceInfo& resourceInfo){
    SysMemInfo sysMemInfo;
    if(resourceInfo.sysTimeInfo.cpuUsage>HIGH_CPU_USAGE && sysMemInfo.memUsage>HIGH_MEM_USAGE) return CPU_MEM_BOUND;
    if(resourceInfo.sysTimeInfo.cpuUsage>HIGH_CPU_USAGE) return CPU_BOUND;
    resourceInfo.GetMemInfo(sysMemInfo);
    if(sysMemInfo.memUsage>HIGH_MEM_USAGE) return MEM_BOUND;
    if(resourceInfo.sysTimeInfo.cpuUsage<LOW_CPU_USAGE) return CPU_ENOUGH;
    if(sysMemInfo.memUsage<LOW_MEM_USAGE) return MEM_ENOUGH;
    return OTHER;
}

int main(int argc, char *argv[]) {
    ResourceInfo stat;
    while(true){
        sleep(1);
        std::cout<<"the resorce state is: "<< getResState(stat)<<endl;
    }
}