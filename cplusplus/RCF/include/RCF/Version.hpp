
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

#ifndef INCLUDE_RCF_VERSION_HPP
#define INCLUDE_RCF_VERSION_HPP

#include <RCF/BuildVersion.hpp>
#include <RCF/Export.hpp>

#include <boost/cstdint.hpp>

namespace RCF {

    // Runtime versioning.

    // legacy       - version number 1

    // 2007-04-26   - version number 2
    // Released in 0.9c

    // 2008-03-29   - version number 3
    //      - Using I_SessionObjectFactory instead of I_ObjectFactoryService for session object creation and deletion.
    // Released in 0.9d

    // 2008-09-06   - version number 4
    //      - ByteBuffer compatible with std::vector etc.
    // Released in 1.0

    // 2008-12-06   - version number 5
    //      - Pingback field in MethodInvocationRequest
    // Released in 1.1

    // 2010-01-21   - version number 6
    //      - Archive version field in MethodInvocationRequest
    //      - Embedded version stamps in SF archives.
    //      - SF: Serialization of error arguments in RemoteException.
    // Released in 1.2

    // 2010-03-20   - version number 7
    //      - User data fields in request and response headers
    // Interim release (rev 1414).

    // 2010-03-30   - version number 8
    //      - Ping intervals between publishers and subscribers.
    //      - Byte reordering for fast vector serialization.
    //      - BSer: Serialization of error arguments in RemoteException.
    //      - Non-polymorphic marshaling of reference parameters
    //      - UTF-8 serialization of wstring (native as an option). Changes to request header.
    //      - BSer: remote exceptions serialized through raw pointer rather than auto_ptr.
    //      - Error response messages contain two custom args, rather than one.
    // Released in 1.3

    // 2011-02-27   - version number 9
    //      - Only do non-polymorphic marshaling of reference parameters, if using SF and object caching is enabled for the marshaling type. Otherwise polymorphic marshaling, as in version 7 and earlier.
    //      - Optimize SF serialization of 32 bit integers < 128, to a single byte.
    // Released in 1.3.1

    // 2012-01-13   - version number 10
    //      - Request and response headers include pointer tracking setting for SF archives.
    //      - SF archive metadata includes pointer tracking setting.

    // 2012-09-05   - version number 11
    //      - FileInfo serialization includes last-modified timestamp. 

    // 2013-09-01   - version number 12
    //      - Request and response headers include out of band request and response.
    //      - Request of transport filters done through out of band message.
 

    // Inherent runtime version - can't be changed.
    RCF_EXPORT boost::uint32_t  getLibraryVersion();

    // Default runtime version.
    RCF_EXPORT boost::uint32_t  getDefaultRuntimeVersion();
    RCF_EXPORT void             setDefaultRuntimeVersion(boost::uint32_t version);

    // Default archive version.
    RCF_EXPORT boost::uint32_t  getDefaultArchiveVersion();
    RCF_EXPORT void             setDefaultArchiveVersion(boost::uint32_t version);

} // namespace RCF

#endif // ! INCLUDE_RCF_VERSION_HPP
