/************************************************************************
 *   IRC - Internet Relay Chat, src/m_unkline.c
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
 *   $Id: m_undline.c,v 1.2 2003/06/24 03:57:16 ievil Exp $
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

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

extern ConfigFileEntryType ConfigFileEntry; /* defined in ircd.c */

static int flush_write(aClient *, FBFILE *in, FBFILE *out , char *, char *);

/*
 * flush_write()
 *
 * inputs       - pointer to client structure of oper requesting unkline
 *              - out is the file descriptor
 *              - buf is the buffer to write
 *              - ntowrite is the expected number of character to be written
 *              - temppath is the temporary file name to be written
 * output       - -1 for error on write
 *              - 0 for success
 * side effects - if successful, the buf is written to output file
 *                if a write failure happesn, and the file pointed to
 *                by temppath, if its non NULL, is removed.
 *
 * The idea here is, to be as robust as possible when writing to the 
 * kline file.
 *
 * -Dianora
 */
static int
flush_write(aClient *sptr, FBFILE *in, FBFILE *out, char *buf, char *temppath)
{
  int error_on_write = (fbputs(buf, out) < 0) ? YES : NO;

  if (error_on_write)
    {
      sendto_one(sptr,":%s NOTICE %s :Unable to write to %s",
        me.name, sptr->name, temppath );
      fbclose(in);
      fbclose(out);
      if(temppath != (char *)NULL)
        (void)unlink(temppath);
    }
  return(error_on_write);
}

/*
** m_undline
** added May 28th 2000 by Toby Verrall <toot@melnet.co.uk>
** based totally on m_unkline
**
**      parv[0] = sender nick
**      parv[1] = dline to remove
*/
int
m_undline (aClient *cptr,aClient *sptr,int parc,char *parv[])
{
  FBFILE* in;
  FBFILE* out;
  int   pairme = NO;
  char  buf[BUFSIZE];
  char  buff[BUFSIZE];  /* matches line definition in s_conf.c */
  char  temppath[256];

  const char  *filename;                /* filename to use for undline */

  char  *cidr;
  char  *p;
  unsigned long ip_host;
  unsigned long ip_mask;
  mode_t oldumask;

  ircsprintf(temppath, "%s.tmp", ConfigFileEntry.dlinefile);

  if (check_registered(sptr))
    {
      return -1;
    }

  if (!IsAnOper(sptr))
    {
      sendto_one(sptr, form_str(ERR_NOPRIVILEGES), me.name,
                 parv[0]);
      return 0;
    }

  if (!IsSetOperUnkline(sptr))
    {
      sendto_one(sptr,":%s NOTICE %s :You have no U flag",me.name,
                 parv[0]);
      return 0;
    }

  if ( parc < 2 )
    {
      sendto_one(sptr, form_str(ERR_NEEDMOREPARAMS),
                 me.name, parv[0], "UNDLINE");
      return 0;
    }
  
  cidr = parv[1];

  if (!is_address(cidr,&ip_host,&ip_mask))
    {
      sendto_one(sptr, ":%s NOTICE %s :Invalid parameters",
                 me.name, parv[0]);
      return 0;
    }

  filename = get_conf_name(DLINE_TYPE);

  if ((in = fbopen(filename, "r")) == 0)
    {
      sendto_one(sptr, ":%s NOTICE %s :Cannot open %s",
        me.name,parv[0],filename);
      return 0;
    }

  oldumask = umask(0);                  /* ircd is normally too paranoid */
  if ((out = fbopen(temppath, "w")) == 0)
    {
      sendto_one(sptr, ":%s NOTICE %s :Cannot open %s",
        me.name,parv[0],temppath);
      fbclose(in);
      umask(oldumask);                  /* Restore the old umask */
      return 0;
    }
  umask(oldumask);                    /* Restore the old umask */


/*
#toot!~toot@127.0.0.1 D'd: 123.4.5.0/24:test (2000/05/28 12.48)
D:123.4.5.0/24:test (2000/05/28 12.48)
*/

  while(fbgets(buf, sizeof(buf), in))
    {
      if ((buf[1] == ':') && ((buf[0] == 'd') || (buf[0] == 'D')))
        {
          /* its a D: line */
          char *found_cidr;

          strncpy_irc(buff, buf, BUFSIZE);      /* extra paranoia */

          if((p = strchr(buff, '\n')) != NULL)
            *p = '\0';

          found_cidr = buff + 2;        /* point past the D: */
          if((p = strchr(found_cidr, ':'))  == NULL)
            {
              sendto_one(sptr, ":%s NOTICE %s :D-Line file corrupted",
                         me.name, parv[0]);
              sendto_one(sptr, ":%s NOTICE %s :Couldn't find CIDR",
                         me.name, parv[0]);
              if (flush_write(sptr, in, out, buf, temppath) < 0)
		return 0;
	      continue;
            }
         *p = '\0';   
         
         if(irccmp(cidr,found_cidr))
            {
	      if(flush_write(sptr, in, out, buf, temppath) < 0)
		return 0;
	      continue;
            }
          else
            pairme++;

        } 
      else if(buf[0] == '#')
        {
          char *found_cidr;

          strncpy_irc(buff, buf, BUFSIZE);

/*
#toot!~toot@127.0.0.1 D'd: 123.4.5.0/24:test (2000/05/28 12.48)
D:123.4.5.0/24:test (2000/05/28 12.48)

If its a comment coment line, i.e.
#ignore this line
Then just ignore the line
*/

          if((p = strchr(buff, ':')) == NULL)
            {
              if (flush_write(sptr, in, out, buf, temppath) < 0)
		return 0;
	      continue;
            }
          *p++ = '\0';

          found_cidr = p;

          if ((p = strchr(found_cidr, ':')) == NULL)
            {
	      if (flush_write(sptr, in, out, buf, temppath) < 0)
		return 0;
	      continue;
            }
          *p = '\0';

          while(*found_cidr == ' ')
            found_cidr++;

          if ((irccmp(found_cidr,cidr)))
            {
	      if (flush_write(sptr, in, out, buf, temppath) < 0)
		return 0;
	      continue;
            }
        }

      else      /* its the ircd.conf file, and not a D line or comment */
        {
	  if (flush_write(sptr, in, out, buf, temppath) < 0)
	    return 0;
	  continue;
        }
    }

  fbclose(in);
  fbclose(out);
  (void)rename(temppath, filename);
  rehash(cptr,sptr,0);

  if(!pairme)
    {
      sendto_one(sptr, ":%s NOTICE %s :No D-Line for %s",
                 me.name, parv[0],cidr);
      return 0;
    }

  sendto_one(sptr, ":%s NOTICE %s :D-Line for [%s] is removed",
             me.name, parv[0], cidr);
  sendto_ops("%s has removed the D-Line for: [%s]",
             parv[0], cidr);

  ilog(L_NOTICE, "%s removed D-Line for [%s]", parv[0], cidr);
  return 0;
}
