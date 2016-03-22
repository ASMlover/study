
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

#include <RCF/CallbackConnectionService.hpp>

#include <RCF/Marshal.hpp>
#include <RCF/RcfServer.hpp>

#if RCF_FEATURE_LEGACY==1
#include <RCF/ServerInterfaces.hpp>
#endif

namespace RCF {

    CallbackConnectionService::CallbackConnectionService() : mpServer(NULL)
    {
    }

    void CallbackConnectionService::onServiceAdded(RcfServer & server)
    {

#if RCF_FEATURE_LEGACY==1
        server.bind<I_CreateCallbackConnection>(*this);
#endif

    }

    void CallbackConnectionService::onServiceRemoved(RcfServer & server)
    {

#if RCF_FEATURE_LEGACY==1
        server.unbind<I_CreateCallbackConnection>();
#endif

    }

    void CallbackConnectionService::onServerStart(RcfServer & server)
    {
        mOnCallbackConnectionCreated = server.getOnCallbackConnectionCreated();
    }

    void CallbackConnectionService::CreateCallbackConnection()
    {
        if ( !mOnCallbackConnectionCreated )
        {
            RCF_THROW( Exception(_RcfError_ServerCallbacksNotSupported()) );
        }

        RCF::convertRcfSessionToRcfClient( mOnCallbackConnectionCreated );
    }

} // namespace RCF
