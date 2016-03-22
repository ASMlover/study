
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

#ifndef INCLUDE_SF_TUPLE_TR1_HPP
#define INCLUDE_SF_TUPLE_TR1_HPP

#include <RCF/Config.hpp>

#include RCF_TR1_HEADER(tuple)

namespace SF {

    class Archive;

    // tr1::tuple

    template<
        typename T0>
        void serialize(SF::Archive &ar, std::tr1::tuple<T0> &t)
    {
        ar 
            & std::tr1::get<0>(t);
    }

    template<
        typename T0, 
        typename T1>
    void serialize(SF::Archive &ar, std::tr1::tuple<T0,T1> &t)
    {
        ar 
            & std::tr1::get<0>(t) 
            & std::tr1::get<1>(t);
    }

    template<
        typename T0, 
        typename T1, 
        typename T2>
    void serialize(SF::Archive &ar, std::tr1::tuple<T0,T1,T2> &t)
    {
        ar 
            & std::tr1::get<0>(t) 
            & std::tr1::get<1>(t) 
            & std::tr1::get<2>(t);
    }

    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3>
    void serialize(SF::Archive &ar, std::tr1::tuple<T0,T1,T2,T3> &t)
    {
        ar 
            & std::tr1::get<0>(t) 
            & std::tr1::get<1>(t) 
            & std::tr1::get<2>(t) 
            & std::tr1::get<3>(t);
    }

    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3, 
        typename T4>
    void serialize(SF::Archive &ar, std::tr1::tuple<T0,T1,T2,T3,T4> &t)
    {
        ar 
            & std::tr1::get<0>(t) 
            & std::tr1::get<1>(t) 
            & std::tr1::get<2>(t) 
            & std::tr1::get<3>(t) 
            & std::tr1::get<4>(t);
    }

    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3, 
        typename T4, 
        typename T5>
    void serialize(SF::Archive &ar, std::tr1::tuple<T0,T1,T2,T3,T4,T5> &t)
    {
        ar 
            & std::tr1::get<0>(t) 
            & std::tr1::get<1>(t) 
            & std::tr1::get<2>(t) 
            & std::tr1::get<3>(t) 
            & std::tr1::get<4>(t) 
            & std::tr1::get<5>(t);
    }

    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3, 
        typename T4, 
        typename T5, 
        typename T6>
    void serialize(SF::Archive &ar, std::tr1::tuple<T0,T1,T2,T3,T4,T5,T6> &t)
    {
        ar 
            & std::tr1::get<0>(t) 
            & std::tr1::get<1>(t) 
            & std::tr1::get<2>(t) 
            & std::tr1::get<3>(t) 
            & std::tr1::get<4>(t) 
            & std::tr1::get<5>(t) 
            & std::tr1::get<6>(t);
    }

    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3, 
        typename T4, 
        typename T5, 
        typename T6, 
        typename T7>
    void serialize(SF::Archive &ar, std::tr1::tuple<T0,T1,T2,T3,T4,T5,T6,T7> &t)
    {
        ar 
            & std::tr1::get<0>(t) 
            & std::tr1::get<1>(t) 
            & std::tr1::get<2>(t) 
            & std::tr1::get<3>(t) 
            & std::tr1::get<4>(t) 
            & std::tr1::get<5>(t) 
            & std::tr1::get<6>(t) 
            & std::tr1::get<7>(t);
    }

    // Visual C++ 11 doesn't support any more args to std::tuple<>.

    /*
    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3, 
        typename T4, 
        typename T5, 
        typename T6, 
        typename T7, 
        typename T8>
    void serialize(SF::Archive &ar, std::tr1::tuple<T0,T1,T2,T3,T4,T5,T6,T7,T8> &t)
    {
        ar 
            & std::tr1::get<0>(t) 
            & std::tr1::get<1>(t) 
            & std::tr1::get<2>(t) 
            & std::tr1::get<3>(t) 
            & std::tr1::get<4>(t) 
            & std::tr1::get<5>(t) 
            & std::tr1::get<6>(t) 
            & std::tr1::get<7>(t) 
            & std::tr1::get<8>(t);
    }

    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3, 
        typename T4, 
        typename T5, 
        typename T6, 
        typename T7, 
        typename T8, 
        typename T9>
    void serialize(SF::Archive &ar, std::tr1::tuple<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9> &t)
    {
        ar 
            & std::tr1::get<0>(t) 
            & std::tr1::get<1>(t) 
            & std::tr1::get<2>(t) 
            & std::tr1::get<3>(t) 
            & std::tr1::get<4>(t) 
            & std::tr1::get<5>(t) 
            & std::tr1::get<6>(t) 
            & std::tr1::get<7>(t) 
            & std::tr1::get<8>(t) 
            & std::tr1::get<9>(t);
    }
    */

} // namespace SF

#endif // ! INCLUDE_SF_TUPLE_TR1_HPP
