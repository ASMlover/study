
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

#include <RCF/Version.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/Exception.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    // Runtime versioning.

    const boost::uint32_t gRuntimeVersionInherent = 12;

    boost::uint32_t gRuntimeVersionDefault = gRuntimeVersionInherent;

    boost::uint32_t getLibraryVersion()
    {
        return gRuntimeVersionInherent;
    }

    boost::uint32_t getDefaultRuntimeVersion()
    {
        return gRuntimeVersionDefault;
    }

    void setDefaultRuntimeVersion(boost::uint32_t version)
    {
        RCF_VERIFY(
            1 <= version && version <= gRuntimeVersionInherent,
            Exception(_RcfError_UnsupportedRuntimeVersion(version, gRuntimeVersionInherent)))
            (version)(gRuntimeVersionInherent);

        gRuntimeVersionDefault = version;
    }

    // Archive versioning.

    boost::uint32_t gArchiveVersion = 0;

    boost::uint32_t getDefaultArchiveVersion()
    {
        return gArchiveVersion;
    }

    void setDefaultArchiveVersion(boost::uint32_t version)
    {
        gArchiveVersion = version;
    }

} // namespace RCF
