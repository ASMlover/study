
#ifndef INCLUDE_TESTRCF_HPP
#define INCLUDE_TESTRCF_HPP

#include <string>

#include <boost/algorithm/string/case_conv.hpp>

#include <RCF/test/PrintTestHeader.hpp>
#include <RCF/test/Test.hpp>
#include <RCF/test/TransportFactories.hpp>

#include <RCF/util/CommandLine.hpp>




#include <RCF/InitDeinit.hpp>

#include <RCF/TcpServerTransport.hpp>
#include <RCF/Asio.hpp>
#include <RCF/Config.hpp>

#ifdef RCF_HAS_LOCAL_SOCKETS
#include <RCF/UnixLocalClientTransport.hpp>
#include <RCF/UnixLocalServerTransport.hpp>
#endif

#if RCF_FEATURE_NAMEDPIPE==1
#include <RCF/Win32NamedPipeClientTransport.hpp>
#include <RCF/Win32NamedPipeEndpoint.hpp>
#include <RCF/Win32NamedPipeServerTransport.hpp>
#endif

#include <RCF/TcpClientTransport.hpp>
#include <RCF/UdpClientTransport.hpp>
#include <RCF/UdpServerTransport.hpp>

class CompareNoCase
{
public:
    bool operator()(const std::string & lhs, const std::string & rhs) const
    {
        //int ret = stricmp(lhs.c_str(), rhs.c_str());
        return boost::to_upper_copy(lhs) < boost::to_upper_copy(rhs);
    }
};

class TestCase;
typedef std::map<std::string, TestCase *, CompareNoCase> TestCases;

TestCases& getAllTestCases();

class TestCase
{
public:
    TestCase(const std::string & testName, const std::string & testDescription);
    virtual ~TestCase();

    std::string     getName() const;
    std::string     getDescription() const;
    virtual void    run(int argc, char ** argv) = 0;

    std::string     mName;
    std::string     mDescription;

    int             mArgc;
    char **         mArgv;
};

namespace RCF {
    std::string getFilterName(int filterId);
    bool isFilterRemovable(int filterId);
}

extern bool gInstrumented;
extern bool gExpectAllocations;
extern std::size_t gnAllocations;

#endif // ! INCLUDE_TESTRCF_HPP
