#include <iostream>
#include <dirent.h>
#include <climits>
#include "linuxserverstate.h"

using namespace std;
using namespace LINUXSERV_STATE;

int main(int argc, char *argv[])
{

    //cpu利用率
    LinuxServerState stat;

    cout << "cpu_use:"<<stat.GetCpuUsage()<<endl;

    unsigned long cpuTime,idleTime;
    //get_cpuusage(cpuTime,idleTime);
    cout<<"cpuTime:"<<cpuTime<<endl;
    cout<<"idleTime:"<<idleTime<<endl;

    //内存
    SysMemInfo meminfo;
    stat.GetMemInfo(meminfo);
    cout<<"meminfo:"<<endl;
    cout<<"\ttotal:"<<meminfo.total<<"M"<<endl;
    cout<<"\tfree:"<<meminfo.free<<"M"<<endl;
    cout<<"\t"<<((double)(meminfo.total - meminfo.free))*100/meminfo.total<<endl;

    //磁盘
    vector<SysDiskInfo> vdiskinfo;
    unsigned long disktotal,diskavail;
    stat.GetDiskInfo(vdiskinfo,disktotal,diskavail);
    cout<<"diskinfo:"<<endl;
    cout<<"\ttotal:"<<disktotal<<"M"<<endl;
    cout<<"\tfree:"<<diskavail<<"M"<<endl;
    cout<<"\t"<<((double)(disktotal - diskavail))*100/disktotal<<endl;
    vector<SysDiskInfo>::iterator itdisk = vdiskinfo.begin();
    cout<<endl;
    for(; itdisk != vdiskinfo.end(); ++itdisk)
    {
        cout<<"\tname:"<<itdisk->name<<endl;
        cout<<"\ttotal:"<<itdisk->total<<"M"<<endl;
        cout<<"\tfree:"<<itdisk->free<<"M"<<endl;
        cout<<endl;
    }
    //NET信息
    SysNetInfo  netinfo = stat.GetNetInfo();
    cout<<"netinfo:"<<endl;
    cout<<"\trecv:"<<netinfo.recv<<"kb"<<endl;
    cout<<"\tsend:"<<netinfo.send<<"kb"<<endl;
    cout<<"\ttotal:"<<netinfo.total<<"kb"<<endl;

    //proc信息
    sleep(5);
    vector<SysProcInfo> procinfo ;
    stat.GetProcInfo(procinfo);
    cout<<"procinfo:"<<endl;
    vector<SysProcInfo>::iterator itproc = procinfo.begin();


    for(; itproc != procinfo.end(); ++itproc)
    {
        cout<<"\tname:"<<itproc->name<<endl;
        cout<<"\tpid:"<<itproc->pid<<endl;
        cout<<"\tmem:"<<itproc->mem<<"Kb"<<endl;
        cout<<"\tcpu:"<<itproc->cpu<<endl;
        cout<<endl;
    }
    //连接信息
    vector<SysNetConnInfo> netConnInfo;
    stat.GetNetConnectionInfo(netConnInfo);
    vector<SysNetConnInfo>::iterator itconn = netConnInfo.begin();
    cout<<"netinfo:"<<endl;
    for(; itconn!= netConnInfo.end(); ++itconn)
    {
        cout<<"\tid:"<<itconn->id<<endl;
        cout<<"\tprotocol:"<<itconn->protocol<<endl;
        cout<<"\tlocalAddr:"<<itconn->localAddr<<":"<<itconn->localPort<<endl;
        cout<<"\tremoteAddr:"<<itconn->remoteAddr<<":"<<itconn->remotePort<<endl;
        cout<<"\tstate:"<<itconn->state<<endl;
        cout<<endl;
    }
}

/**
int main(int argc, char *argv[]){
    struct dirent* ent  = NULL;
    char *endptr        = NULL;
    DIR* pDirProc       = opendir("/proc");
    while((ent = readdir(pDirProc))!= NULL) {
        //如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
        /**
        if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0))
            continue;
        if (DT_DIR != ent->d_type)
            continue;
        //long pidNum = ent->d_name;
        long pidNum = strtol(ent->d_name, &endptr, 10);
        if (pidNum == LONG_MAX || pidNum == LONG_MIN || endptr == ent->d_name)//
            continue;

        long pidNum = strtol(ent->d_name,&endptr,10);
        if(pidNum == LONG_MAX || pidNum == LONG_MIN || endptr == ent->d_name )//
            continue;
        //cout << ent->d_name << ": ";
        //long pidNum = strtol(ent->d_name, &endptr, 10);
        cout << ent->d_name << ": "<<pidNum << endl;
    }
    return 0;
}
**/