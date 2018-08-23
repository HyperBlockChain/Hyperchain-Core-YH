/*Copyright 2016-2018 hyperchain.net (Hyperchain)

Distributed under the MIT software license, see the accompanying
file COPYING or https://opensource.org/licenses/MIT.

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software
without restriction, including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifdef WIN32
#include <windows.h>
#include <psapi.h>    
#pragma comment(lib,"psapi.lib")
#else
#include <sys/sysinfo.h>
#include <stdio.h>  
#include <unistd.h>  
#include <linux/kernel.h>  
#include <stdlib.h>

#define VMRSS_LINE 17  
#define VMSIZE_LINE 13  
#define PROCESS_ITEM 14

#endif

#include "OSFunctions.h"

#define MB (1024*1024)

int OSFunctions::GetCurrentFreeMemoryOS()
{
#ifdef WIN32
	MEMORYSTATUS status;
	GlobalMemoryStatus(&status);
	int mems = status.dwAvailPhys / MB;

	return mems;
#else
	struct sysinfo sys;  
	int err = sysinfo(&sys); 
	int mems = sys.freeram/MB;

	return mems;
#endif	
}

int OSFunctions::GetTotalMemoryOS()
{
#ifdef WIN32
	MEMORYSTATUS status;
	GlobalMemoryStatus(&status);
	int mems=status.dwTotalPhys/MB;

	return mems;
#else
	struct sysinfo sys;  
	int err = sysinfo(&sys);
	int mems = sys.totalram / MB;

	return mems;
#endif
}


int OSFunctions::GetUsedMemoryHC()
{
#ifdef WIN32
	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle,&pmc,sizeof(pmc)); 

	return pmc.WorkingSetSize / MB;
#else	
	pid_t pid = getpid();
	char file_name[64] = { 0 };
	FILE *fd;
	char line_buff[512] = { 0 };
	sprintf(file_name, "/proc/%d/status", pid);

	fd = fopen(file_name, "r");
	if (nullptr == fd){
		return 0;
	}

	char name[64];
	int vmrss;
	for (int i = 0; i<VMRSS_LINE - 1; i++){
		fgets(line_buff, sizeof(line_buff), fd);
	}

	fgets(line_buff, sizeof(line_buff), fd);
	sscanf(line_buff, "%s %d", name, &vmrss);
	fclose(fd);

	return vmrss/MB;
#endif
}

int OSFunctions::GetCurrentCPUIdlePercentageOS()
{
#ifdef WIN32
	int cpuUsage;
	FILETIME preIdleTime;
	FILETIME preKernelTime;
	FILETIME preUserTime;
	GetSystemTimes(&preIdleTime, &preKernelTime, &preUserTime);

	Sleep(1000);
	FILETIME idleTime;
	FILETIME kernelTime;
	FILETIME userTime;
	GetSystemTimes(&idleTime, &kernelTime, &userTime);

	long idle = CompareFileTime(&preIdleTime, &idleTime);
	long kernel = CompareFileTime(&preKernelTime, &kernelTime);
	long user = CompareFileTime(&preUserTime, &userTime);
	cpuUsage = (kernel + user - idle) * 100 / (kernel + user);

	return cpuUsage;
#else
	FILE*fd;
	char buffer[1024] = { 0 };

	typedef struct {
		unsigned int user;
		unsigned int nice; 
		unsigned int system;
		unsigned int idle;
	}total_cpu_occupy_t;

	total_cpu_occupy_t t;
	fd = fopen("/proc/stat", "r");
	fgets(buffer, sizeof(buffer), fd);

	char name[16];	
	sscanf(buffer, "%s %u %u %u %u", name, &t.user, &t.nice, &t.system, &t.idle);
	fclose(fd);

	return t.user + t.nice + t.system + t.idle;
#endif	
}