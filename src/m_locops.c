/************************************************************************
 *   IRC - Internet Relay Chat, src/m_locops.c
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
 *   $Id: m_locops.c,v 1.3 2003/01/05 19:47:47 gregp Exp $
 */
#include "m_commands.h"
#include "client.h"
#include "ircd.h"
#include "irc_string.h"
#include "numeric.h"
#include "send.h"
#include "s_user.h"
#include "s_conf.h"
#include "hash.h"

/*
 * m_functions execute protocol messages on this server:
 *
 *      cptr    is always NON-NULL, pointing to a *LOCAL* client
 *              structure (with an open socket connected!). This
 *              identifies the physical socket where the message
 *              originated (or which caused the m_function to be
 *              executed--some m_functions may call others...).
 *
 *      sptr    is the source of the message, defined by the
 *              prefix part of the message if present. If not
 *              or prefix not found, then sptr==cptr.
 *
 *              (!IsServer(cptr)) => (cptr == sptr), because
 *              prefixes are taken *only* from servers...
 *
 *              (IsServer(cptr))
 *                      (sptr == cptr) => the message didn't
 *                      have the prefix.
 *
 *                      (sptr != cptr && IsServer(sptr) means
 *                      the prefix specified servername. (?)
 *
 *                      (sptr != cptr && !IsServer(sptr) means
 *                      that message originated from a remote
 *                      user (not local).
 *
 *              combining
 *
 *              (!IsServer(sptr)) means that, sptr can safely
 *              taken as defining the target structure of the
 *              message in this server.
 *
 *      *Always* true (if 'parse' and others are working correct):
 *
 *      1)      sptr->from == cptr  (note: cptr->from == cptr)
 *
 *      2)      MyConnect(sptr) <=> sptr == cptr (e.g. sptr
 *              *cannot* be a local connection, unless it's
 *              actually cptr!). [MyConnect(x) should probably
 *              be defined as (x == x->from) --msa ]
 *
 *      parc    number of variable parameter strings (if zero,
 *              parv is allowed to be NULL)
 *
 *      parv    a NULL terminated list of parameter pointers,
 *
 *                      parv[0], sender (prefix string), if not present
 *                              this points to an empty string.
 *                      parv[1]...parv[parc-1]
 *                              pointers to additional parameters
 *                      parv[parc] == NULL, *always*
 *
 *              note:   it is guaranteed that parv[0]..parv[parc-1] are all
 *                      non-NULL pointers.
 */

/*
 * m_locops - LOCOPS message handler
 * (write to *all* local opers currently online)
 *      parv[0] = sender prefix
 *      parv[1] = message text
 */
int m_locops(struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{
  char *message = NULL;
#ifdef SLAVE_SERVERS
  char *slave_oper;
  struct Client *acptr;
  struct ConfItem *aconf;
#endif

  if (IsServer(sptr))
  {
#ifdef SLAVE_SERVERS
	  aconf = find_special_conf(sptr->name, CONF_ULINE);
	  if (!aconf)
	  {
		  sendto_realops("Received LOCOPS from non-slave server %s", sptr->name);
		  return 0;
	  }
	  else if (!(aconf->port & ULFL_ACCEPTLOCOPS))
	  {
		  sendto_realops("Refused LOCOPS from unauthorized slave %s", sptr->name);
		  return 0;
	  }

	  if (parc > 2)
	  {
		  slave_oper = parv[1];
		  message = parv[2];

		  if ((acptr = hash_find_client(slave_oper, NULL)) && IsPerson(acptr))
		  {
			  /* is it not pointless to check if parv[1] != NULL, since if it's
			   * not true parse() is busted? */
			  send_operwall(acptr, "SLOCOPS", message);
#ifdef HUB
			  sendto_slaves(sptr, ULFL_SENDLOCOPS, "LOCOPS", slave_oper, parc - 1, parv + 1);
#endif /* HUB */

			  return 0;
		  }
		  else
			  sendto_realops("Received slave LOCOPS from non-existant client %s", slave_oper);
	  }
#endif /* SLAVE_SERVERS */
      return 0;
  }

  message = parc > 1 ? parv[1] : NULL;

  if (EmptyString(message))
    {
      sendto_one(sptr, form_str(ERR_NEEDMOREPARAMS),
                 me.name, parv[0], "LOCOPS");
      return 0;
    }

  if(MyConnect(sptr) && IsAnOper(sptr))
    {

#ifdef SLAVE_SERVERS
      sendto_slaves(NULL, ULFL_SENDLOCOPS, "LOCOPS", sptr->name, parc, parv);
#endif
      send_operwall(sptr, "LOCOPS", message);
    }
  else
    {
      sendto_one(sptr, form_str(ERR_NOPRIVILEGES), me.name, parv[0]);
      return(0);
    }

  return(0);
}


