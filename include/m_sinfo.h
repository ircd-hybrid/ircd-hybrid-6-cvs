/************************************************************************
 *   IRC - Internet Relay Chat, include/m_sinfo.h
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
 * $Id: m_sinfo.h,v 1.1 2004/05/23 16:52:50 ievil Exp $
 */

#ifndef INCLUDED_m_sinfo_h
#define INCLUDED_m_sinfo_h

#define TOKEN_NONE      0
#define TOKEN_SPLIT     1
#define TOKEN_KLINES    2
#define TOKEN_DLINES    3
#define TOKEN_XLINES    4
#define TOKEN_TKLINES   5

typedef struct SINFO_LIST
{
  char* cmd;     /* the command */
  int   token;   /* token */
} list_sinfo;

list_sinfo lsinfo[] = {
  { "SPLIT",    TOKEN_SPLIT },
  { "KLINES",   TOKEN_KLINES },
  { "DLINES",   TOKEN_DLINES },
  { "XLINES",   TOKEN_XLINES },
  { "TKLINES",  TOKEN_TKLINES },
  { 0,          TOKEN_NONE }
};

#endif /* INCLUDED_m_sinfo_h */


