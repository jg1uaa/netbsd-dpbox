/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/

#define BOX_FILE_G
#include "boxlocal.h"
#include "boxglobl.h"
#include "pastrix.h"
#include "main.h"
#include "tools.h"
#include "sort.h"
#include "crc.h"
#include "box_mem.h"
#include "box_rout.h"
#include "box_send.h"
#include "box_inou.h"
#include "boxfserv.h"
#include "box_logs.h"
#include "box_sub.h"
#include "box_file.h"
#include "boxbcast.h"
#include "box.h"
#include "box_sf.h"
#include "box_sys.h"
#include "box_init.h"
#include "box_wp.h"


static void check_mybbsfile(void)
{
  mybbstyp	nheader;
  pathstr	oidxname, nidxname;
  long		ct, dsize;
  short		kin_index, kout_index;

  debug0(2, 0, 6);

  strcpy(oidxname, mybbs_box);
  strcpy(nidxname, oidxname);
  nidxname[strlen(nidxname) - 1] = 'N';
  dsize		= sfsize(oidxname);

  if (DFree(oidxname) < (dsize / 1024) + MINDISKFREE) {
    debug(0, 0, 6, "disk full");
    return;
  }
  
  kin_index	= sfopen(oidxname, FO_READ);
  kout_index	= sfcreate(nidxname, FC_FILE);
  if (kin_index >= minhandle && kout_index >= minhandle) {
    for (ct = 1; ct <= dsize / sizeof(mybbstyp); ct++) {
      if (sfread(kin_index, sizeof(mybbstyp), (char *)(&nheader)) != sizeof(mybbstyp)) {
	debug(0, 0, 6, "read error");
	break;
      }
      if (callsign(nheader.call)) {
	if (callsign(nheader.bbs)) {
	  /* the next line was an update check for dpbox > 5.02 */
	  if (nheader.mode != 'U' && nheader.mode != 'G') nheader.mode = 'U';
	  if ((nheader.mode == 'U' && clock_.ixtime - nheader.ix_time < SECPERDAY * 365 * 5) /* fuenf jahre mybbs stehen lassen */
	    || (clock_.ixtime - nheader.ix_time < SECPERDAY * 365))
	    sfwrite(kout_index, sizeof(mybbstyp), (char *)(&nheader));
	}
      }
    }

    sfclose(&kin_index);
    sfclose(&kout_index);
    sfdelfile(oidxname);
    filemove(nidxname, oidxname);
  }

  sfclose(&kin_index);
  sfclose(&kout_index);
  sfdelfile(nidxname);
}


static void ordne_ein(blogmem *lroot, blogmem **last, time_t date1, long logct1)
{
  blogmem	*temp;
  short		erg;
  blogmem	*entry_;

  entry_	= malloc(sizeof(blogmem));

  if (entry_ == NULL)
    return;

  entry_->date	= date1;
  entry_->logidxct = logct1;

  if (entry_->date > (*last)->date)
    temp	= *last;
  else
    temp	= lroot;

  erg		= 0;
  while (erg == 0) {
    if (temp->nach != NULL) {
      if (entry_->date > temp->date)
	temp	= temp->nach;
      else
	erg	= 1;
    } else {
      if (entry_->date > temp->date)
	erg	= 2;
      else
	erg	= 3;
    }
  }

  if (erg == 2) {
    temp->nach		= entry_;
    entry_->vor		= temp;
    entry_->nach	= NULL;
  } else {
    entry_->nach	= temp;
    entry_->vor		= temp->vor;
    temp->vor		= entry_;
    entry_->vor->nach	= entry_;
  }

  *last		= entry_;
}


static void conv_idx_to_blog(char *brett1, short idxct, indexstruct header, boxlogstruct *blog)
{
  char			hs[256];
  unsigned short	d1, d2;

  memset(blog, 0, sizeof(boxlogstruct));
  strcpy(blog->brett, brett1);
  strcpy(blog->obrett, header.dest);
  blog->msgnum		= header.msgnum;
  blog->date		= header.rxdate;
  blog->idxnr		= idxct;
  blog->msgflags	= header.msgflags;
  strcpy(blog->bid, header.id);
  blog->size		= truesize(header);
  blog->lifetime	= header.lifetime;
  blog->deleted		= header.deleted;
  blog->pmode		= header.pmode;
  d1			= 1;
  d2			= blog->lifetime;
  check_lt_acc(brett1, &d2, &d1);
  blog->level		= d1;
  blog->msgtype = header.msgtype;
  strcpy(blog->absender, header.absender);
  strcpy(hs, header.verbreitung);
  cut(hs, LEN_CALL);
  strcpy(blog->verbreitung, hs);
  strcpy(hs, header.betreff);
  cut(hs, LEN_CHECKSUBJECT);
  strcpy(blog->betreff, hs);
}


static void schreibe_templog(short templog, char *brett1, short idxct, indexstruct *header)
{
  boxlogstruct		blog;

  conv_idx_to_blog(brett1, idxct, *header, &blog);
  write_log(templog, -1, &blog);
}


static boolean schreibe_log(short templog, short log, blogmem *lroot)
{
  blogmem		*temp, *hp;
  boxlogstruct		blog;
  long			ct;
  boolean		disordered_msgnums;
  long			lastmsgnum;
  char	      	      	hs[256];

  ct			= 0;
  lastmsgnum		= 0;
  disordered_msgnums	= false;
  temp			= lroot;
  while (temp != NULL) {
    ct++;
    if (temp->logidxct > 0) {
      if (ct % 100 == 0)   /*Watchdog resetten*/
	dp_watchdog(2, 4711);
      read_log(templog, temp->logidxct, &blog);
      if (blog.msgnum <= lastmsgnum) {
        disordered_msgnums = true;
	append_profile(-1, "disordered msgnums:");
	snprintf(hs, 255, "last message number = %ld", lastmsgnum);
	append_profile(-1, hs);
	snprintf(hs, 255, "%s %d = %ld", blog.brett, blog.idxnr, blog.msgnum); 
	append_profile(-1, hs);
      }
      lastmsgnum	= blog.msgnum;
      write_log(log, -1, &blog);
    }
    hp			= temp;
    temp		= temp->nach;
    free(hp);
  }
  
  if (!disordered_msgnums) {
    if (lastmsgnum > 998000 - MSGNUMOFFSET && ct < 998000 - MSGNUMOFFSET)
      disordered_msgnums = true;
  }
  
  return disordered_msgnums;
}


static boolean cnb_sem = false;

static boolean create_new_boxlog1(short unr, boolean bullids)
{
  short			fmem;
  DTA			dirinfo;
  short			result, k, list;
  indexstruct		header;
  short			log, templog, msglog;
  char			brett[256], hs[256];
  char			fmemfile[256];
  char			tempname[256];
  long			totalsize;
  boolean		recalc_msgnums;
  blogmem		*lroot, *lastptr;
  long			err, logct, bseek;
  char			STR1[256];
  char			TEMP;


  debug0(2, unr, 13);

  cnb_sem		= true;
  recalc_msgnums 	= false;
  msglog		= nohandle;
  *brett		= '\0';
  boxsgdial1();
  sprintf(fmemfile, "%sdpchecklist%cXXXXXX", tempdir, extsep);
  mymktemp(fmemfile);
  *tempname		= '\0';
  totalsize		= 0;
  fmem			= sfcreate(fmemfile, FC_FILE);
  if (fmem < minhandle) {
    debug(0, unr, 13, "create error");
    return false;
  }

  sprintf(brett, "%c", allquant);
  idxfname(STR1, brett);
  result		= sffirst(STR1, 0, &dirinfo);
  while (result == 0) {
    dp_watchdog(2, 4711);   /*Watchdog resetten*/
    strcpy(hs, dirinfo.d_fname);
    del_ext(hs);
    str2file(&fmem, hs, true);
    totalsize		+= dirinfo.d_length / sizeof(indexstruct);
    result		= sfnext(&dirinfo);
  }
  sfclose(&fmem);

  sort_file(fmemfile, false);

  fmem			= sfopen(fmemfile, FO_READ);
  if (fmem < minhandle) {
    debug(0, unr, 13, "open error");
    return false;
  }

  if (bullids) {
    msglog		= sfopen(msgidlog, FO_RW);
    if (msglog < minhandle)
      msglog		= sfcreate(msgidlog, FC_FILE);
    if (msglog < minhandle) {
      debug(0, unr, 13, "open error 2");
      sfclose(&fmem);
      sfdelfile(fmemfile);
      return false;
    }
  }

  logct			= 0;
  lroot			= malloc(sizeof(blogmem));
  lroot->vor		= NULL;
  lroot->nach		= NULL;
  lroot->date		= 0;
  lroot->logidxct	= 0;
  lastptr		= lroot;
  log			= sfcreate(boxlog, FC_FILE);

  sprintf(tempname, "%sdptemplog%cXXXXXX", tempdir, extsep);
  mymktemp(tempname);
  templog		= sfcreate(tempname, FC_FILE);
  if (templog >= minhandle && log >= minhandle) {
    while (file2str(fmem, brett)) {
      dp_watchdog(2, 4711);   /*Watchdog resetten*/
      del_ext(brett);
      boxrgdial1(brett);   /* anzeigen */
      if (strlen(brett) <= 1 &&
	  (strlen(brett) != 1 || (TEMP = upcase_(brett[0]), TEMP == 'M' || TEMP == 'X')))
	continue;

      idxfname(STR1, brett);
      list		= sfopen(STR1, FO_READ);
      if (list < minhandle)
	continue;

      k			= 0;
      do {
	k++;
	err		= sfread(list, sizeof(indexstruct), (char *)(&header));
	if (err == sizeof(indexstruct)) {
	  logct++;
	  ordne_ein(lroot, &lastptr, header.rxdate, logct);
	  schreibe_templog(templog, brett, k, &header);
	  if (bullids) {
	    if (*header.id != '\0') {
	      if (bullidseek >= maxbullids) {
		bullidseek	= 0;
		bseek		= bullidseek * sizeof(header.id);
		sfseek(bseek, msglog, SFSEEKSET);
	      }
	      sfwrite(msglog, sizeof(header.id), header.id);
	      bullidseek++;
	    }
	  }
	}
      } while (err == sizeof(indexstruct));
      sfclose(&list);
    }
    strcpy(brett, "writing");
    boxrgdial1(brett);   /* anzeigen */
    recalc_msgnums	= schreibe_log(templog, log, lroot);
  } else
    debug(0, unr, 13, "file error - not created");

  sfclose(&log);
  sfclose(&templog);
  sfdelfile(tempname);
  sfclose(&fmem);
  sfdelfile(fmemfile);
  if (bullids) {
    sfclose(&msglog);
    update_bidseek();
  }
  boxegdial1();
  cnb_sem		= false;
  return recalc_msgnums;
}


void create_new_boxlog(short unr, boolean bullids)
{
  debug(2, unr, 13, "01");
  free_boxbcastdesc();
  wln_btext(unr, 32);
  boxspoolread();
  if (bullids) {
    sfdelfile(msgidlog);
    clear_bidhash();
    bullidseek	= 0;
  }
  sfdelfile(eralog);
  if (create_new_boxlog1(unr, bullids)) {
    wlnuser(unr, "recreating msgnums");
    create_all_msgnums(true);
  }
  if (bullids)
    check_bullidseek();
  fill_msgnumarr();
}


static void gberror1(boolean xgar, short unr, char *s, boolean *garbage_error)
{
  debug(0, unr, 14, s);
  debug(0, unr, 14, "garbage aborted");
  boxprotokoll(s);
  *garbage_error = true;
  boxsetgdial(xgar, "1", 0, 0, 0, 0, 0, s);
  boxwait(10);
}

static short gberasenumber;

static void gberrclose(long bct, char *dname, short unr, boolean xgar,
		       char **copybuf, boolean *garbage_error, short posi,
		       short *k, short ct, char *temp, boolean cserr)
{
  char		hs[256];

  *garbage_error= true;
  sfclose(k);
  *k		= nohandle;
  if (*temp != '\0')
    sfdelfile(temp);
  if (cserr) {
    sprintf(hs, "%s %d info checksum error (7/%d)", dname, ct, posi);
    gberror1(xgar, unr, hs, garbage_error);
    gberasenumber	= ct;
  } else if (bct <= 0) {
    sprintf(hs, "%s %d read error (7/%d)", dname, ct, posi);
    gberror1(xgar, unr, hs, garbage_error);
  } else {
    sprintf(hs, "%s %d write error (7/%d)", dname, ct, posi);
    gberror1(xgar, unr, hs, garbage_error);
  }
  if (*copybuf != NULL) {
    free(*copybuf);
    *copybuf = NULL;
  }
}


static void replace_x_nr(short old_x, short new_x)
{
  short		z, y;
  userstruct	*WITH;

  for (y = 1; y <= MAXUSER; y++) {
    if (user[y] != NULL) {
      WITH	= user[y];
      if (WITH->f_bbs) {
	for (z = 0; z < MAXFBBPROPS; z++) {
	  if (WITH->fbbprop[z].x_nr == old_x)
	    WITH->fbbprop[z].x_nr	= new_x;
	}
      }
    }
  }
}


void delete_tempboxfiles(void)
{
  pathstr	STR1;

  sprintf(STR1, "%sSFBOX%c%c", newmaildir, extsep, allquant);
  del_dir(STR1);
  sprintf(STR1, "%sAUTOBOX%c%c", newmaildir, extsep, allquant);
  del_dir(STR1);
  sprintf(STR1, "%sSENDING%c%c", newmaildir, extsep, allquant);
  del_dir(STR1);
  sprintf(STR1, "%sBOXCUT%c%c%c", newmaildir, allquant, extsep, allquant);
  del_dir(STR1);
  sprintf(STR1, "%s&%c%c%c", newmaildir, allquant, extsep, allquant);
  del_dir(STR1);
  sprintf(STR1, "%s%%%c%c%c", newmaildir, allquant, extsep, allquant);
  del_dir(STR1);
  sprintf(STR1, "%s$%c%c%c", newmaildir, allquant, extsep, allquant);
  del_dir(STR1);
}


static void imp_sysfile(char *fname, char *titel)
{
  pathstr	STR1;

  if (exist(fname)) {
    sprintf(STR1, "%s", fname);
    send_sysmsg("Y", Console_call, titel, STR1, y_lifetime, 'B', 1);
    sfdelfile(fname);
  }
}


static void extract_readcalls(boolean rej, char *readby, char *w)
{
  short		x;
  char		hs[256];

  x		= 1;
  *w		= '\0';
  while (x < strlen(readby) && strlen(w) < 59) {
    strsub(hs, readby, x, LEN_CALL+1);
    x		+= LEN_CALL+1;
    if ((rej && hs[0] == '%') || (!rej && (hs[0] == '!' || hs[0] == ',' || hs[0] == '.'))) {
      strdelete(hs, 1, 1);
      sprintf(w + strlen(w), "%s ", hs);
    }
  }
  del_lastblanks(w);
}


static short retmailinf(indexstruct *hptr, boolean unread, boolean unknown)
{
  short		k, x;
  pathstr	hs;
  char		bbs[256];
  char		w[256];

  sprintf(hs, "%simport%cXXXXXX", newmaildir, extsep);
  mymktemp(hs);
  k		= sfcreate(hs, FC_FILE);
  if (k < minhandle)
    return k;

  strcpy(bbs, hptr->sendbbs);
  complete_hierarchical_adress(bbs);
  str2file(&k, Console_call, true);
  str2file(&k, Console_call, true);
  str2file(&k, hptr->absender, true);
  str2file(&k, bbs, true);
  strcpy(hs, "30                   ");   /* Lifetime */
  str2file(&k, hs, true);
  *hs		= '\0';
  rspacing(hs, 80);   /* BID */
  str2file(&k, hs, true);
  if (unknown)
    sprintf(hs, "User %s unknown", hptr->dest);
  else if (unread)
    sprintf(hs, "Unread mail for %s", hptr->dest);
  else
    sprintf(hs, "Can't forward your mail for %s", hptr->dest);
  str2file(&k, hs, true);
  sprintf(hs, "From: %s @ %s", Console_call, ownhiername);
  str2file(&k, hs, true);
  sprintf(hs, "To  : %s @ %s", hptr->absender, bbs);
  str2file(&k, hs, true);
  *hs		= '\0';
  str2file(&k, hs, true);

  if (unread || unknown) {
    extract_readcalls(false, hptr->readby, w);
    if (*w != '\0') {
      if (unknown)
	strcpy(hs, "Error        : recipient unknown");
      else
	strcpy(hs, "Error        : mail not read by recipient");
      str2file(&k, hs, true);
      sprintf(hs, "Read by      : %s", w);
    } else {
      if (unknown)
	strcpy(hs, "Error        : recipient unknown");
      else
	strcpy(hs, "Error        : Unread mail, now deleted");
    }
  } else {
    if ((hptr->msgflags & MSG_REJECTED) != 0) {
      extract_readcalls(true, hptr->readby, w);
      sprintf(hs, "Error        : Your mail was rejected by %s", w);
    } else if ((hptr->msgflags & MSG_SFERR) != 0) {
      extract_readcalls(true, hptr->readby, w);
      sprintf(hs,
	"Error        : The next bbs (%s) in the fwd has a problem with your mail", w);
    } else
      strcpy(hs, "Error        : Can't forward due to bad link or bad route");
  }

  str2file(&k, hs, true);
  sprintf(hs, "Your mail to : %s@%s", hptr->dest, hptr->verbreitung);
  str2file(&k, hs, true);
  sprintf(hs, "Your subject : %s", hptr->betreff);
  str2file(&k, hs, true);
  sprintf(w, "%ld", (clock_.ixtime - hptr->rxdate) / SECPERDAY);
  if (unread || unknown)
    sprintf(hs, "Deleted after: %s days", w);
  else
    sprintf(hs, "Tried for    : %s days", w);
  str2file(&k, hs, true);
  *hs = '\0';
  str2file(&k, hs, true);
  sprintf(hs, "73s from %s %s", ownhiername, ownfheader);
  str2file(&k, hs, true);
  *hs = '\0';
  str2file(&k, hs, true);
  if (strstr(hptr->betreff, "CP ") == NULL)
  {
    sfclose(&k);
    return k;
  }
  if (unknown)
    strcpy(hs, "undeliverable mail follows:");
  else if (unread)
    strcpy(hs, "unread mail follows:");
  else
    strcpy(hs, "unrouteable mail follows:");
  str2file(&k, hs, true);

  *hs		= '\0';
  for (x = 1; x <= 79; x++)
    strcat(hs, "-");
  str2file(&k, hs, true);
  return k;
}


static void returnmail(boolean pack, boolean gzip, short *handle, char *buf, long size)
{
  long		size2;
  short		k, i;
  boolean	first;
  char		hs[256], s[256], s1[256], lastr[256];
  pathstr	tname, tname2;
  char		STR1[256], STR7[256];

  if (*handle < minhandle)
    return;

  sprintf(tname, "%sretmail%cXXXXXX", tempdir, extsep);
  mymktemp(tname);
  k		= sfcreate(tname, FC_FILE);
  if (k >= minhandle) {
    sfwrite(k, size, buf);
    sfclose(&k);

    if (pack || gzip) {
      sprintf(tname2, "%sretmail%cXXXXXX", tempdir, extsep);
      mymktemp(tname2);
      if (packer(gzip, false, false, tname, tname2, false) != 0) {
	sfdelfile(tname);
	sfdelfile(tname2);
	sfclose(handle);
	return;
      }
      sfdelfile(tname);
      strcpy(tname, tname2);
    }

    k		= sfopen(tname, FO_READ);
    if (k >= minhandle) {
      *s	= '\0';
      *lastr	= '\0';
      first	= true;
      while (file2str(k, hs) && strstr(hs, "R:") == hs) {
	strcpy(lastr, hs);
	i	= strpos2(hs, "@", 1);
	if (i <= 0)
	  continue;

	strdelete(hs, 1, i);
	if (hs[0] == ':')
	  strdelete(hs, 1, 1);
	get_word(hs, s1);
	unhpath(s1, hs);
	if (first) {
	  strcpy(s, Console_call);
	  first	= false;
	}
	sprintf(s + strlen(s), "!%s", hs);
	if (strlen(s) >= 67) {
	  sprintf(s, "Path: %s", strcpy(STR7, s));
	  str2file(handle, s, true);
	  *s	= '\0';
	}
      }
      if (*s != '\0') {
	sprintf(s, "Path: %s!", strcpy(STR1, s));
	str2file(handle, s, true);
      }
      if (*lastr != '\0') {
	sprintf(lastr, "Sent: %s", strcpy(STR1, lastr));
	cut(lastr, 79);
	str2file(handle, lastr, true);
      }
      *s	= '\0';
      str2file(handle, s, true);
      str2file(handle, hs, true);
      size2	= strlen(hs);
      while (size2 < retmailmaxbytes && file2str(k, hs) &&
	     strstr(hs, "#BIN#") != hs) {
	if (strstr(hs, "/ACK") != hs) {
	  str2file(handle, hs, true);
	  size2	+= strlen(hs);
	}
      }
      if (strstr(hs, "#BIN#") == hs) {
	strcpy(hs, "(*** binary file)");
	str2file(handle, hs, true);
	size2	= retmailmaxbytes;
      }
      if (size2 >= retmailmaxbytes) {
	*hs	= '\0';
	for (i = 1; i <= 79; i++)
	  strcat(hs, "-");
	str2file(handle, hs, true);
	strcpy(hs, "returned mail shortened");
	str2file(handle, hs, true);
      }
      sfclose(&k);
    }
    sfdelfile(tname);
  }
  sfclose(handle);
}



static void delete_invalid_board(char *fname)
{
  pathstr	hs;
  
  sprintf(hs, "%s%s", indexdir, fname);
  sfdelfile(hs);
  sprintf(hs, "%s%s", infodir, fname);
  new_ext(hs, EXT_INF);
  sfdelfile(hs);
}


void garbage_collection(boolean xgar, boolean fill_cbyte, boolean check_all,
			boolean immediate, short unr)
{
  indexstruct *hptr, header, *nhptr;
  boxlogstruct blog;
  short kin_index, kout_index, kin_info, kout_info;
  boolean output_opened, m_changed;
  long in_info_size, spos, tomorrowct;
  short ct, k;
  DTA dirinfo;
  short result, sflfsp, tomorrow;
  boolean garbage_error, ugzip;
  long err = 0;
  long obsolete, dsize;
  short fmem;
  char *copybuf;
  long cbsize = 0;
  long bct = 0;
  long boardct, hsize, rs;
  boolean do_it;
  long ddiff1, ddiff2;
  short kx, k1;
  boolean no_info, unknown_user;
  long diff, packgewinn;
  char swappart;
  boolean swap_info, swap_index;
  long seconds, tbytes, tmsgs, bps;
  short delct;
  long tbzs, tmsgszs;
  char *ipuffer;
  long isize;
  short new_ct;
  boolean copied;
  long lastend;
  boolean from_disk, iscall;
  char fbyte;
  short returnhandle, nhv;
  long l1;
  boolean invheader, reroute;
  unsigned short ics;
  char temp1[256], temp2[256];
  char hs[256], w[256], w1[256], umbbs[256];
  char dname[256], fmemfile[256];
  char oinfname[256];
  char oidxname[256];
  char ninfname[256];
  char nidxname[256];
  char STR1[256];
  short FORLIM;
  char STR13[256];
  char filebase[256];
  char tomorrowname[256];

  x_garbage_waiting = false;
  gberasenumber = 0;

  if (!xgar) {  /* alle rauswerfen */
    debug(1, unr, 15, "start");
    disc_user(unr, "ALL *** system reorganisation");
    if (boxhoststarted())
      boxsendallbufs();
    sprintf(STR1, "%sgarbage%cbat", boxsysdir, extsep);
    run_sysbatch(STR1);
    boxsaveallbuffers();
    boxfreemostram(true);
    free_boxbcastdesc();
    set_lastgarbagetime(clock_.ixtime);
    if (maxavail__() <= 100000L) {
      debug(0, unr, 15, "no mem - aborted");
      return;
    }
  } else
    debug(2, unr, 15, "(X) start");

  m_changed = false;
  bps = 0;
  tbytes = 0;
  tmsgs = 0;
  obsolete = 0;
  packgewinn = 0;
  seconds = sys_ixtime();
  *ninfname = '\0';
  *nidxname = '\0';
  ipuffer = NULL;
  copybuf = NULL;
  kin_info = nohandle;
  kout_info = nohandle;
  kout_index = nohandle;
  kin_index = nohandle;
  tomorrow = nohandle;
  fmem = nohandle;
  returnhandle = nohandle;
  nhv = nohandle;
  tomorrowct = 0;
  boardct = 0;

  if (!xgar) {
    wln_btext(unr, 33);
    wlnuser0(unr);
    boxspoolread();
    boxsgdial();

    boxsetgdial(xgar, "", 0, 0, 0, 0, 0, "");

    sprintf(filebase, "%sDPGARBAGE%cXXXXXX", tempdir, extsep);
    mymktemp(filebase);
    strcat(filebase, ".XXXXXX");
    strcpy(temp1, filebase);
    strcpy(temp2, filebase);
    strcpy(fmemfile, filebase);
    strcpy(tomorrowname, filebase);
    mymktemp(temp1);
    mymktemp(temp2);
    mymktemp(fmemfile);
    mymktemp(tomorrowname);

    boxisbusy(true);

    boxsetgdial(xgar, "", 0, 0, 0, 0, 0, "deleting boxlog");
    sfdelfile(boxlog);
    sfdelfile(eralog);
    append_profile(unr, "starting garbage collection");
    boxsetgdial(xgar, "", 0, 0, 0, 0, 0, "checking diskspace");

    boxsetgdial(xgar, "", 0, 0, 0, 0, 0, "creating filelist");

    delete_tempboxfiles();

    tomorrow = sfcreate(tomorrowname, FC_FILE);
    str2file(&tomorrow, "expired lifetime:", true);
    str2file(&tomorrow, "", true);

    fmem = sfcreate(fmemfile, FC_FILE);

    if (fmem <= minhandle) {
      sprintf(hs, "can't create sortfile %s", fmemfile);
      debug(0, unr, 15, hs);
      sfclosedel(&tomorrow);
      boxisbusy(false);
      return;
    }

    sprintf(STR1, "%s%c%c%s", indexdir, allquant, extsep, EXT_IDX);
    result = sffirst(STR1, 0, &dirinfo);

    while (result == 0) {
      strcpy(hs, dirinfo.d_fname);
      if (dirinfo.d_length > 0 && dirinfo.d_length % sizeof(indexstruct) == 0) {
	del_ext(hs);
	del_lastblanks(hs);
	if (*hs != '\0') {
	  sprintf(w, "%ld", dirinfo.d_length);
	  sprintf(hs + strlen(hs), " %s", w);
	  str2file(&fmem, hs, true);
	} else {
	  sprintf(STR1, "invalid filename: %s - deleted.", dirinfo.d_fname);
	  debug(0, unr, 14, STR1);
	  delete_invalid_board(dirinfo.d_fname);
	}
      } else {
	sprintf(STR1, "odd length: %s - deleted", hs);
	debug(0, unr, 14, STR1);
	delete_invalid_board(dirinfo.d_fname);
      }
      result = sfnext(&dirinfo);
    }
    sfclose(&fmem);
    sort_file(fmemfile, false);

    fmem = sfopen(fmemfile, FO_READ);
    if (fmem <= minhandle) {
      sprintf(hs, "can't open sortfile %s", fmemfile);
      debug(0, unr, 15, hs);
      boxisbusy(false);
      return;
    }

  } else {
    if (!exist(sflist))
      return;

    strcpy(dname, "X");
    fmem = nohandle;   /* doppelt, aber wer weiss */
  }


  while ((xgar && boardct == 0) || file2str(fmem, hs)) {
    dsize = 0;
    if (!xgar) {
      get_word(hs, dname);
      dsize = atol(hs);
    }

    boardct++;

    dp_watchdog(2, 4711);   /* Watchdog resetten         */
    swap_index = false;
    swap_info = false;
    unknown_user = false;
    iscall = callsign(dname);

    debug(4, unr, 16, dname);
    sprintf(oidxname, "%s%s%c%s", indexdir, dname, extsep, EXT_IDX);
    sprintf(oinfname, "%s%s%c%s", infodir, dname, extsep, EXT_INF);
    sprintf(nidxname, "%s%s%c%s", indexdir, dname, extsep, EXT_GIDX);
    sprintf(ninfname, "%s%s%c%s", infodir, dname, extsep, EXT_GINF);
    upper(dname);

    if (xgar)
      dsize = sfsize(oidxname);
      
    delct = 0;
    boxsetgdial(xgar, dname, 0, delct, tbytes, tmsgs, bps, "checking...");
    no_info = !strcmp("M", dname) || !strcmp("X", dname);
    garbage_error = false;
    kin_info = nohandle;
    kout_info = nohandle;
    kout_index = nohandle;
    kin_index = nohandle;
    copybuf = NULL;
    ipuffer = NULL;
    isize = 0;
    if (maxavail__() - dsize > 500000L)
      sfbread(false, oidxname, &ipuffer, &isize);

    from_disk = (isize <= 0);
    if (from_disk) {
      kin_index = sfopen(oidxname, FO_READ);
      if (kin_index <= minhandle) {
        sprintf(hs, "can't open index file %s", oidxname);
	debug(0, unr, 15, hs);
	goto _L2;
      }
      boxsetgdial(xgar, dname, 0, delct, tbytes, tmsgs, bps,
		  "checking (on disk)...");
    } else
      dsize = isize;

    output_opened = false;
    spos = 0;

    if (dsize / sizeof(indexstruct) > SHORT_MAX-1)
      dsize = sizeof(indexstruct) * (SHORT_MAX-1); /* cut index file */

    k = dsize / sizeof(indexstruct);
    ct = 1;
    tmsgszs = 0;
    tbzs = 0;
    umbbs[0] = '\0';

    if (lastxreroute == 0)
      lastxreroute = clock_.ixtime;
    reroute = (!strcmp(dname, "X") && clock_.ixtime - lastxreroute > 14400);
    if (reroute)
      lastxreroute = clock_.ixtime;

    do_it = (!strcmp(dname, "X") || !strcmp(dname, "M") ||
	     ((fill_cbyte || check_all || reroute) && k > 0));

    while (ct <= k && !do_it) {
      if (from_disk) {
	read_index(kin_index, ct, &header);
	hptr = &header;
      } else
	hptr = (indexstruct *)(&ipuffer[(ct - 1) * sizeof(indexstruct)]);

      if (check_hcs(*hptr)) {
	tmsgszs++;
	if (!no_info)
	  tbzs += hptr->size;

	do_it = hptr->deleted;
	if (do_it) {
	  if (!disk_full && erasewait > 0 && strcmp(dname, "X") && strcmp(dname, "M") &&
	      hptr->erasetime > 0 &&
	      clock_.ixtime - hptr->erasetime < erasewait && !immediate)
	    do_it = false;
	}

	if (!do_it) {
	  sflfsp = ct;

	  if (!hptr->deleted) {
	    ddiff2 = clock_.ixtime - hptr->rxdate;
	    if (hptr->lifetime > 0)
	      do_it = (ddiff2 >= hptr->lifetime * SECPERDAY);
	  }

	  if (!do_it) {
	    ddiff1 = clock_.ixtime - hptr->rxdate;
	    do_it = (hptr->size >= PACKMIN && (hptr->pmode & (PM_HUFF2 | PM_GZIP)) == 0 &&
		     !no_info && (ddiff1 >= packdelay * SECPERDAY || disk_full));
	  }

	  if (!do_it)
	    do_it = !callsign(hptr->absender);

	  if (!do_it && iscall && strcmp(dname, Console_call))
	  {  /* return mailer */
	    if (hptr->fwdct == 0 && clock_.ixtime - hptr->rxdate >= returntime) {
	      if ((hptr->msgflags & (MSG_CUT | MSG_OWNHOLD)) == 0) {
		unhpath(hptr->verbreitung, hs);
		if (strcmp(hs, Console_call))
		  do_it = true;
		else {
		  /* @console_call, nachschauen, ob wir den User kennen */
		  if (*umbbs == '\0')
		    user_mybbs(dname, umbbs);
		  unhpath(umbbs, w);
		  if (strcmp(w, Console_call)) {
		    do_it = true;
		    unknown_user = true;
		  }
		}
	      }
	    }
	  }

	  if (!do_it && iscall) {
	    if (hptr->fwdct == 0 &&
		(hptr->msgflags & (MSG_REJECTED | MSG_DOUBLE | MSG_SFNOFOUND)) != 0)
	      do_it = true;
	  }

	  if (!do_it && iscall) {
	    unhpath(hptr->verbreitung, hs);
	    do_it = !callsign(hs);
	  }

	}

      } else
	do_it = true;
      ct++;
    }


    if (do_it) {
      in_info_size = sfsize(oinfname);
      boxsetgdial(xgar, "1", 0, delct, tbytes, tmsgs, bps, "optimizing...");
      swappart = '0';
    }


    if (!do_it) {
      sfclose(&kin_index);
      if (ipuffer != NULL) {
	free(ipuffer);
	ipuffer = NULL;
      }
      isize = 0;
      tbytes += tbzs;
      tmsgs += tmsgszs;
    } else {
      k = dsize / sizeof(indexstruct);

      new_ct = 0;
      lastend = 0;
      nhptr = NULL;
      FORLIM = k;
      for (ct = 1; ct <= FORLIM; ct++) {
	fbyte = 0;
	copybuf = NULL;
	sfclose(&returnhandle);

	if (ct % 20 == 0 || ct == 1)   /* Watchdog resetten         */
	  dp_watchdog(2, 4711);

	boxsetgdial(xgar, "1", ct, delct, tbytes, tmsgs, bps, "1");

	if (from_disk) {
	  read_index(kin_index, ct, &header);
	  hptr = &header;
	} else
	  hptr = (indexstruct *)(&ipuffer[(ct - 1) * sizeof(indexstruct)]);

	if (!check_hcs(*hptr)) {
	  hptr->deleted = true;
	  invheader = true;
	} else
	  invheader = false;

	if (!hptr->deleted) {  /* denn dann fliegt's sowieso raus-- */
	  if (!strcmp(dname, "X")) {
	    if (!xgar) {
	      if (hptr->pmode == 16)
		hptr->pmode = 4;
	      hptr->pmode &= ~128;
	      if ((hptr->pmode & 2) != 0 || (hptr->pmode & 4) != 0) {
		if (labs(clock_.ixtime - hptr->rxdate) > bullsfwait)
		  hptr->deleted = true;
	      }
	      else {  /* alten Mist aus der FWD-Liste loeschen */
	        if (labs(clock_.ixtime - hptr->rxdate) > bullsfmaxage)
	          hptr->deleted = true;
	      }
	    }
	    if (reroute && hptr->pmode != 16 && !hptr->deleted
	      /* this is the forward list, and here, the destination board is stored in "betreff"... */
	      	&& (strcmp(hptr->betreff, "W") && strcmp(hptr->betreff, "WP"))) {
	      unhpath(hptr->verbreitung, hs);
	      if (callsign(hs)) {
		if ((smart_routing || do_wprot_routing) || clock_.ixtime - hptr->rxdate > returntime >> 1) {
		  find_neighbour(-3, hs, w);
		  if (*w != '\0' && strcmp(w, hptr->rxfrom))
		    strcpy(hptr->dest, w);
		}
	      }
	    }
	  } else if (!strcmp(dname, "M")) {
	    hptr->eraseby = 0;   /* read_today loeschen */
	    ddiff2 = hptr->rxdate;
	    if (hptr->lastread > ddiff2)
	      ddiff2 = hptr->lastread;
	    if (hptr->txdate > ddiff2)
	      ddiff2 = hptr->txdate;
	    if (ddiff2 > 0 && clock_.ixtime - ddiff2 > 31536000) {
              if (hptr->msgflags < 10 && hptr->level != 0) { /* DL3NEU, DL8HBS */
		sprintf(hs, "deleting local user entry of %s (>365 days of inactivity)", hptr->absender);
	        append_profile(unr, hs);
	        hptr->deleted = true;
	      } else if (hptr->msgflags >= 10) { /* Sysops duerfen nicht geloescht werden */
                sprintf(hs, "User entry of %s shows >365 days of inactivity! But it is a Sysop!", hptr->absender);            
                append_profile(unr, hs);
              }
	    }
	  } else {
	    ddiff2 = clock_.ixtime - hptr->rxdate;

	    if (!hptr->deleted && iscall) {  /* fehlerhafte usermails */
	      if ((hptr->msgflags & MSG_OWNHOLD) == 0) {
		if (hptr->lifetime > default_lifetime) {
		  unhpath(hptr->verbreitung, hs);
		  if (!callsign(hs))
		    hptr->lifetime = default_lifetime;
		}
	      }
	    }

	    if (hptr->lifetime > 0) {
	      if (ddiff2 >= hptr->lifetime * SECPERDAY) {
		hptr->eraseby = EB_LIFETIME;

		if (iscall) {
		  if (hptr->fwdct == 0) {
		    if ((hptr->msgflags & (MSG_CUT | MSG_OWNHOLD | MSG_DOUBLE)) == 0) {
		      unhpath(hptr->verbreitung, hs);
		      if (callsign(hs)) {
		      	if (strstr(hptr->betreff, "ACK:") != hptr->betreff && !is_bbs(hptr->absender)) {
			  if (*hptr->sendbbs != '\0' && strstr(hptr->readby, dname) == NULL) {
			    returnhandle = retmailinf(hptr, (hptr->msgflags & MSG_SFERR) == 0, unknown_user);
			    sprintf(hs, "informed %s@%s about deletion of unread mail for %s@%s",
			      	    hptr->absender, hptr->sendbbs, dname, hptr->verbreitung);
			    append_profile(unr, hs);
			  }
			  hptr->eraseby = EB_RETMAIL;
		      	}
		      }
		    }
		  }

		}


		hptr->deleted = true;
		hptr->erasetime = clock_.ixtime;

		if (!xgar && tomorrow >= minhandle) {
		  conv_idx_to_blog(dname, new_ct + 1, *hptr, &blog);
		  if (tomorrowct == 0) {
		    get_btext(unr, 17, hs);
		    str2file(&tomorrow, hs, true);
		    *hs = '\0';
		    str2file(&tomorrow, hs, true);
		  }
		  tomorrowct++;
		  disp_logptr(-1, tomorrowct, false, &blog, hs);
		  str2file(&tomorrow, hs, true);
		}

	      }

	    }

/*
      	    / * delete if sender is non-callsign * /
	    if (!hptr->deleted)
	      hptr->deleted = !callsign(hptr->absender);
*/
	    if (!hptr->deleted && iscall && strcmp(dname, Console_call))
	    {  /* return mailer */
	      if (hptr->fwdct == 0 && clock_.ixtime - hptr->rxdate >= returntime) {
		if ((hptr->msgflags & (MSG_CUT | MSG_OWNHOLD | MSG_DOUBLE)) == 0) {
		  if (strstr(hptr->betreff, "ACK:") != hptr->betreff &&
		      strstr(hptr->betreff, "CP ") != hptr->betreff) {
		    unhpath(hptr->verbreitung, hs);
		    if (strcmp(hs, Console_call) || unknown_user) {
		      if (callsign(hptr->absender)
		      	  && callsign(hs) /* else, even SB aa1aa@ww gets a return mail */
		      	  && !is_bbs(hptr->absender)
		      	  && *hptr->sendbbs != '\0'
			  && strstr(hptr->readby, dname) == NULL
			  && strcmp(hptr->absender, Console_call)) {
			returnhandle = retmailinf(hptr, false, unknown_user);
			if (unknown_user)
			  sprintf(hs, "unknown: %s@%s informed sender %s@%s",
				  dname, hptr->verbreitung, hptr->absender,
				  hptr->sendbbs);
			else
			  sprintf(hs, "informed %s@%s about fwd err for %s@%s",
				  hptr->absender, hptr->sendbbs, dname,
				  hptr->verbreitung);
			append_profile(unr, hs);
		      } else {
			if (unknown_user)
			  sprintf(hs, "unknown: %s@%s, sender not informed",
				  dname, hptr->verbreitung);
			else
			  sprintf(hs, "fwd failed for %s@%s, sender not informed",
				  dname, hptr->verbreitung);
			append_profile(unr, hs);
		      }
		      hptr->deleted = true;
		      hptr->erasetime = clock_.ixtime;
		      hptr->eraseby = EB_RETMAIL;
		    }
		  } else {
		    hptr->deleted = true;
		    hptr->erasetime = clock_.ixtime;
		    hptr->eraseby = EB_SFERR;
		  }
		}
	      }
	    }

	    if (!hptr->deleted && iscall && hptr->fwdct == 0)
	    {  /* rerouter nummer 2 */
	      if ((hptr->msgflags & (MSG_REJECTED | MSG_DOUBLE | MSG_SFNOFOUND)) != 0) {
		if ((hptr->msgflags & (MSG_CUT | MSG_OWNHOLD)) == 0) {
		  unhpath(hptr->verbreitung, hs);
		  if (strcmp(hs, Console_call)) {
		    if (callsign(hs) && *hptr->sendbbs != '\0' &&
			strstr(hptr->readby, dname) == NULL) {
		      *hs = '\0';
		      hptr->msgflags &= ~(MSG_REJECTED | MSG_DOUBLE | MSG_SFNOFOUND);
		      hptr->msgflags &= vermerke_sf(-1, false, dname, hptr->rxfrom, hs, *hptr, hs);
		    }
		  }
		}
	      }
	    }

	  }
	}


	if (hptr->deleted && !strcmp(dname, "M")) {
	  m_changed = true;
	  sprintf(STR1, "%s%s%cEXT", extuserdir, hptr->absender, extsep);
	  sfdelfile(STR1);
	}

	if (!hptr->deleted
	|| (!hptr->deleted && returnhandle >= minhandle)
	|| (erasewait > 0 && !invheader && strcmp(dname, "X")
	  && strcmp(dname, "M")
	  && hptr->erasetime > 0
	  && clock_.ixtime - hptr->erasetime < erasewait
	  && !immediate
	  && !disk_full)) {
	  if (!output_opened) {
	    if (in_info_size >= hptr->start + hptr->packsize || no_info) {
	      if (!no_info)
		kin_info = sfopen(oinfname, FO_READ);
	      kout_index = sfcreate(nidxname, FC_FILE);
	      if (!no_info)
		kout_info = sfcreate(ninfname, FC_FILE);
	      if (!((kin_info >= minhandle || no_info) &&
		    kout_index >= minhandle &&
		    (kout_info >= minhandle || no_info))) {
		gberror1(xgar, unr, "output error (4)", &garbage_error);
		goto _L2;
	      }
	      output_opened = true;
	    } else
	      obsolete += hptr->packsize;
	  }

	  if (output_opened) {
	    if (!no_info) {
	      spos = sfseek(0, kout_info, SFSEEKEND);
	      err = sfseek(hptr->start, kin_info, SFSEEKSET);
	    }

	    if (!(err == hptr->start || no_info)) {
	      gberror1(xgar, unr, "read error (14)", &garbage_error);
	      goto _L2;
	    }
	    err = 0;

	    if (!no_info)
	      tmsgs++;

	    copybuf = NULL;
	    if (!no_info) {
	      tbytes += hptr->size;

	      if (hptr->packsize > maxram())
		cbsize = maxram();
	      else
		cbsize = hptr->packsize;
	      /* nur bei grossen files testen, ob genug ram da ist */
	      if (cbsize > 100000L && cbsize > maxavail__())
		cbsize = maxavail__();
	      if (cbsize > 0)
		copybuf = malloc(cbsize);
	    }

	    if (!(copybuf != NULL || no_info)) {
	      gberror1(xgar, unr, TXTNOMEM, &garbage_error);
	      goto _L1;
	    }

	    if (!no_info) {
	      copied = false;

	      if (hptr->size >= PACKMIN && (hptr->pmode & (PM_HUFF2 | PM_GZIP)) == 0) {
		ddiff1 = clock_.ixtime - hptr->rxdate;
		if (ddiff1 >= packdelay * SECPERDAY || disk_full) {
		  if (cbsize == hptr->packsize) {
		    bct = sfread(kin_info, cbsize, copybuf);
		    ics = 0;
		    checksum16_buf(copybuf, bct, &ics);
		    fbyte = copybuf[0];
		    if (!(bct == cbsize && hptr->infochecksum == ics &&
			  (fill_cbyte || fbyte == hptr->firstbyte))) {
		      bct = 0;
		      gberrclose(bct, dname, unr, xgar, &copybuf,
				 &garbage_error, 2, &nhv, ct, "",
				 hptr->infochecksum != ics);
		      goto _L2;
		    }
		    if (returnhandle >= minhandle)
		      returnmail(false, false, &returnhandle, copybuf, bct);
		    l1 = hptr->packsize;
		    kx = hptr->pmode;
		    diff = l1;
		    pack_entry(&copybuf, &l1, &kx);
		    hptr->infochecksum = 0;
		    checksum16_buf(copybuf, l1, &hptr->infochecksum);
		    fbyte = copybuf[0];
		    hptr->firstbyte = fbyte;
		    packgewinn += diff - l1;
		    hptr->packsize = l1;
		    hptr->pmode = kx;
		    if (sfwrite(kout_info, hptr->packsize, copybuf) !=
			hptr->packsize) {
		      gberrclose(bct, dname, unr, xgar, &copybuf,
				 &garbage_error, 1, &nhv, ct, "", false);
		      goto _L2;
		    }
		    copied = true;
		    err = hptr->packsize;
		  } else {
		    k1 = sfcreate(temp1, FC_FILE);
		    if (k1 >= minhandle) {
		      hsize = 0;
		      err = 0;
		      ics = 0;
		      rs = hptr->packsize;
		      while (rs > 0) {
			if (rs > cbsize)
			  rs = cbsize;
			bct = sfread(kin_info, rs, copybuf);
			checksum16_buf(copybuf, bct, &ics);
			if (hsize == 0)
			  fbyte = copybuf[0];
			if (bct <= 0 || sfwrite(k1, bct, copybuf) < 0 ||
			    (fbyte != hptr->firstbyte && !fill_cbyte)) {
			  if (fbyte != hptr->firstbyte && !fill_cbyte)
			    bct = 0;
			  gberrclose(bct, dname, unr, xgar, &copybuf,
				     &garbage_error, 3, &k1, ct, temp1,
				     false);
			  goto _L2;
			}
			if (returnhandle >= minhandle)
			  returnmail(false, false, &returnhandle, copybuf, bct);
			hsize += bct;
			rs = hptr->packsize - hsize;
		      }
		      if (hptr->infochecksum != ics) {
			gberrclose(bct, dname, unr, xgar, &copybuf,
				   &garbage_error, 13, &k1, ct, temp1, true);
			goto _L2;
		      }
		      sfclose(&k1);

		      ugzip = use_gzip(hptr->packsize);
		      if (packer(ugzip, false, true, temp1, temp2, false) == 0) {
			sfdelfile(temp1);
			k1 = sfopen(temp2, FO_READ);
			if (k1 >= minhandle) {
			  hptr->packsize = sfsize(temp2);
			  if (ugzip) hptr->pmode |= PM_GZIP;
			  else hptr->pmode |= PM_HUFF2;

			  hsize = 0;
			  err = 0;
			  hptr->infochecksum = 0;
			  rs = hptr->packsize;
			  while (rs > 0) {
			    if (rs > cbsize)
			      rs = cbsize;
			    bct = sfread(k1, rs, copybuf);
			    checksum16_buf(copybuf, bct, &hptr->infochecksum);
			    if (hsize == 0) {
			      fbyte = copybuf[0];
			      hptr->firstbyte = fbyte;
			    }
			    if (bct <= 0 ||
				sfwrite(kout_info, bct, copybuf) < 0) {
			      bct = 0;
			      gberrclose(bct, dname, unr, xgar, &copybuf,
					 &garbage_error, 4, &k1, ct, temp2,
					 false);
			      goto _L2;
			    }
			    hsize += bct;
			    rs = hptr->packsize - hsize;
			  }
			  sfclose(&k1);
			  sfdelfile(temp2);
			  copied = true;
			  err = hptr->packsize;

			}
		      } else
			sfdelfile(temp1);
		      sfdelfile(temp2);
		    }
		  }


		}

	      }

	      if (!copied) {
		hsize = 0;
		err = 0;
		ics = 0;
		rs = hptr->packsize;
		while (rs > 0) {
		  if (rs > cbsize)
		    rs = cbsize;
		  bct = sfread(kin_info, rs, copybuf);
		  checksum16_buf(copybuf, bct, &ics);
		  if (hsize == 0)
		    fbyte = copybuf[0];
		  if (!(bct > 0 && (fill_cbyte || fbyte == hptr->firstbyte))) {
		    bct = 0;
		    gberrclose(bct, dname, unr, xgar, &copybuf,
			       &garbage_error, 6, &nhv, ct, "", false);
		    goto _L2;
		  }
		  if (returnhandle >= minhandle)
		    returnmail((hptr->pmode & PM_HUFF2) != 0, (hptr->pmode & PM_GZIP) != 0,
		    		&returnhandle,
			     copybuf, bct);
		  if (sfwrite(kout_info, bct, copybuf) != bct) {
		    gberrclose(bct, dname, unr, xgar, &copybuf,
			       &garbage_error, 5, &nhv, ct, "", false);
		    goto _L2;
		  }
		  err += bct;
		  hsize += bct;
		  rs = hptr->packsize - hsize;
		}
		if (hptr->infochecksum != 0) {
		  if (ics != hptr->infochecksum) {
		    gberrclose(bct, dname, unr, xgar, &copybuf,
			       &garbage_error, 15, &nhv, ct, "", true);
		    goto _L2;
		  }
		} else
		  hptr->infochecksum = ics;
	      } else
		err = hptr->packsize;

	      free(copybuf);
	      copybuf = NULL;

	    }

	    if (!(err == hptr->packsize || no_info)) {
	      gberror1(xgar, unr, "write error-disk full", &garbage_error);
	      sfclose(&kin_index);
	      sfclose(&kin_info);
	      sfclose(&kout_index);
	      sfclose(&kout_info);
	      sfdelfile(nidxname);
	      sfdelfile(ninfname);
	      goto _L1;
	    }


	    if (!no_info)
	      hptr->start = spos;

	    if (fill_cbyte)
	      hptr->firstbyte = fbyte;

	    create_hcs(hptr);
	    if (sfwrite(kout_index, sizeof(indexstruct), (char *)hptr) !=
		sizeof(indexstruct)) {
	      gberror1(xgar, unr, "write error (12)", &garbage_error);
	      goto _L2;
	    }

	    new_ct++;
	    if (!strcmp(dname, "X")) {
	      if (ct != new_ct)
		replace_x_nr(ct, new_ct);
	    }

	    if (copybuf != NULL)
	      free(copybuf);
	      copybuf = NULL;
	  }

	} else {
	  obsolete += hptr->packsize;
	  delct++;
	}

      }

      boxsetgdial(xgar, "1", k, delct, tbytes, tmsgs, bps, "1");

      sfclose(&kin_index);
      sfclose(&kin_info);
      sfclose(&kout_index);
      sfclose(&kout_info);

      if (!garbage_error) {
	sfdelfile(oidxname);
	if (!no_info)
	  sfdelfile(oinfname);

	if (output_opened) {
	  if (swap_index)
	    filemove(nidxname, oidxname);
	  else
	    sfrename(nidxname, oidxname);

	  if (!no_info) {
	    if (swap_info)
	      filemove(ninfname, oinfname);
	    else
	      sfrename(ninfname, oinfname);
	  }
	}


      } else {
	sfdelfile(nidxname);
	if (!no_info)
	  sfdelfile(ninfname);
      }


    }


    if (ipuffer != NULL) {
      free(ipuffer);
      ipuffer = NULL;
      isize = 0;
    }

    if (m_changed) {
      m_changed = false;
      load_mptr();
      init_ufcache();
    }

  }


_L2:
  sfclose(&kin_index);   /* wenn schon geschlossen, dann  */
  sfclose(&kin_info);    /* merkt das sfclose und tut     */
  sfclose(&kout_index);  /* nichts                        */
  sfclose(&kout_info);

_L1:
  if (copybuf != NULL) {
    free(copybuf);
    copybuf = NULL;
  }

  sfclose(&returnhandle);

  if (garbage_error) {
    sfdelfile(nidxname);
    sfdelfile(ninfname);
  }

  if (ipuffer != NULL) {
    free(ipuffer);
    ipuffer = NULL;
    isize = 0;
  }

  if (tomorrowct == 0)
    sfclosedel(&tomorrow);
  else
    sfclose(&tomorrow);
  sfclosedel(&fmem);

  if (xgar)
    return;

  boxegdial();
  
  if (gberasenumber > 0) {
    kin_index = sfopen(oidxname, FO_RW);
    if (kin_index >= minhandle) {
      read_index(kin_index, gberasenumber, &header);
      header.deleted = true;
      header.erasetime = clock_.ixtime - erasewait;
      write_index(kin_index, gberasenumber, &header);
      sfclose(&kin_index);
    }
  }

  sprintf(hs, "%ld", obsolete);
  sprintf(hs, "Deleted          : %s Bytes", strcpy(STR13, hs));
  wlnuser(unr, hs);
  append_profile(unr, hs);
  sprintf(hs, "%ld", packgewinn);
  sprintf(hs, "Saved by packing : %s Bytes", strcpy(STR1, hs));
  wlnuser(unr, hs);
  append_profile(unr, hs);
  sprintf(hs, "%ld", tbytes);
  sprintf(hs, "Total Bytes      : %s", strcpy(STR1, hs));
  wlnuser(unr, hs);
  append_profile(unr, hs);
  sprintf(hs, "%ld", tmsgs);
  sprintf(hs, "Total Msgs       : %s", strcpy(STR1, hs));
  wlnuser(unr, hs);
  append_profile(unr, hs);
  wlnuser0(unr);
  create_new_boxlog(unr, false);
  wlnuser0(unr);
  wlnuser(unr, "Checking HPATH.BOX");
  boxspoolread();
  dp_watchdog(2, 4711);   /* watchdog resetten         */
  check_hpath(true);
  wlnuser(unr, "Checking MYBBS.BOX");
  boxspoolread();
  dp_watchdog(2, 4711);   /* watchdog resetten         */
  check_mybbsfile();
  init_ufcache();
  wlnuser(unr, "Checking routing stats");
  boxspoolread();
  dp_watchdog(2, 4711);   /* watchdog resetten         */
  cleanup_routing_stat(31);
  wlnuser(unr, "Checking fileserver lifetimes");
  boxspoolread();
  dp_watchdog(2, 4711);   /* watchdog resetten         */
  smode_timer();
  dp_watchdog(2, 4711);   /* watchdog resetten         */
  kill_resume();
  utc_clock();
  seconds = clock_.ixtime - seconds;
  sprintf(hs, "%ld", seconds / 3600);
  sprintf(w, "%ld", seconds % 3600 / 60);
  sprintf(w1, "%ld", seconds % 60);
  sprintf(hs, "used time: %sh %smin %ssec", strcpy(STR13, hs), w, w1);
  wlnuser(unr, hs);

  boxcheckdiskspace();

  append_profile(unr, "ending garbage collection");
  
  check_disk_full();

  if (!disk_full) {
    imp_sysfile(profile_box, "profile");
    imp_sysfile(r_erase_box, "remote erases");
    sprintf(hs, "%sfile_err%ctxt", boxprotodir, extsep);
    imp_sysfile(hs, "file errors");
    sprintf(hs, "%slostfile%cbox", boxprotodir, extsep);
    imp_sysfile(hs, "lost files");
    
    if (import_logs) {
      sprintf(hs, "%ssyslog%cbox", boxprotodir, extsep);
      imp_sysfile(hs, "syslog");
      sprintf(hs, "%suserlog%cbox", boxprotodir, extsep);
      imp_sysfile(hs, "userlog");
      sprintf(hs, "%susrsflog%cbox", boxprotodir, extsep);
      imp_sysfile(hs, "usersflog");
      sprintf(hs, "%sreadlog%cbox", boxprotodir, extsep);
      imp_sysfile(hs, "readlog");
      sprintf(hs, "%ssflog%cbox", boxprotodir, extsep);
      imp_sysfile(hs, "sflog");
      sprintf(hs, "%sconvlog%cbox", boxprotodir, extsep);
      imp_sysfile(hs, "convlog");
      sprintf(hs, "%srejlog%cbox", boxprotodir, extsep);
      imp_sysfile(hs, "rejectlog");
    }

    sprintf(STR13, "%s%c%cPRO", boxprotodir, allquant, extsep);
    result = sffirst(STR13, 0, &dirinfo);
    while (result == 0) {
      strcpy(w, dirinfo.d_fname);
      sprintf(hs, "%s%s", boxprotodir, w);
      imp_sysfile(hs, w);
      result = sfnext(&dirinfo);
    }

    sprintf(STR13, "%s%c%cERR", boxprotodir, allquant, extsep);
    result = sffirst(STR13, 0, &dirinfo);
    while (result == 0) {
      strcpy(w, dirinfo.d_fname);
      sprintf(hs, "%s%s", boxprotodir, w);
      del_ext(w);
      if (in_real_sf(w)) {
	if (forwerr) {
	  sprintf(STR13, "%s", hs);
	  send_sysmsg(w, w, "forward errors", STR13, 3, 'P', 1);
	}
	strcat(w, " forward errors");
	imp_sysfile(hs, w);   /* Kopie an Y */
      } else
	sfdelfile(hs);
      result = sfnext(&dirinfo);
    }

    imp_sysfile(tomorrowname, "expired mails");

    if (clock_.day == 1) {
      wlnuser(unr, "Generating monthly statistics");
      boxspoolread();
      generate_sfstat(-1);
    } else if (clock_.day > 24) {
      k = clock_.mon;
      if (k == 12)
	k = 1;
      else
	k++;
      sprintf(hs, "%d", k);
      sprintf(hs, "%sboxlog%s%cbox", boxprotodir, strcpy(STR1, hs), extsep);
      sfdelfile(hs);
    }

    sprintf(STR1, "%simport%c%c", newmaildir, extsep, allquant);
    sort_new_mail(-1, STR1, Console_call);   /* return-mails und andere... */

  }

  sfdelfile(tomorrowname);
  sfdelfile(temp1);
  sfdelfile(temp2);
  sfdelfile(fmemfile);

  boxreadallbuffers();
  boxisbusy(false);
  load_boxbcastparms(bcast_box);
  check_disk_full();
}
