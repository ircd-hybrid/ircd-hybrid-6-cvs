/************************************************************************
 *   IRC - Internet Relay Chat, src/m_etrace.c
 *   Copyright (C) 2004 Hybrid Development Team
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
 *   $Id: m_etrace.c,v 1.1 2004/05/23 16:25:22 ievil Exp $
 */

#include "m_commands.h"
#include "class.h"
#include "client.h"
#include "common.h"
#include "hash.h"
#include "irc_string.h"
#include "ircd.h"
#include "numeric.h"
#include "s_bsd.h"
#include "s_serv.h"
#include "send.h"

#include <string.h>
#include <time.h>

/*
** m_etrace - Extended trace - see docs/ETRACE.txt - ideas based on W. Campbell
**      parv[0] = sender prefix
**      parv[1] = servername
*/

int m_etrace(struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{
  int   i;
  struct Client       *acptr = NULL;

  if (check_registered(sptr))
    return 0;
  if (!IsClient(sptr) || !MyConnect(sptr))
    return 0;

  if(!IsAnOper(sptr))
  {
    sendto_one(sptr, form_str(ERR_NOPRIVILEGES), me.name, parv[0]);
    return 0;
  }

  sendto_realops_flags(FLAGS_SPY, "etrace requested by %s (%s@%s)",
                       sptr->name, sptr->username, sptr->host);

  /* report all direct connections */
  for (i = 0; i <= highest_fd; i++)
    {
      const char* ip;
      
      if (!(acptr = local[i])) /* Local Connection? */
        continue;

#ifdef HIDE_SPOOF_IPS
      if (IsIPSpoof(acptr))
        ip = "255.255.255.255";
      else
#endif  
      ip = inetntoa((const char*) &acptr->ip);

      switch(acptr->status)
        {
        case STAT_CLIENT:
          sendto_one(sptr, form_str(RPL_ETRACE), me.name, parv[0],
                     IsAnOper(acptr) ? "Oper" : "User",
                     get_client_class(acptr), acptr->name,
                     acptr->username, acptr->host, ip, acptr->info);
          break;
        default: /* We will ignore all of the rest */
          break;
        }
    }

  sendto_one(sptr, form_str(RPL_ENDOFTRACE),me.name, parv[0], me.name);
  return 0;
}

