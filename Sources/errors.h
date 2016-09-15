// errors.h : uglyness incarnated

#include "fabtypes.h"
#include "CLogFile.h"

#define Err_exit(val,msg)  {InitFail(0,msg);return val;}
BOOL InitFail(LPCTSTR lpFmt, ...);

extern	CLogFile* LOG;
