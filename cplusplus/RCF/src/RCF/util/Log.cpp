
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

#include <RCF/util/Log.hpp>

#include <RCF/ByteBuffer.hpp>
#include <RCF/Exception.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/Tools.hpp>
#include <RCF/util/Assert.hpp>

#include <stdlib.h>
#include <time.h>

#ifndef BOOST_WINDOWS
#include <sys/time.h>
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4995) // 'sprintf': name was marked as #pragma deprecated
#pragma warning(disable: 4996) // 'sprintf'/'localtime': This function or variable may be unsafe.
#endif

namespace RCF {

    LoggerPtr gRcfLoggerPtr;

    void enableLogging(
        const LogTarget &       logTarget, 
        int                     logLevel, 
        const std::string &     logFormat)
    {
        Lock lock(LogManager::instance().DefaultLoggerPtrMutex);

        LoggerPtr & defaultLoggerPtr = LogManager::instance().DefaultLoggerPtr;

        if (defaultLoggerPtr)
        {
            defaultLoggerPtr->deactivate();
        }

        defaultLoggerPtr.reset();

        defaultLoggerPtr.reset( new RCF::Logger(
            RCF::LogNameRcf, 
            logLevel, 
            logTarget,
            logFormat) );

        defaultLoggerPtr->activate();
    }

    void disableLogging()
    {
        Lock lock(LogManager::instance().DefaultLoggerPtrMutex);

        LoggerPtr & defaultLoggerPtr = LogManager::instance().DefaultLoggerPtr;

        if (defaultLoggerPtr)
        {
            defaultLoggerPtr->deactivate();
        }

        defaultLoggerPtr.reset();
    }

    void printToOstream(RCF::MemOstream & os, boost::uint16_t n)
    {
        char buffer[50] = {0};
        sprintf(buffer, "%u", boost::uint32_t(n));
        os << buffer;
    }

    void printToOstream(RCF::MemOstream & os, boost::uint32_t n)
    {
        char buffer[50] = {0};
        sprintf(buffer, "%u", n);
        os << buffer;
    }

    void printToOstream(RCF::MemOstream & os, boost::uint64_t n)
    {
        char buffer[50] = {0};

#ifdef _MSC_VER
        sprintf(buffer, "%llu", n);
#else
        sprintf(buffer, "%llu", (long long unsigned int) n);
#endif
        os << buffer;
    }

    LogManager * gpLogManager;

    LogManager::LogManager() : 
        mLoggersMutex(WriterPriority), 
        DefaultLogFormat("%E(%F): [Thread: %D][Time: %H] %X")
    {
    }

    LogManager::~LogManager()
    {
    }

    void LogManager::init()
    {
        gpLogManager = new LogManager();
    }
    
    void LogManager::deinit()
    {
        delete gpLogManager;
        gpLogManager = NULL;
    }
    
    LogManager & LogManager::instance()
    {
        if (!gpLogManager)
        {
            throw Exception(_RcfError_RcfNotInitialized());
        }
        return *gpLogManager;
    }

    void LogManager::deactivateAllLoggers()
    {
        RCF::WriteLock lock(mLoggersMutex);

        mLoggers.clear();
    }

    void LogManager::deactivateAllLoggers(int name)
    {
        RCF::WriteLock lock(mLoggersMutex);

        Loggers::iterator iter = mLoggers.find(name);
        if (iter != mLoggers.end())
        {
            mLoggers.erase(iter);
        }
    }

    void LogManager::activateLogger(LoggerPtr loggerPtr)
    {
        RCF::WriteLock lock(mLoggersMutex);

        int name = loggerPtr->getName();
        std::vector<LoggerPtr> & loggers = mLoggers[name];
        loggers.push_back(loggerPtr);
    }

    void LogManager::deactivateLogger(LoggerPtr loggerPtr)
    {
        RCF::WriteLock lock(mLoggersMutex);

        int name = loggerPtr->getName();
        Loggers::iterator iter = mLoggers.find(name);
        if (iter != mLoggers.end())
        {
            std::vector<LoggerPtr> & loggers = iter->second;
            RCF::eraseRemove(loggers, loggerPtr);
            if (loggers.empty())
            {
                mLoggers.erase(iter);
            }
        }
    }

    bool LogManager::isLoggerActive(LoggerPtr loggerPtr)
    {
        RCF::ReadLock lock(mLoggersMutex);

        int name = loggerPtr->getName();
        Loggers::iterator iter = mLoggers.find(name);
        if (iter != mLoggers.end())
        {
            std::vector<LoggerPtr> & loggers = iter->second;
            return std::find(loggers.begin(), loggers.end(), loggerPtr) != loggers.end();
        }
        return false;
    }

    void LogManager::writeToLoggers(const LogEntry & logEntry)
    {
        RCF::ReadLock lock(mLoggersMutex);

        int name = logEntry.mName;
        int level = logEntry.mLevel;

        Loggers::iterator iter = mLoggers.find(name);
        if (iter != mLoggers.end())
        {
            std::vector<LoggerPtr> & loggers = iter->second;
            for (std::size_t i=0; i<loggers.size(); ++i)
            {
                if (loggers[i]->getLevel() >= level)
                {
                    loggers[i]->write(logEntry);
                }
            }
        }
    }

    bool LogManager::isEnabled(int name, int level)
    {
        RCF::ReadLock lock(mLoggersMutex);

        Loggers::iterator iter = mLoggers.find(name);
        if (iter != mLoggers.end())
        {
            std::vector<LoggerPtr> & loggers = iter->second;
            for (std::size_t i=0; i<loggers.size(); ++i)
            {
                if (loggers[i]->getLevel() >= level)
                {
                    return true;
                }
            }
        }
        return false;
    }

    LogToStdout::LogToStdout(bool flushAfterEachWrite) : mFlush(flushAfterEachWrite)
    {
    }

    LogTarget * LogToStdout::clone() const
    {
        return new LogToStdout(*this);
    }

    Mutex LogToStdout::sIoMutex;

    void LogToStdout::write(const RCF::ByteBuffer & output)
    {
        output.getPtr()[output.getLength() - 2] = '\n';

        {
            Lock lock(sIoMutex);
            fwrite(output.getPtr(), sizeof(char), output.getLength() - 1, stdout);
            if (mFlush)
            {
                fflush(stdout);
            }
        }

        output.getPtr()[output.getLength() - 2] = '\0';
    }

#ifdef BOOST_WINDOWS

    LogTarget * LogToDebugWindow::clone() const
    {
        return new LogToDebugWindow(*this);
    }

    void LogToDebugWindow::write(const RCF::ByteBuffer & output)
    {
        output.getPtr()[output.getLength() - 2] = '\n';
        OutputDebugStringA(output.getPtr());
        output.getPtr()[output.getLength() - 2] = '\0';
    }

    LogToEventLog::LogToEventLog(const std::string & appName, int eventLogLevel) : 
        mhEventLog(NULL), 
        mAppName(appName),
        mEventLogLevel(eventLogLevel)
    {
        mhEventLog = RegisterEventSourceA(NULL, mAppName.c_str());
        if (!mhEventLog)
        {
            // TODO: error handling.
        }
    }

    LogToEventLog::~LogToEventLog()
    {
        DeregisterEventSource(mhEventLog);
    }

    LogTarget * LogToEventLog::clone() const
    {
        return new LogToEventLog(mAppName, mEventLogLevel);
    }

    void LogToEventLog::write(const RCF::ByteBuffer & output)
    {
        const char * parms[1] = { output.getPtr() };

        BOOL ret = ReportEventA(
            mhEventLog, 
            (WORD) mEventLogLevel, 
            0, 
            0, 
            0, 
            1, 
            0, 
            parms, 
            NULL);

        RCF_UNUSED_VARIABLE(ret);
    }

#endif

    LogToFile::LogToFile(const std::string & filePath, bool flushAfterEachWrite) : 
        mFilePath(filePath), 
        mOpened(false), 
        mFp(NULL),
        mFlush(flushAfterEachWrite)
    {
    }

    LogToFile::LogToFile(const LogToFile & rhs) : 
        mFilePath(rhs.mFilePath), 
        mOpened(false), 
        mFp(NULL),
        mFlush(rhs.mFlush)
    {
    }

    LogToFile::~LogToFile()
    {
        if (mFp)
        {
            fclose(mFp);
            mFp = NULL;
        }
    }

    LogTarget * LogToFile::clone() const
    {
        return new LogToFile(*this);
    }

    void LogToFile::write(const RCF::ByteBuffer & output)
    {
        Lock lock(mMutex);

        if (!mOpened)
        {
            mFp = fopen(mFilePath.c_str(), "a");
            if (!mFp)
            {
                std::string errMsg = "Unable to open log file for appending. File: " + mFilePath;
                throw std::runtime_error(errMsg);
            }
            mOpened = true;
        }

        output.getPtr()[output.getLength() - 2] = '\n';

        fwrite(output.getPtr(), sizeof(char), output.getLength() - 1, mFp);

        if (mFlush)
        {
            fflush(mFp);
        }

        output.getPtr()[output.getLength() - 2] = '\0';
    }

    LogToFunc::LogToFunc(LogFunctor logFunctor) : mLogFunctor(logFunctor)
    {
    }

    LogTarget * LogToFunc::clone() const
    {
        return new LogToFunc(*this);
    }

    void LogToFunc::write(const RCF::ByteBuffer & output)
    {
        mLogFunctor(output);
    }

#ifdef BOOST_WINDOWS

    boost::uint32_t getCurrentMsValue()
    {
        SYSTEMTIME st; 
        GetSystemTime(&st);
        return st.wMilliseconds;
    }

#else

    boost::uint32_t getCurrentMsValue()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);     
        return tv.tv_usec/1000;
    }

#endif

    LogEntry::LogEntry(int name, int level) : 
        mName(name), 
        mLevel(level),
        mFile(NULL),
        mLine(0),
        mFunc(NULL),
        mThreadId( RCF::getCurrentThreadId() ),
        mTime(0),
        mTimeMs(0)
    {
        // Current time.
        mTime = time(NULL);
        mTimeMs = getCurrentMsValue();

        LogBuffers & logBuffers = getTlsLogBuffers();
        mpOstream = & logBuffers.mTlsUserBuffer;
        mpOstream->clear();
        mpOstream->rewind();
    }

    LogEntry::LogEntry(int name, int level, const char * szFile, int line, const char * szFunc) : 
        mName(name), 
        mLevel(level), 
        mFile(szFile), 
        mLine(line),
        mFunc(szFunc),
        mThreadId( RCF::getCurrentThreadId() ),
        mTime(0),
        mTimeMs(0)
    {
        // Current time.
        mTime = time(NULL);
        mTimeMs = getCurrentMsValue();

        LogBuffers & logBuffers = getTlsLogBuffers();
        mpOstream = & logBuffers.mTlsUserBuffer;
        mpOstream->clear();
        mpOstream->rewind();
    }

    LogEntry::~LogEntry()
    {
        *mpOstream << '\0';
        LogManager::instance().writeToLoggers(*this);
    }

    Logger::Logger(
        int name, 
        int level, 
        const LogTarget& logTarget, 
        const std::string & logFormat) :
            mName(name), 
            mLevel(level), 
            mTargetPtr( logTarget.clone() ), 
            mFormat(logFormat)
    {
        if (mFormat.empty())
        {
            mFormat = LogManager::instance().DefaultLogFormat;
        }
    }

    Logger::Logger(
        int name, 
        int level, 
        const LogTarget& logTarget, 
        LogFormatFunctor logFormatFunctor) :
            mName(name), 
            mLevel(level), 
            mTargetPtr( logTarget.clone() ), 
            mFormatFunctor(logFormatFunctor)
    {
    }

    Logger::Logger(
        int name, 
        int level, 
        LogTargetPtr logTargetPtr, 
        const std::string & logFormat) :
            mName(name), 
            mLevel(level), 
            mTargetPtr(logTargetPtr), 
            mFormat(logFormat)
    {
        if (mFormat.empty())
        {
            mFormat = LogManager::instance().DefaultLogFormat;
        }
    }

    Logger::Logger(
        int name, 
        int level, 
        LogTargetPtr logTargetPtr, 
        LogFormatFunctor logFormatFunctor) :
            mName(name), 
            mLevel(level), 
            mTargetPtr(logTargetPtr), 
            mFormatFunctor(logFormatFunctor)
    {
    }

    void Logger::setName(int name)
    {
        if (isActive())
        {
            deactivate();
            mName = name;
            activate();
        }
        else
        {
            mName = name;
        }
    }

    void Logger::setLevel(int level)
    {
        if (isActive())
        {
            deactivate();
            mLevel = level;
            activate();
        }
        else
        {
            mLevel = level;
        }
    }

    void Logger::setTarget(const LogTarget & logTarget)
    {
        if (isActive())
        {
            deactivate();
            mTargetPtr.reset( logTarget.clone() );
            activate();
        }
        else
        {
            mTargetPtr.reset( logTarget.clone() );
        }
    }

    void Logger::setFormat(const std::string & logFormat)
    {
        if (isActive())
        {
            deactivate();
            mFormat = logFormat;
            activate();
        }
        else
        {
            mFormat = logFormat;
        }
    }

    int Logger::getName() const
    {
        return mName;
    }

    int Logger::getLevel() const
    {
        return mLevel;
    }

    const LogTarget& Logger::getTarget() const
    {
        return *mTargetPtr;
    }

    std::string Logger::getFormat() const
    {
        return mFormat;
    }

    // A: Log name
    // B: Log level
    // C: time
    // D: thread id
    // E: __FILE__
    // F: __LINE__
    // G: __FUNCTION__
    // H: time in ms since RCF initialization
    // X: output

    // %E(%F): [Thread id: %D][Log: %A][Log level: %B]

    void Logger::write(const LogEntry & logEntry)
    {
        // Format the log entry info into a string.
        RCF::ByteBuffer output;
        
        if (mFormatFunctor)
        {
            mFormatFunctor(logEntry, output);
        }
        else
        {
            std::size_t len = static_cast<std::size_t>(logEntry.mpOstream->tellp());
            RCF::ByteBuffer logEntryOutput(logEntry.mpOstream->str(), len);
            
            LogBuffers & logBuffers = getTlsLogBuffers();
            RCF::MemOstream & os = logBuffers.mTlsLoggerBuffer;
            os.clear();
            os.rewind();

            tm * ts = NULL;
            char timeBuffer[80] = {0};

            std::size_t pos = 0;
            while (pos < mFormat.length())
            {
                if (mFormat[pos] == '%' && pos < mFormat.length())
                {
                    switch (mFormat[pos+1])
                    {
                    case '%': 
                        os << '%';                                    
                        break;

                    case 'A': 
                        os << logEntry.mName;                            
                        break;

                    case 'B': 
                        os << logEntry.mLevel;                        
                        break;

                    case 'C': 

                        ts = localtime(&logEntry.mTime);

                        sprintf(
                            timeBuffer, 
                            "%02d:%02d:%02d:%03u", 
                            ts->tm_hour, 
                            ts->tm_min, 
                            ts->tm_sec,
                            logEntry.mTimeMs);

                        os << timeBuffer;
                        break;

                    case 'D': 
                        printToOstream(os, logEntry.mThreadId);
                        break;

                    case 'E': 
                        os << (logEntry.mFile ? logEntry.mFile : "File Not Specified");    
                        break;

                    case 'F': 
                        printToOstream(os, logEntry.mLine);
                        break;

                    case 'G': 
                        os << logEntry.mFunc;                            
                        break;

                    case 'H':
                        printToOstream(os, RCF::getCurrentTimeMs());
                        break;

                    case 'X': 
                        os.write(logEntryOutput.getPtr(), static_cast<std::streamsize>(logEntryOutput.getLength()-1));                
                        break;

                    default:
                        RCF_ASSERT(0);
                    }

                    pos += 2;
                    continue;
                }
                else
                {
                    os << mFormat[pos];
                    pos += 1;
                }
            }

            // Terminate the string with two zeros, that way the log targets can insert a newline if they want.
            os << '\0' << '\0';

            output = RCF::ByteBuffer(os.str(), static_cast<std::size_t>(os.tellp()));
        }

        // Pass the string to the log target.
        if (output)
        {
            mTargetPtr->write(output);
        }
    }

    void Logger::activate()
    {
        LogManager::instance().activateLogger( shared_from_this() );
    }

    void Logger::deactivate()
    {
        LogManager::instance().deactivateLogger( shared_from_this() );
    }

    bool Logger::isActive()
    {
        return LogManager::instance().isLoggerActive( shared_from_this() );
    }   

    void initLogManager()
    {
        LogManager::init();
    }   

    void deinitLogManager()
    {
        LogManager::deinit();
    }

    VariableArgMacroFunctor::VariableArgMacroFunctor() :
        mFile(NULL),
        mLine(0),
        mFunc(NULL)
    {
        RCF::LogBuffers & logBuffers = RCF::getTlsLogBuffers();
        mHeader = & logBuffers.mTlsVarArgBuffer1;
        mArgs = & logBuffers.mTlsVarArgBuffer2;

        mHeader->clear();
        mHeader->rewind();

        mArgs->clear();
        mArgs->rewind();
    }

    VariableArgMacroFunctor::~VariableArgMacroFunctor()
    {}

    VariableArgMacroFunctor & VariableArgMacroFunctor::init(
        const std::string &label,
        const std::string &msg,
        const char *file,
        int line,
        const char *func)
    {
        mFile = file;
        mLine = line;
        mFunc = func;

        unsigned int timestamp = RCF::getCurrentTimeMs();
        RCF::ThreadId threadid = RCF::getCurrentThreadId();
       
        *mHeader
            << file << "(" << line << "): "
            << func << ": "
            << ": Thread-id=" << threadid
            << " : Timestamp(ms)=" << timestamp << ": "
            << label << msg << ": "
            << '\0';

        return *this;
    }

    void printToOstream(RCF::MemOstream & os, const std::exception &e)
    {
        os << RCF::toString(e);
    }

    void printToOstream(RCF::MemOstream & os, const RCF::Exception &e)
    {
        os << RCF::toString(e);
    }

    void printToOstream(RCF::MemOstream & os, const RCF::RemoteException &e)
    {
        os << RCF::toString(e);
    }

    void printToOstream(RCF::MemOstream & os, const RCF::SerializationException &e)
    {
        os << RCF::toString(e);
    }

    void printToOstream(RCF::MemOstream & os, const std::type_info &ti)
    {
        os << ti.name();
    }

    AssertFunctor::AssertFunctor() : mExpr(NULL)
    {
    }

    AssertFunctor::AssertFunctor(const char * expr) : mExpr(expr)
    {
    }

#if defined(_MSC_VER) && !defined(NDEBUG)
#pragma warning(push)
#pragma warning(disable: 4995) // 'sprintf': name was marked as #pragma deprecated
#pragma warning(disable: 4996) // 'sprintf': This function or variable may be unsafe.

    AssertFunctor::~AssertFunctor()
    {
        const char * msg =
            "%s\n"
            "Values: %s\n"
            "Function: %s";

        std::string values(mArgs->str(), static_cast<std::size_t>(mArgs->tellp()));

        char szBuffer[512] = { 0 };
        sprintf(szBuffer, "%s(%d): Assert failed. Expression: %s.\n", mFile, mLine, mExpr);
        OutputDebugStringA(szBuffer);
        fprintf(stdout, "%s", szBuffer);

        std::string assertMsg(szBuffer);
        RCF_LOG_1()(assertMsg) << "Failed assertion!";

        int ret = _CrtDbgReport(_CRT_ASSERT, mFile, mLine, NULL, msg, mExpr, values.c_str(), mFunc);
        if ( ret == 1 )
        {
            // __debugbreak() is more likely to give a proper call stack.
            //DebugBreak();
            __debugbreak();
        }
    }

#pragma warning(pop)
#else

    AssertFunctor::~AssertFunctor()
    {
        std::string values(mArgs->str(), static_cast<std::size_t>(mArgs->tellp()));

        char szBuffer[512] = { 0 };
        sprintf(
            szBuffer,
            "%s:%d: Assertion failed. %s . Values: %s\n",
            mFile,
            mLine,
            mExpr,
            values.c_str());

        fprintf(stdout, "%s", szBuffer);

        std::string assertMsg(szBuffer);
        RCF_LOG_1()(assertMsg) << "Failed assertion!";

        assert(0 && "See line above for assertion details.");
    }

#endif

    VarArgAbort::VarArgAbort()
    {
        abort();
    }

} // namespace RCF

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
