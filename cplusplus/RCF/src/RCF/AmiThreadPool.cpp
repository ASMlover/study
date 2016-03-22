
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

#include <RCF/AmiThreadPool.hpp>

#include <RCF/AmiIoHandler.hpp>
#include <RCF/AsioHandlerCache.hpp>
#include <RCF/ClientStub.hpp>
#include <RCF/ConnectedClientTransport.hpp>
#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/IpAddress.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/ThreadPool.hpp>

namespace RCF {

    void AmiNotification::set(
        Cb cb, 
        LockPtr lockPtr, 
        MutexPtr mutexPtr)
    {
        RCF_ASSERT(lockPtr.get() && lockPtr->locked() && mutexPtr.get());
        mCb = cb;
        mLockPtr = lockPtr;
        mMutexPtr = mutexPtr;
    }

    void AmiNotification::run()
    {
        if (mLockPtr)
        {
            Lock & lock = *mLockPtr;
            RCF_ASSERT(lock.locked());
            lock.unlock();
            Cb cb = mCb;
            clear();
            if (cb)
            {
                cb();
            }
        }
    }

    void AmiNotification::clear()
    {
        mLockPtr.reset();
        mMutexPtr.reset();
        mCb = Cb();
    }

    AsioHandlerCache * gpAmiIoHandlerCache = NULL;
    AsioHandlerCache * gpAmiTimerHandlerCache = NULL;

    void initAmiHandlerCache()
    {
        gpAmiIoHandlerCache = new AsioHandlerCache(); 
        gpAmiTimerHandlerCache = new AsioHandlerCache();
    }

    void deinitAmiHandlerCache()
    {
        delete gpAmiIoHandlerCache; 
        gpAmiIoHandlerCache = NULL; 

        delete gpAmiTimerHandlerCache; 
        gpAmiTimerHandlerCache = NULL;
    }

    void * asio_handler_allocate(std::size_t size, AmiIoHandler * pHandler)
    {
        RCF_UNUSED_VARIABLE(pHandler);
        return gpAmiIoHandlerCache->allocate(size);
    }

    void asio_handler_deallocate(void * pointer, std::size_t size, AmiIoHandler * pHandler)
    {
        RCF_UNUSED_VARIABLE(pHandler);
        return gpAmiIoHandlerCache->deallocate(pointer, size);
    }

    void * asio_handler_allocate(std::size_t size, AmiTimerHandler * pHandler)
    {
        RCF_UNUSED_VARIABLE(pHandler);
        return gpAmiTimerHandlerCache->allocate(size);
    }

    void asio_handler_deallocate(void * pointer, std::size_t size, AmiTimerHandler * pHandler)
    {
        RCF_UNUSED_VARIABLE(pHandler);
        return gpAmiTimerHandlerCache->deallocate(pointer, size);
    }

    AmiIoHandler::AmiIoHandler(OverlappedAmiPtr overlappedPtr) : 
        mOverlappedPtr(overlappedPtr), 
        mIndex(overlappedPtr->mIndex)
    {
    }

    AmiIoHandler::AmiIoHandler(
        OverlappedAmiPtr overlappedPtr, 
        const AsioErrorCode & ec) : 
            mOverlappedPtr(overlappedPtr), 
            mIndex(overlappedPtr->mIndex),
            mEc(ec)
    {
    }

    void AmiIoHandler::operator()()
    {
        mOverlappedPtr->onCompletion(mIndex, mEc, 0);
    }

    void AmiIoHandler::operator()(
        const AsioErrorCode & ec)
    {
        mOverlappedPtr->onCompletion(mIndex, ec, 0);
    }

    void AmiIoHandler::operator()(
        const AsioErrorCode & ec, 
        std::size_t bytesTransferred)
    {
        mOverlappedPtr->onCompletion(mIndex, ec, bytesTransferred);
    }

    AmiTimerHandler::AmiTimerHandler(OverlappedAmiPtr overlappedPtr) : 
        mOverlappedPtr(overlappedPtr), 
        mIndex(overlappedPtr->mIndex)
    {
    }

    void AmiTimerHandler::operator()(
        const AsioErrorCode & ec)
    {
        mOverlappedPtr->onTimerExpired(mIndex, ec);
    }

    static bool gEnableMultithreadedBlockingConnects = false;

    void AmiThreadPool::setEnableMultithreadedBlockingConnects(bool enable)
    {
        gEnableMultithreadedBlockingConnects = enable;
    }

    bool AmiThreadPool::getEnableMultithreadedBlockingConnects()
    {
        return gEnableMultithreadedBlockingConnects;
    }

#ifdef BOOST_WINDOWS

    class Win32ThreadImpersonator
    {
    public:
        Win32ThreadImpersonator(HANDLE hToken) : mClosed(false)
        {
            BOOL ok = SetThreadToken(NULL, hToken);
            if (!ok)
            {
                DWORD dwErr = GetLastError();
                RCF_THROW( Exception( 
                    _RcfError_Win32ApiError("SetThreadToken()"), 
                    dwErr) ); 
            }
        }

        void close()
        {
            if ( !mClosed )
            {
                mClosed = true;
                BOOL ok = SetThreadToken(NULL, NULL);
                if ( !ok )
                {
                    DWORD dwErr = GetLastError();
                    RCF_THROW(Exception(
                        _RcfError_Win32ApiError("SetThreadToken()"),
                        dwErr));
                }
            }
        }

        ~Win32ThreadImpersonator()
        {
            RCF_DTOR_BEGIN
                close();
            RCF_DTOR_END
        }

    private:

        bool mClosed;
    };

#endif

    void OverlappedAmi::onCompletion(
        std::size_t index,
        const AsioErrorCode & ec, 
        std::size_t bytesTransferred)
    {

        RecursiveLock lock(mMutex);

        //mAsioBuffers.mVecPtr->resize(0);
        //mByteBuffers.resize(0);

        if (mIndex == index && mpTransport)
        {
            ++mIndex;

            if (mpTransport->mAsioTimerPtr)
            {
                mpTransport->mAsioTimerPtr->cancel();
            }

#ifdef BOOST_WINDOWS
            // Do we need to impersonate?
            boost::scoped_ptr<Win32ThreadImpersonator> impersonator;
            if (mpTransport->mpClientStub->isClientStub())
            {
                ClientStub * pStub = static_cast<ClientStub *>(mpTransport->mpClientStub);
                HANDLE hImpersonationToken = pStub->getWindowsImpersonationToken();
                if (hImpersonationToken != INVALID_HANDLE_VALUE)
                {
                    impersonator.reset( new Win32ThreadImpersonator(hImpersonationToken) );
                }
            }
#endif

            if (ec)
            {
                try
                {
                    RCF::Exception e;

                    switch (mOpType)
                    {
                    case Wait:
                        RCF_ASSERT(0);
                        break;

                    case Connect:
                        e = RCF::Exception( _RcfError_ClientConnectFail(), ec.value());
                        break;

                    case Write:
                        e = RCF::Exception( _RcfError_ClientWriteFail(), ec.value());
                        break;

                    case Read:
                        e = RCF::Exception( _RcfError_ClientReadFail(), ec.value());
                        break;

                    default:
                        RCF_ASSERT(0);
                    };

                    mOpType = None;

                    mpTransport->mpClientStub->onError(e);
                }
                catch(const std::exception &e)
                {
                    mpTransport->mpClientStub->onError(e);
                }
                catch(...)
                {
                    RCF::Exception e( _RcfError_NonStdException() );
                    mpTransport->mpClientStub->onError(e);
                }
            }
            else
            {
                mOpType = None;

                try
                {
                    mpTransport->onCompletion( static_cast<int>(bytesTransferred) );
                }
                catch(const std::exception &e)
                {
                    mpTransport->mpClientStub->onError(e);
                }
                catch(...)
                {
                    RCF::Exception e( _RcfError_NonStdException() );
                    mpTransport->mpClientStub->onError(e);
                }
            }

            getTlsAmiNotification().run();
        }

    }

    void OverlappedAmi::onTimerExpired(
        std::size_t index,
        const AsioErrorCode & ec)
    {
        RecursiveLock lock(mMutex);

        //mAsioBuffers.mVecPtr->resize(0);
        //mByteBuffers.resize(0);

        if (mpTransport && mIndex == index)
        {
            if (!ec)
            {
                ++mIndex;
                mpTransport->onTimerExpired();
                getTlsAmiNotification().run();
            }
        }
    }

    void OverlappedAmi::ensureLifetime(const ByteBuffer & byteBuffer)
    {
        mByteBuffers.resize(0);
        mByteBuffers.push_back(byteBuffer);
    }

    void OverlappedAmi::ensureLifetime(const std::vector<ByteBuffer> & byteBuffers)
    {
        mByteBuffers.resize(0);
        mByteBuffers = byteBuffers;
    }

    AmiThreadPool::AmiThreadPool() : mThreadPool(1)
    {
        mThreadPool.setThreadName("RCF AMI");
        mThreadPool.enableMuxerType(Mt_Asio);
    }

    AmiThreadPool::~AmiThreadPool()
    {
    }

    void AmiThreadPool::start()
    {
        RCF_ASSERT(!mThreadPool.isStarted());
        mThreadPool.start();
    }

    void AmiThreadPool::stop()
    {
        RCF_ASSERT(mThreadPool.isStarted());
        mThreadPool.stop();
    }

    AsioIoService & AmiThreadPool::getIoService() 
    { 
        return * mThreadPool.getIoService(); 
    }

    AmiThreadPool * gpAmiThreadPool;

    AmiThreadPool & getAmiThreadPool()
    {
        return *gpAmiThreadPool;
    }
    /*
    AsyncTimer::AsyncTimer() : mDummyPtr( new RcfClient<I_Null>( RCF::TcpEndpoint(0)))
    {
    }

    AsyncTimer::~AsyncTimer()
    {
    }

    void AsyncTimer::set(boost::function0<void> onTimer, boost::uint32_t timeoutMs)
    {
        mDummyPtr->getClientStub().wait(onTimer, timeoutMs);
    }

    void AsyncTimer::cancel()
    {
        mDummyPtr.reset( new RcfClient<I_Null>( RCF::TcpEndpoint(0)) );
    }
    */

} // namespace RCF
