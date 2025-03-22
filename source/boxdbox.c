/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#define BOXDBOX_G
#include "main.h"
#include "boxdbox.h"
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
/* * Die Konvertierroutinen fuer einen kompletten DieBox-Daten-Import     * */
/* ************************************************************************ */

static boardtype actboard;


void cdbtboard(char *s)
{
  /* hier holt sich new_entry() den transferierten brettnamen her */
  strcpy(s, actboard);
}


static void iwuser(short unr, char *s)
{
  char STR1[256];

  sprintf(STR1, "%sdiebox%cimp", boxprotodir, extsep);
  append(STR1, s, false);
  wuser(unr, s);
  boxspoolread();
}


static void iwlnuser(short unr, char *s)
{
  char STR1[256];

  sprintf(STR1, "%sdiebox%cimp", boxprotodir, extsep);
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
  if (p[0] == '\\')
    strdelete(p, 1, 1);
  FORLIM = strlen(p);
  for (x = 0; x < FORLIM; x++) {
    if (p[x] == '\\')
      p[x] = '/';
  }
  lower(p);
  sprintf(p, "%s%s", w, strcpy(STR1, p));
}


static boolean check_cpath(char *w_)
{
  boolean Result;
  char w[256];
  boolean ok;

  strcpy(w, w_);
  strcat(w, "config.box");
  ok = exist(w);
  Result = ok;
  if (!ok)
    cverr = true;
  return Result;
}


static void dop(const short unr, const char *s)
{
}


/* holt die neue lifetime aus show.box */

static short get_sblt(char *sbp, long sbs, char *bid)
{
  short Result;
  char hs[256], b2[256];
  long rp;

  Result = -1;
  rp = 0;
  while (rp < sbs) {
    get_line(sbp, &rp, sbs, hs);
    strsub(b2, hs, 71, 12);
    del_lastblanks(b2);
    if (!strcmp(b2, bid)) {
      strsub(b2, hs, 83, 3);
      del_leadblanks(b2);
      return (atoi(b2));
    }
  }
  return Result;
}


static long convert_diebox_mails(short unr, boolean userarea, char *p,
				 char *sb)
{
  char w[256], board[256];
  char hs[256], prot[256];
  char mn[256], w1[256];
  char mnn[256], mn3[256];
  char *sbp;
  long sbs;
  short flh;
  char fmn[256];
  DTA dirinfo;
  boolean improper;
  short result, ph, mh, mnh, ct, x;
  long mct;
  char *pp;
  long ps, err, fp1, fp2;
  short nlt;
  char rxbox[256];
  char STR1[256], STR7[256];

  mct = 0;
  if (userarea)
    sbp = NULL;
  else {
    iwuser(unr,
      "loading show.box for lifetime conversion of sysop-altered mails...");
    sfbread(true, sb, &sbp, &sbs);
    if (sbp == NULL)
      iwlnuser(unr, " not found. doesnt matters.");
    else
      iwlnuser(unr, " done.");
  }

  sprintf(fmn, "%sfmn625702", tempdir);
  flh = sfcreate(fmn, FC_FILE);
  if (flh >= minhandle) {
    iwuser(unr, "creating filelist...");
    sprintf(STR1, "%s%c%c%c", p, allquant, extsep, allquant);
    result = sffirst(STR1, 16, &dirinfo);
    while (result == 0) {
      str2file(&flh, dirinfo.d_fname, true);
      result = sfnext(&dirinfo);
    }
    sfclose(&flh);
    iwuser(unr, " done. sorting list...");
    sort_file(fmn, false);
    iwlnuser(unr, " done");
    flh = sfopen(fmn, FO_READ);
    if (flh >= minhandle) {
      iwlnuser(unr, "importing mails");
      while (file2str(flh, board)) {
	cut(board, 8);
	strcpyupper(actboard, board);
	dp_watchdog(2, 4711);
	improper = false;
	sprintf(STR7, "importing board %s:", board);
	iwlnuser(unr, STR7);
	sprintf(prot, "%s%s%cprotfile.dat", p, board, dirsep);
	iwuser(unr, "opening protfile.dat...");
	ph = sfopen(prot, FO_READ);
	if (ph >= minhandle) {
	  iwlnuser(unr, " done");
	  iwuser(unr, "importing mails into dpbox");
	  ct = 0;
	  while (file2str(ph, hs)) {
	    *rxbox = '\0';
	    dp_watchdog(2, 4711);
	    cut(hs, 6);
	    del_lastblanks(hs);
	    lower(hs);
	    sprintf(mn, "%s%s%c%s", p, board, dirsep, hs);
	    sprintf(w, "%d", ct);
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
		  if (*hs != '\0') {
		    fp1 = sfseek(0, mh, SFSEEKCUR);
		    fp2 = sfseek(0, mh, SFSEEKEND);
		    ps = fp2 - fp1 + 1;
		    sfseek(fp1, mh, SFSEEKSET);

		    if (hs[0] < ' ')
		      strdelete(hs, 1, 1);
		    x = strpos2(hs, " UTC ", 1);
		    if (x > 0) {
		      strdelete(hs, x, 5);
		      strinsert("   0 ", hs, x);
		    } else if (!userarea && sbp != NULL) {
		      file2str(mh, w);   /* subject */
		      file2str(mh, w);   /* BID     */
		      if (strstr(w, "*** Bulletin-ID: ") == w) {
			strdelete(w, 1, 17);
			get_word(w, w1);
			cut(w1, 12);
			if (strlen(w1) > 3) {
			  /* lifetime aus show.box raussuchen */
			  nlt = get_sblt(sbp, sbs, w1);
			} else
			  nlt = -1;

			if (nlt >= 0) {
			  sprintf(w1, "%d", nlt);
			  lspacing(w1, 3);
			  strdelete(hs, 43, 3);
			  strinsert(w1, hs, 43);
			}
		      }
		      file2str(mh, w);   /* received from */
		      if (strstr(w, "*** Received from ") == w) {
			for (x = 1; x <= 4; x++)
			  get_word(w, rxbox);
		      }
		    }


		    if (userarea || sbp == NULL) {
		      file2str(mh, w);   /* subject */
		      file2str(mh, w);   /* leerzeile */
		      file2str(mh, w);   /* received from */
		      if (strstr(w, "*** Received from ") == w) {
			for (x = 1; x <= 4; x++)
			  get_word(w, rxbox);
		      }
		    }
		    /*
DP @DL           de:DF3VI  01.07.95 22:27 200   1242 Bytes
DP @DL           de:DF3VI  01.07.95 22:27 200 999999 Bytes
*/
		    if (strlen(hs) > 52) {
		      for (x = 46; x <= 51; x++)
			hs[x] = '9';
		    }

		    str2file(&mnh, hs, true);

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

	      if (callsign(rxbox)) {
		sprintf(mn3, "%s&%s%c1", newmaildir, rxbox, extsep);
		validate(mn3);
		sfrename(mnn, mn3);
	      } else
		strcpy(mn3, mnn);

	      /* jetzt einsortieren in dpbox! */

	      sort_new_mail(-99, mn3, Console_call);
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
	    iwlnuser(unr, "protfile.dat contained some inconsistencies. doesnt matters.");
	  if (idel) 
	    sfdelfile(prot);
	} else
	  iwlnuser(unr, " failed!");
	if (idel) {
	  sprintf(STR7, "%s%s%c%c%c%c", p, board, dirsep, allquant, extsep, allquant);
	  file_delete(0, STR7, dop);
	}
      }
      iwlnuser(unr, "Import is done");
      sfclose(&flh);
    }
    sfdelfile(fmn);
  } else
    cverr = true;

  if (sbp != NULL)
    free(sbp);

  return mct;
}


static void conv5(short unr, char *w_)
{
  /* BIDs draufhauen */
  char w[256];
  long t, tct, bs;
  char cp[256], p[256], hs[256], ww[256], w1[256], w2[256];
  short ih, oh;
  char STR1[256], STR7[256];

  strcpy(w, w_);
  if (cverr)
    return;
  if (!check_cpath(w)) {
    iwlnuser(unr, "invalid pathname");
    return;
  }
  clear_bidhash();
  wlnuser0(unr);
  iwlnuser(unr, "conversion step 5");
  iwlnuser(unr, "importing all known bulletin IDs");
  t = get_cpuusage();
  tct = 0;
  sprintf(cp, "%sconfig.box", w);
  if (!get_fileline(cp, 38, p)) {  /* pfad auf MBSYS */
    cverr = true;
    iwlnuser(unr, "err 2 - mbsys-path not found in config.box");
    return;
  }
  conv_pname_rel(w, p);
  if (idel) {
    sprintf(STR1, "%sbullid.bak", p);
    sfdelfile(STR1);
    sprintf(STR1, "%scbullid.bak", p);
    sfdelfile(STR1);
    sprintf(STR1, "%scbullid.txt", p);
    sfdelfile(STR1);
  }
  sprintf(ww, "%sbullid.txt", p);
  sprintf(hs, "%ld", sfsize(ww) / 19);
  sprintf(hs, "now trying to import %s BIDs", strcpy(STR7, hs));
  iwlnuser(unr, hs);
  iwuser(unr, "opening bullid.txt... ");
  ih = sfopen(ww, FO_READ);
  if (ih >= minhandle) {
    iwlnuser(unr, "done.");
    bs = sfsize(msgidlog) / 13;
    oh = sfopen(msgidlog, FO_RW);
    if (oh < minhandle)
      oh = sfcreate(msgidlog, FC_FILE);
    if (oh >= minhandle) {
      sfseek(0, oh, SFSEEKEND);
      while (file2str(ih, hs)) {
	get_word(hs, w1);
	if ((unsigned long)strlen(w1) >= 32 ||
	    ((1L << strlen(w1)) & 0x1ff8) == 0)
	  continue;
	tct++;
	if (tct % 100 == 0) {
	  sprintf(w2, "%ld", tct);
	  strcat(w2, " ");
	  if (tct % 1000 == 0)
	    wlnuser(unr, w2);
	  else
	    wuser(unr, w2);
	  boxspoolread();
	}
	if (bs >= maxbullids) {
	  bs = 0;
	  sfseek(0, oh, SFSEEKSET);
	}
	bs++;
	sfwrite(oh, 13, w1);
      }
      sfclose(&oh);
      bullidseek = bs;
      flush_bidseek();
    } else
      iwlnuser(unr, "cannot access to MSGIDMEM.BOX");
    sfclose(&ih);
    if (idel)
      sfdelfile(ww);
    wlnuser0(unr);
    sprintf(w2, "%ld", tct);
    sprintf(w2, "converted %s BIDs", strcpy(STR1, w2));
    iwlnuser(unr, w2);
  } else {
    cverr = true;
    iwlnuser(unr, "failed");
  }

  wlnuser0(unr);
  if (cverr)
    iwlnuser(unr, "some errors occured in step 5");
  else
    iwlnuser(unr, "step 5 finished without errors");
  printtime(unr, get_cpuusage() - t);

}


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


static void convert_sfw(short unr, char *p, char *sfw)
{
  char w[256], w1[256], hs[256], w3[256];
  short ih, oh;
  char *p1;
  char ina[256], ona[256];
  boolean ok;
  short lasttyp;
  char STR7[256];

  ok = false;
  strcpy(w, sfw);
  del_ext(w);
  upper(w);
  if (!callsign(w))
    return;
  sprintf(STR7, "opening %s ...", sfw);
  iwuser(unr, STR7);
  sprintf(ina, "%s%s", p, sfw);
  lower(w);
  sprintf(ona, "%s%s%csf", boxsfdir, w, extsep);
  ih = sfopen(ina, FO_READ);
  if (ih < minhandle) {
    iwlnuser(unr, "cannot access");
    return;
  }
  iwlnuser(unr, "done");
  oh = sfcreate(ona, FC_FILE);
  if (oh >= minhandle) {
    ok = true;

    upper(w);
    str2file(&oh, hs, true);
    strcpy(hs,
      "# the next line is a default. please tell TNT and DPBOX how to connect correctly.");
    str2file(&oh, hs, true);
    sprintf(hs, "# IFQRG XXXXX %s 900", w);
    str2file(&oh, hs, true);
    *hs = '\0';
    str2file(&oh, hs, true);
    strcpy(hs, "SFPARMS 1 1 1 2000000 2000000 200000 600 0000 2359");
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
    while (file2str(ih, w)) {
      while (*w != '\0') {
	get_word(w, w3);

	while (*w3 != '\0') {
	  if (strlen(w3) > 9) {
	    sprintf(w1, "%.9s", w3);
	    strdelete(w3, 1, 9);
	  } else {
	    strcpy(w1, w3);
	    *w3 = '\0';
	  }

	  if (strstr(w1, "*!") != NULL) {
	    flush_sfw(&oh, hs, &lasttyp);
	    continue;
	  }
	  if (*w1 == '\0')
	    continue;
	  if ((p1 = strchr(w1, '?')) != NULL) {
	    *p1 = '\0';
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

	  case '~':
	    if (lasttyp != 2)
	      flush_sfw(&oh, hs, &lasttyp);
	    lasttyp = 2;
	    strdelete(w1, 1, 1);
	    sprintf(hs + strlen(hs), "%s ", w1);
	    break;

	  case '%':
	    if (lasttyp != 4)
	      flush_sfw(&oh, hs, &lasttyp);
	    lasttyp = 4;
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
    }
    flush_sfw(&oh, hs, &lasttyp);

    sfclose(&oh);
  } else
    iwlnuser(unr, "cannot create outfile");
  sfclose(&ih);
  if (ok && idel)
    sfdelfile(ina);
}


static void conv3(short unr, char *w)
{
  /* SFW konvertieren */
  long t;
  char cp[256], p[256];
  DTA dirinfo;
  short result;
  char STR1[256];

  if (cverr)
    return;
  if (!check_cpath(w)) {
    iwlnuser(unr, "invalid pathname");
    return;
  }
  wlnuser0(unr);
  iwlnuser(unr, "conversion step 3");
  iwlnuser(unr, "importing S&F definition files");
  t = get_cpuusage();
  sprintf(cp, "%sconfig.box", w);
  if (!get_fileline(cp, 38, p)) {  /* pfad auf MBSYS */
    cverr = true;
    iwlnuser(unr, "err 2 - mbsys-path not found in config.box");
    return;
  }

  conv_pname_rel(w, p);

  sprintf(STR1, "%s%c%csfw", p, allquant, extsep);
  result = sffirst(STR1, 0, &dirinfo);
  while (result == 0) {
    convert_sfw(unr, p, dirinfo.d_fname);
    if (idel) {
      sprintf(STR1, "%s%s", p, dirinfo.d_fname);
      sfdelfile(STR1);
    }
    result = sfnext(&dirinfo);
  }

  wlnuser0(unr);
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


static void conv4(short unr, char *w)
{
  long t, tctu, tctb;
  char cp[256], p[256], sb[256], ww[256];
  char STR7[256];

  if (cverr)
    return;
  if (!check_cpath(w)) {
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
  sprintf(cp, "%sconfig.box", w);
  if (!(get_fileline(cp, 36, p) && get_fileline(cp, 38, sb)))
      /* pfad auf USER */
      {  /* pfad auf MBSYS */
    cverr = true;
    iwlnuser(unr, "err 2 - user/mbsys-path not found in config.box");
    return;
  }

  conv_pname_rel(w, p);
  conv_pname_rel(w, sb);
  strcat(sb, "show.box");
  tctu = convert_diebox_mails(unr, true, p, sb);
  iwuser(unr, "all user files imported. total files: ");
  sprintf(ww, "%ld", tctu);
  iwlnuser(unr, ww);
  iwlnuser(unr, "importing all info files");
  if (get_fileline(cp, 37, p)) {  /* pfad auf INFO */
    conv_pname_rel(w, p);
    tctb = convert_diebox_mails(unr, false, p, sb);
    iwuser(unr, "all info files imported. total files: ");
    sprintf(ww, "%ld", tctb);
    iwlnuser(unr, ww);
    if (idel)   /* show.box loeschen */
      sfdelfile(sb);

    iwlnuser(unr, "recreating boxlog (check-list) ...");
    create_new_boxlog(unr, false);
    iwlnuser(unr, "done.");
  } else {
    cverr = true;
    iwlnuser(unr, "err 3 - info-path not found in config.box");
  }

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


typedef char ibufft[12];
typedef char dbufft[48];

typedef struct irect {
  char call[7];
  long offset;
} irect;

typedef struct drect {
  char call[7];
  char lan[4];
  long last;
  char mybbs[7];
  char name[16];
  short level, pwmode;
  long mybbsupd;
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


static boolean convert_ibuff(char *ibuff_, irect *irec)
{
  ibufft ibuff;
  short x, y;
  char *p;

  memcpy(ibuff, ibuff_, sizeof(ibufft));
  p = ibuff;
  getcs(p, 6, irec->call);
  upper(irec->call);

  y = 1;
  for (x = 8; x <= 11; x++) {
    switch (y) {

    case 1:
      irec->offset = (long)ibuff[x];
      break;

    case 2:
      irec->offset += (long)ibuff[x] * 256;
      break;

    case 3:
      irec->offset += (long)ibuff[x] * 65536;
      break;

    case 4:
      irec->offset += (long)ibuff[x] * 16777216;
      break;
    }
    y++;
  }

  return (callsign(irec->call) && irec->offset >= 0);
}


static boolean convert_dbuff(char *call, char *dbuff_, drect *drec)
{
  boolean Result;
  dbufft dbuff;
  short x, y;
  char *p;
  char w[256], w1[256];

  memcpy(dbuff, dbuff_, sizeof(dbufft));
  Result = false;

  strcpy(drec->call, call);
  drec->last = 0;
  *drec->mybbs = '\0';
  *drec->name = '\0';
  drec->level = 1;
  drec->mybbsupd = 0;
  drec->pwmode = 0;

  p = dbuff;
  getcs(p, 3, drec->lan);

  getcs((char *)(&p[4]), 8, w1);
  getcs((char *)(&p[13]), 5, w);
  drec->last = string2ixtime(w1, w);

  getcs((char *)(&p[19]), 6, drec->mybbs);
  upper(drec->mybbs);
  if (!strcmp(drec->mybbs, "OWNBBS"))   /* !... */
    strcpy(drec->mybbs, Console_call);

  if (!callsign(drec->mybbs))
    return Result;


  getcs((char *)(&p[26]), 15, drec->name);

  y = 1;
  for (x = 44; x <= 47; x++) {
    switch (y) {

    case 1:
      drec->mybbsupd = (long)dbuff[x];
      break;

    case 2:
      drec->mybbsupd += (long)dbuff[x] * 256;
      break;

    case 3:
      drec->mybbsupd += (long)dbuff[x] * 65536;
      break;

    case 4:
      drec->mybbsupd += (long)dbuff[x] * 16777216;
      break;
    }
    y++;
  }

#define THEBOX_ERRONEOUS_OFFSET 18000

  if (drec->mybbsupd > THEBOX_ERRONEOUS_OFFSET)   /* !... */
    drec->mybbsupd -= THEBOX_ERRONEOUS_OFFSET;
  else
    drec->mybbsupd = 0;

#undef THEBOX_ERRONEOUS_OFFSET

  if (drec->mybbsupd == 0 || drec->mybbsupd < clock_.ixtime - 31536000L * 5 ||
      drec->mybbsupd > clock_.ixtime)
    return Result;

  gkdeutsch(drec->name);

  Result = true;

  switch (p[42]) {   /* der diebox-level */

  case 0:   /*normal*/
    break;

  case 1:
  case 4:   /*sysop*/
    drec->pwmode = 10;
    break;

  case 2:   /*gesperrt*/
    drec->level = 0;
    break;

  case 5:   /*sysop mit schutz*/
    drec->pwmode = 11;
    break;

  case 7:   /*rubrikensysop*/
    drec->pwmode = 8;
    break;

  case 8:   /*rubrikensysop mit schutz*/
    drec->pwmode = 9;
    break;

  case 9:   /*user must priv*/
    drec->pwmode = 2;
    break;
  }

  return Result;
}


static void conv2(short unr, char *w_)
{
  /* Die Userdaten */
  char w[256];
  char cp[256], p[256], w1[256];
  char idn[256], dn[256];
  short idh, dh;
  boolean ok;
  long t, ct;
  ibufft ibuff;
  dbufft dbuff;
  irect irec;
  drect drec;
  long dsize;
  userstruct urec;
  char STR1[256];

  strcpy(w, w_);
  ok = false;
  ct = 0;
  if (cverr)
    return;
  if (!check_cpath(w)) {
    iwlnuser(unr, "invalid pathname");
    return;
  }
  wlnuser0(unr);
  iwlnuser(unr, "conversion step 2");
  iwlnuser(unr, "importing user settings");
  t = get_cpuusage();
  sprintf(cp, "%sconfig.box", w);
  if (!get_fileline(cp, 38, p)) {  /* pfad auf MBSYS */
    cverr = true;
    iwlnuser(unr, "err 1 - mbsys-path not found in config.box");
    return;
  }

  conv_pname_rel(w, p);

  sprintf(dn, "%suser3.dat", p);
  sprintf(idn, "%suser3.idx", p);
  if (exist(dn) && exist(idn)) {
    dsize = sfsize(dn);   /*dsize mod 48 = 0*/
    if (true) {
      if (sfsize(idn) % 12 == 0) {
	idh = sfopen(idn, FO_READ);
	if (idh >= minhandle) {
	  dh = sfopen(dn, FO_READ);
	  if (dh >= 0) {
	    iwuser(unr, "now converting ");
	    sprintf(w1, "%ld", dsize / 48);
	    iwuser(unr, w1);
	    iwlnuser(unr, " user settings");

	    strcpy(w1, "users with extended/restricted settings:");
	    sprintf(STR1, "%sdiebox%cimp", boxprotodir, extsep);
	    append(STR1, w1, true);

	    while (sfread(idh, 12, ibuff) == 12) {
	      dp_watchdog(2, 4711);
	      if (!convert_ibuff(ibuff, &irec))
		continue;
	      if (irec.offset > dsize - 48)
		continue;
	      if (sfseek(irec.offset, dh, SFSEEKSET) != irec.offset)
		continue;
	      if (sfread(dh, 48, dbuff) != 48)
		continue;
	      if (!convert_dbuff(irec.call, dbuff, &drec))
		continue;
	      ct++;
	      strcpy(w1, ".");
	      if (ct % 79 == 0)
		wlnuser(unr, w1);
	      else
		wuser(unr, w1);
	      boxspoolread();

	      if (drec.pwmode > 0 || !strcmp(drec.mybbs, Console_call) ||
		  drec.level != 1) {
		load_userinfo_for_change(false, drec.call, &urec);

		urec.level = drec.level;
		urec.plevel = drec.level;
		strcpy(urec.language, drec.lan);
		urec.lastdate = drec.last;
		strcpy(urec.mybbs, drec.mybbs);
		strcpy(urec.name, drec.name);
		urec.pwmode = drec.pwmode;
		urec.mybbsupd = drec.mybbsupd;

		if (drec.pwmode > 0 || drec.level != 1) {
		  sprintf(STR1, "%sdiebox%cimp", boxprotodir, extsep);
		  append(STR1, urec.call, true);
		}

		save_userfile(&urec);

	      }

	      update_mybbsfile(false, drec.call, &drec.mybbsupd, drec.mybbs, "U");

	    }

	    ok = true;
	    sfclose(&dh);
	    wlnuser0(unr);
	    sprintf(w1, "%ld", ct);
	    strcat(w1, " user settings converted");
	    iwlnuser(unr, w1);
	  } else
	    iwlnuser(unr, "cannot open user3.dat");
	  sfclose(&idh);
	  if (idel && ok) {
	    sfdelfile(dn);
	    sfdelfile(idn);
	  }
	} else
	  iwlnuser(unr, "cannot open user3.idx");
      } else
	iwlnuser(unr, "size of user3.idx improper");
    } else
      iwlnuser(unr, "size of user3.dat improper");
  } else
    iwlnuser(unr, "user3.dat/user3.idx not found");

  wlnuser0(unr);
  if (cverr)
    iwlnuser(unr, "some errors occured in step 2");
  else
    iwlnuser(unr, "step 2 finished without errors");
  printtime(unr, get_cpuusage() - t);
}


static void conv1(short unr, char *w)
{
  /* Die Konfiguration */
  char cp[256], p[256], w1[256];
  char f[256];
  DTA dirinfo;
  short result;
  long t;
  char STR1[256], STR7[256];

  if (cverr)
    return;
  if (!check_cpath(w)) {
    iwlnuser(unr, "invalid pathname");
    return;
  }
  wlnuser0(unr);
  iwlnuser(unr, "conversion step 1");
  iwlnuser(unr, "importing box configuration files");
  t = get_cpuusage();
  sprintf(cp, "%sconfig.box", w);

  iwlnuser(unr, "now adapting config.box");
  sprintf(f, "%sconfig%cbox", boxsysdir, extsep);

  if (get_fileline(cp, 70, p)) {
    upper(p);
    strcpy(w1, ownhiername);
    if (strstr(p, Console_call) == NULL)
      sprintf(ownhiername, "%s.%s", Console_call, p);
    else
      strcpy(ownhiername, p);

    if (strcmp(w1, ownhiername)) {
      iwuser(unr, "ownhiername changed to: ");
      iwlnuser(unr, ownhiername);
      sprintf(p, "OWNHIERNAME %s", ownhiername);
      replace_keyline(f, "OWNHIERNAME ", false, p);
    }
  }

  if (get_fileline(cp, 48, p)) {
    get_word(p, w1);
    if (atol(w1) < 30000) {   /* + 10000 fuer UserBIDs */
      iwlnuser(unr,
	"original diebox-maxbull-setting is too low, will insert 40000");
      strcpy(w1, "40000");
    } else if (atol(w1) > 200000L) {
      iwlnuser(unr,
	"original diebox-maxbull-setting is too high, will insert 200000");
      strcpy(w1, "200000");
    } else
      sprintf(w1, "%ld", atol(w1) + 10000);
    sprintf(w1, "MAXBULLIDS %s", strcpy(STR7, w1));
    iwlnuser(unr, w1);
    replace_keyline(f, "MAXBULLIDS ", false, w1);
  }

  if (get_fileline(cp, 50, p)) {
    if (p[0] == '1')
      strcpy(p, "ON");
    else
      strcpy(p, "OFF");
    sprintf(p, "UFILHIDE %s", strcpy(STR7, p));
    iwlnuser(unr, p);
    replace_keyline(f, "UFILHIDE ", false, p);
  }

  if (get_fileline(cp, 51, p)) {
    get_word(p, w1);
    sprintf(p, "ERASEDELAY %s", w1);
    iwlnuser(unr, p);
    replace_keyline(f, "ERASEDELAY ", false, p);
  }

  if (get_fileline(cp, 50, p)) {
    if (p[0] == '1')
      strcpy(p, "OFF");
    else
      strcpy(p, "ON");
    sprintf(p, "WITH_RLINE %s", strcpy(STR7, p));
    iwlnuser(unr, p);
    replace_keyline(f, "WITH_RLINE ", false, p);
  }

  if (get_fileline(cp, 56, p)) {
    if (p[0] == '1')
      strcpy(p, "THEBOX");
    else
      *p = '\0';
    sprintf(p, "SYSFORWARD %s", strcpy(STR7, p));
    iwlnuser(unr, p);
    replace_keyline(f, "SYSFORWARD ", false, p);
  }

  if (get_fileline(cp, 57, p)) {
    if (p[0] == '0')
      strcpy(w1, "OFF");
    else
      strcpy(w1, "ON");
    sprintf(w1, "REMOTE_ERASE %s", strcpy(STR7, w1));
    iwlnuser(unr, w1);
    replace_keyline(f, "REMOTE_ERASE ", false, w1);
    if (p[0] > '1')
      strcpy(p, "ON");
    else
      strcpy(p, "OFF");
    sprintf(p, "REMOTEERASECHECK %s", strcpy(STR7, p));
    iwlnuser(unr, p);
    replace_keyline(f, "REMOTEERASECHECK ", false, p);
  }

  iwlnuser(unr, "config.box adapted.");


  if (!get_fileline(cp, 38, p)) {  /* pfad auf MBSYS */
    cverr = true;
    iwlnuser(unr, "err 1 - mbsys-path not found in config.box");
    return;
  }
  conv_pname_rel(w, p);
  iwlnuser(unr, "convtit");
  sprintf(STR7, "%sconvtit.box", p);
  sprintf(STR1, "%sconvtit.box", boxsysdir);
  filemove2(STR7, STR1);
  sprintf(STR7, "%sconvtit.box", boxsysdir);
  stripcr(STR7);
  iwlnuser(unr, "convlt");
  sprintf(STR7, "%sconvlt.box", p);
  sprintf(STR1, "%sconvlt.box", boxsysdir);
  filemove2(STR7, STR1);
  sprintf(STR7, "%sconvlt.box", boxsysdir);
  stripcr(STR7);
  iwlnuser(unr, "convname/transfer");
  sprintf(STR7, "%sconvname.box", p);
  sprintf(STR1, "%stransfer.box", boxsysdir);
  filemove2(STR7, STR1);
  sprintf(STR7, "%stransfer.box", boxsysdir);
  stripcr(STR7);
  iwlnuser(unr, "prvcalls");
  sprintf(STR7, "%sprvcalls.box", p);
  sprintf(STR1, "%sprvcalls.box", boxsysdir);
  filemove2(STR7, STR1);
  sprintf(STR7, "%sprvcalls.box", boxsysdir);
  stripcr(STR7);
  iwlnuser(unr, "badnames");
  sprintf(STR7, "%sbadnames.box", p);
  sprintf(STR1, "%sbadnames.box", boxsysdir);
  filemove2(STR7, STR1);
  sprintf(STR7, "%sbadnames.box", boxsysdir);
  stripcr(STR7);
  iwlnuser(unr, "lifetime.box / rubriken.box");
  sprintf(STR7, "%slifetime.box", w);
  sprintf(STR1, "%srubriken.box", boxsysdir);
  filemove2(STR7, STR1);
  sprintf(STR7, "%srubriken.box", boxsysdir);
  stripcr(STR7);

  sprintf(STR7, "%spwd%c%c%cpwd", w, dirsep, allquant, extsep);
  result = sffirst(STR7, 0, &dirinfo);
  while (result == 0) {
    iwlnuser(unr, dirinfo.d_fname);
    strcpylower(w1, dirinfo.d_fname);
    sprintf(STR7, "%spwd%c%s", w, dirsep, dirinfo.d_fname);
    sprintf(STR1, "%s%s", boxsfdir, w1);
    filemove2(STR7, STR1);
    result = sfnext(&dirinfo);
  }
  iwlnuser(unr,
    "remember to set up personal passwords for former users of PWLIST.TXT");
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


void conv_diebox(short unr, char *eingabe_)
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

  case '5':   /* BullIDs addieren  */
    conv5(unr, p);
    break;

  default:
    conv1(unr, w);
    conv2(unr, w);
    conv3(unr, w);
    conv4(unr, w);
    conv5(unr, w);
    break;
  }

  wlnuser0(unr);
  wlnuser(unr, "check logfile <proto/diebox.imp> for all performed actions.");
  wlnuser(unr, "import is ended.");
  boxisbusy(false);
  blocking_on();
}



/* ************************************************************************ */
/* *                   Ende Konvertierroutinen                            * */
/* ************************************************************************ */
