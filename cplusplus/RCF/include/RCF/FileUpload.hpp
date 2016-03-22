
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

#ifndef INCLUDE_RCF_FILEUPLOAD_HPP
#define INCLUDE_RCF_FILEUPLOAD_HPP

#include <RCF/FileStream.hpp>

namespace RCF {

    class RCF_EXPORT FileUpload : public FileStream
    {
    public:
        FileUpload();
        FileUpload(const std::string & filePath);
        FileUpload(const FileManifest & manifest);

    private:
        
        friend class FileStreamImpl;

        FileUpload(FileStreamImplPtr implPtr);
    };

}

#endif // ! INCLUDE_RCF_FILEUPLOAD_HPP
