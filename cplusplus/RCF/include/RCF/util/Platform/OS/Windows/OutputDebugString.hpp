
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

#ifndef INCLUDE_UTIL_PLATFORM_OS_WINDOWS_OUTPUTDEBUGSTRING_HPP
#define INCLUDE_UTIL_PLATFORM_OS_WINDOWS_OUTPUTDEBUGSTRING_HPP

#include "windows.h"

namespace Platform {

    namespace OS {

        inline void OutputDebugString(const char *sz)
        {
            ::OutputDebugStringA(sz);

#ifdef UTIL_OUTPUTDEBUGSTRING_TO_STDERR
            fprintf(stderr, "%s", sz);
#endif

#ifdef UTIL_OUTPUTDEBUGSTRING_TO_STDOUT
            fprintf(stdout, "%s", sz);
#endif

#ifdef UTIL_OUTPUTDEBUGSTRING_TO_FILE
            static FILE *file = fopen( "OutputDebugString.txt", "w" );
            fprintf(file, "%s", sz);
#endif

        }

    }

}

#endif // ! INCLUDE_UTIL_PLATFORM_OS_WINDOWS_OUTPUTDEBUGSTRING_HPP
