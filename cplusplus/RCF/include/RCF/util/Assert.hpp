
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

#ifndef INCLUDE_UTIL_ASSERT_HPP
#define INCLUDE_UTIL_ASSERT_HPP

#include <cassert>
#include <cstdio>
#include <exception>

#include <boost/current_function.hpp>

#include "VariableArgMacro.hpp"

#if defined(_MSC_VER) && !defined(NDEBUG)
#include <crtdbg.h>
#endif

#include <RCF/Export.hpp>

namespace RCF {

    class RCF_EXPORT AssertFunctor : public VariableArgMacroFunctor
    {
    public:

        AssertFunctor();
        AssertFunctor(const char * expr);
        ~AssertFunctor();

        const char * mExpr;
    };

    class VarArgAbort
    {
    public:
        VarArgAbort();

        template<typename T>
        VarArgAbort &operator()(const T &)
        {
            return *this;
        }
    };


#if 0
#define UTIL_ASSERT_DEBUG(cond, e, logName, logLevel)                       \
    if (cond) ;                                                             \
    else RCF::VarArgAssert(__FILE__, __LINE__, #cond)
#endif

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4355 )  // warning C4355: 'this' : used in base member initializer list
#endif

#if defined(__GNUC__) && (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4))
#define UTIL_ASSERT_DEBUG_GCC_33_HACK (const RCF::VariableArgMacro<RCF::ThrowFunctor> &)
#else
#define UTIL_ASSERT_DEBUG_GCC_33_HACK
#endif

    DECLARE_VARIABLE_ARG_MACRO( UTIL_ASSERT_DEBUG, RCF::AssertFunctor );
    #define UTIL_ASSERT_DEBUG(expr, e, logName, logLevel)                                    \
        if (expr);    \
        else\
            UTIL_ASSERT_DEBUG_GCC_33_HACK                                              \
            RCF::VariableArgMacro<RCF::AssertFunctor>(#expr)    \
                .init(                                                          \
                    "",                                                         \
                    "",                                                         \
                    __FILE__,                                                   \
                    __LINE__,                                                   \
                    BOOST_CURRENT_FUNCTION)                                     \
                .cast( (RCF::VariableArgMacro<RCF::AssertFunctor> *) NULL)     \
                .UTIL_ASSERT_DEBUG_A



    #define UTIL_ASSERT_DEBUG_A(x)               UTIL_ASSERT_DEBUG_OP(x, B)
    #define UTIL_ASSERT_DEBUG_B(x)               UTIL_ASSERT_DEBUG_OP(x, A)
    #define UTIL_ASSERT_DEBUG_OP(x, next)        UTIL_ASSERT_DEBUG_A.notify_((x), #x).UTIL_ASSERT_DEBUG_ ## next

#ifdef _MSC_VER
#pragma warning( pop )
#endif




    #ifdef RCF_ALWAYS_ABORT_ON_ASSERT

    #define UTIL_ASSERT_RELEASE(cond, e, logName, logLevel)                     \
        if (cond) ;                                                             \
        else RCF::VarArgAbort()

    #else

    #define UTIL_ASSERT_RELEASE(cond, e, logName, logLevel)                     \
        if (cond) ;                                                             \
        else UTIL_THROW(e, logName, logLevel)(cond)

    #endif

    #define UTIL_ASSERT_NULL(cond, E)                                           \
        DUMMY_VARIABLE_ARG_MACRO()

    #ifdef NDEBUG
    #define UTIL_ASSERT UTIL_ASSERT_RELEASE
    #else
    #define UTIL_ASSERT UTIL_ASSERT_DEBUG
    #endif

} // namespace RCF

#endif //! INCLUDE_UTIL_ASSERT_HPP
