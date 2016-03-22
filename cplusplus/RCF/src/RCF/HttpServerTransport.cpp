
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

#include <RCF/HttpServerTransport.hpp>

#include <RCF/HttpEndpoint.hpp>
#include <RCF/HttpFrameFilter.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/TcpClientTransport.hpp>

namespace RCF {

    HttpServerTransport::HttpServerTransport(const HttpEndpoint & httpEndpoint) : 
        TcpServerTransport(httpEndpoint.getIp(), httpEndpoint.getPort())
    {
        mWireProtocol = Wp_Http;
    }

    TransportType HttpServerTransport::getTransportType()
    {
        return Tt_Http;
    }

} // namespace RCF
