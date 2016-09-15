// fabgrafx.h : faB's Graphics library v1.0
//

#ifndef _FABGRAFX_H_
#define _FABGRAFX_H_

#include "fabtypes.h"

extern	char*	fabgrafx_error;

// swap little/big endian LONG value (4 bytes)
void SwapLong (UBYTE* num);

//unpack a PBM IFF file (if non-NULL p_dest), and palette (if non-NULL p_cmap)
boolean DecrunchPBM (UBYTE *p_lbm, UBYTE *p_dest, LPPALETTEENTRY p_cmap);

// copy a rectangluar area from src bitmap to dest bitmap 8bpp
void CopyBlock (UBYTE *src, int x, int y, int width, int height, int srcPitch,
				UBYTE *dest, int dx, int dy, int destPitch);

//note: second parameter defaults to 'read all file size'
UBYTE*  LoadBinary (char *filename, int nbytes=-1);

#endif /* _FABGRAFX_H_ */