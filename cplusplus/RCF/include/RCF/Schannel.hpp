
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

#ifndef INCLUDE_RCF_SCHANNEL_HPP
#define INCLUDE_RCF_SCHANNEL_HPP

#include <RCF/Filter.hpp>
#include <RCF/SspiFilter.hpp>
#include <RCF/util/Tchar.hpp>

#include <schnlsp.h>

// missing stuff in mingw headers
#ifdef __MINGW32__
#ifndef SP_PROT_NONE
#define SP_PROT_NONE                    0
#endif
#endif // __MINGW32__

namespace RCF {

    static const ULONG DefaultSchannelContextRequirements = 
        ASC_REQ_SEQUENCE_DETECT |
        ASC_REQ_REPLAY_DETECT   |
        ASC_REQ_CONFIDENTIALITY |
        ASC_REQ_EXTENDED_ERROR  |
        ASC_REQ_ALLOCATE_MEMORY |
        ASC_REQ_STREAM;

    class SchannelServerFilter : public SspiServerFilter
    {
    public:
        SchannelServerFilter(
            RcfServer & server,
            DWORD enabledProtocols,
            ULONG contextRequirements);

        int getFilterId() const;
    };

    class SchannelFilterFactory : public FilterFactory
    {
    public:

        SchannelFilterFactory(
            DWORD enabledProtocols = SP_PROT_TLS1_SERVER,
            ULONG contextRequirements = DefaultSchannelContextRequirements);

        FilterPtr                           createFilter(RcfServer & server);
        int                                 getFilterId();

    private:
        
        ULONG mContextRequirements;
        DWORD mEnabledProtocols;
    };

    class SchannelClientFilter : public SspiClientFilter
    {
    public:
        SchannelClientFilter(
            ClientStub * pClientStub,
            DWORD enabledProtocols = SP_PROT_TLS1_CLIENT,
            ULONG contextRequirements = DefaultSchannelContextRequirements);

        int getFilterId() const;
    };

    typedef SchannelClientFilter        SchannelFilter;  

} // namespace RCF

#endif // ! INCLUDE_RCF_SCHANNEL_HPP
