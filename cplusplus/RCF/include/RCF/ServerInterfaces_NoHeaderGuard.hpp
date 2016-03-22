
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

#include <RCF/Config.hpp>

#if RCF_FEATURE_LEGACY==0
#error This header is only supported in builds with RCF_FEATURE_LEGACY=1.
#endif

#include <RCF/Idl.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/TypeTraits.hpp>

#if RCF_FEATURE_SF==1
#include <SF/string.hpp>
#include <SF/vector.hpp>
#endif

#if RCF_FEATURE_BOOST_SERIALIZATION==1
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#endif

#if RCF_FEATURE_FILETRANSFER==1
#include <RCF/FileTransferService.hpp>
#include <RCF/FileStream.hpp>
#endif

namespace RCF {

    //--------------------------------------------------------------------------
    // I_Null

    RCF_BEGIN(I_Null, "")
    RCF_END(I_Null)
   

    //--------------------------------------------------------------------------
    // I_ObjectFactory

    RCF_BEGIN(I_ObjectFactory, "")

        RCF_METHOD_R2(
            boost::int32_t, 
                CreateObject, 
                    const std::string &, 
                    Token &)

        // Deprecated (use I_SessionObjectFactory)
        RCF_METHOD_R1(
            boost::int32_t, 
                CreateSessionObject, 
                    const std::string &)

        RCF_METHOD_R1(
            boost::int32_t, 
                DeleteObject, 
                    const Token &)

        // Deprecated (use I_SessionObjectFactory)
        RCF_METHOD_R0(
            boost::int32_t, 
                DeleteSessionObject)

    RCF_END(I_ObjectFactory)


    //--------------------------------------------------------------------------
    // I_SessionObjectFactory

    RCF_BEGIN(I_SessionObjectFactory, "")

        RCF_METHOD_R1(
            boost::int32_t, 
                CreateSessionObject, 
                    const std::string &)

        RCF_METHOD_R0(
            boost::int32_t, 
                DeleteSessionObject)

    RCF_END(I_SessionObjectFactory)
   

    //--------------------------------------------------------------------------
    // I_EndpointBroker

    RCF_BEGIN(I_EndpointBroker, "")

        RCF_METHOD_R3(
            boost::int32_t, 
                OpenEndpoint, 
                    const std::string &, 
                    const std::string &, 
                    std::string &)

        RCF_METHOD_R2(
            boost::int32_t, 
                CloseEndpoint, 
                    const std::string &, 
                    const std::string &)

        RCF_METHOD_R2(
            boost::int32_t, 
                EstablishEndpointConnection, 
                    const std::string &, 
                    const std::string &)

        RCF_METHOD_R2(
            boost::int32_t, 
                ConnectToEndpoint, 
                    const std::string &, 
                    const std::string &)

    RCF_END(I_EndpointBroker)
   

    //--------------------------------------------------------------------------
    // I_EndpointServer

    RCF_BEGIN(I_EndpointServer, "")

        RCF_METHOD_V1(
            void, 
                SpawnConnections, 
                    boost::uint32_t)

    RCF_END(I_EndpointServer)
   

    //--------------------------------------------------------------------------
    // I_RequestSubscription

    RCF_BEGIN( I_RequestSubscription, "" )

        RCF_METHOD_R1(
            boost::int32_t, 
                RequestSubscription, 
                    const std::string &)

        RCF_METHOD_R3(
            boost::int32_t, 
                RequestSubscription, 
                    const std::string &,    // subscriber name
                    boost::uint32_t,        // sub-to-pub ping interval
                    boost::uint32_t &)      // pub-to-sub ping interval

    RCF_END(I_RequestSubscription)


    //--------------------------------------------------------------------------
    // I_RequestTransportFilters

    RCF_BEGIN(I_RequestTransportFilters, "")

        RCF_METHOD_R1(
            boost::int32_t, 
                RequestTransportFilters, 
                    const std::vector<boost::int32_t> &)

        // No longer supported but still present to improve error message.
        RCF_METHOD_R1(
            boost::int32_t, 
                QueryForTransportFilters, 
                    const std::vector<boost::int32_t> &);

    RCF_END(I_RequestTransportFilters)


    //--------------------------------------------------------------------------
    // I_CreateCallback

    RCF_BEGIN(I_CreateCallbackConnection, "")

        RCF_METHOD_V0(
            void, 
                CreateCallbackConnection)

    RCF_END(I_CreateCallbackConnection)

    //--------------------------------------------------------------------------
    // I_ParentToChild

    RCF_BEGIN(I_ParentToChild, "I_ParentToChild")

        RCF_METHOD_V0(
            void, 
                Ping)
    
    RCF_END(I_ParentToChild)

} // namespace RCF

#if RCF_FEATURE_FILETRANSFER==1

namespace RCF {

    //--------------------------------------------------------------------------
    // I_FileTransferService

    RCF_BEGIN(I_FileTransferService, "I_FileTransferService")

        RCF_METHOD_V7(
            void,
                BeginUpload,
                    const FileManifest &,           // upload manifest
                    const std::vector<FileChunk> &, // optional first chunks
                    FileChunk &,                    // where to start uploading
                    boost::uint32_t &,              // max message length
                    std::string &,                  // upload id
                    boost::uint32_t &,              // bps
                    boost::uint32_t)                // session local id
        
        RCF_METHOD_V2(
            void,
                UploadChunks, 
                    const std::vector<FileChunk> &, // file chunks to upload
                    boost::uint32_t &)              // bps

        RCF_METHOD_V6(
            void,
                BeginDownload,
                    FileManifest &,                 // download manifest
                    const FileTransferRequest &,    // transfer request
                    std::vector<FileChunk> &,       // optional first chunks
                    boost::uint32_t &,              // max message length
                    boost::uint32_t &,              // bps
                    boost::uint32_t)                // session local id

        RCF_METHOD_V1(
            void,
                TrimDownload,
                    const FileChunk &)              // where to start downloading

        RCF_METHOD_V4(
            void,
                DownloadChunks,
                    const FileTransferRequest &,    // transfer request
                    std::vector<FileChunk> &,       // file chunks to download
                    boost::uint32_t &,              // advised wait for next call
                    boost::uint32_t &)              // bps

    RCF_END(I_FileTransferService)

} // namespace RCF

#endif // RCF_FEATURE_FILETRANSFER

