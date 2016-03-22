
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

#include <RCF/ServerObjectService.hpp>

#include <RCF/RcfServer.hpp>

namespace RCF {

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4355 ) // warning C4355: 'this' : used in base member initializer list
#endif

    ServerObjectService::ServerObjectService() :
        mpRcfServer(NULL),
        mPeriodicTimer(*this, 0),
        mHarvestingIntervalS(0),
        mLastHarvestMs(0)
    {
    }

#ifdef _MSC_VER
#pragma warning( pop )
#endif

    void ServerObjectService::onServerStart(RcfServer & server)
    {
        mpRcfServer = & server;

        mHarvestingIntervalS = server.getServerObjectHarvestingIntervalS();
        if (mHarvestingIntervalS)
        {
            mPeriodicTimer.setIntervalMs(1000*mHarvestingIntervalS);
            mPeriodicTimer.start();
        }
    }

    void ServerObjectService::onServerStop(RcfServer & server)
    {
        RCF_UNUSED_VARIABLE(server);
        mPeriodicTimer.stop();
    }

    void ServerObjectService::onTimer()
    {
        boost::uint32_t nowMs = getCurrentTimeMs();
        {
            Lock lock(mMutex);
            ServerObjectMap::iterator iter = mServerObjectMap.begin();
            while ( iter != mServerObjectMap.end() )
            {
                ServerObjectHolder & holder = iter->second;
                if ( holder.mUseCount == 0
                    && (nowMs - holder.mLastTouchMs > holder.mTimeoutMs) )
                {
                    mServerObjectMap.erase(iter++);
                }
                else
                {
                    iter++;
                }
            }
            mLastHarvestMs = nowMs;
        }

        // Clean up HTTP sessions as well.
        mpRcfServer->harvestHttpSessions();
    }

    void ServerObjectService::customDeleter(const std::string & objectKey, void * pt)
    {
        RCF_UNUSED_VARIABLE(pt);

        Lock lock(mMutex);

        ServerObjectMap::iterator iter = mServerObjectMap.find(objectKey);
        RCF_ASSERT(iter != mServerObjectMap.end());
        ServerObjectHolder & holder = iter->second;
        RCF_ASSERT(holder.mUseCount > 0);
        --holder.mUseCount;
        holder.mLastTouchMs = getCurrentTimeMs();
    }

    void ServerObjectService::deleteServerObject(const std::string & objectKey)
    {
        Lock lock(mMutex);
        ServerObjectMap::iterator iter = mServerObjectMap.find(objectKey);
        if (iter != mServerObjectMap.end())
        {
            mServerObjectMap.erase(iter);
        }
    }

} // namespace RCF
