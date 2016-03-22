
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

#ifndef INCLUDE_RCF_HTTPSENDPOINT_HPP
#define INCLUDE_RCF_HTTPSENDPOINT_HPP

#include <RCF/Export.hpp>
#include <RCF/TcpEndpoint.hpp>

namespace RCF {

    /// Represents a HTTPS endpoint.
    class RCF_EXPORT HttpsEndpoint : public TcpEndpoint
    {
    public:

        // *** SWIG BEGIN ***

        /// Constructs an HTTPS endpoint on the given port number.
        HttpsEndpoint(int port);
        
        /// Constructs an HTTPS endpoint on the given IP address and port number.
        HttpsEndpoint(const std::string & ip, int port);

        // *** SWIG END ***

        /// Returns a string representation of the HTTP endpoint.
        std::string asString() const;

        ServerTransportAutoPtr createServerTransport() const;
        ClientTransportAutoPtr createClientTransport() const;
        EndpointPtr clone() const;
    };

} // namespace RCF

#endif // INCLUDE_RCF_HTTPSENDPOINT_HPP
