
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

#define RCF_VERSION_MAJOR                   2
#define RCF_VERSION_MINOR                   1
#define RCF_VERSION_MINOR_MINOR             0
#define RCF_VERSION_BUILD                   0
#define CAT_HELPER_1(s) #s
#define CAT_HELPER_2(s) CAT_HELPER_1(s)

#define RCF_VERSION_STR_BASE                            \
    CAT_HELPER_2(RCF_VERSION_MAJOR)         "."         \
    CAT_HELPER_2(RCF_VERSION_MINOR)         "."         \
    CAT_HELPER_2(RCF_VERSION_MINOR_MINOR)   "."         \
    CAT_HELPER_2(RCF_VERSION_BUILD)

#ifdef NDEBUG
#define RCF_VERSION_STR RCF_VERSION_STR_BASE 
#else
#define RCF_VERSION_STR RCF_VERSION_STR_BASE " Debug"
#endif
