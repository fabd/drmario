// faB's Graphics library v1.0
//

#include <ddraw.h>
#include <io.h>
#include <fcntl.h>

#include "fabgrafx.h"

#define  GRAFXERROR(s)    fabgrafx_error=s;

	// holds a pointer to a string describing the last error encountered
	// (when a function returns with false)
	char*	fabgrafx_error;

// --------------------------------------------------------------------------
// Convert number from Little Endian to Big Endian (or vice versa).
// num : holds a pointer to the 4bytes that should be swapped.
// --------------------------------------------------------------------------
void SwapLong (UBYTE* num)
{
    UBYTE   temp;

    temp = *num;
    *num = *(num + 3);
    *(num + 3) = temp;
    temp = *(num + 1);
    *(num + 1) = *(num + 2);
    *(num + 2) = temp;
}


// --------------------------------------------------------------------------
// Decrunch an 256 color ILBM/PBM picture into a given destination buffer.
// If a non-NULL colormap pointer is given, the palette is copied into it.
// The palette should be 256 entries of Red,Green,Blue components 8bits
// NOTE:
//   Currently, all the picture data is uncompressed, regardless of what
//   size it is. You should know in advance the size of your picture.
//   Also, the picture should be in 1 byte-per-pixel format (no bitplanes!)
//
// p_lbm  <- holds a pointer to the ILBM/PBM data.
// p_dest <- if non-NULL, the picture is uncompressed here.
// p_cmap <- if non-NULL, the palette of the picture is copied here.
// --------------------------------------------------------------------------
boolean DecrunchPBM (UBYTE *p_lbm, UBYTE *p_dest, LPPALETTEENTRY p_cmap)
{
	ULONG	chunkname;
	ULONG	chunksize;
	ULONG	formsize;
	SWORD	lbm_done;
	UBYTE	*p_src;
	UBYTE	control;
	SWORD	count;
	UBYTE	*p_endlbm;

	UWORD	largeur_iff;
	UWORD	hauteur_iff;	/* not actually used */

#define ILBM	0x4D424C49
#define PBM		0x204D4250
#define FORM	0x4D524F46
#define BODY	0x59444F42
#define CMAP	0x50414D43
#define BMHD	0x44484D42	/* 4-bytes reversed Intel-sucks format */

	chunkname = *( (ULONG *) p_lbm );
	if( chunkname != FORM )
	{
		GRAFXERROR("LBMDecrunch : not a valid IFF file.")
		return false;
	}
	
	p_lbm += 4;
	SwapLong( p_lbm );					// swap from Motorola to Intel
	formsize = *( (ULONG *) p_lbm );
	SwapLong( p_lbm );					// restore as it was
	
	p_lbm += 4;
	chunkname = *( (ULONG *) p_lbm );
	if ( (chunkname != ILBM ) && (chunkname != PBM ) )
	{
		GRAFXERROR("LBMDecrunch : not an ILBM or PBM IFF file.")
		return false;
	}
	
	formsize -= 4;		    // soustrait le 'PBM ' ou 'ILBM'
	p_lbm   += 4;		    // pointe le premier chunk.
	
	lbm_done = 0;		    // nothing done yet.
	
	p_endlbm = p_lbm + formsize;    // pointe la fin du FORM IFF.
	
	largeur_iff = 320;
	hauteur_iff = 200;		    /* valeurs par défaut */
	
	do
	{
		
		chunkname = *( (ULONG *) p_lbm );	   // CHUNKNAME is 4bytes
		p_lbm += 4;
		SwapLong( p_lbm );			    // swap ChunkSize
		chunksize = *( (ULONG *) p_lbm );
		SwapLong( p_lbm );			    // reswap pr la prochaŒne!
		p_lbm += 4;				    // points start of chunk DATA
		
		p_src = p_lbm;			// points chunk data
		p_lbm += chunksize;		// points to next chunk.
		
		if( chunksize & 1 )
			p_lbm++;			// make it EVEN
		
		if( chunkname == BMHD )
		{
			largeur_iff = *( (UWORD *) p_src );
			p_src += 2;
			hauteur_iff = *( (UWORD *) p_src );
		}
		else if( chunkname == CMAP )
		{
			if( p_cmap != NULL )
			{
				//
				// Read the color map into a PALETTEENTRY table
				//
				if( chunksize == 768 )
				{
					// use PALETTEENTRY 's for DirectX
					for( count = 0; count < 256; count++ )
					{
						p_cmap->peRed  = *(p_src++);
						p_cmap->peGreen= *(p_src++);
						p_cmap->peBlue = *(p_src++);
						p_cmap->peFlags= 0;
						p_cmap++;			// next palette entry
					}
				}
			}
			lbm_done++;
		}
		else if( chunkname == BODY )
		{
			if( p_dest != NULL )
			{
				//
				// Uncompress the picture into a 8bit (one byte per pixel) surface
				//
				do
				{
					control = *( p_src++ ); 	    // read CONTROL byte.
					chunksize--;
					
					if( control != 0x80 )
					{
						if( control & 0x80 )		// REPEAT next byte
						{
							count	= 257 - control;
							control = *( p_src++ ); 	// value to repeat
							chunksize--;
							do
							{
								*( p_dest++ ) = control;
							} while( --count > 0 );
						}
						else				// COPY next bytes
						{
							count = control + 1;
							chunksize -= count;
							do
							{
								*( p_dest++ ) = *( p_src++ );
							} while( --count > 0 );
						}
					}
				} while( chunksize > 0 );
			}
			lbm_done += 2;
		}
		
	} while( (p_lbm < p_endlbm) && (lbm_done != 3) );
	
	return true;
}


// --------------------------------------------------------------------------
// Copy a rectangular area from one bitmap to another (8bpp)
// srcPitch, destPitch : width of source and destination bitmaps
// --------------------------------------------------------------------------
void CopyBlock (UBYTE *src, int x, int y, int width, int height, int srcPitch,
				UBYTE *dest, int dx, int dy, int destPitch)
{
    src += (y * srcPitch) + x;
	dest += (dy * destPitch) + dx;
	while (height--)
    {
        memcpy (dest, src, width);
		dest += destPitch;
		src += srcPitch;
    }
}


// --------------------------------------------------------------------------
// Load file into automatically allocated memory buffer, pass -1 to read all
// file or a positive number to read only a part of it.
// - returns NULL in case  of problem.
// --------------------------------------------------------------------------
UBYTE*  LoadBinary (char *filename, int nbytes)
{
	int		handle;
	UBYTE	*buffer;
	
    if ( (handle = _open (filename , _O_RDONLY | _O_BINARY) ) == NULL)
		return NULL;
	
    if (nbytes == -1)
		nbytes = _filelength (handle);		// read COMPLETE file
	
    //cprintf("Size of file: %08ld.\r\n", nbytes );
	
    if ((buffer = (UBYTE *) malloc(nbytes)) == NULL)
		goto outtahere;
	
    if (_read (handle, buffer, nbytes) != nbytes)
    {
		free (buffer);
		buffer = NULL;
    }
	
outtahere:
    _close (handle);
    return buffer;
}
