/*
 * fdlist.h
 *
 * $Id: fdlist.h,v 1.6 1999/07/06 05:38:58 tomh Exp $
 */
#ifndef _IRCD_DOG3_FDLIST
#define _IRCD_DOG3_FDLIST

struct FDList {
  unsigned char entry [MAXCONNECTIONS+2];
};

typedef struct FDList fdlist;

void addto_fdlist(int a, fdlist *b);
void delfrom_fdlist( int a, fdlist *b);
void init_fdlist(fdlist *b);

#endif /* _IRCD_DOG3_FDLIST */
