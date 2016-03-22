
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

#ifndef INCLUDE_RCF_CURRENTSERIALIZATIONPROTOCOL_HPP
#define INCLUDE_RCF_CURRENTSERIALIZATIONPROTOCOL_HPP

#include <cstddef>

#include <RCF/Export.hpp>

namespace RCF {

    class SerializationProtocolIn;
    class SerializationProtocolOut;

    RCF_EXPORT SerializationProtocolIn *getCurrentSerializationProtocolIn();
    RCF_EXPORT SerializationProtocolOut *getCurrentSerializationProtocolOut();

} // namespace RCF

#endif // ! INCLUDE_RCF_CURRENTSERIALIZATIONPROTOCOL_HPP
