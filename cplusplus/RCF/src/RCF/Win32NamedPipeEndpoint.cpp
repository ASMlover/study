
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

#include <RCF/Win32NamedPipeEndpoint.hpp>

#include <RCF/InitDeinit.hpp>
#include <RCF/util/Tchar.hpp>
#include <RCF/Win32NamedPipeClientTransport.hpp>
#include <RCF/Win32NamedPipeServerTransport.hpp>

namespace RCF {

    Win32NamedPipeEndpoint::Win32NamedPipeEndpoint()
    {}

    Win32NamedPipeEndpoint::Win32NamedPipeEndpoint(
        const tstring & pipeName) :
            mPipeName(pipeName)
    {}

    ServerTransportAutoPtr Win32NamedPipeEndpoint::createServerTransport() const
    {
        return ServerTransportAutoPtr(
            new Win32NamedPipeServerTransport(mPipeName));
    }

    ClientTransportAutoPtr Win32NamedPipeEndpoint::createClientTransport() const
    {            
        return ClientTransportAutoPtr(
            new Win32NamedPipeClientTransport(mPipeName));
    }

    EndpointPtr Win32NamedPipeEndpoint::clone() const
    {
        return EndpointPtr( new Win32NamedPipeEndpoint(*this) );
    }

    std::pair<tstring, HANDLE> generateNewPipeName()
    {
        tstring pipePrefix = RCF_T("\\\\.\\pipe\\RcfTestPipe_");
        static unsigned int i = 0;
        while (true)
        {
            MemOstream os;
            os 
                << toAstring(pipePrefix)
                << ++i;

            tstring candidateName = toTstring(os.string());

            DWORD dwOpenMode = 
                    PIPE_ACCESS_DUPLEX 
                |   FILE_FLAG_OVERLAPPED 
                |   FILE_FLAG_FIRST_PIPE_INSTANCE;

            DWORD dwPipeMode = 
                    PIPE_TYPE_BYTE 
                |   PIPE_READMODE_BYTE 
                |   PIPE_WAIT;

            HANDLE hPipe = CreateNamedPipe( 
                candidateName.c_str(),
                dwOpenMode,
                dwPipeMode,
                1,          // MaxPipeInstances
                4096,       // OutBufferSize
                4096,       // InBufferSize
                0,          // DefaultTimeoutMs
                NULL);      // pSecurityASttributes

            DWORD dwErr = GetLastError();
            RCF_UNUSED_VARIABLE(dwErr);

            if (hPipe != INVALID_HANDLE_VALUE)
            {
                return std::make_pair(candidateName, hPipe);
            }

            CloseHandle(hPipe);
        }
    }

    std::string Win32NamedPipeEndpoint::asString() const
    {
        MemOstream os;
        os << "pipe://" << RCF::toAstring(mPipeName);
        return os.string();
    }

    tstring Win32NamedPipeEndpoint::getPipeName()
    {
        return mPipeName;
    }

} // namespace RCF
