
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

#ifndef INCLUDE_SF_ENCODING_HPP
#define INCLUDE_SF_ENCODING_HPP

#include <RCF/ByteBuffer.hpp>
#include <RCF/ByteOrdering.hpp>
#include <RCF/Exception.hpp>

#include <SF/DataPtr.hpp>
#include <SF/Tools.hpp>

namespace SF {

    // Binary encoding/decoding of bool, int and string. Mostly used by RCF, but
    // also by SF to encode version numbers into archives.

    RCF_EXPORT 
    void encodeBool(
        bool                        value, 
        std::vector<char> &         vec, 
        std::size_t &               pos);

    RCF_EXPORT 
    void encodeInt(
        int                         value, 
        std::vector<char> &         vec, 
        std::size_t &               pos);
    
    RCF_EXPORT 
    void encodeString(
        const std::string &         value, 
        std::vector<char> &         vec, 
        std::size_t &               pos);

    RCF_EXPORT 
    void encodeByteBuffer(
        RCF::ByteBuffer             value, 
        std::vector<char> &         vec, 
        std::size_t &               pos);

    RCF_EXPORT 
    void encodeBool(
        bool                        value, 
        const RCF::ByteBuffer &     byteBuffer, 
        std::size_t &               pos);
    
    RCF_EXPORT 
    void encodeInt(
        int                         value, 
        const RCF::ByteBuffer &     byteBuffer, 
        std::size_t &               pos);
    
    RCF_EXPORT 
    void encodeString(
        const std::string &         value, 
        const RCF::ByteBuffer &     byteBuffer, 
        std::size_t &               pos);

    RCF_EXPORT 
    void decodeBool(
        bool &                      value, 
        const RCF::ByteBuffer &     byteBuffer, 
        std::size_t &               pos);
    
    RCF_EXPORT 
    void decodeInt(
        int &                       value, 
        const RCF::ByteBuffer &     byteBuffer, 
        std::size_t &               pos);

    RCF_EXPORT 
    void decodeInt(
        boost::uint32_t &           value, 
        const RCF::ByteBuffer &     byteBuffer, 
        std::size_t &               pos);
    
    RCF_EXPORT 
    void decodeString(
        std::string &               value, 
        const RCF::ByteBuffer &     byteBuffer, 
        std::size_t &               pos);

    RCF_EXPORT 
    void decodeByteBuffer(
        RCF::ByteBuffer &           value, 
        const RCF::ByteBuffer &     byteBuffer, 
        std::size_t &               pos);

} // namespace SF

#endif // !INCLUDE_SF_ENCODING_HPP
