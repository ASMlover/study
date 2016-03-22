
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

#ifndef INCLUDE_RCF_TEST_TESTMINIMAL_HPP
#define INCLUDE_RCF_TEST_TESTMINIMAL_HPP

#ifdef RCF_VLD
#include <vld.h>
#endif

// Include valarray early so it doesn't get trampled by min/max macro definitions.
#if defined(_MSC_VER) && _MSC_VER == 1310 && RCF_FEATURE_BOOST_SERIALIZATION==1
#include <valarray>
#endif

// For msvc-7.1, prevent including <locale> from generating warnings.
#if defined(_MSC_VER) && _MSC_VER == 1310
#pragma warning( disable : 4995 ) //  'func': name was marked as #pragma deprecated
#include <locale>
#pragma warning( default : 4995 )
#endif

#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/Tools.hpp>

#include <RCF/test/ProgramTimeLimit.hpp>
#include <RCF/test/Test.hpp>

#include <RCF/util/Platform/OS/BsdSockets.hpp>
#include <RCF/util/Log.hpp>

#include <iostream>
#include <sstream>

#ifdef _MSC_VER
#include <RCF/test/MiniDump.hpp>
#endif

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

// Test custom allocation support, if applicable.
#if RCF_FEATURE_CUSTOM_ALLOCATOR==1
#if defined(_MSC_VER) && !defined(NDEBUG)
#include <RCF/test/AllocationHookCRT.hpp>
#endif
#endif

int gProcessReturnValue = 0;

#if defined(RCF_VLD) && !defined(NDEBUG)

int vldLeakReporter(int reportType, wchar_t * message, int * returnValue)
{
    RCF_UNUSED_VARIABLE(reportType);
    RCF_UNUSED_VARIABLE(message);

    // We allow one leak (for the root mutex).
    int leakCount = VLDGetLeaksCount();
    if (leakCount > 1)
    {
        std::wcout << L"VLD has detected " << leakCount << L" leaks. Call stacks follow." << std::endl;
        VLDSetReportHook(VLD_RPTHOOK_REMOVE, vldLeakReporter);
        VLDReportLeaks();
        *returnValue = gProcessReturnValue + leakCount;
        exit(*returnValue);
    }
    else
    {
        *returnValue = gProcessReturnValue;
        exit(*returnValue);
    }
    return 0;
}

#endif // defined(RCF_VLD) && !defined(NDEBUG)

int test_main(int argc, char **argv);

int main(int argc, char **argv)
{

#ifdef RCF_VLD
    VLDSetReportHook(VLD_RPTHOOK_INSTALL, vldLeakReporter);
#endif

    Platform::OS::BsdSockets::disableBrokenPipeSignals();

    RCF::RcfInitDeinit rcfInit;

    RCF::Timer testTimer;

    std::cout << "Commandline: ";
    for (int i=0; i<argc; ++i)
    {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;

    bool shouldNotCatch = false;

#ifndef NDEBUG
    unsigned int defaultTimeLimit = 0;
#else
    unsigned int defaultTimeLimit = 5 * 60;
#endif

    {
        RCF::CommandLineOption<std::string>    clTestCase( "testcase",     "",     "Run a specific test case.");
        RCF::CommandLineOption<bool>           clListTests("list",         false,  "List all test cases.");
        RCF::CommandLineOption<bool>           clAssert(   "assert",       false,  "Enable assert popups, and assert on test failures.");
        RCF::CommandLineOption<int>            clLogLevel( "loglevel",     1,      "Set RCF log level.");
        RCF::CommandLineOption<std::string>    clLogFormat("logformat",    "",     "Set RCF log format.");
        RCF::CommandLineOption<std::string>    clLogFile("logfile",        "", "Set RCF log file.");
        RCF::CommandLineOption<bool>           clNoCatch(  "nocatch",      false,  "Don't catch exceptions at top level.");
        RCF::CommandLineOption<unsigned int>   clTimeLimit("timelimit",     defaultTimeLimit, "Set program time limit in seconds. 0 to disable.");

#ifdef _MSC_VER
        RCF::CommandLineOption<bool>           clMinidump("minidump",      true,   "Enable minidump creation.");
#endif

        bool exitOnHelp = false;
        RCF::CommandLine::getSingleton().parse(argc, argv, exitOnHelp);

        // -testcase
        std::string testCase = clTestCase.get();
        if (!testCase.empty())
        {
            RCF::gTestEnv().setTestCaseToRun(testCase);
        }

        // -list
        bool list = clListTests.get();
        if (list)
        {
            RCF::gTestEnv().setEnumerationOnly();
        }

        // -assert
        bool assertOnFail = clAssert.get();
        RCF::gTestEnv().setAssertOnFail(assertOnFail);

#ifdef BOOST_WINDOWS
        if (!assertOnFail)
        {
            // Try to prevent those pesky crash dialogs from popping up.

            DWORD dwFlags = SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS;
            DWORD dwOldFlags = SetErrorMode(dwFlags);
            SetErrorMode(dwOldFlags | dwFlags);

#ifdef _MSC_VER
            // Disable CRT asserts.
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
            _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
            _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG); 
#endif

        }
#endif


        // -loglevel
        int logName = RCF::LogNameRcf;
        int logLevel = clLogLevel.get();

        std::string logFormat = clLogFormat.get();
        if (logFormat.empty())
        {
            logFormat = "%E(%F): [Thread: %D][Time: %H] %X";
        }

#ifdef BOOST_WINDOWS
        RCF::LoggerPtr loggerPtr(new RCF::Logger(logName, logLevel, RCF::LogToDebugWindow(), logFormat) );
        loggerPtr->activate();
#else
        RCF::LoggerPtr loggerPtr(new RCF::Logger(logName, logLevel, RCF::LogToStdout(), logFormat) );
        loggerPtr->activate();
#endif

        std::string logFile = clLogFile.get();
        if ( logFile.size() > 0 )
        {
            RCF::LoggerPtr loggerPtr(new RCF::Logger(logName, logLevel, RCF::LogToFile(logFile, true), logFormat));
            loggerPtr->activate();
        }

        // -minidump
#if defined(_MSC_VER)
        bool enableMinidumps = clMinidump.get();
        if (enableMinidumps)
        {
            setMiniDumpExceptionFilter();
        }
#endif

        // -timelimit
        unsigned int timeLimitS = clTimeLimit.get();
        gpProgramTimeLimit = new ProgramTimeLimit(timeLimitS);

        shouldNotCatch = clNoCatch.get();
    }

    int ret = 0;
    
    bool shouldCatch = !shouldNotCatch;
    if (shouldCatch)
    {
        try
        {
            ret = test_main(argc, argv);
        }
        catch(const RCF::RemoteException & e)
        {
            std::cout << "Caught top-level exception (RCF::RemoteException): " << e.getErrorString() << std::endl;
            RCF_CHECK(1==0);
        }
        catch(const RCF::Exception & e)
        {
            std::cout << "Caught top-level exception (RCF::Exception): " << e.getErrorString() << std::endl;
            RCF_CHECK(1==0);
        }
        catch(const std::exception & e)
        {
            std::cout << "Caught top-level exception (std::exception): " << e.what() << std::endl;
            RCF_CHECK(1==0);
        }
        catch (...)
        {
            std::cout << "Caught top-level exception (...)" << std::endl;
            RCF_CHECK(1==0);
        }
    }
    else
    {
        ret = test_main(argc, argv);
    }

    std::string exitMsg;
    std::size_t failCount = RCF::gTestEnv().getFailCount();
    if (failCount)
    {
        std::ostringstream os;
        os << "*** Test Failures: " << failCount << " ***" << std::endl;
        exitMsg = os.str();
    }
    else
    {
        exitMsg = "*** All Tests Passed ***\n";
    }

    RCF::gTestEnv().printTestMessage(exitMsg);

    // Print out how long the test took.
    boost::uint32_t durationMs = testTimer.getDurationMs();
    std::cout << "Time elapsed: " << durationMs/1000 << " (s)" << std::endl;

    RCF::deinit();

    // Delete the time limit object and its thread, otherwise VLD reports allocations.
    delete gpProgramTimeLimit;
    gpProgramTimeLimit = NULL;

    gProcessReturnValue = ret + static_cast<int>(failCount);
    return gProcessReturnValue;
}

// Minidump creation code, for Visual C++ 2003 and later.
#if defined(_MSC_VER)
#include <RCF/test/MiniDump.cpp>
#endif

#include <RCF/../../src/RCF/test/Test.cpp>

#endif // ! INCLUDE_RCF_TEST_TESTMINIMAL_HPP
