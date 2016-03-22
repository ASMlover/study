
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

#include <RCF/FileTransferService.hpp>

#include <boost/function.hpp>

#include <cstdio>
#include <iomanip>
#include <boost/limits.hpp>

#include <sys/stat.h>

#include <RCF/Exception.hpp>
#include <RCF/FileIoThreadPool.hpp>
#include <RCF/ObjectFactoryService.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/ServerInterfaces.hpp>
#include <RCF/ThreadLocalData.hpp>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>

#include <boost/config.hpp>
#include <boost/version.hpp>

namespace SF {

#if RCF_FEATURE_SF==1

    void serialize(SF::Archive &ar, boost::filesystem::path &p)
    {
        if (ar.isWrite())
        {
            std::string s = p.string();
            ar & s;
        }
        else
        {
            std::string s;
            ar & s;
            p = boost::filesystem::path(s);
        }
    }

#endif

}

namespace RCF {

    namespace fs = boost::filesystem;

#if BOOST_VERSION <= 104500

    std::string nativeString(const fs::path & p)
    {
        return p.file_string();
    }

#else

    // path::native() on Windows does not appear to convert all forward slashes 
    // to backward slashes, as file_string did in previous Boost.Filesystem 
    // versions. So we roll our own.
    
#ifdef BOOST_WINDOWS

    std::string nativeString(const fs::path & p)
    {
        std::string strPath = p.string();
        std::size_t pos = std::string::npos;
        while ((pos = strPath.find('/')) != std::string::npos)
        {
            strPath.replace(pos, 1, "\\");
        }
        return strPath;
    }

#else

    std::string nativeString(const fs::path & p)
    {
        return p.native();
    }

#endif

#endif

    FileTransferService::FileTransferService() :
            mUploadDirectory(),
            mTransferWindowS(5)            
    {
    }

    namespace fs = boost::filesystem;

    fs::path makeTempDir(const fs::path & basePath, const std::string & prefix)
    {
        std::size_t tries = 0;
        while (tries++ < 500)
        {
            std::ostringstream os;
            os 
                << prefix
                << std::setw(10)
                << std::setfill('0')
                << rand();

            fs::path tempFolder = basePath / os.str();

            if (!fs::exists(tempFolder))
            {
                bool ok = fs::create_directories(tempFolder);
                if (ok)
                {
                    return tempFolder;
                }
            }
        }

        // TODO
        RCF_ASSERT(0 && "Too many leftover temp folders!");
        return fs::path();
    }

    void trimManifest(
        const FileManifest & manifest, 
        boost::uint64_t & bytesAlreadyTransferred,
        FileChunk & startPos);

    void FileTransferService::processZeroSizeEntries(RCF::FileUploadInfo & uploadInfo)
    {
        if (uploadInfo.mCurrentPos != 0)
        {
            return;
        }
        
        while (     0 <= uploadInfo.mCurrentFile 
                &&  uploadInfo.mCurrentFile < uploadInfo.mManifest.mFiles.size())
        {
            FileInfo & info = uploadInfo.mManifest.mFiles[uploadInfo.mCurrentFile];
            if (info.mFileSize == 0)
            {
                fs::path targetPath = uploadInfo.mUploadPath / info.mFilePath;
                if (info.mRenameFile.size() > 0)
                {
                    targetPath = uploadInfo.mUploadPath / info.mRenameFile;
                }

                if (info.mIsDirectory)
                {
                    fs::create_directories(targetPath);
                }
                else
                {
                    fs::path targetDir = (targetPath / "..").normalize();
                    fs::create_directories(targetDir);

                    std::ofstream fout( 
                        nativeString(targetPath).c_str(), 
                        std::ios::binary | std::ios::trunc );

                    fout.close();

                    std::time_t writeTime = static_cast<std::time_t>(info.mLastWriteTime);
                    fs::last_write_time(targetPath, writeTime);
                }

                ++uploadInfo.mCurrentFile;
                continue;
            }
            
            break;
        }
    }

    void FileTransferService::checkForUploadCompletion(FileUploadInfoPtr uploadInfoPtr)
    {
        FileUploadInfo & uploadInfo = *uploadInfoPtr;

        if (uploadInfo.mCurrentFile == uploadInfo.mManifest.mFiles.size())
        {
            RCF_LOG_3()(uploadInfo.mCurrentFile) 
                << "FileTransferService - upload completed.";

            uploadInfo.mCompleted = true;

            if (uploadInfo.mSessionLocalId)
            {
                RcfSession& session = getTlsRcfSession();
                Lock lock(session.mMutex);
                session.mSessionUploads[uploadInfo.mSessionLocalId] = uploadInfoPtr;
                session.mUploadInfoPtr.reset();
            }

            if (uploadInfo.mUploadId.size() > 0)
            {
                removeUpload(uploadInfo.mUploadId);
            }
        }
    }

    void FileTransferService::BeginUpload(
        const FileManifest & manifest,
        const std::vector<FileChunk> & chunks,
        FileChunk & startPos,
        boost::uint32_t & maxMessageLength,
        std::string & uploadId,
        boost::uint32_t & bps,
        boost::uint32_t sessionLocalId)
    {

        RCF_LOG_3()(sessionLocalId) << "FileTransferService::BeginUpload() - entry.";

        RCF_UNUSED_VARIABLE(chunks);

        namespace fs = boost::filesystem;

        NetworkSession & networkSession = getTlsRcfSession().getNetworkSession();
        maxMessageLength = (boost::uint32_t) networkSession.getServerTransport().getMaxMessageLength();

        RCF::BandwidthQuotaPtr quotaPtr = mUploadQuotaCallback ? 
            mUploadQuotaCallback(RCF::getCurrentRcfSession()) : 
            mUploadQuota;

        FileUploadInfoPtr uploadInfoPtr( new FileUploadInfo(quotaPtr) );
        uploadInfoPtr->mManifest = manifest;
        uploadInfoPtr->mTimeStampMs = RCF::getCurrentTimeMs();
        uploadInfoPtr->mSessionLocalId = sessionLocalId;

        if (uploadId.size() > 0)
        {
            fs::path uploadPath = findUpload(uploadId);
            if (!uploadPath.empty())
            {
                uploadInfoPtr->mUploadPath = uploadPath;

                // Trim the manifest to account for already uploaded fragments.
                const_cast<FileManifest &>(manifest).mManifestBase = uploadPath;

                boost::uint64_t bytesAlreadyTransferred = 0;
                trimManifest(manifest, bytesAlreadyTransferred, startPos);
                uploadInfoPtr->mResume = true;
            }
        }

        if (uploadInfoPtr->mUploadPath.empty())
        {
            // Create a temp folder to upload to.
            uploadInfoPtr->mUploadPath = makeTempDir(fs::path(mUploadDirectory), "");

            uploadId = addUpload(uploadInfoPtr->mUploadPath);
            startPos = FileChunk();
        }

        uploadInfoPtr->mUploadId = uploadId;
        uploadInfoPtr->mCurrentFile = startPos.mFileIndex;
        uploadInfoPtr->mCurrentPos = startPos.mOffset;

        {
            RcfSession& session = getTlsRcfSession();
            Lock lock(session.mMutex);
            session.mUploadInfoPtr = uploadInfoPtr;
        }

        bps = uploadInfoPtr->mQuotaPtr->calculateLineSpeedLimit();

        processZeroSizeEntries(*uploadInfoPtr);
        checkForUploadCompletion(uploadInfoPtr);

        if (mOnFileUploadProgress)
        {
            mOnFileUploadProgress(getCurrentRcfSession(), *uploadInfoPtr);
        }

        RCF_LOG_3()(startPos.mFileIndex)(startPos.mOffset)(maxMessageLength)(uploadId)(bps) 
            << "FileTransferService::BeginUpload() - exit.";
    }

    void FileTransferService::UploadChunks(
        const std::vector<FileChunk> & chunks,
        boost::uint32_t & bps)
    {
        RCF_LOG_3()(chunks.size()) 
            << "FileTransferService::UploadChunks() - entry.";

        namespace fs = boost::filesystem;

        // Find the upload.
        FileUploadInfoPtr uploadInfoPtr = getTlsRcfSession().mUploadInfoPtr;
        
        if (!uploadInfoPtr)
        {
            RCF_THROW( Exception(_RcfError_NoUpload()) );
        }

        FileUploadInfo & uploadInfo = * uploadInfoPtr;

        if (uploadInfo.mCompleted)
        {
            RCF_THROW( Exception(_RcfError_UploadAlreadyCompleted()) );
        }

        
        RCF::BandwidthQuotaPtr quotaPtr = uploadInfo.mQuotaPtr;
        bps = quotaPtr->calculateLineSpeedLimit();

        for (std::size_t i=0; i<chunks.size(); ++i)
        {
            const FileChunk & chunk = chunks[i];

            if (chunk.mFileIndex != uploadInfo.mCurrentFile)
            {
                processZeroSizeEntries(uploadInfo);
                if (chunk.mFileIndex != uploadInfo.mCurrentFile)
                {
                    RCF_THROW( Exception(_RcfError_FileIndex(uploadInfo.mCurrentFile, chunk.mFileIndex)) );
                }
            }

            RCF_ASSERT_EQ( chunk.mFileIndex , uploadInfo.mCurrentFile );

            FileInfo file = uploadInfo.mManifest.mFiles[uploadInfo.mCurrentFile];

            fs::path filePath = uploadInfo.mUploadPath / file.mFilePath;

            if (uploadInfo.mCurrentPos == 0 || uploadInfo.mResume)
            {
                RCF_ASSERT(uploadInfo.mWriteOp->isCompleted());

                if (file.mRenameFile.size() > 0)
                {
                    filePath = uploadInfo.mUploadPath / file.mRenameFile;
                }

                if ( !fs::exists( filePath.branch_path() ) )
                {
                    fs::create_directories( filePath.branch_path() );
                }

                RCF_LOG_3()(uploadInfo.mCurrentFile)(filePath) 
                    << "FileTransferService::UploadChunks() - opening file.";

                uploadInfo.mFileStreamPath = filePath;

                if (uploadInfo.mResume)
                {
                    uploadInfo.mFileStream->open( 
                        filePath.string().c_str(), 
                        std::ios::binary | std::ios::app );
                }
                else
                {
                    uploadInfo.mFileStream->open( 
                        filePath.string().c_str(), 
                        std::ios::binary | std::ios::trunc );
                }

                if (! uploadInfo.mFileStream->good())
                {
                    RCF_THROW( Exception(_RcfError_FileWrite(nativeString(filePath), 0)));
                }

                if (uploadInfo.mResume && uploadInfo.mCurrentPos > 0)
                {
                    RCF_LOG_3()(uploadInfo.mCurrentFile)(uploadInfo.mCurrentPos) 
                        << "FileTransferService::UploadChunks() - seeking in file.";

                    uploadInfo.mFileStream->seekp( 
                        static_cast<std::streamoff>(uploadInfo.mCurrentPos) );
                }

                uploadInfo.mResume = false;
            }

            OfstreamPtr fout = uploadInfoPtr->mFileStream;

            // Wait for previous write to complete.
            if (uploadInfo.mWriteOp->isInitiated())
            {
                uploadInfo.mWriteOp->complete();

                boost::uint64_t bytesWritten = uploadInfo.mWriteOp->getBytesTransferred();
                if (bytesWritten == 0)
                {
                    fout->close();
                    RCF_THROW( Exception(_RcfError_FileWrite(nativeString(filePath), uploadInfo.mCurrentPos)) );
                }
            }

            // Initiate next write.

            // Check stream state.
            if (!fout->good())
            {
                RCF_THROW( Exception(_RcfError_FileWrite(nativeString(uploadInfo.mFileStreamPath), uploadInfo.mCurrentPos)) );
            }

            // Check the offset position.
            uploadInfo.mCurrentPos = fout->tellp();
            if (chunk.mOffset != uploadInfo.mCurrentPos)
            {
                RCF_THROW( Exception(_RcfError_FileOffset(uploadInfo.mCurrentPos, chunk.mOffset)) );
            }

            // Check the chunk size.
            boost::uint64_t fileSize = file.mFileSize;
            boost::uint64_t remainingFileSize = fileSize - uploadInfo.mCurrentPos;
            if (chunk.mData.getLength() > remainingFileSize)
            {
                RCF_THROW( Exception(_RcfError_UploadFileSize()) );
            }

            uploadInfo.mWriteOp->initateWrite(uploadInfo.mFileStream, chunk.mData);

            uploadInfoPtr->mTimeStampMs = RCF::getCurrentTimeMs();

            // Check if last chunk.
            uploadInfo.mCurrentPos += chunk.mData.getLength();
            if (uploadInfo.mCurrentPos == fileSize)
            {
                RCF_LOG_3()(uploadInfo.mCurrentFile) 
                    << "FileTransferService::UploadChunks() - closing file.";

                uploadInfo.mWriteOp->complete();
                fout->close();

                std::time_t writeTime = static_cast<std::time_t>(file.mLastWriteTime);
                fs::last_write_time(filePath, writeTime);
                
                ++uploadInfo.mCurrentFile;
                uploadInfo.mCurrentPos = 0;

                processZeroSizeEntries(uploadInfo);

                checkForUploadCompletion(uploadInfoPtr);                
            }
        }

        if (mOnFileUploadProgress)
        {
            mOnFileUploadProgress(getCurrentRcfSession(), uploadInfo);
        }

        RCF_LOG_3() << "FileTransferService::UploadChunks() - exit.";
    }

    namespace fs = boost::filesystem;

    std::string FileTransferService::addUpload(const fs::path & uploadPath)
    {
        Lock lock(mUploadsInProgressMutex);
        std::string uploadId;
        while (true)
        {
            // Largest 32 bit number: 4294967296 (10 digits).
            boost::uint32_t n = static_cast<boost::uint32_t>(rand());
            std::ostringstream os;
            os << std::setw(10) << std::setfill('0') << n;
            uploadId = os.str();

            if (mUploadsInProgress.find(uploadId) == mUploadsInProgress.end())
            {
                break;
            }
        }
        mUploadsInProgress[uploadId] = uploadPath; 
        return uploadId;
    }

    void FileTransferService::removeUpload(const std::string & uploadId)
    {
        Lock lock(mUploadsInProgressMutex);

        UploadsInProgress::iterator iter = mUploadsInProgress.find(uploadId);

        if (iter != mUploadsInProgress.end())
        {
            mUploadsInProgress.erase(iter);
        }
    }

    fs::path FileTransferService::findUpload(const std::string & uploadId)
    {
        Lock lock(mUploadsInProgressMutex);

        UploadsInProgress::iterator iter = mUploadsInProgress.find(uploadId);

        if (iter != mUploadsInProgress.end())
        {
            return iter->second;
        }

        return fs::path();
    }

    void FileTransferService::BeginDownload(
        FileManifest & manifest,
        const FileTransferRequest & request,
        std::vector<FileChunk> & chunks,
        boost::uint32_t & maxMessageLength,
        boost::uint32_t & bps,
        boost::uint32_t sessionLocalId)
    {
        RCF_LOG_3()(sessionLocalId) << "FileTransferService::BeginDownload() - entry.";

        FileDownloadInfoPtr downloadInfoPtr;

        if (sessionLocalId)
        {
            BandwidthQuotaPtr quotaPtr = mDownloadQuotaCallback ? 
                mDownloadQuotaCallback(RCF::getCurrentRcfSession()) : 
                mDownloadQuota;

            RcfSession& session = getTlsRcfSession();
            Lock lock(session.mMutex);
            FileStream & fs = session.mSessionDownloads[sessionLocalId];
            downloadInfoPtr.reset( new FileDownloadInfo(quotaPtr) );
            downloadInfoPtr->mManifest = fs.mImplPtr->mManifest;
            downloadInfoPtr->mDownloadPath = downloadInfoPtr->mManifest.mManifestBase;
            downloadInfoPtr->mSessionLocalId = sessionLocalId;
            session.mDownloadInfoPtr = downloadInfoPtr;
        }
        else
        {
            RCF_THROW( Exception(_RcfError_NoDownload()) );
        }
        
        FileDownloadInfo & di = * downloadInfoPtr;

        manifest = di.mManifest;
        di.mCurrentFile = 0;
        di.mCurrentPos = 0;
        if (!di.mManifest.mFiles.empty())
        {
            di.mCurrentPos = di.mManifest.mFiles[0].mFileStartPos;
        }

        bps = di.mQuotaPtr->calculateLineSpeedLimit();

        // TODO: optional first chunks.
        RCF_UNUSED_VARIABLE(request);
        chunks.clear();

        NetworkSession & networkSession = getTlsRcfSession().getNetworkSession();
        maxMessageLength = (boost::uint32_t) networkSession.getServerTransport().getMaxMessageLength();

        if (mOnFileDownloadProgress)
        {
            mOnFileDownloadProgress(getCurrentRcfSession(), di);
        }

        RCF_LOG_3()(manifest.mFiles.size())(maxMessageLength) 
            << "FileTransferService::BeginDownload() - exit.";
    }

    void FileTransferService::TrimDownload(
        const FileChunk & startPos)
    {
        RCF_LOG_3()(startPos.mFileIndex)(startPos.mOffset) 
            << "FileTransferService::TrimDownload() - entry.";

        FileDownloadInfoPtr downloadInfoPtr = getTlsRcfSession().mDownloadInfoPtr;

        if (!downloadInfoPtr)
        {
            RCF_THROW( Exception(_RcfError_NoDownload()) );
        }

        FileDownloadInfo & di = * downloadInfoPtr;

        RCF_ASSERT_LTEQ(startPos.mFileIndex , di.mManifest.mFiles.size());
        if (startPos.mFileIndex < di.mManifest.mFiles.size())
        {
            RCF_ASSERT_LT(startPos.mOffset , di.mManifest.mFiles[startPos.mFileIndex].mFileSize)(startPos.mFileIndex);
        }
        else
        {
            RCF_ASSERT_EQ(startPos.mOffset , 0);
        }

        di.mCurrentFile = startPos.mFileIndex;
        di.mCurrentPos = startPos.mOffset;
        di.mResume = true;

        RCF_LOG_3() << "FileTransferService::TrimDownload() - exit.";
    }

    void FileTransferService::DownloadChunks(
        const FileTransferRequest & request,
        std::vector<FileChunk> & chunks,
        boost::uint32_t & adviseWaitMs,
        boost::uint32_t & bps)
    {
        RCF_LOG_3()(request.mFile)(request.mPos)(request.mChunkSize) 
            << "FileTransferService::DownloadChunks() - entry.";

        // Find the download.
        FileDownloadInfoPtr & diPtr = getTlsRcfSession().mDownloadInfoPtr;

        if (!diPtr)
        {
            RCF_THROW( Exception(_RcfError_NoDownload()) );
        }

        FileDownloadInfo & di = *diPtr;

        if (di.mCancel)
        {
            // TODO: reset mDownloadInfoPtr?
            RCF_THROW( Exception(_RcfError_DownloadCancelled()) );
        }

        adviseWaitMs = 0;

        // Skip past any zero-length entries.
        while (     di.mCurrentFile < di.mManifest.mFiles.size() 
            &&  di.mManifest.mFiles[di.mCurrentFile].mFileSize == 0)
        {
            ++di.mCurrentFile;
        }

        // Check offset.
        if (request.mFile != di.mCurrentFile)
        {
            RCF_THROW( Exception(_RcfError_FileIndex(di.mCurrentFile, request.mFile)) );
        }

        if (request.mPos != di.mCurrentPos)
        {
            RCF_THROW( Exception(_RcfError_FileOffset(di.mCurrentPos, request.mPos)) );
        }

        chunks.clear();

        boost::uint32_t chunkSize = request.mChunkSize;

        // Trim the chunk size, according to throttle settings.
        bps = di.mQuotaPtr->calculateLineSpeedLimit();

        if (bps)
        {
            RCF_LOG_3()(bps)(mTransferWindowS)(di.mTransferWindowBytesTotal)(di.mTransferWindowBytesSoFar) 
                << "FileTransferService::DownloadChunks() - checking throttle setting.";

            if (di.mTransferWindowTimer.elapsed(mTransferWindowS*1000))
            {
                RCF_ASSERT_GTEQ(di.mTransferWindowBytesTotal , di.mTransferWindowBytesSoFar);

                boost::uint32_t carryOver = 
                    di.mTransferWindowBytesTotal - di.mTransferWindowBytesSoFar;

                di.mTransferWindowTimer.restart();

                di.mTransferWindowBytesTotal = bps * mTransferWindowS;
                di.mTransferWindowBytesTotal += carryOver;

                di.mTransferWindowBytesSoFar = 0;

                RCF_LOG_3()(mTransferWindowS)(di.mTransferWindowBytesTotal)(di.mTransferWindowBytesSoFar)(carryOver) 
                    << "FileTransferService::DownloadChunks() - new throttle transfer window.";
            }

            if (di.mTransferWindowBytesTotal == 0)
            {
                di.mTransferWindowBytesTotal = bps * mTransferWindowS;
            }

            boost::uint32_t bytesWindowRemaining = 
                di.mTransferWindowBytesTotal - di.mTransferWindowBytesSoFar;

            if (bytesWindowRemaining < chunkSize)
            {
                boost::uint32_t windowStartMs = di.mTransferWindowTimer.getStartTimeMs();
                boost::uint32_t windowEndMs = windowStartMs + 1000*mTransferWindowS;
                boost::uint32_t nowMs = getCurrentTimeMs();
                if (nowMs < windowEndMs)
                {
                    adviseWaitMs = windowEndMs - nowMs;

                    RCF_LOG_3()(adviseWaitMs) 
                        << "FileTransferService::DownloadChunks() - advising client wait.";
                }
            }

            RCF_LOG_3()(chunkSize)(bytesWindowRemaining)(di.mTransferWindowBytesTotal) 
                << "FileTransferService::DownloadChunks() - trimming chunk size to transfer window.";

            chunkSize = RCF_MIN(chunkSize, bytesWindowRemaining);
        }

        boost::uint32_t totalBytesRead = 0;

        while (
                totalBytesRead < chunkSize 
            &&  di.mCurrentFile != di.mManifest.mFiles.size())
        {
            FileInfo & currentFileInfo = di.mManifest.mFiles[di.mCurrentFile];

            if (di.mCurrentPos == 0 || di.mResume)
            {
                di.mResume = false;

                fs::path manifestBase = di.mDownloadPath;
                FileInfo & currentFileInfo = di.mManifest.mFiles[di.mCurrentFile];
                fs::path filePath = currentFileInfo.mFilePath;
                fs::path totalPath = manifestBase / filePath;
                boost::uint64_t fileSize = currentFileInfo.mFileSize;

                RCF_LOG_3()(di.mCurrentFile)(fileSize)(totalPath)
                    << "FileTransferService::DownloadChunks() - opening file.";

                di.mFileStreamPath = totalPath;

                di.mFileStream->clear();
                di.mFileStream->open(
                    totalPath.string().c_str(),
                    std::ios::in | std::ios::binary);

                RCF_VERIFY(
                    di.mFileStream->good(), 
                    Exception(_RcfError_FileOpen(nativeString(totalPath))));

                if (di.mCurrentPos != 0)
                {
                    RCF_LOG_3()(di.mCurrentFile)(di.mCurrentPos) 
                        << "FileTransferService::DownloadChunks() - seeking in file.";

                    di.mFileStream->seekg( static_cast<std::streamoff>(di.mCurrentPos) );

                    RCF_VERIFY(
                        di.mFileStream->good(), 
                        Exception(_RcfError_FileSeek(nativeString(totalPath), diPtr->mCurrentPos)));
                }
            }
            
            boost::uint64_t fileSize = currentFileInfo.mFileSize;
            boost::uint64_t bytesRemainingInFile =  fileSize - di.mCurrentPos;
            boost::uint64_t bytesRemainingInChunk = chunkSize - totalBytesRead;

            if (di.mReadOp->isInitiated())
            {
                RCF_LOG_3() 
                    << "FileTransferService::DownloadChunks() - completing read.";

                // Wait for async read to complete.
                di.mReadOp->complete();

                std::size_t bytesRead = static_cast<std::size_t>(
                    di.mReadOp->getBytesTransferred());

                if (bytesRead == 0)
                {
                    RCF_THROW( Exception(_RcfError_FileRead(nativeString(di.mFileStreamPath), di.mCurrentPos)));
                }
                di.mSendBuffer.swap(di.mReadBuffer);
                di.mSendBufferRemaining = ByteBuffer(di.mSendBuffer, 0, bytesRead);

                RCF_LOG_3()(bytesRead) 
                    << "FileTransferService::DownloadChunks() - read completed.";
            }

            ByteBuffer byteBuffer;
            IfstreamPtr fin = di.mFileStream;

            if (di.mSendBufferRemaining)
            {
                std::size_t bytesToRead = RCF_MIN(
                    di.mSendBufferRemaining.getLength(), 
                    static_cast<std::size_t>(bytesRemainingInChunk));

                byteBuffer = ByteBuffer(di.mSendBufferRemaining, 0, bytesToRead);
                di.mSendBufferRemaining = ByteBuffer(di.mSendBufferRemaining, bytesToRead);
            }
            else
            {
                // No asynchronously data available. Do a synchronous read.
                byteBuffer = ByteBuffer( static_cast<std::size_t>( 
                    RCF_MIN(bytesRemainingInChunk, bytesRemainingInFile) ));

                RCF_LOG_3()(di.mCurrentFile)(byteBuffer.getLength()) 
                    << "FileTransferService::DownloadChunks() - reading from file.";

                boost::uint32_t bytesRead = static_cast<boost::uint32_t>(fin->read( 
                    byteBuffer.getPtr(), 
                    static_cast<std::size_t>(byteBuffer.getLength()) ).gcount());

                if (fin->fail() && !fin->eof())
                {
                    RCF_THROW( Exception(_RcfError_FileRead(nativeString(di.mFileStreamPath), di.mCurrentPos)) );
                }

                byteBuffer = ByteBuffer(byteBuffer, 0, bytesRead);
            }

            FileChunk fileChunk;
            fileChunk.mFileIndex = di.mCurrentFile;
            fileChunk.mOffset = di.mCurrentPos;
            fileChunk.mData = byteBuffer;
            chunks.push_back(fileChunk);

            totalBytesRead += (boost::uint32_t) byteBuffer.getLength();
            diPtr->mCurrentPos += byteBuffer.getLength();

            if (diPtr->mCurrentPos == currentFileInfo.mFileSize)
            {
                RCF_LOG_3()(diPtr->mCurrentFile) 
                    << "FileTransferService::DownloadChunks() - closing file.";

                fin->close();
                ++di.mCurrentFile;
                di.mCurrentPos = 0;

                // Skip past any zero-length entries.
                while (     di.mCurrentFile < di.mManifest.mFiles.size() 
                        &&  di.mManifest.mFiles[di.mCurrentFile].mFileSize == 0)
                {
                    ++di.mCurrentFile;
                }

                if (di.mCurrentFile < di.mManifest.mFiles.size())
                {
                    FileInfo & nextFile = di.mManifest.mFiles[di.mCurrentFile];
                    di.mCurrentPos = nextFile.mFileStartPos;
                }
            }
        }

        di.mTransferWindowBytesSoFar += totalBytesRead;

        if (di.mCurrentFile == di.mManifest.mFiles.size())
        {
            RCF_LOG_3()(di.mCurrentFile) 
                << "FileTransferService::DownloadChunks() - download completed.";

            // TODO: this is broken if there is more than one FileStream.
            if (diPtr->mSessionLocalId)
            {
                std::map<boost::uint32_t, FileDownload> & downloads = 
                    getTlsRcfSession().mSessionDownloads;

                std::map<boost::uint32_t, FileDownload>::iterator iter = 
                    downloads.find(diPtr->mSessionLocalId);

                RCF_ASSERT(iter != downloads.end());

                downloads.erase(iter);
            }
            diPtr.reset();
        }

        // Initiate read for next chunk.
        if (    diPtr.get()
            &&  di.mSendBufferRemaining.isEmpty()
            &&  di.mCurrentFile < di.mManifest.mFiles.size()
            &&  0 < di.mCurrentPos
            &&  ! di.mReadOp->isInitiated())
        {
            boost::uint64_t fileSize = di.mManifest.mFiles[di.mCurrentFile].mFileSize;
            if (di.mCurrentPos < fileSize)
            {
                if (di.mReadBuffer.isEmpty())
                {
                    RCF_ASSERT(di.mSendBuffer.isEmpty());
                    RCF_ASSERT(di.mSendBufferRemaining.isEmpty());

                    di.mReadBuffer = ByteBuffer(request.mChunkSize);
                    di.mSendBuffer = ByteBuffer(request.mChunkSize);
                }

                std::size_t bytesToRead = static_cast<std::size_t>(
                    fileSize - di.mCurrentPos);

                bytesToRead = RCF_MIN(bytesToRead, di.mReadBuffer.getLength());

                RCF_LOG_3()(di.mCurrentFile)(di.mCurrentPos)(fileSize)(bytesToRead) 
                    << "FileTransferService::DownloadChunks() - initiate read for next chunk.";

                di.mReadOp->initiateRead(di.mFileStream, ByteBuffer(di.mReadBuffer, 0, bytesToRead));
            }
        }

        if (mOnFileDownloadProgress)
        {
            mOnFileDownloadProgress(getCurrentRcfSession(), di);
        }

        RCF_LOG_3()(chunks.size()) 
            << "FileTransferService::DownloadChunks() - exit.";
    }

    void FileTransferService::onServerStart(RcfServer & server)
    {
        mUploadDirectory = server.getFileUploadDirectory();
        if (mUploadDirectory.empty())
        {
            mUploadDirectory = fs::initial_path() / "RCF-Uploads";
        }

        mUploadQuota.reset(new BandwidthQuota( server.getFileUploadBandwidthLimit() ));
        mDownloadQuota.reset(new BandwidthQuota( server.getFileDownloadBandwidthLimit() ));

        mUploadQuotaCallback = server.mFileUploadQuotaCb;
        mDownloadQuotaCallback = server.mFileDownloadQuotaCb;

        mOnFileDownloadProgress = server.mOnFileDownloadProgress;
        mOnFileUploadProgress = server.mOnFileUploadProgress;

        server.bind<I_FileTransferService>(*this);
    }

    void FileTransferService::onServerStop(RcfServer & server)
    {
        server.unbind<I_FileTransferService>();
    }

#if RCF_FEATURE_SF==1

    void FileManifest::serialize(SF::Archive & ar) 
    {
        ar & mFiles;
    }

    void FileInfo::serialize(SF::Archive & ar) 
    {
        ar 
            & mIsDirectory 
            & mFilePath 
            & mFileStartPos 
            & mFileSize 
            & mFileCrc 
            & mRenameFile;

        if (ar.getRuntimeVersion() >= 11)
        {
            ar & mLastWriteTime;
        }
    }

    void FileChunk::serialize(SF::Archive & ar)
    {
        ar & mFileIndex & mOffset & mData;
    }

    void FileTransferRequest::serialize(SF::Archive & ar)
    {
        ar & mFile & mPos & mChunkSize;
    }

#endif

    fs::path getRelativePath(const fs::path & basePath, const fs::path & fullPath)
    {
        fs::path::iterator baseIter = basePath.begin();
        fs::path::iterator fullIter = fullPath.begin();

        while ( baseIter != basePath.end() ) 
        {
            if (fullIter == fullPath.end())
            {
                return fs::path();
            }
            if (*baseIter != *fullIter) 
            {
                return fs::path();
            }
            ++fullIter; 
            ++baseIter;
        }

        fs::path relativePath;
        while (fullIter != fullPath.end())
        {
            relativePath /= *fullIter;
            ++fullIter;
        }
        return relativePath;
    }

    FileManifest::FileManifest(boost::filesystem::path pathToFiles) 
    {
        if (!fs::exists(pathToFiles))
        {
            RCF_THROW( RCF::Exception( _RcfError_FileOpen(nativeString(pathToFiles)) ) );
        }

        if (fs::is_directory(pathToFiles))
        {
            for ( 
                fs::recursive_directory_iterator iter(pathToFiles); 
                iter != fs::recursive_directory_iterator(); 
                ++iter )
            { 

                fs::path fullPath = *iter;
                
                FileInfo fileInfo;

                if ( fs::is_directory(fullPath) )
                {
                    fileInfo.mIsDirectory = true;
                    fileInfo.mFileSize = 0;
                    fileInfo.mFileCrc = 0;
                }
                else
                {
                    fileInfo.mIsDirectory = false;
                    fileInfo.mFileSize = fs::file_size(fullPath);
                    fileInfo.mFileCrc = 0;
                    fileInfo.mLastWriteTime = fs::last_write_time(fullPath);
                }

                fs::path basePath = (pathToFiles / "..").normalize();
                fs::path relativePath = getRelativePath(basePath, fullPath);
                fileInfo.mFilePath = relativePath;

                mFiles.push_back(fileInfo);
            }
        }
        else
        {
            FileInfo fileInfo;
            fileInfo.mFileSize = fs::file_size(pathToFiles);
            fileInfo.mFileCrc = 0;
            fileInfo.mLastWriteTime = fs::last_write_time(pathToFiles);

#if BOOST_VERSION <= 104300
            fileInfo.mFilePath = pathToFiles.leaf();
#else
            fileInfo.mFilePath = pathToFiles.filename();
#endif

            mFiles.push_back(fileInfo);
        }

        if (fs::is_directory( fs::path(pathToFiles) ))
        {
            mManifestBase = (fs::path(pathToFiles) / "..").normalize();
        }
        else
        {
            mManifestBase = fs::path(pathToFiles).branch_path();
        }
    }

    boost::uint64_t FileManifest::getTotalByteSize() const
    {
        boost::uint64_t totalByteSize = 0;
        for (std::size_t i=0; i<mFiles.size(); ++i)
        {
            const FileInfo & fi = mFiles[i];
            boost::uint64_t fileSize = fi.mFileSize - fi.mFileStartPos;
            totalByteSize += fileSize;
        }
        return totalByteSize;
    }

    FileUploadInfo::FileUploadInfo(BandwidthQuotaPtr quotaPtr) : 
        mFileStream( new std::ofstream() ),
        mWriteOp( new FileIoRequest() ),
        mCompleted(false),
        mResume(false),
        mTimeStampMs(0),
        mCurrentFile(0),
        mCurrentPos(0),
        mSessionLocalId(0),
        mQuotaPtr(quotaPtr)
    {
        mQuotaPtr->addUpload(this);
    }

    FileUploadInfo::~FileUploadInfo()
    {
        mFileStream->close();

        mQuotaPtr->removeUpload(this);
        mQuotaPtr.reset();

        // Best effort only.
        try
        {
            if (!mCompleted && mUploadId.empty())
            {
                fs::remove_all(mUploadPath);
            }
        }
        catch(const std::exception & e)
        {
            std::string error = e.what();
        }
        catch(...)
        {

        }
    }

    FileDownloadInfo::FileDownloadInfo(BandwidthQuotaPtr quotaPtr) :
        mFileStream( new std::ifstream() ),
        mReadOp( new FileIoRequest() ),
        mCurrentFile(0),
        mCurrentPos(0),
        mResume(false),
        mTransferWindowBytesSoFar(0),
        mTransferWindowBytesTotal(0),
        mCancel(false),
        mSessionLocalId(0),
        mQuotaPtr(quotaPtr)
    {
        mQuotaPtr->addDownload(this);
    }

    FileDownloadInfo::~FileDownloadInfo()
    {
        mQuotaPtr->removeDownload(this);
        mQuotaPtr.reset();
    }

    void FileTransferService::setTransferWindowS(boost::uint32_t transferWindowS)
    {
        mTransferWindowS = transferWindowS;
    }

    boost::uint32_t FileTransferService::getTransferWindowS()
    {
        return mTransferWindowS;
    }

    BandwidthQuota::BandwidthQuota() : mQuotaBps(0)
    {
    }

    BandwidthQuota::BandwidthQuota(boost::uint32_t quotaBps) : mQuotaBps(quotaBps)
    {
    }

    void BandwidthQuota::addUpload(FileUploadInfo * pUpload)
    {
        Lock lock(mMutex);
        mUploadsInProgress.insert(pUpload);
    }
    void BandwidthQuota::removeUpload(FileUploadInfo * pUpload)
    {
        Lock lock(mMutex);
        mUploadsInProgress.erase(pUpload);
    }

    void BandwidthQuota::addDownload(FileDownloadInfo * pDownload)
    {
        Lock lock(mMutex);
        mDownloadsInProgress.insert(pDownload);
    }

    void BandwidthQuota::removeDownload(FileDownloadInfo * pDownload)
    {
        Lock lock(mMutex);
        mDownloadsInProgress.erase(pDownload);
    }

    void BandwidthQuota::setQuota(boost::uint32_t quotaBps)
    {
        Lock lock(mMutex);
        mQuotaBps = quotaBps;
    }

    boost::uint32_t BandwidthQuota::calculateLineSpeedLimit()
    {
        Lock lock(mMutex);
        std::size_t transfers = mUploadsInProgress.size() + mDownloadsInProgress.size();
        RCF_ASSERT(transfers > 0);
        return mQuotaBps / (boost::uint32_t) transfers;
    }

} // namespace RCF
