
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

#include <RCF/ObjectFactoryService.hpp>
#include <RCF/SessionObjectFactoryService.hpp>

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/RcfServer.hpp>
#include <RCF/ServerInterfaces.hpp>
#include <RCF/StubEntry.hpp>
#include <RCF/StubFactory.hpp>

namespace RCF {

    ObjectFactoryService::ObjectFactoryService() :
            mTokenFactory(),
            mClientStubTimeoutS(0),            
            mCleanupIntervalS(0),
            mCleanupThreshold(0.0),
            mStubMapMutex(WriterPriority),
            mLazyStarted(false)
    {
    }

    // remotely accessible
    boost::int32_t ObjectFactoryService::CreateObject(
        const std::string &objectName,
        Token &token)
    {
        RCF_LOG_3()(objectName);

        {
            Lock lock(mCleanupThresholdMutex);
            if (!mLazyStarted)
            {
                mTaskEntries[0].start();
                mLazyStarted = true;
            }
        }

        // TODO: seems unnecessary to be triggering a sweep here
        std::size_t nAvail = mTokenFactory->getAvailableTokenCount();
        std::size_t nTotal = mTokenFactory->getTokenSpace().size();
        float used = float(nTotal - nAvail) / float(nTotal);
        if (used > mCleanupThreshold)
        {
            Lock lock(mCleanupThresholdMutex);
            mCleanupThresholdCondition.notify_all(lock);
        }

        boost::int32_t ret = RcfError_Ok;

        StubFactoryPtr stubFactoryPtr( getStubFactory( objectName));
        if (stubFactoryPtr.get())
        {
            RcfClientPtr rcfClientPtr( stubFactoryPtr->makeServerStub());
            StubEntryPtr stubEntryPtr( new StubEntry(rcfClientPtr));

            ret = addObject(TokenMappedPtr(stubEntryPtr), token);
            
            if (ret == RcfError_Ok)
            {
                getTlsRcfSession().setCachedStubEntryPtr(stubEntryPtr);

                RCF_LOG_3()(objectName)(token) << "Dynamically bound object created.";
            }
        }
        else
        {
            ret = RcfError_ObjectFactoryNotFound;
        }

        return ret;
    }
    
    boost::int32_t ObjectFactoryService::addObject(
        TokenMappedPtr tokenMappedPtr, 
        Token & token)
    {
        // TODO: exception safety
        Token myToken;
        bool ok = mTokenFactory->requestToken(myToken);
        if (ok)
        {
            WriteLock writeLock(mStubMapMutex);
            RCF_UNUSED_VARIABLE(writeLock);
            RCF_ASSERT(mStubMap.find(myToken) != mStubMap.end())(myToken);
            mStubMap[myToken].second = tokenMappedPtr;
            token = myToken;
            return RcfError_Ok;
        }
        else
        {
            return RcfError_TokenRequestFailed;
        }
    }

    // remotely accessible
    boost::int32_t ObjectFactoryService::CreateSessionObject(
        const std::string &objectName)
    {
        StubFactoryPtr stubFactoryPtr( getStubFactory( objectName));
        if (stubFactoryPtr.get())
        {
            RcfClientPtr rcfClientPtr( stubFactoryPtr->makeServerStub());
            getTlsRcfSession().setDefaultStubEntryPtr(
                StubEntryPtr( new StubEntry(rcfClientPtr)));
            return RcfError_Ok;
        }
        return RcfError_ObjectFactoryNotFound;
    }

    // remotely accessible
    boost::int32_t SessionObjectFactoryService::CreateSessionObject(
        const std::string &objectName)
    {
        StubFactoryPtr stubFactoryPtr( getStubFactory( objectName));
        if (stubFactoryPtr.get())
        {
            RcfClientPtr rcfClientPtr( stubFactoryPtr->makeServerStub());
            getTlsRcfSession().setDefaultStubEntryPtr(
                StubEntryPtr( new StubEntry(rcfClientPtr)));
            return RcfError_Ok;
        }
        return RcfError_ObjectFactoryNotFound;
    }

    // remotely accessible
    boost::int32_t ObjectFactoryService::DeleteObject(const Token &token)
    {
        WriteLock writeLock(mStubMapMutex);
        RCF_UNUSED_VARIABLE(writeLock);

        if (mStubMap.find(token) == mStubMap.end())
        {
            return RcfError_DynamicObjectNotFound;
        }
        else
        {
            mStubMap[token].second.reset();
            mTokenFactory->returnToken(token);
            RCF_LOG_3()(token) << "Dynamically bound object deleted.";
            return RcfError_Ok;
        }
    }

    // remotely accessible
    boost::int32_t ObjectFactoryService::DeleteSessionObject()
    {
        getTlsRcfSession().setDefaultStubEntryPtr(StubEntryPtr());
        return RcfError_Ok;
    }

    // remotely accessible
    boost::int32_t SessionObjectFactoryService::DeleteSessionObject()
    {
        getTlsRcfSession().setDefaultStubEntryPtr(StubEntryPtr());
        return RcfError_Ok;
    }

    StubEntryPtr ObjectFactoryService::getStubEntryPtr(const Token &token)
    {
        return boost::dynamic_pointer_cast<StubEntry>(
            getTokenMappedPtr(token));
    }

    TokenMappedPtr ObjectFactoryService::getTokenMappedPtr(const Token &token)
    {
        ReadLock readLock(mStubMapMutex);
        RCF_VERIFY(
            mStubMap.find(token) != mStubMap.end(),
            Exception(_RcfError_DynamicObjectNotFound(token.getId())))
            (token);

        Lock lock(*mStubMap[token].first);
        TokenMappedPtr tokenMappedPtr = mStubMap[token].second;
        return tokenMappedPtr;
    }

    void ObjectFactoryService::onServiceAdded(RcfServer &server)
    {
        server.bind<I_ObjectFactory>(*this);

        mTaskEntries.clear();

        mTaskEntries.push_back(
            TaskEntry(
            boost::bind(&ObjectFactoryService::cycleCleanup, this, _1),
            boost::bind(&ObjectFactoryService::stopCleanup, this),
            "RCF Ofs cleanup",
            false));
    }

    void ObjectFactoryService::onServiceRemoved(RcfServer &server)
    {
        server.unbind<I_ObjectFactory>();
    }

    void ObjectFactoryService::onServerStart(RcfServer & server)
    {
        mTokenFactory.reset( new TokenFactory(server.getOfsMaxNumberOfObjects()) );
        mClientStubTimeoutS = server.getOfsObjectTimeoutS();
        mCleanupIntervalS = server.getOfsCleanupIntervalS();
        mCleanupThreshold = server.getOfsCleanupThreshold();

        RCF_ASSERT(0.0 <= mCleanupThreshold && mCleanupThreshold <= 1.0);

        // up-front initialization, before threads get into the picture
        typedef std::vector<Token>::const_iterator Iter;
        for (
            Iter iter = mTokenFactory->getTokenSpace().begin();
            iter != mTokenFactory->getTokenSpace().end();
            ++iter)
        {
            mStubMap[*iter].first.reset(new Mutex());
        }
    }

    void ObjectFactoryService::onServerStop(RcfServer &)
    {
    }

    void ObjectFactoryService::stopCleanup()
    {
        Lock lock(mCleanupThresholdMutex);
        mCleanupThresholdCondition.notify_all(lock);
    }

    void ObjectFactoryService::cycleCleanup(
        int timeoutMs)
    {
        RCF::ThreadInfoPtr tiPtr = getTlsThreadInfoPtr();
        RCF::ThreadPool & threadPool = tiPtr->getThreadPool();

        if (timeoutMs == 0)
        {
            cleanupStubMap(mClientStubTimeoutS);
        }
        else
        {
            Lock lock(mCleanupThresholdMutex);
            if (!threadPool.shouldStop())
            {
                if (mCleanupIntervalS)
                {
                    unsigned int cleanupIntervalMs = 1000*mCleanupIntervalS;
                    mCleanupThresholdCondition.timed_wait(lock, cleanupIntervalMs);
                }
                else
                {
                    mCleanupThresholdCondition.wait(lock);
                }
                
                if (!threadPool.shouldStop())
                {
                    cleanupStubMap(mClientStubTimeoutS);
                }
            }
        }
    }

    StubFactoryRegistry::StubFactoryRegistry() :
        mStubFactoryMapMutex(WriterPriority)
    {}

    bool StubFactoryRegistry::insertStubFactory(
        const std::string &objectName,
        const std::string &desc,
        StubFactoryPtr stubFactoryPtr)
    {
        RCF_UNUSED_VARIABLE(desc);
        WriteLock writeLock(mStubFactoryMapMutex);
        mStubFactoryMap[ objectName ] = stubFactoryPtr;
        return true;
    }

    bool StubFactoryRegistry::removeStubFactory(
        const std::string &objectName)
    {
        WriteLock writeLock(mStubFactoryMapMutex);
        mStubFactoryMap.erase(mStubFactoryMap.find(objectName));
        return true;
    }

    StubFactoryPtr StubFactoryRegistry::getStubFactory(
        const std::string &objectName)
    {
        ReadLock readLock(mStubFactoryMapMutex);
        return mStubFactoryMap.find(objectName)  != mStubFactoryMap.end() ?
            mStubFactoryMap[objectName] :
            StubFactoryPtr();
    }

    void ObjectFactoryService::cleanupStubMap(unsigned int timeoutS)
    {
        // Clean up the stub map
        std::size_t nAvail = mTokenFactory->getAvailableTokenCount();
        std::size_t nTotal = mTokenFactory->getTokenSpace().size();
        float used = float(nTotal - nAvail) / float(nTotal);
        if (used > mCleanupThreshold)
        {
            RCF_LOG_3() << "ObjectFactoryService - cleaning up stub map.";
            typedef std::vector<Token>::const_iterator Iter;
            for (
                Iter iter = mTokenFactory->getTokenSpace().begin();
                iter != mTokenFactory->getTokenSpace().end();
                ++iter)
            {
                Token token = *iter;

                bool removeStub = false;
                {
                    ReadLock readLock(mStubMapMutex);
                    RCF_ASSERT(mStubMap.find(token) != mStubMap.end())(token);
                    Lock lock(*mStubMap[token].first);
                    TokenMappedPtr & tokenMappedPtr = mStubMap[token].second;
                    if (
                        tokenMappedPtr.get() &&
                        tokenMappedPtr.unique() &&
                        tokenMappedPtr->getElapsedTimeS() > timeoutS)
                    {
                        removeStub = true;
                        tokenMappedPtr.reset();
                    }
                }
                if (removeStub)
                {
                    mTokenFactory->returnToken(token);
                }
            }
        }
    }

} // namespace RCF
