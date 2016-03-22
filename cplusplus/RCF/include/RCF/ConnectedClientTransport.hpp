
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

#ifndef INCLUDE_RCF_CONNECTIONORIENTEDCLIENTTRANSPORT_HPP
#define INCLUDE_RCF_CONNECTIONORIENTEDCLIENTTRANSPORT_HPP

#include <boost/enable_shared_from_this.hpp>

#include <RCF/AmiThreadPool.hpp>
#include <RCF/AsioBuffers.hpp>
#include <RCF/AsioDeadlineTimer.hpp>
#include <RCF/Filter.hpp>
#include <RCF/ByteOrdering.hpp>
#include <RCF/ClientProgress.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/Export.hpp>
#include <RCF/RecursionLimiter.hpp>

namespace RCF {

    class ConnectedClientTransport;

    class ClientFilterProxy;

    class TcpClientTransport;
    typedef boost::shared_ptr<TcpClientTransport> TcpClientTransportPtr;
    class TcpClientFilterProxy;

    class OverlappedAmi;
    typedef boost::shared_ptr<OverlappedAmi> OverlappedAmiPtr;

    class RCF_EXPORT ConnectedClientTransport : 
        public ClientTransport, 
        public WithProgressCallback
    {
    public:

        ConnectedClientTransport(const ConnectedClientTransport &rhs);
        ConnectedClientTransport();
        ~ConnectedClientTransport();

        void                    close();
        void                    setMaxSendSize(std::size_t maxSendSize);
        std::size_t             getMaxSendSize();

    private:

        void                    read(const ByteBuffer &byteBuffer_, std::size_t bytesRequested);
        void                    write(const std::vector<ByteBuffer> &byteBuffers);
        std::size_t             timedSend(const std::vector<ByteBuffer> &data);
        std::size_t             timedReceive(ByteBuffer &byteBuffer, std::size_t bytesRequested);

        void                    setTransportFilters(const std::vector<FilterPtr> &filters);
        void                    getTransportFilters(std::vector<FilterPtr> &filters);
        void                    connectTransportFilters();
        
        void                    connect(ClientTransportCallback &clientStub, unsigned int timeoutMs);
        void                    disconnect(unsigned int timeoutMs);
        int                     timedSend(const char *buffer, std::size_t bufferLen);
        int                     timedReceive(char *buffer, std::size_t bufferLen);

    protected:

        void                    onReadCompleted(const ByteBuffer &byteBuffer);
        void                    onWriteCompleted(std::size_t bytes);

        friend class HttpServerTransport;
        friend class AsioServerTransport;
        friend class PublishingService;
        void                    setWireFilters(const std::vector<FilterPtr> & wireFilters);

        bool                                    mOwn;
        bool                                    mClosed;
        std::size_t                             mMaxSendSize;
        std::size_t                             mBytesTransferred;
        std::size_t                             mBytesSent;
        std::size_t                             mBytesRead;
        std::size_t                             mBytesTotal;
        int                                     mError;
        bool                                    mNoTimeout;
        unsigned int                            mEndTimeMs;

        std::vector<FilterPtr>                  mTransportFilters;
        std::vector<FilterPtr>                  mWireFilters;
        std::vector<ByteBuffer>                 mByteBuffers;
        std::vector<ByteBuffer>                 mSlicedByteBuffers;
        ReallocBufferPtr                        mReadBufferPtr;
        ReallocBufferPtr                        mReadBuffer2Ptr;

        friend class ClientFilterProxy;
        friend class ClientTcpFrame;
        friend class ClientHttpFrame;

    private:

        virtual std::size_t implRead(
            const ByteBuffer &byteBuffer_,
            std::size_t bytesRequested) = 0;

        virtual std::size_t implReadAsync(
            const ByteBuffer &byteBuffer_,
            std::size_t bytesRequested) = 0;

        virtual std::size_t implWrite(
            const std::vector<ByteBuffer> &byteBuffers) = 0;

        virtual std::size_t implWriteAsync(
            const std::vector<ByteBuffer> &byteBuffers) = 0;

        virtual void implConnect(
            ClientTransportCallback &clientStub, 
            unsigned int timeoutMs) = 0;

        virtual void implConnectAsync(
            ClientTransportCallback &clientStub, 
            unsigned int timeoutMs) = 0;

        virtual void implClose() = 0;

    public:

        enum State {
            Connecting, 
            Reading, 
            Writing
        };

        State                       mPreState;
        State                       mPostState;
        std::size_t                 mReadBufferPos;
        std::size_t                 mWriteBufferPos;
        
        ClientTransportCallback *   mpClientStub;
        
        ByteBuffer *                mpClientStubReadBuffer;
        ByteBuffer                  mReadBuffer;
        std::size_t                 mBytesToRead;
        std::size_t                 mBytesRequested;
        ByteBuffer                  mByteBuffer;        

    protected:
        friend class Subscription;
        OverlappedAmiPtr            mOverlappedPtr;

        
        MutexPtr                    mSocketOpsMutexPtr;

        AsioBuffers                 mAsioBuffers;

    public:

        typedef boost::shared_ptr<Lock>     LockPtr;

        AsioDeadlineTimerPtr                mAsioTimerPtr;

        friend class TcpClientFilterProxy;

    public:

        void cancel();

        void        setTimer(
                        boost::uint32_t timeoutMs,
                        ClientTransportCallback *pClientStub);

        void        onTimerExpired();

    private:
        RecursionState<std::size_t, int> mRecursionState;

        // TODO: Access control.
    public:

        void        onTransitionCompleted(std::size_t bytesTransferred);

        void        onCompletion(int bytesTransferred);
        void        onTimedRecvCompleted(int ret, int err);
        void        onTimedSendCompleted(int ret, int err);
        void        onConnectCompleted(int err);

        void        transition();
        void        onTransitionCompleted_(std::size_t bytesTransferred);
        void        issueRead(const ByteBuffer &buffer, std::size_t bytesToRead);
        void        issueWrite(const std::vector<ByteBuffer> &byteBuffers);

        int         send(
                        ClientTransportCallback &clientStub, 
                        const std::vector<ByteBuffer> &data, 
                        unsigned int timeoutMs);

        int         receive(
                        ClientTransportCallback &clientStub, 
                        ByteBuffer &byteBuffer, 
                        unsigned int timeoutMs);

        void        setSocketOpsMutex(MutexPtr mutexPtr);

        friend class OverlappedAmi;

    };

} // namespace RCF

#endif // ! INCLUDE_RCF_CONNECTIONORIENTEDCLIENTTRANSPORT_HPP
