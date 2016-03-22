
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

#include <RCF/Tools.hpp>

#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/util/Log.hpp>
#include <RCF/ThreadLibrary.hpp>

#include <sys/types.h>
#include <sys/stat.h>

namespace RCF {

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4995) // 'sprintf': name was marked as #pragma deprecated
#pragma warning(disable: 4996) // 'sprintf': This function or variable may be unsafe.
#endif

    DummyVariableArgMacroObject rcfThrow(const char * szFile, int line, const char * szFunc, const Exception & e)
    {
        std::string context = szFile;
        context += ":";
        char szBuffer[32] = {0};
        sprintf(szBuffer, "%d", line);
        context += szBuffer;
        const_cast<Exception&>(e).setContext(context);

        if (RCF::LogManager::instance().isEnabled(LogNameRcf, LogLevel_1))
        {
            RCF::LogEntry entry(LogNameRcf, LogLevel_1, szFile, line, szFunc);

            entry
                << "RCF exception thrown. Error message: "
                << e.getErrorString();
        }

        e.throwSelf();

        return DummyVariableArgMacroObject();
    }

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

}

namespace RCF {

    std::string toString(const std::exception &e)
    {
        MemOstream os;

        const RCF::Exception *pE = dynamic_cast<const RCF::Exception *>(&e);
        if (pE)
        {
            int err = pE->getErrorId();
            std::string errMsg = pE->getErrorString();
            os << "[RCF: " << err << ": " << errMsg << "]";
        }
        else
        {
            os << "[What: " << e.what() << "]" ;
        }

        return os.string();
    }

    // Generate a timeout value for the given ending time.
    // Returns zero if endTime <= current time <= endTime+10% of timer resolution,
    // otherwise returns a nonzero duration in ms.
    // Timer resolution as above (49 days).
    boost::uint32_t generateTimeoutMs(unsigned int endTimeMs)
    {
        // 90% of the timer interval
        boost::uint32_t currentTimeMs = getCurrentTimeMs();
        boost::uint32_t timeoutMs = endTimeMs - currentTimeMs;
        return (timeoutMs <= MaxTimeoutMs) ? timeoutMs : 0;
    }

#ifdef BOOST_WINDOWS

    boost::uint64_t fileSize(const std::string & path)
    {
        struct _stat fileInfo = {0};
        int ret = _stat(path.c_str(), &fileInfo);
        RCF_VERIFY(ret == 0, Exception(_RcfError_FileOpen(path)));
        return fileInfo.st_size;
    }

#else

    boost::uint64_t fileSize(const std::string & path)
    {
        struct stat fileInfo = { 0 };
        int ret = stat(path.c_str(), &fileInfo);
        RCF_VERIFY(ret == 0, Exception(_RcfError_FileOpen(path)));
        return fileInfo.st_size;
    }

#endif

    void rcfDtorCatchHandler(const std::exception & e)
    {
        if (!std::uncaught_exception())
        {
            throw;
        }
        else
        {
            RCF_LOG_1()(e);
        }
    }

} // namespace RCF
