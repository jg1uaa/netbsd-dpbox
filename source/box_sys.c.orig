/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#define BOX_SYS_G
#include <ctype.h>
#include <math.h>
#include "box_sys.h"
#include "tools.h"
#include "sort.h"
#include "yapp.h"
#include "boxlocal.h"
#include "box_logs.h"
#include "box.h"
#include "box_file.h"
#include "box_sf.h"
#include "box_sub.h"
#include "box_send.h"
#include "box_mem.h"
#include "box_rout.h"
#include "box_inou.h"
#include "box_tim.h"
#include "status.h"
#include "shell.h"
#include "box_wp.h"

/* wird in PACKETL1 gebraucht, procedure pr_wprg3 und pr_wprg2           */
/* der tempbinname wird hier als Zwischenspeicher fuer den #BIN#-Header  */
/* verwendet...                                                          */

void box_rtitle(boolean set_it, short unr, char *rtitle)
{
  char	hs[256];

  if (!boxrange(unr))
    return;
  if (!set_it) {
    strcpy(rtitle, user[unr]->tempbinname);
    return;
  }
  if (strlen(rtitle) <= 80) {
    strcpy(user[unr]->tempbinname, rtitle);
    return;
  }
  strcpy(hs, rtitle);
  del_path(hs);
  cut(hs, 80);
  strcpy(user[unr]->tempbinname, hs);
}

/* reset timeout counter (during binary transfer, e.g.) */
void box_ttouch(short unr)
{
  if (boxrange(unr)) {
    user[unr]->lastatime	= clock_.ixtime;
    user[unr]->lastcmdtime	= clock_.ixtime;
  }
}

/* make some assumptions on the default user language */
boolean is_german(char *w)
{
  return (   (w[0] == 'D' && w[1] != 'U')
	  || (w[0] == 'O' && w[1] == 'E')
	  || (w[0] == 'H' && w[1] == 'B')  );
}

boolean for_german_readers(char *call1, char *call2, char *mbx)
{
  mbxtype hs;
  
  unhpath(mbx, hs);
  return ((!*call1 || is_german(call1))
       && (!*call2 || is_german(call2))
       && is_german(Console_call)
       && strcmp(mbx, "EU")
       && strcmp(mbx, "WW")
       && strcmp(mbx, "AMSAT")
       && strcmp(mbx, "ARL")
       && strcmp(mbx, "ARRL")
       && strcmp(mbx, "ALL")
       && (!callsign(hs) || is_german(hs)));
}

/* use german or english text for indication of an unauthenticated upload ? */
void sig_german_authinfo(char *call, char *mbx, char *ret)
{
  if (for_german_readers(call, call, mbx))
    strcpy(ret, TXTUNAUTHUPGER);
  else
    strcpy(ret, TXTUNAUTHUPENG);
}



void update_digimap(short unr, char *updatefile)
{
  pathstr hs, base;

  if (insecure(updatefile)) {
    wln_btext(unr, 100);
    return;
  }
  snprintf(hs, 255, "%s%s", boxdir, updatefile);
  if (!exist(hs)) {
    wln_btext(unr, 4);
    return;
  }
  snprintf(base, 255, "%sdigimap%cstn", boxsysdir, extsep);
  if (!exist(base)) {
    wlnuser(unr, "system/digimap.stn:");
    wln_btext(unr, 4);
    return;
  }
  wlnuser(unr, "OK");
  if (merge_digimap_data(base, hs) < 0) wln_btext(unr, 149);
  else wlnuser(unr, "done");
}

void calc_lonstr(double lon, char *str)
{
  sprintf(str, "%3.4f", fabs(lon));
  if (lon >= 0.0) strcat(str, "E"); else strcat(str, "W");
}

void calc_latstr(double lat, char *str)
{
  sprintf(str, "%2.4f", fabs(lat));
  if (lat >= 0.0) strcat(str, "N"); else strcat(str, "S");
}

void show_prefix_information(short unr, char *call_, char *loc)
{
  double	lon, lat, l1, b1;
  double	entfernung, richtung, gegenrichtung;
  long		dist;
  boolean	userloc;
  short		waz, itu;
  char		subst[256], name[256], continent[256], hs[256], call[256];
  char		mbsysop[80], allsysops[80], qthloc[80], qthname[80];
  char		mbsystem[80], digisystem[80], remarks[256], updatetime[20];

  if (*call_ == '\0') {
    wlnuser(unr, "Usage: PREFIX <CALL|PREFIX> [<own WW-LOC>]");
    return;
  }

  strcpyupper(call, call_);
  
  if (!prefix(call, subst, name, &lon, &lat, &waz, &itu, continent)) {
    sprintf(hs, "%sprefix%cinf", boxsysdir, extsep);
    if (!exist(hs))
      wlnuser(unr, "prefix database not found");
    else
      wln_btext(unr, 45);
    return;
    
  } else {

    wuser(unr, "Call/Prefix : ");
    wlnuser(unr, call);
    wuser(unr, "Prefix Area : ");
    wlnuser(unr, name);
    if (*continent != '\0') {
      wuser(unr, "Continent   : ");
      wlnuser(unr, continent);
    }
    if (waz > 0) {
      wuser(unr, "WAZ         : ");
      swuser(unr, waz);
      wlnuser0(unr);
    }
    if (itu > 0) {
      wuser(unr, "ITU         : ");
      swuser(unr, itu);
      wlnuser0(unr);
    }
    
    get_digimap_data(false, call, allsysops, mbsysop, qthloc,
			qthname, mbsystem, digisystem, remarks,
			updatetime);

    l1		= mylaenge;
    b1		= mybreite;
    
    if (calc_qth(loc, &l1, &b1)) userloc = true;
    else userloc = false;
    
    if (userloc || myqthwwlocvalid) {
    
      calc_qth(qthloc, &lon, &lat);
      loc_dist(l1, b1, lon, lat, &entfernung, &richtung, &gegenrichtung);
      if (userloc) {
        sprintf(hs, "Distance    : %.0f km", entfernung);
        wlnuser(unr, hs);
        sprintf(hs, "Direction>  : %.0f deg", richtung);
        wlnuser(unr, hs); 
        sprintf(hs, "Direction<  : %.0f deg", gegenrichtung);
        wlnuser(unr, hs); 
      } else {
        dist	= entfernung;
        if (dist > 500) {
          dist	= (dist / 100) * 100; /* Genauigkeit heruntersetzen */
	  sprintf(hs, "Distance    : %ld km (estimated)", dist);
	  wlnuser(unr, hs);
        }
      }
    }
        
    if (*qthname != '\0') {
      wuser(unr, "QTH         : ");
      wlnuser(unr, qthname);
    }
  
    if (*qthloc != '\0') {
      wuser(unr, "QTHLOC      : ");
      wlnuser(unr, qthloc);
      wuser(unr, "Latitude    : ");
      calc_latstr(lat, hs);
      wlnuser(unr, hs);
      wuser(unr, "Longitude   : ");
      calc_lonstr(lon, hs);
      wlnuser(unr, hs);
    }
  
    if (*allsysops != '\0') {
      wuser(unr, "Sysop       : ");
      wlnuser(unr, allsysops);
    }
  
    if (*digisystem != '\0') {
      wuser(unr, "Node-Soft   : ");
      wlnuser(unr, digisystem);
    }
  
    if (*mbsystem != '\0') {
      wuser(unr, "BBS-Soft    : ");
      wlnuser(unr, mbsystem);
    }
  
    if (*remarks != '\0') {
      wuser(unr, "Remarks     : ");
      wlnuser(unr, remarks);
    }
    
    if (*updatetime != '\0') {
      wuser(unr, "Last Update : ");
      wlnuser(unr, updatetime);
    }
    
  }
  
}

void calc_qthdist(short unr, char *loc1, char *loc2)
{
  char		hs[256];
  double	l2, b2, l1, b1;
  double	entfernung, richtung, gegenrichtung;

  if (*loc1 == '\0') {
    wlnuser(unr, "Usage: QTHLOC <WW-LOC1> [<WW-LOC2>]");
    return;
  }
  
  upper(loc1);
  upper(loc2);

  if (!calc_qth(loc1, &l1, &b1)) {
    sprintf(hs, "LOC1 (%s) invalid", loc1);
    wlnuser(unr, hs);
    return;
  }
  
  if (*loc2 == '\0') {
    sprintf(hs, "%-12s: ", loc1);
    wuser(unr, hs);
    calc_latstr(b1, hs);
    wuser(unr, hs);
    chwuser(unr, ' ');
    calc_lonstr(l1, hs);
    wlnuser(unr, hs);
    return;
  }
  
  if (!calc_qth(loc2, &l2, &b2)) {
    sprintf(hs, "LOC2 (%s) invalid", loc2);
    wlnuser(unr, hs);
    return;
  }
  
  loc_dist(l1, b1, l2, b2, &entfernung, &richtung, &gegenrichtung);
  
  sprintf(hs, "%-12s: ", loc1);
  wuser(unr, hs);
  calc_latstr(b1, hs);
  wuser(unr, hs);
  chwuser(unr, ' ');
  calc_lonstr(l1, hs);
  wlnuser(unr, hs);
  sprintf(hs, "%-12s: ", loc2);
  wuser(unr, hs);
  calc_latstr(b2, hs);
  wuser(unr, hs);
  chwuser(unr, ' ');
  calc_lonstr(l2, hs);
  wlnuser(unr, hs);
  sprintf(hs, "Distance    : %.0f km", entfernung);
  wlnuser(unr, hs);
  sprintf(hs, "Direction 1 : %.1f deg", richtung);
  wlnuser(unr, hs); 
  sprintf(hs, "Direction 2 : %.1f deg", gegenrichtung);
  wlnuser(unr, hs); 
  
}


void check_sysanswer(short unr, char *eingabe_)
{
  short		l;
  userstruct	*WITH;
  char		eingabe[256], hs[256];

  if (!boxrange(unr))
    return;

  strcpy(eingabe, eingabe_);
  WITH	= user[unr];
  l	= strlen(WITH->input2);
  if (l == 32)
    lower(eingabe);
  if ((l == 4 && !strcmp(WITH->input2, eingabe)) ||		/* PRIV		*/
      (l == 5 && strstr(eingabe, WITH->input2) != NULL) ||	/* SYS		*/
      (l == 32 && !strcmp(WITH->input2, eingabe)))		/* MD2, MD5	*/
  {
    if (WITH->plevel > WITH->level)	/* if we don´t check that, a console user	*/
      WITH->level	= WITH->plevel;	/* loses some privileges			*/

    WITH->is_authentic	= (l == 4 || WITH->pwsetat + holddelay < clock_.ixtime);

    switch (WITH->pwmode) {

    case PWM_MUST_PRIV:
    case PWM_SEND_ERASE:
      WITH->se_ok	= true;
      break;

    case PWM_RSYSOPPRIV:
    case PWM_RSYSOP:
      WITH->se_ok	= true;
      WITH->rsysop	= true;
      strcpy(WITH->lastcmd, "accepted as board-sysop");
      append_syslog(unr);
      *WITH->lastcmd	= '\0';
      break;

    case PWM_SYSOPPRIV:
    case PWM_SYSOP:
      WITH->se_ok	= true;
      WITH->supervisor	= true;
      strcpy(WITH->lastcmd, "accepted as sysop");
      append_syslog(unr);
      *WITH->lastcmd	= '\0';
      break;
    }
  } else if (l == 4)
    append_syslog(unr);

  WITH->action		= 0;
  *WITH->input2		= '\0';
  
  if (WITH->se_ok && WITH->force_priv > 0) {
    WITH->force_priv	= 0;

    if (WITH->supervisor) {
      show_hold(unr, MAXDISPLOGINHOLD);
    }

    if (*WITH->logincommands != '\0')
      strcpy(hs, WITH->logincommands);
    else
      strcpy(hs, "L");
    box_input(unr, false, hs, true);
    WITH->action	= 86; /* promptunterdrueckung */
  }
}


void answer_sysrequest(short unr, boolean MD2, boolean MD5)
{
  boolean	okpw;
  userstruct	*WITH;
  char		nstr[256], pw[256], hs[256], STR1[256];

  okpw		= false;

  if (!boxrange(unr))
    return;

  WITH		= user[unr];
  if (strlen(WITH->password) >= MINPWLEN) {
    okpw	= true;
    switch (WITH->pwmode) {

    case PWM_MUST_PRIV:
    case PWM_SEND_ERASE:
      strcpy(hs, "send/erase-login");
      break;

    case PWM_RSYSOPPRIV:
    case PWM_RSYSOP:
      strcpy(hs, "board-sysop-login");
      break;

    case PWM_SYSOPPRIV:
    case PWM_SYSOP:
      strcpy(hs, "sysop-login");
      break;

    default:
      okpw = false;
      break;
    }
  }

  if (!okpw)
    return;

  sprintf(WITH->lastcmd, "tries %s", hs);
  if (WITH->pwmode >= 32 ||
      ((1L << WITH->pwmode) & ((1L << PWM_MUST_PRIV) | (1L << PWM_SEND_ERASE))) == 0)
    append_syslog(unr);
  *WITH->lastcmd = '\0';

  if (MD2) {
    calc_MD_prompt(md2_only_numbers, nstr);
    calc_MD2_pw(nstr, WITH->password, pw);
    *hs		= '\0';
    if (boxboxssid() > 0)
      sprintf(hs, "-%d", boxboxssid()); 
    sprintf(nstr, "%s%s> [%s]", Console_call, hs, strcpy(STR1, nstr));
  } else if (MD5) {
    calc_MD_prompt(false, nstr);
    calc_MD5_pw(nstr, WITH->password, pw);
    *hs		= '\0';
    if (boxboxssid() > 0)
      sprintf(hs, "-%d", boxboxssid()); 
    sprintf(nstr, "%s%s> [%s]", Console_call, hs, strcpy(STR1, nstr));
  } else {
    gimme_five_pw_numbers(WITH->password, hs);
    calc_pwtn(WITH->call, hs, pw);
    sprintf(nstr, "%s> %s", Console_call, hs);    
  }

  WITH->action	= 77;
  strcpy(WITH->input2, pw);
  wlnuser(unr, nstr);
}


static char s_space[]	= " ";
static char s_on[]	= " ON";
static char s_off[]	= " OFF";
static char s_dash[]	= "-";

static void p_ix(short unr, long time, char *ts)
{
  char	hs[256];

  wuser(unr, ts);
  if (time != 0) {
    ix2string4(time, hs);
    cut(hs, 16);   /* keine Sekunden */
    wlnuser(unr, hs);
  } else
    wlnuser(unr, s_dash);
}

void par_onoff(short unr, const char *par, boolean on)
{
  wuser(unr, par);
  if (on) wlnuser(unr, s_on);
  else wlnuser(unr, s_off);
}

void par_s(short unr, const char *par, short value)
{
  wuser(unr, par);
  wuser(unr, s_space);
  swuser(unr, value);
  wlnuser0(unr);
}

void par_l(short unr, const char *par, long value)
{
  wuser(unr, par);
  wuser(unr, s_space);
  lwuser(unr, value);
  wlnuser0(unr);
}

static void calc_timestring(time_t seconds, char *hs)
{
  sprintf(hs, "%ld h %ld min %ld", seconds / 3600, seconds % 3600 / 60, seconds % 60);
}


void show_stat(short unr, short x)
{
  time_t	seconds0, seconds1, seconds2;
  userstruct	*WITH;
  char		hs[256], w[256];

  if (!(boxrange(unr) && boxrange(x)))
    return;

  WITH		= user[x];
  seconds0	= clock_.ixtime - WITH->logindate;
  calc_timestring(seconds0, w);
  sprintf(hs, "Logintime   : %s sec", w);
  wlnuser(unr, hs);

  seconds1	= WITH->processtime / 200;
  seconds2	= (WITH->processtime % 200) >> 1;
  calc_timestring(seconds1, w);
  sprintf(hs, "CPU-Time    : %s.%.2ld sec", w, seconds2);
  wlnuser(unr, hs);
  sprintf(hs, "Bytecount   : RX %ld / TX %ld", WITH->rbytes, WITH->sbytes);
  wlnuser(unr, hs);
  if (seconds0 < 1)
    seconds0	= 1;
  if (seconds1 < 1)
    seconds1	= 1;
  sprintf(hs, "Speed       : %ld bps (HF) / %ld bps (CPU)",
		(WITH->rbytes + WITH->sbytes) * 8 / seconds0,
		(WITH->rbytes + WITH->sbytes) * 8 / seconds1);
  wlnuser(unr, hs);
}


/* sub-functions for show_user, mostly only one of them is actually executed */

static void show_u_u(short unr)
{
  print_dpboxusage(unr, wlnuser);
}

static void show_u_at(short unr, char *call, char *option)
{
  /* alle Calls mit MyBBS @CALLS ausgeben */

  if (callsign(call))
    show_all_user_at(unr, call, (strchr(option, '-') != NULL), false, (strchr(option, '/') != NULL), "");
  else
    wln_btext(unr, 2);
}

static void show_u_ct(short unr, char *call)
{
  char		hs[256];

  /* alle BBS ausgeben mit Anzahl Calls */

  strcpy(hs, call);
  show_all_user_at(unr, call, false, false, true, hs);
  return;
}

static void show_single_user(short unr, char *call)
{
  short		x;
  boolean	owner;
  long		sb, rb, l;
  userstruct	uf;
  char		hs[256], w[256];

  load_userfile(false, true, call, &uf);

  if (*uf.call == '\0' && actual_user(call) == 0) { /* no such user */
    wuser(unr, call);
    chwuser(unr, ' ');
    wln_btext(unr, 24);
    return;
  }

  if (*uf.call == '\0') {
    strcpyupper(uf.call, call);
    *uf.name	= '\0';
    user_language(&whichlangmem, &whichlangsize, whotalks_lan, uf.call, w);
    cut(w, 3);
    strcpy(uf.language, w);
    *uf.mybbs	= '\0';
  }

  owner		= !strcmp(user[unr]->call, uf.call);

  wuser(unr, "Call        : ");
  wlnuser(unr, uf.call);
  if (*uf.name != '\0') {
    wuser(unr, "Name        : ");
    wlnuser(unr, uf.name);
  }

  wuser(unr, "Language    : ");
  wlnuser(unr, uf.language);

  if (*uf.mybbs != '\0') {
    unhpath(uf.mybbs, hs);
    wuser(unr, "MyBBS       : ");
    wuser(unr, uf.mybbs);
    if (ring_bbs && !strcmp(hs, Console_call) && !uf.lock_here) {
      wlnuser(unr, " (swap)");
    } else {
      if (uf.mybbsmode == 'G')
        wuser(unr, " (guessed)");
      wlnuser0(unr);
    }
  }

  if (uf.lastdate > 0)	p_ix(unr, uf.lastdate,  "LastLogin   : ");
  if (uf.mybbsupd > 0)	p_ix(unr, uf.mybbsupd,  "LastMyBBS   : ");
  if (uf.lastatime > 0)	p_ix(unr, uf.lastatime, "LastTouch   : ");

  x	= actual_user(uf.call);
  if (uf.sslogins > 0 || x > 0) {
    if (x > 0) {
      show_stat(unr, x);
      sb	= user[x]->sbytes;
      rb	= user[x]->rbytes;
    } else {
      sb	= 0;
      rb	= 0;
    }

    if (user[unr]->supervisor) {
      if (x > 0)
	l	= user[x]->read_today;
      else
	l	= uf.read_today;
	if (l > 0 && uf.maxread_day > 0) {
	wuser(unr, "Total/day   : ");
	lwuser(unr, l);
	wuser(unr, " of max. ");
	if (uf.maxread_day > 0) {
	  lwuser(unr, uf.maxread_day);
	  wuser(unr, " (personal)");
	} else if (all_maxread_day < 999999999L) {
	  lwuser(unr, all_maxread_day);
	  wuser(unr, " (general)");
	} else
	  wuser(unr, "infinite (general)");
	wlnuser0(unr);
      }
    }

    wuser(unr, "Total/month : RX ");
    lwuser(unr, uf.srbytes + rb);
    wuser(unr, " / TX ");
    lwuser(unr, uf.ssbytes + sb);
    wlnuser0(unr);

    if (user[unr]->supervisor) {
      wuser(unr, "Total/ever  : RX ");
      lwuser(unr, uf.ssrbytes + rb);
      wuser(unr, " / TX ");
      lwuser(unr, uf.sssbytes + sb);
      wlnuser0(unr);
    }

    par_l(unr, "Logins/month:", uf.logins);

    if (user[unr]->supervisor) {
      par_l(unr, "Logins/ever :", uf.sslogins);
    }
  }

  if (uf.readlock != 0) {
    x = uf.readlock;
    if (uf.login_priv) x += 2;
    par_s(unr, "Readlock    :", x);
  }

  if (uf.ttl != gttl && (user[unr]->supervisor || owner)) {
    par_onoff(unr, "TTL         :", uf.ttl);
  }

  if (uf.sfmd2pw > 0 && (user[unr]->supervisor || owner)) {
    if (uf.sfmd2pw == 1)
      wlnuser(unr, "MD2SF       : ON");
    else if (uf.sfmd2pw == 2)
      wlnuser(unr, "MD5SF       : ON");
  }

  if (uf.fbbmode && (user[unr]->supervisor || owner)) {
    par_onoff(unr, "FBBMODE     :", uf.fbbmode);
  }

  if (uf.unproto_ok && (user[unr]->supervisor || owner)) {
    par_onoff(unr, "UNPROTO     :", uf.unproto_ok);
  }

  if (uf.hidebeacon && (user[unr]->supervisor || owner))
    wlnuser(unr, "MailBeacon  : OFF");

  if (user[unr]->supervisor) {
    par_s(unr, "Level       :", uf.level);

    if (uf.sf_level == 0) {
      if (in_real_sf(uf.call))
	uf.sf_level = 1;
    }
      
    if (uf.sf_level > 0) {
      par_s(unr, "SF-Level    :", uf.sf_level);
    }
      
    if (uf.pwmode > 0) {
      par_s(unr, "PW-Mode     :", uf.pwmode);
    }
  }

  if (!(user[unr]->supervisor || owner))
    return;

  if (*uf.checkboards != '\0') {
    if (uf.wantboards)
      wuser(unr, "WantBoard   : ");
    else
      wuser(unr, "NotBoard    : ");
    show_puffer(unr, &uf.checkboards[1], strlen(uf.checkboards)-2);
    wlnuser0(unr);
  }

  if (*uf.promptmacro != '\0') {
    wuser(unr, "Prompt      : ");
    wlnuser(unr, uf.promptmacro);
  }

  if (*uf.logincommands != '\0') {
    wuser(unr, "LoginCmd    : ");
    wlnuser(unr, uf.logincommands);
  }

  wuser(unr, "Password    : ");
  if (*uf.password == '\0') {
    strcpylower(w, call);
    sprintf(hs, "%s%s%cpwd", boxsfdir, w, extsep);
    if (exist(hs)) {
      if (user[unr]->supervisor)
	wuser(unr, "1620 chars");
      else
	chwuser(unr, '+');
    } else
      chwuser(unr, '-');
    wlnuser0(unr);

  } else {

    if (user[unr]->console)
      wlnuser(unr, uf.password);
    else {
      if (user[unr]->supervisor) {
        swuser(unr, strlen(uf.password));
        wuser(unr, " chars");
      } else
        chwuser(unr, '+');
      wlnuser0(unr);
    }

    if (user[unr]->supervisor)
      p_ix(unr, uf.pwsetat, "Activated   : ");
    if (uf.pwsetat + holddelay > clock_.ixtime) {
      if (user[unr]->supervisor || !strcmp(user[unr]->call, uf.call))
        p_ix(unr, uf.pwsetat + holddelay, "Active at   : ");
    }

  }
}


static void show_u_cur(short unr)
{
  short		x, last, fit;
  userstruct	*WITH;
  char		hs[256];

  fit		= 79 / (LEN_CALL+5);
  last		= MAXUSER;
  while (last > 0 && user[last] == NULL) last--;
  last		= last / fit + 1;
  last		*= fit;

  for (x = 1; x <= last; x++) {
    sprintf(hs, "%3d:", x);
    wuser(unr, hs);
    if (user[x] == NULL)
      *hs	= '\0';
    else {
      WITH	= user[x];
      if (WITH->pchan > 0) {
	strcpy(hs, WITH->call);
	if (WITH->f_bbs)
	  lower(hs);
      } else {
	if (user[unr]->supervisor) {
	  if (WITH->console)
	    strcpy(hs, "local");
	  else
	    strcpy(hs, WITH->call);
	  hs[0]	= lowcase(hs[0]);   /* Modem */
	} else
	  *hs	= '\0';
      }
    }
    rspacing(hs, LEN_CALL+1);
    wuser(unr, hs);
    if (x % fit == 0) wlnuser0(unr);
  }
  wlnuser0(unr);
}

static void show_u_path(short unr)
{
  short		x;
  char		hs[256];

  for (x = 1; x <= MAXUSER; x++) {
    if (boxrange(x)) {
      if (!user[x]->console || user[unr]->supervisor) {
	sprintf(hs, "%3d:", x);
	wuser(unr, hs);
	fill_logline(false, x, hs);
	wlnuser(unr, hs);
      }
    }
  }
}

static void show_u_all(short unr)
{
  char		arg[256];

  sprintf(arg, "%sboxlog%d%cbox -a -TN 1500", boxprotodir, clock_.mon, extsep);
  if (strstr(arg, boxdir) == arg)
    strdelete(arg, 1, strlen(boxdir));
  read_file_immediately(false, unr, arg);
}

void show_user(short unr, char *call, char *option)
{

  debug(2, unr, 93, call);

  if (strchr(option, '%')) show_u_u(unr);
  else if (strchr(option, '@')) show_u_at(unr, call, option);
  else if (strchr(option, '!')) show_u_ct(unr, call);
  else if (strchr(option, '+')) {
    if (strchr(option, '-')) show_all_user_at(unr, call, true, true, false, "");
    else show_single_user(unr, call);
  }
  else { /* no special args */
    show_u_cur(unr);
    if (compare(call, "PATH")) show_u_path(unr);
    if (compare(call, "ALL")) show_u_all(unr);
  }

}

/************************************************************************/
/* Functions for changing user settings					*/
/************************************************************************/

void set_password(short unr, boolean superv, char *eingabe_)
{
  short		x;
  long		t2;
  short		l1;
  userstruct	ufil;
  char		eingabe[256], w[256], hs[256], STR1[32];

  debug0(1, unr, 157);

  if (!boxrange(unr))
    return;

  strcpy(eingabe, eingabe_);

  if (superv) {
    get_word(eingabe, w);
    upper(w);
  }
  else
    strcpyupper(w, user[unr]->call);

  if (!callsign(w)) {
    wln_btext(unr, 3);
    return;
  }

  load_userinfo_for_change(false, w, &ufil);

  l1	= strlen(ufil.password);

  if (*eingabe == '\0') {
    strcpylower(STR1, w);
    sprintf(hs, "%s%s%cpwd", boxsfdir, STR1, extsep);
    if (!exist(hs)) {
      ufil.pwmode = 0;
      ufil.plevel = 1;
    }
    ufil.pwsetat = 0;
    if (l1 > 0) {
      *ufil.password = '\0';
      wln_btext(unr, 139);
      if (l1 >= MINPWLEN) {
	sprintf(STR1, "%s password deleted", ufil.call);
	debug(0, unr, 157, STR1);
      }
    } else if (!superv)
      wln_btext(unr, 140);
    else
      wlnuser(unr, "? ");
  }

  l1 = strlen(ufil.password);

  if (l1 + strlen(eingabe) <= 80)
    strcat(ufil.password, eingabe);
  else
    wln_btext(unr, 141);

  /* Modemuser mit pwmode 0 duerfen ihn nicht hochsetzen */
  if (strlen(ufil.password) >= MINPWLEN &&
      (ufil.pwmode != 0 || user[unr]->pchan > 0 ||
       user[unr]->console != false)) {
    if (ufil.pwmode < 2)
      ufil.pwmode = 2;
    w_btext(unr, 142);
    chwuser(unr, 32);
    swuser(unr, strlen(ufil.password));
    wlnuser0(unr);

    if (l1 < MINPWLEN) {
      sprintf(STR1, "%s now with active password", ufil.call);
      debug(0, unr, 157, STR1);
      if (superv)
	ufil.pwsetat = clock_.ixtime - holddelay;
      else {
	ufil.pwsetat = clock_.ixtime;
	if (holddelay > 0) {
	  t2 = ufil.pwsetat + holddelay;
	  ix2string(t2, w);
	  w_btext(unr, 143);
	  chwuser(unr, 32);
	  wlnuser(unr, w);
	}
      }
    }
  } else if (ufil.pwmode != 0 || user[unr]->pchan > 0 ||
	     user[unr]->console != false) {
    sprintf(w, "%d", strlen(ufil.password));
    w_btext(unr, 144);
    chwuser(unr, 32);
    wlnuser(unr, w);
  }

  for (x = 1; x <= MAXUSER; x++) {
    if (boxrange(x)) {
      if (!strcmp(user[x]->call, ufil.call)) {
	strcpy(user[x]->password, ufil.password);
	user[x]->pwmode		= ufil.pwmode;
	user[x]->pwsetat	= ufil.pwsetat;
	user[x]->plevel		= ufil.plevel;
      }
    }
  }
  save_userfile(&ufil);
  wlnuser(unr, "OK");
}

/* DL3NEU: PW-Import aus einem Textfile  */
/* Format des Files:                     */
/* <Call> <Passwort>                     */
/* mehrere Passwoerter pro File moeglich */

void import_pw(short unr, char *eingabe_)
{
  short		ih, x;
  userstruct	ufil;
  char		hs[256], w[256], w1[256];
  char		eingabe[256], ifn[256];

  strcpy(eingabe, eingabe_);
  get_word(eingabe, hs);
  if (insecure(hs)) {
    w_btext(unr, 4);
    return;
  }
  strcpy(ifn, boxdir);
  strcat(ifn, hs);
  if (!exist(ifn)) {
    w_btext(unr, 4);
    return;
  }
  ih = sfopen(ifn, FO_READ);
  if (ih < minhandle) {
    w_btext(unr, 149);
    return;
  }
  while (file2str(ih, hs)) {
    get_word(hs, w);
    upper(w);
    if (!callsign(w))
      continue;
    get_word(hs, w1);
    cut(w1, 80);
/*    if (strchr(w1, ';') != NULL)
      continue; */
    load_userinfo_for_change(false, w, &ufil);
    strcpy(ufil.password, w1);
    if (ufil.pwmode == 0 && strlen(ufil.password) >= MINPWLEN)
      ufil.pwmode = 2;
    ufil.pwsetat = clock_.ixtime - holddelay;
    for (x = 1; x <= MAXUSER; x++) {
      if (boxrange(x)) {
        if (!strcmp(user[x]->call, ufil.call)) {
 	 strcpy(user[x]->password, ufil.password);
 	 user[x]->pwmode	= ufil.pwmode;
 	 user[x]->pwsetat	= ufil.pwsetat;
        }
      }
    }
    save_userfile(&ufil);
  }
  sfclose(&ih);
}


static boolean local_prepare_change(short unr, char *eingabe, char *outpar, userstruct *ufil)
{
  char		w[256], c[256];

  if (!boxrange(unr)) return false;

  strcpy(w, eingabe);
  get_word(w, c);
  if (callsign(c) && user[unr]->supervisor) {
    upper(c);
    strcpy(outpar, w);
  } else {
    strcpy(c, user[unr]->call);
    strcpy(outpar, eingabe);
  }
  load_userinfo_for_change(false, c, ufil);
  return true;
}

static void adjust_and_save_userstructs(userstruct *ufil, long offset, long size)
{
  short		x;

  for (x = 1; x <= MAXUSER; x++) {
    if (user[x] != NULL) {
      if (!strcmp(user[x]->call, ufil->call)) {
        memcpy(&((char *)user[x])[offset], &((char *)ufil)[offset], size);
      }
    }
  }
  save_userfile(ufil);
}


void set_maxread(short unr, char *w, char *eingabe)
{
  userstruct	ufil;
  char		w1[256], w2[256];

  if (!boxrange(unr))
    return;

  if (!callsign(w)) {
    wln_btext(unr, 3);
    return;
  }

  load_userinfo_for_change(false, w, &ufil);

  ufil.maxread_day	= (atol(eingabe) + MAXREADDIVISOR - 1) / MAXREADDIVISOR * MAXREADDIVISOR;
  if (ufil.maxread_day > MAXREADDIVISOR * 255) ufil.maxread_day = MAXREADDIVISOR * 255;

  adjust_and_save_userstructs(&ufil, (long)&ufil.maxread_day - (long)&ufil, sizeof(ufil.maxread_day));

  if (ufil.maxread_day == 0)
    strcpy(w1, "infinite");
  else
    sprintf(w1, "%ld", ufil.maxread_day);

  sprintf(w2, "OK, max. read/day (%s) = %s bytes", w, w1);
  wlnuser(unr, w2);
}


void change_prompt(short unr, char *eingabe)
{
  userstruct	ufil;
  char		par[256];

  if (!local_prepare_change(unr, eingabe, par, &ufil))
    return;

  strcpy(ufil.promptmacro, par);
  adjust_and_save_userstructs(&ufil, (long)&ufil.promptmacro - (long)&ufil, sizeof(ufil.promptmacro));
  wlnuser(unr, "OK");
}


void change_startup(short unr, char *eingabe)
{
  userstruct ufil;
  char par[256];

  if (!local_prepare_change(unr, eingabe, par, &ufil))
    return;

  strcpy(ufil.logincommands, par);
  adjust_and_save_userstructs(&ufil, (long)&ufil.logincommands - (long)&ufil, sizeof(ufil.logincommands));
  wlnuser(unr, "OK");
}


void change_ttl(short unr, char *eingabe)
{
  userstruct	ufil;
  char		par[256];

  if (!local_prepare_change(unr, eingabe, par, &ufil))
    return;

  if (*par != '\0') {
    ufil.ttl = positive_arg(par);
    adjust_and_save_userstructs(&ufil, (long)&ufil.ttl - (long)&ufil, sizeof(ufil.ttl));
  }

  par_onoff(unr, "TTL", ufil.ttl);
}


void change_unprotomode(short unr, char *eingabe)
{
  userstruct	ufil;
  char		par[256];

  if (!local_prepare_change(unr, eingabe, par, &ufil))
    return;

  if (*par != '\0') {
    ufil.unproto_ok = positive_arg(par);
    adjust_and_save_userstructs(&ufil, (long)&ufil.unproto_ok - (long)&ufil, sizeof(ufil.unproto_ok));
  }

  par_onoff(unr, "UNPROTO", ufil.unproto_ok);
}


void change_md2sf(short unr, char *eingabe, boolean MD5)
{
  userstruct	ufil;
  char		par[256];

  if (!local_prepare_change(unr, eingabe, par, &ufil))
    return;

  if (*par != '\0') {
    ufil.sfmd2pw = 0;
    if (positive_arg(par)) {
      if (MD5)
        ufil.sfmd2pw = 2;
      else
        ufil.sfmd2pw = 1;
    }
    adjust_and_save_userstructs(&ufil, (long)&ufil.sfmd2pw - (long)&ufil, sizeof(ufil.sfmd2pw));
  }

  switch (ufil.sfmd2pw) {
    case 0:
      wlnuser(unr, "MD2SF/MD5SF OFF");
      break;
    case 1:
      wlnuser(unr, "MD2SF ON");
      break;
    case 2:
      wlnuser(unr, "MD5SF ON");
      break;
  }
}


void change_fbbmode(short unr, char *eingabe)
{
  userstruct	ufil;
  char		par[256];

  if (!local_prepare_change(unr, eingabe, par, &ufil))
    return;

  if (*par != '\0') {
    ufil.fbbmode = positive_arg(par);
    adjust_and_save_userstructs(&ufil, (long)&ufil.fbbmode - (long)&ufil, sizeof(ufil.fbbmode));
  }

  par_onoff(unr, "FBBMODE", ufil.fbbmode);
}


void change_readlock(short unr, char *eingabe)
{
  short		x;
  userstruct	ufil;
  char		par[256], hs[256];

  if (!local_prepare_change(unr, eingabe, par, &ufil))
    return;

  if (*par != '\0') {
    x = atoi(par);
    if (x >= 0 && x <= 4) {
      if (x > 2) {
        if (x == 3) x = 1;
        if (x == 4) x = 2;
	strcpylower(par, ufil.call);
	sprintf(hs, "%s%s%cpwd", boxsfdir, par, extsep);
        if (strlen(ufil.password) >= MINPWLEN || exist(hs))
          ufil.login_priv = true;
        else
          ufil.login_priv = false;
      } else {
        ufil.login_priv = false;
      }
      ufil.readlock = x;

      for (x = 1; x <= MAXUSER; x++) {
	if (user[x] != NULL) {
	  if (!strcmp(user[x]->call, ufil.call))
	    user[x]->readlock = ufil.readlock;
	    user[x]->login_priv = ufil.login_priv;
	}
      }
      save_userfile(&ufil);
    }
  }
  x = ufil.readlock;
  if (ufil.login_priv) x += 2;
  par_s(unr, "ReadLock", x);
}


void change_mailbeacon(short unr, char *eingabe)
{
  userstruct	ufil;
  char		par[256];

  if (!local_prepare_change(unr, eingabe, par, &ufil))
    return;

  if (*par != '\0') {
    ufil.hidebeacon = !positive_arg(par);
    adjust_and_save_userstructs(&ufil, (long)&ufil.hidebeacon - (long)&ufil, sizeof(ufil.hidebeacon));
  }

  par_onoff(unr, "MailBeacon", !ufil.hidebeacon);
}

void change_levels(short unr, short cnr, char *callx, short lvl)
{
  short		x;
  userstruct	ufil;

  debug(2, unr, 95, callx);

  load_userinfo_for_change(false, callx, &ufil);
  if (lvl > 127) {
    wlnuser(unr, "max. level = 127!");
    lvl = 127;
  }

  switch (cnr) {

  case 25:
    if (lvl > 1) lvl	= 1;
    ufil.sf_level	= lvl;
    ufil.ssf_level	= lvl;
    break;

  case 29:
    ufil.level		= lvl;
    ufil.plevel		= lvl;
    break;

  case 66:
    ufil.pwmode		= lvl;
    break;
  }

  for (x = 1; x <= MAXUSER; x++) {
    if (user[x] != NULL) {
      if (!strcmp(user[x]->call, ufil.call)) {

	switch (cnr) {

	case 25:
	  user[x]->sf_level	= lvl;
	  user[x]->ssf_level	= lvl;
	  break;

	case 29:
	  if (user[x]->se_ok)
	    user[x]->level	= lvl;
	  user[x]->plevel	= lvl;
	  break;

	case 66:
	  user[x]->pwmode	= lvl;
	  break;
	}

      }
    }
  }
  save_userfile(&ufil);
  wln_btext(unr, 57);
}


void change_name(short unr, char *callx, char *w_)
{
  userstruct	ufil;
  char		w[256], STR1[256];

  debug(2, unr, 96, callx);

  strcpy(w, w_);

  if (strlen(w) <= 1) {
    w_btext(unr, 60);
    user[unr]->action	= 78;
    return;
  }

  cut(w, 80);
  gkdeutsch(w);
  load_userinfo_for_change(false, callx, &ufil);
  strcpy(ufil.name, w);
  adjust_and_save_userstructs(&ufil, (long)&ufil.name - (long)&ufil, sizeof(ufil.name));

  if (!strcmp(callx, user[unr]->call)) {
    w_btext(unr, 58);
    chwuser(unr, 32);
    wuser(unr, w);
    wln_btext(unr, 59);
  } else {
    sprintf(STR1, "OK, %s -> %s", callx, w);
    wlnuser(unr, STR1);
  }
  user[unr]->action	= 0;
}


static boolean chmbxsemaphore;


void change_mybbs(short unr, char *callx_, char *bbs_, long updatetime,
		  char *level_, char mybbsmode, boolean update, boolean fwd)
{
  short		x, mssid;
  boolean	found, iscall;
  userstruct	ufil;
  calltype	callx;
  char	      	*p;
  char		bbs[256], level[256], hs[256], w[256], uname[256];

  if (chmbxsemaphore)	/* sonst ruft sich das doppelt auf wegen	*/
    return;		/* update_mybbsfile				*/

  chmbxsemaphore	= true;
  debug(2, unr, 94, callx_);
  strcpy(callx, callx_);
  strcpy(bbs, bbs_);
  strcpy(level, level_);
  
  if (updatetime == 0)
    updatetime		= clock_.ixtime;
  
  if (updatetime >= clock_.ixtime + 12*60*60) {
    chmbxsemaphore	= false;
    return;
  }

  load_userinfo_for_change(update, callx, &ufil);

    /* nachschauen ob
     * a) not update (dann wars der User in der Box selber)
     * b) found and mode != 'U' and mybbsmode == 'U'
     * c) found and mode != 'U' and mybbsmode != 'U' and updatetime > bbsrec.ix_time
     * d) found and mode == 'U' and mybbsmode == 'U' and updatetime > bbsrec.ix_time
     */
    
  found		= ufil.lastatime > 0;

  if (found) strcpy(uname, ufil.name); /* name für WP merken */
  else *uname = '\0';  

  if (!update ||
        (  (found && ufil.mybbsmode != 'U' && mybbsmode == 'U')
        || (found && ufil.mybbsmode != 'U' && mybbsmode != 'U' && ufil.mybbsupd < updatetime)
        || (found && ufil.mybbsmode == 'U' && mybbsmode == 'U' && ufil.mybbsupd < updatetime)
     ))
    {

    cut(bbs, LEN_MBX);

    x	= strpos2(bbs, ".", 1);
    if (x > 0) {
      sprintf(hs, "%.*s", x - 1, bbs);
      strdelete(bbs, 1, x);
    } else {
      strcpy(hs, bbs);
      *bbs = '\0';
    }

    if (boxrange(unr)) {
      x	= strpos2(hs, "-", 1);
      if (x > 0) {
	strcpy(w, hs);
	strdelete(w, 1, x);
	strdelete(hs, x, strlen(hs) - x + 1);
	mssid			= atoi(w);

	if (!strcmp(hs, Console_call))
	  ufil.lock_here	= (mssid == boxboxssid());
	else
	  ufil.lock_here	= false;
      } else
	ufil.lock_here		= (!strcmp(hs, Console_call) && multi_master);
    } else {
      if ((p = strchr(hs, '-')) != NULL) *p = '\0';
      if (strcmp(hs, Console_call))
	ufil.lock_here		= false;
      else
        ufil.lock_here		= !ring_bbs;
    }

    iscall	= callsign(hs);

    if (*bbs != '\0')
      sprintf(bbs, "%s.%s", hs, strcpy(w, bbs));
    else {
      complete_hierarchical_adress(hs);
      strcpy(bbs, hs);
    }

    strcpy(ufil.mybbs, bbs);	/* see below, we need "bbs" to be filled with the mybbs... */
    ufil.mybbsupd		= updatetime;
    ufil.mybbsmode		= mybbsmode;
    if (zahl(level))
      ufil.level		= atoi(level);

    for (x = 1; x <= MAXUSER; x++) {
      if (user[x] != NULL) {
	if (!strcmp(user[x]->call, ufil.call)) {
	  strcpy(user[x]->mybbs, ufil.mybbs);
	  user[x]->mybbsupd	= ufil.mybbsupd;
	  user[x]->mybbsmode	= ufil.mybbsmode;
	  user[x]->level	= ufil.level;
	  user[x]->lock_here	= ufil.lock_here;
	}
      }
    }

    save_userfile(&ufil);

    if (boxrange(unr)) {  /* if !boxrange, this function was called by update_mybbsfile() */
      update_mybbsfile(true, callx, &updatetime, bbs, &mybbsmode);

      if (fwd && mybbsmode == 'U') {
	if (!update) { /* hmm... that's ok. This is for locally entered infos (21.01.2000) */
	  get_word(uname, w); /* sorry, but WP only accepts one word for the name */
	  if (!*w) strcpy(w, "?");
      	  snprintf(hs, 255, "M @ %s < %s $%s %s %ld ? %s ?",
      	      	    e_m_verteiler, callx, WPDUMMYBID, bbs, updatetime+1, w);
      	  sf_rx_emt1(hs, Console_call);
	}
      }

      if (!iscall) wlnuser(unr, "warning: bbs is non-callsign!");
      if (!strcmp(callx, user[unr]->call)) {
	w_btext(unr, 56);
	chwuser(unr, 32);
	wlnuser(unr, bbs);
      } else {
	sprintf(hs, "OK, %s -> @%s level = %d", callx, bbs, ufil.level);
	wlnuser(unr, hs);
      }

      sprintf(hs, "%s@%s", callx, bbs);
      gen_sftest(unr, hs);
    }
  }
  chmbxsemaphore = false;

}



boolean valid_language(char *lan)
{
  languagetyp *hptr;

  hptr		= langmemroot;
  while (hptr != NULL) {
    if (hptr->puffer != NULL) {
      if (!strcmp(hptr->sprache, lan))
	return true;
    }
    hptr	= hptr->next;
  }
  return false;
}


void show_languages(short unr)
{
  languagetyp	*hptr;
  short		x;
  char		hs[256];

  wln_btext(unr, 66);
  wlnuser0(unr);
  hptr		= langmemroot;
  x		= 0;
  while (hptr != NULL) {
    if (hptr->puffer != NULL) {
      strcpy(hs, hptr->sprache);
      rspacing(hs, 4);
      wuser(unr, hs);
      x++;
      if (x % 18 == 0) wlnuser0(unr);
    }
    hptr	= hptr->next;
  }
  if (x % 18 != 0) wlnuser0(unr);
}


void change_language(short unr, char *callx, char *w_)
{
  userstruct	ufil;
  char		w[256];

  debug(2, unr, 97, callx);

  strcpyupper(w, w_);

  if ((unsigned long)strlen(w) >= 32 || ((1L << strlen(w)) & 0xe) == 0) { /* CHECK!!! */
    wln_btext(unr, 62);
    show_languages(unr);
    return;
  }

  if (!valid_language(w)) {
    wln_btext(unr, 62);
    show_languages(unr);
    return;
  }

  load_userinfo_for_change(false, callx, &ufil);
  strcpy(ufil.language, w);
  adjust_and_save_userstructs(&ufil, (long)&ufil.language - (long)&ufil, sizeof(ufil.language));

  if (strcmp(callx, user[unr]->call)) {
    sprintf(w, "OK, %s -> %s", callx, ufil.language);
    wlnuser(unr, w);
    return;
  }

  w_btext(unr, 61);
  chwuser(unr, 32);
  wlnuser(unr, ufil.language);
}


void show_systemtime(short unr)
{
  utc_clock();
  wuser(unr, clock_.zeit);
  wuser(unr, " UTC ");
  wlnuser(unr, clock_.datum4);
}


void new_login(short unr, char *times_)
{
  short		d, m, y;
  userstruct	*WITH;
  short		TEMP;
  char	      	*p;
  char		hs[256], times[256];

  if (!boxrange(unr))
    return;

  strcpy(times, times_);

  WITH		= user[unr];
  while ((p = strchr(times, '.')) != NULL)
    *p = ' ';

  TEMP	= count_words(times);
  if ((unsigned)TEMP < 32 && ((1L << TEMP) & 0xc) != 0) {
    get_word(times, hs);
    d		= atoi(hs);
    if (d >= 32 || ((1L << d) & 0xfffffffe) == 0)
      d		= clock_.day;
    get_word(times, hs);
    m		= atoi(hs);
    if (m >= 32 || ((1L << m) & 0x1ffe) == 0)
      m		= clock_.mon;
    get_word(times, hs);
    y		= atoi(hs);
    if ((hs[0] == '\0') || (y > 99))
      y		= clock_.year;
    else if (y < 38) y += 100;
    WITH->lastdate = calc_ixtime(d, m, y, 0, 0, 0);
  }

  w_btext(unr, 63);
  chwuser(unr, ' ');
  ix2string(WITH->lastdate, hs);
  cut(hs, 9);	/* Keine Sekunden zeigen	*/
  wlnuser(unr, hs);
}


void show_version(short unr, boolean extended)
{
  char		*p;
  long		sz, rp;
  short		s1, s2, s3;
  short		x, y;
  char		hs[256];

  if (!extended) {
#undef __dp_version_ok
#ifdef __macos__
    sprintf(hs, "dpbox (MacOS) v%s%s %s", dp_vnr, dp_vnr_sub, dp_date);
    wuser(unr, hs);
    wlnuser(unr, " (c) 1990-2000 Joachim Schurig, DL8HBS");
    wlnuser(unr, "parts of the code: (c) 1994-1997 Mark Wahl, DL4YBG");
#define __dp_version_ok
#endif
#ifdef __linux__
    sprintf(hs, "dpbox (Linux) v%s%s %s", dp_vnr, dp_vnr_sub, dp_date);
    wuser(unr, hs);
    wlnuser(unr, " (c) 1990-2000 Joachim Schurig, DL8HBS");
    wlnuser(unr, "Linux porting  : (c) 1994-1997 Mark Wahl, DL4YBG");
#define __dp_version_ok
#endif
#ifdef __NetBSD__
    sprintf(hs, "dpbox (NetBSD) v%s%s %s", dp_vnr, dp_vnr_sub, dp_date);
    wuser(unr, hs);
    wlnuser(unr, " (c) 1990-2000 Joachim Schurig, DL8HBS");
    wlnuser(unr, "Linux porting  : (c) 1994-1997 Mark Wahl, DL4YBG");
    wlnuser(unr, "NetBSD porting : (c) 1999 Berndt Josef Wulf, VK5ABN");
#define __dp_version_ok
#endif
#ifndef __dp_version_ok
    sprintf(hs, "dpbox v%s%s %s", dp_vnr, dp_vnr_sub, dp_date);
    wuser(unr, hs);
    wlnuser(unr, " (c) 1990-2000 Joachim Schurig, DL8HBS");
    wlnuser(unr, "Linux porting  : (c) 1994-1997 Mark Wahl, DL4YBG");
    wlnuser(unr, "NetBSD porting : (c) 1999 Berndt Josef Wulf, VK5ABN");
#endif
#undef __dp_version_ok

    wuser(unr, "dpbox runtime  : ");
    get_boxruntime_s(1, hs);
    wlnuser(unr, hs);
    wuser(unr, "total runtime  : ");
    get_boxruntime_s(2, hs);
    wlnuser(unr, hs);
    wuser(unr, "system runtime : ");
    calc_ixsecs_to_string(get_sysruntime(), hs);
    wlnuser(unr, hs);

    y = 0;
    for (x = 1; x <= MAXUSER; x++) {
      if (user[x] != NULL)
	y++;
    }

    wuser(unr, "Logins         : ");
    swuser(unr, y);
    sprintf(hs, "%d", MAXUSER);
    wuser(unr, " of max. ");
    swuser(unr, MAXUSER);
    wlnuser0(unr);

    get_dpboxusage(&s1, &s2, &s3);
    sprintf(hs, "dpbox sysload  : %d%% %d%% %d%%", s1, s2, s3);
    wlnuser(unr, hs);
    wuser(unr, "System load    : ");
    get_sysload(hs);
    wlnuser(unr, hs);

    wuser(unr, "System version : ");
    get_sysversion(hs);
    wlnuser(unr, hs);

    wlnuser0(unr);
    w_btext(unr, 148);
    wlnuser(unr, " V +");

    return;
  }

  p	= NULL;
  boxgetstatus(user[unr]->supervisor, &p, &sz);
  if (p == NULL)
    return;

  rp	= 0;
  while (rp < sz) {
    get_line(p, &rp, sz, hs);
    wlnuser(unr, hs);
  }
  boxfreestatp(&p);
}


void disc_user(short unr, char *eingabe_)
{
  short		x, lo, hi;
  boolean	allsf, alluser, all;
  char		eingabe[256];
  char		hs[256];

  debug(2, unr, 98, eingabe_);

  strcpy(eingabe, eingabe_);
  get_word(eingabe, hs);
  upper(hs);
  cut(hs, LEN_CALL);

  if (!strcmp(hs, "ALL")) {
    lo		= 1;
    hi		= MAXUSER;
    allsf	= true;
    alluser	= true;
    all		= true;
  } else if (!strcmp(hs, "USER")) {
    lo		= 1;
    hi		= MAXUSER;
    allsf	= false;
    alluser	= true;
    all		= false;
  } else if (!strcmp(hs, "SF")) {
    lo		= 1;
    hi		= MAXUSER;
    allsf	= true;
    alluser	= false;
    all		= false;
  } else {
    if (callsign(hs))
      lo	= actual_user(hs);
    else
      lo	= atoi(hs);
    hi		= lo;
    allsf	= false;
    all		= true;
    alluser	= false;
  }

  for (x = lo; x <= hi; x++) {
    if (x >= 1 && x <= MAXUSER) {
      if (x != unr) {
	if (boxrange(x)) {
	  if ((user[x]->f_bbs && allsf) || (!user[x]->f_bbs && alluser) || all) {

	    abort_useroutput(x);

	    if (*eingabe != '\0' && !user[x]->f_bbs) {
	      wlnuser(x, "");
	      wlnuser(x, eingabe);
	      wlnuser(x, "");
	    }

	    if (boxgetcompmode(user[x]->pchan)) {
	      wlnuser0(x);
	      wlnuser(x, "//COMP 0");
	      boxsetcompmode(x, false);
	    }

	    boxspoolread();

	    abort_useroutput(x);
	    end_boxconnect(x);

	  }

	}
      }
    }
  }

}


void write_ctext(short unr, char *eingabe)
{
  char		hs[256];

  debug(3, unr, 99, eingabe);
  if (uspos("***END", eingabe) > 0 || useq(eingabe, "/EX") || strchr(eingabe, '\032') != NULL) {
    sfclose(&user[unr]->sendchan);
    user[unr]->action	= 0;
    sprintf(hs, "%sctext%cnew", boxstatdir, extsep);

    if (sfsize(hs) == 0) {
      sfdelfile(hs);
      sfdelfile(ctext_box);
      wlnuser(unr, "ctext deleted");
      return;
    }

    sfdelfile(ctext_box);
    sfrename(hs, ctext_box);
    wlnuser(unr, "ctext changed");
    return;
  }

  if (strchr(eingabe, '\030') == NULL) {
    str2file(&user[unr]->sendchan, eingabe, true);
    return;
  }

  sfclose(&user[unr]->sendchan);
  user[unr]->action	= 0;
  sprintf(hs, "%sctext%cnew", boxstatdir, extsep);
  sfdelfile(hs);
  wlnuser(unr, "ctext unchanged");
}


void enter_ctext(short unr)
{
  short		k;
  char		STR1[256];

  wlnuser(unr, "CTEXT:");
  show_textfile(unr, ctext_box);
  w_btext(unr, 47);
  wlnuser(unr, " CTEXT ");
  wln_btext(unr, 48);
  wlnuser0(unr);
  sprintf(STR1, "%sctext%cnew", boxstatdir, extsep);
  k	= sfcreate(STR1, FC_FILE);
  if (k >= minhandle) {
    user[unr]->action	= 79;
    user[unr]->sendchan	= k;
  } else
    wlnuser(unr, "can't create file");
}


void search_bull(short unr, char *w, boolean kill, boolean add)
{
  long		pos;
  char		hs[256];

  if (*w != '\0') {
    wuser(unr, "searching ");
    wlnuser(unr, w);
    pos	= bull_mem(w, kill);
    if (pos >= 0) {
      sprintf(hs, "found at position %ld", pos);
      if (kill)
        strcat(hs, "  - now deleted");
      wlnuser(unr, hs);
    } else
      wlnuser(unr, "... not found");

    if (add) {
      write_msgid(-1, w);
      wlnuser(unr, "now added");
    }
  }
  par_l(unr, "MaxBullIDs:", maxbullids);
  par_l(unr, "BullIDSeek:", bullidseek);
  par_l(unr, "BullIDs   :", sfsize(msgidlog) / sizeof(bidtype));
}


void set_talk_mode(short unr, char *w)
{
  short		xchan;

  if (!boxrange(unr))
    return;

  xchan		= str2disturb_chan(w);
  if (!boxrange(xchan))
    return;

  if (xchan <= 0 || xchan == unr) {
    wln_btext(unr, 11);
    return;
  }

  wlnuser(unr, "link setup...");
  wlnuser0(xchan);
  wuser(xchan, "*** connect request of ");
  wlnuser(xchan, user[unr]->call);
  boxspoolfend(user[xchan]->pchan);
  wuser(unr,   "*** connected to ");
  wuser(xchan, "*** connected to ");
  wlnuser(unr, user[xchan]->call);
  wlnuser(xchan, user[unr]->call);
  wln_btext(unr, 145);
  wln_btext(xchan, 145);
  wlnuser0(unr);
  wlnuser0(xchan);
  user[unr]->action	= 80;
  user[xchan]->action	= 80;
  user[unr]->talk_to	= xchan;
  user[xchan]->talk_to	= unr;
}


void end_talk_mode(short unr, boolean prompt)
{
  short		ssid;

  if (!boxrange(unr))
    return;

  user[unr]->talk_to	= 0;
  user[unr]->action	= 0;
  wlnuser0(unr);
  wuser(unr, "*** reconnected to ");
  wuser(unr, Console_call);
  ssid	= boxboxssid();
  if (ssid != 0) {
    chwuser(unr, '-');
    swuser(unr, ssid);
  }
  wlnuser0(unr);
  wlnuser0(unr);
  if (prompt) show_prompt(unr);
}


void talk_line(short unr, char *eingabe)
{
  boolean	endtalk;
  char		*p;

  if (!boxrange(unr))
    return;

  endtalk	= false;
  p		= strchr(eingabe, '\032');
  if (!p) {
    if (useq(eingabe, "/Q")) {
      p = eingabe;
    }
  }
  if (p) {
    endtalk	= true;
    *p		= '\0';
  }

  wlnuser(user[unr]->talk_to, eingabe);

  if (!boxrange(user[unr]->talk_to))
    endtalk	= true;

  if (endtalk) {
    end_talk_mode(user[unr]->talk_to, true);
    end_talk_mode(unr, false);
  }
}


void msgwuser(short unr, boolean immediate, char *s, boolean cr)
{
  char		STR1[256];

  if (immediate) {
    if (cr)
      wlnuser(unr, s);
    else
      wuser(unr, s);
    return;
  }
  if (user[unr] != NULL) {
    sprintf(STR1, "%s%s%cMSG", boxstatdir, user[unr]->call, extsep);
    append(STR1, s, cr);
    user[unr]->newmsg	= true;
  }
}


void send_msg(short unr, char *w, char *eingabe)
{
  short		xchan, fp, ct, umb;
  boolean	noprompt, allf, immediate;
  short		lastchan;
  char		hs[256], z2[256];

  if (!*eingabe) {
    wln_btext(unr, 2);
    return;
  }

  if (!strcmp(w, "ALL") /* && user[unr]->supervisor */ ) {
    allf	= true;
    xchan	= 0;
  } else {
    xchan	= str2disturb_chan2(w) - 1;
    allf	= false;
  }

  if (xchan < 0) {
    wln_btext(unr, 11);
    return;
  }

  noprompt	= (eingabe[0] == '&' && xchan == unr);
  if (noprompt) {
    strdelete(eingabe, 1, 1);
    umb	= 79;
  } else
    umb	= 55;

  if (allf)
    lastchan	= MAXUSER;
  else
    lastchan	= xchan + 1;

  strcpy(z2, eingabe);

  while (xchan < lastchan) {
    xchan++;
    if (user[xchan] == NULL || user[xchan]->f_bbs)
      continue;

    immediate	= (	user[xchan]->action == 0 &&
			user[xchan]->lastprocnumber == 0 &&
			boxspoolstatus(user[xchan]->tcon, user[xchan]->pchan, -1) == 0 );
    strcpy(eingabe, z2);
    msgwuser(xchan, immediate, "", true);
    msgwuser(xchan, immediate, "", true);
    if (!noprompt) {
      sprintf(hs, "MSG de %s (%d)", user[unr]->call, unr);
      rspacing(hs, 20);
      strcat(hs, ": ");
      msgwuser(xchan, immediate, hs, false);
    }

    ct		= 0;
    do {
      ct++;
      strcpy(hs, eingabe);
      if (strlen(hs) > umb) {
	fp	= umb;
	while (hs[fp - 1] != ' ' && fp > 1)
	  fp--;
	if (fp == 1)
	  fp	= umb;
	cut(hs, fp);
	if (ct > 1 && !noprompt)
	  msgwuser(xchan, immediate, "                      ", false);
	msgwuser(xchan, immediate, hs, true);
	strdelete(eingabe, 1, fp);
      } else {
	if (ct > 1 && !noprompt)
	  msgwuser(xchan, immediate, "                      ", false);
	msgwuser(xchan, immediate, hs, true);
	*eingabe = '\0';

      }

    } while (*eingabe != '\0');

    if (noprompt)
      continue;

    if (immediate) show_prompt(xchan);
    else wuser(unr, "DELAYED ");
    sprintf(hs, "MSG OK FOR %s (%d)", user[xchan]->call, xchan);
    wlnuser(unr, hs);
  }
}



void set_comp_mode(short unr, char *w)
{
  short		chan;

  chan	= user[unr]->pchan;
  if (!strcmp(w, "ON")) {
    if (boxgetcompmode(chan) != false)
      return;
    boxsetcompwait(chan, 1);
    wlnuser(unr, "//COMP 1");
    boxspoolfend(chan);
    boxsetcompmode(chan, true);
    return;
  }

  if (!strcmp(w, "OFF")) {
    if (!boxgetcompmode(chan))
      return;
    boxsetcompwait(chan, 2);
    wlnuser(unr, "//COMP 0");
    boxspoolfend(chan);
    boxsetcompmode(chan, false);
    return;
  }

  if (!strcmp(w, "1")) {
    boxsetcompmode(chan, true);
    return;
  }

  if (!strcmp(w, "0"))
    boxsetcompmode(chan, false);
  else
    wln_btext(unr, 2);
}


void reset_to_term(short unr, boolean to_sat, boolean to_node)
{
  debug0(2, unr, 100);
  wln_btext(unr, 1);
}


void show_ext_command_result(short unr)
{
  userstruct	*WITH;
  short		outfiles;
  char		STR1[256];

  if (!boxrange(unr))
    return;

  WITH		= user[unr];
  if (*WITH->wait_file == '\0')
    return;

  outfiles	= 0;
  if (exist(WITH->wait_file)) outfiles++;
  new_ext(WITH->wait_file, "trn");
  if (exist(WITH->wait_file)) outfiles++;
  new_ext(WITH->wait_file, "bin");
  if (exist(WITH->wait_file)) outfiles++;
  new_ext(WITH->wait_file, "yap");
  if (exist(WITH->wait_file)) outfiles++;
  
  if (outfiles == 0) {
    WITH->wait_file[0] = '\0';
    return;
  }

  new_ext(WITH->wait_file, "txt"); 
  if (exist(WITH->wait_file)) {
    strdelete(WITH->wait_file, 1, strlen(boxdir));
    sprintf(STR1, "%s -A", WITH->wait_file);
    if (outfiles == 1)
      read_file(true, unr, STR1);
    else
      read_file_immediately(true, unr, STR1);
    sprintf(STR1, "%s%s", boxdir, WITH->wait_file);
    strcpy(WITH->wait_file, STR1);
  }

  new_ext(WITH->wait_file, "trn");
  if (exist(WITH->wait_file)) {
    strdelete(WITH->wait_file, 1, strlen(boxdir));
    sprintf(STR1, "%s -F", WITH->wait_file);
    if (outfiles == 1)
      read_file(true, unr, STR1);
    else
      read_file_immediately(true, unr, STR1);
    sprintf(STR1, "%s%s", boxdir, WITH->wait_file);
    strcpy(WITH->wait_file, STR1);
  }

  new_ext(WITH->wait_file, "bin");
  if (exist(WITH->wait_file)) {
    strdelete(WITH->wait_file, 1, strlen(boxdir));
    sprintf(STR1, "%s", WITH->wait_file);
    if (outfiles == 1)
      read_file(true, unr, STR1);
    else
      read_file_immediately(true, unr, STR1);
    sprintf(STR1, "%s%s", boxdir, WITH->wait_file);
    strcpy(WITH->wait_file, STR1);
  }

  new_ext(WITH->wait_file, "yap");
  if (exist(WITH->wait_file)) {
    if (outfiles != 1) {
      sfdelfile(WITH->wait_file);
    } else {
      strdelete(WITH->wait_file, 1, strlen(boxdir));
      sprintf(STR1, "%s -Y", WITH->wait_file);
      read_file(true, unr, STR1);
    }
  }

  WITH->wait_file[0] = '\0';
  
  if (outfiles == 1) {
    if (read_filepart(unr))
      WITH->action = 96;
  }
}


static unsigned short lastrunnum;

boolean open_shell(short unr, boolean transparent)
{
  short		olda;
  userstruct	*WITH;

  if (!boxrange(unr))
    return false;

  WITH		= user[unr];
  if (!WITH->supervisor)
    return false;

  WITH->wait_file[0]	= '\0';
  olda			= WITH->action;
  WITH->action		= 99;
  boxsetrwmode(WITH->pchan, RW_RAW);
  if (cmd_shell(unr, transparent) == 0) {
    boxsetrwmode(WITH->pchan, RW_NORMAL);
    WITH->action	= olda;
    return false;
  } else {
    wlnuser(unr, "OK");
    return true;
  }
}


void call_runprg(boolean extern_, short unr, char *fname_, char *rkommando_,
		 boolean full_gem, char *add_environment)
{
  short		olda;
  userstruct	*WITH;
  char		fname[256], rkommando[256];
  char		w[256], rd[256], ofi[256];
  char		adir[256];
  char		STR1[256];

  debug(2, unr, 101, fname_);

  if (!boxrange(unr))
    return;

  strcpy(fname, fname_);

  WITH	= user[unr];
  strcpy(rd, boxrundir);
  if (insecure(fname))
    *fname = '\0';
  if (!extern_)
    lower(fname);
  sprintf(fname, "%s%s", rd, strcpy(STR1, fname));

  if (*fname == '\0')
    return;

  recompile_log(unr);
  recompile_fwd();

  lastrunnum++;

  sfgetdir(0, adir);
  sprintf(w, "%d", lastrunnum);
  sprintf(ofi, "%sout%s%ctrn", rd, w, extsep);
  sfdelfile(ofi);
  new_ext(ofi, "bin");
  sfdelfile(ofi);
  new_ext(ofi, "txt");
  sfdelfile(ofi);

  olda			= WITH->action;
  WITH->action		= 99;
  strcpy(WITH->wait_file, ofi);

  sfchdir(rd);
  boxsetrwmode(WITH->pchan, RW_RAW);
  sprintf(rkommando, "%s %s", fname, rkommando_);
  if (cmd_run(unr, full_gem, rkommando, ofi, add_environment) == 0) {
    boxsetrwmode(WITH->pchan, RW_NORMAL);
    WITH->action	= olda;
    *WITH->wait_file	= '\0';
  }
  sfchdir(adir);
}


typedef char est[2];

void stelle_uhr(char *eingabe, boolean datum)
{
  static est est1 = ".", est2 = "/", est3 = ":", est4 = "-";

  short	h, m, s;
  char	w[256];
  char	datestr[256];

  strcpy(datestr, eingabe);
  ersetze(est1, "  ", datestr);
  ersetze(est2, "  ", datestr);
  ersetze(est3, "  ", datestr);
  ersetze(est4, "  ", datestr);
  get_word(datestr, w);
  if (*w != '\0') {
    h = atoi(w);
    get_word(datestr, w);
    m = atoi(w);
    get_word(datestr, w);
    s = atoi(w);
/*
    if (datum)
      dpsetdate(s + 1900, m, h);
    else
      dpsettime(h, m, s);
*/
  }

  utc_clock();
}


void calc_boxactivity(short unr, cbaproc outputproc)
{
  short		x;
  long		isec;
  userstruct	*WITH;
  char		hs[512], w[256];

  utc_clock();
  outputproc(unr, "Call   BCh     TxBuff Idle     CPU Board    Command");

  for (x = 1; x <= MAXUSER; x++) {
    if (boxrange(x)) {
      WITH	= user[x];
      strcpy(hs, WITH->call);
      if (WITH->f_bbs) {
	lower(hs);
	if (WITH->sf_to)
	  strcat(hs, "<");
	else
	  strcat(hs, ">");
      }
      rspacing(hs, 7);
      sprintf(w, "%3d", x);
      strcat(hs, w);
      if (WITH->umode < 32 &&
	  ((1L << WITH->umode) & ((1L << UM_MODEM_IN) | (1L << UM_MODEM_OUT))) != 0)
	strcpy(w, "MDM");
      else if (WITH->pchan > 0)
	strcpy(w, "   ");
      else
	strcpy(w, "TRM");
      sprintf(hs + strlen(hs), " %s %6ld", w, boxspoolstatus(WITH->tcon, WITH->pchan, -1));
      isec = clock_.ixtime - WITH->lastcmdtime;
      if (isec > 5999)
	isec = 5999;
      sprintf(hs + strlen(hs), " %.2ld:%.2ld", isec / 60, isec % 60);
      
      isec = cpu_usage(x);
      sprintf(w, "%ld.%.2ld", isec / 200, (isec % 200) >> 1);
      lspacing(w, 7);
      strcat(hs, w);
      
      if (WITH->smode) {
        strcpy(w, "(files)");
      } else {
        strcpy(w, WITH->brett);
      }
      rspacing(w, 8);
      sprintf(hs + strlen(hs), " %s %s", w, WITH->lastcmd);
      cut(hs, 79);
      outputproc(unr, hs);
    }
  }
}


void show_boxactivity(short unr)
{
  calc_boxactivity(unr, wlnuser);
}


void yapp_responses(short unr, char *info, long infosize)
{
  short		olda;
  userstruct	*WITH;

  if (infosize <= 0)
    return;

  if (!boxrange(unr))
    return;

  WITH			= user[unr];
  if (WITH->yapp == NULL)
    return;

  if (WITH->yapp->filefd < minhandle) {
    WITH->yapp->filefd	= sfopen(WITH->yapp->fname, FO_READ);
    sfseek(WITH->yapp->seekpos, WITH->yapp->filefd, SFSEEKSET);
  }

  WITH->yapp->touch	= clock_.ixtime;
  if (yapp_upload(false, false, WITH->yapp, info, infosize))
    return;

  sfclose(&WITH->yapp->filefd);
  if (WITH->yapp->delete_)
    sfdelfile(WITH->yapp->fname);
  free(WITH->yapp);
  WITH->yapp		= NULL;
  olda			= WITH->action;
  WITH->action		= 0;
  boxsetrwmode(WITH->pchan, RW_NORMAL);
  boxspoolfend(WITH->pchan);
  if (olda == 96) show_prompt(unr);
}

#define BUFLEN	4096

boolean read_filepart(short unr)
{
  boolean	Result, abort;
  long		len, fill;
  char		buff[BUFLEN];
  userstruct	*WITH;

  if (!boxrange(unr))
    return false;

  WITH				= user[unr];

  if (WITH->yapp != NULL) {
    abort			= false;
    if (WITH->yapp->filefd < minhandle) {
      WITH->yapp->filefd	= sfopen(WITH->yapp->fname, FO_READ);
      abort			= WITH->yapp->filefd < minhandle;
      if (!abort)
        abort 			= (sfseek(WITH->yapp->seekpos, WITH->yapp->filefd, SFSEEKSET)
                 		   != WITH->yapp->seekpos);
    }

    if (!abort) {
      WITH->yapp->touch		= clock_.ixtime;
      if (yapp_upload(false, WITH->yapp->filefd < minhandle, WITH->yapp, NULL, 0))
        return true;
    }
    
    sfclose(&WITH->yapp->filefd);
    if (WITH->yapp->delete_)
      sfdelfile(WITH->yapp->fname);
    free(WITH->yapp);
    WITH->yapp			= NULL;
    boxsetrwmode(WITH->pchan, RW_NORMAL);
    boxspoolfend(WITH->pchan);
    if (WITH->action == 96) show_prompt(unr);
    return false;
  }

  if (WITH->bin == NULL)
    return false;

  Result			= false;

  if (WITH->bin->posi < WITH->bin->total) {
    if (WITH->bin->filefd < minhandle)
      WITH->bin->filefd		= sfopen(WITH->bin->fname, FO_READ);

    if (WITH->bin->filefd < minhandle) {
      wlnuser0(unr);
      wlnuser(unr, "#ABORT#");
      wlnuser0(unr);
      wln_btext(unr, 106);
      WITH->bin->posi		= WITH->bin->total;
    } else {
      if (sfseek(WITH->bin->posi, WITH->bin->filefd, SFSEEKSET) != WITH->bin->posi) {
	if (!WITH->bin->ascii) {
	  wlnuser0(unr);
	  wlnuser(unr, "#ABORT#");
	}
	wlnuser0(unr);
	wln_btext(unr, 149);
	WITH->bin->posi		= WITH->bin->total;
      } else {
	fill			= 0;
	do {
	  len			= sfread(WITH->bin->filefd, BUFLEN, buff);
	  if (len > 0) {
	    WITH->bin->touch	= clock_.ixtime;
            upd_statistik(unr, 0, len, 0, 0);
	    boxmemspool(WITH->tcon, WITH->pchan, -1, WITH->bin->ascii, buff, len);
	    WITH->bin->posi	+= len;
	    fill		+= len;
	    Result		= true;
	  } else if (len < 0 || WITH->bin->posi < WITH->bin->total) {
	    if (!WITH->bin->ascii) {
	      wlnuser0(unr);
	      wlnuser(unr, "#ABORT#");
	    }
	    wlnuser0(unr);
	    wln_btext(unr, 149);
	    WITH->bin->posi	= WITH->bin->total;
	  }
	} while (WITH->bin->posi < WITH->bin->total && fill <= WITH->bin->maxfill);
      }
    }
  }

  if (WITH->bin->posi < WITH->bin->total)
    return Result;

  sfclose(&WITH->bin->filefd);
  if (WITH->bin->delete_) sfdelfile(WITH->bin->fname);
  free(WITH->bin);
  WITH->bin			= NULL;
  boxspoolfend(WITH->pchan);
  if (WITH->action == 96) show_prompt(unr);
  return false;

}

#undef BUFLEN


void abort_fileread(short unr)
{
  userstruct	*WITH;

  if (!boxrange(unr))
    return;

  WITH		= user[unr];
  if (WITH->yapp != NULL) {
    yapp_upload(false, true, WITH->yapp, NULL, 0);
    sfclose(&WITH->yapp->filefd);
    if (WITH->yapp->delete_)
      sfdelfile(WITH->yapp->fname);
    free(WITH->yapp);
    WITH->yapp	= NULL;
    boxsetrwmode(WITH->pchan, RW_NORMAL);
  }

  if (WITH->bin != NULL) {
    sfclose(&WITH->bin->filefd);
    if (WITH->bin->delete_ || WITH->bin->write_)
      sfdelfile(WITH->bin->fname);
    free(WITH->bin);
    WITH->bin	= NULL;
  }

  boxspoolfend(WITH->pchan);
  if (WITH->action == 96)
    show_prompt(unr);
}


static void yappprotokoll(const short unr, const char *s)
{
  char		STR1[256];

  sprintf(STR1, "YAPP: %s", s);
  boxprotokoll(STR1);
}


static void yappbufout(short unr, char *base, long size)
{
  upd_statistik(unr, 0, size, 0, 0);
  boxmemspool(user[unr]->tcon, user[unr]->pchan, -1, false, base, size);
}


#define psize_          16384
#define ymaxfill        20000

boolean read_file(boolean delete_afterwards, short unr, char *eingabe_)
{
  unsigned short	crc, date, time;
  short			k;
  boolean		ascii, yapp_, pack, transparent, gzip, on_linestart;
  long			size, tail, fsize, start;
  userstruct		*WITH;
  char			eingabe[256], hs[256], w[256], fname[256], pname[256], dname[256];
  char			STR1[256], STR7[256];

  debug(2, unr, 102, eingabe_);

  if (!boxrange(unr))
    return false;

  strcpy(eingabe, eingabe_);

  abort_fileread(unr);

  if (insecure(eingabe))
    return false;

  sprintf(eingabe, "%s%s", boxdir, strcpy(STR1, eingabe));
  get_word(eingabe, fname);
  strcpy(pname, fname);
  strcpy(dname, fname);
  k		= strlen(boxdir) + 1;
  strsub(dname, fname, k, strlen(fname) - k + 1);

  upper(eingabe);
  transparent	= (strstr(eingabe, "-F") != NULL);
  ascii		= (strstr(eingabe, "-A") != NULL);
  pack		= (!ascii && strstr(eingabe, "-P") != NULL);
  gzip		= (!ascii && strstr(eingabe, "-Z") != NULL);
  if (gzip) pack = true;
  yapp_		= (!ascii && strstr(eingabe, "-Y") != NULL);
  tail		= 0;
  on_linestart	= false;

  if (!yapp_) {
    k		= strpos2(eingabe, "-TN", 1);
    if (k == 0)
      k		= strpos2(eingabe, "-T", 1);
    else
      on_linestart = true;

    if (k > 0 && !pack) {
      if (on_linestart)
	strdelete(eingabe, 1, k + 2);
      else
	strdelete(eingabe, 1, k + 1);
      get_word(eingabe, w);
      tail	= atol(w);
      if (tail < 0)
	tail	= 0;
    }
  }

  fsize		= sfsize(fname);
  if (fsize <= 0) {
    wln_btext(unr, 4);
    return false;
  }
  size		= fsize;

  if (pack) {
    sprintf(pname, "%sTDHPF%c001", tempdir, extsep);
    validate(pname);
    pack	= (packer(gzip, true, true, fname, pname, false) >= 0);
    if (!pack) {
      sfdelfile(pname);
      strcpy(pname, fname);
    }
    fsize	= sfsize(pname);
    size	= fsize;
  }

  start		= 0;
  if (tail > 0) {
    if (tail > size)
      tail	= size;
    start	= size - tail;
    size	= tail;
  }

  if (!ascii && !transparent && !yapp_) {
    wlnuser0(unr);
    if (pack && !gzip)
      wlnuser(unr, TXTDPLZHBIN);
    crc		= file_crc(2, pname, 0, start, size);
    sfgetdatime(fname, &date, &time);
    int2hstr(date, w);
    while (strlen(w) < 4)
      sprintf(w, "0%s", strcpy(STR1, w));
    int2hstr(time, STR7);
    while (strlen(STR7) < 4)
      sprintf(STR7, "0%s", strcpy(STR1, STR7));
    del_path(dname);
    if (gzip) strcat(dname, ".gz");
    sprintf(hs, "#BIN#%ld#|%d#$%s%s#%s", size, crc, w, STR7, dname);
    wlnuser(unr, hs);
  } else if (!transparent)
    wlnuser0(unr);

  k	= sfopen(pname, FO_READ);
  if (k >= minhandle) {
    if (ascii && on_linestart) {
      sfseek(start, k, SFSEEKSET);
      file2str(k, hs);
      tail	= sfseek(0, k, SFSEEKCUR);
      start	= tail;
    }

    if (yapp_) {

      user[unr]->yapp		= malloc(sizeof(yapptype));
      if (user[unr]->yapp == NULL)
	return false;

      WITH    	      	      	= user[unr];
      WITH->yapp->state		= 0;
      WITH->yapp->filefd	= k;
      WITH->yapp->delete_	= pack || delete_afterwards;
      WITH->yapp->write_	= false;
      WITH->yapp->yappc		= 0;
      WITH->yapp->total		= 0;
      WITH->yapp->filelength	= fsize;
      WITH->yapp->seekpos	= 0;
      WITH->yapp->touch		= clock_.ixtime;
      WITH->yapp->maxfill	= ymaxfill;
      WITH->yapp->unr		= unr;
      WITH->yapp->chout		= chwuser;
      WITH->yapp->lineout	= wuser;
      WITH->yapp->buffout	= yappbufout;
      WITH->yapp->statout	= yappprotokoll;
      WITH->yapp->outlen	= 0;
      WITH->yapp->outbufptr	= 0;
      WITH->yapp->buflen	= 0;
      WITH->yapp->fdate		= 0;
      WITH->yapp->ftime		= 0;
      strcpy(WITH->yapp->fname, pname);
      sprintf(WITH->yapp->yappdir, "%s%sincoming%c", boxdir, fservdir, dirsep);
      boxsetrwmode(WITH->pchan, RW_RAW);
      wln_btext(unr, 187);
      return (yapp_upload(true, false, WITH->yapp, NULL, 0));
    }

    user[unr]->bin		= malloc(sizeof(bintype));
    if (user[unr]->bin != NULL) {
      WITH			= user[unr];
      WITH->bin->filefd		= k;
      WITH->bin->write_		= false;
      WITH->bin->delete_	= pack || delete_afterwards;
      WITH->bin->ascii		= ascii;
      WITH->bin->maxfill	= ymaxfill;
      WITH->bin->posi		= start;
      WITH->bin->total		= fsize;
      WITH->bin->touch		= clock_.ixtime;
      strcpy(WITH->bin->fname, pname);
      return true;
    }
    wln_btext(unr, 106);
    sfclose(&k);
    if (pack)
      sfdelfile(pname);
    return false;
  }

  wln_btext(unr, 4);
  if (pack)
    sfdelfile(pname);
  return false;
}

#undef psize_
#undef ymaxfill


void read_file_immediately(boolean delete_afterwards, short unr, char *eingabe)
{
  if (read_file(delete_afterwards, unr, eingabe)) {
    do {
    } while (read_filepart(unr));
  }
}


void write_file2(short unr, char *eingabe_)
{
  userstruct	*WITH;
  char		eingabe[256], hs[256];

  strcpy(eingabe, eingabe_);
  debug(2, unr, 124, eingabe);
  if (!boxrange(unr))
    return;

  WITH			= user[unr];
  if (true_bin(eingabe) >= 0) {
    WITH->errors	= -5;
    boxprwprg2(WITH->pchan, eingabe);
    *hs			= '\0';
    box_rtitle(true, unr, hs);
    if (WITH->action == 88)
      WITH->action	= 89;
    return;
  }

  if (!strcmp(eingabe, "SHOWPROMPT")) {
    WITH->action	= 0;
    WITH->errors	= -5;
    return;
  }

  if (strstr(eingabe, "#ABORT#") != NULL) {
    WITH->errors	= 0;
    boxsetrwmode(WITH->pchan, RW_NORMAL);
    wln_btext(unr, 1);
    WITH->action	= 0;
    return;
  }

  WITH->errors++;
  if (WITH->errors < 0)
    return;

  boxsetrwmode(WITH->pchan, RW_NORMAL);
  wln_btext(unr, 1);
  WITH->action		= 0;
}


void yapp_input(short unr, char *info, long infosize)
{
  userstruct	*WITH;

  if (infosize <= 0)
    return;

  if (!boxrange(unr))
    return;

  WITH		= user[unr];
  if (WITH->yapp == NULL)
    return;

  if (WITH->yapp->filefd < minhandle && WITH->action == 98) {
    WITH->yapp->filefd = sfopen(WITH->yapp->fname, FO_READ);
    sfseek(0, WITH->yapp->filefd, SFSEEKEND);
  }

  WITH->yapp->touch = clock_.ixtime;
  if (yapp_download(WITH->action == 97, false, WITH->yapp, info, infosize)) {
    if (WITH->action == 97)
      WITH->action = 98;
    return;
  }

  sfclose(&WITH->yapp->filefd);
  if (WITH->yapp->delete_)
    sfdelfile(WITH->yapp->fname);
  free(WITH->yapp);
  WITH->yapp	= NULL;
  WITH->action	= 0;
  boxsetrwmode(WITH->pchan, RW_NORMAL);
  boxspoolfend(WITH->pchan);
  show_prompt(unr);
}


void write_file(short unr, char *eingabe_)
{
  boolean	yapp_;
  userstruct	*WITH;
  char		eingabe[256], fname[256], STR1[256];

  debug(2, unr, 103, eingabe_);

  if (!boxrange(unr))
    return;

  strcpy(eingabe, eingabe_);

  WITH	= user[unr];
  if (WITH->pchan <= 0)
    return;

  get_word(eingabe, fname);
  if (*fname == '\0')
    return;

  sprintf(fname, "%s%s", boxdir, strcpy(STR1, fname));
  if (insecure(fname))
    return;

  upper(eingabe);
  yapp_		= (strstr(eingabe, "-Y") != NULL);

  if (yapp_) {
    WITH->yapp			= malloc(sizeof(yapptype));
    if (WITH->yapp == NULL)
      return;

    WITH->yapp->state		= 0;
    WITH->yapp->filefd		= nohandle;
    WITH->yapp->delete_		= false;
    WITH->yapp->write_		= true;
    WITH->yapp->yappc		= 0;
    WITH->yapp->total		= 0;
    WITH->yapp->filelength	= 0;
    WITH->yapp->seekpos		= 0;
    WITH->yapp->touch		= clock_.ixtime;
    WITH->yapp->maxfill		= 20000;
    WITH->yapp->unr		= unr;
    WITH->yapp->chout		= chwuser;
    WITH->yapp->lineout		= wuser;
    WITH->yapp->buffout		= yappbufout;
    WITH->yapp->statout 	= yappprotokoll;
    WITH->yapp->outlen		= 0;
    WITH->yapp->outbufptr	= 0;
    WITH->yapp->buflen		= 0;
    WITH->yapp->fdate		= 0;
    WITH->yapp->ftime		= 0;
    strcpy(WITH->yapp->fname, fname);
    sprintf(WITH->yapp->yappdir, "%s%sincoming%c", boxdir, fservdir, dirsep);
    wln_btext(unr, 182);
    boxspoolread();
    boxsetrwmode(WITH->pchan, RW_RAW);
    WITH->action		= 97;
    return;
  }

  wln_btext(unr, 146);
  boxspoolread();
  WITH->action			= 87;
  WITH->errors			= -5;
  boxsetinpfile(WITH->pchan, fname);
  boxsetrwmode(WITH->pchan, RW_ABIN_WAIT);
}


void update_dp2(short unr)
{
  if (exist(dp_new_prg)) {
    sfdelfile(dp_prg);
    if (sfrename(dp_new_prg, dp_prg) == 0)
      wlnuser(unr, "UPDATE completed");
    else
      wlnuser(unr, "error while renaming");
  } else
    wlnuser(unr, "update file not found");
  user[unr]->action	= 87;
}


void update_dp(short unr, char *eingabe)
{
  if (!boxrange(unr))
    return;
  if (user[unr]->pchan <= 0)
    return;
  wlnuser(unr, "preparing upload of DP.PRG");
  write_file(unr, dp_new_prg);
  user[unr]->action	= 88;
}


void show_file_dir(short unr, char *eingabe_)
{
  boolean	err;
  short		x;
  char		eingabe[256], fn[256], cmd[256];

  err		= insecure(eingabe_);
  sprintf(eingabe, "%s%s", boxdir, eingabe_);

  get_word(eingabe, cmd);
  if (err) {
    wln_btext(unr, 2);
    return;
  }

  if (cmd[strlen(cmd) - 1] == dirsep)
    sprintf(cmd + strlen(cmd), "%c%c%c", allquant, extsep, allquant);

  x	= strlen(boxdir) + 1;
  strsub(fn, cmd, x, strlen(cmd) - x + 1);

  wuser(unr, "Content of ");
  wuser(unr, fn);
  wlnuser(unr, " :");
  wlnuser0(unr);
  wlnuser(unr, "    Filename                      Size  Date      Time");
  wlnuser0(unr);
  file_dir(user[unr]->supervisor, cmd, "-L", unr, wlnuser);
}


void calc_pwdb(char *call_, char *code, char *pw)
{
  short		spalte, min, k, zeile, dat;
  long		fsize;
  calltype	call;
  char		d[256], t[256], fname[256];

  if (!(strlen(code) == 10 && zahl(code)))
    return;

  strcpylower(call, call_);
  *pw		= '\0';
  sprintf(d, "%.2s", code);
  strsub(t, code, 7, 4);
  dat		= atoi(d);
  sprintf(d, "%.2s", t);
  strdelete(t, 1, 2);
  spalte	= atoi(d);
  min		= atoi(t);
  zeile		= min + dat;
  if (zeile >= 60)
    zeile	-= 60;

  sprintf(fname, "%s%s%cpwd", boxsfdir, call, extsep);
  fsize		= sfsize(fname);
  if (fsize < 1620)
    return;

  k		= sfopen(fname, FO_READ);
  if (k < minhandle)
    return;

  if (fsize == 1620)   /* ohne CR */
    sfseek(zeile * 27 + spalte, k, SFSEEKSET);
  else  /* mit CR */
    sfseek(zeile * 29 + spalte, k, SFSEEKSET);
  /* Achtung Linux: Offset = 28 */
  sfread(k, 4, pw);
  pw[4]		= '\0';
  sfclose(&k);
}

void gimme_five_pw_numbers(char *pw, char *numbers)
{
  short		x, y, z, d, ct, dubarr[5];
  char		dubcharr[5], c;
  boolean	dub, dub1;

  *numbers	= '\0';
  y		= strlen(pw);
  if (y < 5) return;
  for (x = 0; x <= 4; x++) {
    ct		= 0;
    do {
      dub	= false;
      dub1	= false;
      z		= dp_randomize(1, y);
      c		= pw[z - 1];
      if (c == ' ') dub = true;
      else {
        for (d = 0; d < x; d++) {
	  if (dubarr[d]   == z) dub1	= true;
          if (dubcharr[d] == c) dub	= true;
        }
      }
    } while (dub1 || (ct++ < 50 && dub));
    dubarr[x]	= z;
    dubcharr[x]	= c;
    sprintf(numbers + strlen(numbers), "%d ", z);
  }
  numbers[strlen(numbers) - 1] = '\0';
}

void calc_pwtn(char *call, char *code_, char *pw)
{
  short		x, y;
  char		code[256], pws[256], w[256];

  strcpy(code, code_);
  *pw	= '\0';
  if (count_words(code) != 5)
    return;

  *pws	= '\0';
  x	= actual_user(call);
  if (x > 0)
    strcpy(pws, user[x]->password);
  if (*pws == '\0')
    return;

  for (x = 1; x <= 5; x++) {
    get_word(code, w);
    y	= atoi(w);
    if (y > 0 && y <= strlen(pws))
      sprintf(pw + strlen(pw), "%c", pws[y - 1]);
  }
}


void jitter_pwtn(char *pw)
{
  short		x;
  char		c;
  char		hs[256];

  if (*pw == '\0')
    return;

  *hs	= '\0';
  for (x = 1; x <= 53; x++) {
    do {
      c	= dp_randomize(48, 122);
    } while (!isalnum(c));
    sprintf(hs + strlen(hs), "%c", c);

  }
  x	= dp_randomize(1, 52);
  strinsert(pw, hs, x);
  strcpy(pw, hs);
}


void generate_dbpwfile(short unr, char *fname_, char *options)
{
  short		k, x;
  char		c;
  boolean	fwd;
  char		fname[256];
  char		fcall[256];
  char		hs[256];
  char		STR1[256];
  char		STR7[256];

  strcpy(fname, fname_);
  fwd		= false;
  if (callsign(fname)) {
    lower(fname);
    strcpy(fcall, fname);
    fwd		= (strchr(options, '+') != NULL);
    if (fwd) {
      strcpy(hs, Console_call);
      lower(hs);
      sprintf(fname, "%s%s%cpwd", boxdir, hs, extsep);
    } else
      sprintf(fname, "%s%s%cpwd", boxsfdir, strcpy(STR1, fname), extsep);
  } else {
    *fcall	= '\0';

    if (insecure(fname))
      return;

    sprintf(fname, "%s%s", boxdir, strcpy(STR1, fname));
  }

  k		= sfcreate(fname, FC_FILE);
  if (k < minhandle) {
    wln_btext(unr, 98);
    return;
  }

  for (x = 1; x <= 1620; x++) {
    do {
      c		= dp_randomize(48, 122);
    } while (!isalnum(c));
    sfwrite(k, 1, &c);
  }
  sfclose(&k);

  if (*fcall != '\0') {
    if (fwd) {
      sprintf(STR7, "%s.PWD", Console_call);
      sprintf(STR1, "%s", fname);
      send_sysmsg(fcall, "", STR7, STR1, 3, 'P', 2);
      sprintf(STR1, "%s%s%cpwd", boxdir, fcall, extsep);
      filemove(fname, STR1);
    }
  }

  wlnuser(unr, "OK");
}



static void idis(boolean ident, short unr)
{
  char		hs[256];

  if (!ident)
    return;

  *hs	= '\0';
  lspacing(hs, 15);
  if (unr > 0)
    wuser(unr, hs);
}


static void op(short unr, short *oh, char *s)
{
  if (unr > 0)
    wlnuser(unr, s);
  else if (*oh >= minhandle)
    str2file(oh, s, true);
}


typedef struct stt {
  struct stt		*next;
  calltype		call;
  unsigned short	uh, bh, u;
  unsigned short	b, s, bin;
  unsigned short	d;
  long			size;
} stt;


void create_sfstat(short unr, char *eingabe_)
{
  short		k, lv, x, list, oh, bf;
  long		tli, ttuh, ttbh, ttu, ttb, tts, ttsize, rtu, rtb, rts, urtsize;
  long		uttsize, utli, tu, urtsize2, uttsize2, utli2, tbn, td, rtsize;
  boolean	ident, ttofl, readjust, is_superv;
  stt		*st, *hp, *hp1;
  sfdeftype	*sfptr;
  indexstruct	*hpointer, header;
  userstruct	rec;
  char		w[256], w1[256], bn[256], hs[256], STR1[256], eingabe[256];

  if (boxrange(unr)) { /* do not recompile the fwd-list if called by users */
    if (user[unr]->supervisor)
      recompile_fwd();
  } else recompile_fwd();

  strcpyupper(eingabe, eingabe_);
  st		= NULL;
  sfptr		= sfdefs;
  oh		= nohandle;
  ttofl		= false;
  ident		= false;
  is_superv   	= boxrange(unr) && user[unr]->supervisor;

  readjust = (!strcmp(eingabe, "-") && (unr < 0 || is_superv));

  if (!readjust) {
    while (sfptr != NULL) {
      if (sfptr->usersf && !is_superv) {
      	sfptr = sfptr->next;
	continue;
      }
      hp1	= malloc(sizeof(stt));
      if (hp1 != NULL) {
	strcpy(hp1->call, sfptr->call);

	hp1->next	= NULL;
	hp1->uh		= 0;
	hp1->bh		= 0;
	hp1->u		= 0;
	hp1->b		= 0;
	hp1->s		= 0;
	hp1->bin	= 0;
	hp1->d		= 0;
	hp1->size	= 0;
      }
      if (st == NULL)
	st		= hp1;
      else {
	hp		= st;
	while (hp->next != NULL)
	  hp		= hp->next;
	if (hp != NULL)
	  hp->next	= hp1;
      }
      sfptr		= sfptr->next;
    }

    ident = (strchr(eingabe, '+') == eingabe || strchr(eingabe, '!') == eingabe);

    sprintf(bn, "%sbcstat%cbox", boxstatdir, extsep);

    if (strstr(eingabe, "RESET") != eingabe && unr > 0) {
      if (st != NULL) {
	/*ausstehende Mails zu den registrierten Partnern anzeigen*/

	list		= nohandle;
	lv		= sfsize(sflist) / sizeof(indexstruct);
	if (lv > 0) {
	  hpointer	= NULL;
	  list		= open_index("X", FO_READ, true, true);
	  hpointer	= &header;   /* bleibt ja gleich */

	  for (k = 1; k <= lv; k++) {
	    if (k % 20 == 0)
	      dp_watchdog(2, 4711);

	    read_index(list, k, &header);

	    if (check_hcs(*hpointer)) {
	      hp	= st;
	      while (strcmp(hp->call, hpointer->dest) && hp->next != NULL)
		hp	= hp->next;

	      if (!strcmp(hp->call, hpointer->dest)) {
		if (hpointer->deleted)
		  hp->d++;
		else {
		  if ((hpointer->pmode & TBIN) != 0)
		    hp->bin++;
		  if ((hpointer->msgflags & (MSG_LOCALHOLD | MSG_SFHOLD)) != 0) {
		    if ((hpointer->pmode & 1) != 0)
		      hp->uh++;
		    if ((hpointer->pmode & 2) != 0)
		      hp->bh++;
		  } else {
		    if ((hpointer->pmode & 1) != 0)
		      hp->u++;
		    if ((hpointer->pmode & 2) != 0)
		      hp->b++;
		  }

		  if ((hpointer->pmode & 4) != 0 ||
		      (hpointer->pmode & 16) != 0)
		    hp->s++;
		  else
		    hp->size += hpointer->size;
		}
	      }
	    }
	  }
	}

	close_index(&list);

	ttuh		= 0;
	ttbh		= 0;
	ttu		= 0;
	ttb		= 0;
	tts		= 0;
	tbn		= 0;
	td		= 0;
	ttsize		= 0;
	idis(ident, unr);
	idis(ident, unr);
	wlnuser(unr, "waiting msgs in s&f:");
	wlnuser0(unr);
	idis(ident, unr);
	wlnuser(unr, "     P = Usermails  B = Bulletins  S = System-Mails");
	wlnuser0(unr);
	idis(ident, unr);
	wlnuser(unr, "Call       P     B     S   BIN   P(H)  B(H)    D     Size");
	idis(ident, unr);
	wlnuser(unr, "---------------------------------------------------------");

	hp		= st;
	while (hp != NULL) {
	  ttuh		+= hp->uh;
	  ttbh		+= hp->bh;
	  ttu		+= hp->u;
	  ttb		+= hp->b;
	  tts		+= hp->s;
	  td		+= hp->d;
	  tbn		+= hp->bin;
	  ttsize	+= hp->size;
          sprintf(hs, "%-*s%6d%6d%6d%6d%6d%6d%6d%9ld", LEN_CALL,
			hp->call, hp->u, hp->b, hp->s, hp->bin, hp->uh, hp->bh, hp->d, hp->size);
	  idis(ident, unr);
	  wlnuser(unr, hs);
	  hp		= hp->next;
	}
	idis(ident, unr);
	wlnuser(unr, "---------------------------------------------------------");

        sprintf(hs, "%-*s%6ld%6ld%6ld%6ld%6ld%6ld%6ld%9ld", LEN_CALL,
			"Total", ttu, ttb, tts, tbn, ttuh, ttbh, td, ttsize);
	idis(ident, unr);
	wlnuser(unr, hs);
      }

    }

    /* ---------------------------------------------------------------- */
  }


  get_word(eingabe, w1);

  if (readjust || (!strcmp(w1, "RESET") && (unr < 0 || (boxrange(unr) && user[unr]->supervisor)))) {
    get_word(eingabe, w);
    if (!strcmp(w, "ALL"))
      sfdelfile(bn);
    if (readjust || !strcmp(w, "ALL") || callsign(w)) {
      lv	= sfsize(userinfos) / sizeof(indexstruct);
      if (lv > 0) {
	list	= open_index("M", FO_RW, true, true);
	if (list >= minhandle) {
	  for (k = 1; k <= lv; k++) {
	    read_index(list, k, &header);
	    if (check_hcs(header)) {
	      if (!header.deleted && header.size != 0) {
		convert_ufil(true, header, &rec);
		if (*rec.call != '\0') {
		  if (readjust) {
		    if (in_real_sf(rec.call)) {
		      rec.dstat_rx_p		= rec.sfstat_rx_p;
		      rec.dstat_rx_b		= rec.sfstat_rx_b;
		      rec.dstat_rx_s		= rec.sfstat_rx_s;
		      rec.dstat_rx_bytes	= rec.srbytes;
		      rec.dstat_tx_p		= rec.sfstat_tx_p;
		      rec.dstat_tx_b		= rec.sfstat_tx_b;
		      rec.dstat_tx_s		= rec.sfstat_tx_s;
		      rec.dstat_tx_bytes	= rec.ssbytes;
		      rec.dlogins		= rec.logins;
		      code_ufil(&rec, &header);
		      write_index(list, k, &header);
		      for (x = 1; x <= MAXUSER; x++) {
			if (user[x] != NULL) {
			  if (!strcmp(user[x]->call, rec.call)) {
			    user[x]->dstat_rx_p	= rec.dstat_rx_p;
			    user[x]->dstat_rx_b	= rec.dstat_rx_b;
			    user[x]->dstat_rx_s	= rec.dstat_rx_s;
			    user[x]->dstat_rx_bytes = rec.dstat_rx_bytes;
			    user[x]->dstat_tx_p	= rec.dstat_tx_p;
			    user[x]->dstat_tx_b	= rec.dstat_tx_b;
			    user[x]->dstat_tx_s	= rec.dstat_tx_s;
			    user[x]->dstat_tx_bytes = rec.dstat_tx_bytes;
			    user[x]->dlogins	= rec.dlogins;
			  }
			}
		      }
		    }
		  } else if (!strcmp(w, "ALL") || !strcmp(rec.call, w)) {
		    rec.sfstat_tx_p	= 0;
		    rec.sfstat_tx_b	= 0;
		    rec.sfstat_tx_s	= 0;
		    rec.ssbytes		= 0;
		    rec.rbytes		= 0;
		    rec.sbytes		= 0;
		    rec.fstat_rx_p	= 0;
		    rec.fstat_rx_b	= 0;
		    rec.fstat_rx_s	= 0;
		    rec.fstat_tx_p	= 0;
		    rec.fstat_tx_b	= 0;
		    rec.fstat_tx_s	= 0;
		    rec.sfstat_rx_p	= 0;
		    rec.sfstat_rx_b	= 0;
		    rec.sfstat_rx_s	= 0;
		    rec.srbytes		= 0;
		    rec.logins		= 0;
		    rec.dstat_rx_p	= 0;
		    rec.dstat_rx_b	= 0;
		    rec.dstat_rx_s	= 0;
		    rec.dstat_rx_bytes	= 0;
		    rec.dstat_tx_p	= 0;
		    rec.dstat_tx_b	= 0;
		    rec.dstat_tx_s	= 0;
		    rec.dstat_tx_bytes	= 0;
		    rec.dlogins		= 0;

		    code_ufil(&rec, &header);
		    write_index(list, k, &header);
		    for (x = 1; x <= MAXUSER; x++) {
		      if (user[x] != NULL) {
			if (!strcmp(user[x]->call, rec.call)) {
			  user[x]->sfstat_tx_p	= 0;
			  user[x]->sfstat_tx_b	= 0;
			  user[x]->sfstat_tx_s	= 0;
			  user[x]->ssbytes	= 0;
			  user[x]->rbytes	= 0;
			  user[x]->sbytes	= 0;
			  user[x]->fstat_rx_p	= 0;
			  user[x]->fstat_rx_b	= 0;
			  user[x]->fstat_rx_s	= 0;
			  user[x]->fstat_tx_p	= 0;
			  user[x]->fstat_tx_b	= 0;
			  user[x]->fstat_tx_s	= 0;
			  user[x]->sfstat_rx_p	= 0;
			  user[x]->sfstat_rx_b	= 0;
			  user[x]->sfstat_rx_s	= 0;
			  user[x]->srbytes	= 0;
			  user[x]->logins	= 0;
			  user[x]->dstat_rx_p	= 0;
			  user[x]->dstat_rx_b	= 0;
			  user[x]->dstat_rx_s	= 0;
			  user[x]->dstat_rx_bytes = 0;
			  user[x]->dstat_tx_p	= 0;
			  user[x]->dstat_tx_b	= 0;
			  user[x]->dstat_tx_s	= 0;
			  user[x]->dstat_tx_bytes = 0;
			  user[x]->dlogins	= 0;
			}
		      }
		    }
		  }
		}
	      }
	    }
	  }
	  close_index(&list);
	  init_ufcache();
	}
      }
    }
  } else if (!strcmp(w1, "+") || !strcmp(w1, "!")) {
    for (x = 1; x <= MAXUSER; x++) {
      if (user[x] != NULL && user[x]->f_bbs)
	save_userfile(user[x]);
    }

    ttu		= 0;
    ttb		= 0;
    tts		= 0;
    ttsize	= 0;
    rtu		= 0;
    rtb		= 0;
    rts		= 0;
    rtsize	= 0;
    tli		= 0;
    urtsize	= 0;
    uttsize	= 0;
    utli	= 0;
    tu		= 0;
    urtsize2	= 0;
    uttsize2	= 0;
    utli2	= 0;

    if (unr > 0) {
      wlnuser0(unr);
      wlnuser0(unr);
      wlnuser0(unr);
      idis(ident, unr);
      wlnuser(unr, "       total of exchanged files since 00:00 UTC:");
      op(unr, &oh, "");
      op(unr, &oh, "        -------- received from ---------   -------- transmitted to -------");
      op(unr, &oh, "Call         P       B       S      Size       P       B       S      Size Conn");
      op(unr, &oh, "-------------------------------------------------------------------------------");

      list	= nohandle;
      lv	= sfsize(userinfos) / sizeof(indexstruct);

      if (lv > 0) {
	hpointer	= NULL;
	list		= open_index("M", FO_READ, true, true);
	hpointer	= &header;   /* bleibt ja gleich */

	if (list >= minhandle) {
	  for (k = 1; k <= lv; k++) {
	    if (k % 20 == 0)
	      dp_watchdog(2, 4711);

	    read_index(list, k, &header);

	    if (check_hcs(*hpointer)) {
	      if (!hpointer->deleted && hpointer->size != 0) {
		convert_ufil(true, *hpointer, &rec);
		if (in_real_sf(rec.call)) {
		  ttu		+= rec.sfstat_tx_p - rec.dstat_tx_p;
		  ttb		+= rec.sfstat_tx_b - rec.dstat_tx_b;
		  tts		+= rec.sfstat_tx_s - rec.dstat_tx_s;
		  ttsize	+= rec.ssbytes     - rec.dstat_tx_bytes;
		  rtu		+= rec.sfstat_rx_p - rec.dstat_rx_p;
		  rtb		+= rec.sfstat_rx_b - rec.dstat_rx_b;
		  rts		+= rec.sfstat_rx_s - rec.dstat_rx_s;
		  rtsize	+= rec.srbytes     - rec.dstat_rx_bytes;
		  tli		+= rec.logins      - rec.dlogins;

		  if (rec.logins - rec.dlogins > 9999)
		    strcpy(w, " >>>>");
		  else
		    sprintf(w, "%5d", rec.logins - rec.dlogins);

		  sprintf(hs, "%-*s%8ld%8ld%8ld%10ld%8ld%8ld%8ld%10ld%s", LEN_CALL,
				rec.call,
				rec.sfstat_rx_p - rec.dstat_rx_p,
				rec.sfstat_rx_b - rec.dstat_rx_b,
				rec.sfstat_rx_s - rec.dstat_rx_s,
				rec.srbytes - rec.dstat_rx_bytes,
				rec.sfstat_tx_p - rec.dstat_tx_p,
				rec.sfstat_tx_b - rec.dstat_tx_b,
				rec.sfstat_tx_s - rec.dstat_tx_s,
				rec.ssbytes - rec.dstat_tx_bytes,
				w );
		  op(unr, &oh, hs);
		}
	      }
	    }
	  }

	} else
	  op(unr, &oh, "read error");
      }


      close_index(&list);

      op(unr, &oh, "-------------------------------------------------------------------------------");

      if (tli > 9999)
	strcpy(w, " >>>>");
      else
	sprintf(w, "%5ld", tli);

      sprintf(hs, "%-*s%8ld%8ld%8ld%10ld%8ld%8ld%8ld%10ld%s", LEN_CALL,
			"Total", rtu, rtb, rts, rtsize, ttu, ttb, tts, ttsize, w);
      op(unr, &oh, hs);

      wlnuser0(unr);
      wlnuser0(unr);
      wlnuser0(unr);
      idis(ident, unr);

      wlnuser(unr, "         total of exchanged files this month:");
    } else {
      sprintf(STR1, "%sSFSTAT", tempdir);
      oh	= sfcreate(STR1, FC_FILE);
      if (oh < minhandle)
	goto _STOP;
      str2file(&oh, "                 total of exchanged files during the last month:", true);
    }

    op(unr, &oh, "");
    if (unr <= 0) {
      op(unr, &oh, "                 P = Usermails  B = Bulletins  S = System-Mails");
      op(unr, &oh, "");
    }
    op(unr, &oh,   "        -------- received from ---------   -------- transmitted to -------");
    op(unr, &oh,   "Call         P       B       S      Size       P       B       S      Size Conn");
    op(unr, &oh,   "-------------------------------------------------------------------------------");

    ttu		= 0;
    ttb		= 0;
    tts		= 0;
    ttsize	= 0;
    rtu		= 0;
    rtb		= 0;
    rts		= 0;
    rtsize	= 0;
    tli		= 0;

    list	= nohandle;
    lv		= sfsize(userinfos) / sizeof(indexstruct);
    if (lv > 0) {
      hpointer	= NULL;
      list	= open_index("M", FO_READ, true, true);
      hpointer	= &header;   /* bleibt ja gleich */

      if (list >= minhandle) {
	for (k = 1; k <= lv; k++) {
	  if (k % 20 == 0)
	    dp_watchdog(2, 4711);

	  read_index(list, k, &header);

	  if (check_hcs(*hpointer)) {
	    if (!hpointer->deleted && hpointer->size != 0) {
	      convert_ufil(false, *hpointer, &rec);
	      if (in_real_sf(rec.call)) {
		ttu		+= rec.sfstat_tx_p;
		ttb		+= rec.sfstat_tx_b;
		tts		+= rec.sfstat_tx_s;
		ttsize		+= rec.ssbytes;
		rtu		+= rec.sfstat_rx_p;
		rtb		+= rec.sfstat_rx_b;
		rts		+= rec.sfstat_rx_s;
		rtsize		+= rec.srbytes;
		tli		+= rec.logins;
		uttsize2	+= rec.sssbytes;
		urtsize2	+= rec.ssrbytes;
		if (urtsize2 < 0 || uttsize2 < 0)
		  ttofl		= true;
		utli2		+= rec.sslogins;

		if (rec.logins > 9999)
		  strcpy(w, " >>>>");
		else
		  sprintf(w, "%5d", rec.logins);

		sprintf(hs, "%-*s%8ld%8ld%8ld%10ld%8ld%8ld%8ld%10ld%s", LEN_CALL,
				rec.call,
				rec.sfstat_rx_p,
				rec.sfstat_rx_b,
				rec.sfstat_rx_s,
				rec.srbytes,
				rec.sfstat_tx_p,
				rec.sfstat_tx_b,
				rec.sfstat_tx_s,
				rec.ssbytes,
				w );
		op(unr, &oh, hs);

	      } else {

		uttsize		+= rec.ssbytes;
		urtsize		+= rec.srbytes;
		utli		+= rec.logins;
		uttsize2	+= rec.sssbytes;
		urtsize2	+= rec.ssrbytes;
		if (urtsize2 < 0 || uttsize2 < 0)
		  ttofl		= true;
		utli2		+= rec.sslogins;
		if (rec.logins > 0)
		  tu++;
	      }
	    }
	  }
	}

      } else
	op(unr, &oh, "read error");
    }

    close_index(&list);

    op(unr, &oh, "-------------------------------------------------------------------------------");

    if (tli > 9999)
      strcpy(w, " >>>>");
    else
      sprintf(w, "%5ld", tli);

    sprintf(hs, "%-*s%8ld%8ld%8ld%10ld%8ld%8ld%8ld%10ld%s", LEN_CALL,
			"Total", rtu, rtb, rts, rtsize, ttu, ttb, tts, ttsize, w);
    op(unr, &oh, hs);
    rtu		+= rtb + rts;
    ttu		+= ttb + tts;
    ttb		= rtu + ttu;
    ttsize	+= rtsize;

    if (!strcmp(w1, "!")) {
      op(unr, &oh, "");
      op(unr, &oh, "");
      op(unr, &oh, "forward this month:");
      op(unr, &oh, "-------------------------------------");
      sprintf(STR1, "Total connects in s&f   : %11ld", tli);
      op(unr, &oh, STR1);
      sprintf(STR1, "Total received files    : %11ld", rtu);
      op(unr, &oh, STR1);
      sprintf(STR1, "Total transmitted files : %11ld", ttu);
      op(unr, &oh, STR1);
      sprintf(STR1, "Total exchanged files   : %11ld", ttb);
      op(unr, &oh, STR1);
      sprintf(STR1, "Total exchanged bytes   : %11ld", ttsize);
      op(unr, &oh, STR1);
      op(unr, &oh, "-------------------------------------");
      op(unr, &oh, "");
      op(unr, &oh, "users this month:");
      op(unr, &oh, "-------------------------------------");
      sprintf(STR1, "Total rx from users     : %11ld", urtsize);
      op(unr, &oh, STR1);
      sprintf(STR1, "Total tx to users       : %11ld", uttsize);
      op(unr, &oh, STR1);
      sprintf(STR1, "Total logins of users   : %11ld", utli);
      op(unr, &oh, STR1);
      sprintf(STR1, "Total count of users    : %11ld", tu);
      op(unr, &oh, STR1);
      op(unr, &oh, "-------------------------------------");
      op(unr, &oh, "");
      op(unr, &oh, "users and forward:");
      op(unr, &oh, "-------------------------------------");
      if (!ttofl) {
	sprintf(STR1, "Total rx since install. : %11ld", urtsize2);
	op(unr, &oh, STR1);
	sprintf(STR1, "Total tx since install. : %11ld", uttsize2);
	op(unr, &oh, STR1);
      }
      sprintf(STR1, "Total logins since ever : %11ld", utli2);
      op(unr, &oh, STR1);
      op(unr, &oh, "-------------------------------------");

      if (exist(bn)) {
	bf	= sfopen(bn, FO_READ);
	op(unr, &oh, "");
	op(unr, &oh, "broadcast this month:");
	op(unr, &oh, "-------------------------------------");
	strcpy(w, "0");
	file2str(bf, w);
	lspacing(w, 11);
	sprintf(STR1, "Total transmitted bytes : %s", w);
	op(unr, &oh, STR1);
	strcpy(w, "0");
	file2str(bf, w);
	sfclose(&bf);
	lspacing(w, 11);
	sprintf(STR1, "Total transmitted files : %s", w);
	op(unr, &oh, STR1);
	op(unr, &oh, "-------------------------------------");
      }

      if (unr < 0)
	sfclose(&oh);
    }
  }

_STOP:

  /*alle Pointer freigeben*/

  hp	= st;
  while (hp != NULL) {
    hp1	= hp;
    hp	= hp1->next;
    free(hp1);
  }

}


void generate_sfstat(short unr)
{
  char	STR1[81];
  char	STR2[256];

  create_sfstat(unr, "!");
  if (unr >= 0)
    return;

  sprintf(STR1, "monthly s&f-statistics of %s", Console_call);
  sprintf(STR2, "%sSFSTAT", tempdir);
  send_sysmsg("STATISTI", Console_call, STR1, STR2, 0, 'B', 1);
  create_sfstat(-1, "RESET ALL");
  sfdelfile(STR2);
}


void readjust_dayly_sfcount(void)
{
  create_sfstat(-1, "-");
}


void do_config(short unr, char *filename, char *eingabe_)
{
  char	eingabe[256];
  char	key[256], zeile[256], replace[256], fn[256];
  char	STR1[256], STR7[256];

  strcpy(eingabe, eingabe_);
  get_word(eingabe, key);

  if (*key == '\0') {
    wlnuser(unr, "Usage: CONFIG <Filename> <Keyword> [<Replacement>]");
    wlnuser(unr, "       <Keyword> is the first word of a line");
    wlnuser(unr, "       if <Replacement> is <->, line will be deleted");
    wlnuser(unr, "       if no <Replacement> was added nothing will be changed");
    wlnuser(unr, "       <Filename> is automatically expanded to system/ or sf/");
    return;
  }

  strcpy(fn, filename);
  strdelete(fn, 1, strlen(boxdir));
  sprintf(fn, "File: %s", strcpy(STR1, fn));
  wlnuser(unr, fn);
  wlnuser0(unr);
  upper(key);
  strcpy(replace, eingabe);
  if (get_keyline(filename, key, false, zeile)) {
    wlnuser(unr, "Key found:");
    wlnuser(unr, zeile);
  } else {
    wlnuser(unr, "Key not found");
    if (!strcmp(replace, "-")) return;
  }
  
  if (*replace == '\0')
    return;

  wlnuser0(unr);

  if (strcmp(replace, "-"))
    sprintf(replace, "%s %s", key, strcpy(STR7, replace));
  else
    *replace = '\0';

  if (!replace_keyline(filename, key, false, replace)) {
    wlnuser(unr, "now added");
    return;
  }

  if (*replace == '\0') {
    wlnuser(unr, "line deleted");
  } else {
    wlnuser(unr, "added/changed:");
    wlnuser(unr, replace);
  }
}


void do_stripcr(short unr, char *filename)
{
  char	fn[256];
  char	STR1[256];

  if (*filename == '\0') {
    wlnuser(unr, "Usage: STRIPCR <Filename>");
    wlnuser(unr, "       <Filename> is automatically expanded to system/ or sf/");
    return;
  }

  strcpy(fn, filename);

  strdelete(fn, 1, strlen(boxdir));
  sprintf(fn, "File: %s", strcpy(STR1, fn));
  wlnuser(unr, fn);
  wlnuser0(unr);
  stripcr(filename);
  wlnuser(unr, "OK, stripped <CR>");
}


void _box_sys_init(void)
{
  static int _was_initialized = 0;
  if (_was_initialized++)
    return;

  chmbxsemaphore	= false;
  lastrunnum		= 0;
}

