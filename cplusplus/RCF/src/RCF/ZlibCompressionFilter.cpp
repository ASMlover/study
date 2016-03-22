
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

#include <RCF/ZlibCompressionFilter.hpp>

#include "zlib.h"

#include <RCF/DynamicLib.hpp>
#include <RCF/Exception.hpp>
#include <RCF/Globals.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/RecursionLimiter.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    typedef boost::shared_ptr<DynamicLib> DynamicLibPtr;

    class ZlibDll
    {
    public:
        ZlibDll();

        void                loadFunctionPtrs();

        typedef int         (*Pfn_deflateInit_)(z_streamp strm, int level, const char *version, int stream_size);
        typedef int         (*Pfn_deflate)(z_streamp strm, int flush);
        typedef int         (*Pfn_deflateEnd)(z_streamp strm);
        typedef int         (*Pfn_inflateInit_)(z_streamp strm, const char *version, int stream_size);
        typedef int         (*Pfn_inflate)(z_streamp strm, int flush);
        typedef int         (*Pfn_inflateEnd)(z_streamp strm);

        Pfn_deflateInit_    pfn_deflateInit_;
        Pfn_deflate         pfn_deflate;
        Pfn_deflateEnd      pfn_deflateEnd;

        Pfn_inflateInit_    pfn_inflateInit_;
        Pfn_inflate         pfn_inflate;
        Pfn_inflateEnd      pfn_inflateEnd;

        DynamicLibPtr       mDynamicLibPtr;
    };

    ZlibDll::ZlibDll()
    {

#ifndef RCF_ZLIB_STATIC 

        mDynamicLibPtr.reset( new DynamicLib( getGlobals().getZlibDllName() ) );

#endif

        loadFunctionPtrs();
    }

#ifdef RCF_ZLIB_STATIC

// Load static function pointers.
// Requires linking to zlib or building with zlib source.
#define RCF_ZLIB_LOAD_FUNCTION RCF_LOAD_LIB_FUNCTION

#else

// Load dynamic function pointers directly from zlib DLL.
// Requires zlib DLL to be present at runtime.
// Does not require linking to zlib.
#define RCF_ZLIB_LOAD_FUNCTION RCF_LOAD_DLL_FUNCTION

#endif

    void ZlibDll::loadFunctionPtrs()
    {

#ifndef RCF_ZLIB_STATIC
        RCF_ASSERT(mDynamicLibPtr);
#endif

        RCF_ZLIB_LOAD_FUNCTION(deflateInit_)
        RCF_ZLIB_LOAD_FUNCTION(deflate)
        RCF_ZLIB_LOAD_FUNCTION(deflateEnd)

        RCF_ZLIB_LOAD_FUNCTION(inflateInit_)
        RCF_ZLIB_LOAD_FUNCTION(inflate)
        RCF_ZLIB_LOAD_FUNCTION(inflateEnd)

    }

    ZlibDll & Globals::getZlibDll()
    {
        Lock lock(getRootMutex());

        if (!mpZlibDll)
        {
            mpZlibDll = new ZlibDll();
        }
        return *mpZlibDll;
    }

#if RCF_FEATURE_ZLIB==1

    void Globals::deleteZlibDll()
    {
        if (mpZlibDll)
        {
            delete mpZlibDll;
            mpZlibDll = NULL;
        }
    }

#endif

    class ZlibCompressionReadFilter
    {
    public:
        ZlibCompressionReadFilter(
            ZlibCompressionFilterBase &filter,
            bool serverSide);

        ~ZlibCompressionReadFilter();
        void reset();
        void read(const ByteBuffer &byteBuffer, std::size_t bytesRequested);
        void onReadCompleted(const ByteBuffer &byteBuffer);

        void clearPreBuffer();

    private:
        void resetDecompressionState();
        bool decompress();

        ZlibCompressionFilterBase & mFilter;
        z_stream                    mDstream;
        std::size_t                 mBytesRequested;
        ByteBuffer                  mPreBuffer;
        ByteBuffer                  mPostBuffer;
        int                         mZerr;
        bool                        mDecompressionStateInited;

        ByteBuffer                  mOrigBuffer;
        ReallocBufferPtr            mVecPtr;
    };

    class ZlibCompressionWriteFilter
    {
    public:
        ZlibCompressionWriteFilter(
            ZlibCompressionFilterBase &filter,
            bool stateful);

        ~ZlibCompressionWriteFilter();
        void reset();
        void write(const std::vector<ByteBuffer> &byteBuffers);
        void onWriteCompleted(std::size_t bytesTransferred);

    private:
        void resetCompressionState();
        void compress();

        ZlibCompressionFilterBase & mFilter;
        z_stream                    mCstream;
        std::size_t                 mTotalBytesIn;
        std::size_t                 mTotalBytesOut;
        int                         mZerr;
        bool                        mCompressionStateInited;
        const bool                  mStateful;

        std::vector<ByteBuffer>     mPostBuffers;
        std::vector<ByteBuffer>     mPreBuffers;

        ReallocBufferPtr            mVecPtr;
    };

    const static std::size_t ZlibFilterBandwidthLimit = 1024*1024;

    ZlibCompressionReadFilter::ZlibCompressionReadFilter(
        ZlibCompressionFilterBase &filter,
        bool serverSide) :
            mFilter(filter),
            mDstream(),
            mBytesRequested(),
            mZerr(Z_OK),
            mDecompressionStateInited()
    {
        RCF_UNUSED_VARIABLE(serverSide);

        memset(&mDstream, 0, sizeof(mDstream));

        resetDecompressionState();
    }

    ZlibCompressionReadFilter::~ZlibCompressionReadFilter()
    {
        RCF_DTOR_BEGIN
            if (mDecompressionStateInited)
            {
                mZerr = mFilter.mZlibDll.pfn_inflateEnd(&mDstream);
                RCF_VERIFY(
                    mZerr == Z_OK,
                    FilterException(
                        _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                        "inflateEnd() failed"))(mZerr);
                mDecompressionStateInited = false;
            }
        RCF_DTOR_END
    }

    void ZlibCompressionReadFilter::clearPreBuffer()
    {
        mPreBuffer.clear();
    }

    void ZlibCompressionReadFilter::reset()
    {
        resetDecompressionState();
    }

    void ZlibCompressionReadFilter::resetDecompressionState()
    {
        if (mDecompressionStateInited)
        {
            mZerr = mFilter.mZlibDll.pfn_inflateEnd(&mDstream);

            RCF_VERIFY(
                mZerr == Z_OK,
                FilterException(
                    _RcfError_Zlib(),
                    mZerr,
                    RcfSubsystem_Zlib,
                    "inflateEnd() failed"))
                (mZerr);

            mDecompressionStateInited = false;
        }
        mDstream.zalloc = NULL;
        mDstream.zfree = NULL;
        mDstream.opaque = NULL;
        mZerr = mFilter.mZlibDll.pfn_inflateInit_(&mDstream, ZLIB_VERSION, sizeof(mDstream));
        RCF_VERIFY(
            mZerr == Z_OK,
            FilterException(
                _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                "inflateInit() failed"))(mZerr);
        mDecompressionStateInited = true;
    }

    // TODO: do the right thing with the byteBuffer parameter
    void ZlibCompressionReadFilter::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        if (byteBuffer.isEmpty() && bytesRequested == 0)
        {
            if (mPostBuffer.getLength() > 0)
            {
                mFilter.getPreFilter().onReadCompleted(byteBuffer);
            }
            else
            {
                mBytesRequested = bytesRequested;
                mFilter.getPostFilter().read(ByteBuffer(), 0);
            }
        }
        else
        {
            bytesRequested = RCF_MIN( ZlibFilterBandwidthLimit, bytesRequested);

            mOrigBuffer = ByteBuffer(
                byteBuffer,
                0,
                RCF_MIN(byteBuffer.getLength(), bytesRequested));

            if (mPostBuffer.getLength() > 0)
            {
                onReadCompleted(mPostBuffer);
            }
            else
            {
                mPostBuffer = ByteBuffer();

                mBytesRequested = bytesRequested;

                // The bigger the chunk size, the better. Need a lower limit 
                // on the chunk size, though, as zlib seems to have problems 
                // if it is too small.
                std::size_t bytesToRead = RCF_MAX(
                    std::size_t(4096), 
                    bytesRequested);

                mFilter.getPostFilter().read(ByteBuffer(), bytesToRead);
            }
        }
    }

    void ZlibCompressionReadFilter::onReadCompleted(
        const ByteBuffer &byteBuffer)
    {
        if (mBytesRequested == 0)
        {
            RCF_ASSERT(byteBuffer.isEmpty());
            mFilter.getPreFilter().onReadCompleted(ByteBuffer());
        }
        else 
        {
            if (mOrigBuffer.getLength() > 0)
            {
                mPreBuffer = mOrigBuffer;
            }
            else
            {
                if (mVecPtr.get() == NULL || !mVecPtr.unique())
                {
                    mVecPtr.reset( new ReallocBuffer());
                }
                mVecPtr->resize(mBytesRequested);
                mPreBuffer = ByteBuffer(mVecPtr);
                mOrigBuffer = mPreBuffer;
            }
            mPostBuffer = byteBuffer;
            if (decompress())
            {
                mOrigBuffer.clear();
                mFilter.getPreFilter().onReadCompleted(mPreBuffer);
            }
            else
            {
                mPreBuffer.clear();
                read(mOrigBuffer, mBytesRequested);
            }
        }
    }

    bool ZlibCompressionReadFilter::decompress()
    {
        mDstream.next_in = (Bytef*) mPostBuffer.getPtr();
        mDstream.avail_in = static_cast<uInt>(mPostBuffer.getLength());
        mDstream.next_out = (Bytef*) mPreBuffer.getPtr();
        mDstream.avail_out = static_cast<uInt>(mPreBuffer.getLength());
        mZerr = mFilter.mZlibDll.pfn_inflate(&mDstream, Z_SYNC_FLUSH);
        RCF_VERIFY(
            mZerr == Z_OK || mZerr == Z_STREAM_END || mZerr == Z_BUF_ERROR,
            FilterException(
                _RcfError_ZlibInflate(), mZerr, RcfSubsystem_Zlib,
                "inflate() failed"))
            (mZerr)(mPreBuffer.getLength())(mPostBuffer.getLength());
        if (mZerr == Z_STREAM_END)
        {
            resetDecompressionState();
        }

        mPreBuffer = ByteBuffer(
            mPreBuffer,
            0,
            mPreBuffer.getLength() - mDstream.avail_out);

        mPostBuffer = ByteBuffer(
            mPostBuffer,
            mPostBuffer.getLength() - mDstream.avail_in);

        if (mPostBuffer.getLength() == 0)
        {
            mPostBuffer.clear();
        }

        return mPreBuffer.getLength() > 0;
    }

    ZlibCompressionWriteFilter::ZlibCompressionWriteFilter(
        ZlibCompressionFilterBase &filter,
        bool stateful) :
            mFilter(filter),
            mCstream(),
            mTotalBytesIn(),
            mTotalBytesOut(),
            mZerr(Z_OK),
            mCompressionStateInited(),
            mStateful(stateful)
    {
        memset(&mCstream, 0, sizeof(mCstream));
    }

    ZlibCompressionWriteFilter::~ZlibCompressionWriteFilter()
    {
        RCF_DTOR_BEGIN
            if (mCompressionStateInited)
            {
                mZerr = mFilter.mZlibDll.pfn_deflateEnd(&mCstream);
                RCF_VERIFY(
                    mZerr == Z_OK || mZerr == Z_DATA_ERROR,
                    FilterException(
                        _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                        "deflateEnd() failed"))(mZerr)(mStateful);
                mCompressionStateInited = false;
            }
        RCF_DTOR_END
    }

    void ZlibCompressionWriteFilter::reset()
    {
        resetCompressionState();
    }

    void ZlibCompressionWriteFilter::resetCompressionState()
    {
        if (mCompressionStateInited)
        {
            mZerr = mFilter.mZlibDll.pfn_deflateEnd(&mCstream);
            RCF_VERIFY(
                mZerr == Z_OK || mZerr == Z_DATA_ERROR,
                FilterException(
                    _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                    "deflateEnd() failed"))(mZerr);
            mCompressionStateInited = false;
        }
        mCstream.zalloc = NULL;
        mCstream.zfree = NULL;
        mCstream.opaque = NULL;
        mZerr = mFilter.mZlibDll.pfn_deflateInit_(&mCstream, Z_DEFAULT_COMPRESSION, ZLIB_VERSION, sizeof(mCstream));
        RCF_VERIFY(
            mZerr == Z_OK,
            FilterException(
                _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                "deflateInit() failed"))(mZerr);
        mCompressionStateInited = true;
    }

    void ZlibCompressionWriteFilter::write(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        if (mStateful == false || mCompressionStateInited == false)
        {
            resetCompressionState();
        }

        sliceByteBuffers(mPreBuffers, byteBuffers, 0, ZlibFilterBandwidthLimit);

        compress();
        mFilter.getPostFilter().write(mPostBuffers);
    }

    void ZlibCompressionWriteFilter::onWriteCompleted(
        std::size_t bytesTransferred)
    {
        // 1. if partial buffer was written -> write remaining part of buffer
        // 2. if whole buffer was written -> check if any more compression or writing is needed
        // 3. if no more compression or writing needed, notify previous filter of completion

        RCF_ASSERT_LTEQ(bytesTransferred , lengthByteBuffers(mPostBuffers));

        if (bytesTransferred < lengthByteBuffers(mPostBuffers))
        {
            // TODO: optimize
            std::vector<ByteBuffer> slicedBuffers;
            sliceByteBuffers(slicedBuffers, mPostBuffers, bytesTransferred);
            mPostBuffers = slicedBuffers;
            mFilter.getPostFilter().write(mPostBuffers);
        }
        else
        {
            mPreBuffers.resize(0);
            mPostBuffers.resize(0);
            mFilter.getPreFilter().onWriteCompleted(mTotalBytesIn);
        }
    }

    void ZlibCompressionWriteFilter::compress()
    {
        mPostBuffers.resize(0);

        // TODO: buffer size
        std::size_t bufferSize = 2*(lengthByteBuffers(mPreBuffers)+7+7);
        std::size_t leftMargin = mPreBuffers.front().getLeftMargin();

        if (mVecPtr.get() == NULL || !mVecPtr.unique())
        {
            mVecPtr.reset( new ReallocBuffer());
        }
        mVecPtr->resize(leftMargin + bufferSize);

        if (leftMargin > 0)
        {
            mPostBuffers.push_back(
                ByteBuffer(
                    &(*mVecPtr)[0] + leftMargin,
                    mVecPtr->size() - leftMargin,
                    leftMargin,
                    mVecPtr));
        }
        else
        {
            mPostBuffers.push_back(
                ByteBuffer(
                    &(*mVecPtr)[0],
                    mVecPtr->size(),
                    mVecPtr));
        }

        ByteBuffer &outBuffer = mPostBuffers.back();
        std::size_t outPos = 0;
        std::size_t outRemaining = outBuffer.getLength() - outPos;
        mTotalBytesIn = 0;
        mTotalBytesOut = 0;

        for (std::size_t i=0; i<mPreBuffers.size(); ++i)
        {
            RCF_ASSERT_LT(outPos , outBuffer.getLength());

            ByteBuffer &inBuffer = mPreBuffers[i];
            mCstream.next_in = (Bytef*) inBuffer.getPtr();
            mCstream.avail_in = static_cast<uInt>(inBuffer.getLength());
            mCstream.next_out = (Bytef*) &outBuffer.getPtr()[outPos];
            mCstream.avail_out = static_cast<uInt>(outRemaining);

            mZerr = (i<mPreBuffers.size()-1) ?
                mFilter.mZlibDll.pfn_deflate(&mCstream, Z_NO_FLUSH) :
                mFilter.mZlibDll.pfn_deflate(&mCstream, Z_SYNC_FLUSH);

            RCF_VERIFY(
                mZerr == Z_OK || mZerr == Z_BUF_ERROR,
                FilterException(
                    _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                    "deflate() failed"))
                (mZerr)(inBuffer.getLength())(outBuffer.getLength());

            std::size_t bytesIn = inBuffer.getLength() - mCstream.avail_in;
            std::size_t bytesOut = outRemaining - mCstream.avail_out;
            mTotalBytesIn += bytesIn;
            mTotalBytesOut += bytesOut;
            outPos += bytesOut;
            outRemaining -= bytesOut;
        }

        if (!mStateful)
        {
            mCstream.next_in = NULL;
            mCstream.avail_in = 0;
            mCstream.next_out = (Bytef*) &outBuffer.getPtr()[outPos];
            mCstream.avail_out = static_cast<uInt>(outRemaining);

            mZerr = mFilter.mZlibDll.pfn_deflate(&mCstream, Z_FINISH);
            RCF_VERIFY(
                mZerr == Z_BUF_ERROR || mZerr == Z_STREAM_END,
                FilterException(
                    _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                    "deflate() failed"))
                    (mZerr)(outPos)(outRemaining);

            RCF_ASSERT_GT(mCstream.avail_out , 0);

            std::size_t bytesOut = outRemaining - mCstream.avail_out;
            mTotalBytesOut += bytesOut;
            outPos += bytesOut;
            outRemaining -= bytesOut;
        }

        mPreBuffers.resize(0);
        outBuffer = ByteBuffer(outBuffer, 0, mTotalBytesOut);
    }

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4355 )  // warning C4355: 'this' : used in base member initializer list
#endif

    ZlibCompressionFilterBase::ZlibCompressionFilterBase(bool stateful, bool serverSide) :
        mZlibDll(getGlobals().getZlibDll()),
        mPreState(Ready),
        mReadFilter( new ZlibCompressionReadFilter(*this, serverSide) ),
        mWriteFilter( new ZlibCompressionWriteFilter(*this, stateful) )
    {}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

    int ZlibStatelessCompressionFilter::getFilterId() const
    {
        return RcfFilter_ZlibCompressionStateless;
    }

    int ZlibStatefulCompressionFilter::getFilterId() const
    {
        return RcfFilter_ZlibCompressionStateful;
    }

    void ZlibCompressionFilterBase::resetState()
    {
        mPreState = Ready;
        mReadFilter->reset();
        mWriteFilter->reset();
    }

    void ZlibCompressionFilterBase::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        mReadFilter->clearPreBuffer();

        mPreState = Reading;
        mReadFilter->read(byteBuffer, bytesRequested);
    }

    void ZlibCompressionFilterBase::write(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        mReadFilter->clearPreBuffer();

        mPreState = Writing;
        mWriteFilter->write(byteBuffers);
    }

    void ZlibCompressionFilterBase::onReadCompleted(
        const ByteBuffer &byteBuffer)
    {
        mReadFilter->onReadCompleted(byteBuffer);
    }

    void ZlibCompressionFilterBase::onWriteCompleted(
        std::size_t bytesTransferred)
    {
        mWriteFilter->onWriteCompleted(bytesTransferred);
    }

    ZlibStatelessCompressionFilterFactory::ZlibStatelessCompressionFilterFactory()
    {}

    FilterPtr ZlibStatelessCompressionFilterFactory::createFilter(RcfServer &)
    {
        return FilterPtr( new ZlibStatelessCompressionFilter(
            (ServerSide *) NULL));
    }

    int ZlibStatelessCompressionFilterFactory::getFilterId()
    {
        return RcfFilter_ZlibCompressionStateless;
    }

    ZlibStatefulCompressionFilterFactory::ZlibStatefulCompressionFilterFactory()
    {}

    FilterPtr ZlibStatefulCompressionFilterFactory::createFilter(RcfServer &)
    {
        return FilterPtr( new ZlibStatefulCompressionFilter( 
            (ServerSide *) NULL));
    }

    int ZlibStatefulCompressionFilterFactory::getFilterId()
    {
        return RcfFilter_ZlibCompressionStateful;
    }

} // namespace RCF
