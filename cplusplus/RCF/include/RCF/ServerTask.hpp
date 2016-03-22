
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

#ifndef INCLUDE_RCF_SERVERTASK_HPP
#define INCLUDE_RCF_SERVERTASK_HPP

#include <RCF/Export.hpp>
#include <RCF/ThreadPool.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    class RcfServer;

    class RCF_EXPORT TaskEntry
    {
    public:
        TaskEntry(
            Task                    task,
            StopFunctor             stopFunctor,
            const std::string &     threadName,
            bool                    autoStart = true);

        TaskEntry(
            MuxerType               muxerType);

        ThreadPool &   
                getThreadPool();

        void    setThreadPoolPtr(ThreadPoolPtr threadPoolPtr);
        Task    getTask();
        bool    getAutoStart();
        void    start();
        void    stop();

        void    resetMuxers();

    private:

        friend class    RcfServer;

        MuxerType       mMuxerType;

        Task            mTask;
        StopFunctor     mStopFunctor;
        std::string     mThreadName;

        ThreadPoolPtr   mLocalThreadPoolPtr;
        ThreadPoolPtr   mWhichThreadPoolPtr;

        bool            mAutoStart;
    };

    typedef std::vector<TaskEntry> TaskEntries;

} // namespace RCF

#endif // ! INCLUDE_RCF_SERVERTASK_HPP
