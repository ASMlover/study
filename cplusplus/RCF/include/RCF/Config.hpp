
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

#ifndef INCLUDE_RCF_CONFIG_HPP
#define INCLUDE_RCF_CONFIG_HPP

#include <boost/config.hpp>
#include <boost/mpl/int.hpp>

#ifdef BOOST_NO_STD_WSTRING
#error RCF requires compiler support for std::wstring.
#endif

#ifdef RCF_USE_BOOST_THREADS
#error RCF_USE_BOOST_THREADS is no longer supported. RCF now uses an internal threading library.
#endif

#ifndef RCF_MAX_METHOD_COUNT
#define RCF_MAX_METHOD_COUNT 100
#endif

// On Cygwin, need to compile against external Boost.Asio.
#if defined(__CYGWIN__) && !defined(RCF_USE_BOOST_ASIO)
#define RCF_USE_BOOST_ASIO
#endif

// C++11 requires us to mark some destructors as throwing. Boost.Config needs to 
// be from a fairly recent version of Boost to detect whether the compiler has 
// noexcept() support.
#if BOOST_VERSION >= 104900

#ifdef BOOST_NO_NOEXCEPT
#define RCF_DTOR_THROWS
#else
#define RCF_DTOR_THROWS noexcept(false)
#endif

#else

#define RCF_DTOR_THROWS

#endif

//------------------------------------------------------------------------------
// Determine which features to compile.

#ifndef RCF_PRO
#define RCF_PRO 1
#endif

// For a minimum build.
//#define RCF_FEATURE_ZLIB                0
//#define RCF_FEATURE_OPENSSL             0
//#define RCF_FEATURE_IPV6                0
//#define RCF_FEATURE_PROTOBUF            0
//#define RCF_FEATURE_FILETRANSFER        0
//#define RCF_FEATURE_JSON                0
//#define RCF_FEATURE_CUSTOM_ALLOCATOR    0
//#define RCF_FEATURE_SSPI                0
//#define RCF_FEATURE_SERVER              0
//#define RCF_FEATURE_PUBSUB              0
//#define RCF_FEATURE_LEGACY              0
//#define RCF_FEATURE_HTTP                0
//#define RCF_FEATURE_UDP                 0
//#define RCF_FEATURE_NAMEDPIPE           0
//#define RCF_FEATURE_LOCALSOCKET         0
//#define RCF_FEATURE_TCP                 1
//#define RCF_FEATURE_SF                  1
//#define RCF_FEATURE_BOOST_SERIALIZATION 0

// RCF_FEATURE_LOCALSOCKET not supported on Windows platforms.
#if defined(RCF_FEATURE_LOCALSOCKET) && defined(BOOST_WINDOWS)
#undef RCF_FEATURE_LOCALSOCKET
#define RCF_FEATURE_LOCALSOCKET     0
#endif

// RCF_FEATURE_NAMEDPIPE not supported on non-Windows platforms.
#if defined(RCF_FEATURE_NAMEDPIPE) && !defined(BOOST_WINDOWS)
#undef RCF_FEATURE_NAMEDPIPE
#define RCF_FEATURE_NAMEDPIPE       0
#endif

// RCF_FEATURE_SSPI not supported on non-Windows platforms.
#if defined(RCF_FEATURE_SSPI) && !defined(BOOST_WINDOWS)
#undef RCF_FEATURE_SSPI
#define RCF_FEATURE_SSPI            0
#endif


// For any features left undefined, define to default value.

// Zlib feature.
#ifndef RCF_FEATURE_ZLIB
#ifdef RCF_USE_ZLIB
#define RCF_FEATURE_ZLIB            1
#else
#define RCF_FEATURE_ZLIB            0
#endif
#endif

// OpenSSL feature.
#ifndef RCF_FEATURE_OPENSSL
#ifdef RCF_USE_OPENSSL
#define RCF_FEATURE_OPENSSL         1
#else
#define RCF_FEATURE_OPENSSL         0
#endif
#endif

// SSPI feature.
#ifndef RCF_FEATURE_SSPI
#if defined(BOOST_WINDOWS) && RCF_PRO == 1
#define RCF_FEATURE_SSPI                1
#else
#define RCF_FEATURE_SSPI                0
#endif
#endif

// File transfer feature
#ifndef RCF_FEATURE_FILETRANSFER
#ifdef RCF_USE_BOOST_FILESYSTEM
#define RCF_FEATURE_FILETRANSFER    1
#else
#define RCF_FEATURE_FILETRANSFER    0
#endif
#endif

// Server feature
#ifndef RCF_FEATURE_SERVER
#define RCF_FEATURE_SERVER          1
#endif

// Publish/subscribe feature.
#ifndef RCF_FEATURE_PUBSUB
#define RCF_FEATURE_PUBSUB          1
#endif

// Legacy feature
#ifndef RCF_FEATURE_LEGACY
#define RCF_FEATURE_LEGACY          1
#endif

// HTTP/HTTPS feature.
#ifndef RCF_FEATURE_HTTP
#define RCF_FEATURE_HTTP            1
#endif

// UDP feature
#ifndef RCF_FEATURE_UDP
#define RCF_FEATURE_UDP             1
#endif

// Win32 named pipes feature.
#ifndef RCF_FEATURE_NAMEDPIPE
#ifdef BOOST_WINDOWS
#define RCF_FEATURE_NAMEDPIPE           1
#else
#define RCF_FEATURE_NAMEDPIPE           0
#endif
#endif

// Unix local sockets feature.
#ifndef RCF_FEATURE_LOCALSOCKET
#ifdef BOOST_WINDOWS
#define RCF_FEATURE_LOCALSOCKET         0
#else
#define RCF_FEATURE_LOCALSOCKET         1
#endif
#endif

// TCP feature.
#ifndef RCF_FEATURE_TCP
#define RCF_FEATURE_TCP             1
#endif

// JSON feature
#ifndef RCF_FEATURE_JSON
#define RCF_FEATURE_JSON            0
#endif

// IPv6 feature
#ifndef RCF_FEATURE_IPV6
#define RCF_FEATURE_IPV6            1
#endif

// Protocol Buffers feature
#ifndef RCF_FEATURE_PROTOBUF
#ifdef RCF_USE_PROTOBUF
#define RCF_FEATURE_PROTOBUF    1
#else
#define RCF_FEATURE_PROTOBUF    0
#endif
#endif

// Custom allocator feature.
#ifndef RCF_FEATURE_CUSTOM_ALLOCATOR
#ifdef RCF_USE_CUSTOM_ALLOCATOR
#define RCF_FEATURE_CUSTOM_ALLOCATOR    1
#else
#define RCF_FEATURE_CUSTOM_ALLOCATOR    0
#endif
#endif


// RCF_FEATURE_SF / RCF_FEATURE_BOOST_SERIALIZATION.
// For backward compatibility we need to interpret RCF_USE_SF_SERIALIZATION / RCF_USE_BOOST_SERIALIZATION correctly.

#ifndef RCF_FEATURE_SF
#ifdef RCF_USE_SF_SERIALIZATION
#define RCF_FEATURE_SF                      1
#elif !defined(RCF_USE_SF_SERIALIZATION) && defined(RCF_USE_BOOST_SERIALIZATION)
#define RCF_FEATURE_SF                      0
#else
#define RCF_FEATURE_SF                      1
#endif
#endif

#ifndef RCF_FEATURE_BOOST_SERIALIZATION
#ifdef RCF_USE_BOOST_SERIALIZATION
#define RCF_FEATURE_BOOST_SERIALIZATION     1
#else
#define RCF_FEATURE_BOOST_SERIALIZATION     0
#endif
#endif

//------------------------------------------------------------------------------

// Detect TR1 availability.
#ifndef RCF_USE_TR1

    // MSVC
    #if defined(_MSC_VER) && (_MSC_VER >= 1600 || (_MSC_VER == 1500 && _MSC_FULL_VER >= 150030729))
    #define RCF_USE_TR1
    #define RCF_TR1_HEADER(x) <x>
    #endif

    // GCC
    #if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
    #define RCF_USE_TR1
    #define RCF_TR1_HEADER(x) <tr1/x>
    #endif

#endif // RCF_USE_TR1

// Detect hash_map/hash_set availability.
#ifndef RCF_USE_HASH_MAP

    #if (defined(_MSC_VER) && _MSC_VER >= 1310) || (defined(__GNUC__) && __GNUC__ == 3)
        #define RCF_USE_HASH_MAP
        #if defined(_MSC_VER)
            #define RCF_HASH_MAP_HEADER(x) <x>
            #define RCF_HASH_MAP_NS stdext
        #elif defined(__GNUC__)
            #define RCF_HASH_MAP_HEADER(x) <ext/x>
            #define RCF_HASH_MAP_NS __gnu_cxx
        #endif
    #endif

#endif // RCF_USE_HASH_MAP

namespace RCF {

    #ifndef RCF_USE_SF_SERIALIZATION
    typedef boost::mpl::int_<0> RcfConfig_SF;
    #else
    typedef boost::mpl::int_<1> RcfConfig_SF;
    #endif

    #ifndef RCF_USE_BOOST_SERIALIZATION
    typedef boost::mpl::int_<0> RcfConfig_BSer;
    #else
    typedef boost::mpl::int_<1> RcfConfig_BSer;
    #endif

    #ifndef RCF_USE_BOOST_FILESYSTEM
    typedef boost::mpl::int_<0> RcfConfig_BFs;
    #else
    typedef boost::mpl::int_<1> RcfConfig_BFs;
    #endif

    #if RCF_FEATURE_JSON==0
    typedef boost::mpl::int_<0> RcfConfig_Json;
    #else
    typedef boost::mpl::int_<1> RcfConfig_Json;
    #endif

    template<int N1, int N2, int N3, int N4>
    struct RcfConfigurationDetectMismatches
    {
    };

    typedef RcfConfigurationDetectMismatches<
        RcfConfig_SF::value, 
        RcfConfig_BSer::value, 
        RcfConfig_BFs::value, 
        RcfConfig_Json::value> RcfConfigT;

} // namespace RCF

#endif // ! INCLUDE_RCF_CONFIG_HPP
