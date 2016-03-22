
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

#include <RCF/ClientStub.hpp>
#include <RCF/Config.hpp>
#include <RCF/RcfServer.hpp>

#if RCF_FEATURE_LEGACY==0

namespace RCF {

    void ClientStub::requestTransportFilters_Legacy(const std::vector<FilterPtr> &filters)
    {
        RCF_UNUSED_VARIABLE(filters);

        RCF_THROW( Exception( 
            _RcfError_NotSupportedInThisBuild("Legacy transport filter request") ) );
    }

    void ClientStub::requestTransportFiltersAsync_Legacy(
        const std::vector<FilterPtr> &filters,
        boost::function0<void> onCompletion)
    {
        RCF_UNUSED_VARIABLE(filters);
        RCF_UNUSED_VARIABLE(onCompletion);

        std::auto_ptr<Exception> ePtr( new Exception(
            _RcfError_NotSupportedInThisBuild("Legacy transport filter request")) );

        setAsyncException(ePtr);
    }

    void createCallbackConnectionImpl_Legacy(
        ClientStub & clientStubOrig, 
        ServerTransport & callbackServer)
    {
        RCF_UNUSED_VARIABLE(clientStubOrig);
        RCF_UNUSED_VARIABLE(callbackServer);

        RCF_THROW( Exception( 
            _RcfError_NotSupportedInThisBuild("Legacy callback connection request") ) );
    }

    StubEntryPtr RcfServer::findStubEntryForToken(const Token & token)
    {
        RCF_UNUSED_VARIABLE(token);

        return StubEntryPtr();
    }

} // namespace RCF

#else

#include <RCF/ServerInterfaces.hpp>
#include <RCF/ObjectFactoryService.hpp>

namespace RCF {

    void ClientStub::requestTransportFilters_Legacy(const std::vector<FilterPtr> &filters)
    {
        // TODO: the current message filter sequence is not being used,
        // when making the filter request call to the server.

        using namespace boost::multi_index::detail; // for scope_guard

        std::vector<boost::int32_t> filterIds;

        for (std::size_t i=0; i<filters.size(); ++i)
        {
            filterIds.push_back( filters[i]->getFilterId());
        }

        if (!isConnected())
        {
            connect();
        }
        RCF::RcfClient<RCF::I_RequestTransportFilters> client(*this);
        client.getClientStub().setTransport( releaseTransport());
        client.getClientStub().setTargetToken( Token());

        RestoreClientTransportGuard guard(*this, client.getClientStub());
        RCF_UNUSED_VARIABLE(guard);

        client.getClientStub().setRemoteCallTimeoutMs( getRemoteCallTimeoutMs() );
        int ret = client.RequestTransportFilters(RCF::Twoway, filterIds);
        RCF_VERIFY(ret == RcfError_Ok, RemoteException( Error(ret) ))(filterIds);

        for (std::size_t i=0; i<filters.size(); ++i)
        {
            filters[i]->resetState();
        }

        client.getClientStub().getTransport().setTransportFilters(filters);
    }

    class Handler
    {
    public:

        virtual ~Handler()
        {
        }

        void handle(
            Future<boost::int32_t>      fRet,
            I_RcfClient &               rcfClient,
            ClientStub &                clientStubOrig,
            boost::function0<void>      onCompletion)
        {
            ClientStubPtr clientStubPtr = 
                rcfClient.getClientStub().shared_from_this();

            ClientStubPtr clientStubOrigPtr = clientStubOrig.shared_from_this();

            clientStubOrigPtr->setTransport( 
                clientStubPtr->releaseTransport() );

            clientStubOrigPtr->setSubRcfClientPtr( RcfClientPtr() );

            std::auto_ptr<Exception> ape(clientStubPtr->getAsyncException());

            bool failed = (ape.get() != NULL);

            clientStubOrigPtr->setAsyncException(ape);

            if (failed)
            {
                onCompletion();
            }
            else
            {
                mClientStubPtr = clientStubOrigPtr;

                boost::int32_t ret = fRet;
                if (ret == RcfError_Ok)
                {
                    handleOk();
                    onCompletion();
                }
                else
                {
                    std::auto_ptr<Exception> apException(
                        new RemoteException( Error(ret) ));

                    clientStubOrigPtr->setAsyncException(apException);

                    handleFail();

                    onCompletion();
                }
            }
        }

        virtual void handleOk()
        {
        }

        virtual void handleFail()
        {
        }

    protected:
        ClientStubPtr mClientStubPtr;
    };

    typedef boost::shared_ptr<Handler> HandlerPtr;

    class RequestTransportFiltersHandler : public Handler
    {
    public :
        RequestTransportFiltersHandler(
            boost::shared_ptr< std::vector<FilterPtr> > filtersPtr) :
        mFiltersPtr(filtersPtr)
        {
        }

    private:
        void handleOk()
        {
            for (std::size_t i=0; i<mFiltersPtr->size(); ++i)
            {
                (*mFiltersPtr)[i]->resetState();
            }
            mClientStubPtr->getTransport().setTransportFilters(*mFiltersPtr);
        }

        boost::shared_ptr< std::vector<FilterPtr> > mFiltersPtr;
    };

    void ClientStub::requestTransportFiltersAsync_Legacy(
        const std::vector<FilterPtr> &filters,
        boost::function0<void> onCompletion)
    {
        std::vector<boost::int32_t> filterIds;

        for (std::size_t i=0; i<filters.size(); ++i)
        {
            filterIds.push_back( filters[i]->getFilterId() );
        }

        boost::shared_ptr<std::vector<FilterPtr> > filtersPtr(
            new std::vector<FilterPtr>(filters) );

        typedef RcfClient<I_RequestTransportFilters> RtfClient;
        typedef boost::shared_ptr<RtfClient> RtfClientPtr;

        RtfClientPtr rtfClientPtr( new RtfClient(*this) );

        rtfClientPtr->getClientStub().setTransport( releaseTransport());
        rtfClientPtr->getClientStub().setTargetToken( Token());

        setSubRcfClientPtr(rtfClientPtr);

        setAsync(true);

        Future<boost::int32_t> fRet;

        HandlerPtr handlerPtr( new RequestTransportFiltersHandler(filtersPtr));

        fRet = rtfClientPtr->RequestTransportFilters(

            RCF::AsyncTwoway( boost::bind(
            &Handler::handle, 
            handlerPtr,
            fRet,
            boost::ref(*rtfClientPtr),
            boost::ref(*this),
            onCompletion)),

            filterIds);
    }

    void createCallbackConnectionImpl_Legacy(
        ClientStub & clientStubOrig, 
        ServerTransport & callbackServer)
    {
        RcfClient<I_CreateCallbackConnection> client(clientStubOrig);
        client.getClientStub().setTransport( clientStubOrig.releaseTransport() );
        client.CreateCallbackConnection();
        convertRcfClientToRcfSession(client.getClientStub(), callbackServer);
    }

    //**************************************************************************
    // Synchronous create object calls.

    namespace {

        void reinstateClientTransport(
            ClientStub &clientStub,
            I_RcfClient &factory)
        {
            clientStub.setTransport(factory.getClientStub().releaseTransport());
        }

    }

    void ClientStub::createRemoteObject(
        const std::string &objectName_)
    {
        const std::string &objectName = objectName_.empty() ? mInterfaceName : objectName_;
        unsigned int timeoutMs = getRemoteCallTimeoutMs();
        connect();
        RcfClient<I_ObjectFactory> factory(*this);
        factory.getClientStub().setTransport( releaseTransport());
        factory.getClientStub().setTargetToken( Token());
        // TODO: should only be using the remainder of the timeout
        factory.getClientStub().setRemoteCallTimeoutMs(timeoutMs);
        using namespace boost::multi_index::detail;
        scope_guard guard = make_guard(
            reinstateClientTransport,
            boost::ref(*this),
            boost::ref(factory));
        RCF_UNUSED_VARIABLE(guard);
        RCF::Token token;
        boost::int32_t ret = factory.CreateObject(RCF::Twoway, objectName, token);
        if (ret == RcfError_Ok)
        {
            setTargetToken(token);
        }
        else
        {
            setTargetToken(Token());

            Error err(ret);
            RemoteException e(err);
            RCF_THROW(e);
        }
    }

    // ObjectFactoryClient is an abstraction of RcfClient<I_ObjectFactoryService>,
    // and RcfClient<I_SessionObjectFactoryService>. We need to use either one,
    // depending on what the RCF runtime version is.

    class ObjectFactoryClient
    {
    public:
        ObjectFactoryClient(ClientStub & clientStub) :
            mRuntimeVersion(clientStub.getRuntimeVersion()),
            mCutoffVersion(2)
        {
            mRuntimeVersion <= mCutoffVersion ?
                client1.reset( new RcfClient<I_ObjectFactory>(clientStub)) :
                client2.reset( new RcfClient<I_SessionObjectFactory>(clientStub));
        }

        ClientStub &getClientStub()
        {
            return mRuntimeVersion <= mCutoffVersion ?
                client1->getClientStub() :
                client2->getClientStub();
        }

        RcfClientPtr getRcfClientPtr()
        {
            return mRuntimeVersion <= mCutoffVersion ?
                RcfClientPtr(client1) :
                RcfClientPtr(client2);
        }

        FutureImpl<boost::int32_t> CreateSessionObject(
            const ::RCF::CallOptions &callOptions,
            const std::string & objectName)
        {
            return mRuntimeVersion <= mCutoffVersion ?
                client1->CreateSessionObject(callOptions, objectName) :
                client2->CreateSessionObject(callOptions, objectName);
        }

        FutureImpl<boost::int32_t> DeleteSessionObject(
            const ::RCF::CallOptions &callOptions)
        {
            return mRuntimeVersion <= mCutoffVersion ?
                client1->DeleteSessionObject(callOptions) :
                client2->DeleteSessionObject(callOptions);
        }

        void reinstateClientTransport(ClientStub & clientStub)
        {
            ClientTransportAutoPtr clientTransportAutoPtr = 
                mRuntimeVersion <= mCutoffVersion ?
                    client1->getClientStub().releaseTransport() :
                    client2->getClientStub().releaseTransport();

            clientStub.setTransport(clientTransportAutoPtr);
        }

    private:
        boost::shared_ptr<RcfClient<I_ObjectFactory> >          client1;
        boost::shared_ptr<RcfClient<I_SessionObjectFactory> >   client2;

        const int                                               mRuntimeVersion;
        const int                                               mCutoffVersion;
    };

    void ClientStub::createRemoteSessionObject(
        const std::string &objectName_)
    {
        const std::string &objectName = objectName_.empty() ? mInterfaceName : objectName_;
        unsigned int timeoutMs = getRemoteCallTimeoutMs();
        
        ObjectFactoryClient factory(*this);
        
        factory.getClientStub().setTransport( releaseTransport());
        factory.getClientStub().setTargetToken( Token());
        // TODO: should only be using the remainder of the timeout
        factory.getClientStub().setRemoteCallTimeoutMs(timeoutMs);

        factory.getClientStub().connect();

        using namespace boost::multi_index::detail;
        scope_guard guard = make_obj_guard(
            factory,
            &ObjectFactoryClient::reinstateClientTransport,
            boost::ref(*this));
        RCF_UNUSED_VARIABLE(guard);

        boost::int32_t ret = factory.CreateSessionObject(RCF::Twoway, objectName);
        if (ret == RcfError_Ok)
        {
            setTargetName("");
            setTargetToken(Token());
        }
        else
        {
            Error err(ret);
            RemoteException e(err);
            RCF_THROW(e);
        }
    }

    void ClientStub::deleteRemoteSessionObject()
    {
        ObjectFactoryClient factory(*this);
        factory.getClientStub().setTransport( releaseTransport());
        factory.getClientStub().setTargetToken( Token());

        using namespace boost::multi_index::detail;
        scope_guard guard = make_obj_guard(
            factory,
            &ObjectFactoryClient::reinstateClientTransport,
            boost::ref(*this));
        RCF_UNUSED_VARIABLE(guard);

        boost::int32_t ret = factory.DeleteSessionObject(RCF::Twoway);
        RCF_VERIFY(ret == RcfError_Ok, RCF::RemoteException( Error(ret) ));
    }

    void ClientStub::deleteRemoteObject()
    {
        Token token = getTargetToken();
        if (token != Token())
        {
            RcfClient<I_ObjectFactory> factory(*this);
            factory.getClientStub().setTransport( releaseTransport());
            factory.getClientStub().setTargetToken( Token());
            using namespace boost::multi_index::detail;
            scope_guard guard = make_guard(
                reinstateClientTransport,
                boost::ref(*this),
                boost::ref(factory));
            RCF_UNUSED_VARIABLE(guard);

            boost::int32_t ret = factory.DeleteObject(RCF::Twoway, token);
            RCF_VERIFY(ret == RcfError_Ok, RCF::RemoteException( Error(ret) ));
        }
    }

    StubEntryPtr RcfServer::findStubEntryForToken(const Token & token)
    {
        ObjectFactoryServicePtr ofsPtr = getObjectFactoryServicePtr();
        RCF_ASSERT(ofsPtr);
        StubEntryPtr stubEntryPtr = ofsPtr->getStubEntryPtr(token);
        return stubEntryPtr;
    }

} // namespace RCF

#endif
