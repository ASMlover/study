
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

#ifndef INCLUDE_RCF_BYTEBUFFER_HPP
#define INCLUDE_RCF_BYTEBUFFER_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/MemStream.hpp>
#include <RCF/MinMax.hpp>
#include <RCF/ReallocBuffer.hpp>

namespace RCF {

    class MemOstream;
    typedef boost::shared_ptr<MemOstream> MemOstreamPtr;

    // ByteBuffer class for facilitating zero-copy transmission and reception

    class RCF_EXPORT ByteBuffer
    {
    public:

        ByteBuffer();

        explicit
        ByteBuffer(std::size_t pvlen);

        explicit
        ByteBuffer(
            const std::vector<char> & vc);

        explicit
        ByteBuffer(
            const std::string & s);

        explicit
        ByteBuffer(
            boost::shared_ptr<std::vector<char> > spvc,
            bool readOnly = false);

        explicit
        ByteBuffer(
            ReallocBufferPtr sprb,
            bool readOnly = false);

        explicit
        ByteBuffer(
            MemOstreamPtr spos,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            std::size_t leftMargin,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            boost::shared_ptr<MemOstream> spos,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            std::size_t leftMargin,
            boost::shared_ptr<MemOstream> spos,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            boost::shared_ptr<std::vector<char> > spvc,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            std::size_t leftMargin,
            boost::shared_ptr<std::vector<char> > spvc,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            ReallocBufferPtr sprb,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            std::size_t leftMargin,
            ReallocBufferPtr sprb,
            bool readOnly = false);

        ByteBuffer(
            const ByteBuffer & byteBuffer,
            std::size_t offset = 0,
            std::size_t len = std::size_t(-1));

        char *              getPtr()            const;
        std::size_t         getLength()         const;
        std::size_t         getLeftMargin()     const;
        bool                getReadOnly()       const;
        bool                isEmpty()           const;
        std::string         string()            const;

        void                setLeftMargin(std::size_t len);
        void                expandIntoLeftMargin(std::size_t len);
        ByteBuffer          release();
        void                swap(ByteBuffer & rhs);
        void                clear();

                            operator bool();
        bool                operator !();

        static const std::size_t npos;

    private:
        // sentries
        boost::shared_ptr< std::vector<char> >      mSpvc;
        boost::shared_ptr< MemOstream >             mSpos;
        boost::shared_ptr< ReallocBuffer >          mSprb;

        char *                                      mPv;
        std::size_t                                 mPvlen;
        std::size_t                                 mLeftMargin;
        bool                                        mReadOnly;
    };

    RCF_EXPORT bool operator==(const ByteBuffer &lhs, const ByteBuffer &rhs);

    RCF_EXPORT std::size_t lengthByteBuffers(
        const std::vector<ByteBuffer> &byteBuffers);

    template<typename Functor>
    inline void forEachByteBuffer(
        const Functor &functor,
        const std::vector<ByteBuffer> &byteBuffers,
        std::size_t offset,
        std::size_t length = -1)
    {
        std::size_t pos0        = 0;
        std::size_t pos1        = 0;
        std::size_t remaining   = length;

        for (std::size_t i=0; i<byteBuffers.size(); ++i)
        {
            pos1 = pos0 + byteBuffers[i].getLength() ;

            if (pos1 <= offset)
            {
                pos0 = pos1;
            }
            else if (pos0 <= offset && offset < pos1)
            {
                std::size_t len = RCF_MIN(pos1-offset, remaining);

                ByteBuffer byteBuffer(
                    byteBuffers[i],
                    offset-pos0,
                    len);

                functor(byteBuffer);
                pos0 = pos1;
                remaining -= len;
            }
            else if (remaining > 0)
            {
                std::size_t len = RCF_MIN(pos1-pos0, remaining);

                ByteBuffer byteBuffer(
                    byteBuffers[i],
                    0,
                    len);

                functor(byteBuffer);
                pos1 = pos0;
                remaining -= len;
            }
        }
    }
    
    RCF_EXPORT ByteBuffer sliceByteBuffer(
        const std::vector<ByteBuffer> &slicedBuffers,
        std::size_t offset,
        std::size_t length = -1);

    RCF_EXPORT void sliceByteBuffers(
        std::vector<ByteBuffer> &slicedBuffers,
        const std::vector<ByteBuffer> &byteBuffers,
        std::size_t offset,
        std::size_t length = std::size_t(-1));

    RCF_EXPORT void copyByteBuffers(
        const std::vector<ByteBuffer> &byteBuffers,
        char *pch);

    RCF_EXPORT void copyByteBuffers(
        const std::vector<ByteBuffer> &byteBuffers,
        ByteBuffer &byteBuffer);

} // namespace RCF

namespace SF {

    class Archive;

    RCF_EXPORT void serialize(SF::Archive &ar, RCF::ByteBuffer &byteBuffer);

}

#endif // ! INCLUDE_RCF_BYTEBUFFER_HPP
