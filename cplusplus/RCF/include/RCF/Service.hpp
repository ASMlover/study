
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

#ifndef INCLUDE_RCF_SERVICE_HPP
#define INCLUDE_RCF_SERVICE_HPP

#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/ServerTask.hpp>
#include <RCF/ThreadLibrary.hpp>

namespace RCF {

    class                                   I_Service;
    class                                   RcfServer;
    class                                   StubEntry;
    class                                   Token;
    typedef boost::shared_ptr<I_Service>    ServicePtr;
    typedef boost::shared_ptr<StubEntry>    StubEntryPtr;

    class RCF_EXPORT I_Service
    {
    public:
        I_Service();

        virtual ~I_Service()
        {}
       
        virtual void        onServiceAdded(RcfServer &server);
        virtual void        onServiceRemoved(RcfServer &server);
        virtual void        onServerStart(RcfServer &server);
        virtual void        onServerStop(RcfServer &server);

        void                setThreadPool(ThreadPoolPtr threadPoolPtr);
        void                resetMuxers();

        virtual void        onTimer() 
        {}

    protected:

        friend class        RcfServer;

        TaskEntries         mTaskEntries;

        ThreadPoolPtr       mThreadPoolPtr;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_SERVICE_HPP
