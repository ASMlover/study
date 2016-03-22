
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

#ifndef INCLUDE_RCF_FILEDOWNLOAD_HPP
#define INCLUDE_RCF_FILEDOWNLOAD_HPP

#include <RCF/FileStream.hpp>

namespace RCF {

    class RCF_EXPORT FileDownload : public FileStream
    {
    public:
        FileDownload();

        // Server-side constructors.
        FileDownload(const std::string & fileDownloadPath);
        FileDownload(const FileManifest & manifest);

    private:

        friend class FileStreamImpl;

        FileDownload(FileStreamImplPtr implPtr);
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_FILEDOWNLOAD_HPP
