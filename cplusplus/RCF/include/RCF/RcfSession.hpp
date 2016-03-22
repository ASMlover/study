
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

#ifndef INCLUDE_RCF_RCFSESSION_HPP
#define INCLUDE_RCF_RCFSESSION_HPP

#include <vector>

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Filter.hpp>
#include <RCF/Export.hpp>
#include <RCF/MethodInvocation.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/ServerTransport.hpp>
#include <RCF/StubEntry.hpp>

#if RCF_FEATURE_FILETRANSFER==1
#include <RCF/FileDownload.hpp>
#include <RCF/FileUpload.hpp>
#endif

#include <typeinfo>

namespace RCF {

    class Filter;

    typedef boost::shared_ptr<Filter> FilterPtr;

    class RcfSession;

    typedef boost::shared_ptr<RcfSession> RcfSessionPtr;
    typedef boost::weak_ptr<RcfSession> RcfSessionWeakPtr;

    class I_Future;

    class I_Parameters;

    class UdpServerTransport;
    class UdpNetworkSession;

    class FileTransferService;
    class FileUploadInfo;
    class FileDownloadInfo;

    class FileStreamImpl;

    typedef boost::shared_ptr<FileUploadInfo>   FileUploadInfoPtr;
    typedef boost::shared_ptr<FileDownloadInfo> FileDownloadInfoPtr;

    typedef std::pair<boost::uint32_t, RcfSessionWeakPtr>   PingBackTimerEntry;

    class Certificate;
    typedef boost::shared_ptr<Certificate> CertificatePtr;

    class AsioNetworkSession;

    template<
        typename R, 
        typename A1, 
        typename A2, 
        typename A3, 
        typename A4, 
        typename A5, 
        typename A6, 
        typename A7, 
        typename A8, 
        typename A9, 
        typename A10, 
        typename A11, 
        typename A12, 
        typename A13, 
        typename A14, 
        typename A15>
    class AllocateServerParameters;

    template<
        typename R, 
        typename A1, 
        typename A2, 
        typename A3,
        typename A4,
        typename A5, 
        typename A6, 
        typename A7, 
        typename A8, 
        typename A9, 
        typename A10, 
        typename A11, 
        typename A12, 
        typename A13, 
        typename A14, 
        typename A15>
    class ServerParameters;

    class PingBackService;

    struct TypeInfoCompare
    {
        bool operator()(
            const std::type_info* lhs,
            const std::type_info* rhs) const
        { 
            if (lhs->before(*rhs))
            {
                return true;
            }
            return false;
        }
    };

    class RCF_EXPORT RcfSession : 
        public boost::enable_shared_from_this<RcfSession>
    {
    public:
        RcfSession(RcfServer &server);
        ~RcfSession();

        typedef boost::function1<void, RcfSession&> OnWriteCompletedCallback;
        typedef boost::function1<void, RcfSession&> OnWriteInitiatedCallback;
        typedef boost::function1<void, RcfSession&> OnDestroyCallback;

        typedef std::map<const std::type_info *, boost::any, TypeInfoCompare> SessionObjectMap;
        SessionObjectMap mSessionObjects;

    private:

        template<typename T>
        T * getSessionObjectImpl(bool createIfDoesntExist)
        {
            typedef boost::shared_ptr<T> TPtr;

            const std::type_info & whichType = typeid(T);
            const std::type_info * pWhichType = &whichType;

            SessionObjectMap::iterator iter = mSessionObjects.find(pWhichType);
            if (iter != mSessionObjects.end())
            {
                boost::any & a = iter->second;
                TPtr * ptPtr = boost::any_cast<TPtr>(&a);
                RCF_ASSERT(ptPtr && *ptPtr);
                return ptPtr->get();
            }
            else if (createIfDoesntExist)
            {
                TPtr tPtr( new T() );
                mSessionObjects[pWhichType] = tPtr;
                return tPtr.get();
            }
            else
            {
                return NULL;
            }
        }

    public:

        template<typename T>
        void deleteSessionObject()
        {
            typedef boost::shared_ptr<T> TPtr;

            const std::type_info & whichType = typeid(T);
            const std::type_info * pWhichType = &whichType;

            SessionObjectMap::iterator iter = mSessionObjects.find(pWhichType);
            if (iter != mSessionObjects.end())
            {
                mSessionObjects.erase(iter);
            }
        }

        template<typename T>
        T & createSessionObject()
        {
            deleteSessionObject<T>();
            T * pt = getSessionObjectImpl<T>(true);
            RCF_ASSERT(pt);
            if ( !pt )
            {
                RCF_THROW(Exception(_RcfError_SessionObjectNotCreated(typeid(T).name())));
            }
            return *pt; 
        }

        template<typename T>
        T & getSessionObject(bool createIfDoesntExist = false)
        {
            T * pt = getSessionObjectImpl<T>(createIfDoesntExist);
            if (!pt)
            {
                RCF_THROW( Exception(_RcfError_SessionObjectDoesNotExist(typeid(T).name())));
            }
            return *pt; 
        }

        template<typename T>
        T * querySessionObject()
        {
            T * pt = getSessionObjectImpl<T>(false);
            return pt;
        }



        //*******************************
        // callback tables - synchronized

        // may well be called on a different thread than the one that executed the remote call
        void addOnWriteCompletedCallback(
            const OnWriteCompletedCallback &        onWriteCompletedCallback);

        void extractOnWriteCompletedCallbacks(
            std::vector<OnWriteCompletedCallback> & onWriteCompletedCallbacks);

        void setOnDestroyCallback(
            OnDestroyCallback                       onDestroyCallback);

        //*******************************

        const RemoteAddress &
                        getClientAddress();

        RcfServer &     getRcfServer();

        void            disconnect();

        bool            hasDefaultServerStub();
        StubEntryPtr    getDefaultStubEntryPtr();
        void            setDefaultStubEntryPtr(StubEntryPtr stubEntryPtr);
        void            setCachedStubEntryPtr(StubEntryPtr stubEntryPtr);

        void            setEnableSfPointerTracking(bool enable);
        bool            getEnableSfPointerTracking() const;

        boost::uint32_t getRuntimeVersion();
        void            setRuntimeVersion(boost::uint32_t version);

        boost::uint32_t getArchiveVersion();
        void            setArchiveVersion(boost::uint32_t version);

        bool            getNativeWstringSerialization();
        void            setNativeWstringSerialization(bool enable);

        void            setUserData(const boost::any & userData);
        boost::any &    getUserData();

        void            getMessageFilters(std::vector<FilterPtr> &filters);
        void            getTransportFilters(std::vector<FilterPtr> &filters);

        void            lockTransportFilters();
        void            unlockTransportFilters();
        bool            transportFiltersLocked();

        SerializationProtocolIn &   getSpIn();
        SerializationProtocolOut &  getSpOut();

        bool                        getFiltered();
        void                        setFiltered(bool filtered);

        std::vector<FilterPtr> &    getFilters();

        void            setCloseSessionAfterWrite(bool close);

        boost::uint32_t getPingBackIntervalMs();

        boost::uint32_t getPingTimestamp();
        void            setPingTimestamp();

        boost::uint32_t getPingIntervalMs();
        void            setPingIntervalMs(boost::uint32_t pingIntervalMs);

        boost::uint32_t getTouchTimestamp();

        void            touch();

        void            sendPingBack();
        bool            getAutoSend();

        void            setWeakThisPtr();

        void            setRequestUserData(const std::string & userData);
        std::string     getRequestUserData();

        void            setResponseUserData(const std::string & userData);
        std::string     getResponseUserData();

        bool            isOneway();

        void            cancelDownload();

#if RCF_FEATURE_FILETRANSFER==1

        void            addDownloadStream(
                            boost::uint32_t sessionLocalId, 
                            FileStream fileStream);

#endif

        Mutex                                   mStopCallInProgressMutex;
        bool                                    mStopCallInProgress;
        
    private:

        template<
            typename R, 
            typename A1, 
            typename A2, 
            typename A3, 
            typename A4, 
            typename A5, 
            typename A6, 
            typename A7, 
            typename A8, 
            typename A9, 
            typename A10, 
            typename A11, 
            typename A12, 
            typename A13, 
            typename A14, 
            typename A15>
        friend class AllocateServerParameters;

        template<
            typename R, 
            typename A1, 
            typename A2, 
            typename A3,
            typename A4,
            typename A5, 
            typename A6, 
            typename A7, 
            typename A8, 
            typename A9, 
            typename A10, 
            typename A11, 
            typename A12, 
            typename A13, 
            typename A14, 
            typename A15>
        friend class ServerParameters;

        friend class PingBackService;
        friend class FilterService;

        friend class StubAccess;

        RcfServer &                             mRcfServer;

        Mutex                                   mMutex;
        std::vector<OnWriteCompletedCallback>   mOnWriteCompletedCallbacks;
        std::vector<OnWriteInitiatedCallback>   mOnWriteInitiatedCallbacks;
        OnDestroyCallback                       mOnDestroyCallback;

        boost::uint32_t                         mRuntimeVersion;
        boost::uint32_t                         mArchiveVersion;

        bool                                    mUseNativeWstringSerialization;
        bool                                    mEnableSfPointerTracking;
        
        bool                                    mTransportFiltersLocked;

        SerializationProtocolIn                 mIn;
        SerializationProtocolOut                mOut;

        // message filters
        std::vector<FilterPtr>                  mFilters;
        bool                                    mFiltered;

        MethodInvocationRequest                 mRequest;

        bool                                    mCloseSessionAfterWrite;
        boost::uint32_t                         mPingTimestamp;
        boost::uint32_t                         mPingIntervalMs;
        boost::uint32_t                         mTouchTimestamp;
        ByteBuffer                              mPingBackByteBuffer;
        PingBackTimerEntry                      mPingBackTimerEntry;

        Mutex                                   mIoStateMutex;
        bool                                    mWritingPingBack;
        std::vector<ByteBuffer>                 mQueuedSendBuffers;

        void clearParameters();

        void onReadCompleted();
        void onWriteCompleted();

        void processJsonRpcRequest();

        void processRequest();
        void processOobMessages();
        void invokeServant();
        
        void sendResponse();
        void sendResponseException(const std::exception &e);
        void sendResponseUncaughtException();

        void encodeRemoteException(
            SerializationProtocolOut & out, 
            const RemoteException & e);

        void sendSessionResponse(); 

        void registerForPingBacks();
        void unregisterForPingBacks();

        void verifyTransportProtocol(RCF::TransportProtocol protocol);

        friend class RcfServer;
        friend class RemoteCallContextImpl;

        I_Parameters *                          mpParameters;
        std::vector<char>                       mParametersVec;

        // For individual parameters.
        std::vector< std::vector<char> >        mParmsVec;

        bool                                    mAutoSend;

        RcfSessionWeakPtr                       mWeakThisPtr;

    private:

        // UdpServerTransport needs to explicitly set mIoState to Reading,
        // since it doesn't use async I/O with callbacks to RcfServer.
        friend class UdpServerTransport;
        friend class UdpNetworkSession;
        friend class FileStreamImpl;

#if RCF_FEATURE_FILETRANSFER==1

    private:

        friend class FileTransferService;

        FileDownloadInfoPtr                     mDownloadInfoPtr;
        FileUploadInfoPtr                       mUploadInfoPtr;

        typedef std::map<boost::uint32_t, FileUploadInfoPtr> SessionUploads;
        typedef std::map<boost::uint32_t, FileDownload> SessionDownloads;

        SessionUploads                          mSessionUploads;
        SessionDownloads                        mSessionDownloads;

#endif

    private:

        boost::any                              mUserData;
        StubEntryPtr                            mDefaultStubEntryPtr;
        StubEntryPtr                            mCachedStubEntryPtr;

    public:
        NetworkSession & getNetworkSession() const;
        void setNetworkSession(NetworkSession & networkSession);

    private:
        friend class HttpSessionFilter;
        NetworkSession * mpNetworkSession;

    public:
        std::string mCurrentCallDesc;

    public:

        tstring getClientUsername();
        TransportProtocol getTransportProtocol();
        TransportType getTransportType();

        bool getEnableCompression();

        CertificatePtr getClientCertificatePtr();

        bool getIsCallbackSession() const;
        void setIsCallbackSession(bool isCallbackSession);

        RemoteCallRequest getRemoteCallRequest() const;

        time_t getConnectedAtTime() const;

        std::size_t getConnectionDuration() const;

        std::size_t getRemoteCallCount() const;
        boost::uint64_t getTotalBytesReceived() const;
        boost::uint64_t getTotalBytesSent() const;

        bool isConnected() const;

    private:

        void setConnectedAtTime(time_t connectedAtTime);

        friend class SspiServerFilter;
        friend class Win32NamedPipeNetworkSession;

        tstring mClientUsername;
        TransportProtocol mTransportProtocol;
        bool mEnableCompression;

        bool mTransportProtocolVerified;
        bool mIsCallbackSession;

        time_t mConnectedAtTime;

        std::size_t mRemoteCallCount;
    };       

} // namespace RCF

#endif // ! INCLUDE_RCF_RCFSESSION_HPP
