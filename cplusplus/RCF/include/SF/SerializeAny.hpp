
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

#ifndef INCLUDE_SF_SERIALIZEANY_HPP
#define INCLUDE_SF_SERIALIZEANY_HPP

#include <boost/any.hpp>

namespace SF {

    class Archive;

    class I_SerializerAny
    {
    public:
        virtual ~I_SerializerAny() 
        {}

        virtual void serialize(
            SF::Archive &ar, 
            boost::any &a) = 0;
    };

    template<typename T>
    class SerializerAny : public I_SerializerAny
    {
    public:
        void serialize(SF::Archive &ar, boost::any &a);
    };

} // namespace SF

#include <SF/Archive.hpp>

namespace SF {

    template<typename T>
    void SerializerAny<T>::serialize(SF::Archive &ar, boost::any &a)
    {
        if (ar.isWrite())
        {
            ar & boost::any_cast<T>(a);
        }
        else
        {
            T t;
            ar & t;
            a = t;
        }
    }

} // namespace SF

#endif // ! INCLUDE_SF_SERIALIZEANY_HPP
