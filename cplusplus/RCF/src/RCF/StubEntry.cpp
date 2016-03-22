
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

#include <RCF/StubEntry.hpp>

#include <RCF/Exception.hpp>
#include <RCF/Tools.hpp>

#include <time.h>

namespace RCF {

    // time in s since ca 1970, may fail after year 2038
    inline unsigned int getCurrentTimeS()
    {
        return static_cast<unsigned int>(time(NULL));
    }

    TokenMapped::TokenMapped() :
        mTimeStamp(getCurrentTimeS())
    {
    }

    StubEntry::StubEntry(RcfClientPtr rcfClientPtr) :
        mRcfClientPtr(rcfClientPtr)
    {
        RCF_ASSERT(rcfClientPtr);
    }

    RcfClientPtr StubEntry::getRcfClientPtr() const
    {
        return mRcfClientPtr;
    }

    void TokenMapped::touch()
    {
        // TODO: if we need sync at all for this, then InterlockedExchange etc
        // would be better

        Lock lock(mMutex);
        mTimeStamp = getCurrentTimeS();
    }

    unsigned int TokenMapped::getElapsedTimeS() const
    {
        Lock lock(mMutex);
        if (mTimeStamp == 0)
        {
            return 0;
        }
        else
        {
            unsigned int currentTimeS = getCurrentTimeS();
            return currentTimeS > mTimeStamp ?
                currentTimeS - mTimeStamp :
                mTimeStamp - currentTimeS;
        }
    }

} // namespace RCF
