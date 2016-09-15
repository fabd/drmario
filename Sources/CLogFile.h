// CLogFile.h: interface for the LogFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLOGFILE_H__6BDB34E3_55A1_11D3_B616_00201834E35C__INCLUDED_)
#define AFX_CLOGFILE_H__6BDB34E3_55A1_11D3_B616_00201834E35C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fabtypes.h"

class CLogFile  
{
public:
	CLogFile( LPCSTR sFilename );
	virtual ~CLogFile();
	void Log( LPCTSTR lpFmt, ... );
private:
	HANDLE hFile;
};

#endif // !defined(AFX_CLOGFILE_H__6BDB34E3_55A1_11D3_B616_00201834E35C__INCLUDED_)
