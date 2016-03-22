
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

#ifndef INCLUDE_RCF_FILTER_HPP
#define INCLUDE_RCF_FILTER_HPP

#include <string>
#include <vector>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/ByteBuffer.hpp>
#include <RCF/Enums.hpp>
#include <RCF/Export.hpp>

namespace RCF {

    // collect these constants here so we can avoid collisions
    static const int RcfFilter_Unknown                      = 0;
    static const int RcfFilter_Identity                     = 1;
    static const int RcfFilter_OpenSsl                      = 2;
    static const int RcfFilter_ZlibCompressionStateless     = 3;
    static const int RcfFilter_ZlibCompressionStateful      = 4;
    static const int RcfFilter_SspiNtlm                     = 5;
    static const int RcfFilter_SspiKerberos                 = 6;
    static const int RcfFilter_SspiNegotiate                = 7;
    static const int RcfFilter_SspiSchannel                 = 8;

    static const int RcfFilter_Xor                          = 101;

    class Filter;

    typedef boost::shared_ptr<Filter> FilterPtr;

    class RCF_EXPORT Filter
    {
    public:

        Filter();
        virtual ~Filter();
        virtual void resetState();

        // TODO: for generality, should take a vector<ByteBuffer> &
        // (applicable if message arrives fragmented through the transport)
        // BTW, bytesRequested is meaningful if byteBuffer is empty
        virtual void read(
            const ByteBuffer &byteBuffer,
            std::size_t bytesRequested) = 0;
       
        virtual void write(const std::vector<ByteBuffer> &byteBuffers) = 0;

        virtual void onReadCompleted(const ByteBuffer &byteBuffer) = 0;

        virtual void onWriteCompleted(std::size_t bytesTransferred) = 0;

        virtual int getFilterId() const = 0;

        void setPreFilter(Filter &preFilter);
        void setPostFilter(Filter &postFilter);

        virtual std::size_t getFrameSize()
        {
            return 0;
        }

    protected:

        Filter &getPreFilter();
        Filter &getPostFilter();

        Filter *mpPreFilter;
        Filter *mpPostFilter;
    };

    class RCF_EXPORT IdentityFilter : public Filter
    {
    public:
        void read(const ByteBuffer &byteBuffer, std::size_t bytesRequested);
        void write(const std::vector<ByteBuffer> &byteBuffers);
        void onReadCompleted(const ByteBuffer &byteBuffer);
        void onWriteCompleted(std::size_t bytesTransferred);
       
        virtual int getFilterId() const;
    };
   
    typedef boost::shared_ptr<Filter>                       FilterPtr;
    typedef std::vector<FilterPtr>                          VectorFilter;
    typedef boost::shared_ptr< std::vector<FilterPtr> >     VectorFilterPtr;

    class RcfServer;

    class FilterFactory
    {
    public:
        virtual ~FilterFactory()
        {}

        virtual FilterPtr createFilter(RcfServer & server) = 0;

        virtual int getFilterId() = 0;
    };

    typedef boost::shared_ptr<FilterFactory> FilterFactoryPtr;

    RCF_EXPORT void connectFilters(const std::vector<FilterPtr> &filters);

    RCF_EXPORT bool filterData(
        const std::vector<ByteBuffer> &unfilteredData,
        std::vector<ByteBuffer> &filteredData,
        const std::vector<FilterPtr> &filters);

    RCF_EXPORT bool unfilterData(
        const ByteBuffer &filteredByteBuffer,
        std::vector<ByteBuffer> &unfilteredByteBuffers,
        std::size_t unfilteredDataLen,
        const std::vector<FilterPtr> &filters);

    RCF_EXPORT bool unfilterData(
        const ByteBuffer &filteredByteBuffer,
        ByteBuffer &unfilteredByteBuffer,
        std::size_t unfilteredDataLen,
        const std::vector<FilterPtr> &filters);

} // namespace RCF

#endif // ! INCLUDE_RCF_FILTER_HPP
