//
// Created by lynne on 2/27/20.
//

#include <iostream>
#include <dirent.h>
#include <climits>
#include "resInfoCollection.h"

using namespace std;
using namespace RESOURCE_INFO;

struct ResInfoSchema{
    SysTimeInfo sysTimeInfo;
    SysMemInfo sysMemInfo;
    SysDiskInfo sysDiskInfo;
    SysNetInfo sysNetInfo;
    SysProcInfo sysProcInfo;
    SysNetConnInfo sysNetConnInfo;
};

void getResInfo(ResourceInfo& stat){

    //ResourceInfo stat;
    //cpu利用率
    cout << "cpu_use:" << stat.GetCpuUsage() << endl;

    //unsigned long cpuTime,idleTime;
    cout << "cpuTime:" << stat.sysTimeInfo.cpuTime << endl;
    cout << "idleTime:" << stat.sysTimeInfo.idleTime << endl;
    cout << "cpuUsage:" << stat.sysTimeInfo.cpuUsage << endl;

    //内存
    SysMemInfo meminfo;
    stat.GetMemInfo(meminfo);
    cout << "meminfo:" << endl;
    cout << "\ttotal:" << meminfo.total << "M" << endl;
    cout << "\tfree:" << meminfo.free << "M" << endl;
    cout << "\t" << ((double) (meminfo.total - meminfo.free)) * 100 / meminfo.total << endl;

    //磁盘
    vector <SysDiskInfo> vdiskinfo;
    unsigned long disktotal, diskavail;
    stat.GetDiskInfo(vdiskinfo, disktotal, diskavail);
    cout << "diskinfo:" << endl;
    cout << "\ttotal:" << disktotal << "M" << endl;
    cout << "\tfree:" << diskavail << "M" << endl;
    cout << "\t" << ((double) (disktotal - diskavail)) * 100 / disktotal << endl;
    vector<SysDiskInfo>::iterator itdisk = vdiskinfo.begin();
    cout << endl;
    for (; itdisk != vdiskinfo.end(); ++itdisk) {
        cout << "\tname:" << itdisk->name << endl;
        cout << "\ttotal:" << itdisk->total << "M" << endl;
        cout << "\tfree:" << itdisk->free << "M" << endl;
        cout << endl;
    }
    //NET信息
    SysNetInfo netinfo = stat.GetNetInfo();
    cout << "netinfo:" << endl;
    cout << "\trecv:" << netinfo.recv << "kb" << endl;
    cout << "\tsend:" << netinfo.send << "kb" << endl;
    cout << "\ttotal:" << netinfo.total << "kb" << endl;

    //进程process资源信息
    vector <SysProcInfo> procinfo;
    stat.GetProcInfo(procinfo);
    cout << "procinfo:" << endl;
    vector<SysProcInfo>::iterator itproc = procinfo.begin();
    for (; itproc != procinfo.end(); ++itproc) {
        cout << "\tname:" << itproc->name << endl;
        cout << "\tpid:" << itproc->pid << endl;
        cout << "\tmem:" << itproc->mem << "Kb" << endl;
        cout << "\tcpu:" << itproc->cpu << endl;
        cout << endl;
    }

    //连接信息
    vector <SysNetConnInfo> netConnInfo;
    stat.GetNetConnectionInfo(netConnInfo);
    vector<SysNetConnInfo>::iterator itconn = netConnInfo.begin();
    cout << "netinfo:" << endl;
    for (; itconn != netConnInfo.end(); ++itconn) {
        cout << "\tid:" << itconn->id << endl;
        cout << "\tprotocol:" << itconn->protocol << endl;
        cout << "\tlocalAddr:" << itconn->localAddr << ":" << itconn->localPort << endl;
        cout << "\tremoteAddr:" << itconn->remoteAddr << ":" << itconn->remotePort << endl;
        cout << "\tstate:" << itconn->state << endl;
        cout << endl;
    }

}

int monitor(int time,int& flag) {
    ResourceInfo stat;
    while (flag) {
        sleep(time);
        getResInfo(stat);
    }
}
