
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

#include <RCF/Service.hpp>

#include <RCF/RcfServer.hpp>

namespace RCF {

    I_Service::I_Service()
    {}

    void I_Service::setThreadPool(ThreadPoolPtr threadPoolPtr)
    {
        mThreadPoolPtr = threadPoolPtr;
    }

    void I_Service::resetMuxers()
    {
        if (mThreadPoolPtr)
        {
            mThreadPoolPtr->resetMuxers();
        }

        for (std::size_t i=0; i<mTaskEntries.size(); ++i)
        {
            mTaskEntries[i].resetMuxers();          
        }
    }

    void I_Service::onServiceAdded(RcfServer &server) 
    {
        RCF_UNUSED_VARIABLE(server);
    }

    void I_Service::onServiceRemoved(RcfServer &server) 
    {
        RCF_UNUSED_VARIABLE(server);
    }
   
    void I_Service::onServerStart(RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);
    }
   
    void I_Service::onServerStop(RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);
    }

} // namespace RCF
