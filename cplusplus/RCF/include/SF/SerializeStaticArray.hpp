
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

#ifndef INCLUDE_SF_SERIALIZESTATICARRAY_HPP
#define INCLUDE_SF_SERIALIZESTATICARRAY_HPP

#include <SF/Archive.hpp>

namespace SF {

    // serialize C-style static arrays

    template<typename T, unsigned int N>
    inline void serializeFundamentalStaticArray(
        Archive &           ar, 
        T                   (*pt)[N])
    {
        serializeFundamental(ar, (*pt)[0], N);
    }

    template<typename T, unsigned int N>
    inline void serializeNonfundamentalStaticArray(
        Archive &           ar, 
        T                   (*pt)[N])
    {
        for (unsigned int i=0; i<N; i++)
            ar & (*pt)[i];
    }


    template<bool IsFundamental>
    class SerializeStaticArray;

    template<>
    class SerializeStaticArray<true>
    {
    public:
        template<typename T, unsigned int N>
        void operator()(Archive &ar, T (*pt)[N])
        {
            serializeFundamentalStaticArray(ar, pt);
        }
    };

    template<>
    class SerializeStaticArray<false>
    {
    public:
        template<typename T, unsigned int N>
        void operator()(Archive &ar, T (*pt)[N])
        {
            serializeNonfundamentalStaticArray(ar, pt);
        }
    };


    template<typename T, unsigned int N>
    inline void preserialize(SF::Archive &ar, T (*pt)[N], const unsigned int)
    {
        static const bool IsFundamental = RCF::IsFundamental<T>::value;
        SerializeStaticArray<IsFundamental>()(ar, pt);
    }

} // namespace SF

#endif // ! INCLUDE_SF_SERIALIZESTATICARRAY_HPP
