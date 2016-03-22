
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

#ifndef INCLUDE_UTIL_NTSERVICE_HPP
#define INCLUDE_UTIL_NTSERVICE_HPP

#include <Windows.h>
#include <fstream>

#include <RCF/util/Tchar.hpp>

namespace RCF {

    class NtService
    {
    public:

        NtService(
            const std::string & serviceName, 
            const std::string & serviceDisplayName);

        virtual ~NtService()
        {}

        int             RunAsServiceOrInteractive();        
        void            SetServiceStatus(DWORD dwState, DWORD waitHint = 0);
        bool            Install();
        bool            Install(const tstring & commandLineArgs);
        bool            Uninstall();
        bool            IsInstalled() const;

    private:
        
        void            ServiceMain(DWORD dwArgc, LPTSTR * lpszArgv);

        virtual int     Start() = 0;
        virtual void    Handler(DWORD dwOpcode);

        friend static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
        friend static void WINAPI _Handler(DWORD dwOpcode);
        static                    NtService * spThis;

    protected:

        std::string             mServiceName;
        std::string             mServiceDisplayName;

        SERVICE_STATUS_HANDLE   mhServiceStatus;
        SERVICE_STATUS          mStatus;

        volatile bool           mStopFlag;
        bool                    mRunningAsService;
        std::ofstream           mLog;
    };

} // namespace RCF

#endif // ! INCLUDE_UTIL_NTSERVICE_HPP

// Sample usage.
/*
#include <RCF/util/NtService.hpp>

#include <RCF/util/../../../src/RCF/util/NtService.cpp>

class MyService : public RCF::NtService
{
public:

    MyService() : RCF::NtService("MyService", "My Really Useful Service")
    {
    }

    void Start()
    {
        std::ofstream fout("c:\\serviceOut.log");
        int n = 0;
        while (!mStopFlag)
        {
            fout << ++n << std::endl;
            Sleep(1000);
        }
    }
};

MyService gService;

int main(int argc, char ** argv)
{
    bool install = false;
    bool uninstall = false;

    // TODO: parse the command line args.
    // ...

    if (install)
    {
        gService.Install();
        return 0;
    }
    else if (uninstall)
    {
        gService.Uninstall();
        return 0;
    }

    gService.RunAsServiceOrInteractive();
    return 0;
}
*/
