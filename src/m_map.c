/************************************************************************
 *   IRC - Internet Relay Chat, src/m_map.c
 *   Copyright (C) 2003 Hybrid Development Team
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   $Id: m_map.c,v 1.1 2003/10/13 11:33:13 ievil Exp $
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "ircd.h"
#include "config.h"
#include "client.h"
#include "numeric.h"
#include "m_commands.h"
#include "send.h"
#include "s_conf.h"

static char buf[BUFSIZE];

static void dump_map(struct Client *cptr,struct Client *root_p, char *pbuf);

int m_map(struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{
#ifdef SERVERHIDE
  if (!IsAnOper(cptr))
  {
    sendto_one(cptr, form_str(ERR_NOPRIVILEGES), me.name, parv[0]);
    return;
  }
#endif
  dump_map(cptr,&me,buf);  
  sendto_one(cptr, form_str(RPL_MAPEND), me.name, cptr->name);
  sendto_realops_flags(FLAGS_SPY, "MAP requested by %s (%s@%s)",
                       sptr->name, sptr->username, sptr->host);
  return;
}

static void dump_map(struct Client *cptr,struct Client *root_p, char *pbuf)
{
  int cnt = 0, i = 0, len;
  int users;
  struct Client *server_p;

  *pbuf= '\0';

  strncat(pbuf,root_p->name,BUFSIZE - ((size_t) pbuf - (size_t) buf));
  len = strlen(buf);
  buf[len] = ' ';

  users = root_p->serv->usercnt;

  snprintf(buf + len, BUFSIZE, " [Users: %d (%1.1f%%)]", users,
           100 * (float)users / (float)Count.total);

  sendto_one(cptr, form_str(RPL_MAP),me.name,cptr->name,buf);

  if ((server_p = root_p->serv->servers))
  {
    for (; server_p; server_p = server_p->lnext)
    {
      cnt++;
    }

    if (cnt)
    {
      if (pbuf > buf + 3)
      {
        pbuf[-2] = ' ';
        if (pbuf[-3] == '`')
          pbuf[-3] = ' ';
      }
    }
  }

  for (i = 1,server_p = root_p->serv->servers; server_p; server_p=server_p->lnext)
  {
    *pbuf = ' ';
    if (i < cnt)
      *(pbuf + 1) = '|';
    else
      *(pbuf + 1) = '`';

    *(pbuf + 2) = '-';
    *(pbuf + 3) = ' ';
    dump_map(cptr,server_p,pbuf+4);

    i++;
   }
}

