
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

#include <RCF/NamedPipeEndpoint.hpp>

#include <RCF/InitDeinit.hpp>
#include <RCF/util/Tchar.hpp>

#if RCF_FEATURE_NAMEDPIPE==1
#include <RCF/Win32NamedPipeClientTransport.hpp>
#include <RCF/Win32NamedPipeServerTransport.hpp>
#elif RCF_FEATURE_LOCALSOCKET==1
#include <RCF/UnixLocalServerTransport.hpp>
#include <RCF/UnixLocalClientTransport.hpp>
#endif

namespace RCF {

    NamedPipeEndpoint::NamedPipeEndpoint()
    {}

    NamedPipeEndpoint::NamedPipeEndpoint(const tstring & pipeName) :
        mPipeName(pipeName)
    {}

#if RCF_FEATURE_NAMEDPIPE==1

    ServerTransportAutoPtr NamedPipeEndpoint::createServerTransport() const
    {
        return ServerTransportAutoPtr(
            new Win32NamedPipeServerTransport(mPipeName));
    }

    ClientTransportAutoPtr NamedPipeEndpoint::createClientTransport() const
    {            
        return ClientTransportAutoPtr(
            new Win32NamedPipeClientTransport(mPipeName));
    }

#else
        
    ServerTransportAutoPtr NamedPipeEndpoint::createServerTransport() const
    {
        return ServerTransportAutoPtr(new UnixLocalServerTransport(mPipeName));
    }

    ClientTransportAutoPtr NamedPipeEndpoint::createClientTransport() const
    {
        return ClientTransportAutoPtr(new UnixLocalClientTransport(mPipeName));
    }

#endif

    EndpointPtr NamedPipeEndpoint::clone() const
    {
        return EndpointPtr( new NamedPipeEndpoint(*this) );
    }

    std::string NamedPipeEndpoint::asString() const
    {
        MemOstream os;
        os << "pipe://" << RCF::toAstring(mPipeName);
        return os.string();
    }

} // namespace RCF
