/************************************************************************
 *   IRC - Internet Relay Chat, src/m_ungline.c
 *   Copyright (C) 1990 Jarkko Oikarinen and
 *                      University of Oulu, Computing Center
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
 *   $Id: m_ungline.c,v 1.2 2003/06/24 03:57:16 ievil Exp $
 */
#include "m_commands.h"
#include "channel.h"
#include "client.h"
#include "common.h"
#include "dline_conf.h"
#include "fileio.h"
#include "irc_string.h"
#include "ircd.h"
#include "mtrie_conf.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_log.h"
#include "s_misc.h"
#include "send.h"
#include "struct.h"
#ifdef GLINES
#include "m_gline.h"
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

extern ConfigFileEntryType ConfigFileEntry; /* defined in ircd.c */

/*
** m_ungline
** added May 29th 2000 by Toby Verrall <toot@melnet.co.uk>
**
**      parv[0] = sender nick
**      parv[1] = gline to remove
*/

int
m_ungline (aClient *cptr,aClient *sptr,int parc,char *parv[])
{
#ifdef GLINES

  char  *user,*host;

  if (check_registered(sptr))
    {
      return -1;
    }

  if (!IsOper(sptr))
    {
      sendto_one(sptr, form_str(ERR_NOPRIVILEGES), me.name,
                 parv[0]);
      return 0;
    }

  if (!IsSetOperUnkline(sptr) || !IsSetOperGline(sptr))
    {
      sendto_one(sptr,":%s NOTICE %s :You have no U and G flag",
                 me.name,parv[0]);
      return 0;
    }

  if ( parc < 2 )
    {
      sendto_one(sptr, form_str(ERR_NEEDMOREPARAMS),
                 me.name, parv[0], "UNGLINE");
      return 0;
    }

  if ((host = strchr(parv[1], '@')) || *parv[1] == '*')
    {
      /* Explicit user@host mask given */

      if(host != NULL)		/* Found user@host */
        {
          user = parv[1];       /* here is user part */
          *(host++) = '\0';     /* and now here is host */
        }
      else
        {
          user = "*";           /* no @ found, assume its *@somehost */
          host = parv[1];
        }
    }
  else
    {
      sendto_one(sptr, ":%s NOTICE %s :Invalid parameters",
                 me.name, parv[0]);
      return 0;
    }

  if(remove_gline_match(user, host))
    {
      sendto_one(sptr, ":%s NOTICE %s :Un-glined [%s@%s]",
                 me.name, parv[0],user, host);
      sendto_ops("%s has removed the G-Line for: [%s@%s]",
                 parv[0], user, host );
      ilog(L_NOTICE, "%s removed G-Line for [%s@%s]",
          parv[0], user, host);
      return 0;
    }
  else
    {
      sendto_one(sptr, ":%s NOTICE %s :No G-Line for %s@%s",
                 me.name, parv[0],user,host);
      return 0;
    }
#else
  sendto_one(sptr,":%s NOTICE %s :UNGLINE disabled",me.name,parv[0]);
#endif
  return 0;
}
