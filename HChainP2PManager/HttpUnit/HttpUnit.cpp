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

#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifndef WIN32
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "HttpUnit.h"
#include "switchsock.h"
#include "convert.h"

#include <vector>
#pragma hdrstop
#include "../debug/Log.h"

#define WriteLog() MangerLog::Instance()

#define MAX_RECV_BUFF_LEN 1024

//////////////////////////////////////
void UseSwitchInHttpDownload(bool bUse)
{

}

int HttpDownloadT(string URL,char **body, unsigned int &recvLen, bool bOnce, int timeout)
{
	return HttpDownload(URL, body, recvLen, "", "", false, "/", bOnce, timeout);
}

int HttpDownloadF(string URL, char **body, unsigned int &recvLen, bool bOnce, int timeout)
{
	return HttpDownloadFile(URL, body, recvLen, "", "", false, "/", bOnce, timeout);
}

int HttpDownloadFile(string url, char **body, unsigned int &recvLen, const string& post,
	const string& AdditionHead,
	bool IsHead, string aToken,
	bool bOnce, int timeout)
{

	string Token = "\\";
	if (!aToken.empty())
		Token = aToken;

	if (url.find(Token, 8) == -1) { url += Token; }
	int pos = url.find(Token, 8);
	if (pos == -1)
	{
		return 401;
	}


	string host = url.substr(7, pos - 7);
	string remotepath = url.substr(pos, url.length());
	pos = host.find(':');
	int port = 80;
	if (pos != -1)
	{
		port = atoi(host.substr(pos + 1, host.length()).c_str());
		if (port == 0)
			port = 80;
		host = host.substr(0, pos);
	}


	SwitchSock m_sock;
	if (!m_sock.Create())
	{
		return 402;
	}

	if (!m_sock.Connect(host, port))
	{
		return 403;
	}

	string cmd;
	if (post == "")
	{
		if (IsHead)
			cmd = "HEAD ";
		else
			cmd = "GET ";

		cmd += remotepath + " HTTP/1.1\r\nAccept: */* \r\nAccept-Language: zh-CN \r\nUser_Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322)\r\nAccept-Encoding: gzip, deflate\r\nHost: " + host + "\r\nConnection: Keep-Alive\r\n\r\n";

	}
	else
	{
		if (post == "DELETE") {
			cmd = "DELETE " + remotepath + " HTTP/1.1\r\nHost: " + host + "\r\nAccept: */* \r\nUser_Agent:Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322)\r\nContent-Length: " + CConvert::IntToStr(post.length()) + "\r\n" + AdditionHead + "\r\n\r\n";
		}
		else {
			cmd = "POST " + remotepath + " HTTP/1.1\r\nHost: " + host + "\r\nAccept: */* \r\nUser_Agent:Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322)\r\nContent-Length: " + CConvert::IntToStr(post.length()) + "\r\n" + AdditionHead + "\r\n\r\n" + post;
		}
	}

	string str;

	m_sock.Send((char*)cmd.c_str(), cmd.length());

	int nHttpCode = 444;

	char* buf = new char[MAX_RECV_BUFF_LEN];

	if (buf == NULL) {
		return nHttpCode;
	}

	bool head = true;
	unsigned int total = 0;
	unsigned int temp_num = 0;
	unsigned int uiLastLen = 0;
	try
	{
		int len = 0;
		do
		{
			len = 0;
			timeout = 20;
			len = m_sock.Recv(buf, MAX_RECV_BUFF_LEN - 1, timeout);
			temp_num += len;
			if (len>0)
			{
				buf[len] = '\0';
				if (head)
				{
					char* pStatue = strchr(buf, ' ');
					char* headend = strstr(buf, "\r\n\r\n");
					char* temp_total = NULL;
					if (pStatue[1] == '2')
					{
						temp_total = strstr(buf, "Content-Length:");
						temp_total += 15;
						total = atol(temp_total);
						recvLen = total;
					}
					nHttpCode = atol(pStatue);

					if ((pStatue == NULL) || ((pStatue[1] != '2') && (pStatue[1] != '1')))
					{
						string strNewURL;
						if (pStatue[1] == '3')
						{
							char* pNewURL = strstr(buf, "Location:");
							if (pNewURL != NULL)
							{
								pNewURL += 9;
								while ((*pNewURL) == ' ')
								{
									pNewURL++;
								}
								char* pEnd = strchr(pNewURL, '\n');
								if (pEnd) *pEnd = '\0';
								pEnd = strchr(pNewURL, '\r');
								if (pEnd) *pEnd = '\0';
								strNewURL = pNewURL;
							}
						}

						*body = (char*)malloc(sizeof(char) * len);
						memset(*body, 0, (sizeof(char) * len));
						memcpy(*body, buf, len);

						delete[] buf;
						buf = NULL;

						m_sock.Close();

						if (strNewURL != "")
						{
							if (strNewURL.substr(0, 7) != "http://")
							{
								if (strNewURL[0] == '/')
								{
									strNewURL = "http://" + host + ":" + CConvert::IntToStr(port) + strNewURL;
								}
								else
								{
									int pos = url.rfind("/");
									if (pos != string::npos)
									{
										url = url.substr(0, pos + 1);
									}
									strNewURL = url + strNewURL;
								}
							}
							return HttpDownload(strNewURL, body, recvLen, post, AdditionHead, false, "/");
						}

						return nHttpCode;
					}

					if (headend != NULL)
						headend += 4;
					else
					{
						headend = strstr(buf, "\n\n");
						if (headend != NULL)
							headend += 2;
					}
					if (headend != NULL)
					{
						head = false;

						int memLen = buf + len - headend;
						*body = (char*)malloc(sizeof(char)*(total + 1));
						memset(*body, 0, sizeof(char)*(total + 1));
						memcpy(*body, headend, memLen);

						uiLastLen = memLen;

					}
				}
				else{
					memcpy(*body + uiLastLen, buf, len);
					uiLastLen += len;

				}

			}
			else if (len <= 0)
			{
				nHttpCode = 407;
				break;
			}
		} while (temp_num < total && !bOnce); 
	}
	catch (...){}

	delete[] buf;
	buf = NULL;
	m_sock.Close();

	return nHttpCode;
}


int HttpDownload(string url, char **body, unsigned int &recvLen, const string& post,
	const string& AdditionHead,
	bool IsHead, string aToken,
	bool bOnce, int timeout)
{
	//body="";
	string Token = "\\";
	if (!aToken.empty())
		Token = aToken;

	if (url.find(Token, 8) == -1) { url += Token; }
	int pos = url.find(Token, 8);
	if (pos == -1)
	{
		return 401;
	}


	string host = url.substr(7, pos - 7);
	string remotepath = url.substr(pos, url.length());
	pos = host.find(':');
	int port = 80;
	if (pos != -1)
	{
		port = atoi(host.substr(pos + 1, host.length()).c_str());
		if (port == 0)
			port = 80;
		host = host.substr(0, pos);
	}


	SwitchSock m_sock;
	if (!m_sock.Create())
	{
		return 402;
	}

	if (!m_sock.Connect(host, port))
	{
		return 403;
	}

	string cmd;
	if (post == "")
	{
		if (IsHead)
			cmd = "HEAD ";
		else
			cmd = "GET ";

		cmd += remotepath + " HTTP/1.1\r\nAccept: */* \r\nAccept-Language: zh-CN \r\nUser_Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322)\r\nAccept-Encoding: gzip, deflate\r\nHost: " + host + "\r\nConnection: Keep-Alive\r\n\r\n";
	}
	else
	{
		if (post == "DELETE") {
			cmd = "DELETE " + remotepath + " HTTP/1.1\r\nHost: " + host + "\r\nAccept: */* \r\nUser_Agent:Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322)\r\nContent-Length: " + CConvert::IntToStr(post.length()) + "\r\n" + AdditionHead + "\r\n\r\n";
		}
		else {
			cmd = "POST " + remotepath + " HTTP/1.1\r\nHost: " + host + "\r\nAccept: */* \r\nUser_Agent:Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322)\r\nContent-Length: " + CConvert::IntToStr(post.length()) + "\r\n" + AdditionHead + "\r\n\r\n" + post;
		}
	}

	string str;

	m_sock.Send((char*)cmd.c_str(), cmd.length());

	int nHttpCode = 444;

	char* buf = new char[MAX_RECV_BUFF_LEN];
	if (buf == NULL) {
		return nHttpCode;
	}

	bool head = true;
	unsigned int uiLastLen = 0;
	try
	{
		int len = 0;
		timeout = 20;
		len = m_sock.Recv(buf, MAX_RECV_BUFF_LEN - 1, timeout);

		if (len > 0)
		{
			buf[len] = '\0';
			if (head)
			{
				char* pStatue = strchr(buf, ' ');
				nHttpCode = atol(pStatue);
			}
		}
		else
		{
			nHttpCode = 407;
		}
	}
	catch (...){}

	delete[] buf;
	buf = NULL;
	m_sock.Close();

	return nHttpCode;
}
