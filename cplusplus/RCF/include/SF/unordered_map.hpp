
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

#ifndef INCLUDE_SF_UNORDERED_MAP_HPP
#define INCLUDE_SF_UNORDERED_MAP_HPP

#include <RCF/Config.hpp>
#ifdef RCF_USE_TR1

#include RCF_TR1_HEADER(unordered_map)

#include <SF/SerializeStl.hpp>

namespace SF {

    // std::tr1::unordered_map
    template<typename Key, typename Value, typename Hash, typename Pred, typename Alloc>
    inline void serialize_vc6(Archive &ar, std::tr1::unordered_map<Key, Value, Hash, Pred, Alloc> &t, const unsigned int)
    {
        serializeStlContainer<InsertSemantics, NoReserveSemantics>(ar, t);
    }

    // std::tr1::unordered_multimap
    template<typename Key, typename Value, typename Hash, typename Pred, typename Alloc>
    inline void serialize_vc6(Archive &ar, std::tr1::unordered_multimap<Key, Value, Hash, Pred, Alloc> &t, const unsigned int)
    {
        serializeStlContainer<InsertSemantics, NoReserveSemantics>(ar, t);
    }

}

#endif // RCF_USE_TR1

#endif // ! INCLUDE_SF_UNORDERED_MAP_HPP
