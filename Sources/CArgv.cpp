// CArgv.cpp: implementation of the CArgv class.
//
//////////////////////////////////////////////////////////////////////

#include "CArgv.h"

// TODO:
// use win32 functions to support unicode..

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArgv::CArgv( LPCSTR lpCmdline )
{
	int	iLen = lstrlen( lpCmdline );

	iFound = 0;
	myargc = 0;

	myCmdline = (LPSTR) GlobalAlloc( GMEM_FIXED, iLen+1 );
	if (myCmdline==NULL)
		return;
	
	// myCmdline will be written to, make sure we are in writable memory
	CopyMemory( myCmdline, lpCmdline, iLen );
	myCmdline[iLen] = 0;
	myargc = ParseCmdline( myCmdline, myargv );
}

CArgv::~CArgv()
{
	if( myCmdline!=NULL )
		GlobalFree( myCmdline );
	myCmdline = NULL;
}


// -------------+
// CheckParm	: Checks for the given parameter in the command line arguments
// Returns		: The argument number (1 to argc-1) or 0 if not present
// -------------+
int CArgv::CheckParm( LPCSTR sParm )
{
    int i;

    for( i = 1; i<myargc; i++ )
    {
        if( !lstrcmp( sParm, myargv[i]) )
        {
            iFound = i;
            return i;
        }
    }
    iFound = 0;
    return 0;
}


// -------------+
// IsNextParm	: returns TRUE if there is a value to come, after you called CheckParm()
//				: use CheckParm() to find a '+option <value>' style of parameter, then
//				: use IsNextParm() to find if the <value> is present, it will return FALSE
//				: if the next parameter starts with '+' or '-'
// -------------+
BOOL CArgv::IsNextParm()
{
    if(iFound>0 && (iFound+1<myargc) && myargv[iFound+1][0] != '-' && myargv[iFound+1][0] != '+')
        return TRUE;
    return FALSE;
}


// -------------+
// GetNextParm	: Returns the next parameter after a M_CheckParm()
//				: NULL if not found use M_IsNext to fin,d if there is a parameter
// -------------+
LPSTR CArgv::GetNextParm()
{
    if( IsNextParm() )
    {
        iFound++;
        return myargv[iFound];
    }
    return NULL;
}


// -------------+
// ParseCmdline	: Split the cmdline string into argv[] up to MAXCMDLINEARGS
//				: Spaces become part of a token when the token is delimited with double quotes " "
//				: Quake(tm)-like arguments beginning with '+' will force the next token to store
//				: the " " quotes as well if they are specified, eg:
//				: name fabbe score 500		-> 4 args
//				: +name "fab five"          -> 2 args '+name' and '"fab five"'
// Returns:		: - tokens in argv[] parameter, starting at 0, with a NULL pointer after the last token
//              : - args count
// -------------+
int CArgv::ParseCmdline( LPSTR Cmdline,		// IN
						 LPSTR argv[] )		// OUT
{
    char*   token;
    int     i, len, argc;
    char    cSep;
    BOOL    bCvar = FALSE, prevCvar = FALSE;

    // split arguments of command line into argv
    len = lstrlen (Cmdline);

    argv[0] = "dummy.exe";
    argc = 1;
    i = 0;
    cSep = ' ';
    while( argc < MAXCMDLINEARGS )
    {
        // get token
        while ( Cmdline[i] == cSep )
            i++;
        if ( i >= len )
            break;
        token = Cmdline + i;
        if ( Cmdline[i] == '"' ) {
            cSep = '"';
            i++;
            if ( !prevCvar )    //cvar leave the "" in
                token++;
        }
        else
            cSep = ' ';

        //cvar
        if ( Cmdline[i] == '+' && cSep == ' ' )   //a + begins a cvarname, but not after quotes
            bCvar = TRUE;
        else
            bCvar = FALSE;

        while ( Cmdline[i] &&
                Cmdline[i] != cSep )
            i++;

        if ( Cmdline[i] == '"' ) {
             cSep = ' ';
             if ( prevCvar )
                 i++;       // get ending " quote in arg
        }

        prevCvar = bCvar;

        if ( Cmdline + i > token )
        {
            argv[argc++] = token;
        }

        if ( !Cmdline[i] || i >= len )
            break;

        Cmdline[i++] = '\0';
    }
    argv[argc] = NULL;

	return argc;
}
