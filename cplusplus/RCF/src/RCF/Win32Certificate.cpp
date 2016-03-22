
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

#include <RCF/Win32Certificate.hpp>

#include <RCF/Exception.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    // Certificate utility classes.

    PfxCertificate::PfxCertificate(
        const std::string & pathToCert, 
        const RCF::tstring & password,
        const RCF::tstring & certName) : 
            mPfxStore(NULL)
    {
        initFromFile(pathToCert, password, certName);
    }

    PfxCertificate::PfxCertificate(
        ByteBuffer pfxBlob, 
        const tstring & password,
        const tstring & certName) :
            mPfxStore(NULL)
    {
        init(pfxBlob, password, certName);
    }

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996) // 'fopen': This function or variable may be unsafe.
#endif

    void PfxCertificate::initFromFile(
        const std::string & pathToCert, 
        const RCF::tstring & password,
        const RCF::tstring & certName)
    {
        std::size_t fileSize = static_cast<std::size_t>(RCF::fileSize(pathToCert));
        ByteBuffer pfxBlob(fileSize);

        FILE * fp = fopen(pathToCert.c_str(), "rb");
        std::size_t bytesRead = fread(pfxBlob.getPtr(), sizeof(char), pfxBlob.getLength(), fp);
        fclose(fp);
        RCF_ASSERT_EQ(bytesRead , fileSize);
        RCF_UNUSED_VARIABLE(bytesRead);

        init(pfxBlob, password, certName);
    }

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    void PfxCertificate::init(
        ByteBuffer pfxBlob, 
        const tstring & password,
        const tstring & certName)
    {
        CRYPT_DATA_BLOB blob = {0};
        blob.cbData   = static_cast<DWORD>(pfxBlob.getLength());
        blob.pbData   = (BYTE*) pfxBlob.getPtr();

        BOOL recognizedPFX = PFXIsPFXBlob(&blob);
        DWORD dwErr = GetLastError();


        RCF_VERIFY(
            recognizedPFX, 
            Exception(_RcfError_ApiError("PFXIsPFXBlob()"), dwErr));

        std::wstring wPassword = RCF::toWstring(password);

        // For Windows 98, the flag CRYPT_MACHINE_KEYSET is not valid.
        mPfxStore = PFXImportCertStore(
            &blob, 
            wPassword.c_str(),
            CRYPT_MACHINE_KEYSET | CRYPT_EXPORTABLE);

        dwErr = GetLastError();

        RCF_VERIFY(
            mPfxStore, 
            Exception(_RcfError_ApiError("PFXImportCertStore()"), dwErr));

        PCCERT_CONTEXT pCertStore = NULL;

        // Tempting to use CERT_FIND_ANY for the case where there is just a single
        // certificate in the PFX file. However, doing so appears to not load the
        // private key information. So we use CERT_FIND_SUBJECT_STR instead, and
        // require the certificate name to be passed in.
        
        DWORD dwFindType = CERT_FIND_SUBJECT_STR;
        std::wstring wCertName = RCF::toWstring(certName);
        const void * pvFindPara = wCertName.c_str();

        pCertStore = CertFindCertificateInStore(
            mPfxStore, 
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
            0,
            dwFindType,
            pvFindPara,
            pCertStore);

        dwErr = GetLastError();

        RCF_VERIFY(
            pCertStore, 
            Exception(_RcfError_ApiError("CertFindCertificateInStore()"), dwErr));

        BOOL bFreeHandle;
        HCRYPTPROV hProv;
        DWORD dwKeySpec;

        BOOL bResult = CryptAcquireCertificatePrivateKey(
            pCertStore, 
            0, 
            NULL, 
            &hProv, 
            &dwKeySpec, 
            &bFreeHandle);

        dwErr = GetLastError();

        RCF_VERIFY(
            bResult, 
            Exception(_RcfError_ApiError("CryptAcquireCertificatePrivateKey()"), dwErr));

        mpCert = pCertStore; 
    }

    void PfxCertificate::addToStore(
        Win32CertificateLocation certStoreLocation, 
        Win32CertificateStore certStore)
    {

        std::wstring wStoreName;
        switch (certStore)
        {
        case Cs_AddressBook:            wStoreName = L"AddressBook";      break;
        case Cs_AuthRoot:               wStoreName = L"AuthRoot";         break;
        case Cs_CertificateAuthority:   wStoreName = L"CA";               break;
        case Cs_Disallowed:             wStoreName = L"Disallowed";       break;
        case Cs_My:                     wStoreName = L"MY";               break;
        case Cs_Root:                   wStoreName = L"Root";             break;
        case Cs_TrustedPeople:          wStoreName = L"TrustedPeople";    break;
        case Cs_TrustedPublisher:       wStoreName = L"TrustedPublisher"; break;
        default:
            RCF_ASSERT(0 && "Invalid certificate store value.");
        }

        DWORD dwFlags = 0;
        switch (certStoreLocation)
        {
        case Cl_CurrentUser:            dwFlags = CERT_SYSTEM_STORE_CURRENT_USER;   break;
        case Cl_LocalMachine:           dwFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;  break;
        default:
            RCF_ASSERT(0 && "Invalid certificate store location value.");
        }

        HCERTSTORE hCertStore = CertOpenStore(
            (LPCSTR) CERT_STORE_PROV_SYSTEM,
            X509_ASN_ENCODING,
            0,
            dwFlags,
            wStoreName.c_str());

        DWORD dwErr = GetLastError();

        RCF_VERIFY(
            hCertStore, 
            RCF::Exception(
                _RcfError_CryptoApiError("CertOpenStore()"), 
                dwErr, 
                RCF::RcfSubsystem_Os));

        BOOL ret = CertAddCertificateContextToStore(
            hCertStore,
            mpCert,
            CERT_STORE_ADD_USE_EXISTING,
            NULL);

        dwErr = GetLastError();

        RCF_VERIFY(
            ret, 
            RCF::Exception(
                _RcfError_CryptoApiError("CertAddCertificateContextToStore()"), 
                dwErr, 
                RCF::RcfSubsystem_Os));

        CertCloseStore(hCertStore, 0);
    }

    Win32Certificate::Win32Certificate() : 
        mpCert(NULL), 
        mHasBeenDeleted(false)
    {
    }

    Win32Certificate::Win32Certificate(PCCERT_CONTEXT pContext) :
        mpCert(pContext), 
        mHasBeenDeleted(false)
    {
    }

    Win32Certificate::~Win32Certificate()
    {
        if (mpCert && !mHasBeenDeleted)
        {
            CertFreeCertificateContext(mpCert);
            mpCert = NULL;
        }
    }

    tstring Win32Certificate::getSubjectName()
    {
        return getCertAttribute(szOID_COMMON_NAME);
    }

    tstring Win32Certificate::getOrganizationName()
    {
        return getCertAttribute(szOID_ORGANIZATION_NAME);
    }

    tstring Win32Certificate::getCertificateName()
    {
        DWORD bufferSize = CertNameToStr(
            X509_ASN_ENCODING,
            &mpCert->pCertInfo->Subject,
            CERT_X500_NAME_STR,
            NULL,
            0);

        std::vector<TCHAR> buffer(bufferSize);

        bufferSize = CertNameToStr(
            X509_ASN_ENCODING,
            &mpCert->pCertInfo->Subject,
            CERT_X500_NAME_STR,
            &buffer[0],
            bufferSize);

        tstring strName(&buffer[0]);
        return strName;
    }

    tstring Win32Certificate::getIssuerName()
    {
        DWORD bufferSize = CertNameToStr(
            X509_ASN_ENCODING,
            &mpCert->pCertInfo->Issuer,
            CERT_X500_NAME_STR,
            NULL,
            0);

        std::vector<TCHAR> buffer(bufferSize);

        bufferSize = CertNameToStr(
            X509_ASN_ENCODING,
            &mpCert->pCertInfo->Issuer,
            CERT_X500_NAME_STR,
            &buffer[0],
            bufferSize);

        tstring strName(&buffer[0]);
        return strName;
    }

    tstring Win32Certificate::getCertAttribute(const char * whichAttr)
    {
        PCCERT_CONTEXT pContext = getWin32Context();

        // How much space do we need.
        DWORD cbNameInfo = 0;

        DWORD dwRet = CryptDecodeObject(
            X509_ASN_ENCODING,
            X509_NAME,
            pContext->pCertInfo->Subject.pbData,
            pContext->pCertInfo->Subject.cbData,
            CRYPT_DECODE_NOCOPY_FLAG,
            NULL,
            &cbNameInfo);

        DWORD dwErr = GetLastError();

        RCF_VERIFY(
            dwRet, 
            RCF::Exception(
                _RcfError_CryptoApiError("CryptDecodeObject()"), 
                dwErr, 
                RCF::RcfSubsystem_Os));

        std::vector<char> vec(cbNameInfo);

        // Retrieve name info.
        dwRet = CryptDecodeObject(X509_ASN_ENCODING,
            X509_NAME,
            pContext->pCertInfo->Subject.pbData,
            pContext->pCertInfo->Subject.cbData,
            CRYPT_DECODE_NOCOPY_FLAG,
            &vec[0],
            &cbNameInfo);

        dwErr = GetLastError();

        RCF_VERIFY(
            dwRet, 
            RCF::Exception(
                _RcfError_CryptoApiError("CryptDecodeObject()"), 
                dwErr, 
                RCF::RcfSubsystem_Os));

        PCERT_NAME_INFO pCertNameInfo = (PCERT_NAME_INFO) &vec[0];

        PCERT_RDN_ATTR pCertAttr = CertFindRDNAttr(whichAttr, pCertNameInfo);
        if (pCertAttr)
        {
            DWORD cbLen = CertRDNValueToStr(pCertAttr->dwValueType, &pCertAttr->Value, NULL, 0);
            std::vector<TCHAR> vecAttr(cbLen);

            CertRDNValueToStr(
                pCertAttr->dwValueType, 
                &pCertAttr->Value, 
                &vecAttr[0],
                static_cast<DWORD>(vecAttr.size()));

            tstring attr(&vecAttr[0]);
            return attr;
        }
        
        return tstring();
    }

    Win32CertificatePtr Win32Certificate::findRootCertificate(
        Win32CertificateLocation certStoreLocation, 
        Win32CertificateStore certStore)
    {
        std::wstring storeName;
        switch (certStore)
        {
        case Cs_AddressBook:            storeName = L"AddressBook";      break;
        case Cs_AuthRoot:               storeName = L"AuthRoot";         break;
        case Cs_CertificateAuthority:   storeName = L"CA";               break;
        case Cs_Disallowed:             storeName = L"Disallowed";       break;
        case Cs_My:                     storeName = L"MY";               break;
        case Cs_Root:                   storeName = L"Root";             break;
        case Cs_TrustedPeople:          storeName = L"TrustedPeople";    break;
        case Cs_TrustedPublisher:       storeName = L"TrustedPublisher"; break;
        default:
            RCF_ASSERT(0 && "Invalid certificate store value.");
        }

        DWORD dwFlags = 0;
        switch (certStoreLocation)
        {
        case Cl_CurrentUser:            dwFlags = CERT_SYSTEM_STORE_CURRENT_USER;   break;
        case Cl_LocalMachine:           dwFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;  break;
        default:
            RCF_ASSERT(0 && "Invalid certificate store location value.");
        }

        Win32CertificatePtr issuerCertPtr;

        HCERTSTORE hCertStore = CertOpenStore(
            (LPCSTR) CERT_STORE_PROV_SYSTEM,
            X509_ASN_ENCODING,
            0,
            dwFlags,
            storeName.c_str());

        DWORD dwErr = GetLastError();

        RCF_VERIFY(
            hCertStore, 
            Exception(_RcfError_ApiError("CertOpenStore()"), dwErr));

        PCCERT_CONTEXT  pSubjectContext = getWin32Context();

        DWORD           dwCertFlags = 0;
        PCCERT_CONTEXT  pIssuerContext = NULL;

        pSubjectContext = CertDuplicateCertificateContext(pSubjectContext);
        if (pSubjectContext)
        {
            do 
            {
                dwCertFlags = 
                        CERT_STORE_REVOCATION_FLAG 
                    |   CERT_STORE_SIGNATURE_FLAG 
                    |   CERT_STORE_TIME_VALIDITY_FLAG;

                pIssuerContext = CertGetIssuerCertificateFromStore(
                    hCertStore,
                    pSubjectContext, 
                    0, 
                    &dwCertFlags);

                if (pIssuerContext) 
                {
                    CertFreeCertificateContext(pSubjectContext);
                    pSubjectContext = pIssuerContext;
                    if (dwCertFlags & CERT_STORE_NO_CRL_FLAG)
                    {
                        // No CRL list available. Proceed anyway.
                        dwCertFlags &= ~(CERT_STORE_NO_CRL_FLAG | CERT_STORE_REVOCATION_FLAG);
                    }
                    if (dwCertFlags) 
                    {
                        if ( dwCertFlags & CERT_STORE_TIME_VALIDITY_FLAG)
                        {
                            // Certificate is expired.
                            // ...
                        }
                        break;
                    }
                } 
                else if (GetLastError() == CRYPT_E_SELF_SIGNED) 
                {
                    // Got the root certificate.
                    issuerCertPtr.reset( new Win32Certificate(pSubjectContext) );
                }
            } 
            while (pIssuerContext);
        }

        CertCloseStore(hCertStore, 0);

        return issuerCertPtr;
    }

    PfxCertificate::~PfxCertificate()
    {
        CertCloseStore(mPfxStore, 0);
    }

    PCCERT_CONTEXT Win32Certificate::getWin32Context()
    {
        return mpCert;
    }

    StoreCertificate::StoreCertificate(
        Win32CertificateLocation certStoreLocation, 
        Win32CertificateStore certStore,
        const tstring & certName) :
            mStore(0)
    {
        std::wstring wStoreName;
        switch (certStore)
        {
        case Cs_AddressBook:            wStoreName = L"AddressBook";      break;
        case Cs_AuthRoot:               wStoreName = L"AuthRoot";         break;
        case Cs_CertificateAuthority:   wStoreName = L"CA";               break;
        case Cs_Disallowed:             wStoreName = L"Disallowed";       break;
        case Cs_My:                     wStoreName = L"MY";               break;
        case Cs_Root:                   wStoreName = L"Root";             break;
        case Cs_TrustedPeople:          wStoreName = L"TrustedPeople";    break;
        case Cs_TrustedPublisher:       wStoreName = L"TrustedPublisher"; break;
        default:
            RCF_ASSERT(0 && "Invalid certificate store value.");
        }

        DWORD dwFlags = 0;
        switch (certStoreLocation)
        {
        case Cl_CurrentUser:            dwFlags = CERT_SYSTEM_STORE_CURRENT_USER;   break;
        case Cl_LocalMachine:           dwFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;  break;
        default:
            RCF_ASSERT(0 && "Invalid certificate store location value.");
        }

        mStore = CertOpenStore(
            (LPCSTR) CERT_STORE_PROV_SYSTEM,
            X509_ASN_ENCODING,
            0,
            dwFlags,
            &wStoreName[0]);

        DWORD dwErr = GetLastError();

        RCF_VERIFY(
            mStore, 
            RCF::Exception(
                _RcfError_CryptoApiError("CertOpenStore()"), 
                dwErr, 
                RCF::RcfSubsystem_Os));

        std::wstring wCertName(certName.begin(), certName.end());

        DWORD dwFindType = CERT_FIND_SUBJECT_STR;

        PCCERT_CONTEXT pStoreCert = CertFindCertificateInStore(
            mStore, 
            X509_ASN_ENCODING, 
            0,
            dwFindType,
            wCertName.c_str(),
            NULL);

        dwErr = GetLastError();

        RCF_VERIFY(
            pStoreCert, 
            RCF::Exception(
            _RcfError_CryptoApiError("CertFindCertificateInStore()"), 
            dwErr, 
            RCF::RcfSubsystem_Os));

        mpCert = pStoreCert;
    }

    void StoreCertificate::removeFromStore()
    {
        if (mpCert)
        {
            BOOL ret = CertDeleteCertificateFromStore(mpCert);
            DWORD dwErr = GetLastError();

            RCF_VERIFY(
                ret, 
                RCF::Exception(
                    _RcfError_CryptoApiError("CertDeleteCertificateFromStore()"), 
                    dwErr, 
                    RCF::RcfSubsystem_Os));

            setHasBeenDeleted();
            mpCert = NULL;
        }
    }

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996) // 'fopen': This function or variable may be unsafe.
#endif

    void Win32Certificate::exportToPfx(const std::string & pfxFilePath)
    {
        RCF::ByteBuffer pfxBuffer = exportToPfx();

        // Write the data to file.
        FILE * fp = fopen(pfxFilePath.c_str(), "wb");
        if (!fp)
        {
            RCF_THROW( Exception(_RcfError_FileOpenWrite(pfxFilePath)) );
        }
        fwrite(pfxBuffer.getPtr(), sizeof(char), pfxBuffer.getLength(), fp);
        fclose(fp);
    }

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    RCF::ByteBuffer Win32Certificate::exportToPfx()
    {
        PCCERT_CONTEXT pContext = getWin32Context();

        // Create in-memory store
        HCERTSTORE  hMemoryStore;

        hMemoryStore = CertOpenStore(
            CERT_STORE_PROV_MEMORY,    // Memory store
            0,                         // Encoding type, not used with a memory store
            NULL,                      // Use the default provider
            0,                         // No flags
            NULL);                     // Not needed

        DWORD dwErr = GetLastError();

        RCF_VERIFY(
            hMemoryStore, 
            Exception(_RcfError_ApiError("CertOpenStore()"), dwErr));

        // Add the certificate.
        BOOL ok = CertAddCertificateContextToStore(
            hMemoryStore,                // Store handle
            pContext,                   // Pointer to a certificate
            CERT_STORE_ADD_USE_EXISTING,
            NULL);

        dwErr = GetLastError();

        RCF_VERIFY(
            ok, 
            Exception(_RcfError_ApiError("CertAddCertificateContextToStore()"), dwErr));

        // Export in-memory store.
        CRYPT_DATA_BLOB pfxBlob = {};
        BOOL exportOk = PFXExportCertStore(hMemoryStore, &pfxBlob, L"", 0/*EXPORT_PRIVATE_KEYS*/);

        dwErr = GetLastError();

        RCF_VERIFY(
            exportOk, 
            Exception(_RcfError_ApiError("PFXExportCertStore()"), dwErr));

        RCF::ByteBuffer pfxBuffer(pfxBlob.cbData);
        pfxBlob.pbData = (BYTE *) pfxBuffer.getPtr();

        exportOk = PFXExportCertStore(hMemoryStore, &pfxBlob, L"", 0/*EXPORT_PRIVATE_KEYS*/);
        
        dwErr = GetLastError();

        RCF_VERIFY(
            exportOk, 
            Exception(_RcfError_ApiError("PFXExportCertStore()"), dwErr));

        CertCloseStore(hMemoryStore, 0);

        return pfxBuffer;
    }

    StoreCertificate::~StoreCertificate()
    {
        CertCloseStore(mStore, 0);
        mStore = NULL;
    }

    StoreCertificateIterator::StoreCertificateIterator(
        Win32CertificateLocation certStoreLocation, 
        Win32CertificateStore certStore) : 
            mhCertStore(NULL),
            mpCertIterator(NULL)
    {
        std::wstring strCertStore;
        switch (certStore)
        {
            case Cs_AddressBook:            strCertStore = L"AddressBook";      break;
            case Cs_AuthRoot:               strCertStore = L"AuthRoot";         break;
            case Cs_CertificateAuthority:   strCertStore = L"CA";               break;
            case Cs_Disallowed:             strCertStore = L"Disallowed";       break;
            case Cs_My:                     strCertStore = L"MY";               break;
            case Cs_Root:                   strCertStore = L"Root";             break;
            case Cs_TrustedPeople:          strCertStore = L"TrustedPeople";    break;
            case Cs_TrustedPublisher:       strCertStore = L"TrustedPublisher"; break;
            default:
                RCF_ASSERT(0 && "Invalid certificate store value.");
        }

        DWORD dwStoreLocation = 0;
        switch (certStoreLocation)
        {
            case Cl_CurrentUser:            dwStoreLocation = CERT_SYSTEM_STORE_CURRENT_USER;   break;
            case Cl_LocalMachine:           dwStoreLocation = CERT_SYSTEM_STORE_LOCAL_MACHINE;  break;
            default:
                RCF_ASSERT(0 && "Invalid certificate store location value.");
        }
        
        mhCertStore = CertOpenStore(
            (LPCSTR) CERT_STORE_PROV_SYSTEM,
            X509_ASN_ENCODING,
            0,
            dwStoreLocation,
            strCertStore.c_str());

        DWORD dwErr = GetLastError();

        RCF_VERIFY(
            mhCertStore, 
            Exception(_RcfError_ApiError("CertOpenStore()"), dwErr));   
    }

    StoreCertificateIterator::~StoreCertificateIterator()
    {
        if (mpCertIterator)
        {
            CertFreeCertificateContext(mpCertIterator);
            mpCertIterator = NULL;
        }

        CertCloseStore(mhCertStore, 0);
        mhCertStore = NULL;
    }

    bool StoreCertificateIterator::moveNext()
    {
        mpCertIterator = CertFindCertificateInStore(
            mhCertStore, 
            X509_ASN_ENCODING, 
            0,
            CERT_FIND_ANY,
            NULL,
            mpCertIterator);

        if (mpCertIterator)
        {
            PCCERT_CONTEXT pCert = CertDuplicateCertificateContext(mpCertIterator);
            mCurrentCertPtr.reset( new Win32Certificate(pCert) );
            return true;
        }
        else
        {
            mCurrentCertPtr.reset();
            return false;
        }
    }

    void StoreCertificateIterator::reset()
    {
        if (mpCertIterator)
        {
            CertFreeCertificateContext(mpCertIterator);
            mpCertIterator = NULL;
        }

        mCurrentCertPtr.reset();
    }

    Win32CertificatePtr StoreCertificateIterator::current()
    {
        return mCurrentCertPtr;
    }

} // namespace RCF
