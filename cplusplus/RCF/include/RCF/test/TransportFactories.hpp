
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

#ifndef INCLUDE_RCF_TEST_TRANSPORTFACTORIES_HPP
#define INCLUDE_RCF_TEST_TRANSPORTFACTORIES_HPP

#include <RCF/RcfClient.hpp>
#include <RCF/ServerTransport.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/IpAddress.hpp>

template<typename Interface>
inline bool tryCreateRemoteObject(
    RCF::I_RcfClient &rcfClient,
    std::string objectName = "")
{
    try
    {
        rcfClient.getClientStub().createRemoteObject(objectName);
        return true;
    }
    catch (const RCF::Exception &e)
    {
        RCF_LOG_1()(e);
        return false;
    }
}


namespace RCF {

    typedef boost::shared_ptr<ClientTransportAutoPtr> ClientTransportAutoPtrPtr;

    typedef std::pair<ServerTransportPtr, ClientTransportAutoPtrPtr> TransportPair;

    class I_TransportFactory
    {
    public:
        virtual ~I_TransportFactory() {}
        virtual TransportPair createTransports() = 0;
        virtual TransportPair createNonListeningTransports() = 0;
        virtual bool isConnectionOriented() = 0;
        virtual bool supportsTransportFilters() = 0;
        virtual std::string desc() = 0;
    };

    typedef boost::shared_ptr<I_TransportFactory> TransportFactoryPtr;

    typedef std::vector<TransportFactoryPtr> TransportFactories;

    TransportFactories &getTransportFactories();

    TransportFactories &getIpTransportFactories();

    //**************************************************
    // transport factories

#if RCF_FEATURE_NAMEDPIPE==1

    class Win32NamedPipeTransportFactory : public I_TransportFactory
    {
    public:
        TransportPair createTransports();
        TransportPair createNonListeningTransports();
        bool isConnectionOriented();
        bool supportsTransportFilters();
        std::string desc();
    };

#endif

#if RCF_FEATURE_TCP==1

    class TcpTransportFactory : public I_TransportFactory
    {
    public:
        
        TcpTransportFactory(IpAddress::Type type = IpAddress::V4);
        TransportPair createTransports();
        TransportPair createNonListeningTransports();
        bool isConnectionOriented();
        bool supportsTransportFilters();
        std::string desc();

    private:

        std::string mLoopback;
    };

#endif

#if RCF_FEATURE_HTTP==1

    class HttpTransportFactory : public I_TransportFactory
    {
    public:

        HttpTransportFactory();
        TransportPair createTransports();
        TransportPair createNonListeningTransports();
        bool isConnectionOriented();
        bool supportsTransportFilters();
        std::string desc();

    private:
        std::string mLoopback;
    };

    class HttpsTransportFactory : public I_TransportFactory
    {
    public:

        HttpsTransportFactory();
        TransportPair createTransports();
        TransportPair createNonListeningTransports();
        bool isConnectionOriented();
        bool supportsTransportFilters();
        std::string desc();

    private:
        std::string mLoopback;
    };


#endif

#if RCF_FEATURE_LOCALSOCKET==1

    class UnixLocalTransportFactory : public I_TransportFactory
    {
    public:

        UnixLocalTransportFactory();

    private:

        TransportPair createTransports();
        TransportPair createNonListeningTransports();
        bool isConnectionOriented();
        bool supportsTransportFilters();

    private:

        bool fileExists(const std::string & path);
        std::string generateNewPipeName();
        std::string desc();

        int mIndex;

    };

#endif // RCF_HAS_LOCAL_SOCKETS

#if RCF_FEATURE_UDP==1

    class UdpTransportFactory : public I_TransportFactory
    {
    public:

        UdpTransportFactory(IpAddress::Type type = IpAddress::V4);
        TransportPair createTransports();
        TransportPair createNonListeningTransports();
        bool isConnectionOriented();
        bool supportsTransportFilters();
        std::string desc();

    private:

        std::string mLoopback;
    };

#endif

    void initializeTransportFactories();
    
} // namespace RCF

#endif // ! INCLUDE_RCF_TEST_TRANSPORTFACTORIES_HPP
