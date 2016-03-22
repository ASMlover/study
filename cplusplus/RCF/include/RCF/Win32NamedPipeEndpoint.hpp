
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

#ifndef INCLUDE_RCF_WIN32NAMEDPIPEENDPOINT_HPP
#define INCLUDE_RCF_WIN32NAMEDPIPEENDPOINT_HPP

#include <RCF/Endpoint.hpp>
#include <RCF/Export.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/ServerTransport.hpp>

#include <RCF/util/Tchar.hpp>
#include <tchar.h>

namespace RCF {

    /// Represents a Win32 named pipe endpoint. Only available on Windows platforms.
    class RCF_EXPORT Win32NamedPipeEndpoint : public Endpoint
    {
    public:

        Win32NamedPipeEndpoint();

        // *** SWIG BEGIN ***

        /// Constructs a Win32 named pipe endpoint with the given pipe name.
        Win32NamedPipeEndpoint(const tstring & pipeName);

        // *** SWIG END ***

        ServerTransportAutoPtr createServerTransport() const;
        ClientTransportAutoPtr createClientTransport() const;
        EndpointPtr clone() const;

        std::string asString() const;

        tstring getPipeName();

    private:
        tstring mPipeName;
    };

    RCF_EXPORT std::pair<tstring, HANDLE> generateNewPipeName();

} // namespace RCF

#endif // ! INCLUDE_RCF_WIN32NAMEDPIPEENDPOINT_HPP
