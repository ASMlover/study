
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

#include <RCF/RcfClient.hpp>

namespace RCF {

    I_RcfClient::~I_RcfClient()
    {}

    I_RcfClient::I_RcfClient(const std::string & interfaceName)
    {
        mInterfaceName = interfaceName;
    }

    I_RcfClient::I_RcfClient(
        const std::string &     interfaceName, 
        ServerBindingPtr        serverStubPtr)
    {
        mInterfaceName = interfaceName;
        mServerStubPtr = serverStubPtr;
    }

    I_RcfClient::I_RcfClient(
        const std::string &     interfaceName, 
        const Endpoint &        endpoint, 
        const std::string &     targetName_)
    {
        mInterfaceName = interfaceName;
        std::string targetName = targetName_;
        if (targetName.empty())
        {
            targetName = mInterfaceName;
        }
        ClientStubPtr clientStubPtr( new ClientStub(mInterfaceName, targetName) );
        clientStubPtr->setEndpoint(endpoint);
        setClientStubPtr(clientStubPtr);
    }

    I_RcfClient::I_RcfClient(
        const std::string &     interfaceName, 
        ClientTransportAutoPtr  clientTransportAutoPtr, 
        const std::string &     targetName_)
    {
        mInterfaceName = interfaceName;
        std::string targetName = targetName_;
        if (targetName.empty())
        {
            targetName = mInterfaceName;
        }
        ClientStubPtr clientStubPtr( new ClientStub(mInterfaceName, targetName) );
        clientStubPtr->setTransport(clientTransportAutoPtr);
        setClientStubPtr(clientStubPtr);
    }

    I_RcfClient::I_RcfClient(
        const std::string &     interfaceName, 
        const ClientStub &      clientStub, 
        const std::string &     targetName_)
    {
        mInterfaceName = interfaceName;
        std::string targetName = targetName_;
        if (targetName.empty())
        {
            targetName = mInterfaceName;
        }
        ClientStubPtr clientStubPtr( new ClientStub(clientStub) );
        clientStubPtr->setInterfaceName(mInterfaceName);
        clientStubPtr->setTargetName(targetName);
        clientStubPtr->setTargetToken(Token());
        setClientStubPtr(clientStubPtr);
    }

    I_RcfClient::I_RcfClient(const std::string & interfaceName, const I_RcfClient & rhs)
    {
        mInterfaceName = interfaceName;
        if (rhs.getClientStubPtr())
        {
            const std::string & targetName = mInterfaceName;
            ClientStubPtr clientStubPtr( new ClientStub(rhs.getClientStub()));
            clientStubPtr->setInterfaceName(mInterfaceName);
            clientStubPtr->setTargetName(targetName);
            clientStubPtr->setTargetToken(Token());
            setClientStubPtr(clientStubPtr);
        }
    }

    I_RcfClient & I_RcfClient::operator=(const I_RcfClient & rhs)
    {
        if (&rhs != this)
        {
            if (rhs.mClientStubPtr)
            {
                ClientStubPtr clientStubPtr( new ClientStub(rhs.getClientStub()));
                setClientStubPtr(clientStubPtr);
            }
            else
            {
                RCF_ASSERT(!rhs.mServerStubPtr);
                mClientStubPtr = rhs.mClientStubPtr;
            }
        }
        return *this;
    }

    void I_RcfClient::swap(I_RcfClient & rhs)
    {
        ClientStubPtr clientStubPtr = rhs.mClientStubPtr;
        ServerBindingPtr serverStubPtr = rhs.mServerStubPtr;

        rhs.mClientStubPtr = mClientStubPtr;
        rhs.mServerStubPtr = mServerStubPtr;

        mClientStubPtr = clientStubPtr;
        mServerStubPtr = serverStubPtr;
    }

    void I_RcfClient::setClientStubPtr(ClientStubPtr clientStubPtr)
    {
        mClientStubPtr = clientStubPtr;
    }

    ClientStub & I_RcfClient::getClientStub()
    {
        return *mClientStubPtr;
    }

    const ClientStub & I_RcfClient::getClientStub() const
    {
        return *mClientStubPtr;
    }

    ClientStubPtr I_RcfClient::getClientStubPtr() const
    {
        return mClientStubPtr;
    }

    ServerBindingPtr I_RcfClient::getServerStubPtr() const
    {
        return mServerStubPtr;
    }

    ServerBinding & I_RcfClient::getServerStub()
    {
        return *mServerStubPtr;
    }

} // namespace RCF
