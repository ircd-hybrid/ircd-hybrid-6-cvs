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
 *   $Id: m_unkline.c,v 1.54 2003/06/24 03:57:16 ievil Exp $
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
#include "s_serv.h"
#include "send.h"
#include "struct.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

extern ConfigFileEntryType ConfigFileEntry; /* defined in ircd.c */

static int flush_write(aClient *, FBFILE *in, FBFILE *out , char *, char *);
static int remove_tkline_match(char *,char *, unsigned long);

/*
** m_unkline
** Added Aug 31, 1997 
** common (Keith Fralick) fralick@gate.net
**
**      parv[0] = sender
**      parv[1] = address to remove
*
*
* re-worked and cleanedup for use in hybrid-5 
* -Dianora
*
*/
int m_unkline (aClient *cptr,aClient *sptr,int parc,char *parv[])
{
  FBFILE* in;
  FBFILE* out;
  int   pairme = NO;
  char  buf[BUFSIZE];
  char  buff[BUFSIZE];  /* matches line definition in s_conf.c */
  char  temppath[BUFSIZE];

  const char  *filename;                /* filename to use for unkline */

  char  *user,*host;
  char  *p;
  mode_t oldumask;
  unsigned long ip;
  unsigned long ip_mask;

  ircsprintf(temppath, "%s%s.tmp", ConfigFileEntry.dpath,
	     ConfigFileEntry.klinefile);
  
  if (IsServer(cptr))
    {
#ifdef HUB
      if (parc == 4)
         sendto_match_cap_servs(NULL, cptr, CAP_UNKLN,":%s UNKLINE %s %s %s",
                                parv[0], parv[1], parv[2], parv[3]);
#endif
      return 0;
    }

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
      sendto_one(sptr,":%s NOTICE %s :You have no U flag",me.name,parv[0]);
      return 0;
    }

  if (parc < 2)
    {
      sendto_one(sptr, form_str(ERR_NEEDMOREPARAMS),
                 me.name, parv[0], "UNKLINE");
      return 0;
    }

  if ((host = strchr(parv[1], '@')) || *parv[1] == '*')
    {
      /* Explicit user@host mask given */

      if (host != NULL)		/* Found user@host */
        {
          user = parv[1];	/* here is user part */
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

  if ((user[0] == '*') && (user[1] == '\0')
      && (host[0] == '*') && (host[1] == '\0') )
    {
      sendto_one(sptr, ":%s NOTICE %s :Cannot UNK-Line everyone",
                 me.name, parv[0]);
      return 0;
    }

  if (!(is_address(host, &ip, &ip_mask)))
    ip = 0L;
    
  if (remove_tkline_match(host,user,(unsigned long)ip))
    {
      sendto_one(sptr, ":%s NOTICE %s :Un-klined [%s@%s] from temporary k-lines",
                 me.name, parv[0],user, host);
      sendto_ops("%s has removed the temporary K-Line for: [%s@%s]",
                 parv[0], user, host );

      ilog(L_NOTICE, "%s removed temporary K-Line for [%s@%s]",
          parv[0], user, host);
      return 0;
    }

  filename = get_conf_name(KLINE_TYPE);

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
#Dianora!db@ts2-11.ottawa.net K'd: foo@bar:No reason
K:bar:No reason (1997/08/30 14.56):foo
*/

  while(fbgets(buf, sizeof(buf), in)) 
    {
      if ((buf[1] == ':') && ((buf[0] == 'k') || (buf[0] == 'K')))
        {
          /* its a K: line */
          char *found_host;
          char *found_user;
          char *found_comment;

          strncpy_irc(buff, buf, BUFSIZE);      /* extra paranoia */

          if ((p = strchr(buff,'\n')) != NULL)
            *p = '\0';

          found_host = buff + 2;        /* point past the K: */
          if ((p = strchr(found_host, ':')) == NULL)
            {
              sendto_one(sptr, ":%s NOTICE %s :K-Line file corrupted",
                         me.name, parv[0]);
              sendto_one(sptr, ":%s NOTICE %s :Couldn't find host",
                         me.name, parv[0]);
              ilog(L_ERROR, "K-Line file corrupted (couldn't find host)");
              if (flush_write(sptr, in, out, buf, temppath) < 0)
		return 0;
	      continue;
            }
          *p++ = '\0';
 
          found_comment = p;
          if ((p = strchr(found_comment, ':')) == NULL)
            {
              sendto_one(sptr, ":%s NOTICE %s :K-Line file corrupted",
                         me.name, parv[0]);
              sendto_one(sptr, ":%s NOTICE %s :Couldn't find comment",
                         me.name, parv[0]);
              ilog(L_ERROR, "K-Line file corrupted (couldn't find comment)");
              if (flush_write(sptr, in, out, buf, temppath) < 0)
		return 0;
	      continue;
            }
          *p++ = '\0';
          found_user = p;

/*
 * Ok, if its not an exact match on either the user or the host
 * then, write the K: line out, and I add it back to the K line
 * tree
 */
          if (irccmp(host,found_host) || irccmp(user,found_user))
            {
	      if (flush_write(sptr, in, out, buf, temppath) < 0)
		return 0;
	      continue;
            }
          else
            pairme++;

        }                               
      else if(buf[0] == '#')
        {
          char *userathost;
          char *found_user;
          char *found_host;

          strncpy_irc(buff, buf, BUFSIZE);
/*
#Dianora!db@ts2-11.ottawa.net K'd: foo@bar:No reason
K:bar:No reason (1997/08/30 14.56):foo

If its a comment coment line, i.e.
#ignore this line
Then just ignore the line
*/
          if ((p = strchr(buff, ':')) == NULL)
            {
              if (flush_write(sptr, in, out, buf, temppath) < 0)
		return 0;
              continue;
            }
          *p++ = '\0';

          userathost = p;

          if ((p = strchr(userathost, ':')) == NULL)
            {
              if (flush_write(sptr, in, out, buf, temppath) < 0)
		return 0;
              continue;
            }
          *p = '\0';

          while(*userathost == ' ')
            userathost++;

          found_user = userathost;
          if ((p = strchr(found_user, '@')) == NULL)
            {
              if (flush_write(sptr, in, out, buf, temppath) < 0)
		return 0;
	      continue;
            }
          *p = '\0';
          found_host = p;
          found_host++;

          if ((irccmp(found_host,host)) || (irccmp(found_user,user)))
            {
              if (flush_write(sptr, in, out, buf, temppath) < 0)
		return 0;
	      continue;
            }
        }
      else      /* its the ircd.conf file, and not a K line or comment */
        {
          if (flush_write(sptr, in, out, buf, temppath) < 0)
	    return 0;
	  continue;
        }
    }

  fbclose(in);

/* The result of the rename should be checked too... oh well */
/* If there was an error on a write above, then its been reported
 * and I am not going to trash the original kline /conf file
 * -Dianora
 */
      fbclose(out);
      (void)rename(temppath, filename);
      rehash(cptr,sptr,0);

  if (!pairme)
    {
      sendto_one(sptr, ":%s NOTICE %s :No K-Line for %s@%s",
                 me.name, parv[0],user,host);
      return 0;
    }
  sendto_one(sptr, ":%s NOTICE %s :K-Line for [%s@%s] is removed", 
             me.name, parv[0], user,host);
  sendto_realops("%s has removed the K-Line for: [%s@%s]",
             parv[0], user, host);

  ilog(L_NOTICE, "%s removed K-Line for [%s@%s]", parv[0], user, host);
  return 0; 
}

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
** remove_tkline_match()
*
* un-kline a temporary k-line. 
*
*/
static int remove_tkline_match(char *host,char *user, unsigned long ip)
{
  aConfItem *kill_list_ptr;
  aConfItem *last_kill_ptr=(aConfItem *)NULL;

  if(!temporary_klines && !temporary_ip_klines)
    return NO;

  if(ip != 0)
  {
    kill_list_ptr = temporary_ip_klines;

    while(kill_list_ptr)
    {
      if((ip & kill_list_ptr->ip_mask) == kill_list_ptr->ip)
      {
        if(last_kill_ptr)
	  last_kill_ptr->next = kill_list_ptr->next;
	else
	  temporary_ip_klines = kill_list_ptr->next;
	free_conf(kill_list_ptr);
	return YES;
      }
      else
      {
        last_kill_ptr = kill_list_ptr;

        kill_list_ptr = kill_list_ptr->next;
      }
    }

    return NO;
  }
  else
  {
    kill_list_ptr = temporary_klines;

    while(kill_list_ptr)
    {
      if( !irccmp(kill_list_ptr->host,host)
          && !irccmp(kill_list_ptr->user,user)) /* match */
        {
          if(last_kill_ptr)
            last_kill_ptr->next = kill_list_ptr->next;
          else
            temporary_klines = kill_list_ptr->next;
          free_conf(kill_list_ptr);
          return YES;
        }
      else
        {
          last_kill_ptr = kill_list_ptr;
          kill_list_ptr = kill_list_ptr->next;
        }
    }
    return NO;
  }
}
