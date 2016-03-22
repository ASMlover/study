
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

#ifndef INCLUDE_SF_SERIALIZEPARENT_HPP
#define INCLUDE_SF_SERIALIZEPARENT_HPP

#include <SF/Archive.hpp>
#include <SF/SerializePolymorphic.hpp>

namespace SF {

    template<typename Base, typename Derived>
    void serializeParent(Base *, Archive &ar, Derived &derived)
    {
        ar & SF::Archive::PARENT & static_cast<Base &>(derived);
    }

    template<typename Base, typename Derived>
    void serializeParent(Archive &ar, Derived &derived)
    {
        serializeParent( (Base *) 0, ar, derived);
    }

} // namespace SF

#endif // ! INCLUDE_SF_SERIALIZEPARENT_HPP
