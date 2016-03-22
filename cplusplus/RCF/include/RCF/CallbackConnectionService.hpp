
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

#ifndef INCLUDE_RCF_CALLBACKCONNECTIONSERVICE_HPP
#define INCLUDE_RCF_CALLBACKCONNECTIONSERVICE_HPP

#include <RCF/Service.hpp>

namespace RCF {

    class RcfSession;
    typedef boost::shared_ptr<RcfSession> RcfSessionPtr;

    class ClientTransport;
    typedef std::auto_ptr<ClientTransport> ClientTransportAutoPtr;

    typedef boost::function2<void, RcfSessionPtr, ClientTransportAutoPtr> OnCallbackConnectionCreated;

    class I_CreateCallbackConnection;
    template<typename T> class RcfClient;

    class RcfServer;

    class CallbackConnectionService : public I_Service, boost::noncopyable
    {
    public:

        CallbackConnectionService();

    private:

        void onServiceAdded(RcfServer & server);
        void onServiceRemoved(RcfServer & server);
        void onServerStart(RcfServer & server);

    public:
        friend class RcfClient<I_CreateCallbackConnection>;
        void CreateCallbackConnection();

    private:
        RcfServer * mpServer;
        OnCallbackConnectionCreated mOnCallbackConnectionCreated;
    };

    typedef boost::shared_ptr<CallbackConnectionService> CallbackConnectionServicePtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_CALLBACKCONNECTIONSERVICE_HPP
