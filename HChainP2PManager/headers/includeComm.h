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
#ifndef __INCLUDE_COMM_H
#define __INCLUDE_COMM_H


#include <map>
#include <list>
#include <time.h>
#include <vector>
#include <string>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#define DEF_SHA256_LEN	32
#define DEF_SHA512_LEN  128
#define DEF_STR_HASH256_LEN 64

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <atlconv.h>
//#include <winsock.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>  
#include <netdb.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <linux/if.h>
#include <net/if.h>
#include <sys/time.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/errno.h>
#endif// WIN32

using namespace std;

#ifdef WIN32
#define SLEEP(x)                    Sleep(x) //mil
#define IERROR GetLastError()
#else
#define SLEEP(x)                    usleep(x*1000)	//mil
#define IERROR errno
#endif


//#include "gen_int.h"
//#include "commonstruct.h"
//#include "MutexObj.h"
//#include "Log.h"
//#include "crc32.h"
//#include "platform.h"
//#include "UUFile.h"

//#include "CallbackFuncObj.h"
//#include "ThreadObj.h"
//#include "udpsocket.h"

#endif //__INCLUDE_COMM_H