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
#ifndef  __TRHEAD_OBJECT__
#define  __TRHEAD_OBJECT__
//
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif


#include <stdio.h>

#include "MutexObj.h"
#include "CallbackFuncObj.h"

extern  int 	ERR_JTHREAD_CANINITMUTEX;
extern  int 	ERR_JTHREAD_CANTSTARTTHREAD;
extern  int  	ERR_JTHREAD_THREADFUNCNOTSET;
extern  int 	ERR_JTHREAD_NOTRUNNING;
extern  int  	ERR_JTHREAD_ALREADYRUNNING;


typedef void* (*pThreadCallbackFunc) (void* pParam);


class CThreadObj
{
public:
	CThreadObj();
	CThreadObj(TCallbackFuncObj<pThreadCallbackFunc>* pCObj);
	~CThreadObj();

	int Start();
	int Start(TCallbackFuncObj<pThreadCallbackFunc>* pCObj);
	int Kill();
	void Join();
	bool IsRunning();	

	void ThreadStarted();

private:
#ifdef WIN32
	static DWORD WINAPI ThreadEntry( LPVOID param );
#else
	static	void* ThreadEntry(void* param);
#endif 
	
	void* ThreadImp();
	
private:
#ifdef WIN32
	HANDLE		m_ThreadID;
#else
	pthread_t	m_ThreadID;
#endif
	
	bool 		m_bIsRunning;
	CMutexObj 	m_muxRunning;
	CMutexObj 	m_muxContinue;
	CMutexObj 	m_muxContinue2;

	TCallbackFuncObj<pThreadCallbackFunc>* 	m_pCallbackFuncObj;
};

#endif 

