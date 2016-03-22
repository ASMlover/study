
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

#ifndef INCLUDE_RCF_ASIO_HPP
#define INCLUDE_RCF_ASIO_HPP

// VS 2013 Update 3 - a number of WinSock functions have been deprecated.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996) // error C4996: 'WSAAddressToStringA': Use WSAAddressToStringW() instead or define _WINSOCK_DEPRECATED_NO_WARNINGS to disable deprecated API warnings
#endif

#include <RCF/Config.hpp>

// Turn off auto-linking for Boost Date Time lib. Asio headers include some boost/date_time/ headers.
#define BOOST_DATE_TIME_NO_LIB
#define BOOST_REGEX_NO_LIB

#include <RCF/AsioFwd.hpp>

// Some issues with asio headers.
//#if defined(__MACH__) && defined(__APPLE__)
//#include <limits.h>
//#ifndef IOV_MAX
//#define IOV_MAX 1024
//#endif
//#endif

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
# if defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
#  error WinSock.h has already been included. Define WIN32_LEAN_AND_MEAN in your build, to avoid implicit inclusion of WinSock.h.
# endif // defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4913) // user defined binary operator ',' exists but no overload could convert all operands, default built-in binary operator ',' used
#endif

#ifdef RCF_USE_BOOST_ASIO
#include <boost/asio.hpp>
#else
#include <RCF/external/asio/asio.hpp>
#endif

// Do we have local sockets?
#ifdef RCF_USE_BOOST_ASIO
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
#define RCF_HAS_LOCAL_SOCKETS
#endif
#else
#ifdef ASIO_HAS_LOCAL_SOCKETS
#define RCF_HAS_LOCAL_SOCKETS
#endif
#endif

#include <RCF/AsioDeadlineTimer.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace RCF {


    typedef ASIO_NS::ip::tcp::socket                    AsioSocket;
    typedef boost::shared_ptr<AsioSocket>               AsioSocketPtr;

    typedef ASIO_NS::const_buffer                       AsioConstBuffer;

    typedef AsioSocket                              TcpSocket;
    typedef boost::shared_ptr<TcpSocket>            TcpSocketPtr;

#ifdef RCF_HAS_LOCAL_SOCKETS

    using ASIO_NS::local::stream_protocol;
    typedef stream_protocol::socket                 UnixLocalSocket;
    typedef boost::shared_ptr<UnixLocalSocket>      UnixLocalSocketPtr;

#else

    typedef TcpSocket                               UnixLocalSocket;
    typedef TcpSocketPtr                            UnixLocalSocketPtr;

#endif

} // namespace RCF

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // ! INCLUDE_RCF_ASIO_HPP
