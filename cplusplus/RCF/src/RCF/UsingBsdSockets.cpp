
//******************************************************************************
// RCF - Remote Call Framework
//
// Copyright (c) 2005 - 2013, Delta V Software. All rights reserved.
// http://www.deltavsoft.com
//
// RCF is distributed under dual licenses - closed source or GPL.
// Consult your particular license for conditions of use.
//
// If you have not purchased a commercial license, you are using RCF 
// under GPL terms.
//
// Version: 2.0
// Contact: support <at> deltavsoft.com 
//
//******************************************************************************

#include <string>

#include <boost/config.hpp>

#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/Tools.hpp>
#include <RCF/util/Platform/OS/BsdSockets.hpp>

#ifdef BOOST_WINDOWS

namespace RCF {

    void initWinsock()
    {
        WORD wVersion = MAKEWORD( 1, 0 );
        WSADATA wsaData;
        int ret = WSAStartup(wVersion, &wsaData);
        int err = Platform::OS::BsdSockets::GetLastError();
        RCF_VERIFY(ret == 0, Exception( _RcfError_Socket("WSAStartup()"), err, RcfSubsystem_Os) );
    }

    void deinitWinsock()
    {
        WSACleanup();
    }

} // namespace RCF

#endif


