
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

#include <RCF/SerializationProtocol.hpp>

#include <RCF/Config.hpp>
#include <RCF/ObjectPool.hpp>
#include <RCF/Version.hpp>

namespace RCF {

#if RCF_FEATURE_SF==1
    const SerializationProtocol DefaultSerializationProtocol = Sp_SfBinary;
#else
    const SerializationProtocol DefaultSerializationProtocol = Sp_BsBinary;
#endif

    const MarshalingProtocol DefaultMarshalingProtocol = Mp_Rcf;

    bool isSerializationProtocolSupported(int protocol)
    {
        switch (protocol)
        {
        case 1: return Protocol< boost::mpl::int_<1> >::getName() != "";
        case 2: return Protocol< boost::mpl::int_<2> >::getName() != "";
        case 3: return Protocol< boost::mpl::int_<3> >::getName() != "";
        case 4: return Protocol< boost::mpl::int_<4> >::getName() != "";
        case 5: return Protocol< boost::mpl::int_<5> >::getName() != "";
        case 6: return Protocol< boost::mpl::int_<6> >::getName() != "";
        case 7: return Protocol< boost::mpl::int_<7> >::getName() != "";
        case 8: return Protocol< boost::mpl::int_<8> >::getName() != "";
        case 9: return Protocol< boost::mpl::int_<9> >::getName() != "";
        case 10: return Protocol< boost::mpl::int_<10> >::getName() != "";
        default: return false;
        }
    }

    std::string getSerializationProtocolName(int protocol)
    {
        switch (protocol)
        {
        case 1: return Protocol< boost::mpl::int_<1> >::getName();
        case 2: return Protocol< boost::mpl::int_<2> >::getName();
        case 3: return Protocol< boost::mpl::int_<3> >::getName();
        case 4: return Protocol< boost::mpl::int_<4> >::getName();
        case 5: return Protocol< boost::mpl::int_<5> >::getName();
        case 6: return Protocol< boost::mpl::int_<6> >::getName();
        case 7: return Protocol< boost::mpl::int_<7> >::getName();
        case 8: return Protocol< boost::mpl::int_<8> >::getName();
        case 9: return Protocol< boost::mpl::int_<9> >::getName();
        case 10: return Protocol< boost::mpl::int_<10> >::getName();
        default: return "";
        }
    }

    SerializationProtocolIn::SerializationProtocolIn() :
        mProtocol(DefaultSerializationProtocol),
        mRuntimeVersion( RCF::getDefaultRuntimeVersion() ),
        mArchiveVersion( RCF::getDefaultArchiveVersion() )
    {
    }

    SerializationProtocolIn::~SerializationProtocolIn()
    {
        RCF_DTOR_BEGIN

            // Can't delete the stream if an archive is still bound to it.
            unbindProtocol();

        RCF_DTOR_END
    }

    void SerializationProtocolIn::setSerializationProtocol(int protocol)
    {
        mProtocol = protocol;
    }

    int SerializationProtocolIn::getSerializationProtocol() const
    {
        return mProtocol;
    }

    static const char chZero = 0;

    void SerializationProtocolIn::reset(const ByteBuffer &data, int protocol, int runtimeVersion, int archiveVersion, bool enableSfPointerTracking)
    {
        mRuntimeVersion = runtimeVersion;
        mArchiveVersion = archiveVersion;

        unbindProtocol();

        mByteBuffer = data;

        if (mByteBuffer)
        {
            mIs.reset(
                mByteBuffer.getPtr(),
                mByteBuffer.getLength());
        }
        else
        {
            mIs.reset(NULL, 0);
        }

        setSerializationProtocol(protocol);
        bindProtocol();

#if RCF_FEATURE_SF==1
        if (protocol == Sp_SfBinary)
        {
            mInProtocol1.getIStream().setEnablePointerTracking(enableSfPointerTracking);
        }
#endif

    }

    void SerializationProtocolIn::bindProtocol()
    {
        std::size_t archiveLength = mByteBuffer.getLength();
        switch (mProtocol)
        {
        case 1: mInProtocol1.bind(mIs, archiveLength, mRuntimeVersion, mArchiveVersion, *this); break;
        case 2: mInProtocol2.bind(mIs, archiveLength, mRuntimeVersion, mArchiveVersion, *this); break;
        case 3: mInProtocol3.bind(mIs, archiveLength, mRuntimeVersion, mArchiveVersion, *this); break;
        case 4: mInProtocol4.bind(mIs, archiveLength, mRuntimeVersion, mArchiveVersion, *this); break;
        case 5: mInProtocol5.bind(mIs, archiveLength, mRuntimeVersion, mArchiveVersion, *this); break;
        default: RCF_ASSERT(0)(mProtocol);
        }
    }

    void SerializationProtocolIn::unbindProtocol()
    {
        switch (mProtocol)
        {
        case 1: mInProtocol1.unbind(); break;
        case 2: mInProtocol2.unbind(); break;
        case 3: mInProtocol3.unbind(); break;
        case 4: mInProtocol4.unbind(); break;
        case 5: mInProtocol5.unbind(); break;
        default: RCF_ASSERT(0)(mProtocol);
        }
    }

    void SerializationProtocolIn::extractSlice(
        ByteBuffer &byteBuffer,
        std::size_t len)
    {
        if (len == 0)
        {
            byteBuffer.clear();
        }
        else
        {
            std::size_t pos = static_cast<std::size_t>(mIs.getReadPos());
            mIs.moveReadPos(pos+len);
            byteBuffer = ByteBuffer(mByteBuffer, pos, len);
        }
    }

    void SerializationProtocolIn::clearByteBuffer()
    {
        mByteBuffer = ByteBuffer();
    }

    std::size_t SerializationProtocolIn::getArchiveLength()
    {
        return mByteBuffer.getLength();
    }

    std::size_t SerializationProtocolIn::getRemainingArchiveLength()
    {
        std::size_t pos = static_cast<std::size_t>(mIs.getReadPos());
        std::size_t len = mByteBuffer.getLength();
        RCF_ASSERT_LTEQ(pos , len);
        return len - pos;
    }

    void SerializationProtocolIn::clear()
    {
        unbindProtocol();
    }

    int SerializationProtocolIn::getRuntimeVersion()
    {
        return mRuntimeVersion;
    }

    SerializationProtocolOut::SerializationProtocolOut() :
        mProtocol(DefaultSerializationProtocol),
        mMargin(),
        mRuntimeVersion( RCF::getDefaultRuntimeVersion() ),
        mArchiveVersion( RCF::getDefaultArchiveVersion() )
    {}

    void SerializationProtocolOut::setSerializationProtocol(int protocol)
    {
        mProtocol = protocol;
    }

    int SerializationProtocolOut::getSerializationProtocol() const
    {
        return mProtocol;
    }

    void SerializationProtocolOut::reset(
        int protocol,
        std::size_t margin,
        ByteBuffer byteBuffer,
        int runtimeVersion,
        int archiveVersion,
        bool enableSfPointerTracking)
    {

        mRuntimeVersion = runtimeVersion;
        mArchiveVersion = archiveVersion;

        unbindProtocol();
        if (!mOsPtr)
        {
            mOsPtr = getObjectPool().getMemOstreamPtr();
        }
        else
        {
            mOsPtr->clear(); // freezing may have set error state
            mOsPtr->rewind();
        }

        // set up margin
        mMargin = margin;
        for (std::size_t i=0; i<mMargin; ++i)
        {
            mOsPtr->write("%", 1);
        }

        // copy byte buffer contents
        if (!byteBuffer.isEmpty())
        {
            mOsPtr->write(
                byteBuffer.getPtr(), 
                static_cast<std::streamsize>(byteBuffer.getLength()));
        }

        if (protocol != 0)
        {
            setSerializationProtocol(protocol);
        }
        bindProtocol();

#if RCF_FEATURE_SF==1
        if (protocol == Sp_SfBinary)
        {
            mOutProtocol1.getOStream().setEnablePointerTracking(enableSfPointerTracking);
        }
#endif

    }

    void SerializationProtocolOut::bindProtocol()
    {
        switch (mProtocol)
        {
        case 1: mOutProtocol1.bind(*mOsPtr, mRuntimeVersion, mArchiveVersion, *this); break;
        case 2: mOutProtocol2.bind(*mOsPtr, mRuntimeVersion, mArchiveVersion, *this); break;
        case 3: mOutProtocol3.bind(*mOsPtr, mRuntimeVersion, mArchiveVersion, *this); break;
        case 4: mOutProtocol4.bind(*mOsPtr, mRuntimeVersion, mArchiveVersion, *this); break;
        case 5: mOutProtocol5.bind(*mOsPtr, mRuntimeVersion, mArchiveVersion, *this); break;
        default: RCF_ASSERT(0)(mProtocol);
        }
    }

    void SerializationProtocolOut::unbindProtocol()
    {
        switch (mProtocol)
        {
        case 1: mOutProtocol1.unbind(); break;
        case 2: mOutProtocol2.unbind(); break;
        case 3: mOutProtocol3.unbind(); break;
        case 4: mOutProtocol4.unbind(); break;
        case 5: mOutProtocol5.unbind(); break;
        default: RCF_ASSERT(0)(mProtocol);
        }
    }

    void SerializationProtocolOut::insert(const ByteBuffer &byteBuffer)
    {
        std::size_t streamPos = static_cast<std::size_t>(mOsPtr->tellp());
        mByteBuffers.push_back( std::make_pair(streamPos, byteBuffer));
    }

    void SerializationProtocolOut::extractByteBuffers()
    {
        mByteBuffers.resize(0);
    }

    void SerializationProtocolOut::extractByteBuffers(
        std::vector<ByteBuffer> &byteBuffers)
    {
        byteBuffers.resize(0);
        char *pch = mOsPtr->str();
        std::size_t offset = 0;
        std::size_t offsetPrev = 0;
        std::size_t len = static_cast<std::size_t>(mOsPtr->tellp());
        for (std::size_t i=0; i<mByteBuffers.size(); ++i)
        {
            offset = mByteBuffers[i].first;

            RCF_ASSERT(
                offsetPrev <= offset && offset <= len)
                (offsetPrev)(offset)(len);

            if (offset-offsetPrev > 0)
            {
                if (offsetPrev == 0)
                {
                    byteBuffers.push_back(
                        ByteBuffer(
                        pch+offsetPrev+mMargin,
                        offset-offsetPrev-mMargin,
                        mMargin,
                        mOsPtr));
                }
                else
                {
                    byteBuffers.push_back(
                        ByteBuffer(
                        pch+offsetPrev,
                        offset-offsetPrev,
                        mOsPtr));
                }

            }
            byteBuffers.push_back(mByteBuffers[i].second);
            offsetPrev = offset;
        }
        RCF_ASSERT(
            offsetPrev <= offset && offset <= len)
            (offsetPrev)(offset)(len);

        if (len-offset > 0)
        {
            if (offset == 0)
            {
                byteBuffers.push_back(
                    ByteBuffer(
                    pch+offset+mMargin,
                    len-offset-mMargin,
                    mMargin,
                    mOsPtr));
            }
            else
            {
                byteBuffers.push_back(
                    ByteBuffer(
                    pch+offset,
                    len-offset,
                    mOsPtr));
            }

        }
        mByteBuffers.resize(0);
    }

    void SerializationProtocolOut::clear()
    {
        unbindProtocol();
        mOsPtr.reset();
    }

    int SerializationProtocolOut::getRuntimeVersion()
    {
        return mRuntimeVersion;
    }

} // namespace RCF
