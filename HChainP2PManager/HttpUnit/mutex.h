﻿/*copyright 2016-2018 hyperchain.net (Hyperchain)
/*
/*Distributed under the MIT software license, see the accompanying
/*file COPYING or https://opensource.org/licenses/MIT。
/*
/*Permission is hereby granted, free of charge, to any person obtaining a copy of this 
/*software and associated documentation files (the "Software"), to deal in the Software
/*without restriction, including without limitation the rights to use, copy, modify, merge,
/*publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
/*to whom the Software is furnished to do so, subject to the following conditions:
/*
/*The above copyright notice and this permission notice shall be included in all copies or
/*substantial portions of the Software.
/*
/*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
/*INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
/*PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
/*FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
/*OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/*DEALINGS IN THE SOFTWARE.
*/
#ifdef WIN32
#include "windows.h"
#else
#include "pthread.h"
#endif

class MMutex
{
public:

    MMutex()
	{
#ifdef WIN32
	    InitializeCriticalSection(&fMutex);
#else
		pthread_mutex_init(&fMutex, NULL);
#endif
	}

	~MMutex()
	{
#ifdef WIN32
		DeleteCriticalSection(&fMutex);
#else
		pthread_mutex_destroy(&fMutex);
#endif
	}

    inline void Lock() 
	{
#ifdef WIN32
		::EnterCriticalSection(&fMutex);
#else
		(void)pthread_mutex_lock(&fMutex);
#endif
	};

    inline void Unlock() 
	{
#ifdef WIN32
	    ::LeaveCriticalSection(&fMutex);
#else
		pthread_mutex_unlock(&fMutex);
#endif
	};
    
private:

#ifdef WIN32
    CRITICAL_SECTION fMutex;                     
#else
    pthread_mutex_t fMutex;       
#endif       
};
