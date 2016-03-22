
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

#ifndef INCLUDE_RCF_METHODINVOCATION_HPP
#define INCLUDE_RCF_METHODINVOCATION_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <RCF/Filter.hpp>
#include <RCF/ByteBuffer.hpp>
#include <RCF/Export.hpp>
#include <RCF/Exception.hpp>
#include <RCF/SerializationProtocol_Base.hpp>
#include <RCF/Token.hpp>

namespace RCF {

    class RcfServer;
    class ClientStub;
    class StubEntry;
    typedef boost::shared_ptr<StubEntry> StubEntryPtr;
    class RcfSession;
    typedef boost::shared_ptr<RcfSession> RcfSessionPtr;
    class SerializationProtocolIn;
    class SerializationProtocolOut;

    class MethodInvocationResponse;
    class MethodInvocationRequest;

    // message types
    static const int Descriptor_Error               = 0;
    static const int Descriptor_Request             = 1;
    static const int Descriptor_Response            = 2;
    static const int Descriptor_FilteredPayload     = 3;

    void encodeServerError(RcfServer & server, ByteBuffer & byteBuffer, int error);
    void encodeServerError(RcfServer & server, ByteBuffer & byteBuffer, int error, int arg0, int arg1);

    class Protobufs;

    class RemoteCallRequest
    {
    public:

        RemoteCallRequest(const MethodInvocationRequest & req);

        std::string             mServantBindingName;
        std::string             mInterfaceName;
        int                     mFnId;
        bool                    mOneway;
        SerializationProtocol   mSerializationProtocol;
        int                     mRuntimeVersion;
        int                     mArchiveVersion;
        boost::uint32_t         mPingBackIntervalMs;
        bool                    mUseNativeWstringSerialization;
        bool                    mEnableSfPointerTracking;

    };

    class RCF_EXPORT MethodInvocationRequest : boost::noncopyable
    {
    public:
        MethodInvocationRequest();

        void            init(                            
                            int                             runtimeVersion);

        void            init(
                            const MethodInvocationRequest & rhs);

        void            init(
                            const Token &                   token,
                            const std::string &             service,
                            const std::string &             subInterface,
                            int                             fnId,
                            SerializationProtocol           serializationProtocol,
                            MarshalingProtocol              marshalingProtocol,
                            bool                            oneway,
                            bool                            close,
                            int                             runtimeVersion,
                            bool                            ignoreRuntimeVersion,
                            boost::uint32_t                 pingBackIntervalMs,
                            int                             archiveVersion,
                            bool                            useNativeWstringSerialization,
                            bool                            enableSfPointerTracking);

        Token           getToken() const;
        const std::string & getSubInterface() const;
        int             getFnId() const;
        bool            getOneway() const;
        bool            getClose() const;
        const std::string & getService() const;
        void            setService(const std::string &service);
        int             getPingBackIntervalMs();

        ByteBuffer      encodeRequestHeader();

        void            encodeRequest(
                            const std::vector<ByteBuffer> & buffers,
                            std::vector<ByteBuffer> &       message,
                            const std::vector<FilterPtr> &  filters);

        bool            decodeRequest(
                            const ByteBuffer &              message,
                            ByteBuffer &                    messageBody,
                            RcfSessionPtr                   rcfSessionPtr,
                            RcfServer &                     rcfServer);

        bool            encodeResponse(
                            const RemoteException *         pRe,
                            ByteBuffer &                    buffer,
                            bool                            enableSfPointerTracking);

        void            decodeResponse(
                            const ByteBuffer &              message,
                            ByteBuffer &                    buffer,
                            MethodInvocationResponse &      response,
                            const std::vector<FilterPtr> &  filters);

        StubEntryPtr    locateStubEntryPtr(
                            RcfServer &                     rcfServer);

    private:

        friend class RcfSession;
        friend class ClientStub;
        friend class RemoteCallRequest;

        void            decodeFromMessage(
                            const ByteBuffer &              message,
                            ByteBuffer &                    buffer,
                            RcfServer *                     pRcfServer,
                            RcfSessionPtr                   rcfSessionPtr,
                            const std::vector<FilterPtr> &  existingFilters);

        void            encodeToMessage(
                            std::vector<ByteBuffer> &       message,
                            const std::vector<ByteBuffer> & buffers,
                            const std::vector<FilterPtr> &  filters);

        Token                   mToken;
        std::string             mSubInterface;
        int                     mFnId;
        SerializationProtocol   mSerializationProtocol;
        MarshalingProtocol      mMarshalingProtocol;
        bool                    mOneway;
        bool                    mClose;
        std::string             mService;
        boost::uint32_t         mRuntimeVersion;
        bool                    mIgnoreRuntimeVersion; // Legacy field, no longer used.
        int                     mPingBackIntervalMs;
        boost::uint32_t         mArchiveVersion;
        ByteBuffer              mRequestUserData;
        ByteBuffer              mResponseUserData;
        bool                    mUseNativeWstringSerialization;
        bool                    mEnableSfPointerTracking;
        ByteBuffer              mOutOfBandRequest;
        ByteBuffer              mOutOfBandResponse;

        boost::shared_ptr<std::vector<char> >   mVecPtr;
        

        friend RCF::MemOstream& operator<<(RCF::MemOstream& os, const MethodInvocationRequest& r)
        {
            os
                << NAMEVALUE(r.mToken)
                << NAMEVALUE(r.mSubInterface)
                << NAMEVALUE(r.mFnId)
                << NAMEVALUE(r.mSerializationProtocol)
                << NAMEVALUE(r.mMarshalingProtocol)
                << NAMEVALUE(r.mOneway)
                << NAMEVALUE(r.mClose)
                << NAMEVALUE(r.mService)
                << NAMEVALUE(r.mRuntimeVersion)
                << NAMEVALUE(r.mPingBackIntervalMs)
                << NAMEVALUE(r.mArchiveVersion);

            return os;
        }
    };

    class RCF_EXPORT MethodInvocationResponse
    {
    public:
        MethodInvocationResponse();

        bool    isException() const;
        bool    isError() const;
        int     getError() const;
        int     getArg0() const;
        int     getArg1() const;
        bool    getEnableSfPointerTracking() const;

        std::auto_ptr<RemoteException> getExceptionPtr();

    private:
        friend class MethodInvocationRequest;
        

        typedef std::auto_ptr<RemoteException> RemoteExceptionPtr;

        bool                mException;
        RemoteExceptionPtr  mExceptionPtr;
        bool                mError;
        int                 mErrorCode;
        int                 mArg0;
        int                 mArg1;
        bool                mEnableSfPointerTracking;

        friend RCF::MemOstream& operator<<(RCF::MemOstream& os, const MethodInvocationResponse& r)
        {
            os    << NAMEVALUE(r.mException);
            if (r.mExceptionPtr.get())
            {
                os << NAMEVALUE(*r.mExceptionPtr);
            }

            os    << NAMEVALUE(r.mError);
            if (r.mError)
            {
                os << NAMEVALUE(r.mErrorCode);
                os << NAMEVALUE(r.mArg0);
                os << NAMEVALUE(r.mArg1);
            }

            return os;
        }
    };

    // Out of band messages

    enum OobMessageType
    {
        Omt_RequestTransportFilters = 1,
        Omt_CreateCallbackConnection = 2,
        Omt_RequestSubscription = 3
    };

    class OobMessage;
    typedef boost::shared_ptr<OobMessage> OobMessagePtr;
    typedef boost::shared_ptr< std::vector<char> > VecPtr;

    class RCF_EXPORT OobMessage
    {
    public:

        OobMessage(int runtimeVersion);
        virtual ~OobMessage();

        virtual OobMessageType  getMessageType() = 0;

        virtual void            encodeRequest(ByteBuffer & buffer) = 0;
        virtual void            decodeRequest(const ByteBuffer & buffer, std::size_t & pos) = 0;

        virtual void            encodeResponse(ByteBuffer & buffer);
        virtual void            decodeResponse(const ByteBuffer & buffer);

    protected:
        void                    encodeRequestCommon(VecPtr vecPtr, std::size_t & pos);

    public:
        static OobMessagePtr    decodeRequestCommon(const ByteBuffer & buffer);

    protected:
        void                    encodeResponseCommon(VecPtr vecPtr, std::size_t & pos);
        void                    decodeResponseCommon(const ByteBuffer & buffer, std::size_t & pos);

        int                     mRuntimeVersion;

    public:

        // Common return values.
        boost::uint32_t         mResponseError;
        std::string             mResponseErrorString;
    };

    class RCF_EXPORT OobRequestTransportFilters : public OobMessage
    {
    public:
        OobRequestTransportFilters(int runtimeVersion);

        OobRequestTransportFilters(
            int runtimeVersion, 
            const std::vector<FilterPtr> &filters);

        virtual OobMessageType  getMessageType();
        virtual void            encodeRequest(ByteBuffer & buffer);
        virtual void            decodeRequest(const ByteBuffer & buffer, std::size_t & pos);

        std::vector<boost::int32_t>        mFilterIds;
    };

    class RCF_EXPORT OobCreateCallbackConnection : public OobMessage
    {
    public:
        OobCreateCallbackConnection(int runtimeVersion);

        virtual OobMessageType  getMessageType();
        virtual void            encodeRequest(ByteBuffer & buffer);
        virtual void            decodeRequest(const ByteBuffer & buffer, std::size_t & pos);
    };

    class RCF_EXPORT OobRequestSubscription : public OobMessage
    {
    public:
        OobRequestSubscription(int runtimeVersion);

        OobRequestSubscription(
            int                     runtimeVersion, 
            const std::string &     publisherName, 
            boost::uint32_t         subToPubPingIntervalMs);

        virtual OobMessageType  getMessageType();
        virtual void            encodeRequest(ByteBuffer & buffer);
        virtual void            decodeRequest(const ByteBuffer & buffer, std::size_t & pos);
        virtual void            encodeResponse(ByteBuffer & buffer);
        virtual void            decodeResponse(const ByteBuffer & buffer);


        std::string             mPublisherName;
        boost::uint32_t         mSubToPubPingIntervalMs;
        boost::uint32_t         mPubToSubPingIntervalMs;
    };


} // namespace RCF

#endif // ! INCLUDE_RCF_METHODINVOCATION_HPP
