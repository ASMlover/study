
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

#ifndef INCLUDE_SF_SERIALIZESTL_HPP
#define INCLUDE_SF_SERIALIZESTL_HPP

#include <SF/Archive.hpp>

namespace SF {

    class PushBackSemantics
    {
    public:
        template<typename Container, typename Value>
        void add(Container &container, const Value &value)
        {
            container.push_back(value);
        }        
    };

    class InsertSemantics
    {
    public:
        template<typename Container, typename Value>
        void add(Container &container, const Value &value)
        {
            container.insert(value);
        }
    };

    class ReserveSemantics
    {
    public:
        template<typename Container>
        void reserve(Container &container, std::size_t newSize)
        {
            container.reserve(newSize);
        }
    };

    class NoReserveSemantics
    {
    public:
        template<typename Container>
        void reserve(Container &container, std::size_t newSize)
        {
            RCF_UNUSED_VARIABLE(container);
            RCF_UNUSED_VARIABLE(newSize);
        }
    };

    template<typename AddFunc, typename ReserveFunc, typename StlContainer>
    void serializeStlContainer(Archive &ar, StlContainer &t)
    {

        typedef typename StlContainer::iterator Iterator;
        typedef typename StlContainer::value_type Value;

        if (ar.isRead())
        {
            t.clear();
            unsigned int count = 0;
            ar & count;

            std::size_t minSerializedLength = sizeof(Value);
            if (ar.verifyAgainstArchiveSize(count*minSerializedLength))
            {
                ReserveFunc().reserve(t, count);
            }

            for (unsigned int i=0; i<count; i++)
            {
                Value value;
                ar & value;
                AddFunc().add(t, value);
            }
        }
        else if (ar.isWrite())
        {
            unsigned int count = static_cast<unsigned int>(t.size());
            ar & count;
            Iterator it = t.begin();
            for (unsigned int i=0; i<count; i++)
            {
                ar & *it;
                it++;
            }
        }
    }

}

#endif // ! INCLUDE_SF_SERIALIZESTL_HPP
