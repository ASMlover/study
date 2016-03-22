
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

#ifndef INCLUDE_RCF_TEST_PROGRAMTIMELIMIT_HPP
#define INCLUDE_RCF_TEST_PROGRAMTIMELIMIT_HPP

#include <iostream>
#include <boost/bind.hpp>
#include <boost/config.hpp>

#include <RCF/ThreadLibrary.hpp>
#include <RCF/ThreadPool.hpp>
#include <RCF/util/CommandLine.hpp>

class ProgramTimeLimit
{
public:
    ProgramTimeLimit(unsigned int timeLimitS)
    {
        mStartTimeMs = RCF::getCurrentTimeMs();
        mTimeLimitMs = timeLimitS*1000;
        mStopFlag = false;
        if (timeLimitS)
        {
            mThreadPtr.reset( new RCF::Thread( boost::bind(&ProgramTimeLimit::poll, this)));
        }
    }

    ~ProgramTimeLimit()
    {
        if (mThreadPtr)
        {
            {
                RCF::Lock lock(mStopMutex);
                mStopFlag = true;
                mStopCondition.notify_all(lock);
            }
            mThreadPtr->join();
        }
    }

private:

    void poll()
    {
        // Set our thread name.
        RCF::setWin32ThreadName("RCF Program Time Limit");

        while (true)
        {
            unsigned int pollIntervalMs = 1000;
            RCF::Lock lock(mStopMutex);
            mStopCondition.timed_wait(lock, pollIntervalMs);
            if (mStopFlag)
            {
                break;
            }
            else
            {
                unsigned int currentTimeMs = RCF::getCurrentTimeMs();
                if (currentTimeMs - mStartTimeMs > mTimeLimitMs)
                {
                    std::cout 
                        << "Time limit expired (" << mTimeLimitMs/1000 << " (s) ). Killing this test." 
                        << std::endl;

#if defined(_MSC_VER)

                    // By simulating an access violation , we will trigger the 
                    // creation  of a minidump, which will aid postmortem analysis.

                    int * pn = 0;
                    *pn = 1;

#elif defined(BOOST_WINDOWS)
                    
                    TerminateProcess(GetCurrentProcess(), 1);

#else

                    abort();

#endif
                }
            }
        }
    }

    unsigned int mStartTimeMs;
    unsigned int mTimeLimitMs;
    RCF::ThreadPtr mThreadPtr;
    bool mStopFlag;

    RCF::Mutex mStopMutex;
    RCF::Condition mStopCondition;
};

ProgramTimeLimit * gpProgramTimeLimit = NULL;

#endif // ! INCLUDE_RCF_TEST_PROGRAMTIMELIMIT_HPP
