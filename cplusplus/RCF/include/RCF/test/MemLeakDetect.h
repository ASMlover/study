
// JL: This code originated from a CodeProject article: 
// http://www.codeproject.com/KB/cpp/MemLeakDetect.aspx

#pragma once

#include "CallStack.hpp"

// CRT memory tracking only works in debug builds.
// Note also that stack traces are only available on x86 builds.
#if defined(_DEBUG)

#define _CRTDBG_MAP_ALLOC
#include <map>
#define _CRTBLD
#include <windows.h>

#include <..\crt\src\dbgint.h>
#include <dbghelp.h>
#include <crtdbg.h>

#include <boost/filesystem/path.hpp>

class CMemLeakDetect
{
	public:

		struct AllocBlockInfo : public CallStack
		{
			AllocBlockInfo() : address(NULL), size(0), lineNumber(0), occurance(0)
			{
				memset(fileName, 0, MLD_MAX_NAME_LENGTH * sizeof(TCHAR));
			}

			void*				address;
			DWORD				size;
			TCHAR				fileName[MLD_MAX_NAME_LENGTH];
			DWORD				lineNumber;
			DWORD				occurance;
		};

		typedef std::map<LPVOID, AllocBlockInfo>				KEYMAP;
		typedef std::map<LPVOID, AllocBlockInfo>::iterator	POSITION;
		typedef std::pair<LPVOID, AllocBlockInfo>			KEYVALUE;

		class CMapMem
		{
			public:

				KEYMAP			m_Map;
				POSITION		m_Pos;

				inline BOOL Lookup(LPVOID pAddr,  AllocBlockInfo& aInfo) { 

					m_Pos = m_Map.find(pAddr);
					//
					if (m_Pos == m_Map.end())
					{
						return FALSE;
					}
					//
					pAddr = m_Pos->first;
					aInfo = m_Pos->second;

					return TRUE;
				};

				inline POSITION end() { 

					return m_Map.end(); 
				};

				inline void RemoveKey(LPVOID pAddr) { 
					
					m_Map.erase(pAddr);
				};

				inline void RemoveAll() {
					m_Map.clear();
				};

				void SetAt(LPVOID pAddr, AllocBlockInfo& aInfo) {

					m_Map[pAddr] = aInfo;
				};

				inline POSITION GetStartPosition() { 
					POSITION pos = m_Map.begin(); 
					return pos;
				};

				inline void GetNextAssoc(POSITION& pos, LPVOID& rAddr, AllocBlockInfo& aInfo) {

					rAddr = pos->first;
					aInfo = pos->second;
					pos++;
				};

				void InitHashTable(int preAllocEntries, BOOL flag) 	{
					 preAllocEntries	= NULL;
					 flag				= NULL;
				};

		};

		CMemLeakDetect(const TCHAR * outputDir = NULL);
		~CMemLeakDetect();
		void Init();
		void End();
		void addMemoryTrace(void* addr,  DWORD asize,  TCHAR *fname, DWORD lnum);
		void redoMemoryTrace(void* addr,  void* oldaddr, DWORD asize,  TCHAR *fname, DWORD lnum);
		void removeMemoryTrace(void* addr, void* realdataptr);
		void cleanupMemoryTrace();
		void writeOutput(const TCHAR * szOutput);
		void dumpMemoryTrace();
		//

		CMapMem			 m_AllocatedMemoryList;
		DWORD memoccurance;
		bool  isLocked;
	//
	private:

	public:
		std::vector<std::basic_string<TCHAR> >	m_funcsToIgnore;
		boost::filesystem::path					m_outputFolder;
		

	private:
		std::string								m_outputPath;
		std::ofstream							m_output;
};

#else

// Dummy replacement for release.
class CMemLeakDetect
{
public:
	CMemLeakDetect(const TCHAR * = NULL)
	{
	}
};

#endif

// JL: Example instrumentation usage - in a loop that causes memory growth, 
// get stack traces of outstanding allocations after each iteration. Stack 
// traces get written to a sequence of text files.

//std::auto_ptr<CMemLeakDetect> leakDetectorPtr;
//while (true)
//{
//	leakDetectPtr.reset( new CMemLeakDetect() );
//
//	// Leaky code here
//	// ...
//	
//	// A new text file will be produced for each iteration.
//	leakDetectPtr.reset();
//}
