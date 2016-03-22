
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

#include <SF/Stream.hpp>

#include <RCF/ByteOrdering.hpp>
#include <RCF/Version.hpp>

#include <SF/DataPtr.hpp>
#include <SF/Encoding.hpp>
#include <SF/Node.hpp>
#include <SF/Tools.hpp>

#include <vector>

namespace SF {

    // ContextRead

    ContextRead::ContextRead() : mEnabled(true)
    {}

    ContextRead::~ContextRead()
    {}

    void ContextRead::setEnabled(bool enabled)
    {
        mEnabled = enabled;
    }

    bool ContextRead::getEnabled() const
    {
        return mEnabled;
    }

    void ContextRead::add(UInt32 nid, const ObjectId &id)
    {
        RCF_ASSERT(mEnabled);
        if (mNidToIdMap.get() == NULL)
        {
            mNidToIdMap.reset( new std::map<UInt32, ObjectId>());
        }
        (*mNidToIdMap)[nid] = id;
    }

    void ContextRead::add(void *ptr, const std::type_info &objType, void *pObj)
    {
        RCF_ASSERT(mEnabled);
        if (mTypeToObjMap.get() == NULL)
        {
            mTypeToObjMap.reset( new std::map<std::string, std::map< void *, void * > >() );
        }
        (*mTypeToObjMap)[ objType.name() ][ ptr ] = pObj;
    }

    bool ContextRead::query(UInt32 nid, ObjectId &id)
    {
        RCF_ASSERT(mEnabled);
        if (mNidToIdMap.get() && mNidToIdMap->find( nid ) != mNidToIdMap->end())
        {
            id = (*mNidToIdMap)[ nid ];
            return true;
        }
        else
        {
            return false;
        }
    }

    bool ContextRead::query(void *ptr, const std::type_info &objType, void *&pObj)
    {
        RCF_ASSERT(mEnabled);
        if (mTypeToObjMap.get() &&
            (*mTypeToObjMap)[ objType.name() ].find(ptr) != (*mTypeToObjMap)[ objType.name() ].end() )
        {
            pObj = (*mTypeToObjMap)[ objType.name() ][ ptr ];
            return true;
        }
        else
        {
            return false;
        }
    }

    void ContextRead::clear()
    {
        if (mNidToIdMap.get())
        {
            mNidToIdMap->clear();
        }
        if (mTypeToObjMap.get())
        {
            mTypeToObjMap->clear();
        }
    }

    // ContextWrite

    ContextWrite::ContextWrite() :
        mEnabled(false),
        mCurrentId(1)
    {}

    ContextWrite::~ContextWrite()
    {}

    void ContextWrite::setEnabled(bool enabled)
    {
        mEnabled = enabled;
        if (mEnabled && mIdToNidMap.get() == NULL)
        {
            mIdToNidMap.reset( new std::map<ObjectId, UInt32>());
        }
    }

    bool ContextWrite::getEnabled() const
    {
        return mEnabled;
    }

    void ContextWrite::add(const ObjectId &id, UInt32 &nid)
    {
        if (mEnabled)
        {
            if (mIdToNidMap->find( id ) != mIdToNidMap->end())
            {
                nid = (*mIdToNidMap)[ id ];
            }
            else
            {
                nid = (*mIdToNidMap)[ id ] = mCurrentId++;
            }
        }
    }
    bool ContextWrite::query(const ObjectId &id, UInt32 &nid)
    {
        if ( mEnabled && mIdToNidMap->find( id ) != mIdToNidMap->end() )
        {
            nid = (*mIdToNidMap)[ id ];
            return true;
        }
        else
        {
            return false;
        }
    }

    void ContextWrite::clear()
    {
        if (mIdToNidMap.get())
        {
            mIdToNidMap->clear();
        }
        mCurrentId = 1;
    }

    // LocalStorage

    LocalStorage::LocalStorage() :
        mpNode()
    {}

    LocalStorage::~LocalStorage()
    {}

    void LocalStorage::setNode(Node *pNode)
    {
        mpNode = pNode;
    }

    Node *LocalStorage::getNode()
    {
        return mpNode;
    }

    enum {
        Begin,
        Data,
        End,
        Blank,
        BeginArchiveMetadata
    };


    // IStream

    IStream::IStream() :
        mpIs(),
        mArchiveSize(0),
        mRuntimeVersion( RCF::getDefaultRuntimeVersion() ),
        mArchiveVersion( RCF::getDefaultArchiveVersion() ),
        mIgnoreVersionStamp(false),
        mpSerializationProtocolIn(NULL)
    {
    }

    IStream::IStream(
        RCF::MemIstream &  is, 
        std::size_t     archiveSize, 
        int             runtimeVersion, 
        int             archiveVersion) :

            mpIs(),
            mArchiveSize(0),
            mRuntimeVersion( runtimeVersion ),
            mArchiveVersion( archiveVersion ),
            mIgnoreVersionStamp(false),
            mpSerializationProtocolIn(NULL)
    {
        setIs(is, archiveSize, runtimeVersion, archiveVersion);
    }

    IStream::IStream(
        std::istream &  is, 
        std::size_t     archiveSize, 
        int             runtimeVersion, 
        int             archiveVersion) :

            mpIs(),
            mArchiveSize(),
            mRuntimeVersion( runtimeVersion ),
            mArchiveVersion( archiveVersion ),
            mIgnoreVersionStamp(false),
            mpSerializationProtocolIn(NULL)
    {
        setIs(is, archiveSize, runtimeVersion, archiveVersion);
    }


    IStream::~IStream()
    {
    }

    void IStream::setIs(
        std::istream &  is, 
        std::size_t     archiveSize, 
        int             runtimeVersion, 
        int             archiveVersion) 
    { 
        mpIs = &is; 
        mArchiveSize = archiveSize;

        mRuntimeVersion = runtimeVersion;
        if (mRuntimeVersion == 0)
        {
            mRuntimeVersion = RCF::getDefaultRuntimeVersion();
        }

        mArchiveVersion = archiveVersion;
    }

    bool IStream::verifyAgainstArchiveSize(std::size_t bytesToRead)
    {
        if (mArchiveSize)
        {
            std::size_t bytesRead = static_cast<std::size_t>(mpIs->tellg());
            std::size_t bytesRemaining = mArchiveSize - bytesRead;
            return bytesToRead <= bytesRemaining;
        }
        else
        {
            return false;
        }
    }

    void IStream::clearState() 
    { 
        getTrackingContext().clear();
    }

    bool IStream::begin(Node &node)
    {
        while (true)
        {
            Byte8 byte = 0;
            read_byte(byte);

            switch (byte)
            {
            case Blank: 
                {
                    Byte8 count = 0;
                    read_byte(count);
                    std::vector<Byte8> buffer(count);
                    UInt32 bytesRead = read( &(buffer[0]), count);
                    if (bytesRead != static_cast<UInt32>(count))
                    {
                        RCF::Exception e(RCF::_SfError_DataFormat());
                        RCF_THROW(e)(bytesRead)(count);
                    }
                    continue;
                }

            case BeginArchiveMetadata:
                {
                    int runtimeVersion = 0;
                    int archiveVersion = 0;
                    bool pointerTrackingEnabled = false;
                    bool * pPointerTrackingEnabled = NULL;

                    const size_t BufferLen = 11;
                    char buffer[BufferLen] = {0};
                    RCF::ByteBuffer byteBuffer( &buffer[0], BufferLen);
                    std::size_t pos0 = static_cast<std::size_t>(mpIs->tellg());

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 )  // warning C4996: 'std::basic_istream<_Elem,_Traits>::readsome': Function call with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct. To disable this warning, use -D_SCL_SECURE_NO_WARNINGS. See documentation on how to use Visual C++ 'Checked Iterators'
#endif
                    std::size_t bytesRead = static_cast<std::size_t>(mpIs->readsome(buffer, BufferLen));

#ifdef _MSC_VER
#pragma warning( pop )
#endif
                    byteBuffer = RCF::ByteBuffer(byteBuffer, 0, bytesRead);
                    std::size_t pos1 = 0;
                    decodeInt(runtimeVersion, byteBuffer, pos1);
                    decodeInt(archiveVersion, byteBuffer, pos1);

                    if (runtimeVersion >= 10)
                    {
                        decodeBool(pointerTrackingEnabled, byteBuffer, pos1);
                        pPointerTrackingEnabled = &pointerTrackingEnabled;
                    }

                    mpIs->seekg(
                        static_cast<std::istream::off_type>(pos0 + pos1), 
                        std::ios_base::beg);

                    if (!mIgnoreVersionStamp)
                    {
                        if (runtimeVersion)
                        {
                            mRuntimeVersion = runtimeVersion;
                        }
                        if (archiveVersion)
                        {
                            mArchiveVersion = archiveVersion;
                        }
                    }

                    if (pPointerTrackingEnabled && !*pPointerTrackingEnabled)
                    {
                        getTrackingContext().setEnabled(false);
                    }

                    continue;
                }

            case Begin:
                {
                    read_byte( byte );
                    Byte8 attrSpec = byte;

                    // id
                    if (attrSpec & 1)
                    {
                        read_int(node.id);
                    }

                    // ref
                    attrSpec = attrSpec >> 1;
                    if (attrSpec & 1)
                    {
                        node.ref = 1;
                    }

                    // type
                    attrSpec = attrSpec >> 1;
                    if (attrSpec & 1)
                    {
                        UInt32 length = 0;
                        read_int(length);
                        node.type.allocate(length);
                        read(node.type.get(), length );
                    }

                    // label
                    attrSpec = attrSpec >> 1;
                    if (attrSpec & 1)
                    {
                        UInt32 length = 0;
                        read_int(length);
                        node.label.allocate(length);
                        read(node.label.get(), length);
                    }

                    return true;
                }

            default:
                {
                    RCF::Exception e(RCF::_SfError_DataFormat());
                    RCF_THROW(e)(byte);
                }
            }
        }

    }

    bool IStream::get(DataPtr &value)
    {
        Byte8 byte;
        read_byte( byte );
        if (byte == Data)
        {
            UInt32 length = 0;
            read_int(length);
            value.allocate(length);
            read(value.get(), length);
            return true;
        }
        else
        {
            putback_byte(byte);
            return false;
        }
    }

    void IStream::end()
    {
        Byte8 byte;
        read_byte(byte);
        if (byte != End)
        {
            RCF::Exception e(RCF::_SfError_DataFormat(), "no end symbol");
            RCF_THROW(e)(byte);
        }
    }

    UInt32 IStream::read_int(UInt32 &n)
    {
        if (mRuntimeVersion < 9)
        {
            UInt32 bytesRead = read( reinterpret_cast<SF::Byte8 *>(&n), 4);
            RCF::networkToMachineOrder( &n, 4, 1);
            return bytesRead;
        }
        else
        {
            // Integers less than 128 are stored as a single byte.
            Byte8 byte = 0;
            boost::uint8_t ubyte = 0;
            UInt32 bytesRead = read_byte(byte);
            ubyte = byte;
            if (ubyte < 128)
            {
                n = ubyte;
            }
            else
            {
                RCF_ASSERT(ubyte == 128);
                bytesRead += read( reinterpret_cast<SF::Byte8 *>(&n), 4);
                RCF::networkToMachineOrder( &n, 4, 1);
            }
            return bytesRead;
        }
    }

    UInt32 IStream::read_byte(Byte8 &byte)
    {
        UInt32 bytesRead = read(&byte, 1);
        return bytesRead;
    }

    UInt32 IStream::read(Byte8 *pBytes, UInt32 nLength)
    {
        mpIs->read(pBytes, nLength);
        if (mpIs->fail())
        {
            RCF::Exception e(RCF::_SfError_ReadFailure());
            RCF_THROW(e)(nLength)(mpIs->gcount());
        }
        return static_cast<UInt32>(mpIs->gcount());
    }

    void IStream::putback_byte( Byte8 byte )
    {
        mpIs->putback(byte);
    }

    int IStream::getRuntimeVersion()
    {
        return mRuntimeVersion;
    }

    int IStream::getArchiveVersion()
    {
        return mArchiveVersion;
    }

    void IStream::setArchiveVersion(int archiveVersion)
    {
        mArchiveVersion = archiveVersion;
    }

    void IStream::setRuntimeVersion(int runtimeVersion)
    {
        mRuntimeVersion = runtimeVersion;
    }

    void IStream::ignoreVersionStamp(bool ignore)
    {
        mIgnoreVersionStamp = ignore;
    }

    void IStream::setRemoteCallContext(
        RCF::SerializationProtocolIn * pSerializationProtocolIn)
    {
        mpSerializationProtocolIn = pSerializationProtocolIn;
    }

    RCF::SerializationProtocolIn *
        IStream::getRemoteCallContext()
    {
        return mpSerializationProtocolIn;
    }

    ContextRead & IStream::getTrackingContext() 
    { 
        return mContextRead; 
    }

    const ContextRead & IStream::getTrackingContext() const
    {
        return mContextRead;
    }
    
    LocalStorage & IStream::getLocalStorage() 
    { 
        return mLocalStorage; 
    }

    void IStream::setEnablePointerTracking(bool enable)
    {
        getTrackingContext().setEnabled(enable);
    }

    bool IStream::getEnablePointerTracking() const
    {
        return getTrackingContext().getEnabled();
    }

    // OStream

    OStream::OStream() : 
        mpOs(), 
        mRuntimeVersion( RCF::getDefaultRuntimeVersion() ), 
        mArchiveVersion( RCF::getDefaultArchiveVersion() ),
        mSuppressArchiveMetadata(false),
        mArchiveMetadataWritten(false),
        mpSerializationProtocolOut(NULL)
    {
    }

    OStream::OStream(
        RCF::MemOstream &  os, 
        int             runtimeVersion, 
        int             archiveVersion) : 

            mpOs(), 
            mRuntimeVersion(runtimeVersion),
            mSuppressArchiveMetadata(false),
            mArchiveMetadataWritten(false),
            mpSerializationProtocolOut(NULL)
    {
        setOs(os, runtimeVersion, archiveVersion);
    }

    OStream::OStream(
        std::ostream &  os, 
        int             runtimeVersion, 
        int             archiveVersion) : 

            mpOs(), 
            mRuntimeVersion(runtimeVersion),
            mSuppressArchiveMetadata(false),
            mArchiveMetadataWritten(false),
            mpSerializationProtocolOut(NULL)
    {
        setOs(os, runtimeVersion, archiveVersion);
    }

    OStream::~OStream()
    {
    }

    void OStream::setOs(
        std::ostream &  os, 
        int             runtimeVersion, 
        int             archiveVersion) 
    { 
        mpOs = &os; 

        mRuntimeVersion = runtimeVersion;
        if (mRuntimeVersion == 0)
        {
            mRuntimeVersion = RCF::getDefaultRuntimeVersion();
        }

        mArchiveVersion = archiveVersion;
    }

    void OStream::clearState() 
    { 
        getTrackingContext().clear(); 

        mArchiveMetadataWritten = false;
    }

    void OStream::writeArchiveMetadata()
    {
        if (mRuntimeVersion >= 6)
        {
            // Max possible size.
            const std::size_t BufferLen = 1+5+5+1;

            char buffer[BufferLen] = {0};
            buffer[0] = BeginArchiveMetadata;

            RCF::ByteBuffer byteBuffer(&buffer[0], BufferLen);
            std::size_t pos = 1;
            encodeInt(mRuntimeVersion, byteBuffer, pos);
            encodeInt(mArchiveVersion, byteBuffer, pos);

            if (mRuntimeVersion >= 10)
            {
                encodeBool(getTrackingContext().getEnabled(), byteBuffer, pos);
            }

            writeRaw(&buffer[0], static_cast<UInt32>(pos));
        }
    }

    void OStream::begin( const Node &node )
    {
        if (!mSuppressArchiveMetadata && !mArchiveMetadataWritten)
        {
            writeArchiveMetadata();
            mArchiveMetadataWritten = true;
        }

        write_byte( (Byte8) Begin );

        Byte8 attrSpec = 0;
        if (node.id != 0)
        {
            attrSpec |= 1<<0;
        }
        if (node.ref != 0)
        {
            attrSpec |= 1<<1;
        }
        if (!node.type.empty())
        {
            attrSpec |= 1<<2;
        }
        if (!node.label.empty())
        {
            attrSpec |= 1<<3;
        }

        write_byte( attrSpec );

        if (node.id != 0)
        {
            write_int(node.id);
        }
        if (!node.type.empty())
        {
            write(node.type.get(), node.type.length());
        }
        if (!node.label.empty())
        {
            write(node.label.get(), node.label.length() );
        }
    }

    void OStream::put(const DataPtr &value)
    {
        write_byte( (Byte8) Data );
        write(value.get(), value.length());
    }

    void OStream::end()
    {
        write_byte( (Byte8) End );
    }

    UInt32 OStream::write_int(UInt32 n)
    {
        BOOST_STATIC_ASSERT( sizeof(n) == 4 );

        if (mRuntimeVersion < 9)
        {
            RCF::machineToNetworkOrder(&n, 4, 1);
            mpOs->write( reinterpret_cast<char*>(&n), 4);
            if (mpOs->fail())
            {
                RCF::Exception e(RCF::_SfError_WriteFailure());
                RCF_THROW(e)(n);
            }
            return 4;
        }
        else
        {
            // Integers less than 128 are stored as a single byte.
            if (0 <= n && n <= 127)
            {
                boost::uint8_t byte = static_cast<boost::uint8_t>(n);
                write_byte(byte);
                return 1;
            }
            else
            {
                boost::uint8_t byte = 128;
                write_byte(byte);
            
                RCF::machineToNetworkOrder(&n, 4, 1);
                mpOs->write( reinterpret_cast<char*>(&n), 4);
                if (mpOs->fail())
                {
                    RCF::Exception e(RCF::_SfError_WriteFailure());
                    RCF_THROW(e)(n);
                }
                return 5;
            }
        }
    }

    UInt32 OStream::write_byte(Byte8 byte)
    {
        mpOs->write(&byte, 1);
        if (mpOs->fail())
        {
            RCF::Exception e(RCF::_SfError_WriteFailure());
            RCF_THROW(e);
        }
        return 1;
    }

    UInt32 OStream::write(const Byte8 *pBytes, UInt32 nLength)
    {
        UInt32 bytesWritten = 0;
        bytesWritten += write_int(nLength);
        mpOs->write(pBytes, nLength);
        if (mpOs->fail())
        {
            RCF::Exception e(RCF::_SfError_WriteFailure());
            RCF_THROW(e)(nLength);
        }
        bytesWritten += nLength;
        return bytesWritten;
    }

    UInt32 OStream::writeRaw(const Byte8 *pBytes, UInt32 nLength)
    {
        mpOs->write(pBytes, nLength);
        if (mpOs->fail())
        {
            RCF::Exception e(RCF::_SfError_WriteFailure());
            RCF_THROW(e)(nLength);
        }
        return nLength;
    }

    int OStream::getRuntimeVersion()
    {
        return mRuntimeVersion;
    }

    int OStream::getArchiveVersion()
    {
        return mArchiveVersion;
    }

    void OStream::setArchiveVersion(int archiveVersion)
    {
        mArchiveVersion = archiveVersion;
    }

    void OStream::setRuntimeVersion(int runtimeVersion)
    {
        mRuntimeVersion = runtimeVersion;
    }

    void OStream::suppressArchiveMetadata(bool suppress)
    {
        mSuppressArchiveMetadata = suppress;
    }

    void OStream::setRemoteCallContext(
        RCF::SerializationProtocolOut * pSerializationProtocolOut)
    {
        mpSerializationProtocolOut = pSerializationProtocolOut;
    }

    RCF::SerializationProtocolOut *
        OStream::getRemoteCallContext()
    {
        return mpSerializationProtocolOut;
    }

    ContextWrite & OStream::getTrackingContext() 
    { 
        return mContextWrite; 
    }

    const ContextWrite & OStream::getTrackingContext() const
    { 
        return mContextWrite; 
    }
    
    LocalStorage & OStream::getLocalStorage() 
    { 
        return mLocalStorage; 
    }

    void OStream::setEnablePointerTracking(bool enable)
    {
        getTrackingContext().setEnabled(enable); 
    }

    bool OStream::getEnablePointerTracking() const
    {
        return getTrackingContext().getEnabled();
    }

} // namespace SF
