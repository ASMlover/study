
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

#ifndef INCLUDE_SF_TOOLS_HPP
#define INCLUDE_SF_TOOLS_HPP

#include <RCF/Tools.hpp>

#include <boost/cstdint.hpp>

//****************************************************************************
// Helper macro to generate code for fundamental types

#define SF_FOR_EACH_FUNDAMENTAL_TYPE_(arg)  \
    arg(char)                               \
    arg(int)                                \
    arg(bool)                               \
    arg(float)                              \
    arg(double)                             \
    arg(short)                              \
    arg(long)                               \
    arg(unsigned short)                     \
    arg(signed char)                        \
    arg(unsigned char)                      \
    arg(unsigned int)                       \
    arg(unsigned long)                      \
    arg(long double)                        \
    //arg(wchar_t)

#ifdef _MSC_VER

#define SF_FOR_EACH_FUNDAMENTAL_TYPE(arg)   \
    SF_FOR_EACH_FUNDAMENTAL_TYPE_(arg)      \
    arg(__int64)                            \
    arg(unsigned __int64)

#else

#define SF_FOR_EACH_FUNDAMENTAL_TYPE(arg)   \
    SF_FOR_EACH_FUNDAMENTAL_TYPE_(arg)      \
    arg(long long)                          \
    arg(unsigned long long)

#endif

#endif // ! INCLUDE_SF_TOOLS_HPP

