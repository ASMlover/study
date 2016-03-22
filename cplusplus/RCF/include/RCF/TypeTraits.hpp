
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

#ifndef INCLUDE_RCF_TYPETRAITS_HPP
#define INCLUDE_RCF_TYPETRAITS_HPP

#include <boost/type_traits.hpp>

namespace RCF {

    template<typename T>
    struct IsFundamental : public boost::is_fundamental<T>
    {};

    template<typename T>
    struct IsConst : public boost::is_const<T>
    {};

    template<typename T>
    struct IsPointer : public boost::is_pointer<T>
    {};

    template<typename T>
    struct IsReference : public boost::is_reference<T>
    {};

    template<typename T>
    struct RemovePointer : public boost::remove_pointer<T>
    {};

    template<typename T>
    struct RemoveReference : public boost::remove_reference<T>
    {};

    template<typename T>
    struct RemoveCv : public boost::remove_cv<T>
    {};

    template<typename T>
    struct Out
    {
    };

    template<typename T>
    struct RemoveOut
    {
        typedef T type;
    };

    template<typename T>
    struct IsOut : public boost::mpl::false_
    {
    };

    template<typename T>
    struct RemoveOut< Out<T> >
    {
        typedef T type;
    };

    template<typename T>
    struct IsOut< Out<T> >
    {
        typedef boost::mpl::true_ type;
        enum { value = type::value };
    };

} // namespace RCF

namespace SF {

    template<typename T> struct GetIndirection;

} // namespace SF

#endif // ! INCLUDE_RCF_TYPETRAITS_HPP
