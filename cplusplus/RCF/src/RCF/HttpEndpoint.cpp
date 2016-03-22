
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

#include <RCF/HttpEndpoint.hpp>

#include <RCF/HttpClientTransport.hpp>
#include <RCF/HttpServerTransport.hpp>

namespace RCF {

    HttpEndpoint::HttpEndpoint(int port) : 
        TcpEndpoint(port)
    {
    }

    HttpEndpoint::HttpEndpoint(const std::string & ip, int port) : 
        TcpEndpoint(ip, port)
    {
    }

    std::string HttpEndpoint::asString() const
    {
        MemOstream os;
        std::string ip = getIp();
        if (ip.empty())
        {
            ip = "127.0.0.1";
        }
        os << "http://" << ip << ":" << getPort();
        return os.string();
    }

    ServerTransportAutoPtr HttpEndpoint::createServerTransport() const
    {
        return ServerTransportAutoPtr( new HttpServerTransport(*this) );
    }

    ClientTransportAutoPtr HttpEndpoint::createClientTransport() const
    {
        return ClientTransportAutoPtr( new HttpClientTransport(*this) );
    }

    EndpointPtr HttpEndpoint::clone() const
    {
        return EndpointPtr( new HttpEndpoint(*this) );
    }

} // namespace RCF

