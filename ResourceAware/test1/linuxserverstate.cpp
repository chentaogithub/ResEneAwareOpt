
#include "linuxserverstate.h"
#include <sys/sysinfo.h>//getmemInfo
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <climits>
#include <cstring>
#include <time.h>
#include <dirent.h>
#include <map>
#include <errno.h>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;
namespace LINUXSERV_STATE {


    void get_netstat(unsigned long& inOctets,unsigned long& outOctets,time_t& sysTime)
    {
        unsigned long   inNoRoutes = 0, inTruncPkts = 0;
        unsigned long   inMcastPkts = 0, outMcastPkts = 0;
        unsigned long   inBcastPkts = 0, outBcastPkts = 0;
        unsigned long   inMcastOctets = 0, outMcastOctets = 0;
        unsigned long   inBcastOctets = 0, outBcastOctets = 0;
        fstream     fProcNetStat("/proc/net/netstat",ios_base::in);
        char        bufNetStat[2048] = {'\0'}, ipExt[16] = {'\0'};
        while(!fProcNetStat.eof())
        {
            fProcNetStat.getline(bufNetStat,2048);
            if(strncasecmp(bufNetStat,"tcpext",6) == 0)
                continue;
            if(strncasecmp(bufNetStat,"ipext",5) == 0)
            {
                fProcNetStat.getline(bufNetStat,2048);
                sscanf(bufNetStat,"%s %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",ipExt,
                       &inNoRoutes,&inTruncPkts,&inMcastPkts,&outMcastPkts,&inBcastPkts,&outBcastPkts,
                       &inOctets,&outOctets,&inMcastOctets,&outMcastOctets,&inBcastOctets,&outBcastOctets);
                sysTime = time(NULL);
                break;
            }
        }
    }

    void get_cpuusage(unsigned long& cpuTime,unsigned long& idleTime)
    {
        fstream     fProcStat("/proc/stat",ios_base::in);
        char          cpubuf[8] = {'\0'};
        unsigned long userT = 0, niceT = 0, systemT = 0, idleT = 0;
        unsigned long ioT = 0, irqT = 0, softirqT = 0;
        fProcStat>>cpubuf>>userT>>niceT>>systemT>>idleT>>ioT>>irqT>>softirqT;
        cpuTime = userT + niceT + systemT + idleT + ioT + irqT + softirqT;
        idleTime = idleT;
    }

    void get_procinfo(LinuxServerState* pMain,map<pid_t,SysProcTimeInfo>& procTimeInfo)
    {
        struct dirent* ent  = NULL;
        char *endptr        = NULL;
        DIR* pDirProc       = opendir("/proc");
        long utime = 0, stime = 0;

        if(pDirProc == NULL)
            return ;

        char dirName[256] = {'\0'};
        char tmpbuf[2048] = {'\0'};
        if (pMain->m_procinfo_cal == NULL )
            return;

        pMain->m_procinfo_cal->clear();
        while((ent = readdir(pDirProc))!= NULL)
        {
            //如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0))
                continue;
            if (DT_DIR != ent->d_type)
                continue;
            //long pidNum = ent->d_name;
            long pidNum = strtol(ent->d_name,&endptr,10);
            if(pidNum == LONG_MAX || pidNum == LONG_MIN || endptr == ent->d_name )//
                continue;

            fstream fPidStat,fPidMem;
            int i = 0;
            SysProcTimeInfo tmpProcTimeInfo;
            SysProcInfo     tmpProcInfo;
            //sprintf(dirName, "/proc/%s/status", ent->d_name);//生成要读取的文件的路径
            snprintf(dirName,256,"%s%d%s","/proc/",pidNum,"/stat");
            try
            {
                fPidStat.open(dirName,ios_base::in);
            }
            catch(std::exception& e)
            {
                continue;
            }

            fPidStat>>tmpbuf;//获取pid
            i++;
            fPidStat>>tmpbuf;//获取进程名字，名字由"()"包括
            i++;

            // process name
            if(strlen(tmpbuf) > 2)
                tmpProcInfo.name  = string(tmpbuf).substr(1,strlen(tmpbuf) -2);
            else
                continue;

            for(; i < 14;i++)
                fPidStat>>tmpbuf;

            //14项表示utime,15项表示stime
            fPidStat >> utime;
            i++;
            fPidStat >> stime;
            i++;
            tmpProcTimeInfo.user    = utime;
            tmpProcTimeInfo.kernel  = stime;

            // process cpu usage
            map<pid_t,SysProcTimeInfo>::iterator it = procTimeInfo.find((pid_t)pidNum);
            if(it == procTimeInfo.end())
            {
                procTimeInfo.insert(pair<pid_t,SysProcTimeInfo>(pidNum,tmpProcTimeInfo));
                tmpProcInfo.cpu = 0;
            }
            else
            {
                tmpProcInfo.cpu = utime + stime - it->second.kernel - it->second.user;
                if(tmpProcInfo.cpu > 100)
                    tmpProcInfo.cpu = 100;
                if(tmpProcInfo.cpu < 0)
                    tmpProcInfo.cpu = 0;
                it->second.kernel   = stime;
                it->second.user     = utime;
            }

            utime           = 0;
            stime           = 0;
            tmpProcInfo.pid = pidNum;

            // process mem
            char *pstrtolong;
            memset(dirName,'\0',128);
            snprintf(dirName,128,"%s%d%s","/proc/",pidNum,"/status");
            fPidMem.open(dirName,ios_base::in);
            while(!fPidMem.eof())
            {
                fPidMem >> tmpbuf;
                if(strncasecmp(tmpbuf,"vmrss",5) == 0)
                {
                    fPidMem >> tmpbuf;
                    unsigned long vmrss = strtol(tmpbuf,&pstrtolong,10);
                    if(vmrss == LONG_MAX || vmrss == LONG_MIN || pstrtolong == tmpbuf)
                    {
                        tmpProcInfo.mem = -1;
                    }
                    else
                        tmpProcInfo.mem = vmrss;
                    if(0 == pthread_mutex_trylock(&pMain->m_mutex))
                    {
                        pMain->m_procinfo_cal->push_back(tmpProcInfo);
                        pthread_mutex_unlock(&pMain->m_mutex);
                    }
                    break;
                }
            }
            memset(dirName,'\0',256);
            memset(tmpbuf,'\0',2048);
        }

        closedir(pDirProc);
        pDirProc    = NULL;

        if(0 == pthread_mutex_trylock(&pMain->m_mutex))
        {
            std::swap(pMain->m_procinfo_cal ,pMain->m_procinfo);
            pthread_mutex_unlock(&pMain->m_mutex);
        }
    }


    void* CPUNetProcessCount(void* arg)
    {
        LinuxServerState* pMain = (LinuxServerState*)arg;
        unsigned long   inOctets = 0, outOctets = 0;
        time_t          sysTime;
        unsigned long   cpuTime = 0, idleTime = 0;
        map<pid_t,SysProcTimeInfo> procTimeInfo;

        while(pMain->m_running)
        {
            //cpu total usage
            unsigned long cpuTimeNow = 0, idleTimeNow = 0;
            int cpu = 0;
            get_cpuusage(cpuTimeNow,idleTimeNow);

            cpu = 100 - (idleTimeNow - idleTime)*100/(cpuTimeNow - cpuTime);
            idleTime  = idleTimeNow;
            cpuTime   = cpuTimeNow;

            if(0 == pthread_mutex_trylock(&pMain->m_mutex))
            {
                pMain->m_cpuUsage = cpu;
                pthread_mutex_unlock(&pMain->m_mutex);
            }
            //else暂时不处理

            //net stat
            unsigned long inOctetsTmp = 0,outOctetsTmp = 0;
            time_t    timeNow = time(NULL);
            get_netstat(inOctetsTmp,outOctetsTmp,timeNow);

            SysNetInfo tmpNetInfo;
            tmpNetInfo.recv     = (inOctetsTmp - inOctets)/1024/(timeNow - sysTime);
            tmpNetInfo.send     = (outOctetsTmp - outOctets)/1024/(timeNow - sysTime);
            tmpNetInfo.total    = tmpNetInfo.recv + tmpNetInfo.send;
            sysTime   = timeNow;
            inOctets  = inOctetsTmp;
            outOctets = outOctetsTmp;
            if(0 == pthread_mutex_trylock(&pMain->m_mutex))
            {
                pMain->m_netInfo = tmpNetInfo;
                pthread_mutex_unlock(&pMain->m_mutex);
            }

            //proc info
            get_procinfo(pMain,procTimeInfo);

            sleep(1);
        }
        return NULL;
    }

    /**
    void *CPUNetProcessCount(void *arg) {
        LinuxServerState *pMain = (LinuxServerState *) arg;
        //get all cpu usage
        unsigned long cpuTime, idleTime;
        unsigned long userT = 0, niceT = 0, systemT = 0, idleT = 0, ioT = 0, irqT = 0, softirqT = 0;
        char cpubuf[8] = {'\0'};
        fstream fProcStat("/proc/stat", ios_base::in);
        fProcStat >> cpubuf >> userT >> niceT >> systemT >> idleT >> ioT >> irqT >> softirqT;
        cpuTime = userT + niceT + systemT + idleT + ioT + irqT + softirqT;
        idleTime = idleT;
        //get net stat
        fstream fProcNetStat("/proc/net/netstat", ios_base::in);
        char bufNetStat[2048] = {'\0'}, ipExt[16] = {'\0'};
        unsigned long inNoRoutes = 0, inTruncPkts = 0, inMcastPkts = 0, outMcastPkts = 0, inBcastPkts = 0, outBcastPkts = 0;
        unsigned long inOctets = 0, outOctets = 0, inMcastOctets = 0, outMcastOctets = 0, inBcastOctets = 0, outBcastOctets = 0;
        time_t sysTime;
        while (!fProcNetStat.eof()) {
            fProcNetStat.getline(bufNetStat, 2048);
            if (strncasecmp(bufNetStat, "tcpext", 6) == 0)
                continue;
            if (strncasecmp(bufNetStat, "ipext", 5) == 0) {
                fProcNetStat.getline(bufNetStat, 2048);
                sscanf(bufNetStat, "%s %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", ipExt,
                       &inNoRoutes, &inTruncPkts, &inMcastPkts, &outMcastPkts, &inBcastPkts, &outBcastPkts,
                       &inOctets, &outOctets, &inMcastOctets, &outMcastOctets, &inBcastOctets, &outBcastOctets);
                sysTime = time(NULL);
                break;
            }
        }
        //initialize proc info
        char dirName[128] = {'\0'}, tmpbuf[2048] = {'\0'};
        long utime, stime;

        struct dirent *ent = NULL;
        char pidBuf[128] = {'\0'}, *endptr;
        DIR *pDirProc = opendir("/proc");
        map<pid_t, SysProcTimeInfo> procTimeInfo;
        if (pDirProc == NULL)
            return NULL;
        while ((ent = readdir(pDirProc)) != NULL) {
            long pidNum = strtol(ent->d_name, &endptr, 10);
            if (pidNum == LONG_MAX || pidNum == LONG_MIN || endptr == ent->d_name) {
                continue;
            } else {
                //初始化时间
                fstream fPidStat, fPidMem;
                int i = 0;
                SysProcTimeInfo tmpProcTimeInfo;
                SysProcInfo tmpProcInfo;
                snprintf(dirName, 128, "%s%d%s", "/proc/", pidNum, "/stat");
                try {
                    fPidStat.open(dirName, ios_base::in);
                }
                catch (std::exception &e) {
                    return NULL;
                }
                fPidStat >> tmpbuf;//获取pid
                i++;
                fPidStat >> tmpbuf;//获取进程名字，名字由“（）”包括
                i++;
                if (strlen(tmpbuf) > 2)
                    tmpProcInfo.name = string(tmpbuf).substr(1, strlen(tmpbuf) - 2);
                else
                    continue;
                for (; i < 14; i++)
                    fPidStat >> tmpbuf;

                //14项表示utime,15项表示stime
                fPidStat >> utime;
                i++;
                fPidStat >> stime;
                i++;
                tmpProcTimeInfo.user = utime;
                tmpProcTimeInfo.kernel = stime;
                utime = 0;
                stime = 0;

                procTimeInfo.insert(pair<pid_t, SysProcTimeInfo>(pidNum, tmpProcTimeInfo));
                tmpProcInfo.cpu = 0;
                tmpProcInfo.pid = pidNum;

                //初始化内存,
                char *pstrtolong;
                memset(dirName, '\0', 128);
                snprintf(dirName, 128, "%s%d%s", "/proc/", pidNum, "/status");
                fPidMem.open(dirName, ios_base::in);
                while (!fPidMem.eof()) {
                    fPidMem >> tmpbuf;
                    if (strncasecmp(tmpbuf, "vmrss", 5) == 0) {
                        fPidMem >> tmpbuf;
                        unsigned long vmrss = strtol(tmpbuf, &pstrtolong, 10);
                        if (vmrss == LONG_MAX || vmrss == LONG_MIN || pstrtolong == tmpbuf) {
                            tmpProcInfo.mem = -1;
                        } else
                            tmpProcInfo.mem = vmrss;
                        if (0 == pthread_mutex_trylock(&pMain->m_mutex)) {
                            pMain->m_procInfo.push_back(tmpProcInfo);
                            pthread_mutex_unlock(&pMain->m_mutex);
                        }
                        break;
                    }
                }

                memset(dirName, '\0', 128);
                memset(tmpbuf, '\0', 2048);
            }
        }
        closedir(pDirProc);

        pDirProc = NULL;
        while (1) {
            sleep(1);
            //cpu usage
            unsigned long cpuTimeNow = 0, idleTimeNow = 0;
            int cpu = 0;
            fProcStat.seekg(0, ios_base::beg);
            fProcStat >> cpubuf >> userT >> niceT >> systemT >> idleT >> ioT >> irqT >> softirqT;
            cpuTimeNow = userT + niceT + systemT + idleT + ioT + irqT + softirqT;
            idleTimeNow = idleT;

            cpu = 100 - (idleTimeNow - idleTime) * 100 / (cpuTimeNow - cpuTime);
            idleTime = idleTimeNow;
            cpuTime = cpuTimeNow;
            if (0 == pthread_mutex_trylock(&pMain->m_mutex)) {
                pMain->m_cpuUsage = cpu;
                pthread_mutex_unlock(&pMain->m_mutex);
            }
            //else暂时不处理
            //net stat
            unsigned long inOctetsTmp = 0, outOctetsTmp = 0;
            fProcNetStat.seekg(0, ios_base::beg);
            time_t timeNow = time(NULL);
            while (!fProcNetStat.eof()) {
                fProcNetStat.getline(bufNetStat, 2048);
                if (strncasecmp(bufNetStat, "tcpext", 6) == 0)
                    continue;
                if (strncasecmp(bufNetStat, "ipext", 5) == 0) {
                    fProcNetStat.getline(bufNetStat, 2048);
                    sscanf(bufNetStat, "%s %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", ipExt,
                           &inNoRoutes, &inTruncPkts, &inMcastPkts, &outMcastPkts, &inBcastPkts, &outBcastPkts,
                           &inOctetsTmp, &outOctetsTmp, &inMcastOctets, &outMcastOctets, &inBcastOctets,
                           &outBcastOctets);
                    timeNow = time(NULL);
                    break;
                }
            }
            SysNetInfo tmpNetInfo;
            tmpNetInfo.recv = (inOctetsTmp - inOctets) / 1024 / (timeNow - sysTime);
            tmpNetInfo.send = (outOctetsTmp - outOctets) / 1024 / (timeNow - sysTime);
            tmpNetInfo.total = tmpNetInfo.recv + tmpNetInfo.send;

            sysTime = timeNow;
            inOctets = inOctetsTmp;
            outOctets = outOctetsTmp;
            if (0 == pthread_mutex_trylock(&pMain->m_mutex)) {
                pMain->m_netInfo = tmpNetInfo;
                pthread_mutex_unlock(&pMain->m_mutex);
            }
            //proc info
            if (0 == pthread_mutex_trylock(&pMain->m_mutex)) {
                pMain->m_procInfo.clear();
                pthread_mutex_unlock(&pMain->m_mutex);
            }
            pDirProc = opendir("/proc");
            while ((ent = readdir(pDirProc)) != NULL) {
                long pidNum = strtol(ent->d_name, &endptr, 10);
                if (pidNum == LONG_MAX || pidNum == LONG_MIN || endptr == ent->d_name) {
                    continue;
                } else {
                    //初始化时间
                    fstream fPidStat, fPidMem;
                    int i = 0;
                    snprintf(dirName, 128, "%s%d%s", "/proc/", pidNum, "/stat");
                    fPidStat.open(dirName, ios_base::in);
                    fPidStat >> tmpbuf;//获取pid
                    i++;
                    fPidStat >> tmpbuf;//获取进程名字，名字由“（）”包括
                    i++;
                    SysProcInfo tmpProcInfo;
                    if (strlen(tmpbuf) > 2)
                        tmpProcInfo.name = string(tmpbuf).substr(1, strlen(tmpbuf) - 2);
                    else
                        cout << "pid:\t" << pidNum << "tmpbuf:\t" << tmpbuf << endl;
                    for (; i < 14; i++) {
                        fPidStat >> tmpbuf;
                    }
                    //14项表示utime,15项表示stime
                    fPidStat >> utime;
                    fPidStat >> stime;
                    map<pid_t, SysProcTimeInfo>::iterator it = procTimeInfo.find((pid_t) pidNum);
                    if (it == procTimeInfo.end()) {
                        SysProcTimeInfo tmpProcTimeInfo;
                        tmpProcTimeInfo.user = utime;
                        tmpProcTimeInfo.kernel = stime;
                        procTimeInfo.insert(pair<pid_t, SysProcTimeInfo>(pidNum, tmpProcTimeInfo));
                        tmpProcInfo.cpu = 0;
                    } else {
                        tmpProcInfo.cpu = utime + stime - it->second.kernel - it->second.user;
                        if (tmpProcInfo.cpu > 100)
                            tmpProcInfo.cpu = 100;
                        if (tmpProcInfo.cpu < 0)
                            tmpProcInfo.cpu = 0;
                        it->second.kernel = stime;
                        it->second.user = utime;
                    }
                    utime = 0;
                    stime = 0;
                    tmpProcInfo.pid = pidNum;
                    //获取内存
                    char *pstrtolong;
                    memset(dirName, '\0', 128);
                    snprintf(dirName, 128, "%s%d%s", "/proc/", pidNum, "/status");
                    fPidMem.open(dirName, ios_base::in);
                    while (!fPidMem.eof()) {
                        fPidMem >> tmpbuf;
                        if (strncasecmp(tmpbuf, "vmrss", 5) == 0) {
                            fPidMem >> tmpbuf;
                            unsigned long vmrss = strtol(tmpbuf, &pstrtolong, 10);
                            if (vmrss == LONG_MAX || vmrss == LONG_MIN || pstrtolong == tmpbuf) {
                                tmpProcInfo.mem = -1;
                            } else
                                tmpProcInfo.mem = vmrss;
                            if (0 == pthread_mutex_trylock(&pMain->m_mutex)) {
                                pMain->m_procInfo.push_back(tmpProcInfo);
                                pthread_mutex_unlock(&pMain->m_mutex);
                            }
                            break;
                        }
                    }
                    memset(dirName, '\0', 128);
                    memset(tmpbuf, '\0', 2048);
                }
            }
            closedir(pDirProc);
        }//main

        return NULL;
    }
    **/

    LinuxServerState::LinuxServerState() {
        m_cpuUsage = 0;
        m_countThread = 0;
        m_threadRet = NULL;
        m_netInfo.total = 0;
        m_netInfo.recv = 0;
        m_netInfo.send = 0;

        m_procinfo_cal      = new vector<SysProcInfo>();
        m_procinfo          = new vector<SysProcInfo>();
        m_running           = true;

        initialize();
    }

    LinuxServerState::LinuxServerState(const LinuxServerState &other) {
    }

    LinuxServerState::~LinuxServerState() {
        if (m_countThread != 0)
            pthread_join(m_countThread, NULL);
        pthread_mutex_destroy(&m_mutex);
    }

    LinuxServerState &LinuxServerState::operator=(const LinuxServerState &other) {
        return *this;
    }

    bool LinuxServerState::operator==(const LinuxServerState &other) const {
///TODO: return ...;
    }

    void LinuxServerState::initialize() {
        //pthread_mutexatt matt;
        //pthread_mutexattr_init(&matt);

        pthread_mutex_init(&m_mutex, NULL);
        pthread_create(&m_countThread, NULL, &CPUNetProcessCount, this);
    }

    int LinuxServerState::GetCpuUsage() {
        timespec tmp;
        tmp.tv_nsec = 0;
        tmp.tv_sec = 1;
        int cpu = 0;
        if (0 == pthread_mutex_timedlock(&m_mutex, &tmp)) {
            cpu = m_cpuUsage;
            pthread_mutex_unlock(&m_mutex);
        } else
            return -1;
        if (cpu > 100)
            return 100;
        if (cpu < 0)
            return 0;
        return cpu;
    }

    int LinuxServerState::GetMemInfo(SysMemInfo &memInfo) {
        struct sysinfo tmp;
        int ret = 0;
        ret = sysinfo(&tmp);
        if (ret == 0) {
            memInfo.free = (unsigned long) tmp.freeram / (1024 * 1024);
            memInfo.total = (unsigned long) tmp.totalram / (1024 * 1024);
        }
        return ret;
    }

    int
    LinuxServerState::GetDiskInfo(std::vector<SysDiskInfo> &diskInfo, unsigned long &total, unsigned long &available) {
        total = 0;
        available = 0;
        char buf[128] = {'\0'};
        string fileName;
        char *p;
        fstream fProc("/proc/partitions", ios_base::in);
        unsigned long total2 = 0, free2 = 0, sumTotal = 0, sumFree = 0;
        int major = 0, minor = 0;
        //首先计算总大小及各个分区的大小，包括交换分区，然后用df命令获取可用空间
        while (!fProc.eof()) {
            fProc >> fileName;
            total2 = strtol(fileName.c_str(), &p, 10);
            if (total2 == 0)
                continue;
            else if (total2 == LONG_MAX || total2 == LONG_MIN || p == fileName.c_str())
                return total2;
            else {
                major = total2;
                fProc >> minor;
                fProc >> total2;

                fProc >> fileName;

                SysDiskInfo tmp;
                tmp.total = (unsigned long) (total2 / 1024);
                tmp.free = 0;
                tmp.name = fileName;

                diskInfo.push_back(tmp);
                if ((fileName.find("sd") != string::npos || fileName.find("hd") != string::npos) &&
                    !(fileName[fileName.size() - 1] >= '0' && fileName[fileName.size() - 1] <= '9'))
                    sumTotal += tmp.total;
                fileName.clear();
            }
        }
        //获取可用空间
        const char *command = "df";
        const char *type = "r";
        FILE *pFile = popen(command, type);
        char buf2[1024] = {'\0'};
        char filesys[32] = {'\0'}, usage[8] = {'\0'}, mountedon[32] = {'\0'};
        unsigned long kblocks = 0, used = 0, available2 = 0;
        if (pFile == NULL)
            return -1;
        vector<SysDiskInfo>::iterator it;
        while (!feof(pFile)) {
            if (fgets(buf2, 1024, pFile) == NULL)
                break;
            sscanf(buf2, "%s %ld %ld %ld %s %s", filesys, &kblocks, &used, &available2, usage, mountedon);
            string ff(filesys);
            if (ff.rfind("/") == string::npos)
                continue;
            for (it = diskInfo.begin(); it != diskInfo.end(); it++) {
                if (it->name == ff.substr(ff.rfind("/") + 1)) {
                    it->free = (unsigned long) (available2 / 1024);
                    sumFree += it->free;
                }
            }
            memset(buf2, '\0', 1024);
            memset(filesys, '\0', 32);
            memset(mountedon, '\0', 32);
            memset(usage, '\0', 8);
            kblocks = 0;
            used = 0;
            available2 = 0;
        }
        pclose(pFile);
        total = sumTotal;
        available = sumFree;
        return 0;
    }

    SysNetInfo LinuxServerState::GetNetInfo() {
        SysNetInfo tmpNetInfo;
        tmpNetInfo.recv = 0;
        tmpNetInfo.send = 0;
        tmpNetInfo.total = 0;
        timespec tmp;
        tmp.tv_nsec = 0;
        tmp.tv_sec = 1;
        if (0 == pthread_mutex_timedlock(&m_mutex, &tmp)) {
            tmpNetInfo = m_netInfo;
            pthread_mutex_unlock(&m_mutex);
        }
        return tmpNetInfo;
    }

    void LinuxServerState::GetProcInfo(std::vector<SysProcInfo>& procInfo) {
        timespec tmp;
        tmp.tv_nsec = 0;
        tmp.tv_sec = 1;

        if (0 == pthread_mutex_timedlock(&m_mutex, &tmp)) {
            procInfo = (*m_procinfo);
            pthread_mutex_unlock(&m_mutex);
        }
    }

    void LinuxServerState::GetNetConnectionInfo(std::vector<SysNetConnInfo> &netConnInfo) {
        int i = 0, id = 0;
        char buf[1024] = {'\0'};
        while (i++ < 4) {
            fstream fInfo;
            SockType stateType = UNKNOWN;

            if (i == 1) {
                fInfo.open("/proc/net/tcp", ios_base::in);
                stateType = TCP;
            } else if (i == 2) {
                fInfo.open("/proc/net/udp", ios_base::in);
                stateType = UDP;
            } else if (i == 3) {
                fInfo.open("/proc/net/tcp6", ios_base::in);
                stateType = TCP6;
            } else if (i == 4) {
                fInfo.open("/proc/net/udp6", ios_base::in);
                stateType = UDP6;
            }
            if (stateType == UNKNOWN)
                continue;

            while (!fInfo.eof()) {
                SysNetConnInfo tmpConnInfo;

                char sl[6] = {'\0'}, localaddr[32] = {'\0'}, remoteaddr[32] = {'\0'};
                fInfo.getline(buf, 1024);
                unsigned long llocal = 0, lremote = 0;
                int localPort = 0, remotePort = 0, st = 0;
                int ret;
                if (i == 1 || i == 2) {
                    ret = sscanf(buf, "%s%x:%x%x:%x%x", sl, &llocal, &localPort, &lremote, &remotePort, &st);
                    if (ret != 6)
                        continue;
                    in_addr tmpAddr;
                    tmpAddr.s_addr = llocal;
                    tmpConnInfo.localAddr = inet_ntoa(tmpAddr);
                    tmpAddr.s_addr = lremote;
                    tmpConnInfo.remoteAddr = inet_ntoa(tmpAddr);
                } else if (i == 3 || i == 4) {
                    ret = sscanf(buf, "%s%32s:%x%32s:%x%x", sl, &localaddr, &localPort, &remoteaddr, &remotePort, &st);
                    if (ret != 6)
                        continue;
                    char tmp[64] = {'\0'};
                    int pos = 0;
                    for (int j = 0; j < 32; j++) {
                        tmp[pos++] = localaddr[j];
                        if ((j % 4) == 3 && j != 31)
                            tmp[pos++] = ':';
                    }
                    tmpConnInfo.localAddr = string(tmp);
                    memset(tmp, '\0', 64);
                    pos = 0;
                    for (int j = 0; j < 32; j++) {
                        tmp[pos++] = remoteaddr[j];
                        if ((j % 4) == 3 && j != 31)
                            tmp[pos++] = ':';
                    }
                    tmpConnInfo.remoteAddr = string(tmp);
                }

                tmpConnInfo.localPort = localPort;
                tmpConnInfo.remotePort = remotePort;
                tmpConnInfo.id = id++;
                tmpConnInfo.protocol = stateType;
                tmpConnInfo.state = (SysNetState) st;
                netConnInfo.push_back(tmpConnInfo);
            }
            fInfo.close();
        }

    }
}

