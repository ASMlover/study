
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

#include "TransportFactories.hpp"

#include <iostream>
#include <typeinfo>
#include <utility>
#include <vector>

#include <sys/stat.h>

#include <boost/config.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/version.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/ThreadLibrary.hpp>

#include <RCF/TcpServerTransport.hpp>
#include <RCF/Asio.hpp>
#include <RCF/Config.hpp>

#ifdef RCF_HAS_LOCAL_SOCKETS
#include <RCF/UnixLocalClientTransport.hpp>
#include <RCF/UnixLocalServerTransport.hpp>
#endif

#if RCF_FEATURE_NAMEDPIPE==1
#include <RCF/Win32NamedPipeClientTransport.hpp>
#include <RCF/Win32NamedPipeEndpoint.hpp>
#include <RCF/Win32NamedPipeServerTransport.hpp>
#endif

#if RCF_FEATURE_HTTP==1
#include <RCF/HttpClientTransport.hpp>
#include <RCF/HttpServerTransport.hpp>
#include <RCF/HttpEndpoint.hpp>

#include <RCF/HttpsClientTransport.hpp>
#include <RCF/HttpsServerTransport.hpp>
#include <RCF/HttpsEndpoint.hpp>
#endif

#include <RCF/TcpClientTransport.hpp>
#include <RCF/UdpClientTransport.hpp>
#include <RCF/UdpServerTransport.hpp>

#include <RCF/ObjectFactoryService.hpp>

namespace RCF {

    TransportFactories &getTransportFactories()
    {
        static TransportFactories transportFactories;
        return transportFactories;
    }

    TransportFactories &getIpTransportFactories()
    {
        static TransportFactories ipTransportFactories;
        return ipTransportFactories;
    }

    //**************************************************
    // transport factories

    static std::string loopBackV4 = "127.0.0.1";
    static std::string loopBackV6 = "::1";

#if RCF_FEATURE_NAMEDPIPE==1

    TransportPair Win32NamedPipeTransportFactory::createTransports()
    {
        typedef boost::shared_ptr<Win32NamedPipeServerTransport> Win32NamedPipeServerTransportPtr;
        Win32NamedPipeServerTransportPtr serverTransportPtr(
            new Win32NamedPipeServerTransport(RCF_T("")));

        tstring pipeName = serverTransportPtr->getPipeName();

        ClientTransportAutoPtrPtr clientTransportAutoPtrPtr(
            new ClientTransportAutoPtr(
                new Win32NamedPipeClientTransport(pipeName)));

        return std::make_pair(
            ServerTransportPtr(serverTransportPtr), 
            clientTransportAutoPtrPtr);

    }

    TransportPair Win32NamedPipeTransportFactory::createNonListeningTransports()
    {
        return std::make_pair(
            ServerTransportPtr( new Win32NamedPipeServerTransport( RCF_T("")) ),
            ClientTransportAutoPtrPtr());

    }

    bool Win32NamedPipeTransportFactory::isConnectionOriented()
    {
        return true;
    }

    bool Win32NamedPipeTransportFactory::supportsTransportFilters()
    {
        return true;
    }

    std::string Win32NamedPipeTransportFactory::desc()
    {
        return "Win32NamedPipeTransportFactory";
    }

#endif

#if RCF_FEATURE_TCP==1
       
    TcpTransportFactory::TcpTransportFactory(IpAddress::Type type)
    {
        switch (type)
        {
        case IpAddress::V4: mLoopback = loopBackV4; break;
        case IpAddress::V6: mLoopback = loopBackV6; break;
        default: RCF_ASSERT(0);
        }
    }

    TransportPair TcpTransportFactory::createTransports()
    {
        typedef boost::shared_ptr<TcpServerTransport> TcpServerTransportPtr;
        TcpServerTransportPtr tcpServerTransportPtr(
            new TcpServerTransport( IpAddress(mLoopback, 0)));

        tcpServerTransportPtr->open();
        int port = tcpServerTransportPtr->getPort();

        ClientTransportAutoPtrPtr clientTransportAutoPtrPtr(
            new ClientTransportAutoPtr(
                new TcpClientTransport( IpAddress(mLoopback, port))));

        return std::make_pair(
            ServerTransportPtr(tcpServerTransportPtr), 
            clientTransportAutoPtrPtr);
    }

    TransportPair TcpTransportFactory::createNonListeningTransports()
    {
        return std::make_pair(
            ServerTransportPtr( new TcpServerTransport( IpAddress(mLoopback, 0)) ),
            ClientTransportAutoPtrPtr());
    }

    bool TcpTransportFactory::isConnectionOriented()
    {
        return true;
    }

    bool TcpTransportFactory::supportsTransportFilters()
    {
        return true;
    }

    std::string TcpTransportFactory::desc()
    {
        return "TcpTransportFactory (" + mLoopback + ")";
    }

#endif

#if RCF_FEATURE_HTTP==1

    HttpTransportFactory::HttpTransportFactory()
    {
        mLoopback = loopBackV4;
    }

    TransportPair HttpTransportFactory::createTransports()
    {
        typedef boost::shared_ptr<HttpServerTransport> HttpServerTransportPtr;
        HttpServerTransportPtr serverTransportPtr(
            new HttpServerTransport( HttpEndpoint(mLoopback, 0)));

        serverTransportPtr->open();
        int port = serverTransportPtr->getPort();

        ClientTransportAutoPtrPtr clientTransportAutoPtrPtr( new ClientTransportAutoPtr(
                new HttpClientTransport(HttpEndpoint(mLoopback, port))));

        return std::make_pair(
            ServerTransportPtr(serverTransportPtr),
            clientTransportAutoPtrPtr);
    }

    TransportPair HttpTransportFactory::createNonListeningTransports()
    {
        return std::make_pair(
            ServerTransportPtr(new HttpServerTransport( HttpEndpoint(mLoopback, 0))),
            ClientTransportAutoPtrPtr());
    }

    bool HttpTransportFactory::isConnectionOriented()
    {
        return true;
    }

    bool HttpTransportFactory::supportsTransportFilters()
    {
        return true;
    }

    std::string HttpTransportFactory::desc()
    {
        return "HttpTransportFactory (" + mLoopback + ")";
    }

    HttpsTransportFactory::HttpsTransportFactory()
    {
        mLoopback = loopBackV4;
    }

    TransportPair HttpsTransportFactory::createTransports()
    {
        typedef boost::shared_ptr<HttpServerTransport> HttpServerTransportPtr;
        HttpServerTransportPtr serverTransportPtr(
            new HttpServerTransport(HttpEndpoint(mLoopback, 0)));

        serverTransportPtr->open();
        int port = serverTransportPtr->getPort();

        ClientTransportAutoPtrPtr clientTransportAutoPtrPtr(new ClientTransportAutoPtr(
            new HttpClientTransport(HttpEndpoint(mLoopback, port))));

        return std::make_pair(
            ServerTransportPtr(serverTransportPtr),
            clientTransportAutoPtrPtr);
    }

    TransportPair HttpsTransportFactory::createNonListeningTransports()
    {
        return std::make_pair(
            ServerTransportPtr(new HttpServerTransport(HttpEndpoint(mLoopback, 0))),
            ClientTransportAutoPtrPtr());
    }

    bool HttpsTransportFactory::isConnectionOriented()
    {
        return true;
    }

    bool HttpsTransportFactory::supportsTransportFilters()
    {
        return true;
    }

    std::string HttpsTransportFactory::desc()
    {
        return "HttpsTransportFactory (" + mLoopback + ")";
    }

#endif

#if RCF_FEATURE_LOCALSOCKET==1

    UnixLocalTransportFactory::UnixLocalTransportFactory() : mIndex(0)
    {
    }

    TransportPair UnixLocalTransportFactory::createTransports()
    {
        std::string pipeName = generateNewPipeName();

        RCF_LOG_2()(pipeName) << "Creating unix local socket transport pair";

        return std::make_pair(
            ServerTransportPtr( new UnixLocalServerTransport(pipeName) ),
            ClientTransportAutoPtrPtr(
                new ClientTransportAutoPtr(
                    new UnixLocalClientTransport(pipeName))));
    }

    TransportPair UnixLocalTransportFactory::createNonListeningTransports()
    {
        return std::make_pair(
            ServerTransportPtr( new UnixLocalServerTransport("") ),
            ClientTransportAutoPtrPtr());
    }

    bool UnixLocalTransportFactory::isConnectionOriented()
    {
        return true;
    }

    bool UnixLocalTransportFactory::supportsTransportFilters()
    {
        return true;
    }

    bool UnixLocalTransportFactory::fileExists(const std::string & path)
    {
        struct stat stFileInfo = {};
        int ret = stat(path.c_str(), &stFileInfo);
        return ret == 0;
    }

    std::string UnixLocalTransportFactory::generateNewPipeName()
    {
        std::string tempDir = RCF::getRelativeTestDataPath();

        std::string candidate;

        while (candidate.empty() || fileExists(candidate))
        {
            std::ostringstream os;
            os 
                << tempDir 
                << "TestPipe_" 
                << ++mIndex;

            candidate = os.str();
        }

        return candidate;
    }

    std::string UnixLocalTransportFactory::desc()
    {
        return "UnixLocalTransportFactory";
    }

    int mIndex;

#endif // RCF_HAS_LOCAL_SOCKETS

#if RCF_FEATURE_UDP==1

    UdpTransportFactory::UdpTransportFactory(IpAddress::Type type)
    {
        switch (type)
        {
        case IpAddress::V4: mLoopback = loopBackV4; break;
        case IpAddress::V6: mLoopback = loopBackV6; break;
        default: RCF_ASSERT(0);
        }
    }

    TransportPair UdpTransportFactory::createTransports()
    {
        typedef boost::shared_ptr<UdpServerTransport> UdpServerTransportPtr;
        UdpServerTransportPtr udpServerTransportPtr(
            new UdpServerTransport( IpAddress(mLoopback, 0) ));

        udpServerTransportPtr->open();
        int port = udpServerTransportPtr->getPort();

        ClientTransportAutoPtrPtr clientTransportAutoPtrPtr(
            new ClientTransportAutoPtr(
                new UdpClientTransport( IpAddress(mLoopback, port) )));

        return std::make_pair(
            ServerTransportPtr(udpServerTransportPtr), 
            clientTransportAutoPtrPtr);
    }

    TransportPair UdpTransportFactory::createNonListeningTransports()
    {
        return std::make_pair(
            ServerTransportPtr( new UdpServerTransport( IpAddress(mLoopback, 0) ) ),
            ClientTransportAutoPtrPtr());
    }

    bool UdpTransportFactory::isConnectionOriented()
    {
        return false;
    }

    bool UdpTransportFactory::supportsTransportFilters()
    {
        return false;
    }

    std::string UdpTransportFactory::desc()
    {
        return "UdpTransportFactory (" + mLoopback + ")";
    }

#endif

    void initializeTransportFactories()
    {

#if RCF_FEATURE_IPV6==1
        const bool compileTimeIpv6 = true;
        ExceptionPtr ePtr;
        IpAddress("::1").resolve(ePtr);
        const bool runTimeIpv6 = (ePtr.get() == NULL);
#else
        const bool compileTimeIpv6 = false;
        const bool runTimeIpv6 = false;
#endif

#if RCF_FEATURE_NAMEDPIPE==1

        getTransportFactories().push_back(
            TransportFactoryPtr( new Win32NamedPipeTransportFactory()));

#endif

#if RCF_FEATURE_TCP==1

        getTransportFactories().push_back(
            TransportFactoryPtr( new TcpTransportFactory(IpAddress::V4)));

        getIpTransportFactories().push_back(
            TransportFactoryPtr( new TcpTransportFactory(IpAddress::V4)));

        if (compileTimeIpv6 && runTimeIpv6)
        {
            getTransportFactories().push_back(
                TransportFactoryPtr( new TcpTransportFactory(IpAddress::V6)));

            getIpTransportFactories().push_back(
                TransportFactoryPtr( new TcpTransportFactory(IpAddress::V6)));
        }

#endif

#if RCF_FEATURE_HTTP==1

        getTransportFactories().push_back(
            TransportFactoryPtr(new HttpTransportFactory()));

        getIpTransportFactories().push_back(
            TransportFactoryPtr(new HttpTransportFactory()));

        getTransportFactories().push_back(
            TransportFactoryPtr(new HttpsTransportFactory()));

        getIpTransportFactories().push_back(
            TransportFactoryPtr(new HttpsTransportFactory()));

#endif

#if RCF_FEATURE_LOCALSOCKET==1

        getTransportFactories().push_back(
            TransportFactoryPtr( new UnixLocalTransportFactory()));

#endif

#if RCF_FEATURE_UDP==1

        getTransportFactories().push_back(
            TransportFactoryPtr( new UdpTransportFactory(IpAddress::V4)));

        getIpTransportFactories().push_back(
            TransportFactoryPtr( new UdpTransportFactory(IpAddress::V4)));

        if (compileTimeIpv6 && runTimeIpv6)
        {
            getTransportFactories().push_back(
                TransportFactoryPtr( new UdpTransportFactory(IpAddress::V6)));

            getIpTransportFactories().push_back(
                TransportFactoryPtr( new UdpTransportFactory(IpAddress::V6)));
        }

#endif

    }
    
} // namespace RCF
