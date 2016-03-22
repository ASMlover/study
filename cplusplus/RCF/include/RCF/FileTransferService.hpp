
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

#ifndef INCLUDE_RCF_FILETRANSFERSERVICE_HPP
#define INCLUDE_RCF_FILETRANSFERSERVICE_HPP

#include <fstream>
#include <map>
#include <set>

#include <RCF/FileStream.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/Service.hpp>

#include <SF/vector.hpp>
#include <SF/map.hpp>

#include <boost/filesystem/path.hpp>

#if RCF_FEATURE_SF==1

namespace SF {

    RCF_EXPORT void serialize(SF::Archive &ar, boost::filesystem::path &p);

} // namespace SF

#endif

#if RCF_FEATURE_BOOST_SERIALIZATION==1

#include <boost/config.hpp>
#include <boost/version.hpp>

#if BOOST_VERSION <= 104300
#define BOOST_FS_NAMESPACE filesystem
#elif BOOST_VERSION <= 104500
#define BOOST_FS_NAMESPACE filesystem2
#else
#define BOOST_FS_NAMESPACE filesystem3
#endif

namespace boost { namespace BOOST_FS_NAMESPACE {

    template<typename Archive>
    void serialize(Archive & ar, boost::BOOST_FS_NAMESPACE::path & p, const unsigned int)
    {
        typedef typename Archive::is_saving IsSaving;
        const bool isSaving = IsSaving::value;

        if (isSaving)
        {
            std::string s = p.string();
            ar & s;
        }
        else
        {
            std::string s;
            ar & s;
            p = path(s);
        }
    }

} }

#undef BOOST_FS_NAMESPACE

#endif

namespace RCF {

    std::string nativeString(const boost::filesystem::path & p);

    class FileTransferService;

    class FileUploadInfo;
    class FileDownloadInfo;

    class RCF_EXPORT BandwidthQuota : boost::noncopyable
    {
    public:
        BandwidthQuota();
        BandwidthQuota(boost::uint32_t quotaBps);

        void setQuota(boost::uint32_t quotaBps);
        boost::uint32_t calculateLineSpeedLimit();

    private:

        friend class FileUploadInfo;
        friend class FileDownloadInfo;

        void addUpload(FileUploadInfo * pUpload);
        void removeUpload(FileUploadInfo * pUpload);
        void addDownload(FileDownloadInfo * pDownload);
        void removeDownload(FileDownloadInfo * pDownload);       

    private:

        Mutex                           mMutex;
        boost::uint32_t                 mQuotaBps;

        std::set<FileDownloadInfo *>    mDownloadsInProgress;
        std::set<FileUploadInfo *>      mUploadsInProgress;
    };

    typedef boost::shared_ptr<BandwidthQuota> BandwidthQuotaPtr;
   

    class FileUploadInfo;
    class FileDownloadInfo;

    typedef boost::shared_ptr<FileUploadInfo>                   FileUploadInfoPtr;
    typedef boost::shared_ptr<FileDownloadInfo>                 FileDownloadInfoPtr;

    typedef boost::function1<bool, const FileUploadInfo &>      UploadAccessCallback;
    typedef boost::function1<bool, const FileDownloadInfo &>    DownloadAccessCallback;

    typedef boost::shared_ptr< std::ifstream > IfstreamPtr;
    typedef boost::shared_ptr< std::ofstream > OfstreamPtr;

    class FileIoRequest;
    typedef boost::shared_ptr<FileIoRequest> FileIoRequestPtr;
    
    class FileUploadInfo : boost::noncopyable
    {
    public:
        FileUploadInfo(BandwidthQuotaPtr quotaPtr);
        ~FileUploadInfo();

        FileManifest            mManifest;
        boost::filesystem::path mUploadPath;
        
        OfstreamPtr             mFileStream;
        boost::filesystem::path mFileStreamPath;
        FileIoRequestPtr        mWriteOp;

        bool                    mCompleted;
        bool                    mResume;
        boost::uint32_t         mTimeStampMs;
        boost::uint32_t         mCurrentFile;
        boost::uint64_t         mCurrentPos;
        boost::uint32_t         mSessionLocalId;
        std::string             mUploadId;

        BandwidthQuotaPtr       mQuotaPtr;
    };

    class FileDownloadInfo : boost::noncopyable
    {
    public:
        FileDownloadInfo(BandwidthQuotaPtr quotaPtr);
        ~FileDownloadInfo();
        
        boost::filesystem::path mDownloadPath;
        FileManifest            mManifest;

        IfstreamPtr             mFileStream;
        boost::filesystem::path mFileStreamPath;
        FileIoRequestPtr        mReadOp;
        ByteBuffer              mReadBuffer;
        ByteBuffer              mSendBuffer;
        ByteBuffer              mSendBufferRemaining;

        boost::uint32_t         mCurrentFile;
        boost::uint64_t         mCurrentPos;
        bool                    mResume;

        Timer                   mTransferWindowTimer;
        boost::uint32_t         mTransferWindowBytesSoFar;
        boost::uint32_t         mTransferWindowBytesTotal;

        bool                    mCancel;

        boost::uint32_t         mSessionLocalId;

        BandwidthQuotaPtr       mQuotaPtr;
    };

    typedef boost::shared_ptr<FileUploadInfo>   FileUploadInfoPtr;
    typedef boost::shared_ptr<FileDownloadInfo> FileDownloadInfoPtr;

    class FileChunk;
    class FileTransferRequest;

    class RCF_EXPORT FileTransferService : public I_Service
    {
    public:
        FileTransferService();

        typedef boost::function<void(RcfSession&, FileDownloadInfo &)> OnFileDownloadProgress;
        typedef boost::function<void(RcfSession&, FileUploadInfo &)> OnFileUploadProgress;

        typedef boost::function1<BandwidthQuotaPtr, RcfSession &> BandwidthQuotaCallback;
        typedef BandwidthQuotaCallback UploadQuotaCallback;
        typedef BandwidthQuotaCallback DownloadQuotaCallback;

        // For testing.
        void                setTransferWindowS(boost::uint32_t transferWindowS);
        boost::uint32_t     getTransferWindowS();

        //----------------------------------------------------------------------
        // Remotely accessible.

        void                BeginUpload(
                                const FileManifest & manifest,
                                const std::vector<FileChunk> & chunks,
                                FileChunk & startPos,
                                boost::uint32_t & maxMessageLength,
                                std::string & uploadId,
                                boost::uint32_t & bps,
                                boost::uint32_t sessionLocalId);

        void                UploadChunks(
                                const std::vector<FileChunk> & chunks,
                                boost::uint32_t & bps);

        void                BeginDownload(
                                FileManifest & manifest,
                                const FileTransferRequest & request,
                                std::vector<FileChunk> & chunks,
                                boost::uint32_t & maxMessageLength,
                                boost::uint32_t & bps,
                                boost::uint32_t sessionLocalId);

        void                TrimDownload(
                                const FileChunk & startPos);

        void                DownloadChunks(
                                const FileTransferRequest & request,
                                std::vector<FileChunk> & chunks,
                                boost::uint32_t & adviseWaitMs,
                                boost::uint32_t & bps);

        //----------------------------------------------------------------------

    private:

        std::string                 addUpload(const boost::filesystem::path & uploadPath);
        void                        removeUpload(const std::string & uploadId);
        boost::filesystem::path     findUpload(const std::string & uploadId);
        
        void                        processZeroSizeEntries(RCF::FileUploadInfo & uploadInfo);
        void                        checkForUploadCompletion(FileUploadInfoPtr uploadInfoPtr);

        typedef std::map<
            std::string, 
            boost::filesystem::path> UploadsInProgress;

        Mutex                   mUploadsInProgressMutex;
        UploadsInProgress       mUploadsInProgress; 

        void                onServerStart(RcfServer & server);
        void                onServerStop(RcfServer & server);

        boost::filesystem::path mUploadDirectory;

        OnFileDownloadProgress  mOnFileDownloadProgress;
        OnFileUploadProgress    mOnFileUploadProgress;

        boost::uint32_t         mTransferWindowS;

        BandwidthQuotaPtr       mUploadQuota;
        BandwidthQuotaPtr       mDownloadQuota;

        BandwidthQuotaCallback  mUploadQuotaCallback;
        BandwidthQuotaCallback  mDownloadQuotaCallback;
    };

    typedef boost::shared_ptr<FileTransferService> FileTransferServicePtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_FILETRANSFERSERVICE_HPP
