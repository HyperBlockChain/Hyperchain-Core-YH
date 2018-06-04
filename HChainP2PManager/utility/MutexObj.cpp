/*copyright 2016-2018 hyperchain.net (Hyperchain)
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

#include "MutexObj.h"

CMutexObj::CMutexObj()
{
#ifdef WIN32
	InitializeCriticalSection(&m_oSection);
#else

	pthread_mutexattr_t   attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_hMutex,&attr);
#endif
}

CMutexObj::~CMutexObj()
{
#ifdef WIN32
	DeleteCriticalSection(&m_oSection);
#else
	pthread_mutex_destroy(&m_hMutex);
#endif
}

void  CMutexObj::Lock()
{
#ifdef WIN32
	EnterCriticalSection(&m_oSection);
#else
	pthread_mutex_lock(&m_hMutex);
#endif
}

void CMutexObj::UnLock()
{
#ifdef WIN32
	LeaveCriticalSection(&m_oSection);
#else
	pthread_mutex_unlock(&m_hMutex);
#endif
};

CAutoMutexLock::CAutoMutexLock(CMutexObj& aCriticalSection) :m_oCriticalSection(aCriticalSection)
{
	m_oCriticalSection.Lock();
}

CAutoMutexLock::~CAutoMutexLock()
{
	m_oCriticalSection.UnLock();
}
