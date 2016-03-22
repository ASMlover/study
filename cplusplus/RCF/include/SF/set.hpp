
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

#ifndef INCLUDE_SF_SET_HPP
#define INCLUDE_SF_SET_HPP

#include <set>

#include <SF/SerializeStl.hpp>

namespace SF {

    // std::set
    template<typename K, typename T, typename A>
    inline void serialize_vc6(SF::Archive &ar, std::set<K,T,A> &t, const unsigned int)
    {
        serializeStlContainer<InsertSemantics, NoReserveSemantics>(ar, t);
    }

    // std::multiset
    template<typename K, typename T, typename A>
    inline void serialize_vc6(SF::Archive &ar, std::multiset<K,T,A> &t, const unsigned int)
    {
        serializeStlContainer<InsertSemantics, NoReserveSemantics>(ar, t);
    }

} // namespace SF

#endif // ! INCLUDE_SF_SET_HPP
