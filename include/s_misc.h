/*
 * $Id: s_misc.h,v 1.7 2000/10/06 03:00:57 lusky Exp $ 
 */

#ifndef INCLUDED_s_misc_h
#define INCLUDED_s_misc_h
#ifndef INCLUDED_sys_types_h
#include <sys/types.h>
#define INCLUDED_sys_types_h
#endif

struct Client;
struct ConfItem;

extern void    serv_info (struct Client *, char *);
extern char*   date(time_t);
extern const char* smalldate(time_t);
extern char    *small_file_date(time_t);

#endif


