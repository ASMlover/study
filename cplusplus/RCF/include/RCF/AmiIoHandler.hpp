
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

#ifndef INCLUDE_RCF_AMIIOHANDLER_HPP
#define INCLUDE_RCF_AMIIOHANDLER_HPP

#include <RCF/Asio.hpp> // AsioErrorCode
#include <RCF/OverlappedAmi.hpp>

namespace RCF {

    class AmiIoHandler
    {
    public:

        AmiIoHandler(OverlappedAmiPtr overlappedPtr);

        AmiIoHandler(OverlappedAmiPtr overlappedPtr, const AsioErrorCode & ec);

        void operator()();

        void operator()(
            const AsioErrorCode & ec);

        void operator()(
            const AsioErrorCode & ec, 
            std::size_t bytesTransferred);

        OverlappedAmiPtr mOverlappedPtr;
        std::size_t mIndex;

        AsioErrorCode mEc;
    };

    class AmiTimerHandler
    {
    public:

        AmiTimerHandler(OverlappedAmiPtr overlappedPtr);

        void operator()(const AsioErrorCode & ec);

        OverlappedAmiPtr mOverlappedPtr;
        std::size_t mIndex;
    };

    void *  asio_handler_allocate(std::size_t size, AmiIoHandler * pHandler);
    void    asio_handler_deallocate(void * pointer, std::size_t size, AmiIoHandler * pHandler);
    void *  asio_handler_allocate(std::size_t size, AmiTimerHandler * pHandler);
    void    asio_handler_deallocate(void * pointer, std::size_t size, AmiTimerHandler * pHandler);

} // namespace RCF

#endif // ! INCLUDE_RCF_AMIIOHANDLER_HPP
