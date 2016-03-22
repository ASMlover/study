
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

#include <RCF/Globals.hpp>

#include <RCF/DynamicLib.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    Globals * gpGlobals = NULL;

    Globals & getGlobals()
    {
        RCF_ASSERT(gpGlobals);
        return *gpGlobals;
    }

    Globals::Globals() :
        mpZlibDll(NULL),
        mpOpenSslDll(NULL),
        mpOpenSslCryptoDll(NULL)
    {

#ifdef BOOST_WINDOWS

        mZlibDllName            = "zlib.dll";
        mOpenSslDllName         = "ssleay32.dll";
        mOpenSslCryptoDllName   = "libeay32.dll";

#else

        mZlibDllName            = "libz.so";
        mOpenSslDllName         = "libssl.so";
        mOpenSslCryptoDllName   = "libcrypto.so";

#endif
    }

    Globals::~Globals()
    {
        deleteZlibDll();
        deleteOpenSslCryptoDll();
        deleteOpenSslDll();
    }

    void Globals::setZlibDllName(const std::string & dllName)
    {
        mZlibDllName = dllName;
    }

    std::string Globals::getZlibDllName() const
    {
        return mZlibDllName;
    }

    void Globals::setOpenSslDllName(const std::string & dllName)
    {
        mOpenSslDllName = dllName;
    }

    std::string Globals::getOpenSslDllName() const
    {
        return mOpenSslDllName;
    }

    void Globals::setOpenSslCryptoDllName(const std::string & dllName)
    {
        mOpenSslCryptoDllName = dllName;
    }

    std::string Globals::getOpenSslCryptoDllName() const
    {
        return mOpenSslCryptoDllName;
    }

#if RCF_FEATURE_ZLIB==0

    void Globals::deleteZlibDll()
    {
        RCF_ASSERT(!mpZlibDll);
    }

#endif

#if RCF_FEATURE_OPENSSL==0

    void Globals::deleteOpenSslDll()
    {
        RCF_ASSERT(!mpOpenSslDll);
    }

    void Globals::deleteOpenSslCryptoDll()
    {
        RCF_ASSERT(!mpOpenSslCryptoDll);
    }

#endif

} // namespace RCF
