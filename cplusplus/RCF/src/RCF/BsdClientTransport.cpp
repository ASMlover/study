
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

#include <RCF/BsdClientTransport.hpp>

#include <RCF/AmiIoHandler.hpp>
#include <RCF/AmiThreadPool.hpp>
#include <RCF/AsioBuffers.hpp>
#include <RCF/ClientStub.hpp>
#include <RCF/Exception.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/TimedBsdSockets.hpp>

#include <RCF/AsioServerTransport.hpp>

namespace RCF {

    BsdClientTransport::BsdClientTransport() :
        mFd(-1),
        mpIoService(NULL),
        mWriteCounter(0)
    {}

    BsdClientTransport::BsdClientTransport(TcpSocketPtr socketPtr) :
        mFd(-1),
        mTcpSocketPtr(socketPtr),
        mpIoService(& socketPtr->get_io_service()),
        mWriteCounter(0)
    {
        mClosed = false;
        mAsioTimerPtr.reset( new AsioDeadlineTimer( *mpIoService ));
    }

#ifdef RCF_HAS_LOCAL_SOCKETS

    BsdClientTransport::BsdClientTransport(UnixLocalSocketPtr socketPtr) :
        mFd(-1),
        mLocalSocketPtr(socketPtr),
        mpIoService(& socketPtr->get_io_service()),
        mWriteCounter(0)
    {
        mClosed = false;
        mAsioTimerPtr.reset( new AsioDeadlineTimer( *mpIoService ));
    }

#endif

    BsdClientTransport::BsdClientTransport(const BsdClientTransport & rhs) :
        ConnectedClientTransport(rhs),
        mFd(-1),
        mpIoService(NULL),
        mWriteCounter(0)
    {}

    BsdClientTransport::~BsdClientTransport()
    {
        RCF_DTOR_BEGIN
            close();
        RCF_DTOR_END
    }

#ifdef BOOST_WINDOWS

    // return -2 for timeout, -1 for error, 0 for ready
    int pollSocketWithProgressMwfmo(
        const ClientProgressPtr &clientProgressPtr,
        ClientProgress::Activity activity,
        unsigned int endTimeMs,
        int fd,
        int &err,
        bool bRead)
    {
        RCF_UNUSED_VARIABLE(err);
        RCF_UNUSED_VARIABLE(activity);

        ClientStub & clientStub = *getTlsClientStubPtr();

        int uiMessageFilter = clientProgressPtr->mUiMessageFilter;

        HANDLE readEvent = WSACreateEvent();
        using namespace boost::multi_index::detail;
        scope_guard WSACloseEventGuard = make_guard(WSACloseEvent, readEvent);
        RCF_UNUSED_VARIABLE(WSACloseEventGuard);

        int nRet = WSAEventSelect(fd, readEvent, bRead ? FD_READ : FD_WRITE);
        RCF_ASSERT_EQ(nRet , 0);
        RCF_UNUSED_VARIABLE(nRet);
        HANDLE handles[] = { readEvent };

        while (true)
        {
            unsigned int timeoutMs = generateTimeoutMs(endTimeMs);
            timeoutMs = clientStub.generatePollingTimeout(timeoutMs);

            DWORD dwRet = MsgWaitForMultipleObjects(
                1, 
                handles, 
                0, 
                timeoutMs, 
                uiMessageFilter);

            if (dwRet == WAIT_TIMEOUT)
            {
                clientStub.onPollingTimeout();

                if (generateTimeoutMs(endTimeMs) == 0)
                {
                    return -2;
                }
            }
            else if (dwRet == WAIT_OBJECT_0)
            {
                // File descriptor is ready to be read.
                return 0;
            }
            else if (dwRet == WAIT_OBJECT_0 + 1)
            {
                clientStub.onUiMessage();                
            }
        }
    }

#endif

    PollingFunctor::PollingFunctor(
        ClientProgressPtr clientProgressPtr,
        ClientProgress::Activity activity,
        unsigned int endTimeMs) :
            mClientProgressPtr(clientProgressPtr),
            mActivity(activity),
            mEndTimeMs(endTimeMs)
    {}

#ifdef BOOST_WINDOWS

    // On Windows, the user may have requested callbacks on UI messages, in 
    // which case we'll need to use MsgWaitForMultipleObjects() rather than
    // plain old select().

    int PollingFunctor::operator()(int fd, int &err, bool bRead)
    {
        if (
            mClientProgressPtr.get() &&
            mClientProgressPtr->mTriggerMask & ClientProgress::UiMessage)
        {
            return pollSocketWithProgressMwfmo(
                mClientProgressPtr,
                mActivity,
                mEndTimeMs,
                fd,
                err,
                bRead);
        }
        else
        {
            return pollSocket(
                mEndTimeMs,
                fd,
                err,
                bRead);
        }
    }

#else

    int PollingFunctor::operator()(int fd, int &err, bool bRead)
    {
        return pollSocket(
            mEndTimeMs,
            fd,
            err,
            bRead);
    }

#endif

    std::size_t BsdClientTransport::implRead(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        mWriteCounter = 0;

        std::size_t bytesToRead = RCF_MIN(bytesRequested, byteBuffer.getLength());

        RCF_ASSERT(!mNoTimeout);

        PollingFunctor pollingFunctor(
            mClientProgressPtr,
            ClientProgress::Receive,
            mEndTimeMs);

        RCF_LOG_4()(byteBuffer.getLength())(bytesToRead) << "BsdClientTransport - initiating read from socket.";

        int fd = getNativeHandle();

        int err = 0;
        int ret = RCF::timedRecv(
            *this,
            pollingFunctor,
            err,
            fd,
            byteBuffer,
            bytesToRead,
            0);

        switch (ret)
        {
        case -2:
            {
                Exception e(_RcfError_ClientReadTimeout());
                RCF_THROW(e);
            }
            break;

        case -1:
            {
                Exception e(
                    _RcfError_ClientReadFail(),
                    err,
                    RcfSubsystem_Os);
            
                RCF_THROW(e);
            }
            break;

        case  0:
            {
                Exception e(_RcfError_PeerDisconnect());
                RCF_THROW(e);
            }
            break;

        default:
            
            RCF_ASSERT(
                0 < ret && ret <= static_cast<int>(bytesRequested))
                (ret)(bytesRequested);
        }

        return ret;
    }

    std::size_t BsdClientTransport::implReadAsync(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        mWriteCounter = 0;

        RecursiveLock lock(mOverlappedPtr->mMutex);

        mOverlappedPtr->ensureLifetime(byteBuffer);

        mOverlappedPtr->mOpType = Read;

        if (mTcpSocketPtr)
        {
            mTcpSocketPtr->async_read_some(
                ASIO_NS::buffer( byteBuffer.getPtr(), bytesRequested),
                AmiIoHandler(mOverlappedPtr));
        }
        else
        {
            RCF_ASSERT(mLocalSocketPtr);

            mLocalSocketPtr->async_read_some(
                ASIO_NS::buffer( byteBuffer.getPtr(), bytesRequested),
                AmiIoHandler(mOverlappedPtr));
        }

        if (mNoTimeout)
        {
            // Timeouts are being handled at a higher level (MulticastClientTransport).
            // ...
        }
        else
        {
            boost::uint32_t nowMs = getCurrentTimeMs();
            boost::uint32_t timeoutMs = mEndTimeMs - nowMs;
            mAsioTimerPtr->expires_from_now( boost::posix_time::milliseconds(timeoutMs) );
            mAsioTimerPtr->async_wait( AmiTimerHandler(mOverlappedPtr) );
        }

        return 0;
    }

    std::size_t BsdClientTransport::implWrite(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        ++mWriteCounter;

        if (mWriteCounter > 1)
        {
            // Put a breakpoint here to catch write buffer fragmentation.
            mWriteCounter = mWriteCounter;
        }

        RCF_ASSERT(!mNoTimeout);

        PollingFunctor pollingFunctor(
            mClientProgressPtr,
            ClientProgress::Send,
            mEndTimeMs);

        int err = 0;

        RCF_LOG_4()(lengthByteBuffers(byteBuffers)) 
            << "BsdClientTransport - initiating send on socket.";

        int fd = getNativeHandle();

        int ret = RCF::timedSend(
            pollingFunctor,
            err,
            fd,
            byteBuffers,
            getMaxSendSize(),
            0);

        switch (ret)
        {
        case -2:
            {
                Exception e(_RcfError_ClientWriteTimeout());
                RCF_THROW(e);
            }
            break;

        case -1:
            {
                Exception e(
                    _RcfError_ClientWriteFail(),
                    err,
                    RcfSubsystem_Os);
            
                RCF_THROW(e);
            }
            break;

        case 0:
            {
                Exception e(_RcfError_PeerDisconnect());            
                RCF_THROW(e);
            }
            break;

        default:
            
            RCF_ASSERT(
                0 < ret && ret <= static_cast<int>(lengthByteBuffers(byteBuffers)))
                (ret)(lengthByteBuffers(byteBuffers));            

            onTimedSendCompleted(ret, 0);
        }

        return ret;
    }

    std::size_t BsdClientTransport::implWriteAsync(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        ++mWriteCounter;

        if (mWriteCounter > 1)
        {
            // Put a breakpoint here to catch write buffer fragmentation.
            mWriteCounter = mWriteCounter;
        }

        RecursiveLock lock(mOverlappedPtr->mMutex);

        mAsioBuffers.mVecPtr->resize(0);

        for (std::size_t i=0; i<byteBuffers.size(); ++i)
        {
            const ByteBuffer & buffer = byteBuffers[i];
            mAsioBuffers.mVecPtr->push_back( AsioConstBuffer(
                buffer.getPtr(), 
                buffer.getLength()));
        }

        mOverlappedPtr->ensureLifetime(byteBuffers);

        mOverlappedPtr->mOpType = Write;

        if (mTcpSocketPtr)
        {
            ASIO_NS::async_write(
                *mTcpSocketPtr, 
                mAsioBuffers,
                AmiIoHandler(mOverlappedPtr));
        }
        else
        {
            RCF_ASSERT(mLocalSocketPtr);

            ASIO_NS::async_write(
                *mLocalSocketPtr, 
                mAsioBuffers, 
                AmiIoHandler(mOverlappedPtr));
        }

        if (mNoTimeout)
        {
            // Timeouts are being handled at a higher level (MulticastClientTransport).
            // ...
        }
        else
        {
            boost::uint32_t nowMs = getCurrentTimeMs();
            boost::uint32_t timeoutMs = mEndTimeMs - nowMs;
            mAsioTimerPtr->expires_from_now( boost::posix_time::milliseconds(timeoutMs) );
            mAsioTimerPtr->async_wait( AmiTimerHandler(mOverlappedPtr) );
        }

        return 0;
    }

    bool BsdClientTransport::isConnected()
    {
        int fd = getNativeHandle();
        return isFdConnected(fd);
    }

    TcpSocketPtr BsdClientTransport::releaseTcpSocket()
    {
        RCF_ASSERT( mFd == -1 );
        RCF_ASSERT( mTcpSocketPtr );

        TcpSocketPtr socketPtr = mTcpSocketPtr;
        mTcpSocketPtr.reset();
        return socketPtr;
    }

    UnixLocalSocketPtr BsdClientTransport::releaseLocalSocket()
    {
        RCF_ASSERT( mFd == -1 );
        RCF_ASSERT( mLocalSocketPtr );

        UnixLocalSocketPtr socketPtr = mLocalSocketPtr;
        mLocalSocketPtr.reset();
        return socketPtr;
    }

    int BsdClientTransport::getNativeHandle() const
    {
        if (mTcpSocketPtr)
        {
            return static_cast<int>(mTcpSocketPtr->native());
        }
        else if (mLocalSocketPtr)
        {
            return static_cast<int>(mLocalSocketPtr->native());
        }
        else
        {
            return mFd;
        }
    }
    
} // namespace RCF
