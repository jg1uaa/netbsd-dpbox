/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#define BOXBCM_G
#include "main.h"
#include "boxbcm.h"
#include "tools.h"
#include "sort.h"
#include "boxlocal.h"
#include "box_init.h"
#include "box_garb.h"
#include "box_send.h"
#include "box_sub.h"
#include "box_sys.h"
#include "box_sf.h"
#include "box.h"
#include "box_file.h"
#include "box_inou.h"
#include "box_mem.h"


/* ************************************************************************ */
/* * Die Konvertierroutinen fuer einen kompletten BayBox-Daten-Import     * */
/* *                                                                      * */
/* * (c) 1997 Daniel Goetz, DL3NEU                                        * */
/* * A lot of code was adapted from boxdbox.c by Joachim Schurig, DL8HBS  * */
/* ************************************************************************ */

static boardtype actboard;


void cbbtboard(char *s)
{
  /* hier holt sich new_entry() den transferierten brettnamen her */
  strcpy(s, actboard);
}


static void iwuser(short unr, char *s)
{
  char STR1[256];

  sprintf(STR1, "%sbaybox%cimp", boxprotodir, extsep);
  append(STR1, s, false);
  wuser(unr, s);
  boxspoolread();
}


static void iwlnuser(short unr, char *s)
{
  char STR1[256];

  sprintf(STR1, "%sbaybox%cimp", boxprotodir, extsep);
  append(STR1, s, true);
  wlnuser(unr, s);
  boxspoolread();
}



static boolean cverr, idel;


static void filemove2(char *n1, char *n2)
{
  if (idel)
    filemove(n1, n2);
  else
    filecopy(n1, n2);
}


static void printtime(short unr, long t)
{
  char hs[256];
  char STR7[256];

  t /= TICKSPERSEC;
  sprintf(hs, "%ld", t / 60);
  sprintf(STR7, "used time: %s minutes, ", hs);
  iwuser(unr, STR7);
  sprintf(hs, "%ld", t % 60);
  sprintf(STR7, "%s seconds.", hs);
  iwlnuser(unr, STR7);
}


static void conv_pname_rel(char *w, char *p)
{
  short x, FORLIM;
  char STR1[256];

  if (p[1] == ':')
    strdelete(p, 1, 2);
  if ((p[0] == '\\') || (p[0] == '/'))
    strdelete(p, 1, 1);
  FORLIM = strlen(p);
  for (x = 0; x < FORLIM; x++) {
    if (p[x] == '\\')
      p[x] = '/';
  }
  if (p[FORLIM - 1] != '/')
    sprintf(p, "%s/", strcpy(STR1, p));
  lower(p);
  sprintf(p, "%s%s", w, strcpy(STR1, p));
}


static boolean check_cpath(char *w_)
{
  boolean Result;
  char w[256];
  boolean ok;

  strcpy(w, w_);
  strcat(w, "init.bcm");
  ok = exist(w);
  Result = ok;
  if (!ok)
    cverr = true;
  return Result;
}

static void conv_comment(char *fn, boolean delcom)
{
  short fli, flo, x;
  char fno[256], hs[256];

  strcpy(fno, fn);
  del_path(fno);
  new_ext(fno, "tmp");
  sprintf(fno, "%s%s", tempdir, strcpy(hs, fno));
  *hs = '\0';
  fli = sfopen(fn, FO_RW);
  if (fli < minhandle)
    return;
  flo = sfcreate(fno, FC_FILE);
  if (flo < minhandle)
    return;
  while (file2str(fli, hs)) {
    x = strpos2(hs, ";", 1);
    if (x > 1)
      cut(hs, --x);    
    else if (x == 1) {
      if (delcom)
        continue;
      else
        hs[0] = '#';
    }
    str2file(&flo, hs, true);
  }
  sfclose(&flo);
  sfclose(&fli);
  filemove(fno, fn);
}


static void dop(const short unr, const char *s)
{
}


static long convert_diebox_mails(short unr, boolean userarea, char *p)
{
  char w[256], board[256];
  char boardpath[256];
  char hs[256], prot[256];
  char mn[256], w1[256];
  char mnn[256], mn3[256];
  short flh, flh1, flh2;
  char fmn[256], fmn1[256], fmn2[256];
  DTA dirinfo;
  boolean improper;
  short result, ph, mh, mnh, ct, x;
  long mct;
  char *pp;
  long ps, err, fp1, fp2;
  char rxbox[256];
  char STR1[256], STR7[256];

  mct = 0;

  sprintf(fmn, "%sfmn625702", tempdir);
  flh = sfcreate(fmn, FC_FILE);
  if (flh < minhandle) {
    cverr = true;
    return 0;
  }
  iwuser(unr, "creating filelist...");
  sprintf(STR1, "%s%c%c%c", p, allquant, extsep, allquant);
  result = sffirst(STR1, 16, &dirinfo);
  while (result == 0) {
    sprintf(STR7, "%s%s", p, dirinfo.d_fname);
    str2file(&flh, STR7, true);
    result = sfnext(&dirinfo);
  }
  sfclose(&flh);

  sprintf(fmn1, "%sfmn625702.1", tempdir);
  flh1 = sfcreate(fmn1, FC_FILE);
  flh = sfopen(fmn, FO_READ);
  if (flh1 < minhandle) {
    cverr = true;
    return 0;
  }
  while (file2str(flh, hs)) {
    sprintf(STR1, "%s%c%c%c%c", hs, dirsep, allquant, extsep, allquant);
    result = sffirst(STR1, 16, &dirinfo);
    while (result == 0) {
      sprintf(STR7, "%s%c%s", hs, dirsep, dirinfo.d_fname);
      str2file(&flh1, STR7, true);
      result = sfnext(&dirinfo);
    }
  }
  sfclose(&flh);
  sfclose(&flh1);

  sprintf(fmn2, "%sfmn625702.2", tempdir);
  flh2 = sfcreate(fmn2, FC_FILE);
  flh1 = sfopen(fmn1, FO_READ);
  if (flh2 < minhandle) {
    cverr = true;
    return 0;
  }
  while (file2str(flh1, hs)) {
    sprintf(STR1, "%s%c%c%c%c", hs, dirsep, allquant, extsep, allquant);
    result = sffirst(STR1, 16, &dirinfo);
    while (result == 0) {
      sprintf(STR7, "%s%c%s", hs, dirsep, dirinfo.d_fname);
      str2file(&flh, STR7, true);
      result = sfnext(&dirinfo);
    }
  }
  sfclose(&flh2);
  sfclose(&flh1);

  flh = sfopen(fmn, FO_WRITE);
  sfseek(0, flh, SFSEEKEND);
  app_file(fmn1, flh, true);
  app_file(fmn2, flh, true);
  sfclose(&flh);
  
  iwuser(unr, " done. sorting list...");
  sort_file(fmn, false);
  iwlnuser(unr, " done");
  flh = sfopen(fmn, FO_READ);
  if (flh >= minhandle) {
    iwlnuser(unr, "importing mails");
    while (file2str(flh, boardpath)) {
      strcpy(board, boardpath);
      del_path(board);
      strcpyupper(actboard, board);
      dp_watchdog(2, 4711);
      improper = false;
      sprintf(STR7, "importing board %s:", actboard);
      iwlnuser(unr, STR7);
      sprintf(prot, "%s%clist.bcm", boardpath, dirsep);
      if (!exist(prot)) {
        iwlnuser(unr, "Board is empty.");
        continue;
      }
      iwuser(unr, "opening list.bcm...");
      ph = sfopen(prot, FO_READ);
      if (ph >= minhandle) {
	iwlnuser(unr, " done");
	iwuser(unr, "importing mails into dpbox");
	ct = 0;
	while (file2str(ph, hs)) {
	  *rxbox = '\0';
	  dp_watchdog(2, 4711);
          if (hs[14] != '>') {
            continue;            
	  }
          cut(hs, 7);
	  del_lastblanks(hs);
	  lower(hs);
	  sprintf(mn, "%s%c%s", boardpath, dirsep, hs);
	  sprintf(mnn, "%sSENDING%c%s", newmaildir, extsep, w);
	  validate(mnn);
	  mh = sfopen(mn, FO_READ);
	  if (mh >= minhandle) {
	    mnh = sfcreate(mnn, FC_FILE);
	    if (mnh >= minhandle) {
              strcpy(w, ".");
              iwuser(unr, w);
              if (file2str(mh, hs)) {
		if (*hs == '\0')
                  file2str(mh, hs);
                  
  /* Vorsicht bei Mails, die nicht von einer BCM generiert wurden, sondern
     durch RUN-Programme usw. */

        	if (*hs != '\0') {
                  strcpy(w, hs);			/* Rxfrom */
                  x = strpos2(w, "~", 1);
                  strdelete(w, 1, x);
                  del_leadblanks(w);
                  get_word(w, rxbox);
                  cut(rxbox, 6);
                  if (!callsign(rxbox))
                    strcpy(rxbox, Console_call);
                  rspacing(rxbox, 74);
		  str2file(&mnh, rxbox, true);

                  strcpy(w, hs);			/* Sender */
                  x = strpos2(w, "<", 1);
                  strdelete(w, 1, x);
                  del_leadblanks(w);
                  get_word(w, w1);
                  cut(w1, 6);
                  if (!callsign(w1))
                    strcpy(w1, Console_call);
                  rspacing(w1, 74);
		  str2file(&mnh, w1, true);

                  str2file(&mnh, actboard, true); /* Ziel */

                  strcpy(w, hs);			/* @mbx */
                  x = strpos2(w, "@", 1);
                  strdelete(w, 1, x);
                  del_blanks(w);
                  if (w[0] == '$')
                  /*  strcpy(w1, Console_call); */
                    *w1 = '\0';
                  else
                    get_word(w, w1);
		  rspacing(w1, 72);
                  str2file(&mnh, w1, true);

                  strcpy(w, hs);			/* Lifetime */
                  x = strpos2(w, "#", 1);
                  strdelete(w, 1, x);
                  del_blanks(w);
                  get_word(w, w1);
                  cut(w1, 3);
		  str2file(&mnh, w1, true);

                  strcpy(w, hs);			/* BID */
                  x = strpos2(w, "$", 1);
                  strdelete(w, 1, x);
                  del_blanks(w);
                  get_word(w, w1);
                  cut(w1, 12);
		  str2file(&mnh, w1, true);

		  file2str(mh, hs);		/* unnoetig */
		  file2str(mh, hs);		/* unnoetig */
		  file2str(mh, hs);		/* subject */
   		  str2file(&mnh, hs, true);
                    
		  fp1 = sfseek(0, mh, SFSEEKCUR);
		  fp2 = sfseek(0, mh, SFSEEKEND);

		  sfseek(fp1, mh, SFSEEKSET);
                  file2str(mh, hs);
                  if ((strstr(hs, "R:") == hs) &&
                      (strstr(hs, Console_call) != NULL)) {
                    fp1 = sfseek(0, mh, SFSEEKCUR);
                    fp2 = sfseek(0, mh, SFSEEKEND);
                  }
                      
		  ps = fp2 - fp1 + 1;

		  sfseek(fp1, mh, SFSEEKSET);
                  pp = malloc(ps);
		  if (pp != NULL) {
		    err = sfread(mh, ps, pp);
		    if (sfwrite(mnh, err, pp) != err)
                      cverr = true;
		    free(pp);
		  } else
		    cverr = true;

        	}

   	      }
              sfclose(&mnh);
	    }
	    sfclose(&mh);
	    stripcr(mnn);

	    if (callsign(rxbox) && strcmp(rxbox, Console_call)) {
              sprintf(mn3, "%s&%s%c1", newmaildir, rxbox, extsep);
              validate(mn3);
              sfrename(mnn, mn3);
            } else
              strcpy(mn3, mnn);

            /* Diese Zeile bitte stehenlassen, sonst schmiert die Box beim */
            /* Import aus irgendeinem mir nicht bekannten Grund ab. */
            sprintf(w, "%d", ct);

            /* jetzt einsortieren in dpbox! */
	    sort_new_mail(-97, mn3, Console_call);
	    ct++;

	    sfdelfile(mn3);   /* falls das noch stehenblieb... */
          } else
	    improper = true;
	  if (idel)
	    sfdelfile(mn);
        }
	sfclose(&ph);
	mct += ct;
	sprintf(w, "%d", ct);
	sprintf(STR7, " done. %s mails imported.", w);
	iwlnuser(unr, STR7);
	if (improper)
	  iwlnuser(unr,
	    "list.bcm contained some inconsistencies. doesnt matters.");
	if (idel)
	  sfdelfile(prot);
      } else
	iwlnuser(unr, " failed!");
      if (idel) {
	sprintf(STR7, "%s%c%c%c%c", boardpath, dirsep, allquant, extsep,
                 allquant);
	file_delete(0, STR7, dop);
      }
    }
    iwlnuser(unr, "Import is done");
    sfclose(&flh);
  }
  sfdelfile(fmn);
  return mct;
}


/* BID - Import ist noch nicht fertig, aber eigentlich auch nicht wichtig */


static void flush_sfw(short *oh, char *hs, short *lasttyp)
{
  char STR1[256];

  if (*hs != '\0') {
    switch (*lasttyp) {

    case 0:
      /* blank case */
      break;

    case 1:
      sprintf(hs, "FOR %s", strcpy(STR1, hs));
      break;

    case 2:
      sprintf(hs, "NOT %s", strcpy(STR1, hs));
      break;

    case 3:
      sprintf(hs, "NOTRUBRIK %s", strcpy(STR1, hs));
      break;

    case 4:
      sprintf(hs, "RUBRIK %s", strcpy(STR1, hs));
      break;
    }
    del_lastblanks(hs);
    if ((unsigned)(*lasttyp) < 32 && ((1L << (*lasttyp)) & 0x1e) != 0)
      str2file(oh, hs, true);
  }
  *hs = '\0';
  *lasttyp = 0;
}


static void convert_sfw(short unr, char *sfw)
{
  char w[256], w1[256], hs[256], w3[256];
  char	*p;
  short ih, oh;
  char ina[256], ona[256];
  boolean ok, firstline;
  short lasttyp;

  ok = false;
  iwuser(unr, "opening fwd.bcm ...");
  strcpy(ina, sfw);
  sprintf(w, "%sfwd.tmp", boxsfdir);
  filemove2(ina, w);
  stripcr(w);
  strcpy(ina, w);
  conv_comment(ina, true);

  ih = sfopen(ina, FO_READ);
  if (ih < minhandle) {
    iwlnuser(unr, "cannot access");
    return;
  }
  iwlnuser(unr, "done");
  firstline = true;
  while (file2str(ih, w)) {
    if (w[0] != ' ') {
      if (!firstline)
        sfclose(&oh);
      firstline = false;
      get_word(w, w1);
      upper(w1);
      if (!strcmp(w1, "DUMMY"))
        sprintf(ona, "%sdummy%csfx", boxsfdir, extsep);
      else {
        if (!callsign(w1))
          continue;
        lower(w1);
        sprintf(ona, "%s%s%csf", boxsfdir, w1, extsep);
      }
      oh = sfcreate(ona, FC_FILE);
      if (oh < minhandle)
        iwlnuser(unr, "cannot create outfile");
      else {
        ok = true;
        upper(w1);
        *hs = '\0';
        str2file(&oh, hs, true);
        strcpy(hs,
          "# the next line is a default. please tell TNT and DPBOX how to connect correctly.");
        str2file(&oh, hs, true);
        sprintf(hs, "# IFQRG XXXXX %s 900", w1);
        str2file(&oh, hs, true);
        *hs = '\0';
        str2file(&oh, hs, true);
        strcpy(hs, "SFPARMS 1 0 1 2000000 2000000 200000 600 0000 2359");
        str2file(&oh, hs, true);
        *hs = '\0';
        str2file(&oh, hs, true);
        strcpy(hs,
          "# ------------- now starting definition of files, areas, mailboxes ------------");
        str2file(&oh, hs, true);
        strcpy(hs, "# keywords in these definitions are (at start of line):");
        str2file(&oh, hs, true);
        strcpy(hs, "# FOR NOT NOTFROM NOTRUBRIK");
        str2file(&oh, hs, true);
        strcpy(hs, "# NOT is the negation of FOR , NOTFROM is a neighbour BBS");
        str2file(&oh, hs, true);
        strcpy(hs,
          "# hierarchical names and abbreviations with <*> are allowed for FOR|NOT|NOTFROM");
        str2file(&oh, hs, true);
        strcpy(hs,
          "# keyword not for regular use in S&F is: RUBRIK . selects all boards files");
        str2file(&oh, hs, true);
        strcpy(hs,
          "# -----------------------------------------------------------------------------");
        str2file(&oh, hs, true);
        *hs = '\0';
        str2file(&oh, hs, true);

        lasttyp = 0;
        *hs = '\0';
      } 
    } else {
      strdelete(w, 1, 1);
      while (*w != '\0') {
        get_word(w, w3);
        if (w3[0] == '-')
          continue;

        while (*w3 != '\0' ) {
          if (strlen(w3) > 9) {
            sprintf(w1, "%.9s", w3);
            strdelete(w3, 1, 9);
          } else {
            strcpy(w1, w3);
            *w3 = '\0';
          }
          if (strchr(w1, '-') != NULL)
            continue;
          if (*w1 == '\0')
            continue;
          if ((p = strchr(w1, '?')) != NULL) {
            *p = '\0';
            strcat(w1, "*");
          }
          if (strlen(hs) > 70)
            flush_sfw(&oh, hs, &lasttyp);
          switch (w1[0]) {

          case '*':
            if (lasttyp != 3)
              flush_sfw(&oh, hs, &lasttyp);
            lasttyp = 3;
            strdelete(w1, 1, 1);
            sprintf(hs + strlen(hs), "%s ", w1);
            break;

          default:
	    if (lasttyp != 1)
	      flush_sfw(&oh, hs, &lasttyp);
            lasttyp = 1;
            sprintf(hs + strlen(hs), "%s ", w1);
            break;
          }
        }

      }
      flush_sfw(&oh, hs, &lasttyp);

    }
  }
  sfclose(&oh);
  sfclose(&ih);
  sfdelfile(ina);
}


static void conv3(short unr, char *w)
{
  /* SF-Definitionen konvertieren */
  long t;
  char cp[256];

  if (cverr)
    return;
  if (!check_cpath(w)) {
    iwlnuser(unr, "invalid pathname");
    return;
  }
  wlnuser0(unr);
  iwlnuser(unr, "conversion step 3");
  iwlnuser(unr, "importing S&F definition file");
  t = get_cpuusage();
  sprintf(cp, "%sfwd.bcm", w);
  if (!exist(cp)) {  /* Forward-Datei existiert nicht */
    cverr = true;
    iwlnuser(unr, "err 2 - Forwardfile fwd.bcm not found");
    return;
  }
    convert_sfw(unr, cp);
  if (idel)
    sfdelfile(cp);

  wlnuser0(unr);
  iwlnuser(unr, "not all forward definitions can be converted automatically.");
  iwlnuser(unr, "so please check your .SF files if all is done correct.");
  iwlnuser(unr, "remember to set up the connect path in .SF - files.");
  if (cverr)
    iwlnuser(unr, "some errors occured in step 3");
  else
    iwlnuser(unr, "step 3 finished without errors");
  printtime(unr, get_cpuusage() - t);
  iwuser(unr, "now reloading new settings...");
  load_all_parms(unr);
  iwlnuser(unr, " done");

}


static void conv4(short unr, char *improotdir)
{
  short ifn;
  long t, tctu, tctb;
  char cp[256], pu[256], pi[256], ww[256];
  char w[256], hs[256], STR7[256];

  if (cverr)
    return;
  if (!check_cpath(improotdir)) {
    iwlnuser(unr, "invalid pathname");
    return;
  }
  wlnuser0(unr);
  iwlnuser(unr, "conversion step 4");
  if (packdelay == 0)
    iwlnuser(unr, "all files will be compressed immediately while conversion.");
  iwlnuser(unr, "importing all user files");
  t = get_cpuusage();
  tctb = 0;
  sprintf(cp, "%sinit.bcm", improotdir);
  cverr = true;
  ifn = sfopen(cp, FO_READ);
  if (ifn >= minhandle) {
    while (file2str(ifn, hs)) {
      if (hs[0] == ';')
        continue;
      upper(hs);
      get_word(hs, w);
      get_word(hs, ww);
      if (!strcmp(w, "USERPATH")) {
        conv_pname_rel(improotdir, ww);
        strcpy(pu, ww);
        cverr = false;
      }
    }
  }
  sfclose(&ifn);  
  if (cverr) {
    iwlnuser(unr, "err 2 - user-path not found in init.bcm");
    return;
  }

  cverr = true;
  ifn = sfopen(cp, FO_READ);
  if (ifn >= minhandle) {
    while (file2str(ifn, hs)) {
      if (hs[0] == ';')
        continue;
      upper(hs);
      get_word(hs, w);
      get_word(hs, ww);
      if (!strcmp(w, "INFOPATH")) {
        conv_pname_rel(improotdir, ww);
	strcpy(pi, ww);
        cverr = false;
      }
    }
  }
  sfclose(&ifn);
  if (cverr) {
    iwlnuser(unr, "err 3 - info-path not found in init.bcm");
    return;
  }

  tctu = convert_diebox_mails(unr, true, pu);
  iwuser(unr, "all user files imported. total files: ");
  sprintf(ww, "%ld", tctu);
  iwlnuser(unr, ww);
  iwlnuser(unr, "importing all info files");
  
  tctb = convert_diebox_mails(unr, false, pi);
  iwuser(unr, "all info files imported. total files: ");
  sprintf(ww, "%ld", tctb);
  iwlnuser(unr, ww);

  iwlnuser(unr, "recreating boxlog (check-list) ...");
  create_new_boxlog(unr, false);
  iwlnuser(unr, "done.");

  tctu += tctb;
  sprintf(ww, "%ld", tctu);
  sprintf(ww, "file import over all: %s files.", strcpy(STR7, ww));
  iwlnuser(unr, ww);

  wlnuser0(unr);
  if (cverr)
    iwlnuser(unr, "some errors occured in step 4");
  else
    iwlnuser(unr, "step 4 finished without errors");
  printtime(unr, get_cpuusage() - t);

}


typedef char dbufft1[216];
typedef char dbufft2[66];
typedef char dbufft3[233];

/* ich habe die DOS-Baybox-Userdatenstruktur einfach mal hier her kopiert */
/* alles was nicht verwertbar ist, ist auskommentiert */
typedef struct drect
{ char     call[7];                 /*  7 Rufzeichen des Users */
  char     name[15];                /* 15 A NAME */
  char     mybbs[25];               /* 25 A FORWARD */
/*char     prompt[PROMPTLEN+1];    / * 21 A PROMPT */
  char     sprache[4];              /*  4 A SPEECH */
  char     password[40];            /* 40 A PW */
/*char     notvisible[REJECTLEN+1];/ * 80 A REJECT */
/*char     firstcmd[FIRSTCMDLEN+1];/ * 21 A COMMAND */
/*short unsigned helplevel;        / *  2 A HELPLEVEL */
/*short unsigned zeilen;           / *  2 A LINES */
/*char     nopurge;                / *  1 Board wird nicht geputzt */
/*char     nameok;                 / *  1 Name selbst eingegeben */
  time_t   lastboxlogin;            /*  4 letzter Login (UNIX) */
/*time_t   lastdirnews;            / *  4 letztes DIR NEWS / Check (UNIX) */
/*time_t   lastquit;               / *  4 letztes QUIT */
/*long     mailsent;               / *  4 gesendete Nachrichten */
/*long     mailgot;                / *  4 erhaltene Nachrichten */
/*long     mailread;               / *  4 gelesene Nachrichten */
/*long unsigned logins;            / *  4 logins */
  char     mybbsok;                 /*  1 MYBBS selbst eingegeben (1) */
/*long unsigned  opt[8];           / * 32 eingestellte Optionen */
/*short unsigned lf;               / *  2 A LF */
/*char     umlaut;                 / *  1 A UMLAUT (nicht impl) */
  time_t   mybbstime;               /*  4 Datum des letzten MYBBS (UNIX) */
/*char     ttypw[9];               / *  9 A TTYPW (TTY-Password) */
/*long     lastload;               / *  4 interner Merker für Verwaltung */
/*long unsigned  daybytes;         / *  4 gelesene Bytes heute */
  short unsigned status;            /*  2 A STATUS */
/*char     uplink[10];             / * 10 uplink-Digi */
/*char     readlock;               / *  1 Privatmails nicht lesbar (") */
/*char     pwline;                 / *  1 Leerzeile beim Login */
/*char     echo;                   / *  1 Eigene Eingaben zurückschicken */
/*short unsigned fdelay;           / *  2 Verz÷gerung für eigene Nachrichten */
/*short unsigned fhold;            / *  2 Verz÷gerung für Nachrichten */
/*char     rlimit;                 / *  1 fremde Privatmails nicht lesbar */
/*char     ufwd[40];               / * 40 Connect-Path fuer User-forwarding */
/*char     restplatz[144];         / *144 ist noch frei (definiert 0) */
/*short unsigned filepos;          / *  2 Position im Userfile */
/*short unsigned nextsamehash;     / *  2 hash list link */
                        	    /*512 insgesamt*/
  short unsigned pwmode;
} drect;


static void getcs(char *p, short max, char *s)
{
  long x;

  *s = '\0';
  x = 0;
  while (x < max && p[x] != 32 && p[x] != 0) {
    sprintf(s + strlen(s), "%c", (char)p[x]);
    x++;
  }
}


static boolean convert_dbuff(short unr, char *dbuff1_, char *dbuff2_,
                	     char *dbuff3_, drect *drec, boolean lxbcm)
{
  boolean Result;
  dbufft1 dbuff1;
  dbufft2 dbuff2;
  dbufft3 dbuff3;
  short x, y;
  char *p;
  char w[256], w1[256], STR1[256];

  memcpy(dbuff1, dbuff1_, sizeof(dbufft1));
  memcpy(dbuff2, dbuff2_, sizeof(dbufft2));
  memcpy(dbuff3, dbuff3_, sizeof(dbufft3));
  Result = false;

  *drec->call = '\0';
  drec->lastboxlogin = 0;
  *drec->mybbs = '\0';
  *drec->name = '\0';
  drec->mybbstime = 0;
  drec->status = 1;
  *drec->password = '\0';
  drec->pwmode = 0;
  
  if (lxbcm)
    getcs((char *)(&dbuff1[168]), 6, w);
  else
    getcs(dbuff1, 6, w);
  if (!callsign(w)) {
    sprintf(w1, "bad call: %s", w); 
    sprintf(STR1, "%sbaybox%cimp", boxprotodir, extsep);
    append(STR1, w1, true);
    return Result;
  }
  strcpy(drec->call, w);
  
  if (lxbcm)
    getcs((char *)(&dbuff1[157]), 15, drec->name);
  else
    getcs((char *)(&dbuff1[7]), 15, drec->name);

  if (lxbcm)
    getcs((char *)(&dbuff1[190]), 6, drec->mybbs);
  else
    getcs((char *)(&dbuff1[22]), 6, drec->mybbs);

  upper(drec->mybbs);
  p = strchr(drec->mybbs, '.');
  if (p != NULL) *p = '\0';
  if (!callsign(drec->mybbs)) {
    sprintf(w1, "bad mybbs of %s: %s", drec->call, drec->mybbs); 
    sprintf(STR1, "%sbaybox%cimp", boxprotodir, extsep);
    append(STR1, w1, true);
    return Result;
  }

  if (lxbcm)
    getcs((char *)(&dbuff2[21]), 3, drec->sprache);
  else
    getcs((char *)(&dbuff1[68]), 3, drec->sprache);

  if (lxbcm)
    getcs((char *)(&dbuff2[25]), 39, drec->password);
  else
    getcs((char *)(&dbuff1[72]), 39, drec->password);

  if (strlen(drec->password) > 20)
    drec->pwmode = 2;
  else
    *drec->password = '\0'; 

  y = 1;
  if (lxbcm) { 
    for (x = 0; x <= 3; x++) {
      switch (y) {

      case 1:
        drec->lastboxlogin = (long)dbuff1[x];
        break;

      case 2:
        drec->lastboxlogin += (long)dbuff1[x] * 256;
        break;

      case 3:
        drec->lastboxlogin += (long)dbuff1[x] * 65536;
        break;

      case 4:
        drec->lastboxlogin += (long)dbuff1[x] * 16777216;
        break;
      }
      y++;
    }
  } else {
    for (x = 4; x <= 7; x++) {
      switch (y) {

      case 1:
        drec->lastboxlogin = (long)dbuff2[x];
        break;

      case 2:
        drec->lastboxlogin += (long)dbuff2[x] * 256;
        break;

      case 3:
        drec->lastboxlogin += (long)dbuff2[x] * 65536;
        break;

      case 4:
        drec->lastboxlogin += (long)dbuff2[x] * 16777216;
        break;
      }
      y++;
    }
  }

  if (lxbcm) {
    if (dbuff3[103] == 1)
      drec->mybbsok = true;
    else
      drec->mybbsok = false;    
  } else {
    if (dbuff2[32] == 1)
      drec->mybbsok = true;
    else
      drec->mybbsok = false;    
  }
  
  y = 1;
  if (lxbcm) {
    for (x = 12; x <= 15; x++) {
      switch (y) {

      case 1:
        drec->mybbstime = (long)dbuff1[x];
        break;

      case 2:
        drec->mybbstime += (long)dbuff1[x] * 256;
        break;

      case 3:
        drec->mybbstime += (long)dbuff1[x] * 65536;
        break;

      case 4:
        drec->mybbstime += (long)dbuff1[x] * 16777216;
        break;
      }
      y++;
    }
  } else {
    for (x = 3; x <= 6; x++) {
      switch (y) {

      case 1:
        drec->mybbstime = (long)dbuff3[x];
        break;

      case 2:
        drec->mybbstime += (long)dbuff3[x] * 256;
        break;

      case 3:
        drec->mybbstime += (long)dbuff3[x] * 65536;
        break;

      case 4:
        drec->mybbstime += (long)dbuff3[x] * 16777216;
        break;
      }
      y++;
    }
  }

  if (drec->mybbstime == 0 || drec->mybbstime < clock_.ixtime - 31536000L * 5 ||
      drec->mybbstime > clock_.ixtime) {
    sprintf(w1, "bad mybbstime of %s", drec->call); 
    sprintf(STR1, "%sbaybox%cimp", boxprotodir, extsep);
    append(STR1, w1, true);
    return Result;
  }

  gkdeutsch(drec->name);

  if (lxbcm) {
    if (dbuff2[6] == 2)
      drec->status = 0;
    else
      drec->status = 1;
  } else {
    if (dbuff3[24] == 2)
      drec->status = 0;
    else
      drec->status = 1;
  }

  Result = true;

  return Result;
}


static void conv2(short unr, char *p_)
{
  /* Die Userdaten */
  char cp[256], p[256], w1[256];
  char dn[256];
  short dh;
  boolean ok, linuxbcm;
  long t, ct;
  dbufft1 dbuff1;
  dbufft2 dbuff2;
  dbufft3 dbuff3;
  drect drec;
  long dsize;
  userstruct urec;
  char STR1[256], STR7[256];

  strcpy(p, p_);
  ok = false;
  ct = 0;
  linuxbcm = false;
  if (cverr)
    return;
  if (!check_cpath(p)) {
    iwlnuser(unr, "invalid pathname");
    return;
  }
  wlnuser0(unr);
  iwlnuser(unr, "conversion step 2");
  iwlnuser(unr, "importing user settings");
  t = get_cpuusage();
  sprintf(cp, "%sinit.bcm", p);
  sprintf(dn, "%susers3.bcm", p);
  if (exist(dn))
    linuxbcm = true;
  else
    sprintf(dn, "%susers.bcm", p);

  if (exist(dn)) {
    dsize = sfsize(dn);   /*dsize mod 512 = 0*/
    if (true) {
      dh = sfopen(dn, FO_READ);
      if (dh >= 0) {
	iwuser(unr, "now converting ");
	sprintf(w1, "%ld", dsize / 512);
	iwuser(unr, w1);
	iwlnuser(unr, " user settings");

	strcpy(w1, "users with special settings or import errors:");
	sprintf(STR1, "%sbaybox%cimp", boxprotodir, extsep);
	append(STR1, w1, true);

	while ((sfread(dh, 215, dbuff1) == 215) &&
               (sfread(dh, 65, dbuff2) == 65) &&
               (sfread(dh, 232, dbuff3) == 232)) {
          dp_watchdog(2, 4711);
          if (!convert_dbuff(unr, dbuff1, dbuff2, dbuff3, &drec, linuxbcm))
            continue;
          ct++;
          strcpy(w1, ".");
          if (ct % 79 == 0)
            wlnuser(unr, w1);
          else
            wuser(unr, w1);
          boxspoolread();

          if (strlen(drec.name) > 2 || !strcmp(drec.mybbs, Console_call) ||
              drec.status != 1) {
            load_userinfo_for_change(false, drec.call, &urec);
            if (!strcmp(drec.sprache, "EXP") || !strcmp(drec.sprache, "XDL"))
              strcpy(drec.sprache, "DLX");
            urec.level = drec.status;
            urec.plevel = drec.status;
            strcpy(urec.language, drec.sprache);
            urec.lastdate = drec.lastboxlogin;
            strcpy(urec.mybbs, drec.mybbs);
            strcpy(urec.name, drec.name);
            urec.pwmode = drec.pwmode;
            urec.mybbsupd = drec.mybbstime;
            strcpy(urec.password, drec.password);


            if (/*drec.pwmode > 0 || */drec.status != 1) {
              sprintf(STR1, "%sbaybox%cimp", boxprotodir, extsep);
              sprintf(STR7, "locked out: %s", urec.call);
              append(STR1, STR7, true);
            }

            save_userfile(&urec);

          }

          if (!drec.mybbsok)
            update_mybbsfile(false, drec.call, &drec.mybbstime, drec.mybbs, "G");
          else
            update_mybbsfile(false, drec.call, &drec.mybbstime, drec.mybbs, "U");

        }

        ok = true;
	sfclose(&dh);
	wlnuser0(unr);
	sprintf(w1, "%ld", ct);
	strcat(w1, " user settings converted");
	iwlnuser(unr, w1);
      } else
	iwlnuser(unr, "cannot open users.bcm");
      if (idel && ok) {
	sfdelfile(dn);
      }
    } else
      iwlnuser(unr, "size of users.bcm improper");
  } else
    iwlnuser(unr, "users.bcm not found");

  wlnuser0(unr);
  if (cverr)
    iwlnuser(unr, "some errors occured in step 2");
  else
    iwlnuser(unr, "step 2 finished without errors");
  printtime(unr, get_cpuusage() - t);
}


static void conv1(short unr, char *ipath)
{
  /* Die Konfiguration */
  char cp[256], p[256], w [256], w1[256];
  char f[256], hs[256];
  DTA dirinfo;
  short result, ifn, ofn;
  long t;
  char STR1[256], STR7[256];

  if (cverr)
    return;
  if (!check_cpath(ipath)) {
    iwlnuser(unr, "invalid pathname");
    return;
  }
  wlnuser0(unr);
  iwlnuser(unr, "conversion step 1");
  iwlnuser(unr, "importing box configuration files");
  t = get_cpuusage();
  
  sprintf(cp, "%sinit.bcm", ipath);
  sprintf(STR1, "%sconfig.tmp", boxsysdir);
  filemove2(cp, STR1);
  stripcr(STR1);
  strcpy(cp, STR1);
  conv_comment(cp, true);
  
  iwlnuser(unr, "now adapting init.bcm to config.box");
  sprintf(f, "%sconfig%cbox", boxsysdir, extsep);

  ifn = sfopen(cp, FO_READ);
  if (ifn >= minhandle) {
    while (file2str(ifn, hs)) {
      upper(hs);
      get_word(hs, w);
      get_word(hs, w1);

      if (!strcmp(w, "BOXADDRESS") && strcmp(w1, ownhiername)) {
        strcpy(ownhiername, w1);
        iwuser(unr, "ownhiername changed to: ");
        iwlnuser(unr, ownhiername);
        sprintf(p, "OWNHIERNAME %s", ownhiername);
        replace_keyline(f, "OWNHIERNAME", false, p);
        
      } else if (!strcmp(w, "MAXBIDS")) {
        if (atol(w1) < 30000) {   /* + 10000 fuer UserBIDs */
          iwlnuser(unr,
            "original baybox-maxbull-setting is too low, will insert 40000");
          strcpy(w1, "40000");
        } else if (atol(w1) > 200000L) {
          iwlnuser(unr,
           "original baybox-maxbull-setting is too high, will insert 200000");
          strcpy(w1, "200000");
        } else
          sprintf(w1, "%ld", atol(w1) + 10000);
        sprintf(p, "MAXBULLIDS %s", strcpy(STR7, w1));
        iwlnuser(unr, p);
        replace_keyline(f, "MAXBULLIDS", false, p);

      } else if (!strcmp(w, "REMERASE")) {
        if (w1[0] == '0')
          strcpy(w1, "OFF");
        else
          strcpy(w1, "ON");
        sprintf(p, "REMOTE_ERASE %s", strcpy(STR7, w1));
        iwlnuser(unr, p);
        replace_keyline(f, "REMOTE_ERASE", false, p);
        if (w1[0] > '1')
          strcpy(w1, "ON");
        else
          strcpy(w1, "OFF");
        sprintf(p, "REMOTEERASECHECK %s", strcpy(STR7, w1));
        iwlnuser(unr, p);
        replace_keyline(f, "REMOTEERASECHECK", false, p);

      } else if (!strcmp(w, "USERLIFE")) {
        if (atol(w1) == 999)
          strcpy(w1, "0");
        if (atol(w1) < 999) {
          sprintf(p, "USERLIFETIME %s", strcpy(STR7, w1));
          iwlnuser(unr, p);
          replace_keyline(f, "USERLIFETIME", false, p);
        }

      } else if (!strcmp(w, "INFOLIFE")) {
        if (atol(w1) == 999)
          strcpy(w1, "0");
        if (atol(w1) < 999) {
          sprintf(p, "UNDEFBOARDSLT %s", strcpy(STR7, w1));
          iwlnuser(unr, p);
          replace_keyline(f, "UNDEFBOARDSLT", false, p);
        }
      } else if (!strcmp(w, "FWDTIMEOUT")) {
        sprintf(p, "SFTIMEOUT %s", strcpy(STR7, w1));
        iwlnuser(unr, p);
        replace_keyline(f, "SFTIMEOUT", false, p);
      
      } else if (!strcmp(w, "MAXLOGINS")) {
        if (atol(w1) == 0)
          strcpy(w1, "5");
        sprintf(p, "MAXUSERCONNECTS %s", strcpy(STR7, w1));
        iwlnuser(unr, p);
        replace_keyline(f, "MAXUSERCONNECTS", false, p);

      } else if (!strcmp(w, "USERQUOTA")) {
        if (atol(w1) == 0)
          strcpy(w1, "999999999");
        else
          sprintf(w1, "%s000", strcpy(STR7, w1));
        sprintf(p, "MAXPERDAY %s", strcpy(STR7, w1));
        iwlnuser(unr, p);
        replace_keyline(f, "MAXPERDAY", false, p);
      } else
        continue;
    }
  }
  sfclose(&ifn);
  sfdelfile(cp);
  iwlnuser(unr, "config.box adapted.");


  iwlnuser(unr, "convlt");
  sprintf(STR7, "%sconvlife.bcm", ipath);
  sprintf(STR1, "%sconvlt.box", boxsysdir);
  filemove2(STR7, STR1);
  sprintf(STR7, "%sconvlt.box", boxsysdir);
  stripcr(STR7);
  conv_comment(STR7, false);
  iwlnuser(unr, "convname/transfer");
  sprintf(STR7, "%sconvname.bcm", ipath);
  sprintf(STR1, "%stransfer.box", boxsysdir);
  filemove2(STR7, STR1);
  sprintf(STR7, "%stransfer.box", boxsysdir);
  stripcr(STR7);
  conv_comment(STR7, false);
  iwlnuser(unr, "badnames");
  iwlnuser(unr, "(File has no functionality in DPBOX. Alter reject.sys instead.)");
  sprintf(STR7, "%sbadnames.bcm", ipath);
  sprintf(STR1, "%sbadnames.box", boxsysdir);
  filemove2(STR7, STR1);
  sprintf(STR7, "%sbadnames.box", boxsysdir);
  stripcr(STR7);
  conv_comment(STR7, false);
  iwlnuser(unr, "bulletin.bcm / rubriken.box");
  sprintf(STR7, "%sbulletin.bcm", ipath);
  sprintf(STR1, "%srubriken.tmp", boxsysdir);
  filemove2(STR7, STR1);
  sprintf(STR7, "%srubriken.tmp", boxsysdir);
  stripcr(STR7);
  conv_comment(STR7, false);

  sprintf(STR1, "%srubriken.out", boxsysdir);
  ifn = sfopen(STR7, FO_READ);
  if (ifn >= minhandle) {
    ofn = sfcreate(STR1, FC_FILE);
    str2file(&ofn, "# Board  Lifetime  Accesslevel", true);
    while (file2str(ifn, hs)) {
      del_leadblanks(hs);
      if (hs[0] != '#') {
        upper(hs);
        get_word(hs, w);
        get_word(hs, w1);
        sprintf(hs, "%s %s 1", w, w1);
      }
      str2file(&ofn, hs, true);
    }
    sfclose(&ifn);
    sfclose(&ofn);
    sprintf(STR1, "%srubriken.out", boxsysdir);
    sprintf(STR7, "%srubriken.box", boxsysdir);
    sort_file(STR1, false);
    filemove(STR1, STR7);
  } else
    sfclose(&ifn);
  sprintf(STR1, "%srubriken.tmp", boxsysdir);
  sfdelfile(STR1);

  sprintf(STR7, "%s%c%cpwd", ipath, allquant, extsep);
  result = sffirst(STR7, 0, &dirinfo);
  while (result == 0) {
    iwlnuser(unr, dirinfo.d_fname);
    strcpylower(w1, dirinfo.d_fname);
    sprintf(STR7, "%s%s", ipath, dirinfo.d_fname);
    sprintf(STR1, "%s%s", boxsfdir, w1);
    filemove2(STR7, STR1);
    result = sfnext(&dirinfo);
  }
  iwlnuser(unr, "remember to set up personal passwords for former users of PASSWD.BCM");
  wlnuser0(unr);

  if (cverr)
    iwlnuser(unr, "some errors occured in step 1");
  else
    iwlnuser(unr, "step 1 finished without errors");
  printtime(unr, get_cpuusage() - t);
  iwuser(unr, "now reloading new settings...");
  load_all_parms(unr);
  iwlnuser(unr, " done");
}


void conv_baybox(short unr, char *eingabe_)
{
  char eingabe[256];
  char w[256], p[256], s[256];

  strcpy(eingabe, eingabe_);
  blocking_off();
  boxisbusy(true);

  cverr = false;
  get_word(eingabe, w);
  get_word(eingabe, p);
  get_word(eingabe, s);
  idel = (!strcmp(s, "-") || !strcmp(p, "-"));

  if (idel)
    iwlnuser(unr, "will delete original files after conversion");
  else
    iwlnuser(unr, "will hold original files after conversion");

  switch (w[0]) {

  case '1':   /* Config-Files      */
    conv1(unr, p);
    break;


  case '2':   /* Userdaten         */
    conv2(unr, p);
    break;


  case '3':   /* Forward-Dateien   */
    conv3(unr, p);
    break;

  case '4':   /* Mails/Bulletins   */
    conv4(unr, p);
    break;

  default:
    conv1(unr, w);
    conv2(unr, w);
    conv3(unr, w);
    conv4(unr, w);
    break;
  }

  wlnuser0(unr);
  wlnuser(unr, "check logfile <proto/baybox.imp> for all performed actions.");
  wlnuser(unr, "import is ended.");
  boxisbusy(false);
  blocking_on();
}



/* ************************************************************************ */
/* *                   Ende Konvertierroutinen                            * */
/* ************************************************************************ */
