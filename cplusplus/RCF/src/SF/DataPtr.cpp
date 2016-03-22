
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

#include <SF/DataPtr.hpp>

#include <RCF/Exception.hpp>

#include <SF/Tools.hpp>

namespace SF {

    DataPtr::DataPtr() :
        ptr_(buffer_),
        length_(),
        allocatedLength_(),
        whichDeleter_(),
        pfn_deleter_()
    {}

    DataPtr::DataPtr(const T *sz) :
        ptr_( const_cast<T*>(sz) ),
        length_( length(sz) ),
        allocatedLength_(length_+1),
        whichDeleter_(0),
        pfn_deleter_()
    {
        // Weak attach
        RCF_ASSERT(ptr_);
    }

    DataPtr::DataPtr(const T *sz, UInt32 length) :
        ptr_( const_cast<T*>(sz) ),
        length_(length),
        allocatedLength_(length_),
        whichDeleter_(),
        pfn_deleter_()
    {
        // Weak attach
        RCF_ASSERT(ptr_);
    }

    DataPtr::DataPtr(const DataPtr &rhs) :
        ptr_(rhs.ptr_),
        length_(rhs.length_),
        allocatedLength_(rhs.allocatedLength_),
        whichDeleter_(0),
        pfn_deleter_()
    {
        // Weak copy
    }

    DataPtr &DataPtr::operator=(const DataPtr &rhs)
    {
        // Weak copy
        ptr_ = rhs.ptr_;
        length_ = rhs.length_;
        allocatedLength_ = rhs.allocatedLength_;
        whichDeleter_ = 0;
        pfn_deleter_ = NULL;
        return *this;
    }
   
    DataPtr::~DataPtr()
    {
        RCF_DTOR_BEGIN
            release();
        RCF_DTOR_END
    }

    void DataPtr::assign(const T *sz, UInt32 length)
    {
        allocate(length);
        memmove( get(), sz, length*sizeof(T) );
    }

    void DataPtr::assign(const T *sz)
    {
        assign(sz, length(sz));
    }
   
    void DataPtr::assign(const std::string &s)
    {
        assign(s.c_str(), static_cast<UInt32>(s.length()));
    }

    void DataPtr::release()
    {
        switch (whichDeleter_)
        {
        case 0:
            break;
        case 1:
            delete ptr_; 
            break;
        case 2:
            delete [] ptr_;
            break;
        case 3:
            RCF_ASSERT(pfn_deleter_);
            pfn_deleter_(ptr_);
            break;
        default:
            RCF_ASSERT(0);
        }
        ptr_ = NULL;
        length_ = 0;
        whichDeleter_ = 0;
        pfn_deleter_ = NULL;
    }

    UInt32 DataPtr::allocate(UInt32 length)
    {
        release();
        if (length <= 63)
        {
            ptr_ = buffer_;
            length_ = length;
            ptr_[length_] = 0;
            allocatedLength_ = length_ + 1;
            whichDeleter_ = 0;
            pfn_deleter_ = NULL;
        }
        else
        {
            RCF_ASSERT(length < UInt32(-1));
            ptr_ = new T[length + 1];
            allocatedLength_ = length + 1;
            ptr_[length] = 0;
            length_ = length;
            whichDeleter_ = 2;
            pfn_deleter_ = NULL;
        }
        return length;
    }

    void DataPtr::terminatebufferwithzero() const
    {
        if (ptr_ && ptr_[length_] != 0)
        {
            RCF_ASSERT_GT(allocatedLength_ , length_);
            ptr_[length_] = 0;
        }
    }

    void DataPtr::update_length()
    {
        if (ptr_)
            length_ = length(ptr_);
    }

    DataPtr::T *DataPtr::get() const
    {
        return ptr_;
    }
   
    UInt32 DataPtr::length() const
    {
        return length_;
    }
   
    bool DataPtr::empty() const
    {
        return length_ == 0;
    }
   
    std::string DataPtr::cpp_str() const
    {
        return std::string(static_cast<char *>(ptr_), length_);
    }

    UInt32 DataPtr::length(const T *sz)
    {
        if (sz)
        {
            UInt32 i=0;
            while (sz[i])
                i++;
            return i;
        }
        else
            return 0;
    }

} // namespace SF
