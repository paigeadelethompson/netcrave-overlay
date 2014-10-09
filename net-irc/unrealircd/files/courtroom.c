/*
 * =================================================================
 * Filename:          courtroom.c
 * Description:       Courtroom channel mode & the suspend system
 * Author:            AngryWolf <angrywolf@flashmail.com>
 * Documentation:     courtroom.txt (comes with the package)
 * =================================================================
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

/* For backward compatibility */
#ifndef GetIP
 #define GetIP(acptr) Inet_ia2p(&(acptr)->ip)
#endif

typedef struct _chanlist ChanList;
typedef struct _suspend suspend;
typedef struct _qflag QFlag;

struct _chanlist
{
	ChanList		*prev, *next;
	aClient			*cptr;
	char			*channel;
};

struct _suspend
{
	suspend		*prev, *next;
	char			*usermask, *hostmask;
	char			*setby, *reason;
	TS			set_at;
};

struct _qflag
{
	QFlag			*prev, *next;
	suspend		*suspend;
	aClient			*cptr;
};

extern void			sendto_one(aClient *to, char *pattern, ...);
extern void			sendto_realops(char *pattern, ...);
extern void			sendto_serv_butone_token(aClient *one, char *prefix, char *command, char *token, char *pattern, ...);

#define FLAG_COURTROOM		'U'
#define UFLAG_suspend	'U'
#define ERR_suspended		494
#define MSG_suspend 		"suspend"
#define TOK_suspend		NULL
#define MSG_QFORWARD 		"QFORWARD"
#define TOK_QFORWARD		"QF"

#define UMODE_DENY		0
#define UMODE_ALLOW		1
#define Q_SENDUMODE		0x01
#define Q_DOPART		0x02

#define IsCourtRoom(x)		((x)->mode.extmode & MODE_COURTROOM)
#define Issuspendable(x)	(IsPerson(x) && !IsULine(x) && !IsAnOper(x))
#define SetCourtRoom(x)		(x)->mode.extmode |= MODE_COURTROOM
#define SkoAdmin(sptr)	(IsAdmin(sptr) || IsNetAdmin(sptr) || IsSAdmin(sptr) || IsCoAdmin(sptr))
#define HasUmodeQ(x)		((x)->umodes & UMODE_suspend)
#define SetUmodeQ(x)		(x)->umodes |= UMODE_suspend
#define ClearUmodeQ(x)		(x)->umodes &= ~UMODE_suspend
#define HasAnOperMode(x)	((x) & (UMODE_OPER | UMODE_LOCOP))
#define QReason(x)		((x)->reason ? (x)->reason : "No reason")
#define IsParam(x)		(parc > (x) && !BadPtr(parv[(x)]))
#define IsNotParam(x)		(parc <= (x) || BadPtr(parv[(x)]))
#define DelCommand(x)		if (x) CommandDel(x); x = NULL
#define DelCmode(x)		if (x) CmodeDel(x); x = NULL
#define DelHook(x)		if (x) HookDel(x); x = NULL
#define DelOverride(cmd, ovr)	if (ovr && CommandExists(cmd)) CmdoverrideDel(ovr); ovr = NULL
#define DelUmode(x)		if (x) UmodeDel(x); x = NULL

static int			m_suspend(aClient *cptr, aClient *sptr, int parc, char *parv[]);
static int			m_qforward(aClient *cptr, aClient *sptr, int parc, char *parv[]);

DLLFUNC int			MOD_UNLOAD(courtroom)(int module_unload);
static Command			*AddCommand(Module *module, char *msg, char *token, iFP func, unsigned char parameters, int flags);
static Cmode			*AddCmode(Module *module, CmodeInfo *req, Cmode_t *mode);
static Umode			*AddUmode(Module *module, char ch, iFP func, long *mode);
static Cmdoverride		*AddOverride(char *msg, iFP cb);
static int			uflag_suspend_allowed(aClient *sptr, int what);
static int			ModeCR_is_ok(aClient *, aChannel *, char *, int, int);
static int			ovr_part(Cmdoverride *, aClient *, aClient *, int, char *[]);
static int			cb_rehash();
static int			cb_rehash_complete();
static int			cb_part(aClient *, aClient *, aChannel *, char *);
static int			cb_pre_join(aClient *, aChannel *, char *[]);
static char			*cb_privmsg(aClient *, aClient *, aClient *, char *, int);
static char			*cb_chanmsg(aClient *, aClient *, aChannel *, char *, int);
static int			cb_connect(aClient *);
static int			cb_quit(aClient *, char *);
static int			cb_umode_change(aClient *, int, long);
static int			cb_oper(aClient *, int);
static int			cb_server_connect(aClient *);
static int			cb_config_test(ConfigFile *, ConfigEntry *, int, int *);
static int			cb_config_run(ConfigFile *, ConfigEntry *, int);
static int			cb_stats(aClient *sptr, char *stats);
static ConfigItem_except	*add_except_chan(char *mask);
static inline ChanList		*add_rejoin(aClient *sptr, char *channel);
static inline void		del_rejoin(ChanList *r);
static inline void		del_qflag(QFlag *q);
static void			del_suspend(suspend *q);

static char			buf[512], *q_ignores = NULL, *q_autojoin = NULL;
static unsigned			rejoin_ok = 0;
static ConfigItem_except	*conf_exceptchan = NULL;
static Command			*Cmdsuspend = NULL, *CmdQforward = NULL;
static Cmode_t			MODE_COURTROOM = 0L;
static Cmode			*ModeCourtRoom = NULL;
static Umode			*Umodesuspend = NULL;
static long			UMODE_suspend = 0;
static Cmdoverride		*OvrPart = NULL;
static Hook			*HookRehashDone = NULL, *HookServConn = NULL;
static Hook			*HookJoin = NULL, *HookPart = NULL;
static Hook			*HookPrivMsg = NULL, *HookChanMsg = NULL;
static Hook			*HookConnect = NULL, *HookQuit = NULL;
static Hook			*HookConfTest = NULL, *HookConfRun = NULL;
static Hook			*HookConfStats = NULL, *HookConfRehash = NULL;
static Hook			*HookUmodeChg = NULL, *HookOper = NULL;
static ChanList			*Rejoins = NULL;
static suspend		*QList = NULL;
static QFlag			*QFlagsList = NULL;
static u_short			module_loaded = 0;

#ifndef STATIC_LINKING
static ModuleInfo		*MyModInfo = NULL;
 #define MyMod			MyModInfo->handle
 #define SAVE_MODINFO		MyModInfo = modinfo;
#else
 #define MyMod			NULL
 #define SAVE_MODINFO
#endif

static char *chanjoin;

// =================================================================

ModuleHeader MOD_HEADER(courtroom)
  = {
	"courtroom",
	"$Id: courtroom.c,v 1.13 2004/09/01 15:31:52 angrywolf; Revised by chevyman2002 $",
	"Channel mode +U (courtrooms) and the suspend system",
	"3.2-b8-1",
	NULL 
    };

static void FreeConf()
{
        ConfigItem_except	*e;
	ListStruct		*next;

        for (e = conf_exceptchan; e; e = (ConfigItem_except *) next)
        {
                next = (ListStruct *) e->next;
                DelListItem(e, conf_exceptchan);
                MyFree(e->mask);
                MyFree(e);
        }

	DelOverride("part", OvrPart);

	if (q_ignores)
		MyFree(q_ignores);
	if (q_autojoin)
		MyFree(q_autojoin);
}

static void InitConf()
{
	q_ignores	= NULL;
	q_autojoin	= NULL;
}

DLLFUNC int MOD_TEST(courtroom)(ModuleInfo *modinfo)
{
	CmodeInfo ModeCR;

	memset(&ModeCR, 0, sizeof ModeCR);

	ModeCR.paracount	= 0;
	ModeCR.is_ok		= ModeCR_is_ok;
	ModeCR.flag		= FLAG_COURTROOM;

	HookConfTest		= HookAddEx(modinfo->handle,
					HOOKTYPE_CONFIGTEST, cb_config_test);
	ModeCourtRoom		= AddCmode(modinfo->handle, &ModeCR,
					&MODE_COURTROOM);
	Umodesuspend		= AddUmode(modinfo->handle, UFLAG_suspend,
					uflag_suspend_allowed,
					&UMODE_suspend);

	if (!ModeCourtRoom || !Umodesuspend)
	{
		MOD_UNLOAD(courtroom)(0);
		return MOD_FAILED;
	}

	return MOD_SUCCESS;
}

DLLFUNC int MOD_INIT(courtroom)(ModuleInfo *modinfo)
{
	SAVE_MODINFO
#ifndef STATIC_LINKING
	ModuleSetOptions(modinfo->handle, MOD_OPT_PERM);
#endif
	InitConf();

	rejoin_ok	= 0;
	QList		= NULL;
	QFlagsList	= NULL;
	Rejoins		= NULL;
        conf_exceptchan	= NULL;

	HookConfRun	= HookAddEx(modinfo->handle, HOOKTYPE_CONFIGRUN,
				cb_config_run);
	HookConfRehash	= HookAddEx(modinfo->handle, HOOKTYPE_REHASH,
				cb_rehash);
	HookRehashDone	= HookAddEx(modinfo->handle, HOOKTYPE_REHASH_COMPLETE,
				cb_rehash_complete);
	HookConfStats	= HookAddEx(modinfo->handle, HOOKTYPE_STATS, cb_stats);
	HookPrivMsg	= HookAddPCharEx(modinfo->handle, HOOKTYPE_USERMSG,
				cb_privmsg);
	HookChanMsg	= HookAddPCharEx(modinfo->handle, HOOKTYPE_CHANMSG,
				cb_chanmsg);
	HookJoin	= HookAddEx(modinfo->handle, HOOKTYPE_PRE_LOCAL_JOIN,
				cb_pre_join);
	HookPart	= HookAddEx(modinfo->handle, HOOKTYPE_LOCAL_PART,
				cb_part);
	HookConnect	= HookAddEx(modinfo->handle, HOOKTYPE_LOCAL_CONNECT,
				cb_connect);
	HookQuit	= HookAddEx(modinfo->handle, HOOKTYPE_LOCAL_QUIT,
				cb_quit);
	HookServConn	= HookAddEx(modinfo->handle, HOOKTYPE_SERVER_CONNECT,
				cb_server_connect);
	HookUmodeChg	= HookAddEx(modinfo->handle, HOOKTYPE_UMODE_CHANGE,
				cb_umode_change);
	HookOper	= HookAddEx(modinfo->handle, HOOKTYPE_LOCAL_OPER,
				cb_oper);
	Cmdsuspend	= AddCommand(modinfo->handle, MSG_suspend,
				TOK_suspend, m_suspend, 2, 0);
	CmdQforward	= AddCommand(modinfo->handle, MSG_QFORWARD,
				TOK_QFORWARD, m_qforward, MAXPARA, M_SERVER);

	if (!Cmdsuspend || !CmdQforward)
		return MOD_FAILED;

	return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(courtroom)(int module_load)
{
	cb_rehash_complete();
	return MOD_SUCCESS;
}

DLLFUNC int MOD_UNLOAD(courtroom)(int module_unload)
{
	ChanList		*r;
	suspend		*q;
	QFlag			*f;
	ListStruct		*next;

	DelCmode(ModeCourtRoom);
	DelUmode(Umodesuspend);
	DelCommand(CmdQforward);
	DelCommand(Cmdsuspend);
	DelHook(HookOper);
	DelHook(HookUmodeChg);
	DelHook(HookServConn);
	DelHook(HookQuit);
	DelHook(HookConnect);
	DelHook(HookPart);
	DelHook(HookJoin);
	DelHook(HookChanMsg);
	DelHook(HookPrivMsg);
	DelHook(HookConfStats);
	DelHook(HookRehashDone);
	DelHook(HookConfRehash);
	DelHook(HookConfRun);
	DelHook(HookConfTest);

	FreeConf();

	for (r = Rejoins; r; r = (ChanList *) next)
	{
		next = (ListStruct *) r->next;
		del_rejoin(r);
	}
	for (q = QList; q; q = (suspend *) next)
	{
		next = (ListStruct *) q->next;
		del_suspend(q);
	}
	for (f = QFlagsList; f; f = (QFlag *) next)
	{
		next = (ListStruct *) f->next;
		del_qflag(f);
	}

	return MOD_SUCCESS;
}

static int cb_rehash()
{
	FreeConf();
	InitConf();
	module_loaded = 0;

	return 0;
}

static int cb_rehash_complete()
{
	if (!module_loaded)
	{
		OvrPart = AddOverride("part", ovr_part);
		module_loaded = 1;
	}

	return 0;
}

static int cb_config_test(ConfigFile *cf, ConfigEntry *ce, int type, int *errs)
{
	int errors = 0;

	if (type != CONFIG_SET)
		return 0;

	if (!strcmp(ce->ce_varname, "suspend-ignore"))
	{
		if (!ce->ce_vardata)
		{
			config_error("%s:%i: set::%s without contents",
					ce->ce_fileptr->cf_filename,
					ce->ce_varlinenum, ce->ce_varname);
			errors++;
		}

		*errs = errors;
		return errors ? -1 : 1;
	}
	else if (!strcmp(ce->ce_varname, "suspend-auto-join"))
	{
		if (!ce->ce_vardata)
		{
			config_error("%s:%i: set::%s without contents",
					ce->ce_fileptr->cf_filename,
					ce->ce_varlinenum, ce->ce_varname);
			errors++;
		}
		else if (*ce->ce_vardata != '#' || strchr(ce->ce_vardata, ',')
			|| strchr(ce->ce_vardata, ':'))
		{
			config_error("%s:%i: set::%s: invalid channel name",
					ce->ce_fileptr->cf_filename,
					ce->ce_varlinenum, ce->ce_varname);
			errors++;
		}

		*errs = errors;
		return errors ? -1 : 1;
	}

	return 0;
}

static int cb_config_run(ConfigFile *cf, ConfigEntry *ce, int type)
{
	if (type != CONFIG_SET)
		return 0;

	if (!strcmp(ce->ce_varname, "suspend-ignore"))
	{
		char *tmp, *ch, *p;

		if (!q_ignores)
		{
			q_ignores = strdup(ce->ce_vardata);
			tmp = strdup(ce->ce_vardata);
			for (ch = strtoken(&p, tmp, ","); ch;
			    ch = strtoken(&p, NULL, ","))
				add_except_chan(ch);
			free(tmp);
		}

		return 1;		
	}
	if (!strcmp(ce->ce_varname, "suspend-auto-join"))
	{
		if (q_autojoin)
			MyFree(q_autojoin);
		q_autojoin = strdup(ce->ce_vardata);
		chanjoin = strdup(ce->ce_vardata);
		return 1;
	}

	return 0;
}

static int cb_stats(aClient *sptr, char *stats)
{
	if (*stats == 'S')
	{
		sendto_one(sptr, ":%s %i %s :suspend-ignore: %s",
			me.name, RPL_TEXT, sptr->name,
			q_ignores ? q_ignores : "<none>");
		sendto_one(sptr, ":%s %i %s :suspend-auto-join: %s",
			me.name, RPL_TEXT, sptr->name,
			q_autojoin ? q_autojoin : "<none>");
	}

        return 0;
}

// =================================================================

static Command *AddCommand(Module *module, char *msg, char *token, iFP func,
                           unsigned char parameters, int flags)
{
	Command *cmd;

	if (CommandExists(msg))
    	{
		config_error("Command %s already exists", msg);
		return NULL;
    	}
    	if (token && CommandExists(token))
	{
		config_error("Token %s already exists", token);
		return NULL;
    	}

	cmd = CommandAdd(module, msg, token, func, parameters, flags);

#ifndef STATIC_LINKING
	if (ModuleGetError(module) != MODERR_NOERROR || !cmd)
#else
	if (!cmd)
#endif
	{
#ifndef STATIC_LINKING
		config_error("Error adding command %s: %s", msg,
			ModuleGetErrorStr(module));
#else
		config_error("Error adding command %s", msg);
#endif
		return NULL;
	}

	return cmd;
}

static Cmode *AddCmode(Module *module, CmodeInfo *req, Cmode_t *mode)
{
	Cmode *cmode;

	*mode = 0;
	cmode = CmodeAdd(module, *req, mode);

#ifndef STATIC_LINKING
        if (ModuleGetError(module) != MODERR_NOERROR || !cmode)
#else
        if (!cmode)
#endif
	{
#ifndef STATIC_LINKING
		config_error("Error adding channel mode +%c when loading module %s: %s",
			req->flag, MOD_HEADER(courtroom).name, ModuleGetErrorStr(module));
#else
		config_error("Error adding channel mode +%c when loading module %s",
			req->flag, MOD_HEADER(courtroom).name);
#endif
		return NULL;
	}

	return cmode;
}

static Umode *AddUmode(Module *module, char ch, iFP func, long *mode)
{
	Umode *umode;

	*mode = 0;
	umode = UmodeAdd(module, ch, UMODE_GLOBAL, func, mode);

#ifndef STATIC_LINKING
        if (ModuleGetError(module) != MODERR_NOERROR || !umode)
#else
        if (!umode)
#endif
	{
#ifndef STATIC_LINKING
		config_error("Error adding user mode +%c when loading module %s: %s",
			ch, MOD_HEADER(noctcp).name, ModuleGetErrorStr(module));
#else
		config_error("Error adding user mode +%c when loading module %s",
			ch, MOD_HEADER(noctcp).name);
#endif
		return NULL;
	}

	return umode;
}

static Cmdoverride *AddOverride(char *msg, iFP cb)
{
	Cmdoverride *ovr = CmdoverrideAdd(MyMod, msg, cb);

#ifndef STATIC_LINKING
        if (ModuleGetError(MyMod) != MODERR_NOERROR || !ovr)
#else
        if (!ovr)
#endif
	{
#ifndef STATIC_LINKING
		config_error("Error replacing command %s when loading module %s: %s",
			msg, MOD_HEADER(operpasswd).name, ModuleGetErrorStr(MyMod));
#else
		config_error("Error replacing command %s when loading module %s",
			msg, MOD_HEADER(operpasswd).name);
#endif
		return NULL;
	}

	return ovr;
}

// =================================================================

static suspend *add_suspend(char *usermask, char *hostmask, char *setby,
                                  char *reason, TS set_at)
{
	suspend *q;

	q = (suspend *) MyMallocEx(sizeof(suspend));
	q->usermask = strdup(usermask);
	q->hostmask = strdup(hostmask);
	q->setby = strdup(setby);
	q->reason = reason ? strdup(reason) : NULL;
	q->set_at = set_at;
	AddListItem(q, QList);
	return q;
}

static void del_suspend(suspend *q)
{
	DelListItem(q, QList);
	MyFree(q->usermask);
	MyFree(q->hostmask);
	MyFree(q->setby);
	if (q->reason)
		MyFree(q->reason);
	MyFree(q);
}

static suspend *find_suspend(char *usermask, char *hostmask)
{
	suspend *q;

	for (q = QList; q; q = q->next)
		if (!strcmp(q->usermask, usermask) &&
		    !strcmp(q->hostmask, hostmask))
			break;

	return q;
}

static inline unsigned suspend_match(suspend *q, aClient *cptr)
{
	return ((!_match(q->hostmask, cptr->user->realhost) ||
		!_match(q->hostmask, GetIP(cptr))) &&
		!_match(q->usermask, cptr->user->username)) ? 1 : 0;
}

static suspend *find_suspend2(aClient *cptr)
{
	suspend *q;

	for (q = QList; q; q = q->next)
		if (suspend_match(q, cptr))
			break;

	return q;
}

static QFlag *find_qflag(aClient *cptr)
{
	QFlag *q;

	for (q = QFlagsList; q; q = q->next)
		if (q->cptr == cptr)
			break;

	return q;
}

static inline void del_qflag(QFlag *q)
{
	DelListItem(q, QFlagsList);
	MyFree(q);
}

static void set_qflag(aClient *cptr, suspend *q)
{
	QFlag *qf;

	if (find_qflag(cptr))
		return;
	if (!q && !(q = find_suspend2(cptr)))
	{
		sendto_realops("*** [BUG] (set_qflag) Couldn't find a "
			"suspend line for %s (%s@%s)",
			cptr->name, cptr->user->username,
			cptr->user->realhost);
		return;
	}

	qf = (QFlag *) MyMalloc(sizeof(QFlag));
	qf->cptr = cptr;
	qf->suspend = q;
	AddListItem(qf, QFlagsList);
}

static void clear_qflag(aClient *cptr)
{
	QFlag *q;

	if ((q = find_qflag(cptr)))
		del_qflag(q);
}

static inline ChanList *add_rejoin(aClient *sptr, char *channel)
{
	ChanList *r;

	r = (ChanList *) MyMallocEx(sizeof(ChanList));
	r->cptr = sptr;
	r->channel = strdup(channel);
	AddListItem(r, Rejoins);

	return r;
}

static inline void del_rejoin(ChanList *r)
{
	DelListItem(r, Rejoins);
	MyFree(r->channel);
	MyFree(r);
}

static ConfigItem_except *add_except_chan(char *mask)
{
        ConfigItem_except *e;

        e = (ConfigItem_except *) MyMallocEx(sizeof(ConfigItem_except));
        e->mask = strdup(mask);

        AddListItem(e, conf_exceptchan);
	return e;
}

static ConfigItem_except *find_except_chan(char *channel)
{
        ConfigItem_except *e;

	for (e = conf_exceptchan; e; e = (ConfigItem_except *) e->next)
		if (!match(e->mask, channel))
			break;

	return e;
}

// =================================================================

static int uflag_suspend_allowed(aClient *sptr, int what)
{
	if (!MyConnect(sptr))
	{
		if (what == MODE_ADD)
			set_qflag(sptr, NULL);
		else
			clear_qflag(sptr);

		return UMODE_ALLOW;
	}

	return UMODE_DENY;
}

static int ModeCR_is_ok(aClient *sptr, aChannel *chptr, char *para, int type,
                        int what)
{
	if ((type == EXCHK_ACCESS) || (type == EXCHK_ACCESS_ERR))
	{
		if (IsPerson(sptr) && !IsULine(sptr))
		{
			if (!SkoAdmin(sptr))
			{
				if (type == EXCHK_ACCESS_ERR)
					sendnotice(sptr,
						"*** Courtroom mode (+%c) can "
						"only be set by IRCOps.",
						FLAG_COURTROOM);
				return EX_ALWAYS_DENY;
			}
		}

		return EX_ALLOW;
	}

	return 0;
}

static void courtroom_force_join(aClient *cptr, char *channel, char *reason)
{
	int		flags;
	aChannel	*chptr;

	flags = (ChannelExists(channel)) ? CHFL_DEOPPED : CHFL_CHANOP;
	chptr = get_channel(cptr, channel, CREATE);

	if (chptr && find_membership_link(cptr->user->channel, chptr))
		return;

	if (reason)
		sendnotice(cptr, "*** You were forced to join %s (%s)",
			chptr->chname, reason);
	else
		sendnotice(cptr, "*** You were forced to join %s",
			chptr->chname);

	join_channel(chptr, cptr, cptr, flags);

	if (!IsCourtRoom(chptr))
	{
		SetCourtRoom(chptr);
		sendto_serv_butone(&me, ":%s MODE %s +%c 0",
			me.name, chptr->chname, FLAG_COURTROOM);
		sendto_channel_butserv(chptr, &me,
			":%s MODE %s +%c", me.name, chptr->chname,
			FLAG_COURTROOM);
	}
}

static void do_rejoins()
{
	ChanList	*r;
	ListStruct	*next;

	for (r = Rejoins; r; r = (ChanList *) next)
	{
		next = (ListStruct *) r->next;
		courtroom_force_join(r->cptr, r->channel,
			"You are not allowed to leave that channel");
		del_rejoin(r);
	}
}

static void set_suspend(aClient *cptr, suspend *q, u_char opts)
{
	char		*parv[3] = { cptr->name, NULL, buf };
	Membership	*lp, *next;

	/* set mode & qflag */
	set_qflag(cptr, q);
	if (opts & Q_SENDUMODE)
	{
		long old_umodes;

		if (!HasUmodeQ(cptr))
		{
			old_umodes = cptr->umodes;
			SetUmodeQ(cptr);
			send_umode_out(cptr, cptr, old_umodes);
		}
	}
	else
		SetUmodeQ(cptr);

	/* notification */
	sendnotice(cptr, "*** You have been suspended (%s)", QReason(q));

	/* part channels */
	if (opts & Q_DOPART)
	{
		snprintf(buf, 512, "User has been suspended (%s)", QReason(q));
		for (lp = cptr->user->channel; lp; lp = next)
		{
			next = lp->next;
			if (!IsCourtRoom(lp->chptr) &&
			    !find_except_chan(lp->chptr->chname))
			{
				parv[1] = lp->chptr->chname;
				//(void) m_part(cptr, cptr, 3, parv);
				do_cmd(cptr, cptr, "PART", 3, parv);
			}
		}
	}

	/* auto join */
	if (q_autojoin)
		courtroom_force_join(cptr, q_autojoin, NULL);
}

static void do_suspend(suspend *q)
{
	aClient		*acptr;
	int		i;

	for (i = 0; i <= LastSlot; i++)
	{
		if (!(acptr = local[i]) || !Issuspendable(acptr))
			continue;
		if (suspend_match(q, acptr))
			set_suspend(acptr, q, (Q_SENDUMODE | Q_DOPART));
	}
}

static void remove_suspend_flags(suspend *q)
{
	int		i;
	aClient		*acptr;
	long		old_umodes;

	for (i = 0; i <= LastSlot; i++)
	{
		if (!(acptr = local[i]) || !Issuspendable(acptr))
			continue;
		if (suspend_match(q, acptr))
		{
			if (HasUmodeQ(acptr))
			{
				old_umodes = acptr->umodes;
				ClearUmodeQ(acptr);
				send_umode_out(acptr, acptr, old_umodes);
			}
			clear_qflag(acptr);
			sendnotice(acptr, "*** You are no longer suspended");
		}
	}
}

// =================================================================

static int ovr_part(Cmdoverride *ovr, aClient *cptr, aClient *sptr, int parc,
                    char *parv[])
{
	int ret;

	rejoin_ok = (IsPerson(sptr) && !IsULine(sptr) && !IsAnOper(sptr) && HasUmodeQ(sptr));
	ret = CallCmdoverride(ovr, cptr, sptr, parc, parv);

	if (rejoin_ok)
		do_rejoins();

	rejoin_ok = 0;
	return ret;
}

static int cb_part(aClient *cptr, aClient *sptr, aChannel *chptr, char *comment)
{
	if (rejoin_ok && IsCourtRoom(chptr))
		add_rejoin(sptr, chptr->chname);

	return 0;
}

static int cb_pre_join(aClient *sptr, aChannel *chptr, char *parv[])
{
	if (Issuspendable(sptr) && !IsCourtRoom(chptr) && find_qflag(sptr)
	    && !find_except_chan(chptr->chname))
	{
		sendto_one(sptr,
			":%s %d %s %s :Cannot join channel (you have been suspended by an IRCOp)",
			me.name, ERR_suspended, sptr->name, chptr->chname);
		return HOOK_DENY;
	}

	return HOOK_CONTINUE;
}

static char *cb_privmsg(aClient *cptr, aClient *from, aClient *to, char *str,
                        int notice)
{
	if (!Issuspendable(from) || !Issuspendable(to))
		return str;

	if (find_qflag(from))
	{
		sendnotice(from, "*** Couldn't send message to %s "
			"(you have been suspended by an IRCOp)", to->name);
		return NULL;
	}
	if (find_qflag(to) && !notice)
		sendnotice(from, "*** Note: %s can't send messages to you "
			"(user has been suspended by an IRCOp)", to->name);

	return str;
}

static char *cb_chanmsg(aClient *cptr, aClient *from, aChannel *to, char *str,
                        int notice)
{
	if (Issuspendable(from) && !IsCourtRoom(to) && find_qflag(from) &&
	    !find_except_chan(to->chname))
	{
		sendnotice(from, "*** Couldn't send message to %s "
			"(you have been suspended by an IRCOp)", to->chname);
		return NULL;
	}

	return str;
}

static int cb_connect(aClient *cptr)
{
	suspend *q;

	for (q = QList; q; q = q->next)
		if (suspend_match(q, cptr) && Issuspendable(cptr))
		{
			set_suspend(cptr, q, 0);
			sendto_snomask(SNO_TKL, "*** Suspended user %s (%s@%s) just connected to the network!", cptr->name, q->usermask, q->hostmask);
			break;
		}

	return 0;
}

static int cb_umode_change(aClient *cptr, int oldmodes, long newmodes)
{
	if (!MyClient(cptr))
		return 0;

	if (HasAnOperMode(oldmodes) && !HasAnOperMode(newmodes))
	{
		suspend *q;

		for (q = QList; q; q = q->next)
			if (suspend_match(q, cptr) && Issuspendable(cptr))
			{
				set_suspend(cptr, q, Q_DOPART);
				break;
			}
	}
	else if (!HasAnOperMode(oldmodes) && HasAnOperMode(newmodes))
	{
		QFlag *q;

		newmodes &= ~UMODE_suspend;
		if ((q = find_qflag(cptr)))
		{
			del_qflag(q);
			sendnotice(cptr, "*** You are no longer suspended");
		}
	}

	return 0;
}

static int cb_oper(aClient *cptr, int what)
{
	QFlag		*q;
	long		old_umodes;

	if (what == 1)
	{
		if (HasUmodeQ(cptr))
		{
			old_umodes = cptr->umodes;
			ClearUmodeQ(cptr);
			send_umode_out(cptr, cptr, old_umodes);
		}
		if ((q = find_qflag(cptr)))
		{
			del_qflag(q);
			sendnotice(cptr, "*** You are no longer suspended");
		}
	}

	return 0;
}

static int cb_quit(aClient *sptr, char *comment)
{
	clear_qflag(sptr);
	return 0;
}

static int cb_server_connect(aClient *cptr)
{
	suspend *q;

	for (q = QList; q; q = q->next)
		sendto_one(cptr,
			":%s %s ! %s %s %s %ld :%s",
			me.name, IsToken(cptr) ? TOK_QFORWARD : MSG_QFORWARD,
			q->setby, q->usermask, q->hostmask, (long) q->set_at,
			BadPtr(q->reason) ? "" : q->reason);

	return 0;
}

// =================================================================

/*
** m_suspend
**
**     parv[0]: sender prefix
**     parv[1]: user@host
**     parv[2]: reason (optional)
*/

static CMD_FUNC(m_suspend)
{
	suspend	*q;
	char		*p, *mask, *usermask, *hostmask, *reason;
	unsigned	add = 1;
	int		count=0;

	if (!IsPerson(sptr))
	{
		sendto_one(sptr, err_str(ERR_NOPRIVILEGES),
			me.name, sptr->name);
		return 0;
	}

	if (!IsNotParam(1)) {
		if (!stricmp(parv[1], "?help")) {
			sendnotice(sptr, "Syntax: \2SUSPEND\2 [+|-]<mask|nick> <reason>");
			return 0;
		}
	}
    	if (IsNotParam(1))
	{
		if (!IsAnOper(sptr))
		{
			sendto_one(sptr, err_str(ERR_NOPRIVILEGES),
				me.name, sptr->name);
			return 0;
		}

		for (q = QList; q; q = q->next)
		{
			strlcpy(buf, asctime(gmtime(&q->set_at)), sizeof buf);
			iCstrip(buf);
			count++;
			sendnotice(sptr,
				"%d) \2%s@%s\2 set by \2%s\2 on %s "
				"(reason: %s)", count,
				q->usermask, q->hostmask, q->setby, buf,
				QReason(q));
		}
		if(count)
			sendnotice(sptr, "End of suspension list [%d match%s displayed]", count, count > 1 ? "es" : "");
		else
			sendnotice(sptr, "There are currently no suspended users.");
		return 0;
	}

	if (!SkoAdmin(sptr))
	{
		sendto_one(sptr, err_str(ERR_NOPRIVILEGES),
			me.name, sptr->name);
		return 0;
	}

	mask	= parv[1];
	reason	= IsParam(2) ? parv[2] : NULL;

	if (*mask == '+')
		mask++;
	else if (*mask == '-')
	{
		add = 0;
		mask++;
	}

	if (strchr(mask, '!'))
	{
		sendnotice(sptr, "[error] Cannot have ! in masks.");
		return 0;
	}
	if (strchr(mask, ' '))
	{
		sendnotice(sptr, "[error] Cannot have spaces in masks.");
		return 0;
	}

	if (!(p = strchr(mask, '@')))
	{
		aClient *acptr;

		if (!(acptr = find_person(mask, NULL)))
		{
			sendto_one(sptr, rpl_str(ERR_NOSUCHNICK),
				me.name, sptr->name, mask);
			return 0;
		}
		if (acptr == sptr)
		{
			sendnotice(sptr,
				"*** You are not allowed to suspend "
				"yourself");
			return 0;
		}
		if (add && IsAnOper(acptr))
		{
			char buf[1024];
			sendnotice(sptr,
				"*** IRC operators are not allowed to be "
				"suspended");
			ircsprintf(buf, "*** %s attempted to suspend %s "
				"(reason: %s)",
				sptr->name, acptr->name,
				reason ? reason : "No reason");
			sendto_snomask(SNO_EYES, "%s", buf);
			sendto_serv_butone_token(NULL, me.name, MSG_SENDSNO,
				TOK_SENDSNO, "e :%s", buf);
			return 0;
		}

		usermask = "*";
		hostmask = acptr->user->realhost;
	}
	else
	{
		*p = 0;
		usermask = mask;
		hostmask = p + 1;

		if (!*usermask || !*hostmask)
		{
			sendnotice(sptr, "[error] %s@%s: Illegal mask",
				usermask, hostmask);
			return 0;
		}

		if (add)
		{
			unsigned i = 0;

			for (p = hostmask; *p; p++)
				if (*p != '*' && *p != '.' && *p != '?')
					i++;

			if (i < 4)
			{
				sendnotice(sptr,
					"[error] %s@%s: Too broad mask",
					usermask, hostmask);
				return 0;
			}
		}
	}

	q = find_suspend(usermask, hostmask);

	if (add)
	{
		if (q)
		{
			sendnotice(sptr, "[error] %s@%s: Mask already added",
				usermask, hostmask);
			return 0;
		}

		q = add_suspend(usermask, hostmask, sptr->name,
			reason, TStime());
	}
	else
	{
		if (!q)
		{
			sendnotice(sptr, "[error] %s@%s: Mask not found",
				usermask, hostmask);
			return 0;
		}
	}

	strlcpy(buf, asctime(gmtime(&q->set_at)), sizeof buf);
	iCstrip(buf);

	if (add)
	{
		sendto_snomask(SNO_TKL,
			"*** %s suspended the mask \2%s@%s\2 at %s "
			"(reason: %s)", q->setby,
			q->usermask, q->hostmask, buf, QReason(q));
		sendto_serv_butone_token(cptr, me.name,
			MSG_QFORWARD, TOK_QFORWARD,
			"+ %s %s %s %ld :%s",
			q->setby, q->usermask, q->hostmask, (long) q->set_at,
			BadPtr(q->reason) ? "" : q->reason);

		do_suspend(q);
	}
	else
	{
/***********Remove User Mode U and delete from list************/
		QFlag	*qf;
		char *mask;

		mask = parv[1];

		for (qf = QFlagsList; qf; qf = qf->next)
		{
			if (mask && _match(mask, qf->cptr->name))
				continue;
			remove_suspend_flags(qf->cptr);
			del_qflag(qf->cptr);
		}
/**************************************************************/
		sendto_snomask(SNO_TKL,
			"*** %s removed suspension for the mask \2%s@%s\2 "
			"(set by %s at %s - reason: %s)",
			sptr->name, q->usermask, q->hostmask, q->setby, buf,
			QReason(q));
		sendto_serv_butone_token(cptr, me.name,
			MSG_QFORWARD, TOK_QFORWARD,
			"- %s %s %s",
			sptr->name, q->usermask, q->hostmask);
		remove_suspend_flags(q);
		del_suspend(q);
	}

	return 0;
}

/*
** m_qforward
**
**     parv[0]: sender prefix
**     parv[1]: + (add), - (delete), ! (sync)
**     parv[2]: set/removed by
**     parv[3]: usermask
**     parv[4]: hostmask
**     parv[5]: set at
**     parv[6]: reason
*/

static CMD_FUNC(m_qforward)
{
	suspend	*q;
	char		*opt, *setby, *usermask, *hostmask, *setat, *reason;

	opt		= parv[1];
	setby		= parv[2];
	usermask	= parv[3];
	hostmask	= parv[4];

	q = find_suspend(usermask, hostmask);

	if (*opt == '+' || *opt == '!')
	{
		setat	= parv[5];
		reason	= *parv[6] ? parv[6] : NULL;

		if (q)
			return 0;

		q = add_suspend(usermask, hostmask, setby,
			reason, (TS) atol(setat));

		if (*opt == '+')
		{
			strlcpy(buf, asctime(gmtime(&q->set_at)), sizeof buf);
			iCstrip(buf);
			sendto_snomask(SNO_TKL,
			        "*** %s suspended the mask \2%s@%s\2 at %s "
			"(reason: %s)", q->setby, 
				q->usermask, q->hostmask, q->setby, buf,
				QReason(q));
		}
		sendto_serv_butone_token(cptr, me.name,
			MSG_QFORWARD, TOK_QFORWARD,
			"%c %s %s %s %ld :%s",
			*opt, q->setby, q->usermask, q->hostmask,
			(long) q->set_at, BadPtr(q->reason) ? "" : q->reason);

		do_suspend(q);
	}
	else if (*opt == '?') {
		sendnotice(setby, "Syntax: \2/suspend [+|-]<mask|nick> <reason>\2");
		return 0;
	}
	else
	{
		if (!q)
			return 0;

		strlcpy(buf, asctime(gmtime(&q->set_at)), sizeof buf);
		iCstrip(buf);
		sendto_snomask(SNO_TKL,
			"*** \2%s\2 removed suspension for the mask \2%s@%s\2 "
			"(set by %s at %s - reason: %s)",
			setby, q->usermask, q->hostmask, q->setby, buf,
			QReason(q));
		sendto_serv_butone_token(cptr, me.name,
			MSG_QFORWARD, TOK_QFORWARD,
			"- %s %s %s",
			setby, q->usermask, q->hostmask);

		del_suspend(q);
		remove_suspend_flags(q);
	}

	return 0;
}
