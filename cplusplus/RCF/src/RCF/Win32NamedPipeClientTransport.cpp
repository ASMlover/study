
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

#include <RCF/Win32NamedPipeClientTransport.hpp>

#include <RCF/AmiIoHandler.hpp>
#include <RCF/AmiThreadPool.hpp>
#include <RCF/ClientStub.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/Win32NamedPipeEndpoint.hpp>

#include <RCF/RcfServer.hpp>

#include <RCF/Asio.hpp>

namespace RCF {

    Win32NamedPipeClientTransport::Win32NamedPipeClientTransport(
        const Win32NamedPipeClientTransport & rhs) :
            ConnectedClientTransport(rhs),
            mPipeName(rhs.mPipeName),
            mEpPipeName(rhs.mEpPipeName),
            mhPipe(INVALID_HANDLE_VALUE),
            mhEvent(INVALID_HANDLE_VALUE),
            mpSec(),
            mAsyncMode(false),
            mpIoService(false)
    {
        HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        DWORD dwErr = GetLastError();
        RCF_VERIFY( hEvent, Exception(_RcfError_Pipe(), dwErr));

        mhEvent = hEvent;
        mpSec = rhs.mpSec;
    }

    Win32NamedPipeClientTransport::Win32NamedPipeClientTransport(
        const tstring & pipeName) :
            mEpPipeName(pipeName),
            mhPipe(INVALID_HANDLE_VALUE),
            mhEvent(INVALID_HANDLE_VALUE),
            mpSec(),
            mAsyncMode(false),
            mpIoService(false)
    {
        if (pipeName.at(0) == RCF_T('\\'))
        {
            mPipeName = pipeName;
        }
        else
        {
            mPipeName = RCF_T("\\\\.\\pipe\\") + pipeName;
        }
        
        HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        DWORD dwErr = GetLastError();
        RCF_VERIFY( hEvent, Exception(_RcfError_Pipe(), dwErr));

        mhEvent = hEvent;
    }

    Win32NamedPipeClientTransport::Win32NamedPipeClientTransport(
        AsioPipeHandlePtr socketPtr, 
        const tstring & pipeName) :
            mEpPipeName(pipeName),
            mSocketPtr(socketPtr),
            mhPipe(socketPtr->native()),
            mhEvent(),
            mpSec(),
            mAsyncMode(false),
            mpIoService(& socketPtr->get_io_service())
    {
        mClosed = false;

        if (pipeName.size() > 0)
        {
            if (pipeName.at(0) == RCF_T('\\'))
            {
                mPipeName = pipeName;
            }
            else
            {
                mPipeName = RCF_T("\\\\.\\pipe\\") + pipeName;
            }
        }

        HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        DWORD dwErr = GetLastError();
        RCF_VERIFY( hEvent, Exception(_RcfError_Pipe(), dwErr));

        mhEvent = hEvent;

        mAsioTimerPtr.reset( new AsioDeadlineTimer(*mpIoService) );
    }

    Win32NamedPipeClientTransport::~Win32NamedPipeClientTransport()
    {
        RCF_DTOR_BEGIN
            close();

            if (mhEvent != INVALID_HANDLE_VALUE)
            {
                BOOL ok = CloseHandle(mhEvent);
                DWORD dwErr = GetLastError();
                RCF_VERIFY(ok, Exception(_RcfError_Pipe(), dwErr));
            }
        RCF_DTOR_END
    }

    TransportType Win32NamedPipeClientTransport::getTransportType()
    {
        return Tt_Win32NamedPipe;
    }

    ClientTransportAutoPtr Win32NamedPipeClientTransport::clone() const
    {
        return ClientTransportAutoPtr(new Win32NamedPipeClientTransport(*this));
    }

    HANDLE Win32NamedPipeClientTransport::getNativeHandle() const
    {
        return mhPipe;
    }

    AsioPipeHandlePtr Win32NamedPipeClientTransport::releaseSocket()
    {
        RCF_ASSERT( mSocketPtr );

        AsioPipeHandlePtr socketPtr = mSocketPtr;
        mSocketPtr.reset();
        mhPipe = INVALID_HANDLE_VALUE;
        return socketPtr;
    }

    EndpointPtr Win32NamedPipeClientTransport::getEndpointPtr() const
    {
        return EndpointPtr( new Win32NamedPipeEndpoint(mEpPipeName));
    }

    tstring Win32NamedPipeClientTransport::getPipeName() const
    {
        return mPipeName;
    }

    void Win32NamedPipeClientTransport::setPipeName(const tstring & pipeName)
    {
        mPipeName = pipeName;
    }

    void Win32NamedPipeClientTransport::setSecurityAttributes(
        LPSECURITY_ATTRIBUTES pSec)
    {
        mpSec = pSec;
    }

    void Win32NamedPipeClientTransport::implClose()
    {
        if (mSocketPtr)
        {
            if (mSocketOpsMutexPtr)
            {
                Lock lock(*mSocketOpsMutexPtr);
                mSocketPtr->close();
            }
            else
            {
                mSocketPtr->close();
            }

            mSocketPtr.reset();
        }
        else if (mhPipe != INVALID_HANDLE_VALUE)
        {
            BOOL ok = CloseHandle(mhPipe);
            DWORD dwErr = GetLastError();

            RCF_VERIFY(
                ok,
                Exception(
                    _RcfError_Pipe(),
                    dwErr,
                    RcfSubsystem_Os,
                    "CloseHandle() failed"))
                (mhPipe);
        }

        mhPipe = INVALID_HANDLE_VALUE;
    }

    void Win32NamedPipeClientTransport::implConnect(
        ClientTransportCallback &clientStub,
        unsigned int timeoutMs)
    {
        unsigned int endTimeMs = getCurrentTimeMs() + timeoutMs;
        HANDLE hPipe = INVALID_HANDLE_VALUE;
        while (hPipe == INVALID_HANDLE_VALUE && generateTimeoutMs(endTimeMs)) 
        { 
            hPipe = CreateFile( 
                mPipeName.c_str(),      // pipe name 
                GENERIC_READ |          // read and write access 
                GENERIC_WRITE, 
                0,                      // no sharing 
                mpSec,                  // default security attributes
                OPEN_EXISTING,          // opens existing pipe 
                FILE_FLAG_OVERLAPPED,   // non-blocking
                NULL);                  // no template file 

            DWORD dwErr = GetLastError();

            if (hPipe == INVALID_HANDLE_VALUE && dwErr == ERROR_PIPE_BUSY) 
            {
                DWORD dwTimeoutMs = 100;
                BOOL ok = WaitNamedPipe(mPipeName.c_str(), dwTimeoutMs);
                dwErr = GetLastError();
                RCF_UNUSED_VARIABLE(ok);
                RCF_UNUSED_VARIABLE(dwErr);
            }
            else if (hPipe == INVALID_HANDLE_VALUE)
            {
                Exception e(
                    _RcfError_ClientConnectFail(),
                    dwErr,
                    RcfSubsystem_Os,
                    "CreateFile() failed");

                RCF_THROW(e);
            }                       
        }
        if (hPipe == INVALID_HANDLE_VALUE)
        {
            Exception e(_RcfError_ClientConnectTimeout(
                timeoutMs, 
                Win32NamedPipeEndpoint(mPipeName).asString()));

            RCF_THROW(e);
        }
        else
        {
            mhPipe = hPipe;

            if (mpIoService)
            {
                mSocketPtr.reset( new AsioPipeHandle(*mpIoService, mhPipe) );
                mAsioTimerPtr.reset(new AsioDeadlineTimer(*mpIoService));
            }
        }

        clientStub.onConnectCompleted();
    }

    void Win32NamedPipeClientTransport::implConnectAsync(
        ClientTransportCallback &clientStub,
        unsigned int timeoutMs)
    {
        RCF_UNUSED_VARIABLE(timeoutMs);

        implClose();

        mpClientStub = &clientStub;

        HANDLE hPipe = INVALID_HANDLE_VALUE;

        hPipe = CreateFile( 
            mPipeName.c_str(),      // pipe name 
            GENERIC_READ |          // read and write access 
            GENERIC_WRITE, 
            0,                      // no sharing 
            mpSec,                  // default security attributes
            OPEN_EXISTING,          // opens existing pipe 
            FILE_FLAG_OVERLAPPED,   // non-blocking
            NULL);                  // no template file 

        DWORD dwErr = GetLastError();

        RecursiveLock lock(mOverlappedPtr->mMutex);

        mOverlappedPtr->mOpType = Connect;

        if (hPipe != INVALID_HANDLE_VALUE)
        {
            mhPipe = hPipe;

            if (mpIoService)
            {
                mSocketPtr.reset( new AsioPipeHandle(*mpIoService, mhPipe) );
                mAsioTimerPtr.reset( new AsioDeadlineTimer(*mpIoService) );
            }

            AsioErrorCode ec;

            //AmiIoHandler(mOverlappedPtr, ec)();

            mpIoService->post( AmiIoHandler(mOverlappedPtr, ec) );
        }
        else
        {
            AsioErrorCode ec(
                dwErr,
                ASIO_NS::error::get_system_category());

            mpIoService->post( AmiIoHandler(mOverlappedPtr, ec) );
        }
    }

    bool Win32NamedPipeClientTransport::isConnected()
    {
        bool connected = 
            mhPipe != INVALID_HANDLE_VALUE &&
            PeekNamedPipe(mhPipe, NULL,0, NULL, NULL, NULL) == TRUE;

        if (mhPipe != INVALID_HANDLE_VALUE && !connected)
        {
            // Will probably be ERROR_PIPE_NOT_CONNECTED.
            DWORD dwErr = GetLastError();
            RCF_UNUSED_VARIABLE(dwErr);
        }

        return connected;
    }

    std::size_t Win32NamedPipeClientTransport::implRead(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        // For now, can't go back to sync calls after doing an async call.
        // Limitations with Windows IOCP.
        RCF_ASSERT(!mAsyncMode);

        std::size_t bytesToRead = RCF_MIN(bytesRequested, byteBuffer.getLength());

        BOOL ok = ResetEvent(mhEvent);
        DWORD dwErr = GetLastError();
        RCF_VERIFY(ok, Exception(_RcfError_Pipe(), dwErr));

        OVERLAPPED overlapped = {0};
        overlapped.hEvent = mhEvent;

        DWORD dwRead = 0;
        DWORD dwBytesToRead = static_cast<DWORD>(bytesToRead);

        ok = ReadFile(
            mhPipe, 
            byteBuffer.getPtr(), 
            dwBytesToRead, 
            &dwRead, 
            &overlapped);
        
        dwErr = GetLastError();

        if (!ok)
        {
            RCF_VERIFY( 
                dwErr == ERROR_IO_PENDING ||
                dwErr == ERROR_MORE_DATA,
                Exception(_RcfError_ClientReadFail(), dwErr));
        }

        ClientStub & clientStub = *getTlsClientStubPtr();

        DWORD dwRet = WAIT_TIMEOUT;
        while (dwRet == WAIT_TIMEOUT)
        {
            boost::uint32_t timeoutMs = generateTimeoutMs(mEndTimeMs);
            timeoutMs = clientStub.generatePollingTimeout(timeoutMs);

            dwRet = WaitForSingleObject(overlapped.hEvent, timeoutMs);
            dwErr = GetLastError();

            RCF_VERIFY( 
                dwRet == WAIT_OBJECT_0 || dwRet == WAIT_TIMEOUT, 
                Exception(_RcfError_Pipe(), dwErr));

            RCF_VERIFY(
                generateTimeoutMs(mEndTimeMs),
                Exception(_RcfError_ClientReadTimeout()))
                (mEndTimeMs)(bytesToRead);

            if (dwRet == WAIT_TIMEOUT)
            {
                clientStub.onPollingTimeout();
            }
        }
        RCF_ASSERT_EQ(dwRet , WAIT_OBJECT_0);

        dwRead = 0;
        ok = GetOverlappedResult(mhPipe, &overlapped, &dwRead, FALSE);
        dwErr = GetLastError();
        RCF_VERIFY(ok && dwRead > 0, Exception(_RcfError_Pipe(), dwErr));

        onTimedRecvCompleted(dwRead, 0);

        return dwRead;
    }

    void Win32NamedPipeClientTransport::associateWithIoService(AsioIoService & ioService)
    {
        if (mSocketPtr)
        {
            RCF_ASSERT(mpIoService == & ioService);
        }
        else
        {
            if (mhPipe != INVALID_HANDLE_VALUE)
            {
                mSocketPtr.reset( new AsioPipeHandle(ioService, mhPipe) );
            }
            else
            {
                mSocketPtr.reset( new AsioPipeHandle(ioService) );
            }
            
            mAsioTimerPtr.reset( new AsioDeadlineTimer(ioService) );
            mpIoService = &ioService;
        }
    }

    bool Win32NamedPipeClientTransport::isAssociatedWithIoService()
    {
        return mpIoService ? true : false;
    }

    std::size_t Win32NamedPipeClientTransport::implReadAsync(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        mAsyncMode = true;

        RecursiveLock lock(mOverlappedPtr->mMutex);

        mOverlappedPtr->ensureLifetime(byteBuffer);

        mOverlappedPtr->mOpType = Read;

        // Construct an OVERLAPPED-derived object to contain the handler.
        ASIO_NS::windows::overlapped_ptr overlapped(
            mSocketPtr->get_io_service(), 
            AmiIoHandler(mOverlappedPtr));

        DWORD dwBytesRead = 0;

        bool readOk = false;

        BOOL ok = ReadFile(
            mhPipe,
            byteBuffer.getPtr(),
            static_cast<DWORD>(bytesRequested),
            &dwBytesRead,
            overlapped.get());

        DWORD dwErr = GetLastError();;

        if (!ok &&  (
                dwErr == ERROR_IO_PENDING 
            ||  dwErr == ERROR_MORE_DATA))
        {
            readOk = true;
        }
        else if (dwBytesRead)
        {
            readOk = true;
        }

        if (readOk)
        {
            // The operation was successfully initiated, so ownership of the
            // OVERLAPPED-derived object has passed to the io_service.
            overlapped.release();

            // Set timer.
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
        }
        else
        {
            // The operation completed immediately, so a completion notification needs
            // to be posted. When complete() is called, ownership of the OVERLAPPED-
            // derived object passes to the io_service.

            AsioErrorCode ec(
                dwErr,
                ASIO_NS::error::get_system_category());

            overlapped.complete(ec, 0);
        }

        return 0;
    }

    std::size_t Win32NamedPipeClientTransport::implWrite(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        // For now, can't go back to sync calls after doing an async call.
        // Limitations with Windows IOCP.
        RCF_ASSERT(!mAsyncMode);

        // Not using overlapped I/O here because it interferes with the
        // server session that might be coupled to this transport.

        const ByteBuffer & byteBuffer = byteBuffers.front();

        DWORD count = 0;
        DWORD dwBytesToWrite = static_cast<DWORD>(byteBuffer.getLength());

        BOOL ok = WriteFile( 
            mhPipe,
            byteBuffer.getPtr(),
            dwBytesToWrite,
            &count,
            NULL);

        DWORD dwErr = GetLastError();

        RCF_VERIFY(ok, Exception(_RcfError_ClientWriteFail(), dwErr));

        // Strangely, WriteFile() sometimes returns 1, but at the same time a much too big value in count.
        RCF_VERIFY(count <= dwBytesToWrite, Exception(_RcfError_ClientWriteFail(), dwErr))(count)(dwBytesToWrite);

        RCF_VERIFY(count > 0, Exception(_RcfError_ClientWriteFail(), dwErr))(count)(dwBytesToWrite);

        onTimedSendCompleted( RCF_MIN(count, dwBytesToWrite), 0);

        return count;
    }

    std::size_t Win32NamedPipeClientTransport::implWriteAsync(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        mAsyncMode = true;

        RecursiveLock lock(mOverlappedPtr->mMutex);

        const ByteBuffer & byteBuffer = byteBuffers.front();

        mOverlappedPtr->ensureLifetime(byteBuffer);

        mOverlappedPtr->mOpType = Write;

        // Construct an OVERLAPPED-derived object to contain the handler.
        ASIO_NS::windows::overlapped_ptr overlapped(
            mSocketPtr->get_io_service(), 
            AmiIoHandler(mOverlappedPtr));

        DWORD dwBytesWritten = 0;

        bool writeOk = false;

        BOOL ok = WriteFile(
            mhPipe,
            byteBuffer.getPtr(),
            static_cast<DWORD>(byteBuffer.getLength()),
            &dwBytesWritten,
            overlapped.get());

        DWORD dwErr = GetLastError();

        if (!ok &&  (
                    dwErr == ERROR_IO_PENDING 
                ||  dwErr == ERROR_MORE_DATA))
        {
            writeOk = true;
        }
        else if (dwBytesWritten)
        {
            writeOk = true;
        }

        if (writeOk)
        {
            // The operation was successfully initiated, so ownership of the
            // OVERLAPPED-derived object has passed to the io_service.
            overlapped.release();

            // Set timer.
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
        }
        else
        {
            // The operation completed immediately, so a completion notification needs
            // to be posted. When complete() is called, ownership of the OVERLAPPED-
            // derived object passes to the io_service.

            AsioErrorCode ec(
                dwErr,
                ASIO_NS::error::get_system_category());

            overlapped.complete(ec, 0);
        }

        return 0;
    }

} // namespace RCF
