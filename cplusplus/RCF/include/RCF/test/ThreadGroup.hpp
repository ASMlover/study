
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

#ifndef INCLUDE_RCF_TEST_THREADGROUP_HPP
#define INCLUDE_RCF_TEST_THREADGROUP_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

#include <RCF/ThreadLibrary.hpp>

typedef RCF::Thread                 Thread;
typedef boost::shared_ptr<Thread>   ThreadPtr;
typedef std::vector<ThreadPtr>      ThreadGroup;

inline void joinThreadGroup(const ThreadGroup &threadGroup)
{
    for (unsigned int i=0; i<threadGroup.size(); ++i)
    {
        threadGroup[i]->join();
    }
}


#endif // ! INCLUDE_RCF_TEST_THREADGROUP_HPP
