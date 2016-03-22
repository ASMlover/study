
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

#include <RCF/InitDeinit.hpp>

#include <RCF/Config.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/AmiThreadPool.hpp>

#include <RCF/Filter.hpp>
#include <RCF/Globals.hpp>

// std::size_t
#include <cstdlib>

// std::size_t for vc6
#include <boost/config.hpp>

#include <csignal>

namespace RCF {

    extern AmiThreadPool * gpAmiThreadPool;

    static Mutex gInitRefCountMutex;
    static std::size_t gInitRefCount = 0;

    void initAmiHandlerCache();
    void initFileIoThreadPool();
    void initAmi();
    void initObjectPool();
    void initPerformanceData();
    void SspiInitialize();
    void initThreadLocalData();
    void initTpHandlerCache();
    void initWinsock();
    void initRegistrySingleton();
    void initLogManager();
    void initPfnGetUserName();

    void deinitAmiHandlerCache();
    void deinitFileIoThreadPool();
    void deinitAmi();
    void deinitObjectPool();
    void deinitPerformanceData();
    void SspiUninitialize();
    void deinitThreadLocalData();
    void deinitTpHandlerCache();
    void deinitWinsock();
    void deinitOpenSsl();
    void deinitRegistrySingleton();
    void deinitLogManager();
    void deinitPfnGetUserName();

    extern Globals * gpGlobals;

    std::size_t getInitRefCount()
    {
        return gInitRefCount;
    }

    bool init(RcfConfigT *)
    {
        Lock lock(getRootMutex());
        if (gInitRefCount == 0)
        {

            gpGlobals = new Globals();

            // General initialization.
            
            RCF::getCurrentTimeMs();
            initAmiHandlerCache();
            initLogManager();
            initAmi();
            initObjectPool();
            initPerformanceData();
            initThreadLocalData();
            initTpHandlerCache();


#if RCF_FEATURE_FILETRANSFER==1
            initFileIoThreadPool();
#endif


#if RCF_FEATURE_SF==1
            initRegistrySingleton();
#endif


#ifdef BOOST_WINDOWS
            initWinsock();
            initPfnGetUserName();
#endif


#ifndef BOOST_WINDOWS
            // Disable broken pipe signals on non-Windows platforms.
            std::signal(SIGPIPE, SIG_IGN);
#endif


#if RCF_FEATURE_SSPI==1
            SspiInitialize();
#endif

           
#if RCF_FEATURE_SERVER==1
            // Start the AMI thread pool.
            gpAmiThreadPool = new AmiThreadPool(); 
            gpAmiThreadPool->start();
#endif

        }

        ++gInitRefCount;
        return gInitRefCount == 1;
    }

    bool deinit()
    {
        Lock lock(getRootMutex());
        --gInitRefCount;
        if (gInitRefCount == 0)
        {


#if RCF_FEATURE_SERVER==1
            // Stop the AMI thread pool.
            gpAmiThreadPool->stop(); 
            delete gpAmiThreadPool; 
            gpAmiThreadPool = NULL;
#endif


#if RCF_FEATURE_FILETRANSFER==1
            deinitFileIoThreadPool();
#endif
            

#ifdef BOOST_WINDOWS
            deinitPfnGetUserName();
            deinitWinsock();
#endif


#if RCF_FEATURE_SSPI==1
            SspiUninitialize();
#endif


#if RCF_FEATURE_SF==1
            deinitRegistrySingleton();
#endif


            // General deinitialization.
            deinitAmi();
            deinitObjectPool();
            deinitAmiHandlerCache();
            deinitPerformanceData();
            deinitThreadLocalData();
            deinitTpHandlerCache();
            deinitLogManager();  

            delete gpGlobals;
            gpGlobals = NULL;
        }
        return gInitRefCount == 0;
    }

    RcfInitDeinit::RcfInitDeinit(RcfConfigT *) : mIsRootInstance(false)
    {
        mIsRootInstance = init();
    }

    RcfInitDeinit::~RcfInitDeinit()
    {
        deinit();
    }

    bool RcfInitDeinit::isRootInstance()
    {
        return mIsRootInstance;
    }

#ifdef RCF_AUTO_INIT_DEINIT
    RcfInitDeinit gRcfAutoInit;
#endif

} // namespace RCF
