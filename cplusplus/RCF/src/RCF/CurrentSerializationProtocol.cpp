
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

#include <RCF/CurrentSerializationProtocol.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    SerializationProtocolIn *getCurrentSerializationProtocolIn()
    {
        ClientStub * pClientStub = RCF::getTlsClientStubPtr();
        RcfSession * pRcfSession = RCF::getTlsRcfSessionPtr();
        if (pClientStub)
        {
            return &pClientStub->getSpIn();
        }
        else if (pRcfSession)
        {
            return &pRcfSession->getSpIn();
        }
        else
        {
            return NULL;
        }
    }

    SerializationProtocolOut *getCurrentSerializationProtocolOut()
    {
        ClientStub * pClientStub = RCF::getTlsClientStubPtr();
        RcfSession * pRcfSession = RCF::getTlsRcfSessionPtr();
        if (pClientStub)
        {
            return &pClientStub->getSpOut();
        }
        else if (pRcfSession)
        {
            return &pRcfSession->getSpOut();
        }
        else
        {
            return NULL;
        }
    }

} // namespace RCF
