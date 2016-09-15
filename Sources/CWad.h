// CWad.h: interface for the CWad class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CWAD_H__7BFAA800_528C_11D3_B616_00201834E35C__INCLUDED_)
#define AFX_CWAD_H__7BFAA800_528C_11D3_B616_00201834E35C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include "fabtypes.h"




struct	WadHeader_s			/* The first bytes of a WAD file: the WAD header */
{
	char    Type[4];        /* Should be "PWAD" or "IWAD" for a (valid) WAD file */
	DWORD	DirSize;        /* Number of entries in the WAD 'directory' */
	DWORD	DirStart;       /* Pointer to start of the WAD 'directory' */
};

struct	Directory_s         /* One entry in the WAD 'directory' */
{
	DWORD	Start;          /* Pointer to the data of this entry */
	DWORD	Size;           /* Length of this data in bytes */
	char	Name[8];        /* Type identifier */
};

#define MYWADTYPE   "PWAD"	//header type id. in DrMario's main WAD.
#define	IWADHANDLE	HANDLE

// return TRUE to continue enumerating entries, FALSE to stop
// iEntry is a simple counter starting at 0, for your convenience
typedef BOOL (* LPENUMSECTIONCALLBACK) ( HANDLE hFile,
										 int    iEntry,
										 struct Directory_s* pEntry );


class CWad
{
public:
	BOOL EnumSection( LPSTR sSectionID, LPENUMSECTIONCALLBACK pUser );
	void* ReadSection( LPSTR sSectionID, UBYTE* pTablePtr[] );
	void* ReadEntry( LPSTR sEntryName );
	inline LPSTR sFilename() { return sPath; };
	inline IWADHANDLE GetFileHandle() { return hFile; };
	int SeekEntry( LPSTR sEntryName );
	CWad (LPSTR sFilename);
	virtual ~CWad();

private:
	LPSTR sPath;
	Directory_s *p_Directory;
	WadHeader_s WadHeader;
	IWADHANDLE hFile;
};

#endif // !defined(AFX_CWAD_H__7BFAA800_528C_11D3_B616_00201834E35C__INCLUDED_)
