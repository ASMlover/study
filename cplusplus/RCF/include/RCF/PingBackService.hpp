
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

#ifndef INCLUDE_RCF_PINGBACKSERVICE_HPP
#define INCLUDE_RCF_PINGBACKSERVICE_HPP

#include <functional>
#include <set>

#include <RCF/Export.hpp>
#include <RCF/Heap.hpp>
#include <RCF/Service.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    class RcfSession;
    typedef boost::shared_ptr<RcfSession> RcfSessionPtr;
    typedef boost::weak_ptr<RcfSession> RcfSessionWeakPtr;
    
    typedef std::pair<boost::uint32_t, RcfSessionWeakPtr>   PingBackTimerEntry;

    class RCF_EXPORT PingBackService : public I_Service
    {
    public:

        typedef boost::weak_ptr<RcfSession> RcfSessionWeakPtr;

        PingBackService();

        PingBackTimerEntry  registerSession(RcfSessionPtr rcfSessionPtr);
        void                unregisterSession(const PingBackTimerEntry & entry);

    private:

        typedef PingBackTimerEntry Entry;

        void                onServiceAdded(RcfServer &server);
        void                onServiceRemoved(RcfServer &server);
        void                stop();
        void                cycle(int timeoutMs);

        bool                            mEnabled;
        bool                            mLazyStarted;

        // Internally synchronized.
        TimerHeap<RcfSessionWeakPtr>    mTimerHeap;

        Mutex                           mMutex;
        Condition                       mCondition;
    };

    typedef boost::shared_ptr<PingBackService> PingBackServicePtr;    

} // namespace RCF

#endif // ! INCLUDE_RCF_PINGBACKSERVICE_HPP
