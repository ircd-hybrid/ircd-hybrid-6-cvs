/************************************************************************
 *   IRC - Internet Relay Chat, include/operspylog.h
 *   Copyright (C) 2003 Hybrid Development Team
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
 * $Id: m_operspylog.h,v 1.1 2003/06/14 23:26:25 ievil Exp $
 */

#ifndef INCLUDED_OPERSPYLOG_h
#define INCLUDED_OPERSPYLOG_h

#ifdef OPERSPY
#ifndef OPERSPYLOG
#define OPERSPYLOG
#endif
#endif

struct Client;


/* enable this to send all operspy usage to +Z (UMODE_SPYLOG) opers */
#define OPERSPY_NOTICE
extern void operspy_log(struct Client *, const char *, const char *);


#endif /* INCLUDED_fdlist_h */

