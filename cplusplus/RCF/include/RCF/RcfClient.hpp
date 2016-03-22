
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

#ifndef INCLUDE_RCF_RCFCLIENT_HPP
#define INCLUDE_RCF_RCFCLIENT_HPP

#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/bool_fwd.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility/enable_if.hpp>

#include <RCF/CheckRtti.hpp>
#include <RCF/ClientStub.hpp>
#include <RCF/Export.hpp>

namespace RCF {

    class ClientStub;
    class ServerBinding;
    class RcfSession;

    typedef boost::shared_ptr<ClientStub> ClientStubPtr;
    typedef boost::shared_ptr<ServerBinding> ServerBindingPtr;

    typedef boost::function2<
        void,
        int,
        RcfSession &> InvokeFunctor;

    typedef std::map<std::string,  InvokeFunctor> InvokeFunctorMap;

    RCF_EXPORT void setCurrentCallDesc(std::string& desc, RCF::MethodInvocationRequest& request, const char * szFunc, const char * szArity);

    // Base class of all RcfClient<> templates.
    class RCF_EXPORT I_RcfClient
    {
    public:

        virtual ~I_RcfClient();

        I_RcfClient(const std::string & interfaceName);

        I_RcfClient(
            const std::string &     interfaceName, 
            ServerBindingPtr        serverStubPtr);

        I_RcfClient(
            const std::string &     interfaceName, 
            const Endpoint &        endpoint, 
            const std::string &     targetName_ = "");

        I_RcfClient(
            const std::string &     interfaceName, 
            ClientTransportAutoPtr  clientTransportAutoPtr, 
            const std::string &     targetName_ = "");

        I_RcfClient(
            const std::string &     interfaceName, 
            const ClientStub &      clientStub, 
            const std::string &     targetName_ = "");

        I_RcfClient(
            const std::string &     interfaceName, 
            const I_RcfClient &     rhs);

        I_RcfClient & operator=(const I_RcfClient & rhs);

        void                swap(I_RcfClient & rhs);

        void                setClientStubPtr(ClientStubPtr clientStubPtr);

        ClientStub &        getClientStub();
        const ClientStub &  getClientStub() const;
        ClientStubPtr       getClientStubPtr() const;
        ServerBindingPtr    getServerStubPtr() const;
        ServerBinding &     getServerStub();

    protected:

        ClientStubPtr                   mClientStubPtr;
        ServerBindingPtr                mServerStubPtr;
        std::string                     mInterfaceName;

        typedef Void                    V;
    };

    typedef boost::shared_ptr<I_RcfClient> RcfClientPtr;

    // some meta-programming functionality needed by the macros in IDL.hpp

    typedef char (&yes_type)[1];
    typedef char (&no_type)[2];

    template<typename U> static yes_type RCF_hasRcfClientTypedef(typename U::RcfClientT *);
    template<typename U> static no_type RCF_hasRcfClientTypedef(...);

    template<typename T>
    struct GetRcfClient
    {
        typedef typename T::RcfClientT type;
    };

    template<typename T>
    struct Identity
    {
        typedef T type;
    };

    template<typename T>
    struct GetInterface
    {
        // tried eval_if here, but got some weird errors with vc71
        typedef typename boost::mpl::if_c<
            sizeof(yes_type) == sizeof(RCF_hasRcfClientTypedef<T>(0)),
            GetRcfClient<T>,
            Identity<T> >::type type0;

        typedef typename type0::type type;
    };

    class default_ { char a[1]; };
    class defined_ { char a[2]; };
    template<typename T> class Dummy {};

} // namespace RCF

#endif // ! INCLUDE_RCF_RCFCLIENT_HPP
