
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

#include <RCF/util/NtService.hpp>

#include <iostream>

#include <RCF/Exception.hpp>

#include "tchar.h"

namespace RCF {

    NtService::NtService(
        const std::string & serviceName, 
        const std::string & serviceDisplayName) :
            mServiceName(serviceName),
            mServiceDisplayName(serviceDisplayName),
            mhServiceStatus(NULL),
            mStopFlag(false),
            mLog("c:\\ServiceLog.log"),
            mRunningAsService(true)
    {
        spThis = this;

        mStatus.dwServiceType               = SERVICE_WIN32_OWN_PROCESS;
        mStatus.dwCurrentState              = SERVICE_STOPPED;
        mStatus.dwControlsAccepted          = SERVICE_ACCEPT_STOP;
        mStatus.dwWin32ExitCode             = 0;
        mStatus.dwServiceSpecificExitCode   = 0;
        mStatus.dwCheckPoint                = 0;
        mStatus.dwWaitHint                  = 0;
    }

    int NtService::RunAsServiceOrInteractive()
    {
        SERVICE_TABLE_ENTRY st[] =
        {
            { (LPSTR) mServiceName.c_str(), _ServiceMain },
            { NULL, NULL }
        };

        // Run as service if we can, otherwise run as interactive process.
        if (IsInstalled())
        {
            // Try running as service
            if (!StartServiceCtrlDispatcher(st))
            {
                // Run as interactive process.
                mRunningAsService = false;
                return Start();
            }
            else
            {
                return 0;
            }
        }
        else
        {
            // Run as interactive process.
            mRunningAsService = false;
            return Start();
        }
    }

    void NtService::SetServiceStatus(DWORD dwState, DWORD waitHint)
    {
        mStatus.dwCurrentState = dwState;

        if (   dwState == SERVICE_CONTINUE_PENDING
            || dwState == SERVICE_STOP_PENDING
            || dwState == SERVICE_PAUSE_PENDING )
        {
            if ( waitHint > 0 )
            {
                mStatus.dwWaitHint = waitHint;
            }
            mStatus.dwCheckPoint++;
        }
        else
        {
            mStatus.dwCheckPoint = 0;
        }

        if ( !::SetServiceStatus(mhServiceStatus, &mStatus) )
        {
            // TODO: error handling (event log).
            // ...
        }
    }

    bool NtService::Install()
    {
        tstring commandLineArgs;
        return Install(commandLineArgs);
    }

    bool NtService::Install(const tstring & commandLineArgs)
    {
        if (IsInstalled())
        {
            std::cout << "Service already installed." << std::endl;
            return true;
        }

        SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (hSCM == NULL)
        {
            DWORD dwErr = GetLastError();
            std::cout << "Unable to open service manager. " + RCF::getErrorString(dwErr) << std::endl;
            return false;
        }

        // Get the executable file path
        TCHAR szFilePath[_MAX_PATH];
        GetModuleFileName(NULL, szFilePath, _MAX_PATH);

        tstring commandLine = szFilePath;
        if (commandLineArgs.size() > 0)
        {
            commandLine += _T(" ");
            commandLine += commandLineArgs;
        }

        SC_HANDLE hService = CreateService(
            hSCM,
            mServiceName.c_str(),
            mServiceDisplayName.c_str(),
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_NORMAL,
            commandLine.c_str(),
            NULL, NULL, 
            NULL,
            NULL, NULL);

        if (hService == NULL)
        {
            DWORD dwErr = GetLastError();
            std::cout << "Unable to create service. " + RCF::getErrorString(dwErr) << std::endl;
            CloseServiceHandle(hSCM);
            return false;
        }

        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        std::cout << "Service installed." << std::endl;
        return true;
    }

    bool NtService::Uninstall()
    {
        if (!IsInstalled())
        {
            std::cout << "Service not installed." << std::endl;
            return true;
        }

        SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

        if (hSCM == NULL)
        {
            DWORD dwErr = GetLastError();
            std::cout << "Unable to open service manager. " + RCF::getErrorString(dwErr) << std::endl;
            return false;
        }

        SC_HANDLE hService = OpenService(hSCM, mServiceName.c_str(), SERVICE_STOP | DELETE);

        if (hService == NULL)
        {
            DWORD dwErr = GetLastError();
            std::cout << "Unable to open service. " + RCF::getErrorString(dwErr) << std::endl;
            CloseServiceHandle(hSCM);
            return false;
        }

        SERVICE_STATUS status;
        ControlService(hService, SERVICE_CONTROL_STOP, &status);

        BOOL bDelete = DeleteService(hService);
        if ( !bDelete )
        {
            DWORD dwErr = GetLastError();
            std::cout << "Unable to delete service. " + RCF::getErrorString(dwErr) << std::endl;
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCM);
            return false;
        }

        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);

        std::cout << "Service uninstalled." << std::endl;
        return true;

    }

    bool NtService::IsInstalled() const
    {
        bool found = false;

        SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

        if (hSCM != NULL)
        {
            SC_HANDLE hService = OpenService(hSCM, mServiceName.c_str(), SERVICE_QUERY_CONFIG);
            if (hService != NULL)
            {
                found = true;
                CloseServiceHandle(hService);
            }
            CloseServiceHandle(hSCM);
        }
        return found;
    }

    void NtService::ServiceMain(DWORD dwArgc, LPTSTR * lpszArgv)
    {
        mhServiceStatus = RegisterServiceCtrlHandler(mServiceName.c_str(), _Handler);
        if (mhServiceStatus == NULL)
        {
            // TODO: error handling. Write to event log.
            // ...

            return;
        }

        SetServiceStatus(SERVICE_START_PENDING);

        SetServiceStatus(SERVICE_RUNNING);

        Start();

        SetServiceStatus(SERVICE_STOPPED);
    }

    void NtService::Handler(DWORD dwOpcode)
    {
        switch (dwOpcode)
        {
        case SERVICE_CONTROL_STOP:
            SetServiceStatus(SERVICE_STOP_PENDING, 0);
            mStopFlag = true;
            break;

        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_CONTINUE:
        case SERVICE_CONTROL_INTERROGATE:
        case SERVICE_CONTROL_SHUTDOWN:
        default:
            break;
        }
    }

    NtService * NtService::spThis;

    static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
    {
        NtService::spThis->ServiceMain(dwArgc, lpszArgv);
    }

    static void WINAPI _Handler(DWORD dwOpcode)
    {
        NtService::spThis->Handler(dwOpcode);
    }

} // namespace RCF
