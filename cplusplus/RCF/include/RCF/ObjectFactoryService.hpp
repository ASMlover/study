
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

#ifndef INCLUDE_RCF_OBJECTFACTORYSERVICE_HPP
#define INCLUDE_RCF_OBJECTFACTORYSERVICE_HPP

#include <RCF/Config.hpp>

#if RCF_FEATURE_LEGACY==0
#error This header is only supported in builds with RCF_FEATURE_LEGACY=1.
#endif

#include <map>
#include <string>
#include <vector>

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/GetInterfaceName.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/Service.hpp>
#include <RCF/StubFactory.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/Token.hpp>
#include <RCF/TypeTraits.hpp>

namespace RCF {

    class RcfServer;
    class StubEntry;
    class StubFactory;
    class I_RcfClient;
    class Token;
    class TokenMapped;

    typedef boost::shared_ptr<StubEntry> StubEntryPtr;
    typedef boost::shared_ptr<StubFactory> StubFactoryPtr;
    typedef boost::shared_ptr<TokenMapped> TokenMappedPtr;

    class RCF_EXPORT StubFactoryRegistry
    {
    public:

        StubFactoryRegistry();

        // Binds an object factory to a name.

        template<typename I1, typename ImplementationT>
        bool bind(const std::string &name_ = "")
        {
            return bind( (I1 *) NULL, (ImplementationT **) NULL, name_);
        }

        template<typename I1, typename ImplementationT>
        bool bind(I1 *, ImplementationT **, const std::string &name_ = "")
        {
            const std::string &name = (name_ == "") ?
                getInterfaceName((I1 *) NULL) :
                name_;

            StubFactoryPtr stubFactoryPtr(
                new RCF::StubFactory_1<ImplementationT, I1>());

            std::string desc;
            return insertStubFactory(name, desc, stubFactoryPtr);
        }

    protected:

        bool            insertStubFactory(
                            const std::string &objectName,
                            const std::string &desc,
                            StubFactoryPtr stubFactoryPtr);

        bool            removeStubFactory(
                            const std::string &objectName);

        StubFactoryPtr  getStubFactory(
                            const std::string &objectName);

    private:

        typedef std::map<
            std::string, 
            StubFactoryPtr>             StubFactoryMap;

        ReadWriteMutex                  mStubFactoryMapMutex;
        StubFactoryMap                  mStubFactoryMap;

    };

    class RCF_EXPORT ObjectFactoryService :
        public I_Service,
        public StubFactoryRegistry,
        boost::noncopyable
    {
    public:

        ObjectFactoryService();

        // Remotely accessible functions.
        boost::int32_t CreateObject(const std::string &objectName, Token &token);
        boost::int32_t DeleteObject(const Token &token);

        boost::int32_t addObject(TokenMappedPtr tokenMappedPtr, Token &token);

        boost::int32_t CreateSessionObject(const std::string &objectName);
        boost::int32_t DeleteSessionObject();

        StubEntryPtr    getStubEntryPtr(const Token &token);
        TokenMappedPtr  getTokenMappedPtr(const Token & token);
        
    private:
        void            onServiceAdded(RcfServer &server);
        void            onServiceRemoved(RcfServer &server);
        void            onServerStart(RcfServer &);
        void            onServerStop(RcfServer &);
        void            stopCleanup();
        void            cycleCleanup(int timeoutMs);
        void            cleanupStubMap(unsigned int timeoutS);

        typedef std::map<
            Token, 
            std::pair<
                MutexPtr, 
                TokenMappedPtr> >         StubMap;

        // TokenFactory is internally synchronized
        typedef boost::shared_ptr<TokenFactory> TokenFactoryPtr;
        TokenFactoryPtr                 mTokenFactory;

        unsigned int                    mClientStubTimeoutS;
        Mutex                           mCleanupThresholdMutex;
        Condition                       mCleanupThresholdCondition;
        unsigned int                    mCleanupIntervalS;
        float                           mCleanupThreshold;

        ReadWriteMutex                  mStubMapMutex;
        StubMap                         mStubMap;

        bool                            mLazyStarted;
    };

    typedef boost::shared_ptr<ObjectFactoryService> 
        ObjectFactoryServicePtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_OBJECTFACTORYSERVICE_HPP
