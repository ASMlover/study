
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

#ifndef INCLUDE_UTIL_PROFILE_HPP
#define INCLUDE_UTIL_PROFILE_HPP

#include <sys/types.h>
#include <sys/timeb.h>

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include "RCF/ThreadLibrary.hpp"

namespace RCF {

    class ProfilingData;

    class ProfilingResults
    {
    private:
        ProfilingResults()
        {
        }

        ~ProfilingResults();

        static ProfilingResults *&getSingletonPtrRef()
        {
            static ProfilingResults *pProfilingResults = NULL;
            if (pProfilingResults == NULL)
            {
                pProfilingResults = new ProfilingResults();
            }
            return pProfilingResults;
        }

        std::vector<ProfilingData *> pTlsData;

    public:
        static ProfilingResults &getSingleton()
        {
            return *getSingletonPtrRef();
        }
        static void deleteSingleton()
        {
            delete getSingletonPtrRef();
            getSingletonPtrRef() = NULL;
        }
        void dump()
        {
            RCF::Lock lock(mMutex);
            for (unsigned int i=0; i<mResults.size(); i++)
            {
                std::ostringstream ostr;
                ostr << "*************************\n";
                ostr << "Profiling results for thread #" << i << ":\n";
               
                for (DataT::iterator iter_i = mResults[i]->begin(); iter_i != mResults[i]->end(); iter_i++) {
                    if (!(*iter_i).first.empty()) {
                        ostr << (*iter_i).first << ": " << (*iter_i).second.first / 1000.0 << " s.\n";
                        SubDataT &subData = (*iter_i).second.second;
                        for (SubDataT::iterator iter_j = subData.begin(); iter_j != subData.end(); iter_j++)
                            ostr << "    " << (*iter_j).first << ": " << (*iter_j).second / 1000.0 << "s.\n";
                    }
                }

                ostr << "*************************\n";
                std::cout << ostr.str();
            }
        }

    private:
        friend class ProfilingData;
        typedef std::map<std::string, unsigned int> SubDataT;
        typedef std::map<std::string, std::pair<unsigned int, SubDataT > > DataT;

        RCF::Mutex mMutex;
        std::vector< DataT * > mResults;

        void add( DataT *data )
        {
            RCF::Lock scoped_lock( mMutex );
            mResults.push_back( data );
        }

    };

    class ProfilingData
    {
    private:
        ProfilingData() : stack_(100), data_(new DataT()) { ProfilingResults::getSingleton().add( data_ );  }
    public:
        ~ProfilingData()
        {
            delete data_;
            data_ = NULL;
        }

        static ProfilingData &getThreadSpecificInstance()
        {
            static RCF::ThreadSpecificPtr<ProfilingData>::Val profilingData;

            if (NULL == profilingData.get())
            {
                profilingData.reset(new ProfilingData());
                ProfilingResults::getSingleton().pTlsData.push_back(profilingData);
                profilingData->push("");
            }
            return *profilingData;
        }

        void push(std::string sz)
        {
            stack_.push_back(sz);
        }

        void pop()
        {
            stack_.pop_back();
        }

        void add(unsigned int timespan)
        {
            std::string cur = stack_[stack_.size()-1];
            std::string prev = stack_[stack_.size()-2];
            (*data_)[cur].first += timespan;
            (*data_)[prev].second[cur] += timespan;
        }
       
    private:
        typedef ProfilingResults::DataT DataT;
        std::vector<std::string> stack_;
        DataT *data_;
    };

    inline unsigned int getTickCount()
    {
        return RCF::getCurrentTimeMs();
    }

    class Profile
    {
    public:
        Profile(const std::string &name) : name(name), t0(getTickCount()), t1(0)
        {
            ProfilingData::getThreadSpecificInstance().push(name);
        }

        void stop()
        {
            t1 = getTickCount();
            ProfilingData::getThreadSpecificInstance().add(t1 - t0);
            ProfilingData::getThreadSpecificInstance().pop();
        }


        ~Profile()
        {
            if (!t1)
            {
                stop();
            }
        }

        int getDurationMs()
        {
            RCF_ASSERT(t1);
            return t1 - t0;
        }

    private:
        std::string name;
        unsigned int t0;
        unsigned int t1;
    };

    class ImmediateProfile
    {
    public:
        ImmediateProfile(const std::string &name) :
            mName(name),
            t0Ms(getTickCount()),
            t1Ms()
        {}

        ~ImmediateProfile()
        {
            t1Ms = getTickCount();
            std::ostringstream ostr;
            ostr << "Profile result: " << mName << ": " << t1Ms-t0Ms << "ms" << std::endl;
            std::cout << ostr.str();
        }

    private:
        std::string mName;
        unsigned int t0Ms;
        unsigned int t1Ms;
    };

    ProfilingResults::~ProfilingResults()
    {
        for (std::size_t i=0; i<pTlsData.size(); ++i)
        {
            delete pTlsData[i];
        }
        pTlsData.clear();
    }


} // namespace RCF

#endif //! INCLUDE_UTIL_PROFILE_HPP
