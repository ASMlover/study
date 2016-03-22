
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

#ifndef INCLUDE_RCF_ASIODEADLINETIMER_HPP
#define INCLUDE_RCF_ASIODEADLINETIMER_HPP

#include <RCF/Asio.hpp>

namespace RCF {

    typedef ASIO_NS::deadline_timer AsioDeadlineTimer;
    typedef boost::shared_ptr<AsioDeadlineTimer> AsioDeadlineTimerPtr;

    // Using a wrapper for deadline_timer so we can do forward declarations.
    class AsioTimer
    {
    public:
        AsioTimer(AsioIoService &ioService) :
            mImpl(ioService)
        {}

        ASIO_NS::deadline_timer mImpl;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_ASIODEADLINETIMER_HPP
