
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

#ifndef INCLUDE_RCF_THREADLOCALDATA_HPP
#define INCLUDE_RCF_THREADLOCALDATA_HPP

#include <boost/shared_ptr.hpp>

#include <RCF/ByteBuffer.hpp>
#include <RCF/Export.hpp>
#include <RCF/RecursionLimiter.hpp>
#include <RCF/ThreadLibrary.hpp>

#ifdef BOOST_WINDOWS
#include <WinSock2.h>
#endif

#ifndef BOOST_WINDOWS
struct iovec;
#endif

namespace RCF {

    class ObjectCache;
    class ClientStub;
    class RcfSession;
    class ThreadInfo;
    class UdpNetworkSession;
    class I_Future;
    class AmiNotification;
    class OverlappedAmi;
    class LogBuffers;
    class Filter;
    class FileUpload;

    typedef boost::shared_ptr<ClientStub>       ClientStubPtr;
    typedef boost::shared_ptr<RcfSession>       RcfSessionPtr;
    typedef boost::shared_ptr<ThreadInfo>       ThreadInfoPtr;
    typedef boost::shared_ptr<UdpNetworkSession>  UdpNetworkSessionPtr;   
    typedef boost::shared_ptr<OverlappedAmi>    OverlappedAmiPtr;
    typedef boost::shared_ptr<LogBuffers>       LogBuffersPtr;
    typedef boost::function1<void, RcfSession&> RcfSessionCallback;
    typedef boost::shared_ptr<Filter>           FilterPtr;

#ifndef BOOST_WINDOWS
    typedef iovec WSABUF;
#endif

    class ThreadLocalData;
    ThreadLocalData &               getThreadLocalData();

    RCF_EXPORT void                 clearThreadLocalDataForThisThread();

    RCF_EXPORT ClientStub *         getTlsClientStubPtr();
    
    RCF_EXPORT void                 pushTlsClientStub(
                                        ClientStub * pClientStub);

    RCF_EXPORT void                 popTlsClientStub();

    RCF_EXPORT RcfSession *         getCurrentRcfSessionPtr();
    RCF_EXPORT RcfSession *         getTlsRcfSessionPtr();

    RCF_EXPORT void                 setTlsRcfSessionPtr(
                                        RcfSession * pRcfSession = NULL);

    RCF_EXPORT ThreadInfoPtr        getTlsThreadInfoPtr();

    RCF_EXPORT void                 setTlsThreadInfoPtr(
                                        ThreadInfoPtr threadInfoPtr);

    RCF_EXPORT UdpNetworkSessionPtr   getTlsUdpNetworkSessionPtr();

    RCF_EXPORT void                 setTlsUdpNetworkSessionPtr(
                                        UdpNetworkSessionPtr udpNetworkSessionPtr);

    RCF_EXPORT RcfSession &         getCurrentRcfSession();
    RCF_EXPORT RcfSession &         getTlsRcfSession();

    RecursionState<int, int> &      getTlsRcfSessionRecursionState();

    RCF_EXPORT AmiNotification &    getTlsAmiNotification();

    RCF_EXPORT LogBuffers &         getTlsLogBuffers();

    RCF_EXPORT std::vector< std::vector<RCF::ByteBuffer> * > &      
                                    getTlsCache(std::vector<RCF::ByteBuffer> *);

    RCF_EXPORT std::vector< std::vector<int> * > &                  
                                    getTlsCache(std::vector<int> *);

    RCF_EXPORT std::vector< std::vector<WSABUF> * > &               
                                    getTlsCache(std::vector<WSABUF> *);

    RCF_EXPORT std::vector< std::vector<FilterPtr> * > &                
                                    getTlsCache(std::vector<FilterPtr> *);

    RCF_EXPORT std::vector< std::vector<RcfSessionCallback> * > &   
                                    getTlsCache(std::vector<RcfSessionCallback> *);

    RCF_EXPORT std::vector< std::vector<FileUpload> * > &      
                                    getTlsCache(std::vector<FileUpload> *);

    template<typename T>
    class ThreadLocalCached
    {
    public:

        ThreadLocalCached() : mpt(NULL)
        {
            std::vector<T *> & tlsCache = getTlsCache( (T *) NULL);
            if (tlsCache.empty())
            {
                mpt = new T();
            }
            else
            {
                mpt = tlsCache.back();
                tlsCache.pop_back();
            }
            RCF_ASSERT(mpt->empty());
        }

        ~ThreadLocalCached()
        {
            mpt->clear();
            std::vector<T *> & tlsCache = getTlsCache( (T *) NULL);
            tlsCache.push_back(mpt);
            mpt = NULL;
        }

        T & get()
        {
            return *mpt;
        }

    private:

        T * mpt;
    };

    void RCF_EXPORT addThreadExitHandler(boost::function<void()> func);

} // namespace RCF

#endif // ! INCLUDE_RCF_THREADLOCALDATA_HPP
