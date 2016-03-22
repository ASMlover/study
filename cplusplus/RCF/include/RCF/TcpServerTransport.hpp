
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

#ifndef INCLUDE_RCF_TCPSERVERTRANSPORT_HPP
#define INCLUDE_RCF_TCPSERVERTRANSPORT_HPP

#include <RCF/AsioServerTransport.hpp>

#include <RCF/Asio.hpp>

namespace RCF {

    class TcpServerTransport;

    class RCF_EXPORT TcpNetworkSession : public AsioNetworkSession
    {
    public:

        TcpNetworkSession(
            TcpServerTransport &transport,
            AsioIoService & ioService);

        const RemoteAddress & implGetRemoteAddress();

        void implRead(char * buffer, std::size_t bufferLen);

        void implWrite(const std::vector<ByteBuffer> & buffers);

        void implWrite(
            AsioNetworkSession &toBeNotified, 
            const char * buffer, 
            std::size_t bufferLen);

        void implAccept();

        bool implOnAccept();

        void implClose();

        void implCloseAfterWrite();

        bool implIsConnected();

        ClientTransportAutoPtr implCreateClientTransport();

        void implTransferNativeFrom(ClientTransport & clientTransport);

        int getNativeHandle();

    private:

        AsioSocketPtr               mSocketPtr;
        IpAddress                   mIpAddress;
        int                         mWriteCounter;
    };

    class RCF_EXPORT TcpServerTransport : 
        public AsioServerTransport,
        public IpServerTransport
    {
    public:
        TcpServerTransport(const IpAddress & ipAddress);
        TcpServerTransport(const std::string & ip, int port);

        TransportType getTransportType();

        ServerTransportPtr clone();

        // IpServerTransport implementation
        int                    getPort() const;

    private:

        AsioNetworkSessionPtr     implCreateNetworkSession();
        void                    implOpen();

        void                    onServerStart(RcfServer & server);

        ClientTransportAutoPtr  implCreateClientTransport(
                                    const Endpoint &endpoint);

    private:
        IpAddress               mIpAddress;

        int                     mAcceptorFd;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_TCPSERVERTRANSPORT_HPP
