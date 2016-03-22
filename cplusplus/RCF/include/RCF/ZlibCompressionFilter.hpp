
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

#ifndef INCLUDE_RCF_ZLIBCOMPRESSIONFILTER_HPP
#define INCLUDE_RCF_ZLIBCOMPRESSIONFILTER_HPP

#include <memory>
#include <vector>

#include <boost/noncopyable.hpp>

#include <RCF/Filter.hpp>
#include <RCF/Export.hpp>

namespace RCF {

    static const int ZlibDefaultBufferSize = 4096;

    class ZlibCompressionReadFilter;
    class ZlibCompressionWriteFilter;
    class ZlibDll;

    class RCF_EXPORT ZlibCompressionFilterBase : 
        public Filter, 
        boost::noncopyable
    {
    public:
        ZlibCompressionFilterBase(bool stateful, bool serverSide);
       
    private:

        ZlibDll & mZlibDll;

        void resetState();

        void read(const ByteBuffer &byteBuffer, std::size_t bytesRequested);
        void write(const std::vector<ByteBuffer> &byteBuffers);
        void onReadCompleted(const ByteBuffer &byteBuffer);
        void onWriteCompleted(std::size_t bytesTransferred);

        enum IoState
        {
            Ready,
            Reading,
            Writing
        };

        // input state
        IoState mPreState;

        friend class ZlibCompressionReadFilter;
        friend class ZlibCompressionWriteFilter;

        boost::shared_ptr<ZlibCompressionReadFilter> mReadFilter;
        boost::shared_ptr<ZlibCompressionWriteFilter> mWriteFilter;
    };

    class ServerSide {};

    class RCF_EXPORT ZlibStatelessCompressionFilter : 
        public ZlibCompressionFilterBase
    {
    private:
        friend class ZlibStatelessCompressionFilterFactory;

        ZlibStatelessCompressionFilter(
            ServerSide *) :
                ZlibCompressionFilterBase(false, true)
        {}

    public:
        ZlibStatelessCompressionFilter() :
                ZlibCompressionFilterBase(false, false)
        {}        

        int getFilterId() const;
    };

    class RCF_EXPORT ZlibStatefulCompressionFilter : 
        public ZlibCompressionFilterBase
    {
    private:
        friend class ZlibStatefulCompressionFilterFactory;

        ZlibStatefulCompressionFilter(
            ServerSide *) :
                ZlibCompressionFilterBase(true, true)
        {}

    public:
        ZlibStatefulCompressionFilter() :
                ZlibCompressionFilterBase(true, false)
        {}

        int getFilterId() const;
    };
   
    class ZlibStatelessCompressionFilterFactory : 
        public FilterFactory
    {
    public:
        ZlibStatelessCompressionFilterFactory();

        FilterPtr createFilter(RcfServer & server);
        int getFilterId();
    };

    class ZlibStatefulCompressionFilterFactory : 
        public FilterFactory
    {
    public:
        ZlibStatefulCompressionFilterFactory();

        FilterPtr createFilter(RcfServer & server);
        int getFilterId();
    };

    typedef ZlibStatefulCompressionFilter               ZlibCompressionFilter;
    typedef boost::shared_ptr<ZlibCompressionFilter>    ZlibCompressionFilterPtr;

    typedef ZlibStatefulCompressionFilterFactory                ZlibCompressionFilterFactory;
    typedef boost::shared_ptr<ZlibCompressionFilterFactory>     ZlibCompressionFilterFactoryPtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_ZLIBCOMPRESSIONFILTER_HPP
