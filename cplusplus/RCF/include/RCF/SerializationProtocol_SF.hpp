
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

#ifndef INCLUDE_RCF_PROTOCOL_SF_HPP
#define INCLUDE_RCF_PROTOCOL_SF_HPP

#include <RCF/SerializationProtocol_Base.hpp>

#include <SF/IBinaryStream.hpp>
#include <SF/OBinaryStream.hpp>
#include <SF/string.hpp>

namespace RCF {

    template<>
    class Protocol< boost::mpl::int_<SfBinary> > : public ProtocolImpl_SF<SF::IBinaryStream, SF::OBinaryStream>
    {
    public:
        static std::string getName()
        {
            return "SF binary serialization protocol";
        }
    };

} // namespace RCF

/*
#include <SF/ITextStream.hpp>
#include <SF/OTextStream.hpp>

namespace RCF {

    template<>
    class Protocol< boost::mpl::int_<SfText> > : public ProtocolImpl1<SF::ITextStream, SF::OTextStream>
    {
    public:
        static std::string getName()
        {
            return "SF text protocol";
        }
    };

    inline void enableSfPointerTracking_2(SF::OTextStream &otextStream, bool enable)
    {
        enable ?
            otextStream.enableContext():
            otextStream.disableContext();
    }

} // namespace RCF
*/

#endif //! INCLUDE_RCF_PROTOCOL_SF_HPP
