/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#define BOX_G
#include <ctype.h>
#include "main.h"
#include "init.h"
#include "box.h"
#include "boxlocal.h"
#include "boxglobl.h"
#include "box_init.h"
#include "box_garb.h"
#include "box_logs.h"
#include "tools.h"
#include "shell.h"
#include "sort.h"
#include "crc.h"
#include "box_inou.h"
#include "box_file.h"
#include "box_mem.h"
#include "box_rout.h"
#include "box_send.h"
#include "box_tim.h"
#include "boxdbox.h"
#include "boxbcm.h"
#include "box_sys.h"
#include "box_sf.h"
#include "box_sub.h"
#include "box_serv.h"
#include "boxfserv.h"
#include "box_wp.h"
#include "box_scan.h"
#include <unistd.h>
#ifdef __macos__
#include <stat.h>
#else
#include <sys/stat.h>
#endif

static void run_batch(short unr, char *cmd);

static void get_msgflags(short msgflags, char *s)
{
  *s = '\0';
  if (msgflags & MSG_SFWAIT)		strcat(s, "SFWAIT ");
  if (msgflags & MSG_PROPOSED)		strcat(s, "PROPOSED ");
  if (msgflags & MSG_SFRX)		strcat(s, "SFRX ");
  if (msgflags & MSG_SFTX)		strcat(s, "SFTX ");
  if (msgflags & MSG_SFERR)		strcat(s, "SFERR ");
  if (msgflags & MSG_SFNOFOUND)		strcat(s, "SFNOFOUND ");
  if (msgflags & MSG_CUT)		strcat(s, "MONCUT ");
  if (msgflags & MSG_MINE)		strcat(s, "MINE ");
  if (msgflags & MSG_REJECTED)		strcat(s, "REJECTED ");
  if (msgflags & MSG_DOUBLE)		strcat(s, "DOUBLE ");
  if (msgflags & MSG_BROADCAST)		strcat(s, "BROADCASTED ");
  if (msgflags & MSG_BROADCAST_RX)	strcat(s, "BROADCASTRX ");
  if (msgflags & MSG_OWNHOLD)		strcat(s, "OWNHOLD ");
  if (msgflags & MSG_LOCALHOLD)		strcat(s, "LOCALHOLD ");
  if (msgflags & MSG_SFHOLD)		strcat(s, "SFHOLD ");
  if (msgflags & MSG_OUTDATED)		strcat(s, "OUTDATED ");
}

static void get_eraseby(short flag, char *s)
{
  switch (flag) {

  case EB_NONE:
	*s = '\0';
	break;

  case EB_SENDER:
	strcpy(s, "sender");
	break;

  case EB_RECEIVER:
	strcpy(s, "recipient");
	break;

  case EB_SYSOP:
	strcpy(s, "sysop");
	break;

  case EB_RSYSOP:
	strcpy(s, "board-sysop");
	break;

  case EB_DOUBLESYSOP:
	strcpy(s, "reread of sysop");
	break;

  case EB_LIFETIME:
	strcpy(s, "lifetime");
	break;

  case EB_SF:
	strcpy(s, "s&f with");
	break;

  case EB_REMOTE:
	strcpy(s, "remote erase of");
	break;

  case EB_SFERR:
	strcpy(s, "s&f error with");
	break;

  case EB_RETMAIL:
	strcpy(s, "return mailer");
	break;

  case EB_TRANSFER:
	strcpy(s, "transfer of");
	break;

  case EB_BIN27PLUS:
	strcpy(s, "bin>7plus conversion");
	break;

  default:
	strcpy(s, "unknown");
	break;
  }
}


void add_eraseby(char *call, indexstruct *header)
{
  short	x;
  char	w2[256];
  char	STR1[256];

  if (*call == '\0')
    return;

  strcpy(w2, call);
  rspacing(w2, LEN_CALL);
  cut(w2, LEN_CALL);
  x	= strpos2(header->readby, w2, 1);
  sprintf(w2, "-%s", strcpy(STR1, w2));
  if (x > 0)
    strdelete(header->readby, x - 1, LEN_CALL+1);

  if (strlen(header->readby) < 134) {
    strcat(header->readby, w2);
    return;
  }

  x	= strpos2(header->readby, "!", 1);
  if (x > 0) {
    strdelete(header->readby, x, LEN_CALL+1);
    strcat(header->readby, w2);
  }
}


static short in_readby(char *call, char *readby)
{
  char w2[256];

  strcpy(w2, call);
  rspacing(w2, LEN_CALL);
  cut(w2, LEN_CALL);
  return strpos2(readby, w2, 1);
}


static void add_readby(short unr, indexstruct *header, boolean add_lt)
{
  short	x;
  char	flag;
  char	w2[256];
  char	STR1[256];

  if (!boxrange(unr))
    return;

  strcpy(w2, user[unr]->call);
  rspacing(w2, LEN_CALL);
  cut(w2, LEN_CALL);

  if (strstr(header->readby, w2) == NULL) {

    if (user[unr]->in_reply)
      flag = ',';
    else if (user[unr]->f_bbs)
      flag = '$';
    else if (user[unr]->supervisor)
      flag = '&';
    else if (!strcmp(user[unr]->call, header->dest))
      flag = '.';
    else
      flag = '!';

    sprintf(w2, "%c%s", flag, strcpy(STR1, w2));

    if (strlen(header->readby) < 134)
      strcat(header->readby, w2);
    else {
      x	= strpos2(header->readby, "!", 1);
      if (x > 0) {
	strdelete(header->readby, x, LEN_CALL+1);
	strcat(header->readby, w2);
      }
    }

  }

  if (user[unr]->in_reply)
    return;

  if (user[unr]->f_bbs || user[unr]->supervisor || user[unr]->rsysop)
    return;

  header->readcount++;
  header->lastread	= clock_.ixtime;

  if (add_lt) {
    if (header->lifetime > 0 && header->lifetime < max_lt_inc)
      header->lifetime++;
  }

}

static void disp_mulsearch(mulsearchtype *root)
{
  mulsearchtype	*hp, *hp2;
  
  hp	= root;
  while (hp != NULL) {
    hp2	= hp;
    hp	= hp->next;
    free(hp2);
  }
}

static mulsearchtype *gen_mulsearch(short threshold, char *search)
{
  mulsearchtype		*root, *hp, *hp2;
  char			hs[256], w[256];
  
  root			= NULL;
  strcpy(hs, search);
  while (*hs != '\0') {
    get_word(hs, w);
    if (*w != '\0') {
      hp		= malloc(sizeof(mulsearchtype));
      if (hp == NULL) {
        disp_mulsearch(root);
        return NULL;
      }

      if (threshold < 100)
        PrepareFuzzySearch(w, hp->wort, &hp->len, &hp->fn1, &hp->fn2);
      else {
        upper(w);
        strcpy(hp->wort, w);
        hp->len		= strlen(w);
        hp->fn1		= -1;
        hp->fn2		= -1;
      }
      hp->threshold	= threshold;
      hp->next		= NULL;
      if (root == NULL)
        root		= hp;
      else {
        hp2		= root;
        while (hp2->next != NULL)
          hp2		= hp2->next;
        hp2->next	= hp;
      }
    }
  }
  return root;
}


static boolean check_search(boolean bidsearch, boolean fwdlist,
			    boolean wildcardsearch, mulsearchtype *search,
			    indexstruct header)
{
  char	hs[256], w[256];

  if (search == NULL) return false;
  
  if (wildcardsearch || search->threshold >= 100) {
    strcpyupper(hs, header.betreff);
  }
  
  w[0]		= '\0';
  
  while (search != NULL) {
    if (wildcardsearch) {
      if (!(wildcardcompare(SHORT_MAX, search->wort, header.absender, w) ||
	   wildcardcompare(SHORT_MAX, search->wort, hs, w) ||
	   (bidsearch && wildcardcompare(SHORT_MAX, search->wort, header.id, w)) ||
	   (fwdlist &&
	    (wildcardcompare(SHORT_MAX, search->wort, header.dest, w) ||
	     wildcardcompare(SHORT_MAX, search->wort, header.verbreitung, w)))))
	return false;
    } else {
      if (search->threshold < 100) {
        if ((FuzzyMatching(header.betreff, search->wort, search->len, search->fn1, search->fn2)
             < search->threshold)
           && (FuzzyMatching(header.absender, search->wort, search->len, search->fn1,
               search->fn2) < search->threshold)
           && (!bidsearch || FuzzyMatching(header.id, search->wort, search->len,
               search->fn1, search->fn2) < search->threshold))
          return false;
      } else {
        if (!(strstr(header.absender, search->wort) !=NULL ||
	     strstr(hs, search->wort) != NULL ||
	     (bidsearch && strstr(header.id, search->wort) != NULL) ||
	     (fwdlist && (strstr(header.dest, search->wort) != NULL ||
		    strstr(header.verbreitung, search->wort) != NULL))))
	  return false;
      }
    }
    search	= search->next;
  }
  return true;
}

static boolean check_logsearch(boolean bidsearch,
			       boolean wildcardsearch, mulsearchtype *search,
			       boxlogstruct *header)
{
  char	hs[256], w[256];

  if (search == NULL) return false;
  
  if (wildcardsearch || search->threshold >= 100) {
    strcpyupper(hs, header->betreff);
  }
  
  w[0]		= '\0';
  
  while (search != NULL) {
    if (wildcardsearch) {
      if (!(wildcardcompare(SHORT_MAX, search->wort, header->absender, w) ||
	   wildcardcompare(SHORT_MAX, search->wort, hs, w) ||
	   (bidsearch && wildcardcompare(SHORT_MAX, search->wort, header->bid, w))))
	return false;
    } else {
      if (search->threshold < 100) {
        if ((FuzzyMatching(header->betreff, search->wort, search->len, search->fn1, search->fn2)
             < search->threshold)
           && (FuzzyMatching(header->absender, search->wort, search->len, search->fn1,
               search->fn2) < search->threshold)
           && (!bidsearch || FuzzyMatching(header->bid, search->wort, search->len,
               search->fn1, search->fn2) < search->threshold))
          return false;
      } else {
        if (!(strstr(header->absender, search->wort) != NULL ||
	     strstr(hs, search->wort) != NULL ||
	     (bidsearch && strstr(header->bid, search->wort) != NULL)))
	  return false;
      }
    }
    search	= search->next;
  }
  return true;
}


void signalise_readlimit(short unr, boolean today,
			 char *name, short von, short bis)
{
  char	hs[256];

  if (!user[unr]->smode) {
    wlnuser0(unr);
    show_prompt(unr);
    wlnuser0(unr);
  }
  if (today) {
    w_btext(unr, 74);
    sprintf(hs, " %ld \007", user[unr]->maxread_day >> 10);
    wlnuser(unr, hs);
  }
  if (user[unr]->smode)
    return;

  sprintf(hs, " R %s %d", name, von);
  if (von < bis) {
    sprintf(hs + strlen(hs), "-%d", bis);
  }
  strcat(hs, " \007");
  w_btext(unr, 75);
  wlnuser(unr, hs);
  debug(2, unr, 41, hs);
  wlnuser(unr, hs);
}


static long calc_readers_size(boolean is_call, indexstruct header)
{
  if (is_call) {
    if (*header.readby != '\0')
      return (strlen(header.readby) + 9);
    else
      return 0;
  } else
    return 0;
}


static void show_readers(short unr, boolean is_call, unsigned short rc,
			 char *r, boolean all)
{
  boolean	sv;
  short		x;
  char		w[LEN_CALL*2];
  char		hs[256];

  if (!boxrange(unr))
    return;

  sv		= user[unr]->supervisor;
  if (is_call || (all && sv) || (all && show_readers_to_everyone)) {
    if (rc > 0) {
      sprintf(hs, "Read: %d ", rc);
    } else
      strcpy(hs, "Read: ");
    x		= 1;
    while (x < strlen(r)) {
      strsub(w, r, x, LEN_CALL+1);
      x		+= LEN_CALL+1;
      if (sv || w[0] == '!' || w[0] == '.' || w[0] == ',') {
	strdelete(w, 1, 1);
	sprintf(hs + strlen(hs), "%s ", w);
      }
      if (strlen(hs) > 72) {
	wlnuser(unr, hs);
	strcpy(hs, "Read: ");
      }
    }
    if (strlen(hs) > LEN_CALL)
      wlnuser(unr, hs);
    return;
  }

  if (rc <= 0)
    return;

  sprintf(hs, "Read: %d", rc);
  wlnuser(unr, hs);
}


void add_readday(short unr, long count)
{
  short	x;

  if (count <= 0)
    return;

  for (x = 1; x <= MAXUSER; x++) {
    if (user[x] != NULL) {
      if (!strcmp(user[x]->call, user[unr]->call))
	user[x]->read_today	+= count;
    }
  }
}


static unsigned short ttl_lifetime(short unr, unsigned short lt, long rx)
{
  short	ttl;

  if (boxrange(unr)) {
    if (user[unr]->ttl) {
      if (lt == 0) {
	ttl	= 0;
	return ((unsigned short)ttl);
      }
      ttl	= (short)lt - (short)((clock_.ixtime - rx) / SECPERDAY);
      if (ttl < 1)
	ttl	= 1;
      return ((unsigned short)ttl);
    } else
      return lt;
  } else
    return lt;
}


/* ---------------------------------------------------------------------------------------- */

short search_by_bid(char *brett, char *bid, boolean hidden)
{
  short		k, mct, lv;
  boolean	hit;
  long		isize;
  indexstruct	header;
  pathstr	STR1;

  debug(3, 0, 36, bid);
  if (strlen(bid) >= LEN_BID+1)
    return 0;

  hit	= false;
  idxfname(STR1, brett);
  isize	= sfsize(STR1);
  if (isize <= 0)
    return 0;

  lv	= isize / sizeof(indexstruct);
  k	= open_index(brett, FO_READ, true, true);
  if (k < minhandle)
    return 0;
  
  mct	= 1;
  while (mct <= lv && hit == false) {
    read_index(k, mct, &header);
    if (!check_hcs(header) || strcmp(header.id, bid) ||
      header.deleted != hidden)
      mct++;
    else
      hit = true;
  }

  close_index(&k);
  if (hit)
    return mct;
  else
    return 0;
}

#define cpentries       CHECKBLOCKSIZE
#define cpsize          (cpentries * sizeof(boxlogstruct))

boolean erase_by_bid(boolean reread, char *sbid_, char *eraseabsender_)
{
  boolean	hit, double_;
  short		log, x, list;
  long		cpstart, seekpos, lognr, lastlog;
  boxlogstruct	logheader, *logptr;
  indexstruct	header;
  char		*ipuffer;
  bidtype	bid;
  calltype	eraseabsender;
  char		sbid[256], w[256];
  pathstr	hs;

  debug(3, 0, 37, sbid_);

  strcpy(sbid, sbid_);
  strcpy(eraseabsender, eraseabsender_);
  double_	= false;

  x		= strlen(sbid);
  if (x < 1 || x > LEN_BID+1) /* + 1 weil auch führendes $ akzeptiert wird */
    return false;

  if (sbid[0] == '$') {
    if (x < 2)
      return false;

    strsub(bid, sbid, 2, x - 1);
  } else if (x <= LEN_BID)
    strcpy(bid, sbid);
  else
    return false;

  /* (not(reread)) ist ein flag fuer remote erase */

  if (bull_mem(bid, false) < 0)
    return (!double_);

  boxprotokoll("erase by bid");
  hit		= false;
  lastlog	= sfsize(boxlog) / sizeof(boxlogstruct);
  lognr		= lastlog;
  log		= sfopen(boxlog, FO_RW);
  cpstart	= 0;
  ipuffer	= malloc(cpsize);

  if (log >= minhandle && lastlog > 0) {
    do {
      if ((lastlog & 127) == 0)   /*Watchdog resetten*/
	dp_watchdog(2, 4711);

      if (ipuffer == NULL) {
	read_log(log, lognr, &logheader);
	logptr		= &logheader;
      } else {
	if (lognr < cpstart || cpstart == 0) {
	  if (cpstart == 0)
	    cpstart	= lastlog - cpentries + 1;
	  else
	    cpstart	-= cpentries;
	  if (cpstart <= 1)
	    cpstart	= 1;
	  seekpos	= (cpstart - 1) * sizeof(boxlogstruct);
	  if (sfseek(seekpos, log, SFSEEKSET) != seekpos)
	    break;
	  if (sfread(log, cpsize, ipuffer) <= 0)
	    break;
	  if (lognr < cpstart)
	    break;
	}
	logptr		= (boxlogstruct *)(&ipuffer[(lognr - cpstart) * sizeof(boxlogstruct)]);
      }

      if (!logptr->deleted) {
	if (!strcmp(logptr->bid, bid)) {
	  idxfname(hs, logptr->brett);
	  list		= sfopen(hs, FO_RW);
	  if (list >= minhandle) {
	    read_index(list, logptr->idxnr, &header);
	    if (!strcmp(header.id, bid) && check_hcs(header)) {
	      if ((reread && (header.msgflags & MSG_CUT) != 0) || !reread) {
		hit			= true;
		header.deleted		= true;
		header.erasetime	= clock_.ixtime;
		if (!reread) {
		  if (!exist(r_erase_box)) {
		    strcpy(hs,
		      "Date  BID          Board      Nr. Subject                  By     Erase-BID");
		    append(r_erase_box, hs, true);
		    *hs			= '\0';
		    append(r_erase_box, hs, true);
		  }
		  ix2string(clock_.ixtime, w);
		  cut(w, 5);
		  sprintf(hs, "%s %-*s %-*s %5d %s", w, LEN_BID, bid, LEN_BOARD, logptr->brett,
							logptr->idxnr, header.betreff);
		  cut(hs, 58);
		  rspacing(hs, 59);
		  append(r_erase_box, hs, false);
		  header.eraseby	= EB_REMOTE;
		  add_eraseby(eraseabsender, &header);
		} else {
		  bull_mem(bid, true);
		  header.eraseby	= EB_DOUBLESYSOP;
		}
		write_index(list, logptr->idxnr, &header);
		logptr->deleted		= true;
		write_log(log, lognr, logptr);
	      }
	      if ((reread && (header.msgflags & MSG_CUT) == 0) || !reread)
		double_		= true;
	    }

	    sfclose(&list);
	  }
	}
      }
      lognr--;
    } while (!(lognr <= 0 || hit));
  }
  
  sfclose(&log);

  if (ipuffer != NULL)
    free(ipuffer);

  return (!double_);
}

#undef cpentries
#undef cpsize


void check_remote_erase(long *seekp)
{
  short	k;
  long	tc;
  char	hs[256], absender[256], id1[256], rxfrom[256], id2[256];

  tc			= get_cpuusage();
  k			= sfopen(temperase, FO_READ);
  if (k >= minhandle) {
    if (sfseek(*seekp, k, SFSEEKSET) == *seekp) {
      do {
	if (file2str(k, hs)) {
	  *seekp	+= strlen(hs) + 1;
	  get_word(hs, absender);
	  cut(absender, LEN_CALL);
	  get_word(hs, id1);
	  get_word(hs, rxfrom);
	  get_word(hs, id2);
	  if (!erase_by_bid(false, id2, absender)) {
	    sprintf(hs, "%-*s %-*s", LEN_CALL, absender, LEN_BID, id1);
	    append(r_erase_box, hs, true);
	  }
	} else
	  *seekp	= 0;
      } while (*seekp != 0 && get_cpuusage() - tc < ttask);
    } else
      *seekp		= 0;
    sfclose(&k);
  } else
    *seekp		= 0;

  if (*seekp == 0) {
    sfdelfile(temperase);
    new_remote_erases	= false;
  }
}


void add_remote_erase(char *absender, char *id1, char *rxfrom, char *id2)
{
  char	hs[256];

  if (strlen(id2) > LEN_BID+1) /* + 1 wegen führendem $ */
    return;

  sprintf(hs, "%s %s %s %s", absender, id1, rxfrom, id2);
  append(temperase, hs, true);
  new_remote_erases	= true;
}

static void create_erase_message(char *erasebid, char *mbx)
{
  char	hs[256];
  
  snprintf(hs, 255, "E @ %s < %s $%s %s", mbx, Console_call, WPDUMMYBID, erasebid);
  sf_rx_emt1(hs, Console_call);
}


/* wird nur fuer X und T verwendet: */

void delete_brett_by_bid(char *brett, char *sfcall, char *bid, boolean release, boolean hidden)
{
  short		list, k, lv;
  long		TotalSize;
  char		whatc;
  indexstruct	header, *rpointer;
  pathstr	fname;

  debug(4, 0, 39, bid);
  
  if (!strcmp(brett, "X")) {
    if (release)
      whatc	= 'R';
    else if (hidden)
      whatc	= 'E';
    else
      whatc	= 'U';
    alter_fwd(whatc, bid, 0, "", sfcall);
    return;
  }

  idxfname(fname, brett);  
  if (!exist(fname))
    return;
    
  TotalSize	= sfsize(fname);
  lv		= TotalSize / sizeof(indexstruct);
  list		= nohandle;
  rpointer	= &header;   /*bleibt ggf. gleich*/
  list		= open_index(brett, FO_RW, true, true);

  if (list < minhandle)
    return;
  
  k		= 1;
  while (k <= lv) {
    read_index(list, k, &header);
    if (!strcmp(rpointer->id, bid) && check_hcs(*rpointer)) {
      if (release) {
	rpointer->msgflags	&= ~(MSG_SFHOLD | MSG_LOCALHOLD);
	alter_log(true, rpointer->msgnum, rpointer->msgflags, '!', "");
      } else {
	rpointer->deleted	= hidden;
	if (rpointer->deleted)
	  whatc			= 'E';
	else
	  whatc			= 'U';
	alter_log(true, rpointer->msgnum, rpointer->msgflags, whatc, "");
      }
      write_index(list, k, rpointer);
    }
    k++;
  }
  close_index(&list);
}

static void zeroize_content(indexstruct *header, char *brett)
{
#define zerosize 1000
  short handle;
  long ct, size;
  char zero[zerosize];
  char hs[256];
  
  inffname(hs, brett);  
  if ((handle = sfopen(hs, FO_RW)) < minhandle) return;
  if (sfseek(header->start, handle, SFSEEKSET) != header->start) {
    sfclose(&handle);
    return;
  }
  memset(zero, 0, zerosize);
  ct = header->packsize;
  size = zerosize;
  while (ct > 0) {
    if (size > ct) size = ct;
    if (sfwrite(handle, size, zero) != size) break;
    ct -= size;
  }
  sfclose(&handle);
  memset(header->betreff, 0, LEN_SUBJECT);
  memset(header->absender, 0, LEN_CALL);
/*  memset(header->id, 0, LEN_MSGID); */
  header->firstbyte = 0;
  header->size = header->packsize;
  header->pmode = 0;
  header->infochecksum = 0;
#undef zerosize
}

static void erase_brett(short unr, char *name_, short von, short bis, short threshold,
			char *option_, char *search_, boolean release, boolean kill)
{
  short		lv, k, x, list;
  long		ect;
  char		whatc;
  boolean	valid, e_mode, bidsearch, inrboards, loginsearch, noperm, wcsearch;
  mulsearchtype	*mst;
  indexstruct	header;
  boardtype	name;
  char		option[256], search[256], w[256];

  debug(2, unr, 40, name_);

  strcpy(name, name_);
  strcpy(option, option_);
  strcpy(search, search_);
  if (!boxrange(unr))
    return;
  
  if (!strcmp(name, "X"))
    recompile_fwd();

  noperm		= false;
  loginsearch		= (strchr(option, '!') != NULL);
  if (loginsearch) {
    if (release || callsign(name)) {  /*loginsearch nur bei usermails*/
      von		= 1;
      bis		= SHORT_MAX;
    } else {
      loginsearch	= false;
      von		= 0;
      bis		= 0;
    }
  }

  /* delete last read message */
  if (strchr(option, ')') != NULL) {
    strcpy(name, user[unr]->reply_brett);
    von = bis 	      	= user[unr]->reply_nr;
    loginsearch       	= false;
    *search   	      	= '\0';
  }
  
  if (von == 0 || bis == 0) {
    wln_btext(unr, 37);
    return;
  }

  if (user[unr]->lastprocnumber > 0) {
    von			= user[unr]->lastprocnumber;
    ect			= user[unr]->lastprocnumber3;
    user[unr]->lastprocnumber = 0;
    lv			= user[unr]->lastprocnumber2;
    user[unr]->lastprocnumber2 = 0;
  } else {
    ect			= 0;
    lv	      	      	= 0;
  }
  
  if (lv == 0) lv     	= last_valid(unr, name);

  list			= open_index(name, FO_RW, true, true);
  if (list >= minhandle && lv > 0) {
    bidsearch		= (strchr(option, '$') != NULL);
    inrboards		= may_sysaccess(unr, name);
    e_mode		= ((strchr(option, '+') != NULL) && user[unr]->supervisor);
				/* remote sysops may not */

    if (bis > lv) bis = lv;

    if (create_syslog && ect == 0 && inrboards && strcmp(name, user[unr]->call))
      append_syslog(unr);

    wcsearch		= (*search != '\0' && strchr(search, '*') != NULL);
    mst			= gen_mulsearch(threshold, search);

    for (k = von; k <= bis; k++) {
      if (k != von && ect > 0 && get_cpuusage() - user[unr]->cputime >= ttask) {
	user[unr]->lastprocnumber	= k;
	user[unr]->lastprocnumber2	= lv;
	user[unr]->lastprocnumber3	= ect;
	break;
      }

      if (k % 10 == 0) dp_watchdog(2, 4711);

      read_index(list, k, &header);
      if ((!strcmp(header.absender, user[unr]->call) ||
	   !strcmp(name, user[unr]->call) || inrboards)
	  && (check_hcs(header) || kill)) {
	if (header.deleted == user[unr]->hidden) {
	  if (loginsearch)
	    valid			= (header.rxdate > user[unr]->lastdate);
	  else if (mst != NULL)
	    valid			= check_search(bidsearch, false, wcsearch, mst, header);
	  else
	    valid			= true;

	  if (valid) {
	    if (release) {
	      header.msgflags		&= ~(MSG_SFHOLD | MSG_LOCALHOLD);
	      alter_log(true, header.msgnum, header.msgflags, '!', "");
	      strcpy(w, "X");
	      delete_brett_by_bid(w, "", header.id, true, false);
	    } else {
	      header.deleted		= !user[unr]->hidden;

	      if (header.deleted) {
	        if (kill) {
		  header.erasetime = clock_.ixtime - erasewait;
		  zeroize_content(&header, name);
		} else header.erasetime	= clock_.ixtime;
		if (!strcmp(name, user[unr]->call))
		  header.eraseby	= EB_RECEIVER;
		else if (!strcmp(header.absender, user[unr]->call))
		  header.eraseby	= EB_SENDER;
		else if (user[unr]->supervisor)
		  header.eraseby	= EB_SYSOP;
		else if (inrboards)
		  header.eraseby	= EB_RSYSOP;
		else
		  header.eraseby	= EB_NONE;
		add_eraseby(user[unr]->call, &header);

		if (!strcmp(name, "M"))
		  clear_uf_cache(header.absender);

	      } else {  /* unerase */
	        if (user[unr]->supervisor
	           || header.eraseby == EB_RECEIVER
	           || header.eraseby == EB_SF) {
		  header.erasetime	= 0;
		  header.eraseby	= EB_NONE;
		  x			= 1;
		  while (x > 0) {
		    x			= strpos2(header.readby, "-", 1);
		    if (x > 0)
		      strdelete(header.readby, x, LEN_CALL+1);
		  }
		}
	      }
	    }

	    write_index(list, k, &header);

	    if (!release && strcmp(name, "M") && strcmp(name, "X")) {
	      /*   unhpath(header.verbreitung,hmbx); */
	      if (header.deleted) {
		if (kill) whatc = 'K';
		else whatc	= 'E';
	      } else
		whatc	= 'U';
	      alter_log(true, header.msgnum, header.msgflags, whatc, "");

/*	      if (kill) {
		bull_mem(header.id, true);
	      } */

	      if (header.deleted && *e_m_verteiler != '\0'
		  && (!strcmp(header.absender, user[unr]->call) || e_mode)
		  && !callsign(header.dest)
		  && (header.fwdct > 0 || *header.rxfrom != '\0' || e_mode)
		  && (user[unr]->is_authentic || holddelay == 0)
		  && !user[unr]->hidden)
	      {
		create_erase_message(header.id, e_m_verteiler);
		if (e_mode && ect == 0)
		  wln_btext(unr, 76);
	      }

	      if (header.deleted && *pacsrv_verteiler != '\0')
		create_erase_message(header.id, pacsrv_verteiler);

	      strcpy(w, "X");
	      if (header.fwdct == 0 || header.deleted)
		delete_brett_by_bid(w, "", header.id, false, header.deleted);

	    }
	    ect++;
	  }
	}
      } else
	noperm	= true;
    }

    close_index(&list);
    disp_mulsearch(mst);

    if (user[unr]->lastprocnumber != 0)
      return;

    if (ect > 0) {
      if (release) {
	w_btext(unr, 77);
      } else {
        if (user[unr]->hidden)
	  w_btext(unr, 78);
        else
	  w_btext(unr, 18);
      }
      if (ect == 1)
        sprintf(w, " (%ld message)", ect);
      else
        sprintf(w, " (%ld messages)", ect);
      wlnuser(unr, w);
      return;
    }

    if (noperm)
      wln_btext(unr, 7);
    else
      wln_btext(unr, 37);
    return;
  }

  close_index(&list);
  if (lv == 0)
    no_files(unr, name);
  else if (lv == -1)
    wln_btext(unr, 147);
}


static void release_hold(short unr, char *brett, short s, short e, char *option, char *search)
{
  erase_brett(unr, brett, s, e, 100, option, search, true, false);
}


static void unerase_users_mails(short unr)
{
  boolean	hstat;
  char		ds[256];

  if (!boxrange(unr))
    return;

  *ds			= '\0';
  hstat			= user[unr]->hidden;
  user[unr]->hidden	= true;
  erase_brett(unr, user[unr]->call, 1, SHORT_MAX, 100, ds, ds, false, false);
  user[unr]->hidden	= hstat;
}


/********************************************************************************/
/* This is the read function for DPBOX. Everything that has to be read out of	*/
/* the message base is processed here.						*/
/*										*/
/* blocksize should count at least as long as the expected header size of mails	*/
/* (all that is in a mail before the mail text itself starts = before the first	*/
/* empty line). 16K should be more than enough. A message is only split in 	*/
/* chunks of blocksize length if the host system does not provide enough 	*/
/* (virtual) memory for a readout in one block. Running unixes, this will never	*/
/* happen and complicates the functions algorithm, but, on the other hand, even	*/
/* on unixes, this allows real hughe message sizes getting processed (that	*/
/* won't occur in reality, but having a fallback is nice, or?).			*/
/*										*/
/* maxrcount is the amount of messages that should be read in one turn (if the	*/
/* user specified a range of messages to be read). After maxrcount read		*/
/* messages, the processing stops and the next user is shifted in the execution	*/
/* chain (at the main control loop). After a complete rowing of all users, the	*/
/* first one (whose execution was aborted) is processed again. This process is	*/
/* transparent to the user (what, in other words, dear german readers, means, 	*/
/* er bekommt das nicht mit).							*/
/*										*/
/********************************************************************************/
/*										*/
/* For a while I have been thinking of implementing real thread functionality	*/
/* in DPBOX. Remember that DPBOX first was written on a single tasking OS	*/
/* and ported to Linux before Threading was supported on Linux. I know about	*/
/* the concepts used in Baycom-BBS, FBB-BBS and AA4RE-BBS (possibly also good	*/
/* ole W0RLI bbs, but I have not seen the source code) of implementing		*/
/* something like threading by manually switching the context of the processor	*/
/* (by assembly language inlines swapping the processor registers) without	*/
/* support of the OS. I doubt they were running on RISC processors, 68k systems	*/
/* and the more. As I started programming in the Z80 and 68k domain, the Intel	*/
/* intrinsics are highly suspectible to me (of course a matter of personal	*/
/* choice). I never went into the details of Intel processors and won't in the	*/
/* future (anyone still to remember the wonderful linear assembly instruction	*/
/* set of the 68000? THAT was a revolution!).					*/
/*										*/
/* Today I am happy I never cared about those machine details for DPBOX:	*/
/*    	      	      	      	      	      	      	      	      	      	*/
/*  - todays processors are so fast that all those low-level approaches		*/
/*     are needless and only complicate porting of the code			*/
/*    	      	      	      	      	      	      	      	      	      	*/
/*  - it urged me to make DPBOX so dammned fast that it never needed context	*/
/*     switching (even not on 68k with 8Mhz). For todays processors, that means	*/
/*     you will never notice any remarkable system load by DPBOX. It should	*/
/*     always stay near 0%							*/
/*										*/
/* There is also a drawback: if you did not understand the underlying		*/
/* concept of the "pseudo" multitasking, the code is harder to read and		*/
/* modify. The basic approach is as follows:					*/
/*										*/
/*  - all processing that may need a noticeable amount of time is made		*/
/*     interruptible. In almost all cases this means that loops, such as in	*/
/*     the read command, are stopped after a specific amount of time has	*/
/*     passed by								*/
/*										*/
/* - the current state of the stopped function is stored in some fancy		*/
/*     variables of the users userstruct processing that function. The		*/
/*     variables are named lastproc****						*/
/*										*/
/* - the current (interrupted) command is written back into the command stack	*/
/*     of that user								*/
/*										*/
/* - after processing outstanding requests of other users, the command is	*/
/*     fetched back from the stack. But because the lastprocxxx variables	*/
/*     are set, the corresponding function is not started again from scratch	*/
/*     but from a defined starting point (usually the point where the		*/
/*     interruption occured)							*/
/*										*/
/* - this means that each function that might last longer than desireable	*/
/*     needs some extra code, as there is no implicit context switching		*/
/*										*/
/* - compared with the extend of DPBOX, only some 0.1% of the code need		*/
/*     that "pseudo" multitasking. Everything else is done one shot		*/
/*										*/
/* Of course, this concept needs a state machine. Writing a file to a user	*/
/* very seldom is done by a simple 						*/
/*										*/
/*         while (!eof(handle)) {						*/
/*		read(handle, buf, size);					*/
/*              while (output_stalled(out)) sleep(1);		      	      	*/
/*		write(out, buf, size);					      	*/
/*         }									*/
/*										*/
/* but usually in several steps, respecting the extreme slow output conditions	*/
/* over radio networks. Doing that in real multitasking would be much easier	*/
/* and, as the above example shows, simplify the code extremly.			*/
/*										*/
/********************************************************************************/
/*										*/
/* As this function is used for each kind of read access to the messages,	*/
/* several output conditions have to be differentiated.				*/
/* A read request could get issued by a user, the forward machine, the	      	*/
/* pacsat broadcast loop, an export request of the sysop; still more?		*/
/* Output format varies.							*/
/*										*/
/********************************************************************************/

#define blocksize       16384
#define maxrcount       5

unsigned short read_brett(short unr, short outch, char *board, short von,
			  short bis, short threshold, char *option, char *search,
			  long sfoffset, indexstruct *header)

  /* returns bodychecksum (for pacsat style broadcast)	*/

{
  short			outchan, ct, bbsmode, ki, kx, ka, ko, unpackerr, lv, i, rcount;
  boolean		ok, sf, sf_pack, valid, sf_ishuff, rheader, loginsearch;
  boolean		is_binary, bcast, is_call, export, crawler, hidflag;
  boolean		add_lt, inc_read, fill_readlog, bidsearch, check_readday;
  boolean		with_readers, only_headers, ack_msg, none_displayed;
  boolean		check_rcount, readday_ok, wcsearch, ownboard, tpksf;
  long			hl, bct, rs, hsize, fsize, nsize, bstart, err;
  unsigned short	bodychecksum, ics;
  mulsearchtype		*mst;
  userstruct		*WITH, uf;
  char			fbyte;
  char			*copybuf, *nmem, *rp;
  pathstr		indexname, infoname, temp5, temp4, temp3, temp2, temp1;
  char			status[256], hs[256], srline[256], STR1[256];


  debug(2, unr, 42, board);

  none_displayed	= true;
  fsize			= 0;
  outchan		= outch;
  rcount		= 0;
  bodychecksum		= 0;
  is_call		= callsign(board);

  rheader		= (strchr(option, '+') != NULL);
  bcast			= (strchr(option, 'B') != NULL);
  crawler		= (strchr(option, 'C') != NULL);
  bidsearch		= (strchr(option, '$') != NULL);
  with_readers		= (strchr(option, ':') != NULL);
  only_headers		= (strchr(option, '=') != NULL);
  tpksf			= (strchr(option, 'T') != NULL && !bcast);
  wcsearch		= (strchr(search, '*') != NULL);

  mst			= gen_mulsearch(threshold, search);

  hidflag		= (!bcast && user[unr]->hidden);
  export		= (!bcast && user[unr]->action == 76);
  sf			= (!export && !bcast && user[unr]->f_bbs);

  if (!bcast && sf)
    bbsmode		= user[unr]->fwdmode;
  else
    bbsmode		= 0;

  sf_pack		= (!bcast && bbsmode > 0 && sf);

  if (tpksf) {
    if (user[unr]->fwdmode == 0) user[unr]->fwdmode = 1;
    bbsmode		= user[unr]->fwdmode;
    sf_pack		= true;
  }

  /* to suppress the sent data in the atari version connection window: */

  if (sf || tpksf) boxsetboxbin(user[unr]->pchan, true);

  loginsearch		= (!bcast && !sf && !tpksf && strchr(option, '!') != NULL);
  if (loginsearch) {
    von			= 1;
    bis			= SHORT_MAX;
  }

  check_rcount 	= (boxrange(unr) && !bcast && !export && outch < minhandle && (!sf || tpksf));
  fill_readlog	= (check_rcount && !user[unr]->console && !only_headers);
  check_readday	= (fill_readlog && !user[unr]->supervisor && !(user[unr]->rsysop
		   && in_rsysops_boards(user[unr]->call, board)));
  fill_readlog	= (fill_readlog && create_readlog);
  ownboard	= (export || (is_call && unr > 0 && !strcmp(board, user[unr]->call)));
  inc_read	= (unr > 0 && !bcast && !export && !only_headers
		   && (!user[unr]->console || !strcmp(user[unr]->call, board)));
  add_lt	= (inc_read && max_lt_inc > 0 && (!sf || tpksf) && !is_call);

  uf.readlock	= 0;
  if (unr > 0 && !bcast && !export && (!sf || tpksf) && is_call) {
    load_userfile(true, false, board, &uf);
    if (*uf.call == '\0')
      uf.readlock = 2;
  }

  if (check_rcount) {
    if (user[unr]->lastprocnumber > 0) {
      von	= user[unr]->lastprocnumber;
      user[unr]->lastprocnumber = 0;
      none_displayed = false;
    }
  }

  if (*board == '\0' || von <= 0 || bis <= 0)
    return bodychecksum;

  if (strlen(board) == 1 && (board[0] == 'M' || board[0] == 'X'))
    return bodychecksum;

  idxfname(indexname, board);
  inffname(infoname, board);

  if ((sf && !tpksf) || bcast || export)
    lv		= sfsize(indexname) / sizeof(indexstruct);
  else if (check_rcount) {
    lv		= user[unr]->lastprocnumber2;
    user[unr]->lastprocnumber2 = 0;
    if (lv == 0)
      lv	= last_valid(unr, board);
  } else
    lv		= last_valid(unr, board);

  rp		= NULL;

  if (lv > 0) {
    sprintf(temp1, "%sINFO%cR", tempdir, extsep);
    sprintf(temp2, "%s2", temp1);
    sprintf(temp4, "%s4", temp1);
    sprintf(temp5, "%s5", temp1);
    strcat(temp1, "1");

    if (inc_read)
      kx	= open_index(board, FO_RW, loginsearch || *search != '\0', true);
    else
      kx	= open_index(board, FO_READ, loginsearch || *search != '\0', true);
  } else
    kx 		= nohandle;


  if (kx >= minhandle && lv > 0) {

    ki		= sfopen(infoname, FO_READ);
    if (ki >= minhandle) {

      if (bis > lv) bis = lv;

      for (ct = von; ct <= bis; ct++) {
	fbyte	= 0;

	if (check_rcount) {
	  if (!none_displayed
	      && (rcount >= maxrcount
		|| boxspoolstatus(user[unr]->tcon, user[unr]->pchan, -1) >= MAXSPOOLBYTES
		|| get_cpuusage() - user[unr]->cputime >= ttask * 4)) {

	    user[unr]->lastprocnumber	= ct;
	    user[unr]->lastprocnumber2	= lv;
	    break;
	  }
	}

	if (check_readday)
	  readday_ok	= ((user[unr]->maxread_day == 0 && all_maxread_day > user[unr]->read_today)
			   || user[unr]->maxread_day > user[unr]->read_today);
	else
	  readday_ok	= true;

	if (unr > 0 && !bcast && !export && (!user[unr]->f_bbs || tpksf)
	    && !user[unr]->console && !user[unr]->supervisor && !readday_ok) {
	  signalise_readlimit(unr, !readday_ok, board, ct, bis);
	  none_displayed = false;
	  break;
	}

	dp_watchdog(2, 4711);   /* Watchdog resetten */
	read_index(kx, ct, header);

	if ((header->deleted == hidflag || (sf && !tpksf) || bcast) && check_hcs(*header)) {
	  if (bcast || sf)
	    valid	= true;
	  else if (loginsearch) {
	    valid	= header->rxdate > user[unr]->lastdate;
	    if (mst != NULL) {
	      if (valid)
		valid	= check_search(bidsearch, false, wcsearch, mst, *header);
	    }
	  } else if (mst != NULL)
	    valid	= check_search(bidsearch, false, wcsearch, mst, *header);
	  else
	    valid	= true;

	  if (valid && (is_call || header->msgtype != 'B') && unr > 0
	      && !user[unr]->supervisor && (!sf || tpksf) && !bcast && !export) {

	    unhpath(header->verbreitung, hs);

	    if (ufilhide || uf.readlock == 2 || strcmp(hs, Console_call))
	      valid	= (   ownboard
			   || !strcmp(header->absender, user[unr]->call)
			   || !strcmp(header->dest, user[unr]->call) );
	    else if (uf.readlock == 1)
	      valid	= (   ownboard
			   || !strcmp(header->absender, user[unr]->call)
			   || !strcmp(header->dest, user[unr]->call)
			   || in_readby(board, header->readby) > 0);
	  }

	  if (valid) {

	    if (check_rcount) rcount++;
	    none_displayed	= false;

	    if (check_readday) add_readday(unr, header->size);

	    if (fill_readlog) {
	      sprintf(hs, "%s %d <%s $%s", board, ct, header->absender, header->id);
	      append_readlog(unr, hs);
	    }

	    if (inc_read) {
	      add_readby(unr, header, add_lt);
	      write_index(kx, ct, header);
	    }


	    is_binary	= (header->pmode & TBIN) != 0;
	    ack_msg	= header->msgtype == 'A';

	    if (!bcast) {
	      if (!sf && !(export && is_binary)) {
		if (boxrange(unr)) {
		  WITH		= user[unr];
		  strcpy(WITH->reply_brett, board);
		  WITH->reply_nr= ct;

		  header->lifetime = ttl_lifetime(unr, header->lifetime, header->rxdate);
		  create_status(calc_readers_size(is_call, *header), false,
				header->dest, *header, status);
		  if (only_headers)
		    sprintf(status, "|%s", strcpy(STR1, status));
		  if (outchan >= minhandle)
		    str2file(&outchan, status, true);
		  else
		    wlnuser(unr, status);
		}
	      }

	      if (outchan >= minhandle)
		str2file(&outchan, header->betreff, true);
	      else if (!(export && is_binary)) {
		if (!sf_pack) wlnuser(unr, header->betreff);
	      }

	    }


	    if (sf) {
	      if (user[unr]->sf_level < 3) boxspoolfend(user[unr]->pchan);
	    }

	    srline[0]	= '\0';
	    if (sf || bcast) {

	      if (with_rline
	          &&
		  (   !no_rline_if_exterior
		   || !strcmp(header->rxfrom, Console_call)
		   || *header->rxfrom == '\0'
		  )
		  &&
		  (   !ack_msg
		   || ((header->msgflags & MSG_MINE) != 0)
		   || (sf && (strchr(user[unr]->SID, 'A') == NULL))
		   || (sf && (strchr(user[unr]->SID, 'M') == NULL))
		  )
		 ) {

		/* First BBS only inserts a R: line header with /ACK messages */
      	      	create_my_rline(header->rxdate, header->id, hs);
		if (sf_pack) strcpy(srline, hs);
		else if (bcast) {
		  for (i = 0; hs[i] != '\0'; i++)
		    checksum16(hs[i], &bodychecksum);
		  checksum16(13, &bodychecksum);
		  checksum16(10, &bodychecksum);

		  if (outchan >= minhandle) str2file(&outchan, hs, true);
		} else {
		  wlnuser(unr, hs);
		  if (user[unr]->sf_level < 3) boxspoolfend(user[unr]->pchan);
		}

	      }
	      
	    } else if (*header->id != '\0' && !(export && is_binary)) {

	      if (outchan >= minhandle) {
		if (header->msgtype == 'B') {
		  sprintf(STR1, "*** Bulletin-ID: %s ***", header->id);
		  str2file(&outchan, STR1, true);
		} else {
		  sprintf(STR1, "*** Message-ID: %s ***", header->id);
		  str2file(&outchan, STR1, true);
		}
		if (rheader) {
		  if (*header->rxfrom != '\0') {
		    sprintf(STR1, "*** Received from %s ***", header->rxfrom);
		    str2file(&outchan, STR1, true);
		  }
		}
		str2file(&outchan, "", true);

	      } else {

		if (header->msgtype == 'B') {
		  sprintf(STR1, "*** Bulletin-ID: %s ***", header->id);
		  wlnuser(unr, STR1);
		} else {
		  sprintf(STR1, "*** Message-ID: %s ***", header->id);
		  wlnuser(unr, STR1);
		}
		if (rheader) {
		  if (*header->rxfrom != '\0') {
		    sprintf(STR1, "*** Received from %s ***", header->rxfrom);
		    wlnuser(unr, STR1);
		  }
		}
		wlnuser0(unr);
		if (!rheader)
		  show_readers(unr, is_call, header->readcount, header->readby, with_readers);
	      }
	    }

	    /* ------------------- end of header display ----------------------------- */

	    unpackerr	= 0;
	    sf_ishuff	= false;
	    err		= sfseek(header->start, ki, SFSEEKSET);
	    if (export) user[unr]->tempbinname[0] = '\0';

	    /* ----------------- unpack message on disk ? ---------------------------- */

	    if (header->size > maxram() - 500
		|| (header->size > 100000L && header->size > maxavail__() - 500)) {

	    /* ----------------------- yes, on disk ---------------------------------- */

	      debug(3, unr, 42, "on disk");
	      copybuf	= malloc(blocksize);
	      if (copybuf != NULL) {
		ka = sfcreate(temp1, FC_FILE);
		if (ka >= minhandle) {
		  if (header->pmode == 0 && *srline != '\0' && sf_pack) {
		    str2file(&ka, srline, true);
		    sf_ishuff	= true;
		     /* das flag wird hier anders verwendet	*/
		  }  /* als der name vermuten laesst		*/
		  else
		    sf_ishuff	= false;

		  ok		= true;
		  hsize		= 0;
		  ics		= 0;
		  rs		= header->packsize;
		  while (rs > 0 && ok) {
		    if (rs > blocksize)
		      rs	= blocksize;
		    bct		= sfread(ki, rs, copybuf);
		    if (header->infochecksum != 0) checksum16_buf(copybuf, bct, &ics);
		    if (hsize == 0) fbyte = copybuf[0];
		    if (fbyte == header->firstbyte && sfwrite(ka, bct, copybuf) == bct)
		      hsize	+= bct;
		    else
		      ok	= false;
		    rs		= header->packsize - hsize;
		  }

		  sfclose(&ka);

		  if (hsize != header->packsize) {
		    sprintf(hs, "%s %d : read error (2)", board, ct);
		    debug(0, unr, 42, hs);
		    bodychecksum	= ERR_READBOARD;
		  } else if (header->infochecksum != ics) {
		    sprintf(hs, "%s %d : info checksum error (2)", board, ct);
		    debug(0, unr, 42, hs);
		    bodychecksum	= ERR_READBOARD;
		  } else {
		    if ((header->pmode & (PM_HUFF2 | PM_GZIP)) != 0) {
		      packer((header->pmode & PM_GZIP) != 0, false, false, temp1, temp2, false);
		      sfdelfile(temp1);
		      strcpy(temp3, temp2);
		    } else
		      strcpy(temp3, temp1);

		    if (sf_pack) {
		      hsize	= fsize;   /* ist ein Flag ... */
		      if (!sf_ishuff) {
			if (*srline != '\0') {
			  fsize		= sfsize(temp3);
			  ka		= sfopen(temp3, FO_READ);
			  if (ka >= minhandle) {   /* flag ... */
			    ko		= sfcreate(temp4, FC_FILE);
			    if (ko >= minhandle) {
			      str2file(&ko, srline, true);
			      hsize	= 0;
			      ok	= true;
			      rs	= fsize;
			      while (rs > 0 && ok) {
				if (rs > blocksize) rs = blocksize;
				bct	= sfread(ka, rs, copybuf);
				if (sfwrite(ko, bct, copybuf) == bct)
				  hsize	+= bct;
				else
				  ok	= false;
				rs	= fsize - hsize;
			      }
			      sfclose(&ko);
			      if (hsize != fsize) {
				debug(0, unr, 42, "file error 2");
			      }
			    } else
			      hsize	= fsize + 1;
			    sfclose(&ka);
			    sfdelfile(temp3);
			    strcpy(temp3, temp4);
			  } else
			    hsize	= fsize + 1;

			}

		      }

		      if (hsize == fsize) {
			packer(false, false, true, temp3, temp5, true);
			sfdelfile(temp3);
			send_pfbbdisk(bbsmode, unr, sfoffset, temp5, header->betreff);
			boxspoolread();
		      } else
			bodychecksum	= ERR_READBOARD;

		      sfdelfile(temp5);
		    } else {
		      fsize		= sfsize(temp3);
		      ka		= sfopen(temp3, FO_READ);
		      if (ka >= minhandle) {
			hsize		= 0;
			ok		= true;
			rs		= fsize;
			bct   	      	= 0;
			while (rs > 0 && ok) {
			  if (rs > blocksize) rs = blocksize;
			  bct		= sfread(ka, rs, copybuf);
			  if (bct > 0) {
			    if (bcast) checksum16_buf(copybuf, bct, &bodychecksum);
			    if (outchan >= minhandle) {
			      bct	= sfwrite(outchan, bct, copybuf);
			    } else if (!(export && is_binary)) {
			      if (hsize == 0)
				show_rfile(unr, copybuf, bct, rheader, sf, only_headers);
			      else
				show_puffer(unr, copybuf, bct);
			    } else {
			      if (hsize == 0)
			      {  /* nur im ersten Block suchen ... */
				bstart = get_binstart(copybuf, bct, user[unr]->tempbinname);
				if (bstart > 0)
				  trans_show_puffer(unr, (char *)(&copybuf[bstart]), bct - bstart);
				else
				  show_puffer(unr, copybuf, bct);
			      } else
				show_puffer(unr, copybuf, bct);
			    }
			    hsize	+= bct;

			  } else {
			    debug(0, unr, 42, "file error 3");
			    bodychecksum= ERR_READBOARD;
			    ok		= false;
			  }
			  rs		= fsize - hsize;
			}

			if (bcast && ok) {
			  if (bodychecksum == ERR_READBOARD) bodychecksum = 0;
			}
			sfclose(&ka);

			if (!bcast) {
			  if (sf) {
			    if (!is_binary) {
			      /* insert a newline, if last char wasn't a newline char */
			      if ((bct > 0) && (copybuf[bct-1] != 10)) {
			      	wlnuser0(unr);
			      }
			      if (user[unr]->umode == UM_FILESF) {
			      	wlnuser(unr, "/EX");
			      } else {
			      	chwuser(unr, 0x1a);
			      	wlnuser0(unr);
			      }
			    }
			  }
			  if (!sf && is_binary) user[unr]->errors -= 5;
			  boxspoolread();
			}
			sfdelfile(temp3);
		      }
		    }
		  }
		}
		free(copybuf);
		copybuf = NULL;
	      } else {
		debug(2, unr, 42, "NO MEMORY");
		if (sf) abort_sf(unr, false, "no mem");
		bodychecksum	= ERR_READBOARD;
	      }

	    } else {

	    /* ------------------------ no, do everything in RAM -------------------  */

	      rp	= malloc(header->packsize);
	      if (rp != NULL) {
		err	= sfread(ki, header->packsize, rp);
		ics	= 0;
		if (header->infochecksum != 0) checksum16_buf(rp, err, &ics);
		if (err != header->packsize || rp[0] != header->firstbyte) {
		  sprintf(hs, "%s %d : read error (4)", board, ct);
		  debug(0, unr, 42, hs);
		  if (!sf) wlnuser(unr, "read error");
		  bodychecksum	= ERR_READBOARD;
		} else if (ics != header->infochecksum) {
		  sprintf(hs, "%s %d : info checksum error (4)", board, ct);
		  debug(0, unr, 42, hs);
		  if (!sf) wlnuser(unr, "read error");
		  bodychecksum	= ERR_READBOARD;
		} else {

		  if ((header->pmode & (PM_HUFF2 | PM_GZIP)) != 0) {

		    /* Muessen wir eventuell die msg nicht auspacken und neu packen?  */

		    if (false) {   /* geht nicht wegen CRLF-Wandlung (und mittlerweile */
		      sf_ishuff = true; /* auch nicht wegen gzip-Kompression) 	       */
		      
		      /* Doch, leider. Entpacker aufrufen.                             */

		    } else {
		      unpackerr	= mempacker((header->pmode & PM_GZIP) != 0,
		      		      	     false, rp, err, &nmem, &nsize, temp2, false);
		      free(rp);
		      rp	= NULL;
		      if (nsize != 0) {
			rp	= nmem;
			err	= nsize;
		      }
		    }

		    /* Alles ausgepackt ?                                            */

		    if (unpackerr != 0) {
		    
		      debug(2, unr, 42, TXTNOMEM);
		      wlnuser(unr, TXTNOMEM);
		      bodychecksum	= ERR_READBOARD;
		    
		    } else {
		  
		      if (rp == NULL) {

			/* in RAM zuruecklesen, Entpacker hat auf Disk geschrieben       */

			err		= 0;
			hl		= sfsize(temp2);
			if (hl < maxram() && hl < maxavail__()) {
			  ka		= sfopen(temp2, FO_READ);
			  if (ka < minhandle)
			    wlnuser(unr, TXTDISKFULL);
			  else {
			    rp		= malloc(hl);
			    if (rp != NULL)
			      err	= sfread(ka, hl, rp);
			    else if (sf) {
			      bodychecksum = ERR_READBOARD;
			    }
			    sfclose(&ka);
			  }

			} else {
			
			  debug(2, unr, 42, "no mem (22)");
			  wlnuser(unr, "no mem (22)");
			  bodychecksum	= ERR_READBOARD;
			
			}
			sfdelfile(temp2);
		      }


		    }

		  }


		  if (rp != NULL) {

		    if (sf_pack) {
		      if (sf_ishuff == false) {
			if (*srline != '\0') add_line_to_buff(&rp, &err, 0, srline);
			nsize		= 0;
			nmem		= NULL;
			unpackerr 	= mempacker(false, true, rp, err, &nmem, &nsize, temp2, true);
			free(rp);
			rp		= NULL;

			if (unpackerr == 0) {
			  if (nsize != 0) {
			    rp		= nmem;
			    err		= nsize;
			  } else {
			    err		= 0;
			    hl		= sfsize(temp2);
			    ka		= sfopen(temp2, FO_READ);
			    if (ka >= minhandle) {
			      rp	= malloc(hl);
			      if (rp != NULL) err = sfread(ka, hl, rp);
			      sfclose(&ka);
			    }
			    sfdelfile(temp2);
			  }
			} else
			  bodychecksum	= ERR_READBOARD;
		      }
		      
		      if (rp != NULL && err > 0 && unpackerr == 0) {
			send_pfbbram(bbsmode, unr, rp, err, sfoffset, header->betreff);
			boxspoolread();
		      } else
		      	bodychecksum = ERR_READBOARD;

		    } else {

		      if (outchan >= minhandle) {
			if (bcast) {
			  checksum16_buf(rp, err, &bodychecksum);
			  if (bodychecksum == ERR_READBOARD) bodychecksum = 0;
			}
			err	= sfwrite(outchan, err, rp);
		      } else {
			if (!(export && is_binary)) {
			  show_rfile(unr, rp, err, rheader, sf, only_headers);
			  if (sf) {
			    if (!is_binary) {
			      /* insert a newline, if last char wasn't a newline char */
			      if ((err > 0) && (rp[err-1] != 10)) {
			      	wlnuser0(unr);
			      }
			      if (user[unr]->umode == UM_FILESF) {
				wlnuser(unr, "/EX");
			      } else {
			      	chwuser(unr, 0x1a);
			      	wlnuser0(unr);
			      }
			    }
			  }
			  if (!sf && is_binary)
			    user[unr]->errors -= 5;
			} else {
			  bstart	= get_binstart(rp, err, user[unr]->tempbinname);
			  if (bstart > 0)
			    trans_show_puffer(unr, (char *)(&rp[bstart]), err - bstart);
			  else
			    show_puffer(unr, rp, err);
			}
		      }
		      if (!bcast)
			boxspoolread();
		    }

		  }

		}

		if (rp != NULL) {
		  free(rp);
		  rp	= NULL;
		}
		  
	      } else {
		debug(2, unr, 42, "no mem (23)");
		wlnuser(unr, TXTNOMEM);
		if (sf) abort_sf(unr, false, "no mem");
		bodychecksum	= ERR_READBOARD;
	      }

	    }

	    /* ---------------------- ok, message is read ---------------------------- */

	    if (!sf && !(export && is_binary) && outchan < minhandle) {
	      if (add_ex && unr > 0 && user[unr]->tell < minhandle) wlnuser(unr, "/EX");
	      wlnuser0(unr);
	    }
	  }

	} else if (check_hcs(*header) == false) {
	  sprintf(hs, "%s %d : index checksum error", board, ct);
	  debug(0, unr, 42, hs);
	  bodychecksum	= ERR_READBOARD;
	}

      }

      if (none_displayed) {
	if (!(bcast || sf)) wln_btext(unr, 37);
      }

      sfclose(&ki);
    }
    close_index(&kx);
  } else {
    close_index(&kx);
    if (sf)
      bodychecksum	= ERR_READBOARD;
    else if (lv == 0)
      no_files(unr, board);
    else if (lv == -1)
      wln_btext(unr, 147);
  }
  
  disp_mulsearch(mst);

  if (rp != NULL) free(rp);
  return bodychecksum;
}

#undef blocksize
#undef maxrcount


void list_brett(short unr, char *board, short von, short bis, short threshold,
		char *option, char *search)
{
  short			k, ct, x, y, list, lv, pwm, pwh, FORLIM1;
  boolean		bidsearch, loginsearch, valid, xlist, flist, header_displayed;
  boolean		fwdlist, no_betreff, ownboard, pw_show, lev_show, mlist;
  boolean		rset, wcsearch, iscall, lastflag, ufloaded;
  long			asz;
  mulsearchtype		*mst;
  indexstruct		header;
  userstruct		uf;
  char			zs[256], ds[256], w[256], hs[256], w1[256];
  char			flagins[9], binins[9], pwn[256];

  debug(2, unr, 43, board);

  if (!boxrange(unr))
    return;

  fwdlist		= (!strcmp(board, "X"));
  if (fwdlist && user[unr]->supervisor) recompile_fwd();

  lv			= last_valid(unr, board);
  if (lv > 0) {
    pwh			= nohandle;
    header_displayed	= false;
    pw_show		= (strchr(option, '/') != NULL);
    lev_show  	      	= !pw_show && (strchr(option, '\\') != NULL);
    if (pw_show || lev_show) {
      sprintf(pwn, "%sDPpwshow%cXXXXXX", tempdir, extsep);
      mymktemp(pwn);
      pwh		= sfcreate(pwn, FC_FILE);
      if (pwh < minhandle) return;
    }

    mlist		= (!strcmp(board, "M") && !pw_show && !lev_show);
    if (!mlist) {
      iscall		= callsign(board);
      xlist		= (strchr(option, '+') != NULL);
      flist		= (strchr(option, ':') != NULL);
      bidsearch		= (strchr(option, '$') != NULL);
    } else {
      iscall		= false;
      xlist		= false;
      flist		= false;
      bidsearch		= false;
    }

    no_betreff		= (mlist && !user[unr]->console);
    loginsearch		= (strchr(option, '!') != NULL);
    if (loginsearch) {
      von		= 1;
      bis		= SHORT_MAX;
    }

    ufloaded		= false;
    uf.readlock		= 2;
    *uf.name		= '\0';
    *uf.mybbs		= '\0';
    ownboard		= (iscall && strcmp(user[unr]->call, board) == 0);
    wcsearch		= (strchr(search, '*') != NULL);
    mst			= gen_mulsearch(threshold, search);

    list		= open_index(board, FO_READ, von != bis, true);
    if (list >= minhandle) {
      if (bis > lv) bis = lv;
      x			= strpos2(option, "&", 1);
      if (x > 0) {
	*w1		= '\0';
	for (y = x; y <= x + 2; y++)
	  sprintf(w1 + strlen(w1), "%c", option[y]);
	von		= bis - atoi(w1) + 1;
	loginsearch 	= false;
      }
      if (von < 1)
	von		= 1;
      if (von > bis) {
	von		= bis;
	wln_btext(unr, 25);
	wlnuser0(unr);
      }

      for (k = von; k <= bis; k++) {
	lastflag	= false;
	read_index(list, k, &header);

	if (header.deleted == user[unr]->hidden) {
	  if (loginsearch) {
	    valid	= (header.rxdate > user[unr]->lastdate);
	    if (mst != NULL) {
	      if (valid)
		valid	= check_search(bidsearch, fwdlist, wcsearch, mst, header);
	    } else {
	      if (k == bis && header_displayed == false && valid == false) {
		valid	= true;
		lastflag= true;
	      }
	    }
	  } else if (mst != NULL)
	    valid	= check_search(bidsearch, fwdlist, wcsearch, mst, header);
	  else
	    valid	= true;

	  if (valid && !(fwdlist || pw_show || lev_show)) {
	    valid	= (user[unr]->msgselection == 0
			   || (header.msgflags & user[unr]->msgselection) != 0);

	    if (valid && (user[unr]->msgselection == (MSG_SFHOLD | MSG_LOCALHOLD))) {
	      valid	= clock_.ixtime - header.rxdate < holddelay;
	    }

	    if (valid && (iscall || header.msgtype != 'B') && !user[unr]->supervisor) {
	      if (iscall && !ufilhide && !ufloaded) {
		load_userfile(false, false, board, &uf);
		if (*uf.call == '\0') uf.readlock = 2;
		ufloaded	= true;
	      }

	      unhpath(header.verbreitung, hs);

	      if (ufilhide || uf.readlock == 2 || strcmp(hs, Console_call))
		valid	= (   ownboard
			   || !strcmp(header.absender, user[unr]->call)
			   || !strcmp(header.dest, user[unr]->call) );
	      else if (uf.readlock == 1)
		valid	= (   ownboard
			   || !strcmp(header.absender, user[unr]->call)
			   || !strcmp(header.dest, user[unr]->call)
			   || in_readby(board, header.readby) > 0);
	    }

	  } else if (valid && fwdlist) {
	    if (xlist)
	      valid	= header.msgtype == 'P' || header.msgtype == 'A';
	    else if (flist) {
	      strcpy(w, header.betreff);
	      get_word(w, w1);
	      valid	= header.msgtype == 'B' && strcmp(w1, "M") && strcmp(w1, "E");
	    }
	  }

	  if (valid) {
	    if (!header_displayed) {
	      if (fwdlist) {
		wlnuser(unr, "FORWARD-LIST:");
		wln_btext(unr, 29);
		wlnuser0(unr);
	      } else if (pw_show) {
		wlnuser(unr, "Users with password: (call-pwlen-pwmode)");
		wlnuser0(unr);
	      } else if (lev_show) {
		wlnuser(unr, "Users with non-standard level: (call-level)");
		wlnuser0(unr);
	      } else {
		if (lastflag) {
		  wln_btext(unr, 25);
		  wlnuser0(unr);
		}

		if (iscall) {
		  if (!ufloaded) {
		    load_userfile(false, false, board, &uf);
		    ufloaded	= true;
		  }

		  wuser(unr, "User-File: ");
		  if (*uf.mybbs != '\0') {
		    sprintf(w, "%s @ %s", board, uf.mybbs);
		    wuser(unr, w);
		    if (!uf.lock_here && ring_bbs) {
		      unhpath(uf.mybbs, hs);
		      if (!strcmp(hs, Console_call)) wuser(unr, " (swap)");
		    }
		  } else
		    wuser(unr, board);
		  if (*uf.name != '\0') {
		    sprintf(w, " (%s)", uf.name);
		    wlnuser(unr, w);
		  } else
		    wlnuser0(unr);
      		} else {
		  w_btext(unr, 15);
		  sprintf(w, " %s", board);
		  wlnuser(unr, w);
		}

		if (mlist)
		  wln_btext(unr, 79);
		if (flist)
		  wln_btext(unr, 27);
		else if (xlist)
		  wln_btext(unr, 26);
		else
		  wln_btext(unr, 16);
		wlnuser0(unr);
	      }
	      header_displayed	= true;
	    }

	    if (!check_hcs(header)) {
	      if (!may_sysaccess(unr, board))
		break;
	      wln_btext(unr, 80);
	    }

	    if (pw_show) {

	      if (header.size == 0)
		pwm	= header.msgflags;
	      else
		pwm	= header.fwdct;
	      if (pwm > MAXPWMODE)
		pwm	= 0;
	      if (*header.betreff != '\0' || pwm > 1) {
		sprintf(hs, "%s-%d-%d", header.absender, strlen(header.betreff), pwm);
		str2file(&pwh, hs, true);
	      }

	    } else if (lev_show) {

	      if (header.level != 1) {
		sprintf(hs, "%s-%d", header.absender, header.level);
		str2file(&pwh, hs, true);
	      }

	    } else {

	      flagins[0]= '\0';
	      binins[0]	= '\0';
	      if (!mlist) {
		if ((header.pmode & TBIN) != 0)
		  strcpy(binins, "(BIN) ");
		else if ((header.pmode & T7PLUS) != 0)
		  strcpy(binins, "(7PL) ");
		else if ((header.pmode & THTML) != 0)
		  strcpy(binins, "(WWW) ");
	      }

	      if (bidsearch) {
		strcpy(ds, header.id);
		rspacing(ds, LEN_BID);
	      } else
		ixdatum2string(header.rxdate, ds);

	      ixzeit2string(header.rxdate, zs);   /*Keine Sekunden zeigen*/
	      cut(zs, 5);

	      rspacing(header.rxfrom, LEN_CALL);
	      rspacing(header.absender, LEN_CALL);
	      rspacing(header.verbreitung, LEN_CALL);
	      cut(header.verbreitung, LEN_CALL);
	      if (mlist)
		asz = 0;
	      else
		asz = truesize(header);

	      if (fwdlist) {

		cut(header.betreff, LEN_BOARD);
		get_word(header.betreff, w);
		rspacing(w, LEN_BOARD);
		rspacing(header.dest, LEN_CALL);
		rspacing(header.id, LEN_BID);
	/*	sprintf(w1, "%ld", header.rxqrg); */ /* geht momentan nicht unter Linux */
	        strcpy(w1, "?");
		lspacing(w1, 7);
		sprintf(hs, "%4d %s %s %s %s %s %s %s %3d %6ld %c",
				k, header.dest, header.rxfrom, w1, header.absender,
				w, header.verbreitung, header.id,
				ttl_lifetime(unr, header.txlifetime, header.rxdate),
				asz, header.msgtype );

	      } else {

		if (ownboard ||
		    (iscall && (user[unr]->supervisor || !strcmp(user[unr]->call, header.absender)))) {
		  x	= in_readby(board, header.readby);
		  if (x <= 0)
		    strcpy(flagins, "*");
		  else if (header.readby[x - 2] == ',')
		    strcpy(flagins, "+");
		  else
		    strcpy(flagins, ".");
		} else if (user[unr]->supervisor) {
		  if (strchr(header.readby, '&') != NULL)
		    strcpy(flagins, ".");
		}

		rspacing(header.dest, LEN_BOARD);
		if (flist) {
		  if (!bidsearch)
		    cut(ds, 5);

		  sprintf(hs, "%4d %s %s %s%4d%4d %s%4d %s %6ld %3d%%%4d%5d %c",
				k, header.absender, header.dest, header.verbreitung,
				ttl_lifetime(unr, header.lifetime, header.rxdate),
				ttl_lifetime(unr, header.txlifetime, header.rxdate),
				header.rxfrom, header.level, ds, asz,
				calc_prozent(header.packsize, header.size),
				header.fwdct, header.msgflags, header.msgtype);
		  wlnuser(unr, hs);

		  if (!no_betreff) {
		    sprintf(hs, "     %s%s%s", flagins, binins, header.betreff);
		    wlnuser(unr, hs);
		  }

		  if (user[unr]->supervisor
		    || (user[unr]->rsysop && in_rsysops_boards(user[unr]->call, board))) {
		    rset	= false;
		    if (header.readcount > 0) {
		      sprintf(w, "     Read: %d ", header.readcount);
		      wuser(unr, w);
		      rset	= true;
		    }
		    if (*header.readby != '\0') {
		      if (!rset) wuser(unr, "     Read: ");
		      strcpy(hs, header.readby);
		      x	= 1;
		      while (x > 0) {
			x	= strpos2(hs, "-", 1);
			if (x > 0) strdelete(hs, x, LEN_CALL+1);
		      }
		      FORLIM1 = strlen(hs);
		      for (x = 0; x < FORLIM1; x++) {
			if (!(isupper(hs[x]) || isdigit(hs[x]))) hs[x] = ' ';
		      }
		      wlnuser(unr, hs);
		      rset	= false;
		    }
		    if (rset) wlnuser0(unr);

		    if (header.readcount > 0) {
		      ix2string(header.lastread, hs);
		      sprintf(hs, "     Last Read: %s", strcpy(w, hs));
		      wlnuser(unr, hs);
		    }

		    if (header.eraseby != EB_NONE) {

		      get_eraseby(header.eraseby, w1);
		      ix2string(header.erasetime, w);
		      sprintf(hs, "     Erased at: %s by %s", w, w1);
		      x = strpos2(header.readby, "-", 1);
		      if (x > 0) {
			strsub(w1, header.readby, x + 1, LEN_CALL);
			del_lastblanks(w1);
			sprintf(hs + strlen(hs), " %s", w1);
		      }
		      wlnuser(unr, hs);

		    }

		  } else {
		    if (header.readcount > 0) {
		      sprintf(w, "     Read: %d", header.readcount);
		      wlnuser(unr, w);
		    }
		  }
		  
		  wuser(unr, "     Flags: ");
		  get_msgflags(header.msgflags, hs);
		  wlnuser(unr, hs);
		  *hs = '\0';
		  
		} else if (xlist) {

		  if (no_betreff)
		    *w	= '\0';
		  else
		    sprintf(w, "  %s%s%s", flagins, binins, header.betreff);

		  sprintf(hs, "%4d %s %s%4d %s %6ld %3d%%%s",
				k, header.absender, header.verbreitung,
				ttl_lifetime(unr, header.lifetime, header.rxdate),
				ds, asz,
				calc_prozent(header.packsize, header.size),
				w);

		} else {

		  if (no_betreff)
		    *w	= '\0';
		  else
		    sprintf(w, "  %s%s%s", flagins, binins, header.betreff);

		  sprintf(hs, "%4d %s %s %s %6ld%s",
				k, header.absender, ds, zs, asz, w);

		  if (user[unr]->in_begruessung && hs[strlen(hs) - 1] == '>')
		    hs[strlen(hs) - 1] = ']';

		}
	      }

/*	      if (user[unr]->console)
		cut(hs, 118);
	      else
*/		cut(hs, 79);

	      wlnuser(unr, hs);
	    }
	  }
	}
      }
      close_index(&list);

      if (pw_show || lev_show) {
	sfclose(&pwh);
	sort_file(pwn, false);
	ct	= 0;
	pwh	= sfopen(pwn, FO_READ);
	if (pwh >= minhandle) {
	  if (pw_show) x = LEN_CALL + 1 + 2 + 1 + 2 + 1;
	  else x = LEN_CALL + 1 + 3 + 1;
	  y = 80 / x;
	  while (file2str(pwh, hs)) {
	    if (ct % y == 0) wlnuser0(unr);
	    rspacing(hs, x);
	    wuser(unr, hs);
	    ct++;
	  }
	  if (ct > 0) wlnuser0(unr);
	  sfclosedel(&pwh);
	}
      }

      if (!header_displayed) wln_btext(unr, 37);

    } else {
      close_index(&list);
      no_files(unr, board);
    }
    sfclosedel(&pwh);
    disp_mulsearch(mst);
    return;
  }
  if (lv == -1)
    wln_btext(unr, 147);
  else
    no_files(unr, board);
}


static void show_pwuser(short unr)
{
  list_brett(unr, "M", 1, SHORT_MAX, 100, "/", "");
}

static void show_leveluser(short unr)
{
  list_brett(unr, "M", 1, SHORT_MAX, 100, "\\", "");
}


static void change_mbx(short unr, char *brett, short von, short bis,
		       char *newmbx_, char *option)
{
  short		k, list, lv;
  boolean	forwarding, only_show, disp, valid, userfile;
  indexstruct	header;
  mbxtype	newmbx, mbx1;
  char		w[256];

  debug(2, unr, 44, brett);
  if (!boxrange(unr)) return;

  strcpy(newmbx, newmbx_);

  only_show	= (strchr(option, '+') != NULL);
  unhpath(newmbx, mbx1);
  userfile	= callsign(brett);
  if (userfile && !callsign(mbx1) && !only_show) {
    wln_btext(unr, 34);
    return;
  }
  if (callsign(mbx1)) {
    if (strchr(newmbx, '.') == NULL) {
      complete_hierarchical_adress(newmbx);
      unhpath(newmbx, mbx1);
    }
  }
  valid		= false;

  lv		= user[unr]->lastprocnumber2;
  user[unr]->lastprocnumber2 = 0;
  if (lv == 0)
    lv		= last_valid(unr, brett);

  if (lv > 0) {
    if (user[unr]->lastprocnumber > 0) {
      von	= user[unr]->lastprocnumber;
      user[unr]->lastprocnumber = 0;
      disp	= true;
      valid	= true;
    } else
      disp	= false;

    if (bis > lv) bis = lv;

    list	= open_index(brett, FO_RW, false, true);
    if (list < minhandle || lv <= 0) {
      close_index(&list);
      no_files(unr, brett);
      return;
    }

    for (k = von; k <= bis; k++) {
      if (disp && k != von && get_cpuusage() - user[unr]->cputime >= ttask) {
	user[unr]->lastprocnumber	= k;
	user[unr]->lastprocnumber2	= lv;
	break;
      }

      if (list >= minhandle) {
	read_index(list, k, &header);
	if (header.deleted == user[unr]->hidden && check_hcs(header)) {
	  valid	= true;
	  if (!only_show) {
	    if (strcmp(header.verbreitung, newmbx)) {
	      strcpy(header.verbreitung, newmbx);
	      header.msgflags &= ~(MSG_SFWAIT | MSG_SFERR | MSG_SFNOFOUND | MSG_REJECTED | MSG_DOUBLE);
	      write_index(list, k, &header);
	      alter_log(true, header.msgnum, header.msgflags, '@', newmbx);
	    } else
	      unhpath(header.verbreitung, mbx1);

	    if (header.deleted == false) {
	      if (userfile)
		forwarding	= (strcmp(mbx1, Console_call) != 0);
	      else
		forwarding	= ((strstr(header.id, Console_call) != NULL
				    || (header.msgflags & MSG_SFRX) != 0)
				   && strcmp(mbx1, Console_call));
	    } else
	      forwarding	= false;

	    if (forwarding) {
	      close_index(&list);
	      *w		= '\0';
	      set_forward(-1, -1, brett, w, k, k, "FORWARD", w, w);
	      list		= open_index(brett, FO_RW, false, true);
	    }
	  } else {
	    w_btext(unr, 35);
	    chwuser(unr, ' ');
	    wlnuser(unr, header.verbreitung);
	  }
	}
      }
    }

    close_index(&list);

    if (valid && !disp) {
      w_btext(unr, 36);
      chwuser(unr, ' ');
      wlnuser(unr, newmbx);
    } else if (!valid)
      wln_btext(unr, 37);
    return;
  }

  if (lv == -1)
    wln_btext(unr, 147);
  else
    no_files(unr, brett);
}


static void change_lifetime(short unr, char *brett, short von, short bis, short threshold,
			    char *newlt_, char *option, char *search)
{
  short			k, list, lv, lt, xlt;
  boolean		valid, disp, wcsearch, bidsearch;
  mulsearchtype		*mst;
  indexstruct		header;
  char			newlt[256];

  debug(2, unr, 45, brett);

  if (!boxrange(unr)) return;

  strcpy(newlt, newlt_);

  if (!may_sysaccess(unr, brett)) {
    wln_btext(unr, 1);
    return;
  }

  if ((unsigned long)strlen(newlt) >= 32 ||
      ((1L << strlen(newlt)) & 0x1e) == 0) {
    wln_btext(unr, 39);
    return;
  }

  if (newlt[0] == '#')
    strdelete(newlt, 1, 1);

  lt	= atoi(newlt);
  if ((unsigned)lt > 999) {
    wln_btext(unr, 39);
    return;
  }

  valid	= false;
  lv	= user[unr]->lastprocnumber2;
  user[unr]->lastprocnumber2 = 0;
  if (lv == 0)
    lv	= last_valid(unr, brett);

  if (lv > 0) {
    if (user[unr]->lastprocnumber > 0) {
      von	= user[unr]->lastprocnumber;
      user[unr]->lastprocnumber = 0;
      disp	= true;
      valid	= true;
    } else
      disp	= false;

    bidsearch	= (strchr(option, '$') != NULL);
    wcsearch	= (*search != '\0' && strchr(search, '*') !=NULL);
    mst		= gen_mulsearch(threshold, search);

    if (bis > lv) bis = lv;

    list	= open_index(brett, FO_RW, bis - von > 5, true);
    if (list < minhandle || lv <= 0) {
      close_index(&list);
      no_files(unr, brett);
      return;
    }


    for (k = von; k <= bis; k++) {
      if (disp && k != von && get_cpuusage() - user[unr]->cputime >= ttask) {
	user[unr]->lastprocnumber	= k;
	user[unr]->lastprocnumber2	= lv;
	break;
      }

      if (k % 10 == 0) dp_watchdog(2, 4711);
      read_index(list, k, &header);
      if (header.deleted == user[unr]->hidden && check_hcs(header)) {

	if (*search == '\0' || check_search(bidsearch, false, wcsearch, mst, header)) {

	  xlt		= lt;
	  if (user[unr]->ttl) {
	    if (lt > 0)
	      xlt	= (clock_.ixtime - header.rxdate) / SECPERDAY + lt;
	  }

	  if (header.lifetime != xlt) {
	    header.lifetime	= xlt;
	    write_index(list, k, &header);
	    sprintf(newlt, "%d", xlt);
	    alter_log(true, header.msgnum, header.msgflags, '#', newlt);
	  }
	  valid		= true;
	}
      }
    }

    disp_mulsearch(mst);
    close_index(&list);
    if (!valid || disp) {
      if (!valid) wln_btext(unr, 37);
      return;
    }

    w_btext(unr, 38);
    sprintf(newlt, " %d", lt);
    wlnuser(unr, newlt);
    return;
  }

  if (lv == -1)
    wln_btext(unr, 147);
  else
    no_files(unr, brett);
}





#define maxcheckc       30000
#define maxchecks       30000

static long   	      	maxcheck, lastcheck;
static char   	      	dots[LEN_BOARD+5];

  
void disp_logptr(short unr, long checkct, boolean bidsearch, boxlogstruct *logptr, char *out)
{
  char		ns[256], bname[LEN_BOARD+sizeof(dots)+10], datum[256], binins[10];

  if (logptr == NULL) return;

  if ((logptr->pmode & TBIN) != 0)
    strcpy(binins, "(BIN) ");
  else if ((logptr->pmode & T7PLUS) != 0)
    strcpy(binins, "(7PL) ");
  else if ((logptr->pmode & THTML) != 0)
    strcpy(binins, "(WWW) ");
  else
    *binins	= '\0';

  if (unr > 0 && user[unr]->fbbmode) {

    unhpath(logptr->verbreitung, logptr->verbreitung);

    sprintf(out, "%-7ld%c%6ld %-*.*s@%-*.*s %-*.*s ",
		logptr->msgnum + MSGNUMOFFSET,
		logptr->msgtype, logptr->size,
		LEN_BOARD, LEN_BOARD, logptr->obrett,
		LEN_CALL, LEN_CALL, logptr->verbreitung,
		LEN_CALL, LEN_CALL, logptr->absender);

    if (bidsearch) {
      strcpy(ns, logptr->bid);
      rspacing(ns, LEN_BID+1);
      strcat(out, ns);
    } else {
      ix2string(logptr->date, ns);
      sprintf(out + strlen(out), "%c%c%c%c/%c%c%c%c ",
	      ns[3], ns[4], ns[0], ns[1], ns[9], ns[10], ns[12], ns[13]);
    }
    strcat(out, binins);
    strcat(out, logptr->betreff);

  } else {

    strcpy(bname, logptr->brett);
    if (!defined_board(bname)) lower(bname);
    sprintf(ns, "%d", logptr->idxnr);
    strcat(bname, dots);
    bname[LEN_BOARD+4 - strlen(ns)]	= '\0';

    if (bidsearch) {
      strcpy(datum, logptr->bid);
      rspacing(datum, LEN_BID);
    } else {
      ixdatum2string(logptr->date, datum);
    }

    rspacing(logptr->absender, LEN_CALL);
    unhpath(logptr->verbreitung, logptr->verbreitung);
    rspacing(logptr->verbreitung, LEN_CALL);
    if (bidsearch)
      cut(logptr->verbreitung, 2);
    else
      cut(logptr->verbreitung, 6);

    sprintf(out, "%5ld %s > %s%s %s %s%6ld %3d %s%s",
		checkct, logptr->absender, bname, ns, datum, logptr->verbreitung, logptr->size,
		ttl_lifetime(unr, logptr->lifetime, logptr->date),
		binins, logptr->betreff);
  }

  cut(out, 79);
}


static void show_logptr(short unr, long checkct, boolean bidsearch,
			boxlogstruct *logptr, long *gefunden,
			short *sorthandle)
{
  char		hs[256], ns[256];

  if (*gefunden < maxcheck) {
    disp_logptr(unr, checkct, bidsearch, logptr, hs);

    if (*sorthandle >= minhandle) {
      if (unr > 0 && user[unr]->fbbmode)
	strcpy(ns, logptr->obrett);
      else {
	strcpy(ns, logptr->brett);
	if (!defined_board(ns))
	  lower(ns);
      }
      rspacing(ns, LEN_BOARD+1);
      strcat(ns, hs);
      str2file(sorthandle, ns, true);
    } else
      wlnuser(unr, hs);

    lastcheck = *gefunden + 1;
    if (*sorthandle < minhandle && *gefunden % 30 == 0)
      boxspoolread();
  }

  (*gefunden)++;
}


boolean check_access_ok(short unr, boolean userfiles, boolean nochb, boolean wantchb,
      	      	      	 boolean wants_distcheck, boolean wants_sendercheck, boxlogstruct *log)
{
  static char 	stemp[] = ",TEMP,";
  
  short		acc;
  boolean	found, found2;
  char		msgboard[LEN_BOARD+5], msgdist[LEN_CALL+5];

  if ((log->msgtype == 'B') == userfiles)
    return false;

  if (nochb || wantchb) {
    
    sprintf(msgboard, ",%s,", log->brett);
    found	= (strstr(user[unr]->checkboards, msgboard) != NULL);
    
    if (wants_distcheck) {
      sprintf(msgdist, ",@%s,", log->verbreitung);
      found2	= (strstr(user[unr]->checkboards, msgdist) != NULL);
    } else found2 = false;
    
    if (wants_sendercheck && !found2) {
      sprintf(msgdist, ",&%s,", log->absender);
      found2	= (strstr(user[unr]->checkboards, msgdist) != NULL);
    }
    
    if (nochb) {
      if (strstr(mustcheckboards, msgboard) == NULL) {
	if (found || found2) return false;
	else if (strstr(user[unr]->checkboards, stemp) != NULL) {
      	  if (!defined_board(log->brett)) return false;
        }
      }
    }
    
    if (wantchb) {
      if (!found && !found2) {
        if (strstr(mustcheckboards, msgboard) == NULL) {
          if (strstr(user[unr]->checkboards, stemp) != NULL) {
            if (defined_board(log->brett)) return false; 
	  }
      	  else return false;
	}
      }
    }

  }

  if (user[unr]->console) {
    acc		= 1;

  } else if (log->brett[1] == '\0') { /* == (strlen(log->brett) == 1) */
    if (may_sysaccess(unr, log->brett))
      acc	= log->level;
    else
      acc	= SHORT_MAX;
  } else
    acc		= log->level;

  if (user[unr]->level < acc)
    return false;

  if (log->deleted == user[unr]->hidden) {
    if (user[unr]->supervisor || log->msgtype == 'B' ||
	!strcmp(log->absender, user[unr]->call) || !strcmp(log->brett, user[unr]->call)) {
      return true;
    }
  }
  
  return false;
}

#define FTRSTRSIZE 10000

void show_fullcheck(short unr)
{
  short		handle;
  long		msgnum, checkct;
  boxlogstruct	log;
  char		w[256], w1[256], hs[FTRSTRSIZE+1];

  if (!boxrange(unr))
    return;

  if (*user[unr]->wait_file == '\0')
    return;

  checkct	= 1;
  handle	= sfopen(user[unr]->wait_file, FO_READ);
  if (handle >= minhandle) {
    *w1		= '\0';
    file2str(handle, w1); /* skip copyright 	      	*/
    file2str(handle, w1); /* word counts		*/
    get_word(w1, w);	  /* total scanned documents	*/
    wlnuser(unr, w);
    wlnuser(unr, w1);
    file2lstr(handle, hs, FTRSTRSIZE); /* found messages*/
    file2str(handle, w1); /* document count 		*/
/*    wlnuser(unr, w1); */

    if (*hs) {

      sfclose(&handle);
      wlnuser0(unr);
      if (user[unr]->fbbmode)
        wln_btext(unr, 183);
      else
        wln_btext(unr, 17);
      wlnuser0(unr);
      recompile_log(unr);

      while (*hs && checkct <= 2000) {
        get_word(hs, w);
        msgnum	= atol(w);
        if (msgnum2centry(msgnum, &log, false) >= 0) {
          if (check_access_ok(unr, (crawl_private && log.msgtype != 'B'), false, false, false, false, &log)) {
	    disp_logptr(unr, checkct++, false, &log, w);
	    wlnuser(unr, w);
	  }
    	}
      }

    } else {

      while (file2str(handle, w)) /* alternate words */
      	wlnuser(unr, w);
      sfclose(&handle);

    }
  }

  sfdelfile(user[unr]->wait_file);
  *user[unr]->wait_file	= '\0';
  if (checkct == 1) {
    wlnuser0(unr);
    wlnuser(unr, "no matching document found");
  }
}

#undef FTRSTRSIZE

void show_sortcheck(short unr)
{
  short	sorthandle;
  char	w[256], w1[256], hs[256];

  if (!boxrange(unr))
    return;

  if (*user[unr]->wait_file == '\0')
    return;

  sorthandle	= sfopen(user[unr]->wait_file, FO_READ);
  if (sorthandle >= minhandle) {
    *w1		= '\0';
    while (file2str(sorthandle, hs)) {
      sprintf(w, "%.*s", LEN_BOARD+1, hs);
      if (*w1 != '\0' && strcmp(w1, w))
	wlnuser0(unr);
      strcpy(w1, w);
      wlnuser(unr, (char *) (&hs[LEN_BOARD+1]));
    }
    sfclose(&sorthandle);
  }
  sfdelfile(user[unr]->wait_file);
  *user[unr]->wait_file	= '\0';
}

#define cpentries       CHECKBLOCKSIZE
#define cpsize          (cpentries * sizeof(boxlogstruct))

static void check(short unr, char *suche, long von, long bis, short threshold,
		  boolean userfiles, char *option, char *board,
		  boolean sorted)
{

  short			sorthandle, log, cmode;
  long			cpstart, seekpos, err, lognr, loganz, checkct, gefunden;
  boolean		check_end, bidsearch, nochb, wantchb, resumed, wcsearch, first;
  boolean     	      	wants_distcheck, wants_sendercheck, abort;
  mulsearchtype		*mst;
  boxlogstruct		*logptr, logheader;
  char			*ipuffer;
  char			hs[256], STR7[256];
  pathstr		fn, logname;

  debug0(2, unr, 46);

  if (!boxrange(unr)) return;

  strcpy(logname, boxlog);

  resumed	= (user[unr]->lastprocnumber > 0);
      /* diesmal ist es nur ein Flag (0/1(/2)) */

  if (resumed)
    err		= user[unr]->lastprocnumber7;
  else
    err		= sfsize(logname);


  if (err % sizeof(boxlogstruct) != 0 || err == 0) {
    create_new_boxlog(unr, false);
    err		= sfsize(logname);
  }

  if (!resumed) {
    recompile_log(unr);
    checkct	= 0;
    gefunden	= 0;
    lastcheck	= 0;
  } else {
    checkct	= user[unr]->lastprocnumber4;
    gefunden	= user[unr]->lastprocnumber5;
    lastcheck	= user[unr]->lastprocnumber6;
  }

  cpstart	= 0;
  check_end	= false;
  abort       	= false;
  cmode		= 2;

  if (sorted)
    maxcheck	= maxchecks;
  else
    maxcheck	= maxcheckc;

  wcsearch	= (strchr(suche, '*') != NULL);
  mst		= gen_mulsearch(threshold, suche);

  if (*suche != '\0')	/* CHECK 1-10 */
    cmode	= 1;	/* Search */
  else if (strchr(option, '!') != NULL)
    cmode	= 2;	/* loginsearch */
  else if (bis > 0)
    cmode	= 3;
  if (*board != '\0')	/* + DIR */
    cmode	+= 10;

  bidsearch	= (strchr(option, '$') != NULL);

  nochb		= (!userfiles && cmode < 10 && *suche == '\0' && user[unr]->wantboards == false
			&& user[unr]->msgselection == 0 && *user[unr]->checkboards != '\0');

  wantchb	= (!userfiles && cmode < 10 && !nochb && *suche == '\0'
			&& user[unr]->msgselection == 0 && user[unr]->wantboards == true
			&& *user[unr]->checkboards != '\0');

  wants_distcheck= ((nochb || wantchb) && strchr(user[unr]->checkboards, '@') != NULL);
  wants_sendercheck= ((nochb || wantchb) && strchr(user[unr]->checkboards, '&') != NULL);

  if (!resumed) {

    wuser(unr, "Check: ");
    if (*suche != '\0') {
      sprintf(STR7, "(%s)", suche);
      wlnuser(unr, STR7);
    } else
      wlnuser0(unr);

    wlnuser0(unr);
    if (user[unr]->fbbmode)
      wln_btext(unr, 183);
    else
      wln_btext(unr, 17);
    wlnuser0(unr);

  }


  ipuffer	= malloc(cpsize);
  if (ipuffer == NULL) {
    debug(2, unr, 46, "from disk");
  }

  if (resumed) {
    lognr			= user[unr]->lastprocnumber3;
    bis				= user[unr]->lastprocnumber2;
    user[unr]->lastprocnumber3	= 0;
    user[unr]->lastprocnumber	= 0;
    user[unr]->lastprocnumber7	= err;
  } else {
    lognr			= err / sizeof(boxlogstruct);
    if (bis > lognr)
      bis			= lognr;
    user[unr]->lastprocnumber7	= err;
  }

  log				= sfopen(logname, FO_READ);
  loganz			= lognr;

  if (log >= minhandle && lognr > 0) {   /* wenn das log ganz leer ist ... */
    sorthandle			= nohandle;
    if (sorted) {
      sprintf(fn, "%sCS1823%c%d", tempdir, extsep, unr);
      if (!resumed) {
	sorthandle		= sfcreate(fn, FC_FILE);
	user[unr]->prochandle	= sorthandle;
      } else
	sorthandle		= user[unr]->prochandle;
    }

    first			= true;

    do {
      dp_watchdog(2, 4711);   /* Watchdog resetten */
      if (ipuffer == NULL) {
	if (!first && get_cpuusage() - user[unr]->cputime >= ttask) {
	  user[unr]->lastprocnumber	= 1;
	  user[unr]->lastprocnumber2	= bis;
	  user[unr]->lastprocnumber3	= lognr;
	  user[unr]->lastprocnumber4	= checkct;
	  user[unr]->lastprocnumber5	= gefunden;
	  user[unr]->lastprocnumber6	= lastcheck;
	  abort       	      	      	= true;
	  break;
	}

	read_log(log, lognr, &logheader);
	logptr				= &logheader;
      } else {
	if (lognr < cpstart || cpstart == 0) {
	  if (!first && get_cpuusage() - user[unr]->cputime >= ttask) {
	    user[unr]->lastprocnumber	= 1;
	    user[unr]->lastprocnumber2	= bis;
	    user[unr]->lastprocnumber3	= lognr;
	    user[unr]->lastprocnumber4	= checkct;
	    user[unr]->lastprocnumber5	= gefunden;
	    user[unr]->lastprocnumber6	= lastcheck;
	    abort       	      	= true;
	    break;
	  }

	  if (cpstart == 0)
	    cpstart			= loganz - cpentries + 1;
	  else
	    cpstart			-= cpentries;
	  if (cpstart <= 1)
	    cpstart			= 1;
	  seekpos			= (cpstart - 1) * sizeof(boxlogstruct);
	  if (sfseek(seekpos, log, SFSEEKSET) != seekpos) {
	    abort       	      	= true;
	    break;
	  }
	  if (sfread(log, cpsize, ipuffer) <= 0) {
	    abort       	      	= true;
	    break;
	  }
	  if (lognr < cpstart) {
	    abort       	      	= true;
	    break;
	  }
	}

	logptr	= (boxlogstruct *)(&ipuffer[(lognr - cpstart) * sizeof(boxlogstruct)]);
      }

      first	= false;

      if ((user[unr]->msgselection == 0
	  || (((logptr->msgflags & user[unr]->msgselection) != 0)
	     && (user[unr]->msgselection != (MSG_SFHOLD | MSG_LOCALHOLD)
		|| clock_.ixtime - logptr->date < holddelay)))
	  && check_access_ok(unr, userfiles, nochb, wantchb, wants_distcheck, wants_sendercheck, logptr)) {

	if (!user[unr]->undef || !defined_board(logptr->brett)) {
	  checkct++;

	  switch (cmode) {

	  case 1:  /* Search */
	    if (checkct > bis)
	      check_end		= true;
	    else if (checkct >= von) {
	      if (check_logsearch(bidsearch, wcsearch, mst, logptr))
	        show_logptr(unr, checkct, bidsearch, logptr, &gefunden, &sorthandle);
	    }
	    break;

	  case 2:  /* loginsearch */
	    if (user[unr]->lastdate > logptr->date) {
	      if (checkct == 1)
		show_logptr(unr, checkct, bidsearch, logptr, &gefunden, &sorthandle);
	      check_end		= true;
	    } else
	      show_logptr(unr, checkct, bidsearch, logptr, &gefunden, &sorthandle);
	    break;

	  case 3:  /* C 1-10 */
	    if (checkct > bis)
	      check_end		= true;
	    else if (checkct >= von)
	      show_logptr(unr, checkct, bidsearch, logptr, &gefunden, &sorthandle);
	    break;

	  case 11:  /* Search + board */
	    if (checkct > bis)
	      check_end		= true;
	    else if (checkct >= von) {
	      if (!strcmp(board, logptr->brett)) {
		if (check_logsearch(bidsearch, wcsearch, mst, logptr))
		  show_logptr(unr, checkct, bidsearch, logptr, &gefunden, &sorthandle);
	      }
	    }
	    break;

	  case 12:  /* loginsearch + board */
	    if (user[unr]->lastdate > logptr->date)
	      check_end		= true;
	    else if (!strcmp(board, logptr->brett))
	      show_logptr(unr, checkct, bidsearch, logptr, &gefunden, &sorthandle);
	    break;

	  case 13:  /* C 1-10 + board */
	    if (checkct > bis)
	      check_end		= true;
	    else if (checkct >= von) {
	      if (!strcmp(board, logptr->brett))
		show_logptr(unr, checkct, bidsearch, logptr, &gefunden, &sorthandle);
	    }
	    break;
	  }
	}

      }

      lognr--;

    } while (!(check_end || lognr < 1));

    if (!abort) {
      /* das ist doppelt, aber mit Absicht */ 
      if (ipuffer != NULL) {
	free(ipuffer);
	ipuffer = NULL;
      }
      sfclose(&log);

      if (sorthandle >= minhandle) {
	sfclose(&sorthandle);
	user[unr]->prochandle	= nohandle;
	strcpy(user[unr]->wait_file, fn);
	user[unr]->action		= 71;
	user[unr]->wait_pid	= fork();
	if (user[unr]->wait_pid <= 0) {
	  if (user[unr]->wait_pid == 0) setsid();
	  sort_file(fn, false);
	  if (user[unr]->wait_pid == 0) exit(0);
	  show_sortcheck(unr);
	  user[unr]->action	= 0;
	}
      }
    }

  } else
    wln_btext(unr, 8);

  if (ipuffer != NULL) free(ipuffer);
  sfclose(&log);
  disp_mulsearch(mst);

  if (user[unr]->lastprocnumber != 0 || gefunden < maxcheck)
    return;

  wlnuser0(unr);
  wln_btext(unr, 41);
  if (cmode != 2)
    return;
  w_btext(unr, 42);
  sprintf(hs, ": %ld ", checkct);
  wuser(unr, hs);
  w_btext(unr, 81);
  sprintf(hs, " %ld", lastcheck);
  wlnuser(unr, hs);
}

#undef cpentries
#undef cpsize


static void form_out(short unr, char *rub, char *lts, short *k)
{
  strcat(rub, dots);
  rub[LEN_BOARD+5 - strlen(lts)] = '\0';
  strcat(rub, lts);
  strcat(rub, " ");
  wuser(unr, rub);
  if (*k % 5 == 0)
    wlnuser0(unr);
  (*k)++;
}


static void show_dir(short unr, boolean with_user, boolean msgcount, boolean lost)
{
  short			k, result, ofi, msgct;
  unsigned short	lt, acc;
  boolean		eofil, ok, first;
  DTA			dirinfo;
  pathstr		ofiname, STR1;
  char			hs[256], hs2[256], rubrik[256], rubrik1[256];

  debug0(2, unr, 47);

  if (!boxrange(unr))
    return;

  if (user[unr]->lastprocnumber == 0) {
    user[unr]->lastprocnumber2	= 1;
    sprintf(ofiname, "%sSHOWDIR%c%d", tempdir, extsep, unr);
    ofi	= sfcreate(ofiname, FC_FILE);
    if (ofi >= minhandle) {
      wlnuser(unr, "Files:");
      wlnuser0(unr);

      sprintf(STR1, "%s%c%c%s", indexdir, allquant, extsep, EXT_IDX);
      result	= sffirst(STR1, 0, &dirinfo);
      while (result == 0) {
	strcpy(hs, dirinfo.d_fname);
	del_ext(hs);
	if (callsign(hs) == with_user) {
	  if (!with_user) {
	    if (!defined_board(hs))
	      lower(hs);
	  }
	  sprintf(hs2, "%s %ld", hs, dirinfo.d_length);
	  str2file(&ofi, hs2, true);
	}
	result	= sfnext(&dirinfo);
      }
      sfclose(&ofi);
      sort_file(ofiname, false);
      user[unr]->prochandle	= sfopen(ofiname, FO_READ);
    }

  }

  ofi		= user[unr]->prochandle;
  if (ofi < minhandle) {
    wlnuser(unr, TXTNOMEM);
    user[unr]->lastprocnumber	= 0;
    return;
  }

  k		= user[unr]->lastprocnumber2;
  eofil		= false;
  first		= true;
  while (!eofil && (first || get_cpuusage() - user[unr]->cputime < ttask)) {
    first	= false;
    if (!file2str(ofi, hs)) {
      eofil	= true;
      break;
    }

    user[unr]->lastprocnumber++;

    dp_watchdog(2, 4711);   /*Watchdog resetten*/
    get_word(hs, rubrik);
    strcpyupper(rubrik1, rubrik);
    get_word(hs, hs2);

    if (with_user) {  /*Userfiles*/
      if (lost) {  /* nur Userfiles mit MyBBS <> eigenem anzeigen */
	user_mybbs(rubrik1, hs2);
	unhpath(hs2, hs2);
	ok	= (strcmp(hs2, Console_call) != 0);
      } else
	ok	= true;

      if (!ok)
	continue;

      msgct	= last_valid(unr, rubrik1);
      if (msgct <= 0)
	continue;
      if (msgcount) {
	sprintf(hs2, "%d", msgct);
	form_out(unr, rubrik, hs2, &k);
	continue;
      }
      rspacing(rubrik, LEN_BOARD+1);
      wuser(unr, rubrik);
      if ((k & 7) == 0)
	wlnuser0(unr);
      k++;
      continue;
    }

    msgct	= last_valid(unr, rubrik1);
    if (msgct <= 0)
      continue;

    lt		= 0;
    if (msgcount)
      sprintf(hs2, "%d", msgct);
    else {
      check_lt_acc(rubrik, &lt, &acc);
      sprintf(hs2, "%d", lt);
    }
    form_out(unr, rubrik, hs2, &k);
  }

  if (!eofil) {
    user[unr]->lastprocnumber2	= k;
    return;
  }

  sfclosedel(&user[unr]->prochandle);
  user[unr]->lastprocnumber	= 0;
  user[unr]->lastprocnumber2	= 0;
  wlnuser0(unr);

}


static void show_statistik(short unr, char *option)
{
  short			k, ct, result, ofi, anz, prozent, msgs, dmsgs;
  unsigned short	lt, acc;
  long			size1, size2, gsize1, gsize2, hsize, gmsgs, gdmsgs;
  boolean		short_stat, short_user, short_bulls, single_file, first, eofil, take_it;
  DTA			dirinfo;
  indexstruct		header;
  pathstr		datei, ofiname, STR1;
  char			hs[256], w[2], hs2[256], hname[256];

  debug(2, unr, 48, option);

  if (!boxrange(unr))
    return;

  datei[0]	= '\0';
  single_file	= false;

  if (!strcmp(option, "-S")) {
    short_stat	= true;
    short_user	= true;
    short_bulls	= true;
  } else if (!strcmp(option, "-U") || !strcmp(option, "-P")) {
    short_stat	= true;
    short_user	= true;
    short_bulls	= false;
  } else if (!strcmp(option, "-B")) {
    short_stat	= true;
    short_user	= false;
    short_bulls	= true;
  } else {
    short_stat	= false;
    short_user	= false;
    short_bulls	= false;
    if (valid_boardname(option)) {
      strcpyupper(datei, option);
      idxfname(STR1, datei);
      single_file	= exist(STR1);
    }
  }

  if (user[unr]->lastprocnumber == 0) {
    wln_btext(unr, 19);
    wlnuser0(unr);
    wln_btext(unr, 20);
    wlnuser0(unr);

    gsize1	= 0;
    gsize2	= 0;
    gmsgs	= 0;
    gdmsgs	= 0;

    sprintf(ofiname, "%sSHOWDIR%c%d", tempdir, extsep, unr);
    ofi		= sfcreate(ofiname, FC_FILE);
    if (ofi >= minhandle) {
      if (single_file) {
	idxfname(STR1, datei);
      } else {
	sprintf(w, "%c", allquant);
	idxfname(STR1, w);
      }
      result = sffirst(STR1, 0, &dirinfo);

      while (result == 0) {
	strcpy(hs, dirinfo.d_fname);
	del_ext(hs);
	sprintf(hs2, "%s %ld", hs, dirinfo.d_length);
	str2file(&ofi, hs2, true);
	result	= sfnext(&dirinfo);
      }
      sfclose(&ofi);
      sort_file(ofiname, false);
      user[unr]->prochandle	= sfopen(ofiname, FO_READ);
    }

  } else {
    gmsgs	= user[unr]->lastprocnumber3;
    gdmsgs	= user[unr]->lastprocnumber4;
    gsize1	= user[unr]->lastprocnumber5;
    gsize2	= user[unr]->lastprocnumber6;
  }

  ofi		= user[unr]->prochandle;
  if (ofi < minhandle) {
    wlnuser(unr, TXTNOMEM);
    user[unr]->lastprocnumber	= 0;
    return;
  }

  eofil	= false;
  first	= true;
  while (!eofil && (first || get_cpuusage() - user[unr]->cputime < ttask)) {
    first	= false;

    if (!file2str(ofi, hs)) {
      eofil	= true;
      break;
    }

    user[unr]->lastprocnumber++;

    dp_watchdog(2, 4711);   /* Watchdog resetten */

    get_word(hs, hname);
    get_word(hs, hs2);
    hsize	= atol(hs2);

    if (!strcmp(hname, "M") || !strcmp(hname, "X"))
      take_it	= false;
    else if (short_stat) {
      if (callsign(hname))
	take_it	= short_user;
      else
	take_it	= short_bulls;
    } else
      take_it	= true;

    msgs	= 0;
    dmsgs	= 0;
    size1	= 0;
    size2	= 0;

    if (!take_it)
      continue;
    anz		= hsize / sizeof(indexstruct);
    k		= open_index(hname, FO_READ, true, true);
    if (k >= minhandle) {
      for (ct = 1; ct <= anz; ct++) {
	read_index(k, ct, &header);
	if (check_hcs(header)) {
	  size1	+= header.size;
	  size2	+= header.packsize;
	  if (header.deleted)
	    dmsgs++;
	  else
	    msgs++;
	}
      }
      close_index(&k);
    }

    gmsgs	+= msgs;
    gdmsgs	+= dmsgs;
    gsize1	+= size1;
    gsize2	+= size2;

    if (short_stat)
      continue;

    prozent	= calc_prozent(size2, size1);
    lt		= 0;
    check_lt_acc(hname, &lt, &acc);
    if (!(acc <= user[unr]->level || user[unr]->console))
      continue;

    sprintf(hs, "%-*s %5d %5d%10ld%10ld   %3d %3d %3d" , LEN_BOARD, hname, msgs, dmsgs,
						    	size1, size2, prozent, lt, acc);
    wlnuser(unr, hs);
  }

  if (eofil) {
    if (!single_file) {
      if (!short_stat)
	wlnuser0(unr);
      get_btext(unr, 42, hs);
      rspacing(hs, 9);
      cut(hs, 9);
      wuser(unr, hs);
      prozent = calc_prozent(gsize2, gsize1);
      sprintf(hs, "%5ld %5ld%10ld%10ld   %3d" , gmsgs, gdmsgs, gsize1, gsize2, prozent);
      wlnuser(unr, hs);
      wlnuser0(unr);
    }

    sfclosedel(&user[unr]->prochandle);
    user[unr]->lastprocnumber	= 0;
    return;
  }

  user[unr]->lastprocnumber3	= gmsgs;
  user[unr]->lastprocnumber4	= gdmsgs;
  user[unr]->lastprocnumber5	= gsize1;
  user[unr]->lastprocnumber6	= gsize2;
}


void set_forward(short unr, short unr_msg, char *quelle, char *option,
		 short nr, short bis, char *to_box, char *from_box,
		 char *lastvias)
{
  short		list, lv, x;
  boolean	hit;
  indexstruct	header;
  pathstr	index;

  debug(2, unr, 49, quelle);

  if (strchr(option, '(') != NULL || strchr(option, ')') != NULL) {
    wln_btext(unr, 2);
    return;
  }

  x	= strlen(quelle);
  if (x < 1 || x > LEN_BOARD) {
    wln_btext(unr, 2);
    return;
  }

  idxfname(index, quelle);
  if (!exist(index)) {
    wln_btext(unr, 2);
    return;
  }

  lv	= sfsize(index) / sizeof(indexstruct);
  if (bis > lv) bis = lv;
  list	= open_index(quelle, FO_RW, bis - nr > 5, true);

  if (create_syslog) {
    if (boxrange(unr)) {
      if (user[unr]->supervisor)
	append_syslog(unr);
    }
  }

  hit	= false;
  while (nr <= bis && list >= minhandle) {
    if (nr % 20 == 0)   /*Watchdog resetten*/
      dp_watchdog(2, 4711);
    read_index(list, nr, &header);
    if (check_hcs(header)) {
      if (unr < 0) {  /*das System*/
	if (!header.deleted)
	  header.msgflags |= vermerke_sf(unr_msg, false, quelle, from_box, "*", header, lastvias);
      } else if (boxrange(unr)) {
	if (header.deleted == user[unr]->hidden) {
	  if (!strcmp(header.absender, user[unr]->call) ||
	      !strcmp(header.dest, user[unr]->call) ||
	      !strcmp(quelle, user[unr]->call) || user[unr]->supervisor) {
	    hit	= true;
	    header.msgflags |= vermerke_sf(unr_msg, false, quelle, from_box, to_box, header, lastvias);
	  } else {
	    if (!hit)
	      wln_btext(unr, 7);
	    hit	= true;
	  }
 	}
      }
      write_index(list, nr, &header);
      alter_log(false, header.msgnum, header.msgflags, '!', "");
    }
    nr++;
  }
  close_index(&list);
  
  if (unr > 0 && !hit)
    wln_btext(unr, 2);
}


#define blocksize       16384

static void transfer(short unr, char *quelle, short nr, short bis, char *com_)
{
  short			list, nlist, lv, k, x, l, fidx;
  long			start, rs, hsize, psize, ct;
  unsigned short	new_lt, lt, acc1, acc2, ics;
  boolean		hit, first_disp, same_board, first, cerr, ok;
  userstruct		*WITH;
  char			*copybuf;
  indexstruct		header;
  pathstr		in_index, in_info, out_index, out_info;
  mbxtype		new_mbx;
  char			fbyte;
  char			STR1[LEN_MBX+10], STR7[256];
  char			nach[256], hs[256], w[256], w1[256], com[256];

  debug(2, unr, 50, quelle);

  if (!boxrange(unr)) return;

  strcpy(com, com_);
  first_disp	= true;
  hit		= false;
  WITH		= user[unr];
  new_mbx[0]	= '\0';
  new_lt	= 60000L;

  k		= strpos2(com, "#", 1);
  if (k > 0) {
    *hs		= '\0';
    x		= k + 1;
    l		= strlen(com);
    while (x <= l && com[x - 1] == ' ') x++;
    while (x <= l && isdigit(com[x - 1])) {
      sprintf(hs + strlen(hs), "%c", com[x - 1]);
      x++;
    }
    if (*hs != '\0') new_lt = (unsigned short)atoi(hs);
    strdelete(com, k, x - k);
  }

  k = strpos2(com, "@", 1);
  if (k > 0) {
    if (strlen(com) - k <= LEN_MBX) {
      strsub(new_mbx, com, k + 1, strlen(com) - k);
      cut(com, k - 1);
      del_lastblanks(com);
      del_blanks(new_mbx);
      if (!callsign(com) && !callsign(quelle))
	new_mbx[0]	= '\0';
    }
  }
  get_word(com, nach);
  if (*nach != '\0')
    switch_to_default_board(nach);
  check_lt_acc(quelle, &lt, &acc1);
  check_lt_acc(nach, &lt, &acc2);
  if (*new_mbx == '\0' && callsign(nach)) {
    user_mybbs(nach, new_mbx);
  }
  add_hpath(new_mbx);
  if (callsign(nach)) {
    if (*new_mbx == '\0') {
      wln_btext(unr, 112);
      wln_btext(unr, 113);
      return;
    }
    else if (!gen_sftest2(-1, nach, new_mbx)) {
      wln_btext(unr, 13);
      wln_btext(unr, 113);
      return;
    }
  }

  x	= strlen(quelle);
  l	= strlen(nach);
  if (!(   x > 0
	&& x <= LEN_BOARD
	&& l > 0
	&& l <= LEN_BOARD
	&& (x != 1 || (quelle[0] != 'T' && quelle[0] != 'M' && quelle[0] != 'X'))
	&& (l != 1 || (nach[0] != 'E' && nach[0] != 'T' && nach[0] != 'M' && nach[0] != 'X'))
	&& (x != 1 || may_sysaccess(unr, quelle))
	&& (l != 1 || may_sysaccess(unr, nach))
	&& valid_boardname(nach)
	&& acc1 <= WITH->level
	&& acc2 <= WITH->level	)
     ) {
    wln_btext(unr, 2);
    return;
  }

  if (!strcmp(quelle, nach)) {
    if (!callsign(quelle)) {
      wln_btext(unr, 2);
      return;
    }
    same_board	= true;
  }
  else
    same_board	= false;

  if (WITH->lastprocnumber > 0) {
    nr		= WITH->lastprocnumber;
    WITH->lastprocnumber	= 0;
    first_disp	= false;

  }

  if (create_syslog && first_disp && may_sysaccess(unr, quelle))
    append_syslog(unr);
  idxfname(in_index, quelle);
  inffname(in_info, quelle);
  if (!(exist(in_index) && exist(in_info))) {
    wln_btext(unr, 2);
    return;
  }

  lv	= sfsize(in_index) / sizeof(indexstruct);
  if (bis > lv) bis = lv;
  first	= true;
  while (nr <= bis) {
    if (!first && get_cpuusage() - WITH->cputime >= ttask) {
      WITH->lastprocnumber	= nr;
      break;
    }

    cerr	= false;
    fbyte	= 0;
    list	= sfopen(in_index, FO_RW);
    if (list >= minhandle) {
      read_index(list, nr, &header);
      ics	= 0;
      if (header.deleted == WITH->hidden && header.level <= WITH->level &&
	  check_hcs(header)) {
	if (!strcmp(header.absender, WITH->call)
	    || !strcmp(header.dest, WITH->call)
	    || !strcmp(quelle, WITH->call)
	    || may_sysaccess(unr, quelle)) {
	  first	= false;

	  if (!callsign(quelle)) {
	    if (!callsign(nach) || !strcmp(quelle, "D")) {
	      header.deleted	= true;
	      header.erasetime	= clock_.ixtime;
	      header.eraseby	= EB_TRANSFER;
	      add_eraseby(user[unr]->call, &header);
	      write_index(list, nr, &header);
	    }
	  } else {
	    if (!same_board) add_readby(unr, &header, false);
	  }

	  if (!same_board) sfclose(&list);
	  header.deleted	= false;
	  header.eraseby	= EB_NONE;
	  x			= 1;
	  while (x > 0) {
	    x			= strpos2(header.readby, "-", 1);
	    if (x > 0) strdelete(header.readby, x, 7);
	  }

	  if (same_board) { /* der MBX-Befehl waere zwar logischer, aber nun gut */
	  
	    if (*new_mbx != '\0') {
	      strcpy(header.dest, nach);
	      strcpy(header.verbreitung, new_mbx);
	    }
	    header.msgflags &= ~(MSG_SFWAIT | MSG_PROPOSED
				| MSG_SFTX | MSG_SFERR | MSG_SFNOFOUND
				| MSG_REJECTED | MSG_DOUBLE);
	    write_index(list, nr, &header);
	    sfclose(&list);
	    if (first_disp) {
	      hit	= true;
	      w_btext(unr, 82);
	      sprintf(STR7, " %s -> %s", quelle, nach);
	      wuser(unr, STR7);
	    }
	    if (*new_mbx != '\0') {
	      *w	= '\0';
	      if (first_disp) {
		sprintf(STR1, "@%s", new_mbx);
		wlnuser(unr, STR1);
		set_forward(-1, unr, nach, w, nr, nr, "FORWARD", w, w);
	      } else
		set_forward(-1, -1, nach, w, nr, nr, "FORWARD", w, w);
	    }
	  
	  } else { /* if !same_board */
	  
	    psize	= blocksize;
	    if (psize > header.packsize)
	      psize	= header.packsize;

	    copybuf	= malloc(psize);
	    if (copybuf != NULL) {
	      list	= sfopen(in_info, FO_READ);
	      if (list >= minhandle) {
	        sfseek(header.start, list, SFSEEKSET);
		inffname(out_info, nach);
	        if (exist(out_info))
		  nlist	= sfopen(out_info, FO_RW);
	        else
		  nlist	= sfcreate(out_info, FC_FILE);

	        if (nlist >= minhandle) {
		  start	= sfseek(0, nlist, SFSEEKEND);
		  hsize	= 0;
		  ok	= true;
		  rs	= header.packsize;
		  while (rs > 0 && ok) {
		    if (rs > psize) rs = psize;
		    ct	= sfread(list, rs, copybuf);
		    if (hsize == 0) fbyte = copybuf[0];
		    if (ct == rs && fbyte == header.firstbyte) {
		      checksum16_buf(copybuf, rs, &ics);
		      ct	= sfwrite(nlist, rs, copybuf);
		      if (ct == rs)
		        hsize	+= ct;
		      else {
		        debug(0, unr, 50, "write error");
		        cerr	= true;
		        ok	= false;
		      }
		    } else {
		      debug(0, unr, 50, "read error");
		      cerr	= true;
		      ok	= false;
		    }
		    rs	= header.packsize - hsize;
		  }

		  if (!cerr && header.infochecksum != 0) {
		    cerr= (header.infochecksum != ics);
		    if (cerr) {
		      debug(0, unr, 50, "checksum error in mail");
		    }
		  }

		  if (!cerr && strcmp(quelle, "D") &&
		      (header.pmode & (PM_GZIP | PM_HUFF2 | TBIN)) == 0) {
		    /* Transfervermerk geht nur bei ungepackten nicht-BIN - mails */
		    if (!may_sysaccess(unr, nach) || header.msgtype != 'B' || callsign(nach)) {
		      sprintf(hs, "TRANSFER by %s@%s at %s %s UTC",
				WITH->call, Console_call, clock_.datum, clock_.zeit);
		      checksum16(10, &ics);
		      checksum16_buf(hs, strlen(hs), &ics);
		      checksum16(10, &ics);
		      str2file(&nlist, "", true);
		      str2file(&nlist, hs, true);
		      hsize		+= strlen(hs) + 2;
		      header.packsize	= hsize;
		      header.size	= hsize;
		      header.infochecksum = ics;
		    }
		  }

		  sfclose(&list);
		  sfclose(&nlist);
		  free(copybuf);
		  copybuf = NULL;

		  if (!cerr) {
		    idxfname(out_index, nach);
		    header.rxdate	= messagerxtime();
		    header.start	= start;
		    header.level	= acc2;
		    if (new_lt > 1000)
		      new_lt		= header.txlifetime;
		    check_lt_acc(nach, &new_lt, &acc2);
		    header.lifetime	= new_lt;

		    if (callsign(nach) && header.msgtype == 'B')
		      header.msgtype	= 'P';
		    else if (!callsign(nach) && header.msgtype == 'P')
		      header.msgtype	= 'B';

		    if (callsign(nach) && strcmp(header.absender, WITH->call) && strcmp(quelle, "D")) {
		      sprintf(hs, "CP %s: %s", WITH->call, header.betreff);
		      cut(hs, 79);
		      strcpy(header.betreff, hs);
		    }

		    if (callsign(quelle) || callsign(nach)) {
		      *w		= '\0';
		      if (!strcmp(quelle, "D")) {
		        if ((header.msgflags & (MSG_MINE | MSG_SFRX)) != 0){
		          if (*new_mbx == '\0' || !strcmp(nach, header.dest))
		            strcpy(new_mbx, header.verbreitung);
		        }
		        alter_log(true, header.msgnum, header.msgflags, 'E', "");
		      } else
		        new_bid(w);
		      if (*new_mbx != '\0') {
		        strcpy(header.dest, nach);
		        strcpy(header.verbreitung, new_mbx);
		        header.fwdct	= 0;
		        *header.readby	= '\0';
		      }
		      if (*w != '\0') {
		        unhpath(header.verbreitung, w1);
		        if (callsign(w1))
			  strcpy(header.id, w);
		      }
		      header.msgflags &= ~(MSG_SFWAIT | MSG_PROPOSED
		  	  | MSG_SFTX | MSG_SFERR | MSG_SFNOFOUND
		  	  | MSG_REJECTED | MSG_DOUBLE);
		      header.msgnum	= new_msgnum();
		    } else {
		      if (!strcmp(quelle, "D"))
		        if ((header.msgflags & (MSG_MINE | MSG_SFRX)) != 0){
		          if (*new_mbx == '\0' || !strcmp(nach, header.dest))
		            strcpy(new_mbx, header.verbreitung);
		        }
		      alter_log(true, header.msgnum, header.msgflags, 'E', "");
		      header.msgnum	= new_msgnum();
		      alter_fwd('T', header.id, header.msgnum, nach, "");
		    }
		  
		    write_log_and_bid(nach, sfsize(out_index) / sizeof(indexstruct) + 1, header);

		    if (exist(out_index))
		      list	= sfopen(out_index, FO_RW);
		    else
		      list	= sfcreate(out_index, FC_FILE);

		    if (list >= minhandle) {
		      write_index(list, -1, &header);
		      sfclose(&list);
		      if (first_disp) {
		        hit	= true;
		        w_btext(unr, 82);
		        sprintf(STR7, " %s -> %s", quelle, nach);
		        wuser(unr, STR7);
		      }
		      if (*new_mbx != '\0') {
		        *w	= '\0';
		        fidx	= sfsize(out_index) / sizeof(indexstruct);
		        if (first_disp) {
			  sprintf(STR1, "@%s", new_mbx);
			  wlnuser(unr, STR1);
			  set_forward(-1, unr, nach, w, fidx, fidx, "FORWARD", w, w);
		        } else
			  set_forward(-1, -1, nach, w, fidx, fidx, "FORWARD", w, w);
		      }
		    } else
		      wlnuser(unr, "file error (14)");

		  } else
		    wlnuser(unr, "file error (12)");

		  if (first_disp) {
		    wlnuser0(unr);
		    first_disp	= false;
		  }

	        } else
		  sfclose(&list);
	      } else
	        wlnuser(unr, "file error (17)");
	      if (copybuf != NULL) {
                free(copybuf);
		copybuf = NULL;
	      }
	    } else
	      boxprotokoll("Error #1026");
	  } /* of !same_board */
	} else {
	  sfclose(&list);
	  wln_btext(unr, 7);
	}

      } else {
	sfclose(&list);
      }
    } else
      wlnuser(unr, "file error (18)");
    nr++;
  }

  if (!hit && first_disp)
    wln_btext(unr, 2);
}

#undef blocksize


boolean select_file(short unr, char *pfad, char *name, char *titel)
{

  if (user[unr]->console)
    return (boxgetfilename(pfad, name, titel));
  else
    return false;
}


void export_brett(short unr, char *brett, short s, short e, short threshold,
		  char *option1, char *search, char *fname)
{
  boolean	ok, sf;
  short		olda, handle;
  indexstruct	lheader;
  pathstr	exname, pfad, name;
  char		option[256];

  debug(2, unr, 51, brett);

  strcpy(name, fname);
  ok	= true;
  sf	= user[unr]->f_bbs;
  strcpy(option, option1);

  if (*name == '\0')
    sprintf(name, "%sexport%c001", savedir, extsep);
  validate(name);

  if (!sf) {
    wuser(unr, "export: ");
    wlnuser(unr, name);
  }

  handle = sfcreate(name, FC_FILE);

  if (handle < minhandle) {
    if (!sf) wln_btext(unr, 46);
    return;
  }

  sprintf(user[unr]->input2, "%d", handle);
  olda				= user[unr]->action;
  user[unr]->action		= 76;
  user[unr]->tempbinname[0]	= '\0';
  strcat(option, "+");
  read_brett(unr, nohandle, brett, s, e, threshold, option, search, 0, &lheader);
  sfclose(&handle);
  if (user[unr] == NULL) return;
  user[unr]->action 		= olda;

  /* Wenn es sich um ein Binaerfile gehandelt hat, so bekommen   */
  /* wir den Filenamen im tempbinname zurueck und koennen so die */
  /* Datei nachtraeglich umbenennen                              */

  strcpy(pfad, user[unr]->tempbinname);
  del_path(pfad);

  if (*pfad == '\0') return;
  if (sfsize(name) == 0) sfdelfile(name);
  strcpy(exname, name);
  get_path(exname);
  strcat(exname, pfad);
  if (!strcmp(exname, name)) return;
  validate(exname);	/* sicherstellen, dass nicht bereits eine    */
			/* Datei gleichen Namens existiert           */
  sfrename(name, exname);
  strcpy(fname, exname);
  if (!sf) {
    wuser(unr, "export renamed to: ");
    wlnuser(unr, fname);
  }
}

static boolean read_for_view(short unr, char *cmd_, char *outfile, indexstruct *header)
{
  boolean     	ok;
  short		nr, handle;
  char		cmd[256], rubrik[256], w1[256], vname[256];

  debug(2, unr, 52, cmd_);

  strcpy(cmd, cmd_);
  get_word(cmd, rubrik);
  upper(rubrik);
  get_word(cmd, w1);
  if (*w1 == '\0') return false;
  if (!*outfile) {
    snprintf(vname, 255, "%s%s%c%s%cXXXXXX", tempdir, rubrik, extsep, w1, extsep);
    mymktemp(vname);
  } else {
    strcpy(vname, outfile);
  }
  nr		= atoi(w1);
  handle	= sfcreate(vname, FC_FILE);
  if (handle < minhandle) return false;
  ok  	      	= read_brett(unr, handle, rubrik, nr, nr, 100, "+", "", 0, header) != ERR_READBOARD;
  sfclose(&handle);
  if (!ok) {
    sfdelfile(vname);
    *outfile  = '\0';
  } else {
    nstrcpy(outfile, vname, 255);
    conv_file_to_local(outfile);
  }
  return (ok && *outfile);
}

static boolean strip_headers(boolean for_comment, short unr, char *fname, char quotechar, indexstruct *header)
{
  short     k, l;
  char      subject[256];
  char	    hs[256], w[258], tmp[256];
  boardtype ziel;
  mbxtype   mbx;
  
  if (!boxrange(unr)) return false;
  snprintf(tmp, 255, "%sDPSHD_XXXXXX", tempdir);
  mymktemp(tmp);
  l = sfcreate(tmp, FC_FILE);
  if (l < minhandle) return false;
  k = sfopen(fname, FO_READ);
  if (k < minhandle) {
    sfclosedel(&l);
    return false;
  }
  
  file2str(k, hs); /* header line */
  file2str(k, hs); /* subject */

  snprintf(subject, 79, "RE:%s", header->betreff);
  check_replytitle(subject);
  
  str2file(&l, Console_call, true); /* frombox */
  str2file(&l, "From:     ", false);
  str2file(&l, user[unr]->call, true); /* absender */
  str2file(&l, "To:       ", false);
  if (for_comment) {
    strcpy(ziel, header->dest);
  } else {
    strcpy(ziel, header->absender);
  }
  str2file(&l, ziel, true);   /* an */
  str2file(&l, "MBX:      ", false);
  if (for_comment) {
    strcpy(mbx, header->verbreitung);
  } else {
    user_mybbs(header->absender, mbx);
    complete_hierarchical_adress(mbx);
    if (!*mbx) strcpy(mbx, header->sendbbs);
  }
  str2file(&l, mbx, true); /* mbx */
  str2file(&l, "Lifetime: ", false);
  *hs = '\0';
  if (for_comment && header->txlifetime > 0)
    sprintf(hs, "%d", header->txlifetime);
  str2file(&l, hs, true);      /* lt */
  str2file(&l, "BID:      ", false);
  *hs = '\0';
  rspacing(hs, 69);
  str2file(&l, hs, true);      /* BID */
  str2file(&l, "Subject:  ", false);
  str2file(&l, subject, true);/* subject */
  sprintf(hs, "From: %s @ ", user[unr]->call);
  if (tpkbbs) strcat(hs, default_tpkbbs);
  else strcat(hs, ownhiername);
  str2file(&l, hs, true);
  if (!*mbx)
    sprintf(hs, "To:   %s", ziel);
  else
    sprintf(hs, "To:   %s @ %s", ziel, mbx);
  str2file(&l, hs, true);

  str2file(&l, "", true);
  
  unhpath(header->verbreitung, w);
  if (for_german_readers(user[unr]->call, header->absender, mbx)) {
    snprintf(hs, 255, "%s@%s schrieb in MSG $%s an %s@%s:",
      	     header->absender, header->sendbbs, header->id, header->dest, w);
  } else {
    snprintf(hs, 255, "%s@%s wrote in MSG $%s to %s@%s:",
      	     header->absender, header->sendbbs, header->id, header->dest, w);
  }
  str2file(&l, hs, true);
  
  while (file2str(k, hs) && hs[0] != '\0');
  while (file2str(k, hs) && hs[0] != '\0');
  
  while (file2str(k, hs)) {
    if (quotechar) {
      snprintf(w, 257, "%c %s", quotechar, hs);
      str2file(&l, w, true);
    } else {
      str2file(&l, hs, true);
    }
  }

  sfclosedel(&k);
  sfclose(&l);
  sfrename(tmp, fname);
  return true;
}

static boolean read_for_reply(boolean for_comment, short unr, char *cmd, char *outfile)
{
  indexstruct header;
  
  *outfile = '\0';
  if (!read_for_view(unr, cmd, outfile, &header)) return false;
  strip_headers(for_comment, unr, outfile, '>', &header);
  return true;
}


unsigned short read_for_bcast(char *rubrik, short nr, char *vname, indexstruct *header)
{
  unsigned short	bodychecksum;
  short			handle;

  debug(2, 0, 53, rubrik);

  handle	= sfcreate(vname, FC_FILE);
  if (handle < minhandle) return 0xffffL;
  bodychecksum	= read_brett(-1, handle, rubrik, nr, nr, 100, "+B", "", 0, header);
  sfclose(&handle);
  return bodychecksum;
}


static void name_ok(short unr)
{
  userstruct	*WITH;

  if (!boxrange(unr)) return;
  WITH		= user[unr];
  if (!WITH->se_ok) return;
  if (*WITH->name == '\0'  && !login_check) wln_btext(unr, 21);
  if ((*WITH->mybbs == '\0' && !login_check)
      || (WITH->mybbsmode == 'G' && !login_check)
      || (WITH->mybbsupd > 0 && clock_.ixtime - WITH->mybbsupd >= 90*SECPERDAY)) {
    wuser(unr, WITH->call);
    wuser(unr, ": ");
    wln_btext(unr, 22);
  }
}


void begruessung(short unr)
{
  short		d, m, y, h, min, s;
  userstruct	*WITH;
  struct stat	info;
  char		hs[256], w[256], fname[256], pwd[256];

  debug0(1, unr, 54);

  if (!boxrange(unr)) return;

  WITH	= user[unr];
  *pwd	= '\0';
  strcpylower(hs, WITH->call);
  sprintf(w, "%s%s%cpwd", boxsfdir, hs, extsep);
  if (exist(w)) {
    utc_clock();
    strcpy(pwd, clock_.datum);
    strdelete(pwd, 6, 1);
    strdelete(pwd, 3, 1);
    strcpy(hs, clock_.zeit);
    strdelete(hs, 6, 3);
    strdelete(hs, 3, 1);
    sprintf(w, "%s%s", pwd, hs);
    *WITH->sfpwtn	= '\0';
    calc_pwdb(WITH->call, w, WITH->sfpwdb);
    sprintf(pwd, " %s%s", strcpy(w, pwd), hs);
  } else if (!WITH->se_ok && *WITH->password != '\0') {
    if (WITH->sfmd2pw > 0) {  /* soll im sf mit MD2/MD5 einloggen */
      *WITH->sfpwdb	= '\0';
      calc_MD_prompt(md2_only_numbers && WITH->sfmd2pw == 1, pwd);
      if (WITH->sfmd2pw == 1)
        calc_MD2_pw(pwd, WITH->password, WITH->sfpwtn);
      else
        calc_MD5_pw(pwd, WITH->password, WITH->sfpwtn);
      sprintf(pwd, " [%s]", strcpy(w, pwd));
    } else {
      gimme_five_pw_numbers(WITH->password, pwd);
      calc_pwtn(WITH->call, pwd, WITH->sfpwtn);
      lspacing(pwd, strlen(pwd) + 1); /* space an den Anfang stellen */
    }
  }

  create_my_sid(w, pwd);
  if ((WITH->sf_level > 0 || WITH->direct_sf) && !(WITH->console || WITH->supervisor)) {
    if (packed_sf) {
      wlnuser(unr, w);
    } else {
      wlnuser(unr, w);
      w_btext(unr, 52);
      sprintf(w, " %s!", WITH->name);
      wlnuser(unr, w);
    }
    if (WITH->sf_level < 3)
      boxspoolfend(WITH->pchan);
    if (WITH->console) {
      WITH->supervisor	= true;
      WITH->se_ok	= true;
      WITH->level	= 127;
      WITH->is_authentic= true;
      wlnuser0(unr);
      wln_btext(unr, 12);
    }
    chwuser(unr, '>');
    wlnuser0(unr);
    return;
  } else {
    wlnuser(unr, w);  
  }

  /*DB0BRB Mailbox Brandenburg - Login: 13.04.93 13:44 UTC   Logins: 5*/

  wuser(unr, connecttext);
  wuser(unr, " - Login: ");
  ixdatum2string(WITH->logindate, hs);
  wuser(unr, hs);
  chwuser(unr, 32);
  ixzeit2string(WITH->logindate, hs);
  strdelete(hs, 6, 3);   /* Keine Sekunden zeigen */
  wuser(unr, hs);
  wuser(unr, " UTC   Logins: ");
  sprintf(hs, "%d", actual_connects());
  wlnuser(unr, hs);
#ifdef __NetBSD__
  sprintf(w, "dpbox (NetBSD) v%s%s", dp_vnr, dp_vnr_sub);
#else
  #ifdef __linux__
  sprintf(w, "dpbox (Linux) v%s%s", dp_vnr, dp_vnr_sub);
  #else 
    #ifdef __macos__
  sprintf(w, "dpbox (MacOS) v%s%s", dp_vnr, dp_vnr_sub);
    #else
  sprintf(w, "dpbox v%s%s", dp_vnr, dp_vnr_sub);
    #endif
  #endif
#endif
  wlnuser(unr, w);

  w_btext(unr, 53);
  sprintf(w, " %s!", WITH->name);
  wlnuser(unr, w);
  if (WITH->lastdate != WITH->logindate) {
    w_btext(unr, 54);
    decode_ixtime(WITH->lastdate, &d, &m, &y, &h, &min, &s);
    wochentag(WITH->language, d, m, y, hs);
    sprintf(w, " %s, ", hs);
    wuser(unr, w);
    ixdatum2string4(WITH->lastdate, hs);
    wuser(unr, hs);
    chwuser(unr, 32);
    ixzeit2string(WITH->lastdate, hs);
    strdelete(hs, 6, 3);   /* Keine Sekunden zeigen*/
    wlnuser(unr, hs);
  } else
    WITH->lastdate	= WITH->logindate - SECPERDAY;

  WITH->in_begruessung	= true;
  if (exist(ctext_box))
    show_textfile(unr, ctext_box);

  sprintf(fname, "%sNEWS%c%s", boxlanguagedir, extsep, user[unr]->language);
  if (!exist(fname))
    new_ext(fname, "G");
  if (exist(fname)) {
    /* DL5HBF: NEWS nur aussenden, wenn neuer als LastLoginTime */
    stat(fname, &info);
    if (info.st_mtime > WITH->lastdate) {
      wlnuser0(unr);
      show_textfile(unr, fname);
    }
  }

  if (WITH->console) {
    WITH->supervisor	= true;
    WITH->se_ok		= true;
    WITH->level		= 127;
    WITH->is_authentic	= true;
    wlnuser0(unr);
    wln_btext(unr, 12);
  }

  if (WITH->force_priv == 1) {
    show_prompt(unr);
    wlnuser0(unr);
    WITH->in_begruessung= false;
    return;
  }
  
  name_ok(unr);
  wlnuser0(unr);

  WITH->in_begruessung	= false;

  if (WITH->supervisor) {
    show_hold(unr, MAXDISPLOGINHOLD);
  }

  if (*WITH->logincommands != '\0')
    strcpy(hs, WITH->logincommands);
  else
    strcpy(hs, "L");
  box_input(unr, true, hs, true);
}


/* Nur bei Anwahl von Modem-Port: erstmal das Password eingeben. Das   */
/* PW wird, anders als ueberall sonst, KOMPLETT abgefordert. Ueber     */
/* Draht hoert ja "nur" der Verfassungsschutz zu...                    */

static void enter_password(short unr, char *eingabe, boolean last)
{
  userstruct	*WITH;

  debug(2, unr, 55, eingabe);

  if (!boxrange(unr)) return;
  WITH	= user[unr];
  if (!strcmp(eingabe, WITH->password)) {
    WITH->se_ok		= false;
    WITH->level		= WITH->plevel;
    WITH->is_authentic	= (WITH->pwsetat + holddelay < clock_.ixtime);

    switch (WITH->pwmode) {

    case PWM_SEND_ERASE:
    case PWM_MUST_PRIV:
      WITH->se_ok	= true;
      break;

    case PWM_RSYSOP:
    case PWM_RSYSOPPRIV:
      WITH->se_ok	= true;
      WITH->rsysop	= true;
      break;

    case PWM_SYSOPPRIV:
    case PWM_SYSOP:
      WITH->se_ok	= true;
      WITH->supervisor	= true;
      break;
    }
    begruessung(unr);
    if (user[unr] != NULL)
      /* suppress duplicate Logon prompts */
      WITH->action	= 86;
    return;
  }
  if (last)
    do_quit(unr, false, false);
  else {
    wuser(unr, "Password: ");
    WITH->action++;
  }
}


static void switch_bbsbcast(short unr, char *w)
{
  wuser(unr, "BROADCAST was ");
  if (send_bbsbcast)
    wlnuser(unr, "ON");
  else
    wlnuser(unr, "OFF");
  if (*w != '\0')
    send_bbsbcast = (strcmp(w, "ON") == 0);
  wuser(unr, "BROADCAST now ");
  if (send_bbsbcast)
    wlnuser(unr, "ON");
  else
    wlnuser(unr, "OFF");
}


static void set_debuglevel(short unr, char *w)
{
  short k;

  if (*w != '\0') {
    k		= atoi(w);
    if (k < 0)
      k		= 0;
    else if (k > 6)
      k		= 6;
    debug_level	= k;
  }
  wuser(unr, "DEBUG ");
  swuser(unr, debug_level);
  wlnuser0(unr);
}


boolean set_reply_flag(short unr, char *brett, short nr)
{
  boolean	Result;
  short		k, x;
  indexstruct	header;
  pathstr	STR1;

  Result = false;
  if (!boxrange(unr))
    return Result;
  if (strcmp(user[unr]->call, brett))
    return Result;
  user[unr]->in_reply = true;
  idxfname(STR1, brett);
  k = sfopen(STR1, FO_RW);
  if (k >= minhandle) {
    read_index(k, nr, &header);
    if (check_hcs(header)) {
      x = in_readby(brett, header.readby);
      if (x > 0)
	header.readby[x - 2] = ',';
      else
	add_readby(unr, &header, false);
      write_index(k, nr, &header);
      Result = true;
    }
    sfclose(&k);
  }
  user[unr]->in_reply = false;
  return Result;
}


static boolean get_reply_info(short unr, char *eingabe, indexstruct *rep_header)
{
  boolean	direct_reply;
  short		i;
  short		rep_handle;
  userstruct	*WITH;
  char		w[256], w1[256];
  pathstr	STR1;

  if (!boxrange(unr))
    return false;
  WITH = user[unr];
  if (*eingabe != '\0') {
    direct_reply = false;
    get_word(eingabe, w);
    get_word(eingabe, w1);
    if (zahl(w) && atoi(w) > 0) {
      strcpy(WITH->reply_brett, WITH->brett);
      WITH->reply_nr = atoi(w);
    } else {
      i = strlen(w);
      if (i > 0 && i <= LEN_BOARD && atoi(w1) > 0) {
	strcpy(WITH->reply_brett, w);
	WITH->reply_nr = atoi(w1);
      }
    }
  } else {
    direct_reply = true;
  }
  
  upper(WITH->reply_brett);

  idxfname(STR1, WITH->reply_brett);

  if (WITH->reply_nr <= 0)
    return false;
  if (direct_reply || WITH->supervisor) {
    if (sfsize(STR1) / sizeof(indexstruct) < WITH->reply_nr)
      return false;
  } else {
    if (last_valid(unr, WITH->reply_brett) < WITH->reply_nr)
      return false;
  }
  
  rep_handle = sfopen(STR1, FO_READ);
  if (rep_handle < minhandle)
    return false;
  read_index(rep_handle, WITH->reply_nr, rep_header);
  sfclose(&rep_handle);
  if (!rep_header->deleted || direct_reply || WITH->supervisor)
    return true;
  return false;
}


static void set_reply_flag_directly(short unr, char *eingabe)
{
  indexstruct	header;

  if (!get_reply_info(unr, eingabe, &header))
    return;
  if (set_reply_flag(unr, user[unr]->reply_brett, user[unr]->reply_nr))
    wlnuser(unr, "OK");
  else
    wlnuser(unr, "NO");
}


static boolean set_reply_address(boolean for_comment, short unr,
				 char *eingabe, char *hs, char *msgtype)
{
  boolean	Result;
  userstruct	*WITH;
  indexstruct	rep_header;
  calltype	rep_call;
  boardtype	rep_brett;
  subjecttype	rep_title, rep_title2;
  calltype	rep_mbx, rep_box;
  char		w2[256];
  char		STR13[256];

  Result = false;
  if (!get_reply_info(unr, eingabe, &rep_header))
    return Result;

  WITH = user[unr];
  strcpy(rep_call, rep_header.absender);
  strcpy(rep_brett, rep_header.dest);
  strcpy(rep_title2, rep_header.betreff);
  unhpath(rep_header.verbreitung, rep_mbx);
  strcpy(rep_box, rep_header.sendbbs);
  cut(rep_title2, 77);
  sprintf(rep_title, "RE:%s", rep_title2);
  check_replytitle(rep_title);
  strcpy(eingabe, rep_title);

  if (!for_comment) {
    user_mybbs(rep_call, w2);
    if (*w2 == '\0')
      strcpy(w2, rep_box);
    if (*w2 != '\0')
      sprintf(hs, "%s @ %s", rep_call, w2);
    else
      strcpy(hs, rep_call);

    sprintf(STR13, "REPLY: %s", hs);
    wlnuser(unr, STR13);
    sprintf(hs + strlen(hs), " %s", eingabe);
    sprintf(STR13, "       %s", eingabe);
    wlnuser(unr, STR13);
    wlnuser0(unr);
    *msgtype = 'P';
    return true;
  }

  if (callsign(rep_brett)) {
    user_mybbs(rep_brett, w2);
    if (*w2 == '\0')
      strcpy(w2, rep_mbx);
  } else
    strcpy(w2, rep_mbx);
  if (*w2 != '\0')
    sprintf(hs, "%s @ %s", rep_brett, w2);
  else
    strcpy(hs, rep_brett);
  sprintf(STR13, "COMMENT: %s", hs);
  wlnuser(unr, STR13);
  sprintf(hs + strlen(hs), " %s", eingabe);
  sprintf(STR13, "         %s", eingabe);
  wlnuser(unr, STR13);
  wlnuser0(unr);
  *msgtype = '\0';   /* sucht send_check() sich dann raus */
  return true;
}

static void xreply(boolean for_comment, short unr, char *eingabe)
{
  indexstruct	header;
  char	hs[256], w[256], crcs[256+40];

  if (!get_reply_info(unr, eingabe, &header)) return;
  if (!for_comment) set_reply_flag(unr, user[unr]->reply_brett, user[unr]->reply_nr);
  snprintf(hs, 255, "%s %d", user[unr]->reply_brett, user[unr]->reply_nr);
  if (!read_for_reply(for_comment, unr, hs, w)) return;
  snprintf(hs, 255, "%s %s", xeditor, w);
  snprintf(crcs, 256+39, "%s CRC:%d %ld", w, file_crc(0, w, 0xFFFF, 0, 0), sfsize(w));
  add_zombie(my_exec(hs), crcs, 1); /* this imports the tmp file after exit of editor */
  wuser(unr, "OK, started X-editor with file ");
  wlnuser(unr, w);
}

static void read_next(short unr)
{
  short		number;
  indexstruct	lheader;
  userstruct	*WITH;
  boardtype	board;
  char		search[2];

  if (!boxrange(unr))
    return;
  WITH = user[unr];
  strcpy(board, WITH->reply_brett);
  number = WITH->reply_nr;
  number++;
  *search = '\0';
  read_brett(unr, nohandle, board, number, number, 100, WITH->lastroption, search, 0, &lheader);
}


static void mkf_cmd(short unr, char *eingabe)
{
  indexstruct	header;
  char		hs[256], w[256], STR1[256];

  if (!boxrange(unr))
    return;

  if (!get_reply_info(unr, eingabe, &header)) {
    wln_btext(unr, 65);
    return;
  }

  sprintf(hs, "%s/%s/$%s", header.absender, header.dest, header.id);
  cut(hs, 79);
  wlnuser(unr, hs);
  wlnuser0(unr);
  sprintf(STR1, "F@%s #14 %s", e_m_verteiler, hs);
  send_check(unr, STR1, true, 'B');
  if (user[unr]->action != 74) {
    wln_btext(unr, 11);
    return;
  }

#ifdef __NetBSD__
  sprintf(hs, "FMAIL / dpbox (NetBSD) v%s%s", dp_vnr, dp_vnr_sub);
#else
  #ifdef __linux__
  sprintf(hs, "FMAIL / dpbox (Linux) v%s%s", dp_vnr, dp_vnr_sub);
  #else
    #ifdef __macos__
  sprintf(hs, "FMAIL / dpbox (MacOS) v%s%s", dp_vnr, dp_vnr_sub);
    #else
  sprintf(hs, "FMAIL / dpbox v%s%s", dp_vnr, dp_vnr_sub);
    #endif
  #endif
#endif
  wlnuser(unr, hs);
  send_text3(unr, true, hs, true);
  if (user[unr]->action == 74)
    user[unr]->action = 75;
  strcpy(hs, "-----------------------------------");
  wlnuser(unr, hs);
  send_text3(unr, false, hs, true);
  sprintf(hs, "Sender     : %s", header.absender);
  wlnuser(unr, hs);
  send_text3(unr, false, hs, true);
  sprintf(hs, "Board      : %s @ %s", header.dest, header.verbreitung);
  wlnuser(unr, hs);
  send_text3(unr, false, hs, true);
  sprintf(hs, "BID        : $%s", header.id);
  wlnuser(unr, hs);
  send_text3(unr, false, hs, true);
  sprintf(hs, "Lifetime   : %d", header.txlifetime);
  wlnuser(unr, hs);
  send_text3(unr, false, hs, true);
  sprintf(hs, "Subject    : %s", header.betreff);
  cut(hs, 79);
  wlnuser(unr, hs);
  send_text3(unr, false, hs, true);
  strcpy(STR1, header.sendbbs);
  if (*STR1 != '\0') {
    complete_hierarchical_adress(STR1);
    sprintf(hs, "Created by : %s", STR1);
    wlnuser(unr, hs);
    send_text3(unr, false, hs, true);
  }
  if (header.txdate != 0) {
    ix2string4(header.txdate, w); /* 27.12.2000 12:12 */
    w[16] = '\0';
    sprintf(hs, "Created at : %s UTC", w);
    wlnuser(unr, hs);
    send_text3(unr, false, hs, true);
  }
  strcpy(hs, "-----------------------------------");
  wlnuser(unr, hs);
  send_text3(unr, false, hs, true);
  *hs = '\0';
  wlnuser(unr, hs);
  send_text3(unr, false, hs, true);
  wln_btext(unr, 83);
}


static void set_selection(short unr, char *hs)
{
  short		x;
  userstruct	*WITH;
  char		w[256];

  if (!boxrange(unr)) return;

  WITH			= user[unr];
  WITH->msgselection	= 0;
  WITH->hidden		= false;
  WITH->undef		= false;

  for (x = 0; hs[x] != '\0'; x++) {
    if (hs[x] == ',')
      hs[x]	= ' ';
  }

  get_word(hs, w);
  while (*w != '\0') {
    upper(w);
    if (compare(w, "SFWAIT"))
      WITH->msgselection |= MSG_SFWAIT;
    if (compare(w, "PROPOSED"))
      WITH->msgselection |= MSG_PROPOSED;
    if (compare(w, "SFRX"))
      WITH->msgselection |= MSG_SFRX;
    if (compare(w, "SFTX"))
      WITH->msgselection |= MSG_SFTX;
    if (compare(w, "SFERR"))
      WITH->msgselection |= MSG_SFERR;
    if (compare(w, "SFNOFOUND"))
      WITH->msgselection |= MSG_SFNOFOUND;
    if (compare(w, "CUT"))
      WITH->msgselection |= MSG_CUT;
    if (compare(w, "MINE"))
      WITH->msgselection |= MSG_MINE;
    if (compare(w, "BROADCAST"))
      WITH->msgselection |= MSG_BROADCAST;
    if (compare(w, "BROADCAST_RX"))
      WITH->msgselection |= MSG_BROADCAST_RX;
    if (compare(w, "OWNHOLD"))
      WITH->msgselection |= MSG_OWNHOLD;
    if (compare(w, "LOCALHOLD"))
      WITH->msgselection |= MSG_LOCALHOLD;
    if (compare(w, "SFHOLD"))
      WITH->msgselection |= MSG_SFHOLD;
    if (compare(w, "REJECTED"))
      WITH->msgselection |= MSG_REJECTED;
    if (compare(w, "DOUBLE"))
      WITH->msgselection |= MSG_DOUBLE;
    if (compare(w, "OUTDATED"))
      WITH->msgselection |= MSG_OUTDATED;
    if (compare(w, "UNDEF"))
      WITH->undef = true;
    if (compare(w, "HIDDEN") && WITH->supervisor)
      WITH->hidden = true;
    get_word(hs, w);
  }
}


static boolean find_env(char *w)
{
  char	res[256];

  sprintf(res, "%s%s", boxdir, w);
  if (exist(res)) {
    strcpy(w, res);
    return true;
  }
  sprintf(res, "%s%s", boxsysdir, w);
  if (exist(res)) {
    strcpy(w, res);
    return true;
  }
  sprintf(res, "%s%s", boxsfdir, w);
  if (exist(res)) {
    strcpy(w, res);
    return true;
  }
  sprintf(res, "%s%s", boxdir, w);
  if (exist(res)) {
    strcpy(w, res);
    return true;
  }
  sprintf(res, "%s%s%cbox", boxsysdir, w, extsep);
  if (exist(res)) {
    strcpy(w, res);
    return true;
  }
  sprintf(res, "%s%s%csf", boxsfdir, w, extsep);
  if (exist(res)) {
    strcpy(w, res);
    return true;
  }
  if (!strcmp(w, "dpbox.ini")) {
    strcpy(w, dpbox_initfile);
    return true;
  }
  return false;
}


static void do_checkboards(short unr, boolean want, char *w)
{
  char		cb[LEN_CHECKBOARDS+1];
  userstruct	*WITH;
  char		STR1[256];

  if (!boxrange(unr))
    return;

  WITH = user[unr];
  if (*w != '\0') {
    if (w[0] != ',')
      sprintf(w, ",%s", strcpy(STR1, w));
    if (w[strlen(w) - 1] != ',')
      strcat(w, ",");

    if (want != WITH->wantboards)
      *WITH->checkboards = '\0';
    WITH->wantboards = want;
    strcpy(cb, WITH->checkboards);

    if (*cb != '\0') {
      strdelete(w, 1, 1);
      if (strlen(cb) + strlen(w) <= LEN_CHECKBOARDS)
	strcat(cb, w);
      else
	wln_btext(unr, 84);
    } else
      strcpy(cb, w);

    strcpy(WITH->checkboards, cb);
    if (want) {
      wuser(unr, "WantBoard : ");
    } else {
      wuser(unr, "NotBoard : ");
    }

    strdelete(cb, 1, 1);
    strdelete(cb, strlen(cb), 1);
    show_puffer(unr, cb, strlen(cb));
    wlnuser0(unr);

  } else {
    *WITH->checkboards = '\0';
    wln_btext(unr, 85);
  }

  save_userfile(user[unr]);

}

static boolean may_invoke_xeditor(unr)
{
  if (!boxrange(unr)) return false;
  return (user[unr]->pchan == 0 && user[unr]->supervisor
      	  && user[unr]->umode == UM_USER && *xeditor);
}

boolean analyse_boxcommand(short unr, char *eingabe, char *voll, boolean return_)
{
  short			x, threshold, cnr, TEMP;
  long			s, e;
  unsigned short	lt, a;
  boolean		plus, auto7ptmp, Result, valid_command, valid_command2, resumed;
  boolean		onlysys, rep_ok;
  userstruct		*WITH;
  indexstruct		lheader;
  char	      	      	*sp;
  char			msgtype;
  char			w[256], w1[256], w2[256], cmd[256], search[256], hs[256], option[256];
  char			STR1[256], STR7[256], STR14[34];

  debug(6, unr, 56, eingabe);

  Result = false;

  if (!boxrange(unr)) {
    *eingabe = '\0';
    return Result;
  }

  WITH			= user[unr];
  valid_command		= true;
  valid_command2	= false;
  option[0]		= '\0';
  plus			= false;
  resumed		= (WITH->lastprocnumber != 0);

  if (WITH->action == 0) {
    WITH->in_reply	= false;

    if (*eingabe != '\0')
      strcpy(WITH->lastcmd, eingabe);

    if (debug_level > 0 && debug_level < 6) {
      if (resumed && (strlen(eingabe)+10 < 256)) {
	sprintf(STR1, "%s (resumed)", eingabe);
	debug(1, unr, 56, STR1);
      } else
	debug(1, unr, 56, eingabe);
    }

    *w			= '\0';
    get_word(eingabe, w2);
    strcpyupper(cmd, w2);
    cut(cmd, 50);
    
    cnr			= find_command(unr, cmd, &onlysys);
    if (cnr == 71)
      onlysys		= false;

    if (cnr == 7  || cnr == 31  || cnr == 104)
      WITH->brett[0]	= '\0';

    if (cnr == 1  || cnr == 2   || cnr == 5  || cnr == 7  || cnr == 19 ||
	cnr == 27 || cnr == 104 || cnr == 95 || cnr == 47 || cnr == 38 ||
	cnr == 36 || (cnr >= 30 && cnr <= 32))

      get_numbers(unr, (cnr == 7 || cnr == 31 || cnr == 104),
      			eingabe, &s, &e, &threshold, option, search);

    if ((cnr >= 8 && cnr <= 13)  || (cnr >= 15 && cnr <= 17) || cnr == 20  ||
	(cnr >= 22 && cnr <= 25) || cnr == 28  || cnr == 29  || cnr == 117 ||
	cnr == 111 || cnr == 102 || cnr == 94  || cnr == 93  || cnr == 92  ||
	cnr == 89  || cnr == 88  || cnr == 80  || cnr == 78  || cnr == 73  ||
	cnr == 72  || cnr == 70  || (cnr >= 64 && cnr <= 66) || cnr == 55  ||
	(cnr >= 51 && cnr <= 52) || (cnr >= 37 && cnr <= 38) || cnr == 33  ||
	cnr == 128 || cnr == 129 ){

      get_word(eingabe, w);
      upper(w);

    } else if (cnr == 3  || cnr == 26  || cnr == 27  || cnr == 101 || cnr == 100 ||
	       cnr == 91 || (cnr >= 81 && cnr <= 83) || cnr == 76  ||
	       cnr == 57 || cnr == 54)

      get_word(eingabe, w);   /* Linux will gerne auch kleine Filenamen */

    if (cnr != 2 && cnr != 108 && cnr != 95)
      *WITH->lastroption = '\0';

    if (cnr == 43)
      strcpy(WITH->lastcmd, "*** PASSWORD");

    if (!resumed) {
      if (create_syslog && onlysys && unr != 11)
	append_syslog(unr);
      else if (create_sflog && WITH->f_bbs)
	append_sflog(unr);
      else if (create_userlog /* && !onlysys */ && !WITH->f_bbs && cnr != 45)
	append_userlog(unr);
      if (in_protocalls(user[unr]->call))
	append_protolog(unr);
    }
    
    if (cnr == 9) {  /* Fuer die lieben Baycom-Benutzer */
      if (compare(w, "NEWS") || compare(w, "MESSAGES")) {
	cnr		= 104;   /* DIR NEWS -> CS / DIR MESSAGES -> CS < search */
        WITH->brett[0]	= '\0';
	get_numbers(unr, true, eingabe, &s, &e, &threshold, option, search);
      } else if (compare(w, "SENT")) {
	cnr		= 104;   /* DIR SENT -> CS < call */
        WITH->brett[0]	= '\0';
	s		= 1;
	e		= LONG_MAX;
	strcpy(search, WITH->call);
      } else if (compare(w, "USERS")) {
	if (*eingabe != '\0') {
	  cnr		= 104;   /* DIR USERS search -> CS + < search */
          WITH->brett[0]= '\0';
	  get_numbers(unr, true, eingabe, &s, &e, &threshold, option, search);
	  strcat(option, "+");
	}
      }
    }
    
    /* falls da jemand s&f einleiten will etc. keine Leerzeile vorneweg senden	*/
    /* auch nicht bei FEXECUTE, RUN, TRUN, SHELL, TSHELL und UM_FILEOUT		*/
    if (!resumed
    	&& WITH->umode != UM_FILEOUT
    	&& WITH->umode != UM_SINGLEREQ
    	&& cnr != 45 && cnr != 83  && cnr != 98
    	&& cnr != 99 && cnr != 121 && cnr != 122
	&& (WITH->errors >= 0 || WITH->errors == -5))
      wlnuser0(unr);

    if (WITH->force_priv > 0) {
      if ((cnr != 10)  && (cnr != 45) && (cnr != 123)	 /* HELP, SID, MD5	*/
        && (cnr != 14) && (cnr != 15) && (cnr != 65)	 /* ABORT, QUIT, COMP	*/
        && (cnr != 41) && (cnr != 91) && (cnr != 117)) { /* SYSTEM, PRIV, MD2	*/
        wln_btext(unr, 65);
        WITH->errors++;
        return false;
      }
    }

    /* DL5HBF: Wenn LOGINCHECK=ON und User keine unbekannte Box ist, wird nur */
    /*         die Eingabe von NAME und MYBBS und HELP zugelassen!!!          */
    /*         Wenn unbekannte User S&F machen wollen, muessen sie auch ERST  */
    /*         NAME und MYBBS benutzen!!!                                     */
    /* 961113 J.Schurig: some modifications                                   */
    /* 961216 J.Schurig: only if umode == UM_USER (!TELL-requests...)         */
    /* 000222 J.Schurig: only if !direct_sf   	      	      	      	      */
      
    if (login_check && (WITH->sf_level == 0) && (!WITH->direct_sf)
    	&& (WITH->umode == UM_USER || WITH->umode == UM_FILEOUT)) {
      if ((*WITH->name == '\0') || (*WITH->mybbs == '\0') || (WITH->mybbsmode != 'U')) {
        if ( (cnr != 20) && (cnr != 28) && (cnr != 10)	   /* HELP, NAME, MYBBS	*/
          && (cnr != 14) && (cnr != 15) && (cnr != 123)	   /* ABORT, QUIT, MD5	*/
          && (cnr != 41) && (cnr != 91) && (cnr != 117)) { /* SYSTEM, PRIV, MD2	*/
          if (*WITH->name == '\0')
            wln_btext(unr, 21);
          if (*WITH->mybbs == '\0' || WITH->mybbsmode != 'U')
            wln_btext(unr, 22);
          WITH->errors++;
          return false;
        }
      }
    }

    /* ============================================================================= */
    /* Kommandointerpreter der Box. Im Pseudo-Multitasking ausgefuehrte Befehle sind */
    /* mit einem fuehrenden * markiert                                               */
    /* ============================================================================= */

    switch (cnr) {

    case 0:
      valid_command = false;
      if (WITH->errors < maxerrors && WITH->errors >= 0)
	wln_btext(unr, 1);
      break;

    /* LIST */
    case 1:
      if (WITH->brett[0] == '\0')
        strcpy(WITH->brett, WITH->call);
      list_brett(unr, WITH->brett, s, e, threshold, option, search);
      break;

    /* *READ / *HEADER */
    case 2:
    case 95:
      if (WITH->brett[0] == '\0')
        strcpy(WITH->brett, WITH->call);
      if (cnr == 95)   /*HEADER*/
	strcat(option, "=");
      strcpy(WITH->lastroption, option);
      read_brett(unr, nohandle, WITH->brett, s, e, threshold, option, search, 0, &lheader);
      break;

    /* IMPORT */
    case 3:
      if (disk_full)
	wln_btext(unr, 86);
      else {
	strcpyupper(w1, w);
	if (!strcmp(w1, "DIEBOX")) {
	  auto7ptmp = auto7plusextract;
	  auto7plusextract = false;
	  start_crawl(-1, "-q");
	  conv_diebox(unr, eingabe);
	  auto7plusextract = auto7ptmp;
	}
	else if (!strcmp(w1, "BAYBOX")) {
	  auto7ptmp = auto7plusextract;
	  auto7plusextract = false;
	  start_crawl(-1, "-q");
	  conv_baybox(unr, eingabe);
	  auto7plusextract = auto7ptmp;
	}
	else if (!strcmp(w1, "PW"))
          import_pw(unr, eingabe);
        else {
	  if (!WITH->console || *w != '/') {
	    if (insecure(w))
	      *w = '\0';
	    else
	      sprintf(w, "%s%s", boxdir, strcpy(STR1, w));
	  }
	  if (exist(w))
	    send_file0(unr, false, w);
	  else
	    wln_btext(unr, 4);
	}
      }
      break;

    /* SEND/REPLY/COMMENT */
    /* SB/SP/SR		  */
    case 4:
    case 48:
    case 49:
      msgtype = '\0';
      if (WITH->tell == nohandle && !WITH->in_begruessung) {

	/* se_ok wird jetzt in SEND_CHECK geprueft, msg an   	*/
	/* CONSOLE_CALL und Sysops ist erlaubt  		*/

	if (disk_full)
	  wln_btext(unr, 86);
	else {
	  if (in_real_sf(WITH->call) && !WITH->supervisor) {
	    get_btext(unr, 87, w2);
	    sprintf(hs, "%d %s", unr, w2);
	    debug(0, unr, 56, "attempt to send without SID");
	    disc_user(-1, hs);
	  } else {
	    WITH->input2[0] = '\0';
	    if (cnr == 49 || cnr == 48) {
	      if (may_invoke_xeditor(unr)) {
	      	xreply(cnr == 49, unr, eingabe);
		break;
	      } else {
	      	rep_ok = set_reply_address(cnr == 49, unr, eingabe, hs, &msgtype);
	      	WITH->in_reply = (rep_ok && !strcmp(WITH->reply_brett, WITH->call));
	      }
	    } else {
	      rep_ok = false;
	      strcpy(hs, eingabe);
	      if (strlen(cmd) == 2) {
		if (cmd[1] == 'P' || cmd[1] == 'B')
		  msgtype = cmd[1];
	      }
	    }
	    if (cnr != 4 && !rep_ok)
	      wln_btext(unr, 37);
	    else
	      send_check(unr, hs, true, msgtype);
	  }
	}
      } else
	wln_btext(unr, 65);
      break;

    /* *ERASE / *KILL */
    case 5:
    case 47:
      if (WITH->se_ok)
	erase_brett(unr, WITH->brett, s, e, threshold, option, search, false, cnr == 47);
      else
	wln_btext(unr, 65);
      break;

    /* TELL */
    case 6:
      if (WITH->se_ok && !WITH->in_begruessung) {
	upper(eingabe);
	get_word(eingabe, w);		/* destination call	*/
	unhpath(w, hs);
	if (callsign(hs)) {
	  get_word(eingabe, w1);	/* remote command	*/
	  if (strcmp(hs, Console_call) && strchr(eingabe, ';') == NULL && strlen(eingabe) <= 80) {
	    sprintf(w2, "%s %s @ %s", w1, eingabe, ownhiername);
	    sprintf(STR1, "%s%s%cTELXXXXXX", tempdir, WITH->call, extsep);
      	    mymktemp(STR1);
	    append(STR1, "        ", true);
	    strcpy(user[unr]->input2, STR1);
	    sprintf(hs, "T @ %s < %s %s", w, WITH->call, w2);
	    user[unr]->tell	= -99;	/* ???	*/
	    send_check(unr, hs, false, 'P');
	    user[unr]->tell	= nohandle;
	    sfdelfile(STR1);		/* delete the dummy file after import	*/
	    WITH->brett[0]	= '\0';
	  } else
	    wln_btext(unr, 2);
	} else
	  wln_btext(unr, 2);
      } else
	wln_btext(unr, 65);
      break;
      
    /* HOLD */
    case 31:
      show_hold(unr, SHORT_MAX);
      break;

    /* *CHECK / *CS */
    case 7:
    case 104:
      if (s == e)
	s = 1;
      plus = (strchr(option, '+') != NULL);
      if (WITH->lastprocnumber != 2)
	check(unr, search, s, e, threshold, plus, option, WITH->brett, cnr == 104);
      break;

    /* USAGE */
    case 8:
      if (WITH->supervisor && !strcmp(w, "PW")) {
	show_pwuser(unr);
	break;
      }

      if (WITH->supervisor && !strcmp(w, "LEVEL")) {
	show_leveluser(unr);
	break;
      }

      if (WITH->supervisor && (!strcmp(w, "OFF") || !strcmp(w, "KILL"))) {
	gesperrt = true;
	if (!strcmp(w, "KILL"))
	  disc_user(unr, "USER maintenance works, sri. try later.");
	if (create_syslog)
	  append_syslog(unr);
	break;
      }

      if (WITH->supervisor && !strcmp(w, "ON")) {
	gesperrt = false;
	if (create_syslog)
	  append_syslog(unr);
      	break;
      }

      if (WITH->supervisor && (!strcmp(w, "HISCORE") || !strcmp(w, "HIGHSCORE"))) {
	sprintf(hs, " %d", hiscore_connects);
	wln_btext(unr, 88);
	wlnuser(unr, hs);
	if (useq(eingabe, "-d")) {
	  hiscore_connects = 0;
	  update_bidseek();
	  flush_bidseek();
	  wln_btext(unr, 89);
	}
	break;
      }

      if (callsign(w))
	strcpy(option, "+");
      else if (w[0] == 'U' || w[0] == '%')
	strcpy(option, "%");
      else if (w[0] == '@') {
	strcpy(option, "@");
	strdelete(w, 1, 1);
	if (WITH->supervisor && uspos("-A", eingabe) > 0)
	  strcpy(option, "!");
	else if (uspos("-C", eingabe) > 0)
	  strcat(option, "/");
      }
      get_word(eingabe, w1);
      if (WITH->supervisor && useq(w1, "-D") && strchr(option, '!') == NULL) {
      	strcat(option, "-");
	if (create_syslog)
	  append_syslog(unr);
      }
      show_user(unr, w, option);
      break;

    /* *DIR */
    case 9:
      if (compare(w, "USERS"))
	show_dir(unr, true, true, false);
      else if (compare(w, "INFO"))
	show_dir(unr, false, true, false);
      else if (compare(w, "ALL")) {
	if (WITH->lastprocnumber == 0)
	  WITH->lastprocnumber3 = 0;
	if (WITH->lastprocnumber3 == 0)
	  show_dir(unr, false, false, false);
	if (WITH->lastprocnumber == 0) {
	  WITH->lastprocnumber3 = 1;
	  wlnuser0(unr);
	}
	if (WITH->lastprocnumber3 == 1)
	  show_dir(unr, true, false, false);
      } else if (compare(w, "LOST") /* && WITH->supervisor */ )
	show_dir(unr, true, true, true);
      else
	show_dir(unr, false, false, false);
      break;

    /* HELP */
    case 10:
      show_help(unr, w);
      break;

    /* HIDDEN */
    case 11:
      WITH->hidden = positive_arg(w);
      break;

    /* LOGINTIME */
    case 12:
      new_login(unr, w);
      break;

    /* QUIT / ABORT */
    case 13:
    case 14:
      if (WITH->tell == nohandle && !WITH->in_begruessung) {
	if (!WITH->se_ok && WITH->pwmode >= 8)
	  cnr = 14;

	if (cnr == 13 && !strcmp(w, "R"))
	{  /* reset der loginzeit, nichts weiter */
	  WITH->lastdate = WITH->logindate;
	  save_userfile(user[unr]);
	  box_logbuch(unr);
	  WITH->logindate = clock_.ixtime;
	  for (x = 1; x <= MAXUSER; x++) {
	    if (user[x] != NULL) {
	      if (!strcmp(user[x]->call, WITH->call)) {
		user[x]->lastdate = WITH->lastdate;
		user[x]->logindate = WITH->logindate;
	      }
	    }
	  }
	  wlnuser(unr, "OK");
	} else
	  do_quit(unr, cnr == 14, true);

      }
      break;

    /* *STATISTIK */
    case 15:
      show_statistik(unr, w);
      break;

    /* VERSION */
    case 16:
      show_version(unr, !strcmp(w, "X") || !strcmp(w, "+"));
      break;

    /* TIME */
    case 18:
      show_systemtime(unr);
      break;

    /* *TRANSFER */
    case 19:
      if (WITH->se_ok) {
	upper(eingabe);
	transfer(unr, WITH->brett, s, e, eingabe);
      } else
	wln_btext(unr, 65);
      break;

    /* NAME */
    case 20:
      if (WITH->se_ok) {
	if (callsign(w) && WITH->supervisor && *eingabe != '\0') {
	  if (create_syslog)
	    append_syslog(unr);
	  strcpy(w2, w);
	} else {
	  strcpy(w2, WITH->call);
	  sprintf(eingabe, "%s %s", w, strcpy(STR7, eingabe));
	}
	del_mulblanks(eingabe);
	del_lastblanks(eingabe);
	if (*eingabe != '\0' || WITH->tell == nohandle)
	  change_name(unr, w2, eingabe);
      } else
	wln_btext(unr, 65);
      break;

    /* BBS */
    case 21:
      del_leadblanks(eingabe);
      upper(eingabe);
      if (WITH->supervisor && !strcmp(eingabe, "HASH")) {
	disp_hboxhash(unr);
      	break;
      }
      if (*eingabe != '\0' || WITH->console)
	show_bbs_info(unr, eingabe);
      else
	wln_btext(unr, 2);
      break;

    /* SPEAK */
    case 22:
      if (WITH->se_ok) {
	if (callsign(w) && WITH->supervisor && *eingabe != '\0') {
	  if (create_syslog)
	    append_syslog(unr);
	  strcpy(w2, w);
	  get_word(eingabe, w);
	  upper(w);
	} else
	  strcpy(w2, user[unr]->call);
	change_language(unr, w2, w);
      } else
	wln_btext(unr, 65);
      break;

    /* SF */
    case 23:
      if (callsign(w))
	start_sf(unr, w, eingabe);
      else {
	if (!strcmp(w, "ON"))
	  sf_allowed = true;
	else if (!strcmp(w, "OFF") || !strcmp(w, "KILL")) {
	  sf_allowed = false;
	  if (!strcmp(w, "KILL"))
	    disc_user(unr, "SF");
	  kill_all_routing_flags();
	}
      }
      break;

    /* SFLEVEL / LEVEL / PWMODE */
    case 25:
    case 29:
    case 66:
      if (WITH->tell == nohandle && WITH->supervisor) {
	if (callsign(w) && *eingabe != '\0') {
	  get_word(eingabe, w1);
	  s = atoi(w1);
	  change_levels(unr, cnr, w, s);
	} else
	  wln_btext(unr, 3);
      }
      break;

    /* EXPORT */
    case 27:
      if (WITH->se_ok) {
	if (disk_full)
	  wln_btext(unr, 86);
	else {
	  if (!WITH->console) {
	    if (insecure(w))
	      *w = '\0';
	    else if (*w != '\0')
	      sprintf(w, "%s%s", boxdir, strcpy(STR7, w));
	  }
	  export_brett(unr, WITH->brett, s, e, threshold, option, search, w);
	}
      } else
	wln_btext(unr, 65);
      break;

    /* MYBBS / SETUSR */
    case 28:
      if (WITH->se_ok) {
      	if (WITH->supervisor && useq(w, "WP")) {
	  generate_wp_files();
	  generate_wprot_files();
	  break;
	}
	unhpath(w, w1);
	if (callsign(w1)) {
	  get_word(eingabe, w1);
	  upper(w1);
	  unhpath(w1, w2);
	  if ((*w2 != '\0') && WITH->supervisor) {
	    if (callsign(w2)) {
	      if (create_syslog)
	        append_syslog(unr);
	      cut(eingabe, 8);
	      change_mybbs(unr, w, w1, 0, eingabe, 'U', false, strchr(eingabe, '+') != NULL);
	    } else
	      wln_btext(unr, 3);
	  }
	  else if (*w2 != '\0') {
	    wln_btext(unr, 2);
	  }
	  else {
	    if (useq(WITH->call, w) && !gen_sftest2(-1, WITH->call, WITH->call)) {
	      wln_btext(unr, 13);
	      wln_btext(unr, 11);
	    } 
	    else change_mybbs(unr, WITH->call, w, 0, "", 'U', false, strcmp(w1, "-") != 0);
          }
	} else
	  wln_btext(unr, 3);
      } else
	wln_btext(unr, 65);
      break;

    /* *MBX */
    case 30:
      if (WITH->se_ok) {
	upper(eingabe);
	if (strlen(eingabe) >= 1 && strlen(eingabe) <= 40)
	  change_mbx(unr, WITH->brett, s, e, eingabe, option);
	else
	  wln_btext(unr, 2);
      } else
	wln_btext(unr, 65);
      break;

    /* *LIFETIME */
    case 32:
      if (WITH->se_ok)
	change_lifetime(unr, WITH->brett, s, e, threshold, eingabe, option, search);
      else
	wln_btext(unr, 65);
      break;

    /* GARBAGE */
    case 33:
      garbage_collection(strcmp(w, "+") == 0, strcmp(w, "-F") == 0,
			 strcmp(w, "-A") == 0, strcmp(w, "-I") == 0, unr);
      break;

    /* FHEADER */
    case 34:
      sprintf(STR14, "[%s]", ownfheader);
      wlnuser(unr, STR14);
      break;

    /* HNAME */
    case 35:
      wlnuser(unr, ownhiername);
      break;

    /* *RELEASE */
    case 36:
      release_hold(unr, WITH->brett, s, e, option, search);
      break;

    /* SFPARMS */
    case 37:
      if (callsign(w)) {
	show_sfparms(unr, w);
	TEMP = count_words(eingabe);
	if ((unsigned)TEMP < 32 && ((1L << TEMP) & 0x44) != 0) {
	  set_sfparms(w, eingabe);
	  show_sfparms(unr, w);
	}
      } else {
	*w = '\0';
	show_sfparms(unr, w);
      }
      break;

    /* FORWARD */
    case 38:
      if (WITH->se_ok) {
	check_lt_acc(WITH->brett, &lt, &a);
	if (!strcmp(w, "@")) {
	  get_word(eingabe, w);
	  upper(w);
	}
	if (a <= WITH->level && callsign(w)) {
	  if (strchr(option, '+') != NULL)
	    sprintf(w, "!%s", strcpy(STR7, w));
	  set_forward(unr, -1, WITH->brett, option, s, e, w, "", "");
	} else
	  wln_btext(unr, 3);
      } else
	wln_btext(unr, 65);
      break;

    /* RELOAD */
    case 39:
      load_all_parms(unr);
      wlnuser(unr, "OK, reloaded complete configuration");
      break;

    /* BOXLOG / REBUILD */
    case 40:
    case 50:
      create_new_boxlog(unr, cnr == 50);
      break;

    /* SYSTEM */
    case 41:
      if (WITH->tell == nohandle)
	answer_sysrequest(unr, false, false);
      break;

    /* SYSCHK */
    case 42:
      if (WITH->supervisor)
	wlnuser(unr, "OK");
      break;

    /* PASSWORD */
    case 43:
      if ((WITH->se_ok && WITH->pwmode != 8 && WITH->pwmode !=10 
           && (!(in_real_sf(WITH->call) && WITH->pwmode == 0)))
          || (WITH->se_ok && WITH->is_authentic)) {
	get_word(voll, w);   /* clear first word (PASSWORD) */
	Result = true;
	set_password(unr, WITH->supervisor, voll);
      } else
	wln_btext(unr, 65);
      break;

    /* DISCONNECT */
    case 44:
      disc_user(unr, eingabe);
      break;

    /* BOX-SID */
    case 45:
      if (WITH->tell == nohandle) {
	Result = true;
	sprintf(hs, "%d", unr);
	if (sf_allowed) {
	  get_word(voll, w2);	/* the SID	*/
	  get_word(voll, w);	/* the password	*/
	  if (*WITH->sfpwdb != '\0') {
	    get_btext(unr, 90, w1);
	    sprintf(hs + strlen(hs), " %s", w1);
	    if (!WITH->is_authentic && strcmp(w, WITH->sfpwdb))
	      disc_user(-1, hs);
	    else {
	      WITH->is_authentic = true;
	      WITH->force_priv = 0;
	      analyse_sid(true, unr, w2);
	    }
	  } else if (*WITH->sfpwtn != '\0') {
	    get_btext(unr, 90, w1);
	    sprintf(hs + strlen(hs), " %s", w1);
	    if (!WITH->is_authentic && strstr(w, WITH->sfpwtn) == NULL)
	      disc_user(-1, hs);
	    else {
	      WITH->is_authentic = (WITH->pwsetat + holddelay < clock_.ixtime);
	      WITH->force_priv = 0;
	      analyse_sid(true, unr, w2);
	    }
	  } else if (WITH->se_ok)
	    analyse_sid(true, unr, w2);
	  else {
	    get_btext(unr, 91, w1);
	    sprintf(hs + strlen(hs), " %s", w1);
	    disc_user(-1, hs);
	  }
	} else {
	  get_btext(unr, 92, w1);
	  sprintf(hs + strlen(hs), " %s", w1);
	  disc_user(-1, hs);
	}
      }
      break;

    /* LANGUAGES */
    case 46:
      show_languages(unr);
      break;

    /* BULLID */
    case 51:
      if (!strcmp(w, "HASH"))
	disp_bidhash(unr);
      else
	search_bull(unr, w, strchr(eingabe, '-') != NULL, strchr(eingabe, '+') != NULL);
      break;

    /* MSG */
    case 52:
      if (WITH->tell == nohandle && WITH->se_ok) {
	Result = true;
	get_word(voll, w1);	/* MSG	*/
	get_word(voll, w1);	/* CALL	*/
	send_msg(unr, w, voll);
      } else if (!WITH->se_ok)
	wln_btext(unr, 65);
      break;

    /* SELECTION */
    case 53:
      set_selection(unr, eingabe);
      break;

    /* BATCH */
    case 54:
      run_batch(unr, w);
      break;

    /* SETSF */
    case 55:
      if (callsign(w)) {
	get_word(eingabe, w1);
	get_word(eingabe, w2);
	upper(w1);
	upper(w2);
	if ((callsign(w2) || !strcmp(w2, "NIL")) &&
	    (w1[0] == 'S' || w1[0] == 'P' || w1[0] == 'B' || w1[0] == 'A'))
	  change_sfentries(1, SHORT_MAX, w, w1[0], w2);
	else
	  wln_btext(unr, 3);
      } else {
	if (strlen(w) == 1 && (w[0] == 'S' || w[0] == 'P' || w[0] == 'B' || w[0] == 'A'))
	  strcpy(w1, w);
	else {
	  strcpy(w1, "A");
	  sprintf(eingabe, "%s %s", w, strcpy(STR1, eingabe));
	}
	get_numbers(unr, false, eingabe, &s, &e, &threshold, option, search);
	strcpy(WITH->brett, "X");
	get_word(eingabe, w);
	upper(w);
	if (callsign(w) || !strcmp(w, "NIL"))
	  change_sfentries(s, e, "", w1[0], w);
	else
	  wln_btext(unr, 3);
      }
      break;

    /* FDELETE */
    case 57:
      if (WITH->supervisor)
	file_delete(unr, w, wlnuser);
      break;

    /* CTEXT */
    case 63:
      if (WITH->tell == nohandle) {
	if (WITH->supervisor)
	  enter_ctext(unr);
      }
      break;

    /* TALK */
    case 64:
      if (WITH->tell == nohandle && WITH->se_ok)
	set_talk_mode(unr, w);
      else if (!WITH->se_ok)
	wln_btext(unr, 65);
      break;

    /* //COMP */
    case 65:
      if (WITH->tell == nohandle)
	set_comp_mode(unr, w);
      break;

    /* SFTEST */
    case 69:
      gen_sftest(unr, eingabe);
      break;

    /* CLEARBUF */
    case 70:
      boxfreemostram(strcmp(w, "ALL") == 0);
      break;

    /* ACTIVITY */
    case 71:
      show_boxactivity(unr);
      break;

    /* BROADCAST */
    case 72:
      switch_bbsbcast(unr, w);
      break;

    case 73:
      set_debuglevel(unr, w);
      break;

    /* FREAD */
    case 74:
      if (read_file(false, unr, eingabe)) {
	if (read_filepart(unr))
	  WITH->action = 96;
      }
      break;

    /* FWRITE */
    case 75:
      write_file(unr, eingabe);
      break;

    /* FDIR */
    case 76:
      show_file_dir(unr, w);
      break;

    /* #OK# / <GP> */
    case 77:
    case 112:
      if (cnr == 112) {
	sprintf(w, "%d", boxboxssid());
	if (!strcmp(w, "0"))
	  *w = '\0';
	else
	  sprintf(w, "-%s", strcpy(STR1, w));
	w_btext(unr, 93);
	sprintf(w, " %s%s ", Console_call, strcpy(STR7, w));
	wuser(unr, w);
	wln_btext(unr, 94);
      }
      valid_command = false;
      WITH->errors = -5;
      break;

    /* NOTBOARD / WANTBOARD */
    case 78:
    case 111:
      if (WITH->se_ok)
	do_checkboards(unr, cnr == 111, w);
      else
	wln_btext(unr, 65);
      break;

    /* UPDATE */
    case 79:
      update_dp(unr, w);
      break;

    /* PWGEN */
    case 80:
      if (WITH->tell == nohandle && WITH->supervisor && *w != '\0')
	generate_dbpwfile(unr, w, eingabe);
      else
	wln_btext(unr, 2);
      break;

    /* FCOPY / FMOVE */
    case 81:
    case 82:
      if (WITH->tell == nohandle && *w != '\0') {
	get_word(eingabe, w1);
	if (*w1 != '\0') {
	  if (!WITH->console) {
	    if (insecure(w))
	      *w = '\0';
	    else
	      sprintf(w, "%s%s", boxdir, strcpy(STR7, w));
	    if (insecure(w1))
	      *w = '\0';
	    else
	      sprintf(w1, "%s%s", boxdir, strcpy(STR7, w1));
	    strcpy(w2, boxdir);
	  } else
	    *w2 = '\0';

	  if (*w != '\0') {
	    filecp(w, w1, w2, cnr == 82, unr, wlnuser);
	  } else
	    wln_btext(unr, 4);
	} else
	  wln_btext(unr, 2);
      } else
	wln_btext(unr, 4);
      break;

    /* SERVER */
    case 84:
      if (WITH->se_ok) {
	if (!config_server(unr, eingabe))
	  wln_btext(unr, 2);
      } else
	wln_btext(unr, 65);
      break;

    /* PROFILE */
    case 85:
      if (*eingabe == '\0')
	show_textfile(unr, profile_box);
      else if (WITH->se_ok)
	append_profile(unr, eingabe);
      else
	wln_btext(unr, 65);
      break;

    /* SFSTAT */
    case 86:
      create_sfstat(unr, eingabe);
      break;

    /* *UNERASE */
    case 87:
      if (WITH->se_ok) {
	unerase_users_mails(unr);
	strcpy(WITH->brett, WITH->call);
      } else
	wln_btext(unr, 65);
      break;

    /* PROTO */
    case 88:
      get_word(eingabe, w1);
      upper(w1);
      proto_cmd(unr, w, w1);
      break;

    /* TRACE */
    case 89:
      WITH->fulltrace = false;
      get_word(eingabe, w1);
      upper(w1);
      trace_cmd(unr, w, w1);
      break;

    /* MKF */
    case 90:
      if (WITH->tell == nohandle)
	mkf_cmd(unr, eingabe);
      break;

    /* PRIV */
    case 91:
      Result = true;
      if (*WITH->sfpwdb != '\0') {
	get_word(voll, w);	/* PRIV		*/
	get_word(voll, w);	/* das PW	*/
	strcpy(WITH->input2, WITH->sfpwdb);
	check_sysanswer(unr, w);
	name_ok(unr);
      } else
	append_syslog(unr);
      break;

    /* FULLTRACE */
    case 92:
      WITH->fulltrace = true;
      get_word(eingabe, w1);
      upper(w1);
      trace_cmd(unr, w, w1);
      break;

    /* CONVERS */
    case 93:
      if (WITH->tell == nohandle && WITH->se_ok)
	box_convers(unr, w);
      else if (!WITH->se_ok)
	wln_btext(unr, 65);
      break;

    /* CD */
    case 94:
      cut(w, 8);
      if (valid_boardname(w)) {
        check_transfers(w);
	strcpy(WITH->brett, w);
      }
      else
	wln_btext(unr, 2);
      break;

    /* PROMPT */
    case 96:
      if (WITH->se_ok) {
	Result = true;
	get_word(voll, w);	/* PROMPT */
	change_prompt(unr, voll);
      } else
	wln_btext(unr, 65);
      break;

    /* STARTUP */
    case 97:
      if (WITH->se_ok) {
	Result = true;
	get_word(voll, w);	/* STARTUP */
	for (x = 0; voll[x] != '\0'; x++) {
	  if (voll[x] == ';')
	    voll[x] = ',';
	}
	change_startup(unr, voll);
      } else
	wln_btext(unr, 65);
      break;

    /* (*)FEXECUTE */
    case 83:
      if (WITH->tell == nohandle && *w != '\0')
	call_runprg(true, unr, w, eingabe, false, "");
      else
	wln_btext(unr, 2);
      break;

    /* (*)RUN */
    case 98:
    case 99:
      expand_command(unr, w2);
      lower(w2);
      call_runprg(false, unr, w2, eingabe, cnr == 99, "");
      break;

    /* CONFIG */
    case 100:
      if (WITH->se_ok) {
	if (insecure(w)) {
	  *w = '\0';
	  do_config(unr, w, w);
	} else {
	  if (*w != '\0') {
	    if (!find_env(w)) {
	      *w = '\0';
	      *eingabe = '\0';
	    }
	    do_config(unr, w, eingabe);
	  } else
	    do_config(unr, w, w);
	}
      } else
	wln_btext(unr, 65);
      break;

    /* STRIPCR */
    case 101:
      if (WITH->se_ok && WITH->supervisor) {
	if (insecure(w)) {
	  *w = '\0';
	  do_stripcr(unr, w);
	} else {
	  if (*w != '\0') {
	    if (!find_env(w))
	      *w = '\0';
	    do_stripcr(unr, w);
	  } else
	    do_stripcr(unr, w);
	}
      } else
	wln_btext(unr, 65);
      break;

    /* MAXREAD */
    case 102:
      if (WITH->supervisor)
	set_maxread(unr, w, eingabe);
      else
	wln_btext(unr, 65);
      break;

    /* TTL */
    case 103:
      if (WITH->se_ok)
	change_ttl(unr, eingabe);
      else
	wln_btext(unr, 65);
      break;

    /* IFUSAGE */
    case 105:
      list_ifaceusage(unr);
      break;

    /* QRG */
    case 106:
      list_qrgs(unr);
      break;

    /* CONNECT */
    case 107:
      if (WITH->tell == nohandle)
	connect_from_box(unr, eingabe);
      break;

    /* TNTCOMM */
    case 109:
      if (WITH->tell == nohandle)
	tnt_command(unr, eingabe);
      break;

    /* NEXT */
    case 108:
      read_next(unr);
      break;

    /* BEACON */
    case 110:
      start_mailbeacon_manually(unr);
      break;

    /* READLOCK */
    case 113:
      if (WITH->se_ok && WITH->tell == nohandle)
	change_readlock(unr, eingabe);
      else
	wln_btext(unr, 65);
      break;

    /* SETREPLY */
    case 114:
      if (WITH->se_ok && WITH->tell == nohandle)
	set_reply_flag_directly(unr, eingabe);
      else
	wln_btext(unr, 65);
      break;

    /* MAILBEACON */
    case 115:
      if (WITH->se_ok && WITH->tell == nohandle)
	change_mailbeacon(unr, eingabe);
      else
	wln_btext(unr, 65);
      break;

    /* FILESERVER */
    case 116:
      if (WITH->tell == nohandle) {
	wln_btext(unr, 95);
	WITH->smode = true;
	WITH->changed_dir = true;
      }
      break;

    /* MD2 / MD5 */
    case 117:
    case 123:
      if (WITH->tell == nohandle)
	answer_sysrequest(unr, cnr == 117, cnr == 123);
      break;

    /* MD2SF / MD5SF */
    case 118:
    case 124:
      if (WITH->se_ok && WITH->tell == nohandle)
 	change_md2sf(unr, eingabe, cnr == 124);
      else
	wln_btext(unr, 65);
      break;

    /* FBBMODE */
    case 119:
      if (WITH->se_ok && WITH->tell == nohandle)
	change_fbbmode(unr, eingabe);
      else
	wln_btext(unr, 65);
      break;

    /* UNPROTO */
    case 120:
      if (WITH->supervisor)
	change_unprotomode(unr, eingabe);
      else
	wln_btext(unr, 65);
      break;

    /* SHELL / TSHELL */
    case 121:
    case 122:
      open_shell(unr, cnr == 122);
      break;
      
    /* PREFIX */
    case 60:
      get_word(eingabe, w1);
      get_word(eingabe, w2);
      if (!strcmp(w1, "-u") && WITH->supervisor)
      	update_digimap(unr, w2);
      else
        show_prefix_information(unr, w1, w2);
      break;
    
    /* QTHLOC */
    case 125:
      get_word(eingabe, w1);
      get_word(eingabe, w2);
      calc_qthdist(unr, w1, w2);
      break;
   
    /* CRAWL */
    case 126:
      start_crawl(unr, eingabe);
      break;
    
    /* VOLLTEXT/FTR */
    case 127:
      query_crawl(unr, eingabe);
      break;

    /* ROUTER */
    case 128:
      if (!strcmp(w, "TABLE")) {
	get_routing_table(unr);
	break;
      }
      if (callsign(w)) {
      	show_routing_stat(unr, w, eingabe, wlnuser);
      	break;
      }
      sp = NULL;
      if (!*w || !strcmp(w, "ROUTES") || (sp = strchr(w, '*')) != NULL) {
      	if (sp == NULL) sp = w;
	*sp = '\0';
	get_routing_targets(unr, w);
	break;
      }
      wln_btext(unr, 45);
      break;

    /* PING */
    case 129:
      if (WITH->se_ok && !WITH->in_begruessung) {
	if (complete_hierarchical_adress(w)) {
	  nstrcpy(w2, WITH->brett, LEN_BOARD);
	  sprintf(STR1, "%s%s%cPINGXXXXXX", tempdir, WITH->call, extsep);
      	  mymktemp(STR1);
	  append(STR1, "/ACK", true);
	  strcpy(user[unr]->input2, STR1);
	  sprintf(hs, "PING @ %s < %s ping request", w, WITH->call);
	  user[unr]->tell	= -99;	/* ???	*/
	  send_check(unr, hs, false, 'P');
	  user[unr]->tell      	= nohandle;
	  sfdelfile(STR1);		/* delete the dummy file after import	*/
	  nstrcpy(WITH->brett, w2, LEN_BOARD);
	} else
	  wln_btext(unr, 13);
      } else
	wln_btext(unr, 65);
      break;
      
    /* EDITOR */
    case 26:
      if (!may_invoke_xeditor(unr)) {
      	wln_btext(unr, 7);
      	break;
      }
      if (insecure(w) || !*w) {
      	wln_btext(unr, 4);
	break;
      }
      x = 0;
      /* special: if filename = <BOARD> <NUMBER>, export message and open in xeditor */
      if (count_words(eingabe) == 1 && zahl(eingabe)) {
      	snprintf(w2, 255, "%s %s", w, eingabe);
	strcpy(hs, w);
	*w  = '\0';
      	if (!read_for_view(unr, w2, w, &lheader)) strcpy(w, hs);
	else x = 1;
      /* special: if filename = <NUMBER>, export message in current board and open in xeditor */
      } else if (count_words(eingabe) == 0 && zahl(w)) {
      	snprintf(w2, 255, "%s %s", WITH->brett, w);
	strcpy(hs, w);
	*w  = '\0';
      	if (!read_for_view(unr, w2, w, &lheader)) strcpy(w, hs);
	else x = 1;
      }
      if (*w != '/') find_env(w);
      if (*w != '/') strcpy(w2, boxdir);
      else *w2 = '\0';
      strcat(w2, w);
      snprintf(w1, 255, "%s %s", xeditor, w2);
      if (x > 0) add_zombie(my_exec(w1), w2, 0); /* this deletes the tmp file after exit of editor */
      else add_zombie(my_exec(w1), "", 0); /* this keeps the file after exit of editor */
      wuser(unr, "OK, started X-editor with file ");
      wlnuser(unr, w2);
      break;
      

    default:
      wlnuser(unr, "unknown command number...");
      break;
    }
  } else {
    switch (WITH->action) {

    /*   71: Vergeben fuer sorted check */
    case 72:
      send_check(unr, eingabe, true, '\0');
      break;

    case 73:
      box_txt2(false, unr, eingabe);
      break;

    case 74:
    case 75:  /* 74 = erste Zeile */
      send_text3(unr, WITH->action == 74, eingabe, return_);
      if (WITH->action == 74)
	WITH->action = 75;
      plus = true;
      break;

    /*           76  Vergeben fuer EXPORT */
    case 77:
      check_sysanswer(unr, eingabe);
      name_ok(unr);
      break;

    case 78:
      change_name(unr, WITH->call, eingabe);
      break;

    case 79:
      write_ctext(unr, eingabe);
      break;

    case 80:
      talk_line(unr, eingabe);
      break;

    case 81:
    case 82:
    case 83:
      enter_password(unr, eingabe, WITH->action >= 83);
      break;

    case 14: /* QUIT / ABORT : Daeumchen drehen, bis der Spooler alle ist */
    case 15:
    case 86: /* reserviert fuer Promptunterdrueckung bei PW-Eingabe */
      break;

    case 87:
    case 88:
      write_file2(unr, eingabe);
      valid_command2 = true;
      break;

    case 89:
      update_dp2(unr);
      break;

    case 90:
      send_conv(unr, eingabe);
      break;

    case 91:   /* zusaetzliche Lifetimeabfrage */
      enter_lifetime(unr, eingabe);
      break;

    /*   92: reserviert fuer ascii-upload fileserver */
    case 96:
      if (WITH->bin == NULL)
	WITH->action = 0;
      else if (*eingabe == '\0' || strstr(eingabe, "#ABORT#") != NULL) {
	abort_fileread(unr);
	abort_useroutput(unr);
	WITH->action = 0;
      }
      break;

    default:
      sprintf(hs, "invalid value of <action>: %d", WITH->action);
      debug(0, unr, 56, hs);
      WITH->action = 15;   /* Abwurf */
      break;
    }
  }


  if (!boxrange(unr)) {
    *eingabe = '\0';
    return Result;
  }

  if (!valid_command)
    WITH->errors++;

  if (WITH->errors > maxerrors) {
    wlnuser0(unr);
    WITH->action = 15;   /* Warten auf Spooler, dann Abbruch    */
  } else if ((WITH->action < 72 || WITH->action > 91) && !plus && !valid_command2)
    WITH->errors = 0;

  if (WITH->lastprocnumber > 0)
    strcpy(eingabe, WITH->lastcmd);
  else
    *eingabe = '\0';

  return Result;
}


static void run_batch(short unr, char *cmd_)
{
  short		inf;
  boolean	ok;
  char		cmd[256], pfad[256], name[256], hs[256];

  strcpy(cmd, cmd_);
  ok		= false;
  if (*cmd == '\0' || exist(cmd) == false) {
    if (user[unr]->console) {
      sprintf(pfad, "%s%c%c%c", boxsysdir, allquant, extsep, allquant);
      name[0]	= '\0';
      if (boxgetfilename(pfad, name, "select batchfile")) {
	if (exist(name)) {
	  strcpy(cmd, name);
	  ok	= true;
	}
      }
    } else {
      sprintf(hs, "Batchfile <%s> not found", cmd);
      wlnuser(unr, hs);
    }
  } else
    ok		= true;

  if (!ok) return;

  inf		= sfopen(cmd, FO_READ);
  if (inf < minhandle) return;

  debug(1, unr, 57, cmd);
  while (file2str(inf, hs)) box_input(unr, false, hs, true);
  sfclose(&inf);
  debug(1, unr, 57, "closed. Batch will proceed in background.");
}


void run_sysbatch(char *name)
{
  short	unr;

  if (!exist(name)) return;

  debug(1, 0, 58, name);
  unr	= melde_user_an(Console_call, 0, 0, UM_SYSREQ, false);
  if (unr > 0) {
    run_batch(unr, name);
    box_input(unr, false, "ABORT", true);
  }
  debug(1, 0, 58, "closed. BATCH will proceed in background.");
}


void box_command_fract(short unr, char *command, boolean return_)
{
  short		x;
  boolean	ret2, nop, v;
  userstruct	*WITH;
  char		teil[256], voll[256], STR7[256];

  debug(5, unr, 59, command);
  boxsetbmouse();
  if (boxrange(unr)) {
    WITH	= user[unr];

    if (WITH->action == 0)
      utc_clock();

    if (WITH->f_bbs) {
      analyse_sf_command(unr, command, return_);
      *command		= '\0';
    } else if (WITH->smode) {
      analyse_smode_command(unr, command, return_);
      WITH->isfirstchar	= return_;
      *command		= '\0';
    } else {
      dp_watchdog(2, 4711);
      nop		= false;
      strcpy(voll, command);
      ret2		= return_;
      if (WITH->action == 0) {
	del_blanks(command);
	x		= strpos2(command, ";", 1);
	if (x > 0) {
	  ret2		= true;
	  sprintf(teil, "%.*s", x - 1, command);
	  strdelete(command, 1, x);
	} else {
	  strcpy(teil, command);
	  command[0]	= '\0';
	}
	del_blanks(teil);

      } else {
	strcpy(teil, command);
	command[0]	= '\0';
      }
      
      if (WITH->lastprocnumber == 0)
        create_outfile(unr);
        
      v	= analyse_boxcommand(unr, teil, voll, ret2);
      
      if (WITH->lastprocnumber == 0 && WITH->wait_pid <= 0)
        close_outfile(unr);

      if (boxrange(unr)) {
	if (v)
	  command[0]	= '\0';
	else if (*teil != '\0') {
	  nop		= true;
	  if (*command != '\0')
	    sprintf(command, "%s;%s", teil, strcpy(STR7, command));
	  else
	    strcpy(command, teil);
	}

	if (*command != '\0' && WITH->lastprocnumber == 0) {
	  sfclosedel(&WITH->prochandle);
	  WITH->lastprocnumber2 = 0;
	}

	WITH->isfirstchar = ret2;

	if (WITH->action == 0 && (WITH->errors >= 0 || WITH->errors == -5)
		&& !nop && WITH->wait_pid <= 0) {
	  show_prompt(unr);
	}
	else if (WITH->action == 86)
	  WITH->action	= 0;
	
      }
    }
  }
  boxsetamouse();
}


void _box_init(void)
{
  static int _was_initialized = 0;
  int i;
  
  if (_was_initialized++)
    return;

  for (i=0; i < LEN_BOARD+4; i++) dots[i] = '.';
}

