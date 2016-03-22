
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

#ifndef INCLUDE_RCF_REMOTECALLCONTEXT_HPP
#define INCLUDE_RCF_REMOTECALLCONTEXT_HPP

//#include <RCF/AsioServerTransport.hpp>
#include <RCF/Export.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    class I_Parameters;
    class RcfSession;
    class AsioNetworkSession;

    typedef boost::shared_ptr<RcfSession> RcfSessionPtr;
    typedef boost::shared_ptr<AsioNetworkSession> AsioNetworkSessionPtr;

    class RCF_EXPORT RemoteCallContextImpl
    {
    public:

        RemoteCallContextImpl(RCF::RcfSession & session);

        void commit();
        void commit(const std::exception &e);
        bool isCommitted() const;

    private:
        RcfSessionPtr       mRcfSessionPtr;
        AsioNetworkSessionPtr mNetworkSessionPtr;
        bool                mCommitted;

    protected:
        I_Parameters *      mpParametersUntyped;

    };

    template<
        typename R, 
        typename A1  = Void,
        typename A2  = Void,
        typename A3  = Void,
        typename A4  = Void,
        typename A5  = Void,
        typename A6  = Void,
        typename A7  = Void,
        typename A8  = Void,
        typename A9  = Void,
        typename A10 = Void,
        typename A11 = Void,
        typename A12 = Void,
        typename A13 = Void,
        typename A14 = Void,
        typename A15 = Void>
    class RemoteCallContext : public RemoteCallContextImpl
    {
    public:

        // If return type is void, change it to RCF::Void.
        typedef typename boost::mpl::if_<
            boost::is_same<R, void>,
            Void,
            R>::type R_;

        typedef ServerParameters<
            R_, 
            A1, A2, A3, A4, A5, A6, A7, A8,
            A9, A10, A11, A12, A13, A14, A15> ParametersT;

        RemoteCallContext(RCF::RcfSession & session) : RemoteCallContextImpl(session)
        {
            RCF_ASSERT( dynamic_cast<ParametersT *>(mpParametersUntyped) );
        }

        ParametersT &parameters()
        {
            return * static_cast<ParametersT *>(mpParametersUntyped);;
        }
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_REMOTECALLCONTEXT_HPP
