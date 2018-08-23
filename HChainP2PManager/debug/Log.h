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

#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <time.h>
#ifdef WIN32

#define LOG_ERR     3   
#define LOG_WARNING 4  
#define LOG_NOTICE  5  
#define LOG_INFO    6  

#else
	#include <syslog.h>
#endif

#define XLOG_INFO __FILE__, __LINE__, LOG_INFO
#define XLOG_ERR __FILE__, __LINE__, LOG_ERR
#define XLOG_WARNING __FILE__, __LINE__, LOG_WARNING
#define XLOG_NOTICE __FILE__, __LINE__, LOG_NOTICE

#define FILENAMESIZE 1024

#define 	PRF_LOG_ERROR   1 // 1 < 0
#define 	PRF_LOG_WARNING 2 // 1 < 1
#define 	PRF_LOG_INFO    4 // 1 < 2
#define 	PRF_LOG_NOTICE  8 // 1 < 3
#define     PRF_LOG_ALL     (PRF_LOG_ERROR |  PRF_LOG_WARNING | PRF_LOG_INFO | PRF_LOG_NOTICE) // is default
#define 	PRF_LOG_NON 0

typedef struct LOG_HELPER 
{
    FILE* fp;
    int fp_seq;
    char seperator[8];
	char file_name[FILENAMESIZE];
    int file_max_size;
    int file_time_span;
    time_t last_time_endpt;
	int level;
}
LOG_HELPER;
#ifdef __cplusplus
extern "C" {
#endif
    void set_logfile_max_size(LOG_HELPER* log_helper, int max_size);
    void set_logfile_time_span(LOG_HELPER* log_helper, int span);
    LOG_HELPER* open_logfile(const char* file_name);
    void close_logfile(LOG_HELPER* log_helper);
    LOG_HELPER* open_logfile_r(FILE * fp);
    void close_logfile_r(LOG_HELPER* log_helper);

    void log_info(LOG_HELPER* log_helper, const char *fmt0, ...);
    void log_warning(LOG_HELPER* log_helper, char *fmt0, ...);
    void log_notice(LOG_HELPER* log_helper, char *fmt0, ...);
    void log_err(LOG_HELPER* log_helper, const char *fmt0, ...);
    void log_write(LOG_HELPER* log_helper, const char* file, int line, int level, char *fmt0, ...);
    void set_log_level(LOG_HELPER* log_helper, int level);

#ifdef __cplusplus
}
#endif

extern LOG_HELPER* g_pLogHelper;

#endif

