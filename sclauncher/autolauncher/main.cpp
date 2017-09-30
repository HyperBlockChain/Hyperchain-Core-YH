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
    //if (!CreateProcessWithTokenW(hPtoken, LOGON_WITH_PROFILE, NULL, const_cast<LPWSTR>(L"E:\\me\\code\\wk\\gy\\FilesNew\\hc.exe"), NULL, NULL, NULL, &si, &pi))
    if (!CreateProcessWithTokenW(hPtoken, LOGON_WITH_PROFILE, NULL, const_cast<LPWSTR>(L"hc.exe"), NULL, NULL, NULL, &si, &pi))
    {
		dwError = GetLastError();
		if (dwError == 1314)
		{
            //MessageBox(NULL, _T("启动进程权限不够(需要管理员权限)"), _T("提示"), MB_OK);
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