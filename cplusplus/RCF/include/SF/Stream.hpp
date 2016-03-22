
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

#ifndef INCLUDE_SF_STREAM_HPP
#define INCLUDE_SF_STREAM_HPP

#include <map>
#include <string>

#include <boost/noncopyable.hpp>

#include <RCF/Export.hpp>

#include <SF/DataPtr.hpp>
#include <SF/Encoding.hpp>
#include <SF/I_Stream.hpp>

#include <iosfwd>

namespace RCF {

    class SerializationProtocolIn;
    class SerializationProtocolOut;

    class MemIstream;
    class MemOstream;

}
namespace SF {

    //**************************************************
    // Encoding of object data

    template<typename E>
    class Encoding : public I_Encoding
    {
    public:
        virtual UInt32 getCount(
            DataPtr &               data, 
            const std::type_info &  type)
        {
            return countElements( (E *) 0, data, type);
        }

        virtual void toData(
            DataPtr &               data, 
            void *                  pvObject, 
            const std::type_info &  type, 
            int                     nCount)
        {
            encodeElements( (E *) 0, data, pvObject, type, nCount);
        }

        virtual void toObject(
            DataPtr &               data, 
            void *                  pvObject, 
            const std::type_info &  type, 
            int                     nCount)
        {
            decodeElements( (E *) 0, data, pvObject, type, nCount);
        }
    };

    //**************************************************
    // Context handling

    class RCF_EXPORT ContextRead
    {
    public:
        ContextRead();
        ~ContextRead();
        void add(SF::UInt32 nid, const ObjectId &id);
        void add(void *ptr, const std::type_info &objType, void *pObj);
        bool query(SF::UInt32 nid, ObjectId &id);
        bool query(void *ptr, const std::type_info &objType, void *&pObj);
        void clear();

        void setEnabled(bool enabled);
        bool getEnabled() const;

    private:
        bool mEnabled;
        std::auto_ptr<std::map<UInt32, ObjectId> >                          mNidToIdMap;
        std::auto_ptr<std::map<std::string, std::map< void *, void * > > >  mTypeToObjMap;
    };

    class RCF_EXPORT ContextWrite
    {
    public:
        ContextWrite();
        ~ContextWrite();
        void setEnabled(bool enabled);
        bool getEnabled() const;
        void add(const ObjectId &id, UInt32 &nid);
        bool query(const ObjectId &id, UInt32 &nid);
        void clear();
    private:
        bool                                            mEnabled;
        UInt32                                          mCurrentId;
        std::auto_ptr<std::map<ObjectId, UInt32> >      mIdToNidMap;
    };

    //**************************************************
    // Stream local storage

    class RCF_EXPORT LocalStorage : boost::noncopyable
    {
    public:
        LocalStorage();
        ~LocalStorage();
        void setNode(Node *);
        Node *getNode();

    private:
        Node * mpNode;
    };

    //****************************************************
    // Base stream classes

    class Node;
    class SerializerBase;

    class RCF_EXPORT IStream : boost::noncopyable
    {
    public:

        IStream();

        IStream(
            RCF::MemIstream & is,
            std::size_t     archiveSize = 0, 
            int             runtimeVersion = 0, 
            int             archiveVersion = 0);

        IStream(
            std::istream &  is,
            std::size_t     archiveSize = 0, 
            int             runtimeVersion = 0, 
            int             archiveVersion = 0);

        virtual ~IStream();

        void        setIs(
                        std::istream &  is, 
                        std::size_t     archiveSize = 0, 
                        int             runtimeVersion = 0, 
                        int             archiveVersion = 0);

        void        clearState();

        UInt32      read(Byte8 *pBytes, UInt32 nLength);

        bool        verifyAgainstArchiveSize(std::size_t bytesToRead);

        bool        begin(Node &node);
        bool        get(DataPtr &value);
        void        end();
        UInt32      read_int(UInt32 &n);
        UInt32      read_byte(Byte8 &byte);
        void        putback_byte(Byte8 byte);

        virtual I_Encoding &
                    getEncoding() = 0;

        int         getRuntimeVersion();
        int         getArchiveVersion();

        void        setArchiveVersion(int archiveVersion);
        void        setRuntimeVersion(int runtimeVersion);

        void        ignoreVersionStamp(bool ignore = true);

        void        setRemoteCallContext(
                        RCF::SerializationProtocolIn * pSerializationProtocolIn);

        RCF::SerializationProtocolIn *
                    getRemoteCallContext();

        ContextRead &           getTrackingContext();
        const ContextRead &     getTrackingContext() const;
        LocalStorage &          getLocalStorage();

        void            setEnablePointerTracking(bool enable);
        bool            getEnablePointerTracking() const;

        // Streaming operators.

        template<typename T>
        IStream & operator>>(T &t);

        template<typename T>
        IStream & operator>>(const T &t);

    private:

        ContextRead         mContextRead;
        LocalStorage        mLocalStorage;

        std::istream *      mpIs;
        std::size_t         mArchiveSize;
        int                 mRuntimeVersion;
        int                 mArchiveVersion;
        bool                mIgnoreVersionStamp;

        RCF::SerializationProtocolIn * mpSerializationProtocolIn;
    };

    class RCF_EXPORT OStream : boost::noncopyable
    {
    public:
        OStream();

        OStream(
            RCF::MemOstream &   os,
            int                 runtimeVersion = 0, 
            int                 archiveVersion = 0);

        OStream(
            std::ostream &      os,
            int                 runtimeVersion = 0, 
            int                 archiveVersion = 0);

        virtual ~OStream();

        void        setOs(
                        std::ostream &  os,
                        int             runtimeVersion = 0, 
                        int             archiveVersion = 0);

        void        clearState();

        UInt32      writeRaw(const Byte8 *pBytes, UInt32 nLength);

        void        begin(const Node &node);
        void        put(const DataPtr &value);
        void        end();
        UInt32      write_int(UInt32 n);
        UInt32      write_byte(Byte8 byte);
        UInt32      write(const Byte8 *pBytes, UInt32 nLength);

        virtual I_Encoding &
                    getEncoding() = 0;

        int         getRuntimeVersion();
        int         getArchiveVersion();

        void        setArchiveVersion(int archiveVersion);
        void        setRuntimeVersion(int runtimeVersion);

        void        suppressArchiveMetadata(bool suppress = true);

        void        setRemoteCallContext(
                        RCF::SerializationProtocolOut * pSerializationProtocolOut);

        RCF::SerializationProtocolOut *
                    getRemoteCallContext();

        ContextWrite &          getTrackingContext();
        const ContextWrite &    getTrackingContext() const;

        LocalStorage &  getLocalStorage();

        void            setEnablePointerTracking(bool enable);
        bool            getEnablePointerTracking() const;

        // Streaming operator.

        template<typename T>
        OStream & operator<<(const T &t);

    private:

        void            writeArchiveMetadata();

        ContextWrite    mContextWrite;
        LocalStorage    mLocalStorage;

        std::ostream *  mpOs;
        int             mRuntimeVersion;
        int             mArchiveVersion;
        bool            mSuppressArchiveMetadata;
        bool            mArchiveMetadataWritten;

        RCF::SerializationProtocolOut * mpSerializationProtocolOut;
    };

} // namespace SF

#include <SF/Archive.hpp>
#include <SF/Serializer.hpp>

namespace SF {

    template<typename T>
    IStream & IStream::operator>>(T &t)
    {
        Archive ar(Archive::READ, this);
        ar & t;
        return *this;
    }

    template<typename T>
    IStream & IStream::operator>>(const T &t)
    {
        Archive ar(Archive::READ, this);
        ar & t;
        return *this;
    }

    template<typename T>
    OStream & OStream::operator<<(const T &t)
    {
        Archive ar(Archive::WRITE, this);
        ar & t;
        return *this;
    }

} // namespace SF

#endif // !INCLUDE_SF_STREAM_HPP
