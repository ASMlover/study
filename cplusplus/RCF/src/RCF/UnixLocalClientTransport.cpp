
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

#include <RCF/UnixLocalClientTransport.hpp>

#include <RCF/Exception.hpp>
#include <RCF/TimedBsdSockets.hpp>
#include <RCF/Tools.hpp>
#include <RCF/UnixLocalEndpoint.hpp>

namespace RCF {

    UnixLocalClientTransport::UnixLocalClientTransport(
        const UnixLocalClientTransport &rhs) : 
            BsdClientTransport(rhs),
            mRemoteAddr(rhs.mRemoteAddr),
            mFileName(rhs.mFileName)
    {}

    UnixLocalClientTransport::UnixLocalClientTransport(
        const std::string &fileName) :
            BsdClientTransport(),
            mRemoteAddr(),
            mFileName(fileName)
    {
        memset(&mRemoteAddr, 0, sizeof(mRemoteAddr));
    }

    UnixLocalClientTransport::UnixLocalClientTransport(
        const sockaddr_un &remoteAddr) :
            BsdClientTransport(),
            mRemoteAddr(remoteAddr),
            mFileName()
    {}

    UnixLocalClientTransport::UnixLocalClientTransport(
        UnixLocalSocketPtr socketPtr, 
        const std::string & fileName) :
            BsdClientTransport(socketPtr),
            mRemoteAddr(),
            mFileName(fileName)
    {
        memset(&mRemoteAddr, 0, sizeof(mRemoteAddr));
    }

    std::string UnixLocalClientTransport::getPipeName() const
    {
        return mFileName;
    }

    UnixLocalClientTransport::~UnixLocalClientTransport()
    {
        RCF_DTOR_BEGIN
            close();
        RCF_DTOR_END
    }

    TransportType UnixLocalClientTransport::getTransportType()
    {
        return Tt_UnixNamedPipe;
    }

    ClientTransportAutoPtr UnixLocalClientTransport::clone() const
    {
        return ClientTransportAutoPtr( new UnixLocalClientTransport(*this) );
    }

    void UnixLocalClientTransport::implConnect(unsigned int timeoutMs)
    {
        // close the current connection
        implClose();

        RCF_ASSERT(!mAsync);

        setupSocket();

        unsigned int startTimeMs = getCurrentTimeMs();
        mEndTimeMs = startTimeMs + timeoutMs;

        PollingFunctor pollingFunctor(
            mClientProgressPtr,
            ClientProgress::Connect,
            mEndTimeMs);

        sockaddr_un remote;
        memset(&remote, 0, sizeof(remote));
        remote.sun_family = AF_UNIX;

        std::size_t pipeNameLimit = sizeof(remote.sun_path);
        
        RCF_VERIFY(
            mFileName.length() < pipeNameLimit, 
            Exception(_RcfError_PipeNameTooLong(mFileName, pipeNameLimit)))(pipeNameLimit);

        strcpy(remote.sun_path, mFileName.c_str());
//#ifdef SUN_LEN
//        int remoteLen = SUN_LEN(&remote);
//#else
        int remoteLen = 
            sizeof(remote) 
            - sizeof(remote.sun_path) 
            + strlen(remote.sun_path);
//#endif

        int err = 0;

        int ret = timedConnect(
            pollingFunctor,
            err,
            mFd,
            (sockaddr*) &remote,
            remoteLen);

        if (ret != 0)
        {
            implClose();

            if (err == 0)
            {
                Exception e( _RcfError_ClientConnectTimeout(
                    timeoutMs, 
                    mFileName));

                RCF_THROW(e);
            }
            else
            {
                Exception e( _RcfError_ClientConnectFail(), err, RcfSubsystem_Os);
                RCF_THROW(e)(mFileName);
            }
        }
    }

    void UnixLocalClientTransport::implConnect(
        ClientTransportCallback &clientStub, 
        unsigned int timeoutMs)
    {
        implConnect(timeoutMs);
        clientStub.onConnectCompleted();
    }

    void UnixLocalClientTransport::implConnectAsync(
        ClientTransportCallback &clientStub, 
        unsigned int timeoutMs)
    {
        // TODO: sort this out
        RCF_UNUSED_VARIABLE(timeoutMs);

        RCF_ASSERT(mAsync);

        implClose();
        
        mpClientStub = &clientStub;

        setupSocket();

        ASIO_NS::local::stream_protocol::endpoint endpoint( getPipeName() );

        RecursiveLock lock(mOverlappedPtr->mMutex);

        RCF_ASSERT(mLocalSocketPtr);

        mOverlappedPtr->mOpType = Connect;

        mLocalSocketPtr->async_connect( 
            endpoint, 
            AmiIoHandler(mOverlappedPtr));
    }

    void UnixLocalClientTransport::setupSocket()
    {
        RCF::Exception e;
        setupSocket(e);
        if (e.bad())
        {
            RCF_THROW(e);
        }
    }

    void UnixLocalClientTransport::setupSocket(Exception & e)
    {
        e = Exception();

        RCF_ASSERT_EQ(mFd , INVALID_SOCKET);

        mFd = static_cast<int>( ::socket(AF_UNIX, SOCK_STREAM, 0) );
        int err = Platform::OS::BsdSockets::GetLastError();

        RCF_VERIFY(
            mFd != -1,
            Exception(
            _RcfError_Socket("socket()"), err, RcfSubsystem_Os));

        Platform::OS::BsdSockets::setblocking(mFd, false);

        if (mpIoService)
        {
            mAsioTimerPtr.reset( new AsioDeadlineTimer(*mpIoService) );
            RCF_LOG_1()(mLocalSocketPtr.get()) << "Clearing mLocalSocketPtr";
            mLocalSocketPtr.reset( new UnixLocalSocket(*mpIoService) );
            mLocalSocketPtr->assign(ASIO_NS::local::stream_protocol(), mFd);
            mFd = -1;
        }
    }

    void UnixLocalClientTransport::associateWithIoService(AsioIoService & ioService)
    {
        if (mLocalSocketPtr)
        {
            RCF_ASSERT(mpIoService == & ioService);
        }
        else
        {
            mpIoService = &ioService;

            mLocalSocketPtr.reset( new UnixLocalSocket(ioService) );
            if (mFd != -1)
            {
                mLocalSocketPtr->assign(ASIO_NS::local::stream_protocol(), mFd);
            }
            mAsioTimerPtr.reset(new AsioDeadlineTimer(*mpIoService));
            mFd = -1;
        }
    }

    bool UnixLocalClientTransport::isAssociatedWithIoService()
    {
        return mpIoService ? true : false;
    }
    
    void UnixLocalClientTransport::implClose()
    {
        if (mLocalSocketPtr)
        {
            if (mSocketOpsMutexPtr)
            {
                Lock lock(*mSocketOpsMutexPtr);
                mLocalSocketPtr->close();
            }
            else
            {
                mLocalSocketPtr->close();
            }

            mLocalSocketPtr.reset();
        }
        else if (mFd != -1)
        {
            int ret = Platform::OS::BsdSockets::closesocket(mFd);
            int err = Platform::OS::BsdSockets::GetLastError();

            RCF_VERIFY(
                ret == 0,
                Exception(
                    _RcfError_Socket("closesocket()"),
                    err,
                    RcfSubsystem_Os))
                (mFd);
        }

        mFd = -1;
    }

    EndpointPtr UnixLocalClientTransport::getEndpointPtr() const
    {
        return EndpointPtr( new UnixLocalEndpoint(mFileName) );
    }

    void UnixLocalClientTransport::setRemoteAddr(const sockaddr_un &remoteAddr)
    {
        mRemoteAddr = remoteAddr;
    }

    const sockaddr_un &UnixLocalClientTransport::getRemoteAddr() const
    {
        return mRemoteAddr;
    }

} // namespace RCF
