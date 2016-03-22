
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

#ifndef INCLUDE_RCF_ENDPOINT_HPP
#define INCLUDE_RCF_ENDPOINT_HPP

#include <memory>
#include <string>

#include <boost/shared_ptr.hpp>

#include <RCF/Exception.hpp>
#include <RCF/SerializationProtocol.hpp>

namespace RCF {

    class ServerTransport;
    class ClientTransport;

    class Endpoint;
    typedef boost::shared_ptr<Endpoint> EndpointPtr;

    /// Base class for all network endpoint types.
    class RCF_EXPORT Endpoint
    {
    public:

        // *** SWIG BEGIN ***
        
        virtual ~Endpoint() {}
        
        // *** SWIG END ***

        virtual std::auto_ptr<ServerTransport>  createServerTransport() const = 0;
        virtual std::auto_ptr<ClientTransport>  createClientTransport() const = 0;
        virtual EndpointPtr                     clone() const = 0;
        virtual std::string                     asString() const = 0;
        void                                    serialize(SF::Archive &) {}
    };

} // namespace RCF

#include <boost/version.hpp>

#endif // ! INCLUDE_RCF_ENDPOINT_HPP
