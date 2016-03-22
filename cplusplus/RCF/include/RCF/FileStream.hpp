
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

#ifndef INCLUDE_RCF_FILESTREAM_HPP
#define INCLUDE_RCF_FILESTREAM_HPP

#include <RCF/Config.hpp>

#if RCF_FEATURE_FILETRANSFER==0
#error RCF_FEATURE_FILETRANSFER=1 must be defined
#endif

#include <RCF/ByteBuffer.hpp>
#include <RCF/Export.hpp>

#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/function.hpp>

#if RCF_FEATURE_BOOST_SERIALIZATION==1
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#endif

namespace SF {

    class Archive;

} // namespace SF

namespace RCF {

    class FileUploadInfo;
    typedef boost::shared_ptr<FileUploadInfo> FileUploadInfoPtr;

    class FileInfo;
    class FileManifest;
    class FileStreamImpl;

    typedef boost::shared_ptr<FileStreamImpl> FileStreamImplPtr;

    class ClientStub;

    class RCF_EXPORT FileInfo
    {
    public:
        FileInfo() : 
            mIsDirectory(false), 
            mFileStartPos(0), 
            mFileSize(0), 
            mFileCrc(0),
            mLastWriteTime(0)
        {}

        bool                        mIsDirectory;
        boost::filesystem::path     mFilePath;
        boost::uint64_t             mFileStartPos;
        boost::uint64_t             mFileSize;
        boost::uint32_t             mFileCrc;
        std::string                 mRenameFile;
        boost::uint64_t             mLastWriteTime;

#if RCF_FEATURE_SF==1
        void serialize(SF::Archive & ar);
#endif

#if RCF_FEATURE_BOOST_SERIALIZATION==1
        template<typename Archive>
        void serialize(Archive & ar, const unsigned int)
        {
            ar 
                & mIsDirectory 
                & mFilePath 
                & mFileStartPos 
                & mFileSize 
                & mFileCrc 
                & mRenameFile;

            int runtimeVersion = getRuntimeVersionOfThisRemoteCall();
            if (runtimeVersion >= 11)
            {
                ar & mLastWriteTime;
            }
        }
#endif

    };

    class RCF_EXPORT FileManifest
    {
    public:
        typedef std::vector< FileInfo > Files;
        Files mFiles;

        boost::filesystem::path mManifestBase;

        FileManifest() {}

        FileManifest(boost::filesystem::path pathToFiles);

        boost::uint64_t getTotalByteSize() const;

#if RCF_FEATURE_SF==1
        void serialize(SF::Archive & ar);
#endif

#if RCF_FEATURE_BOOST_SERIALIZATION==1
        template<typename Archive>
        void serialize(Archive & ar, const unsigned int)
        {
            ar & mFiles;
        }
#endif

    };

    class RCF_EXPORT FileStreamImpl : public boost::enable_shared_from_this<FileStreamImpl>
    {
    public:
        
        enum Direction
        {
            Unspecified,
            Upload,
            Download
        };


        FileStreamImpl();
        FileStreamImpl(const std::string & filePath);
        FileStreamImpl(const FileManifest & manifest);

        ~FileStreamImpl();

        void serializeGeneric(
            bool isWriting,
            boost::function2<void, boost::uint32_t &, Direction &> serializeImpl);

#if RCF_FEATURE_SF==1
        void serializeImplSf(SF::Archive & ar, boost::uint32_t & transferId, Direction & dir);
        void serialize(SF::Archive & ar);
#endif

#if RCF_FEATURE_BOOST_SERIALIZATION==1

        template<typename Archive>
        void serializeImplBser(Archive & ar, boost::uint32_t & transferId, Direction & dir)
        {
            ar & transferId & dir;
        }

        template<typename Archive>
        void serialize(Archive & ar, const unsigned int)
        {
            typedef typename Archive::is_saving IsSaving;
            const bool isSaving = IsSaving::value;

            serializeGeneric( 
                isSaving,
                boost::bind( 
                    &FileStreamImpl::serializeImplBser<Archive>, 
                    this, 
                    boost::ref(ar),
                    _1, 
                    _2) );
        }

#endif

        std::string                 mUploadId;
        boost::filesystem::path     mDownloadToPath;
        FileManifest                mManifest;
        boost::uint32_t             mTransferRateBps;
        boost::uint32_t             mSessionLocalId;

        Direction                   mDirection;
    };

    class RCF_EXPORT FileStream
    {
    protected:

        FileStream();
        FileStream(FileStreamImplPtr implPtr);
        FileStream(const std::string & filePath);
        FileStream(const FileManifest & manifest);
  
    public:

        // Made this inline as it was not being linked in, in DLL builds.
        FileStream & operator=(const FileStream & rhs)
        {
            *mImplPtr = *rhs.mImplPtr;
            return *this;
        }

        // FileStream recipient calls these.
        std::string getLocalPath() const;
        FileManifest & getManifest() const;

        // Client calls these.
        void setDownloadToPath(const std::string & downloadToPath);
        std::string getDownloadToPath() const;

        void setTransferRateBps(boost::uint32_t transferRateBps);
        boost::uint32_t getTransferRateBps();
        
        void upload(RCF::ClientStub & clientStub);
        void download(RCF::ClientStub & clientStub);

        FileStreamImplPtr mImplPtr;

#if RCF_FEATURE_SF==1
        void serialize(SF::Archive & ar);
#endif

#if RCF_FEATURE_BOOST_SERIALIZATION==1
        template<typename Archive>
        void serialize(Archive & ar, const unsigned int)
        {
            ar & *mImplPtr;
        }
#endif

    };

    class FileTransferProgress
    {
    public:

        FileTransferProgress() : 
            mBytesTotalToTransfer(0),
            mBytesTransferredSoFar(0),
            mServerLimitBps(0)
        {

        }

        boost::uint64_t mBytesTotalToTransfer;
        boost::uint64_t mBytesTransferredSoFar;
        boost::uint32_t mServerLimitBps;
    };

    class FileChunk
    {
    public:

        FileChunk() : mFileIndex(0), mOffset(0)
        {}

        boost::uint32_t mFileIndex;
        boost::uint64_t mOffset;
        ByteBuffer mData;

#if RCF_FEATURE_SF==1
        void serialize(SF::Archive & ar);
#endif

#if RCF_FEATURE_BOOST_SERIALIZATION==1
        template<typename Archive>
        void serialize(Archive & ar, const unsigned int)
        {
            ar & mFileIndex & mOffset & mData;
        }
#endif

    };

    class FileTransferRequest
    {
    public:
        FileTransferRequest() : mFile(0), mPos(0), mChunkSize(0)
        {}

        boost::uint32_t mFile;
        boost::uint64_t mPos;
        boost::uint32_t mChunkSize;

#if RCF_FEATURE_SF==1
        void serialize(SF::Archive & ar);
#endif

#if RCF_FEATURE_BOOST_SERIALIZATION==1
        template<typename Archive>
        void serialize(Archive & ar, const unsigned int)
        {
            ar & mFile & mPos & mChunkSize;
        }
#endif

    };

} // namespace RCF

#endif // ! INCLUDE_RCF_FILESTREAM_HPP
