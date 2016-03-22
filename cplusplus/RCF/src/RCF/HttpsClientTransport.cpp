
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

#include <RCF/HttpsClientTransport.hpp>

#if RCF_FEATURE_OPENSSL==1
#include <RCF/OpenSslEncryptionFilter.hpp>
#endif

#if RCF_FEATURE_SSPI==1
#include <RCF/Schannel.hpp>
#endif

#include <RCF/ClientStub.hpp>
#include <RCF/HttpConnectFilter.hpp>
#include <RCF/HttpFrameFilter.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    HttpsClientTransport::HttpsClientTransport(const HttpsEndpoint & httpsEndpoint) : 
        TcpClientTransport(httpsEndpoint.getIp(), httpsEndpoint.getPort())
    {
        std::vector<FilterPtr> wireFilters;

        // HTTP framing.
        wireFilters.push_back( FilterPtr( new HttpFrameFilter(
            getRemoteAddr().getIp(), 
            getRemoteAddr().getPort())));

        // SSL.
        ClientStub * pClientStub = getTlsClientStubPtr();
        RCF_ASSERT(pClientStub);

        FilterPtr sslFilterPtr;

#if RCF_FEATURE_SSPI==1 && RCF_FEATURE_OPENSSL==1

        if (pClientStub->getSslImplementation() == Si_Schannel)
        {
            sslFilterPtr.reset( new SchannelFilter(pClientStub) );
        }
        else
        {
            RCF_ASSERT(pClientStub->getSslImplementation() == Si_OpenSsl);
            sslFilterPtr.reset( new OpenSslEncryptionFilter(pClientStub) );
        }

#elif RCF_FEATURE_SSPI==1

        sslFilterPtr.reset( new SchannelFilter(pClientStub) );

#elif RCF_FEATURE_OPENSSL==1

        sslFilterPtr.reset( new OpenSslEncryptionFilter(pClientStub) );

#endif

        if (!sslFilterPtr)
        {
            RCF_THROW( Exception(_RcfError_SslNotSupported()) );
        }

        wireFilters.push_back(sslFilterPtr);

        // HTTP CONNECT filter for passing through a proxy.
        wireFilters.push_back( FilterPtr( new HttpConnectFilter(
            getRemoteAddr().getIp(), 
            getRemoteAddr().getPort())));

        setWireFilters(wireFilters);
    }

    TransportType HttpsClientTransport::getTransportType()
    {
        return Tt_Https;
    }

} // namespace RCF
