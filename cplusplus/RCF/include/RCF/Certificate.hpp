
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

#ifndef INCLUDE_RCF_CERTIFICATE_HPP
#define INCLUDE_RCF_CERTIFICATE_HPP

#include <RCF/Enums.hpp>
#include <RCF/Export.hpp>

#include <boost/shared_ptr.hpp>

namespace RCF {

    class Certificate;
    typedef boost::shared_ptr<Certificate> CertificatePtr;

    class Win32Certificate;
    typedef boost::shared_ptr<Win32Certificate> Win32CertificatePtr;

    class X509Certificate;
    typedef boost::shared_ptr<X509Certificate> X509CertificatePtr;

    /// Base class of all certificate classes.
    class RCF_EXPORT Certificate
    {
    public:

        // *** SWIG BEGIN ***

        virtual CertificateImplementationType _getType();       

        Win32CertificatePtr _downcastToWin32Certificate(CertificatePtr certPtr);
        X509CertificatePtr _downcastToX509Certificate(CertificatePtr certPtr);

        // *** SWIG END ***

        virtual ~Certificate()
        {
        }
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_CERTIFICATE_HPP
