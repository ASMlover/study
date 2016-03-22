
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

#ifndef INCLUDE_SF_NODE_HPP
#define INCLUDE_SF_NODE_HPP

#include <boost/noncopyable.hpp>

#include <SF/DataPtr.hpp>
#include <SF/PortableTypes.hpp>

namespace SF {

    //****************************************************************************
    // Node class represents a node in the serialized hierarchy of objects
    // (eg XML streams would translate it to an element in a DOM tree)

    class Node : boost::noncopyable
    {
    public:
        Node() :
            type(),
            label(),
            id(),
            ref()
        {}

        Node(
            const DataPtr & type, 
            const DataPtr & label,  
            const UInt32    id, 
            const UInt32    nullPtr) :
                type(type),
                label(label),
                id(id),
                ref(nullPtr)
        {}

        DataPtr type;
        DataPtr label;
        UInt32 id;
        UInt32 ref;
    };

} // namespace SF

#endif // ! INCLUDE_SF_NODE_HPP
