
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

#ifndef INCLUDE_RCF_TOOLS_HPP
#define INCLUDE_RCF_TOOLS_HPP

// Various utilities

#include <deque>
#include <stdexcept>
#include <typeinfo>
#include <vector>

#include <boost/config.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/util/UnusedVariable.hpp>
#include <RCF/util/VariableArgMacro.hpp>

// Logging mechanism
#include <RCF/util/Log.hpp>

namespace RCF {
    static const int LogNameRcf = 1;
    static const int LogLevel_1 = 1; // Error and exceptions.
    static const int LogLevel_2 = 2; // Larger scale setup/teardown.
    static const int LogLevel_3 = 3; // Messages sent and received (RCF level), RCF client and session lifetime.
    static const int LogLevel_4 = 4; // Messages sent and received (network level), network client and session lifetime.

} // namespace RCF

#define RCF_LOG_1() UTIL_LOG(RCF::LogNameRcf, RCF::LogLevel_1)
#define RCF_LOG_2() UTIL_LOG(RCF::LogNameRcf, RCF::LogLevel_2)
#define RCF_LOG_3() UTIL_LOG(RCF::LogNameRcf, RCF::LogLevel_3)
#define RCF_LOG_4() UTIL_LOG(RCF::LogNameRcf, RCF::LogLevel_4)

// Assertion mechanism
#ifndef NDEBUG

// Debug build asserts.
#include <RCF/util/Assert.hpp>
#define RCF_ASSERT(x) UTIL_ASSERT(x, RCF::AssertionFailureException(), RCF::LogNameRcf, RCF::LogLevel_1)

#define RCF_ASSERT_EQ(a,b)      RCF_ASSERT(a == b)(a)(b)
#define RCF_ASSERT_NEQ(a,b)     RCF_ASSERT(a != b)(a)(b)

#define RCF_ASSERT_LT(a,b)      RCF_ASSERT(a < b)(a)(b)
#define RCF_ASSERT_LTEQ(a,b)    RCF_ASSERT(a <= b)(a)(b)

#define RCF_ASSERT_GT(a,b)      RCF_ASSERT(a > b)(a)(b)
#define RCF_ASSERT_GTEQ(a,b)    RCF_ASSERT(a >= b)(a)(b)

#else

// Release build - strip out asserts.
#define RCF_ASSERT(x)           DUMMY_VARIABLE_ARG_MACRO()

#define RCF_ASSERT_EQ(a,b)      DUMMY_VARIABLE_ARG_MACRO()
#define RCF_ASSERT_NEQ(a,b)     DUMMY_VARIABLE_ARG_MACRO()

#define RCF_ASSERT_LT(a,b)      DUMMY_VARIABLE_ARG_MACRO()
#define RCF_ASSERT_LTEQ(a,b)    DUMMY_VARIABLE_ARG_MACRO()

#define RCF_ASSERT_GT(a,b)      DUMMY_VARIABLE_ARG_MACRO()
#define RCF_ASSERT_GTEQ(a,b)    DUMMY_VARIABLE_ARG_MACRO()

#endif

// Throw mechanism

namespace RCF {
    class Exception;
    RCF_EXPORT DummyVariableArgMacroObject rcfThrow(const char * szFile, int line, const char * szFunc, const Exception & e);
}

#ifndef NDEBUG

// Debug build throw - embed file and line info
#define RCF_THROW(e)            RCF::rcfThrow(__FILE__, __LINE__, __FUNCTION__, e)

#else

// Release build throw.
#define RCF_THROW(e)            RCF::rcfThrow(__FILE__, __LINE__, __FUNCTION__, e)
//#define RCF_THROW(e)            RCF::rcfThrow(__FILE__, __LINE__, "", e)
//#define RCF_THROW(e)            RCF::rcfThrow("", 0, "", e)
//#define RCF_THROW(e)            throw e;

#endif

// Verification mechanism
#define RCF_VERIFY(cond, e)     if (cond); else RCF_THROW(e)

// Scope guard mechanism
#include <boost/multi_index/detail/scope_guard.hpp>

namespace RCF {

    // null deleter, for use with for shared_ptr
    class NullDeleter
    {
    public:
        template<typename T>
        void operator()(T)
        {}
    };

    class SharedPtrIsNull
    {
    public:
        template<typename T>
        bool operator()(boost::shared_ptr<T> spt) const
        {
            return spt.get() == NULL;
        }
    };

} // namespace RCF

namespace RCF {

    RCF_EXPORT void rcfDtorCatchHandler(const std::exception & e);

} // namespace RCF

// destructor try/catch blocks
#define RCF_DTOR_BEGIN                              \
    try {

#define RCF_DTOR_END                                \
    }                                               \
    catch (const std::exception &e)                 \
    {                                               \
        RCF::rcfDtorCatchHandler(e);                \
    }

//#if defined(_MSC_VER) && _MSC_VER < 1310
//#define RCF_PFTO_HACK long
//#else
//#define RCF_PFTO_HACK
//#endif
#define RCF_PFTO_HACK

// Auto linking on VC++
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "crypt32.lib")
#endif

namespace RCF {

    struct Void {};

    template<typename Container, typename Element>
    void eraseRemove(Container & container, const Element & element)
    {
        container.erase(
            std::remove(
                container.begin(),
                container.end(),
                element),
            container.end());
    }

    RCF_EXPORT boost::uint64_t fileSize(const std::string & path);

} // namespace RCF

namespace boost {
    
    template<typename T>
    inline bool operator==(
        const boost::weak_ptr<T> & lhs, 
        const boost::weak_ptr<T> & rhs)
    {
        return ! (lhs < rhs) && ! (rhs < lhs);
    }

    template<typename T>
    inline bool operator!=(
        const boost::weak_ptr<T> & lhs, 
        const boost::weak_ptr<T> & rhs)
    {
        return ! (lhs == rhs);
    }

} // namespace boost

#endif // ! INCLUDE_RCF_TOOLS_HPP
