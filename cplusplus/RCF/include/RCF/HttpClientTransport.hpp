
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

#ifndef INCLUDE_RCF_HTTPCLIENTTRANSPORT_HPP
#define INCLUDE_RCF_HTTPCLIENTTRANSPORT_HPP

#include <RCF/TcpClientTransport.hpp>

namespace RCF { 

    class HttpEndpoint;

    class RCF_EXPORT HttpClientTransport : public TcpClientTransport
    {
    public:
        HttpClientTransport(const HttpClientTransport & rhs);
        HttpClientTransport(const HttpEndpoint & httpEndpoint);
        TransportType getTransportType();
        EndpointPtr getEndpointPtr() const;

        std::auto_ptr<ClientTransport> clone() const;

    };

} // namespace RCF

#endif // INCLUDE_RCF_HTTPCLIENTTRANSPORT_HPP
