/*
 * $Id: sprintf_irc.h,v 1.3 2001/12/08 17:56:52 db Exp $ 
 */

#ifndef SPRINTF_IRC
#define SPRINTF_IRC

#include <stdarg.h>

/*=============================================================================
 * Proto types
 */

extern int vsprintf_irc(char *str, const char *format, va_list);
extern int ircsprintf(char *str, const char *format, ...);

#endif /* SPRINTF_IRC */
