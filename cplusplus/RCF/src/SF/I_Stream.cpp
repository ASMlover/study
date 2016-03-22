
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

#include <SF/I_Stream.hpp>

#include <sstream>

#include <SF/DataPtr.hpp>

#include <RCF/ByteOrdering.hpp>
#include <RCF/Exception.hpp>
#include <RCF/MemStream.hpp>

namespace SF {

    //--------------------------------------------------------------------------
    // EncodingText

    static char chSeparator = ':';

    // Generic EncodingText::getCount().

    template<typename T>
    UInt32 EncodingText_getCountImpl(DataPtr & data, T *)
    {
        // Return 1 + number of separator characters.

        UInt32 count = 0;
        for (UInt32 i=1; i<data.length()-1; i++)
        {
            if (data.get()[i] == Byte8(chSeparator))
            {
                count++;
            }
        }
        return count+1;
    }

    // Specializations of EncodingText::getCount().

    UInt32 EncodingText_getCountImpl(DataPtr &data, char *)
    {
        return data.length();
    }

    UInt32 EncodingText_getCountImpl(DataPtr &data, unsigned char *)
    {
        return data.length();
    }

    // Generic EncodingText::toData() and EncodingText::toObject().

    template<typename T>
    void EncodingText_toDataImpl(
        DataPtr &       data, 
        T *             t, 
        int             nCount)
    {
        RCF::MemOstream ostr;
        ostr << t[0];
        for (int i=1; i<nCount; i++)
        {
            ostr.put(chSeparator);
            ostr << t[i];
        }
        std::string s = ostr.string();
        data.assign(
            reinterpret_cast<const Byte8 *>(s.c_str()), 
            static_cast<UInt32>(s.length()));
    }

    template<typename T>
    void EncodingText_toObjectImpl(
        DataPtr &       data, 
        T *             t, 
        int             nCount)
    {
        if (data.length() == 0)
        {
            RCF::Exception e(RCF::_SfError_DataFormat());
            RCF_THROW(e);
        }
        RCF::MemIstream istr(data.get(), data.length());
        istr >> t[0];
        for (int i=1; i<nCount; i++)
        {
            char ch;
            istr.get(ch);
            RCF_ASSERT_EQ( ch , chSeparator );
            istr >> t[i];
        }
    }

    // Specializations of EncodingText::toData() and EncodingText::toObject().

    // char
    void EncodingText_toDataImpl(
        DataPtr &       data, 
        char *          t, 
        int             nCount)
    {
        data.assign(reinterpret_cast<Byte8 *>(t), nCount);
    }

    void EncodingText_toObjectImpl(
        DataPtr &       data, 
        char *          t, 
        int             nCount)
    {
        memcpy(t, data.get(), nCount);
    }

    // unsigned char
    void EncodingText_toDataImpl(
        DataPtr &       data, 
        unsigned char * t, 
        int             nCount)
    {
        data.assign(reinterpret_cast<Byte8 *>(t), nCount);
    }

    void EncodingText_toObjectImpl(
        DataPtr &       data, 
        unsigned char *          t, 
        int             nCount)
    {
        memcpy(t, data.get(), nCount);
    }

    // wchar_t
    void EncodingText_toDataImpl(
        DataPtr &       data, 
        wchar_t *       t, 
        int             nCount)
    {
        data.assign(reinterpret_cast<Byte8 *>(t), nCount*sizeof(wchar_t));
    }

    void EncodingText_toObjectImpl(
        DataPtr &       data, 
        wchar_t *       t, 
        int             nCount)
    {
        memcpy(t, data.get(), nCount*sizeof(wchar_t));
    }

    // Generate EncodingText virtual functions.

#define SF_IMPLEMENT_ENCODINGTEXT(T)                        \
    UInt32 EncodingText::getCount(DataPtr & data, T * pt)   \
    {                                                       \
        return EncodingText_getCountImpl(data, pt);         \
    }                                                       \
                                                            \
    void EncodingText::toData(                              \
        DataPtr &       data,                               \
        T *             t,                                  \
        int             nCount)                             \
    {                                                       \
        EncodingText_toDataImpl(data, t, nCount);           \
    }                                                       \
                                                            \
    void EncodingText::toObject(                            \
        DataPtr &       data,                               \
        T *             t,                                  \
        int             nCount)                             \
    {                                                       \
        EncodingText_toObjectImpl(data, t, nCount);         \
    }


    SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_IMPLEMENT_ENCODINGTEXT )

#undef SF_IMPLEMENT_ENCODINGTEXT


    //--------------------------------------------------------------------------
    // EncodingBinaryNative

    // Generic EncodingBinaryNative::getCount().
    template<typename T>
    UInt32 EncodingBinaryNative_getCountImpl(DataPtr & data, T *)
    {
        RCF_ASSERT(data.length() % sizeof(T) == 0);
        return data.length() / sizeof(T);
    }

    // Generic EncodingBinaryNative::toData().
    template<typename T>
    void EncodingBinaryNative_toDataImpl(
        DataPtr &       data, 
        T *             t, 
        int             nCount)
    {
        data.assign(reinterpret_cast<Byte8 *>(t), sizeof(T)*nCount );
    }

    // Generic EncodingBinaryNative::toObject().
    template<typename T>
    void EncodingBinaryNative_toObjectImpl(
        DataPtr &       data, 
        T *             t, 
        int             nCount)
    {
        RCF_ASSERT_EQ( data.length() , sizeof(T)*nCount);
        memcpy(t, data.get(), sizeof(T)*nCount);
    }

    // Generate EncodingBinaryNative virtual functions.

#define SF_IMPLEMENT_ENCODINGBINARYNATIVE(T)                        \
    UInt32 EncodingBinaryNative::getCount(DataPtr & data, T * pt)   \
    {                                                               \
        return EncodingBinaryNative_getCountImpl(data, pt);         \
    }                                                               \
                                                                    \
    void EncodingBinaryNative::toData(                              \
        DataPtr &       data,                                       \
        T *             t,                                          \
        int             nCount)                                     \
    {                                                               \
        EncodingBinaryNative_toDataImpl(data, t, nCount);           \
    }                                                               \
                                                                    \
    void EncodingBinaryNative::toObject(                            \
        DataPtr &       data,                                       \
        T *             t,                                          \
        int             nCount)                                     \
    {                                                               \
        EncodingBinaryNative_toObjectImpl(data, t, nCount);         \
    }


    SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_IMPLEMENT_ENCODINGBINARYNATIVE )

#undef SF_IMPLEMENT_ENCODINGBINARYNATIVE


    //--------------------------------------------------------------------------
    // EncodingBinaryPortable

    // Generic EncodingBinaryPortable::getCount().
    template<typename T>
    UInt32 EncodingBinaryPortable_getCountImpl(DataPtr & data, T *)
    {
        RCF_ASSERT(data.length() % sizeof(T) == 0);
        return data.length() / sizeof(T);
    }

    // Generic EncodingBinaryPortable::toData().
    template<typename T>
    void EncodingBinaryPortable_toDataImpl(
        DataPtr &       data, 
        T *             t, 
        int             nCount)
    {
        UInt32 nBufferSize = sizeof(T) * nCount;
        UInt32 nAlloc = data.allocate(nBufferSize);
        RCF_ASSERT_EQ(nAlloc , nBufferSize);
        RCF_UNUSED_VARIABLE(nAlloc);
        T *buffer = reinterpret_cast<T *>(data.get());
        memcpy(buffer, t, nBufferSize);
        RCF::machineToNetworkOrder(buffer, sizeof(T), nCount);
    }

    // Generic EncodingBinaryPortable::toObject().
    template<typename T>
    void EncodingBinaryPortable_toObjectImpl(
        DataPtr &       data, 
        T *             t, 
        int             nCount)
    {
        if (data.length() != sizeof(T)*nCount)
        {
            RCF::Exception e(RCF::_SfError_DataFormat());
            RCF_THROW(e)(data.length())(nCount)(typeid(T).name());
        }
        T *buffer = reinterpret_cast<T *>(data.get());
        RCF::networkToMachineOrder(buffer, sizeof(T), nCount);
        memcpy(t, buffer, nCount*sizeof(T));
    }

    // Generate EncodingBinaryPortable virtual functions.

#define SF_IMPLEMENT_ENCODINGBINARYPORTABLE(T)                      \
    UInt32 EncodingBinaryPortable::getCount(DataPtr & data, T * pt) \
    {                                                               \
        return EncodingBinaryPortable_getCountImpl(data, pt);       \
    }                                                               \
                                                                    \
    void EncodingBinaryPortable::toData(                            \
        DataPtr &       data,                                       \
        T *             t,                                          \
        int             nCount)                                     \
    {                                                               \
        EncodingBinaryPortable_toDataImpl(data, t, nCount);         \
    }                                                               \
                                                                    \
    void EncodingBinaryPortable::toObject(                          \
        DataPtr &       data,                                       \
        T *             t,                                          \
        int             nCount)                                     \
    {                                                               \
        EncodingBinaryPortable_toObjectImpl(data, t, nCount);       \
    }

    SF_FOR_EACH_FUNDAMENTAL_TYPE( SF_IMPLEMENT_ENCODINGBINARYPORTABLE )

#undef SF_IMPLEMENT_ENCODINGBINARYPORTABLE


} // namespace SF
