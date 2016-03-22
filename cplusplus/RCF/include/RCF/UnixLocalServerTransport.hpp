
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

#ifndef INCLUDE_RCF_UNIXLOCALSERVERTRANSPORT_HPP
#define INCLUDE_RCF_UNIXLOCALSERVERTRANSPORT_HPP

#if defined(BOOST_WINDOWS)
#error Unix domain sockets not supported on Windows.
#endif

#ifndef RCF_HAS_LOCAL_SOCKETS
#error Unix domain sockets not supported by this version of Boost.Asio.
#endif

#include <RCF/AsioServerTransport.hpp>
#include <RCF/Export.hpp>

namespace RCF {

    using ASIO_NS::local::stream_protocol;
    typedef stream_protocol::socket                 UnixLocalSocket;
    typedef boost::shared_ptr<UnixLocalSocket>      UnixLocalSocketPtr;

    class UnixLocalServerTransport;

    class RCF_EXPORT UnixLocalNetworkSession : public AsioNetworkSession
    {
    public:
        UnixLocalNetworkSession(
            UnixLocalServerTransport & transport,
            AsioIoService & ioService);

        const RemoteAddress & implGetRemoteAddress();

        void implRead(char * buffer, std::size_t bufferLen);

        void implWrite(const std::vector<ByteBuffer> & buffers);

        void implAccept();

        bool implOnAccept();

        bool implIsConnected();

        void implClose();

        void implCloseAfterWrite();

        ClientTransportAutoPtr implCreateClientTransport();

        void implTransferNativeFrom(ClientTransport & clientTransport);

        int getNativeHandle();

    private:
        UnixLocalSocketPtr          mSocketPtr;
        std::string                 mRemoteFileName;
        NoRemoteAddress             mRemoteAddress;
    };

    class RCF_EXPORT UnixLocalServerTransport : 
        public AsioServerTransport
    {
    public:

        UnixLocalServerTransport(const std::string & fileName);

        TransportType getTransportType();

        ServerTransportPtr clone();

        AsioNetworkSessionPtr implCreateNetworkSession();
        
        void implOpen();

        ClientTransportAutoPtr implCreateClientTransport(
            const Endpoint &endpoint);

        std::string getPipeName() const;

        void onServerStart(RcfServer & server);
        void onServerStop(RcfServer & server);

    private:

        const std::string               mFileName;      
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_UNIXLOCALSERVERTRANSPORT_HPP
