/************************************************************************
 *   IRC - Internet Relay Chat, include/m_encap.h
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
 * $Id: m_encap.h,v 1.1 2004/05/23 16:33:54 ievil Exp $
 */

#ifndef INCLUDED_m_encap_h
#define INCLUDED_m_encap_h

#define E_DENY 0
#define E_ACCEPT 1

typedef struct ACL_ENCAP
{
  char* cmd;  /* the command */
  int   acl;  /* allow or deny */
} acl_encap;

acl_encap encap_acl[] = {
  { "OPERSPY", E_ACCEPT },
  { 0, E_DENY }
};


#endif /* INCLUDED_m_encap_h */

