/************************************************************************
 *   IRC - Internet Relay Chat, src/m_encap.c
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
 *   $Id: m_encap.c,v 1.3 2004/05/23 16:33:56 ievil Exp $
 */

#include "m_encap.h"
#include "common.h"
#include "m_commands.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "irc_string.h"
#include "s_serv.h"
#include "parse.h"
#include "msg.h"
#include <string.h>

/* ms_encap()
 *
 * inputs	- local source, actual source, parc, parv
 * outputs	- none
 * side effects	- propogate, parse if understood
 */

int ms_encap(struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{
  char buffer[BUFSIZE], *ptr = buffer;
  unsigned int cur_len = 0, len, i;
  struct Message *mptr;
  acl_encap *aclptr;
  int acl_action = 0;


  if (!IsServer(cptr))
    return 0;

  for (i = 1; i < (unsigned int) parc - 1; i++)
  {
    len = strlen(parv[i]) + 1;

    if ((cur_len + len) >= sizeof(buffer))
      return 0;

    ircsprintf(ptr, "%s ", parv[i]);
    cur_len += len;
    ptr += len;
  }

  len = strlen(parv[i]);

  if (parc == 3)
    ircsprintf(ptr, "%s", parv[2]);
  else
    ircsprintf(ptr, ":%s", parv[parc-1]);

  if ((cur_len + len) >= sizeof(buffer))
    buffer[sizeof(buffer)-1] = '\0';

  sendto_match_cap_servs_butone(sptr, cptr, parv[1], CAP_ENCAP, "ENCAP %s",
                                buffer);

  for (aclptr = encap_acl; TRUE; aclptr++)
  {
    if (!aclptr->cmd || (!irccmp(parv[2], aclptr->cmd)))
      {
        acl_action = aclptr->acl;
        break;
      }
  }
  if (acl_action == E_ACCEPT)
  {
  if (!match(parv[1], me.name))
    return 0;

  mptr = tree_parse(parv[2]);
  if ((mptr == NULL) || (mptr->cmd == NULL))
    return 0;
  mptr->bytes += strlen(buffer);
  ptr = parv[0];
  parv+=2;
  parc-=2;
  parv[0] = ptr;
  return (*mptr->func)(cptr, sptr, parc, parv);
  }
  return 0;
}
