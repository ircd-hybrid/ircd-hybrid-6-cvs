/************************************************************************
 *   IRC - Internet Relay Chat, src/m_unresv.c
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
 *   $Id: m_unresv.c,v 1.1 2004/05/23 21:07:01 ievil Exp $
 */

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

/* m_unresv()
 *
 * inputs       - local source, actual source, parc, parv
 * outputs      - none
 * side effects - propogate
 */

int m_unresv(struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{
  if (IsServer(cptr))
  {
#ifdef HUB
    if (parc == 3)
      sendto_match_cap_servs(NULL, cptr, CAP_CLUSTER, ":%s UNRESV %s %s",
                             parv[0], parv[1], parv[2]);
#endif  /* HUB */
    return 0;
  }
  return 0;
}

