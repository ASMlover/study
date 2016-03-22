
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

#ifndef RCF_TEST_TEST_HPP
#define RCF_TEST_TEST_HPP

#include <iostream>
#include <string>
#include <vector>
#include <string>

#include <RCF/util/VariableArgMacro.hpp>

#include <RCF/CustomAllocator.hpp>

#if defined(__GNUC__) && __GNUC__ >= 4 
    #ifdef RCF_TEST_BUILD_DLL
        #define RCF_TEST_EXPORT __attribute__((visibility("default")))
    #else
        #define RCF_TEST_EXPORT __attribute__((visibility("default")))
    #endif
#elif defined(__GNUC__)
    #ifdef RCF_TEST_BUILD_DLL
        #define RCF_TEST_EXPORT
    #else
        #define RCF_TEST_EXPORT
    #endif
#else
    #ifdef RCF_TEST_BUILD_DLL
        #define RCF_TEST_EXPORT __declspec(dllexport)
    #else
        #define RCF_TEST_EXPORT
    #endif
#endif

#if defined(RCF_TEST_BUILD_DLL) && defined(_MSC_VER) && !defined(_DLL)
#error "Error: DLL builds of RCF require dynamic runtime linking. Select one of the DLL options in Properties -> C/C++ -> Code Generation -> Runtime Library."
#endif

namespace RCF {

// Test hierarchies.
class RCF_TEST_EXPORT TestHierarchy
{
public:

                TestHierarchy();
    void        pushTestCase(const std::string & name);
    void        popTestCase();
    std::string currentTestCase();
    void        onlyRunTestCase(const std::string & testCase, bool caseSensitive);
    void        enumerateTestCasesOnly();
    bool        shouldCurrentTestCaseRun();
    bool        didTestCaseRun();

private:

    void        split(
                    const std::string & s, 
                    char delim, 
                    std::vector<std::string> & elems);

    bool        doesCurrentTestCaseMatch();

    std::vector<std::string> mCurrentTestCase;

    std::vector<std::string> mTestCaseToRun;
    bool mCaseSensitive;
    bool mHasTestCaseRun;

    bool mEnumerateOnly;
};

class RCF_TEST_EXPORT TestCaseSentry
{
public:
                TestCaseSentry(const std::string & name);
                TestCaseSentry(std::size_t n);
                ~TestCaseSentry();

    bool        shouldRun();
    void        setHasRun();

private:

    std::string mName;
    bool mRunnable;
    bool mHasRun;
};

#define TEST_CASE(name) TEST_CASE_IMPL(name, BOOST_PP_CAT(testCase, __LINE__))
#define TEST_CASE_IMPL(name, instName)  \
    for (RCF::TestCaseSentry instName((name)); instName.shouldRun(); instName.setHasRun())

class RCF_TEST_EXPORT TestEnv
{
public:
    TestEnv();

    void setTestCaseToRun(const std::string & testCase, bool caseSensitive = true);
    void setEnumerationOnly();
    void setAssertOnFail(bool assertOnFail);

    typedef void (*PfnAssert)(const char * file, int line, const char * condition , const char * info);
    void setPfnAssert(PfnAssert pfnAssert);

    std::size_t getFailCount();
    bool didTestCaseRun();

    void printTestMessage(const std::string & msg);

    void reportTestFailure(
        const char * file, 
        int line, 
        const char * condition,
        const char * info);

private:

    friend class TestCaseSentry;

    TestHierarchy mTestHierarchy;

    std::size_t mFailCount;

    bool mAssertOnFail;
    PfnAssert mPfnAssert;
};

RCF_TEST_EXPORT TestEnv & gTestEnv();

    std::string getRelativePathToCheckoutRoot();
    std::string getRelativeTestDataPath();
    std::string getWorkingDir();

} // namespace RCF

// RCF_CHECK

class RcfCheckFunctor : public RCF::VariableArgMacroFunctor
{
public:

    RcfCheckFunctor & setArgs(const char * file, int line, const char * cond)
    {
        mFile = file;
        mLine = line;
        mCond = cond;
        return *this;
    }

    ~RcfCheckFunctor()
    {
        std::string values(mArgs->str(), static_cast<std::size_t>(mArgs->tellp()));
        RCF::gTestEnv().reportTestFailure(mFile, mLine, mCond, values.c_str());
    }

private:
    const char *    mFile;
    int             mLine;
    const char *    mCond;
};

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4355 )  // warning C4355: 'this' : used in base member initializer list
#endif

DECLARE_VARIABLE_ARG_MACRO( RCF_CHECK, RcfCheckFunctor );

#define RCF_CHECK(cond)                                             \
    if (cond);                                                      \
    else                                                            \
        ::VariableArgMacro<RcfCheckFunctor>()                       \
            .setArgs(__FILE__, __LINE__, #cond)                     \
            .cast( (::VariableArgMacro<RcfCheckFunctor> *) NULL )   \
            .RCF_CHECK_A

#define RCF_CHECK_A(x)                         RCF_CHECK_OP(x, B)
#define RCF_CHECK_B(x)                         RCF_CHECK_OP(x, A)
#define RCF_CHECK_OP(x, next)                  RCF_CHECK_A.notify_((x), #x).RCF_CHECK_ ## next

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#define RCF_CHECK_EQ(a,b) RCF_CHECK(a == b)(a)(b)
#define RCF_CHECK_NEQ(a,b) RCF_CHECK(a != b)(a)(b)

#define RCF_CHECK_LT(a,b) RCF_CHECK(a < b)(a)(b)
#define RCF_CHECK_LTEQ(a,b) RCF_CHECK(a <= b)(a)(b)

#define RCF_CHECK_GT(a,b) RCF_CHECK(a > b)(a)(b)
#define RCF_CHECK_GTEQ(a,b) RCF_CHECK(a >= b)(a)(b)

#define RCF_CHECK_FAIL() RCF_CHECK(0)
#define RCF_CHECK_OK() RCF_CHECK(1)

#endif // ! RCF_TEST_TEST_HPP
