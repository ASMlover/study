
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

#ifndef INCLUDE_RCF_SESSIONTIMEOUTSERVICE_HPP
#define INCLUDE_RCF_SESSIONTIMEOUTSERVICE_HPP

#include <set>

#include <RCF/Export.hpp>
#include <RCF/PeriodicTimer.hpp>
#include <RCF/Service.hpp>
#include <RCF/Timer.hpp>

namespace RCF {

    class NetworkSession;
    typedef boost::shared_ptr<NetworkSession> NetworkSessionPtr;
    typedef boost::weak_ptr<NetworkSession> NetworkSessionWeakPtr;

    class RCF_EXPORT SessionTimeoutService : public I_Service
    {
    public:
        SessionTimeoutService();

    private:

        void onServerStart(RcfServer & server);
        void onServerStop(RcfServer & server);

        void onTimer();

    private:

        std::vector<NetworkSessionWeakPtr>  mSessionsTemp;

        boost::uint32_t                 mSessionTimeoutMs;
        boost::uint32_t                 mReapingIntervalMs;
        RcfServer *                     mpRcfServer;
        PeriodicTimer                   mPeriodicTimer;
    };

    typedef boost::shared_ptr<SessionTimeoutService> SessionTimeoutServicePtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_SESSIONTIMEOUTSERVICE_HPP
