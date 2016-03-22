
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

#include <RCF/ByteBuffer.hpp>

#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    const std::size_t ByteBuffer::npos = std::size_t(-1);

    ByteBuffer::ByteBuffer() :
        mPv(),
        mPvlen(),
        mLeftMargin(),
        mReadOnly()
    {}

    ByteBuffer::ByteBuffer(std::size_t pvlen) :
        mSpvc(new std::vector<char>(pvlen)),
        mPv( mSpvc->empty() ? NULL : &mSpvc->front()),
        mPvlen(pvlen),
        mLeftMargin(),
        mReadOnly()
        
    {}

    ByteBuffer::ByteBuffer(const std::vector<char> & vc) :
        mSpvc( new std::vector<char>(vc) ),
        mSpos(),
        mPv( mSpvc->empty() ? NULL : const_cast<char*>(&mSpvc->front())),
        mPvlen(mSpvc->size()),
        mLeftMargin(),
        mReadOnly(false)
    {}

    ByteBuffer::ByteBuffer(const std::string & s) :
        mSpvc( new std::vector<char>(s.c_str(), s.c_str() + s.length()) ),
        mSpos(),
        mPv( mSpvc->empty() ? NULL : const_cast<char*>(&mSpvc->front())),
        mPvlen(mSpvc->size()),
        mLeftMargin(),
        mReadOnly(false)
    {}

    ByteBuffer::ByteBuffer(
        boost::shared_ptr<std::vector<char> > spvc,
        bool readOnly) :
            mSpvc(spvc),
            mSpos(),
            mPv( spvc->empty() ? NULL : const_cast<char*>(&spvc->front())),
            mPvlen(spvc->size()),
            mLeftMargin(),
            mReadOnly(readOnly)
            
    {}

    ByteBuffer::ByteBuffer(
        ReallocBufferPtr sprb,
        bool readOnly) :
            mSpos(),
            mSprb(sprb),
            mPv( sprb->empty() ? NULL : const_cast<char*>(&(*sprb)[0]) ),
            mPvlen(sprb->size()),
            mLeftMargin(),
            mReadOnly(readOnly)
    {}

    ByteBuffer::ByteBuffer(
        MemOstreamPtr spos,
        bool readOnly) :
            mSpos(spos),
            mSprb(),
            mPv( spos->str() ),
            mPvlen( spos->length() ),
            mLeftMargin(),
            mReadOnly(readOnly)
    {}


    ByteBuffer::ByteBuffer(
        char *pv,
        std::size_t pvlen,
        bool readOnly) :
            mPv(pv),
            mPvlen(pvlen),
            mLeftMargin(),
            mReadOnly(readOnly)
    {}

    ByteBuffer::ByteBuffer(
        char *pv,
        std::size_t pvlen,
        std::size_t leftMargin,
        bool readOnly) :
            mPv(pv),
            mPvlen(pvlen),
            mLeftMargin(leftMargin),
            mReadOnly(readOnly)
    {}

    ByteBuffer::ByteBuffer(
        char *pv,
        std::size_t pvlen,
        boost::shared_ptr<RCF::MemOstream> spos,
        bool readOnly) :
            mSpos(spos),
            mPv(pv),
            mPvlen(pvlen),
            mLeftMargin(),
            mReadOnly(readOnly)
    {}

    ByteBuffer::ByteBuffer(
        char *pv,
        std::size_t pvlen,
        std::size_t leftMargin,
        boost::shared_ptr<RCF::MemOstream> spos,
        bool readOnly) :
            mSpos(spos),
            mPv(pv),
            mPvlen(pvlen),
            mLeftMargin(leftMargin),
            mReadOnly(readOnly)
    {}

    ByteBuffer::ByteBuffer(
        char *pv,
        std::size_t pvlen,
        boost::shared_ptr<std::vector<char> > spvc,
        bool readOnly) :
            mSpvc(spvc),          
            mPv(pv),
            mPvlen(pvlen),
            mLeftMargin(),
            mReadOnly(readOnly)
    {}

    ByteBuffer::ByteBuffer(
        char *pv,
        std::size_t pvlen,
        std::size_t leftMargin,
        boost::shared_ptr<std::vector<char> > spvc,
        bool readOnly) :
            mSpvc(spvc),
            mPv(pv),
            mPvlen(pvlen),
            mLeftMargin(leftMargin),
            mReadOnly(readOnly)
    {}

    ByteBuffer::ByteBuffer(
        char *pv,
        std::size_t pvlen,
        ReallocBufferPtr sprb,
        bool readOnly) :
            mSprb(sprb),
            mPv(pv),
            mPvlen(pvlen),
            mLeftMargin(),
            mReadOnly(readOnly)
    {}

    ByteBuffer::ByteBuffer(
        char *pv,
        std::size_t pvlen,
        std::size_t leftMargin,
        ReallocBufferPtr sprb,
        bool readOnly) :
            mSprb(sprb),
            mPv(pv),
            mPvlen(pvlen),
            mLeftMargin(leftMargin),
            mReadOnly(readOnly)
    {}


    ByteBuffer::ByteBuffer(
        const ByteBuffer &byteBuffer,
        std::size_t offset,
        std::size_t len) :
            mSpvc(byteBuffer.mSpvc),
            mSpos(byteBuffer.mSpos),
            mSprb(byteBuffer.mSprb),
            mPv(byteBuffer.mPv + offset),
            mPvlen( len == npos ? byteBuffer.mPvlen-offset : len),
            mLeftMargin( offset ? 0 : byteBuffer.mLeftMargin),
            mReadOnly(byteBuffer.mReadOnly)
            
    {
        RCF_ASSERT_LTEQ(offset , byteBuffer.mPvlen);

        RCF_ASSERT(
            len == npos || offset+len <= byteBuffer.mPvlen)
            (offset)(len)(byteBuffer.mPvlen);
    }

    ByteBuffer::operator bool()
    {
        return getLength() != 0;
    }

    bool ByteBuffer::operator !()
    {
        return getLength() == 0;
    }

    bool operator==(const ByteBuffer & lhs, const ByteBuffer & rhs)
    {
        if (lhs.isEmpty() && rhs.isEmpty())
        {
            return true;
        }
        else if (!lhs.isEmpty() && !rhs.isEmpty())
        {
            return 
                    lhs.getLength() == rhs.getLength()
                &&  0 == memcmp(lhs.getPtr(), rhs.getPtr(), lhs.getLength());
        }
        else
        {
            return false;
        }
    }

    char *ByteBuffer::getPtr() const
    {
        return mPv;
    }

    std::size_t ByteBuffer::getLength() const
    {
        return mPvlen;
    }

    std::size_t ByteBuffer::getLeftMargin() const
    {
        return mLeftMargin;
    }

    bool ByteBuffer::getReadOnly() const
    {
        return mReadOnly;
    }

    bool ByteBuffer::isEmpty() const
    {
        return getLength() == 0;
    }

    void ByteBuffer::setLeftMargin(std::size_t len)
    {
        RCF_ASSERT_LTEQ(len , mLeftMargin + mPvlen);

        mPv                     = mPv - mLeftMargin + len;
        mPvlen                  = mPvlen + mLeftMargin - len;
        mLeftMargin             = len;
    }

    void ByteBuffer::expandIntoLeftMargin(std::size_t len)
    {
        RCF_ASSERT_LTEQ(len , mLeftMargin);
        mPv -= len;
        mPvlen += len;
        mLeftMargin -= len;
    }

    std::string ByteBuffer::string() const
    {
        return std::string(getPtr(), getLength());
    }

    ByteBuffer ByteBuffer::release()
    {
        ByteBuffer byteBuffer(*this);
        *this = ByteBuffer();
        return byteBuffer;
    }

    void ByteBuffer::swap(ByteBuffer & rhs)
    {
        ByteBuffer temp = *this;
        *this = rhs;
        rhs = temp;
    }

    void ByteBuffer::clear()
    {
        *this = ByteBuffer();
    }

    std::size_t lengthByteBuffers(const std::vector<ByteBuffer> &byteBuffers)
    {
        std::size_t length = 0;
        for (std::size_t i=0; i<byteBuffers.size(); ++i)
        {
            length += byteBuffers[i].getLength() ;
        }
        return length;
    }

    class GetFirstByteBuffer
    {
    public:
        GetFirstByteBuffer(ByteBuffer &byteBuffer) :
            mByteBuffer(byteBuffer),
            mFirst(true)
        {}

        void operator()(const ByteBuffer &byteBuffer) const
        {
            if (mFirst)
            {
                mByteBuffer = byteBuffer;
                mFirst = false;
            }
        }

    private:
        ByteBuffer &mByteBuffer;
        mutable bool mFirst;
    };

    ByteBuffer sliceByteBuffer(
        const std::vector<ByteBuffer> &buffers,
        std::size_t offset,
        std::size_t length)
    {
        ByteBuffer buffer;

        forEachByteBuffer(
            GetFirstByteBuffer(buffer),
            buffers, 
            offset, 
            length);

        return buffer;
    }

    class PushBackByteBuffer
    {
    public:
        PushBackByteBuffer(std::vector<ByteBuffer> &byteBuffers) :
            mpByteBuffers(&byteBuffers)
        {}

        void operator()(const ByteBuffer &byteBuffer) const
        {
            mpByteBuffers->push_back(byteBuffer);
        }

    private:
        std::vector<ByteBuffer> *mpByteBuffers;
    };

    void sliceByteBuffers(
        std::vector<ByteBuffer> &slicedBuffers,
        const std::vector<ByteBuffer> &byteBuffers,
        std::size_t offset,
        std::size_t length)
    {
        slicedBuffers.resize(0);

        forEachByteBuffer(
            PushBackByteBuffer(slicedBuffers),
            byteBuffers, offset, length);
    }   

    void copyByteBuffers(
        const std::vector<ByteBuffer> &byteBuffers,
        char *pch)
    {
        for (std::size_t i=0; i<byteBuffers.size(); ++i)
        {
            memcpy(pch, byteBuffers[i].getPtr() , byteBuffers[i].getLength() );
            pch += byteBuffers[i].getLength() ;
        }
    }

    void copyByteBuffers(
        const std::vector<ByteBuffer> &byteBuffers,
        ByteBuffer &byteBuffer)
    {
        boost::shared_ptr<std::vector<char> > vecPtr(
            new std::vector<char>(lengthByteBuffers(byteBuffers)));

        copyByteBuffers(byteBuffers, &(*vecPtr)[0]);
        byteBuffer = ByteBuffer(
            &(*vecPtr)[0],
            (*vecPtr).size(),
            vecPtr);
    }

} // namespace RCF

#if RCF_FEATURE_SF==1

#include <RCF/ClientStub.hpp>
#include <RCF/CurrentSerializationProtocol.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <SF/Archive.hpp>
#include <SF/Stream.hpp>

namespace SF {

    void serialize(SF::Archive &ar, RCF::ByteBuffer &byteBuffer)
    {
        if (ar.isRead())
        {
            boost::uint32_t len = 0;
            ar & len;

            byteBuffer.clear();

            // See if we have a remote call context.
            RCF::SerializationProtocolIn *pIn = 
                ar.getIstream()->getRemoteCallContext();

            if (pIn && len)
            {
                pIn->extractSlice(byteBuffer, len);
            }
            else if (len)
            {
                if (byteBuffer.getLength() >= len)
                {
                    byteBuffer = RCF::ByteBuffer(byteBuffer, 0, len);
                }
                else
                {
                    byteBuffer = RCF::ByteBuffer(len);
                }

                SF::IStream &is = *ar.getIstream();

                boost::uint32_t bytesToRead = len;
                boost::uint32_t bytesRead = is.read( (SF::Byte8 *) byteBuffer.getPtr(), bytesToRead);

                RCF_VERIFY(
                    bytesRead == bytesToRead,
                    RCF::Exception(RCF::_SfError_ReadFailure()))
                    (bytesToRead)(bytesRead);
            }
        }
        else if (ar.isWrite())
        {
            boost::uint32_t len = static_cast<boost::uint32_t>(byteBuffer.getLength());
            ar & len;

            // See if we have a remote call context.
            RCF::SerializationProtocolOut *pOut = 
                ar.getOstream()->getRemoteCallContext();

            if (pOut && len)
            {
                pOut->insert(byteBuffer);
            }
            else if (len)
            {
                boost::uint32_t bytesToWrite = len;
                ar.getOstream()->writeRaw(
                    (SF::Byte8 *) byteBuffer.getPtr(),
                    bytesToWrite);
            }
        }
    }

} // namespace SF

#endif // RCF_FEATURE_SF==1
