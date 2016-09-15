// CWad.cpp: implementation of the CWad class.
//
//////////////////////////////////////////////////////////////////////

#include "CWad.h"
#include "errors.h"
#include "fabtypes.h"


// OPTIMISATIONS
// - use DWORD compares to search for lump names instead of strncmp


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


// -------------+
// CWad Constructor
//				: Open the WAD file given filename, ready for use
// -------------+
CWad::CWad(LPSTR sFilename)
{
	DWORD	dwBytesRead;

    // Open file
	hFile = CreateFile(sFilename,
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template  
	
	if( hFile == INVALID_HANDLE_VALUE )
        InitFail("Can not open WAD file '%s'",sFilename);

    if( !ReadFile( hFile, &WadHeader, sizeof(struct WadHeader_s), &dwBytesRead, NULL) )
        InitFail("Error reading from file '%s'", sFilename);
		
	// Check for valid IWAD header

    if( strncmp( WadHeader.Type, MYWADTYPE, 4 ) )
        InitFail("Not a valid wad file '%s'", sFilename);

    // Allocate buffer, load IWAD directory
	
	if( ( p_Directory = (Directory_s*) AllocMem( WadHeader.DirSize * sizeof( struct Directory_s ) ) ) == NULL )
        InitFail("Cannot allocate memory for directory of WAD file '%s'", sFilename);

	if ( SetFilePointer( hFile, WadHeader.DirStart, NULL, FILE_BEGIN ) == -1L )
		InitFail("Cannot SetFilePointer() on WAD file '%s'", sFilename);

	if( !ReadFile( hFile, p_Directory,
				   sizeof(struct Directory_s) * WadHeader.DirSize,
				   &dwBytesRead , NULL)	)
        InitFail("Cannot read Directory from WAD file '%s'", sFilename);


	// save filename
	sPath = (LPSTR) AllocMem( strlen( sFilename ) + 1 );
	strcpy( sPath, sFilename);
}


// -------------+
// CWad Destructor
//				: Releases all resources used by the WAD file, close the file
// -------------+
CWad::~CWad()
{
	FreeMem( sPath );
	FreeMem( p_Directory );
	CloseHandle( hFile );
}


// -------------+
// SeekEntry	: Move the WAD file pointer to the start of an entry
// Returns		: Size in bytes of the entry, or -1L for error
// -------------+
int CWad::SeekEntry( LPSTR sEntryName )
{
	ULONG	i,j;
	struct  Directory_s *p_src;
	char    lmpname[9];

    // PAD name with 0 bytes
    j = 1;
    for( i=0; i<9; i++ )
    {
        if( j && ( sEntryName[i] != '\0' ) )
            lmpname[i] = sEntryName[i];
        else
        {
            j = 0;
            lmpname[i] = '\0';
        }
    }

    p_src = p_Directory;

    for( i=0; i < WadHeader.DirSize; i++ )
    {
        if( !strncmp( p_src->Name, sEntryName, 8 ) )
        {
            if( SetFilePointer( hFile, p_src->Start, NULL, FILE_BEGIN ) == -1L )
                InitFail("Cannot Seek on WAD file '%s'", sPath);
            else
                return( p_src->Size );
        }
        p_src++;
    }

    return -1L;
}


// -------------+
// ReadEntry	: Allocate buffer and load entry into it
// Returns		: Pointer to the buffer
// Note			: REMEMBER to FREE the buffer!
// -------------+
void* CWad::ReadEntry(LPSTR sEntryName)
{
	void    *buf;
	int		iLumpSize;
	DWORD	dwBytesRead;

    if( (iLumpSize = SeekEntry( sEntryName )) <= 0 )
        return NULL;

    if( ( buf = (void *) AllocMem( iLumpSize ) ) == NULL )
        InitFail("CWad::ReadEntry - AllocMem() FAILED");

    if( !ReadFile( hFile, buf, iLumpSize, &dwBytesRead, NULL ) )
        InitFail("CWad::ReadEntry - ReadFile() FAILED");

    return  buf;
}


/************************************************************************

  Cette routine charge tous les samples au format WAV trouvés dans la
  section indiquée. Pour chaque sample chargé avec succès, un pointeur
  est placé dans le tableau Sound_ptr[xxx]. Si un sample n'a pu ˆtre
  chargé proprement par MW_LoadWavFP, le message d'erreur de MikMod
  est affiché et le programme quitte.

  Un pointeur NULL est ajouté dans le tableau Sound_ptr[] pour indiquer
  la fin des samples chargés (pour la routine qui libère la mémoire).

 ************************************************************************/
/*
static  void    WAD_LoadSamples( char *sectname )
{

   struct Directory_s *p_src,*p_sect;
   booleantrouve;
   LONG   i;

    // Repérer la section, compter la taille totale nécessaire pour
    // charger tous les lumps de cette section.                     

    p_src = (struct Directory_s *) p_Directory;
    trouve = FAIL;
    for( i=0; i < WadHeader.DirSize; i++ )
    {
        if( !strncmp( p_src->Name, sectname, 8 ) )
        {
            trouve = OK;
            break;
        }
        p_src++;
    }

    if( !trouve )
        Err_exit("Err:Samples Section not found");

    // pointe le premier lump après début de section
    p_src++;

    // charger chaque lump suivant avec LoadWAV_FP, on quitte si jamais
    // un sample WAV n'est pas conforme. On s'arrˆte quand on trouve la fin
    // de la section.
    i = 0;
    while( strncmp( p_src->Name, "E_", 2 ) )
    {

        if( fseek( IWAD_handle, p_src->Start, SEEK_SET ) )
            Err_exit("Err:Seek error in WAD_LoadSamples.");

        if( (Sound_ptr[i] = MW_LoadWavFP( IWAD_handle )) == NULL )
        {
            cprintf(" error loading sample (%02ld): name (%.8s) size (%08ld).\r\n", i, p_src->Name, p_src->Size );
            cprintf("MikMod Error: %s\r\n",myerr);
            Err_exit("Err: aaargh!");
        }
        putch('.');

        p_src++;
        if( ++i >= MAX_SAMPLES )
            Err_exit("Err: trop de samples rallonge MAX_SAMPLES!");

    }
    Sound_ptr[i] = NULL;        // marque la fin des samples chargés
    cputs("\r\n");

    #if( DEBUG )
        cputs("finished loading samples ok.\r\n");
    #endif
}
*/

/**** libère les samples chargés par WAD_LoadSamples()
static  void    Free_Samples(void)
{
   UWORD   i;

    // normalement, j'aime bien libérer la mémoire à l'ENVERS (améliorer)
    i = 0;
    while( Sound_ptr[i] != NULL )
    {
        MW_FreeWav( Sound_ptr[i] );
        i++;
    }

}
*/


// -------------+
// ReadSection	: Load a sequence of entries, store a pointer to each entry
//				: into table
//				: The given section ID should be a dummy entry of size 0,
//				: all entries following are loaded until an entry starting
//				: name with 'E_' is found.
// Returns		: void pointer to the buffer with all loaded entries
// Note			: you MUST FREE the returned pointer!
//				: Section starts with dummy entry 'S_xxxxxx' and ends with 'E_xxxxxx'
// -------------+
void* CWad::ReadSection(LPSTR sSectionID, UBYTE *pTablePtr[] )
{

	struct  Directory_s *p_src,*p_sect;
	BOOL	trouve;
	int		bufsize;
	void    *buf;
	UBYTE   *p_dest;
	ULONG   s_entries;
	ULONG   i;
	DWORD	dwBytesRead;

	// Find section ID, count space needed to load all entries in section
   
    p_src = p_Directory;
    trouve = false;
    for( i=0; i < WadHeader.DirSize; i++ )
    {
        if( !strncmp( p_src->Name, sSectionID, 8 ) )
        {
            trouve = true;
            break;
        }
        p_src++;
    }

    if( !trouve )
        InitFail("CWad::ReadSection() - Section ID '%s' not found", sSectionID);

    p_src++;
    p_sect = p_src;		// start of section in wad directory
    bufsize = 0;
    s_entries = 0;		// count of entries
    while( strncmp( p_src->Name, "E_", 2 ) )
    {

        bufsize += ( ( p_src->Size + 4 ) & ~3UL );		// each entry will be DWORD aligned
        //#if( DEBUG )
        //    cprintf(" lump %.8s in section found. Bufsize is now %08ld.\r\n", p_src->Name, bufsize );
        //#endif
        p_src++;
        s_entries++;
    }

    //#if( DEBUG )
    //   cprintf("Section %s - %ld entries - %ld bytes buffer.\r\n", sectname, s_entries, bufsize);
    //#endif

	// Allocate memory to load all entries in section
	// Store pointer to each entry in the table

    if( (buf = AllocMem( bufsize+4 )) == NULL )
        return NULL;

    p_dest = (UBYTE *) buf;             // début du buffer
    for( i=0; i < s_entries; i++ )
    {

        if( SetFilePointer( hFile, p_sect->Start, NULL, FILE_BEGIN ) == -1L )
            InitFail("CWad::ReadSection() - SetFilePointer() FAILED");

        if( !ReadFile( hFile, p_dest, p_sect->Size, &dwBytesRead, NULL ) )
        {
        //    cprintf("\r\nErr: reading lump %.8s (size %08ld) in section %.8s at position %08ld.\r\n", p_sect->Name, p_sect->Size, sectname, p_sect->Start );
        //    cprintf("\r\nActually read %ld bytes at buffer %ld.\r\n", bytes, p_dest );
            InitFail("CWad::ReadSection() - ReadFile() FAILED");
        }

        //#if( DEBUG )
        //    cprintf(" loaded lump #%02ld of section. name (%.8s) size (%08ld).\r\n", i, p_sect->Name, p_sect->Size );
        //#endif

		pTablePtr[ i ] = p_dest;

        p_dest += ( ( p_sect->Size + 4 ) & ~3UL );
        p_sect++;

    }

    //#if( DEBUG )
    //    cputs("finished loading section ok.\r\n");
    //#endif

    return buf;
}


// -------------+
// EnumSection	: call back your function for each entry in the section, starting at the
//				: entry just after the given section name.
// -------------+
BOOL CWad::EnumSection( LPSTR sSectionID, LPENUMSECTIONCALLBACK pUser )
{
	struct  Directory_s *p_src;
	ULONG   i;

	LOG->Log( "CWad::EnumSection( '%s' ) :\n", sSectionID );

	// Find section ID, count space needed to load all entries in section
    p_src = p_Directory;
    for( i=0; i < WadHeader.DirSize; i++ )
    {
        if( !strncmp( p_src->Name, sSectionID, 8 ) )
            break;
        p_src++;
    }

    if( i >= WadHeader.DirSize ) {
        InitFail("CWad::EnumSection() - Section ID '%s' not found", sSectionID);
		return FALSE;
	}

    p_src++;		// start of section in wad directory

	// allow an empty section, so test at the start of the loop
	i=0;
	while( strncmp( p_src->Name, "E_", 2 ) )
    {
		LOG->Log( "\t\t%#03d:'%s'\t%d bytes\n", i, p_src->Name, p_src->Size );
        
		if( SetFilePointer( hFile, p_src->Start, NULL, FILE_BEGIN ) == -1L ) {
            InitFail("CWad::EnumSection() - SetFilePointer() FAILED");
			return FALSE;
		}

		// call the user callback function, with the current file pointer
		// set to the beginning of the entry
		if ( ! (*pUser) ( hFile, i, p_src ) ) {
			LOG->Log( "\t\tInterrupted by callback\n" );
			break;
		}
		
		i++;
        p_src++;
    }

	LOG->Log( "\tEnumSection Ok.\n" );
	return TRUE;
}
