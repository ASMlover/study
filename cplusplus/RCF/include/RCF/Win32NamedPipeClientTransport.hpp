
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

#ifndef INCLUDE_RCF_WIN32NAMEDPIPECLIENTTRANSPORT_HPP
#define INCLUDE_RCF_WIN32NAMEDPIPECLIENTTRANSPORT_HPP

#include <RCF/ConnectedClientTransport.hpp>

#include <RCF/util/Tchar.hpp>

#include <RCF/AsioFwd.hpp>

namespace RCF {

    class RCF_EXPORT Win32NamedPipeClientTransport :
        public ConnectedClientTransport
    {
    public:

        Win32NamedPipeClientTransport(
            const Win32NamedPipeClientTransport & rhs);

        Win32NamedPipeClientTransport(
            const tstring & pipeName);

        Win32NamedPipeClientTransport(AsioPipeHandlePtr socketPtr, const tstring & pipeName);

        ~Win32NamedPipeClientTransport();

        TransportType getTransportType();

        ClientTransportAutoPtr clone() const;

        HANDLE getNativeHandle() const;

        AsioPipeHandlePtr releaseSocket();

        void setDisconnectBeforeClosing(bool disconnectBeforeClosing);

        tstring getPipeName() const;
        void setPipeName(const tstring & pipeName);

        void setSecurityAttributes(LPSECURITY_ATTRIBUTES pSec);

        void            associateWithIoService(AsioIoService & ioService);
        bool            isAssociatedWithIoService();

    private:

        std::size_t implRead(
            const ByteBuffer &byteBuffer,
            std::size_t bytesRequested);

        std::size_t implReadAsync(
            const ByteBuffer &byteBuffer,
            std::size_t bytesRequested);

        std::size_t implWrite(
            const std::vector<ByteBuffer> &byteBuffers);

        std::size_t implWriteAsync(
            const std::vector<ByteBuffer> &byteBuffers);

        void implClose();

        void implConnect(
            ClientTransportCallback &clientStub,
            unsigned int timeoutMs);

        void implConnectAsync(
            ClientTransportCallback &clientStub,
            unsigned int timeoutMs);

        // I_ClientTransport
        EndpointPtr             getEndpointPtr() const;
        bool                    isConnected();

    private:

        tstring                 mEpPipeName;
        tstring                 mPipeName;
        HANDLE                  mhPipe;
        HANDLE                  mhEvent;

        LPSECURITY_ATTRIBUTES   mpSec;

        bool                    mAsyncMode;

        AsioPipeHandlePtr       mSocketPtr; 
        AsioIoService *         mpIoService;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_WIN32NAMEDPIPECLIENTTRANSPORT_HPP
