
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

#ifndef INCLUDE_RCF_GETINTERFACENAME_HPP
#define INCLUDE_RCF_GETINTERFACENAME_HPP

#include <string>

namespace RCF {

    // Returns the runtime name of the given RCF interface.
    template<typename Interface>
    inline std::string getInterfaceName(Interface * = 0)
    {
        return Interface::getInterfaceName();
    }

} // namespace RCF

#endif // ! INCLUDE_RCF_GETINTERFACENAME_HPP
