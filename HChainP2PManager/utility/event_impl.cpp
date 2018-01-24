/*Copyright 2017 hyperchain.net (Hyper Block Chain)
/*
/*Distributed under the MIT software license, see the accompanying
/*file COPYING or https:
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
#include "event_impl.h"
#include<Winbase.h>

    event_impl::event_impl()
    {
        m_bManulReset = false;
        m_hEvent = 0;
    }

    event_impl::~event_impl()
    {
        term();
    }


    int event_impl::init()
    {
        int nRet = 0;

        if( !m_hEvent )
        {
            m_hEvent = CreateEvent( 0, FALSE, FALSE, 0 );
            m_bManulReset = false;
        }

        ResetEvent( m_hEvent );

        return nRet;
    }

    int event_impl::term()
    {
        int nRet = 0;

        if( m_hEvent )
        {
            CloseHandle( m_hEvent );
            m_hEvent = 0;
        }

        m_bManulReset = false;

        return nRet;
    }


    unsigned long event_impl::wait( unsigned long ulMilliseconds )
    {
		unsigned long ulRet = 0;
        int nRet = 0;

        int dwRet = WaitForSingleObject( m_hEvent, ulMilliseconds );

        switch( dwRet )
        {
        case WAIT_TIMEOUT:
            {
                ulRet = 1;
            }
            break;
        case WAIT_OBJECT_0:
            {
                ulRet = 2;
            }
            break;
        }

        return ulRet;
    }

    int event_impl::set_event_type( bool bManulReset )
    {
        int nRet = 0;

        if( m_bManulReset != bManulReset )
        {
            m_bManulReset = bManulReset;
            CloseHandle( m_hEvent );
            m_hEvent = CreateEvent( 0, bManulReset, FALSE, 0 );
        }

        return nRet;
    }

    int event_impl::reset_event()
    {
        int nRet = 0;
        ResetEvent( m_hEvent );
        return nRet;
    }

    int event_impl::set_event()
    {
        int nRet = 0;
        SetEvent( m_hEvent );
        return nRet;
    }

	int event_impl::release()
	{
		int nRet = 0;

		term();

		return nRet;
	}

