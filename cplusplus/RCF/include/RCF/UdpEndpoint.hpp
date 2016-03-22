
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

#ifndef INCLUDE_RCF_UDPENDPOINT_HPP
#define INCLUDE_RCF_UDPENDPOINT_HPP

#include <memory>
#include <string>

#include <boost/shared_ptr.hpp>

#include <RCF/Endpoint.hpp>
#include <RCF/Export.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/IpAddress.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/TypeTraits.hpp>

namespace RCF {

    class ServerTransport;
    class ClientTransport;

    /// Represents a UDP endpoint.
    class RCF_EXPORT UdpEndpoint : public Endpoint
    {
    public:

        // *** SWIG BEGIN ***

        /// Constructs a UdpEndpoint from a port number. The IP address defaults to 127.0.0.1 .
        UdpEndpoint(int port);

        /// Constructs a UdpEndpoint from an IP address and port number.
        UdpEndpoint(const std::string &ip, int port);

        std::string         getIp() const;
        int                 getPort() const;
        std::string         asString() const;

        UdpEndpoint &       enableSharedAddressBinding(bool enable = true);
        UdpEndpoint &       listenOnMulticast(const IpAddress & multicastIp);
        UdpEndpoint &       listenOnMulticast(const std::string & multicastIp);

        // *** SWIG END ***

        UdpEndpoint();
        UdpEndpoint(const IpAddress & ipAddress);
        UdpEndpoint(const UdpEndpoint &rhs);
       
        std::auto_ptr<ServerTransport>    createServerTransport() const;
        std::auto_ptr<ClientTransport>    createClientTransport() const;
        EndpointPtr                         clone() const;
       
    private:
        IpAddress           mIp;
        IpAddress           mMulticastIp;
        bool                mEnableSharedAddressBinding;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_UDPENDPOINT_HPP
