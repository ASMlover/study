
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

/*************************************************************
 Author     : David A. Jones
 File Name  : MemLeakDetect.h
 Date       : July 30, 2004
 Synopsis        
            A trace memory feature for source code to trace and
            find memory related bugs. 


****************************************************************/
// July 2009: tim.s.stevens@bt.com : Modified it to work with ANSI or UNICODE. Based on:
// http://www.codeproject.com/cpp/MemLeakDetect.asp

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

#include <RCF/Tools.hpp>

#include <iostream>
#include <tchar.h>
#include <vector>
#include <deque>

#include "MemLeakDetect.h"

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4995 )  // warning C4995: 'sprintf': name was marked as #pragma deprecated
#pragma warning( disable : 4996 )  // warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
#endif

#if defined(_DEBUG)

static CMemLeakDetect*  g_pMemTrace         = NULL;
static _CRT_ALLOC_HOOK  pfnOldCrtAllocHook  = NULL;

int catchMemoryAllocHook(int    allocType, 
                         void   *userData, 
                         size_t size, 
                         int    blockType, 
                         long   requestNumber, 
          const unsigned char   *filename, // Can't be UNICODE
                         int    lineNumber)
{
    _CrtMemBlockHeader *pCrtHead;
    long prevRequestNumber;
#ifdef UNICODE
    wchar_t Wname[1024] ;
    Wname[0] = '\0' ;
#endif
    // internal C library internal allocations
    if ( blockType == _CRT_BLOCK )
    {
        return( TRUE );
    }

    // check if someone has turned off mem tracing
    
    // CRT memory tracking can be turned off by the app. MFC sometimes does this temporarily.
    
    if  ((( _CRTDBG_ALLOC_MEM_DF & _crtDbgFlag) == 0) 
        &&      (   ( allocType         == _HOOK_ALLOC)     
                ||  ( allocType         == _HOOK_REALLOC)))
    {
        if (pfnOldCrtAllocHook)
        {
            pfnOldCrtAllocHook(allocType, userData, size, blockType, requestNumber, filename, lineNumber);
        }
        return TRUE;
    }

    // protect if mem trace is not initialized
    if (g_pMemTrace == NULL)
    {
        if (pfnOldCrtAllocHook)
        {
            pfnOldCrtAllocHook(allocType, userData, size, blockType, requestNumber, filename, lineNumber);
        }
        return TRUE;
    }

    // protect internal mem trace allocs
    if (g_pMemTrace->isLocked)
    {
        if (pfnOldCrtAllocHook)
        {
            pfnOldCrtAllocHook(allocType, userData, size, blockType, requestNumber, filename, lineNumber);
        }
        return( TRUE);
    }
    // lock the function
    g_pMemTrace->isLocked = true;
    //
#ifdef UNICODE
    int len ;
    if (NULL != filename)
    {
        len = strlen((char *)filename) + 1 ;
        MultiByteToWideChar(CP_ACP, 0, (char *)filename, len, Wname, len) ;
    }
    else
        len = 0 ;
#else
    #define Wname (char*)filename
#endif
    if (allocType == _HOOK_ALLOC)
    {

        g_pMemTrace->addMemoryTrace((void *) requestNumber, size, Wname, lineNumber);
    }
    else
    if (allocType == _HOOK_REALLOC)
    {
        if (_CrtIsValidHeapPointer(userData))
        {
            pCrtHead = pHdr(userData);
            prevRequestNumber = pCrtHead->lRequest;
            //
            //if (pCrtHead->nBlockUse == _IGNORE_BLOCK)
            //{
            //  if (pfnOldCrtAllocHook)
            //  {
            //      pfnOldCrtAllocHook(allocType, userData, size, blockType, requestNumber, filename, lineNumber);
            //  }
            //  goto END;
            //}
            g_pMemTrace->redoMemoryTrace((void *) requestNumber, (void *) prevRequestNumber, size, Wname, lineNumber);
        }
    }
    else
    if (allocType == _HOOK_FREE)
    {
        if (_CrtIsValidHeapPointer(userData))
        {
            pCrtHead = pHdr(userData);
            requestNumber = pCrtHead->lRequest;
            //
            //if (pCrtHead->nBlockUse == _IGNORE_BLOCK)
            //{
            //  if (pfnOldCrtAllocHook)
            //  {
            //      pfnOldCrtAllocHook(allocType, userData, size, blockType, requestNumber, filename, lineNumber);
            //  }
            //  goto END;
            //}
            g_pMemTrace->removeMemoryTrace((void *) requestNumber, userData);
        }
    }
//END:
    // unlock the function
    g_pMemTrace->isLocked = false;
    return TRUE;
}


void CMemLeakDetect::addMemoryTrace(void* addr,  DWORD asize,  TCHAR *fname, DWORD lnum)
{
    AllocBlockInfo ainfo;
    //
    if (m_AllocatedMemoryList.Lookup(addr, ainfo))
    {
        // already allocated
        assert(0);
        return;
    }
    //
    ainfo.address       = addr;
    ainfo.lineNumber    = lnum;
    ainfo.size          = asize;
    ainfo.occurance     = memoccurance++;
    ainfo.capture();
    //
    if (fname)
        _tcsncpy_s(&ainfo.fileName[0], MLD_MAX_NAME_LENGTH, fname, MLD_MAX_NAME_LENGTH);
    else
      ainfo.fileName[0] = 0;
    //
    m_AllocatedMemoryList.SetAt(addr, ainfo);
};
void CMemLeakDetect::redoMemoryTrace(void* addr,  void* oldaddr, DWORD asize,  /*char*/TCHAR *fname, DWORD lnum)
{
    AllocBlockInfo ainfo;

    if (m_AllocatedMemoryList.Lookup(oldaddr,(AllocBlockInfo &) ainfo))
    {
        m_AllocatedMemoryList.RemoveKey(oldaddr);
    }
    else
    {
        // Didn't find it, just means we didn't track the allocation of this piece of memory.
        // ...
    }
    //
    ainfo.address       = addr;
    ainfo.lineNumber    = lnum;
    ainfo.size          = asize;
    ainfo.occurance     = memoccurance++;

    ainfo.capture();
    //
    if (fname)
        _tcsncpy_s(&ainfo.fileName[0], MLD_MAX_NAME_LENGTH, fname, MLD_MAX_NAME_LENGTH);
    else
      ainfo.fileName[0] = 0;

    m_AllocatedMemoryList.SetAt(addr, ainfo);
};
void CMemLeakDetect::removeMemoryTrace(void* addr, void* realdataptr)
{
    RCF_UNUSED_VARIABLE(realdataptr);

    AllocBlockInfo ainfo;
    //
    if (m_AllocatedMemoryList.Lookup(addr,(AllocBlockInfo &) ainfo))
    {
        m_AllocatedMemoryList.RemoveKey(addr);
    }
    else
    {
        // Didn't find it, just means we didn't track the allocation of this piece of memory.
        // ...
    }
};

void CMemLeakDetect::cleanupMemoryTrace()
{
    m_AllocatedMemoryList.RemoveAll();
}

void CMemLeakDetect::writeOutput(const TCHAR * szOutput)
{
    OutputDebugString(szOutput);
    if (m_output)
    {
        m_output << szOutput;
    }
}

void CMemLeakDetect::dumpMemoryTrace()
{
    POSITION            pos;
    LPVOID              addr;
    AllocBlockInfo      ainfo;
    TCHAR               buf[MLD_MAX_NAME_LENGTH];
    TCHAR               symInfo[MLD_MAX_NAME_LENGTH];
    TCHAR               srcInfo[MLD_MAX_NAME_LENGTH];
    int                 totalSize                       = 0;
    int                 numLeaks                        = 0;
    CallStackFrameEntry*    p                               = 0;

    //
    _tcscpy_s(symInfo, MLD_MAX_NAME_LENGTH, MLD_TRACEINFO_NOSYMBOL);
    _tcscpy_s(srcInfo, MLD_MAX_NAME_LENGTH, MLD_TRACEINFO_NOSYMBOL);
    //
    pos = m_AllocatedMemoryList.GetStartPosition();
    //
    
    std::string outputFileBase = "MemLeakDetect-" + boost::lexical_cast<std::string>(GetCurrentProcessId()) + "-";

    int n = 0;
    std::string candidate = outputFileBase + boost::lexical_cast<std::string>(++n) + ".txt";
    boost::filesystem::path outputPath = m_outputFolder / candidate;
    while ( boost::filesystem::exists(outputPath) )
    {
        candidate = outputFileBase + boost::lexical_cast<std::string>(++n) + ".txt";
        outputPath = m_outputFolder / candidate;
    }
    m_outputPath = outputPath.file_string();
    m_output.open(m_outputPath.c_str());

    int reported = 0;
    
    int suppressedTotal = 0;

    std::vector<int> suppressed( m_funcsToIgnore.size() );

    while(pos != m_AllocatedMemoryList.end())
    {
        std::basic_string<TCHAR> stackTrace;

        stackTrace += "*****************************************************\n";

        numLeaks++;
        _stprintf_s(buf, _T("Memory Leak(%d)------------------->\n"), numLeaks);
        stackTrace += buf;

        //
        m_AllocatedMemoryList.GetNextAssoc(pos, (LPVOID &) addr, (AllocBlockInfo&) ainfo);
        if (ainfo.fileName[0] != NULL)
        {
            _stprintf_s(buf, _T("Memory Leak <0x%X> bytes(%d) occurance(%d) %s(%d)\n"), 
                    ainfo.address, ainfo.size, ainfo.occurance, ainfo.fileName, ainfo.lineNumber);
        }
        else
        {
            _stprintf_s(buf, _T("Memory Leak <0x%X> bytes(%d) occurance(%d)\n"), 
                    ainfo.address, ainfo.size, ainfo.occurance);
        }
        //
        stackTrace += buf;
        //
        p = &ainfo.traceinfo[0];
        
        stackTrace += ainfo.toString();

        stackTrace += "*****************************************************\n";

        totalSize += ainfo.size;

        bool suppress = false;
        for (std::size_t i=0; i<m_funcsToIgnore.size(); ++i)
        {
            if (stackTrace.find( m_funcsToIgnore[i] ) != std::basic_string<TCHAR>::npos)
            {
                suppress = true;
                ++suppressed[i];
                break;
            }
        }
        
        if (suppress)
        {
            ++suppressedTotal;
        }
        else
        {
            ++reported;
            writeOutput(stackTrace.c_str());
        }
    }

    _stprintf_s(buf, _T("\n-----------------------------------------------------------\n"));
    writeOutput(buf);
    if(!totalSize) 
    {
        _stprintf_s(buf,_T("No Memory Leaks Detected for %d Allocations\n\n"), memoccurance);
        writeOutput(buf);
    }
    else
    {
        _stprintf_s(buf, _T("Total %d Memory Leaks: %d bytes Total Allocations %d\n\n"), numLeaks, totalSize, memoccurance);
        writeOutput(buf);

        _stprintf_s(buf, _T("Suppressed Leaks: %d, Reported Leaks: %d.\n\n"), suppressedTotal, reported);
        writeOutput(buf);

        std::ostringstream os;
        os << "Suppressed Leaks: " << std::endl;
        for (std::size_t i=0; i<m_funcsToIgnore.size(); ++i)
        {
            os << m_funcsToIgnore[i] << " (" << suppressed[i] << ")" << std::endl;
        }
        writeOutput(os.str().c_str());
    }

    m_output.close();
}

void CMemLeakDetect::Init()
{
      AllocBlockInfo::initSymbols();
      m_AllocatedMemoryList.InitHashTable(10211, TRUE);
      isLocked              = false;
      g_pMemTrace           = this;
      pfnOldCrtAllocHook    = _CrtSetAllocHook( catchMemoryAllocHook ); 
      _crtDbgFlag = _crtDbgFlag | _CRTDBG_ALLOC_MEM_DF;
}
void CMemLeakDetect::End()
{
    isLocked                = true;
    _CrtSetAllocHook(pfnOldCrtAllocHook);
    dumpMemoryTrace();
    cleanupMemoryTrace();
    g_pMemTrace             = NULL;
    AllocBlockInfo::deinitSymbols();
}
CMemLeakDetect::CMemLeakDetect(const TCHAR * szOutputDir)
{
    // Set the output file and the ignored functions.
    std::string outputDir = ".";
    if (szOutputDir)
    {
        outputDir = szOutputDir;
    }
    m_outputFolder = outputDir.c_str();
    boost::filesystem::create_directories(m_outputFolder);

    m_funcsToIgnore.push_back("boost::thread_specific_ptr");
    m_funcsToIgnore.push_back("ATL::CPerfMon::Initialize");

    Init();
}

CMemLeakDetect::~CMemLeakDetect()
{
    End();
}

#endif // _DEBUG

#if defined(_MSC_VER)
#pragma warning( pop )
#endif
