
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

#include <RCF/Certificate.hpp>

#ifdef BOOST_WINDOWS
#include <RCF/Win32Certificate.hpp>
#endif

#if RCF_FEATURE_OPENSSL==1
#include <RCF/OpenSslEncryptionFilter.hpp>
#endif


namespace RCF {

    CertificateImplementationType Certificate::_getType()
    {
        return Cit_Unspecified;
    }

#ifdef BOOST_WINDOWS
    Win32CertificatePtr Certificate::_downcastToWin32Certificate(CertificatePtr certPtr)
    {
        return boost::dynamic_pointer_cast<Win32Certificate>(certPtr);
    }
#else
    Win32CertificatePtr Certificate::_downcastToWin32Certificate(CertificatePtr certPtr)
    {
        return Win32CertificatePtr();
    }
#endif

#if RCF_FEATURE_OPENSSL==1
    X509CertificatePtr Certificate::_downcastToX509Certificate(CertificatePtr certPtr)
    {
        return boost::dynamic_pointer_cast<X509Certificate>(certPtr);
    }
#else
    X509CertificatePtr Certificate::_downcastToX509Certificate(CertificatePtr certPtr)
    {
        return X509CertificatePtr();
    }
#endif

} // namespace RCF
