
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

#include <RCF/TcpServerTransport.hpp>

#include <RCF/Asio.hpp>
#include <RCF/IpAddress.hpp>
#include <RCF/TcpClientTransport.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/TimedBsdSockets.hpp>

namespace RCF {

    IpAddress boostToRcfIpAdress(const ASIO_NS::ip::tcp::endpoint & endpoint)
    {
        ASIO_NS::ip::address asioAddr = endpoint.address();

        IpAddress ipAddress;

        if (asioAddr.is_v4())
        {
            sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(endpoint.port());
            addr.sin_addr.s_addr = htonl(asioAddr.to_v4().to_ulong());
            ipAddress = IpAddress(addr);
        }
#if RCF_FEATURE_IPV6==1
        else if (asioAddr.is_v6())
        {
            RCF_ASSERT(asioAddr.is_v6());

            SockAddrIn6 addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin6_family = AF_INET6;
            addr.sin6_port = htons(endpoint.port());

            ASIO_NS::ip::address_v6 asioAddrV6 = asioAddr.to_v6();
            ASIO_NS::ip::address_v6::bytes_type bytes = asioAddrV6.to_bytes();
            memcpy(addr.sin6_addr.s6_addr, &bytes[0], bytes.size());

            ipAddress = IpAddress(addr);
        }
#endif

        return ipAddress;
    }

    class TcpAcceptor : public AsioAcceptor
    {
    public:
        TcpAcceptor(
            AsioIoService & ioService, 
            ASIO_NS::ip::tcp::acceptor::protocol_type protocolType, 
            int acceptorFd) : 
                mAcceptor(ioService, protocolType, acceptorFd)
        {}

        ASIO_NS::ip::tcp::acceptor mAcceptor;
    };

    // TcpNetworkSession

    TcpNetworkSession::TcpNetworkSession(
        TcpServerTransport &transport,
        AsioIoService & ioService) :
            AsioNetworkSession(transport, ioService),
            mSocketPtr(new AsioSocket(ioService)),
            mWriteCounter(0)
    {
    }

    const RemoteAddress & TcpNetworkSession::implGetRemoteAddress()
    {
        return mIpAddress;
    }

    void TcpNetworkSession::implRead(char * buffer, std::size_t bufferLen)
    {
        if ( !mSocketPtr )
        {
            RCF_LOG_4() << "TcpNetworkSession - connection has been closed.";
            return;
        }

        mWriteCounter = 0;

        RCF_LOG_4()(bufferLen) 
            << "TcpNetworkSession - calling async_read_some().";

        mSocketPtr->async_read_some(
            ASIO_NS::buffer( buffer, bufferLen),
            ReadHandler(sharedFromThis()));
    }

    void TcpNetworkSession::implWrite(const std::vector<ByteBuffer> & buffers)
    {
        if ( !mSocketPtr )
        {
            RCF_LOG_4() << "TcpNetworkSession - connection has been closed.";
            return;
        }

        ++mWriteCounter;

        if (mWriteCounter > 1)
        {
            // Put a breakpoint here to catch write buffer fragmentation.
            mWriteCounter = mWriteCounter;
        }

        RCF_LOG_4()(RCF::lengthByteBuffers(buffers))
            << "TcpNetworkSession - calling async_write_some().";

        mBufs.mVecPtr->resize(0);
        for (std::size_t i=0; i<buffers.size(); ++i)
        {
            ByteBuffer buffer = buffers[i];

            mBufs.mVecPtr->push_back( 
                AsioConstBuffer(buffer.getPtr(), buffer.getLength()) );
        }

        mSocketPtr->async_write_some(
            mBufs,
            WriteHandler(sharedFromThis()));
    }

    void TcpNetworkSession::implWrite(
        AsioNetworkSession &toBeNotified, 
        const char * buffer, 
        std::size_t bufferLen)
    {
        ASIO_NS::async_write(
            *mSocketPtr,
            ASIO_NS::buffer(buffer, bufferLen),
            WriteHandler(toBeNotified.sharedFromThis()));
    }

    void TcpNetworkSession::implAccept()
    {
        RCF_LOG_4()
            << "TcpNetworkSession - calling async_accept().";

        TcpAcceptor & tcpAcceptor = 
            static_cast<TcpAcceptor &>(mTransport.getAcceptor());

        tcpAcceptor.mAcceptor.async_accept(
            *mSocketPtr,
            boost::bind(
                &AsioNetworkSession::onAcceptCompleted,
                sharedFromThis(),
                ASIO_NS::placeholders::error));
    }

    bool TcpNetworkSession::implOnAccept()
    {
        ASIO_NS::ip::tcp::endpoint endpoint = 
            mSocketPtr->remote_endpoint();

        mIpAddress = boostToRcfIpAdress(endpoint);

        TcpServerTransport & transport = 
            static_cast<TcpServerTransport &>(mTransport);

        bool ipAllowed = transport.isIpAllowed(mIpAddress);
        if (!ipAllowed)
        {
            RCF_LOG_2()(mIpAddress.getIp()) 
                << "Client IP does not match server's IP access rules. Closing connection.";
        }

        return ipAllowed;
    }

    void TcpNetworkSession::implClose()
    {
        mSocketPtr.reset();
    }

    void TcpNetworkSession::implCloseAfterWrite()
    {
        int fd = static_cast<int>(mSocketPtr->native());
        const int BufferSize = 8*1024;
        char buffer[BufferSize];
        while (recv(fd, buffer, BufferSize, 0) > 0);
#ifdef BOOST_WINDOWS
        int ret = shutdown(fd, SD_BOTH);
#else
        int ret = shutdown(fd, SHUT_RDWR);
#endif
        RCF_UNUSED_VARIABLE(ret);

        postRead();
    }

    bool TcpNetworkSession::implIsConnected()
    {
        int fd = static_cast<int>(mSocketPtr->native());
        return isFdConnected(fd);
    }

    ClientTransportAutoPtr TcpNetworkSession::implCreateClientTransport()
    {
        std::auto_ptr<TcpClientTransport> tcpClientTransportPtr(
            new TcpClientTransport(mSocketPtr));

        ASIO_NS::ip::tcp::endpoint endpoint = 
            mSocketPtr->remote_endpoint();

        IpAddress ipAddress = boostToRcfIpAdress(endpoint);
        tcpClientTransportPtr->setRemoteAddr(ipAddress);

        return ClientTransportAutoPtr(tcpClientTransportPtr.release());
    }

    void TcpNetworkSession::implTransferNativeFrom(ClientTransport & clientTransport)
    {
        TcpClientTransport *pTcpClientTransport =
            dynamic_cast<TcpClientTransport *>(&clientTransport);

        if (pTcpClientTransport == NULL)
        {
            Exception e("Incompatible client transport.");
            RCF_THROW(e)(typeid(clientTransport));
        }

        TcpClientTransport & tcpClientTransport = *pTcpClientTransport;

        tcpClientTransport.associateWithIoService(mIoService);

        mSocketPtr = tcpClientTransport.releaseTcpSocket();
    }

    int TcpNetworkSession::getNativeHandle()
    {
        return static_cast<int>(mSocketPtr->native());
    }

    // TcpServerTransport

    TcpServerTransport::TcpServerTransport(
        const IpAddress & ipAddress) :
            mIpAddress(ipAddress),
            mAcceptorFd(-1)
    {
    }

    TcpServerTransport::TcpServerTransport(
        const std::string & ip, 
        int port) :
            mIpAddress(ip, port),
            mAcceptorFd(-1)
    {
    }

    TransportType TcpServerTransport::getTransportType()
    {
        return Tt_Tcp;
    }

    ServerTransportPtr TcpServerTransport::clone()
    {
        return ServerTransportPtr(new TcpServerTransport(mIpAddress));
    }

    AsioNetworkSessionPtr TcpServerTransport::implCreateNetworkSession()
    {
        return AsioNetworkSessionPtr( new TcpNetworkSession(*this, getIoService()) );
    }

    int TcpServerTransport::getPort() const
    {
        return mIpAddress.getPort();
    }

    void TcpServerTransport::implOpen()
    {
        // We open the port manually, without asio. Then later, when we know
        // which io_service to use, we attach the socket to a regular tcp::acceptor.

        if (mAcceptorFd != -1)
        {
            // Listening socket has already been opened.
            return;
        }

        RCF_ASSERT_EQ(mAcceptorFd , -1);

        if (mIpAddress.getPort() != -1)
        {
            mIpAddress.resolve();
            mAcceptorFd = mIpAddress.createSocket(SOCK_STREAM, IPPROTO_TCP);

            sockaddr * pSockAddr = NULL;
            Platform::OS::BsdSockets::socklen_t sockAddrSize = 0;
            mIpAddress.getSockAddr(pSockAddr, sockAddrSize);

            int ret = 0;
            int err = 0;

#ifdef BOOST_WINDOWS
            bool runningOnWindows = true;
#else
            bool runningOnWindows = false;
#endif

            // Only set SO_REUSEADDR on non-Windows OS.
            if (!runningOnWindows)
            {
                // Set SO_REUSEADDR socket option.
                int enable = 1;
                ret = setsockopt(mAcceptorFd, SOL_SOCKET, SO_REUSEADDR, (char *) &enable, sizeof(enable));
                err = Platform::OS::BsdSockets::GetLastError();
            
                RCF_VERIFY(
                    ret ==  0,
                    Exception(
                        _RcfError_Socket("setsockopt() with SO_REUSEADDR"),
                        err,
                        RcfSubsystem_Os));
            }

            ret = ::bind(
                mAcceptorFd, 
                pSockAddr, 
                sockAddrSize);

            if (ret < 0)
            {
                err = Platform::OS::BsdSockets::GetLastError();
                if (err == Platform::OS::BsdSockets::ERR_EADDRINUSE)
                {
                    Exception e(_RcfError_PortInUse(mIpAddress.getIp(), mIpAddress.getPort()), err, RcfSubsystem_Os, "bind() failed");
                    RCF_THROW(e)(mAcceptorFd);
                }
                else
                {
                    Exception e(_RcfError_SocketBind(mIpAddress.getIp(), mIpAddress.getPort()), err, RcfSubsystem_Os, "bind() failed");
                    RCF_THROW(e)(mAcceptorFd);
                }
            }

            // listen on listener socket
            ret = listen(mAcceptorFd, 200);

            if (ret < 0)
            {
                err = Platform::OS::BsdSockets::GetLastError();
                Exception e(_RcfError_Socket("listen()"), err, RcfSubsystem_Os);
                RCF_THROW(e);
            }

            // retrieve the port number, if it's generated by the system
            if (mIpAddress.getPort() == 0)
            {
                IpAddress ip(mAcceptorFd, mIpAddress.getType());
                mIpAddress.setPort(ip.getPort());
            }

            RCF_LOG_2() << "TcpServerTransport - listening on port " << mIpAddress.getPort() << ".";
        }
    }

    void TcpServerTransport::onServerStart(RcfServer & server)
    {
        AsioServerTransport::onServerStart(server);

        mpIoService = mTaskEntries[0].getThreadPool().getIoService();

        if (mAcceptorFd != -1)
        {
            ASIO_NS::ip::tcp::acceptor::protocol_type protocolType = 
                ASIO_NS::ip::tcp::v4();

            if (mIpAddress.getType() == IpAddress::V6)
            {
                protocolType = ASIO_NS::ip::tcp::v6();
            }

            mAcceptorPtr.reset(
                new TcpAcceptor(*mpIoService, protocolType, mAcceptorFd));

            mAcceptorFd = -1;

            startAccepting();
        }
    }

    ClientTransportAutoPtr TcpServerTransport::implCreateClientTransport(
        const Endpoint &endpoint)
    {
        const TcpEndpoint &tcpEndpoint = 
            dynamic_cast<const TcpEndpoint &>(endpoint);

        ClientTransportAutoPtr clientTransportAutoPtr(
            new TcpClientTransport(tcpEndpoint.getIpAddress()));

        return clientTransportAutoPtr;
    }

} // namespace RCF
