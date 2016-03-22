
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

#include <iostream>

#include <RCF/test/Test.hpp>

#ifdef _MSC_VER
#include <RCF/test/StackWalker.h>
#else
class StackTrace {
public:
    std::string toString() const { return "<Call stack unavailable>"; }
};
#endif

bool gInstrumented = false;
bool gExpectAllocations = true;
std::size_t gnAllocations = 0;

// User-defined operator new.
void *operator new(size_t bytes)
{
    if (gInstrumented)
    {
        
        if (!gExpectAllocations)
        {
            // Only flag the first unexpected allocation, so we don't end up 
            // with thousands of failures.

            gInstrumented = false;
            
            std::cout << "***************************************" << std::endl;
            std::cout << "Unexpected memory allocation. Call stack:" << std::endl;

            std::cout << StackTrace().toString() << std::endl;

            std::cout << "***************************************" << std::endl;

            RCF_CHECK(0 && "Unexpected memory allocation.");

        }
        ++gnAllocations;
    }
    return malloc(bytes);
}

void operator delete (void *pv) throw()
{
    free(pv);
}

void *operator new [](size_t bytes)
{
    if (gInstrumented)
    {

        if (!gExpectAllocations)
        {
            // Only flag the first unexpected allocation, so we don't end up 
            // with thousands of failures.

            gInstrumented = false;
            RCF_CHECK(gExpectAllocations);
            std::cout << "Unexpected memory allocation." << std::endl;
        }
        ++gnAllocations;
    }
    return malloc(bytes);
}

// User-defined operator delete.
void operator delete [](void *pv) throw()
{
    free(pv);
}
