
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

#include <RCF/test/Test.hpp>

#include <RCF/Tools.hpp>
#include <RCF/Exception.hpp>

// getcwd
#ifdef _MSC_VER
#include <direct.h>
#else
#include <unistd.h>
#endif

// Boost string algo is more portable than stricmp, strcasecmp, etc.
#include <boost/algorithm/string.hpp>

#include <fstream>

namespace RCF {
bool compareNoCase(const std::string & s1, const std::string & s2)
{
    return 
            boost::to_upper_copy(s1) 
        ==  boost::to_upper_copy(s2);
}

TestEnv gTestEnv_;

TestEnv & gTestEnv()
{
    return gTestEnv_;
}

TestHierarchy::TestHierarchy() : 
    mCaseSensitive(false), 
    mHasTestCaseRun(false),
    mEnumerateOnly(false)
{
}

void TestHierarchy::pushTestCase(const std::string & name)
{
    mCurrentTestCase.push_back(name);
}

void TestHierarchy::popTestCase()
{
    mCurrentTestCase.pop_back();        
}

std::string TestHierarchy::currentTestCase()
{
    std::string s;
    for (std::size_t i=0; i<mCurrentTestCase.size(); ++i)
    {
        s += mCurrentTestCase[i];
        if (i != mCurrentTestCase.size() - 1)
        {
            s += '/';
        }
    }
    return s;
}

void TestHierarchy::onlyRunTestCase(const std::string & testCase, bool caseSensitive)
{
    split(testCase, '/', mTestCaseToRun);
    mCaseSensitive = caseSensitive;
}

void TestHierarchy::enumerateTestCasesOnly()
{
    mEnumerateOnly = true;
}

bool TestHierarchy::shouldCurrentTestCaseRun()
{
    if (mEnumerateOnly)
    {
        std::cout << "Test case: " << currentTestCase() << std::endl;

        bool match = doesCurrentTestCaseMatch();
        return match && mCurrentTestCase.size() <= mTestCaseToRun.size();
    }
    else if (mTestCaseToRun.empty())
    {
        return true;
    }
    else
    {
        bool matches = doesCurrentTestCaseMatch();
        if (matches && mTestCaseToRun.size() == mCurrentTestCase.size())
        {
            mHasTestCaseRun = true;
        }
        return matches;
    }
}

bool TestHierarchy::didTestCaseRun()
{
    return mHasTestCaseRun;
}

void TestHierarchy::split(
    const std::string & s, 
    char delim, 
    std::vector<std::string> & elems) 
{
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

bool TestHierarchy::doesCurrentTestCaseMatch()
{
    for (std::size_t i=0; i<mCurrentTestCase.size(); ++i)
    {
        if (i < mTestCaseToRun.size())
        {
            if (mCaseSensitive)
            {
                if (mCurrentTestCase[i] != mTestCaseToRun[i])
                {
                    return false;
                }

            }
            else
            {
                if (!compareNoCase(mCurrentTestCase[i], mTestCaseToRun[i]))
                {
                    return false;
                }
            }
        }
    }

    return true;
}


TestCaseSentry::TestCaseSentry(const std::string & name) : mName(name), mHasRun(false)
{
    TestHierarchy & th = gTestEnv().mTestHierarchy;
    th.pushTestCase(mName);
    mRunnable = th.shouldCurrentTestCaseRun();

    if (mRunnable)
    {
        std::cout << "Entering test case " << th.currentTestCase() << "." << std::endl;
    }
}

#if defined(_MSC_VER) && _MSC_VER == 1310
// C4267: 'argument' : conversion from 'size_t' to 'unsigned int', possible loss of data
#pragma warning(push)
#pragma warning( disable: 4267 ) 
#endif

TestCaseSentry::TestCaseSentry(std::size_t n) : mRunnable(false), mHasRun(false)
{
    TestHierarchy & th = gTestEnv().mTestHierarchy;

    std::ostringstream os;
    os << n;
    mName = os.str();

    th.pushTestCase(mName);

    mRunnable = th.shouldCurrentTestCaseRun();

    if (mRunnable)
    {
        std::cout << "Entering test case " << th.currentTestCase() << "." << std::endl;
    }
}

#if defined(_MSC_VER) && _MSC_VER == 1310
#pragma warning(pop)
#endif

TestCaseSentry::~TestCaseSentry()
{
    TestHierarchy & th = gTestEnv().mTestHierarchy;

    if (mRunnable)
    {
        std::cout << "Leaving test case " << th.currentTestCase() << "." << std::endl;
    }

    th.popTestCase();
}

bool TestCaseSentry::shouldRun()
{
    return mRunnable && !mHasRun;
}

void TestCaseSentry::setHasRun()
{
    mHasRun = true;
}

#ifdef BOOST_WINDOWS

void TestEnv::printTestMessage(const std::string & msg)
{
    std::cout << msg << std::flush;
    OutputDebugStringA(msg.c_str());
}

#else

void TestEnv::printTestMessage(const std::string & msg)
{
    std::cout << msg << std::flush;
}

#endif

void TestEnv::reportTestFailure(
    const char * file, 
    int line, 
    const char * condition,
    const char * info)
{
    std::ostringstream os;
    
    os 
        << file << "(" << line << "): Test case: " 
        << mTestHierarchy.currentTestCase() 
        << std::endl;

    os
        << file << "(" << line << "): Test failed: " 
        << condition;
    
    if (info)
    {
        os << " : " << info;
    }
    
    os << std::endl;

    printTestMessage( os.str() );
    
    ++mFailCount;

    if (mAssertOnFail)
    {
        if (mPfnAssert)
        {
            mPfnAssert(file, line, condition, info);
        }
        else
        {

#ifdef BOOST_WINDOWS
            // __debugbreak() is more likely to give a legible call stack.
            //DebugBreak();
            __debugbreak(); 
#else
            assert(0);
#endif

        }
    }
}

TestEnv::TestEnv() : mFailCount(0), mAssertOnFail(false), mPfnAssert(NULL)
{
}

void TestEnv::setTestCaseToRun(const std::string & testCase, bool caseSensitive)
{
    mTestHierarchy.onlyRunTestCase(testCase, caseSensitive);
}

void TestEnv::setEnumerationOnly()
{
    mTestHierarchy.enumerateTestCasesOnly();
}

void TestEnv::setAssertOnFail(bool assertOnFail)
{
    mAssertOnFail = assertOnFail;
}

void TestEnv::setPfnAssert(PfnAssert pfnAssert)
{
    mPfnAssert = pfnAssert;
}

std::size_t TestEnv::getFailCount()
{
    return mFailCount;
}

bool TestEnv::didTestCaseRun()
{
    return mTestHierarchy.didTestCaseRun();
}

std::string getRelativePathToCheckoutRoot()
{
    std::string testPath = ".";
    std::string testFile = testPath  + "/.hgtags";
    while (!std::ifstream(testFile.c_str()).good() && testPath.size() < 100)
    {
        testPath = testPath + "/..";
        testFile = testPath  + "/.hgtags";
    }

    if (std::ifstream(testFile.c_str()).good())
    {
        return testPath;
    }

    return "";
}

std::string getRelativeTestDataPath()
{
    std::string checkoutRoot = getRelativePathToCheckoutRoot();
    RCF_ASSERT(checkoutRoot.size() > 0)(checkoutRoot);

    std::string testDataPath = checkoutRoot + "/RCF/test/data/";
    return testDataPath;
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996) // warning C4996: 'getcwd': The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name: _getcwd. See online help for details.
#endif

std::string getWorkingDir()
{
    std::vector<char> vec(1024);
    char * szRet = getcwd(&vec[0], static_cast<int>( vec.size() ));
    RCF_UNUSED_VARIABLE(szRet);
    return std::string(&vec[0]);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace RCF
