
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

#include <RCF/RcfSession.hpp>

#include <RCF/ClientTransport.hpp>
#include <RCF/Marshal.hpp>
#include <RCF/PerformanceData.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/Version.hpp>

#include <boost/bind.hpp>

#if RCF_FEATURE_SSPI==1
#include <RCF/Schannel.hpp>
#include <RCF/Win32Certificate.hpp>
#endif

#if RCF_FEATURE_OPENSSL==1
#include <RCF/OpenSslEncryptionFilter.hpp>
#endif

#if RCF_FEATURE_SERVER==1
#include <RCF/PingBackService.hpp>
#include <RCF/SessionTimeoutService.hpp>
#endif

namespace RCF {

    RcfSession::RcfSession(RcfServer &server) :
        mStopCallInProgress(),
        mRcfServer(server),
        mRuntimeVersion(RCF::getDefaultRuntimeVersion()),
        mArchiveVersion(RCF::getDefaultArchiveVersion()),
        mUseNativeWstringSerialization(RCF::getDefaultNativeWstringSerialization()),
        mEnableSfPointerTracking(false),
        mTransportFiltersLocked(),
        mFiltered(),
        mCloseSessionAfterWrite(),
        mPingTimestamp(),
        mPingIntervalMs(),
        mTouchTimestamp(0),
        mWritingPingBack(false),
        mpParameters(),
        mParmsVec(1+15), // return value + max 15 arguments
        mAutoSend(true),
        mpNetworkSession(NULL),
        mTransportProtocol(Tp_Clear),
        mEnableCompression(false),
        mTransportProtocolVerified(false),
        mIsCallbackSession(false),
        mConnectedAtTime(0),
        mRemoteCallCount(0)
    {

        time_t now = 0;
        now = time(NULL);
        setConnectedAtTime(now);
        touch();

        Lock lock(getPerformanceData().mMutex);
        ++getPerformanceData().mRcfSessions;
    }

    RcfSession::~RcfSession()
    {
        RCF_DTOR_BEGIN

            {
                Lock lock(getPerformanceData().mMutex);
                --getPerformanceData().mRcfSessions;
            }

            // no locks here, relying on dtor thread safety of reference counted objects
            clearParameters();
            if (mOnDestroyCallback)
            {
                mOnDestroyCallback(*this);
            }
        RCF_DTOR_END
    }

    NetworkSession & RcfSession::getNetworkSession() const
    {
        return *mpNetworkSession;
    }

    void RcfSession::setNetworkSession(NetworkSession & networkSession)
    {
        mpNetworkSession = &networkSession;
    }

    void RcfSession::clearParameters()
    {
        if (mpParameters)
        {
            mpParameters->~I_Parameters();
            mpParameters = NULL;
        }

    }

    void RcfSession::setOnDestroyCallback(OnDestroyCallback onDestroyCallback)
    {
        Lock lock(mMutex);
        mOnDestroyCallback = onDestroyCallback;
    }

    void RcfSession::setEnableSfPointerTracking(bool enable)
    {
        mEnableSfPointerTracking = enable;
    }
    
    bool RcfSession::getEnableSfPointerTracking() const
    {
        return mEnableSfPointerTracking;
    }

    void RcfSession::addOnWriteCompletedCallback(
        const OnWriteCompletedCallback &onWriteCompletedCallback)
    {
        Lock lock(mMutex);
        mOnWriteCompletedCallbacks.push_back(onWriteCompletedCallback);
    }

    void RcfSession::extractOnWriteCompletedCallbacks(
        std::vector<OnWriteCompletedCallback> &onWriteCompletedCallbacks)
    {
        Lock lock(mMutex);
        onWriteCompletedCallbacks.clear();
        onWriteCompletedCallbacks.swap( mOnWriteCompletedCallbacks );
    }

    const RCF::RemoteAddress &RcfSession::getClientAddress()
    {
        return getNetworkSession().getRemoteAddress();
    }

    void RcfSession::disconnect()
    {
        NetworkSessionPtr networkSessionPtr = getNetworkSession().shared_from_this();
        networkSessionPtr->setEnableReconnect(false);
        networkSessionPtr->postClose();
    }

    bool RcfSession::hasDefaultServerStub()
    {
        Lock lock(mMutex);
        return mDefaultStubEntryPtr.get() ? true : false;
    }

    StubEntryPtr RcfSession::getDefaultStubEntryPtr()
    {
        Lock lock(mMutex);
        return mDefaultStubEntryPtr;
    }

    void RcfSession::setDefaultStubEntryPtr(StubEntryPtr stubEntryPtr)
    {
        Lock lock(mMutex);
        mDefaultStubEntryPtr = stubEntryPtr;
    }

    void RcfSession::setCachedStubEntryPtr(StubEntryPtr stubEntryPtr)
    {
        mCachedStubEntryPtr = stubEntryPtr;
    }

    void RcfSession::getMessageFilters(std::vector<FilterPtr> &filters)
    {
        filters = mFilters;
    }

    void RcfSession::getTransportFilters(std::vector<FilterPtr> &filters)
    {
        getNetworkSession().getTransportFilters(filters);
    }

    boost::uint32_t RcfSession::getRuntimeVersion()
    {
        return mRuntimeVersion;
    }

    void RcfSession::setRuntimeVersion(boost::uint32_t version)
    {
        mRuntimeVersion = version;
    }

    boost::uint32_t RcfSession::getArchiveVersion()
    {
        return mArchiveVersion;
    }

    void RcfSession::setArchiveVersion(boost::uint32_t version)
    {
        mArchiveVersion = version;
    }

    bool RcfSession::getNativeWstringSerialization()    
    {
        return mUseNativeWstringSerialization;
    }

    void RcfSession::setNativeWstringSerialization(bool useNativeWstringSerialization)
    {
        mUseNativeWstringSerialization = useNativeWstringSerialization;
    }

    void RcfSession::setUserData(const boost::any & userData)
    {
        mUserData = userData;
    }

    boost::any & RcfSession::getUserData()
    {
        return mUserData;
    }

    void RcfSession::lockTransportFilters()
    {
        mTransportFiltersLocked = true;
    }

    void RcfSession::unlockTransportFilters()
    {
        mTransportFiltersLocked = false;
    }

    bool RcfSession::transportFiltersLocked()
    {
        return mTransportFiltersLocked;
    }

    SerializationProtocolIn & RcfSession::getSpIn()
    {
        return mIn;
    }

    SerializationProtocolOut & RcfSession::getSpOut()
    {
        return mOut;
    }

    bool RcfSession::getFiltered()
    {
        return mFiltered;
    }

    void RcfSession::setFiltered(bool filtered)
    {
        mFiltered = filtered;
    }

    std::vector<FilterPtr> & RcfSession::getFilters()
    {
        return mFilters;
    }

    RcfServer & RcfSession::getRcfServer()
    {
        return mRcfServer;
    }

    void RcfSession::setCloseSessionAfterWrite(bool close)
    {
        mCloseSessionAfterWrite = close;
    }

    boost::uint32_t RcfSession::getPingBackIntervalMs()
    {
        return mRequest.getPingBackIntervalMs();
    }

    boost::uint32_t RcfSession::getPingTimestamp()
    {
        Lock lock(mMutex);
        return mPingTimestamp;
    }

    void RcfSession::setPingTimestamp()
    {
        Lock lock(mMutex);
        mPingTimestamp = RCF::getCurrentTimeMs();
    }

    boost::uint32_t RcfSession::getPingIntervalMs()
    {
        return mPingIntervalMs;
    }

    void RcfSession::setPingIntervalMs(boost::uint32_t pingIntervalMs)
    {
        mPingIntervalMs = pingIntervalMs;
    }

    boost::uint32_t RcfSession::getTouchTimestamp()
    {
        Lock lock(mMutex);
        return mTouchTimestamp;
    }

    void RcfSession::touch()
    {
        Lock lock(mMutex);
        mTouchTimestamp = RCF::getCurrentTimeMs();
    }

    void RcfSession::registerForPingBacks()
    {
        // Register for ping backs if appropriate.

        if (    mRequest.getPingBackIntervalMs() > 0 
            &&  !mRequest.getOneway())
        {
            PingBackServicePtr pbsPtr = mRcfServer.getPingBackServicePtr();
            if (pbsPtr)
            {

#if RCF_FEATURE_SERVER==1

                // Disable reconnecting for this session. After sending a 
                // pingback, a server I/O thread would get a write completion 
                // notification, and if it happened to be an error (unlikely 
                // but possible), we definitely would not want a reconnect, as 
                // the session would still in use.
                getNetworkSession().setEnableReconnect(false);

                PingBackTimerEntry pingBackTimerEntry = 
                    pbsPtr->registerSession(shared_from_this());

                Lock lock(mIoStateMutex);
                RCF_ASSERT_EQ( mPingBackTimerEntry.first , 0 );
                mPingBackTimerEntry = pingBackTimerEntry;

#endif

            }
            else

            {
                // TODO: something more efficient than throwing
                Exception e(_RcfError_NoPingBackService());
                RCF_THROW(e);
            }
        }
    }

    void RcfSession::unregisterForPingBacks()
    {
        // Unregister for ping backs if appropriate.

        if (    mRequest.getPingBackIntervalMs() > 0 
            &&  !mRequest.getOneway())
        {
            PingBackServicePtr pbsPtr = mRcfServer.getPingBackServicePtr();
            if (pbsPtr)
            {

#if RCF_FEATURE_SERVER==1

                pbsPtr->unregisterSession(mPingBackTimerEntry);
                mPingBackTimerEntry = PingBackTimerEntry();

#endif

            }
        }
    }

    void RcfSession::sendPingBack()
    {
        //RCF_ASSERT( mIoStateMutex.locked() );

        mWritingPingBack = true;

        ThreadLocalCached< std::vector<ByteBuffer> > tlcByteBuffers;
        std::vector<ByteBuffer> &byteBuffers = tlcByteBuffers.get();

        byteBuffers.push_back(mPingBackByteBuffer);

        boost::uint32_t pingBackIntervalMs = getPingBackIntervalMs();

        encodeServerError(
            mRcfServer,
            byteBuffers.front(),
            RcfError_PingBack,
            pingBackIntervalMs,
            0);

        getNetworkSession().postWrite(byteBuffers);
    }

    bool RcfSession::getAutoSend()
    {
        return mAutoSend;
    }

    void RcfSession::setWeakThisPtr()
    {
        mWeakThisPtr = shared_from_this();
    }

    void RcfSession::setRequestUserData(const std::string & userData)
    {
        mRequest.mRequestUserData = ByteBuffer(userData);
    }

    std::string RcfSession::getRequestUserData()
    {
        if ( mRequest.mRequestUserData.isEmpty() )
        {
            return std::string();
        }

        return std::string(
            mRequest.mRequestUserData.getPtr(), 
            mRequest.mRequestUserData.getLength());
    }

    void RcfSession::setResponseUserData(const std::string & userData)
    {
        mRequest.mResponseUserData = ByteBuffer(userData);
    }

    std::string RcfSession::getResponseUserData()
    {
        if ( mRequest.mResponseUserData.isEmpty() )
        {
            return std::string();
        }

        return std::string(
            mRequest.mResponseUserData.getPtr(), 
            mRequest.mResponseUserData.getLength());    
    }

    bool RcfSession::isOneway()
    {
        return mRequest.mOneway;
    }

#if RCF_FEATURE_FILETRANSFER==1

    void RcfSession::cancelDownload()
    {
        Lock lock(mMutex);
        if (mDownloadInfoPtr)
        {
            mDownloadInfoPtr->mCancel = true;
        }
    }

    void RcfSession::addDownloadStream(
        boost::uint32_t sessionLocalId, 
        FileStream fileStream)
    {
        Lock lock(mMutex);
        mSessionDownloads[sessionLocalId].mImplPtr = fileStream.mImplPtr;
    }

#else

    void RcfSession::cancelDownload()
    {

    }

#endif

    tstring RcfSession::getClientUsername()
    {
        return mClientUsername;
    }

    TransportProtocol RcfSession::getTransportProtocol()
    {
        return mTransportProtocol;
    }

    bool RcfSession::getEnableCompression()
    {
        return mEnableCompression;
    }

    CertificatePtr RcfSession::getClientCertificatePtr()
    {
        if (getTransportProtocol() == Tp_Ssl)
        {
            std::vector<FilterPtr> transportFilters;
            getTransportFilters(transportFilters);
            for (std::size_t i=0; i<transportFilters.size(); ++i)
            {

#if RCF_FEATURE_SSPI==1
                if (transportFilters[i]->getFilterId() == RcfFilter_SspiSchannel)
                {
                    SchannelServerFilter & schannelFilter = static_cast<SchannelServerFilter &>(*transportFilters[i]);
                    RCF::CertificatePtr peerCertPtr = schannelFilter.getPeerCertificate();
                    return peerCertPtr;
                }
#endif

#if RCF_FEATURE_OPENSSL==1
                if (transportFilters[i]->getFilterId() == RcfFilter_OpenSsl)
                {
                    OpenSslEncryptionFilter & opensslFilter = static_cast<OpenSslEncryptionFilter &>(*transportFilters[i]);
                    RCF::CertificatePtr peerCertPtr = opensslFilter.getPeerCertificate();
                    return peerCertPtr;
                }
#endif

            }
        }

        return CertificatePtr();
    }

    TransportType RcfSession::getTransportType()
    {
        return getNetworkSession().getServerTransport().getTransportType();
    }

    bool RcfSession::getIsCallbackSession() const
    {
        return mIsCallbackSession;
    }

    void RcfSession::setIsCallbackSession(bool isCallbackSession)
    {
        mIsCallbackSession = isCallbackSession;
    }

    RemoteCallRequest RcfSession::getRemoteCallRequest() const
    {
        return RemoteCallRequest(mRequest);
    }

    time_t RcfSession::getConnectedAtTime() const
    {
        return mConnectedAtTime;
    }

    void RcfSession::setConnectedAtTime(time_t connectedAtTime)
    {
        mConnectedAtTime = connectedAtTime;
    }

    std::size_t RcfSession::getConnectionDuration() const
    {
        if (mConnectedAtTime)
        {
            time_t now = time(NULL);
            time_t durationS = now - mConnectedAtTime;
            return static_cast<std::size_t>(durationS);
        }
        return 0;
    }

    std::size_t RcfSession::getRemoteCallCount() const
    {
        return mRemoteCallCount;
    }

    boost::uint64_t RcfSession::getTotalBytesReceived() const
    {
        return mpNetworkSession->getTotalBytesReceived();
    }

    boost::uint64_t RcfSession::getTotalBytesSent() const
    {
        return mpNetworkSession->getTotalBytesSent();
    }

    bool RcfSession::isConnected() const
    {
        return getNetworkSession().isConnected();
    }

} // namespace RCF
