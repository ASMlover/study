
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

#include <boost/config.hpp>
#include <boost/version.hpp>

#include <RCF/Config.hpp>

// Problems with BSer. Include valarray early so it doesn't get trampled by min/max macro definitions.
#if defined(_MSC_VER) && _MSC_VER == 1310 && RCF_FEATURE_BOOST_SERIALIZATION==1
#include <valarray>
#endif

// Problems with BSer. Suppress some static warnings.
#if defined(_MSC_VER) && RCF_FEATURE_BOOST_SERIALIZATION==1 && BOOST_VERSION >= 104100
#pragma warning( push )
#pragma warning( disable : 4308 )  // warning C4308: negative integral constant converted to unsigned type
#endif

// VS 2013 Update 3 - a number of WinSock functions have been deprecated.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996) // error C4996: 'WSAAddressToStringA': Use WSAAddressToStringW() instead or define _WINSOCK_DEPRECATED_NO_WARNINGS to disable deprecated API warnings
#endif

#include "AmiThreadPool.cpp"
#include "AsioHandlerCache.cpp"
#include "AsioServerTransport.cpp"
#include "BsdClientTransport.cpp"
#include "ByteBuffer.cpp"
#include "ByteOrdering.cpp"
#include "Certificate.cpp"
#include "CheckRtti.cpp"
#include "ClientStub.cpp"
#include "ClientStubLegacy.cpp"
#include "ClientTransport.cpp"
#include "ConnectedClientTransport.cpp"
#include "CurrentSerializationProtocol.cpp"
#include "CurrentSession.cpp"
#include "CustomAllocator.cpp"
#include "DynamicLib.cpp"
#include "Endpoint.cpp"
#include "Enums.cpp"
#include "Exception.cpp"
#include "Filter.cpp"
#include "FilterService.cpp"
#include "Future.cpp"
#include "Globals.cpp"
#include "InitDeinit.cpp"
#include "IpAddress.cpp"
#include "IpClientTransport.cpp"
#include "IpServerTransport.cpp"
#include "Marshal.cpp"
#include "MemStream.cpp"
#include "MethodInvocation.cpp"
#include "ObjectPool.cpp"
#include "PerformanceData.cpp"
#include "PeriodicTimer.cpp"
#include "RcfClient.cpp"
#include "RcfServer.cpp"
#include "RcfSession.cpp"
#include "RemoteCallContext.cpp"
#include "ReallocBuffer.cpp"
#include "SerializationProtocol.cpp"
#include "ServerStub.cpp"
#include "ServerTask.cpp"
#include "ServerTransport.cpp"
#include "Service.cpp"
#include "StubEntry.cpp"
#include "StubFactory.cpp"
#include "ThreadLibrary.cpp"
#include "ThreadLocalData.cpp"
#include "ThreadPool.cpp"
#include "TimedBsdSockets.cpp"
#include "Timer.cpp"
#include "Token.cpp"
#include "Tools.cpp"
#include "UsingBsdSockets.cpp"
#include "Version.cpp"

#include "util/Log.cpp"
#include "util/Platform.cpp"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4244)
#endif

#include <RCF/utf8/convert.cpp>
#include <RCF/utf8/utf8_codecvt_facet.cpp>
#include <RCF/thread/impl/thread_src.cpp>

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#ifdef BOOST_WINDOWS
#include "Win32Username.cpp"
#include "Win32Certificate.cpp"
#endif

#if RCF_FEATURE_TCP==1
#include "TcpServerTransport.cpp"
#include "TcpClientTransport.cpp"
#include "TcpEndpoint.cpp"
#endif


#if RCF_FEATURE_UDP==1
#include "UdpClientTransport.cpp"
#include "UdpEndpoint.cpp"
#include "UdpServerTransport.cpp"
#endif


#if RCF_FEATURE_HTTP==1
#include "HttpEndpoint.cpp"
#include "HttpClientTransport.cpp"
#include "HttpServerTransport.cpp"
#include "HttpsEndpoint.cpp"
#include "HttpsClientTransport.cpp"
#include "HttpsServerTransport.cpp"
#include "HttpFrameFilter.cpp"
#include "HttpConnectFilter.cpp"
#include "HttpSessionFilter.cpp"
#endif


#if RCF_FEATURE_SERVER==1
#include "CallbackConnectionService.cpp"
#include "PingBackService.cpp"
#include "ServerObjectService.cpp"
#include "SessionTimeoutService.cpp"
#endif


#if RCF_FEATURE_PUBSUB==1
#include "MulticastClientTransport.cpp"
#include "PublishingService.cpp"
#include "SubscriptionService.cpp"
#include "SubscriptionServiceLegacy.cpp"
#endif


#if RCF_FEATURE_LEGACY==1
#include "ObjectFactoryService.cpp"
#include "SessionObjectFactoryService.cpp"
#endif

#include <RCF/Asio.hpp> // For RCF_HAS_LOCAL_SOCKETS

#if RCF_FEATURE_LOCALSOCKET==1 && defined(RCF_HAS_LOCAL_SOCKETS)
#include "UnixLocalServerTransport.cpp"
#include "UnixLocalClientTransport.cpp"
#include "UnixLocalEndpoint.cpp"
#elif RCF_FEATURE_LOCALSOCKET==1
namespace RCF {
    class UnixLocalEndpoint
    {
    public:
        UnixLocalEndpoint(const std::string & socketName)
        {
            RCF_UNUSED_VARIABLE(socketName);
            RCF_ASSERT(0 && "UNIX local socket endpoints are not supported on this platform.");
        }
    };
}
#endif


#if RCF_FEATURE_SSPI==1
#include "Schannel.cpp"
#include "SspiFilter.cpp"
#endif


#if RCF_FEATURE_NAMEDPIPE==1
#include "Win32NamedPipeClientTransport.cpp"
#include "Win32NamedPipeEndpoint.cpp"
#include "Win32NamedPipeServerTransport.cpp"
#endif


#if RCF_FEATURE_OPENSSL==1
#include "OpenSslEncryptionFilter.cpp"
#endif


#if RCF_FEATURE_ZLIB==1
#include "ZlibCompressionFilter.cpp"
#endif


#if RCF_FEATURE_SF==1
#include "../SF/SF.cpp"
#else
#include "../SF/Encoding.cpp"
#endif


#if RCF_FEATURE_FILETRANSFER==1
#include "FileIoThreadPool.cpp"
#include "FileTransferService.cpp"
#include "FileStream.cpp"
#endif


#if RCF_FEATURE_JSON==1
#include "JsonRpc.cpp"
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Problems with BSer. Suppress some static warnings.
#if defined(_MSC_VER) && RCF_FEATURE_BOOST_SERIALIZATION==1 && BOOST_VERSION >= 104100
#pragma warning( pop )
#endif
