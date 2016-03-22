
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

#include "CallStack.hpp"

#include <cassert>
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <vector>
#include <deque>

#if defined(_MSC_VER)
#pragma comment(lib, "dbghelp.lib")
#endif

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4995 )  // warning C4995: 'sprintf': name was marked as #pragma deprecated
#pragma warning( disable : 4996 )  // warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
#endif

HANDLE              CallStack::m_hProcess;
PIMAGEHLP_SYMBOL    CallStack::m_pSymbol;
DWORD               CallStack::m_dwsymBufSize;

void CallStack::capture()
{
    MLD_STACKWALKER(&traceinfo[0]);
}

void CallStack::initSymbols()
{
    m_dwsymBufSize        = (MLD_MAX_NAME_LENGTH + sizeof(PIMAGEHLP_SYMBOL));
    m_hProcess            = GetCurrentProcess();
    m_pSymbol             = (PIMAGEHLP_SYMBOL)GlobalAlloc( GMEM_FIXED, m_dwsymBufSize);

    initSymInfo( NULL);
}

void CallStack::deinitSymbols()
{
    cleanupSymInfo();
    GlobalFree(m_pSymbol);
}

std::basic_string<TCHAR> CallStack::toString()
{
    TCHAR               symInfo[MLD_MAX_NAME_LENGTH];
    TCHAR               srcInfo[MLD_MAX_NAME_LENGTH];
    int                 totalSize                       = 0;
    CallStackFrameEntry*    p                           = 0;

    // Make sure initSymbols() has been called.
    assert(m_pSymbol);

    std::basic_string<TCHAR> stackTrace;

    p = &traceinfo[0];

    while(p[0].addrPC.Offset)
    {
        symFunctionInfoFromAddresses(p[0].addrPC.Offset, p[0].addrFrame.Offset, symInfo, MLD_MAX_NAME_LENGTH);
        symSourceInfoFromAddress(p[0].addrPC.Offset, srcInfo, MLD_MAX_NAME_LENGTH);

        TCHAR buffer[3*MLD_MAX_NAME_LENGTH] = {0};
        _stprintf(buffer, _T("%s->%s()\n"), srcInfo, symInfo);
        stackTrace += buffer;
        p++;
    }

    return stackTrace;
}

void CallStack::symbolPaths( TCHAR* lpszSymbolPath, UINT BufSizeTCHARs)
{
    TCHAR lpszPath[MLD_MAX_NAME_LENGTH];

    // Creating the default path where the dgbhelp.dll is located
    // ".;%_NT_SYMBOL_PATH%;%_NT_ALTERNATE_SYMBOL_PATH%;%SYSTEMROOT%;%SYSTEMROOT%\System32;"
    _tcscpy_s(lpszSymbolPath, BufSizeTCHARs, _T(".;..\\;..\\..\\"));

    // environment variable _NT_SYMBOL_PATH
    if ( GetEnvironmentVariable(_T("_NT_SYMBOL_PATH"), lpszPath, MLD_MAX_NAME_LENGTH ))
    {
        _tcscat_s( lpszSymbolPath, BufSizeTCHARs, _T(";"));
        _tcscat_s( lpszSymbolPath, BufSizeTCHARs, lpszPath );
    }

    // environment variable _NT_ALTERNATE_SYMBOL_PATH
    if ( GetEnvironmentVariable( _T("_NT_ALTERNATE_SYMBOL_PATH"), lpszPath, MLD_MAX_NAME_LENGTH ))
    {
        _tcscat_s( lpszSymbolPath, BufSizeTCHARs, _T(";"));
        _tcscat_s( lpszSymbolPath, BufSizeTCHARs, lpszPath );
    }

    // environment variable SYSTEMROOT
    if ( GetEnvironmentVariable( _T("SYSTEMROOT"), lpszPath, MLD_MAX_NAME_LENGTH ) )
    {
        _tcscat_s( lpszSymbolPath, BufSizeTCHARs, _T(";"));
        _tcscat_s( lpszSymbolPath, BufSizeTCHARs, lpszPath);
        _tcscat_s( lpszSymbolPath, BufSizeTCHARs, _T(";"));

        // SYSTEMROOT\System32
        _tcscat_s( lpszSymbolPath, BufSizeTCHARs, lpszPath );
        _tcscat_s( lpszSymbolPath, BufSizeTCHARs, _T("\\System32"));
    }
}

BOOL CallStack::cleanupSymInfo()
{
    return SymCleanup( GetCurrentProcess() );
}

// Initializes the symbol files
BOOL CallStack::initSymInfo( TCHAR* lpszUserSymbolPath)
{
    TCHAR    lpszSymbolPath[MLD_MAX_NAME_LENGTH];
    DWORD   symOptions = SymGetOptions();

    symOptions |= SYMOPT_LOAD_LINES; 
    symOptions &= ~SYMOPT_UNDNAME;
    SymSetOptions( symOptions );

    // Get the search path for the symbol files
    symbolPaths( lpszSymbolPath, MLD_MAX_NAME_LENGTH);
    //
    if (lpszUserSymbolPath)
    {
        _tcscat_s(lpszSymbolPath, MLD_MAX_NAME_LENGTH, _T(";"));
        _tcscat_s(lpszSymbolPath, MLD_MAX_NAME_LENGTH, lpszUserSymbolPath);
    }

#ifdef UNICODE
    int len = _tcslen(lpszSymbolPath) + 1 ;
    char dest[1024] ;
    WideCharToMultiByte( CP_ACP, 0, lpszSymbolPath, -1, dest, len, NULL, NULL );
    BOOL bret = SymInitialize( GetCurrentProcess(), dest, TRUE);
#else
    BOOL bret = SymInitialize( GetCurrentProcess(), lpszSymbolPath, TRUE) ;
#endif

    return bret ;
}

void CallStack::symStackTrace(CallStackFrameEntry* pStacktrace )
{
    STACKFRAME     callStack;
    BOOL           bResult;
    CONTEXT        context;
    HANDLE         hThread  = GetCurrentThread();

    // get the context
    memset( &context, NULL, sizeof(context) );
    context.ContextFlags = CONTEXT_FULL;
    if ( !GetThreadContext( hThread, &context ) )
    {
        assert(0);
        return;
    }
    //initialize the call stack
    memset( &callStack, NULL, sizeof(callStack) );
    callStack.AddrPC.Offset    = context.Eip;
    callStack.AddrStack.Offset = context.Esp;
    callStack.AddrFrame.Offset = context.Ebp;
    callStack.AddrPC.Mode      = AddrModeFlat;
    callStack.AddrStack.Mode   = AddrModeFlat;
    callStack.AddrFrame.Mode   = AddrModeFlat;
    //
    for( DWORD index = 0; index < MLD_MAX_TRACEINFO; index++ ) 
    {

        bResult = StackWalk(IMAGE_FILE_MACHINE_I386,
            m_hProcess,
            hThread,
            &callStack,
            NULL, 
            NULL,
            SymFunctionTableAccess,
            SymGetModuleBase,
            NULL);

        //if ( index == 0 )
        //  continue;

        if( !bResult || callStack.AddrFrame.Offset == 0 ) 
            break;
        //
        pStacktrace[0].addrPC    = callStack.AddrPC;
        pStacktrace[0].addrFrame = callStack.AddrFrame;
        pStacktrace++;
    }
    //clear the last entry
    memset(pStacktrace, NULL, sizeof(CallStackFrameEntry));
}

//
// This code is still under investigation
// I have to test this code and make sure it is compatible
// with the other stack walker!
//
void CallStack::symStackTrace2(CallStackFrameEntry* pStacktrace )
{
    ADDR            FramePtr                = NULL;
    ADDR            InstructionPtr          = NULL;
    ADDR            OriFramePtr             = NULL;
    ADDR            PrevFramePtr            = NULL;
    long            StackIndex              = NULL;

#if defined(_WIN64)
    assert(0 && "Call stack functionality not implemented on x64")
#endif

    // Get frame pointer
    _asm mov DWORD PTR [OriFramePtr], ebp

    FramePtr = OriFramePtr;

    //
    while (FramePtr)
    {
        InstructionPtr = ((ADDR *)FramePtr)[1];

        pStacktrace[StackIndex].addrPC.Offset   = InstructionPtr;
        pStacktrace[StackIndex].addrPC.Segment  = NULL;
        pStacktrace[StackIndex].addrPC.Mode     = AddrModeFlat;
        //
        StackIndex++;
        PrevFramePtr            = FramePtr;
        FramePtr                = ((ADDR *)FramePtr)[0];
    }
}

BOOL CallStack::symFunctionInfoFromAddresses(ULONG fnAddress, ULONG stackAddress, /*LPTSTR*/TCHAR * lpszSymbol, 
    UINT BufSizeTCHARs)
{
    stackAddress;

    DWORD             dwDisp    = 0;

    ::ZeroMemory(m_pSymbol, m_dwsymBufSize );
    m_pSymbol->SizeOfStruct     = m_dwsymBufSize;
    m_pSymbol->MaxNameLength    = m_dwsymBufSize - sizeof(IMAGEHLP_SYMBOL);
    // Set the default to unknown
    _tcscpy_s( lpszSymbol, BufSizeTCHARs, MLD_TRACEINFO_NOSYMBOL);

    // Get symbol info for IP
    if ( SymGetSymFromAddr( m_hProcess, (ULONG)fnAddress, &dwDisp, m_pSymbol ) )
    {
#ifdef UNICODE
        int len = strlen(m_pSymbol->Name) + 1 ;
        wchar_t dest[1024] = {};
        MultiByteToWideChar(CP_ACP, 0, m_pSymbol->Name, len, dest, len );
        _tcscpy_s(lpszSymbol, BufSizeTCHARs, dest);
#else
        _tcscpy_s(lpszSymbol, BufSizeTCHARs, m_pSymbol->Name);
#endif
        return TRUE;
    }

    //create the symbol using the address because we have no symbol
    _stprintf_s(lpszSymbol, BufSizeTCHARs, _T("0x%08X"), fnAddress);
    return FALSE;
}

BOOL CallStack::symSourceInfoFromAddress(UINT address, TCHAR* lpszSourceInfo, UINT BufSizeTCHARs)
{
    BOOL           ret = FALSE;
    IMAGEHLP_LINE  lineInfo;
    DWORD          dwDisp;
    TCHAR          lpModuleInfo[MLD_MAX_NAME_LENGTH] = MLD_TRACEINFO_EMPTY;

    _tcscpy_s(lpszSourceInfo, BufSizeTCHARs, MLD_TRACEINFO_NOSYMBOL);

    memset( &lineInfo, NULL, sizeof( IMAGEHLP_LINE ) );
    lineInfo.SizeOfStruct = sizeof( IMAGEHLP_LINE );

    if ( SymGetLineFromAddr( m_hProcess, address, &dwDisp, &lineInfo ) )
    {
        // Using the "sourcefile(linenumber)" format
#ifdef UNICODE
        wchar_t dest[1024] ;
        int len = strlen((char *)lineInfo.FileName) + 1 ;
        MultiByteToWideChar(CP_ACP, 0, (char *)lineInfo.FileName, len, dest, len) ;
        _stprintf_s(lpszSourceInfo, BufSizeTCHARs, _T("%s(%d): 0x%08X"), dest, lineInfo.LineNumber, address );//    <--- Size of the char thing.
#else
        _stprintf_s(lpszSourceInfo, BufSizeTCHARs, _T("%s(%d): 0x%08X"), lineInfo.FileName, lineInfo.LineNumber, address );//   <--- Size of the char thing.
#endif
        ret = TRUE;
    }
    else
    {
        // Using the "modulename!address" format
        symModuleNameFromAddress( address, lpModuleInfo, MLD_MAX_NAME_LENGTH);

        if ( lpModuleInfo[0] == _T('?') || lpModuleInfo[0] == _T('\0'))
        {
            // Using the "address" format
            _stprintf_s(lpszSourceInfo, BufSizeTCHARs, _T("0x%08X"), lpModuleInfo, address );
        }
        else
        {
            _stprintf_s(lpszSourceInfo, BufSizeTCHARs, _T("%sdll! 0x%08X"), lpModuleInfo, address );
        }
        ret = FALSE;
    }
    //
    return ret;
}

BOOL CallStack::symModuleNameFromAddress( UINT address, TCHAR* lpszModule, UINT BufSizeTCHARs)
{
    BOOL              ret = FALSE;
    IMAGEHLP_MODULE   moduleInfo;

    ::ZeroMemory( &moduleInfo, sizeof(IMAGEHLP_MODULE) );
    moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

    if ( SymGetModuleInfo( m_hProcess, (DWORD)address, &moduleInfo ) )
    {
        // Note. IMAGEHLP_MODULE::ModuleName seems to be hardcoded as 32 char/wchar_t (VS2008).
#ifdef UNICODE
        int len = _tcslen(lpszModule) + 1 ;
        char dest[1024] ;
        WideCharToMultiByte( CP_ACP, 0, lpszModule, -1, dest, len, NULL, NULL );

        strcpy_s(moduleInfo.ModuleName, 32, dest);  //  ANSI!
#else
        strcpy_s(moduleInfo.ModuleName, 32, lpszModule);
#endif
        ret = TRUE;
    }
    else
    {
        _tcscpy_s( lpszModule, BufSizeTCHARs, MLD_TRACEINFO_NOSYMBOL);
    }

    return ret;
}

#if defined(_MSC_VER)
#pragma warning( pop )
#endif
