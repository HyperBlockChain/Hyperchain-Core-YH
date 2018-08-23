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
#include <string.h>
#include <windows.h>
#else
#include <time.h>
#endif

#include "ElapsedTime.h"

ElapsedTime::ElapsedTime()
{
	m_start = m_end = 0;
}

ElapsedTime::~ElapsedTime()
{	
}

void ElapsedTime::Start()
{	
#ifdef WIN32
	m_start = m_end = ::GetTickCount();
#else
	struct timespec ts; 
	clock_gettime(CLOCK_MONOTONIC, &ts);
	m_start=m_end=ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
}

void ElapsedTime::End()
{
#ifdef WIN32
	m_end = ::GetTickCount();
#else
	struct timespec ts; 
	clock_gettime(CLOCK_MONOTONIC, &ts);
	m_end = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
}

long ElapsedTime::GetYears()const
{
	long years = GetMonths()/12.0;

	return years;
}

long ElapsedTime::GetMonths()const
{
	long months = GetDays()/30.0;

	return months;
}

long ElapsedTime::GetDays()const
{
	long days = GetHours()/24.0;

	return days;
}

long ElapsedTime::GetHours()const
{
	long hours = GetMinutes() / 60.0;

	return hours;
}

long ElapsedTime::GetMinutes()const
{
	long minutes = GetSeconds() / 60.0;

	return minutes;
}

long ElapsedTime::GetSeconds()const
{
	return (m_end - m_start)*1e-3;
}