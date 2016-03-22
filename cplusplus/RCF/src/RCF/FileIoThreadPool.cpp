
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

#include <RCF/FileIoThreadPool.hpp>

#include <RCF/InitDeinit.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    FileIoThreadPool::FileIoThreadPool() : 
        mSerializeFileIo(false),
        mThreadPool(1,10) 
    {
        mThreadPool.setThreadName("RCF Async File IO");
        mThreadPool.setThreadIdleTimeoutMs(30*1000);
        mThreadPool.setReserveLastThread(false);

        mThreadPool.setTask( boost::bind(
            &FileIoThreadPool::ioTask,
            this));

        mThreadPool.setStopFunctor( boost::bind(
            &FileIoThreadPool::stopIoTask,
            this));
    }

    FileIoThreadPool::~FileIoThreadPool()
    {
        RCF_DTOR_BEGIN
            mThreadPool.stop();
        RCF_DTOR_END
    }

    void FileIoThreadPool::setSerializeFileIo(bool serializeFileIo)
    {
        mSerializeFileIo = serializeFileIo;
    }

    void FileIoThreadPool::stop()
    {
        mThreadPool.stop();
    }

    void FileIoThreadPool::registerOp(FileIoRequestPtr opPtr)
    {
        RCF::Lock lock(mOpsMutex);

        // Lazy start of the thread pool.
        if (!mThreadPool.isStarted())
        {
            mThreadPool.start();
        }

        if (    std::find(mOpsQueued.begin(), mOpsQueued.end(), opPtr) 
            !=  mOpsQueued.end())
        {
            RCF_ASSERT(0);
        }
        else if (       std::find(mOpsInProgress.begin(), mOpsInProgress.end(), opPtr) 
                    !=  mOpsInProgress.end())
        {
            RCF_ASSERT(0);
        }
        else
        {
            mOpsQueued.push_back(opPtr);
            mOpsCondition.notify_all(lock);
        }
    }

    void FileIoThreadPool::unregisterOp(FileIoRequestPtr opPtr)
    {
        RCF::Lock lock(mOpsMutex);
        RCF::eraseRemove(mOpsQueued, opPtr);
        RCF::eraseRemove(mOpsInProgress, opPtr);
    }

    bool FileIoThreadPool::ioTask()
    {
        FileIoRequestPtr opPtr;

        {
            RCF::Lock lock(mOpsMutex);
            while (mOpsQueued.empty() && !mThreadPool.shouldStop())
            {
                mOpsCondition.timed_wait(lock, 1000);
            }
            if (mOpsQueued.empty() || mThreadPool.shouldStop())
            {
                return false;
            }
            RCF_ASSERT_GT(mOpsQueued.size() , 0);
            mOpsInProgress.push_back( mOpsQueued.front() );
            mOpsQueued.pop_front();
            opPtr = mOpsInProgress.back();
        }

        RCF::ThreadInfoPtr threadInfoPtr = RCF::getTlsThreadInfoPtr();
        if (threadInfoPtr)
        {
            threadInfoPtr->notifyBusy();
        }

        // This is the part that blocks.
        opPtr->doTransfer();

        // Unregister op.
        unregisterOp(opPtr);

        // Notify completion.
        {
            RCF::Lock lock(mCompletionMutex);
            opPtr->mCompleted = true;
            mCompletionCondition.notify_all(lock);
        }

        return false;
    }

    void FileIoThreadPool::stopIoTask()
    {
        RCF::Lock lock(mOpsMutex);
        mOpsCondition.notify_all(lock);
    }

    FileIoRequest::FileIoRequest() :
        mFts( getFileIoThreadPool() ),
        mBytesTransferred(0),
        mInitiated(false),
        mCompleted(true)
    {
        RCF_LOG_4() << "FileIoRequest::FileIoRequest";
    }

    FileIoRequest::~FileIoRequest()
    {
        RCF_LOG_4() << "FileIoRequest::~FileIoRequest";
    }

    bool FileIoRequest::isInitiated()
    {
        RCF_LOG_4() << "FileIoRequest::isInitiated()";

        RCF::Lock lock(mFts.mCompletionMutex);
        return mInitiated;
    }

    bool FileIoRequest::isCompleted()
    {
        RCF_LOG_4() << "FileIoRequest::isCompleted";

        RCF::Lock lock(mFts.mCompletionMutex);
        return mCompleted;
    }

    void FileIoRequest::complete()
    {
        RCF_LOG_4() << "FileIoRequest::complete() - entry";

        RCF::Lock lock(mFts.mCompletionMutex);
        while (!mCompleted)
        {
            mFts.mCompletionCondition.timed_wait(lock, 1000);
        }
        mInitiated = false;

        RCF_LOG_4() << "FileIoRequest::complete() - exit";
    }

    void FileIoRequest::initiateRead(boost::shared_ptr<std::ifstream> finPtr, RCF::ByteBuffer buffer)
    {
        RCF_LOG_4()(finPtr.get())((void*)buffer.getPtr())(buffer.getLength()) << "FileIoRequest::read()";

        mFinPtr = finPtr;
        mFoutPtr.reset();
        mBuffer = buffer;
        mBytesTransferred = 0;
        mInitiated = true;
        mCompleted = false;

        mFts.registerOp( shared_from_this() );

        // For debugging purposes, we can wait in this function until the file I/O is completed.
        if (mFts.mSerializeFileIo)
        {
            RCF::Lock lock(mFts.mCompletionMutex);
            while (!mCompleted)
            {
                mFts.mCompletionCondition.wait(lock);
            }
        }
    }

    void FileIoRequest::initateWrite(boost::shared_ptr<std::ofstream> foutPtr, RCF::ByteBuffer buffer)
    {
        RCF_LOG_4()(foutPtr.get())((void*)buffer.getPtr())(buffer.getLength()) << "FileIoRequest::write()";

        mFinPtr.reset();
        mFoutPtr = foutPtr;
        mBuffer = buffer;
        mBytesTransferred = 0;
        mInitiated = true;
        mCompleted = false;

        mFts.registerOp( shared_from_this() );

        // For debugging purposes, we can wait in this function until the file I/O is completed.
        if (mFts.mSerializeFileIo)
        {
            RCF::Lock lock(mFts.mCompletionMutex);
            while (!mCompleted)
            {
                mFts.mCompletionCondition.wait(lock);
            }
        }
    }

    void FileIoRequest::doTransfer()
    {
        if (mFinPtr)
        {
            RCF_LOG_4() << "FileIoRequest::doTransfer() - initiate read.";

            char * szBuffer = mBuffer.getPtr();
            std::size_t szBufferLen = mBuffer.getLength();
            mFinPtr->read(szBuffer, szBufferLen);
            mBytesTransferred = mFinPtr->gcount();
            mFinPtr.reset();

            RCF_LOG_4()(mBytesTransferred) << "FileIoRequest::doTransfer() - read complete.";
        }
        else if (mFoutPtr)
        {
            RCF_LOG_4() << "FileIoRequest::doTransfer() - initiate write.";

            char * szBuffer = mBuffer.getPtr();
            std::size_t szBufferLen = mBuffer.getLength();
            
            boost::uint64_t pos0 = mFoutPtr->tellp();
            mFoutPtr->write(szBuffer, szBufferLen);
            boost::uint64_t pos1 = mFoutPtr->tellp();

            RCF_ASSERT_GTEQ(pos1 , pos0);
            mBytesTransferred = pos1 - pos0;
            RCF_ASSERT_EQ(mBytesTransferred , szBufferLen);
            mFoutPtr.reset();

            RCF_LOG_4()(mBytesTransferred) << "FileIoRequest::doTransfer() - write complete.";
        }
        else
        {
            RCF_ASSERT(0);
            mBytesTransferred = 0;
        }
    }

    boost::uint64_t FileIoRequest::getBytesTransferred()
    {
        RCF_LOG_4()(mBytesTransferred) << "FileIoRequest::getBytesTransferred()";

        return mBytesTransferred;
    }

    static FileIoThreadPool * gpFileIoThreadPool = NULL;

    void initFileIoThreadPool()
    {
        gpFileIoThreadPool = new FileIoThreadPool();
    }

    void deinitFileIoThreadPool()
    {
        delete gpFileIoThreadPool; 
        gpFileIoThreadPool = NULL;
    }

    FileIoThreadPool & getFileIoThreadPool()
    {
        FileIoThreadPool * pFileIoThreadPool = gpFileIoThreadPool;
        return *pFileIoThreadPool;
    }

} // namespace RCF
