
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

#include <RCF/ServerTask.hpp>

namespace RCF {

    TaskEntry::TaskEntry(
        Task                    task,
        StopFunctor             stopFunctor,
        const std::string &     threadName,
        bool                    autoStart) :
            mMuxerType(Mt_None),
            mTask(task),
            mStopFunctor(stopFunctor),
            mThreadName(threadName),
            mAutoStart(autoStart)
    {
    }

    TaskEntry::TaskEntry(MuxerType muxerType) : 
        mMuxerType(muxerType), 
        mAutoStart(true)
    {
    }

    ThreadPool & TaskEntry::getThreadPool()
    {
        return *mWhichThreadPoolPtr;
    }

    void TaskEntry::setThreadPoolPtr(ThreadPoolPtr threadPoolPtr)
    {
        mLocalThreadPoolPtr = threadPoolPtr;
        threadPoolPtr->setTask(mTask);
        threadPoolPtr->setStopFunctor(mStopFunctor);
    }

    Task TaskEntry::getTask()
    {
        return mTask;
    }

    bool TaskEntry::getAutoStart()
    {
        return mAutoStart;
    }

    void TaskEntry::start()
    {
        mWhichThreadPoolPtr->start();
    }

    void TaskEntry::stop()
    {
        if (mWhichThreadPoolPtr)
        {
            mWhichThreadPoolPtr->stop();
        }
    }

    void TaskEntry::resetMuxers()
    {
        if (mLocalThreadPoolPtr)
        {
            mWhichThreadPoolPtr.reset();
            mLocalThreadPoolPtr->resetMuxers();
        }
    }

} // namespace RCF
