
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

#ifndef INCLUDE_RCF_OBJECTPOOL_HPP
#define INCLUDE_RCF_OBJECTPOOL_HPP

#include <vector>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Tools.hpp>
#include <RCF/ThreadLibrary.hpp>

namespace RCF {

    static const std::size_t CbSize = 128;

    class ObjectPool;

    class RCF_EXPORT CbAllocatorBase
    {
    public:

        CbAllocatorBase(ObjectPool & objectPool);
        CbAllocatorBase(const CbAllocatorBase & rhs);

        void * allocate();
        void deallocate(void * pcb) ;

    private:
        ObjectPool & mObjectPool;
    };

    template<typename T>
    class CbAllocator : public CbAllocatorBase
    {
    public:

        typedef T value_type;
        typedef value_type* pointer;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

        template<typename U>
        struct rebind 
        {
            typedef CbAllocator<U> other;
        };

        CbAllocator(ObjectPool & objectPool) : CbAllocatorBase(objectPool)
        {
        }

        template<typename U>
        CbAllocator( const CbAllocator<U> & rhs) : CbAllocatorBase(rhs)
        {
        }

        pointer allocate(
            size_type cnt, 
            typename std::allocator<void>::const_pointer = 0)
        {
            BOOST_STATIC_ASSERT( sizeof(T) <= CbSize );
            RCF_ASSERT_EQ(cnt , 1);
            RCF_UNUSED_VARIABLE(cnt);
            return reinterpret_cast<pointer>(CbAllocatorBase::allocate());
        }

        void deallocate(pointer p, size_type) 
        { 
            CbAllocatorBase::deallocate(p);
        }
    };

    class TypeInfo
    {
    public:
        TypeInfo(const std::type_info & ti) : mpTypeInfo(&ti)
        {

        }

        bool operator<(const TypeInfo & rhs) const
        {
            return (*mpTypeInfo).before(*rhs.mpTypeInfo) ? true : false;
        }

    private:
        const std::type_info * mpTypeInfo;
    };

    class ReallocBuffer;
    typedef boost::shared_ptr<ReallocBuffer> ReallocBufferPtr;

    class RCF_EXPORT ObjectPool
    {
    public:

        ObjectPool();
        ~ObjectPool();

        template<typename T>
        void enableCaching(std::size_t maxCount, boost::function1<void, T *> clearFunc)
        {
            enableCaching( (T *) NULL, maxCount, clearFunc);
        }

        template<typename T>
        void disableCaching()
        {
            disableCaching( (T *) NULL);
        }

        template<typename T>
        void enableCaching(T *, std::size_t maxCount, boost::function1<void, T *> clearFunc)
        {
            RCF::WriteLock lock(mObjPoolMutex);
            RCF::TypeInfo ti( typeid(T) );
            mObjPool[ti].reset( new RCF::ObjectPool::ObjList() );
            mObjPool[ti]->mMaxSize = maxCount;
            mObjPool[ti]->mOps.reset( new RCF::ObjectPool::Ops<T>(clearFunc) );
        }

        template<typename T>
        void disableCaching(T *)
        {
            RCF::WriteLock lock(mObjPoolMutex);
            RCF::TypeInfo ti( typeid(T) );
            mObjPool[ti]->mMaxSize = 0;
            mObjPool[ti]->clear();
        }

        template<typename T>
        bool isCachingEnabled(T *)
        {
            ReadLock lock(mObjPoolMutex);
            if (!mObjPool.empty())
            {
                RCF::TypeInfo ti( typeid(T) );
                ObjPool::iterator iter = mObjPool.find(ti);
                if (iter != mObjPool.end())
                {
                    if (iter->second->mMaxSize > 0)
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        MemOstreamPtr getMemOstreamPtr();
        ReallocBufferPtr getReallocBufferPtr();

        void enumerateWriteBuffers(std::vector<std::size_t> & bufferSizes);
        void enumerateReadBuffers(std::vector<std::size_t> & bufferSizes);

        void setBufferCountLimit(std::size_t bufferCountLimit);
        std::size_t getBufferCountLimit();

        void setBufferSizeLimit(std::size_t bufferSizeLimit);
        std::size_t getBufferSizeLimit();

        template<typename T>
        void getObj(boost::shared_ptr<T> & objPtr, bool alwaysCreate = true)
        {
            T * pt = NULL;
            void * pv = NULL;
            boost::shared_ptr<void> spv;
            bool pfnDeleter = false;


            {
                ReadLock poolLock(mObjPoolMutex);

                if (mObjPool.empty())
                {
                    if (alwaysCreate)
                    {
                        pt = new T;
                    }
                    else
                    {
                        return;
                    }
                }
                else
                {
                    TypeInfo ti( typeid(T) );
                    ObjPool::iterator iter = mObjPool.find(ti);
                    if (iter == mObjPool.end())
                    {
                        if (alwaysCreate)
                        {
                            pt = new T;
                        }
                        else
                        {
                            return;
                        }
                    }
                    else
                    {
                        ObjList & objList = *(iter->second);
                        Lock listLock(objList.mMutex);
                        if (objList.mMaxSize == 0)
                        {
                            if (alwaysCreate)
                            {
                                pt = new T;
                            }
                            else
                            {
                                return;
                            }
                        }
                        else if (objList.mVec.empty())
                        {
                            pt = new T;
                            pfnDeleter = true;
                        }
                        else
                        {
                            pv = objList.mVec.back();
                            pt = static_cast<T *>(pv);
                            objList.mVec.pop_back();
                            pfnDeleter = true;
                        }
                    }
                }
            }

            RCF_ASSERT(pt);
            if (pfnDeleter)
            {
                TypeInfo ti( typeid(T) );

#if BOOST_VERSION < 103400

                // 1.33.1 shared_ptr, and earlier, does not have allocator support. Consequently we
                // have a (small) allocation each time a buffer is requested, and a corresponding
                // deallocation when the buffer is returned to the pool.

                objPtr = boost::shared_ptr<T>( 
                    pt, 
                    boost::bind(&ObjectPool::putObj, this, ti, _1));

#else

                // 1.34.0 shared_ptr has allocator support. Consequently we have no allocations
                // at all when a buffer is requested.

                objPtr = boost::shared_ptr<T>( 
                    pt, 
                    boost::bind(&ObjectPool::putObj, this, ti, _1),
                    CbAllocator<void>(*this) );

#endif
            }
            else
            {
                objPtr = boost::shared_ptr<T>(pt);
            }
        }

        void putObj(const TypeInfo & ti, void * pv)
        {
            ReadLock readLock(mObjPoolMutex);
            RCF_ASSERT(!mObjPool.empty());
            ObjPool::iterator iter = mObjPool.find(ti);
            RCF_ASSERT(iter != mObjPool.end());
            ObjList & objList = *(iter->second);
            Lock lock(objList.mMutex);
            if (objList.mVec.size() >= objList.mMaxSize)
            {
                lock.unlock();
                readLock.unlock();
                objList.mOps->kill(pv);
            }
            else
            {
                objList.mOps->clear(pv);
                objList.mVec.push_back(pv);
            }
        }

        class I_Ops
        {
        public:
            virtual ~I_Ops() {}
            virtual void kill(void * pv) = 0;
            virtual void clear(void * pv) = 0;
        };

        template<typename T>
        class Ops : public I_Ops
        {
        public:
            Ops(boost::function1<void, T *> clearFunc) : 
                mClearFunc(clearFunc)
            {
            }

            void kill(void * pv)
            {
                T * pt = static_cast<T *>(pv);
                delete pt;
            }

            void clear(void * pv)
            {
                if (mClearFunc)
                {
                    T * pt = static_cast<T *>(pv);
                    mClearFunc(pt);
                }
            }

            boost::function1<void, T *> mClearFunc;
        };

        class ObjList : boost::noncopyable
        {
        public:
            ObjList() : mMaxSize(0)
            {
            }
            Mutex mMutex;
            std::size_t mMaxSize;
            std::vector<void *> mVec;
            boost::scoped_ptr<I_Ops> mOps;

            void clear()
            {
                for (std::size_t i=0; i<mVec.size(); ++i)
                {
                    mOps->kill(mVec[i]);
                }
                mVec.clear();
            }
        };

        typedef boost::shared_ptr<ObjList> ObjListPtr;

        typedef std::map< TypeInfo, ObjListPtr > ObjPool;
        ReadWriteMutex mObjPoolMutex;
        ObjPool mObjPool;

    private:

        friend class CbAllocatorBase;

        void * getPcb();
        void putPcb(void * pcb);

        void putMemOstream(MemOstream * pOs);
        void putReallocBuffer(ReallocBuffer * pRb);

        std::size_t                             mBufferCountLimit;
        std::size_t                             mBufferSizeLimit;

        Mutex                                   mOsPoolMutex;
        std::vector< MemOstream * >             mOsPool;

        Mutex                                   mRbPoolMutex;
        std::vector< ReallocBuffer * >          mRbPool;

        Mutex                                   mCbPoolMutex;
        std::vector< void * >                   mCbPool;

        template<typename T, typename Spt, typename PtrList, typename Pfn>
        void getPtr(
            T *,
            Spt &           spt, 
            PtrList &       ptrList,
            Mutex &         ptrListMutex,
            Pfn             pfn)
        {
            T * pt = NULL;

            {
                Lock lock(ptrListMutex);

                if (ptrList.empty())
                {
                    pt = new T();
                }
                else
                {
                    pt = ptrList.back();
                    ptrList.pop_back();
                }
            }

#if BOOST_VERSION < 103400

            // 1.33.1 shared_ptr, and earlier, does not have allocator support. Consequently we
            // have a (small) allocation each time a buffer is requested, and a corresponding
            // deallocation when the buffer is returned to the pool.

            spt = boost::shared_ptr<T>( 
                pt, 
                boost::bind(pfn, this, _1));

#else

            // 1.34.0 shared_ptr has allocator support. Consequently we have no allocations
            // at all when a buffer is requested.

            spt = boost::shared_ptr<T>( 
                pt, 
                boost::bind(pfn, this, _1), 
                CbAllocator<void>(*this) );

#endif

        }

    };

    RCF_EXPORT ObjectPool & getObjectPool();

} // namespace RCF

#endif // ! INCLUDE_RCF_OBJECTPOOL_HPP
