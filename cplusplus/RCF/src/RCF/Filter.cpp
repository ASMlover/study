
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

#include <RCF/Filter.hpp>

#include <RCF/ByteBuffer.hpp>
#include <RCF/ClientStub.hpp>
#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/Tools.hpp>

#if RCF_FEATURE_SSPI==1
#include <RCF/Schannel.hpp>
#endif

#if RCF_FEATURE_OPENSSL==1
#include <RCF/OpenSslEncryptionFilter.hpp>
#endif

namespace RCF {

    // Filter

    Filter::Filter() :
        mpPreFilter(),
        mpPostFilter()
    {}

    Filter::~Filter()
    {}

    void Filter::resetState()
    {}

    void Filter::setPreFilter(Filter &preFilter)
    {
        mpPreFilter = &preFilter;
    }

    void Filter::setPostFilter(Filter &postFilter)
    {
        mpPostFilter = &postFilter;
    }

    Filter &Filter::getPreFilter()
    {
        return *mpPreFilter;
    }

    Filter &Filter::getPostFilter()
    {
        return *mpPostFilter;
    }


    // IdentityFilter

    void IdentityFilter::read(const ByteBuffer &byteBuffer, std::size_t bytesRequested)
    {
        getPostFilter().read(byteBuffer, bytesRequested);
    }

    void IdentityFilter::write(const std::vector<ByteBuffer> &byteBuffers)
    {
        getPostFilter().write(byteBuffers);
    }

    void IdentityFilter::onReadCompleted(const ByteBuffer &byteBuffer)
    {
        getPreFilter().onReadCompleted(byteBuffer);
    }

    void IdentityFilter::onWriteCompleted(std::size_t bytesTransferred)
    {
        getPreFilter().onWriteCompleted(bytesTransferred);
    }

    int IdentityFilter::getFilterId() const
    {
        return RcfFilter_Identity;
    }

    class ReadProxy : public IdentityFilter
    {
    public:
        ReadProxy() :
            mInByteBufferPos(),
            mBytesTransferred()
        {}

        std::size_t getOutBytesTransferred() const
        {
            return mBytesTransferred;
        }

        ByteBuffer getOutByteBuffer()
        {
            ByteBuffer byteBuffer = mOutByteBuffer;
            mOutByteBuffer = ByteBuffer();
            return byteBuffer;
        }

        void setInByteBuffer(const ByteBuffer &byteBuffer)
        {
            mInByteBuffer = byteBuffer;
        }

        void clear()
        {
            mInByteBuffer = ByteBuffer();
            mOutByteBuffer = ByteBuffer();
            mInByteBufferPos = 0;
            mBytesTransferred = 0;
        }

    private:

        void read(const ByteBuffer &byteBuffer, std::size_t bytesRequested)
        {
            RCF_ASSERT(byteBuffer.isEmpty())(byteBuffer.getLength());

            RCF_ASSERT_LT(mInByteBufferPos, mInByteBuffer.getLength());

            std::size_t bytesRemaining = mInByteBuffer.getLength() - mInByteBufferPos;
            std::size_t bytesToRead = RCF_MIN(bytesRemaining, bytesRequested);
            ByteBuffer myByteBuffer(mInByteBuffer, mInByteBufferPos, bytesToRead);
            mInByteBufferPos += bytesToRead;
            getPreFilter().onReadCompleted(myByteBuffer);
        }

        void onReadCompleted(const ByteBuffer &byteBuffer)
        {
            mOutByteBuffer = byteBuffer;
            mBytesTransferred = byteBuffer.getLength();
        }

        int getFilterId() const
        {
            return RcfFilter_Unknown;
        }

    private:

        std::size_t mInByteBufferPos;
        std::size_t mBytesTransferred;

        ByteBuffer mInByteBuffer;
        ByteBuffer mOutByteBuffer;
    };

    class WriteProxy : public IdentityFilter, boost::noncopyable
    {
    public:
        WriteProxy() :
            mBytesTransferred(),
            mTlcByteBuffers(),
            mByteBuffers(mTlcByteBuffers.get())
        {
        }

        const std::vector<ByteBuffer> &getByteBuffers() const
        {
            return mByteBuffers;
        }

        void clear()
        {
            mByteBuffers.resize(0);
            mBytesTransferred = 0;
        }

        std::size_t getBytesTransferred() const
        {
            return mBytesTransferred;
        }

    private:

        void write(const std::vector<ByteBuffer> &byteBuffers)
        {
            std::copy(
                byteBuffers.begin(),
                byteBuffers.end(),
                std::back_inserter(mByteBuffers));

            std::size_t bytesTransferred = lengthByteBuffers(byteBuffers);
            getPreFilter().onWriteCompleted(bytesTransferred);
        }

        void onWriteCompleted(std::size_t bytesTransferred)
        {
            mBytesTransferred = bytesTransferred;
        }

        int getFilterId() const
        {
            return RcfFilter_Unknown;
        }

    private:
        std::size_t mBytesTransferred;

        RCF::ThreadLocalCached< std::vector<RCF::ByteBuffer> > mTlcByteBuffers;
        std::vector<RCF::ByteBuffer> &mByteBuffers;
    };

    bool filterData(
        const std::vector<ByteBuffer> &unfilteredData,
        std::vector<ByteBuffer> &filteredData,
        const std::vector<FilterPtr> &filters)
    {
        std::size_t bytesTransferred        = 0;
        std::size_t bytesTransferredTotal   = 0;

        WriteProxy writeProxy;
        writeProxy.setPreFilter(*filters.back());
        filters.back()->setPostFilter(writeProxy);
        filters.front()->setPreFilter(writeProxy);

        std::size_t unfilteredDataLen = lengthByteBuffers(unfilteredData);
        while (bytesTransferredTotal < unfilteredDataLen)
        {

            ThreadLocalCached< std::vector<ByteBuffer> > tlcSlicedByteBuffers;
            std::vector<ByteBuffer> &slicedByteBuffers = tlcSlicedByteBuffers.get();
            sliceByteBuffers(slicedByteBuffers, unfilteredData, bytesTransferredTotal);
            filters.front()->write(slicedByteBuffers);

            // TODO: error handling
            bytesTransferred = writeProxy.getBytesTransferred();
            bytesTransferredTotal += bytesTransferred;
        }
        RCF_ASSERT_EQ(bytesTransferredTotal , unfilteredDataLen);

        filteredData.resize(0);

        std::copy(
            writeProxy.getByteBuffers().begin(),
            writeProxy.getByteBuffers().end(),
            std::back_inserter(filteredData));

        return bytesTransferredTotal == unfilteredDataLen;
    }

    bool unfilterData(
        const ByteBuffer &filteredByteBuffer,
        std::vector<ByteBuffer> &unfilteredByteBuffers,
        std::size_t unfilteredDataLen,
        const std::vector<FilterPtr> &filters)
    {
        int error                           = 0;
        std::size_t bytesTransferred        = 0;
        std::size_t bytesTransferredTotal   = 0;

        ByteBuffer byteBuffer;
        unfilteredByteBuffers.resize(0);

        ReadProxy readProxy;
        readProxy.setInByteBuffer(filteredByteBuffer);
        readProxy.setPreFilter(*filters.back());
        filters.back()->setPostFilter(readProxy);
        filters.front()->setPreFilter(readProxy);

        while (!error && bytesTransferredTotal < unfilteredDataLen)
        {
            filters.front()->read(ByteBuffer(), unfilteredDataLen - bytesTransferredTotal);
            bytesTransferred = readProxy.getOutBytesTransferred();
            byteBuffer = readProxy.getOutByteBuffer();
            // TODO: error handling
            bytesTransferredTotal += (error) ? 0 : bytesTransferred;
            unfilteredByteBuffers.push_back(byteBuffer);
        }
        return bytesTransferredTotal == unfilteredDataLen;
    }

    bool unfilterData(
        const ByteBuffer &filteredByteBuffer,
        ByteBuffer &unfilteredByteBuffer,
        std::size_t unfilteredDataLen,
        const std::vector<FilterPtr> &filters)
    {
        ThreadLocalCached< std::vector<ByteBuffer> > tlcUnfilteredByteBuffers;
        std::vector<ByteBuffer> &unfilteredByteBuffers = tlcUnfilteredByteBuffers.get();

        bool ret = unfilterData(
            filteredByteBuffer,
            unfilteredByteBuffers,
            unfilteredDataLen,
            filters);

        if (unfilteredByteBuffers.empty())
        {
            unfilteredByteBuffer = ByteBuffer();
        }
        else if (unfilteredByteBuffers.size() == 1)
        {
            unfilteredByteBuffer = unfilteredByteBuffers[0];
        }
        else
        {
            // TODO: maybe check for adjacent buffers, in which case one should not need to make a copy
            copyByteBuffers(unfilteredByteBuffers, unfilteredByteBuffer);
        }
        return ret;
    }

    void connectFilters(const std::vector<FilterPtr> &filters)
    {
        for (std::size_t i=0; i<filters.size(); ++i)
        {
            if (i>0)
            {
                filters[i]->setPreFilter(*filters[i-1]);
            }
            if (i<filters.size()-1)
            {
                filters[i]->setPostFilter(*filters[i+1]);
            }
        }
    }

} // namespace RCF

#if RCF_FEATURE_ZLIB==1
#include <RCF/ZlibCompressionFilter.hpp>
#endif

#if RCF_FEATURE_OPENSSL==1
#include <RCF/OpenSslEncryptionFilter.hpp>
#endif

#if RCF_FEATURE_SSPI==1
#include <RCF/Schannel.hpp>
#include <RCF/SspiFilter.hpp>
#endif

namespace RCF {

    void ClientStub::createFilterSequence(
        std::vector<FilterPtr> & filters)
    {
        filters.clear();
    
        // Setup compression if configured.
        if (mEnableCompression)
        {
#if RCF_FEATURE_ZLIB==1
            FilterPtr filterPtr( new ZlibStatefulCompressionFilter() );
            filters.push_back(filterPtr);
#else
            RCF_ASSERT(0);
#endif
        }

        FilterPtr filterPtr;
        if (mTransportProtocol != Tp_Clear && mTransportProtocol != Tp_Unspecified)
        {
            switch (mTransportProtocol)
            {
#if RCF_FEATURE_SSPI==1
            case Tp_Ntlm:       filterPtr.reset( new NtlmFilter(this) ); break;
            case Tp_Kerberos:   filterPtr.reset( new KerberosFilter(this) ); break;
            case Tp_Negotiate:  filterPtr.reset( new NegotiateFilter(this) ); break;
#endif

#if RCF_FEATURE_OPENSSL==1 && RCF_FEATURE_SSPI==1
            case Tp_Ssl:        if (mSslImplementation == Si_Schannel)
                                {
                                    filterPtr.reset( new SchannelFilter(this) ); 
                                }
                                else
                                {
                                    RCF_ASSERT(mSslImplementation == Si_OpenSsl);
                                    filterPtr.reset( new OpenSslEncryptionFilter(this) ); 
                                }
                                break;
#elif RCF_FEATURE_OPENSSL==1
            case Tp_Ssl:        filterPtr.reset( new OpenSslEncryptionFilter(this) ); break;
#elif RCF_FEATURE_SSPI==1
            case Tp_Ssl:        filterPtr.reset( new SchannelFilter(this) ); break;
#else
            // Single case just to keep the compiler warnings quiet.
            case Tp_Ssl:
#endif

            default:
                RCF_THROW( Exception( _RcfError_TransportProtocolNotSupported( getTransportProtocolName(mTransportProtocol)) ) );
            }
        }
        if (filterPtr)
        {
            filters.push_back(filterPtr);
        }
    }

} // namespace RCF
