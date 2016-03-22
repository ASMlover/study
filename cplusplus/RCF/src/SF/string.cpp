
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

#include <SF/string.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace SF {

    bool getCurrentNativeWstringSerialization()
    {
        bool useNativeWstringSerialization = RCF::getDefaultNativeWstringSerialization();
        RCF::ClientStub * pClientStub = RCF::getTlsClientStubPtr();
        RCF::RcfSession * pRcfSession = RCF::getTlsRcfSessionPtr();

        if (pClientStub)
        {
            useNativeWstringSerialization = pClientStub->getNativeWstringSerialization();
        }
        else if (pRcfSession)
        {
            useNativeWstringSerialization = pRcfSession->getNativeWstringSerialization();
        }
        return useNativeWstringSerialization;
    }

}
