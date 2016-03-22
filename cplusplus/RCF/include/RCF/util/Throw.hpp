
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

#ifndef INCLUDE_UTIL_THROW_HPP
#define INCLUDE_UTIL_THROW_HPP

#include <exception>
#include <memory>

#include <boost/current_function.hpp>

#include "VariableArgMacro.hpp"

namespace RCF {

    namespace detail {

        class I_InvokeThrow
        {
        public:
            virtual ~I_InvokeThrow() {}
            virtual void invoke(const std::string &context, const std::string & args, int logName, int logLevel, const char * file, int line, const char * func) = 0;
        };

        template<typename E>
        class InvokeThrow : public I_InvokeThrow
        {
        public:
            InvokeThrow(const E &e) : mE(e)
            {}

            void invoke(const std::string &context, const std::string & args, int logName, int logLevel, const char * file, int line, const char * func)
            {
                const_cast<E &>(mE).setContext(context);

                if (RCF::LogManager::instance().isEnabled(logName, logLevel))
                {
                    RCF::LogEntry entry(logName, logLevel, file, line, func);

                    entry
                        << "Exception thrown. "
                        << mE;

                    if (!args.empty())
                    {
                        entry
                            << " Values: "
                            << args;
                    }
                }

                throw mE;
            }

        private:
            const E &mE;
        };

        template<>
        class InvokeThrow<std::runtime_error> : public I_InvokeThrow
        {
        public:
            InvokeThrow(const std::runtime_error &e) : mE(e)
            {}

            void invoke(const std::string &context, const std::string & args, int logName, int logLevel, const char * file, int line, const char * func)
            {
                if (RCF::LogManager::instance().isEnabled(logName, logLevel))
                {
                    RCF::LogEntry entry(logName, logLevel, file, line, func);

                    entry
                        << "Exception thrown. "
                        << mE;

                    if (!args.empty())
                    {
                        entry
                            << " Values: "
                            << args;
                    }
                }

                throw std::runtime_error( std::string(mE.what()) + ": " + context);
            }

        private:
            const std::runtime_error &mE;
        };

        template<typename T>
        const char *getTypeName(const T &t)
        {
            return typeid(t).name();
        }

    }

    class ThrowFunctor : public VariableArgMacroFunctor
    {
    public:
        ThrowFunctor() : mThrown(false), mLogName(0), mLogLevel(0)
        {}

        template<typename E>
        ThrowFunctor(const E &e, int logName, int logLevel) :
            VariableArgMacroFunctor(),
            mInvokeThrow(new detail::InvokeThrow<E>(e)),
            mThrown(false),
            mLogName(logName), 
            mLogLevel(logLevel)
        {}

        ~ThrowFunctor()
        {
            // dtor gets called repeatedly by borland, believe it or not
            if (!mThrown)
            {
                mThrown = true;

                std::string args(mArgs->str(), static_cast<std::size_t>(mArgs->tellp()));

                std::string context = mHeader->str();
                context += args;

                if (!std::uncaught_exception())
                {
                    mInvokeThrow->invoke(context, args, mLogName, mLogLevel, mFile, mLine, mFunc);
                }
            }
        }

    private:
        std::auto_ptr<detail::I_InvokeThrow> mInvokeThrow;
        bool mThrown;

        int mLogName;
        int mLogLevel;
    };


#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4355 )  // warning C4355: 'this' : used in base member initializer list
#endif

#if defined(__GNUC__) && (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4))
#define UTIL_THROW_GCC_33_HACK (const RCF::VariableArgMacro<RCF::ThrowFunctor> &)
#else
#define UTIL_THROW_GCC_33_HACK
#endif

DECLARE_VARIABLE_ARG_MACRO( UTIL_THROW, ThrowFunctor );
#define UTIL_THROW(e, logName, logLevel)                                    \
    while (true)                                                            \
        UTIL_THROW_GCC_33_HACK                                              \
        RCF::VariableArgMacro<RCF::ThrowFunctor>(e, logName, logLevel)    \
            .init(                                                          \
                "",                                                         \
                "",                                                         \
                __FILE__,                                                   \
                __LINE__,                                                   \
                BOOST_CURRENT_FUNCTION)                                     \
            .cast( (RCF::VariableArgMacro<RCF::ThrowFunctor> *) NULL)     \
            .UTIL_THROW_A



#define UTIL_THROW_A(x)               UTIL_THROW_OP(x, B)
#define UTIL_THROW_B(x)               UTIL_THROW_OP(x, A)
#define UTIL_THROW_OP(x, next)        UTIL_THROW_A.notify_((x), #x).UTIL_THROW_ ## next

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#define UTIL_VERIFY(cond, e, logName, logLevel)       if (cond); else UTIL_THROW(e, logName, logLevel)

} // namespace RCF

#endif // ! INCLUDE_UTIL_THROW_HPP
