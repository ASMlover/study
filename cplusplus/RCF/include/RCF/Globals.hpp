
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

#ifndef INCLUDE_RCF_GLOBALS_HPP
#define INCLUDE_RCF_GLOBALS_HPP

#include <string>

#include <RCF/Export.hpp>

namespace RCF {

    class ZlibDll;
    class OpenSslDll;
    class OpenSslCryptoDll;

    class RCF_EXPORT Globals
    {

    public:
        Globals();
        ~Globals();

        void                setZlibDllName(const std::string & dllName);
        std::string         getZlibDllName() const;

        void                setOpenSslDllName(const std::string & dllName);
        std::string         getOpenSslDllName() const;

        void                setOpenSslCryptoDllName(const std::string & dllName);
        std::string         getOpenSslCryptoDllName() const;
        
        ZlibDll &           getZlibDll();
        OpenSslDll &        getOpenSslDll();
        OpenSslCryptoDll &  getOpenSslCryptoDll();

    private:

        void                deleteZlibDll();
        void                deleteOpenSslDll();
        void                deleteOpenSslCryptoDll();

        ZlibDll *           mpZlibDll;
        OpenSslDll *        mpOpenSslDll;
        OpenSslCryptoDll *  mpOpenSslCryptoDll;

        std::string         mZlibDllName;
        std::string         mOpenSslDllName;
        std::string         mOpenSslCryptoDllName;
    };

    RCF_EXPORT Globals & getGlobals();

} // namespace RCF

#endif // ! INCLUDE_RCF_GLOBALS_HPP
