
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

#ifndef INCLUDE_RCF_AMITHREADPOOL_HPP
#define INCLUDE_RCF_AMITHREADPOOL_HPP

#include <memory>

#include <RCF/Export.hpp>
#include <RCF/ThreadPool.hpp>

namespace RCF {

    class RCF_EXPORT AmiThreadPool
    {
    public:
        AmiThreadPool();
        ~AmiThreadPool();

        void start();
        void stop();

        AsioIoService & getIoService();

        static void setEnableMultithreadedBlockingConnects(bool enable = true);
        static bool getEnableMultithreadedBlockingConnects();

    private:
        ThreadPool mThreadPool;
    };

    RCF_EXPORT AmiThreadPool & getAmiThreadPool();

    /*
    class I_RcfClient;
    
    // Deprecated.
    class RCF_EXPORT AsyncTimer
    {
    public:
        AsyncTimer();
        ~AsyncTimer();
        void set(boost::function0<void> onTimer, boost::uint32_t timeoutMs);
        void cancel();

    private:
        boost::shared_ptr<I_RcfClient> mDummyPtr;

    };
    */

} // namespace RCF

#endif // ! INCLUDE_RCF_AMITHREADPOOL_HPP
