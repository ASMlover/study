
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

#ifndef INCLUDE_RCF_IDL_HPP
#define INCLUDE_RCF_IDL_HPP

#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/int.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/Config.hpp>
#include <RCF/Endpoint.hpp>
#include <RCF/Exception.hpp>
#include <RCF/Future.hpp>
#include <RCF/GetInterfaceName.hpp>
#include <RCF/Marshal.hpp>
#include <RCF/RcfClient.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ServerStub.hpp>
#include <RCF/ThreadLocalData.hpp>

#ifndef RCF_EXPORT_INTERFACE
#define RCF_EXPORT_INTERFACE
#endif

#define RCF_BEGIN_INLINE(InterfaceT, Name) RCF_BEGIN_I0_INLINE(InterfaceT, Name)

#define RCF_BEGIN_I0_INLINE(InterfaceT, Name)                               \
    RCF_BEGIN_IMPL_PRELUDE(InterfaceT, Name)                                \
    RCF_BEGIN_IMPL_INHERITED_0(InterfaceT, Name)                            \
    RCF_BEGIN_IMPL_POSTLUDE(InterfaceT, Name)

#define RCF_BEGIN_I1_INLINE(InterfaceT, Name, InheritT1)                    \
    RCF_BEGIN_IMPL_PRELUDE(InterfaceT, Name)                                \
    RCF_BEGIN_IMPL_INHERITED_1(InterfaceT, Name, InheritT1)                 \
    RCF_BEGIN_IMPL_POSTLUDE(InterfaceT, Name)

#define RCF_BEGIN_I2_INLINE(InterfaceT, Name, InheritT1, InheritT2)         \
    RCF_BEGIN_IMPL_PRELUDE(InterfaceT, Name)                                \
    RCF_BEGIN_IMPL_INHERITED_2(InterfaceT, Name, InheritT1, InheritT2)      \
    RCF_BEGIN_IMPL_POSTLUDE(InterfaceT, Name)

#define RCF_BEGIN_IMPL_PRELUDE(InterfaceT, Name)                            \
                                                                            \
    template<typename T>                                                    \
    class RcfClient;                                                        \
                                                                            \
    class RCF_EXPORT_INTERFACE InterfaceT                                   \
    {                                                                       \
    public:                                                                 \
        typedef RcfClient<InterfaceT> RcfClientT;                           \
        static std::string getInterfaceName()                               \
        {                                                                   \
            std::string interfaceName(Name);                                \
            if (interfaceName.empty())                                      \
            {                                                               \
                interfaceName = #InterfaceT;                                \
            }                                                               \
            return interfaceName;                                           \
        }                                                                   \
    };

#define RCF_BEGIN_IMPL_INHERITED_0(InterfaceT, Name)                        \
    template<>                                                              \
    class RCF_EXPORT_INTERFACE RcfClient< InterfaceT > :                    \
        public /*virtual*/ ::RCF::I_RcfClient                                   \
    {                                                                       \
    private:                                                                \
        template<typename DerefPtrT>                                        \
        void registerInvokeFunctors(                                        \
            ::RCF::InvokeFunctorMap &invokeFunctorMap,                      \
            DerefPtrT derefPtr)                                             \
        {                                                                   \
            ::RCF::registerInvokeFunctors(                                  \
                *this,                                                      \
                invokeFunctorMap,                                           \
                derefPtr);                                                  \
        }                                                                   \
        void setClientStubPtr(::RCF::ClientStubPtr clientStubPtr)           \
        {                                                                   \
            I_RcfClient::setClientStubPtr(clientStubPtr);                   \
        }

#define RCF_BEGIN_IMPL_POSTLUDE(InterfaceT, Name)                           \
    public:                                                                 \
                                                                            \
        RcfClient() :                                                       \
            I_RcfClient( ::RCF::getInterfaceName( (InterfaceT *) NULL) )    \
        {                                                                   \
        }                                                                   \
                                                                            \
        template<typename DerefPtrT>                                        \
        RcfClient(                                                          \
            ::RCF::ServerBindingPtr         serverStubPtr,                  \
            DerefPtrT                       derefPtr,                       \
            boost::mpl::true_ *             ) :                             \
                I_RcfClient(                                                \
                    ::RCF::getInterfaceName( (InterfaceT *) NULL),          \
                    serverStubPtr)                                          \
        {                                                                   \
            serverStubPtr->registerInvokeFunctors(*this, derefPtr);         \
        }                                                                   \
                                                                            \
        RcfClient(                                                          \
            const ::RCF::Endpoint &         endpoint,                       \
            const std::string &             targetName = "") :              \
                I_RcfClient(                                                \
                    ::RCF::getInterfaceName( (InterfaceT *) NULL),          \
                    endpoint,                                               \
                    targetName)                                             \
        {                                                                   \
        }                                                                   \
                                                                            \
        RcfClient(                                                          \
            ::RCF::ClientTransportAutoPtr   clientTransportAutoPtr,         \
            const std::string &             targetName = "") :              \
                I_RcfClient(                                                \
                    ::RCF::getInterfaceName( (InterfaceT *) NULL),          \
                    clientTransportAutoPtr,                                 \
                    targetName)                                             \
        {                                                                   \
        }                                                                   \
                                                                            \
        RcfClient(                                                          \
            const ::RCF::ClientStub &       clientStub,                     \
            const std::string &             targetName = "") :              \
                I_RcfClient(                                                \
                    ::RCF::getInterfaceName( (InterfaceT *) NULL),          \
                    clientStub,                                             \
                    targetName)                                             \
        {                                                                   \
        }                                                                   \
                                                                            \
        RcfClient(                                                          \
            const ::RCF::I_RcfClient &      rhs) :                          \
                I_RcfClient(                                                \
                    ::RCF::getInterfaceName( (InterfaceT *) NULL),          \
                    rhs)                                                    \
        {                                                                   \
        }                                                                   \
                                                                            \
        ~RcfClient()                                                        \
        {                                                                   \
        }                                                                   \
                                                                            \
    private:                                                                \
                                                                            \
        template<typename N, typename T>                                    \
        void invoke(                                                        \
            const N &,                                                      \
            ::RCF::RcfSession &,                                            \
            const T &)                                                      \
        {                                                                   \
            ::RCF::Exception e(RCF::_RcfError_FnId(N::value));              \
            RCF_THROW(e);                                                   \
        }                                                                   \
                                                                            \
        const char * getFunctionName(...)                                   \
        {                                                                   \
            RCF_ASSERT(0 && "getFunctionName() - invalid function id");     \
            return "";                                                      \
        }                                                                   \
                                                                            \
        const char * getArity(...)                                          \
        {                                                                   \
            return "";                                                      \
        }                                                                   \
                                                                            \
        typedef RcfClient< InterfaceT > ThisT;                              \
        typedef ::RCF::Dummy<ThisT>     DummyThisT;                         \
                                                                            \
        friend class ::RCF::StubAccess;                                     \
                                                                            \
        friend ::RCF::default_ RCF_make_next_dispatch_id_func(              \
            DummyThisT *,                                                   \
            ThisT *,                                                        \
            ...);                                                           \
    public:                                                                 \
        typedef InterfaceT              Interface;
        


#define RCF_END_INLINE( InterfaceT )                                        \
    };

#define RCF_METHOD_PLACEHOLDER()                                            \
    RCF_METHOD_PLACEHOLDER_(RCF_MAKE_UNIQUE_ID(PlaceHolder, V0))

#define RCF_METHOD_PLACEHOLDER_(id)                                         \
    public:                                                                 \
        RCF_MAKE_NEXT_DISPATCH_ID(id)                                       \
    private:



// For declarations only.

#define RCF_BEGIN_DECL RCF_BEGIN_INLINE
#define RCF_BEGIN_I0_DECL RCF_BEGIN_I0_INLINE
#define RCF_END_DECL RCF_END_INLINE

// For definitions only.

#define RCF_BEGIN_DEF(Interface, Name)                                                                  \
    RCF_BEGIN_DEF_(Interface, Name, RCF_PP_CAT(rcf_interface_id_, Interface, _, __LINE__))

#define RCF_BEGIN_I0_DEF(Interface, Name)                                                               \
    RCF_BEGIN_DEF_(Interface, Name, RCF_PP_CAT(rcf_interface_id_, Interface, _, __LINE__))

#define RCF_BEGIN_DEF_(Interface, Name, interfaceId)                                                    \
    static ::RCF::default_ RCF_interface_id_helper(int *, int *, ...);                                  \
    static ::RCF::default_ RCF_def_dispatch_id_helper(::RCF::Dummy< RcfClient<Interface> > *, RcfClient<Interface> *, ...);     \
    RCF_ADVANCE_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int, static)                       \
    template<typename N> class GeneratorParms;                                                          \
    template<>                                                                                          \
    class GeneratorParms<interfaceId>                                                                   \
    {                                                                                                   \
    public:                                                                                             \
        typedef Interface InterfaceT;                                                                   \
        typedef RcfClient<InterfaceT> RcfClientT;                                                       \
    };

#define RCF_END_DEF( Interface )



#include "RcfMethodGen.hpp"

// RCF_MAKE_UNIQUE_ID

BOOST_STATIC_ASSERT( sizeof(RCF::defined_) != sizeof(RCF::default_));

#define RCF_PP_CAT(arg1, arg2, arg3, arg4)      BOOST_PP_CAT( arg1, BOOST_PP_CAT( arg2, BOOST_PP_CAT(arg3, arg4) ) )
#define RCF_MAKE_UNIQUE_ID(func, sig)           RCF_PP_CAT(rcf_unique_id_, func, sig, __LINE__)

#define RCF_MAKE_NEXT_DISPATCH_ID(next_dispatch_id)                                                     \
    RCF_ADVANCE_STATIC_ID(next_dispatch_id, RCF_make_next_dispatch_id_func, DummyThisT, ThisT, friend)

#if RCF_MAX_METHOD_COUNT <= 35

#define RCF_ADVANCE_STATIC_ID(next_static_id, helper_func, T1, T2, friend_or_not)                                                                 \
    typedef                                                                                                                                       \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 0> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 1> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 2> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 3> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 4> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 5> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 6> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 7> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 8> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 9> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<10> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<11> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<12> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<13> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<14> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<15> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<16> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<17> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<18> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<19> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<20> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<21> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<22> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<23> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<24> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<25> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<26> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<27> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<28> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<29> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<30> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<31> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<32> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<33> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<34> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::int_<35>,                                                                                                                         \
    boost::mpl::int_<34> >::type,                                                                                                                 \
    boost::mpl::int_<33> >::type,                                                                                                                 \
    boost::mpl::int_<32> >::type,                                                                                                                 \
    boost::mpl::int_<31> >::type,                                                                                                                 \
    boost::mpl::int_<30> >::type,                                                                                                                 \
    boost::mpl::int_<29> >::type,                                                                                                                 \
    boost::mpl::int_<28> >::type,                                                                                                                 \
    boost::mpl::int_<27> >::type,                                                                                                                 \
    boost::mpl::int_<26> >::type,                                                                                                                 \
    boost::mpl::int_<25> >::type,                                                                                                                 \
    boost::mpl::int_<24> >::type,                                                                                                                 \
    boost::mpl::int_<23> >::type,                                                                                                                 \
    boost::mpl::int_<22> >::type,                                                                                                                 \
    boost::mpl::int_<21> >::type,                                                                                                                 \
    boost::mpl::int_<20> >::type,                                                                                                                 \
    boost::mpl::int_<19> >::type,                                                                                                                 \
    boost::mpl::int_<18> >::type,                                                                                                                 \
    boost::mpl::int_<17> >::type,                                                                                                                 \
    boost::mpl::int_<16> >::type,                                                                                                                 \
    boost::mpl::int_<15> >::type,                                                                                                                 \
    boost::mpl::int_<14> >::type,                                                                                                                 \
    boost::mpl::int_<13> >::type,                                                                                                                 \
    boost::mpl::int_<12> >::type,                                                                                                                 \
    boost::mpl::int_<11> >::type,                                                                                                                 \
    boost::mpl::int_<10> >::type,                                                                                                                 \
    boost::mpl::int_< 9> >::type,                                                                                                                 \
    boost::mpl::int_< 8> >::type,                                                                                                                 \
    boost::mpl::int_< 7> >::type,                                                                                                                 \
    boost::mpl::int_< 6> >::type,                                                                                                                 \
    boost::mpl::int_< 5> >::type,                                                                                                                 \
    boost::mpl::int_< 4> >::type,                                                                                                                 \
    boost::mpl::int_< 3> >::type,                                                                                                                 \
    boost::mpl::int_< 2> >::type,                                                                                                                 \
    boost::mpl::int_< 1> >::type,                                                                                                                 \
    boost::mpl::int_< 0> >::type next_static_id;                                                                                                  \
    friend_or_not RCF::defined_ helper_func(T1 *, T2 *, next_static_id *);


#define RCF_CURRENT_STATIC_ID(current_static_id, helper_func, T1, T2)                                                                             \
    typedef                                                                                                                                       \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 0> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 1> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 2> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 3> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 4> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 5> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 6> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 7> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 8> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 9> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<10> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<11> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<12> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<13> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<14> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<15> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<16> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<17> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<18> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<19> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<20> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<21> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<22> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<23> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<24> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<25> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<26> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<27> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<28> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<29> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<30> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<31> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<32> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<33> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<34> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::int_<34>,                                                                                                                         \
    boost::mpl::int_<33> >::type,                                                                                                                 \
    boost::mpl::int_<32> >::type,                                                                                                                 \
    boost::mpl::int_<31> >::type,                                                                                                                 \
    boost::mpl::int_<30> >::type,                                                                                                                 \
    boost::mpl::int_<29> >::type,                                                                                                                 \
    boost::mpl::int_<28> >::type,                                                                                                                 \
    boost::mpl::int_<27> >::type,                                                                                                                 \
    boost::mpl::int_<26> >::type,                                                                                                                 \
    boost::mpl::int_<25> >::type,                                                                                                                 \
    boost::mpl::int_<24> >::type,                                                                                                                 \
    boost::mpl::int_<23> >::type,                                                                                                                 \
    boost::mpl::int_<22> >::type,                                                                                                                 \
    boost::mpl::int_<21> >::type,                                                                                                                 \
    boost::mpl::int_<20> >::type,                                                                                                                 \
    boost::mpl::int_<19> >::type,                                                                                                                 \
    boost::mpl::int_<18> >::type,                                                                                                                 \
    boost::mpl::int_<17> >::type,                                                                                                                 \
    boost::mpl::int_<16> >::type,                                                                                                                 \
    boost::mpl::int_<15> >::type,                                                                                                                 \
    boost::mpl::int_<14> >::type,                                                                                                                 \
    boost::mpl::int_<13> >::type,                                                                                                                 \
    boost::mpl::int_<12> >::type,                                                                                                                 \
    boost::mpl::int_<11> >::type,                                                                                                                 \
    boost::mpl::int_<10> >::type,                                                                                                                 \
    boost::mpl::int_< 9> >::type,                                                                                                                 \
    boost::mpl::int_< 8> >::type,                                                                                                                 \
    boost::mpl::int_< 7> >::type,                                                                                                                 \
    boost::mpl::int_< 6> >::type,                                                                                                                 \
    boost::mpl::int_< 5> >::type,                                                                                                                 \
    boost::mpl::int_< 4> >::type,                                                                                                                 \
    boost::mpl::int_< 3> >::type,                                                                                                                 \
    boost::mpl::int_< 2> >::type,                                                                                                                 \
    boost::mpl::int_< 1> >::type,                                                                                                                 \
    boost::mpl::int_< 0> >::type,                                                                                                                 \
    boost::mpl::int_<-1> >::type current_static_id;


#elif RCF_MAX_METHOD_COUNT <= 100

#define RCF_ADVANCE_STATIC_ID(next_static_id, helper_func, T1, T2, friend_or_not)                                                                 \
    typedef                                                                                                                                       \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 0> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 1> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 2> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 3> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 4> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 5> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 6> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 7> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 8> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 9> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<10> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<11> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<12> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<13> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<14> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<15> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<16> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<17> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<18> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<19> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<20> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<21> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<22> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<23> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<24> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<25> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<26> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<27> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<28> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<29> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<30> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<31> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<32> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<33> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<34> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<35> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<36> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<37> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<38> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<39> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<40> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<41> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<42> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<43> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<44> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<45> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<46> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<47> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<48> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<49> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<50> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<51> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<52> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<53> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<54> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<55> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<56> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<57> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<58> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<59> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<60> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<61> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<62> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<63> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<64> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<65> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<66> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<67> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<68> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<69> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<70> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<71> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<72> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<73> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<74> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<75> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<76> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<77> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<78> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<79> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<80> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<81> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<82> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<83> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<84> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<85> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<86> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<87> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<88> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<89> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<90> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<91> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<92> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<93> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<94> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<95> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<96> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<97> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<98> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<99> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::int_<100>,                                                                                                                        \
    boost::mpl::int_<99> >::type,                                                                                                                 \
    boost::mpl::int_<98> >::type,                                                                                                                 \
    boost::mpl::int_<97> >::type,                                                                                                                 \
    boost::mpl::int_<96> >::type,                                                                                                                 \
    boost::mpl::int_<95> >::type,                                                                                                                 \
    boost::mpl::int_<94> >::type,                                                                                                                 \
    boost::mpl::int_<93> >::type,                                                                                                                 \
    boost::mpl::int_<92> >::type,                                                                                                                 \
    boost::mpl::int_<91> >::type,                                                                                                                 \
    boost::mpl::int_<90> >::type,                                                                                                                 \
    boost::mpl::int_<89> >::type,                                                                                                                 \
    boost::mpl::int_<88> >::type,                                                                                                                 \
    boost::mpl::int_<87> >::type,                                                                                                                 \
    boost::mpl::int_<86> >::type,                                                                                                                 \
    boost::mpl::int_<85> >::type,                                                                                                                 \
    boost::mpl::int_<84> >::type,                                                                                                                 \
    boost::mpl::int_<83> >::type,                                                                                                                 \
    boost::mpl::int_<82> >::type,                                                                                                                 \
    boost::mpl::int_<81> >::type,                                                                                                                 \
    boost::mpl::int_<80> >::type,                                                                                                                 \
    boost::mpl::int_<79> >::type,                                                                                                                 \
    boost::mpl::int_<78> >::type,                                                                                                                 \
    boost::mpl::int_<77> >::type,                                                                                                                 \
    boost::mpl::int_<76> >::type,                                                                                                                 \
    boost::mpl::int_<75> >::type,                                                                                                                 \
    boost::mpl::int_<74> >::type,                                                                                                                 \
    boost::mpl::int_<73> >::type,                                                                                                                 \
    boost::mpl::int_<72> >::type,                                                                                                                 \
    boost::mpl::int_<71> >::type,                                                                                                                 \
    boost::mpl::int_<70> >::type,                                                                                                                 \
    boost::mpl::int_<69> >::type,                                                                                                                 \
    boost::mpl::int_<68> >::type,                                                                                                                 \
    boost::mpl::int_<67> >::type,                                                                                                                 \
    boost::mpl::int_<66> >::type,                                                                                                                 \
    boost::mpl::int_<65> >::type,                                                                                                                 \
    boost::mpl::int_<64> >::type,                                                                                                                 \
    boost::mpl::int_<63> >::type,                                                                                                                 \
    boost::mpl::int_<62> >::type,                                                                                                                 \
    boost::mpl::int_<61> >::type,                                                                                                                 \
    boost::mpl::int_<60> >::type,                                                                                                                 \
    boost::mpl::int_<59> >::type,                                                                                                                 \
    boost::mpl::int_<58> >::type,                                                                                                                 \
    boost::mpl::int_<57> >::type,                                                                                                                 \
    boost::mpl::int_<56> >::type,                                                                                                                 \
    boost::mpl::int_<55> >::type,                                                                                                                 \
    boost::mpl::int_<54> >::type,                                                                                                                 \
    boost::mpl::int_<53> >::type,                                                                                                                 \
    boost::mpl::int_<52> >::type,                                                                                                                 \
    boost::mpl::int_<51> >::type,                                                                                                                 \
    boost::mpl::int_<50> >::type,                                                                                                                 \
    boost::mpl::int_<49> >::type,                                                                                                                 \
    boost::mpl::int_<48> >::type,                                                                                                                 \
    boost::mpl::int_<47> >::type,                                                                                                                 \
    boost::mpl::int_<46> >::type,                                                                                                                 \
    boost::mpl::int_<45> >::type,                                                                                                                 \
    boost::mpl::int_<44> >::type,                                                                                                                 \
    boost::mpl::int_<43> >::type,                                                                                                                 \
    boost::mpl::int_<42> >::type,                                                                                                                 \
    boost::mpl::int_<41> >::type,                                                                                                                 \
    boost::mpl::int_<40> >::type,                                                                                                                 \
    boost::mpl::int_<39> >::type,                                                                                                                 \
    boost::mpl::int_<38> >::type,                                                                                                                 \
    boost::mpl::int_<37> >::type,                                                                                                                 \
    boost::mpl::int_<36> >::type,                                                                                                                 \
    boost::mpl::int_<35> >::type,                                                                                                                 \
    boost::mpl::int_<34> >::type,                                                                                                                 \
    boost::mpl::int_<33> >::type,                                                                                                                 \
    boost::mpl::int_<32> >::type,                                                                                                                 \
    boost::mpl::int_<31> >::type,                                                                                                                 \
    boost::mpl::int_<30> >::type,                                                                                                                 \
    boost::mpl::int_<29> >::type,                                                                                                                 \
    boost::mpl::int_<28> >::type,                                                                                                                 \
    boost::mpl::int_<27> >::type,                                                                                                                 \
    boost::mpl::int_<26> >::type,                                                                                                                 \
    boost::mpl::int_<25> >::type,                                                                                                                 \
    boost::mpl::int_<24> >::type,                                                                                                                 \
    boost::mpl::int_<23> >::type,                                                                                                                 \
    boost::mpl::int_<22> >::type,                                                                                                                 \
    boost::mpl::int_<21> >::type,                                                                                                                 \
    boost::mpl::int_<20> >::type,                                                                                                                 \
    boost::mpl::int_<19> >::type,                                                                                                                 \
    boost::mpl::int_<18> >::type,                                                                                                                 \
    boost::mpl::int_<17> >::type,                                                                                                                 \
    boost::mpl::int_<16> >::type,                                                                                                                 \
    boost::mpl::int_<15> >::type,                                                                                                                 \
    boost::mpl::int_<14> >::type,                                                                                                                 \
    boost::mpl::int_<13> >::type,                                                                                                                 \
    boost::mpl::int_<12> >::type,                                                                                                                 \
    boost::mpl::int_<11> >::type,                                                                                                                 \
    boost::mpl::int_<10> >::type,                                                                                                                 \
    boost::mpl::int_< 9> >::type,                                                                                                                 \
    boost::mpl::int_< 8> >::type,                                                                                                                 \
    boost::mpl::int_< 7> >::type,                                                                                                                 \
    boost::mpl::int_< 6> >::type,                                                                                                                 \
    boost::mpl::int_< 5> >::type,                                                                                                                 \
    boost::mpl::int_< 4> >::type,                                                                                                                 \
    boost::mpl::int_< 3> >::type,                                                                                                                 \
    boost::mpl::int_< 2> >::type,                                                                                                                 \
    boost::mpl::int_< 1> >::type,                                                                                                                 \
    boost::mpl::int_< 0> >::type next_static_id;                                                                                                  \
    friend_or_not RCF::defined_ helper_func(T1 *, T2 *, next_static_id *);


#define RCF_CURRENT_STATIC_ID(current_static_id, helper_func, T1, T2)                                                                             \
    typedef                                                                                                                                       \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 0> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 1> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 2> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 3> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 4> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 5> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 6> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 7> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 8> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 9> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<10> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<11> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<12> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<13> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<14> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<15> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<16> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<17> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<18> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<19> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<20> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<21> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<22> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<23> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<24> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<25> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<26> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<27> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<28> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<29> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<30> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<31> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<32> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<33> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<34> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<35> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<36> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<37> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<38> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<39> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<40> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<41> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<42> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<43> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<44> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<45> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<46> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<47> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<48> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<49> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<50> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<51> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<52> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<53> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<54> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<55> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<56> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<57> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<58> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<59> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<60> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<61> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<62> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<63> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<64> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<65> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<66> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<67> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<68> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<69> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<70> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<71> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<72> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<73> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<74> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<75> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<76> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<77> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<78> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<79> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<80> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<81> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<82> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<83> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<84> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<85> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<86> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<87> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<88> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<89> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<90> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<91> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<92> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<93> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<94> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<95> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<96> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<97> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<98> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<99> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::int_<99>,                                                                                                                         \
    boost::mpl::int_<98> >::type,                                                                                                                 \
    boost::mpl::int_<97> >::type,                                                                                                                 \
    boost::mpl::int_<96> >::type,                                                                                                                 \
    boost::mpl::int_<95> >::type,                                                                                                                 \
    boost::mpl::int_<94> >::type,                                                                                                                 \
    boost::mpl::int_<93> >::type,                                                                                                                 \
    boost::mpl::int_<92> >::type,                                                                                                                 \
    boost::mpl::int_<91> >::type,                                                                                                                 \
    boost::mpl::int_<90> >::type,                                                                                                                 \
    boost::mpl::int_<89> >::type,                                                                                                                 \
    boost::mpl::int_<88> >::type,                                                                                                                 \
    boost::mpl::int_<87> >::type,                                                                                                                 \
    boost::mpl::int_<86> >::type,                                                                                                                 \
    boost::mpl::int_<85> >::type,                                                                                                                 \
    boost::mpl::int_<84> >::type,                                                                                                                 \
    boost::mpl::int_<83> >::type,                                                                                                                 \
    boost::mpl::int_<82> >::type,                                                                                                                 \
    boost::mpl::int_<81> >::type,                                                                                                                 \
    boost::mpl::int_<80> >::type,                                                                                                                 \
    boost::mpl::int_<79> >::type,                                                                                                                 \
    boost::mpl::int_<78> >::type,                                                                                                                 \
    boost::mpl::int_<77> >::type,                                                                                                                 \
    boost::mpl::int_<76> >::type,                                                                                                                 \
    boost::mpl::int_<75> >::type,                                                                                                                 \
    boost::mpl::int_<74> >::type,                                                                                                                 \
    boost::mpl::int_<73> >::type,                                                                                                                 \
    boost::mpl::int_<72> >::type,                                                                                                                 \
    boost::mpl::int_<71> >::type,                                                                                                                 \
    boost::mpl::int_<70> >::type,                                                                                                                 \
    boost::mpl::int_<69> >::type,                                                                                                                 \
    boost::mpl::int_<68> >::type,                                                                                                                 \
    boost::mpl::int_<67> >::type,                                                                                                                 \
    boost::mpl::int_<66> >::type,                                                                                                                 \
    boost::mpl::int_<65> >::type,                                                                                                                 \
    boost::mpl::int_<64> >::type,                                                                                                                 \
    boost::mpl::int_<63> >::type,                                                                                                                 \
    boost::mpl::int_<62> >::type,                                                                                                                 \
    boost::mpl::int_<61> >::type,                                                                                                                 \
    boost::mpl::int_<60> >::type,                                                                                                                 \
    boost::mpl::int_<59> >::type,                                                                                                                 \
    boost::mpl::int_<58> >::type,                                                                                                                 \
    boost::mpl::int_<57> >::type,                                                                                                                 \
    boost::mpl::int_<56> >::type,                                                                                                                 \
    boost::mpl::int_<55> >::type,                                                                                                                 \
    boost::mpl::int_<54> >::type,                                                                                                                 \
    boost::mpl::int_<53> >::type,                                                                                                                 \
    boost::mpl::int_<52> >::type,                                                                                                                 \
    boost::mpl::int_<51> >::type,                                                                                                                 \
    boost::mpl::int_<50> >::type,                                                                                                                 \
    boost::mpl::int_<49> >::type,                                                                                                                 \
    boost::mpl::int_<48> >::type,                                                                                                                 \
    boost::mpl::int_<47> >::type,                                                                                                                 \
    boost::mpl::int_<46> >::type,                                                                                                                 \
    boost::mpl::int_<45> >::type,                                                                                                                 \
    boost::mpl::int_<44> >::type,                                                                                                                 \
    boost::mpl::int_<43> >::type,                                                                                                                 \
    boost::mpl::int_<42> >::type,                                                                                                                 \
    boost::mpl::int_<41> >::type,                                                                                                                 \
    boost::mpl::int_<40> >::type,                                                                                                                 \
    boost::mpl::int_<39> >::type,                                                                                                                 \
    boost::mpl::int_<38> >::type,                                                                                                                 \
    boost::mpl::int_<37> >::type,                                                                                                                 \
    boost::mpl::int_<36> >::type,                                                                                                                 \
    boost::mpl::int_<35> >::type,                                                                                                                 \
    boost::mpl::int_<34> >::type,                                                                                                                 \
    boost::mpl::int_<33> >::type,                                                                                                                 \
    boost::mpl::int_<32> >::type,                                                                                                                 \
    boost::mpl::int_<31> >::type,                                                                                                                 \
    boost::mpl::int_<30> >::type,                                                                                                                 \
    boost::mpl::int_<29> >::type,                                                                                                                 \
    boost::mpl::int_<28> >::type,                                                                                                                 \
    boost::mpl::int_<27> >::type,                                                                                                                 \
    boost::mpl::int_<26> >::type,                                                                                                                 \
    boost::mpl::int_<25> >::type,                                                                                                                 \
    boost::mpl::int_<24> >::type,                                                                                                                 \
    boost::mpl::int_<23> >::type,                                                                                                                 \
    boost::mpl::int_<22> >::type,                                                                                                                 \
    boost::mpl::int_<21> >::type,                                                                                                                 \
    boost::mpl::int_<20> >::type,                                                                                                                 \
    boost::mpl::int_<19> >::type,                                                                                                                 \
    boost::mpl::int_<18> >::type,                                                                                                                 \
    boost::mpl::int_<17> >::type,                                                                                                                 \
    boost::mpl::int_<16> >::type,                                                                                                                 \
    boost::mpl::int_<15> >::type,                                                                                                                 \
    boost::mpl::int_<14> >::type,                                                                                                                 \
    boost::mpl::int_<13> >::type,                                                                                                                 \
    boost::mpl::int_<12> >::type,                                                                                                                 \
    boost::mpl::int_<11> >::type,                                                                                                                 \
    boost::mpl::int_<10> >::type,                                                                                                                 \
    boost::mpl::int_< 9> >::type,                                                                                                                 \
    boost::mpl::int_< 8> >::type,                                                                                                                 \
    boost::mpl::int_< 7> >::type,                                                                                                                 \
    boost::mpl::int_< 6> >::type,                                                                                                                 \
    boost::mpl::int_< 5> >::type,                                                                                                                 \
    boost::mpl::int_< 4> >::type,                                                                                                                 \
    boost::mpl::int_< 3> >::type,                                                                                                                 \
    boost::mpl::int_< 2> >::type,                                                                                                                 \
    boost::mpl::int_< 1> >::type,                                                                                                                 \
    boost::mpl::int_< 0> >::type,                                                                                                                 \
    boost::mpl::int_<-1> >::type current_static_id;

#elif RCF_MAX_METHOD_COUNT <= 200

#define RCF_ADVANCE_STATIC_ID(next_static_id, helper_func, T1, T2, friend_or_not)                                                                  \
    typedef                                                                                                                                        \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  0> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  1> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  2> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  3> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  4> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  5> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  6> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  7> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  8> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  9> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 10> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 11> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 12> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 13> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 14> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 15> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 16> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 17> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 18> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 19> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 20> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 21> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 22> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 23> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 24> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 25> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 26> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 27> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 28> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 29> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 30> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 31> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 32> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 33> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 34> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 35> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 36> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 37> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 38> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 39> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 40> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 41> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 42> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 43> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 44> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 45> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 46> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 47> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 48> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 49> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 50> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 51> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 52> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 53> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 54> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 55> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 56> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 57> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 58> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 59> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 60> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 61> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 62> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 63> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 64> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 65> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 66> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 67> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 68> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 69> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 70> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 71> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 72> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 73> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 74> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 75> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 76> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 77> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 78> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 79> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 80> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 81> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 82> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 83> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 84> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 85> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 86> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 87> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 88> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 89> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 90> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 91> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 92> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 93> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 94> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 95> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 96> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 97> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 98> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 99> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<100>*) 0)) == sizeof(RCF::defined_)) >,          \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<101> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<102> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<103> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<104> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<105> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<106> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<107> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<108> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<109> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<110> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<111> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<112> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<113> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<114> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<115> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<116> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<117> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<118> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<119> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<120> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<121> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<122> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<123> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<124> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<125> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<126> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<127> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<128> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<129> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<130> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<131> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<132> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<133> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<134> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<135> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<136> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<137> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<138> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<139> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<140> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<141> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<142> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<143> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<144> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<145> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<146> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<147> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<148> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<149> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<150> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<151> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<152> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<153> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<154> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<155> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<156> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<157> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<158> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<159> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<160> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<161> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<162> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<163> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<164> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<165> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<166> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<167> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<168> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<169> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<170> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<171> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<172> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<173> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<174> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<175> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<176> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<177> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<178> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<179> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<180> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<181> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<182> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<183> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<184> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<185> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<186> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<187> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<188> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<189> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<190> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<191> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<192> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<193> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<194> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<195> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<196> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<197> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<198> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<199> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::int_<200>,                                                                                                                         \
    boost::mpl::int_<199> >::type,                                                                                                                 \
    boost::mpl::int_<198> >::type,                                                                                                                 \
    boost::mpl::int_<197> >::type,                                                                                                                 \
    boost::mpl::int_<196> >::type,                                                                                                                 \
    boost::mpl::int_<195> >::type,                                                                                                                 \
    boost::mpl::int_<194> >::type,                                                                                                                 \
    boost::mpl::int_<193> >::type,                                                                                                                 \
    boost::mpl::int_<192> >::type,                                                                                                                 \
    boost::mpl::int_<191> >::type,                                                                                                                 \
    boost::mpl::int_<190> >::type,                                                                                                                 \
    boost::mpl::int_<189> >::type,                                                                                                                 \
    boost::mpl::int_<188> >::type,                                                                                                                 \
    boost::mpl::int_<187> >::type,                                                                                                                 \
    boost::mpl::int_<186> >::type,                                                                                                                 \
    boost::mpl::int_<185> >::type,                                                                                                                 \
    boost::mpl::int_<184> >::type,                                                                                                                 \
    boost::mpl::int_<183> >::type,                                                                                                                 \
    boost::mpl::int_<182> >::type,                                                                                                                 \
    boost::mpl::int_<181> >::type,                                                                                                                 \
    boost::mpl::int_<180> >::type,                                                                                                                 \
    boost::mpl::int_<179> >::type,                                                                                                                 \
    boost::mpl::int_<178> >::type,                                                                                                                 \
    boost::mpl::int_<177> >::type,                                                                                                                 \
    boost::mpl::int_<176> >::type,                                                                                                                 \
    boost::mpl::int_<175> >::type,                                                                                                                 \
    boost::mpl::int_<174> >::type,                                                                                                                 \
    boost::mpl::int_<173> >::type,                                                                                                                 \
    boost::mpl::int_<172> >::type,                                                                                                                 \
    boost::mpl::int_<171> >::type,                                                                                                                 \
    boost::mpl::int_<170> >::type,                                                                                                                 \
    boost::mpl::int_<169> >::type,                                                                                                                 \
    boost::mpl::int_<168> >::type,                                                                                                                 \
    boost::mpl::int_<167> >::type,                                                                                                                 \
    boost::mpl::int_<166> >::type,                                                                                                                 \
    boost::mpl::int_<165> >::type,                                                                                                                 \
    boost::mpl::int_<164> >::type,                                                                                                                 \
    boost::mpl::int_<163> >::type,                                                                                                                 \
    boost::mpl::int_<162> >::type,                                                                                                                 \
    boost::mpl::int_<161> >::type,                                                                                                                 \
    boost::mpl::int_<160> >::type,                                                                                                                 \
    boost::mpl::int_<159> >::type,                                                                                                                 \
    boost::mpl::int_<158> >::type,                                                                                                                 \
    boost::mpl::int_<157> >::type,                                                                                                                 \
    boost::mpl::int_<156> >::type,                                                                                                                 \
    boost::mpl::int_<155> >::type,                                                                                                                 \
    boost::mpl::int_<154> >::type,                                                                                                                 \
    boost::mpl::int_<153> >::type,                                                                                                                 \
    boost::mpl::int_<152> >::type,                                                                                                                 \
    boost::mpl::int_<151> >::type,                                                                                                                 \
    boost::mpl::int_<150> >::type,                                                                                                                 \
    boost::mpl::int_<149> >::type,                                                                                                                 \
    boost::mpl::int_<148> >::type,                                                                                                                 \
    boost::mpl::int_<147> >::type,                                                                                                                 \
    boost::mpl::int_<146> >::type,                                                                                                                 \
    boost::mpl::int_<145> >::type,                                                                                                                 \
    boost::mpl::int_<144> >::type,                                                                                                                 \
    boost::mpl::int_<143> >::type,                                                                                                                 \
    boost::mpl::int_<142> >::type,                                                                                                                 \
    boost::mpl::int_<141> >::type,                                                                                                                 \
    boost::mpl::int_<140> >::type,                                                                                                                 \
    boost::mpl::int_<139> >::type,                                                                                                                 \
    boost::mpl::int_<138> >::type,                                                                                                                 \
    boost::mpl::int_<137> >::type,                                                                                                                 \
    boost::mpl::int_<136> >::type,                                                                                                                 \
    boost::mpl::int_<135> >::type,                                                                                                                 \
    boost::mpl::int_<134> >::type,                                                                                                                 \
    boost::mpl::int_<133> >::type,                                                                                                                 \
    boost::mpl::int_<132> >::type,                                                                                                                 \
    boost::mpl::int_<131> >::type,                                                                                                                 \
    boost::mpl::int_<130> >::type,                                                                                                                 \
    boost::mpl::int_<129> >::type,                                                                                                                 \
    boost::mpl::int_<128> >::type,                                                                                                                 \
    boost::mpl::int_<127> >::type,                                                                                                                 \
    boost::mpl::int_<126> >::type,                                                                                                                 \
    boost::mpl::int_<125> >::type,                                                                                                                 \
    boost::mpl::int_<124> >::type,                                                                                                                 \
    boost::mpl::int_<123> >::type,                                                                                                                 \
    boost::mpl::int_<122> >::type,                                                                                                                 \
    boost::mpl::int_<121> >::type,                                                                                                                 \
    boost::mpl::int_<120> >::type,                                                                                                                 \
    boost::mpl::int_<119> >::type,                                                                                                                 \
    boost::mpl::int_<118> >::type,                                                                                                                 \
    boost::mpl::int_<117> >::type,                                                                                                                 \
    boost::mpl::int_<116> >::type,                                                                                                                 \
    boost::mpl::int_<115> >::type,                                                                                                                 \
    boost::mpl::int_<114> >::type,                                                                                                                 \
    boost::mpl::int_<113> >::type,                                                                                                                 \
    boost::mpl::int_<112> >::type,                                                                                                                 \
    boost::mpl::int_<111> >::type,                                                                                                                 \
    boost::mpl::int_<110> >::type,                                                                                                                 \
    boost::mpl::int_<109> >::type,                                                                                                                 \
    boost::mpl::int_<108> >::type,                                                                                                                 \
    boost::mpl::int_<107> >::type,                                                                                                                 \
    boost::mpl::int_<106> >::type,                                                                                                                 \
    boost::mpl::int_<105> >::type,                                                                                                                 \
    boost::mpl::int_<104> >::type,                                                                                                                 \
    boost::mpl::int_<103> >::type,                                                                                                                 \
    boost::mpl::int_<102> >::type,                                                                                                                 \
    boost::mpl::int_<101> >::type,                                                                                                                 \
    boost::mpl::int_<100> >::type,                                                                                                                 \
    boost::mpl::int_< 99> >::type,                                                                                                                 \
    boost::mpl::int_< 98> >::type,                                                                                                                 \
    boost::mpl::int_< 97> >::type,                                                                                                                 \
    boost::mpl::int_< 96> >::type,                                                                                                                 \
    boost::mpl::int_< 95> >::type,                                                                                                                 \
    boost::mpl::int_< 94> >::type,                                                                                                                 \
    boost::mpl::int_< 93> >::type,                                                                                                                 \
    boost::mpl::int_< 92> >::type,                                                                                                                 \
    boost::mpl::int_< 91> >::type,                                                                                                                 \
    boost::mpl::int_< 90> >::type,                                                                                                                 \
    boost::mpl::int_< 89> >::type,                                                                                                                 \
    boost::mpl::int_< 88> >::type,                                                                                                                 \
    boost::mpl::int_< 87> >::type,                                                                                                                 \
    boost::mpl::int_< 86> >::type,                                                                                                                 \
    boost::mpl::int_< 85> >::type,                                                                                                                 \
    boost::mpl::int_< 84> >::type,                                                                                                                 \
    boost::mpl::int_< 83> >::type,                                                                                                                 \
    boost::mpl::int_< 82> >::type,                                                                                                                 \
    boost::mpl::int_< 81> >::type,                                                                                                                 \
    boost::mpl::int_< 80> >::type,                                                                                                                 \
    boost::mpl::int_< 79> >::type,                                                                                                                 \
    boost::mpl::int_< 78> >::type,                                                                                                                 \
    boost::mpl::int_< 77> >::type,                                                                                                                 \
    boost::mpl::int_< 76> >::type,                                                                                                                 \
    boost::mpl::int_< 75> >::type,                                                                                                                 \
    boost::mpl::int_< 74> >::type,                                                                                                                 \
    boost::mpl::int_< 73> >::type,                                                                                                                 \
    boost::mpl::int_< 72> >::type,                                                                                                                 \
    boost::mpl::int_< 71> >::type,                                                                                                                 \
    boost::mpl::int_< 70> >::type,                                                                                                                 \
    boost::mpl::int_< 69> >::type,                                                                                                                 \
    boost::mpl::int_< 68> >::type,                                                                                                                 \
    boost::mpl::int_< 67> >::type,                                                                                                                 \
    boost::mpl::int_< 66> >::type,                                                                                                                 \
    boost::mpl::int_< 65> >::type,                                                                                                                 \
    boost::mpl::int_< 64> >::type,                                                                                                                 \
    boost::mpl::int_< 63> >::type,                                                                                                                 \
    boost::mpl::int_< 62> >::type,                                                                                                                 \
    boost::mpl::int_< 61> >::type,                                                                                                                 \
    boost::mpl::int_< 60> >::type,                                                                                                                 \
    boost::mpl::int_< 59> >::type,                                                                                                                 \
    boost::mpl::int_< 58> >::type,                                                                                                                 \
    boost::mpl::int_< 57> >::type,                                                                                                                 \
    boost::mpl::int_< 56> >::type,                                                                                                                 \
    boost::mpl::int_< 55> >::type,                                                                                                                 \
    boost::mpl::int_< 54> >::type,                                                                                                                 \
    boost::mpl::int_< 53> >::type,                                                                                                                 \
    boost::mpl::int_< 52> >::type,                                                                                                                 \
    boost::mpl::int_< 51> >::type,                                                                                                                 \
    boost::mpl::int_< 50> >::type,                                                                                                                 \
    boost::mpl::int_< 49> >::type,                                                                                                                 \
    boost::mpl::int_< 48> >::type,                                                                                                                 \
    boost::mpl::int_< 47> >::type,                                                                                                                 \
    boost::mpl::int_< 46> >::type,                                                                                                                 \
    boost::mpl::int_< 45> >::type,                                                                                                                 \
    boost::mpl::int_< 44> >::type,                                                                                                                 \
    boost::mpl::int_< 43> >::type,                                                                                                                 \
    boost::mpl::int_< 42> >::type,                                                                                                                 \
    boost::mpl::int_< 41> >::type,                                                                                                                 \
    boost::mpl::int_< 40> >::type,                                                                                                                 \
    boost::mpl::int_< 39> >::type,                                                                                                                 \
    boost::mpl::int_< 38> >::type,                                                                                                                 \
    boost::mpl::int_< 37> >::type,                                                                                                                 \
    boost::mpl::int_< 36> >::type,                                                                                                                 \
    boost::mpl::int_< 35> >::type,                                                                                                                 \
    boost::mpl::int_< 34> >::type,                                                                                                                 \
    boost::mpl::int_< 33> >::type,                                                                                                                 \
    boost::mpl::int_< 32> >::type,                                                                                                                 \
    boost::mpl::int_< 31> >::type,                                                                                                                 \
    boost::mpl::int_< 30> >::type,                                                                                                                 \
    boost::mpl::int_< 29> >::type,                                                                                                                 \
    boost::mpl::int_< 28> >::type,                                                                                                                 \
    boost::mpl::int_< 27> >::type,                                                                                                                 \
    boost::mpl::int_< 26> >::type,                                                                                                                 \
    boost::mpl::int_< 25> >::type,                                                                                                                 \
    boost::mpl::int_< 24> >::type,                                                                                                                 \
    boost::mpl::int_< 23> >::type,                                                                                                                 \
    boost::mpl::int_< 22> >::type,                                                                                                                 \
    boost::mpl::int_< 21> >::type,                                                                                                                 \
    boost::mpl::int_< 20> >::type,                                                                                                                 \
    boost::mpl::int_< 19> >::type,                                                                                                                 \
    boost::mpl::int_< 18> >::type,                                                                                                                 \
    boost::mpl::int_< 17> >::type,                                                                                                                 \
    boost::mpl::int_< 16> >::type,                                                                                                                 \
    boost::mpl::int_< 15> >::type,                                                                                                                 \
    boost::mpl::int_< 14> >::type,                                                                                                                 \
    boost::mpl::int_< 13> >::type,                                                                                                                 \
    boost::mpl::int_< 12> >::type,                                                                                                                 \
    boost::mpl::int_< 11> >::type,                                                                                                                 \
    boost::mpl::int_< 10> >::type,                                                                                                                 \
    boost::mpl::int_<  9> >::type,                                                                                                                 \
    boost::mpl::int_<  8> >::type,                                                                                                                 \
    boost::mpl::int_<  7> >::type,                                                                                                                 \
    boost::mpl::int_<  6> >::type,                                                                                                                 \
    boost::mpl::int_<  5> >::type,                                                                                                                 \
    boost::mpl::int_<  4> >::type,                                                                                                                 \
    boost::mpl::int_<  3> >::type,                                                                                                                 \
    boost::mpl::int_<  2> >::type,                                                                                                                 \
    boost::mpl::int_<  1> >::type,                                                                                                                 \
    boost::mpl::int_<  0> >::type next_static_id;                                                                                                  \
    friend_or_not RCF::defined_ helper_func(T1 *, T2 *, next_static_id *);


#define RCF_CURRENT_STATIC_ID(current_static_id, helper_func, T1, T2)                                                                              \
    typedef                                                                                                                                        \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  0> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  1> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  2> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  3> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  4> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  5> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  6> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  7> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  8> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<  9> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 10> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 11> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 12> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 13> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 14> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 15> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 16> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 17> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 18> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 19> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 20> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 21> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 22> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 23> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 24> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 25> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 26> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 27> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 28> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 29> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 30> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 31> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 32> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 33> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 34> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 35> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 36> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 37> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 38> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 39> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 40> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 41> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 42> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 43> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 44> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 45> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 46> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 47> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 48> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 49> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 50> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 51> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 52> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 53> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 54> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 55> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 56> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 57> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 58> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 59> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 60> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 61> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 62> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 63> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 64> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 65> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 66> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 67> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 68> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 69> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 70> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 71> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 72> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 73> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 74> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 75> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 76> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 77> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 78> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 79> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 80> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 81> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 82> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 83> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 84> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 85> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 86> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 87> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 88> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 89> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 90> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 91> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 92> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 93> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 94> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 95> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 96> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 97> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 98> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_< 99> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<100> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<101> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<102> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<103> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<104> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<105> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<106> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<107> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<108> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<109> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<110> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<111> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<112> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<113> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<114> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<115> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<116> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<117> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<118> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<119> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<120> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<121> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<122> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<123> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<124> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<125> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<126> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<127> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<128> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<129> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<130> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<131> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<132> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<133> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<134> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<135> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<136> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<137> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<138> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<139> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<140> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<141> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<142> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<143> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<144> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<145> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<146> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<147> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<148> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<149> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<150> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<151> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<152> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<153> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<154> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<155> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<156> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<157> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<158> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<159> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<160> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<161> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<162> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<163> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<164> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<165> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<166> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<167> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<168> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<169> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<170> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<171> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<172> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<173> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<174> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<175> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<176> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<177> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<178> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<179> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<180> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<181> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<182> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<183> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<184> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<185> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<186> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<187> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<188> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<189> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<190> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<191> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<192> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<193> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<194> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<195> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<196> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<197> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<198> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(helper_func((T1 *) 0, (T2 *) 0, (boost::mpl::int_<199> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::int_<199>,                                                                                                                         \
    boost::mpl::int_<198> >::type,                                                                                                                 \
    boost::mpl::int_<197> >::type,                                                                                                                 \
    boost::mpl::int_<196> >::type,                                                                                                                 \
    boost::mpl::int_<195> >::type,                                                                                                                 \
    boost::mpl::int_<194> >::type,                                                                                                                 \
    boost::mpl::int_<193> >::type,                                                                                                                 \
    boost::mpl::int_<192> >::type,                                                                                                                 \
    boost::mpl::int_<191> >::type,                                                                                                                 \
    boost::mpl::int_<190> >::type,                                                                                                                 \
    boost::mpl::int_<189> >::type,                                                                                                                 \
    boost::mpl::int_<188> >::type,                                                                                                                 \
    boost::mpl::int_<187> >::type,                                                                                                                 \
    boost::mpl::int_<186> >::type,                                                                                                                 \
    boost::mpl::int_<185> >::type,                                                                                                                 \
    boost::mpl::int_<184> >::type,                                                                                                                 \
    boost::mpl::int_<183> >::type,                                                                                                                 \
    boost::mpl::int_<182> >::type,                                                                                                                 \
    boost::mpl::int_<181> >::type,                                                                                                                 \
    boost::mpl::int_<180> >::type,                                                                                                                 \
    boost::mpl::int_<179> >::type,                                                                                                                 \
    boost::mpl::int_<178> >::type,                                                                                                                 \
    boost::mpl::int_<177> >::type,                                                                                                                 \
    boost::mpl::int_<176> >::type,                                                                                                                 \
    boost::mpl::int_<175> >::type,                                                                                                                 \
    boost::mpl::int_<174> >::type,                                                                                                                 \
    boost::mpl::int_<173> >::type,                                                                                                                 \
    boost::mpl::int_<172> >::type,                                                                                                                 \
    boost::mpl::int_<171> >::type,                                                                                                                 \
    boost::mpl::int_<170> >::type,                                                                                                                 \
    boost::mpl::int_<169> >::type,                                                                                                                 \
    boost::mpl::int_<168> >::type,                                                                                                                 \
    boost::mpl::int_<167> >::type,                                                                                                                 \
    boost::mpl::int_<166> >::type,                                                                                                                 \
    boost::mpl::int_<165> >::type,                                                                                                                 \
    boost::mpl::int_<164> >::type,                                                                                                                 \
    boost::mpl::int_<163> >::type,                                                                                                                 \
    boost::mpl::int_<162> >::type,                                                                                                                 \
    boost::mpl::int_<161> >::type,                                                                                                                 \
    boost::mpl::int_<160> >::type,                                                                                                                 \
    boost::mpl::int_<159> >::type,                                                                                                                 \
    boost::mpl::int_<158> >::type,                                                                                                                 \
    boost::mpl::int_<157> >::type,                                                                                                                 \
    boost::mpl::int_<156> >::type,                                                                                                                 \
    boost::mpl::int_<155> >::type,                                                                                                                 \
    boost::mpl::int_<154> >::type,                                                                                                                 \
    boost::mpl::int_<153> >::type,                                                                                                                 \
    boost::mpl::int_<152> >::type,                                                                                                                 \
    boost::mpl::int_<151> >::type,                                                                                                                 \
    boost::mpl::int_<150> >::type,                                                                                                                 \
    boost::mpl::int_<149> >::type,                                                                                                                 \
    boost::mpl::int_<148> >::type,                                                                                                                 \
    boost::mpl::int_<147> >::type,                                                                                                                 \
    boost::mpl::int_<146> >::type,                                                                                                                 \
    boost::mpl::int_<145> >::type,                                                                                                                 \
    boost::mpl::int_<144> >::type,                                                                                                                 \
    boost::mpl::int_<143> >::type,                                                                                                                 \
    boost::mpl::int_<142> >::type,                                                                                                                 \
    boost::mpl::int_<141> >::type,                                                                                                                 \
    boost::mpl::int_<140> >::type,                                                                                                                 \
    boost::mpl::int_<139> >::type,                                                                                                                 \
    boost::mpl::int_<138> >::type,                                                                                                                 \
    boost::mpl::int_<137> >::type,                                                                                                                 \
    boost::mpl::int_<136> >::type,                                                                                                                 \
    boost::mpl::int_<135> >::type,                                                                                                                 \
    boost::mpl::int_<134> >::type,                                                                                                                 \
    boost::mpl::int_<133> >::type,                                                                                                                 \
    boost::mpl::int_<132> >::type,                                                                                                                 \
    boost::mpl::int_<131> >::type,                                                                                                                 \
    boost::mpl::int_<130> >::type,                                                                                                                 \
    boost::mpl::int_<129> >::type,                                                                                                                 \
    boost::mpl::int_<128> >::type,                                                                                                                 \
    boost::mpl::int_<127> >::type,                                                                                                                 \
    boost::mpl::int_<126> >::type,                                                                                                                 \
    boost::mpl::int_<125> >::type,                                                                                                                 \
    boost::mpl::int_<124> >::type,                                                                                                                 \
    boost::mpl::int_<123> >::type,                                                                                                                 \
    boost::mpl::int_<122> >::type,                                                                                                                 \
    boost::mpl::int_<121> >::type,                                                                                                                 \
    boost::mpl::int_<120> >::type,                                                                                                                 \
    boost::mpl::int_<119> >::type,                                                                                                                 \
    boost::mpl::int_<118> >::type,                                                                                                                 \
    boost::mpl::int_<117> >::type,                                                                                                                 \
    boost::mpl::int_<116> >::type,                                                                                                                 \
    boost::mpl::int_<115> >::type,                                                                                                                 \
    boost::mpl::int_<114> >::type,                                                                                                                 \
    boost::mpl::int_<113> >::type,                                                                                                                 \
    boost::mpl::int_<112> >::type,                                                                                                                 \
    boost::mpl::int_<111> >::type,                                                                                                                 \
    boost::mpl::int_<110> >::type,                                                                                                                 \
    boost::mpl::int_<109> >::type,                                                                                                                 \
    boost::mpl::int_<108> >::type,                                                                                                                 \
    boost::mpl::int_<107> >::type,                                                                                                                 \
    boost::mpl::int_<106> >::type,                                                                                                                 \
    boost::mpl::int_<105> >::type,                                                                                                                 \
    boost::mpl::int_<104> >::type,                                                                                                                 \
    boost::mpl::int_<103> >::type,                                                                                                                 \
    boost::mpl::int_<102> >::type,                                                                                                                 \
    boost::mpl::int_<101> >::type,                                                                                                                 \
    boost::mpl::int_<100> >::type,                                                                                                                 \
    boost::mpl::int_< 99> >::type,                                                                                                                 \
    boost::mpl::int_< 98> >::type,                                                                                                                 \
    boost::mpl::int_< 97> >::type,                                                                                                                 \
    boost::mpl::int_< 96> >::type,                                                                                                                 \
    boost::mpl::int_< 95> >::type,                                                                                                                 \
    boost::mpl::int_< 94> >::type,                                                                                                                 \
    boost::mpl::int_< 93> >::type,                                                                                                                 \
    boost::mpl::int_< 92> >::type,                                                                                                                 \
    boost::mpl::int_< 91> >::type,                                                                                                                 \
    boost::mpl::int_< 90> >::type,                                                                                                                 \
    boost::mpl::int_< 89> >::type,                                                                                                                 \
    boost::mpl::int_< 88> >::type,                                                                                                                 \
    boost::mpl::int_< 87> >::type,                                                                                                                 \
    boost::mpl::int_< 86> >::type,                                                                                                                 \
    boost::mpl::int_< 85> >::type,                                                                                                                 \
    boost::mpl::int_< 84> >::type,                                                                                                                 \
    boost::mpl::int_< 83> >::type,                                                                                                                 \
    boost::mpl::int_< 82> >::type,                                                                                                                 \
    boost::mpl::int_< 81> >::type,                                                                                                                 \
    boost::mpl::int_< 80> >::type,                                                                                                                 \
    boost::mpl::int_< 79> >::type,                                                                                                                 \
    boost::mpl::int_< 78> >::type,                                                                                                                 \
    boost::mpl::int_< 77> >::type,                                                                                                                 \
    boost::mpl::int_< 76> >::type,                                                                                                                 \
    boost::mpl::int_< 75> >::type,                                                                                                                 \
    boost::mpl::int_< 74> >::type,                                                                                                                 \
    boost::mpl::int_< 73> >::type,                                                                                                                 \
    boost::mpl::int_< 72> >::type,                                                                                                                 \
    boost::mpl::int_< 71> >::type,                                                                                                                 \
    boost::mpl::int_< 70> >::type,                                                                                                                 \
    boost::mpl::int_< 69> >::type,                                                                                                                 \
    boost::mpl::int_< 68> >::type,                                                                                                                 \
    boost::mpl::int_< 67> >::type,                                                                                                                 \
    boost::mpl::int_< 66> >::type,                                                                                                                 \
    boost::mpl::int_< 65> >::type,                                                                                                                 \
    boost::mpl::int_< 64> >::type,                                                                                                                 \
    boost::mpl::int_< 63> >::type,                                                                                                                 \
    boost::mpl::int_< 62> >::type,                                                                                                                 \
    boost::mpl::int_< 61> >::type,                                                                                                                 \
    boost::mpl::int_< 60> >::type,                                                                                                                 \
    boost::mpl::int_< 59> >::type,                                                                                                                 \
    boost::mpl::int_< 58> >::type,                                                                                                                 \
    boost::mpl::int_< 57> >::type,                                                                                                                 \
    boost::mpl::int_< 56> >::type,                                                                                                                 \
    boost::mpl::int_< 55> >::type,                                                                                                                 \
    boost::mpl::int_< 54> >::type,                                                                                                                 \
    boost::mpl::int_< 53> >::type,                                                                                                                 \
    boost::mpl::int_< 52> >::type,                                                                                                                 \
    boost::mpl::int_< 51> >::type,                                                                                                                 \
    boost::mpl::int_< 50> >::type,                                                                                                                 \
    boost::mpl::int_< 49> >::type,                                                                                                                 \
    boost::mpl::int_< 48> >::type,                                                                                                                 \
    boost::mpl::int_< 47> >::type,                                                                                                                 \
    boost::mpl::int_< 46> >::type,                                                                                                                 \
    boost::mpl::int_< 45> >::type,                                                                                                                 \
    boost::mpl::int_< 44> >::type,                                                                                                                 \
    boost::mpl::int_< 43> >::type,                                                                                                                 \
    boost::mpl::int_< 42> >::type,                                                                                                                 \
    boost::mpl::int_< 41> >::type,                                                                                                                 \
    boost::mpl::int_< 40> >::type,                                                                                                                 \
    boost::mpl::int_< 39> >::type,                                                                                                                 \
    boost::mpl::int_< 38> >::type,                                                                                                                 \
    boost::mpl::int_< 37> >::type,                                                                                                                 \
    boost::mpl::int_< 36> >::type,                                                                                                                 \
    boost::mpl::int_< 35> >::type,                                                                                                                 \
    boost::mpl::int_< 34> >::type,                                                                                                                 \
    boost::mpl::int_< 33> >::type,                                                                                                                 \
    boost::mpl::int_< 32> >::type,                                                                                                                 \
    boost::mpl::int_< 31> >::type,                                                                                                                 \
    boost::mpl::int_< 30> >::type,                                                                                                                 \
    boost::mpl::int_< 29> >::type,                                                                                                                 \
    boost::mpl::int_< 28> >::type,                                                                                                                 \
    boost::mpl::int_< 27> >::type,                                                                                                                 \
    boost::mpl::int_< 26> >::type,                                                                                                                 \
    boost::mpl::int_< 25> >::type,                                                                                                                 \
    boost::mpl::int_< 24> >::type,                                                                                                                 \
    boost::mpl::int_< 23> >::type,                                                                                                                 \
    boost::mpl::int_< 22> >::type,                                                                                                                 \
    boost::mpl::int_< 21> >::type,                                                                                                                 \
    boost::mpl::int_< 20> >::type,                                                                                                                 \
    boost::mpl::int_< 19> >::type,                                                                                                                 \
    boost::mpl::int_< 18> >::type,                                                                                                                 \
    boost::mpl::int_< 17> >::type,                                                                                                                 \
    boost::mpl::int_< 16> >::type,                                                                                                                 \
    boost::mpl::int_< 15> >::type,                                                                                                                 \
    boost::mpl::int_< 14> >::type,                                                                                                                 \
    boost::mpl::int_< 13> >::type,                                                                                                                 \
    boost::mpl::int_< 12> >::type,                                                                                                                 \
    boost::mpl::int_< 11> >::type,                                                                                                                 \
    boost::mpl::int_< 10> >::type,                                                                                                                 \
    boost::mpl::int_<  9> >::type,                                                                                                                 \
    boost::mpl::int_<  8> >::type,                                                                                                                 \
    boost::mpl::int_<  7> >::type,                                                                                                                 \
    boost::mpl::int_<  6> >::type,                                                                                                                 \
    boost::mpl::int_<  5> >::type,                                                                                                                 \
    boost::mpl::int_<  4> >::type,                                                                                                                 \
    boost::mpl::int_<  3> >::type,                                                                                                                 \
    boost::mpl::int_<  2> >::type,                                                                                                                 \
    boost::mpl::int_<  1> >::type,                                                                                                                 \
    boost::mpl::int_<  0> >::type,                                                                                                                 \
    boost::mpl::int_< -1> >::type current_static_id;


#else

#error RCF_MAX_METHOD_COUNT > 200 is currently not implemented.

#endif // RCF_MAX_METHOD_COUNT

#ifdef RCF_SEPARATE_IDL_COMPILATION
#include <RCF/Idl_DeclareOnly.hpp>
#else
#include <RCF/Idl_Inline.hpp>
#endif

#endif // ! INCLUDE_RCF_IDL_HPP
