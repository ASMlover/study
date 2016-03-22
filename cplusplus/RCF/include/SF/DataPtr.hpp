
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

#ifndef INCLUDE_SF_DATAPTR_HPP
#define INCLUDE_SF_DATAPTR_HPP

#include <string>

#include <RCF/Export.hpp>

#include <SF/PortableTypes.hpp>

namespace SF {

    //************************************************************************
    // DataPtr class holds a pointer to a buffer of data. It includes an internal
    // buffer in order to avoid dynamic memory allocation for small buffer sizes, < 64bytes.

    class RCF_EXPORT DataPtr
    {
    private:
        typedef Byte8 T;
    public:
        DataPtr();
        DataPtr(const T *sz);
        DataPtr(const T *sz, UInt32 length);
        DataPtr(const DataPtr &rhs);
        DataPtr &operator=(const DataPtr &rhs);
        ~DataPtr();

        void assign(const T *sz, UInt32 length);
        void assign(const T *sz);
        void assign(const std::string &s);

        void release();
        UInt32 allocate(UInt32 length);
        void terminatebufferwithzero() const;
        void update_length();
        T *get() const;
        UInt32 length() const;
        bool empty() const;
        std::string cpp_str() const;

    private:
        T *ptr_;
        UInt32 length_;
        UInt32 allocatedLength_;
        int whichDeleter_;
        void (*pfn_deleter_)(T *);
        T buffer_[64];
        UInt32 length(const T *sz);
    };

} // namespace SF

#endif // ! INCLUDE_SF_DATAPTR_HPP
