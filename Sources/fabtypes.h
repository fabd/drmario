// fabtypes.h

#include <windows.h>

#ifndef _FABTYPES_H_
#define _FABTYPES_H_


// ------
// Memory allocation
// ------
#define	AllocMem(nBytes)	malloc(nBytes)
#define FreeMem(lpBuf)		if(lpBuf != NULL) { free(lpBuf); lpBuf = NULL; }

#define SAFE_DELETE(x)		if(x != NULL) { delete x; x = NULL; }
#define SAFE_RELEASE(x)		if(x != NULL) { x->Release(); x = NULL; }

// ----------
// Use explicit signed/unsigned types : U for unsigned, S for signed
// ----------

#ifndef SBYTE
typedef signed   char	  SBYTE;	//windef.h defines BYTE as unsigned !
typedef unsigned char     UBYTE;
#endif
#ifndef SWORD
typedef signed   short    SWORD;
typedef unsigned short    UWORD;
#endif
#ifndef SLONG
typedef signed   long	  SLONG;
typedef unsigned long     ULONG;
#endif
#ifndef FLOAT
typedef float             FLOAT;
#endif

#ifndef TRUE
#define TRUE	 1
#endif
#ifndef FALSE
#define FALSE	 0
#endif

// this is from windef.h, in case a module doesn't use windows code, and doesn't
// include windef.h, these are handy
#ifndef LOWORD
#define LOWORD(l)           ((UWORD)(l))
#define HIWORD(l)           ((UWORD)(((ULONG)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((UBYTE)(w))
#define HIBYTE(w)           ((UBYTE)(((UWORD)(w) >> 8) & 0xFF))
#endif

#ifndef NULL
#define NULL	0
#endif

#ifndef boolean
typedef unsigned char		boolean;
#endif
#ifndef true
#define true	1
#endif
#ifndef false
#define false	0
#endif

#endif /* _FABTYPES_H_ */
