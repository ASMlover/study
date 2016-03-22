
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

#include <RCF/ObjectPool.hpp>

#include <RCF/ByteBuffer.hpp>
#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/Tools.hpp>

#include <boost/version.hpp>

namespace RCF {

    CbAllocatorBase::CbAllocatorBase(ObjectPool & objectPool) : 
        mObjectPool(objectPool)
    {}

    CbAllocatorBase::CbAllocatorBase(const CbAllocatorBase & rhs) : 
        mObjectPool(rhs.mObjectPool)
    {}

    void * CbAllocatorBase::allocate() 
    { 
        return mObjectPool.getPcb();
    }

    void CbAllocatorBase::deallocate(void * pcb) 
    { 
        mObjectPool.putPcb(pcb);
    }

    ObjectPool::ObjectPool() : 
        mObjPoolMutex(WriterPriority),
        mBufferCountLimit(10) ,
        mBufferSizeLimit(1024*1024*10)
    {
        mOsPool.reserve(10);
        mCbPool.reserve(10);
        mRbPool.reserve(10);
    }

    ObjectPool::~ObjectPool()
    {
        for (std::size_t i=0; i<mOsPool.size(); ++i)
        {
            delete mOsPool[i];
            mOsPool[i] = NULL;
        }

        for (std::size_t i=0; i<mRbPool.size(); ++i)
        {
            delete mRbPool[i];
            mRbPool[i] = NULL;
        }

        for (std::size_t i=0; i<mCbPool.size(); ++i)
        {
            delete [] (char *) mCbPool[i];
            mCbPool[i] = NULL;
        }

        ObjPool::iterator iter;
        for (iter = mObjPool.begin(); iter != mObjPool.end(); ++iter)
        {
            ObjList & objList = *(iter->second);
            for (std::size_t i=0; i<objList.mVec.size(); ++i)
            {
                objList.mOps->kill(objList.mVec[i]);
            }
        }
    }

    void ObjectPool::setBufferCountLimit(std::size_t bufferCountLimit)
    {
        mBufferCountLimit = bufferCountLimit;
    }

    std::size_t ObjectPool::getBufferCountLimit()
    {
        return mBufferCountLimit;
    }

    void ObjectPool::setBufferSizeLimit(std::size_t bufferSizeLimit)
    {
        mBufferSizeLimit = bufferSizeLimit;
    }

    std::size_t ObjectPool::getBufferSizeLimit()
    {
        return mBufferSizeLimit;
    }

    void * ObjectPool::getPcb()
    {
        void * pcb = NULL;

        Lock lock(mCbPoolMutex);
        if (mCbPool.empty())
        {
            pcb = new char[CbSize];
        }
        else
        {
            pcb = mCbPool.back();
            mCbPool.pop_back();
        }

        return pcb;
    }

    void ObjectPool::putPcb(void * pcb)
    {
        Lock lock(mCbPoolMutex);
        mCbPool.push_back(pcb);
    }

    MemOstreamPtr ObjectPool::getMemOstreamPtr()
    {
        MemOstreamPtr osPtr;
        MemOstream * pt = NULL;
        getPtr(pt, osPtr, mOsPool, mOsPoolMutex, &ObjectPool::putMemOstream);
        return osPtr;
    }

    ReallocBufferPtr ObjectPool::getReallocBufferPtr()
    {
        ReallocBufferPtr rbPtr;
        ReallocBuffer * pt = NULL;
        getPtr(pt, rbPtr, mRbPool, mRbPoolMutex, &ObjectPool::putReallocBuffer);
        return rbPtr;
    }

    void ObjectPool::putMemOstream(MemOstream * pOs)
    {
        std::auto_ptr<MemOstream> osPtr(pOs);
        std::size_t bufferSize = osPtr->capacity();
        pOs->clear(); // freezing may have set error state
        pOs->rewind();

        // Check buffer count limit and buffer size limit.
        Lock lock(mOsPoolMutex);
        if (mOsPool.size() < mBufferCountLimit && bufferSize <= mBufferSizeLimit)
        {
            mOsPool.push_back(osPtr.release());
        }
    }

    void ObjectPool::putReallocBuffer(ReallocBuffer * pRb)
    {
        std::auto_ptr<ReallocBuffer> rbPtr(pRb);
        std::size_t bufferSize = rbPtr->capacity();
        pRb->resize(0);

        // Check buffer count limit and buffer size limit.
        Lock lock(mRbPoolMutex);
        if (mRbPool.size() < mBufferCountLimit && bufferSize <= mBufferSizeLimit)
        {
            mRbPool.push_back(rbPtr.release());
        }
    }
   
    void ObjectPool::enumerateWriteBuffers(std::vector<std::size_t> & bufferSizes)
    {
        bufferSizes.resize(0);

        Lock lock(mOsPoolMutex);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4267)
#endif

        for (std::size_t i=0; i<mOsPool.size(); ++i)
        {
            bufferSizes.push_back( mOsPool[i]->capacity() );
        }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    }

    void ObjectPool::enumerateReadBuffers(std::vector<std::size_t> & bufferSizes)
    {
        bufferSizes.resize(0);

        Lock lock(mRbPoolMutex);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4267)
#endif

        for (std::size_t i=0; i<mRbPool.size(); ++i)
        {
            bufferSizes.push_back( mRbPool[i]->capacity() );
        }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    }

    ObjectPool * gpObjectPool;

    void initObjectPool()
    {
        gpObjectPool = new ObjectPool();
    }

    void deinitObjectPool()
    {
        delete gpObjectPool;
        gpObjectPool = NULL;
    }

    ObjectPool & getObjectPool()
    {
        return *gpObjectPool;
    }

} // namespace RCF
