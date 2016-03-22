
#include <RCF/test/TestMinimal.hpp>

#include <iostream>
#include <iomanip>

#include <boost/algorithm/string/case_conv.hpp>

#include "TestRCF.hpp"

#if defined(_MSC_VER) && !defined(NDEBUG)
// For MSVC debug builds, we use CRT allocation hooks, which will catch all forms of allocations.
#include <RCF/test/AllocationHookCRT.hpp>
#else
// Otherwise we override global operator new and friends.
#include <RCF/test/AllocationHook.hpp>
#endif

TestCases * gpTestCases = NULL;

TestCases& getAllTestCases()
{
    if (gpTestCases == NULL)
    {
        gpTestCases = new TestCases();
    }

    // Debugger seems to be happier with this.
    TestCases& testCases = *gpTestCases;
    return testCases;
}

TestCase::TestCase(const std::string & testName, const std::string & testDescription) : 
    mName(testName),
    mDescription(testDescription),
    mArgc(0), 
    mArgv(NULL)
{
    getAllTestCases()[testName] = this;
}

TestCase::~TestCase()
{
}

std::string TestCase::getName() const
{
    return mName;
}

std::string TestCase::getDescription() const
{
    return mDescription;
}

int test_main(int argc, char **argv)
{
    RCF::CommandLineOption<std::string>    clTest( "test"      , ""        , "Which test to run." );
    RCF::CommandLineOption<bool>           clList( "list"      , false     , "List all tests by name." );

    bool exitOnHelp = false;
    RCF::CommandLine::getSingleton().parse(argc, argv, exitOnHelp);

    TestCases& testCases = getAllTestCases();

    bool shouldList = clList.get();
    if (shouldList)
    {
        std::cout << "Listing all tests by name:" << std::endl << std::endl;;

        for (TestCases::iterator iter = testCases.begin(); iter != testCases.end(); ++iter)
        {
            TestCase& testCase = *iter->second;
            std::cout << "Test: " << std::left << std::setw(20) << testCase.getName();

            std::string desc = testCase.getDescription();
            if (desc.empty())
            {
                desc = "<NO DESCRIPTION>";
            }

            std::cout << desc << std::endl ;
        }
        return 0;
    }

    RCF::initializeTransportFactories();

    std::string whichTest = clTest.get();

    if (testCases.empty())
    {
        std::cout << "No test cases available." << std::endl;
        return 1;
    }

    if (whichTest.empty())
    {
        if (testCases.size() == 1)
        {
            whichTest = testCases.begin()->second->getName();
        }
        else
        {
            RCF_ASSERT(testCases.size() > 1);
            whichTest = "minimal";
        }
    }

    RCF_ASSERT(whichTest.size() > 0);
    TestCases::iterator iter = testCases.find(whichTest);
    if (iter == testCases.end())
    {
        std::cout << "Test case \"" << whichTest << "\" does not exist." << std::endl;
        return 1;
    }

    TestCase& testCase = *iter->second;
    testCase.run(argc, argv);

    testCases.clear();

    delete gpTestCases;
    gpTestCases = NULL;

    return 0;
}

namespace RCF {
    std::string getFilterName(int filterId)
    {
        switch (filterId)
        {
        case RcfFilter_Unknown                      : return "Unknown";
        case RcfFilter_Identity                     : return "Identity";
        case RcfFilter_OpenSsl                      : return "OpenSSL";
        case RcfFilter_ZlibCompressionStateless     : return "Zlib stateless";
        case RcfFilter_ZlibCompressionStateful      : return "Zlib stateful";
        case RcfFilter_SspiNtlm                     : return "NTLM";
        case RcfFilter_SspiKerberos                 : return "Kerberos";
        case RcfFilter_SspiNegotiate                : return "Negotiate";
        case RcfFilter_SspiSchannel                 : return "Schannel";
        case RcfFilter_Xor                          : return "Xor";
        default                                     : return "Unknown";
        }
    }

    bool isFilterRemovable(int filterId)
    {
        switch (filterId)
        {
        case RcfFilter_Unknown                      : return true;
        case RcfFilter_Identity                     : return true;
        case RcfFilter_OpenSsl                      : return true;
        case RcfFilter_ZlibCompressionStateless     : return false;
        case RcfFilter_ZlibCompressionStateful      : return false;
        case RcfFilter_SspiNtlm                     : return true;
        case RcfFilter_SspiKerberos                 : return true;
        case RcfFilter_SspiNegotiate                : return true;
        case RcfFilter_SspiSchannel                 : return true;
        case RcfFilter_Xor                          : return true;
        default                                     : return true;
        }
    }
}

#include <RCF/test/TransportFactories.cpp>
#include <RCF/test/Usernames.cpp>

#ifdef BOOST_WINDOWS
#include <RCF/test/StackWalker.cpp>
#endif
