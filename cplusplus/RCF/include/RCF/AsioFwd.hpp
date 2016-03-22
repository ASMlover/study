
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

#ifndef INCLUDE_RCF_ASIOFWD_HPP
#define INCLUDE_RCF_ASIOFWD_HPP

#include <RCF/Config.hpp>

#include <boost/shared_ptr.hpp>

#if defined(__CYGWIN__) && !defined(RCF_USE_BOOST_ASIO)
#error RCF_USE_BOOST_ASIO must be defined when building on Cygwin.
#endif


namespace boost {
    namespace posix_time {
        class ptime;
    }
}

#ifdef RCF_USE_BOOST_ASIO

//------------------------------------------------------------------------------
// Use Boost.Asio

#define ASIO_NS boost::asio

namespace boost {
namespace asio {

    class io_service;

    namespace windows {
        template<typename T> class basic_stream_handle;

        class stream_handle_service;
        typedef basic_stream_handle<stream_handle_service> stream_handle;
    }

}
namespace system {
    class error_code;
}
}
namespace RCF {
    typedef boost::system::error_code               AsioErrorCode;
};

//------------------------------------------------------------------------------

#else

//------------------------------------------------------------------------------
// Use internal Asio

#define ASIO_NS asio

namespace asio {

    class io_service;
    class error_code;

    namespace windows {
        template<typename T> class basic_stream_handle;

        class stream_handle_service;
        typedef basic_stream_handle<stream_handle_service> stream_handle;
    }

}
namespace RCF {
    typedef asio::error_code                         AsioErrorCode;
};

//------------------------------------------------------------------------------

#endif

namespace RCF {

    typedef ASIO_NS::io_service                         AsioIoService;
    typedef boost::shared_ptr<AsioIoService>            AsioIoServicePtr;

    typedef ASIO_NS::windows::stream_handle             AsioPipeHandle;
    typedef boost::shared_ptr<AsioPipeHandle>           AsioPipeHandlePtr;

    class AsioTimer;
    typedef boost::shared_ptr<AsioTimer>                AsioTimerPtr;

    class                                               AsioBuffers;
    typedef boost::shared_ptr<AsioBuffers>              AsioBuffersPtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_ASIOFWD_HPP
