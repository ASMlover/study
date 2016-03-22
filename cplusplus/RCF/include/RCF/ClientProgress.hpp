
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

#ifndef INCLUDE_RCF_CLIENTPROGRESS_HPP
#define INCLUDE_RCF_CLIENTPROGRESS_HPP

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Tools.hpp>

namespace RCF {

    class ClientProgress
    {
    public:

        enum Activity
        {
            Connect,
            Send,
            Receive
        };

        enum Action
        {
            Cancel,
            Continue
        };

        enum Trigger
        {
            Event = 1,
            Timer = 2,
            UiMessage = 4
        };

        typedef boost::function5<
            void,
                std::size_t,
                std::size_t,
                Trigger,
                Activity,
                Action &> ProgressCallback;

        ClientProgress() :
            mProgressCallback(),
            mTriggerMask(),
            mTimerIntervalMs(),
            mUiMessageFilter()
        {}

        ProgressCallback    mProgressCallback;
        int                 mTriggerMask;
        unsigned int        mTimerIntervalMs;
        int                 mUiMessageFilter;
    };

    typedef boost::shared_ptr<ClientProgress> ClientProgressPtr;

    class WithProgressCallback
    {
    public:

        virtual ~WithProgressCallback()
        {}

        void setClientProgressPtr(ClientProgressPtr clientProgressPtr)
        {
            mClientProgressPtr = clientProgressPtr;
        }

    protected:
        ClientProgressPtr mClientProgressPtr;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_CLIENTPROGRESS_HPP
