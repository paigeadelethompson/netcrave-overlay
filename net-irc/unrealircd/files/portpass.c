/*
 * Turkish :
 * - Bu Modul Sky-Dancer Tarafindan Yazilmis/Editlenmistir.
 * - Bu Modulu Kullaniyorsaniz Telif Hakki Yazilarini Silemezsiniz.
 * - Bu Modulu Kullanarak LICENSE.UNOFF.TR adli okuma dosyasi icindeki Lisansi
 *   Kabul Etmis Sayiliyorsunuz.
 * - Dagitim Tarihi : 09 / 01 / 2007
 */

/*
 * English :
 * - This Module Codded/Edited By Sky-Dancer.
 * - if you are using this module, you can not delete copyright texts on this module.
 * - if you are using this module, You Are Accepting The License in LICENSE.ENG named file.
 * - Release Date : 09 / 01 / 2007
 */

 /*
  * Sorry For Something Before 1.0.4 Version, FOR MY LIMITED TIME!
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
#ifdef _WIN32
#include "version.h"
#endif

// For English Version (ingilizce Versiyon icin)
#define MODENG
// For Turkish Version (Turkce Versiyon icin)
//#undef MODENG 

/* Externs */
//extern void sendto_one(aClient *to, char *pattern, ...);

/* Prototypes */
void Delete_Hook(Hook *Dlt);
static void ConfPassEkle(char *mask);
int ConfPassOku(aClient *sptr);
int ConfPassSwhois(aClient *aptr);
DLLFUNC int kullanici_girisi(aClient *);
// For Config.
DLLFUNC int Call_ConfigRun(ConfigFile *File, ConfigEntry *Entry, int Type);
DLLFUNC int Call_ConfigTest(ConfigFile *File, ConfigEntry *Entry, int Type, int *Error);
DLLFUNC int Call_ConfigRehash();

/* Variables */
static Hook *ConfigTest = NULL, *ConfigRun = NULL, *ConfigRehash = NULL;
static Hook *PreConnect = NULL;
static ConfigItem_except	*portpass = NULL;

#define LANG_MODHEAD "Add Passwords & SWHOIS Info to Specific Ports (By Sky-Dancer, swhois added by chevyman2002)"
#define LANG_NOPASS "You Must Enter a Password"
#define LANG_PASSTOOLONG "Your Password is Too Long"
#define LANG_WRONGPASS "Your Password is Wrong"
#define LANG_LENGTHY "Your swhois info is too long"
//#define LANG_CORRECTPASSWORD "Congratulations, Your Password Accepted."

#ifndef STATIC_LINKING
static ModuleInfo	*Mod_portpasssi;
#endif

char *swhois;

/* Main Module Header For UnrealIRCd */
ModuleHeader MOD_HEADER(portpass) = {
	"portpass",
	LANG_MODHEAD,
	"v2.0",
	"3.2-b8-1",
	NULL 
};

/* MOD_TEST Function */
DLLFUNC int MOD_TEST(portpass)(ModuleInfo *modinfo) {
	ConfigTest	= HookAddEx(modinfo->handle, HOOKTYPE_CONFIGTEST, Call_ConfigTest);
return MOD_SUCCESS;
}

/* MOD_INIT Function */
DLLFUNC int MOD_INIT(portpass)(ModuleInfo *modinfo) {
#ifndef STATIC_LINKING
	// Maybe, later uses :)
	Mod_portpasssi = modinfo;
#endif
	PreConnect	= HookAddEx(modinfo->handle, HOOKTYPE_PRE_LOCAL_CONNECT, kullanici_girisi);
	ConfigRun		= HookAddEx(modinfo->handle, HOOKTYPE_CONFIGRUN, Call_ConfigRun);
	ConfigRehash	= HookAddEx(modinfo->handle, HOOKTYPE_REHASH, Call_ConfigRehash);
return MOD_SUCCESS;
}

/* MOD_LOAD Function */
DLLFUNC int MOD_LOAD(portpass)(int module_load) {
return MOD_SUCCESS;
}

/* MOD_UNLOAD Function */
DLLFUNC int MOD_UNLOAD(portpass)(int module_unload) {
	Delete_Hook(ConfigTest);
	Delete_Hook(ConfigTest);
	Delete_Hook(ConfigRun);
	Delete_Hook(ConfigRehash);
	Delete_Hook(PreConnect);
return MOD_SUCCESS;
}

void Delete_Hook(Hook *Dlt) {
	if (Dlt) {
		HookDel(Dlt);
		Dlt = NULL;
	}
}

/* Local User Connect Hook */
DLLFUNC int kullanici_girisi(aClient *sptr) {
	int ret, retl;
	ret = ConfPassOku(sptr);
	switch (ret) {
		case 1:
		return exit_client(sptr, sptr, sptr, LANG_NOPASS);
		break;

		case 2:
		return exit_client(sptr, sptr, sptr, LANG_PASSTOOLONG);
		break;

		case 3:
		return exit_client(sptr, sptr, sptr, LANG_WRONGPASS);
		break;

		case 4:
//		sendto_one(sptr, ":%s NOTICE %s :*** "LANG_CORRECTPASSWORD, me.name, sptr->name);
		retl = ConfPassSwhois(sptr);
		if(retl == 0)
			return exit_client(sptr, sptr, sptr, LANG_LENGTHY);
		if(!strcmp(swhois,"na"))
			break;
		if(sptr->user->swhois)
			MyFree(sptr->user->swhois);
		sptr->user->swhois = strdup(swhois);
		//sptr->user->swhois = strdup("is using a Web Connection");
		break;

		default:
		break;
	}
return 0;
}

/* Adding passwords to memory from confs reads */
static void ConfPassEkle(char *mask) {
	char maxbuff[80];
	memset(maxbuff,0,sizeof(maxbuff));
	strncpy(maxbuff,mask,75);
	ConfigItem_except *e;
	e = (ConfigItem_except *) MyMallocEx(sizeof(ConfigItem_except));
	e->mask = strdup(maxbuff);
	AddListItem(e, portpass);
}

/* Read the config file for swhois info */
int ConfPassSwhois(aClient *aptr) {
	char *myport = NULL, *mypass, *mywhois;
	ConfigItem_except *e;
	char line[512];
	int len = 0;
	for (e = portpass; e; e = (ConfigItem_except *) e->next) {
		strcpy(line,e->mask);
		myport = strtok(line, ":");
		if (myport) {
			mypass = strtok(NULL, ":");
			mywhois = strtok(NULL, ":");
			if(NULL != mywhois)
				len = strlen(mywhois);
			if ((atoi(myport) == aptr->listener->port)) {
				if (mywhois && len <= 60) {
					swhois = strdup(mywhois);
					return(1);
				}
				else
					return(0);
			}
		}
	}
}

/* Read passwords from memory and return 1 if there is match password. */
int ConfPassOku(aClient *sptr) {
	char *myport = NULL, *mypass = NULL;
	ConfigItem_except *e;
	char line[512];
	int ret = 0;
	for (e = portpass; e; e = (ConfigItem_except *) e->next) {
		strcpy(line,e->mask);
		myport = strtok(line, ":");
		if (myport) {
			mypass = strtok(NULL, ":");
			if ((atoi(myport) == sptr->listener->port)) {
				if (mypass) {
					if (sptr->passwd==NULL) {
						ret = 1;
					break;
					}

					if (strlen(sptr->passwd) > 32){
						ret = 2;
					break;
					}

					if (!strcmp(mypass,sptr->passwd)) {
						ret = 4;
					break;
					}
					else ret = 3;

				}
			}
		}
	}
return (ret);
}

/* On Rehash Hook? :) */
DLLFUNC int Call_ConfigRehash() {
	ListStruct	*next;
	ConfigItem_except	*e;
	for (e = portpass; e; e = (ConfigItem_except *) next) {
		next = (ListStruct *) e->next;
		DelListItem(e, portpass);
		MyFree(e->mask);
		MyFree(e);
	}
return 1;
}

/* On Conf Test Hook. */
DLLFUNC int Call_ConfigTest(ConfigFile *File, ConfigEntry *Entry, int Type, int *Error) {
	int ErrorTemp = 0;
	ConfigEntry	*EntryTemp;

	if (Type != CONFIG_MAIN)
		return 0;

	if (!Entry || !Entry->ce_varname || strcmp(Entry->ce_varname, "portpass"))
		return 0;

	for (EntryTemp = Entry->ce_entries; EntryTemp; EntryTemp = EntryTemp->ce_next)	{
		if (!EntryTemp->ce_varname) {
			config_error("%s:%i: portpass is empty.",
				EntryTemp->ce_fileptr->cf_filename, EntryTemp->ce_varlinenum);
			ErrorTemp++;
			continue;
		}
		else if (!strcmp(EntryTemp->ce_varname, "pass")) {
			if (strlen(EntryTemp->ce_vardata) > 80) {
				config_error("%s:%i: Line in portpass is too long!",
					EntryTemp->ce_fileptr->cf_filename, EntryTemp->ce_varlinenum);
			}
		} else {
			config_error("%s:%i: Unknown Directive portpass::%s",
				EntryTemp->ce_fileptr->cf_filename, EntryTemp->ce_varlinenum, EntryTemp->ce_varname);
			ErrorTemp++;
			continue;
		}
	}
	*Error = ErrorTemp;
return ErrorTemp ? -1 : 1;
}

/* On Conf Run Hook. */
DLLFUNC int Call_ConfigRun(ConfigFile *File, ConfigEntry *Entry, int Type) {
	ConfigEntry	*EntryTemp;

	if (Type != CONFIG_MAIN)
		return 0;

	if (!Entry || !Entry->ce_varname || strcmp(Entry->ce_varname, "portpass"))
		return 0;

	for (EntryTemp = Entry->ce_entries; EntryTemp; EntryTemp = EntryTemp->ce_next) {
		if (!strcmp(EntryTemp->ce_varname, "pass")) {
			ConfPassEkle(EntryTemp->ce_vardata);
		}
		if (!strcmp(EntryTemp->ce_varname, "swhois")) {
			ConfPassEkle(EntryTemp->ce_vardata);
		}
	}
return 1;
}
