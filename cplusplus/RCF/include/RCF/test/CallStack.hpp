
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

// This call stack implementation is used by CMemLeakDetect. It is faster than
// other code I've tried, however it does not appear to work correctly in release
// builds, and does not work at all in x64.

// StackWalker from CodeProject works in debug/release, x86/x64, but is slow (too
// slow to be called on every memory allocation).

#pragma once

#include <windows.h>
#include <dbghelp.h>
#include <crtdbg.h>

// if you want to use the custom stackwalker otherwise
// comment this line out
#define MLD_CUSTOMSTACKWALK         1
//
#define MLD_MAX_NAME_LENGTH         256
#define MLD_MAX_TRACEINFO           256
#define MLD_TRACEINFO_EMPTY         _T("")
#define MLD_TRACEINFO_NOSYMBOL      _T("?(?)")

#ifdef  MLD_CUSTOMSTACKWALK
#define MLD_STACKWALKER             symStackTrace2
#else
#define MLD_STACKWALKER             symStackTrace
#endif

#if !defined(_MSC_VER) || defined(_WIN64)

class CallStack 
{
public:

    void                        capture()
    {
    }

    std::basic_string<TCHAR>    toString()
    {
        return _T("<Call stack unavailable>");
    }

    static void         initSymbols()
    {
    }

    static void         deinitSymbols()
    {
    }
};

#else

typedef DWORD ADDR;

struct CallStackFrameEntry {
    ADDRESS             addrPC;
    ADDRESS             addrFrame;
};

class CallStack 
{
public:

    CallStack()
    {
        memset(traceinfo, 0, MLD_MAX_TRACEINFO * sizeof(CallStackFrameEntry));
    }

    void                        capture();
    std::basic_string<TCHAR>    toString();

    static void         initSymbols();
    static void         deinitSymbols();

private:

    friend class CMemLeakDetect;

    CallStackFrameEntry     traceinfo[MLD_MAX_TRACEINFO];

    static HANDLE               m_hProcess;
    static PIMAGEHLP_SYMBOL     m_pSymbol;
    static DWORD                m_dwsymBufSize;

    static BOOL         initSymInfo(TCHAR* lpUserPath);
    static BOOL         cleanupSymInfo();
    static void         symbolPaths( TCHAR* lpszSymbolPaths, UINT BufSizeTCHARs);
    static void         symStackTrace(CallStackFrameEntry* pStacktrace);
    static void         symStackTrace2(CallStackFrameEntry* pStacktrace);
    static BOOL         symFunctionInfoFromAddresses(ULONG fnAddress, ULONG stackAddress, TCHAR* lpszSymbol, UINT BufSizeTCHARs);
    static BOOL         symSourceInfoFromAddress(UINT address, TCHAR* lpszSourceInfo, UINT BufSizeTCHARs);
    static BOOL         symModuleNameFromAddress(UINT address, TCHAR* lpszModule, UINT BufSizeTCHARs);
};

#endif
