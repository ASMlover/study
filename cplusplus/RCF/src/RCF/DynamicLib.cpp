
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

#include <RCF/DynamicLib.hpp>

#include <RCF/Exception.hpp>

#ifndef BOOST_WINDOWS
#include <dlfcn.h>
#endif

namespace RCF {

#ifdef BOOST_WINDOWS

    DynamicLib::DynamicLib(const std::string & dllName) : 
        mDllName(dllName),
        mhDll(NULL)
    {
        mhDll = LoadLibraryA(dllName.c_str());
        if (mhDll == NULL)
        {
            DWORD dwErr = GetLastError();
            Exception e(_RcfError_DllLoad(dllName), dwErr);
            throw e;
        }
    }

    DynamicLib::~DynamicLib()
    {
        if (mhDll)
        {
            FreeLibrary(mhDll);
            mhDll = NULL;
        }
    }

#else

    DynamicLib::DynamicLib(const std::string & dllName) : 
        mDllName(dllName),
        mhDll(NULL)
    {
        mhDll = dlopen(dllName.c_str(), RTLD_NOW);
        if (mhDll == NULL)
        {
            std::string strErr = dlerror();
            Exception e(_RcfError_UnixDllLoad(dllName, strErr));
            throw e;
        }
    }

    DynamicLib::~DynamicLib()
    {
        if (mhDll)
        {
            dlclose(mhDll);
            mhDll = NULL;
        }
    }

#endif

} // namespace RCF
