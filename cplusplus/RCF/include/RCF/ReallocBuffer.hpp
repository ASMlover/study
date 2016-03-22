
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

#ifndef INCLUDE_RCF_REALLOCBUFFER_HPP
#define INCLUDE_RCF_REALLOCBUFFER_HPP

#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>

namespace RCF {

    class RCF_EXPORT ReallocBuffer
    {
    public:

        ReallocBuffer();
        ReallocBuffer(std::size_t size);
        ~ReallocBuffer();

        void            clear();
        void            resize(std::size_t newSize);
        std::size_t     size();
        std::size_t     capacity();
        bool            empty();

        char *          getPtr();
        char &          operator[](std::size_t pos);
        const char &    operator[](std::size_t pos) const;

    private:

        char *          mpch;
        std::size_t     mSize;
        std::size_t     mCapacity;
    };

    typedef boost::shared_ptr<ReallocBuffer> ReallocBufferPtr;


} // namespace RCF

#endif // ! INCLUDE_RCF_REALLOCBUFFER_HPP
