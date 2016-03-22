
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

#include <RCF/ClientStub.hpp>

#include <sys/stat.h>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <RCF/FileIoThreadPool.hpp>
#include <RCF/FileTransferService.hpp>
#include <RCF/ServerInterfaces.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace fs = boost::filesystem;

namespace RCF {

    // FileStream

    FileStream::FileStream() : mImplPtr( new FileStreamImpl() )
    {
    }

    FileStream::FileStream(FileStreamImplPtr implPtr) : 
        mImplPtr(implPtr)
    {
    }

    FileStream::FileStream(const std::string & filePath) : 
        mImplPtr(new FileStreamImpl(filePath) )
    {
    }

    FileStream::FileStream(const FileManifest & manifest) : 
        mImplPtr(new FileStreamImpl(manifest) )
    {
    }

    std::string FileStream::getLocalPath() const
    {
        RCF_ASSERT(mImplPtr);
        FileManifest & manifest = mImplPtr->mManifest;
        fs::path localPath = manifest.mManifestBase;
        RCF_ASSERT(manifest.mFiles.size() > 0);
        localPath /= (*manifest.mFiles[0].mFilePath.begin());
        return nativeString(localPath);
    }

    FileManifest & FileStream::getManifest() const
    {
        RCF_ASSERT(mImplPtr);
        return mImplPtr->mManifest;
    }

    void FileStream::setDownloadToPath(const std::string & downloadToPath)
    {
        RCF_ASSERT(mImplPtr);
        mImplPtr->mDownloadToPath = downloadToPath;
    }

    std::string FileStream::getDownloadToPath() const
    {
        RCF_ASSERT(mImplPtr);
        return nativeString(mImplPtr->mDownloadToPath);
    }

    void FileStream::setTransferRateBps(boost::uint32_t transferRateBps)
    {
        RCF_ASSERT(mImplPtr);
        mImplPtr->mTransferRateBps = transferRateBps;
    }

    boost::uint32_t FileStream::getTransferRateBps()
    {
        RCF_ASSERT(mImplPtr);
        return mImplPtr->mTransferRateBps;
    }

#if RCF_FEATURE_SF==1

    void FileStream::serialize(SF::Archive & ar)
    {
        ar & *mImplPtr;
    }

    void FileStreamImpl::serializeImplSf(
        SF::Archive & ar, 
        boost::uint32_t & transferId, 
        Direction & dir)
    {
        ar & transferId & dir;
    }

    void FileStreamImpl::serialize(SF::Archive & ar)
    {
        bool isSaving = ar.isWrite(); 

        serializeGeneric( 
            isSaving,
            boost::bind( 
                &FileStreamImpl::serializeImplSf, 
                this, 
                boost::ref(ar),
                _1, 
                _2) );
    }

#endif

    void FileStream::upload(RCF::ClientStub & clientStub)
    {
        boost::uint32_t chunkSize = 1024*1024;
        
        clientStub.uploadFiles(
            mImplPtr->mManifest, 
            mImplPtr->mUploadId, 
            chunkSize, 
            mImplPtr->mTransferRateBps,
            mImplPtr->mSessionLocalId);

        mImplPtr->mSessionLocalId = 0;
    }

    fs::path makeTempDir(const fs::path & basePath, const std::string & prefix);

    void FileStream::download(RCF::ClientStub & clientStub)
    {
        boost::uint32_t chunkSize = 1024*1024;
        
        if (mImplPtr->mDownloadToPath.empty())
        {
            mImplPtr->mDownloadToPath = makeTempDir("RCF-Downloads", "");
        }

        clientStub.downloadFiles(
            nativeString(mImplPtr->mDownloadToPath), 
            mImplPtr->mManifest, 
            chunkSize, 
            mImplPtr->mTransferRateBps,
            mImplPtr->mSessionLocalId);

        mImplPtr->mManifest.mManifestBase = mImplPtr->mDownloadToPath;
        mImplPtr->mSessionLocalId = 0;
    }

    // FileStreamImpl

    FileStreamImpl::FileStreamImpl() : 
        mTransferRateBps(0),
        mSessionLocalId(0), 
        mDirection(Unspecified)
    {
    }

    FileStreamImpl::FileStreamImpl(const std::string & filePath) : 
        mManifest(filePath),
        mTransferRateBps(0),
        mSessionLocalId(0),
        mDirection(Unspecified)
    {
    }

    FileStreamImpl::FileStreamImpl(const FileManifest & manifest) :
        mManifest(manifest),
        mTransferRateBps(0),
        mSessionLocalId(0),
        mDirection(Unspecified)
    {
    }

    FileStreamImpl::~FileStreamImpl()
    {
    }

    void FileStreamImpl::serializeGeneric(
        bool isWriting,
        boost::function2<void, boost::uint32_t &, Direction &> serializeImpl)
    {
        // Determine if we are client side or server side.
        // TODO: what if both of these are non-zero?
        RCF::ClientStub * pClientStub = RCF::getTlsClientStubPtr();
        RCF::RcfSession * pSession = RCF::getTlsRcfSessionPtr();

        // Client side.
        // Append a ref to ourselves the the current ClientStub.
        // After serializing regular parameters, we'll regain control.
        if (pClientStub)
        {
            if (isWriting)
            {
                if (mDirection == Upload)
                {
                    mSessionLocalId = pClientStub->addUploadStream(
                        FileUpload(shared_from_this()));
                }
                else if (mDirection == Download)
                {
                    mSessionLocalId = pClientStub->addDownloadStream(
                        FileDownload(shared_from_this()));
                }

                serializeImpl(mSessionLocalId, mDirection);
            }
            else
            {
                // Shouldn't really be in here.
                boost::uint32_t sessionLocalId = 0;
                Direction dir = Unspecified;
                serializeImpl(sessionLocalId, dir);
            }
        }
        else if (pSession)
        {
            if (isWriting)
            {
                // Shouldn't really be in here.
                boost::uint32_t sessionLocalId = 0;
                Direction dir = Unspecified;
                serializeImpl(sessionLocalId, dir);
            }
            else
            {
                serializeImpl(mSessionLocalId, mDirection);

                if (mDirection == Upload && mSessionLocalId)
                {
                    RcfSession::SessionUploads::iterator iter = 
                        pSession->mSessionUploads.find(mSessionLocalId);

                    if (iter != pSession->mSessionUploads.end())
                    {
                        FileUploadInfoPtr uploadPtr = iter->second;
                        mManifest = uploadPtr->mManifest;
                        mManifest.mManifestBase = uploadPtr->mUploadPath;

                        pSession->mSessionUploads.erase(iter);
                    }
                    else
                    {
                        // Couldn't find the upload.
                    }
                }
                else if (mDirection == Download && mSessionLocalId)
                {
                    pSession->addDownloadStream(
                        mSessionLocalId,
                        FileDownload(shared_from_this()));
                }
            }
        }
    }

    // FileUpload

    FileUpload::FileUpload()
    {
        mImplPtr->mDirection = FileStreamImpl::Upload;
    }

    FileUpload::FileUpload(const std::string & filePath) : FileStream(filePath)
    {
        mImplPtr->mDirection = FileStreamImpl::Upload;
    }

    FileUpload::FileUpload(const FileManifest & manifest) : FileStream(manifest)
    {
        mImplPtr->mDirection = FileStreamImpl::Upload;
    }

    FileUpload::FileUpload(FileStreamImplPtr implPtr) : FileStream(implPtr)
    {
        mImplPtr->mDirection = FileStreamImpl::Upload;
    }

    // FileDownload

    FileDownload::FileDownload()
    {
        mImplPtr->mDirection = FileStreamImpl::Download;
    }

    FileDownload::FileDownload(const std::string & filePath) : FileStream(filePath)
    {
        mImplPtr->mDirection = FileStreamImpl::Download;
    }

    FileDownload::FileDownload(const FileManifest & manifest) : FileStream(manifest)
    {
        mImplPtr->mDirection = FileStreamImpl::Download;
    }

    FileDownload::FileDownload(FileStreamImplPtr implPtr) : FileStream(implPtr)
    {
        mImplPtr->mDirection = FileStreamImpl::Download;
    }

    // ClientStub

    boost::uint32_t ClientStub::addUploadStream(FileUpload fileStream)
    {
        mUploadStreams.push_back(fileStream);
        return static_cast<boost::uint32_t>(mUploadStreams.size());
    }

    void ClientStub::processUploadStreams()
    {
        ThreadLocalCached< std::vector<FileUpload> > tlcFileStreams;
        std::vector<FileUpload> & fileStreams = tlcFileStreams.get();
        RCF_ASSERT(fileStreams.empty());
        fileStreams.swap(mUploadStreams);
        for (std::size_t i=0; i<fileStreams.size(); ++i)
        {
            fileStreams[i].upload(*this);
        }
    }

    boost::uint32_t ClientStub::addDownloadStream(FileDownload fileStream)
    {
        mDownloadStreams.push_back(fileStream);
        return static_cast<boost::uint32_t>(mDownloadStreams.size());
    }

    void ClientStub::setFileProgressCallback(FileProgressCb fileProgressCb)
    {
        mFileProgressCb = fileProgressCb;
    }

    void ClientStub::setTransferWindowS(boost::uint32_t transferWindowS)
    {
        mTransferWindowS = transferWindowS;
    }

    boost::uint32_t ClientStub::getTransferWindowS()
    {
        return mTransferWindowS;
    }

    // TODO: resuming of failed transfers, in either direction.

    boost::uint32_t calculateEffectiveBps(
        boost::uint32_t serverBps, 
        boost::uint32_t clientBps)
    {
        boost::uint32_t effectiveBps = 0;
        if (serverBps == 0)
        {
            effectiveBps = clientBps;
        }
        else if (clientBps == 0)
        {
            effectiveBps = serverBps;
        }
        else
        {
            effectiveBps = (std::min)(serverBps, clientBps);
        }
        return effectiveBps;
    }

    void ClientStub::uploadFiles(
        const std::string & whichFile,
        std::string & uploadId,
        boost::uint32_t chunkSize,
        boost::uint32_t transferRateBps,
        boost::uint32_t sessionLocalId)
    {
        RCF::FileManifest manifest(whichFile);
        uploadFiles(manifest, uploadId, chunkSize, transferRateBps, sessionLocalId);
    }

    void ClientStub::uploadFiles(
        const FileManifest & manifest,
        std::string & uploadId,
        boost::uint32_t chunkSize,
        boost::uint32_t transferRateBps,
        boost::uint32_t sessionLocalId)
    {
        RCF_LOG_3()(manifest.mFiles.size())(chunkSize)(sessionLocalId) 
            << "ClientStub::uploadFiles() - entry.";

        ClientStub & clientStub = *this;

        if (! clientStub.isConnected())
        {
            clientStub.connect();
        }

        RcfClient<I_FileTransferService> ftsClient(clientStub);
        ftsClient.getClientStub().setTransport( clientStub.releaseTransport() );
        ftsClient.getClientStub().setTargetToken( Token());

        RestoreClientTransportGuard guard(clientStub, ftsClient.getClientStub());
        RCF_UNUSED_VARIABLE(guard);

        // 1) Send manifest to server, and an optimistic first chunk. 
        // 2) Server replies, with index, pos and CRC of next chunk to transfer.
        // --> CRC only passed if pos != chunk length
        // 3) Client goes into a loop, sending chunks until all files are transferred.

        namespace fs = boost::filesystem;

        fs::path manifestBase = manifest.mManifestBase;

        FileChunk startPos;
        boost::uint32_t maxMessageLength    = 0;
        boost::uint32_t clientBps           = transferRateBps;
        boost::uint32_t serverBps           = 0;
        boost::uint32_t effectiveBps        = 0;

        RCF_LOG_3()(manifest.mFiles.size())(chunkSize)(sessionLocalId) 
            << "ClientStub::uploadFiles() - calling BeginUpload().";

        ftsClient.BeginUpload(
            manifest, 
            std::vector<FileChunk>(), 
            startPos, 
            maxMessageLength,
            uploadId,
            serverBps,
            sessionLocalId);

        RCF_LOG_3()(startPos.mFileIndex)(startPos.mOffset)(maxMessageLength)(uploadId)(serverBps) 
            << "ClientStub::uploadFiles() - BeginUpload() returned.";
        
        boost::uint64_t totalByteSize = manifest.getTotalByteSize();

        boost::uint64_t totalBytesUploadedSoFar = 0;
        for (std::size_t i=0; i<startPos.mFileIndex; ++i)
        {
            totalBytesUploadedSoFar += manifest.mFiles[i].mFileSize;
        }
        totalBytesUploadedSoFar += startPos.mOffset;

        // Progress callback.
        FileTransferProgress progressInfo;
        progressInfo.mBytesTotalToTransfer = totalByteSize;
        progressInfo.mBytesTransferredSoFar = totalBytesUploadedSoFar;
        progressInfo.mServerLimitBps = serverBps;
        if (mFileProgressCb)
        {
            mFileProgressCb(progressInfo);
        }
        
        boost::uint32_t firstFile = startPos.mFileIndex;
        boost::uint64_t firstPos = startPos.mOffset;

        // Limit the chunk size to 80 % of max message length.
        if (maxMessageLength == 0)
        {
            chunkSize = 1024*1024;
        }
        else
        {
            chunkSize = RCF_MIN(chunkSize, maxMessageLength*8/10);
        }

        effectiveBps = calculateEffectiveBps(serverBps, clientBps);
        
        Timer windowTimer;
        boost::uint32_t windowBytesTotal    = mTransferWindowS*effectiveBps;
        boost::uint32_t windowBytesSoFar    = 0;

        std::vector<FileChunk> chunks;

        // Async file reading.
        FileIoRequestPtr readOp( new FileIoRequest() );
        ByteBuffer bufferRead(chunkSize);
        ByteBuffer bufferSend(chunkSize);

        std::size_t bufferReadPos = 0;

        std::size_t currentFile = firstFile;

        // Skip zero-size entries.
        while (     currentFile < manifest.mFiles.size() 
            &&  manifest.mFiles[currentFile].mFileSize == 0)
        {
            ++currentFile;
        }

        while (currentFile != manifest.mFiles.size())
        {
            const FileInfo & info = manifest.mFiles[currentFile];
            fs::path filePath = manifestBase / info.mFilePath;

            // Upload chunks to the server until we're done.
            
            RCF_LOG_3()(filePath)
                << "ClientStub::uploadFiles() - opening file.";

            IfstreamPtr fin( new std::ifstream(
                filePath.string().c_str(), 
                std::ios::binary));

            RCF_VERIFY(*fin, Exception(_RcfError_FileOpen(nativeString(filePath))));

            boost::uint64_t filePos = 0;        
            if (currentFile == firstFile)
            {
                filePos = firstPos;

                RCF_LOG_3()(filePos)
                    << "ClientStub::uploadFiles() - seeking in file.";

                fin->seekg( static_cast<std::streamoff>(filePos) );
            }

            readOp->complete();

            const boost::uint64_t FileSize = fs::file_size(filePath);
            while (filePos < FileSize)
            {
                std::size_t bytesRead = 0;

                bool shouldSend = false;
                bool shouldReadAhead = true;

                // Wait for current read to complete.
                if (readOp->isInitiated())
                {
                    readOp->complete();
                    
                    bytesRead = static_cast<std::size_t>(
                        readOp->getBytesTransferred());

                    RCF_LOG_3()(bytesRead)
                        << "ClientStub::uploadFiles() - completing read from file.";
                    
                    RCF_VERIFY(
                        bytesRead > 0, 
                        Exception(_RcfError_FileRead(nativeString(filePath), filePos)));

                    FileChunk chunk;
                    chunk.mFileIndex = (boost::uint32_t) currentFile;
                    chunk.mOffset = filePos;
                    chunk.mData = ByteBuffer(bufferRead, bufferReadPos, bytesRead);

                    RCF_LOG_3()(chunk.mFileIndex)(chunk.mOffset)(chunk.mData.getLength())
                        << "ClientStub::uploadFiles() - adding chunk.";

                    chunks.push_back( chunk );

                    bufferReadPos += bytesRead;
                    filePos += bytesRead;
                    windowBytesSoFar += (boost::uint32_t) bytesRead;

                    if (bufferReadPos == bufferRead.getLength())
                    {
                        shouldSend = true;
                    }

                    if (effectiveBps && windowBytesSoFar >= windowBytesTotal)
                    {
                        shouldSend = true;
                    }

                    if (filePos == FileSize)
                    {
                        ++currentFile;

                        // Skip zero-size entries.
                        while (     currentFile < manifest.mFiles.size() 
                                &&  manifest.mFiles[currentFile].mFileSize == 0)
                        {
                            ++currentFile;
                        }

                        // Have we done all the files?
                        if (currentFile == manifest.mFiles.size())
                        {
                            shouldSend = true;
                        }

                        // No read ahead if we are transitioning from one file to another.
                        shouldReadAhead = false;
                    }

                    if (shouldSend)
                    {
                        bufferSend.swap(bufferRead);
                        bufferReadPos = 0;
                    }
                }
                
                // Read ahead.
                if (shouldReadAhead)
                {
                    RCF_LOG_3()(bufferRead.getLength())(bufferReadPos)
                        << "ClientStub::uploadFiles() - initiating read from file.";

                    RCF_ASSERT_LT(bufferReadPos, bufferRead.getLength());

                    std::size_t bytesToRead = 
                        static_cast<std::size_t>(bufferRead.getLength() - bufferReadPos);

                    // Trim to throttle settings.
                    if (effectiveBps)
                    {
                        if (windowBytesSoFar < windowBytesTotal)
                        {
                            boost::uint32_t windowBytesRemaining = 
                                windowBytesTotal - windowBytesSoFar;

                            bytesToRead = RCF_MIN(
                                static_cast<boost::uint32_t>(bytesToRead), 
                                windowBytesRemaining);
                        }
                        else
                        {
                            bytesToRead = 0;
                        }

                        RCF_LOG_3()(bytesToRead)(windowBytesSoFar)(windowBytesTotal)
                            << "ClientStub::uploadFiles() - trimming chunk size to throttle setting.";
                    }

                    if (bytesToRead)
                    {
                        readOp->initiateRead(fin, ByteBuffer(bufferRead, bufferReadPos, bytesToRead));
                    }
                }

                // Upload accumulated file chunks.
                if (shouldSend)
                {
                    RCF_LOG_3()(chunks.size())
                        << "ClientStub::uploadFiles() - calling UploadChunks().";

                    ftsClient.UploadChunks(chunks, serverBps);

                    effectiveBps = calculateEffectiveBps(serverBps, clientBps);

                    RCF_LOG_3()(serverBps)
                        << "ClientStub::uploadFiles() - UploadChunks() returned.";

                    std::size_t bytesSent = 0;
                    for (std::size_t i=0; i<chunks.size(); ++i)
                    {
                        bytesSent += chunks[i].mData.getLength();
                    }
                    totalBytesUploadedSoFar += bytesSent;

                    chunks.clear();

                    // Progress callback.
                    progressInfo.mBytesTotalToTransfer = totalByteSize;
                    progressInfo.mBytesTransferredSoFar = totalBytesUploadedSoFar;
                    progressInfo.mServerLimitBps = serverBps;
                    if (mFileProgressCb)
                    {
                        mFileProgressCb(progressInfo);
                    }

                    if (effectiveBps)
                    {
                        // Recalculate window size based on possibly updated bps setting.
                        windowBytesTotal = mTransferWindowS * effectiveBps;
                        if (windowBytesSoFar >= windowBytesTotal)
                        {
                            RCF_LOG_3()(windowBytesSoFar)(windowBytesTotal)
                                << "ClientStub::uploadFiles() - window capacity reached.";

                            // Exceeded window capacity. Wait for window to expire.
                            boost::uint32_t windowMsSoFar = windowTimer.getDurationMs();
                            if (windowMsSoFar < mTransferWindowS*1000)
                            {
                                boost::uint32_t waitMs = mTransferWindowS*1000 - windowMsSoFar;

                                RCF_LOG_3()(waitMs)
                                    << "ClientStub::uploadFiles() - waiting for next window.";

                                sleepMs(waitMs);
                                while (!windowTimer.elapsed(mTransferWindowS*1000))
                                {
                                    sleepMs(100);
                                }
                            }
                        }

                        // If window has expired, open a new one.
                        if (windowTimer.elapsed(mTransferWindowS*1000))
                        {
                            windowTimer.restart();

                            // Carry over balance from previous window.
                            if (windowBytesSoFar > windowBytesTotal)
                            {
                                windowBytesSoFar = windowBytesSoFar - windowBytesTotal;
                            }
                            else
                            {
                                windowBytesSoFar = 0;
                            }

                            RCF_LOG_3()(mTransferWindowS)(windowBytesSoFar)
                                << "ClientStub::uploadFiles() - new transfer window.";
                        }
                    }
                }
            }
        }
        RCF_LOG_3()(totalByteSize)(totalBytesUploadedSoFar) 
            << "ClientStub::uploadFiles() - exit.";
    }

    void trimManifest(
        const FileManifest & manifest, 
        boost::uint64_t & bytesAlreadyTransferred,
        FileChunk & startPos)
    {
        fs::path manifestBase = manifest.mManifestBase;
        RCF_ASSERT(!manifestBase.empty());

        bytesAlreadyTransferred = 0;
        startPos = FileChunk();

        std::size_t whichFile=0;
        boost::uint64_t offset = 0;

        for (whichFile=0; whichFile<manifest.mFiles.size(); ++whichFile)
        {
            const FileInfo & fileInfo = manifest.mFiles[whichFile];
            fs::path p = manifestBase / fileInfo.mFilePath;

            if (!fs::exists(p))
            {
                break;
            }

            if (fs::is_directory(p) && fileInfo.mIsDirectory)
            {
                continue;
            }

            if (fs::is_directory(p) && !fileInfo.mIsDirectory)
            {
                break;
            }

            RCF_ASSERT(fs::exists(p) && !fs::is_directory(p));

            boost::uint64_t fileSize = fs::file_size(p);
            
            if (fileSize < fileInfo.mFileSize)
            {
                bytesAlreadyTransferred += fileSize;
                offset = fileSize;
                break;
            }
            
            if (fileSize > fileInfo.mFileSize)
            {
                break;
            }

            RCF_ASSERT_EQ(fileSize , fileInfo.mFileSize);

            bytesAlreadyTransferred += fileSize;
        }

        if (whichFile <= manifest.mFiles.size())
        {
            startPos.mFileIndex = (boost::uint32_t) whichFile;
            startPos.mOffset = offset;
        }
    }

    void processZeroSizeEntries(
        FileManifest & manifest, 
        const fs::path & manifestBase, 
        boost::uint32_t & currentFile)
    {
        while (     currentFile < manifest.mFiles.size()
                &&  manifest.mFiles[currentFile].mFileSize == 0)
        {
            FileInfo & info = manifest.mFiles[currentFile];

            fs::path targetPath = manifestBase / info.mFilePath;
            if (info.mRenameFile.size() > 0)
            {
                targetPath = targetPath.branch_path() / info.mRenameFile;
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

                std::time_t writeTime = info.mLastWriteTime;
                fs::last_write_time(targetPath, writeTime);
            }

            ++currentFile;
        }
    }

    void ClientStub::downloadFiles(
        const std::string & downloadLocation,
        FileManifest & totalManifest,
        boost::uint32_t chunkSize,
        boost::uint32_t transferRateBps,
        boost::uint32_t sessionLocalId)
    {
        RCF_LOG_3()(downloadLocation)(chunkSize)(transferRateBps)(sessionLocalId) 
            << "ClientStub::downloadFiles() - entry.";

        ClientStub & clientStub = *this;

        if (! clientStub.isConnected())
        {
            clientStub.connect();
        }

        RCF::RcfClient<RCF::I_FileTransferService> ftsClient(clientStub);
        ftsClient.getClientStub().setTransport( clientStub.releaseTransport() );
        ftsClient.getClientStub().setTargetToken( Token());

        // Disable incoming max message length, as we can't say how big the
        // the manifest is going to be.
        ftsClient.getClientStub().getTransport().setMaxMessageLength(0);

        RestoreClientTransportGuard guard(clientStub, ftsClient.getClientStub());
        RCF_UNUSED_VARIABLE(guard);

        // Download chunks from the server until we're done.

        // TODO: optional first chunks.
        FileManifest manifest;
        FileTransferRequest request;
        std::vector<FileChunk> chunks;
        boost::uint32_t serverMaxMessageLength = 0;
        boost::uint32_t serverBps = 0;

        RCF_LOG_3()(downloadLocation)(chunkSize)(transferRateBps)(sessionLocalId) 
            << "ClientStub::downloadFiles() - calling BeginDownload().";

        ftsClient.BeginDownload(
            manifest, 
            request, 
            chunks, 
            serverMaxMessageLength, 
            serverBps,
            sessionLocalId);

        boost::uint32_t clientMaxMessageLength = static_cast<boost::uint32_t>(
            getTransport().getMaxMessageLength());

        RCF_LOG_3()(manifest.mFiles.size())(serverMaxMessageLength)(clientMaxMessageLength)(serverBps)
            << "ClientStub::downloadFiles() - BeginDownload() returned.";

        chunkSize = RCF_MIN(chunkSize, serverMaxMessageLength*8/10);
        chunkSize = RCF_MIN(chunkSize, clientMaxMessageLength*8/10);

        fs::path manifestBase = downloadLocation;

        boost::uint32_t currentFile = 0;
        boost::uint64_t currentPos = 0;

        // See if we have any fragments already downloaded.
        bool resume = false;
        manifest.mManifestBase = downloadLocation;
        boost::uint64_t bytesAlreadyTransferred = 0;
        FileChunk startPos;
        trimManifest(manifest, bytesAlreadyTransferred, startPos);
        if (bytesAlreadyTransferred)
        {
            RCF_LOG_3()(startPos.mFileIndex)(startPos.mOffset)
                << "ClientStub::downloadFiles() - calling TrimDownload().";

            ftsClient.TrimDownload(startPos);

            RCF_LOG_3()(startPos.mFileIndex)(startPos.mOffset)
                << "ClientStub::downloadFiles() - TrimDownload() returned.";

            currentFile = startPos.mFileIndex;
            currentPos = startPos.mOffset;
            resume = true;
        }

        OfstreamPtr fout( new std::ofstream() );
        FileIoRequestPtr writeOp( new FileIoRequest() );
        
        boost::uint32_t adviseWaitMs = 0;
        
        // Calculate total byte count of the manifest.
        boost::uint64_t totalByteSize = manifest.getTotalByteSize();

        // Did we get any chunks on the BeginDownload() call?
        boost::uint64_t totalBytesReadSoFar = bytesAlreadyTransferred;
        RCF_ASSERT(chunks.empty());
        for (std::size_t i=0; i<chunks.size(); ++i)
        {
            totalBytesReadSoFar += chunks[i].mData.getLength();
        }

        // Progress callback.
        FileTransferProgress progressInfo;
        progressInfo.mBytesTotalToTransfer = totalByteSize;
        progressInfo.mBytesTransferredSoFar = totalBytesReadSoFar;
        progressInfo.mServerLimitBps = serverBps;
        if (mFileProgressCb)
        {
            mFileProgressCb(progressInfo);
        }

        const boost::uint32_t TransferWindowS = 5;
        RCF::Timer transferWindowTimer;
        boost::uint32_t transferWindowBytes = 0;
        bool localWait = false;

        processZeroSizeEntries(manifest, manifestBase, currentFile);

        while (currentFile != manifest.mFiles.size())
        {
            RCF_ASSERT(chunks.empty() || (currentFile == 0 && currentPos == 0));

            // Round trip to the server for more chunks.
            if (chunks.empty())
            {
                FileTransferRequest request;
                request.mFile = currentFile;
                request.mPos = currentPos;
                request.mChunkSize = chunkSize;

                // Respect server throttle settings.
                if (adviseWaitMs)
                {
                    RCF_LOG_3()(adviseWaitMs)
                        << "ClientStub::downloadFiles() - waiting on server throttle.";

                    // This needs to be a sub-second accurate sleep, or the timing tests will
                    // be thrown.
                    sleepMs(adviseWaitMs);
                    adviseWaitMs = 0;
                }

                // Respect local throttle setting.
                if (localWait)
                {
                    boost::uint32_t startTimeMs = transferWindowTimer.getStartTimeMs();
                    boost::uint32_t nowMs = getCurrentTimeMs();
                    if (nowMs < startTimeMs + 1000*TransferWindowS)
                    {
                        boost::uint32_t waitMs = startTimeMs + 1000*TransferWindowS - nowMs;

                        RCF_LOG_3()(waitMs)(mTransferWindowS)
                            << "ClientStub::downloadFiles() - waiting on client throttle.";
                        
                        // Existing behavior - if we're going to wait, wait at least a second.
                        waitMs = RCF_MAX(boost::uint32_t(1000), waitMs);
                        RCF::sleepMs(waitMs);
                        localWait = false;
                    }
                }

                // Trim chunk size according to transfer rate.                
                if (transferRateBps)
                {
                    if (transferWindowTimer.elapsed(TransferWindowS*1000))
                    {
                        transferWindowTimer.restart();
                        transferWindowBytes = 0;
                    }

                    boost::uint32_t bytesTotal = transferRateBps * TransferWindowS;
                    boost::uint32_t bytesRemaining =  bytesTotal - transferWindowBytes;

                    if (bytesRemaining < request.mChunkSize)
                    {
                        localWait = true;
                    }

                    RCF_LOG_3()(request.mChunkSize)(bytesRemaining)
                        << "ClientStub::downloadFiles() - trimming request chunk size to client throttle.";

                    request.mChunkSize = RCF_MIN(request.mChunkSize, bytesRemaining);
                }

                RCF_LOG_3()(request.mFile)(request.mPos)(request.mChunkSize)(startPos.mOffset)
                    << "ClientStub::downloadFiles() - calling DownloadChunks().";

                ftsClient.DownloadChunks(request, chunks, adviseWaitMs, serverBps);

                RCF_LOG_3()(chunks.size())(adviseWaitMs)(serverBps)
                    << "ClientStub::downloadFiles() - DownloadChunks() returned.";

                // Update byte totals.
                for (std::size_t i=0; i<chunks.size(); ++i)
                {
                    totalBytesReadSoFar += chunks[i].mData.getLength();
                    transferWindowBytes += (boost::uint32_t) chunks[i].mData.getLength();
                }

                // Progress callback.
                progressInfo.mBytesTotalToTransfer = totalByteSize;
                progressInfo.mBytesTransferredSoFar = totalBytesReadSoFar;
                progressInfo.mServerLimitBps = serverBps;
                if (mFileProgressCb)
                {
                    mFileProgressCb(progressInfo);
                }
            }

            // Write to disk.
            for (std::size_t i=0; i<chunks.size(); ++i)
            {
                const FileChunk & chunk = chunks[i];

                if (chunk.mFileIndex != currentFile || chunk.mOffset != currentPos)
                {
                    // TODO: error
                    RCF_ASSERT(0);
                }

                FileInfo & info = manifest.mFiles[currentFile];

                fs::path filePath = manifestBase / info.mFilePath;
                if (info.mRenameFile.size() > 0)
                {
                    filePath = filePath.branch_path() / info.mRenameFile;
                }

                if (currentPos == 0 || resume)
                {
                    RCF_ASSERT(writeOp->isCompleted());

                    if (currentPos == 0)
                    {
                        // Create new file.
                        RCF_LOG_3()(filePath)
                            << "ClientStub::downloadFiles() - opening file (truncating).";

                        fs::create_directories(filePath.branch_path());
                        fout->clear();
                        fout->open( filePath.string().c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
                        RCF_VERIFY(fout->good(), Exception(_RcfError_FileOpen(nativeString(filePath))));
                    }
                    else
                    {
                        // Create new file.
                        RCF_LOG_3()(filePath)
                            << "ClientStub::downloadFiles() - opening file (appending).";

                        // Append to existing file.
                        fout->clear();
                        fout->open( filePath.string().c_str(), std::ios::out | std::ios::binary | std::ios::app | std::ios::ate);
                        RCF_VERIFY(fout->good(), Exception(_RcfError_FileOpen(nativeString(filePath))));
                        resume = false;

                        // TODO: verify file position against currentPos.
                        // ...
                    }
                }

                // Wait for previous write to complete.
                if (writeOp->isInitiated())
                {
                    writeOp->complete();
                    boost::uint64_t bytesWritten = writeOp->getBytesTransferred();

                    RCF_LOG_3()(bytesWritten)
                        << "ClientStub::downloadFiles() - file write completed.";
                    
                    if (bytesWritten == 0)
                    {
                        fout->close();
                        
                        std::time_t writeTime = info.mLastWriteTime;
                        fs::last_write_time(filePath, writeTime);

                        Exception e(_RcfError_FileWrite(nativeString(filePath), currentPos));
                        RCF_THROW(e);
                    }
                }

                if (currentPos + chunk.mData.getLength() > manifest.mFiles[chunk.mFileIndex].mFileSize)
                {
                    // TODO: error
                    RCF_ASSERT(0);
                }

                // Check stream state.
                if (!fout->good())
                {
                    Exception e(_RcfError_FileWrite(nativeString(filePath), currentPos));
                    RCF_THROW(e);
                }

                RCF_ASSERT_EQ(currentPos , (boost::uint64_t) fout->tellp());

                RCF_LOG_3()(chunk.mData.getLength())
                    << "ClientStub::downloadFiles() - file write initiated.";

                writeOp->initateWrite(fout, chunk.mData);

                currentPos += chunk.mData.getLength();

                if (currentPos == manifest.mFiles[currentFile].mFileSize)
                {
                    writeOp->complete();
                    boost::uint64_t bytesWritten = writeOp->getBytesTransferred();
                    
                    RCF_LOG_3()(bytesWritten)
                        << "ClientStub::downloadFiles() - file write completed.";

                    RCF_LOG_3()(currentFile)
                        << "ClientStub::downloadFiles() - closing file.";

                    
                    if (bytesWritten == 0)
                    {
                        fout->close();

                        std::time_t writeTime = info.mLastWriteTime;
                        fs::last_write_time(filePath, writeTime);

                        Exception e(_RcfError_FileWrite(nativeString(filePath), currentPos));
                        RCF_THROW(e);
                    }

                    fout->close();
                    
                    std::time_t writeTime = info.mLastWriteTime;
                    fs::last_write_time(filePath, writeTime);

                    currentPos = 0;
                    ++currentFile;

                    processZeroSizeEntries(manifest, manifestBase, currentFile);

                    if (currentFile < manifest.mFiles.size())
                    {
                        currentPos = manifest.mFiles[currentFile].mFileStartPos;
                    }
                }
            }

            chunks.clear();
        }

        totalManifest = manifest;

        RCF_LOG_3()(totalManifest.mFiles.size())
            << "ClientStub::downloadFiles() - exit.";

    }
} // namespace RCF
