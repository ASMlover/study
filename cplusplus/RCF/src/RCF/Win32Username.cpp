
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

#include <RCF/Win32Username.hpp>

#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif

#include <windows.h>
#include <Sspi.h>
#include <secext.h>

#include <RCF/Exception.hpp>

namespace RCF {

    #ifdef UNICODE
    LPCSTR GetUserNameExName = "GetUserNameExW";
#else
    LPCSTR GetUserNameExName = "GetUserNameExA";
#endif

    typedef BOOLEAN (WINAPI *PfnGetUserNameEx)(EXTENDED_NAME_FORMAT, LPTSTR, PULONG);
    HMODULE hModuleSecur32 = 0;
    PfnGetUserNameEx pfnGetUserNameEx = NULL;
   

    void initPfnGetUserName()
    {
        // try to load the GetUserNameEx() function, if we can
        hModuleSecur32 = LoadLibrary( RCF_T("secur32.dll"));
        if (hModuleSecur32)
        {
            pfnGetUserNameEx = (PfnGetUserNameEx) GetProcAddress(hModuleSecur32, GetUserNameExName);
        }
    }

    void deinitPfnGetUserName()
    {
        if (hModuleSecur32)   
        {
            FreeLibrary(hModuleSecur32);
            hModuleSecur32 = 0;
            pfnGetUserNameEx = NULL;
        }
    }

    tstring getMyUserName()
    {
        std::vector<TCHAR> vec;
        DWORD len = 0;
        BOOL ret = GetUserName(NULL, &len);
        BOOL err = 0;
        vec.resize(len);
        ret = GetUserName(&vec[0], &len);
        err = GetLastError();
        RCF_VERIFY(
            ret,
            Exception(
                _RcfError_Win32ApiError("GetUserName()"),
                err,
                RcfSubsystem_Os));
        return tstring(&vec[0]);
    }

    tstring getMyDomain()
    {
        if (pfnGetUserNameEx)
        {
            ULONG count = 0;
            pfnGetUserNameEx(NameSamCompatible, NULL, &count);
            std::vector<TCHAR> vec(count);
            BOOLEAN ok = pfnGetUserNameEx(NameSamCompatible, &vec[0], &count);
            DWORD dwErr = GetLastError();

            RCF_VERIFY(
                ok,
                Exception(
                _RcfError_Win32ApiError("GetUserNameEx()"),
                dwErr,
                RcfSubsystem_Os));

            tstring domainAndUser(&vec[0]);
            tstring domain = domainAndUser.substr(
                0,
                domainAndUser.find('\\'));
            return domain;
        }
        else
        {
            // GetUserNameEx() is not available on older Windows versions, so
            // here's the alternative.

            // This code may fail if we are impersonating another user, and our
            // Windows privileges aren't appropriately enabled. OpenThreadToken()
            // fails with "Access denied".

            using namespace boost::multi_index::detail;

            // obtain current token
            HANDLE hToken;
            BOOL ok = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken);
            DWORD dwErr1 = GetLastError();
            DWORD dwErr2 = 0;
            if (!ok)
            {
                ok = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
                dwErr2 = GetLastError();
            }

            RCF_VERIFY(
                ok,
                Exception(
                    _RcfError_Win32ApiError("OpenProcessToken()"),
                    dwErr2,
                    RcfSubsystem_Os))(dwErr1);

            scope_guard guard = make_guard(&CloseHandle, hToken);
            RCF_UNUSED_VARIABLE(guard);

            PTOKEN_USER ptiUser     = NULL;
            DWORD       cbti        = 0;

            // find the length of the token information buffer
            GetTokenInformation(hToken, TokenUser, NULL, 0, &cbti);

            // allocate buffer for token information
            std::vector<char> vec(cbti);
            ptiUser = (PTOKEN_USER) &vec[0];

            // obtain token information
            GetTokenInformation(hToken, TokenUser, ptiUser, cbti, &cbti);

            // extract domain and username
            TCHAR    szDomain[256];           
            DWORD    szDomainLen = sizeof(szDomain)/sizeof(szDomain[0]);

            TCHAR    szUsername[256];           
            DWORD    szUsernameLen = sizeof(szUsername)/sizeof(szUsername[0]);

            SID_NAME_USE snu;

            ok = LookupAccountSid(
                NULL, ptiUser->User.Sid,
                szUsername, &szUsernameLen,
                szDomain, &szDomainLen,
                &snu);

            DWORD err = GetLastError();

            RCF_VERIFY(
                ok,
                Exception(
                    _RcfError_Win32ApiError("LookupAccountSid()"),
                    err,
                    RcfSubsystem_Os));

            return szDomain;
        }
    }

    tstring getMyMachineName()
    {
        const int BufferSize = MAX_COMPUTERNAME_LENGTH + 1;
        TCHAR buffer[BufferSize];
        DWORD dwSize = sizeof(buffer)/sizeof(buffer[0]);
        BOOL ok = GetComputerName(buffer, &dwSize);
        RCF_ASSERT(ok);
        RCF_UNUSED_VARIABLE(ok);
        return tstring(&buffer[0]);
    }


} // namespace RCF
