
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

#include <RCF/Schannel.hpp>

#include <RCF/Exception.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/SspiFilter.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/Tools.hpp>
#include <RCF/Win32Certificate.hpp>

#include <wincrypt.h>
#include <schnlsp.h>

#ifdef __MINGW32__
#ifndef CERT_STORE_ADD_USE_EXISTING
#define CERT_STORE_ADD_USE_EXISTING                         2
#endif
#endif // __MINGW32__

namespace RCF {

    PSecurityFunctionTable getSft();

    void SspiFilter::encryptWriteBufferSchannel()
    {
        // encrypt the pre buffer to the write buffer

        RCF_ASSERT_EQ(mContextState , AuthOkAck);

        SecPkgContext_Sizes sizes;
        getSft()->QueryContextAttributes(
            &mContext,
            SECPKG_ATTR_SIZES,
            &sizes);

        SecPkgContext_StreamSizes streamSizes;
        getSft()->QueryContextAttributes(
            &mContext,
            SECPKG_ATTR_STREAM_SIZES,
            &streamSizes);

        DWORD cbHeader          = streamSizes.cbHeader;
        DWORD cbMsg             = static_cast<DWORD>(mWriteByteBufferOrig.getLength());
        DWORD cbTrailer         = streamSizes.cbTrailer;
        DWORD cbPacket          = cbHeader + cbMsg + cbTrailer;

        resizeWriteBuffer(cbPacket);
        
        memcpy(
            mWriteBuffer+cbHeader,
            mWriteByteBufferOrig.getPtr(),
            mWriteByteBufferOrig.getLength());

        BYTE *pEncryptedMsg     =((BYTE *) mWriteBuffer);

        SecBuffer rgsb[4]       = {0};
        rgsb[0].cbBuffer        = cbHeader;
        rgsb[0].BufferType      = SECBUFFER_STREAM_HEADER;
        rgsb[0].pvBuffer        = pEncryptedMsg;

        rgsb[1].cbBuffer        = cbMsg;
        rgsb[1].BufferType      = SECBUFFER_DATA;
        rgsb[1].pvBuffer        = pEncryptedMsg + cbHeader;

        rgsb[2].cbBuffer        = cbTrailer;
        rgsb[2].BufferType      = SECBUFFER_STREAM_TRAILER;
        rgsb[2].pvBuffer        = pEncryptedMsg + cbHeader + cbMsg;

        rgsb[3].cbBuffer        = 0;
        rgsb[3].BufferType      = SECBUFFER_EMPTY;
        rgsb[3].pvBuffer        = NULL;

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
                RcfSubsystem_Os))(status);

        RCF_ASSERT_EQ(rgsb[0].cbBuffer , cbHeader);
        RCF_ASSERT_EQ(rgsb[1].cbBuffer , cbMsg);
        RCF_ASSERT_LTEQ(rgsb[2].cbBuffer , cbTrailer);

        cbTrailer               = rgsb[2].cbBuffer;
        cbPacket                = cbHeader + cbMsg + cbTrailer;
        resizeWriteBuffer(cbPacket);
    }

    bool SspiFilter::decryptReadBufferSchannel()
    {
        // decrypt read buffer in place

        RCF_ASSERT_EQ(mContextState , AuthOkAck);

        BYTE *pMsg              = ((BYTE *) mReadBuffer);
        DWORD cbMsg             = static_cast<DWORD>(mReadBufferPos);
        SecBuffer rgsb[4]       = {0};
        
        rgsb[0].cbBuffer        = cbMsg;
        rgsb[0].BufferType      = SECBUFFER_DATA;
        rgsb[0].pvBuffer        = pMsg;

        rgsb[1].cbBuffer        = 0;
        rgsb[1].BufferType      = SECBUFFER_EMPTY;
        rgsb[1].pvBuffer        = NULL;

        rgsb[2].cbBuffer        = 0;
        rgsb[2].BufferType      = SECBUFFER_EMPTY;
        rgsb[2].pvBuffer        = NULL;

        rgsb[3].cbBuffer        = 0;
        rgsb[3].BufferType      = SECBUFFER_EMPTY;
        rgsb[3].pvBuffer        = NULL;

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

        if (status == SEC_E_INCOMPLETE_MESSAGE)
        {
            // Not enough data.
            std::size_t readBufferPos = mReadBufferPos;
            resizeReadBuffer(mReadBufferPos + mReadAheadChunkSize);
            mReadBufferPos = readBufferPos;
            readBuffer();
            return false;
        }
        else
        {
            for (int i=1; i<4; ++i)
            {
                if (rgsb[i].BufferType == SECBUFFER_EXTRA)
                {
                    // Found extra data - set a marker where it begins.
                    char * pRemainingData = (char *) rgsb[i].pvBuffer;
                    mRemainingDataPos = pRemainingData - mReadBuffer;
                    RCF_ASSERT(0 < mRemainingDataPos && mRemainingDataPos < mReadBufferPos);
                    break;
                }
            }            
        }

        trimReadBuffer();

        RCF_VERIFY(
            status == SEC_E_OK,
            FilterException(
                _RcfError_SspiDecrypt("DecryptMessage()"),
                status,
                RcfSubsystem_Os))(status);

        RCF_ASSERT_EQ(rgsb[0].BufferType , SECBUFFER_STREAM_HEADER);
        RCF_ASSERT_EQ(rgsb[1].BufferType , SECBUFFER_DATA);
        RCF_ASSERT_EQ(rgsb[2].BufferType , SECBUFFER_STREAM_TRAILER);

        DWORD cbHeader          = rgsb[0].cbBuffer;
        DWORD cbData            = rgsb[1].cbBuffer;
        DWORD cbTrailer         = rgsb[2].cbBuffer;

        RCF_UNUSED_VARIABLE(cbTrailer);

        mReadBufferPos          = cbHeader;
        mReadBufferLen          = cbHeader + cbData;

        return true;
    }

    bool SspiServerFilter::doHandshakeSchannel()
    {
        // use the block in the read buffer to proceed through the handshake procedure

        // lazy acquiring of implicit credentials
        if (mImplicitCredentials && !mHaveCredentials)
        {
            acquireCredentials();
        }

        DWORD cbPacket          = mPkgInfo.cbMaxToken;

        SecBuffer ob            = {0};
        ob.BufferType           = SECBUFFER_TOKEN;
        ob.cbBuffer             = 0;
        ob.pvBuffer             = NULL;

        SecBufferDesc obd       = {0};
        obd.cBuffers            = 1;
        obd.ulVersion           = SECBUFFER_VERSION;
        obd.pBuffers            = &ob;
        
        SecBuffer ib[2]         = {0};
        ib[0].BufferType        = SECBUFFER_TOKEN;
        ib[0].cbBuffer          = static_cast<DWORD>(mReadBufferPos);
        ib[0].pvBuffer          = mReadBuffer;
        ib[1].BufferType        = SECBUFFER_EMPTY;
        ib[1].cbBuffer          = 0;
        ib[1].pvBuffer          = NULL;

        SecBufferDesc ibd       = {0};
        ibd.cBuffers            = 2;
        ibd.ulVersion           = SECBUFFER_VERSION;
        ibd.pBuffers            = ib;

        DWORD contextRequirements = mContextRequirements;
        if (mCertValidationCallback || mAutoCertValidation.size() > 0)
        {
            // Need this to get the client to send the server a certificate.
            contextRequirements |= ASC_REQ_MUTUAL_AUTH;
        }

        DWORD   CtxtAttr        = 0;
        TimeStamp Expiration    = {0};
        SECURITY_STATUS status  = getSft()->AcceptSecurityContext(
            &mCredentials,
            mHaveContext ? &mContext : NULL,
            &ibd,
            contextRequirements,
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
            mHaveContext = true;
            break;
        default:
            break;
        }

        cbPacket = ob.cbBuffer;

        RCF_ASSERT(
            status != SEC_I_COMPLETE_AND_CONTINUE &&
            status != SEC_I_COMPLETE_NEEDED)
            (status);

        if (status == SEC_E_INCOMPLETE_MESSAGE)
        {
            // Not enough data.
            std::size_t readBufferPos = mReadBufferPos;
            resizeReadBuffer(mReadBufferPos + mReadAheadChunkSize);
            mReadBufferPos = readBufferPos;
            readBuffer();
            return false;
        }
        else if (ib[1].BufferType == SECBUFFER_EXTRA)
        {
            // We consider any extra data at this stage to be a protocol error.
            Exception e(_RcfError_SspiHandshakeExtraData());
            RCF_THROW(e);
        }

        trimReadBuffer();
        
        if (status == SEC_I_CONTINUE_NEEDED)
        {
            // Authorization ok so far, copy outbound data to write buffer.
            resizeWriteBuffer(ob.cbBuffer);
            memcpy(mWriteBuffer, ob.pvBuffer, ob.cbBuffer);
            getSft()->FreeContextBuffer(ob.pvBuffer);
        }
        else if (status == SEC_E_OK)
        {
            // Authorization ok, send last handshake block to the client.
            mContextState = AuthOk;
            RCF_ASSERT_GT(cbPacket , 0);
            resizeWriteBuffer(cbPacket);
            memcpy(mWriteBuffer, ob.pvBuffer, ob.cbBuffer);
            getSft()->FreeContextBuffer(ob.pvBuffer);

            // Extract the peer certificate.
            PCCERT_CONTEXT pRemoteCertContext = NULL;

            status = getSft()->QueryContextAttributes(
                &mContext,
                SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                (PVOID)&pRemoteCertContext);

            if (pRemoteCertContext)
            {
                mRemoteCertPtr.reset( new Win32Certificate(pRemoteCertContext) );
            }

            // If we have a custom validation callback, call it.
            if (mCertValidationCallback)
            {
                mCertValidationCallback(mRemoteCertPtr.get());
            }
        }
        else
        {
            // Authorization failed. Do nothing here, the connection will automatically close.
            RCF_LOG_2() << "Schannel SSL handshake failed. Error: " + RCF::getOsErrorString(status);
        }

        return true;
    }

    bool SspiClientFilter::doHandshakeSchannel()
    {
        // use the block in the read buffer to proceed through the handshake procedure

        // lazy acquiring of implicit credentials
        if (mImplicitCredentials && !mHaveCredentials)
        {
            acquireCredentials();
        }
     
        SecBuffer ob            = {0};
        ob.BufferType           = SECBUFFER_TOKEN;
        ob.cbBuffer             = 0;
        ob.pvBuffer             = NULL;

        SecBufferDesc obd       = {0};
        obd.cBuffers            = 1;
        obd.ulVersion           = SECBUFFER_VERSION;
        obd.pBuffers            = &ob;

        SecBuffer ib[2]         = {0};
        
        ib[0].BufferType        = SECBUFFER_TOKEN;
        ib[0].cbBuffer          = static_cast<DWORD>(mReadBufferPos);
        ib[0].pvBuffer          = mReadBuffer;

        ib[1].BufferType        = SECBUFFER_EMPTY;
        ib[1].cbBuffer          = 0;
        ib[1].pvBuffer          = NULL;

        SecBufferDesc ibd       = {0};
        ibd.cBuffers            = 2;
        ibd.ulVersion           = SECBUFFER_VERSION;
        ibd.pBuffers            = ib;

        tstring strTarget       = mAutoCertValidation;
        const TCHAR *target     = strTarget.empty() ? RCF_T("") : strTarget.c_str();
        DWORD CtxtAttr          = 0;
        TimeStamp Expiration    = {0};

        DWORD contextRequirements = mContextRequirements;
        if (mLocalCertPtr && mLocalCertPtr->getWin32Context())
        {
            contextRequirements |= ISC_REQ_USE_SUPPLIED_CREDS;
        }

        SECURITY_STATUS status  = getSft()->InitializeSecurityContext(
            &mCredentials,
            mHaveContext ? &mContext : NULL,
            (TCHAR *) target,
            mContextRequirements,
            0,
            SECURITY_NATIVE_DREP,
            mHaveContext ? &ibd : NULL,
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

        if (status == SEC_E_INCOMPLETE_MESSAGE)
        {
            // Not enough data.
            std::size_t readBufferPos = mReadBufferPos;            
            resizeReadBuffer(mReadBufferPos + mReadAheadChunkSize);
            mReadBufferPos = readBufferPos;
            readBuffer();
            return false;
        }
        else if (ib[1].BufferType == SECBUFFER_EXTRA)
        {
            // We consider any extra data at this stage to be a protocol error.
            Exception e(_RcfError_SspiHandshakeExtraData());
            RCF_THROW(e);
        }

        trimReadBuffer();
            
        if (status == SEC_I_CONTINUE_NEEDED)
        {
            // Handshake OK so far.

            RCF_ASSERT(ob.cbBuffer);
            mContextState = AuthContinue;
            resizeWriteBuffer(ob.cbBuffer);
            memcpy(mWriteBuffer, ob.pvBuffer, ob.cbBuffer);
            getSft()->FreeContextBuffer(ob.pvBuffer);
            return true;
        }
        else if (status == SEC_E_OK)
        {
            // Handshake OK.

            // Extract the peer certificate.
            PCCERT_CONTEXT pRemoteCertContext = NULL;

            status = getSft()->QueryContextAttributes(
                &mContext,
                SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                (PVOID)&pRemoteCertContext);

            mRemoteCertPtr.reset( new Win32Certificate(pRemoteCertContext) );

            // If we have a custom validation callback, call it.
            if (mCertValidationCallback)
            {
                bool ok = mCertValidationCallback(mRemoteCertPtr.get());
                if (!ok)
                {
                    Exception e( _RcfError_SslCertVerificationCustom() );
                    RCF_THROW(e);
                }
            }

            // And now back to business.
            mContextState = AuthOkAck;
            resumeUserIo();
            return false;
        }
        else
        {
            Exception e(_RcfError_SspiAuthFailClient(), status);
            RCF_THROW(e);
            return false;
        }
    }

    void SspiFilter::setupCredentialsSchannel()
    {
        SCHANNEL_CRED schannelCred          = {0};       
        schannelCred.dwVersion              = SCHANNEL_CRED_VERSION;
        PCCERT_CONTEXT pCertContext         = NULL;
        if(mLocalCertPtr)
        {
            pCertContext                    = mLocalCertPtr->getWin32Context();
            schannelCred.cCreds             = 1;
            schannelCred.paCred             = &pCertContext;
        }

        schannelCred.grbitEnabledProtocols  = mEnabledProtocols;

        if (mServer)
        {
            if (mCertValidationCallback)
            {
                // Server side manual validation.
                schannelCred.dwFlags            = SCH_CRED_MANUAL_CRED_VALIDATION;
            }
            else
            {
                // Server side auto validation.
                schannelCred.dwFlags            = 0;
            }
        }
        else
        {
            if (mCertValidationCallback)
            {
                // Client side manual validation.
                schannelCred.dwFlags            = SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION;
            }
            else
            {
                // Client side auto validation.
                schannelCred.dwFlags            = SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_AUTO_CRED_VALIDATION;
            }
        }

        SECURITY_STATUS status = getSft()->AcquireCredentialsHandle(
            NULL,
            UNISP_NAME,
            mServer ? SECPKG_CRED_INBOUND : SECPKG_CRED_OUTBOUND ,
            NULL,
            &schannelCred,
            NULL, 
            NULL,
            &mCredentials,
            NULL);

        if (status != SEC_E_OK)
        {
            FilterException e(
                _RcfError_Sspi("AcquireCredentialsHandle()"), 
                status, 
                RcfSubsystem_Os);

            RCF_THROW(e)(mPkgInfo.Name)(status);
        }

        mHaveCredentials = true;
    }

    SchannelServerFilter::SchannelServerFilter(
        RcfServer & server,
        DWORD enabledProtocols,
        ULONG contextRequirements) :
            SspiServerFilter(UNISP_NAME, RCF_T(""), BoolSchannel)
    {
        CertificatePtr certificatePtr = server.getCertificate();
        Win32CertificatePtr certificateBasePtr = boost::dynamic_pointer_cast<Win32Certificate>(certificatePtr);
        if (certificateBasePtr)
        {
            mLocalCertPtr = certificateBasePtr;
        }

        mCertValidationCallback = server.getCertificateValidationCallback();
        mAutoCertValidation = server.getEnableSchannelCertificateValidation();

        mContextRequirements = contextRequirements;
        mEnabledProtocols = enabledProtocols;
    }

    SchannelFilterFactory::SchannelFilterFactory(
        DWORD enabledProtocols,
        ULONG contextRequirements) :
            mContextRequirements(contextRequirements),
            mEnabledProtocols(enabledProtocols)/*,
            mEnableClientCertificateValidation(false)*/
    {
    }

    FilterPtr SchannelFilterFactory::createFilter(RcfServer & server)
    {
        boost::shared_ptr<SchannelServerFilter> filterPtr(
            new SchannelServerFilter(
                server,
                mEnabledProtocols,
                mContextRequirements));

        return filterPtr;
    }

    SchannelClientFilter::SchannelClientFilter(
        ClientStub *            pClientStub,
        DWORD                   enabledProtocols,
        ULONG                   contextRequirements) :
            SspiClientFilter(
                pClientStub,
                Encryption, 
                contextRequirements, 
                UNISP_NAME, 
                RCF_T(""),
                BoolSchannel)
    {
        mEnabledProtocols = enabledProtocols;

        CertificatePtr certificatePtr = pClientStub->getCertificate();
        Win32CertificatePtr certificateBasePtr = boost::dynamic_pointer_cast<Win32Certificate>(certificatePtr);
        if (certificateBasePtr)
        {
            mLocalCertPtr = certificateBasePtr;
        }

        mCertValidationCallback = pClientStub->getCertificateValidationCallback();
        mAutoCertValidation = pClientStub->getEnableSchannelCertificateValidation();
    }

    Win32CertificatePtr SspiFilter::getPeerCertificate()
    {
        return mRemoteCertPtr;
    }    

} // namespace RCF
