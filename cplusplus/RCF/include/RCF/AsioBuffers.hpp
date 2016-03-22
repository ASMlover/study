
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

#ifndef INCLUDE_RCF_ASIOBUFFERS_HPP
#define INCLUDE_RCF_ASIOBUFFERS_HPP

#include <RCF/Asio.hpp>

namespace RCF {

    // This adapter around a std::vector prevents asio from making a deep copy
    // of the buffer list, when sending multiple buffers. The deep copy would
    // involve making memory allocations.
    class AsioBuffers
    {
    public:

        typedef std::vector<AsioConstBuffer>            BufferVec;
        typedef boost::shared_ptr<BufferVec>            BufferVecPtr;

        typedef AsioConstBuffer                         value_type;
        typedef BufferVec::const_iterator               const_iterator;

        AsioBuffers()
        {
            mVecPtr.reset( new std::vector<AsioConstBuffer>() );
        }

        const_iterator begin() const
        {
            return mVecPtr->begin();
        }

        const_iterator end() const
        {
            return mVecPtr->end();
        }

        BufferVecPtr mVecPtr;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_ASIOBUFFERS_HPP
