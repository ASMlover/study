
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

#ifndef INCLUDE_RCF_STUBENTRY_HPP
#define INCLUDE_RCF_STUBENTRY_HPP

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/ThreadLibrary.hpp>

namespace RCF {

    class                                       I_RcfClient;
    class                                       StubEntry;
    typedef boost::shared_ptr<I_RcfClient>      RcfClientPtr;
    typedef boost::shared_ptr<StubEntry>        StubEntryPtr;

    class RCF_EXPORT TokenMapped
    {
    public:

        TokenMapped();

        virtual ~TokenMapped()
        {}

        virtual void            touch();
        virtual unsigned int    getElapsedTimeS() const;

    private:
        mutable Mutex   mMutex;
        unsigned int    mTimeStamp;
    };

    typedef boost::shared_ptr<TokenMapped> TokenMappedPtr;

    // TODO: collapse this class into a RcfClientPtr.
    class RCF_EXPORT StubEntry : 
        public TokenMapped,
        boost::noncopyable
    {
    public:
        StubEntry(RcfClientPtr rcfClientPtr);
        RcfClientPtr    getRcfClientPtr() const;

    private:
        RcfClientPtr    mRcfClientPtr;        
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_STUBENTRY_HPP
