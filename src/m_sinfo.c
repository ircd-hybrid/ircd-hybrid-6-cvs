/************************************************************************
 *   IRC - Internet Relay Chat, src/m_sinfo.c
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
 *
 *   $Id: m_sinfo.c,v 1.3 2005/03/25 11:25:25 ievil Exp $
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "ircd.h"
#include "common.h"
#include "config.h"
#include "client.h"
#include "numeric.h"
#include "m_commands.h"
#include "send.h"
#include "s_conf.h"
#include "channel.h"  /* for server_was_split */
#include "m_sinfo.h"
#include "irc_string.h"

int m_sinfo (struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{
  list_sinfo *sinfo;
  int snum;


  if (!MyClient(sptr) || !IsAnOper(sptr))
  {
    sendto_one(sptr, form_str(ERR_NOPRIVILEGES), me.name, parv[0]);
    return 0;
  }
   
  if (parc > 1)
  {
    for (sinfo = lsinfo; TRUE; sinfo++)
    {
      if (!sinfo->cmd || (!irccmp(parv[1], sinfo->cmd)))
      {
        snum = sinfo->token;
        break;
      }
    }
    
    switch(snum)
    {
      case TOKEN_SPLIT:
#ifdef NEED_SPLITCODE
        if (server_was_split)
          sendto_one(sptr, ":%s NOTICE %s :*** Notice -- server is currently in split-mode",
                     me.name, parv[0]);
        else
          sendto_one(sptr, ":%s NOTICE %s :*** Notice -- server is currently not in split-mode",
                     me.name, parv[0]);
#else
        sendto_one(sptr, ":%s NOTICE %s :*** Notice -- server is not running with splitcode enabled!",
                   me.name, parv[0]);
#endif
        return 0;
        break;
      case TOKEN_KLINES:
        sendto_one(sptr, ":%s NOTICE %s :*** Notice -- not yet implemented",
                   me.name, parv[0]);
        break;
      case TOKEN_DLINES:
        sendto_one(sptr, ":%s NOTICE %s :*** Notice -- not yet implemented",
                   me.name, parv[0]);
        break;
      case TOKEN_XLINES:
        sendto_one(sptr, ":%s NOTICE %s :*** Notice -- not yet implemented",
                   me.name, parv[0]);
        break;
      case TOKEN_TKLINES:
        {
          struct ConfItem *cur_p;
          int tknum = 0;
          
          for (cur_p = temporary_klines; cur_p; cur_p = cur_p->next)
          { 
            tknum++;
          }
          sendto_one(sptr, ":%s NOTICE %s :There are currently %d temporary kline(s)", 
                     me.name, parv[0], tknum);  
          return 0;
          break;
        }
      case TOKEN_NONE:
        break;
      default:
        sendto_one(sptr, ":%s NOTICE %s :This shouldn't happend. Tell the hybteam.",
                   me.name, parv[0]);
        break;    
    }
  }
  sendto_one(sptr, ":%s NOTICE %s :Options: SPLIT KLINES DLINES XLINES TKLINES",
             me.name, parv[0]);
  return 0;
}
