
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

#include <RCF/IpServerTransport.hpp>

namespace RCF {

    IpServerTransport::IpServerTransport() :
        mReadWriteMutex(WriterPriority)
    {}

    IpServerTransport::~IpServerTransport() 
    {}

    void IpServerTransport::setAllowIps(
        const std::vector<IpRule> &allowedIps)
    {
        WriteLock writeLock(mReadWriteMutex);
        mAllowedIps = allowedIps;

        // Resolve all.
        for (std::size_t i=0; i<mAllowedIps.size(); ++i)
        {
            mAllowedIps[i].first.resolve();
        }
    }

    void IpServerTransport::setDenyIps(
        const std::vector<IpRule> &disallowedIps)
    {
        WriteLock writeLock(mReadWriteMutex);
        mDisallowedIps = disallowedIps;

        // Resolve all.
        for (std::size_t i=0; i<mDisallowedIps.size(); ++i)
        {
            mDisallowedIps[i].first.resolve();
        }
    }

    std::vector<IpRule> IpServerTransport::getAllowIps() const
    {
        ReadLock readLock(mReadWriteMutex);
        return mAllowedIps;
    }

    std::vector<IpRule> IpServerTransport::getDenyIps() const
    {
        ReadLock readLock(mReadWriteMutex);
        return mDisallowedIps;
    }

    bool IpServerTransport::isIpAllowed(const IpAddress &ip) const
    {
        ReadLock readLock(mReadWriteMutex);

        if (!mAllowedIps.empty())
        {
            for (std::size_t i=0; i<mAllowedIps.size(); ++i)
            {
                if (ip.matches(mAllowedIps[i].first, mAllowedIps[i].second))
                {
                    return true;
                }
            }
            return false;
        }

        if (!mDisallowedIps.empty())
        {
            for (std::size_t i=0; i<mDisallowedIps.size(); ++i)
            {
                if (ip.matches(mDisallowedIps[i].first, mDisallowedIps[i].second))
                {
                    return false;
                }
            }
            return true;
        }

        return true;
    }    

} // namespace RCF
