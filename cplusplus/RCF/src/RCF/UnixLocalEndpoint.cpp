
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

#include <RCF/UnixLocalEndpoint.hpp>

#include <RCF/InitDeinit.hpp>

#include <RCF/UnixLocalServerTransport.hpp>
#include <RCF/UnixLocalClientTransport.hpp>

#include <RCF/Asio.hpp>

#ifndef RCF_HAS_LOCAL_SOCKETS
#error Unix domain sockets not supported by this version of Boost.Asio.
#endif

namespace RCF {

    UnixLocalEndpoint::UnixLocalEndpoint()
    {}

    UnixLocalEndpoint::UnixLocalEndpoint(const std::string & pipeName) :
            mPipeName(pipeName)
    {}

    ServerTransportAutoPtr UnixLocalEndpoint::createServerTransport() const
    {
        return ServerTransportAutoPtr(new UnixLocalServerTransport(mPipeName));
    }

    ClientTransportAutoPtr UnixLocalEndpoint::createClientTransport() const
    {            
        return ClientTransportAutoPtr(new UnixLocalClientTransport(mPipeName));
    }

    EndpointPtr UnixLocalEndpoint::clone() const
    {
        return EndpointPtr( new UnixLocalEndpoint(*this) );
    }

    std::string UnixLocalEndpoint::asString() const
    {
        MemOstream os;
        os << "local://" << mPipeName;
        return os.string();
    }

} // namespace RCF
