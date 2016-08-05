// @file log.h

/*    Copyright 2009 10gen Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <string.h>
#include <sstream>
#include <errno.h>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/tss.hpp>

#include "mongo/base/status.h"
#include "mongo/bson/util/builder.h"
#include "mongo/util/concurrency/mutex.h"
#include "mongo/util/debug_util.h"
#include "mongo/util/exit_code.h"

#ifndef _WIN32
#include <syslog.h>
#endif

namespace mongo {

    enum ExitCode;

    enum LogLevel {  LL_DEBUG , LL_INFO , LL_NOTICE , LL_WARNING , LL_ERROR , LL_SEVERE };

    inline const char * logLevelToString( LogLevel l ) {
        switch ( l ) {
        case LL_DEBUG:
        case LL_INFO:
        case LL_NOTICE:
            return "";
        case LL_WARNING:
            return "warning" ;
        case LL_ERROR:
            return "ERROR";
        case LL_SEVERE:
            return "SEVERE";
        default:
            return "UNKNOWN";
        }
    }
    
#ifndef _WIN32
    inline const int logLevelToSysLogLevel( LogLevel l) {
        switch ( l ) {
        case LL_DEBUG:
            return LOG_DEBUG;
        case LL_INFO:
            return LOG_INFO;
        case LL_NOTICE:
            return LOG_NOTICE;
        case LL_WARNING:
            return LOG_WARNING;
        case LL_ERROR:
            return LOG_ERR;
        case LL_SEVERE:
            return LOG_EMERG;
        default:
            return LL_INFO;
        }
    }
#endif

    class LabeledLevel {
    public:
        LabeledLevel( int level ) : _level( level ) {}
        LabeledLevel( const char* label, int level ) : _label( label ), _level( level ) {}
        LabeledLevel( const string& label, int level ) : _label( label ), _level( level ) {}

        LabeledLevel operator+( int i ) const {
            return LabeledLevel( _label, _level + i );
        }

        LabeledLevel operator+( const char* label ) const {
            if( _label == "" )
                return LabeledLevel( label, _level );
            return LabeledLevel( _label + string("::") + label, _level );
        }

        LabeledLevel operator+( const std::string& label ) const {
            return LabeledLevel( _label + string("::") + label, _level );
        }

        LabeledLevel operator-( int i ) const {
            return LabeledLevel( _label, _level - i );
        }

        const string& getLabel() const { return _label; }
        int getLevel() const { return _level; }

    private:
        string _label;
        int _level;
    };

    inline bool operator<( const LabeledLevel& ll, const int i ) { return ll.getLevel() < i; }
    inline bool operator<( const int i, const LabeledLevel& ll ) { return i < ll.getLevel(); }
    inline bool operator>( const LabeledLevel& ll, const int i ) { return ll.getLevel() > i; }
    inline bool operator>( const int i, const LabeledLevel& ll ) { return i > ll.getLevel(); }
    inline bool operator<=( const LabeledLevel& ll, const int i ) { return ll.getLevel() <= i; }
    inline bool operator<=( const int i, const LabeledLevel& ll ) { return i <= ll.getLevel(); }
    inline bool operator>=( const LabeledLevel& ll, const int i ) { return ll.getLevel() >= i; }
    inline bool operator>=( const int i, const LabeledLevel& ll ) { return i >= ll.getLevel(); }
    inline bool operator==( const LabeledLevel& ll, const int i ) { return ll.getLevel() == i; }
    inline bool operator==( const int i, const LabeledLevel& ll ) { return i == ll.getLevel(); }

    class LazyString {
    public:
        virtual ~LazyString() {}
        virtual string val() const = 0;
    };

    // Utility class for stringifying object only when val() called.
    template< class T >
    class LazyStringImpl : public LazyString {
    public:
        LazyStringImpl( const T &t ) : t_( t ) {}
        virtual string val() const { return t_.toString(); }
    private:
        const T& t_;
    };

    class Tee {
    public:
        virtual ~Tee() {}
        virtual void write(LogLevel level , const string& str) = 0;
    };

    class Nullstream {
    public:
        virtual Nullstream& operator<< (Tee* tee) {
            return *this;
        }
        virtual ~Nullstream() {}
        virtual Nullstream& operator<<(const char *) {
            return *this;
        }
        virtual Nullstream& operator<<(const string& ) {
            return *this;
        }
        virtual Nullstream& operator<<(const StringData& ) {
            return *this;
        }
        virtual Nullstream& operator<<(char *) {
            return *this;
        }
        virtual Nullstream& operator<<(char) {
            return *this;
        }
        virtual Nullstream& operator<<(int) {
            return *this;
        }
        virtual Nullstream& operator<<(ExitCode) {
            return *this;
        }
        virtual Nullstream& operator<<(unsigned long) {
            return *this;
        }
        virtual Nullstream& operator<<(long) {
            return *this;
        }
        virtual Nullstream& operator<<(unsigned) {
            return *this;
        }
        virtual Nullstream& operator<<(unsigned short) {
            return *this;
        }
        virtual Nullstream& operator<<(double) {
            return *this;
        }
        virtual Nullstream& operator<<(void *) {
            return *this;
        }
        virtual Nullstream& operator<<(const void *) {
            return *this;
        }
        virtual Nullstream& operator<<(long long) {
            return *this;
        }
        virtual Nullstream& operator<<(unsigned long long) {
            return *this;
        }
        virtual Nullstream& operator<<(bool) {
            return *this;
        }
        virtual Nullstream& operator<<(const LazyString&) {
            return *this;
        }
        template< class T >
        Nullstream& operator<<(T *t) {
            return operator<<( static_cast<void*>( t ) );
        }
        template< class T >
        Nullstream& operator<<(const T *t) {
            return operator<<( static_cast<const void*>( t ) );
        }
        template< class T >
        Nullstream& operator<<(const boost::shared_ptr<T> &p ) {
            T * t = p.get();
            if ( ! t )
                *this << "null";
            else
                *this << *t;
            return *this;
        }
        template< class T >
        Nullstream& operator<<(const T &t) {
            return operator<<( static_cast<const LazyString&>( LazyStringImpl< T >( t ) ) );
        }

        virtual Nullstream& operator<< (std::ostream& ( *endl )(std::ostream&)) {
            return *this;
        }
        virtual Nullstream& operator<< (std::ios_base& (*hex)(std::ios_base&)) {
            return *this;
        }

        virtual void flush(Tee *t = 0) {}
    };
    extern Nullstream nullstream;

    class Logstream : public Nullstream {
    public:
        static Logstream& get();
    };

    extern int tlogLevel;

    inline Nullstream& out( int level = 0 ) {
        return nullstream;
    }

    /* flush the log stream if the log level is
       at the specified level or higher. */
    inline void logflush(int level = 0) {
         Logstream::get().flush(0);
    }

    /* without prolog */
    inline Nullstream& _log( int level = 0 ) {
        return nullstream;
    }

    /** logging which we may not want during unit tests (dbtests) runs.
        set tlogLevel to -1 to suppress tlog() output in a test program. */
    Nullstream& tlog( int level = 0 );

    // log if debug build or if at a certain level
    inline Nullstream& dlog( int level ) {
        return nullstream;
    }

#define MONGO_LOG(requiredLevel) \
    ::mongo::log()
#define LOG MONGO_LOG

    inline Nullstream& log() {
        return nullstream;
    }

    inline Nullstream& error() {
        return nullstream;
    }

    inline Nullstream& warning() {
        return nullstream;
    }

    inline Nullstream& problem( int level = 0 ) {
        return nullstream;
    }

    /**
       log to a file rather than stdout
       defined in assert_util.cpp
     */
    bool initLogging( const string& logpath , bool append );
    bool rotateLogs();

    std::string toUtf8String(const std::wstring& wide);

    /** output the error # and error message with prefix.
        handy for use as parm in uassert/massert.
        */
    string errnoWithPrefix( const char * prefix );

 
    extern Tee* const warnings; // Things put here go in serverStatus
    extern Tee* const startupWarningsLog; // Things put here get reported in MMS

    string errnoWithDescription(int errorcode = -1);
    void rawOut( const string &s );

    /**
     * Write the current context (backtrace), along with the optional "msg".
     */
    void logContext(const char *msg = NULL);

} // namespace mongo
