// win_dbg.h : structured exception handler

#include <windows.h>

// called in the exception filter of the __try block, writes all useful debugging information
// to a file, using only win32 functions in case the C runtime is in a bad state.
int RecordExceptionInfo (PEXCEPTION_POINTERS data, char const *Message, LPSTR lpCmdLine);
