
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

#ifndef INCLUDE_RCF_IPSERVERTRANSPORT_HPP
#define INCLUDE_RCF_IPSERVERTRANSPORT_HPP

#include <string>
#include <vector>

#include <RCF/Export.hpp>
#include <RCF/IpAddress.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/Tools.hpp>
#include <RCF/util/Platform/OS/BsdSockets.hpp>

namespace RCF {

    // Ip and significant bits.
    typedef std::pair<IpAddress, std::size_t> IpRule;

    class RCF_EXPORT IpServerTransport
    {
    public:
                        IpServerTransport();
        virtual         ~IpServerTransport();

        bool            isIpAllowed(const IpAddress &ip) const;

        void            setAllowIps(
                            const std::vector<IpRule> &allowedIps);

        void            setDenyIps(
                            const std::vector<IpRule> &allowedIps);

        std::vector<IpRule> 
                        getAllowIps() const;

        std::vector<IpRule> 
                        getDenyIps() const;

        virtual int     getPort() const = 0;

    private:

        mutable ReadWriteMutex      mReadWriteMutex;
        std::vector<IpRule>         mAllowedIps;
        std::vector<IpRule>         mDisallowedIps;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_IPSERVERTRANSPORT_HPP
