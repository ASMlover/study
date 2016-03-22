
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

#include <RCF/UdpClientTransport.hpp>
#include <RCF/UdpEndpoint.hpp>

#include <boost/static_assert.hpp>

#include <RCF/Tools.hpp>

#include <RCF/util/Platform/OS/BsdSockets.hpp>

// missing stuff in mingw headers
#ifdef __MINGW32__
#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET           _WSAIOW(IOC_VENDOR,12)
#endif
#endif

namespace RCF {


    UdpClientTransport::UdpClientTransport(const IpAddress & ipAddress) :
        mDestIp(ipAddress),
        mAsync(),
        mSock(-1)
    {
    }

    UdpClientTransport::UdpClientTransport(const UdpClientTransport &rhs) :
        ClientTransport(rhs),
        mDestIp(rhs.mDestIp),
        mAsync(),
        mSock(-1)
    {
    }

    UdpClientTransport::~UdpClientTransport()
    {
        RCF_DTOR_BEGIN
            close();
        RCF_DTOR_END
    }

    TransportType UdpClientTransport::getTransportType()
    {
        return Tt_Udp;
    }

    ClientTransportAutoPtr UdpClientTransport::clone() const
    {
        return ClientTransportAutoPtr( new UdpClientTransport(*this));
    }

    EndpointPtr UdpClientTransport::getEndpointPtr() const
    {
        return EndpointPtr( new UdpEndpoint(mDestIp) );
    }

    void UdpClientTransport::setTimer(
        boost::uint32_t timeoutMs,
        ClientTransportCallback *pClientStub)
    {
        RCF_UNUSED_VARIABLE(timeoutMs);
        RCF_UNUSED_VARIABLE(pClientStub);
    }

    void UdpClientTransport::connect(
        ClientTransportCallback &clientStub, 
        unsigned int timeoutMs)
    {
        RCF_LOG_4()(mSock)(mDestIp.string()) << "UdpClientTransport - creating socket.";

        RCF_UNUSED_VARIABLE(timeoutMs);

        RCF_ASSERT(!mAsync);

        // TODO: replace throw with return value
        if (mSock == -1)
        {
            int ret = 0;
            int err = 0;

            // remote address
            mDestIp.resolve();

            // local address
            if (mLocalIp.empty())
            {
                std::string localIp = mDestIp.getType() == IpAddress::V4 ?
                    "0.0.0.0" :
                    "::0" ;

                mSrcIp = IpAddress(localIp, 0);             
            }
            else
            {
                mSrcIp = mLocalIp;
            }

            mSrcIp.resolve();
            
            mSock = mSrcIp.createSocket(SOCK_DGRAM, IPPROTO_UDP);

            sockaddr * pSrcAddr = NULL;
            Platform::OS::BsdSockets::socklen_t srcAddrSize = 0;
            mSrcIp.getSockAddr(pSrcAddr, srcAddrSize);

            ret = ::bind(mSock, pSrcAddr, srcAddrSize);
            err = Platform::OS::BsdSockets::GetLastError();

            RCF_VERIFY(
                ret == 0,
                Exception(
                    _RcfError_Socket("bind()"), err, RcfSubsystem_Os));

            mAssignedLocalIp = IpAddress(mSock, mSrcIp.getType());

#if defined(BOOST_WINDOWS) && defined(SIO_UDP_CONNRESET)

            // On Windows XP and later, disable the SIO_UDP_CONNRESET socket option.
            BOOL enable = FALSE;
            DWORD dwBytesRet = 0;
            DWORD dwStatus = WSAIoctl(mSock, SIO_UDP_CONNRESET, &enable, sizeof(enable), NULL, 0, &dwBytesRet, NULL, NULL);
            err = Platform::OS::BsdSockets::GetLastError();

            RCF_VERIFY(
                dwStatus == 0,
                Exception(
                    _RcfError_Socket("WSAIoctl() with SIO_UDP_CONNRESET"),
                    err,
                    RcfSubsystem_Os));

#endif // BOOST_WINDOWS

            if (mDestIp.isBroadcast())
            {
                // set socket option to allow transmission of broadcast messages
                int nEnable = 1;
                ret = setsockopt(mSock, SOL_SOCKET, SO_BROADCAST, (char *)&nEnable, sizeof(nEnable));
                err = Platform::OS::BsdSockets::GetLastError();

                RCF_VERIFY(
                    ret ==  0,
                    Exception(
                        _RcfError_Socket("setsockopt() with SO_BROADCAST"),
                        err,
                        RcfSubsystem_Os));
            }

            if (mDestIp.isMulticast())
            {
                // char for Solaris, int for everyone else.
#if defined(__SVR4) && defined(__sun)
                char hops = 16;
#else
                int hops = 16;
#endif
                ret = setsockopt(mSock, IPPROTO_IP, IP_MULTICAST_TTL, (char *) &hops, sizeof (hops));
                err = Platform::OS::BsdSockets::GetLastError();

                RCF_VERIFY(
                    ret ==  0,
                    Exception(
                        _RcfError_Socket("setsockopt() with IPPROTO_IP/IP_MULTICAST_TTL"),
                        err,
                        RcfSubsystem_Os))(hops);
            }
        }

        clientStub.onConnectCompleted();
    }

    int UdpClientTransport::send(
        ClientTransportCallback &clientStub, 
        const std::vector<ByteBuffer> &data,
        unsigned int timeoutMs)
    {
        RCF_LOG_4()(mSock)(mDestIp.string()) << "UdpClientTransport - sending data on socket.";

        RCF_UNUSED_VARIABLE(timeoutMs);

        RCF_ASSERT(!mAsync);

        // TODO: optimize for case of single byte buffer with left margin

        if (mWriteVecPtr.get() == NULL || !mWriteVecPtr.unique())
        {
            mWriteVecPtr.reset( new ReallocBuffer());
        }

        mLastRequestSize = lengthByteBuffers(data);
        mRunningTotalBytesSent += lengthByteBuffers(data);

        ReallocBuffer &buffer = *mWriteVecPtr;
        buffer.resize(lengthByteBuffers(data));

        copyByteBuffers(data, &buffer[0]);

        sockaddr * pDestAddr = NULL;
        Platform::OS::BsdSockets::socklen_t destAddrSize = 0;
        mDestIp.getSockAddr(pDestAddr, destAddrSize);

        int len = sendto(
            mSock,
            &buffer[0],
            static_cast<int>(buffer.size()),
            0,
            pDestAddr, 
            destAddrSize);

        int err = Platform::OS::BsdSockets::GetLastError();
        RCF_VERIFY(
            len > 0,
            Exception(
                _RcfError_Socket("sendto()"),
                err,
                RcfSubsystem_Os));

        clientStub.onSendCompleted();

        return 1;
    }

    int UdpClientTransport::receive(
        ClientTransportCallback &clientStub, 
        ByteBuffer &byteBuffer,
        unsigned int totalTimeoutMs)
    {
        // try to receive a UDP message from server, within the current timeout
        RCF_LOG_4()(mSock)(mDestIp.string())(totalTimeoutMs) << "UdpClientTransport - receiving data from socket.";

        RCF_ASSERT(!mAsync);

        unsigned int startTimeMs = getCurrentTimeMs();
        unsigned int endTimeMs = startTimeMs + totalTimeoutMs;

        while (true)
        {
            unsigned int timeoutMs = generateTimeoutMs(endTimeMs);
            fd_set fdSet;
            FD_ZERO(&fdSet);
            FD_SET( static_cast<SOCKET>(mSock), &fdSet);
            timeval timeout;
            timeout.tv_sec = timeoutMs/1000;
            timeout.tv_usec = 1000*(timeoutMs%1000);

            int ret = Platform::OS::BsdSockets::select(
                mSock+1,
                &fdSet,
                NULL,
                NULL,
                &timeout);

            int err = Platform::OS::BsdSockets::GetLastError();

            RCF_ASSERT(-1 <= ret && ret <= 1)(ret);
            if (ret == -1)
            {
                Exception e(
                    _RcfError_Socket("select()"),
                    err,
                    RcfSubsystem_Os);

                RCF_THROW(e);
            }   
            else if (ret == 0)
            {
                Exception e( _RcfError_ClientReadTimeout() );
                RCF_THROW(e);
            }
            RCF_ASSERT_EQ(ret , 1);

            if (mReadVecPtr.get() == NULL || !mReadVecPtr.unique())
            {
                mReadVecPtr.reset( new ReallocBuffer());
            }

            // TODO: optimize
            ReallocBuffer &buffer = *mReadVecPtr;
            buffer.resize(4);

            SockAddrStorage fromAddr;
            memset(&fromAddr, 0, sizeof(fromAddr));
            int fromAddrLen = sizeof(fromAddr);

            sockaddr * pDestAddr = NULL;
            Platform::OS::BsdSockets::socklen_t destAddrSize = 0;
            mDestIp.getSockAddr(pDestAddr, destAddrSize);

            int len = Platform::OS::BsdSockets::recvfrom(
                mSock,
                &buffer[0],
                4,
                MSG_PEEK,
                (sockaddr *) &fromAddr,
                &fromAddrLen);

            err = Platform::OS::BsdSockets::GetLastError();

            if (    len == 4 
                ||  (len == -1 && err == Platform::OS::BsdSockets::ERR_EMSGSIZE))
            {
                mFromIp.init( (sockaddr&) fromAddr, fromAddrLen, mDestIp.getType());
                if (mDestIp.matches(mFromIp))
                {
                    boost::uint32_t dataLength = 0;
                    memcpy( &dataLength, &buffer[0], 4);
                    RCF::networkToMachineOrder(&dataLength, 4, 1);

                    if (getMaxMessageLength())
                    {
                        RCF_VERIFY(
                            0 < dataLength && dataLength <= getMaxMessageLength(),
                            Exception(_RcfError_ClientMessageLength()));
                    }

                    buffer.resize(4+dataLength);
                    memset(&fromAddr, 0, sizeof(fromAddr));
                    fromAddrLen = sizeof(fromAddr);

                    len = Platform::OS::BsdSockets::recvfrom(
                        mSock,
                        &buffer[0],
                        dataLength+4,
                        0,
                        (sockaddr *) &fromAddr,
                        &fromAddrLen);

                    if (len == static_cast<int>(dataLength+4))
                    {
                        mLastResponseSize = dataLength+4;
                        mRunningTotalBytesReceived += dataLength+4;

                        byteBuffer = ByteBuffer(
                            &buffer[4],
                            dataLength,
                            4,
                            mReadVecPtr);

                        clientStub.onReceiveCompleted();

                        return 1;
                    }
                }
                else
                {
                    // The packet is not a valid response, but we need to read
                    // it so we can receive more packets.

                    const std::size_t BufferSize = 4096;
                    char Buffer[BufferSize];
                    Platform::OS::BsdSockets::recvfrom(
                        mSock,
                        Buffer,
                        BufferSize,
                        0,
                        NULL,
                        NULL);
                }
            }
            else
            {
                RCF_THROW( Exception( _RcfError_ClientReadFail() ) )(len)(err);
            }
        }
    }

    void UdpClientTransport::close()
    {
        if (mSock != -1)
        {
            int ret = Platform::OS::BsdSockets::closesocket(mSock);
            int err = Platform::OS::BsdSockets::GetLastError();
            if (ret < 0)
            {
                RCF_ASSERT(0)(mSock)(ret)(err);
            }
            mSock = -1;
        }
    }

    bool UdpClientTransport::isConnected()
    {
        return mSock != -1;
    }

    void UdpClientTransport::disconnect(unsigned int timeoutMs)
    {
        RCF_UNUSED_VARIABLE(timeoutMs);
        close();
    }

    void UdpClientTransport::setTransportFilters(
        const std::vector<FilterPtr> &filters)
    {
        if (!filters.empty())
        {
            RCF_ASSERT(0);
        }
    }

    void UdpClientTransport::getTransportFilters(
        std::vector<FilterPtr> &filters)
    {
        RCF_UNUSED_VARIABLE(filters);
    }

    int UdpClientTransport::getNativeHandle() const
    {
        return mSock;
    }

} // namespace RCF
