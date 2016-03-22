
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

#ifndef INCLUDE_RCF_HTTPSESSIONFILTER_HPP
#define INCLUDE_RCF_HTTPSESSIONFILTER_HPP

#include <RCF/Filter.hpp>
#include <RCF/ByteBuffer.hpp>

namespace RCF {

    class AsioNetworkSession;

    class RcfSession;
    typedef boost::shared_ptr<RcfSession> RcfSessionPtr;

    class HttpSession
    {
    public:

        HttpSession(const std::string & httpSessionId);
        ~HttpSession();

        RcfSessionPtr               mRcfSessionPtr;

        bool                        mRequestInProgress;
        boost::uint32_t             mLastTouchMs;

        std::string                 mHttpSessionId;
        boost::uint32_t             mHttpSessionIndex;
        std::vector<FilterPtr>      mTransportFilters;

        ByteBuffer                  mCachedReadBuffer;
        std::size_t                 mCachedReadBytesRequested;


    };

    typedef boost::shared_ptr<HttpSession> HttpSessionPtr;

    class HttpSessionFilter : public Filter
    {
    public:

        HttpSessionFilter(AsioNetworkSession& networkSession);
        ~HttpSessionFilter();

        virtual void resetState();

        virtual void read(
            const ByteBuffer &byteBuffer,
            std::size_t bytesRequested);

        virtual void onReadCompleted(const ByteBuffer &byteBuffer);

        virtual void write(const std::vector<ByteBuffer> &byteBuffers);

        virtual void onWriteCompleted(std::size_t bytesTransferred);

        virtual int getFilterId() const;

        ByteBuffer                      mReadBuffer;

        std::vector<ByteBuffer>         mWriteBuffers;

        AsioNetworkSession &            mNetworkSession;
        HttpSessionPtr                  mHttpSessionPtr;

        const std::vector<FilterPtr>    mNoFilters;

        char                            mDummy;

    };

} // namespace RCF


#endif // ! INCLUDE_RCF_HTTPSESSIONFILTER_HPP
