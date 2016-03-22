
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

#include <boost/config.hpp>
#include <boost/version.hpp>

#ifdef _MSC_VER
#pragma warning(disable: 4267) // warning C4267: 'argument' : conversion from 'size_t' to 'DWORD', possible loss of data
#pragma warning(disable: 4244) // warning C4244: 'argument' : conversion from '__int64' to 'int', possible loss of data
#endif

#if BOOST_VERSION <= 104300

#include <boost/filesystem/../../libs/filesystem/src/path.cpp>
#include <boost/filesystem/../../libs/filesystem/src/operations.cpp>
#include <boost/filesystem/../../libs/filesystem/src/portability.cpp>

#elif BOOST_VERSION <= 104500

#include <boost/filesystem/../../libs/filesystem/v2/src/v2_path.cpp>
#include <boost/filesystem/../../libs/filesystem/v2/src/v2_operations.cpp>
#include <boost/filesystem/../../libs/filesystem/v2/src/v2_portability.cpp>

#elif BOOST_VERSION <= 104900

#include <boost/filesystem/../../libs/filesystem/v3/src/codecvt_error_category.cpp>

#define dot dot_1
#define dot_path dot_path_1
#define dot_dot_path dot_dot_path_1

#include <boost/filesystem/../../libs/filesystem/v3/src/operations.cpp>

#undef dot
#undef dot_path
#undef dot_dot_path

#define dot dot_2
#define dot_path dot_path_2
#define dot_dot_path dot_dot_path_2

#include <boost/filesystem/../../libs/filesystem/v3/src/path.cpp>

#define default_codecvt_buf_size default_codecvt_buf_size_2

#include <boost/filesystem/../../libs/filesystem/v3/src/path_traits.cpp>

#include <boost/filesystem/../../libs/filesystem/v3/src/portability.cpp>
#include <boost/filesystem/../../libs/filesystem/v3/src/unique_path.cpp>
#include <boost/filesystem/../../libs/filesystem/v3/src/utf8_codecvt_facet.cpp>
#include <boost/filesystem/../../libs/filesystem/v3/src/windows_file_codecvt.cpp>

#else // 1.50.0+

#include <boost/filesystem/../../libs/filesystem/src/codecvt_error_category.cpp>

#define dot dot_1
#define dot_path dot_path_1
#define dot_dot_path dot_dot_path_1

#include <boost/filesystem/../../libs/filesystem/src/operations.cpp>

#undef dot
#undef dot_path
#undef dot_dot_path

#define dot dot_2
#define dot_path dot_path_2
#define dot_dot_path dot_dot_path_2

#include <boost/filesystem/../../libs/filesystem/src/path.cpp>

#define default_codecvt_buf_size default_codecvt_buf_size_2

#include <boost/filesystem/../../libs/filesystem/src/path_traits.cpp>

#include <boost/filesystem/../../libs/filesystem/src/portability.cpp>
#include <boost/filesystem/../../libs/filesystem/src/unique_path.cpp>
#include <boost/filesystem/../../libs/filesystem/src/utf8_codecvt_facet.cpp>
#include <boost/filesystem/../../libs/filesystem/src/windows_file_codecvt.cpp>

#endif
