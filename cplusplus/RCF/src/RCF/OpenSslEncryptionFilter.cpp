
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

#include <RCF/OpenSslEncryptionFilter.hpp>

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <RCF/DynamicLib.hpp>
#include <RCF/Exception.hpp>
#include <RCF/Globals.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/RecursionLimiter.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    class OpenSslDll
    {
    public:
        OpenSslDll();

        DynamicLibPtr           mDynamicLibPtr;

        void loadFunctionPtrs();

        typedef long            (*Pfn_SSL_get_verify_result)(const SSL *ssl);
        typedef X509 *          (*Pfn_SSL_get_peer_certificate)(const SSL *s);
        typedef int             (*Pfn_SSL_state)(const SSL *ssl);
        typedef void            (*Pfn_SSL_set_bio)(SSL *s, BIO *rbio,BIO *wbio);
        typedef void            (*Pfn_SSL_set_connect_state)(SSL *s);
        typedef void            (*Pfn_SSL_set_accept_state)(SSL *s);
        typedef void            (*Pfn_SSL_set_verify)(SSL *s, int mode, int (*callback)(int ok,X509_STORE_CTX *ctx));
        typedef SSL *           (*Pfn_SSL_new)(SSL_CTX *ctx);
        typedef void            (*Pfn_SSL_free)(SSL *ssl);
        typedef SSL_CTX *       (*Pfn_SSL_CTX_new)(SSL_METHOD *meth);
        typedef void            (*Pfn_SSL_CTX_free)(SSL_CTX *);
        typedef SSL_METHOD *    (*Pfn_SSLv23_method)(void);
        typedef BIO_METHOD *    (*Pfn_BIO_f_ssl)(void);
        typedef int             (*Pfn_SSL_CTX_use_PrivateKey)(SSL_CTX *ctx, EVP_PKEY *pkey);
        typedef int             (*Pfn_SSL_CTX_use_certificate_chain_file)(SSL_CTX *ctx, const char *file); /* PEM type */
        typedef int             (*Pfn_SSL_CTX_load_verify_locations)(SSL_CTX *ctx, const char *CAfile, const char *CApath);
        typedef void            (*Pfn_SSL_load_error_strings)(void );
        typedef int             (*Pfn_SSL_library_init)(void );

        Pfn_SSL_get_verify_result               pfn_SSL_get_verify_result;
        Pfn_SSL_get_peer_certificate            pfn_SSL_get_peer_certificate;
        Pfn_SSL_state                           pfn_SSL_state;
        Pfn_SSL_set_bio                         pfn_SSL_set_bio;
        Pfn_SSL_set_connect_state               pfn_SSL_set_connect_state;
        Pfn_SSL_set_accept_state                pfn_SSL_set_accept_state;
        Pfn_SSL_set_verify                      pfn_SSL_set_verify;
        Pfn_SSL_new                             pfn_SSL_new;
        Pfn_SSL_free                            pfn_SSL_free;
        Pfn_SSL_CTX_new                         pfn_SSL_CTX_new;
        Pfn_SSL_CTX_free                        pfn_SSL_CTX_free;
        Pfn_SSLv23_method                       pfn_SSLv23_method;
        Pfn_BIO_f_ssl                           pfn_BIO_f_ssl;
        Pfn_SSL_CTX_use_PrivateKey              pfn_SSL_CTX_use_PrivateKey;
        Pfn_SSL_CTX_use_certificate_chain_file  pfn_SSL_CTX_use_certificate_chain_file;
        Pfn_SSL_CTX_load_verify_locations       pfn_SSL_CTX_load_verify_locations;
        Pfn_SSL_load_error_strings              pfn_SSL_load_error_strings;
        Pfn_SSL_library_init                    pfn_SSL_library_init;       
    };

    class OpenSslCryptoDll
    {
    public:
        OpenSslCryptoDll();

        DynamicLibPtr           mDynamicLibPtr;

        void loadFunctionPtrs();

        typedef size_t          (*Pfn_BIO_ctrl_pending)(BIO *b);
        typedef int             (*Pfn_BIO_write)(BIO *b, const void *data, int len);
        typedef int             (*Pfn_BIO_read)(BIO *b, void *data, int len);
        typedef int             (*Pfn_BIO_nread0)(BIO *bio, char **buf);
        typedef int             (*Pfn_BIO_nwrite0)(BIO *bio, char **buf);
        typedef size_t          (*Pfn_BIO_ctrl_get_read_request)(BIO *b);
        typedef int             (*Pfn_BIO_nread)(BIO *bio, char **buf, int num);
        typedef int             (*Pfn_BIO_nwrite)(BIO *bio, char **buf, int num);
        typedef long            (*Pfn_BIO_ctrl)(BIO *bp,int cmd,long larg,void *parg);
        typedef int             (*Pfn_BIO_new_bio_pair)(BIO **bio1, size_t writebuf1,BIO **bio2, size_t writebuf2);
        typedef BIO *           (*Pfn_BIO_new)(BIO_METHOD *type);
        typedef int             (*Pfn_BIO_free)(BIO *a);
        typedef void            (*Pfn_EVP_PKEY_free)(EVP_PKEY *pkey);
        typedef BIO_METHOD *    (*Pfn_BIO_s_file)(void );
        typedef void            (*Pfn_ERR_print_errors_cb)(int (*cb)(const char *str, size_t len, void *u),void *u);
        typedef void            (*Pfn_ERR_print_errors)(BIO *bp);
        typedef BIO_METHOD *    (*Pfn_BIO_s_mem)(void);
        typedef void            (*Pfn_ERR_load_crypto_strings)(void);
        typedef int             (*Pfn_BIO_test_flags)(const BIO *b, int flags);
        typedef void            (*Pfn_X509_free)(X509 *a);
        typedef EVP_PKEY *      (*Pfn_PEM_read_bio_PrivateKey)(BIO *bp, EVP_PKEY **x, pem_password_cb *cb, void *u);
        typedef void            (*Pfn_OPENSSL_add_all_algorithms_noconf)(void);

        typedef X509_NAME *     (*Pfn_X509_get_subject_name)(X509 *a);
        typedef X509_NAME *     (*Pfn_X509_get_issuer_name)(X509 *a);
        typedef int             (*Pfn_X509_NAME_print_ex)(BIO *out, X509_NAME *nm, int indent, unsigned long flags);

        Pfn_BIO_ctrl_pending                    pfn_BIO_ctrl_pending;
        Pfn_BIO_write                           pfn_BIO_write;
        Pfn_BIO_read                            pfn_BIO_read;
        Pfn_BIO_nread0                          pfn_BIO_nread0;
        Pfn_BIO_nwrite0                         pfn_BIO_nwrite0;
        Pfn_BIO_ctrl_get_read_request           pfn_BIO_ctrl_get_read_request;
        Pfn_BIO_nread                           pfn_BIO_nread;
        Pfn_BIO_nwrite                          pfn_BIO_nwrite;
        Pfn_BIO_ctrl                            pfn_BIO_ctrl;
        Pfn_BIO_new_bio_pair                    pfn_BIO_new_bio_pair;
        Pfn_BIO_new                             pfn_BIO_new;
        Pfn_BIO_free                            pfn_BIO_free;
        Pfn_EVP_PKEY_free                       pfn_EVP_PKEY_free;
        Pfn_BIO_s_file                          pfn_BIO_s_file;
        Pfn_ERR_print_errors_cb                 pfn_ERR_print_errors_cb;
        Pfn_ERR_print_errors                    pfn_ERR_print_errors;
        Pfn_BIO_s_mem                           pfn_BIO_s_mem;
        Pfn_ERR_load_crypto_strings             pfn_ERR_load_crypto_strings;
        Pfn_BIO_test_flags                      pfn_BIO_test_flags;
        Pfn_X509_free                           pfn_X509_free;
        Pfn_PEM_read_bio_PrivateKey             pfn_PEM_read_bio_PrivateKey;
        Pfn_OPENSSL_add_all_algorithms_noconf   pfn_OPENSSL_add_all_algorithms_noconf;
        Pfn_X509_get_subject_name               pfn_X509_get_subject_name;
        Pfn_X509_get_issuer_name                pfn_X509_get_issuer_name;
        Pfn_X509_NAME_print_ex                  pfn_X509_NAME_print_ex;
    };

    // OpenSslDll

    OpenSslDll::OpenSslDll()
    {

#ifndef RCF_OPENSSL_STATIC
        mDynamicLibPtr.reset( new DynamicLib( getGlobals().getOpenSslDllName() ) );
#endif

        loadFunctionPtrs();

        // Initialize OpenSSL.
        pfn_SSL_library_init(); // always returns 1
        pfn_SSL_load_error_strings(); // no return value
    }

#ifdef RCF_OPENSSL_STATIC

    // Load static function pointers.
    // Requires linking to zlib or building with zlib source.
#define RCF_OPENSSL_LOAD_FUNCTION RCF_LOAD_LIB_FUNCTION

#else

    // Load dynamic function pointers directly from zlib DLL.
    // Requires zlib DLL to be present at runtime.
    // Does not require linking to zlib.
#define RCF_OPENSSL_LOAD_FUNCTION RCF_LOAD_DLL_FUNCTION

#endif

    void OpenSslDll::loadFunctionPtrs()
    {

#ifndef RCF_OPENSSL_STATIC
        RCF_ASSERT(mDynamicLibPtr);
#endif

        RCF_OPENSSL_LOAD_FUNCTION(SSL_get_verify_result);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_get_peer_certificate);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_state);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_set_bio);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_set_connect_state);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_set_accept_state);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_set_verify);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_new);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_free);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_CTX_new);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_CTX_free);
        RCF_OPENSSL_LOAD_FUNCTION(SSLv23_method);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_f_ssl);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_CTX_use_PrivateKey);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_CTX_use_certificate_chain_file);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_CTX_load_verify_locations);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_load_error_strings);
        RCF_OPENSSL_LOAD_FUNCTION(SSL_library_init);
    }

    // OpenSslCryptoDll

    OpenSslCryptoDll::OpenSslCryptoDll()
    {

#ifndef RCF_OPENSSL_STATIC
        mDynamicLibPtr.reset( new DynamicLib( getGlobals().getOpenSslCryptoDllName() ) );
#endif

        loadFunctionPtrs();

        // Initialize OpenSSL.
        pfn_ERR_load_crypto_strings(); // no return value
        pfn_OPENSSL_add_all_algorithms_noconf(); // no return value

    }

    void OpenSslCryptoDll::loadFunctionPtrs()
    {

#ifndef RCF_OPENSSL_STATIC
        RCF_ASSERT(mDynamicLibPtr);
#endif

        RCF_OPENSSL_LOAD_FUNCTION(BIO_ctrl_pending);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_write);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_read);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_nread0);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_nwrite0);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_ctrl_get_read_request);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_nread);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_nwrite);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_ctrl);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_new_bio_pair);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_new);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_free);
        RCF_OPENSSL_LOAD_FUNCTION(EVP_PKEY_free);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_s_file);
        RCF_OPENSSL_LOAD_FUNCTION(ERR_print_errors_cb);
        RCF_OPENSSL_LOAD_FUNCTION(ERR_print_errors);
        RCF_OPENSSL_LOAD_FUNCTION(BIO_s_mem);
        RCF_OPENSSL_LOAD_FUNCTION(ERR_load_crypto_strings); 

        RCF_OPENSSL_LOAD_FUNCTION(BIO_test_flags);
        RCF_OPENSSL_LOAD_FUNCTION(X509_free);
        RCF_OPENSSL_LOAD_FUNCTION(PEM_read_bio_PrivateKey);
        RCF_OPENSSL_LOAD_FUNCTION(OPENSSL_add_all_algorithms_noconf);
        RCF_OPENSSL_LOAD_FUNCTION(X509_get_subject_name);
        RCF_OPENSSL_LOAD_FUNCTION(X509_get_issuer_name);
        RCF_OPENSSL_LOAD_FUNCTION(X509_NAME_print_ex);
        
    }

    OpenSslDll & Globals::getOpenSslDll()
    {
        Lock lock(getRootMutex());

        if (!mpOpenSslDll)
        {
            mpOpenSslDll = new OpenSslDll();
        }
        return *mpOpenSslDll;
    }

    OpenSslCryptoDll & Globals::getOpenSslCryptoDll()
    {
        Lock lock(getRootMutex());

        if (!mpOpenSslCryptoDll)
        {
            mpOpenSslCryptoDll = new OpenSslCryptoDll();
        }
        return *mpOpenSslCryptoDll;
    }

#if RCF_FEATURE_OPENSSL==1

    void Globals::deleteOpenSslDll()
    {
        if (mpOpenSslDll)
        {
            delete mpOpenSslDll;
            mpOpenSslDll = NULL;
        }
    }

    void Globals::deleteOpenSslCryptoDll()
    {
        if (mpOpenSslCryptoDll)
        {
            delete mpOpenSslCryptoDll;
            mpOpenSslCryptoDll = NULL;
        }
    }

#endif

#define SSL_get_verify_result                   mSslDll.pfn_SSL_get_verify_result
#define SSL_get_peer_certificate                mSslDll.pfn_SSL_get_peer_certificate
#define SSL_state                               mSslDll.pfn_SSL_state
#define SSL_set_bio                             mSslDll.pfn_SSL_set_bio
#define SSL_set_connect_state                   mSslDll.pfn_SSL_set_connect_state
#define SSL_set_accept_state                    mSslDll.pfn_SSL_set_accept_state
#define SSL_set_verify                          mSslDll.pfn_SSL_set_verify
#define SSL_new                                 mSslDll.pfn_SSL_new
#define SSL_free                                mSslDll.pfn_SSL_free
#define SSL_CTX_new                             mSslDll.pfn_SSL_CTX_new
#define SSL_CTX_free                            mSslDll.pfn_SSL_CTX_free
#define SSLv23_method                           mSslDll.pfn_SSLv23_method
#define BIO_f_ssl                               mSslDll.pfn_BIO_f_ssl
#define SSL_CTX_use_PrivateKey                  mSslDll.pfn_SSL_CTX_use_PrivateKey
#define SSL_CTX_use_certificate_chain_file      mSslDll.pfn_SSL_CTX_use_certificate_chain_file
#define SSL_CTX_load_verify_locations           mSslDll.pfn_SSL_CTX_load_verify_locations
#define SSL_load_error_strings                  mSslDll.pfn_SSL_load_error_strings
#define SSL_library_init                        mSslDll.pfn_SSL_library_init

#define BIO_ctrl_pending                        mCryptoDll.pfn_BIO_ctrl_pending
#define BIO_write                               mCryptoDll.pfn_BIO_write
#define BIO_read                                mCryptoDll.pfn_BIO_read
#define BIO_nread0                              mCryptoDll.pfn_BIO_nread0
#define BIO_nwrite0                             mCryptoDll.pfn_BIO_nwrite0
#define BIO_ctrl_get_read_request               mCryptoDll.pfn_BIO_ctrl_get_read_request
#define BIO_nread                               mCryptoDll.pfn_BIO_nread
#define BIO_nwrite                              mCryptoDll.pfn_BIO_nwrite
#define BIO_ctrl                                mCryptoDll.pfn_BIO_ctrl
#define BIO_new_bio_pair                        mCryptoDll.pfn_BIO_new_bio_pair
#define BIO_new                                 mCryptoDll.pfn_BIO_new
#define BIO_free                                mCryptoDll.pfn_BIO_free
#define EVP_PKEY_free                           mCryptoDll.pfn_EVP_PKEY_free
#define BIO_s_file                              mCryptoDll.pfn_BIO_s_file
#define ERR_print_errors_cb                     mCryptoDll.pfn_ERR_print_errors_cb
#define ERR_print_errors                        mCryptoDll.pfn_ERR_print_errors
#define BIO_s_mem                               mCryptoDll.pfn_BIO_s_mem
#define ERR_load_crypto_strings                 mCryptoDll.pfn_ERR_load_crypto_strings

#define BIO_test_flags                          mCryptoDll.pfn_BIO_test_flags
#define X509_free                               mCryptoDll.pfn_X509_free
#define PEM_read_bio_PrivateKey                 mCryptoDll.pfn_PEM_read_bio_PrivateKey
#define OPENSSL_add_all_algorithms_noconf       mCryptoDll.pfn_OPENSSL_add_all_algorithms_noconf
#define X509_get_subject_name                   mCryptoDll.pfn_X509_get_subject_name
#define X509_get_issuer_name                    mCryptoDll.pfn_X509_get_issuer_name
#define X509_NAME_print_ex                      mCryptoDll.pfn_X509_NAME_print_ex

/*
    void printErrors(SSL * pSsl, int result)
    {
        if (result <= 0)
        {
            int error = SSL_get_error(pSsl, result);

            switch(error)
            {
            case SSL_ERROR_ZERO_RETURN:
            case SSL_ERROR_NONE: 
            case SSL_ERROR_WANT_READ :

                break;

            default :
                {

                    char buffer[256];

                    while (error != 0)
                    {
                        ERR_error_string_n(error, buffer, sizeof(buffer));

                        std::cout << "Error: " << error << " - " << buffer << std::endl;

                        error = ERR_get_error();

                        printf("Error = %s\n",ERR_reason_error_string(error));
                    }
                }
                break;
            }
        }
    }
*/

    std::string getOpenSslErrors()
    {
        OpenSslCryptoDll & cryptoDll = getGlobals().getOpenSslCryptoDll();

        boost::shared_ptr<BIO> bio( cryptoDll.pfn_BIO_new( cryptoDll.pfn_BIO_s_mem() ), cryptoDll.pfn_BIO_free );
        cryptoDll.pfn_ERR_print_errors(bio.get());
        std::vector<char> buffer(256);
        unsigned int startPos = 0;
        unsigned int bytesRead = 0;
        while (true)
        {
            RCF_ASSERT_GT(buffer.size() , startPos);

            int ret = cryptoDll.pfn_BIO_read(
                bio.get(),
                &buffer[startPos],
                static_cast<int>(buffer.size()-startPos));

            if (ret > 0)
            {
                bytesRead += ret;
            }
            if (bytesRead < buffer.size())
            {
                break;
            }
            startPos = static_cast<unsigned int>(buffer.size());
            buffer.resize( 2*buffer.size() );
        }

        return std::string(&buffer[0], bytesRead);
    }

    // Can't find any OpenSSL function to convert certificate validation error code into
    // a readable string, so here we do it by hand.

    static struct
    {
        int code;
        const char* string;
    }
    X509_message_table[] =
    {
        { X509_V_OK                                         , "X509_V_OK" },
        { X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT              , "X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT" },
        { X509_V_ERR_UNABLE_TO_GET_CRL                      , "X509_V_ERR_UNABLE_TO_GET_CRL" },
        { X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE       , "X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE" },
        { X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE        , "X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE" },
        { X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY     , "X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY" },
        { X509_V_ERR_CERT_SIGNATURE_FAILURE                 , "X509_V_ERR_CERT_SIGNATURE_FAILURE" },
        { X509_V_ERR_CRL_SIGNATURE_FAILURE                  , "X509_V_ERR_CRL_SIGNATURE_FAILURE" },
        { X509_V_ERR_CERT_NOT_YET_VALID                     , "X509_V_ERR_CERT_NOT_YET_VALID" },
        { X509_V_ERR_CERT_HAS_EXPIRED                       , "X509_V_ERR_CERT_HAS_EXPIRED" },
        { X509_V_ERR_CRL_NOT_YET_VALID                      , "X509_V_ERR_CRL_NOT_YET_VALID" },
        { X509_V_ERR_CRL_HAS_EXPIRED                        , "X509_V_ERR_CRL_HAS_EXPIRED" },
        { X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD         , "X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD" },
        { X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD          , "X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD" },
        { X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD         , "X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD" },
        { X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD         , "X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD" },
        { X509_V_ERR_OUT_OF_MEM                             , "X509_V_ERR_OUT_OF_MEM" },
        { X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT            , "X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT" },
        { X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN              , "X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN" },
        { X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY      , "X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY" },
        { X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE        , "X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE" },
        { X509_V_ERR_CERT_CHAIN_TOO_LONG                    , "X509_V_ERR_CERT_CHAIN_TOO_LONG" },
        { X509_V_ERR_CERT_REVOKED                           , "X509_V_ERR_CERT_REVOKED" },
        { X509_V_ERR_INVALID_CA                             , "X509_V_ERR_INVALID_CA" },
        { X509_V_ERR_PATH_LENGTH_EXCEEDED                   , "X509_V_ERR_PATH_LENGTH_EXCEEDED" },
        { X509_V_ERR_INVALID_PURPOSE                        , "X509_V_ERR_INVALID_PURPOSE" },
        { X509_V_ERR_CERT_UNTRUSTED                         , "X509_V_ERR_CERT_UNTRUSTED" },
        { X509_V_ERR_CERT_REJECTED                          , "X509_V_ERR_CERT_REJECTED" },
        { X509_V_ERR_SUBJECT_ISSUER_MISMATCH                , "X509_V_ERR_SUBJECT_ISSUER_MISMATCH" },
        { X509_V_ERR_AKID_SKID_MISMATCH                     , "X509_V_ERR_AKID_SKID_MISMATCH" },
        { X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH            , "X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH" },
        { X509_V_ERR_KEYUSAGE_NO_CERTSIGN                   , "X509_V_ERR_KEYUSAGE_NO_CERTSIGN" },
        { X509_V_ERR_UNABLE_TO_GET_CRL_ISSUER               , "X509_V_ERR_UNABLE_TO_GET_CRL_ISSUER" },
        { X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION           , "X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION" },
        { X509_V_ERR_KEYUSAGE_NO_CRL_SIGN                   , "X509_V_ERR_KEYUSAGE_NO_CRL_SIGN" },
        { X509_V_ERR_UNHANDLED_CRITICAL_CRL_EXTENSION       , "X509_V_ERR_UNHANDLED_CRITICAL_CRL_EXTENSION" },
        { X509_V_ERR_INVALID_NON_CA                         , "X509_V_ERR_INVALID_NON_CA" },
        { X509_V_ERR_PROXY_PATH_LENGTH_EXCEEDED             , "X509_V_ERR_PROXY_PATH_LENGTH_EXCEEDED" },
        { X509_V_ERR_KEYUSAGE_NO_DIGITAL_SIGNATURE          , "X509_V_ERR_KEYUSAGE_NO_DIGITAL_SIGNATURE" },
        { X509_V_ERR_PROXY_CERTIFICATES_NOT_ALLOWED         , "X509_V_ERR_PROXY_CERTIFICATES_NOT_ALLOWED" },
        { X509_V_ERR_INVALID_EXTENSION                      , "X509_V_ERR_INVALID_EXTENSION" },
        { X509_V_ERR_INVALID_POLICY_EXTENSION               , "X509_V_ERR_INVALID_POLICY_EXTENSION" },
        { X509_V_ERR_NO_EXPLICIT_POLICY                     , "X509_V_ERR_NO_EXPLICIT_POLICY" },
        { X509_V_ERR_UNNESTED_RESOURCE                      , "X509_V_ERR_UNNESTED_RESOURCE" }
    };

    #define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

    const char* SSL_get_verify_result_string(int rc)
    {
        const char * ret = "Undefined OpenSSL result code.";

        for (std::size_t i = 0; i < ARRAY_SIZE(X509_message_table); ++i)
        {
            if (X509_message_table[i].code == rc)
            {
                ret = X509_message_table[i].string;
                break;
            }
        }
        return ret;
    }
 

    PemCertificate::PemCertificate(
        const std::string & pathToCert, 
        const std::string & password) :
        mPathToCert(pathToCert),
        mPassword(password)
    {
    }

    X509Certificate::X509Certificate(X509 * pX509) : 
        mSslDll(getGlobals().getOpenSslDll()),
        mCryptoDll(getGlobals().getOpenSslCryptoDll()),
        mpX509(pX509)
    {
    }

    X509 * X509Certificate::getX509()
    {
        return mpX509;
    }

    std::string X509Certificate::getCertificateName()
    {
        X509_NAME *subject = X509_get_subject_name(mpX509);

        // Need a BIO to print the info to.
        BIO *subjectBio = BIO_new(BIO_s_mem());
            
        X509_NAME_print_ex(subjectBio, subject, 0, XN_FLAG_RFC2253);
            
        // Copy the data out of the BIO.
        char * dataStart = NULL;
        long nameLength = BIO_get_mem_data(subjectBio, &dataStart);
        std::string strCertName(dataStart, nameLength);
        return strCertName;
    }

    std::string X509Certificate::getIssuerName()
    {
        X509_NAME *subject = X509_get_issuer_name(mpX509);

        // Need a BIO to print the info to.
        BIO *subjectBio = BIO_new(BIO_s_mem());

        X509_NAME_print_ex(subjectBio, subject, 0, XN_FLAG_RFC2253);

        // Copy the data out of the BIO.
        char * dataStart = NULL;
        long nameLength = BIO_get_mem_data(subjectBio, &dataStart);
        std::string strIssuerName(dataStart, nameLength);
        return strIssuerName;
    }

    class OpenSslEncryptionFilterImpl
    {
    public:
        OpenSslEncryptionFilterImpl(
            OpenSslEncryptionFilter &   openSslEncryptionFilter,
            SslRole                     sslRole,
            const std::string &         certificateFile,
            const std::string &         certificateFilePassword,
            const std::string &         caCertificate,
            const std::string &         ciphers,
            CertificateValidationCb     verifyFunctor,
            unsigned int                bioBufferSize);

        void reset();

        void read(
            const ByteBuffer &          byteBuffer, 
            std::size_t                 bytesRequested);

        void write(
            const std::vector<ByteBuffer> &byteBuffers);

        void onReadWriteCompleted(
            std::size_t                 bytesTransferred);

        SSL *       getSSL();
        SSL_CTX *   getCTX();
        X509 *      getPeerCertificate();

    private:
        void init();

        bool loadCertificate(
            boost::shared_ptr<SSL_CTX>  ctx,
            const std::string &         file,
            const std::string &         password);

        bool loadCaCertificate(
            boost::shared_ptr<SSL_CTX>  ctx,
            const std::string &         file);

        void readWrite();
        void transferData();
        void onDataTransferred(std::size_t bytesTransferred);
        void retryReadWrite();

        OpenSslDll &                    mSslDll;
        OpenSslCryptoDll &              mCryptoDll;

        std::size_t                     mPos;
        std::size_t                     mReadRequested;
        ByteBuffer                      mPreByteBuffer;
        ByteBuffer                      mPostByteBuffer;
        std::vector<ByteBuffer>         mByteBuffers;

        ReallocBufferPtr                mVecPtr;

        enum IoState
        {
            Ready,
            Reading,
            Writing
        };

        SslRole                         mSslRole;
        std::string                     mCertificateFile;
        std::string                     mCertificateFilePassword;
        std::string                     mCaCertificate;
        std::string                     mCiphers;
        IoState                         mPreState;
        IoState                         mPostState;
        bool                            mRetry;
        bool                            mHandshakeOk;
        char *                          mPreBuffer;
        char *                          mPostBuffer;
        std::size_t                     mPostBufferLen;
        std::size_t                     mPostBufferRequested;
        CertificateValidationCb         mVerifyFunctor;
        int                             mErr;

        // OpenSSL members
        // NB: using shared_ptr instead of auto_ptr, since we need custom deleters
        boost::shared_ptr<SSL_CTX>      mSslCtx;
        boost::shared_ptr<SSL>          mSsl;
        boost::shared_ptr<BIO>          mBio;
        boost::shared_ptr<BIO>          mIoBio;
        boost::shared_ptr<BIO>          mSslBio;

        unsigned int                    mBioBufferSize;

        OpenSslEncryptionFilter &       mOpenSslEncryptionFilter;

        RecursionState<int, int>        mRecursionState;
        bool                            mUseRecursionLimiter;

        boost::shared_ptr<X509>         mPeerCertPtr;
    };

#ifdef _MSC_VER
#pragma warning( push )
// warning C4355: 'this' : used in base member initializer list
#pragma warning( disable : 4355 ) 
#endif

    OpenSslEncryptionFilter::OpenSslEncryptionFilter(
        const std::string &certificateFile,
        const std::string &certificateFilePassword,
        const std::string &caCertificate,
        const std::string &ciphers,
        CertificateValidationCb verifyFunctor,
        SslRole sslRole,
        unsigned int bioBufferSize) :
            mImplPtr( new OpenSslEncryptionFilterImpl(
                *this,
                sslRole,
                certificateFile,
                certificateFilePassword,
                caCertificate,
                ciphers,
                verifyFunctor,
                bioBufferSize) )
    {}

    OpenSslEncryptionFilter::OpenSslEncryptionFilter(
        ClientStub * pClientStub,
        SslRole sslRole,
        unsigned int bioBufferSize)
    {
        std::string certificateFile;
        std::string certificateFilePassword;
        std::string caCertificate;

        CertificatePtr mCertPtr = pClientStub->getCertificate();
        PemCertificate * pPemCert = dynamic_cast<PemCertificate *>(mCertPtr.get());
        if (pPemCert)
        {
            certificateFile = pPemCert->mPathToCert;
            certificateFilePassword = pPemCert->mPassword;
        }

        mCertPtr = pClientStub->getCaCertificate();
        pPemCert = dynamic_cast<PemCertificate *>(mCertPtr.get());
        if (pPemCert)
        {
            caCertificate = pPemCert->mPathToCert;
        }

        std::string ciphers = pClientStub->getOpenSslCipherSuite();
        CertificateValidationCb certValidationCb = pClientStub->getCertificateValidationCallback();

        mImplPtr.reset( new OpenSslEncryptionFilterImpl(
            *this,
            sslRole,
            certificateFile,
            certificateFilePassword,
            caCertificate,
            ciphers,
            certValidationCb,
            bioBufferSize) );
    }


#ifdef _MSC_VER
#pragma warning( pop )
#endif

    void OpenSslEncryptionFilter::resetState()
    {
        mImplPtr->reset();
    }

    void OpenSslEncryptionFilter::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        mImplPtr->read(byteBuffer, bytesRequested);
    }

    void OpenSslEncryptionFilter::write(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        mImplPtr->write(byteBuffers);
    }

    void OpenSslEncryptionFilter::onReadCompleted(
        const ByteBuffer &byteBuffer)
    {
        mImplPtr->onReadWriteCompleted(byteBuffer.getLength());
    }

    void OpenSslEncryptionFilter::onWriteCompleted(
        std::size_t bytesTransferred)
    {
        mImplPtr->onReadWriteCompleted(bytesTransferred);
    }

    SSL *OpenSslEncryptionFilter::getSSL()
    {
        return mImplPtr->getSSL();
    }

    SSL_CTX *OpenSslEncryptionFilter::getCTX()
    {
        return mImplPtr->getCTX();
    }

    SSL *OpenSslEncryptionFilterImpl::getSSL()
    {
        return mSsl.get();
    }

    SSL_CTX *OpenSslEncryptionFilterImpl::getCTX()
    {
        return mSslCtx.get();
    }

    CertificatePtr OpenSslEncryptionFilter::getPeerCertificate()
    {
        CertificatePtr peerCertPtr( new X509Certificate(mImplPtr->getPeerCertificate()) );
        return peerCertPtr;
    }

    X509 * OpenSslEncryptionFilterImpl::getPeerCertificate()
    {
        return mPeerCertPtr.get();
    }

    OpenSslEncryptionFilterImpl::OpenSslEncryptionFilterImpl(
        OpenSslEncryptionFilter &openSslEncryptionFilter,
        SslRole sslRole,
        const std::string &certificateFile,
        const std::string &certificateFilePassword,
        const std::string &caCertificate,
        const std::string &ciphers,
        CertificateValidationCb verifyFunctor,
        unsigned int bioBufferSize) :
            mSslDll(getGlobals().getOpenSslDll()),
            mCryptoDll(getGlobals().getOpenSslCryptoDll()),
            mSslRole(sslRole),
            mCertificateFile(certificateFile),
            mCertificateFilePassword(certificateFilePassword),
            mCaCertificate(caCertificate),
            mCiphers(ciphers),
            mPreState(Ready),
            mPostState(Ready),
            mRetry(),
            mHandshakeOk(),
            mPreBuffer(),
            mPostBuffer(),
            mPostBufferLen(),
            mPostBufferRequested(),
            mVerifyFunctor(verifyFunctor),
            mErr(),
            mBioBufferSize(bioBufferSize),
            mOpenSslEncryptionFilter(openSslEncryptionFilter),
            mUseRecursionLimiter(sslRole == SslClient)
    {
        init();
    }

    void OpenSslEncryptionFilterImpl::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        if (byteBuffer.isEmpty() && bytesRequested == 0)
        {
            // If we have any data that hasn't been read, then issue a read 
            // completion, otherwise clear our buffers and issue a zero byte 
            // read on the next filter.

            if (BIO_ctrl_pending(mIoBio.get()) == 0)
            {
                RCF_ASSERT_EQ(mPreState , Ready);
                mPreState = Reading;
                mReadRequested = bytesRequested;
                mOpenSslEncryptionFilter.mpPostFilter->read(ByteBuffer(), 0);
            }
            else
            {
                mOpenSslEncryptionFilter.mpPreFilter->onReadCompleted(byteBuffer);
            }
        }
        else
        {
            RCF_ASSERT(mPreState == Ready || mPreState == Reading)(mPreState);            
            mPreState = Reading;
            if (byteBuffer.getLength() == 0)
            {
                if (mVecPtr.get() == NULL && !mVecPtr.unique())
                {
                    mVecPtr.reset(new ReallocBuffer(bytesRequested));
                }
                mVecPtr->resize(bytesRequested);
                mPreByteBuffer = ByteBuffer(mVecPtr);
            }
            else
            {
                mPreByteBuffer = byteBuffer;
            }
            mReadRequested = bytesRequested;
            readWrite();
        }
    }

    void OpenSslEncryptionFilterImpl::write(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        RCF_ASSERT_EQ(mPreState , Ready);
        mPreState = Writing;
        mPreByteBuffer = byteBuffers.front();
        readWrite();
    }

    void OpenSslEncryptionFilterImpl::onReadWriteCompleted(
        std::size_t bytesTransferred)
    {
        mByteBuffers.resize(0);
        mPostByteBuffer.clear();

        if (bytesTransferred == 0 && mReadRequested == 0 && mPreState == Reading)
        {
            mOpenSslEncryptionFilter.mpPreFilter->onReadCompleted(ByteBuffer());
        }
        else
        {

            // complete the data transfer
            onDataTransferred(bytesTransferred);

            if (mRetry)
            {
                retryReadWrite();
            }
            else
            {
                if (mPreState == Writing && BIO_ctrl_pending(mIoBio.get()) > 0)
                {
                    if (mUseRecursionLimiter)
                    {
                        applyRecursionLimiter(
                            mRecursionState, 
                            &OpenSslEncryptionFilterImpl::transferData, 
                            *this);
                    }
                    else
                    {
                        transferData();
                    }
                }
                else
                {
                    IoState state = mPreState;
                    mPreState = Ready;
                    if (state == Reading)
                    {
                        mPreByteBuffer = ByteBuffer(mPreByteBuffer, 0, mPos);

                        mOpenSslEncryptionFilter.mpPreFilter->onReadCompleted(
                            mPreByteBuffer);
                    }
                    else
                    {
                        mPreByteBuffer.clear();

                        mOpenSslEncryptionFilter.mpPreFilter->onWriteCompleted(
                            mPos);
                    }
                }
            }
        }
    }


    void OpenSslEncryptionFilterImpl::readWrite()
    {
        // set input parameters
        mRetry = true;
        mErr = 0;
        mPos = 0;
        retryReadWrite();
    }

    void OpenSslEncryptionFilterImpl::retryReadWrite()
    {
        RCF_ASSERT(mPreState == Reading || mPreState == Writing)(mPreState);

        int sslState = SSL_get_state(mSsl.get());
        if (!mHandshakeOk && sslState == SSL_ST_OK)
        {
            mHandshakeOk = true;

            mPeerCertPtr.reset(
                SSL_get_peer_certificate(mSsl.get()),
                X509_free);

            if (!mCaCertificate.empty())
            {
                // verify the peers certificate against our CA's
                int ret = SSL_get_verify_result(mSsl.get());
                bool verifyOk = (ret == X509_V_OK);

                if (!verifyOk)
                {
                    std::string openSslErr = SSL_get_verify_result_string(ret);

                    Exception e(
                        _RcfError_SslCertVerification(openSslErr), 
                        ret, 
                        RcfSubsystem_OpenSsl);

                    RCF_THROW(e);
                }
            }
            else if (mVerifyFunctor)
            {
                bool verifyOk = false;
                if (mPeerCertPtr)
                {
                    X509Certificate x509Cert(mPeerCertPtr.get());
                    verifyOk = mVerifyFunctor(&x509Cert);
                }
                else
                {
                    verifyOk = mVerifyFunctor(NULL);
                }
                
                if (!verifyOk)
                {
                    Exception e(
                        _RcfError_SslCertVerificationCustom());

                    RCF_THROW(e);
                }
            }
        }
        else if (mHandshakeOk && sslState != SSL_ST_OK)
        {
            mHandshakeOk = false;
        }
        

        int bioRet = (mPreState == Reading) ?
            BIO_read(mSslBio.get(), mPreByteBuffer.getPtr(), static_cast<int>(mReadRequested)) :
            BIO_write(mSslBio.get(), mPreByteBuffer.getPtr(), static_cast<int>(mPreByteBuffer.getLength()));


        RCF_ASSERT(
            -1 <= bioRet && bioRet <= static_cast<int>(mPreByteBuffer.getLength()))
            (bioRet)(mPreByteBuffer.getLength());

        if (bioRet == -1 && BIO_should_retry(mSslBio.get()))
        {
            // initiate io requests on underlying filters
            mRetry = true;
            
            if (mUseRecursionLimiter)
            {
                applyRecursionLimiter(
                    mRecursionState, 
                    &OpenSslEncryptionFilterImpl::transferData, 
                    *this);
            }
            else
            {
                transferData();
            }
        }
        else if (0 < bioRet && bioRet <= static_cast<int>(mPreByteBuffer.getLength()))
        {
            mRetry = false;
            mPos += bioRet;
            if (mPreState == Writing)
            {
                // TODO: maybe this is not always true
                RCF_ASSERT_GT(BIO_ctrl_pending(mIoBio.get()) , 0);
                
                if (mUseRecursionLimiter)
                {
                    applyRecursionLimiter(
                        mRecursionState, 
                        &OpenSslEncryptionFilterImpl::transferData, 
                        *this);
                }
                else
                {
                    transferData();
                }
            }
            else
            {
                mPreState = Ready;
                
                mPreByteBuffer = ByteBuffer(mPreByteBuffer, 0, mPos);

                mOpenSslEncryptionFilter.mpPreFilter->onReadCompleted(
                    mPreByteBuffer);
            }
        }
        else
        {
            mErr = -1;

            std::string opensslErrors = getOpenSslErrors();
            Exception e( _RcfError_OpenSslError(opensslErrors) );
            RCF_THROW(e);
        }
    }

    void OpenSslEncryptionFilterImpl::transferData()
    {
        if (BIO_ctrl_pending(mIoBio.get()) == 0)
        {
            // move data from network to mIoBio
            mPostState = Reading;
            mPostBufferRequested =
                static_cast<int>(BIO_ctrl_get_read_request(mIoBio.get()));

            mPostBufferLen = BIO_nwrite0(mIoBio.get(), &mPostBuffer);
           
            RCF_ASSERT_LTEQ(mPostBufferRequested , mPostBufferLen);

            // NB: completion routine will call BIO_nwrite(io_bio, len)
            mPostByteBuffer = ByteBuffer(mPostBuffer, mPostBufferLen);

            mOpenSslEncryptionFilter.mpPostFilter->read(
                mPostByteBuffer,
                mPostBufferRequested);
        }
        else
        {
            // move data from mIoBio to network
            mPostState = Writing;
            mPostBufferRequested = static_cast<int>(BIO_ctrl_pending(mIoBio.get()));
            mPostBufferLen = BIO_nread0(mIoBio.get(), &mPostBuffer);
            // NB: completion routine will call BIO_nread(mIoBio, postBufferLen)
            mByteBuffers.resize(0);
            mByteBuffers.push_back( ByteBuffer(mPostBuffer, mPostBufferLen));
            mOpenSslEncryptionFilter.mpPostFilter->write(mByteBuffers);            
        }
    }

    void OpenSslEncryptionFilterImpl::onDataTransferred(std::size_t bytesTransferred)
    {
        // complete a data transfer, in the direction that was requested

        // TODO: assert that, on read, data was actually transferred into postBuffer
        // and not somewhere else

        RCF_ASSERT_GT(bytesTransferred , 0);
        RCF_ASSERT(
            (mPostState == Reading && bytesTransferred <= mPostBufferRequested) ||
            (mPostState == Writing && bytesTransferred <= mPostBufferLen))
            (mPostState)(bytesTransferred)(mPostBufferRequested)(mPostBufferLen);

        if (mPostState == Reading)
        {
            // return value not documented
            BIO_nwrite(
                mIoBio.get(),
                &mPostBuffer,
                static_cast<int>(bytesTransferred));

            mPostBuffer = 0;
            mPostBufferLen = 0;
            mPostState = Ready;
        }
        else if (mPostState == Writing)
        {
            // return value not documented
            BIO_nread(
                mIoBio.get(),
                &mPostBuffer,
                static_cast<int>(bytesTransferred));

            mPostBuffer = 0;
            mPostBufferLen = 0;
            mPostState = Ready;
        }
    }

    void OpenSslEncryptionFilterImpl::reset()
    {
        init();
    }

    void OpenSslEncryptionFilterImpl::init()
    {
        // TODO: sort out any OpenSSL-dependent order of destruction issues

        mSslBio = boost::shared_ptr<BIO>(
            BIO_new(BIO_f_ssl()),
            BIO_free);

        mSslCtx = boost::shared_ptr<SSL_CTX>(
            SSL_CTX_new(SSLv23_method()),
            SSL_CTX_free);

        RCF_ASSERT(mSslRole == SslServer || mSslRole == SslClient)(mSslRole);
        
        if (!mCertificateFile.empty())
        {
            loadCertificate(mSslCtx, mCertificateFile, mCertificateFilePassword);
        }

        if (!mCaCertificate.empty())
        {
            loadCaCertificate(mSslCtx, mCaCertificate);
        }

        mSsl = boost::shared_ptr<SSL>(
            SSL_new(mSslCtx.get()),
            SSL_free);

        if (mSslRole == SslServer && !mCaCertificate.empty())
        {
            SSL_set_verify(mSsl.get(), SSL_VERIFY_PEER, NULL);
        }

        BIO *bio_temp = NULL;
        BIO *io_bio_temp = NULL;
        BIO_new_bio_pair(&bio_temp, mBioBufferSize, &io_bio_temp, mBioBufferSize);
        mBio = boost::shared_ptr<BIO>(
            bio_temp,
            BIO_free);
        mIoBio = boost::shared_ptr<BIO>(
            io_bio_temp,
            BIO_free);

        RCF_ASSERT(mSslRole == SslServer || mSslRole == SslClient)(mSslRole);
        mSslRole == SslServer ?
            SSL_set_accept_state(mSsl.get()) :
            SSL_set_connect_state(mSsl.get());

        SSL_set_bio(mSsl.get(), mBio.get(), mBio.get());
        BIO_set_ssl(mSslBio.get(), mSsl.get(), BIO_NOCLOSE);

        if (
            mSslCtx.get() == NULL ||
            mSsl.get() == NULL ||
            mBio.get() == NULL ||
            mIoBio.get() == NULL)
        {
            std::string opensslErrors = getOpenSslErrors();
            Exception e( _RcfError_OpenSslFilterInit(opensslErrors) );
            RCF_THROW(e);
        }

    }

    bool OpenSslEncryptionFilterImpl::loadCertificate(
        boost::shared_ptr<SSL_CTX> ctx,
        const std::string &file,
        const std::string &password)
    {
        RCF_ASSERT(ctx.get());
        if (1 == SSL_CTX_use_certificate_chain_file(ctx.get(), file.c_str()))
        {
            boost::shared_ptr<BIO> bio(
                BIO_new( BIO_s_file() ),
                BIO_free );
            if (bio.get())
            {
                if (1 == BIO_read_filename(bio.get(), file.c_str()))
                {
                    boost::shared_ptr<EVP_PKEY> evp(
                        PEM_read_bio_PrivateKey(
                            bio.get(),
                            NULL,
                            NULL,
                            (void *) password.c_str()),
                        EVP_PKEY_free);
                    if (evp.get())
                    {
                        if (1 == SSL_CTX_use_PrivateKey(ctx.get(), evp.get()))
                        {
                            return true;
                        }
                    }
                }
            }
        }
        std::string opensslErrors = getOpenSslErrors();
        Exception e(_RcfError_OpenSslLoadCert(file, opensslErrors));
        RCF_THROW(e)(file)(password);
        return false;
    }

    bool OpenSslEncryptionFilterImpl::loadCaCertificate(
        boost::shared_ptr<SSL_CTX> ctx,
        const std::string &file)
    {
        RCF_ASSERT(ctx.get());

        if (SSL_CTX_load_verify_locations(ctx.get(), file.c_str(), NULL) != 1)
        {
            std::string opensslErrors = getOpenSslErrors();
            Exception e(_RcfError_OpenSslLoadCert(file, opensslErrors));
            RCF_THROW(e);
        }
        return true;
    }

    OpenSslEncryptionFilterFactory::OpenSslEncryptionFilterFactory() :
            mRole(SslServer)
    {}

    FilterPtr OpenSslEncryptionFilterFactory::createFilter(RcfServer & server)
    {       
        std::string certificateFile;
        std::string certificateFilePassword;
        std::string caCertificate;

        CertificatePtr mCertPtr = server.getCertificate();
        PemCertificate * pPemCert = dynamic_cast<PemCertificate *>(mCertPtr.get());
        if (pPemCert)
        {
            certificateFile = pPemCert->mPathToCert;
            certificateFilePassword = pPemCert->mPassword;          
        }
        else if (mCertPtr && server.getSslImplementation() == Si_OpenSsl)
        {
            RCF_THROW( RCF::Exception( _RcfError_InvalidOpenSslCertificate() ));
        }

        mCertPtr = server.getCaCertificate();
        pPemCert = dynamic_cast<PemCertificate *>(mCertPtr.get());
        if (pPemCert)
        {
            caCertificate = pPemCert->mPathToCert;
        }
        else if (mCertPtr && server.getSslImplementation() == Si_OpenSsl)
        {
            RCF_THROW( RCF::Exception( _RcfError_InvalidOpenSslCertificate() ));
        }

        std::string ciphers = server.getOpenSslCipherSuite();

        const CertificateValidationCb & certValidationCb = server.getCertificateValidationCallback();

        return FilterPtr( new OpenSslEncryptionFilter(
            certificateFile,
            certificateFilePassword,
            caCertificate,
            ciphers,
            certValidationCb,
            mRole));
    }

    int OpenSslEncryptionFilterFactory::getFilterId()
    {
        return RcfFilter_OpenSsl;
    }

    int OpenSslEncryptionFilter::getFilterId() const
    {
        return RcfFilter_OpenSsl;
    }

} // namespace RCF
