
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

#ifndef INCLUDE_RCF_EXPORT_HPP
#define INCLUDE_RCF_EXPORT_HPP

#if defined(__GNUC__) && __GNUC__ >= 4 
    #ifdef RCF_BUILD_DLL
        #define RCF_EXPORT __attribute__((visibility("default")))
    #else
        #define RCF_EXPORT __attribute__((visibility("default")))
    #endif
#elif defined(__GNUC__)
    #ifdef RCF_BUILD_DLL
        #define RCF_EXPORT
    #else
        #define RCF_EXPORT
    #endif
#else
    #ifdef RCF_BUILD_DLL
        #define RCF_EXPORT __declspec(dllexport)
    #else
        #define RCF_EXPORT
    #endif
#endif

#if defined(RCF_BUILD_DLL) && defined(_MSC_VER) && !defined(_DLL)
#error "Error: DLL builds of RCF require dynamic runtime linking. Select one of the DLL options in Properties -> C/C++ -> Code Generation -> Runtime Library."
#endif

#endif // ! INCLUDE_RCF_EXPORT_HPP
