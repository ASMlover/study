
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

#ifndef INCLUDE_RCF_CURRENTSESSION_HPP
#define INCLUDE_RCF_CURRENTSESSION_HPP

#include <boost/shared_ptr.hpp>

#include <RCF/RcfSession.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    class CurrentRcfSessionSentry
    {
    public:
        CurrentRcfSessionSentry(RcfSession & session)
        {
            setTlsRcfSessionPtr(& session);
        }

        CurrentRcfSessionSentry(RcfSessionPtr sessionPtr)
        {
            RcfSession * pSession = sessionPtr.get();
            setTlsRcfSessionPtr(pSession);
        }

        ~CurrentRcfSessionSentry()
        {
            setTlsRcfSessionPtr();
        }
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_CURRENTSESSION_HPP
