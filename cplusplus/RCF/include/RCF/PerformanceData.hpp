
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

#ifndef INCLUDE_RCF_PERFORMANCEDATA_HPP
#define INCLUDE_RCF_PERFORMANCEDATA_HPP

#include <RCF/Export.hpp>
#include <RCF/ThreadLibrary.hpp>

#include <boost/cstdint.hpp>
#include <vector>

namespace RCF {

    class RCF_EXPORT PerformanceData
    {
    public:
        PerformanceData() : mRcfSessions(0), mBufferCount(0), mTotalBufferSize(0)
        {
        }

        void collect();

        Mutex           mMutex;
        boost::uint32_t mRcfSessions;
        boost::uint32_t mBufferCount;
        boost::uint32_t mTotalBufferSize;

    private:

        std::vector<std::size_t> mInBufferSizes;
        std::vector<std::size_t> mOutBufferSizes;
    };

    RCF_EXPORT PerformanceData & getPerformanceData();

} // namespace RCF

#endif // ! INCLUDE_RCF_PERFORMANCEDATA_HPP
