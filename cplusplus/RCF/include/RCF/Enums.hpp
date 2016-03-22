
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

#ifndef INCLUDE_RCF_ENUMS_HPP
#define INCLUDE_RCF_ENUMS_HPP

#include <string>

#include <RCF/Export.hpp>

namespace RCF {

    /// Describes the transport types used by a RCF connection.
    enum TransportType
    {
        /// Unknown
        Tt_Unknown,

        /// TCP transport
        Tt_Tcp,

        /// UDP transport
        Tt_Udp,

        /// Win32 named pipe transport
        Tt_Win32NamedPipe,

        /// UNIX local domain socket transport
        Tt_UnixNamedPipe,

        /// HTTP/TCP transport
        Tt_Http,

        /// HTTPS/TCP transport
        Tt_Https,

    };

    /// Describes the transport protocols used by a RCF connection. Transport
    /// protocols are layered on top of the transport type.
    enum TransportProtocol
    {
        /// Unspecified
        Tp_Unspecified,

        /// Clear text
        Tp_Clear,

        /// Windows NTLM
        Tp_Ntlm,

        /// Windows Kerberos
        Tp_Kerberos,

        /// Windows Negotiate (Kerberos or NTLM)
        Tp_Negotiate,

        /// SSL
        Tp_Ssl
    };

    /// Describes which SSL implementation to use.
    enum SslImplementation
    {
        /// Schannel
        Si_Schannel,

        /// OpenSSL
        Si_OpenSsl
    };

    /// Win32 certificate store locations.
    enum Win32CertificateLocation
    {
        /// Current User
        Cl_CurrentUser,

        /// Local machine
        Cl_LocalMachine
    };

    /// Win32 certificate stores.
    enum Win32CertificateStore
    {
        /// Other People
        Cs_AddressBook,

        /// Third-Party Root Certification Authorities
        Cs_AuthRoot,

        /// Intermediate Certification Authorities
        Cs_CertificateAuthority,

        /// Untrusted Certificates
        Cs_Disallowed,

        /// Personal
        Cs_My,

        /// Trusted Root Certification Authorities
        Cs_Root,

        /// Trusted People
        Cs_TrustedPeople,

        /// Trusted Publishers
        Cs_TrustedPublisher
    };


    enum CertificateImplementationType
    {
        Cit_Unspecified,
        Cit_Win32,
        Cit_X509
    };

    enum RemoteCallSemantics
    {
        Oneway,
        Twoway
    };

    enum WireProtocol
    {
        Wp_None,
        Wp_Http,
        Wp_Https
    };

    enum AsyncOpType
    {
        None,
        Wait,
        Connect,
        Write,
        Read
    };

    RCF_EXPORT std::string getTransportProtocolName(TransportProtocol protocol);
    RCF_EXPORT std::string getTransportTypeName(TransportType protocol);

} // namespace RCF

#endif // ! INCLUDE_RCF_ENUMS_HPP
