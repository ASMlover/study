
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

#ifndef INCLUDE_RCF_PROTOCOL_BOOSTSERIALIZATION_HPP
#define INCLUDE_RCF_PROTOCOL_BOOSTSERIALIZATION_HPP

#include <RCF/SerializationProtocol_Base.hpp>

#if RCF_FEATURE_BOOST_SERIALIZATION==1
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#endif

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>

namespace RCF {   

#if RCF_FEATURE_BOOST_SERIALIZATION==1

    template<> 
    class Protocol< boost::mpl::int_<BsBinary> > :
        public ProtocolImpl_BSer<boost::archive::binary_iarchive, boost::archive::binary_oarchive>
    {
    public:
        static std::string getName()
        {
            return "Boost.Serialization binary serialization protocol.";
        }
    };

    template<> 
    class Protocol< boost::mpl::int_<BsText> > :
        public ProtocolImpl_BSer<boost::archive::text_iarchive, boost::archive::text_oarchive>
    {
    public:
        static std::string getName()
        {
            return "Boost.Serialization text serialization protocol.";
        }
    };

#endif

} // namespace RCF

#endif //! INCLUDE_RCF_PROTOCOL_BOOSTSERIALIZATION_HPP
