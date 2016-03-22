
#ifndef INCLUDE_RCF_RCFMETHODGEN_HPP
#define INCLUDE_RCF_RCFMETHODGEN_HPP




//------------------------------------------------------------------------------
// Parameters - R0
//------------------------------------------------------------------------------

// RCF_METHOD_R0
#define RCF_METHOD_R0_INLINE(R,func  )                                        \
    RCF_METHOD_R0_INLINE_(R,func  , RCF_MAKE_UNIQUE_ID(func, R0))

#define RCF_METHOD_R0_INLINE_(R,func  , id)                                   \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                )                                                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions()                                      \
                    );                                                        \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions                         \
                )                                                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R                                                     \
                         ,                                                    \
                        V,V,V,V,V,V,V,V,V,V,V,V,V,V,V >()(                    \
                            getClientStub()                                   \
                             ,                                                \
                            V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R0");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R0";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R                                                         \
                     > &p =                                                   \
                    ::RCF::AllocateServerParameters<                          \
                        R                                                     \
                         >()(session);                                        \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        ));                                                   \
            }

// RCF_METHOD_R0_DECL
#define RCF_METHOD_R0_DECL(R,func  )                                          \
    RCF_METHOD_R0_DECL_(R,func  , RCF_MAKE_UNIQUE_ID(func, R0))

#define RCF_METHOD_R0_DECL_(R,func  , id)                                     \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                )                                                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions()                                      \
                    );                                                        \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions                         \
                );                                                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id *                                                           \
               );                                                             \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R0";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R                                                         \
                     > &p =                                                   \
                    ::RCF::AllocateServerParameters<                          \
                        R                                                     \
                         >()(session);                                        \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        ));                                                   \
            }

// RCF_METHOD_R0_DEF
#define RCF_METHOD_R0_DEF(R,func  )                                           \
    RCF_METHOD_R0_DEF_(R,func  , RCF_PP_CAT(rcf_interface_id_1_, func, R0, __LINE__), RCF_MAKE_UNIQUE_ID(func, R0), RCF_PP_CAT(rcf_interface_id_2_, func, R0, __LINE__))

#define RCF_METHOD_R0_DEF_(R,func  , interfaceId, funcId, genParms)           \
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions                                 \
        )                                                                     \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R                                                             \
                 ,                                                            \
                V,V,V,V,V,V,V,V,V,V,V,V,V,V,V >()(                            \
                    getClientStub()                                           \
                     ,                                                        \
                    V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R0");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId *                                                              \
        )                                                                     \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V0
//------------------------------------------------------------------------------

// RCF_METHOD_V0
#define RCF_METHOD_V0_INLINE(R,func  )                                        \
    RCF_METHOD_V0_INLINE_(R,func   , RCF_MAKE_UNIQUE_ID(func, V0))

#define RCF_METHOD_V0_INLINE_(R,func  , id)                                   \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                )                                                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions()                                      \
                    );                                                        \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions                         \
                )                                                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V                                                     \
                         ,                                                    \
                        V,V,V,V,V,V,V,V,V,V,V,V,V,V,V >()(                    \
                            getClientStub()                                   \
                             ,                                                \
                            V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V0");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V0";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V                                                         \
                     > &p =                                                   \
                        ::RCF::AllocateServerParameters<                      \
                            V                                                 \
                             >()(session);                                    \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    );                                                        \
            }

// RCF_METHOD_V0_DECL
#define RCF_METHOD_V0_DECL(R,func  )                                          \
    RCF_METHOD_V0_DECL_(R,func   , RCF_MAKE_UNIQUE_ID(func, V0))

#define RCF_METHOD_V0_DECL_(R,func  , id)                                     \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                )                                                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions()                                      \
                    );                                                        \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions                         \
                );                                                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id *                                                           \
               );                                                             \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V0";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V                                                         \
                     > &p =                                                   \
                        ::RCF::AllocateServerParameters<                      \
                            V                                                 \
                             >()(session);                                    \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    );                                                        \
            }

// RCF_METHOD_V0_DEF
#define RCF_METHOD_V0_DEF(R,func  )                                           \
    RCF_METHOD_V0_DEF_(R,func  , RCF_PP_CAT(rcf_interface_id_1_, func, R0, __LINE__), RCF_MAKE_UNIQUE_ID(func, R0), RCF_PP_CAT(rcf_interface_id_2_, func, R0, __LINE__))

#define RCF_METHOD_V0_DEF_(R,func  , interfaceId, funcId, genParms)           \
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions                                 \
        )                                                                     \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V                                                             \
                 ,                                                            \
                V,V,V,V,V,V,V,V,V,V,V,V,V,V,V >()(                            \
                    getClientStub()                                           \
                     ,                                                        \
                    V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V0");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId *                                                              \
        )                                                                     \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R1
//------------------------------------------------------------------------------

// RCF_METHOD_R1
#define RCF_METHOD_R1_INLINE(R,func , A1)                                     \
    RCF_METHOD_R1_INLINE_(R,func , A1, RCF_MAKE_UNIQUE_ID(func, R1))

#define RCF_METHOD_R1_INLINE_(R,func , A1, id)                                \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1);                                                      \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1 ,                                                  \
                        V,V,V,V,V,V,V,V,V,V,V,V,V,V >()(                      \
                            getClientStub() ,                                 \
                            a1 ,                                              \
                            V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R1");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R1";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1 > &p =                                                 \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1 >()(session);                                      \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get()));                                         \
            }

// RCF_METHOD_R1_DECL
#define RCF_METHOD_R1_DECL(R,func , A1)                                       \
    RCF_METHOD_R1_DECL_(R,func , A1, RCF_MAKE_UNIQUE_ID(func, R1))

#define RCF_METHOD_R1_DECL_(R,func , A1, id)                                  \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1);                                                      \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1);                               \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R1";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1 > &p =                                                 \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1 >()(session);                                      \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get()));                                         \
            }

// RCF_METHOD_R1_DEF
#define RCF_METHOD_R1_DEF(R,func , A1)                                        \
    RCF_METHOD_R1_DEF_(R,func , A1, RCF_PP_CAT(rcf_interface_id_1_, func, R1, __LINE__), RCF_MAKE_UNIQUE_ID(func, R1), RCF_PP_CAT(rcf_interface_id_2_, func, R1, __LINE__))

#define RCF_METHOD_R1_DEF_(R,func , A1, interfaceId, funcId, genParms)        \
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1)                                       \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1 ,                                                          \
                V,V,V,V,V,V,V,V,V,V,V,V,V,V >()(                              \
                    getClientStub() ,                                         \
                    a1 ,                                                      \
                    V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R1");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1)                                       \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V1
//------------------------------------------------------------------------------

// RCF_METHOD_V1
#define RCF_METHOD_V1_INLINE(R,func , A1)                                     \
    RCF_METHOD_V1_INLINE_(R,func  , A1, RCF_MAKE_UNIQUE_ID(func, V1))

#define RCF_METHOD_V1_INLINE_(R,func , A1, id)                                \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1);                                                      \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1 ,                                                  \
                        V,V,V,V,V,V,V,V,V,V,V,V,V,V >()(                      \
                            getClientStub() ,                                 \
                            a1 ,                                              \
                            V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V1");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V1";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1 > &p =                                                 \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1 >()(session);                                  \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get());                                              \
            }

// RCF_METHOD_V1_DECL
#define RCF_METHOD_V1_DECL(R,func , A1)                                       \
    RCF_METHOD_V1_DECL_(R,func  , A1, RCF_MAKE_UNIQUE_ID(func, V1))

#define RCF_METHOD_V1_DECL_(R,func , A1, id)                                  \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1);                                                      \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1);                               \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V1";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1 > &p =                                                 \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1 >()(session);                                  \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get());                                              \
            }

// RCF_METHOD_V1_DEF
#define RCF_METHOD_V1_DEF(R,func , A1)                                        \
    RCF_METHOD_V1_DEF_(R,func , A1, RCF_PP_CAT(rcf_interface_id_1_, func, R1, __LINE__), RCF_MAKE_UNIQUE_ID(func, R1), RCF_PP_CAT(rcf_interface_id_2_, func, R1, __LINE__))

#define RCF_METHOD_V1_DEF_(R,func , A1, interfaceId, funcId, genParms)        \
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1)                                       \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1 ,                                                          \
                V,V,V,V,V,V,V,V,V,V,V,V,V,V >()(                              \
                    getClientStub() ,                                         \
                    a1 ,                                                      \
                    V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V1");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1)                                       \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R2
//------------------------------------------------------------------------------

// RCF_METHOD_R2
#define RCF_METHOD_R2_INLINE(R,func , A1,A2)                                  \
    RCF_METHOD_R2_INLINE_(R,func , A1,A2, RCF_MAKE_UNIQUE_ID(func, R2))

#define RCF_METHOD_R2_INLINE_(R,func , A1,A2, id)                             \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2);                                                   \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2 ,                                               \
                        V,V,V,V,V,V,V,V,V,V,V,V,V >()(                        \
                            getClientStub() ,                                 \
                            a1,a2 ,                                           \
                            V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R2");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R2";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2 > &p =                                              \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2 >()(session);                                   \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get()));                                         \
            }

// RCF_METHOD_R2_DECL
#define RCF_METHOD_R2_DECL(R,func , A1,A2)                                    \
    RCF_METHOD_R2_DECL_(R,func , A1,A2, RCF_MAKE_UNIQUE_ID(func, R2))

#define RCF_METHOD_R2_DECL_(R,func , A1,A2, id)                               \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2);                                                   \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R2";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2 > &p =                                              \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2 >()(session);                                   \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get()));                                         \
            }

// RCF_METHOD_R2_DEF
#define RCF_METHOD_R2_DEF(R,func , A1,A2)                                     \
    RCF_METHOD_R2_DEF_(R,func , A1,A2, RCF_PP_CAT(rcf_interface_id_1_, func, R2, __LINE__), RCF_MAKE_UNIQUE_ID(func, R2), RCF_PP_CAT(rcf_interface_id_2_, func, R2, __LINE__))

#define RCF_METHOD_R2_DEF_(R,func , A1,A2, interfaceId, funcId, genParms)     \
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2)                               \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2 ,                                                       \
                V,V,V,V,V,V,V,V,V,V,V,V,V >()(                                \
                    getClientStub() ,                                         \
                    a1,a2 ,                                                   \
                    V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R2");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2)                               \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V2
//------------------------------------------------------------------------------

// RCF_METHOD_V2
#define RCF_METHOD_V2_INLINE(R,func , A1,A2)                                  \
    RCF_METHOD_V2_INLINE_(R,func  , A1,A2, RCF_MAKE_UNIQUE_ID(func, V2))

#define RCF_METHOD_V2_INLINE_(R,func , A1,A2, id)                             \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2);                                                   \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2 ,                                               \
                        V,V,V,V,V,V,V,V,V,V,V,V,V >()(                        \
                            getClientStub() ,                                 \
                            a1,a2 ,                                           \
                            V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V2");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V2";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2 > &p =                                              \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2 >()(session);                               \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get());                                          \
            }

// RCF_METHOD_V2_DECL
#define RCF_METHOD_V2_DECL(R,func , A1,A2)                                    \
    RCF_METHOD_V2_DECL_(R,func  , A1,A2, RCF_MAKE_UNIQUE_ID(func, V2))

#define RCF_METHOD_V2_DECL_(R,func , A1,A2, id)                               \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2);                                                   \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V2";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2 > &p =                                              \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2 >()(session);                               \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get());                                          \
            }

// RCF_METHOD_V2_DEF
#define RCF_METHOD_V2_DEF(R,func , A1,A2)                                     \
    RCF_METHOD_V2_DEF_(R,func , A1,A2, RCF_PP_CAT(rcf_interface_id_1_, func, R2, __LINE__), RCF_MAKE_UNIQUE_ID(func, R2), RCF_PP_CAT(rcf_interface_id_2_, func, R2, __LINE__))

#define RCF_METHOD_V2_DEF_(R,func , A1,A2, interfaceId, funcId, genParms)     \
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2)                               \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2 ,                                                       \
                V,V,V,V,V,V,V,V,V,V,V,V,V >()(                                \
                    getClientStub() ,                                         \
                    a1,a2 ,                                                   \
                    V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V2");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2)                               \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R3
//------------------------------------------------------------------------------

// RCF_METHOD_R3
#define RCF_METHOD_R3_INLINE(R,func , A1,A2,A3)                               \
    RCF_METHOD_R3_INLINE_(R,func , A1,A2,A3, RCF_MAKE_UNIQUE_ID(func, R3))

#define RCF_METHOD_R3_INLINE_(R,func , A1,A2,A3, id)                          \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3);                                                \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2,A3 ,                                            \
                        V,V,V,V,V,V,V,V,V,V,V,V >()(                          \
                            getClientStub() ,                                 \
                            a1,a2,a3 ,                                        \
                            V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R3");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R3";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3 > &p =                                           \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3 >()(session);                                \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get()));                                         \
            }

// RCF_METHOD_R3_DECL
#define RCF_METHOD_R3_DECL(R,func , A1,A2,A3)                                 \
    RCF_METHOD_R3_DECL_(R,func , A1,A2,A3, RCF_MAKE_UNIQUE_ID(func, R3))

#define RCF_METHOD_R3_DECL_(R,func , A1,A2,A3, id)                            \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3);                                                \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R3";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3 > &p =                                           \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3 >()(session);                                \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get()));                                         \
            }

// RCF_METHOD_R3_DEF
#define RCF_METHOD_R3_DEF(R,func , A1,A2,A3)                                  \
    RCF_METHOD_R3_DEF_(R,func , A1,A2,A3, RCF_PP_CAT(rcf_interface_id_1_, func, R3, __LINE__), RCF_MAKE_UNIQUE_ID(func, R3), RCF_PP_CAT(rcf_interface_id_2_, func, R3, __LINE__))

#define RCF_METHOD_R3_DEF_(R,func , A1,A2,A3, interfaceId, funcId, genParms)  \
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3)                               \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2,A3 ,                                                    \
                V,V,V,V,V,V,V,V,V,V,V,V >()(                                  \
                    getClientStub() ,                                         \
                    a1,a2,a3 ,                                                \
                    V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(), \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R3");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3)                               \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V3
//------------------------------------------------------------------------------

// RCF_METHOD_V3
#define RCF_METHOD_V3_INLINE(R,func , A1,A2,A3)                               \
    RCF_METHOD_V3_INLINE_(R,func  , A1,A2,A3, RCF_MAKE_UNIQUE_ID(func, V3))

#define RCF_METHOD_V3_INLINE_(R,func , A1,A2,A3, id)                          \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3);                                                \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2,A3 ,                                            \
                        V,V,V,V,V,V,V,V,V,V,V,V >()(                          \
                            getClientStub() ,                                 \
                            a1,a2,a3 ,                                        \
                            V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V3");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V3";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3 > &p =                                           \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3 >()(session);                            \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get());                                          \
            }

// RCF_METHOD_V3_DECL
#define RCF_METHOD_V3_DECL(R,func , A1,A2,A3)                                 \
    RCF_METHOD_V3_DECL_(R,func  , A1,A2,A3, RCF_MAKE_UNIQUE_ID(func, V3))

#define RCF_METHOD_V3_DECL_(R,func , A1,A2,A3, id)                            \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3);                                                \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V3";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3 > &p =                                           \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3 >()(session);                            \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get());                                          \
            }

// RCF_METHOD_V3_DEF
#define RCF_METHOD_V3_DEF(R,func , A1,A2,A3)                                  \
    RCF_METHOD_V3_DEF_(R,func , A1,A2,A3, RCF_PP_CAT(rcf_interface_id_1_, func, R3, __LINE__), RCF_MAKE_UNIQUE_ID(func, R3), RCF_PP_CAT(rcf_interface_id_2_, func, R3, __LINE__))

#define RCF_METHOD_V3_DEF_(R,func , A1,A2,A3, interfaceId, funcId, genParms)  \
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3)                               \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2,A3 ,                                                    \
                V,V,V,V,V,V,V,V,V,V,V,V >()(                                  \
                    getClientStub() ,                                         \
                    a1,a2,a3 ,                                                \
                    V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(), \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V3");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3)                               \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R4
//------------------------------------------------------------------------------

// RCF_METHOD_R4
#define RCF_METHOD_R4_INLINE(R,func , A1,A2,A3,A4)                            \
    RCF_METHOD_R4_INLINE_(R,func , A1,A2,A3,A4, RCF_MAKE_UNIQUE_ID(func, R4))

#define RCF_METHOD_R4_INLINE_(R,func , A1,A2,A3,A4, id)                       \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4);                                             \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4 ,                                         \
                        V,V,V,V,V,V,V,V,V,V,V >()(                            \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4 ,                                     \
                            V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R4");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R4";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4 > &p =                                        \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4 >()(session);                             \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get()));                                         \
            }

// RCF_METHOD_R4_DECL
#define RCF_METHOD_R4_DECL(R,func , A1,A2,A3,A4)                              \
    RCF_METHOD_R4_DECL_(R,func , A1,A2,A3,A4, RCF_MAKE_UNIQUE_ID(func, R4))

#define RCF_METHOD_R4_DECL_(R,func , A1,A2,A3,A4, id)                         \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4);                                             \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R4";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4 > &p =                                        \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4 >()(session);                             \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get()));                                         \
            }

// RCF_METHOD_R4_DEF
#define RCF_METHOD_R4_DEF(R,func , A1,A2,A3,A4)                               \
    RCF_METHOD_R4_DEF_(R,func , A1,A2,A3,A4, RCF_PP_CAT(rcf_interface_id_1_, func, R4, __LINE__), RCF_MAKE_UNIQUE_ID(func, R4), RCF_PP_CAT(rcf_interface_id_2_, func, R4, __LINE__))

#define RCF_METHOD_R4_DEF_(R,func , A1,A2,A3,A4, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4)                               \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2,A3,A4 ,                                                 \
                V,V,V,V,V,V,V,V,V,V,V >()(                                    \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4 ,                                             \
                    V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),     \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R4");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4)                               \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V4
//------------------------------------------------------------------------------

// RCF_METHOD_V4
#define RCF_METHOD_V4_INLINE(R,func , A1,A2,A3,A4)                            \
    RCF_METHOD_V4_INLINE_(R,func  , A1,A2,A3,A4, RCF_MAKE_UNIQUE_ID(func, V4))

#define RCF_METHOD_V4_INLINE_(R,func , A1,A2,A3,A4, id)                       \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4);                                             \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2,A3,A4 ,                                         \
                        V,V,V,V,V,V,V,V,V,V,V >()(                            \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4 ,                                     \
                            V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),\
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V4");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V4";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4 > &p =                                        \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4 >()(session);                         \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get());                                          \
            }

// RCF_METHOD_V4_DECL
#define RCF_METHOD_V4_DECL(R,func , A1,A2,A3,A4)                              \
    RCF_METHOD_V4_DECL_(R,func  , A1,A2,A3,A4, RCF_MAKE_UNIQUE_ID(func, V4))

#define RCF_METHOD_V4_DECL_(R,func , A1,A2,A3,A4, id)                         \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4);                                             \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V4";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4 > &p =                                        \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4 >()(session);                         \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get());                                          \
            }

// RCF_METHOD_V4_DEF
#define RCF_METHOD_V4_DEF(R,func , A1,A2,A3,A4)                               \
    RCF_METHOD_V4_DEF_(R,func , A1,A2,A3,A4, RCF_PP_CAT(rcf_interface_id_1_, func, R4, __LINE__), RCF_MAKE_UNIQUE_ID(func, R4), RCF_PP_CAT(rcf_interface_id_2_, func, R4, __LINE__))

#define RCF_METHOD_V4_DEF_(R,func , A1,A2,A3,A4, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4)                               \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2,A3,A4 ,                                                 \
                V,V,V,V,V,V,V,V,V,V,V >()(                                    \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4 ,                                             \
                    V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),     \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V4");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4)                               \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R5
//------------------------------------------------------------------------------

// RCF_METHOD_R5
#define RCF_METHOD_R5_INLINE(R,func , A1,A2,A3,A4,A5)                         \
    RCF_METHOD_R5_INLINE_(R,func , A1,A2,A3,A4,A5, RCF_MAKE_UNIQUE_ID(func, R5))

#define RCF_METHOD_R5_INLINE_(R,func , A1,A2,A3,A4,A5, id)                    \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5);                                          \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5 ,                                      \
                        V,V,V,V,V,V,V,V,V,V >()(                              \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5 ,                                  \
                            V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(), \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R5");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R5";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5 > &p =                                     \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5 >()(session);                          \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get()));                                         \
            }

// RCF_METHOD_R5_DECL
#define RCF_METHOD_R5_DECL(R,func , A1,A2,A3,A4,A5)                           \
    RCF_METHOD_R5_DECL_(R,func , A1,A2,A3,A4,A5, RCF_MAKE_UNIQUE_ID(func, R5))

#define RCF_METHOD_R5_DECL_(R,func , A1,A2,A3,A4,A5, id)                      \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5);                                          \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R5";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5 > &p =                                     \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5 >()(session);                          \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get()));                                         \
            }

// RCF_METHOD_R5_DEF
#define RCF_METHOD_R5_DEF(R,func , A1,A2,A3,A4,A5)                            \
    RCF_METHOD_R5_DEF_(R,func , A1,A2,A3,A4,A5, RCF_PP_CAT(rcf_interface_id_1_, func, R5, __LINE__), RCF_MAKE_UNIQUE_ID(func, R5), RCF_PP_CAT(rcf_interface_id_2_, func, R5, __LINE__))

#define RCF_METHOD_R5_DEF_(R,func , A1,A2,A3,A4,A5, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5)                               \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2,A3,A4,A5 ,                                              \
                V,V,V,V,V,V,V,V,V,V >()(                                      \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5 ,                                          \
                    V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),         \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R5");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5)                               \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V5
//------------------------------------------------------------------------------

// RCF_METHOD_V5
#define RCF_METHOD_V5_INLINE(R,func , A1,A2,A3,A4,A5)                         \
    RCF_METHOD_V5_INLINE_(R,func  , A1,A2,A3,A4,A5, RCF_MAKE_UNIQUE_ID(func, V5))

#define RCF_METHOD_V5_INLINE_(R,func , A1,A2,A3,A4,A5, id)                    \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5);                                          \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2,A3,A4,A5 ,                                      \
                        V,V,V,V,V,V,V,V,V,V >()(                              \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5 ,                                  \
                            V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(), \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V5");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V5";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5 > &p =                                     \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5 >()(session);                      \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get());                                          \
            }

// RCF_METHOD_V5_DECL
#define RCF_METHOD_V5_DECL(R,func , A1,A2,A3,A4,A5)                           \
    RCF_METHOD_V5_DECL_(R,func  , A1,A2,A3,A4,A5, RCF_MAKE_UNIQUE_ID(func, V5))

#define RCF_METHOD_V5_DECL_(R,func , A1,A2,A3,A4,A5, id)                      \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5);                                          \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V5";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5 > &p =                                     \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5 >()(session);                      \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get());                                          \
            }

// RCF_METHOD_V5_DEF
#define RCF_METHOD_V5_DEF(R,func , A1,A2,A3,A4,A5)                            \
    RCF_METHOD_V5_DEF_(R,func , A1,A2,A3,A4,A5, RCF_PP_CAT(rcf_interface_id_1_, func, R5, __LINE__), RCF_MAKE_UNIQUE_ID(func, R5), RCF_PP_CAT(rcf_interface_id_2_, func, R5, __LINE__))

#define RCF_METHOD_V5_DEF_(R,func , A1,A2,A3,A4,A5, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5)                               \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2,A3,A4,A5 ,                                              \
                V,V,V,V,V,V,V,V,V,V >()(                                      \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5 ,                                          \
                    V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),         \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V5");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5)                               \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R6
//------------------------------------------------------------------------------

// RCF_METHOD_R6
#define RCF_METHOD_R6_INLINE(R,func , A1,A2,A3,A4,A5,A6)                      \
    RCF_METHOD_R6_INLINE_(R,func , A1,A2,A3,A4,A5,A6, RCF_MAKE_UNIQUE_ID(func, R6))

#define RCF_METHOD_R6_INLINE_(R,func , A1,A2,A3,A4,A5,A6, id)                 \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6);                                       \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6 ,                                   \
                        V,V,V,V,V,V,V,V,V >()(                                \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6 ,                               \
                            V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),     \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R6");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R6";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6 > &p =                                  \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6 >()(session);                       \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get()));                                         \
            }

// RCF_METHOD_R6_DECL
#define RCF_METHOD_R6_DECL(R,func , A1,A2,A3,A4,A5,A6)                        \
    RCF_METHOD_R6_DECL_(R,func , A1,A2,A3,A4,A5,A6, RCF_MAKE_UNIQUE_ID(func, R6))

#define RCF_METHOD_R6_DECL_(R,func , A1,A2,A3,A4,A5,A6, id)                   \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6);                                       \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R6";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6 > &p =                                  \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6 >()(session);                       \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get()));                                         \
            }

// RCF_METHOD_R6_DEF
#define RCF_METHOD_R6_DEF(R,func , A1,A2,A3,A4,A5,A6)                         \
    RCF_METHOD_R6_DEF_(R,func , A1,A2,A3,A4,A5,A6, RCF_PP_CAT(rcf_interface_id_1_, func, R6, __LINE__), RCF_MAKE_UNIQUE_ID(func, R6), RCF_PP_CAT(rcf_interface_id_2_, func, R6, __LINE__))

#define RCF_METHOD_R6_DEF_(R,func , A1,A2,A3,A4,A5,A6, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6)                               \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2,A3,A4,A5,A6 ,                                           \
                V,V,V,V,V,V,V,V,V >()(                                        \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6 ,                                       \
                    V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),             \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R6");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6)                               \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V6
//------------------------------------------------------------------------------

// RCF_METHOD_V6
#define RCF_METHOD_V6_INLINE(R,func , A1,A2,A3,A4,A5,A6)                      \
    RCF_METHOD_V6_INLINE_(R,func  , A1,A2,A3,A4,A5,A6, RCF_MAKE_UNIQUE_ID(func, V6))

#define RCF_METHOD_V6_INLINE_(R,func , A1,A2,A3,A4,A5,A6, id)                 \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6);                                       \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2,A3,A4,A5,A6 ,                                   \
                        V,V,V,V,V,V,V,V,V >()(                                \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6 ,                               \
                            V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),     \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V6");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V6";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6 > &p =                                  \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6 >()(session);                   \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get());                                          \
            }

// RCF_METHOD_V6_DECL
#define RCF_METHOD_V6_DECL(R,func , A1,A2,A3,A4,A5,A6)                        \
    RCF_METHOD_V6_DECL_(R,func  , A1,A2,A3,A4,A5,A6, RCF_MAKE_UNIQUE_ID(func, V6))

#define RCF_METHOD_V6_DECL_(R,func , A1,A2,A3,A4,A5,A6, id)                   \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6);                                       \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V6";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6 > &p =                                  \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6 >()(session);                   \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get());                                          \
            }

// RCF_METHOD_V6_DEF
#define RCF_METHOD_V6_DEF(R,func , A1,A2,A3,A4,A5,A6)                         \
    RCF_METHOD_V6_DEF_(R,func , A1,A2,A3,A4,A5,A6, RCF_PP_CAT(rcf_interface_id_1_, func, R6, __LINE__), RCF_MAKE_UNIQUE_ID(func, R6), RCF_PP_CAT(rcf_interface_id_2_, func, R6, __LINE__))

#define RCF_METHOD_V6_DEF_(R,func , A1,A2,A3,A4,A5,A6, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6)                               \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2,A3,A4,A5,A6 ,                                           \
                V,V,V,V,V,V,V,V,V >()(                                        \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6 ,                                       \
                    V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),             \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V6");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6)                               \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R7
//------------------------------------------------------------------------------

// RCF_METHOD_R7
#define RCF_METHOD_R7_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7)                   \
    RCF_METHOD_R7_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7, RCF_MAKE_UNIQUE_ID(func, R7))

#define RCF_METHOD_R7_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7, id)              \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7);                                    \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7 ,                                \
                        V,V,V,V,V,V,V,V >()(                                  \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7 ,                            \
                            V(),V(),V(),V(),V(),V(),V(),V()).r.get(),         \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R7");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R7";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7 > &p =                               \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7 >()(session);                    \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get()));                                         \
            }

// RCF_METHOD_R7_DECL
#define RCF_METHOD_R7_DECL(R,func , A1,A2,A3,A4,A5,A6,A7)                     \
    RCF_METHOD_R7_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7, RCF_MAKE_UNIQUE_ID(func, R7))

#define RCF_METHOD_R7_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7, id)                \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7);                                    \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R7";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7 > &p =                               \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7 >()(session);                    \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get()));                                         \
            }

// RCF_METHOD_R7_DEF
#define RCF_METHOD_R7_DEF(R,func , A1,A2,A3,A4,A5,A6,A7)                      \
    RCF_METHOD_R7_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7, RCF_PP_CAT(rcf_interface_id_1_, func, R7, __LINE__), RCF_MAKE_UNIQUE_ID(func, R7), RCF_PP_CAT(rcf_interface_id_2_, func, R7, __LINE__))

#define RCF_METHOD_R7_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7)                               \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2,A3,A4,A5,A6,A7 ,                                        \
                V,V,V,V,V,V,V,V >()(                                          \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7 ,                                    \
                    V(),V(),V(),V(),V(),V(),V(),V()).r.get(),                 \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R7");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7)                               \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V7
//------------------------------------------------------------------------------

// RCF_METHOD_V7
#define RCF_METHOD_V7_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7)                   \
    RCF_METHOD_V7_INLINE_(R,func  , A1,A2,A3,A4,A5,A6,A7, RCF_MAKE_UNIQUE_ID(func, V7))

#define RCF_METHOD_V7_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7, id)              \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7);                                    \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7 ,                                \
                        V,V,V,V,V,V,V,V >()(                                  \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7 ,                            \
                            V(),V(),V(),V(),V(),V(),V(),V()).r.get(),         \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V7");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V7";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7 > &p =                               \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7 >()(session);                \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get());                                          \
            }

// RCF_METHOD_V7_DECL
#define RCF_METHOD_V7_DECL(R,func , A1,A2,A3,A4,A5,A6,A7)                     \
    RCF_METHOD_V7_DECL_(R,func  , A1,A2,A3,A4,A5,A6,A7, RCF_MAKE_UNIQUE_ID(func, V7))

#define RCF_METHOD_V7_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7, id)                \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7);                                    \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V7";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7 > &p =                               \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7 >()(session);                \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get());                                          \
            }

// RCF_METHOD_V7_DEF
#define RCF_METHOD_V7_DEF(R,func , A1,A2,A3,A4,A5,A6,A7)                      \
    RCF_METHOD_V7_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7, RCF_PP_CAT(rcf_interface_id_1_, func, R7, __LINE__), RCF_MAKE_UNIQUE_ID(func, R7), RCF_PP_CAT(rcf_interface_id_2_, func, R7, __LINE__))

#define RCF_METHOD_V7_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7)                               \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2,A3,A4,A5,A6,A7 ,                                        \
                V,V,V,V,V,V,V,V >()(                                          \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7 ,                                    \
                    V(),V(),V(),V(),V(),V(),V(),V()).r.get(),                 \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V7");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7)                               \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R8
//------------------------------------------------------------------------------

// RCF_METHOD_R8
#define RCF_METHOD_R8_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8)                \
    RCF_METHOD_R8_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8, RCF_MAKE_UNIQUE_ID(func, R8))

#define RCF_METHOD_R8_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8, id)           \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8);                                 \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8 ,                             \
                        V,V,V,V,V,V,V >()(                                    \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8 ,                         \
                            V(),V(),V(),V(),V(),V(),V()).r.get(),             \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R8");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R8";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8 > &p =                            \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8 >()(session);                 \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get()));                                         \
            }

// RCF_METHOD_R8_DECL
#define RCF_METHOD_R8_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8)                  \
    RCF_METHOD_R8_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8, RCF_MAKE_UNIQUE_ID(func, R8))

#define RCF_METHOD_R8_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8, id)             \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8);                                 \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R8";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8 > &p =                            \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8 >()(session);                 \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get()));                                         \
            }

// RCF_METHOD_R8_DEF
#define RCF_METHOD_R8_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8)                   \
    RCF_METHOD_R8_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8, RCF_PP_CAT(rcf_interface_id_1_, func, R8, __LINE__), RCF_MAKE_UNIQUE_ID(func, R8), RCF_PP_CAT(rcf_interface_id_2_, func, R8, __LINE__))

#define RCF_METHOD_R8_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8)                               \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8 ,                                     \
                V,V,V,V,V,V,V >()(                                            \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8 ,                                 \
                    V(),V(),V(),V(),V(),V(),V()).r.get(),                     \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R8");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8)                               \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V8
//------------------------------------------------------------------------------

// RCF_METHOD_V8
#define RCF_METHOD_V8_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8)                \
    RCF_METHOD_V8_INLINE_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8, RCF_MAKE_UNIQUE_ID(func, V8))

#define RCF_METHOD_V8_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8, id)           \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8);                                 \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8 ,                             \
                        V,V,V,V,V,V,V >()(                                    \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8 ,                         \
                            V(),V(),V(),V(),V(),V(),V()).r.get(),             \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V8");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V8";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8 > &p =                            \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8 >()(session);             \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get());                                          \
            }

// RCF_METHOD_V8_DECL
#define RCF_METHOD_V8_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8)                  \
    RCF_METHOD_V8_DECL_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8, RCF_MAKE_UNIQUE_ID(func, V8))

#define RCF_METHOD_V8_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8, id)             \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8);                                 \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V8";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8 > &p =                            \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8 >()(session);             \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get());                                          \
            }

// RCF_METHOD_V8_DEF
#define RCF_METHOD_V8_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8)                   \
    RCF_METHOD_V8_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8, RCF_PP_CAT(rcf_interface_id_1_, func, R8, __LINE__), RCF_MAKE_UNIQUE_ID(func, R8), RCF_PP_CAT(rcf_interface_id_2_, func, R8, __LINE__))

#define RCF_METHOD_V8_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8)                               \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8 ,                                     \
                V,V,V,V,V,V,V >()(                                            \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8 ,                                 \
                    V(),V(),V(),V(),V(),V(),V()).r.get(),                     \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V8");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8)                               \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R9
//------------------------------------------------------------------------------

// RCF_METHOD_R9
#define RCF_METHOD_R9_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9)             \
    RCF_METHOD_R9_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9, RCF_MAKE_UNIQUE_ID(func, R9))

#define RCF_METHOD_R9_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9, id)        \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9);                              \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9 ,                          \
                        V,V,V,V,V,V >()(                                      \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9 ,                      \
                            V(),V(),V(),V(),V(),V()).r.get(),                 \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R9");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R9";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9 > &p =                         \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9 >()(session);              \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get()));                                         \
            }

// RCF_METHOD_R9_DECL
#define RCF_METHOD_R9_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9)               \
    RCF_METHOD_R9_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9, RCF_MAKE_UNIQUE_ID(func, R9))

#define RCF_METHOD_R9_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9, id)          \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9);                              \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R9";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9 > &p =                         \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9 >()(session);              \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get()));                                         \
            }

// RCF_METHOD_R9_DEF
#define RCF_METHOD_R9_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9)                \
    RCF_METHOD_R9_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9, RCF_PP_CAT(rcf_interface_id_1_, func, R9, __LINE__), RCF_MAKE_UNIQUE_ID(func, R9), RCF_PP_CAT(rcf_interface_id_2_, func, R9, __LINE__))

#define RCF_METHOD_R9_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9)                               \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9 ,                                  \
                V,V,V,V,V,V >()(                                              \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9 ,                              \
                    V(),V(),V(),V(),V(),V()).r.get(),                         \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R9");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9)                               \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V9
//------------------------------------------------------------------------------

// RCF_METHOD_V9
#define RCF_METHOD_V9_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9)             \
    RCF_METHOD_V9_INLINE_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9, RCF_MAKE_UNIQUE_ID(func, V9))

#define RCF_METHOD_V9_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9, id)        \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9);                              \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9)                               \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9 ,                          \
                        V,V,V,V,V,V >()(                                      \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9 ,                      \
                            V(),V(),V(),V(),V(),V()).r.get(),                 \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V9");                                                    \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V9";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9 > &p =                         \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9 >()(session);          \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get());                                          \
            }

// RCF_METHOD_V9_DECL
#define RCF_METHOD_V9_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9)               \
    RCF_METHOD_V9_DECL_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9, RCF_MAKE_UNIQUE_ID(func, V9))

#define RCF_METHOD_V9_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9, id)          \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9)                               \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9);                              \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9);                              \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9);                              \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V9";                                                  \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9 > &p =                         \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9 >()(session);          \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get());                                          \
            }

// RCF_METHOD_V9_DEF
#define RCF_METHOD_V9_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9)                \
    RCF_METHOD_V9_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9, RCF_PP_CAT(rcf_interface_id_1_, func, R9, __LINE__), RCF_MAKE_UNIQUE_ID(func, R9), RCF_PP_CAT(rcf_interface_id_2_, func, R9, __LINE__))

#define RCF_METHOD_V9_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9)                               \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9 ,                                  \
                V,V,V,V,V,V >()(                                              \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9 ,                              \
                    V(),V(),V(),V(),V(),V()).r.get(),                         \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V9");                                                            \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9)                               \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R10
//------------------------------------------------------------------------------

// RCF_METHOD_R10
#define RCF_METHOD_R10_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10)        \
    RCF_METHOD_R10_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10, RCF_MAKE_UNIQUE_ID(func, R10))

#define RCF_METHOD_R10_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10, id)   \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);                          \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10)                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10 ,                      \
                        V,V,V,V,V >()(                                        \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9,a10 ,                  \
                            V(),V(),V(),V(),V()).r.get(),                     \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R10");                                                   \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R10";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10 > &p =                     \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10 >()(session);          \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get()));                                        \
            }

// RCF_METHOD_R10_DECL
#define RCF_METHOD_R10_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10)          \
    RCF_METHOD_R10_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10, RCF_MAKE_UNIQUE_ID(func, R10))

#define RCF_METHOD_R10_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10, id)     \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);                          \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10);                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10);                            \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R10";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10 > &p =                     \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10 >()(session);          \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get()));                                        \
            }

// RCF_METHOD_R10_DEF
#define RCF_METHOD_R10_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10)           \
    RCF_METHOD_R10_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10, RCF_PP_CAT(rcf_interface_id_1_, func, R10, __LINE__), RCF_MAKE_UNIQUE_ID(func, R10), RCF_PP_CAT(rcf_interface_id_2_, func, R10, __LINE__))

#define RCF_METHOD_R10_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10)                             \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9,A10 ,                              \
                V,V,V,V,V >()(                                                \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10 ,                          \
                    V(),V(),V(),V(),V()).r.get(),                             \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R10");                                                           \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10)                             \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V10
//------------------------------------------------------------------------------

// RCF_METHOD_V10
#define RCF_METHOD_V10_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10)        \
    RCF_METHOD_V10_INLINE_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10, RCF_MAKE_UNIQUE_ID(func, V10))

#define RCF_METHOD_V10_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10, id)   \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);                          \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10)                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10 ,                      \
                        V,V,V,V,V >()(                                        \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9,a10 ,                  \
                            V(),V(),V(),V(),V()).r.get(),                     \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V10");                                                   \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V10";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10 > &p =                     \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9,A10 >()(session);      \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get());                                         \
            }

// RCF_METHOD_V10_DECL
#define RCF_METHOD_V10_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10)          \
    RCF_METHOD_V10_DECL_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10, RCF_MAKE_UNIQUE_ID(func, V10))

#define RCF_METHOD_V10_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10, id)     \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);                          \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10);                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10);                            \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V10";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10 > &p =                     \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9,A10 >()(session);      \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get());                                         \
            }

// RCF_METHOD_V10_DEF
#define RCF_METHOD_V10_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10)           \
    RCF_METHOD_V10_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10, RCF_PP_CAT(rcf_interface_id_1_, func, R10, __LINE__), RCF_MAKE_UNIQUE_ID(func, R10), RCF_PP_CAT(rcf_interface_id_2_, func, R10, __LINE__))

#define RCF_METHOD_V10_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10)                             \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9,A10 ,                              \
                V,V,V,V,V >()(                                                \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10 ,                          \
                    V(),V(),V(),V(),V()).r.get(),                             \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V10");                                                           \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10)                             \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R11
//------------------------------------------------------------------------------

// RCF_METHOD_R11
#define RCF_METHOD_R11_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11)    \
    RCF_METHOD_R11_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11, RCF_MAKE_UNIQUE_ID(func, R11))

#define RCF_METHOD_R11_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11, id)\
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11);                      \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11)                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11 ,                  \
                        V,V,V,V >()(                                          \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11 ,              \
                            V(),V(),V(),V()).r.get(),                         \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R11");                                                   \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R11";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11 > &p =                 \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11 >()(session);      \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get()));                                        \
            }

// RCF_METHOD_R11_DECL
#define RCF_METHOD_R11_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11)      \
    RCF_METHOD_R11_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11, RCF_MAKE_UNIQUE_ID(func, R11))

#define RCF_METHOD_R11_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11, id) \
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11);                      \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11);                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11);                            \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R11";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11 > &p =                 \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11 >()(session);      \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get()));                                        \
            }

// RCF_METHOD_R11_DEF
#define RCF_METHOD_R11_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11)       \
    RCF_METHOD_R11_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11, RCF_PP_CAT(rcf_interface_id_1_, func, R11, __LINE__), RCF_MAKE_UNIQUE_ID(func, R11), RCF_PP_CAT(rcf_interface_id_2_, func, R11, __LINE__))

#define RCF_METHOD_R11_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11)                             \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11 ,                          \
                V,V,V,V >()(                                                  \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11 ,                      \
                    V(),V(),V(),V()).r.get(),                                 \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R11");                                                           \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11)                             \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V11
//------------------------------------------------------------------------------

// RCF_METHOD_V11
#define RCF_METHOD_V11_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11)    \
    RCF_METHOD_V11_INLINE_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11, RCF_MAKE_UNIQUE_ID(func, V11))

#define RCF_METHOD_V11_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11, id)\
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11);                      \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11)                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11 ,                  \
                        V,V,V,V >()(                                          \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11 ,              \
                            V(),V(),V(),V()).r.get(),                         \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V11");                                                   \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V11";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11 > &p =                 \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11 >()(session);  \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get());                                         \
            }

// RCF_METHOD_V11_DECL
#define RCF_METHOD_V11_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11)      \
    RCF_METHOD_V11_DECL_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11, RCF_MAKE_UNIQUE_ID(func, V11))

#define RCF_METHOD_V11_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11, id) \
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11);                      \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11);                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11);                            \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V11";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11 > &p =                 \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11 >()(session);  \
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get());                                         \
            }

// RCF_METHOD_V11_DEF
#define RCF_METHOD_V11_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11)       \
    RCF_METHOD_V11_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11, RCF_PP_CAT(rcf_interface_id_1_, func, R11, __LINE__), RCF_MAKE_UNIQUE_ID(func, R11), RCF_PP_CAT(rcf_interface_id_2_, func, R11, __LINE__))

#define RCF_METHOD_V11_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11)                             \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11 ,                          \
                V,V,V,V >()(                                                  \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11 ,                      \
                    V(),V(),V(),V()).r.get(),                                 \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V11");                                                           \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11)                             \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R12
//------------------------------------------------------------------------------

// RCF_METHOD_R12
#define RCF_METHOD_R12_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12)\
    RCF_METHOD_R12_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12, RCF_MAKE_UNIQUE_ID(func, R12))

#define RCF_METHOD_R12_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12, id)\
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12);                  \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12)                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12 ,              \
                        V,V,V >()(                                            \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12 ,          \
                            V(),V(),V()).r.get(),                             \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R12");                                                   \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R12";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12 > &p =             \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12 >()(session);  \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get()));                                        \
            }

// RCF_METHOD_R12_DECL
#define RCF_METHOD_R12_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12)  \
    RCF_METHOD_R12_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12, RCF_MAKE_UNIQUE_ID(func, R12))

#define RCF_METHOD_R12_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12, id)\
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12);                  \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12);                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12);                            \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R12";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12 > &p =             \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12 >()(session);  \
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get()));                                        \
            }

// RCF_METHOD_R12_DEF
#define RCF_METHOD_R12_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12)   \
    RCF_METHOD_R12_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12, RCF_PP_CAT(rcf_interface_id_1_, func, R12, __LINE__), RCF_MAKE_UNIQUE_ID(func, R12), RCF_PP_CAT(rcf_interface_id_2_, func, R12, __LINE__))

#define RCF_METHOD_R12_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12)                             \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12 ,                      \
                V,V,V >()(                                                    \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12 ,                  \
                    V(),V(),V()).r.get(),                                     \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R12");                                                           \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12)                             \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V12
//------------------------------------------------------------------------------

// RCF_METHOD_V12
#define RCF_METHOD_V12_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12)\
    RCF_METHOD_V12_INLINE_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12, RCF_MAKE_UNIQUE_ID(func, V12))

#define RCF_METHOD_V12_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12, id)\
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12);                  \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12)                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12 ,              \
                        V,V,V >()(                                            \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12 ,          \
                            V(),V(),V()).r.get(),                             \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V12");                                                   \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V12";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12 > &p =             \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12 >()(session);\
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get());                                         \
            }

// RCF_METHOD_V12_DECL
#define RCF_METHOD_V12_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12)  \
    RCF_METHOD_V12_DECL_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12, RCF_MAKE_UNIQUE_ID(func, V12))

#define RCF_METHOD_V12_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12, id)\
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12);                  \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12);                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12);                            \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V12";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12 > &p =             \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12 >()(session);\
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get());                                         \
            }

// RCF_METHOD_V12_DEF
#define RCF_METHOD_V12_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12)   \
    RCF_METHOD_V12_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12, RCF_PP_CAT(rcf_interface_id_1_, func, R12, __LINE__), RCF_MAKE_UNIQUE_ID(func, R12), RCF_PP_CAT(rcf_interface_id_2_, func, R12, __LINE__))

#define RCF_METHOD_V12_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12)                             \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12 ,                      \
                V,V,V >()(                                                    \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12 ,                  \
                    V(),V(),V()).r.get(),                                     \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V12");                                                           \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12)                             \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R13
//------------------------------------------------------------------------------

// RCF_METHOD_R13
#define RCF_METHOD_R13_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13)\
    RCF_METHOD_R13_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13, RCF_MAKE_UNIQUE_ID(func, R13))

#define RCF_METHOD_R13_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13, id)\
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13);              \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13)                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13 ,          \
                        V,V >()(                                              \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13 ,      \
                            V(),V()).r.get(),                                 \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R13");                                                   \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R13";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13 > &p =         \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13 >()(session);\
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get(),                                          \
                        p.a13.get()));                                        \
            }

// RCF_METHOD_R13_DECL
#define RCF_METHOD_R13_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13)\
    RCF_METHOD_R13_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13, RCF_MAKE_UNIQUE_ID(func, R13))

#define RCF_METHOD_R13_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13, id)\
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13);              \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13);                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13);                            \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R13";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13 > &p =         \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13 >()(session);\
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get(),                                          \
                        p.a13.get()));                                        \
            }

// RCF_METHOD_R13_DEF
#define RCF_METHOD_R13_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13)\
    RCF_METHOD_R13_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13, RCF_PP_CAT(rcf_interface_id_1_, func, R13, __LINE__), RCF_MAKE_UNIQUE_ID(func, R13), RCF_PP_CAT(rcf_interface_id_2_, func, R13, __LINE__))

#define RCF_METHOD_R13_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13)                             \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13 ,                  \
                V,V >()(                                                      \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13 ,              \
                    V(),V()).r.get(),                                         \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R13");                                                           \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13)                             \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V13
//------------------------------------------------------------------------------

// RCF_METHOD_V13
#define RCF_METHOD_V13_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13)\
    RCF_METHOD_V13_INLINE_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13, RCF_MAKE_UNIQUE_ID(func, V13))

#define RCF_METHOD_V13_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13, id)\
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13);              \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13)                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13 ,          \
                        V,V >()(                                              \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13 ,      \
                            V(),V()).r.get(),                                 \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V13");                                                   \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V13";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13 > &p =         \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13 >()(session);\
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get(),                                          \
                        p.a13.get());                                         \
            }

// RCF_METHOD_V13_DECL
#define RCF_METHOD_V13_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13)\
    RCF_METHOD_V13_DECL_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13, RCF_MAKE_UNIQUE_ID(func, V13))

#define RCF_METHOD_V13_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13, id)\
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13);              \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13);                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13);                            \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V13";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13 > &p =         \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13 >()(session);\
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get(),                                          \
                        p.a13.get());                                         \
            }

// RCF_METHOD_V13_DEF
#define RCF_METHOD_V13_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13)\
    RCF_METHOD_V13_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13, RCF_PP_CAT(rcf_interface_id_1_, func, R13, __LINE__), RCF_MAKE_UNIQUE_ID(func, R13), RCF_PP_CAT(rcf_interface_id_2_, func, R13, __LINE__))

#define RCF_METHOD_V13_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13)                             \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13 ,                  \
                V,V >()(                                                      \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13 ,              \
                    V(),V()).r.get(),                                         \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V13");                                                           \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13)                             \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R14
//------------------------------------------------------------------------------

// RCF_METHOD_R14
#define RCF_METHOD_R14_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14)\
    RCF_METHOD_R14_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14, RCF_MAKE_UNIQUE_ID(func, R14))

#define RCF_METHOD_R14_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14, id)\
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14);          \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14)                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14 ,      \
                        V >()(                                                \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14 ,  \
                            V()).r.get(),                                     \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R14");                                                   \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R14";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14 > &p =     \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14 >()(session);\
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get(),                                          \
                        p.a13.get(),                                          \
                        p.a14.get()));                                        \
            }

// RCF_METHOD_R14_DECL
#define RCF_METHOD_R14_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14)\
    RCF_METHOD_R14_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14, RCF_MAKE_UNIQUE_ID(func, R14))

#define RCF_METHOD_R14_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14, id)\
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14);          \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14);                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14);                            \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R14";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14 > &p =     \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14 >()(session);\
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get(),                                          \
                        p.a13.get(),                                          \
                        p.a14.get()));                                        \
            }

// RCF_METHOD_R14_DEF
#define RCF_METHOD_R14_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14)\
    RCF_METHOD_R14_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14, RCF_PP_CAT(rcf_interface_id_1_, func, R14, __LINE__), RCF_MAKE_UNIQUE_ID(func, R14), RCF_PP_CAT(rcf_interface_id_2_, func, R14, __LINE__))

#define RCF_METHOD_R14_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14)                             \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14 ,              \
                V >()(                                                        \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14 ,          \
                    V()).r.get(),                                             \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R14");                                                           \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14)                             \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V14
//------------------------------------------------------------------------------

// RCF_METHOD_V14
#define RCF_METHOD_V14_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14)\
    RCF_METHOD_V14_INLINE_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14, RCF_MAKE_UNIQUE_ID(func, V14))

#define RCF_METHOD_V14_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14, id)\
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14);          \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14)                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14 ,      \
                        V >()(                                                \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14 ,  \
                            V()).r.get(),                                     \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V14");                                                   \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V14";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14 > &p =     \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14 >()(session);\
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get(),                                          \
                        p.a13.get(),                                          \
                        p.a14.get());                                         \
            }

// RCF_METHOD_V14_DECL
#define RCF_METHOD_V14_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14)\
    RCF_METHOD_V14_DECL_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14, RCF_MAKE_UNIQUE_ID(func, V14))

#define RCF_METHOD_V14_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14, id)\
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14);          \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14);                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14);                            \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V14";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14 > &p =     \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14 >()(session);\
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get(),                                          \
                        p.a13.get(),                                          \
                        p.a14.get());                                         \
            }

// RCF_METHOD_V14_DEF
#define RCF_METHOD_V14_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14)\
    RCF_METHOD_V14_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14, RCF_PP_CAT(rcf_interface_id_1_, func, R14, __LINE__), RCF_MAKE_UNIQUE_ID(func, R14), RCF_PP_CAT(rcf_interface_id_2_, func, R14, __LINE__))

#define RCF_METHOD_V14_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14)                             \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14 ,              \
                V >()(                                                        \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14 ,          \
                    V()).r.get(),                                             \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V14");                                                           \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14)                             \
    {                                                                         \
    }




//------------------------------------------------------------------------------
// Parameters - R15
//------------------------------------------------------------------------------

// RCF_METHOD_R15
#define RCF_METHOD_R15_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15)\
    RCF_METHOD_R15_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15, RCF_MAKE_UNIQUE_ID(func, R15))

#define RCF_METHOD_R15_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15, id)\
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15);      \
            }                                                                 \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15)                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<R >(                                   \
                    ::RCF::AllocateClientParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15    \
                         >()(                                                 \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15 \
                            ).r.get(),                                        \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "R15");                                                   \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R15";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15 > &p = \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15 >()(session);\
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get(),                                          \
                        p.a13.get(),                                          \
                        p.a14.get(),                                          \
                        p.a15.get()));                                        \
            }

// RCF_METHOD_R15_DECL
#define RCF_METHOD_R15_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15)\
    RCF_METHOD_R15_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15, RCF_MAKE_UNIQUE_ID(func, R15))

#define RCF_METHOD_R15_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15, id)\
        public:                                                               \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<R > func(                                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15);      \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<R > func(                                       \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15);                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15);                            \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "R15";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    R ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15 > &p = \
                    ::RCF::AllocateServerParameters<                          \
                        R ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15 >()(session);\
                p.r.set(                                                      \
                    session.getAutoSend(),                                    \
                    t.func(                                                   \
                        p.a1.get(),                                           \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get(),                                          \
                        p.a13.get(),                                          \
                        p.a14.get(),                                          \
                        p.a15.get()));                                        \
            }

// RCF_METHOD_R15_DEF
#define RCF_METHOD_R15_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15)\
    RCF_METHOD_R15_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15, RCF_PP_CAT(rcf_interface_id_1_, func, R15, __LINE__), RCF_MAKE_UNIQUE_ID(func, R15), RCF_PP_CAT(rcf_interface_id_2_, func, R15, __LINE__))

#define RCF_METHOD_R15_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<R > genParms::RcfClientT::func(    \
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15)                             \
    {                                                                         \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<R >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                R ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15            \
                 >()(                                                         \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15        \
                    ).r.get(),                                                \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "R15");                                                           \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15)                             \
    {                                                                         \
    }



//------------------------------------------------------------------------------
// Parameters - V15
//------------------------------------------------------------------------------

// RCF_METHOD_V15
#define RCF_METHOD_V15_INLINE(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15)\
    RCF_METHOD_V15_INLINE_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15, RCF_MAKE_UNIQUE_ID(func, V15))

#define RCF_METHOD_V15_INLINE_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15, id)\
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15);      \
            }                                                                 \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15)                             \
            {                                                                 \
                getClientStub().setAsync(false);                              \
                return RCF::FutureImpl<V>(                                    \
                    ::RCF::AllocateClientParameters<                          \
                        V ,                                                   \
                        A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15    \
                         >()(                                                 \
                            getClientStub() ,                                 \
                            a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15 \
                            ).r.get(),                                        \
                    getClientStub(),                                          \
                    mInterfaceName,                                           \
                    id::value,                                                \
                    callOptions.apply(getClientStub()),                       \
                    #func,                                                    \
                    "V15");                                                   \
            }                                                                 \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V15";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15 > &p = \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15 >()(session);\
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get(),                                          \
                        p.a13.get(),                                          \
                        p.a14.get(),                                          \
                        p.a15.get());                                         \
            }

// RCF_METHOD_V15_DECL
#define RCF_METHOD_V15_DECL(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15)\
    RCF_METHOD_V15_DECL_(R,func  , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15, RCF_MAKE_UNIQUE_ID(func, V15))

#define RCF_METHOD_V15_DECL_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15, id)\
        public:                                                               \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));          \
            RCF_MAKE_NEXT_DISPATCH_ID(id)                                     \
            ::RCF::FutureImpl<V> func(                                        \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15)                             \
            {                                                                 \
                return func(                                                  \
                    ::RCF::CallOptions() ,                                    \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15);      \
            }                                                                 \
                                                                              \
            ::RCF::FutureImpl<V> func(                                        \
                const ::RCF::CallOptions &callOptions ,                       \
                ::RCF::RemoveOut<A1 >::type a1,                               \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15);                            \
                                                                              \
            void error__method_defined_out_of_order__##func(                  \
               id * ,                                                         \
               ::RCF::RemoveOut<A1 >::type a1,                                \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15);                            \
                                                                              \
            const char * getFunctionName(const id &)                          \
            {                                                                 \
                return #func;                                                 \
            }                                                                 \
            const char * getArity(const id &)                                 \
            {                                                                 \
                return "V15";                                                 \
            }                                                                 \
                                                                              \
        private:                                                              \
            template<typename T>                                              \
            void invoke(                                                      \
                const id &,                                                   \
                ::RCF::RcfSession &session,                                   \
                T &t)                                                         \
            {                                                                 \
                ::RCF::ServerParameters<                                      \
                    V ,                                                       \
                    A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15 > &p = \
                        ::RCF::AllocateServerParameters<                      \
                            V ,                                               \
                            A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15 >()(session);\
                RCF_UNUSED_VARIABLE(p);                                       \
                t.func(                                                       \
                    p.a1.get(),                                               \
                        p.a2.get(),                                           \
                        p.a3.get(),                                           \
                        p.a4.get(),                                           \
                        p.a5.get(),                                           \
                        p.a6.get(),                                           \
                        p.a7.get(),                                           \
                        p.a8.get(),                                           \
                        p.a9.get(),                                           \
                        p.a10.get(),                                          \
                        p.a11.get(),                                          \
                        p.a12.get(),                                          \
                        p.a13.get(),                                          \
                        p.a14.get(),                                          \
                        p.a15.get());                                         \
            }

// RCF_METHOD_V15_DEF
#define RCF_METHOD_V15_DEF(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15)\
    RCF_METHOD_V15_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15, RCF_PP_CAT(rcf_interface_id_1_, func, R15, __LINE__), RCF_MAKE_UNIQUE_ID(func, R15), RCF_PP_CAT(rcf_interface_id_2_, func, R15, __LINE__))

#define RCF_METHOD_V15_DEF_(R,func , A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15, interfaceId, funcId, genParms)\
    RCF_CURRENT_STATIC_ID(interfaceId, RCF_interface_id_helper, int, int)     \
    typedef GeneratorParms<interfaceId> genParms;                             \
    RCF_ADVANCE_STATIC_ID(funcId, RCF_def_dispatch_id_helper, ::RCF::Dummy<genParms::RcfClientT>, genParms::RcfClientT, static)\
    RCF_EXPORT_INTERFACE ::RCF::FutureImpl<::RCF::Void> genParms::RcfClientT::func(\
        const ::RCF::CallOptions &callOptions ,                               \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15)                             \
    {                                                                         \
        typedef ::RCF::Void V;                                                \
        getClientStub().setAsync(false);                                      \
        return RCF::FutureImpl<V >(                                           \
            ::RCF::AllocateClientParameters<                                  \
                V ,                                                           \
                A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15            \
                 >()(                                                         \
                    getClientStub() ,                                         \
                    a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15        \
                    ).r.get(),                                                \
            getClientStub(),                                                  \
            mInterfaceName,                                                   \
            funcId::value,                                                    \
            callOptions.apply(getClientStub()),                               \
            #func,                                                            \
            "V15");                                                           \
    }                                                                         \
    void genParms::RcfClientT::error__method_defined_out_of_order__##func(    \
        funcId * ,                                                            \
        ::RCF::RemoveOut<A1 >::type a1,                                       \
                ::RCF::RemoveOut<A2 >::type a2,                               \
                ::RCF::RemoveOut<A3 >::type a3,                               \
                ::RCF::RemoveOut<A4 >::type a4,                               \
                ::RCF::RemoveOut<A5 >::type a5,                               \
                ::RCF::RemoveOut<A6 >::type a6,                               \
                ::RCF::RemoveOut<A7 >::type a7,                               \
                ::RCF::RemoveOut<A8 >::type a8,                               \
                ::RCF::RemoveOut<A9 >::type a9,                               \
                ::RCF::RemoveOut<A10 >::type a10,                             \
                ::RCF::RemoveOut<A11 >::type a11,                             \
                ::RCF::RemoveOut<A12 >::type a12,                             \
                ::RCF::RemoveOut<A13 >::type a13,                             \
                ::RCF::RemoveOut<A14 >::type a14,                             \
                ::RCF::RemoveOut<A15 >::type a15)                             \
    {                                                                         \
    }

#endif // ! INCLUDE_RCF_RCFMETHODGEN_HPP