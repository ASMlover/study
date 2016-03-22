
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

#ifndef INCLUDE_RCF_INITDEINIT_HPP
#define INCLUDE_RCF_INITDEINIT_HPP

#include <RCF/Export.hpp>

#include <RCF/Config.hpp>

#include <boost/mpl/int.hpp>
#include <boost/mpl/list.hpp>

namespace RCF {

    // For debug use.
    RCF_EXPORT std::size_t getInitRefCount();

    /// Reference-counted initialization of RCF framework. May be called multiple
    /// times (see deinit()).
    RCF_EXPORT bool init(RcfConfigT * = NULL);

    /// Reference-counted deinitialization of RCF framework. For actual deinitialization
    /// to take place, deinit() must be called as many times as init() has been
    /// called.
    RCF_EXPORT bool deinit();

    // Initialization sentry. Ctor calls RCF::init(), dtor calls RCF::deinit().
    class RCF_EXPORT RcfInitDeinit
    {
    public:
        RcfInitDeinit(RcfConfigT * = NULL);
        ~RcfInitDeinit();

        // Returns true if this instance initialized RCF.
        bool isRootInstance();

    private:
        bool mIsRootInstance;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_INITDEINIT_HPP
