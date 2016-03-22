
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

#ifndef INCLUDE_SF_STRING_HPP
#define INCLUDE_SF_STRING_HPP

#include <algorithm>
#include <string>

#include <boost/config.hpp>

#include <RCF/MinMax.hpp>

#include <SF/Archive.hpp>
#include <SF/Stream.hpp>

#include <RCF/utf8/convert.hpp>

namespace SF {

    template<typename C, typename T, typename A>
    inline void serializeString(SF::Archive & ar, std::basic_string<C,T,A> & s)
    {
        if (ar.isRead())
        {
            boost::uint32_t count = 0;
            ar & count;

            SF::IStream &is = *ar.getIstream();

            s.resize(0);

            std::size_t minSerializedLength = sizeof(C);
            if (ar.verifyAgainstArchiveSize(count*minSerializedLength))
            {
                if (count > s.capacity())
                {
                    s.reserve(count);
                }
            }

            boost::uint32_t charsRemaining = count;
            const boost::uint32_t BufferSize = 512;
            C buffer[BufferSize];
            while (charsRemaining)
            {
                boost::uint32_t charsToRead = RCF_MIN(BufferSize, charsRemaining);
                boost::uint32_t bytesToRead = charsToRead*sizeof(C);

                RCF_VERIFY(
                    is.read( (char *) buffer, bytesToRead) == bytesToRead,
                    RCF::Exception(RCF::_SfError_ReadFailure()))
                    (bytesToRead)(BufferSize)(count);

                s.append(buffer, charsToRead);
                charsRemaining -= charsToRead;
            }
        }
        else if (ar.isWrite())
        {
            boost::uint32_t count = static_cast<boost::uint32_t >(s.length());
            ar & count;
            ar.getOstream()->writeRaw(
                (char *) s.c_str(),
                count*sizeof(C));
        }

    }

    RCF_EXPORT bool getCurrentNativeWstringSerialization();

#if !defined(BOOST_NO_STD_WSTRING)

    template<typename C, typename T, typename A>
    inline void serializeEncodedString(SF::Archive & ar, std::basic_string<C,T,A> & ws)
    {
        if (ar.getRuntimeVersion() < 8)
        {
            serializeString(ar, ws);
            return;
        }

        RCF_ASSERT_GTEQ(ar.getRuntimeVersion() , 8);

        if (ar.isRead())
        {
            int encodingType = 0;
            ar & encodingType;
            if (encodingType == 8)
            {
                // UTF-8 serialization.

                std::string s;
                ar & s;
                ws = RcfBoost::boost::from_utf8(s);
            }
            else
            {
                // Native wchar_t serialization.

                RCF_VERIFY(
                    encodingType == sizeof(wchar_t), 
                    RCF::Exception(
                        RCF::_RcfError_WcharSizeMismatch(sizeof(wchar_t), encodingType)));

                serializeString(ar, ws);
            }

        }
        else if (ar.isWrite())
        {       
            bool useNativeWstringSerialization = SF::getCurrentNativeWstringSerialization();
            int encodingType = sizeof(wchar_t);
            if (useNativeWstringSerialization)
            {
                encodingType = sizeof(wchar_t);
            }
            else
            {
                encodingType = 8;
            }

            ar & encodingType;

            if (encodingType == 8)
            {
                // UTF-8 serialization.

                std::string s = RcfBoost::boost::to_utf8(ws);
                ar & s;
            }
            else
            {
                // Native wchar_t serialization.

                RCF_VERIFY(
                    encodingType == sizeof(wchar_t), 
                    RCF::Exception(
                        RCF::_RcfError_WcharSizeMismatch(sizeof(wchar_t), encodingType)));

                serializeString(ar, ws);
            }
        }
    }

    template<typename C, typename T, typename A>
    inline void serializeEncodedStringOrNot(SF::Archive & ar, std::basic_string<C, T, A> & t, boost::mpl::true_ *)
    {
        serializeEncodedString(ar, t);
    }

    template<typename C, typename T, typename A>
    inline void serializeEncodedStringOrNot(SF::Archive & ar, std::basic_string<C, T, A> & t, boost::mpl::false_ *)
    {
        serializeString(ar, t);
    }

    // std::basic_string (with wstring support)
    template<typename C, typename T, typename A>
    inline void serialize_vc6(SF::Archive & ar, std::basic_string<C,T,A> & t, const unsigned int)
    {
        typedef typename boost::is_same<C, wchar_t>::type type;
        serializeEncodedStringOrNot(ar, t, (type *) NULL);
    }

#else

    // std::basic_string (without wstring support)
    template<typename C, typename T, typename A>
    inline void serialize_vc6(SF::Archive & ar, std::basic_string<C,T,A> & t, const unsigned int)
    {
        serializeString(ar, t);
    }

#endif

} // namespace SF

#endif // ! INCLUDE_SF_STRING_HPP
