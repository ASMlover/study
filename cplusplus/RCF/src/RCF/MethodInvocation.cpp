
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

#include <RCF/MethodInvocation.hpp>

#include <vector>

#include <boost/mpl/assert.hpp>

#include <RCF/CurrentSession.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/Service.hpp>
#include <RCF/ThreadLocalData.hpp>

#include <SF/Encoding.hpp>

namespace RCF {

    void encodeServerError(RcfServer & server, ByteBuffer & byteBuffer, int error)
    {
        RCF_UNUSED_VARIABLE(server);

        const std::size_t Len = 4+1+1+4;

        if (byteBuffer.getLength() + byteBuffer.getLeftMargin() < Len)
        {
            byteBuffer = ByteBuffer(Len);
        }

        byteBuffer.setLeftMargin(4);

        std::size_t pos = 0;
        SF::encodeInt(Descriptor_Error, byteBuffer, pos);
        SF::encodeInt(0, byteBuffer, pos);
        SF::encodeInt(error, byteBuffer, pos);
    }

    void encodeServerError(RcfServer & server, ByteBuffer & byteBuffer, int error, int arg0, int arg1)
    {

        const std::size_t Len = 4+1+1+4+4;

        if (byteBuffer.getLength() + byteBuffer.getLeftMargin() < Len)
        {
            byteBuffer = ByteBuffer(Len);
        }

        byteBuffer.setLeftMargin(4);

        int ver = 0;
        if (server.getRuntimeVersion() >= 8)
        {
            ver = 1;
        }

        // without RCF:: qualifiers, borland chooses not to generate any code at all...
        std::size_t pos = 0;
        SF::encodeInt(Descriptor_Error, byteBuffer, pos);
        SF::encodeInt(ver, byteBuffer, pos);
        SF::encodeInt(error, byteBuffer, pos);
        SF::encodeInt(arg0, byteBuffer, pos);

        if (ver == 1)
        {
            SF::encodeInt(arg1, byteBuffer, pos);
        }
    }

    //*************************************
    // MethodInvocationRequest

    RemoteCallRequest::RemoteCallRequest(const MethodInvocationRequest & req) :
        mServantBindingName(req.mService),
        mInterfaceName(req.mSubInterface),
        mFnId(req.mFnId),
        mOneway(req.mOneway),
        mSerializationProtocol(req.mSerializationProtocol),
        mRuntimeVersion(req.mRuntimeVersion),
        mArchiveVersion(req.mArchiveVersion),
        mPingBackIntervalMs(req.mPingBackIntervalMs),
        mUseNativeWstringSerialization(req.mUseNativeWstringSerialization),
        mEnableSfPointerTracking(req.mEnableSfPointerTracking)
    {
    }

    MethodInvocationRequest::MethodInvocationRequest() :
        mToken(),
        mSubInterface(),
        mFnId(0),
        mSerializationProtocol(DefaultSerializationProtocol),
        mMarshalingProtocol(DefaultMarshalingProtocol),
        mOneway(false),
        mClose(false),
        mService(),
        mRuntimeVersion(0),
        mIgnoreRuntimeVersion(false),
        mPingBackIntervalMs(0),
        mArchiveVersion(0),
        mUseNativeWstringSerialization(false),
        mEnableSfPointerTracking(false)
    {
    }
    
    void MethodInvocationRequest::init(
        const Token &           token,
        const std::string &     service,
        const std::string &     subInterface,
        int                     fnId,
        SerializationProtocol   serializationProtocol,
        MarshalingProtocol      marshalingProtocol,
        bool                    oneway,
        bool                    close,
        int                     runtimeVersion,
        bool                    ignoreRuntimeVersion,
        boost::uint32_t         pingBackIntervalMs,
        int                     archiveVersion,
        bool                    useNativeWstringSerialization,
        bool                    enableSfPointerTracking)
    {
        mToken                      = token;
        mService                    = service;
        mSubInterface               = subInterface;
        mFnId                       = fnId;
        mSerializationProtocol      = serializationProtocol;
        mMarshalingProtocol         = marshalingProtocol;
        mOneway                     = oneway;
        mClose                      = close;
        mRuntimeVersion             = runtimeVersion;
        mIgnoreRuntimeVersion       = ignoreRuntimeVersion;
        mPingBackIntervalMs         = pingBackIntervalMs;
        mArchiveVersion             = archiveVersion;
        mUseNativeWstringSerialization = useNativeWstringSerialization;
        mEnableSfPointerTracking    = enableSfPointerTracking;
    }

    void MethodInvocationRequest::init(
        int                     runtimeVersion)
    {
        mRuntimeVersion             = runtimeVersion;
        mOneway                     = true;
    }

    void MethodInvocationRequest::init(
        const MethodInvocationRequest & rhs)
    {
        init(
            rhs.mToken, 
            rhs.mService, 
            rhs.mSubInterface, 
            rhs.mFnId, 
            rhs.mSerializationProtocol,
            rhs.mMarshalingProtocol, 
            rhs.mOneway, 
            rhs.mClose, 
            rhs.mRuntimeVersion, 
            rhs.mIgnoreRuntimeVersion,
            rhs.mPingBackIntervalMs, 
            rhs.mArchiveVersion, 
            rhs.mUseNativeWstringSerialization,
            rhs.mEnableSfPointerTracking);
    }

    Token MethodInvocationRequest::getToken() const
    {
        return mToken;
    }

    const std::string & MethodInvocationRequest::getSubInterface() const
    {
        return mSubInterface;
    }

    int MethodInvocationRequest::getFnId() const
    {
        return mFnId;
    }

    bool MethodInvocationRequest::getOneway() const
    {
        return mOneway;
    }

    bool MethodInvocationRequest::getClose() const
    {
        return mClose;
    }

    const std::string & MethodInvocationRequest::getService() const
    {
        return mService;
    }

    void MethodInvocationRequest::setService(const std::string &service)
    {
        mService = service;
    }

    int MethodInvocationRequest::getPingBackIntervalMs()
    {
        return mPingBackIntervalMs;
    }

    bool MethodInvocationRequest::decodeRequest(
        const ByteBuffer & message,
        ByteBuffer & messageBody,
        RcfSessionPtr rcfSessionPtr,
        RcfServer & rcfServer)
    {
        ByteBuffer buffer;

        ThreadLocalCached< std::vector<FilterPtr> > tlcNoFilters;
        std::vector<FilterPtr> &noFilters = tlcNoFilters.get();

        // Unfilter the message.
        decodeFromMessage(
            message,
            buffer,
            &rcfServer,
            rcfSessionPtr,
            noFilters);

        // Decode the request header.
        std::size_t pos = 0;
        mRuntimeVersion = 1;
         
        int tokenId = 0;
        int msgId = 0;
        int messageVersion = 0;
            
        bool ignoreRuntimeVersion = false;

        // For backwards compatibility.
        mEnableSfPointerTracking = true;

        SF::decodeInt(msgId, buffer, pos);
        RCF_VERIFY(msgId == Descriptor_Request, Exception(_RcfError_Decoding()))(msgId);
        SF::decodeInt(messageVersion, buffer, pos);
            
        if (messageVersion > 7)
        {
            return false;
        }

        SF::decodeString(mService, buffer, pos);
        SF::decodeInt(tokenId, buffer, pos);
        SF::decodeString(mSubInterface, buffer, pos);
        SF::decodeInt(mFnId, buffer, pos);
            
        int sp = 0;
        SF::decodeInt(sp, buffer, pos);
        mSerializationProtocol = SerializationProtocol(sp);

        SF::decodeBool(mOneway, buffer, pos);
        SF::decodeBool(mClose, buffer, pos);

        if (messageVersion == 1)
        {
            SF::decodeInt(mRuntimeVersion, buffer, pos);
            SF::decodeBool(ignoreRuntimeVersion, buffer, pos);
            mPingBackIntervalMs = 0;
        }
        else if (messageVersion == 2)
        {
            SF::decodeInt(mRuntimeVersion, buffer, pos);
            SF::decodeBool(ignoreRuntimeVersion, buffer, pos);
            SF::decodeInt(mPingBackIntervalMs, buffer, pos);
        }
        else if (messageVersion == 3)
        {
            SF::decodeInt(mRuntimeVersion, buffer, pos);
            SF::decodeBool(ignoreRuntimeVersion, buffer, pos);
            SF::decodeInt(mPingBackIntervalMs, buffer, pos);
            SF::decodeInt(mArchiveVersion, buffer, pos);
        }
        else if (messageVersion == 4)
        {
            SF::decodeInt(mRuntimeVersion, buffer, pos);
            SF::decodeBool(ignoreRuntimeVersion, buffer, pos);
            SF::decodeInt(mPingBackIntervalMs, buffer, pos);
            SF::decodeInt(mArchiveVersion, buffer, pos);
            SF::decodeByteBuffer(mRequestUserData, buffer, pos);
        }
        else if (messageVersion == 5)
        {
            SF::decodeInt(mRuntimeVersion, buffer, pos);
            SF::decodeBool(ignoreRuntimeVersion, buffer, pos);
            SF::decodeInt(mPingBackIntervalMs, buffer, pos);
            SF::decodeInt(mArchiveVersion, buffer, pos);
            SF::decodeByteBuffer(mRequestUserData, buffer, pos);
            SF::decodeBool(mUseNativeWstringSerialization, buffer, pos);
        }
        else if (messageVersion == 6)
        {
            SF::decodeInt(mRuntimeVersion, buffer, pos);
            SF::decodeBool(ignoreRuntimeVersion, buffer, pos);
            SF::decodeInt(mPingBackIntervalMs, buffer, pos);
            SF::decodeInt(mArchiveVersion, buffer, pos);
            SF::decodeByteBuffer(mRequestUserData, buffer, pos);
            SF::decodeBool(mUseNativeWstringSerialization, buffer, pos);
            SF::decodeBool(mEnableSfPointerTracking, buffer, pos);
        }
        else if (messageVersion == 7)
        {
            SF::decodeInt(mRuntimeVersion, buffer, pos);
            SF::decodeBool(ignoreRuntimeVersion, buffer, pos);
            SF::decodeInt(mPingBackIntervalMs, buffer, pos);
            SF::decodeInt(mArchiveVersion, buffer, pos);
            SF::decodeByteBuffer(mRequestUserData, buffer, pos);
            SF::decodeBool(mUseNativeWstringSerialization, buffer, pos);
            SF::decodeBool(mEnableSfPointerTracking, buffer, pos);
            SF::decodeByteBuffer(mOutOfBandRequest, buffer, pos);
        }
            
        mToken = Token(tokenId);            

        if (mSubInterface.empty())
        {
            mSubInterface = mService;
        }

        // Check runtime version.
        if (mRuntimeVersion > rcfServer.getRuntimeVersion())
        {
            return false;
        }
        else
        {
            rcfSessionPtr->setRuntimeVersion(mRuntimeVersion);
        }

        // Check archive version.
        boost::uint32_t serverArchiveVersion = rcfServer.getArchiveVersion();
        if (serverArchiveVersion && mArchiveVersion > serverArchiveVersion)
        {
            return false;
        }
        else
        {
            rcfSessionPtr->setArchiveVersion(mArchiveVersion);
        }

        rcfSessionPtr->setNativeWstringSerialization(mUseNativeWstringSerialization);

        messageBody = ByteBuffer(buffer, pos);

        return true;
    }

    bool MethodInvocationRequest::encodeResponse(
        const RemoteException *         pRe,
        ByteBuffer &                    buffer,
        bool                            enableSfPointerTracking)
    {
        bool isException = pRe ? true : false;

        RCF_ASSERT(!mVecPtr || mVecPtr.unique());
        if (!mVecPtr)
        {
            mVecPtr.reset(new std::vector<char>(50));
        }

        int runtimeVersion = mRuntimeVersion;
        int messageVersion = 0;

        if (runtimeVersion < 7)
        {
            messageVersion = 0;
        }
        else if (runtimeVersion <= 9)
        {
            messageVersion = 1;
        }
        else if (runtimeVersion <= 11)
        {
            messageVersion = 2;
        }
        else
        {
            messageVersion = 3;
        }

        std::size_t pos = 0;
        BOOST_STATIC_ASSERT(0 <= Descriptor_Response && Descriptor_Response < 255);
        SF::encodeInt(Descriptor_Response, *mVecPtr, pos);
        SF::encodeInt(messageVersion, *mVecPtr, pos);
        SF::encodeBool(isException, *mVecPtr, pos);

        if (messageVersion == 1)
        {
            SF::encodeByteBuffer(mResponseUserData, *mVecPtr, pos);
        }
        else if (messageVersion == 2)
        {
            SF::encodeByteBuffer(mResponseUserData, *mVecPtr, pos);
            SF::encodeBool(enableSfPointerTracking, *mVecPtr, pos);
        }
        else if (messageVersion == 3)
        {
            SF::encodeByteBuffer(mResponseUserData, *mVecPtr, pos);
            SF::encodeBool(enableSfPointerTracking, *mVecPtr, pos);
            SF::encodeByteBuffer(mOutOfBandResponse, *mVecPtr, pos);
        }

        mVecPtr->resize(pos);

        buffer = ByteBuffer(mVecPtr);

        // If there was an exception, it still needs to be serialized. 
        return true;
    }

    const std::string EmptyString;

    ByteBuffer MethodInvocationRequest::encodeRequestHeader()
    {
        RCF_ASSERT(!mVecPtr || mVecPtr.unique());
        if (!mVecPtr)
        {
            mVecPtr.reset(new std::vector<char>(50));
        }

        int runtimeVersion = mRuntimeVersion;
        int messageVersion = 0;

        if (runtimeVersion < 2)
        {
            messageVersion = 0;
        }
        else if (runtimeVersion < 5)
        {
            messageVersion = 1;
        }
        else if (runtimeVersion == 5)
        {
            messageVersion = 2;
        }
        else if (runtimeVersion == 6)
        {
            messageVersion = 3;
        }
        else if (runtimeVersion == 7)
        {
            messageVersion = 4;
        }
        else if (runtimeVersion <= 9)
        {
            messageVersion = 5;
        }
        else if (runtimeVersion <= 11)
        {
            messageVersion = 6;
        }
        else
        {
            messageVersion = 7;
        }

        std::size_t pos = 0;
        SF::encodeInt(Descriptor_Request, *mVecPtr, pos);
        SF::encodeInt(messageVersion, *mVecPtr, pos);
        SF::encodeString(mService, *mVecPtr, pos);
        SF::encodeInt(mToken.getId(), *mVecPtr, pos);

        mSubInterface == mService ?
            SF::encodeString(EmptyString, *mVecPtr, pos) :
            SF::encodeString(mSubInterface, *mVecPtr, pos);

        SF::encodeInt(mFnId, *mVecPtr, pos);
        SF::encodeInt(mSerializationProtocol, *mVecPtr, pos);
        SF::encodeBool(mOneway, *mVecPtr, pos);
        SF::encodeBool(mClose, *mVecPtr, pos);

        if (messageVersion == 1)
        {
            SF::encodeInt(mRuntimeVersion, *mVecPtr, pos);
            SF::encodeBool(mIgnoreRuntimeVersion, *mVecPtr, pos);
        }
        else if (messageVersion == 2)
        {
            SF::encodeInt(mRuntimeVersion, *mVecPtr, pos);
            SF::encodeBool(mIgnoreRuntimeVersion, *mVecPtr, pos);
            SF::encodeInt(mPingBackIntervalMs, *mVecPtr, pos);
        }
        else if (messageVersion == 3)
        {
            SF::encodeInt(mRuntimeVersion, *mVecPtr, pos);
            SF::encodeBool(mIgnoreRuntimeVersion, *mVecPtr, pos);
            SF::encodeInt(mPingBackIntervalMs, *mVecPtr, pos);
            SF::encodeInt(mArchiveVersion, *mVecPtr, pos);
        }
        else if (messageVersion == 4)
        {
            SF::encodeInt(mRuntimeVersion, *mVecPtr, pos);
            SF::encodeBool(mIgnoreRuntimeVersion, *mVecPtr, pos);
            SF::encodeInt(mPingBackIntervalMs, *mVecPtr, pos);
            SF::encodeInt(mArchiveVersion, *mVecPtr, pos);
            SF::encodeByteBuffer(mRequestUserData, *mVecPtr, pos);
        }
        else if (messageVersion == 5)
        {
            SF::encodeInt(mRuntimeVersion, *mVecPtr, pos);
            SF::encodeBool(mIgnoreRuntimeVersion, *mVecPtr, pos);
            SF::encodeInt(mPingBackIntervalMs, *mVecPtr, pos);
            SF::encodeInt(mArchiveVersion, *mVecPtr, pos);
            SF::encodeByteBuffer(mRequestUserData, *mVecPtr, pos);
            SF::encodeBool(mUseNativeWstringSerialization, *mVecPtr, pos);
        }
        else if (messageVersion == 6)
        {
            SF::encodeInt(mRuntimeVersion, *mVecPtr, pos);
            SF::encodeBool(mIgnoreRuntimeVersion, *mVecPtr, pos);
            SF::encodeInt(mPingBackIntervalMs, *mVecPtr, pos);
            SF::encodeInt(mArchiveVersion, *mVecPtr, pos);
            SF::encodeByteBuffer(mRequestUserData, *mVecPtr, pos);
            SF::encodeBool(mUseNativeWstringSerialization, *mVecPtr, pos);
            SF::encodeBool(mEnableSfPointerTracking, *mVecPtr, pos);
        }
        else if (messageVersion == 7)
        {
            SF::encodeInt(mRuntimeVersion, *mVecPtr, pos);
            SF::encodeBool(mIgnoreRuntimeVersion, *mVecPtr, pos);
            SF::encodeInt(mPingBackIntervalMs, *mVecPtr, pos);
            SF::encodeInt(mArchiveVersion, *mVecPtr, pos);
            SF::encodeByteBuffer(mRequestUserData, *mVecPtr, pos);
            SF::encodeBool(mUseNativeWstringSerialization, *mVecPtr, pos);
            SF::encodeBool(mEnableSfPointerTracking, *mVecPtr, pos);
            SF::encodeByteBuffer(mOutOfBandRequest, *mVecPtr, pos);
        }

        mVecPtr->resize(pos);

        return ByteBuffer(mVecPtr);
    }

    void MethodInvocationRequest::encodeRequest(
        const std::vector<ByteBuffer> &buffers,
        std::vector<ByteBuffer> &message,
        const std::vector<FilterPtr> &filters)
    {
        encodeToMessage(
            message,
            buffers,
            filters);
    }

    void MethodInvocationRequest::decodeResponse(const ByteBuffer &message,
        ByteBuffer &buffer,
        MethodInvocationResponse &response,
        const std::vector<FilterPtr> &filters)
    {
        decodeFromMessage(
            message,
            buffer,
            NULL,
            RcfSessionPtr(),
            filters);

        std::size_t pos = 0;

        // Decode response header.
        int msgId = 0;
        SF::decodeInt(msgId, buffer, pos);

        int ver = 0;
        SF::decodeInt(ver, buffer, pos);
           
        if (msgId == Descriptor_Error)
        {
            RCF_VERIFY(ver <= 1, Exception(_RcfError_Decoding()))(ver);

            int error = 0;
            SF::decodeInt(error, buffer, pos);
            response.mException = false;
            response.mError = true;
            response.mErrorCode = error;
            if (
                error == RcfError_VersionMismatch ||
                error == RcfError_PingBack)
            {
                SF::decodeInt(response.mArg0, buffer, pos);

                if (ver == 1)
                {
                    SF::decodeInt(response.mArg1, buffer, pos);
                }
            }
        }
        else
        {
            RCF_VERIFY(msgId == Descriptor_Response, Exception(_RcfError_Decoding()))(msgId);
            RCF_VERIFY(ver <= 3, Exception(_RcfError_Decoding()))(ver);

            // For backwards compatibility.
            response.mEnableSfPointerTracking = true;

            SF::decodeBool(response.mException, buffer, pos);

            if (ver == 1)
            {
                SF::decodeByteBuffer(mResponseUserData, buffer, pos);
            }
            else if (ver == 2)
            {
                SF::decodeByteBuffer(mResponseUserData, buffer, pos);
                SF::decodeBool(response.mEnableSfPointerTracking, buffer, pos);
            }
            else if (ver == 3)
            {
                SF::decodeByteBuffer(mResponseUserData, buffer, pos);
                SF::decodeBool(response.mEnableSfPointerTracking, buffer, pos);
                SF::decodeByteBuffer(mOutOfBandResponse, buffer, pos);
            }

            response.mError = false;
            response.mErrorCode = 0;
            response.mArg0 = 0;
            response.mArg1 = 0;
        }

        // Return the response body.
        buffer = ByteBuffer(buffer, pos);
    }

    StubEntryPtr MethodInvocationRequest::locateStubEntryPtr(
        RcfServer &rcfServer)
    {
        Token targetToken = getToken();
        const std::string & targetName = getService();
        StubEntryPtr stubEntryPtr;
        RcfSession * pRcfSession = getTlsRcfSessionPtr();

        if (targetToken != Token())
        {
            stubEntryPtr = rcfServer.findStubEntryForToken(targetToken);
        }
        else if (!targetName.empty())
        {
            ReadLock readLock(rcfServer.mStubMapMutex);
            const std::string & servantName = getService();
            RcfServer::StubMap::iterator iter = rcfServer.mStubMap.find(servantName);
            if (iter != rcfServer.mStubMap.end())
            {
                stubEntryPtr = (*iter).second;
            }
        }
        else
        {
            stubEntryPtr = pRcfSession->getDefaultStubEntryPtr();
        }

        return stubEntryPtr;
    }

   
    //*******************************************
    // MethodInvocationResponse

    MethodInvocationResponse::MethodInvocationResponse() :
        mException(),
        mError(),
        mErrorCode(),
        mArg0(),
        mArg1(),
        mEnableSfPointerTracking(false)
    {}

    bool MethodInvocationResponse::isException() const
    {
        return mException;
    }

    bool MethodInvocationResponse::isError() const
    {
        return mError;
    }

    int MethodInvocationResponse::getError() const
    {
        return mErrorCode;
    }

    int MethodInvocationResponse::getArg0() const
    {
        return mArg0;
    }

    int MethodInvocationResponse::getArg1() const
    {
        return mArg1;
    }

    std::auto_ptr<RemoteException> MethodInvocationResponse::getExceptionPtr()
    {
        return mExceptionPtr;
    }

    bool MethodInvocationResponse::getEnableSfPointerTracking() const
    {
        return mEnableSfPointerTracking;
    }

    //*******************************************

    void MethodInvocationRequest::encodeToMessage(
        std::vector<ByteBuffer> &message,
        const std::vector<ByteBuffer> &buffers,
        const std::vector<FilterPtr> &filters)
    {
        if (filters.empty())
        {
            // No filters, so nothing to do really.

            message.resize(0);

            std::copy(
                buffers.begin(),
                buffers.end(),
                std::back_inserter(message));
        }
        else
        {
            // Pass the buffers through the filters.

            ThreadLocalCached< std::vector<ByteBuffer> > tlcFilteredBuffers;
            std::vector<ByteBuffer> &filteredBuffers = tlcFilteredBuffers.get();

            std::size_t unfilteredLen = lengthByteBuffers(buffers);
            bool ok = filterData(buffers, filteredBuffers, filters);
            RCF_VERIFY(ok, Exception(_RcfError_FilterMessage()));

            message.resize(0);

            std::copy(
                filteredBuffers.begin(),
                filteredBuffers.end(),
                std::back_inserter(message));

            if (filteredBuffers.empty())
            {
                // Can happen if buffers has e.g. a single zero length buffer.
                RCF_ASSERT_EQ(lengthByteBuffers(buffers) , 0);
                RCF_ASSERT(!buffers.empty());
                RCF_ASSERT_EQ(buffers.front().getLength() , 0);
                message.push_back(buffers.front());
            }

            // Encode the filter header.
            const std::size_t BufferLen = (5+10)*4;
            char buffer[BufferLen];
            ByteBuffer byteBuffer(&buffer[0], BufferLen);

            std::size_t pos = 0;

            // Descriptor
            SF::encodeInt(Descriptor_FilteredPayload, byteBuffer, pos);

            // Version.
            SF::encodeInt(0, byteBuffer, pos);

            RCF_VERIFY(
                filters.size() <= 10,
                Exception(_RcfError_FilterCount(filters.size(), 10)));

            // Number of filters.
            SF::encodeInt(static_cast<int>(filters.size()), byteBuffer, pos);

            // Filter ids.
            for (std::size_t i=0; i<filters.size(); ++i)
            {
                int filterId = filters[i]->getFilterId();
                SF::encodeInt(filterId, byteBuffer, pos);
            }

            // Legacy, not used anymore.
            int filterHeaderLen = 0;
            SF::encodeInt(filterHeaderLen, byteBuffer, pos);

            // Length of the message when it is unfiltered.
            int len = static_cast<int>(unfilteredLen);
            SF::encodeInt(len, byteBuffer, pos);

            // Copy the filter header into the left margin of the first buffer.
            std::size_t headerLen = pos;
            
            RCF_ASSERT_LTEQ(headerLen, BufferLen);
            
            RCF_ASSERT(
                !message.empty() &&
                message.front().getLeftMargin() >= headerLen)
                (message.front().getLeftMargin())(headerLen);

            if ( headerLen > BufferLen )
            {
                RCF_THROW(_RcfError_MessageHeaderEncoding(BufferLen, headerLen));
            }

            ByteBuffer &front = message.front();
            front.expandIntoLeftMargin(headerLen);
            memcpy(front.getPtr(), &buffer[0], headerLen);
        }
    }

    struct FilterIdComparison
    {
        bool operator()(FilterPtr filterPtr, int filterId)
        {
            return filterPtr->getFilterId() == filterId;
        }
    };

#if RCF_FEATURE_PROTOBUF==1
    static const bool SupportProtobufs = true;
#else
    static const bool SupportProtobufs = false;
#endif

    void MethodInvocationRequest::decodeFromMessage(
        const ByteBuffer &message,
        ByteBuffer &buffer,
        RcfServer *pRcfServer,
        RcfSessionPtr rcfSessionPtr,
        const std::vector<FilterPtr> &existingFilters)
    {
        ThreadLocalCached< std::vector<int> > tlcFilterIds;
        std::vector<int> &filterIds = tlcFilterIds.get();

        // Decode the filter header, if there is one.
        std::size_t pos = 0;
        std::size_t unfilteredLen = 0;

        mMarshalingProtocol = Mp_Rcf;

        char * const pch = (char*) message.getPtr() ;
        if (pch[0] == Descriptor_FilteredPayload)
        {
            int descriptor = 0;
            SF::decodeInt(descriptor, message, pos);
            RCF_VERIFY(descriptor == Descriptor_FilteredPayload, Exception(_RcfError_Decoding()))(descriptor);

            int version = 0;
            SF::decodeInt(version, message, pos);
            RCF_VERIFY(version == 0, Exception(_RcfError_Decoding()))(version);

            int filterCount = 0;
            SF::decodeInt(filterCount, message, pos);
            RCF_VERIFY(0 < filterCount && filterCount <= 10, Exception(_RcfError_Decoding()))(filterCount);

            filterIds.resize(0);
            for (int i=0; i<filterCount; ++i)
            {
                int filterId = 0;
                SF::decodeInt(filterId, message, pos);
                filterIds.push_back(filterId);
            }

            int clearLen = 0;
            SF::decodeInt(clearLen, message, pos);
            RCF_VERIFY(0 <= clearLen, Exception(_RcfError_Decoding()))(clearLen);
               
            int unfilteredLen_ = 0;
            SF::decodeInt(unfilteredLen_, message, pos);
            RCF_VERIFY(0 <= unfilteredLen_, Exception(_RcfError_Decoding()))(unfilteredLen_);
            unfilteredLen = unfilteredLen_;
        }

        ByteBuffer filteredData(message, pos);

        if (pRcfServer)
        {
            // Server side decoding.

            if (filterIds.empty())
            {
                rcfSessionPtr->setFiltered(false);
                buffer = filteredData;
            }
            else
            {
                rcfSessionPtr->setFiltered(true);
                std::vector<FilterPtr> &filters = rcfSessionPtr->getFilters();

                if (    filters.size() != filterIds.size() 
                    ||    !std::equal(
                            filters.begin(),
                            filters.end(),
                            filterIds.begin(),
                            FilterIdComparison()))
                {
                    filters.clear();

                    std::transform(
                        filterIds.begin(),
                        filterIds.end(),
                        std::back_inserter(filters),
                        boost::bind( &RcfServer::createFilter, pRcfServer, _1) );

                    if (
                        std::find_if(
                            filters.begin(),
                            filters.end(),
                            SharedPtrIsNull()) == filters.end())
                    {
                        connectFilters(filters);
                    }
                    else
                    {
                        // TODO: better not to throw exceptions here?
                        Exception e(_RcfError_UnknownFilter());
                        RCF_THROW(e);
                    }
                }

                bool bRet = unfilterData(
                    filteredData,
                    buffer,
                    unfilteredLen,
                    filters);

                RCF_ASSERT(bRet);
                RCF_UNUSED_VARIABLE(bRet);
            }
        }
        else
        {
            // Client side decoding.

            if (    existingFilters.size() == filterIds.size() 
                &&    std::equal(
                        existingFilters.begin(),
                        existingFilters.end(),
                        filterIds.begin(),
                        FilterIdComparison()))
            {
                if (existingFilters.empty())
                {
                    buffer = filteredData;
                }
                else
                {
                    bool bRet = unfilterData(
                        filteredData,
                        buffer,
                        unfilteredLen,
                        existingFilters);

                    RCF_VERIFY(bRet, Exception(_RcfError_UnfilterMessage()));
                }
            }
            else
            {
                Exception e(_RcfError_PayloadFilterMismatch());
                RCF_THROW(e);
            }
        }
    }

    // Out of band messages.

    OobMessage::OobMessage(int runtimeVersion) : 
        mRuntimeVersion(runtimeVersion), 
        mResponseError(0)
    {
    }

    OobMessage::~OobMessage()
    {
    }

    void OobMessage::encodeResponse(ByteBuffer & buffer)
    {
        boost::shared_ptr< std::vector<char> > vecPtr( new std::vector<char>(50) );
        std::size_t pos = 0;
        encodeResponseCommon(vecPtr, pos);
        vecPtr->resize(pos);
        buffer = ByteBuffer(vecPtr);
    }

    void OobMessage::decodeResponse(const ByteBuffer & buffer)
    {
        std::size_t pos = 0;
        decodeResponseCommon(buffer, pos);
    }

    void OobMessage::encodeRequestCommon(VecPtr vecPtr, std::size_t & pos)
    {
        int magicNumber = 65;
        int msgType = getMessageType();
        int msgVersion = mRuntimeVersion;

        SF::encodeInt(magicNumber, *vecPtr, pos);
        SF::encodeInt(msgType, *vecPtr, pos);
        SF::encodeInt(msgVersion, *vecPtr, pos);
    }

    void OobMessage::encodeResponseCommon(VecPtr vecPtr, std::size_t & pos)
    {
        int magicNumber = 66;
        int msgType = getMessageType();
        int msgVersion = mRuntimeVersion;

        SF::encodeInt(magicNumber, *vecPtr, pos);
        SF::encodeInt(msgType, *vecPtr, pos);
        SF::encodeInt(msgVersion, *vecPtr, pos);
        SF::encodeInt(mResponseError, *vecPtr, pos);
        SF::encodeString(mResponseErrorString, *vecPtr, pos);
    }

    void OobMessage::decodeResponseCommon(const ByteBuffer & buffer, std::size_t & pos)
    {
        int magicNumber = 0;
        int msgType = 0;
        int msgVersion = 0;

        SF::decodeInt(magicNumber, buffer, pos);
        RCF_VERIFY(magicNumber == 66, Exception(_RcfError_Decoding()));

        SF::decodeInt(msgType, buffer, pos);
        RCF_VERIFY(msgType == getMessageType(), Exception(_RcfError_Decoding()));

        SF::decodeInt(msgVersion, buffer, pos);
        RCF_VERIFY(msgVersion == mRuntimeVersion, Exception(_RcfError_Decoding()));

        SF::decodeInt(mResponseError, buffer, pos);
        SF::decodeString(mResponseErrorString, buffer, pos);
    }

    OobMessagePtr OobMessage::decodeRequestCommon(const ByteBuffer & buffer)
    {
        std::size_t pos = 0;

        int magicNumber = 0;
        int msgType = 0;
        int msgVersion = 0;

        SF::decodeInt(magicNumber, buffer, pos);
        RCF_VERIFY(magicNumber == 65, Exception(_RcfError_Decoding()));

        SF::decodeInt(msgType, buffer, pos);
        SF::decodeInt(msgVersion, buffer, pos);

        OobMessagePtr msgPtr;
        switch (msgType) 
        {
        case Omt_RequestTransportFilters:
            msgPtr.reset( new OobRequestTransportFilters(msgVersion) );
            break;

        case Omt_CreateCallbackConnection:
            msgPtr.reset( new OobCreateCallbackConnection(msgVersion) );
            break;

        case Omt_RequestSubscription:
            msgPtr.reset( new OobRequestSubscription(msgVersion) );
            break;

        default:
            RCF_THROW( Exception(_RcfError_Decoding()) );
        }

        msgPtr->decodeRequest(buffer, pos);

        return msgPtr;
    }

    // OobRequestTransportFilters

    OobRequestTransportFilters::OobRequestTransportFilters(int runtimeVersion) :
        OobMessage(runtimeVersion)
    {
    }

    OobRequestTransportFilters::OobRequestTransportFilters(
        int runtimeVersion, 
        const std::vector<FilterPtr> &filters) : 
            OobMessage(runtimeVersion)
    {
        for (std::size_t i=0; i<filters.size(); ++i)
        {
            mFilterIds.push_back( filters[i]->getFilterId() );
        }
    }

    OobMessageType OobRequestTransportFilters::getMessageType()
    {
        return Omt_RequestTransportFilters;
    }

    void OobRequestTransportFilters::encodeRequest(ByteBuffer & buffer)
    {
        boost::shared_ptr< std::vector<char> > vecPtr( new std::vector<char>(50) );
        std::size_t pos = 0;

        encodeRequestCommon(vecPtr, pos);

        if (mRuntimeVersion <= 12)
        {
            int filterCount = static_cast<int>(mFilterIds.size());
            SF::encodeInt(filterCount, *vecPtr, pos);
            for (std::size_t i=0; i<mFilterIds.size(); ++i)
            {
                SF::encodeInt(mFilterIds[i], *vecPtr, pos);
            }
        }

        vecPtr->resize(pos);
        buffer = ByteBuffer(vecPtr);
    }

    void OobRequestTransportFilters::decodeRequest(const ByteBuffer & buffer, std::size_t & pos)
    {
        if (mRuntimeVersion <= 12)
        {
            int filterCount = 0;
            SF::decodeInt(filterCount, buffer, pos);
            RCF_VERIFY(filterCount <= 10, Exception(_RcfError_Decoding()));

            mFilterIds.clear();
            for (int i=0; i<filterCount; ++i)
            {
                int filterId = 0;
                SF::decodeInt(filterId, buffer, pos);
                mFilterIds.push_back(filterId);
            }
        }
    }

    // OobCreateCallbackConnection

    OobCreateCallbackConnection::OobCreateCallbackConnection(int runtimeVersion) :
        OobMessage(runtimeVersion)
    {
    }

    OobMessageType OobCreateCallbackConnection::getMessageType()
    {
        return Omt_CreateCallbackConnection;
    }

    void OobCreateCallbackConnection::encodeRequest(ByteBuffer & buffer)
    {
        boost::shared_ptr< std::vector<char> > vecPtr( new std::vector<char>(50) );
        std::size_t pos = 0;
        encodeRequestCommon(vecPtr, pos);
        vecPtr->resize(pos);
        buffer = ByteBuffer(vecPtr);
    }

    void OobCreateCallbackConnection::decodeRequest(
        const ByteBuffer & buffer, 
        std::size_t & pos)
    {
        RCF_UNUSED_VARIABLE(buffer);
        RCF_UNUSED_VARIABLE(pos);
    }

    // OobRequestSubscription

    OobRequestSubscription::OobRequestSubscription(int runtimeVersion) :
        OobMessage(runtimeVersion),
        mPublisherName(),
        mSubToPubPingIntervalMs(0),
        mPubToSubPingIntervalMs(0)
    {
    }

    OobRequestSubscription::OobRequestSubscription(
        int                     runtimeVersion,
        const std::string &     publisherName, 
        boost::uint32_t         subToPubPingIntervalMs) :
            OobMessage(runtimeVersion),
            mPublisherName(publisherName),
            mSubToPubPingIntervalMs(subToPubPingIntervalMs),
            mPubToSubPingIntervalMs(0)
    {
    }

    OobMessageType OobRequestSubscription::getMessageType()
    {
        return Omt_RequestSubscription;
    }

    void OobRequestSubscription::encodeRequest(ByteBuffer & buffer)
    {
        boost::shared_ptr< std::vector<char> > vecPtr( new std::vector<char>(50) );
        std::size_t pos = 0;
        encodeRequestCommon(vecPtr, pos);

        SF::encodeString(mPublisherName, *vecPtr, pos);
        SF::encodeInt(mSubToPubPingIntervalMs, *vecPtr, pos);

        vecPtr->resize(pos);
        buffer = ByteBuffer(vecPtr);
    }

    void OobRequestSubscription::decodeRequest(
        const ByteBuffer & buffer, 
        std::size_t & pos)
    {
        if (mRuntimeVersion <= 12)
        {
            SF::decodeString(mPublisherName, buffer, pos);
            SF::decodeInt(mSubToPubPingIntervalMs, buffer, pos);
        }
    }

    void OobRequestSubscription::encodeResponse(ByteBuffer & buffer)
    {
        boost::shared_ptr< std::vector<char> > vecPtr( new std::vector<char>(50) );
        std::size_t pos = 0;

        encodeResponseCommon(vecPtr, pos);
        SF::encodeInt(mPubToSubPingIntervalMs, *vecPtr, pos);

        vecPtr->resize(pos);
        buffer = ByteBuffer(vecPtr);
        }

    void OobRequestSubscription::decodeResponse(const ByteBuffer & buffer)
    {
        std::size_t pos = 0;
        decodeResponseCommon(buffer, pos);
        SF::decodeInt(mPubToSubPingIntervalMs, buffer, pos);
    }


} // namespace RCF
