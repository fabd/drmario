// CLogFile.cpp: implementation of the LogFile class.
//
//////////////////////////////////////////////////////////////////////

#include "CLogFile.h"

// --------------------------------------------------------------------------
// Print the specified FILETIME to output in a human readable format,
// without using the C run time.
// --------------------------------------------------------------------------
static void PrintTime (char *output, FILETIME TimeToPrint)
{
    WORD Date, Time;
    if (FileTimeToLocalFileTime (&TimeToPrint, &TimeToPrint) &&
        FileTimeToDosDateTime (&TimeToPrint, &Date, &Time))
    {
        // What a silly way to print out the file date/time.
        wsprintf( output, "%d/%d/%d %02d:%02d:%02d",
                         (Date / 32) & 15, Date & 31, (Date / 512) + 1980,
                         (Time / 2048), (Time / 32) & 63, (Time & 31) * 2);
    }
    else
        output[0] = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


// -------------+
// CLogFile		: pass NULL to disable logging, without changing your code
// -------------+
CLogFile::CLogFile( LPCSTR sFilename )
{
	hFile = INVALID_HANDLE_VALUE;

	// do not log anything, do not create a file (for release version)
	if( sFilename == NULL )
		return;

	hFile = CreateFile( sFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL );  //file flag writethrough?
    if( hFile == INVALID_HANDLE_VALUE ) {
        MessageBox( NULL, "CLogFile::CLogFile() - CreateFile() FAILED\r\nLOGGING IS DISABLED!",
						  "CLogFile Class Error", MB_OK|MB_ICONEXCLAMATION );
	}
	else
	{
		// Output current date/time to the log
        FILETIME        CurrentTime;
        char			TimeBuffer[100];
	    GetSystemTimeAsFileTime (&CurrentTime);
        PrintTime (TimeBuffer, CurrentTime);
        Log("Log created at %s.\r\n", TimeBuffer);
	}
}

CLogFile::~CLogFile()
{
	if( hFile != INVALID_HANDLE_VALUE ) {
        CloseHandle( hFile );
		hFile = INVALID_HANDLE_VALUE;
	}
}

void CLogFile::Log( LPCTSTR lpFmt, ... )
{
    char    str[1999];
    va_list arglist;
    DWORD   bytesWritten;

	if( hFile == INVALID_HANDLE_VALUE )
		return;

    va_start (arglist, lpFmt);
    wvsprintf (str, lpFmt, arglist );
    va_end   (arglist);

    WriteFile( hFile, str, lstrlen(str), &bytesWritten, NULL );
}
