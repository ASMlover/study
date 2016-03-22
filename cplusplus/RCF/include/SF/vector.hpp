
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

#ifndef INCLUDE_SF_VECTOR_HPP
#define INCLUDE_SF_VECTOR_HPP

#include <vector>

#include <SF/Serializer.hpp>
#include <SF/SerializeStl.hpp>

#include <boost/type_traits.hpp>

namespace SF {

    // std::vector

    template<typename T, typename A>
    inline void serializeVector(
        SF::Archive &           ar,
        std::vector<T,A> &      vec,
        boost::mpl::false_ *)
    {
        serializeStlContainer<PushBackSemantics, ReserveSemantics>(ar, vec);
    }

    template<typename T, typename A>
    inline void serializeVector(
        SF::Archive &           ar,
        std::vector<T,A> &      vec,
        boost::mpl::true_ *)
    {
        serializeVectorFast(ar, vec);
    }

    template<typename T, typename A>
    inline void serialize(
        SF::Archive &           ar,
        std::vector<T,A> &      vec)
    {
        // We want fast vector serialization for vector<T>, if T is fundamental.
        // Don't need to cover the case where T is a bool, as vector<bool> has
        // its own serialize() function (see below).

        const bool IsBool = boost::is_same<T, bool>::value;
        BOOST_STATIC_ASSERT( !IsBool );

        typedef typename boost::is_fundamental<T>::type type;
        serializeVector(ar, vec, (type *) 0);
    }

    // Special case serialization for vector<bool>.
    RCF_EXPORT void serialize(SF::Archive & ar, std::vector<bool> & bits);

    class I_VecWrapper
    {
    public:
        virtual ~I_VecWrapper() {}

        virtual void            resize(std::size_t newSize) = 0;
        virtual boost::uint32_t size() = 0;
        virtual char *          addressOfElement(std::size_t idx) = 0;
        virtual boost::uint32_t sizeofElement() = 0;
    };

    template<typename Vec>
    class VecWrapper : public I_VecWrapper
    {
    public:
        VecWrapper(Vec & vec) : mVec(vec)
        {
        }

        void resize(std::size_t newSize)
        {
            mVec.resize(newSize);
        }

        boost::uint32_t size()
        {
            return static_cast<boost::uint32_t>(mVec.size());
        }

        char * addressOfElement(std::size_t idx)
        {
            return reinterpret_cast<char *>( &mVec[idx] );
        }

        boost::uint32_t sizeofElement()
        {
            typedef typename Vec::value_type ValueType;
            return sizeof(ValueType);
        }

    private:
        Vec & mVec;
    };

    RCF_EXPORT void serializeVectorFastImpl(
        SF::Archive &           ar,
        I_VecWrapper &          vec);

    template<typename T, typename A>
    inline void serializeVectorFast(
        SF::Archive &           ar,
        std::vector<T,A> &      vec)
    {
        VecWrapper< std::vector<T,A> > vecWrapper(vec);
        serializeVectorFastImpl(ar, vecWrapper);
    }
        
} // namespace SF

#endif // ! INCLUDE_SF_VECTOR_HPP
