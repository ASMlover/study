
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

#ifndef INCLUDE_RCF_ASIOHANDLERCACHE_HPP
#define INCLUDE_RCF_ASIOHANDLERCACHE_HPP

#include <RCF/Export.hpp>
#include <RCF/ThreadLibrary.hpp>

#include <boost/shared_ptr.hpp>

#include <vector>

namespace RCF {

    class RCF_EXPORT AsioHandlerCache
    {
    public:
        typedef boost::shared_ptr< std::vector<char> > VecPtr;
        Mutex mHandlerMutex;
        std::vector<VecPtr> mHandlerFreeList;
        std::vector<VecPtr> mHandlerUsedList;

        AsioHandlerCache();
        ~AsioHandlerCache();

        void * allocate(std::size_t size);
        void deallocate(void * pointer, std::size_t size);
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_ASIOHANDLERCACHE_HPP
