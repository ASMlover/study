
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

#include <RCF/Enums.hpp>

#include <RCF/Tools.hpp>

namespace RCF {

    std::string getTransportProtocolName(TransportProtocol protocol)
    {
        switch (protocol)
        {
        case Tp_Clear               :   return "Clear";
        case Tp_Ntlm                :   return "NTLM";
        case Tp_Kerberos            :   return "Kerberos";
        case Tp_Negotiate           :   return "SSPI Negotiate";
        case Tp_Ssl                 :   return "SSL";
        default                     :   RCF_ASSERT(0); return "Unknown";
        }
    }

    std::string getTransportTypeName(TransportType transportType)
    {
        switch (transportType)
        {
        case Tt_Unknown                 :   return "Unknown";
        case Tt_Tcp                     :   return "TCP";
        case Tt_Udp                     :   return "UDP";
        case Tt_Win32NamedPipe          :   return "Win32 named pipe";
        case Tt_UnixNamedPipe           :   return "Unix local socket";
        case Tt_Http                    :   return "HTTP";
        case Tt_Https                   :   return "HTTPS";
        default                         :   RCF_ASSERT(0); return "Unknown";
        }
    }

} // namespace RCF
