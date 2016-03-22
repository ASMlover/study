
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

#include <RCF/SessionObjectFactoryService.hpp>

#include <RCF/ServerInterfaces.hpp>

namespace RCF {

    void SessionObjectFactoryService::onServiceAdded(RcfServer &server)
    {
        server.bind<I_SessionObjectFactory>(*this);
    }

    void SessionObjectFactoryService::onServiceRemoved(RcfServer &server)
    {
        server.unbind<I_SessionObjectFactory>();
    }

} // namespace RCF
