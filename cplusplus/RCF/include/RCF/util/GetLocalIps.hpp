
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

#ifndef INCLUDE_UTIL_GETLOCALIPS_HPP
#define INCLUDE_UTIL_GETLOCALIPS_HPP

#include <string>
#include <utility>
#include <vector>

#include <RCF/Exception.hpp>
#include <RCF/util/Platform/OS/BsdSockets.hpp>

namespace RCF {

    std::pair<std::string, std::vector<std::string> > getLocalIps()
    {
        std::vector<char> hostname(80);
        int ret = gethostname(&hostname[0], hostname.size());
        int err = Platform::OS::BsdSockets::GetLastError();
        RCF_VERIFY(
            ret != SOCKET_ERROR, 
            RCF::Exception(0, err, RCF::RcfSubsystem_Os))(ret)(err);
        hostent *phe = gethostbyname(&hostname[0]);
        err = Platform::OS::BsdSockets::GetLastError();
        RCF_VERIFY(
            phe, 
            RCF::Exception(0, err, RCF::RcfSubsystem_Os))(err);
        std::vector<std::string> ips;
        for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
            struct in_addr addr;
            memcpy(&addr, phe->h_addr_list[i], sizeof( in_addr));
            ips.push_back(inet_ntoa(addr));
        }
        return std::make_pair( std::string(&hostname[0]), ips);
    }

} // namespace RCF

#endif
