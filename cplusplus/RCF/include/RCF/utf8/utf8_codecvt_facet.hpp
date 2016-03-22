
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

#ifndef RCF_BOOST_PROGRAM_OPTIONS_UTF8_CODECVT_FACET_HPP
#define RCF_BOOST_PROGRAM_OPTIONS_UTF8_CODECVT_FACET_HPP

//#include <boost/program_options/config.hpp>
#include <RCF/Export.hpp>

#define BOOST_UTF8_BEGIN_NAMESPACE \
    namespace RcfBoost { namespace boost { namespace program_options { namespace detail {

#define BOOST_UTF8_END_NAMESPACE }}}}
#define BOOST_UTF8_DECL RCF_EXPORT

#include "detail/utf8_codecvt_facet.hpp"

#undef BOOST_UTF8_BEGIN_NAMESPACE
#undef BOOST_UTF8_END_NAMESPACE
#undef BOOST_UTF8_DECL

#endif
