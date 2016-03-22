
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

#ifndef INCLUDE_SF_HASH_SET_HPP
#define INCLUDE_SF_HASH_SET_HPP

#include <RCF/Config.hpp>
#ifdef RCF_USE_HASH_MAP

#include RCF_HASH_MAP_HEADER(hash_set)

#include <SF/SerializeStl.hpp>

namespace SF {

    // hash_set
    template<typename Key, typename Hash, typename Alloc>
    inline void serialize_vc6(Archive &ar, RCF_HASH_MAP_NS::hash_set<Key, Hash, Alloc> &t, const unsigned int)
    {
        serializeStlContainer<InsertSemantics, NoReserveSemantics>(ar, t);
    }

    // hash_multiset
    template<typename Key, typename Hash, typename Alloc>
    inline void serialize_vc6(Archive &ar, RCF_HASH_MAP_NS::hash_multiset<Key, Hash, Alloc> &t, const unsigned int)
    {
        serializeStlContainer<InsertSemantics, NoReserveSemantics>(ar, t);
    }

}

#endif // RCF_USE_HASH_MAP

#endif // ! INCLUDE_SF_HASH_SET_HPP
