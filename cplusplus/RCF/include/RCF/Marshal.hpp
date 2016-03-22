
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

#ifndef INCLUDE_RCF_MARSHAL_HPP
#define INCLUDE_RCF_MARSHAL_HPP

#include <RCF/AmiThreadPool.hpp>
#include <RCF/ClientStub.hpp>
#include <RCF/CurrentSerializationProtocol.hpp>
#include <RCF/ObjectPool.hpp>
#include <RCF/PublishingService.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/Tools.hpp>
#include <RCF/TypeTraits.hpp>
#include <RCF/Version.hpp>

#include <boost/mpl/and.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/or.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#if RCF_FEATURE_SF==1
#include <SF/memory.hpp>
#endif

#if RCF_FEATURE_BOOST_SERIALIZATION==1
#include <RCF/BsAutoPtr.hpp>
#include <boost/serialization/binary_object.hpp>
#endif

#include <SF/Tools.hpp>

namespace RCF {

    // The following macro hacks are necessitated by Boost.Serialization, which 
    // , apparently on principle, refuses to serialize pointers to (1) fundamental 
    // types, and (2) std::string, while happily serializing pointers to everything
    // else...

#define RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION(type)                \
    inline void serializeImpl(                                          \
        SerializationProtocolOut &out,                                  \
        const type *pt,                                                 \
        long int)                                                       \
    {                                                                   \
        serializeImpl(out, *pt, 0);                                     \
    }                                                                   \
                                                                        \
    inline void serializeImpl(                                          \
        SerializationProtocolOut &out,                                  \
        type *const pt,                                                 \
        long int)                                                       \
    {                                                                   \
        serializeImpl(out, *pt, 0);                                     \
    }                                                                   \
                                                                        \
    inline void deserializeImpl(                                        \
        SerializationProtocolIn &in,                                    \
        type *&pt,                                                      \
        long int)                                                       \
    {                                                                   \
        RCF_ASSERT(pt==NULL);                                           \
        pt = new type();                                                \
        deserializeImpl(in, *pt, 0);                                    \
    }

    SF_FOR_EACH_FUNDAMENTAL_TYPE( RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION )

#define RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION_T3(type)             \
    template<typename T1, typename T2, typename T3>                     \
    inline void serializeImpl(                                          \
        SerializationProtocolOut &out,                                  \
        const type<T1,T2,T3> *pt,                                       \
        int)                                                            \
    {                                                                   \
        serializeImpl(out, *pt, 0);                                     \
    }                                                                   \
                                                                        \
    template<typename T1, typename T2, typename T3>                     \
    inline void serializeImpl(                                          \
        SerializationProtocolOut &out,                                  \
        type<T1,T2,T3> *const pt,                                       \
        int)                                                            \
    {                                                                   \
        serializeImpl(out, *pt, 0);                                     \
    }                                                                   \
                                                                        \
    template<typename T1, typename T2, typename T3>                     \
    inline void deserializeImpl(                                        \
        SerializationProtocolIn &in,                                    \
        type<T1,T2,T3> *&pt,                                            \
        int)                                                            \
    {                                                                   \
        RCF_ASSERT(pt==NULL);                                           \
        pt = new type<T1,T2,T3>();                                      \
        deserializeImpl(in, *pt, 0);                                    \
    }

#if RCF_FEATURE_BOOST_SERIALIZATION==1

    RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION_T3(std::basic_string)

#endif

#undef RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION

#undef RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION_T3

    // Boost.Serialization handles smart pointers very clumsily, so we do those ourselves

#define RefCountSmartPtr boost::shared_ptr

    template<typename T>
    inline void serializeImpl(
        SerializationProtocolOut &out,
        const RefCountSmartPtr<T> *spt,
        int)
    {
        serialize(out, *spt);
    }

    template<typename T>
    inline void serializeImpl(
        SerializationProtocolOut &out,
        RefCountSmartPtr<T> *const spt,
        int)
    {
        serialize(out, *spt);
    }

    template<typename T>
    inline void deserializeImpl(
        SerializationProtocolIn &in,
        RefCountSmartPtr<T> *&spt,
        int)
    {
        spt = new RefCountSmartPtr<T>();
        deserialize(in, *spt);
    }

    template<typename T>
    inline void serializeImpl(
        SerializationProtocolOut &out,
        const RefCountSmartPtr<T> &spt,
        int)
    {
        serialize(out, spt.get());
    }

    template<typename T>
    inline void deserializeImpl(
        SerializationProtocolIn &in,
        RefCountSmartPtr<T> &spt,
        int)
    {
        T *pt = NULL;
        deserialize(in, pt);
        spt = RefCountSmartPtr<T>(pt);
    }

#undef RefCountSmartPtr

#if RCF_FEATURE_BOOST_SERIALIZATION==1
} // namespace RCF

namespace boost { namespace serialization {

    template<class Archive>
    void save(Archive & ar, const RCF::ByteBuffer &byteBuffer, unsigned int)
    {
        // We have to copy the buffer - can't do efficient zero-copy transmission 
        // of ByteBuffer, with B.Ser.

        boost::uint32_t len = byteBuffer.getLength();
        ar & len;
        ar & make_binary_object(byteBuffer.getPtr(), len);
        
    }

    template<class Archive>
    void load(Archive &ar, RCF::ByteBuffer &byteBuffer, unsigned int)
    {
        // We have to copy the buffer - can't do efficient zero-copy transmission 
        // of ByteBuffer, with B.Ser.

        boost::uint32_t len = 0;
        ar & len;

        RCF::ReallocBufferPtr bufferPtr = RCF::getObjectPool().getReallocBufferPtr();
        bufferPtr->resize(len);
        byteBuffer = RCF::ByteBuffer(bufferPtr);

        ar & make_binary_object(byteBuffer.getPtr(), byteBuffer.getLength());
    }

}} // namespace boost namespace serialization

    BOOST_SERIALIZATION_SPLIT_FREE(RCF::ByteBuffer);

namespace RCF {
#endif // RCF_FEATURE_BOOST_SERIALIZATION==1

    // serializeOverride() and deserializeOverride() are used to implement
    // a backwards compatibility workaround, for ByteBuffer interoperability 
    // with RCF runtime version <= 3.

    template<typename U>
    bool serializeOverride(SerializationProtocolOut &, U &)
    {
        return false;
    }

    template<typename U>
    bool serializeOverride(SerializationProtocolOut &, U *)
    {
        return false;
    }

    template<typename U>
    bool deserializeOverride(SerializationProtocolIn &, U &)
    {
        return false;
    }

    RCF_EXPORT bool serializeOverride(SerializationProtocolOut &out, ByteBuffer & u);

    RCF_EXPORT bool serializeOverride(SerializationProtocolOut &out, ByteBuffer * pu);

    RCF_EXPORT bool deserializeOverride(SerializationProtocolIn &in, ByteBuffer & u);

    // For vc6, manual zero-initialization of integral types.

    template<typename T> void vc6DefaultInit(T *) {}

    // -------------------------------------------------------------------------
    // Parameter store.

    template<typename T>
    class ParmStore
    {
    public:
        ParmStore() : mptPtr(), mpT(NULL)
        {
        }

        ParmStore(std::vector<char> & vec) : mptPtr(), mpT(NULL)
        {
            allocate(vec);
        }

        void allocate(std::vector<char> & vec)
        {
            RCF_ASSERT(mpT == NULL);

            getObjectPool().getObj(mptPtr, false);

            if (mptPtr)
            {
                mpT = mptPtr.get();
            }
            else
            {
                // If we didn't get it from the pool, use placement new to construct
                // it in the given vector.

                vec.resize(sizeof(T));
                mpT = (T *) & vec[0];

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4345 )  // warning C4345: behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized
#endif

                new (mpT) T();

                vc6DefaultInit(mpT);

#ifdef _MSC_VER
#pragma warning( pop )
#endif
            }

        }

        void assign(T * pt)
        {
            RCF_ASSERT(mpT == NULL);

            mptPtr.reset(pt);
            mpT = mptPtr.get();
        }

        ~ParmStore()
        {
            if (!mptPtr)
            {
                if (mpT)
                {
                    mpT->~T();
                }
            }
        }

        T & operator*()
        {
            return *mpT;
        }

        T * get()
        {
            return mpT;
        }

        boost::shared_ptr<T> mptPtr;
        T * mpT;
    };

    // Helper class to ensure delete is called for pointers that we allocate as part of reference marshaling.
    template<typename T>
    class Deleter
    {
    public:
        Deleter(T *& pt) : mpt(pt), mDismissed(false)
        {
        }
        ~Deleter()
        {
            if ( !mDismissed && mpt )
            {
                delete mpt;
                mpt = NULL;
            }
        }
        void dismiss()
        {
            mDismissed = true;
        }
    private:
        T*&     mpt;
        bool    mDismissed;
    };


    // -------------------------------------------------------------------------
    // Server marshaling.

    template<typename T>
    class Sm_Value
    {
    public:

        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsReference<T> > ));

        Sm_Value(std::vector<char> & vec) : mPs(vec)
        { 
        }

        T &get() 
        { 
            return *mPs; 
        }

        void set(bool assign, const T &t) 
        {
            if (assign) 
            {
                *mPs = t; 
            }
        }

        void set(const T &t) 
        { 
            *mPs = t; 
        }
        
        void read(SerializationProtocolIn &in) 
        { 
            if (in.getRemainingArchiveLength() != 0)
            {
                if (!deserializeOverride(in, *mPs))
                {
                    deserialize(in, *mPs);
                }
            }            
        }

        void write(SerializationProtocolOut &) 
        { 
            RCF_ASSERT(0);
        }

    private:
        ParmStore<T> mPs;
    };

    template<>
    class Sm_Value<Void>
    {
    public:

        Sm_Value(std::vector<char> &)
        { 
        }

        ~Sm_Value()
        {
        }

        Void &get() 
        { 
            RCF_ASSERT(0);
            static Void v;
            return v;
        }

        void set(bool , const Void &) 
        {
            RCF_ASSERT(0);
        }

        void set(const Void &) 
        { 
            RCF_ASSERT(0);
        }

        void read(SerializationProtocolIn &) 
        { 
            RCF_ASSERT(0);
        }

        void write(SerializationProtocolOut &) 
        { 
            RCF_ASSERT(0);
        }
    };

    template<typename T>
    class Sm_Ret
    {
    public:

        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsReference<T> > ));

        Sm_Ret(std::vector<char> & vec) : mPs(vec)
        { 
        }

        T &get() 
        { 
            return *mPs; 
        }

        void set(bool assign, const T &t) 
        {
            if (assign) 
            {
                *mPs = t; 
            }
        }

        void set(const T &t) 
        { 
            *mPs = t; 
        }
        
        void read(SerializationProtocolIn &) 
        { 
            RCF_ASSERT(0);
        }

        void write(SerializationProtocolOut &out) 
        { 
            if (!serializeOverride(out, *mPs))
            {
                serialize(out, *mPs);
            }
        }

    private:
        ParmStore<T> mPs;
    };

    template<typename CRefT>
    class Sm_CRef
    {
    public:

        typedef typename RemoveReference<CRefT>::type CT;
        typedef typename RemoveCv<CT>::type T;
        BOOST_MPL_ASSERT(( IsReference<CRefT> ));

        BOOST_MPL_ASSERT(( IsConst<CT> ));

        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        Sm_CRef(std::vector<char> & vec) : mPs(), mVec(vec) 
        {}

        T &get() 
        { 
            return *mPs; 
        }
        
        void set(bool assign, const T &t) 
        { 
            if (assign) 
            {
                *mPs = t; 
            }
        }

        void set(const T &t) 
        { 
            *mPs = t; 
        }

        void read(SerializationProtocolIn &in) 
        { 
            if (in.getRemainingArchiveLength() != 0)
            {
                int ver = in.getRuntimeVersion();
                if (ver < 8)
                {
                    // Deserialize as pointer, which means we may get
                    // polymorphic serialization happening.

                    T *pt = NULL;
                    Deleter<T> deleter(pt);
                    deserialize(in, pt);
                    deleter.dismiss();
                    mPs.assign(pt);
                }
                else if (ver == 8)
                {
                    // Deserialize as value.
                    mPs.allocate(mVec);
                    deserialize(in, *mPs);
                }
                else if (ver >= 9)
                {
                    // If BSer, deserialize through pointer.
                    // If SF and caching disabled, deserialize through pointer.
                    // If SF and caching enabled, use object cache and deserialize through value.

                    int sp = in.getSerializationProtocol();
                    if (    (sp == Sp_SfBinary || sp == Sp_SfText)
                        &&  getObjectPool().isCachingEnabled( (T *) NULL ))
                    {
                        mPs.allocate(mVec);
                        deserialize(in, *mPs);
                    }
                    else
                    {
                        T *pt = NULL;
                        Deleter<T> deleter(pt);
                        deserialize(in, pt);
                        if (!pt)
                        {
                            RCF::Exception e(RCF::_RcfError_DeserializationNullPointer());
                            RCF_THROW(e);
                        }
                        deleter.dismiss();
                        mPs.assign(pt);
                    }
                }
            }
            else
            {
                mPs.allocate(mVec);
            }
        }

        void write(SerializationProtocolOut &) 
        { 
            RCF_ASSERT(0);
        }

    private:
        ParmStore<T> mPs;
        std::vector<char> & mVec;
    };

    template<typename RefT>
    class Sm_Ref
    {
    public:

        typedef typename RemoveReference<RefT>::type T;
        typedef typename RemoveCv<T>::type U;
        BOOST_MPL_ASSERT(( IsReference<RefT> ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        Sm_Ref(std::vector<char> & vec) : mVec(vec)
        {}

        T &get() 
        { 
            return *mPs; 
        }
        
        void set(bool assign, const T &t) 
        { 
            if (assign) 
            {
                *mPs = t;
            }
        }
        
        void set(const T &t) 
        { 
            *mPs = t; 
        }

        void read(SerializationProtocolIn &in) 
        { 
            if (in.getRemainingArchiveLength() != 0)
            {
                int ver = in.getRuntimeVersion();
                if (ver < 8)
                {
                    // Deserialize as pointer, which means we may get
                    // polymorphic serialization happening.

                    T * pt = NULL;
                    Deleter<T> deleter(pt);
                    deserialize(in, pt);
                    deleter.dismiss();
                    mPs.assign(pt);
                }
                else if (ver == 8)
                {
                    // Deserialize as value.

                    mPs.allocate(mVec);
                    deserialize(in, *mPs);
                }
                else if (ver >= 9)
                {
                    // If BSer, deserialize through pointer.
                    // If SF and caching disabled, deserialize through pointer.
                    // If SF and caching enabled, use object cache and deserialize through value.

                    int sp = in.getSerializationProtocol();
                    if (    (sp == Sp_SfBinary || sp == Sp_SfText)
                        &&  getObjectPool().isCachingEnabled( (T *) NULL ))
                    {
                        mPs.allocate(mVec);
                        deserialize(in, *mPs);
                    }
                    else
                    {
                        T *pt = NULL;
                        Deleter<T> deleter(pt);
                        deserialize(in, pt);
                        if (!pt)
                        {
                            RCF::Exception e(RCF::_RcfError_DeserializationNullPointer());
                            RCF_THROW(e);
                        }
                        deleter.dismiss();
                        mPs.assign(pt);
                    }
                }
            }
            else
            {
                mPs.allocate(mVec);
            }
        }

        void write(SerializationProtocolOut &out) 
        { 
            RCF_ASSERT(mPs.get());

            if (!serializeOverride(out, *mPs))
            {
                // B.Ser. issues - because the client side proxy for a T& has to 
                // deserialize itself as a value, here we have to serialize as a 
                // value.

                serialize(out, *mPs);
            }
        }

    private:
        ParmStore<T> mPs;
        std::vector<char> & mVec;
    };

    template<typename OutRefT>
    class Sm_OutRef
    {
    public:

        typedef typename RemoveOut<OutRefT>::type RefT;
        typedef typename RemoveReference<RefT>::type T;
        typedef typename RemoveCv<T>::type U;
        BOOST_MPL_ASSERT(( IsReference<RefT> ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        Sm_OutRef(std::vector<char> & vec) : mPs(vec)
        {
        }

        T &get() 
        { 
            return *mPs; 
        }
        
        void set(bool assign, const T &t) 
        { 
            if (assign) 
            {
                *mPs = t;
            }
        }
        
        void set(const T &t) 
        { 
            *mPs = t; 
        }

        void read(SerializationProtocolIn &) 
        { 
            RCF_ASSERT(0);
        }

        void write(SerializationProtocolOut &out) 
        { 
            if (!serializeOverride(out, *mPs))
            {
                // B.Ser. issues - because the client side proxy for a T& has to 
                // deserialize itself as a value, here we have to serialize as a 
                // value.

                serialize(out, *mPs);
            }
        }

    private:
        ParmStore<T> mPs;
    };

    template<typename PtrT>
    class Sm_Ptr
    {
    public:

        typedef typename RemovePointer<PtrT>::type T;
        typedef typename RemoveCv<T>::type U;
        BOOST_MPL_ASSERT(( IsPointer<PtrT> ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        Sm_Ptr(std::vector<char> &)
        {}

        T *get() 
        { 
            return mPs.get(); 
        }
        
        void set(bool assign, const T &t) 
        { 
            if (assign) 
            {
                *mPs = t;
            }
        }
        
        void set(const T &t) 
        { 
            *mPs = t; 
        }

        void read(SerializationProtocolIn &in) 
        { 
            if (in.getRemainingArchiveLength() != 0)
            {
                T *pt = NULL;
                Deleter<T> deleter(pt);
                deserialize(in, pt);
                deleter.dismiss();
                mPs.assign(pt);
            }
        }

        void write(SerializationProtocolOut &) 
        { 
            RCF_ASSERT(0);
        }

    private:
        ParmStore<T> mPs;
    };

    // -------------------------------------------------------------------------
    // Client marshaling.

    template<typename T>
    class Cm_Ret
    {
    public:

        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsReference<T> > ));

        Cm_Ret()
        { 
            RCF::ClientStub * pClientStub = getTlsClientStubPtr();
            std::vector<char> & vec = pClientStub->getRetValVec();
            mPs.allocate(vec);
        }

        T &get() 
        { 
            return *mPs; 
        }

        void set(bool assign, const T &t) 
        {
            if (assign) 
            {
                *mPs = t; 
            }
        }

        void set(const T &t) 
        { 
            *mPs = t; 
        }
        
        void read(SerializationProtocolIn &in) 
        { 
            if (in.getRemainingArchiveLength() != 0)
            {
                if (!deserializeOverride(in, *mPs))
                {
                    deserialize(in, *mPs);
                }
            }            
        }

        void write(SerializationProtocolOut &) 
        { 
            RCF_ASSERT(0);
        }

    private:
        ParmStore<T> mPs;
    };

    template<typename T>
    class Cm_Value
    {
    public:

        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsReference<T> > ));

        // We use const_cast here, in case T's copy constructor is non-const.
        // E.g. if T is a std::auto_ptr.
        Cm_Value(const T &t) : mT( const_cast<T &>(t) ) 
        {
        }

        const T &get() 
        { 
            return mT; 
        }

        void read(SerializationProtocolIn &in) 
        {
            RCF_UNUSED_VARIABLE(in);
        }
        
        void write(SerializationProtocolOut &out) 
        { 
            if (!serializeOverride(out, mT))
            {
                serialize(out, mT);
            }
        }

    private:
        T mT;
    };

    template<typename PtrT>
    class Cm_Ptr
    {
    public:

        typedef typename RemovePointer<PtrT>::type T;

        BOOST_MPL_ASSERT(( IsPointer<PtrT> ));

        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        // We use const_cast here, in case T's copy constructor is non-const.
        // E.g. if T is a std::auto_ptr.
        //Proxy_Ptr(const T &t) : mT( const_cast<T &>(t) ) 
        Cm_Ptr(T * pt) : mpT(pt) 
        {
        }

        T *& get() 
        { 
            return mpT; 
        }

        void read(SerializationProtocolIn &in) 
        {
            RCF_UNUSED_VARIABLE(in);
        }
        
        void write(SerializationProtocolOut &out) 
        { 
            serialize(out, mpT);
        }

    private:
        T * mpT;
    };

    template<typename CRefT>
    class Cm_CRef
    {
    public:

        typedef typename RemoveReference<CRefT>::type CT;
        typedef typename RemoveCv<CT>::type T;
        BOOST_MPL_ASSERT(( IsReference<CRefT> ));

        BOOST_MPL_ASSERT(( IsConst<CT> ));

        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        Cm_CRef(const T &t) : mT(t) 
        {}

        const T &get() 
        { 
            return mT; 
        }

        void read(SerializationProtocolIn &in) 
        { 
            RCF_UNUSED_VARIABLE(in);
        }

        void write(SerializationProtocolOut &out) 
        { 
            int ver = out.getRuntimeVersion();
            if (ver < 8)
            {
                serialize(out, &mT);
            }
            else if (ver == 8)
            {
                serialize(out, mT);
            }
            else if (ver >= 9)
            {
                serialize(out, &mT);
            }
        }

    private:
        const T &mT;
    };

    template<typename RefT>
    class Cm_Ref
    {
    public:

        typedef typename RemoveReference<RefT>::type T;
        BOOST_MPL_ASSERT(( IsReference<RefT> ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsConst<RefT> > ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        Cm_Ref(T &t) : mT(t) 
        {}

        T &get() 
        { 
            return mT; 
        }

        void read(SerializationProtocolIn &in) 
        { 
            if (in.getRemainingArchiveLength() != 0)
            {
                if (!deserializeOverride(in, mT))
                {
                    deserialize(in, mT);
                }
            }            
        }

        void write(SerializationProtocolOut &out) 
        { 
            int ver = out.getRuntimeVersion();
            if (ver < 8)
            {
                serialize(out, &mT);
            }
            else if (ver == 8)
            {
                serialize(out, mT);
            }
            else if (ver >= 9)
            {
                serialize(out, &mT);
            }
        }

    private:
        T & mT;
    };

    template<typename OutRefT>
    class Cm_OutRef
    {
    public:

        typedef typename RemoveOut<OutRefT>::type RefT;
        typedef typename RemoveReference<RefT>::type T;
        BOOST_MPL_ASSERT(( IsReference<RefT> ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsConst<RefT> > ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        Cm_OutRef(T &t) : mT(t) 
        {}

        T &get() 
        { 
            return mT; 
        }

        void read(SerializationProtocolIn &in) 
        { 
            if (in.getRemainingArchiveLength() != 0)
            {
                if (!deserializeOverride(in, mT))
                {
                    deserialize(in, mT);
                }
            }            
        }

        void write(SerializationProtocolOut &) 
        { 
            RCF_ASSERT(0);
        }

    private:
        T &mT;
    };

    template<typename T>
    struct IsConstReference
    {
        typedef typename
            boost::mpl::and_<
                IsReference<T>,
                IsConst< typename RemoveReference<T>::type >
            >::type type;

        enum { value = type::value };
    };

    template<typename T>
    struct ServerMarshalRet
    {
        typedef typename
        boost::mpl::if_<
            boost::is_same<void, T>,
            Sm_Ret<Void>,
            Sm_Ret<T> >::type type;
    };

    template<typename T>
    struct ServerMarshal
    {
        typedef typename
        boost::mpl::if_<
            IsPointer<T>,
            Sm_Ptr<T>,
            typename boost::mpl::if_<
                IsConstReference<T>,
                Sm_CRef<T>,
                typename boost::mpl::if_<
                    IsReference<T>,
                    Sm_Ref<T>,
                    typename boost::mpl::if_<
                        IsOut<T>,
                        Sm_OutRef<T>,
                        Sm_Value<T>
                    >::type



                >::type
            >::type
        >::type type;
    };

    template<typename T>
    struct ClientMarshal
    {
        typedef typename
        boost::mpl::if_<
            IsPointer<T>,
            Cm_Ptr<T>,
            typename boost::mpl::if_<
                IsConstReference<T>,
                Cm_CRef<T>,
                typename boost::mpl::if_<
                    IsReference<T>,
                    Cm_Ref<T>,
                    typename boost::mpl::if_<
                        IsOut<T>,
                        Cm_OutRef<T>,
                        Cm_Value<T>
                    >::type
                >::type
            >::type
        >::type type;
    };


    // ReferenceTo:
    // For generic T, return const T &.
    // For T &, return T &.
    // For const T &, return const T &

    template<typename T>
    struct ReferenceTo
    {
        typedef typename
        boost::mpl::if_<
            IsReference<T>,
            T,
            typename boost::mpl::if_<
                RCF::IsConst<T>,
                typename boost::add_reference<T>::type,
                typename boost::add_reference<
                    typename boost::add_const<T>::type
                >::type
            >::type
        >::type type;
    };

    class I_Parameters
    {
    public:
        virtual ~I_Parameters() {}
        virtual void read(SerializationProtocolIn &in) = 0;
        virtual void write(SerializationProtocolOut &out) = 0;
        virtual bool enrolFutures(RCF::ClientStub *pClientStub) = 0;
    };

    template<typename T>
    struct IsInParameter
    {
        typedef typename boost::mpl::not_< boost::is_same<T,Void> >::type   NotVoid;
        typedef typename boost::mpl::not_< IsOut<T> >::type                 NotExplicitOutParameter;

        typedef typename boost::mpl::and_<
            NotVoid,
            NotExplicitOutParameter
        >::type type;
        
        enum { value = type::value };
    };

    template<typename T>
    struct IsOutParameter
    {
        typedef typename
        boost::mpl::and_<
            IsReference<T>,
            boost::mpl::not_< 
                boost::is_const< 
                    typename RemoveReference<T>::type
                > 
            > 
        >::type NonConstRef_;

        typedef typename IsOut<T>::type ExplicitOutParameter;

        // Following construction doesn't compile with VC6 for some reason.

        //typedef typename boost::mpl::or_<
        //  NonConstRef,
        //  ExplicitOutParameter
        //>::type type;

        enum { value = NonConstRef_::value || ExplicitOutParameter::value };
    };

    template<typename T>
    struct IsReturnValue
    {
        typedef typename boost::mpl::not_< boost::is_same<T, Void> >::type type;
        enum { value = type::value };
    };

    class Candidates 
    {
    public:
        I_Future * find(const void * pv)
        {
            I_Future * pFuture = NULL;
            for (std::size_t i=0; i<mCandidateList.size(); ++i)
            {
                if (mCandidateList[i].first == pv)
                {
                    RCF_ASSERT(!pFuture);
                    pFuture = mCandidateList[i].second;
                }
            }
            return pFuture;
        }

        void erase(const void * pv)
        {
            for (std::size_t i=0; i<mCandidateList.size(); ++i)
            {
                if (mCandidateList[i].first == pv)
                {
                    mCandidateList.erase( mCandidateList.begin() + i );
                    return;
                }
            }
            RCF_ASSERT(0);
        }

        void add(const void * pv, I_Future * pFuture)
        {
            for (std::size_t i=0; i<mCandidateList.size(); ++i)
            {
                if (mCandidateList[i].first == pv)
                {
                    mCandidateList[i].second = pFuture;
                    return;
                }
            }
            mCandidateList.push_back( std::make_pair(pv, pFuture) );
        }

    private:

        typedef std::vector< std::pair<const void *, I_Future *> > CandidateList;
        CandidateList mCandidateList;
    };

    //typedef std::vector< std::pair<const void *, I_Future *> > Candidates;

    RCF_EXPORT Mutex & gCandidatesMutex();
    RCF_EXPORT Candidates & gCandidates();

    template<
        typename R, 
        typename A1,
        typename A2,
        typename A3,
        typename A4,
        typename A5,
        typename A6,
        typename A7,
        typename A8,
        typename A9,
        typename A10,
        typename A11,
        typename A12,
        typename A13,
        typename A14,
        typename A15>
    class ClientParameters : public I_Parameters
    {
    public:

        typedef typename RemoveOut<A1 >::type A1_;
        typedef typename RemoveOut<A2 >::type A2_;
        typedef typename RemoveOut<A3 >::type A3_;
        typedef typename RemoveOut<A4 >::type A4_;
        typedef typename RemoveOut<A5 >::type A5_;
        typedef typename RemoveOut<A6 >::type A6_;
        typedef typename RemoveOut<A7 >::type A7_;
        typedef typename RemoveOut<A8 >::type A8_;
        typedef typename RemoveOut<A9 >::type A9_;
        typedef typename RemoveOut<A10>::type A10_;
        typedef typename RemoveOut<A11>::type A11_;
        typedef typename RemoveOut<A12>::type A12_;
        typedef typename RemoveOut<A13>::type A13_;
        typedef typename RemoveOut<A14>::type A14_;
        typedef typename RemoveOut<A15>::type A15_;

        typedef typename ReferenceTo<A1_ >::type A1Ref;
        typedef typename ReferenceTo<A2_ >::type A2Ref;
        typedef typename ReferenceTo<A3_ >::type A3Ref;
        typedef typename ReferenceTo<A4_ >::type A4Ref;
        typedef typename ReferenceTo<A5_ >::type A5Ref;
        typedef typename ReferenceTo<A6_ >::type A6Ref;
        typedef typename ReferenceTo<A7_ >::type A7Ref;
        typedef typename ReferenceTo<A8_ >::type A8Ref;
        typedef typename ReferenceTo<A9_ >::type A9Ref;
        typedef typename ReferenceTo<A10_>::type A10Ref;
        typedef typename ReferenceTo<A11_>::type A11Ref;
        typedef typename ReferenceTo<A12_>::type A12Ref;
        typedef typename ReferenceTo<A13_>::type A13Ref;
        typedef typename ReferenceTo<A14_>::type A14Ref;
        typedef typename ReferenceTo<A15_>::type A15Ref;

        ClientParameters( 
            A1Ref a1, A2Ref a2, A3Ref a3, A4Ref a4, A5Ref a5, A6Ref a6, 
            A7Ref a7, A8Ref a8, A9Ref a9, A10Ref a10, A11Ref a11, A12Ref a12, 
            A13Ref a13, A14Ref a14, A15Ref a15) : 
                a1(a1), a2(a2), a3(a3), a4(a4), a5(a5), a6(a6), a7(a7), a8(a8),
                a9(a9), a10(a10), a11(a11), a12(a12), a13(a13), a14(a14), a15(a15)
        {
        }

        void read(SerializationProtocolIn &in)
        {
            if (IsReturnValue<R>::value)        r.read(in);
            if (IsOutParameter<A1 >::value)     a1.read(in);
            if (IsOutParameter<A2 >::value)     a2.read(in);
            if (IsOutParameter<A3 >::value)     a3.read(in);
            if (IsOutParameter<A4 >::value)     a4.read(in);
            if (IsOutParameter<A5 >::value)     a5.read(in);
            if (IsOutParameter<A6 >::value)     a6.read(in);
            if (IsOutParameter<A7 >::value)     a7.read(in);
            if (IsOutParameter<A8 >::value)     a8.read(in);
            if (IsOutParameter<A9 >::value)     a9.read(in);
            if (IsOutParameter<A10>::value)     a10.read(in);
            if (IsOutParameter<A11>::value)     a11.read(in);
            if (IsOutParameter<A12>::value)     a12.read(in);
            if (IsOutParameter<A13>::value)     a13.read(in);
            if (IsOutParameter<A14>::value)     a14.read(in);
            if (IsOutParameter<A15>::value)     a15.read(in);
        }

        void write(SerializationProtocolOut &out)
        {
            if (IsInParameter<A1 >::value)      a1.write(out);
            if (IsInParameter<A2 >::value)      a2.write(out);
            if (IsInParameter<A3 >::value)      a3.write(out);
            if (IsInParameter<A4 >::value)      a4.write(out);
            if (IsInParameter<A5 >::value)      a5.write(out);
            if (IsInParameter<A6 >::value)      a6.write(out);
            if (IsInParameter<A7 >::value)      a7.write(out);
            if (IsInParameter<A8 >::value)      a8.write(out);
            if (IsInParameter<A9 >::value)      a9.write(out);
            if (IsInParameter<A10>::value)      a10.write(out);
            if (IsInParameter<A11>::value)      a11.write(out);
            if (IsInParameter<A12>::value)      a12.write(out);
            if (IsInParameter<A13>::value)      a13.write(out);
            if (IsInParameter<A14>::value)      a14.write(out);
            if (IsInParameter<A15>::value)      a15.write(out);
        }

        bool enrolFutures(RCF::ClientStub *pClientStub)
        {
            bool enrolled = false;

            const void * pva[] = {
                &a1.get(),
                &a2.get(),
                &a3.get(),
                &a4.get(),
                &a5.get(),
                &a6.get(),
                &a7.get(),
                &a8.get(),
                &a9.get(),
                &a10.get(),
                &a11.get(),
                &a12.get(),
                &a13.get(),
                &a14.get(),
                &a15.get() 
            };

            for (std::size_t i=0; i<sizeof(pva)/sizeof(pva[0]); ++i)
            {
                const void *pv = pva[i];
                I_Future * pFuture = NULL;

                {
                    Lock lock(gCandidatesMutex());
                    pFuture = gCandidates().find(pv);
                    if (pFuture)
                    {
                        gCandidates().erase(pv);
                    }
                }

                if (pFuture)
                {
                    pClientStub->enrol( pFuture );
                    enrolled = true;
                }
            }

            return enrolled;
        }

        Cm_Ret<R>                               r;
        typename ClientMarshal<A1>::type        a1;
        typename ClientMarshal<A2>::type        a2;
        typename ClientMarshal<A3>::type        a3;
        typename ClientMarshal<A4>::type        a4;
        typename ClientMarshal<A5>::type        a5;
        typename ClientMarshal<A6>::type        a6;
        typename ClientMarshal<A7>::type        a7;
        typename ClientMarshal<A8>::type        a8;
        typename ClientMarshal<A9>::type        a9;
        typename ClientMarshal<A10>::type       a10;
        typename ClientMarshal<A11>::type       a11;
        typename ClientMarshal<A12>::type       a12;
        typename ClientMarshal<A13>::type       a13;
        typename ClientMarshal<A14>::type       a14;
        typename ClientMarshal<A15>::type       a15;
    };

    template<
        typename R, 
        typename A1,
        typename A2,
        typename A3,
        typename A4,
        typename A5,
        typename A6,
        typename A7,
        typename A8,
        typename A9,
        typename A10,
        typename A11,
        typename A12,
        typename A13,
        typename A14,
        typename A15>
    class AllocateClientParameters
    {
    public:

        typedef typename RemoveOut<A1 >::type A1_;
        typedef typename RemoveOut<A2 >::type A2_;
        typedef typename RemoveOut<A3 >::type A3_;
        typedef typename RemoveOut<A4 >::type A4_;
        typedef typename RemoveOut<A5 >::type A5_;
        typedef typename RemoveOut<A6 >::type A6_;
        typedef typename RemoveOut<A7 >::type A7_;
        typedef typename RemoveOut<A8 >::type A8_;
        typedef typename RemoveOut<A9 >::type A9_;
        typedef typename RemoveOut<A10>::type A10_;
        typedef typename RemoveOut<A11>::type A11_;
        typedef typename RemoveOut<A12>::type A12_;
        typedef typename RemoveOut<A13>::type A13_;
        typedef typename RemoveOut<A14>::type A14_;
        typedef typename RemoveOut<A15>::type A15_;

        typedef typename ReferenceTo<A1_ >::type A1Ref;
        typedef typename ReferenceTo<A2_ >::type A2Ref;
        typedef typename ReferenceTo<A3_ >::type A3Ref;
        typedef typename ReferenceTo<A4_ >::type A4Ref;
        typedef typename ReferenceTo<A5_ >::type A5Ref;
        typedef typename ReferenceTo<A6_ >::type A6Ref;
        typedef typename ReferenceTo<A7_ >::type A7Ref;
        typedef typename ReferenceTo<A8_ >::type A8Ref;
        typedef typename ReferenceTo<A9_ >::type A9Ref;
        typedef typename ReferenceTo<A10_>::type A10Ref;
        typedef typename ReferenceTo<A11_>::type A11Ref;
        typedef typename ReferenceTo<A12_>::type A12Ref;
        typedef typename ReferenceTo<A13_>::type A13Ref;
        typedef typename ReferenceTo<A14_>::type A14Ref;
        typedef typename ReferenceTo<A15_>::type A15Ref;

        typedef ClientParameters<
            R, 
            A1, A2, A3, A4, A5, A6, A7, A8,
            A9, A10, A11, A12, A13, A14, A15> ParametersT;

        // TODO: unnecessary copy of a* here, if A* is not a reference
        ParametersT &operator()(
            ClientStub &clientStub, 
            A1Ref  a1, 
            A2Ref  a2, 
            A3Ref  a3, 
            A4Ref  a4, 
            A5Ref  a5, 
            A6Ref  a6, 
            A7Ref  a7, 
            A8Ref  a8, 
            A9Ref  a9, 
            A10Ref a10, 
            A11Ref a11, 
            A12Ref a12, 
            A13Ref a13, 
            A14Ref a14, 
            A15Ref a15) const
        {
            CurrentClientStubSentry sentry(clientStub);

            clientStub.clearParameters();

            clientStub.mParametersVec.resize(sizeof(ParametersT));
            
            clientStub.mpParameters = new ( &clientStub.mParametersVec[0] ) 
                ParametersT(
                    a1,a2,a3,a4,a5,a6,a7,a8,
                    a9,a10,a11,a12,a13,a14,a15);

            if (!clientStub.mpParameters)
            {
                Exception e(_RcfError_ClientStubParms());
                RCF_THROW(e);
            }

            return static_cast<ParametersT &>(*clientStub.mpParameters);
        }
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
    class ServerParameters : public I_Parameters
    {
    public:

        ServerParameters(RcfSession &session) :
            r(session.mParmsVec[0]),
            a1(session.mParmsVec[1]),
            a2(session.mParmsVec[2]),
            a3(session.mParmsVec[3]),
            a4(session.mParmsVec[4]),
            a5(session.mParmsVec[5]),
            a6(session.mParmsVec[6]),
            a7(session.mParmsVec[7]),
            a8(session.mParmsVec[8]),
            a9(session.mParmsVec[9]),
            a10(session.mParmsVec[10]),
            a11(session.mParmsVec[11]),
            a12(session.mParmsVec[12]),
            a13(session.mParmsVec[13]),
            a14(session.mParmsVec[14]),
            a15(session.mParmsVec[15])
        {
            read(session.mIn);
        }

        void read(SerializationProtocolIn &in)
        {
            if (IsInParameter<A1 >::value)      a1.read(in);
            if (IsInParameter<A2 >::value)      a2.read(in);
            if (IsInParameter<A3 >::value)      a3.read(in);
            if (IsInParameter<A4 >::value)      a4.read(in);
            if (IsInParameter<A5 >::value)      a5.read(in);
            if (IsInParameter<A6 >::value)      a6.read(in);
            if (IsInParameter<A7 >::value)      a7.read(in);
            if (IsInParameter<A8 >::value)      a8.read(in);
            if (IsInParameter<A9 >::value)      a9.read(in);
            if (IsInParameter<A10>::value)      a10.read(in);
            if (IsInParameter<A11>::value)      a11.read(in);
            if (IsInParameter<A12>::value)      a12.read(in);
            if (IsInParameter<A13>::value)      a13.read(in);
            if (IsInParameter<A14>::value)      a14.read(in);
            if (IsInParameter<A15>::value)      a15.read(in);
        }

        void write(SerializationProtocolOut &out)
        {
            if (IsReturnValue<R>::value)        r.write(out);
            if (IsOutParameter<A1>::value)      a1.write(out);
            if (IsOutParameter<A2>::value)      a2.write(out);
            if (IsOutParameter<A3>::value)      a3.write(out);
            if (IsOutParameter<A4>::value)      a4.write(out);
            if (IsOutParameter<A5>::value)      a5.write(out);
            if (IsOutParameter<A6>::value)      a6.write(out);
            if (IsOutParameter<A7>::value)      a7.write(out);
            if (IsOutParameter<A8>::value)      a8.write(out);
            if (IsOutParameter<A9>::value)      a9.write(out);
            if (IsOutParameter<A10>::value)     a10.write(out);
            if (IsOutParameter<A11>::value)     a11.write(out);
            if (IsOutParameter<A12>::value)     a12.write(out);
            if (IsOutParameter<A13>::value)     a13.write(out);
            if (IsOutParameter<A14>::value)     a14.write(out);
            if (IsOutParameter<A15>::value)     a15.write(out);
        }

        // TODO: we shouldn't need this here
        bool enrolFutures(RCF::ClientStub *)
        {
            RCF_ASSERT(0);
            return false;
        }

        typename ServerMarshalRet<R>::type      r;
        typename ServerMarshal<A1>::type        a1;
        typename ServerMarshal<A2>::type        a2;
        typename ServerMarshal<A3>::type        a3;
        typename ServerMarshal<A4>::type        a4;
        typename ServerMarshal<A5>::type        a5;
        typename ServerMarshal<A6>::type        a6;
        typename ServerMarshal<A7>::type        a7;
        typename ServerMarshal<A8>::type        a8;
        typename ServerMarshal<A9>::type        a9;
        typename ServerMarshal<A10>::type       a10;
        typename ServerMarshal<A11>::type       a11;
        typename ServerMarshal<A12>::type       a12;
        typename ServerMarshal<A13>::type       a13;
        typename ServerMarshal<A14>::type       a14;
        typename ServerMarshal<A15>::type       a15;
    };

    typedef boost::shared_ptr<I_Parameters> ParametersPtr;

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
    class AllocateServerParameters
    {
    public:
        typedef ServerParameters<
            R, 
            A1, A2, A3, A4, A5, A6, A7, A8,
            A9, A10, A11, A12, A13, A14, A15> ParametersT;

        ParametersT &operator()(RcfSession &session) const
        {
            session.clearParameters();

            session.mParametersVec.resize(sizeof(ParametersT));

            session.mpParameters = new 
                ( &session.mParametersVec[0] ) 
                ParametersT(session);

            if (!session.mpParameters)
            {
                Exception e(_RcfError_ServerStubParms());
                RCF_THROW(e);
            }

            return static_cast<ParametersT &>(*session.mpParameters);
        }
    };

    // Bidirectional connections - converting between RcfClient and RcfSession.

    RCF_EXPORT void convertRcfSessionToRcfClient(
        OnCallbackConnectionCreated func,
        RemoteCallSemantics rcs = RCF::Twoway);


    RCF_EXPORT RcfSessionPtr convertRcfClientToRcfSession(
        ClientStub & clientStub, 
        ServerTransport & serverTransport,
        bool keepClientConnection = false);

    RCF_EXPORT RcfSessionPtr convertRcfClientToRcfSession(
        ClientStub & clientStub, 
        RcfServer & server,
        bool keepClientConnection = false);


    template<typename RcfClientT>
    inline RcfSessionPtr convertRcfClientToRcfSession(
        RcfClientT & client, 
        RcfServer & server,
        bool keepClientConnection = false)
    {
        return convertRcfClientToRcfSession(
            client.getClientStub(),
            server,
            keepClientConnection);
    }

    template<typename RcfClientT>
    inline RcfSessionPtr convertRcfClientToRcfSession(
        RcfClientT & client, 
        ServerTransport & serverTransport,
        bool keepClientConnection = false)
    {
        return convertRcfClientToRcfSession(
            client.getClientStub(),
            serverTransport,
            keepClientConnection);
    }



    RCF_EXPORT void createCallbackConnectionImpl(
        ClientStub & client, 
        ServerTransport & callbackServer);

    RCF_EXPORT void createCallbackConnectionImpl_Legacy(
        ClientStub & client, 
        ServerTransport & callbackServer);

    RCF_EXPORT void createCallbackConnectionImpl(
        ClientStub & client, 
        RcfServer & callbackServer);


    
    template<typename RcfClientT>
    void createCallbackConnection(
        RcfClientT & client, 
        RcfServer & callbackServer)
    {
        createCallbackConnectionImpl(
            client.getClientStub(), 
            callbackServer);
    }

    template<typename RcfClientT>
    void createCallbackConnectionImpl(
        RcfClientT & client, 
        ServerTransport & callbackServer)
    {
        createCallbackConnection(
            client.getClientStub(), 
            callbackServer);
    }

} // namespace RCF

#endif // ! INCLUDE_RCF_MARSHAL_HPP
