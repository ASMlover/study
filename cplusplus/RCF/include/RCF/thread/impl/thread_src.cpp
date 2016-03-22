
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

#include <RCF/Exception.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/Tools.hpp>

#include <boost/config.hpp>

#ifdef BOOST_WINDOWS
#include <RCF/thread/impl/win_event.ipp>
#include <RCF/thread/impl/win_mutex.ipp>
#include <RCF/thread/impl/win_thread.ipp>
#include <RCF/thread/impl/win_tss_ptr.ipp>
#else
#include <RCF/thread/impl/posix_event.ipp>
#include <RCF/thread/impl/posix_mutex.ipp>
#include <RCF/thread/impl/posix_thread.ipp>
#include <RCF/thread/impl/posix_tss_ptr.ipp>
#endif
