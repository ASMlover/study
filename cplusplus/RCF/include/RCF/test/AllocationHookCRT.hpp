
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

#include <crtdbg.h>
#include <iostream>

#include <RCF/test/Test.hpp>

#include <RCF/CustomAllocator.hpp>

#include <RCF/test/StackWalker.h>

#ifdef NDEBUG
#error CRT allocator hook only possible in debug builds.
#endif

bool gInstrumented = false;
bool gExpectAllocations = true;
std::size_t gnAllocations = 0;

static _CRT_ALLOC_HOOK  pfnOldCrtAllocHook  = NULL;

int crtAllocationHook(
    int allocType, 
    void    *userData, 
    size_t size, 
    int blockType, 
    long    requestNumber, 
    const unsigned char *filename, // Can't be UNICODE
    int lineNumber)
{
    // Check for unexpected memory allocations.
    if (    gInstrumented
        &&  (allocType == _HOOK_ALLOC || allocType == _HOOK_REALLOC)
        && !gExpectAllocations)
    {
        // Only flag the first unexpected allocation, so we don't end up 
        // with thousands of failures.

        gInstrumented = false;

        // If we do want to track further allocations, uncomment this.
        //gInstrumented = true;

        std::cout << "***************************************" << std::endl;
        std::cout << "Unexpected memory allocation. Call stack:" << std::endl;
        
        std::cout << StackTrace().toString() << std::endl;

        std::cout << "***************************************" << std::endl;

        RCF_CHECK(0 && "Unexpected memory allocation.");
    }

    if (allocType == _HOOK_ALLOC || allocType == _HOOK_REALLOC)
    {
        ++gnAllocations;
    }

    return pfnOldCrtAllocHook(
        allocType, 
        userData, 
        size, 
        blockType, 
        requestNumber, 
        filename, 
        lineNumber);
}

void setupHook()
{
    pfnOldCrtAllocHook = _CrtSetAllocHook(crtAllocationHook);
}

bool dummy = (setupHook(), false);
