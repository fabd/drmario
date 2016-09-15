// CArgv.h: interface for the CArgv class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CARGV_H__009781A1_5747_11D3_B616_00201834E35C__INCLUDED_)
#define AFX_CARGV_H__009781A1_5747_11D3_B616_00201834E35C__INCLUDED_

#include <windows.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAXCMDLINEARGS   64

class CArgv  
{
public:
	CArgv( LPCSTR lpCmdline );		// pass lpCmdLine from your WinMain() function
	virtual ~CArgv();
	int CheckParm( LPCSTR sParm );
	BOOL IsNextParm();
	LPSTR GetNextParm();
	LPSTR myargv[MAXCMDLINEARGS+1];

private:
	int iFound;
	int myargc;
	int ParseCmdline( LPSTR Cmdline,		// IN
						 LPSTR argv[] );	// OUT
	LPSTR myCmdline;
};

#endif // !defined(AFX_CARGV_H__009781A1_5747_11D3_B616_00201834E35C__INCLUDED_)
