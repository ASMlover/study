
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

#include <RCF/FilterService.hpp>

#include <boost/bind.hpp>

#include <RCF/CurrentSession.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/RcfSession.hpp>

#if RCF_FEATURE_LEGACY==1
#include <RCF/ServerInterfaces.hpp>
#endif

namespace RCF {

    FilterService::FilterService() :
        mFilterFactoryMapMutex(WriterPriority)
    {
    }

    void FilterService::onServerStart(RcfServer & server)
    {

#if RCF_FEATURE_LEGACY==1
        server.bind<I_RequestTransportFilters>(*this);
#endif

    }

    void FilterService::onServerStop(RcfServer & server)
    {

#if RCF_FEATURE_LEGACY==1
        server.unbind<I_RequestTransportFilters>();
#endif

    }

    void FilterService::addFilterFactory(FilterFactoryPtr filterFactoryPtr)
    {
        int filterId = filterFactoryPtr->getFilterId();
        WriteLock writeLock(mFilterFactoryMapMutex);
        mFilterFactoryMap[ filterId ] = filterFactoryPtr;
    }

    void FilterService::addFilterFactory(
        FilterFactoryPtr filterFactoryPtr,
        const std::vector<int> &filterIds)
    {
        WriteLock writeLock(mFilterFactoryMapMutex);
        for (std::size_t i=0; i<filterIds.size(); ++i)
        {
            mFilterFactoryMap[ filterIds[i] ] = filterFactoryPtr;
        }
    }

    // remotely accessible
    boost::int32_t FilterService::QueryForTransportFilters(const std::vector<boost::int32_t> & filterIds)
    {
        RCF_THROW( _RcfError_NoLongerSupported("FilterService::QueryForTransportFilters()") );
        RCF_UNUSED_VARIABLE(filterIds);
        return RcfError_Ok;
    }

    // remotely accessible
    boost::int32_t FilterService::RequestTransportFilters(const std::vector<boost::int32_t> &filterIds)
    {
        RCF_LOG_3()(filterIds) << "FilterService::RequestTransportFilters() - entry";

        RcfSession & session = getCurrentRcfSession();
        RcfServer & server = session.getRcfServer();

        boost::shared_ptr< std::vector<FilterPtr> > filters(
            new std::vector<FilterPtr>());

        ReadLock readLock(mFilterFactoryMapMutex);
        for (unsigned int i=0; i<filterIds.size(); ++i)
        {
            int filterId = filterIds[i];

            if (filterId == RcfFilter_SspiSchannel || filterId == RcfFilter_OpenSsl)
            {               
                if (server.getSslImplementation() == RCF::Si_Schannel)
                {
                    filterId = RcfFilter_SspiSchannel;
                }
                else
                {
                    filterId = RcfFilter_OpenSsl;
                }
            }

            if (mFilterFactoryMap.find(filterId) == mFilterFactoryMap.end())
            {
                RCF_LOG_3()(filterId) << "FilterService::RequestTransportFilters() - filter not supported.";
                return RcfError_UnknownFilter;
            }

            FilterFactoryPtr filterFactoryPtr = mFilterFactoryMap[filterId];
            FilterPtr filterPtr( filterFactoryPtr->createFilter(server) );
            filters->push_back(filterPtr);
        }

        // Determine which protocol, if any, the filter sequence represents.
        
        session.mEnableCompression = false;

        FilterPtr filterPtr;
        if (filters->size() > 0)
        {
            if ( (*filters)[0]->getFilterId() == RcfFilter_ZlibCompressionStateful )
            {
                session.mEnableCompression = true;
                if (filters->size() > 1)
                {
                    filterPtr = (*filters)[1];
                }
            }
            else
            {
                if (filters->size() > 0)
                {
                    filterPtr = (*filters)[0];
                }
            }
        }

        TransportProtocol protocol = Tp_Unspecified;
        if (!filterPtr)
        {
            protocol = Tp_Clear;
        }
        else
        {
            int filterId = filterPtr->getFilterId();
            switch (filterId)
            {
            case RcfFilter_SspiNtlm:        protocol = Tp_Ntlm;         break;
            case RcfFilter_SspiKerberos:    protocol = Tp_Kerberos;     break;
            case RcfFilter_SspiNegotiate:   protocol = Tp_Negotiate;    break;
            case RcfFilter_SspiSchannel:    protocol = Tp_Ssl;          break;
            case RcfFilter_OpenSsl:         protocol = Tp_Ssl;          break;
            default:                        protocol = Tp_Unspecified;  break;
            }
        }

        // Check that the filter sequence is allowed.
        const std::vector<TransportProtocol> & protocols = server.getSupportedTransportProtocols();
        if (protocols.size() > 0)
        {
            if (std::find(protocols.begin(), protocols.end(), protocol) == protocols.end())
            {
                RCF_THROW( Exception(_RcfError_ProtocolNotSupported()) );
            }
        }

        if (protocol != Tp_Unspecified)
        {
            session.mTransportProtocol = protocol;
        }
        
        if (session.transportFiltersLocked())
        {
            RCF_LOG_3() << "FilterService::RequestTransportFilters() - filter sequence already locked.";
            return RcfError_FiltersLocked;
        }
        else
        {
            session.addOnWriteCompletedCallback( boost::bind(
                &FilterService::setTransportFilters, 
                this, 
                _1, 
                filters) );
        }        

        RCF_LOG_3() << "FilterService::RequestTransportFilters() - exit";
        return RcfError_Ok;
    }

    FilterFactoryPtr FilterService::getFilterFactoryPtr(int filterId)
    {
        ReadLock readLock(mFilterFactoryMapMutex);
        return mFilterFactoryMap.find(filterId) == mFilterFactoryMap.end() ?
            FilterFactoryPtr() :
            mFilterFactoryMap[filterId];
    }

    void FilterService::setTransportFilters(
        RcfSession &session,
        boost::shared_ptr<std::vector<FilterPtr> > filters)
    {
        session.getNetworkSession().setTransportFilters(*filters);
    }

} // namespace RCF
