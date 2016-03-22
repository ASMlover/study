
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

#include <RCF/HttpClientTransport.hpp>

#include <RCF/HttpEndpoint.hpp>
#include <RCF/HttpFrameFilter.hpp>

#include <RCF/Filter.hpp>

namespace RCF {

    HttpClientTransport::HttpClientTransport(const HttpClientTransport & rhs) :
        TcpClientTransport(rhs)
    {
        std::vector<FilterPtr> wireFilters;

        wireFilters.push_back(FilterPtr(new HttpFrameFilter(
            getRemoteAddr().getIp(),
            getRemoteAddr().getPort())));

        setWireFilters(wireFilters);
    }

    HttpClientTransport::HttpClientTransport(const HttpEndpoint & httpEndpoint) : 
        TcpClientTransport(httpEndpoint.getIp(), httpEndpoint.getPort())
    {
        std::vector<FilterPtr> wireFilters;

        wireFilters.push_back(FilterPtr(new HttpFrameFilter(
            getRemoteAddr().getIp(),
            getRemoteAddr().getPort())));

        setWireFilters(wireFilters);
    }

    TransportType HttpClientTransport::getTransportType()
    {
        return Tt_Http;
    }

    EndpointPtr HttpClientTransport::getEndpointPtr() const
    {
        EndpointPtr epPtr = TcpClientTransport::getEndpointPtr();
        TcpEndpoint& tcpEndpoint = static_cast<TcpEndpoint&>(*epPtr);
        return EndpointPtr(new HttpEndpoint(tcpEndpoint.getIp(), tcpEndpoint.getPort()));
    }

    ClientTransportAutoPtr HttpClientTransport::clone() const
    {
        return ClientTransportAutoPtr(new HttpClientTransport(*this));
    }

} // namespace RCF

