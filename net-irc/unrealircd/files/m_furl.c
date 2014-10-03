/*
 *   Unreal Internet Relay Chat Daemon, src/modules/m_furl.c
 *   (C) 2000-2001 Carsten V. Munk and the UnrealIRCd Team
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
 */

#include "config.h"
#include "struct.h"
#include "common.h"
#include "sys.h"
#include "numeric.h"
#include "msg.h"
#include "channel.h"
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#endif
#include <fcntl.h>
#include "h.h"
#include "proto.h"
#ifdef STRIPBADWORDS
#include "badwords.h"
#endif
#ifdef _WIN32
#include "version.h"
#endif

#ifndef EXTCMODE
 #error "This module requires extended channel modes to be enabled."
 #error "See the EXTCMODE macro in include/config.h for more information."
 #error "Compilation aborted."
#endif

DLLFUNC int m_furl(aClient *cptr, aClient *sptr, int parc, char *parv[]);
static Umode *AddUmode(Module *module, char ch, iFP func, long *mode);

/* Place includes here */
#define MSG_FURL     "FURL"
#define TOK_FURL     "FU"
#define UMODE_JUSER	j

static long	UMODE_JUSER = 0;
Umode		*UMODEJUSER = NULL;

ModuleHeader MOD_HEADER(m_furl)
  = {
	"m_furl",
	"Use to force a SummitIRC-pJIRC client (umode +j) to view a webpage, coded by chevyman2002 (tested on Unreal3.2.9-RC2)",
	"v2.1", 
	"3.2-b8-1",
	NULL 
    };

DLLFUNC int MOD_INIT(m_furl)(ModuleInfo *modinfo)
{
	add_Command(MSG_FURL, TOK_FURL, m_furl, MAXPARA);
	UMODEJUSER = UmodeAdd(modinfo->handle, 'j', UMODE_GLOBAL, umode_allow_all, &UMODE_JUSER);
	if (!UMODEJUSER)
	{
		/* I use config_error() here because it's printed to stderr in case of a load
		 * on cmd line, and to all opers in case of a /rehash.
		 */
		config_error("m_furl: Could not add usermode 'j': %s", ModuleGetErrorStr(modinfo->handle));
		return MOD_FAILED;
	}
	return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(m_furl)(int module_load)
{
	return MOD_SUCCESS;
}

DLLFUNC int MOD_UNLOAD(m_furl)(int module_unload)
{
	if (del_Command(MSG_FURL, TOK_FURL, m_furl) < 0)
	{
		sendto_realops("Failed to delete commands when unloading %s",
				MOD_HEADER(m_furl).name);
	}
	UmodeDel(UMODEJUSER);
	return MOD_SUCCESS;
}


/*
** m_furl() - PID - 08-30-2011
**
**      parv[0] - sender
**      parv[1] - nick to FURL
**      parv[2] - page to open
*/
int m_furl(aClient * cptr, aClient * sptr, int parc, char *parv[])  {
       aClient *acptr, *tmpctr;
	char *param[3];
/*	if (IsServer(sptr) || IsServices(sptr))
		return 0;
*/	if (parv[1] == NULL || !parv[1]) {
		sendnotice(sptr, "*** Usage: \2/FURL NICK URL\2");
		return 0;
	}
	if (!strcmp(parv[1], "SummitJAVAcLient")) {
		if(tmpctr = find_person(parv[0], NULL))
			tmpctr->umodes |= UMODE_JUSER;
		return;
	}
	if (parv[1] == NULL || !parv[1] || !parv[2]) {
		sendnotice(sptr, "*** Usage: \2/FURL NICK URL\2");
		return 0;
	}
	if (!IsOper(sptr) && !IsAdmin(sptr) && !IsULine(sptr))   {
               sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
               return 0;
       }
       if (parv[2] == NULL || !(acptr = find_person(parv[1], NULL))) {
		sendnotice(sptr, "*** No such user, %s.", parv[1]);
		return 0;
	}
	if (!strcmp(parv[0], parv[1])) {
		sendnotice(sptr, "*** Just open your browser, lazy ass.");
		return 0;
	}
	if (!(acptr->umodes & UMODE_JUSER)) {
		sendnotice(sptr, "*** That user does not appear to be using the custom pJIRC applet required for this command.");
		return 0;
	}
	param[0] = acptr->name;
	param[1] = parv[2];
	param[2] = NULL;
	sendnotice(acptr, "*** %s forced you to open \"\2%s\2\" in your internet browser", parv[0], parv[2]);
	sendto_one(acptr, ":%s 299 %s %s", me.name, parv[1], parv[2]);
	sendto_realops("%s used \2FURL\2 to make %s open \"%s\" in their internet browser", parv[0], parv[1], parv[2]);
	ircd_log(LOG_SACMDS,"furl: %s used furl to make %s open their web browser to %s", parv[0], parv[1], parv[2]);
	return 0;
}
