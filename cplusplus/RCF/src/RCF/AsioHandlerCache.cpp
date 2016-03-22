
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

#include <RCF/AsioHandlerCache.hpp>

#include <RCF/Tools.hpp>

namespace RCF {

    AsioHandlerCache::AsioHandlerCache()
    {
        mHandlerFreeList.reserve(10);
        mHandlerUsedList.reserve(10);
    }

    AsioHandlerCache::~AsioHandlerCache()
    {
    }

    void * AsioHandlerCache::allocate(std::size_t size)
    {
        //return malloc(size);


        VecPtr vecPtr;
        Lock lock(mHandlerMutex);
        if (mHandlerFreeList.empty())
        {
            vecPtr.reset( new std::vector<char>(size) );
        }
        else
        {
            vecPtr = mHandlerFreeList.back();
            mHandlerFreeList.pop_back();
        }

        if (vecPtr->size() != size)
        {
            vecPtr->resize(size);
        }

        mHandlerUsedList.push_back(vecPtr);
        return & (*vecPtr)[0];
    }

    void AsioHandlerCache::deallocate(void * pointer, std::size_t size)
    {
        //free(pointer);
        //return;

        RCF_UNUSED_VARIABLE(size);

        Lock lock(mHandlerMutex);
        for (std::size_t i=0; i<mHandlerUsedList.size(); ++i)
        {
            AsioHandlerCache::VecPtr vecPtr = mHandlerUsedList[i];
            std::vector<char> & vec  = *vecPtr;
            if ( & vec[0]  == pointer )
            {
                mHandlerUsedList.erase( mHandlerUsedList.begin() + i);
                mHandlerFreeList.push_back(vecPtr);
                RCF_ASSERT_EQ(vecPtr->size() , size);
                return;
            }
        }

        RCF_ASSERT(0);
    }

} // namespace RCF

