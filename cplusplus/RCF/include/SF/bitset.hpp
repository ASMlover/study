
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

#ifndef INCLUDE_SF_BITSET_HPP
#define INCLUDE_SF_BITSET_HPP

#include <bitset>

#include <RCF/Export.hpp>

namespace SF {

    class Archive;

    class I_BitsetWrapper
    {
    public:
        virtual std::size_t size() = 0;
        virtual void resize(std::size_t newSize) = 0;
        virtual void setBit(std::size_t idx, bool newValue) = 0;
        virtual bool getBit(std::size_t idx) = 0;
    };

    template<std::size_t N>
    class BitsetWrapper : public I_BitsetWrapper
    {
    public:
        BitsetWrapper(std::bitset<N> & bits) : mBits(bits)
        {
        }

        virtual std::size_t size()
        {
            return mBits.size();
        }

        virtual void resize(std::size_t newSize)
        {
            RCF_ASSERT(newSize == N);
        }

        virtual void setBit(std::size_t idx, bool newValue)
        {
            mBits[idx] = newValue;
        }

        virtual bool getBit(std::size_t idx)
        {
            return mBits[idx];
        }

    private:

        std::bitset<N> & mBits;
    };

    RCF_EXPORT void serializeBitset(SF::Archive & ar, I_BitsetWrapper & bits);

    template<std::size_t N>
    void serialize(SF::Archive & ar, std::bitset<N> & bits)
    {
        BitsetWrapper<N> wrapper(bits);
        serializeBitset(ar, wrapper);
    }

}

#endif // ! INCLUDE_SF_BITSET_HPP
