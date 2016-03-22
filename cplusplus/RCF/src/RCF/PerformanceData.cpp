
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

#include <RCF/PerformanceData.hpp>

#include <RCF/InitDeinit.hpp>
#include <RCF/ObjectPool.hpp>

#include <numeric>

namespace RCF {

    PerformanceData *gpPerformanceData = NULL;

    void initPerformanceData()
    {
        gpPerformanceData = new PerformanceData();
    }

    void deinitPerformanceData()
    {
        delete gpPerformanceData;
        gpPerformanceData = NULL;
    }

    PerformanceData & getPerformanceData()
    {
        return *gpPerformanceData;
    }

    void PerformanceData::collect()
    {
        getObjectPool().enumerateReadBuffers(mInBufferSizes);
        getObjectPool().enumerateWriteBuffers(mOutBufferSizes);

        std::size_t inBufferSize = 
            std::accumulate(mInBufferSizes.begin(), mInBufferSizes.end(), std::size_t(0));

        std::size_t outBufferSize = 
            std::accumulate(mOutBufferSizes.begin(), mOutBufferSizes.end(), std::size_t(0));

        Lock lock(mMutex);
        
        mBufferCount = static_cast<boost::uint32_t>(
            mInBufferSizes.size() + mOutBufferSizes.size());

        mTotalBufferSize = static_cast<boost::uint32_t>(
            inBufferSize + outBufferSize);
    }

} // namespace RCF
