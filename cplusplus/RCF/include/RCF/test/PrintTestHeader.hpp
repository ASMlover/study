
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

#ifndef INCLUDE_RCF_TEST_PRINTTESTHEADER
#define INCLUDE_RCF_TEST_PRINTTESTHEADER

#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include <RCF/Config.hpp>
#include <RCF/test/Test.hpp>
#include <boost/version.hpp>

#include <RCF/Asio.hpp>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 )  // warning C4996: '' was declared deprecated
#endif

inline void printTestHeader(const char *file)
{
    std::cout << "\n*********************\n";

    std::cout << "Compiler: ";

#if defined(_MSC_VER) && _MSC_VER == 1200
    std::cout << "Visual C++ 6";
#elif defined(_MSC_VER) && _MSC_VER == 1310
    std::cout << "Visual C++ 7.1";
#elif defined(_MSC_VER) && _MSC_VER == 1400
    std::cout << "Visual C++ 8.0";
#elif defined(_MSC_VER) && _MSC_VER == 1500
    std::cout << "Visual C++ 9.0";
#elif defined(_MSC_VER) && _MSC_VER == 1600
    std::cout << "Visual C++ 10.0";
#elif defined(_MSC_VER) && _MSC_VER == 1700
    std::cout << "Visual C++ 11.0";
#elif defined(_MSC_VER) 
    std::cout << "Visual C++ <<<version>>> - " << "_MSC_VER is " << _MSC_VER;
#endif

#if defined(__GNUC__)
    std::cout << "gcc " << __GNUC__ << "." << __GNUC_MINOR__;
#endif

#if defined(__BORLANDC__)
    std::cout << "Borland C++ - __BORLANDC__ is " << __BORLANDC__;
#endif

    std::cout << std::endl;
    std::cout << "Architecture (bits): " << 8*sizeof(void*) << std::endl;

#if !defined(NDEBUG) || defined(_DEBUG)
    std::cout << "Debug build" << std::endl;
#else
    std::cout << "Release build" << std::endl;
#endif


    std::cout << "\n*********************\n";
    std::cout << file << std::endl;
    time_t now = time(NULL);
    std::cout << "Time now: " << std::string(ctime(&now));
    std::cout << "Current working directory: " << RCF::getWorkingDir() << std::endl;
    std::cout << "Relative path to test data: " << RCF::getRelativeTestDataPath() << std::endl;

    std::cout << "Defines:" << std::endl;

    std::cout << "BOOST_VERSION: " << BOOST_VERSION << std::endl;

#ifdef RCF_USE_BOOST_ASIO
    std::cout << "BOOST_ASIO_VERSION: " << BOOST_ASIO_VERSION << std::endl;

#if defined(BOOST_ASIO_HAS_IOCP)
    std::cout << "BOOST_ASIO_HAS_IOCP" << std::endl;
#elif defined(BOOST_ASIO_HAS_EPOLL)
    std::cout << "BOOST_ASIO_HAS_EPOLL" << std::endl;
#elif defined(BOOST_ASIO_HAS_KQUEUE)
    std::cout << "BOOST_ASIO_HAS_KQUEUE" << std::endl;
#elif defined(BOOST_ASIO_HAS_DEV_POLL)
    std::cout << "BOOST_ASIO_HAS_DEV_POLL" << std::endl;
#else
    std::cout << "BOOST_ASIO_HAS_*** - using select()" << std::endl;
#endif

#else
    std::cout << "ASIO_VERSION: " << ASIO_VERSION << std::endl;

#if defined(ASIO_HAS_IOCP)
    std::cout << "ASIO_HAS_IOCP" << std::endl;
#elif defined(ASIO_HAS_EPOLL)
    std::cout << "ASIO_HAS_EPOLL" << std::endl;
#elif defined(ASIO_HAS_KQUEUE)
    std::cout << "ASIO_HAS_KQUEUE" << std::endl;
#elif defined(ASIO_HAS_DEV_POLL)
    std::cout << "ASIO_HAS_DEV_POLL" << std::endl;
#else
    std::cout << "ASIO_HAS_*** - using select()" << std::endl;
#endif

#endif
    
    std::cout << "RCF_MAX_METHOD_COUNT: " << RCF_MAX_METHOD_COUNT << std::endl;

    std::cout << "RCF_FEATURE_LEGACY:               : " << RCF_FEATURE_LEGACY << std::endl;

    std::cout << "RCF_FEATURE_SF:                   : " << RCF_FEATURE_SF << std::endl;

    std::cout << "RCF_FEATURE_BOOST_SERIALIZATION   : " << RCF_FEATURE_BOOST_SERIALIZATION << std::endl;

    std::cout << "RCF_FEATURE_ZLIB                  : " << RCF_FEATURE_ZLIB << std::endl;

    std::cout << "RCF_FEATURE_OPENSSL               : " << RCF_FEATURE_OPENSSL << std::endl;

    std::cout << "RCF_FEATURE_FILETRANSFER          : " << RCF_FEATURE_FILETRANSFER << std::endl;

    std::cout << "RCF_FEATURE_JSON                  : " << RCF_FEATURE_JSON << std::endl;

    std::cout << "RCF_FEATURE_IPV6                  : " << RCF_FEATURE_IPV6 << std::endl;

    std::cout << "RCF_FEATURE_PROTOBUF              : " << RCF_FEATURE_PROTOBUF << std::endl;

    std::cout << "RCF_FEATURE_CUSTOM_ALLOCATOR      : " << RCF_FEATURE_CUSTOM_ALLOCATOR << std::endl;

    std::cout << "*********************\n\n";
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif


#endif // ! INCLUDE_RCF_TEST_PRINTTESTHEADER
