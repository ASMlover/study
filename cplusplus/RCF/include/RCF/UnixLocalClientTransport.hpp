
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

#ifndef INCLUDE_RCF_UNIXLOCALCLIENTTRANSPORT_HPP
#define INCLUDE_RCF_UNIXLOCALCLIENTTRANSPORT_HPP

#if defined(BOOST_WINDOWS)
#error Unix domain sockets not supported on Windows.
#endif

#include <RCF/BsdClientTransport.hpp>
#include <RCF/Export.hpp>
#include <RCF/IpClientTransport.hpp>

#include <sys/un.h>

namespace RCF {

    using ASIO_NS::local::stream_protocol;
    typedef stream_protocol::socket                 UnixLocalSocket;
    typedef boost::shared_ptr<UnixLocalSocket>      UnixLocalSocketPtr;

    class RCF_EXPORT UnixLocalClientTransport : 
        public BsdClientTransport
    {
    public:
        UnixLocalClientTransport(const UnixLocalClientTransport &rhs);
        UnixLocalClientTransport(const std::string &fileName);
        UnixLocalClientTransport(const sockaddr_un &remoteAddr);
        UnixLocalClientTransport(UnixLocalSocketPtr socketPtr, const std::string & fileName);

        ~UnixLocalClientTransport();

        TransportType getTransportType();

        ClientTransportAutoPtr clone() const;

        void                    implConnect(unsigned int timeoutMs);

        void                    implConnect(
                                    ClientTransportCallback &clientStub, 
                                    unsigned int timeoutMs);

        void                    implConnectAsync(
                                    ClientTransportCallback &clientStub, 
                                    unsigned int timeoutMs);

        void                    implClose();
        EndpointPtr             getEndpointPtr() const;

        void                    setRemoteAddr(const sockaddr_un &remoteAddr);
        const sockaddr_un &     getRemoteAddr() const;

        bool                    isAssociatedWithIoService();
        void                    associateWithIoService(AsioIoService & ioService);

        std::string             getPipeName() const;

    private:

        void                    setupSocket();
        void                    setupSocket(Exception & e);

        sockaddr_un             mRemoteAddr;
        const std::string       mFileName;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_UNIXLOCALCLIENTTRANSPORT_HPP
