/************************************************************************
 *   IRC - Internet Relay Chat, src/m_operspylog.c
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
 *   $Id: m_operspylog.c,v 1.4 2003/06/24 03:14:32 ievil Exp $
 */

#include "m_operspylog.h"
#include "m_commands.h"
#include "client.h"
#include "ircd.h"
#include "s_serv.h"
#include "irc_string.h"
#include "fileio.h"
#include "s_misc.h"
#include "send.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void operspy_log(struct Client *, const char *, const char *);

/* ms_operspylog()
 *
 * inputs	- local source, actual source, parc, parv
 * outputs	- none
 * side effects	- propogate, parse if understood
 */

int ms_operspylog(struct Client *cptr, struct Client *sptr, int parc, char *parv[])
{
  /* only for logging */
  if (IsPerson(cptr))
    return 0;
#ifdef OPERSPYLOG
  operspy_log(sptr, parv[1], parv[2]);
#endif
  return 0;
}

#ifdef OPERSPYLOG
/* operspy_log()
 *
 * inputs	- client source
 * outputs	- none
 * side effects	- logs operspy usage
 */
void operspy_log(struct Client *cptr, const char *command, const char *target)
{
  /* "nick!user@host{server}\0" */
  char oper_name[NICKLEN + 1 + USERLEN + 1 + HOSTLEN + 1 + HOSTLEN + 2];
  const char *current_date;
  static char buf[BUFSIZE];
  int logfile;

  current_date = smalldate((time_t) 0);
    
  if (MyClient(cptr))
  {
    ircsprintf(oper_name, "%s!%s@%s",
               cptr->name, cptr->username, cptr->host);
               
    sendto_match_cap_servs_butone(cptr, 0, "*", CAP_ENCAP, "ENCAP * OPERSPY %s :%s",command, target);
/*    sendto_match_servs(cptr, "*", CAP_ENCAP, "ENCAP * OPERSPY %s :%s",
 *                      command, target);
 */
#ifdef FNAME_OPERSPYLOG
    if ((logfile = open(FNAME_OPERSPYLOG, O_WRONLY|O_APPEND)) != -1)
      {
        ircsprintf(buf, "[%s] OPERSPY %s %s %s\n",
                   current_date, oper_name, command, target);
        write(logfile, buf, strlen(buf));
        close(logfile);
      }
#endif
  }
  else
  {
    ircsprintf(oper_name, "%s!%s@%s{%s}",
               cptr->name, cptr->username, cptr->host, cptr->user->server);
#ifdef FNAME_OPERSPYRLOG
    if ((logfile = open(FNAME_OPERSPYRLOG, O_WRONLY|O_APPEND)) != -1)
      {
        ircsprintf(buf, "[%s] OPERSPY %s %s %s\n",
                   current_date, oper_name, command, target);
        write(logfile, buf, strlen(buf));
        close(logfile);
      }
#endif
  }
  sendto_realops_flags(FLAGS_OSPYLOG, "OPERSPY %s %s %s", oper_name, command, target);
}

#endif /* OPERSPYLOG */
