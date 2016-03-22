
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

#ifndef INCLUDE_RCF_SERVEROBJECTSERVICE_HPP
#define INCLUDE_RCF_SERVEROBJECTSERVICE_HPP

#include <boost/any.hpp>

#include <RCF/Export.hpp>
#include <RCF/PeriodicTimer.hpp>
#include <RCF/Service.hpp>


namespace RCF {

    class ServerObjectService;
    typedef boost::shared_ptr<ServerObjectService> ServerObjectServicePtr;

    class ServerObjectHolder
    {
    public:

        ServerObjectHolder() : 
            mTimeoutMs(0),
            mLastTouchMs(0), 
            mUseCount(0)
        {
        }

        ServerObjectHolder(const boost::any & serverObject, boost::uint32_t timeoutMs) : 
            mTimeoutMs(timeoutMs), 
            mLastTouchMs(0), 
            mUseCount(0),
            mServerObject(serverObject)
        {
        }

        boost::uint32_t     mTimeoutMs;
        boost::uint32_t     mLastTouchMs;
        int                 mUseCount;
        boost::any          mServerObject;
    };
    
    class RCF_EXPORT ServerObjectService : public I_Service, boost::noncopyable
    {
    public:
        ServerObjectService();

    private:
        void            onServerStart(RcfServer & server);
        void            onServerStop(RcfServer & server);
        void            onTimer();
        void            customDeleter(const std::string & objectKey, void * pt);

        typedef std::map<std::string, ServerObjectHolder>   ServerObjectMap;

        RcfServer *         mpRcfServer;
        PeriodicTimer       mPeriodicTimer;

        boost::uint32_t     mHarvestingIntervalS;
        boost::uint32_t     mLastHarvestMs;

        Mutex               mMutex;
        ServerObjectMap     mServerObjectMap;

        

        template<typename T>
        boost::shared_ptr<T> getServerObjectImpl(
            const std::string & objectKey, 
            boost::uint32_t timeoutMs, 
            bool createIfDoesntExist)
        {
            typedef boost::shared_ptr<T> TPtr;

            Lock lock(mMutex);

            ServerObjectMap::iterator iter = mServerObjectMap.find(objectKey);
            if (iter != mServerObjectMap.end())
            {
                ServerObjectHolder & holder = iter->second;
                boost::any & a = holder.mServerObject;
                TPtr * ptPtr = boost::any_cast<TPtr>(&a);
                RCF_ASSERT(ptPtr);
                TPtr tPtr = *ptPtr;
                T * pt = tPtr.get();
                RCF_ASSERT(pt);

                // Return shared_ptr with custom deleter.
                holder.mLastTouchMs = getCurrentTimeMs();
                RCF_ASSERT(holder.mUseCount >= 0);
                ++holder.mUseCount;
                TPtr ptr(pt, boost::bind(&ServerObjectService::customDeleter, this, objectKey, _1));
                return ptr;
            }
            else if (createIfDoesntExist)
            {
                T * pt = new T();
                TPtr tPtr(pt);
                mServerObjectMap[objectKey] = ServerObjectHolder(boost::any(tPtr), timeoutMs);
                ServerObjectHolder & holder = mServerObjectMap[objectKey];

                // Return shared_ptr with custom deleter.
                holder.mLastTouchMs = getCurrentTimeMs();
                RCF_ASSERT(holder.mUseCount >= 0);
                ++holder.mUseCount;
                TPtr ptr(pt, boost::bind(&ServerObjectService::customDeleter, this, objectKey, _1));
                return ptr;
            }
            else
            {
                return TPtr();
            }
        }

    public:

        template<typename T>
        boost::shared_ptr<T> queryServerObject(
            const std::string & objectKey)
        {
            return getServerObjectImpl<T>(objectKey, 0, false);
        }

        template<typename T>
        boost::shared_ptr<T> getServerObject(
            const std::string & objectKey, 
            boost::uint32_t timeoutMs)
        {
            return getServerObjectImpl<T>(objectKey, timeoutMs, true);
        }

        void deleteServerObject(const std::string & objectKey);
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_SERVEROBJECTSERVICE_HPP
