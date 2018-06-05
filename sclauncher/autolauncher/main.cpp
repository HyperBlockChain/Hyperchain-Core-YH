/*Copyright 2017 hyperchain.net  (Hyper Block Chain)
/*
/*Distributed under the MIT software license, see the accompanying
/*file COPYING or https://opensource.org/licenses/MIT.
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
#include <windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <string.h>
#include <tchar.h>

#include <thread>

using namespace std;

#pragma comment(lib, "Advapi32.lib")

#define MAX_LOADSTRING 100

DWORD GetExplorerToken(OUT PHANDLE  phExplorerToken);

int main()
{

	TCHAR szUsername[MAX_PATH];
	DWORD dwUsernameLen = MAX_PATH;
	GetUserName(szUsername, &dwUsernameLen);
	HANDLE hPtoken = NULL;
	GetExplorerToken(&hPtoken);
	PROCESS_INFORMATION pi;
    STARTUPINFOW si = { sizeof(STARTUPINFO), NULL, const_cast<LPWSTR>(L""), NULL, 0, 0, 0, 0, 0, 0, 0, STARTF_USESHOWWINDOW, 0, 0, NULL, 0, 0, 0 };
	si.wShowWindow = SW_SHOW;
	si.lpDesktop = NULL;
	ZeroMemory(&pi, sizeof(pi));

	DWORD dwError;

    if (!CreateProcessWithTokenW(hPtoken, LOGON_WITH_PROFILE, NULL, const_cast<LPWSTR>(L"hc.exe"), NULL, NULL, NULL, &si, &pi))
    {
		dwError = GetLastError();
		if (dwError == 1314)
		{

		}

        this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	else
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return 0;
}

DWORD  GetExplorerToken(OUT PHANDLE  phExplorerToken)
{
	DWORD       dwStatus = ERROR_FILE_NOT_FOUND;
	BOOL        bRet = FALSE;
	HANDLE      hProcess = NULL;
	HANDLE      hProcessSnap = NULL;
	TCHAR        szExplorerExe[MAX_PATH] = { 0 };
	TCHAR        FileName[MAX_PATH] = { 0 };
	PROCESSENTRY32 pe32 = { 0 };
	DWORD   dwError;
	__try
	{
        wcscat_s(szExplorerExe, TEXT("explorer.exe"));
		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE)
		{
			dwStatus = GetLastError();
			__leave;
		}
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (!Process32First(hProcessSnap, &pe32))
		{
			dwStatus = GetLastError();
			__leave;
		}
		do {

            if (!wcsnicmp(pe32.szExeFile, szExplorerExe, MAX_PATH))
			{
				hProcess = OpenProcess(
					PROCESS_QUERY_INFORMATION,
					FALSE,
					pe32.th32ProcessID);
				if (NULL != hProcess)
				{
					HANDLE  hToken;
					if (OpenProcessToken(hProcess, TOKEN_DUPLICATE, &hToken))
					{
						HANDLE hNewToken = NULL;
						DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hNewToken);
						*phExplorerToken = hNewToken;
						dwStatus = 0;
						CloseHandle(hToken);
					}
					break;

					CloseHandle(hProcess);
					hProcess = NULL;
				}
				else
				{
					dwError = GetLastError();
				}
			}

		} while (Process32Next(hProcessSnap, &pe32));
	}
	__finally
	{
		if (NULL != hProcess)
		{
			CloseHandle(hProcess);
		}
		if (NULL != hProcessSnap)
		{
			CloseHandle(hProcessSnap);
		}
	}
	return dwStatus;
}