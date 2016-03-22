
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

#ifndef INCLUDE_RCF_TCPCLIENTTRANSPORT_HPP
#define INCLUDE_RCF_TCPCLIENTTRANSPORT_HPP

#include <RCF/Filter.hpp>
#include <RCF/BsdClientTransport.hpp>
#include <RCF/ByteOrdering.hpp>
#include <RCF/ClientProgress.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/Exception.hpp>
#include <RCF/Export.hpp>
#include <RCF/IpAddress.hpp>
#include <RCF/IpClientTransport.hpp>

namespace RCF {

    class RCF_EXPORT TcpClientTransport : 
        public BsdClientTransport, 
        public IpClientTransport
    {
    public:
        TcpClientTransport(const TcpClientTransport &rhs);
        TcpClientTransport(const IpAddress &remoteAddr);
        TcpClientTransport(const std::string & ip, int port);
        
        TcpClientTransport(TcpSocketPtr socketPtr);

        ~TcpClientTransport();

        TransportType getTransportType();

        ClientTransportAutoPtr  clone() const;

        void                    implConnect(
                                    ClientTransportCallback &clientStub, 
                                    unsigned int timeoutMs);

        void                    implConnectAsync(
                                    ClientTransportCallback &clientStub, 
                                    unsigned int timeoutMs);

        void                    implClose();
        EndpointPtr             getEndpointPtr() const;

        void                    setRemoteAddr(const IpAddress &remoteAddr);
        IpAddress               getRemoteAddr() const;

        
        bool                    isAssociatedWithIoService();
        void                    associateWithIoService(AsioIoService & ioService);

    private:

        static void             doDnsLookup(
                                    std::size_t index,
                                    unsigned int timeoutMs,
                                    OverlappedAmiPtr overlappedPtr,
                                    IpAddress ipAddress);

        void                    onDnsLookupCompleted(
                                    unsigned int timeoutMs,
                                    IpAddress ipAddress, 
                                    ExceptionPtr e);

        void                    setupSocket();
        void                    setupSocket(Exception & e);

        friend class HttpFrameFilter;

        // Address of the RcfServer.
        IpAddress               mServerAddr;

        // Actual network address we connect to. May be the same as mServerAddr,
        // but could also be the address of a HTTP proxy.
        IpAddress               mConnectionAddr;
    };
    
} // namespace RCF

#endif // ! INCLUDE_RCF_TCPCLIENTTRANSPORT_HPP
