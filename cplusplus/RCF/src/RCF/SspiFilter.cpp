
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

#include <RCF/SspiFilter.hpp>

#include <boost/multi_index/detail/scope_guard.hpp>

#include <RCF/AsioServerTransport.hpp>
#include <RCF/ClientStub.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/CurrentSession.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/ObjectPool.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/Schannel.hpp>
#include <RCF/ServerTransport.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/Tools.hpp>
#include <RCF/Win32Username.hpp>

#include <tchar.h>

#ifdef _UNICODE

#define INIT_SEC_INTERFACE_NAME       "InitSecurityInterfaceW"
typedef unsigned short UTCHAR;

#else

#define INIT_SEC_INTERFACE_NAME       "InitSecurityInterfaceA"
typedef unsigned char UTCHAR;

#endif

// spelling mistake in mingw headers!
#if defined(__MINGW32__) && __GNUC__ == 3 && __GNUC_MINOR__ <= 2
#define cbMaxSignature cbMaxSIgnature
#endif

// missing stuff in mingw headers
#ifdef __MINGW32__
#ifndef SEC_WINNT_AUTH_IDENTITY_VERSION
#define SEC_WINNT_AUTH_IDENTITY_VERSION 0x200

#ifndef SEC_WINNT_AUTH_IDENTITY_ANSI
#define SEC_WINNT_AUTH_IDENTITY_ANSI    0x1
#endif

#ifndef SEC_WINNT_AUTH_IDENTITY_UNICODE
#define SEC_WINNT_AUTH_IDENTITY_UNICODE 0x2
#endif

typedef struct _SEC_WINNT_AUTH_IDENTITY_EXW {
    unsigned long Version;
    unsigned long Length;
    unsigned short SEC_FAR *User;
    unsigned long UserLength;
    unsigned short SEC_FAR *Domain;
    unsigned long DomainLength;
    unsigned short SEC_FAR *Password;
    unsigned long PasswordLength;
    unsigned long Flags;
    unsigned short SEC_FAR * PackageList;
    unsigned long PackageListLength;
} SEC_WINNT_AUTH_IDENTITY_EXW, *PSEC_WINNT_AUTH_IDENTITY_EXW;

// end_ntifs

typedef struct _SEC_WINNT_AUTH_IDENTITY_EXA {
    unsigned long Version;
    unsigned long Length;
    unsigned char SEC_FAR *User;
    unsigned long UserLength;
    unsigned char SEC_FAR *Domain;
    unsigned long DomainLength;
    unsigned char SEC_FAR *Password;
    unsigned long PasswordLength;
    unsigned long Flags;
    unsigned char SEC_FAR * PackageList;
    unsigned long PackageListLength;
} SEC_WINNT_AUTH_IDENTITY_EXA, *PSEC_WINNT_AUTH_IDENTITY_EXA;

#ifdef UNICODE
#define SEC_WINNT_AUTH_IDENTITY_EX  SEC_WINNT_AUTH_IDENTITY_EXW    // ntifs
#define PSEC_WINNT_AUTH_IDENTITY_EX PSEC_WINNT_AUTH_IDENTITY_EXW   // ntifs
#else
#define SEC_WINNT_AUTH_IDENTITY_EX  SEC_WINNT_AUTH_IDENTITY_EXA
#endif

// begin_ntifs
#endif // SEC_WINNT_AUTH_IDENTITY_VERSION      

#define SP_PROT_NONE                    0

#endif // __MINGW__

#include <sspi.h>

#if defined(__MINGW32__) && (!defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0500)
#error The EXTENDED_NAME_FORMAT enum is required, and is only available if _WIN32_WINNT >= 0x500.
#endif

namespace RCF {

    PSecurityFunctionTable getSft();

    SspiFilter::SspiFilter(
        ClientStub * pClientStub,
        const tstring & packageName,
        const tstring & packageList,
        bool server,
        bool schannel) :
            mpClientStub(pClientStub),
            mPackageName(packageName),
            mPackageList(packageList),
            mQop(None),
            mContextRequirements(),
            mServer(server),
            mPreState(Ready),
            mBytesRequestedOrig(0),
            mWriteBuffer(0),
            mWriteBufferPos(0),
            mWriteBufferLen(0),
            mReadBuffer(0),
            mReadBufferPos(0),
            mReadBufferLen(0),
            mPostState(Ready),
            mHaveContext(false),
            mHaveCredentials(false),
            mImplicitCredentials(true),
            mContext(),
            mCredentials(),
            mPkgInfo(),
            mContextState(AuthContinue),
            mEvent(ReadIssued),
            mLimitRecursion(!server),
            mSchannel(schannel),
            mMaxMessageLength(),
            mReadAheadChunkSize(schannel ? 0x10000 : 4),
            mRemainingDataPos(0),
            mEnabledProtocols(0)
    {
        memset(&mContext, 0, sizeof(mContext));
        memset(&mCredentials, 0, sizeof(mCredentials));
        memset(&mPkgInfo, 0, sizeof(mPkgInfo));

        init();
    }

    SspiFilter::SspiFilter(
        ClientStub * pClientStub,
        QualityOfProtection qop,
        ULONG contextRequirements,
        const tstring &packageName,
        const tstring &packageList,
        bool server,
        bool schannel) :
            mpClientStub(pClientStub),
            mPackageName(packageName),
            mPackageList(packageList),
            mQop(qop),
            mContextRequirements(contextRequirements),
            mServer(server),
            mPreState(Ready),
            mBytesRequestedOrig(),
            mWriteBuffer(),
            mWriteBufferPos(),
            mWriteBufferLen(),
            mReadBuffer(),
            mReadBufferPos(),
            mReadBufferLen(),
            mPostState(Ready),
            mHaveContext(),
            mHaveCredentials(),
            mImplicitCredentials(true),
            mContext(),
            mCredentials(),
            mPkgInfo(),
            mContextState(AuthContinue),
            mEvent(ReadIssued),
            mLimitRecursion(!server),
            mSchannel(schannel),
            mMaxMessageLength(),
            mReadAheadChunkSize(schannel ? 0x10000 : 4),
            mRemainingDataPos(),
            mEnabledProtocols(0)
    {
        memset(&mContext, 0, sizeof(mContext));
        memset(&mCredentials, 0, sizeof(mCredentials));

        mPkgInfo.Name = NULL;
        mPkgInfo.Comment = NULL;

        init();
    }

    // client mode ctor, accessible to the public
    SspiFilter::SspiFilter(
        ClientStub * pClientStub,
        QualityOfProtection qop,
        ULONG contextRequirements,
        const tstring &packageName,
        const tstring &packageList,
        bool server) :
            mpClientStub(pClientStub),
            mPackageName(packageName),
            mPackageList(packageList),
            mQop(qop),
            mContextRequirements(contextRequirements),
            mServer(server),
            mPreState(Ready),
            mBytesRequestedOrig(),
            mWriteBuffer(),
            mWriteBufferPos(),
            mWriteBufferLen(),
            mReadBuffer(),
            mReadBufferPos(),
            mReadBufferLen(),
            mPostState(Ready),
            mHaveContext(),
            mHaveCredentials(),
            mImplicitCredentials(),
            mContext(),
            mCredentials(),
            mPkgInfo(),
            mContextState(AuthContinue),
            mEvent(ReadIssued),
            mLimitRecursion(!server),
            mSchannel(false),
            mMaxMessageLength(),
            mReadAheadChunkSize(4),
            mRemainingDataPos(),
            mEnabledProtocols(0)
    {

        memset(&mContext, 0, sizeof(mContext));
        memset(&mCredentials, 0, sizeof(mCredentials));

        mPkgInfo.Name = NULL;
        mPkgInfo.Comment = NULL;

        RCF_ASSERT(pClientStub);
        tstring usernameAndDomain = pClientStub->getUsername();
        
        tstring userName = usernameAndDomain;
        tstring domain = RCF_T("");
        std::size_t pos = usernameAndDomain.find(RCF_T("\\"));
        if (pos != tstring::npos && pos != usernameAndDomain.size()-1)
        {
            domain = usernameAndDomain.substr(0, pos);
            userName = usernameAndDomain.substr(pos+1);
        }
        tstring password = pClientStub->getPassword();

        acquireCredentials(userName, password, domain);

        init();
    }

    SspiFilter::~SspiFilter()
    {
        RCF_DTOR_BEGIN
            deinit();
            freeCredentials();
        RCF_DTOR_END
    }

    SspiFilter::QualityOfProtection SspiFilter::getQop()
    {
        return mQop;
    }

    void SspiFilter::freeCredentials()
    {
        if (mHaveCredentials)
        {
            SECURITY_STATUS status = 0;
            status = getSft()->FreeCredentialsHandle(&mCredentials);

            RCF_VERIFY(
                status == SEC_E_OK || status == SEC_E_INVALID_HANDLE,
                FilterException(
                    _RcfError_Sspi("FreeCredentialsHandle()"),
                    status,
                    RcfSubsystem_Os));
        }

        if (mPkgInfo.Name)
        {
            delete [] mPkgInfo.Name;
        }

        if (mPkgInfo.Comment)
        {
            delete [] mPkgInfo.Comment;
        }

    }

    void SspiFilter::resetState()
    {
        init();
    }

    void SspiFilter::deinit()
    {
        if (mHaveContext)
        {
            SECURITY_STATUS status = 0;       
            status = getSft()->DeleteSecurityContext(&mContext);
            RCF_VERIFY(
                status == SEC_E_OK || status == SEC_E_INVALID_HANDLE,
                FilterException(
                    _RcfError_Sspi("DeleteSecurityContext()"),
                    status,
                    RcfSubsystem_Os));
            mHaveContext = false;
        }

        mReadBufferVectorPtr.reset();
        mWriteBufferVectorPtr.reset();
    }

    void SspiFilter::init()
    {
        deinit();

        mPreState = Ready;
        mPostState = Ready;
        mContextState = AuthContinue;
        mEvent = ReadIssued;

        mProtocolChecked = false;

        resizeReadBuffer(0);
        resizeWriteBuffer(0);

    }

    void SspiFilter::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        // Client-side - cache the current max message length setting.
        if (!mServer)
        {
            ClientStub * pClientStub = getTlsClientStubPtr();
            if (pClientStub)
            {
                ClientTransport & clientTransport = pClientStub->getTransport();
                mMaxMessageLength = clientTransport.getMaxMessageLength();
            }
        }

        if (byteBuffer.isEmpty() && bytesRequested == 0)
        {
            // If we have some data, issue a read completion, otherwise return
            // our buffers to the pool and issue a zero byte read on the next
            // filter.

            if (mReadBufferPos < mReadBufferLen)
            {
                mpPreFilter->onReadCompleted(byteBuffer);
            }
            else if (mReadBufferPos == mReadBufferLen && mRemainingDataPos > 0)
            {
                mpPreFilter->onReadCompleted(byteBuffer);
            }
            else
            {
                RCF_ASSERT_EQ(mReadBufferPos , mReadBufferLen);

                if (mReadBufferVectorPtr)
                {
                    mReadBufferVectorPtr->resize(0);
                }
                if (mWriteBufferVectorPtr)
                {
                    mWriteBufferVectorPtr->resize(0);
                }
                
                mTempByteBuffer.clear();
                mReadByteBuffer.clear();
                mWriteByteBuffer.clear();

                mReadBufferVectorPtr.reset();
                mWriteBufferVectorPtr.reset();

                // Forward the zero-byte read to the next filter.
                mReadByteBufferOrig = byteBuffer;
                mBytesRequestedOrig = bytesRequested;
                mPreState = Reading;

                mpPostFilter->read(ByteBuffer(), 0);
            }
        }
        else
        {
            mReadByteBufferOrig = byteBuffer;
            mBytesRequestedOrig = bytesRequested;
            mPreState = Reading;
            handleEvent(ReadIssued);
        }
    }

    void SspiFilter::write(const std::vector<ByteBuffer> &byteBuffers)
    {
        // Client-side - cache the current max message length setting.
        if (!mServer)
        {
            ClientStub * pClientStub = getTlsClientStubPtr();
            if (pClientStub)
            {
                ClientTransport & clientTransport = pClientStub->getTransport();
                mMaxMessageLength = clientTransport.getMaxMessageLength();
            }
        }

        // TODO: can we pass multiple buffers through to lower layers, and still 
        // have them coalesced at the network send stage?

        // If we are given multiple small buffers, copy them into a single larger
        // buffer to improve network performance. 

        std::size_t MaxMergeBufferLen = 1024*1024; // 1 MB

        // SSL won't do more than ~16536 bytes in one message.
        const std::size_t MaxSchannelLen = 16000;
        if (mSchannel)
        {
            MaxMergeBufferLen = MaxSchannelLen;
        }

        ByteBuffer mergeBuffer;
        if (byteBuffers.size() > 1)
        {
            // Check if we have small enough buffers to merge.
            //std::vector<ByteBuffer> mergeBufferList;
            mMergeBufferList.resize(0);
            sliceByteBuffers(mMergeBufferList, byteBuffers, 0, MaxMergeBufferLen);
            if (mMergeBufferList.size() > 1)
            {
                // Allocate merge buffer.
                ReallocBufferPtr mergeVecPtr = getObjectPool().getReallocBufferPtr();
                mergeVecPtr->resize( RCF_MIN(MaxMergeBufferLen, lengthByteBuffers(byteBuffers)) );

                // Copy to merge buffer.
                copyByteBuffers(mMergeBufferList, &(*mergeVecPtr)[0]);
                std::size_t mergeLength = lengthByteBuffers(mMergeBufferList);
                mergeVecPtr->resize(mergeLength);
                mergeBuffer = ByteBuffer(mergeVecPtr);

                mMergeBufferList.resize(0);
            }
        }

        // If we have a merge buffer, use that for the next write operation.
        if (mergeBuffer.getLength() > 0)
        {
            mWriteByteBufferOrig = mergeBuffer;
        }
        else
        {
            mWriteByteBufferOrig = byteBuffers.front();
        }
        
        if (mSchannel)
        {
            mWriteByteBufferOrig = ByteBuffer( 
                mWriteByteBufferOrig, 
                0, 
                RCF_MIN(mWriteByteBufferOrig.getLength(), MaxSchannelLen));
        }

        mPreState = Writing;
        handleEvent(WriteIssued);
    }

    void SspiFilter::onReadCompleted_(
        const ByteBuffer &byteBuffer)
    {
        if (mPreState == Reading && mBytesRequestedOrig == 0)
        {
            RCF_ASSERT(byteBuffer.isEmpty());
            mpPreFilter->onReadCompleted(ByteBuffer());
        }
        else
        {
            RCF_ASSERT(
                    byteBuffer.isEmpty()
                ||  mReadBuffer + mReadBufferPos == byteBuffer.getPtr())
                (mReadBuffer)(mReadBufferPos)(byteBuffer.getPtr());

            mReadBufferPos += byteBuffer.getLength();

            RCF_ASSERT_LTEQ(mReadBufferPos , mReadBufferLen);

            if ( mSchannel && mServer && !mProtocolChecked && mReadBufferPos >= 1 )
            {
                // SSL 3.0 starts with \x16. SSL 2.0 starts with \x80.
                unsigned char firstByte = mReadBuffer[0];
                if ( firstByte != '\x16' && firstByte != (unsigned char) '\x80')
                {
                    Exception e(_RcfError_NotSslHandshake());
                    RCF_THROW(e);
                }
                mProtocolChecked = true;
            }

            const_cast<ByteBuffer &>(byteBuffer).clear();
            handleEvent(ReadCompleted);
        }
    }

    // Recursion limiter can only be used on synchronous filter stacks, and
    // avoids excessive recursion when reading or writing data in small pieces.
    // On asynchronous filter stacks, it would introduce a race condition by setting
    // filter state after invoking downstream async read/write operations.
    void SspiFilter::onReadCompleted(
        const ByteBuffer &byteBuffer)
    {
        if (mLimitRecursion)
        {
            applyRecursionLimiter(
                mRecursionStateRead,
                &SspiFilter::onReadCompleted_,
                *this,
                byteBuffer);
        }
        else
        {
            onReadCompleted_(byteBuffer);
        }
    }

    void SspiFilter::onWriteCompleted_(
        std::size_t bytesTransferred)
    {
        mByteBuffers.resize(0);
        mWriteBufferPos += bytesTransferred;
       
        RCF_ASSERT_LTEQ(mWriteBufferPos , mWriteBufferLen);

        handleEvent(WriteCompleted);
    }

    void SspiFilter::onWriteCompleted(
        std::size_t bytesTransferred)
    {
        if (mLimitRecursion)
        {
            applyRecursionLimiter(
                mRecursionStateWrite,
                &SspiFilter::onWriteCompleted_,
                *this,
                bytesTransferred);
        }
        else
        {
            onWriteCompleted_(bytesTransferred);
        }
    }

    void SspiFilter::handleEvent(Event event)
    {
        RCF_ASSERT(
            event == ReadIssued || event == WriteIssued ||
            event == ReadCompleted || event == WriteCompleted)(event);

        mTempByteBuffer.clear();

        mEvent = event;
        if ((mEvent == ReadIssued || mEvent == WriteIssued) || completeBlock())
        {
            if (mContextState != AuthOkAck)
            {
                handleHandshakeEvent();
            }
            else
            {
                switch (mEvent)
                {
                case ReadIssued:

                    if (0 < mReadBufferPos && mReadBufferPos < mReadBufferLen)
                    {
                        // read from currently decrypted  block
                        std::size_t bytesAvail = mReadBufferLen - mReadBufferPos;

                        std::size_t bytesToRead =
                            RCF_MIN(bytesAvail, mBytesRequestedOrig);

                        if (mReadByteBufferOrig.getLength() > 0)
                        {
                            memcpy(
                                mReadByteBufferOrig.getPtr(),
                                mReadBuffer+mReadBufferPos,
                                bytesToRead);

                            mTempByteBuffer = ByteBuffer(
                                mReadByteBufferOrig,
                                0,
                                bytesToRead);
                        }
                        else
                        {
                            mTempByteBuffer = ByteBuffer(
                                mReadByteBuffer,
                                mReadBufferPos,
                                bytesToRead);
                        }
                       
                        mReadBufferPos += bytesToRead;
                        mReadByteBufferOrig = ByteBuffer();

                        mRecursionStateRead.clear();
                        mpPreFilter->onReadCompleted(mTempByteBuffer);
                    }
                    else if (mRemainingDataPos)
                    {
                        RCF_ASSERT(mSchannel);
                        shiftReadBuffer();
                        handleEvent(ReadCompleted);
                    }
                    else
                    {
                        // read in a new block
                        resizeReadBuffer(mReadAheadChunkSize);
                        readBuffer();
                    }
                    break;

                case WriteIssued:

                    mSchannel ?
                        encryptWriteBufferSchannel() :
                        encryptWriteBuffer();

                    writeBuffer();
                    break;

                case ReadCompleted:

                    {
                        bool ok = mSchannel ?
                            decryptReadBufferSchannel() :
                            decryptReadBuffer();

                        if (ok)
                        {
                            handleEvent(ReadIssued);
                        }
                    }
                    
                    break;

                case WriteCompleted:

                    {
                        std::size_t bytesTransferred =
                            mWriteByteBufferOrig.getLength();

                        mRecursionStateWrite.clear();
                        mWriteByteBufferOrig = ByteBuffer();
                        mpPreFilter->onWriteCompleted(bytesTransferred);
                    }
                   
                    break;

                default:
                    RCF_ASSERT(0);
                }
            }
        }
    }

    void SspiFilter::readBuffer()
    {
        RCF_ASSERT_LTEQ(mReadBufferPos, mReadBufferLen);
        mPostState = Reading;
        mTempByteBuffer = ByteBuffer(mReadByteBuffer, mReadBufferPos);
        mpPostFilter->read(mTempByteBuffer, mReadBufferLen-mReadBufferPos);
    }

    void SspiFilter::writeBuffer()
    {
        RCF_ASSERT_LTEQ(mWriteBufferPos, mWriteBufferLen);
        mPostState = Writing;
        mByteBuffers.resize(0);
        mByteBuffers.push_back( ByteBuffer(mWriteByteBuffer, mWriteBufferPos));
        mpPostFilter->write(mByteBuffers);
    }

    bool SspiFilter::completeReadBlock()
    {
        if (mSchannel)
        {
            return true;
        }

        RCF_ASSERT_LTEQ(mReadBufferPos, mReadBufferLen);

        if (mReadBufferPos == mReadBufferLen && mReadBufferLen == 4)
        {
            // Got the 4 byte length field, now read the rest of the block.
            BOOST_STATIC_ASSERT( sizeof(unsigned int) == 4 );
            BOOST_STATIC_ASSERT( sizeof(DWORD) == 4 );

            unsigned int len = * (unsigned int *) mReadBuffer;
            bool integrity = (len & (1<<30)) ? true : false;
            bool encryption = (len & (1<<31)) ? true : false;
            len = len & ~(1<<30);
            len = len & ~(1<<31);

            // Check the length against the max message length.
            if (mMaxMessageLength && len > mMaxMessageLength)
            {
                int rcfError = mServer ? 
                    RcfError_ServerMessageLength : 
                    RcfError_ClientMessageLength;

                Error err(rcfError);
                Exception e(err);
                RCF_THROW(e)(mMaxMessageLength)(len);
            }

            * (unsigned int *) mReadBuffer = len;

            RCF_ASSERT( !(integrity && encryption) );

            if (mServer)
            {
                if (integrity)
                {
                    mQop = Integrity;
                }
                else if (encryption)
                {
                    mQop = Encryption;
                }
                else
                {
                    mQop = None;
                }
            }

            resizeReadBuffer(4+len);
            mReadBufferPos = 4;
            readBuffer();
            return false;
        }

        return (mReadBufferPos < mReadBufferLen) ?
            readBuffer(), false :
            true;
    }

    bool SspiFilter::completeWriteBlock()
    {
        RCF_ASSERT_LTEQ(mWriteBufferPos,  mWriteBufferLen);

        return (mWriteBufferPos < mWriteBufferLen) ?
            writeBuffer(), false :
            true;
    }

    bool SspiFilter::completeBlock()
    {
        // check to see if a whole block was read or written
        // if not, issue another read or write
        RCF_ASSERT(
            mPostState == Reading || mPostState == Writing )
            (mPostState);

        return
            mPostState == Reading ?
                completeReadBlock() :
                completeWriteBlock();
    }

    void SspiFilter::resizeReadBuffer(std::size_t newSize)
    {
        mTempByteBuffer.clear();
        mReadByteBuffer.clear();
        if (!mReadBufferVectorPtr)
        {
            mReadBufferVectorPtr = getObjectPool().getReallocBufferPtr();
        }

        std::size_t newSize_ = newSize == 0 ? 1 : newSize;
        mReadBufferVectorPtr->resize(newSize_);
        mReadByteBuffer = ByteBuffer(mReadBufferVectorPtr);
        mReadBuffer = mReadByteBuffer.getPtr();
        mReadBufferPos = 0;
        mReadBufferLen = mReadByteBuffer.getLength();
        mReadBufferLen = (mReadBufferLen == 1) ? 0 : mReadBufferLen;

        RCF_ASSERT_EQ(mReadBufferLen , newSize);
    }

    void SspiFilter::resizeWriteBuffer(std::size_t newSize)
    {
        mWriteByteBuffer.clear();
        if (!mWriteBufferVectorPtr)
        {
            mWriteBufferVectorPtr = getObjectPool().getReallocBufferPtr();
        }

        std::size_t newSize_ = newSize == 0 ? 1 : newSize;
        mWriteBufferVectorPtr->resize(newSize_);
        mWriteByteBuffer = ByteBuffer(mWriteBufferVectorPtr);
        mWriteBuffer = mWriteByteBuffer.getPtr();
        mWriteBufferPos = 0;
        mWriteBufferLen = mWriteByteBuffer.getLength();
        mWriteBufferLen = mWriteBufferLen == 1 ? 0 : mWriteBufferLen;
        RCF_ASSERT_EQ(mWriteBufferLen , newSize);
    }

    void SspiFilter::shiftReadBuffer()
    {
        RCF_ASSERT(     0 < mRemainingDataPos 
                    &&  mRemainingDataPos < mReadBufferVectorPtr->size());

        mReadBufferPos = mReadBufferVectorPtr->size();
        std::size_t bytesToMove = mReadBufferPos - mRemainingDataPos;
        char * pchFrom = mReadBuffer + mRemainingDataPos;
        char * pchTo = mReadBuffer;
        memmove(pchTo, pchFrom, bytesToMove);
        mReadBufferPos = bytesToMove;
        mRemainingDataPos = 0;
        trimReadBuffer();
    }

    void SspiFilter::trimReadBuffer()
    {
        mReadBufferVectorPtr->resize(mReadBufferPos);
        mReadByteBuffer = ByteBuffer(mReadBufferVectorPtr);
        mReadBuffer = mReadByteBuffer.getPtr();
        mReadBufferLen = mReadByteBuffer.getLength();
        mReadBufferLen = (mReadBufferLen == 1) ? 0 : mReadBufferLen;
    }

    void SspiFilter::encryptWriteBuffer()
    {
        // encrypt the pre buffer to the write buffer

        RCF_ASSERT_EQ(mContextState , AuthOkAck);

        if (mQop == Integrity)
        {
            SecPkgContext_Sizes sizes;
            getSft()->QueryContextAttributes(
                &mContext,
                SECPKG_ATTR_SIZES,
                &sizes);

            DWORD cbPacketLength    = 4;
            DWORD cbMsgLength       = 4;
            DWORD cbMsg             = static_cast<DWORD>(mWriteByteBufferOrig.getLength());
            DWORD cbSignature       = sizes.cbMaxSignature;
            DWORD cbPacket            = cbMsgLength + cbMsg + cbSignature;

            resizeWriteBuffer(cbPacketLength + cbPacket);
            memcpy(mWriteBuffer+cbPacketLength, &cbMsg, cbMsgLength);
            memcpy(
                mWriteBuffer+cbPacketLength+cbMsgLength,
                mWriteByteBufferOrig.getPtr(),
                mWriteByteBufferOrig.getLength());

            char *pMsg              = &mWriteBuffer[4];
            SecBuffer rgsb[2]       = {0,0};
            rgsb[0].cbBuffer        = cbMsg;
            rgsb[0].BufferType      = SECBUFFER_DATA;
            rgsb[0].pvBuffer        = pMsg + cbMsgLength;
            rgsb[1].cbBuffer        = cbSignature;
            rgsb[1].BufferType      = SECBUFFER_TOKEN;
            rgsb[1].pvBuffer        = pMsg + cbMsgLength + cbMsg;
            SecBufferDesc sbd       = {0};
            sbd.ulVersion           = SECBUFFER_VERSION;
            sbd.cBuffers            = sizeof(rgsb)/sizeof(*rgsb);
            sbd.pBuffers            = rgsb;

            SECURITY_STATUS status = getSft()->MakeSignature(
                &mContext,
                0,
                &sbd,
                0);

            RCF_VERIFY(
                status == SEC_E_OK,
                FilterException(
                    _RcfError_SspiEncrypt("MakeSignature()"),
                    status,
                    RcfSubsystem_Os))(status);

            cbSignature                = rgsb[1].cbBuffer;
            cbPacket                = cbMsgLength + cbMsg + cbSignature;
            resizeWriteBuffer(cbPacketLength + cbPacket);
            DWORD encodedLength        = cbPacket;
            RCF_ASSERT_LT(encodedLength , (1<<30));
            encodedLength            = encodedLength | (1<<30);
            * (DWORD*) mWriteBuffer = encodedLength;
        }
        else if (mQop == Encryption)
        {
            SecPkgContext_Sizes sizes;
            getSft()->QueryContextAttributes(
                &mContext,
                SECPKG_ATTR_SIZES,
                &sizes);

            DWORD cbPacketLength    = 4;
            DWORD cbMsgLength       = 4;
            DWORD cbMsg             = static_cast<DWORD>(mWriteByteBufferOrig.getLength());
            DWORD cbTrailer         = sizes.cbSecurityTrailer;
            DWORD cbPacket            = cbMsgLength + cbMsg + cbTrailer;

            resizeWriteBuffer(cbPacketLength + cbPacket);
            memcpy(mWriteBuffer+cbPacketLength, &cbMsg, cbMsgLength);
            memcpy(
                mWriteBuffer+cbPacketLength+cbMsgLength,
                mWriteByteBufferOrig.getPtr(),
                mWriteByteBufferOrig.getLength());

            BYTE *pEncryptedMsg     =((BYTE *) mWriteBuffer) + 4;
            SecBuffer rgsb[2]       = {0,0};
            rgsb[0].cbBuffer        = cbMsg;
            rgsb[0].BufferType      = SECBUFFER_DATA;
            rgsb[0].pvBuffer        = pEncryptedMsg + cbMsgLength;
            rgsb[1].cbBuffer        = cbTrailer;
            rgsb[1].BufferType      = SECBUFFER_TOKEN;
            rgsb[1].pvBuffer        = pEncryptedMsg + cbMsgLength + cbMsg;
            SecBufferDesc sbd       = {0};
            sbd.ulVersion           = SECBUFFER_VERSION;
            sbd.cBuffers            = sizeof(rgsb)/sizeof(*rgsb);
            sbd.pBuffers            = rgsb;

            SECURITY_STATUS status = getSft()->EncryptMessage(
                &mContext,
                0,
                &sbd,
                0);

            RCF_VERIFY(
                status == SEC_E_OK,
                FilterException(
                    _RcfError_SspiEncrypt("EncryptMessage()"),
                    status,
                    RcfSubsystem_Os));

            cbTrailer               = rgsb[1].cbBuffer;
            cbPacket                = cbMsgLength + cbMsg + cbTrailer;
            resizeWriteBuffer(cbPacketLength + cbPacket);
            DWORD encodedLength     = cbPacket;
            RCF_ASSERT_LT(encodedLength , (1<<30));
            encodedLength           = encodedLength | (1<<31);
            * (DWORD*) mWriteBuffer = encodedLength;
        }
        else
        {
            RCF_ASSERT_EQ(mQop , None);
            RCF_ASSERT_LT(mWriteByteBufferOrig.getLength() , std::size_t(1) << 31 );

            resizeWriteBuffer(mWriteByteBufferOrig.getLength()+4);
            memcpy(
                mWriteBuffer+4,
                mWriteByteBufferOrig.getPtr(),
                mWriteByteBufferOrig.getLength());

            DWORD dw = static_cast<DWORD>(mWriteByteBufferOrig.getLength());
            *(DWORD*) mWriteBuffer  = dw;
        }

    }

    bool SspiFilter::decryptReadBuffer()
    {
        // decrypt read buffer in place

        RCF_ASSERT_EQ(mContextState , AuthOkAck);

        if (mQop == Integrity)
        {
            BYTE *pMsg              = ((BYTE *) mReadBuffer) + 4;
            DWORD cbPacketLength    = 4;
            DWORD cbPacket          = *(DWORD*) mReadBuffer;

            RCF_VERIFY(
                cbPacket <= mReadBufferLen, 
                Exception(_RcfError_SspiLengthField(
                    cbPacket, 
                    static_cast<int>(mReadBufferLen))));

            DWORD cbMsgLength       = 4;
            DWORD cbMsg             = *(DWORD*) pMsg;

            RCF_VERIFY(
                cbMsg <= mReadBufferLen, 
                Exception(_RcfError_SspiLengthField(
                    cbMsg, 
                    static_cast<int>(mReadBufferLen))));

            DWORD cbSignature       = cbPacket - cbMsgLength - cbMsg;
            SecBuffer rgsb[2]       = {0,0};
            rgsb[0].cbBuffer        = cbMsg;
            rgsb[0].BufferType      = SECBUFFER_DATA;
            rgsb[0].pvBuffer        = pMsg + cbMsgLength;
            rgsb[1].cbBuffer        = cbSignature;
            rgsb[1].BufferType      = SECBUFFER_TOKEN;
            rgsb[1].pvBuffer        = pMsg + cbMsgLength + cbMsg;
            SecBufferDesc sbd       = {0};
            sbd.ulVersion           = SECBUFFER_VERSION;
            sbd.cBuffers            = sizeof(rgsb)/sizeof(*rgsb);
            sbd.pBuffers            = rgsb;
            ULONG qop               = 0;
            SECURITY_STATUS status  = getSft()->VerifySignature(
                &mContext,
                &sbd,
                0,
                &qop);

            RCF_VERIFY(
                status == SEC_E_OK,
                FilterException(
                    _RcfError_SspiDecrypt("VerifySignature()"),
                    status,
                    RcfSubsystem_Os));

            resizeReadBuffer(cbPacketLength + cbMsgLength + cbMsg);
            mReadBufferPos          = cbPacketLength + cbMsgLength;
        }
        else if (mQop == Encryption)
        {
            BYTE *pMsg              = ((BYTE *) mReadBuffer) + 4;
            DWORD cbPacketLength    = 4;
            DWORD cbPacket          = *(DWORD*)mReadBuffer;

            RCF_VERIFY(
                cbPacket <= mReadBufferLen, 
                Exception(_RcfError_SspiLengthField(
                    cbPacket, 
                    static_cast<int>(mReadBufferLen))));

            DWORD cbMsgLength       = 4;
            DWORD cbMsg             = *(DWORD*) pMsg;

            RCF_VERIFY(
                cbMsg <= mReadBufferLen, 
                Exception(_RcfError_SspiLengthField(
                    cbMsg, 
                    static_cast<int>(mReadBufferLen))));

            DWORD cbTrailer         = (cbPacket - cbMsgLength) - cbMsg;
            SecBuffer rgsb[2]       = {0,0};
            rgsb[0].cbBuffer        = cbMsg;
            rgsb[0].BufferType      = SECBUFFER_DATA;
            rgsb[0].pvBuffer        = pMsg + cbMsgLength;
            rgsb[1].cbBuffer        = cbTrailer;
            rgsb[1].BufferType      = SECBUFFER_TOKEN;
            rgsb[1].pvBuffer        = pMsg + cbMsgLength + cbMsg;
            SecBufferDesc sbd       = {0};
            sbd.ulVersion           = SECBUFFER_VERSION;
            sbd.cBuffers            = sizeof(rgsb)/sizeof(*rgsb);
            sbd.pBuffers            = rgsb;
            ULONG qop               = 0;

            SECURITY_STATUS status  = getSft()->DecryptMessage(
                &mContext,
                &sbd,
                0,
                &qop);

            RCF_VERIFY(
                status == SEC_E_OK,
                FilterException(
                    _RcfError_SspiDecrypt("DecryptMessage()"),
                    status,
                    RcfSubsystem_Os));

            resizeReadBuffer(cbPacketLength + cbMsgLength + cbMsg);
            mReadBufferPos          = cbPacketLength + cbMsgLength;
        }
        else
        {
            RCF_ASSERT_EQ(mQop , None);
            mReadBufferPos = 4;
        }

        return true;
    }

    void SspiFilter::resumeUserIo()
    {
        RCF_ASSERT( mPreState == Reading || mPreState == Writing )(mPreState);
        handleEvent( mPreState == Reading ? ReadIssued : WriteIssued );
    }

    SspiImpersonator::SspiImpersonator(SspiFilterPtr sspiFilterPtr) :
        mSspiFilterPtr(sspiFilterPtr)
    {
    }

    SspiImpersonator::SspiImpersonator(RcfSession & session)
    {
        std::vector<FilterPtr> filters;
        session.getTransportFilters(filters);
        for (std::size_t i=0; i<filters.size(); ++i)
        {
            int filterId = filters[i]->getFilterId();

            if (    filterId == RcfFilter_SspiNtlm 
                ||  filterId == RcfFilter_SspiKerberos 
                ||  filterId == RcfFilter_SspiNegotiate)
            {
                mSspiFilterPtr = boost::static_pointer_cast<SspiFilter>(filters[i]);
            }
        }

        if (!mSspiFilterPtr)
        {
            RCF_THROW( Exception(_RcfError_SspiImpersonateNoSspi()) );
        }
    }

    SspiImpersonator::~SspiImpersonator()
    {
        RCF_DTOR_BEGIN
            revertToSelf();
        RCF_DTOR_END
    }

    bool SspiImpersonator::impersonate()
    {
        if (mSspiFilterPtr)
        {
            SECURITY_STATUS status = 
                getSft()->ImpersonateSecurityContext(&mSspiFilterPtr->mContext);

            RCF_VERIFY(
                status == SEC_E_OK,
                FilterException(
                    _RcfError_SspiImpersonation("ImpersonateSecurityContext()"), 
                    status, 
                    RcfSubsystem_Os));

            return true;
        }
        else
        {
            return false;
        }
    }

    void SspiImpersonator::revertToSelf() const
    {
        if (mSspiFilterPtr)
        {
            SECURITY_STATUS status = 
                getSft()->RevertSecurityContext(&mSspiFilterPtr->mContext);

            RCF_VERIFY(
                status == SEC_E_OK,
                FilterException(
                    _RcfError_SspiImpersonation("RevertSecurityContext()"), 
                    status, 
                    RcfSubsystem_Os));
        }
    }
   
    bool SspiServerFilter::doHandshake()
    {
        // use the block in the read buffer to proceed through the handshake procedure

        // lazy acquiring of implicit credentials
        if (mImplicitCredentials && !mHaveCredentials)
        {
            acquireCredentials();
        }

        DWORD cbPacket          = mPkgInfo.cbMaxToken;
        DWORD cbPacketLength    = 4;

        std::vector<char> vec(cbPacketLength + cbPacket);

        BYTE *pPacket           = (BYTE*) &vec[0];
        SecBuffer ob            = {0};
        ob.BufferType           = SECBUFFER_TOKEN;
        ob.cbBuffer             = cbPacket;
        ob.pvBuffer             = pPacket+cbPacketLength;
        SecBufferDesc obd       = {0};
        obd.cBuffers            = 1;
        obd.ulVersion           = SECBUFFER_VERSION;
        obd.pBuffers            = &ob;

        RCF_ASSERT(
            mReadBufferLen == 0 || mReadBufferLen > 4)
            (mReadBufferLen);

        RCF_ASSERT(
            !mServer || (mServer && mReadBufferLen > 4))
            (mServer)(mReadBufferLen);

        SecBufferDesc ibd       = {0};
        SecBuffer ib            = {0};
        if (mReadBufferLen > 4)
        {
            ib.BufferType       = SECBUFFER_TOKEN;
            ib.cbBuffer         = *(DWORD *)mReadBuffer;
            ib.pvBuffer         = mReadBuffer+cbPacketLength;
            ibd.cBuffers        = 1;
            ibd.ulVersion       = SECBUFFER_VERSION;
            ibd.pBuffers        = &ib;
        }

        DWORD   CtxtAttr        = 0;
        TimeStamp Expiration    = {0};
        SECURITY_STATUS status  = getSft()->AcceptSecurityContext(
            &mCredentials,
            mHaveContext ? &mContext : NULL,
            &ibd,
            mContextRequirements,
            SECURITY_NATIVE_DREP,
            &mContext,
            &obd,
            &CtxtAttr,
            &Expiration);

        switch (status)
        {
        case SEC_E_OK:
        case SEC_I_CONTINUE_NEEDED:
        case SEC_I_COMPLETE_NEEDED:
        case SEC_I_COMPLETE_AND_CONTINUE:
        case SEC_E_INCOMPLETE_MESSAGE:
            mHaveContext = true;
            break;
        default:
            break;
        }

        cbPacket = ob.cbBuffer;

        // We only support NTLM, Kerberos and Negotiate SSP's, so there's never
        // a need to call CompleteAuthToken()
        RCF_ASSERT(
            status != SEC_I_COMPLETE_AND_CONTINUE &&
            status != SEC_I_COMPLETE_NEEDED)
            (status);

        if (status == SEC_I_CONTINUE_NEEDED)
        {
            // authorization ok so far, copy outbound data to write buffer
            mContextState = AuthContinue;
            *(DWORD *) pPacket = cbPacket;
            resizeWriteBuffer(cbPacketLength + cbPacket);
            memcpy(mWriteBuffer, pPacket, cbPacketLength + cbPacket);
        }
        else if (status == SEC_E_OK)
        {
            // authorization ok, send a special block of our own to notify client
            mContextState = AuthOk;
            if (cbPacket > 0)
            {
                *(DWORD *) pPacket = cbPacket;
                resizeWriteBuffer(cbPacketLength + cbPacket);
                memcpy(mWriteBuffer, pPacket, cbPacketLength + cbPacket);
            }
            else
            {
                resizeWriteBuffer(4+4+4);
                *(DWORD*) mWriteBuffer = 8;
                *(DWORD*) (mWriteBuffer+4) = RcfError_Ok;
                *(DWORD*) (mWriteBuffer+8) = 0;
            }

            // Impersonate client to get their username.
            SspiFilterPtr thisPtr(this, NullDeleter());
            SspiImpersonator impersonator(thisPtr);
            impersonator.impersonate();
            tstring domainAndUsername = RCF::getMyDomain() + RCF_T("\\") + RCF::getMyUserName();
            getCurrentRcfSession().mClientUsername = domainAndUsername;
        }
        else
        {
            // authorization failed, send a special block of our own to notify client

            RCF_LOG_2() << "SSPI handshake failed. Error: " + RCF::getOsErrorString(status);

            mContextState = AuthFailed;
            resizeWriteBuffer(4+4+4);
            *(DWORD*) mWriteBuffer = 8;
            *(DWORD*) (mWriteBuffer+4) = RcfError_SspiAuthFailServer;
            *(DWORD*) (mWriteBuffer+8) = status;

            RcfSession * pSession = getCurrentRcfSessionPtr();
            if ( pSession )
            {
                NetworkSession& nwSession = pSession->getNetworkSession();
                AsioNetworkSession& asioNwSession = static_cast<AsioNetworkSession&>(nwSession);
                asioNwSession.setCloseAfterWrite();
            }
        }

        return true;
    }

    void SspiServerFilter::handleHandshakeEvent()
    {
        // take another step through the handshake process

        switch (mEvent)
        {
        case ReadIssued:
        case WriteIssued:

            // read first block from client
            RCF_ASSERT_EQ(mEvent , ReadIssued);
            resizeReadBuffer(mReadAheadChunkSize);
            readBuffer();
            break;

        case ReadCompleted:
           
            // process inbound block and write outbound block
            {
                bool written = mSchannel ? 
                    doHandshakeSchannel() : 
                    doHandshake();

                if (written)
                {
                    writeBuffer();
                }
            }
            break;

        case WriteCompleted:

            switch (mContextState)
            {
            case AuthOk:
                mContextState = AuthOkAck;
                resumeUserIo();
                break;

            case AuthFailed:
                {
                    FilterException e(_RcfError_SspiAuthFailServer());
                    RCF_THROW(e);
                }
                break;

            default:
                resizeReadBuffer(mReadAheadChunkSize);
                readBuffer();
            }
            break;
        default:
            RCF_ASSERT(0);
        }
    }

    bool SspiClientFilter::doHandshake()
    {
        // use the block in the read buffer to proceed through the handshake procedure

        // lazy acquiring of implicit credentials
        if (mImplicitCredentials && !mHaveCredentials)
        {
            acquireCredentials();
        }

        if (mContextState == AuthOk)
        {
            if (mReadBufferLen == 12)
            {
                DWORD rcfErr = *(DWORD*) &mReadBuffer[4];
                DWORD osErr = *(DWORD*) &mReadBuffer[8];
                if (rcfErr == RcfError_Ok)
                {
                    mContextState = AuthOkAck;
                    resumeUserIo();
                    return false;
                }
                else
                {
                    RemoteException e( Error(rcfErr), osErr, RcfSubsystem_Os);
                    RCF_THROW(e);
                }
            }
            else
            {
                Exception e(_RcfError_SspiAuthFailServer());
                RCF_THROW(e);
            }
        }
       
        DWORD cbPacketLength    = 4;
        DWORD cbPacket          = mPkgInfo.cbMaxToken;
        std::vector<char> vec(cbPacket + cbPacketLength);

        BYTE *pPacket           = (BYTE*) &vec[0];
        SecBuffer ob            = {0};
        ob.BufferType           = SECBUFFER_TOKEN;
        ob.cbBuffer             = cbPacket;
        ob.pvBuffer             = pPacket + cbPacketLength;
        SecBufferDesc obd       = {0};
        obd.cBuffers            = 1;
        obd.ulVersion           = SECBUFFER_VERSION;
        obd.pBuffers            = &ob;

        RCF_ASSERT(
            mReadBufferLen == 0 || mReadBufferLen > 4)
            (mReadBufferLen);

        RCF_ASSERT(
            !mServer || (mServer && mReadBufferLen > 4))
            (mServer)(mReadBufferLen);

        SecBuffer ib            = {0};
        SecBufferDesc ibd       = {0};

        if (mReadBufferLen > 4)
        {
            ib.BufferType       = SECBUFFER_TOKEN;
            ib.cbBuffer         = *(DWORD *) mReadBuffer;
            ib.pvBuffer         = mReadBuffer + cbPacketLength;
            ibd.cBuffers        = 1;
            ibd.ulVersion       = SECBUFFER_VERSION;
            ibd.pBuffers        = &ib;
        }

        tstring strTarget = mpClientStub->getKerberosSpn();
        const TCHAR *target = strTarget.empty() ? RCF_T("") : strTarget.c_str();

        DWORD CtxtAttr          = 0;
        TimeStamp Expiration    = {0};
        ULONG CtxtReq =  mContextRequirements;

        SECURITY_STATUS status  = getSft()->InitializeSecurityContext(
            &mCredentials,
            mHaveContext ? &mContext : NULL,
            (TCHAR *) target,
            CtxtReq,
            0,
            SECURITY_NATIVE_DREP,
            (mHaveContext && mReadBufferLen > 4) ? &ibd : NULL,
            0,
            &mContext,
            &obd,
            &CtxtAttr,
            &Expiration);

        switch (status)
        {
        case SEC_E_OK:
        case SEC_I_CONTINUE_NEEDED:
        case SEC_I_COMPLETE_NEEDED:
        case SEC_I_COMPLETE_AND_CONTINUE:
        case SEC_E_INCOMPLETE_MESSAGE:
        case SEC_I_INCOMPLETE_CREDENTIALS:
            mHaveContext = true;
            break;
        default:
            break;
        }

        RCF_ASSERT(
            status != SEC_I_COMPLETE_NEEDED &&
            status != SEC_I_COMPLETE_AND_CONTINUE)
            (status);
       
        cbPacket                = ob.cbBuffer;
        if (cbPacket > 0)
        {
            *(DWORD *)pPacket   = cbPacket;
            mContextState       =
                (status == SEC_E_OK) ?
                    AuthOk :
                    (status == SEC_I_CONTINUE_NEEDED) ?
                        AuthContinue :
                        AuthFailed;

            RCF_VERIFY(
                mContextState != AuthFailed,
                Exception(
                    _RcfError_SspiAuthFailClient(),
                    status,
                    RcfSubsystem_Os,
                    "InitializeSecurityContext() failed"))(status);

            resizeWriteBuffer(cbPacketLength + cbPacket);
            memcpy(mWriteBuffer, pPacket, cbPacketLength + cbPacket);
            return true;
        }
        else
        {
            mContextState = AuthOkAck;
            resumeUserIo();
            return false;
        }

    }

    void SspiClientFilter::handleHandshakeEvent()
    {
        // take another step through the handshake process

        switch (mEvent)
        {
        case ReadIssued:
        case WriteIssued:
           
            // create first block to send to server
            //resizeReadBuffer(0);
            mSchannel ? 
                doHandshakeSchannel() : 
                doHandshake();

            writeBuffer();
            break;

        case ReadCompleted:

            // process a block, and send any emitted output block
            {
            bool written = mSchannel ? 
                doHandshakeSchannel() : 
                doHandshake();

            if (written)
            {
                writeBuffer();
            }
            }
            break;

        case WriteCompleted:

            // issue a read for the next block from the server
            resizeReadBuffer(mReadAheadChunkSize);
            readBuffer();
            break;

        default:
            RCF_ASSERT(0);
        }
    }

    void SspiFilter::setupCredentials(
        const tstring &userName,
        const tstring &password,
        const tstring &domain)
    {

        SEC_WINNT_AUTH_IDENTITY_EX identity     = {0};

        UTCHAR *pDomain = reinterpret_cast<UTCHAR*>(const_cast<TCHAR*>(domain.c_str()));
        unsigned long pDomainLen = static_cast<unsigned long>(domain.length());

        UTCHAR *pUsername = reinterpret_cast<UTCHAR*>(const_cast<TCHAR*>(userName.c_str()));
        unsigned long pUsernameLen = static_cast<unsigned long>(userName.length());

        UTCHAR *pPassword = reinterpret_cast<UTCHAR*>(const_cast<TCHAR*>(password.c_str()));
        unsigned long pPasswordLen = static_cast<unsigned long>(password.length());

        UTCHAR *pPackages = reinterpret_cast<UTCHAR*>(const_cast<TCHAR*>(mPackageList.c_str()));
        unsigned long pPackagesLen = static_cast<unsigned long>(mPackageList.length());

        if (!userName.empty())
        {
            if (!domain.empty())
            {
                identity.Domain                 = pDomain;
                identity.DomainLength           = pDomainLen;
            }
            if (!userName.empty())
            {
                identity.User                   = pUsername;
                identity.UserLength             = pUsernameLen;
            }
            if (!password.empty())
            {
                identity.Password               = pPassword;
                identity.PasswordLength         = pPasswordLen;
            }
        }

#ifdef _UNICODE
        identity.Flags                          = SEC_WINNT_AUTH_IDENTITY_UNICODE;
#else
        identity.Flags                          = SEC_WINNT_AUTH_IDENTITY_ANSI;
#endif

        identity.Version                        = SEC_WINNT_AUTH_IDENTITY_VERSION;
        identity.Length                         = sizeof(identity);
        if (!mPackageList.empty())
        {
            identity.PackageList                = pPackages;
            identity.PackageListLength          = pPackagesLen;
        }
        SEC_WINNT_AUTH_IDENTITY_EX *pIdentity = &identity;

        TimeStamp Expiration                    = {0};

        SECURITY_STATUS status = getSft()->AcquireCredentialsHandle(
            NULL,
            mPkgInfo.Name,
            mServer ? SECPKG_CRED_INBOUND : SECPKG_CRED_OUTBOUND ,
            NULL,
            pIdentity,
            NULL, NULL,
            &mCredentials,
            &Expiration);

        if (status != SEC_E_OK)
        {
            FilterException e(
                _RcfError_Sspi("AcquireCredentialsHandle()"), 
                status, 
                RcfSubsystem_Os);

            RCF_THROW(e)(mPkgInfo.Name)(userName)(domain);
        }

        mHaveCredentials = true;
    }

    void SspiFilter::acquireCredentials(
        const tstring &userName,
        const tstring &password,
        const tstring &domain)
    {
        // acquire credentials, implicitly (currently logged on user),
        // or explicitly (supply username and password)

        RCF_ASSERT(!mHaveCredentials);

        // TODO: whats with copying pPackage here?

        // setup security package
        SecPkgInfo *pPackage = NULL;
       
        SECURITY_STATUS status = getSft()->QuerySecurityPackageInfo(
            (TCHAR*) mPackageName.c_str(),
            &pPackage);

        if ( status != SEC_E_OK )
        {
            FilterException e(
                _RcfError_Sspi("QuerySecurityPackageInfo()"), 
                status, 
                RcfSubsystem_Os);

            RCF_THROW(e)(mPackageName.c_str());
        }

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4995 )  // warning C4995: '...': name was marked as #pragma deprecated
#pragma warning( disable : 4996 )  // warning C4996: '...' was declared deprecated
#endif

        TCHAR *pName = new TCHAR[ _tcslen(pPackage->Name) + 1 ];
        _tcscpy(pName, pPackage->Name);

        TCHAR *pComment = new TCHAR[ _tcslen(pPackage->Comment) + 1 ];
        _tcscpy(pComment, pPackage->Comment);

#ifdef _MSC_VER
#pragma warning( pop )
#endif

        memcpy ( (void*)&mPkgInfo, (void*)pPackage, sizeof(SecPkgInfo) );
        mPkgInfo.Name = pName;
        mPkgInfo.Comment = pComment;

        getSft()->FreeContextBuffer( (void*) pPackage );

        mSchannel ?
            setupCredentialsSchannel() :
            setupCredentials(userName, password, domain);
    }

    //**************************************************************************
    // Server filters.

    SspiServerFilter::SspiServerFilter(
        const tstring &packageName,
        const tstring &packageList,
        bool schannel) :
            SspiFilter(NULL, packageName, packageList, BoolServer, schannel)
    {
        RcfSession * pRcfSession = RCF::getTlsRcfSessionPtr();
        if (pRcfSession)
        {
            ServerTransport & serverTransport = 
                pRcfSession->getNetworkSession().getServerTransport();

            mMaxMessageLength = serverTransport.getMaxMessageLength();
        }
    }

    // NTLM
    NtlmServerFilter::NtlmServerFilter() :
        SspiServerFilter(RCF_T("NTLM"), RCF_T(""))
    {}

    int NtlmServerFilter::getFilterId() const
    {
        return RcfFilter_SspiNtlm;
    }

    // Kerberos
    KerberosServerFilter::KerberosServerFilter() :
        SspiServerFilter(RCF_T("Kerberos"), RCF_T(""))
    {}

    int KerberosServerFilter::getFilterId() const
    {
        return RcfFilter_SspiKerberos;
    }

    // Negotiate
    NegotiateServerFilter::NegotiateServerFilter(
        const tstring &packageList) :
            SspiServerFilter(RCF_T("Negotiate"), packageList)
    {}

    int NegotiateServerFilter::getFilterId() const
    {
        return RcfFilter_SspiNegotiate;
    }

    // Schannel
    int SchannelServerFilter::getFilterId() const
    {
        return RcfFilter_SspiSchannel;
    }

    //**************************************************************************
    // Filter factories

    // NTLM
    FilterPtr NtlmFilterFactory::createFilter(RcfServer &)
    {
        return FilterPtr( new NtlmServerFilter() );
    }

    int NtlmFilterFactory::getFilterId()
    {
        return RcfFilter_SspiNtlm;
    }

    // Kerberos
    FilterPtr KerberosFilterFactory::createFilter(RcfServer &)
    {
        return FilterPtr( new KerberosServerFilter() );
    }

    int KerberosFilterFactory::getFilterId()
    {
        return RcfFilter_SspiKerberos;
    }

    // Negotiate
    NegotiateFilterFactory::NegotiateFilterFactory(
        const tstring &packageList) :
            mPackageList(packageList)
    {}

    FilterPtr NegotiateFilterFactory::createFilter(RcfServer &)
    {
        return FilterPtr( new NegotiateServerFilter(mPackageList) );
    }

    int NegotiateFilterFactory::getFilterId()
    {
        return RcfFilter_SspiNegotiate;
    }

    // Schannel
    int SchannelFilterFactory::getFilterId()
    {
        return RcfFilter_SspiSchannel;
    }

    //**************************************************************************
    // Client filters

    // NTLM
    NtlmClientFilter::NtlmClientFilter(
        ClientStub *            pClientStub,
        QualityOfProtection     qop,
        ULONG                   contextRequirements) :
            SspiClientFilter(
                pClientStub,
                qop, 
                contextRequirements, 
                RCF_T("NTLM"), 
                RCF_T(""))
    {}

    int NtlmClientFilter::getFilterId() const
    {
        return RcfFilter_SspiNtlm;
    }

    // Kerberos
    KerberosClientFilter::KerberosClientFilter(
        ClientStub *            pClientStub,
        QualityOfProtection     qop,
        ULONG                   contextRequirements) :
            SspiClientFilter(
                pClientStub,
                qop, 
                contextRequirements, 
                RCF_T("Kerberos"), 
                RCF_T(""))
    {}

    int KerberosClientFilter::getFilterId() const
    {
        return RcfFilter_SspiKerberos;
    }

    // Negotiate
    NegotiateClientFilter::NegotiateClientFilter(
        ClientStub *            pClientStub,
        QualityOfProtection     qop,
        ULONG                   contextRequirements) :
            SspiClientFilter(
                pClientStub,
                qop, 
                contextRequirements, 
                RCF_T("Negotiate"), 
                RCF_T("Kerberos,NTLM"))
    {}

    int NegotiateClientFilter::getFilterId() const
    {
        return RcfFilter_SspiNegotiate;
    }

    // Schannel
    int SchannelClientFilter::getFilterId() const
    {
        return RcfFilter_SspiSchannel;
    }

    //************************************************************************

    // Global init and deinit.

    HINSTANCE               ghProvider          = NULL;      // provider dll's instance
    PSecurityFunctionTable  gpSecurityInterface = NULL;      // security interface table

    PSecurityFunctionTable getSft()
    {
        return gpSecurityInterface;
    }

    void SspiInitialize()
    {
        // load the provider dll
        ghProvider = LoadLibrary ( RCF_T("security.dll") );
        if (ghProvider == NULL)
        {
            int err = GetLastError();

            FilterException e(
                _RcfError_SspiInit("LoadLibrary() with \"security.dll\""),
                err,
                RcfSubsystem_Os);

            RCF_THROW(e);
        }

        INIT_SECURITY_INTERFACE InitSecurityInterface;

        InitSecurityInterface = reinterpret_cast<INIT_SECURITY_INTERFACE> (
            GetProcAddress(ghProvider, INIT_SEC_INTERFACE_NAME));

        if (InitSecurityInterface == NULL)
        {
            int err = GetLastError();

            FilterException e(
                _RcfError_SspiInit("GetProcAddress() with \"InitSecurityInterface\""), 
                err, 
                RcfSubsystem_Os);

            RCF_THROW(e);
        }

        gpSecurityInterface = InitSecurityInterface();
        if (gpSecurityInterface == NULL)
        {
            int err = GetLastError();
            
            FilterException e(
                _RcfError_SspiInit("InitSecurityInterface()"), 
                err, 
                RcfSubsystem_Os);

            RCF_THROW(e);
        }
    }

    void SspiUninitialize()
    {
        FreeLibrary (ghProvider);
        ghProvider = NULL;
        gpSecurityInterface = NULL;
    }

} // namespace RCF
