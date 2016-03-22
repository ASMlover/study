
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

#include <RCF/Future.hpp>

#include <RCF/OverlappedAmi.hpp>

namespace RCF {

    FutureImplBase::FutureImplBase(
        ClientStub &clientStub, 
        const std::string & interfaceName,
        int fnId,
        RemoteCallSemantics rcs,
        const char * szFunc,
        const char * szArity) :
            mpClientStub(&clientStub),
            mFnId(fnId),
            mRcs(rcs),
            mSzFunc(szFunc),
            mSzArity(szArity),
            mOwn(true)
    {
        // TODO: put this in the initializer list instead?
        clientStub.init(interfaceName, fnId, rcs);
    }

    FutureImplBase::FutureImplBase(const FutureImplBase& rhs) :
        mpClientStub(rhs.mpClientStub),
        mFnId(rhs.mFnId),
        mRcs(rhs.mRcs),
        mSzFunc(rhs.mSzFunc),
        mSzArity(rhs.mSzArity),
        mOwn(rhs.mOwn)
    {
        rhs.mOwn = false;
    }

    FutureImplBase & FutureImplBase::operator=(const FutureImplBase &rhs)
    {
        mpClientStub = rhs.mpClientStub;
        mFnId = rhs.mFnId;
        mRcs = rhs.mRcs;
        mSzFunc = rhs.mSzFunc;
        mSzArity = rhs.mSzArity;

        mOwn = rhs.mOwn;
        rhs.mOwn = false;
        return *this;
    }

    void FutureImplBase::call() const
    {

#if RCF_FEATURE_FILETRANSFER==1

        // File uploads are done before the call itself.
        mpClientStub->processUploadStreams();

#endif

        // TODO
        bool async = mpClientStub->getAsync();

        mpClientStub->setTries(0);

        setCurrentCallDesc(mpClientStub->mCurrentCallDesc, mpClientStub->mRequest, mSzFunc, mSzArity);

        if (async)
        {
            callAsync();
        }
        else
        {
            callSync();
        }
    }

    void FutureImplBase::callSync() const
    {
        // ClientStub::onConnectCompleted() uses the contents of mEncodedByteBuffers
        // to determine what stage the current call is in. So mEncodedByteBuffers
        // needs to be cleared after a remote call, even if an exception is thrown.

        // Error handling code here will generally also need to be present in 
        // ClientStub::onError().

        LogEntryExit logEntryExit(*mpClientStub);

        RCF_LOG_3()(mpClientStub)(mpClientStub->mRequest) 
            << "RcfClient - sending synchronous request.";

        try
        {
            mpClientStub->call(mRcs);
        }
        catch(const RCF::RemoteException & e)
        {
            mpClientStub->mEncodedByteBuffers.resize(0);
            if (shouldDisconnectOnRemoteError( e.getError() ))
            {
                mpClientStub->disconnect();
            }
            throw; 
        }
        catch(const RCF::Exception &)
        {
            mpClientStub->mEncodedByteBuffers.resize(0);
            mpClientStub->disconnect();
            throw;
        }
        catch(...)
        {
            mpClientStub->mEncodedByteBuffers.resize(0);
            mpClientStub->disconnect();
            throw;
        }
    }

    void FutureImplBase::callAsync() const
    {
        LogEntryExit logEntryExit(*mpClientStub);

        RCF_LOG_3()(mpClientStub)(mpClientStub->mRequest) 
            << "RcfClient - sending asynchronous request.";

        std::auto_ptr<RCF::Exception> ape;

        try
        {
            mpClientStub->call(mRcs);
        }
        catch(const RCF::Exception & e)
        {
            ape.reset( e.clone().release() );
        }
        catch(...)
        {
            ape.reset( new Exception(_RcfError_NonStdException()) );
        }

        if (ape.get())
        {
            mpClientStub->onError(*ape);
        }

        getTlsAmiNotification().run();
    }

} // namespace RCF
