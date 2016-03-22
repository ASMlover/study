
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

#ifndef INCLUDE_RCF_SSPIFILTER_HPP
#define INCLUDE_RCF_SSPIFILTER_HPP

#include <memory>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Filter.hpp>
#include <RCF/RecursionLimiter.hpp>
#include <RCF/Export.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/RecursionLimiter.hpp>
#include <RCF/Tools.hpp>

#include <RCF/util/Tchar.hpp>

#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif

#include <security.h>
#include <WinCrypt.h>
#include <tchar.h>

namespace RCF {

    static const bool BoolClient = false;
    static const bool BoolServer = true;

    static const bool BoolSchannel = true;

    typedef RCF::tstring tstring;

    class SspiFilter;

    typedef boost::shared_ptr<SspiFilter> SspiFilterPtr;

    class RCF_EXPORT SspiImpersonator
    {
    public:
        SspiImpersonator(SspiFilterPtr sspiFilterPtr);
        SspiImpersonator(RcfSession & session);
        ~SspiImpersonator();

        bool impersonate();
        void revertToSelf() const;
    private:
        SspiFilterPtr mSspiFilterPtr;
    };

    static const ULONG DefaultSspiContextRequirements =
        ISC_REQ_REPLAY_DETECT   |
        ISC_REQ_SEQUENCE_DETECT |
        ISC_REQ_CONFIDENTIALITY |
        ISC_REQ_INTEGRITY       |
        ISC_REQ_DELEGATE        |
        ISC_REQ_MUTUAL_AUTH;

    class SchannelClientFilter;
    typedef SchannelClientFilter SchannelFilter;

    class SchannelFilterFactory;

    class Certificate;
    class Win32Certificate;
    typedef boost::shared_ptr<Win32Certificate> Win32CertificatePtr;
    
    class RCF_EXPORT SspiFilter : public Filter
    {
    public:

        ~SspiFilter();

        enum QualityOfProtection
        {
            None,
            Encryption,
            Integrity
        };

        QualityOfProtection getQop();

        typedef SspiImpersonator Impersonator;

        typedef boost::function<bool(Certificate *)> CertificateValidationCb;

        Win32CertificatePtr getPeerCertificate();

    protected:

        friend class SspiImpersonator;

        SspiFilter(
            ClientStub *            pClientStub,
            const tstring &         packageName,
            const tstring &         packageList,
            bool                    server,
            bool                    schannel);

        SspiFilter(
            ClientStub *            pClientStub,
            QualityOfProtection     qop,
            ULONG                   contextRequirements,
            const tstring &         packageName,
            const tstring &         packageList,
            bool                    server,
            bool                    schannel);

        SspiFilter(
            ClientStub *            pClientStub,
            QualityOfProtection     qop,
            ULONG                   contextRequirements,
            const tstring &         packageName,
            const tstring &         packageList,
            bool                    server);

        enum Event
        {
            ReadIssued,
            WriteIssued,
            ReadCompleted,
            WriteCompleted
        };

        enum ContextState
        {
            AuthContinue,
            AuthOk,
            AuthOkAck,
            AuthFailed
        };

        enum State
        {
            Ready,
            Reading,
            Writing
        };

        void            setupCredentials(
                            const tstring &userName,
                            const tstring &password,
                            const tstring &domain);

        void            setupCredentialsSchannel();

        void            acquireCredentials(
                            const tstring &userName = RCF_T(""),
                            const tstring &password = RCF_T(""),
                            const tstring &domain = RCF_T(""));

        void            freeCredentials();

        void            init();
        void            deinit();
        void            resetState();

        void            read(
                            const ByteBuffer &byteBuffer, 
                            std::size_t bytesRequested);

        void            write(const std::vector<ByteBuffer> &byteBuffers);

        void            onReadCompleted(const ByteBuffer &byteBuffer);
        void            onWriteCompleted(std::size_t bytesTransferred);

        void            handleEvent(Event event);
        void            readBuffer();
        void            writeBuffer();
        
        void            encryptWriteBuffer();
        bool            decryptReadBuffer();

        void            encryptWriteBufferSchannel();
        bool            decryptReadBufferSchannel();
        
        bool            completeReadBlock();
        bool            completeWriteBlock();
        bool            completeBlock();
        void            resumeUserIo();
        void            resizeReadBuffer(std::size_t newSize);
        void            resizeWriteBuffer(std::size_t newSize);

        void            shiftReadBuffer();
        void            trimReadBuffer();

        virtual void    handleHandshakeEvent() = 0;

    protected:

        ClientStub *                            mpClientStub;

        const tstring                           mPackageName;
        const tstring                           mPackageList;
        QualityOfProtection                     mQop;
        ULONG                                   mContextRequirements;

        bool                                    mHaveContext;
        bool                                    mHaveCredentials;
        bool                                    mImplicitCredentials;
        SecPkgInfo                              mPkgInfo;
        CtxtHandle                              mContext;
        CredHandle                              mCredentials;

        ContextState                            mContextState;
        State                                   mPreState;
        State                                   mPostState;
        Event                                   mEvent;
        const bool                              mServer;

        ByteBuffer                              mReadByteBufferOrig;
        ByteBuffer                              mWriteByteBufferOrig;
        std::size_t                             mBytesRequestedOrig;

        ByteBuffer                              mReadByteBuffer;
        ReallocBufferPtr                        mReadBufferVectorPtr;
        char *                                  mReadBuffer;
        std::size_t                             mReadBufferPos;
        std::size_t                             mReadBufferLen;

        ByteBuffer                              mWriteByteBuffer;
        ReallocBufferPtr                        mWriteBufferVectorPtr;
        char *                                  mWriteBuffer;
        std::size_t                             mWriteBufferPos;
        std::size_t                             mWriteBufferLen;

        std::vector<ByteBuffer>                 mByteBuffers;
        ByteBuffer                              mTempByteBuffer;

        const bool                              mSchannel;

        std::size_t                             mMaxMessageLength;
 
        // Schannel-specific members.
        Win32CertificatePtr                     mLocalCertPtr;
        Win32CertificatePtr                     mRemoteCertPtr;
        CertificateValidationCb                 mCertValidationCallback;
        DWORD                                   mEnabledProtocols;
        tstring                                 mAutoCertValidation;
        const std::size_t                       mReadAheadChunkSize;
        std::size_t                             mRemainingDataPos;

        std::vector<RCF::ByteBuffer>            mMergeBufferList;
        std::vector<char>                       mMergeBuffer;

        bool                                    mProtocolChecked;

    private:
        bool                                    mLimitRecursion;
        RecursionState<ByteBuffer, int>         mRecursionStateRead;
        RecursionState<std::size_t, int>        mRecursionStateWrite;

        void onReadCompleted_(const ByteBuffer &byteBuffer);
        void onWriteCompleted_(std::size_t bytesTransferred);

        friend class SchannelFilterFactory;
    };

    // server filters

    class RCF_EXPORT SspiServerFilter : public SspiFilter
    {
    public:
        SspiServerFilter(
            const tstring &packageName, 
            const tstring &packageList,
            bool schannel = false);

    private:
        bool doHandshakeSchannel();
        bool doHandshake();
        void handleHandshakeEvent();
    };

    class NtlmServerFilter : public SspiServerFilter
    {
    public:
        NtlmServerFilter();
        int getFilterId() const;
    };

    class KerberosServerFilter : public SspiServerFilter
    {
    public:
        KerberosServerFilter();
        int getFilterId() const;
    };

    class NegotiateServerFilter : public SspiServerFilter
    {
    public:
        NegotiateServerFilter(const tstring &packageList);
        int getFilterId() const;
    };

    // filter factories

    class NtlmFilterFactory : public FilterFactory
    {
    public:
        FilterPtr createFilter(RcfServer & server);
        int getFilterId();
    };

    class KerberosFilterFactory : public FilterFactory
    {
    public:
        FilterPtr createFilter(RcfServer & server);
        int getFilterId();
    };

    class NegotiateFilterFactory : public FilterFactory
    {
    public:
        NegotiateFilterFactory(const tstring &packageList = RCF_T("Kerberos, NTLM"));
        FilterPtr createFilter(RcfServer & server);
        int getFilterId();
    private:
        tstring mPackageList;
    };

    // client filters

    class SspiClientFilter : public SspiFilter
    {
    public:
        SspiClientFilter(
            ClientStub *            pClientStub,
            QualityOfProtection     qop,
            ULONG                   contextRequirements,
            const tstring &         packageName,
            const tstring &         packageList) :
                SspiFilter(
                    pClientStub,
                    qop, 
                    contextRequirements, 
                    packageName, 
                    packageList,
                    BoolClient)
        {}

        SspiClientFilter(
            ClientStub *            pClientStub,
            QualityOfProtection     qop,
            ULONG                   contextRequirements,
            const tstring &         packageName,
            const tstring &         packageList,
            bool                    schannel) :
                SspiFilter(
                    pClientStub,
                    qop, 
                    contextRequirements, 
                    packageName, 
                    packageList,
                    BoolClient,
                    schannel)
        {}

    private:
        bool doHandshakeSchannel();
        bool doHandshake();
        void handleHandshakeEvent();
    };

    class NtlmClientFilter : public SspiClientFilter
    {
    public:
        NtlmClientFilter(
            ClientStub *            pClientStub,
            QualityOfProtection     qop = SspiFilter::Encryption,
            ULONG                   contextRequirements 
                                        = DefaultSspiContextRequirements);

        int getFilterId() const;
    };

    class KerberosClientFilter : public SspiClientFilter
    {
    public:
        KerberosClientFilter(
            ClientStub *            pClientStub,
            QualityOfProtection     qop = SspiFilter::Encryption,
            ULONG                   contextRequirements 
                                        = DefaultSspiContextRequirements);

        int getFilterId() const;
    };

    class NegotiateClientFilter : public SspiClientFilter
    {
    public:
        NegotiateClientFilter(
            ClientStub *            pClientStub,
            QualityOfProtection     qop = SspiFilter::None,
            ULONG                   contextRequirements 
                                        = DefaultSspiContextRequirements);


        int getFilterId() const;
    };

    typedef NtlmClientFilter            NtlmFilter;
    typedef KerberosClientFilter        KerberosFilter;
    typedef NegotiateClientFilter       NegotiateFilter;
    

    // These SSPI-prefixed typedefs make us compatible with code written for RCF 1.0.
    typedef NtlmFilter                  SspiNtlmFilter;
    typedef KerberosFilter              SspiKerberosFilter;
    typedef NegotiateFilter             SspiNegotiateFilter;

    typedef NtlmServerFilter            SspiNtlmServerFilter;
    typedef KerberosServerFilter        SspiKerberosServerFilter;
    typedef NegotiateServerFilter       SspiNegotiateServerFilter;
    typedef NtlmFilterFactory           SspiNtlmFilterFactory;
    typedef KerberosFilterFactory       SspiKerberosFilterFactory;
    typedef NegotiateFilterFactory      SspiNegotiateFilterFactory;
    typedef NtlmClientFilter            SspiNtlmClientFilter;
    typedef KerberosClientFilter        SspiKerberosClientFilter;
    typedef NegotiateClientFilter       SspiNegotiateClientFilter;

    typedef SspiFilter                  SspiFilterBase;
    typedef SspiFilterPtr               SspiFilterBasePtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_SSPIFILTER_HPP
