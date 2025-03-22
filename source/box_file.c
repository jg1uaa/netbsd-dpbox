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
#include "main.h"
#include "box_file.h"
#include "box_init.h"
#include "tools.h"
#include "sort.h"
#include "shell.h"
#include "crc.h"
#include "boxbcast.h"
#include "box.h"
#include "box_tim.h"
#include "boxdbox.h"
#include "boxbcm.h"
#include "box_sf.h"
#include "box_sub.h"
#include "box_mem.h"
#include "box_sys.h"
#include "box_send.h"
#include "box_inou.h"
#include "box_serv.h"
#include "boxfserv.h"
#include "box_logs.h"
#include "box_wp.h"
#include "box_rout.h"
#include "box_scan.h"


void check_disk_full(void)
{
  disk_full = DFree(indexdir) < MINDISKFREE || DFree(infodir) < MINDISKFREE;
}


/************************************************************************/
/* Functions for cached index access					*/
/************************************************************************/

short open_index(char *brett, short mode, boolean cache, boolean vorwaerts)
{
  indexcachetype	*ic;
  short			handle;
  pathstr		hs;
  
  idxfname(hs, brett);
  handle		= sfopen(hs, mode);
  if (cache && handle >= minhandle) {
    ic			= (indexcachetype *) malloc(sizeof(indexcachetype));
    if (ic != NULL) {
      ic->start		= malloc(INDEXLOOKAHEAD * sizeof(indexstruct));
      if (ic->start != NULL) {
        ic->size	= 0;
        ic->next	= NULL;
        ic->prev	= NULL;
        ic->CurBufOffs	= LONG_MAX;
        ic->handle	= handle;
        ic->vorwaerts	= vorwaerts;
        if (indexcacheroot == NULL) indexcacheroot = ic;
        else {
          ic->next	= indexcacheroot->next;
          ic->prev	= indexcacheroot;
          indexcacheroot->next = ic;
        }
        indexcaches++;
      } else free(ic);
    }
  }
  return handle;
}

static indexcachetype *find_indexcache(short handle)
{
  indexcachetype	*ic;
  
  ic	= indexcacheroot;
  while (ic != NULL && ic->handle != handle) ic = ic->next;
  return ic;
}

void close_index(short *handle)
{
  indexcachetype	*ic;
  
  ic	= find_indexcache(*handle);
  if (ic != NULL) {
    if (ic->start != NULL) free(ic->start);
    if (ic->next  != NULL) ic->next->prev = ic->prev;
    if (ic->prev  != NULL) ic->prev->next = ic->next;
    else /* is root */ indexcacheroot = ic->next;
    free(ic);
    ic	= NULL;
    indexcaches--;
  }
  sfclose(handle);
}

void read_index(short handle, short nr, indexstruct *ibuf)
{
  indexcachetype	*ic;
  long			WantedOffs;
  long			seek, err;
  boolean		ok;
  pathstr		hs, STR1;

  ibuf->absender[0]	= '\0';

  ic			= find_indexcache(handle);
  if (ic != NULL && nr > 0) {
  
    WantedOffs		= (nr - 1) * sizeof(indexstruct);

    if (WantedOffs >= ic->CurBufOffs && WantedOffs + sizeof(indexstruct) <= ic->CurBufOffs + ic->size) {
      memcpy(ibuf, &ic->start[WantedOffs - ic->CurBufOffs], sizeof(indexstruct));
      return;
    }

    if (ic->CurBufOffs < LONG_MAX) /* flag fuer ersten Zugriff */
      ic->vorwaerts	= (WantedOffs > ic->CurBufOffs);

    if (ic->vorwaerts)
      ic->CurBufOffs	= WantedOffs;
    else
      ic->CurBufOffs	= WantedOffs + sizeof(indexstruct) - INDEXLOOKAHEAD*sizeof(indexstruct);

    if (ic->CurBufOffs < 0)
      ic->CurBufOffs	= 0;

    ic->size		= 0;
      
    if (sfseek(ic->CurBufOffs, handle, SFSEEKSET) == ic->CurBufOffs)
      ic->size		= sfread(handle, INDEXLOOKAHEAD*sizeof(indexstruct), ic->start);

    if (WantedOffs >= ic->CurBufOffs && WantedOffs + sizeof(indexstruct) <= ic->CurBufOffs + ic->size) {
      memcpy(ibuf, &ic->start[WantedOffs - ic->CurBufOffs], sizeof(indexstruct));
      return;
    }
  }
  
  if (nr == -1)
    ok			= true;
  else {
    seek		= (nr - 1) * sizeof(indexstruct);
    err			= sfseek(seek, handle, SFSEEKSET);
    ok			= (seek == err);
  }

  if (!ok)
    return;

  if (sfread(handle, sizeof(indexstruct), (char *)ibuf) == sizeof(indexstruct))
    return;

  handle2name(handle, STR1);
  sprintf(hs, "read error: %s", STR1);
  debug(0, 0, 1, hs);
}


short write_index(short handle, short nr, indexstruct *ibuf)
{
  indexcachetype	*ic;
  long			WantedOffs;
  long			seek, err;
  boolean		ok;
  pathstr		hs, STR1;

  ibuf->hver		= HEADERVERSION;
  ibuf->reserved	= 0;
  create_hcs(ibuf);

  if (nr > 0) {
    ic			= find_indexcache(handle);
    if (ic != NULL) {
      WantedOffs	= (nr - 1) * sizeof(indexstruct);
      if (WantedOffs >= ic->CurBufOffs && WantedOffs + sizeof(indexstruct) <= ic->CurBufOffs + ic->size) {
        memcpy(&ic->start[WantedOffs - ic->CurBufOffs], ibuf, sizeof(indexstruct));
      }
    }
  }
  
  if (nr == -1) {
    err			= sfseek(0, handle, SFSEEKEND);   /* Eintrag anhaengen */
    ok			= (err / sizeof(indexstruct) < SHORT_MAX);
  } else {
    seek		= (nr - 1) * sizeof(indexstruct);
    err			= sfseek(seek, handle, SFSEEKSET);
    ok			= (seek == err);
  }

  if (ok) {
    if (sfwrite(handle, sizeof(indexstruct), (char *)ibuf) == sizeof(indexstruct))
      return (err / sizeof(indexstruct) + 1);

    handle2name(handle, STR1);
    sprintf(hs, "write error: %s", STR1);
    debug(0, 0, 2, hs);
    return 0;
  }

  handle2name(handle, STR1);
  sprintf(hs, "seek error: %s", STR1);
  debug(0, 0, 2, hs);
  return 0;
}

/**********************************************************************/


void read_log(short handle, long nr, boxlogstruct *ibuf)
{
  long		seek, err;
  boolean	ok;
  pathstr	hs, STR1;

  if (nr == -1)
    ok		= true;
  else {
    seek	= (nr - 1) * sizeof(boxlogstruct);
    err		= sfseek(seek, handle, SFSEEKSET);
    ok		= (seek == err);
  }

  if (!ok)
    return;

  if (sfread(handle, sizeof(boxlogstruct), (char *)ibuf) == sizeof(boxlogstruct))
    return;

  handle2name(handle, STR1);
  sprintf(hs, "read error: %s", STR1);
  debug(0, 0, 3, hs);
}


void write_log(short handle, long nr, boxlogstruct *ibuf)
{
  long		seek, err;
  boolean	ok;
  pathstr	hs, STR1;

  if (nr == -1) {
    sfseek(0, handle, SFSEEKEND);   /* Eintrag anhaengen */
    ok		= true;
  } else {
    seek	= (nr - 1) * sizeof(boxlogstruct);
    err		= sfseek(seek, handle, SFSEEKSET);
    ok		= (seek == err);
  }

  if (!ok)
    return;

  if (sfwrite(handle, sizeof(boxlogstruct), (char *)ibuf) == sizeof(boxlogstruct))
    return;

  handle2name(handle, STR1);
  sprintf(hs, "write error: %s", STR1);
  debug(0, 0, 4, hs);
}


void write_log_and_bid(char *brett1, short nummer, indexstruct header)
{
  short		k1;
  boxlogstruct	logheader;
  char		hs[256];

  debug(5, 0, 5, brett1);

  if (*header.id != '\0')
    write_msgid(-1, header.id);

  k1 = sfopen(boxlog, FO_RW);
  if (k1 < minhandle)
    k1 = sfcreate(boxlog, FC_FILE);

  if (k1 < minhandle) {
    debug(2, 0, 5, "file not created");
    return;
  }

  memset(&logheader, 0, sizeof(boxlogstruct));
  logheader.date	= clock_.ixtime;
  strcpy(logheader.brett, brett1);
  strcpy(logheader.obrett, header.dest);
  logheader.msgnum	= header.msgnum;
  logheader.idxnr	= nummer;
  strcpy(logheader.bid, header.id);
  logheader.size	= truesize(header);
  logheader.lifetime	= header.lifetime;
  logheader.deleted	= header.deleted;
  logheader.pmode	= header.pmode;
  logheader.level	= header.level;
  logheader.msgtype	= header.msgtype;
  logheader.msgflags	= header.msgflags;
  logheader.pmode	= header.pmode;
  strcpy(logheader.absender, header.absender);
  strcpy(hs, header.verbreitung);
  cut(hs, LEN_CALL);
  strcpy(logheader.verbreitung, hs);
  strcpy(hs, header.betreff);
  cut(hs, LEN_CHECKSUBJECT);
  strcpy(logheader.betreff, hs);
  write_log(k1, -1, &logheader);
  sfclose(&k1);

  if (unproto_list)
    send_new_unproto_header(logheader);
}


#define csustain        4

typedef struct counttyp {
  struct counttyp	*next;
  calltype		bbs;
  char			csum;
  long			anz;
} counttyp;

static unsigned short shall_sem;
static counttyp *countroot;

static void del_ctp(void)
{
  counttyp	*hp, *hp1;

  hp		= countroot;
  while (hp != NULL) {
    hp1		= hp;
    hp		= hp->next;
    free(hp1);
  }
  countroot	= NULL;
}


static void add_ctp(char *bbsc)
{
  counttyp	*hp;
  char		csum;

  csum		= calccs(bbsc);

  hp		= countroot;
  while (hp != NULL && (csum != hp->csum || strcmp(hp->bbs, bbsc)))
    hp		= hp->next;

  if (hp != NULL) {
    hp->anz++;
    return;
  }

  hp		= malloc(sizeof(counttyp));
  if (hp == NULL)
    return;

  hp->next	= countroot;
  strcpy(hp->bbs, bbsc);
  hp->csum	= csum;
  hp->anz     	= 1;
  countroot   	= hp;
}


static void write_ctp(short k)
{
  counttyp	*hp;
  char		hs[256];

  hp		= countroot;
  while (hp != NULL) {
    if (hp->anz >= csustain) {
      sprintf(hs, "%.4ld %s", hp->anz, hp->bbs);
      str2file(&k, hs, true);
    }
    hp		= hp->next;
  }
}


void show_all_user_at(short unr, char *call, boolean del, boolean del2,
		      boolean only_count, char *all_count)
{
  mybbstyp	*bbsptr;
  long		dsize, anz, ct, cct;
  short		k, list, last, outf;
  boolean	countall;
  char		*puffer;
  long		psize, rpos;
  indexstruct	header;
  mybbstyp	bbsrec;
  pathstr	fname;
  pathstr	sname;
  char		hs[256], ls[256];

  debug(2, unr, 7, call);

  countroot	= NULL;
  countall	= (*all_count != '\0');
  if (countall) {
    if (all_count[strlen(all_count) - 1] != '*')
      strcat(all_count, "*");
    del		= false;
    del2	= false;
    only_count	= true;
  }

  *ls		= '\0';
  cct		= 0;
  shall_sem++;
  strcpy(fname, mybbs_box);
  sprintf(sname, "%sSBBSF%c%d", tempdir, extsep, shall_sem);
  dsize		= sfsize(fname);
  if (dsize <= 0)
    return;

  outf		= sfcreate(sname, FC_FILE);
  if (outf < minhandle) {
    wlnuser(unr, "file create error");
    return;
  }

  anz		= dsize / sizeof(mybbstyp);

  if (dsize < maxram())
    puffer	= malloc(dsize);
  else
    puffer	= NULL;

  if (puffer != NULL) {
    psize	= dsize;

    if (del)
      k		= sfopen(fname, FO_RW);
    else
      k		= sfopen(fname, FO_READ);

    if (k >= minhandle) {
      sfread(k, psize, puffer);
      rpos	= 0;
      while (rpos < psize - 1) {
	bbsptr	= (mybbstyp *)(&puffer[rpos]);
	if (countall) {
	  if (wildcardcompare(SHORT_MAX, all_count, bbsptr->bbs, ls) && callsign(bbsptr->call)) {
	    add_ctp(bbsptr->bbs);
	    cct++;
	  }
	} else if ( (!del2 && !strcmp(bbsptr->bbs, call) && callsign(bbsptr->call))
	      	   ||
		    (del2 && !strcmp(bbsptr->call, call)) ) {
	  cct++;
	  str2file(&outf, bbsptr->call, true);
	  if (del) {
	    clear_uf_cache(bbsptr->call);
	    strcpy(bbsptr->call, "******");
	  }
	}
	rpos	+= sizeof(mybbstyp);
      }
      if (del && cct > 0) {
	sfseek(0, k, SFSEEKSET);
	sfwrite(k, psize, puffer);
      }
      sfclose(&k);
    }
    free(puffer);

  } else {

    /* Falls die Datei nicht in den Speicher paßt */
    if (del)
      k	= sfopen(fname, FO_RW);
    else
      k = sfopen(fname, FO_READ);

    if (k >= minhandle) {
      for (ct = 1; ct <= anz; ct++) {
	sfread(k, sizeof(mybbstyp), (char *)(&bbsrec));
	if (countall) {
	  if (wildcardcompare(SHORT_MAX, all_count, bbsrec.bbs, ls) && callsign(bbsrec.call)) {
	    add_ctp(bbsrec.bbs);
	    cct++;
	  }
	} else if ((!del2 && !strcmp(call, bbsrec.bbs) && callsign(bbsrec.call)) ||
		   (del2 && !strcmp(call, bbsrec.call))) {
	  cct++;
	  str2file(&outf, bbsrec.call, true);
	  if (del) {
	    clear_uf_cache(bbsrec.call);
	    strcpy(bbsrec.call, "******");
	    sfseek(-sizeof(mybbstyp), k, SFSEEKCUR);
	    sfwrite(k, sizeof(mybbstyp), (char *)(&bbsrec));
	  }
	}
      }
      sfclose(&k);
    }
  }


  if (!countall) {
    dsize	= sfsize(userinfos);
    last	= dsize / sizeof(indexstruct);
    if (last > 0) {
      if (del)
	list	= open_index("M", FO_RW, true, true);
      else
	list	= open_index("M", FO_READ, true, true);
      if (list >= minhandle) {
	k	= 0;
	while (k < last) {
	  k++;
	  read_index(list, k, &header);
	  if (header.deleted || !check_hcs(header))
	    continue;

	  unhpath(header.verbreitung, hs);
	  if ((del2 || strcmp(hs, call)) &&
	      (!del2 || strcmp(header.absender, call)))
	    continue;

	  cct++;
	  str2file(&outf, header.absender, true);
	  if (del) {
	    clear_uf_cache(header.absender);
	    header.deleted = true;
	    write_index(list, k, &header);
	  }
	}
	close_index(&list);
      }
    }
    anz	+= last;

  } else {
    write_ctp(outf);
    del_ctp();
  }

  sfclose(&outf);

  if (cct > 0 && !del2) {
    w_btext(unr, 42);
    if (countall) {
      get_btext(unr, 107, ls);
      sprintf(hs, " %ld %s %s", cct, ls, all_count);
    } else {
      get_btext(unr, 108, ls);
      sprintf(hs, " %ld %s ", anz, ls);
    }
    wlnuser(unr, hs);
    wlnuser0(unr);
    sort_file(sname, false);
    outf	= sfopen(sname, FO_READ);
    if (outf < minhandle) {
      wln_btext(unr, 106);
      return;
    }

    if (countall) {
      while (file2str(outf, hs)) {
	get_word(hs, ls);
	rspacing(hs, LEN_CALL+1);
	cut(hs, LEN_CALL+1);
	while (ls[0] == '0')
	  strdelete(ls, 1, 1);
	lspacing(ls, 4);
	strcat(hs, ls);
	wlnuser(unr, hs);
      }

    } else {
      cct	= 0;
      *ls	= '\0';
      while (file2str(outf, hs)) {
	if (!strcmp(hs, ls))
	  continue;
	strcpy(ls, hs);
	rspacing(hs, LEN_CALL+1);
	if (!only_count)
	  wuser(unr, hs);
	cct++;
	if (!only_count) {
	  if (cct % 10 == 0)
	    wlnuser0(unr);
	}
      }
    }

    sfclose(&outf);

    if (!countall) {
      if (!only_count)
	wlnuser0(unr);
      get_btext(unr, 108, ls);
      sprintf(hs, "@%s: %ld %s", call, cct, ls);
      if (del) {
	get_btext(unr, 89, ls);
	sprintf(hs + strlen(hs), " %s", ls);
      }
      wlnuser(unr, hs);
    }

    wlnuser0(unr);
  } else if (!del2)
    wln_btext(unr, 45);
  sfdelfile(sname);
}

static void redir_denied(boolean has_mail, char *call, char *newbbs, char *language)
{
  char	betreff[256];
  char	msg[256], msg2[256];
  
  get_language(184, langmemroot, language, betreff);
  expand_macro(-1, betreff);
  get_language(185, langmemroot, language, msg);
  expand_macro(-1, msg);
  strcat(msg, newbbs);
  if (has_mail) {
    get_language(186, langmemroot, language, msg2);
    expand_macro(-1, msg2);
    strcat(msg, msg2);
  }
  send_sysmsg(call, Console_call, betreff, msg, 90, 'P', 0);
  send_sysmsg(call, newbbs, betreff, msg, 90, 'P', 0);
  sprintf(msg, "Warning: MyBBS of %s maybe pirated to bbs %s", call, newbbs);
  append_profile(-1, msg);
}


#define rbsize 2500*sizeof(mybbstyp)

boolean update_mybbsfile(boolean by_usercommand, char *call_, time_t *updatetime,
			 char *mybbs_, char *mybbsmode)
{
  short		k;
  boolean	Result;
  boolean	found, rdout, redirect, has_mails;
  char		*puffer;
  char		gmode;
  time_t	gtime;
  long		psize, dsize, fsize, rpos, wpos, spos, bc;
  long		anz, ct;
  mybbstyp	bbsrec, *bbsptr;
  userstruct	rec;
  calltype	call;
  mbxtype	mybbs;
  char		STR1[256];

  debug(4, 0, 8, mybbs_);

  strcpy(call, call_);
  unhpath(mybbs_, mybbs);

  Result	= false;
  puffer	= NULL;
  gtime		= 0;
  gmode		= '\0';
  wpos		= 0;

  k		= nohandle;
  if (*updatetime == 0)
    *updatetime	= clock_.ixtime;

  if (callsign(mybbs))
    change_mybbs(-1, call, mybbs_, *updatetime, "", mybbsmode[0], true, false);

  if (!(callsign(call) && (callsign(mybbs) || *mybbs == '\0')))
    return Result;

  rdout		= (*mybbs == '\0');
  if (rdout) *mybbsmode = '\0';
  found		= false;
  dsize		= sfsize(mybbs_box);
  if (dsize > 0) {
    psize	= dsize;
    if (psize > rbsize)
      psize	= rbsize;
    puffer	= malloc(psize);
    
    if (puffer != NULL) {
    
      if (rdout)
        k	= sfopen(mybbs_box, FO_READ);
      else
        k	= sfopen(mybbs_box, FO_RW);
        
      if (k >= minhandle) {
        spos		= 0;
        fsize		= sfread(k, psize, puffer);
	while (!found && fsize % sizeof(mybbstyp) == 0 && fsize > 0) {
	  rpos		= 0;
	  while (!found && rpos < (fsize - sizeof(mybbstyp)) + 1) {
	    bbsptr	= (mybbstyp *)(&puffer[rpos]);
	    if (!strcmp(bbsptr->call, call)) {
	      gtime	= bbsptr->ix_time;
	      gmode	= bbsptr->mode;
	      if (gmode != 'U' && gmode != 'G')
	        gmode		= 'U';
	      if (rdout) {
		strcpy(mybbs_, bbsptr->bbs);
		*mybbsmode	= gmode;
		*updatetime	= gtime;
	      }
	      bbsrec	= *bbsptr;
	      found	= true;
	      wpos	= rpos + spos;
	    }
	    if (!found)
	      rpos	+= sizeof(mybbstyp);
	  }
	  if (!found) {
	    spos	+= fsize;
	    fsize	= sfread(k, psize, puffer);
	  }
	}
      }
      free(puffer);
      
    } else {
    
      anz	= dsize / sizeof(mybbstyp);
      if (rdout)
        k	= sfopen(mybbs_box, FO_READ);
      else
        k	= sfopen(mybbs_box, FO_RW);

      found	= false;
      if (k >= minhandle) {
	for (ct = 1; ct <= anz; ct++) {
	  bc	= sfread(k, sizeof(mybbstyp), (char *)(&bbsrec));
	  if (bc <= 0) {
	    debug(0, 0, 8, "read error (11)");
	    goto _STOP;
	  }
	  if (!strcmp(call, bbsrec.call)) {
	    found	= true;
	    gtime	= bbsrec.ix_time;
	    gmode	= bbsrec.mode;
	    if (gmode != 'U' && gmode != 'G')
	      gmode		= 'U';
	    if (rdout) {
	      strcpy(mybbs_, bbsrec.bbs);
	      *mybbsmode	= gmode;
	      *updatetime	= gtime;
	    }
	    break;
	  }
	  wpos		+= bc;	  
	}
      }
    }
  }

  if (!rdout) {

    /* nachschauen, ob wieder ein Witzbold ein sehr zukuenftiges */
    /* Datum angegeben hat...                                    */
    /* die +43200 sind WICHTIG, falls die eigene Uhr mal einige  */
    /* Minuten falsch geht (Fenster von 12 Stunden)              */

    if (*updatetime < clock_.ixtime + 12*60*60) {
    
    /* nachschauen ob
     * a) not found
     * b) found and mode != 'U' and mybbsmode == 'U'
     * c) found and mode != 'U' and mybbsmode != 'U' and updatetime > bbsrec.ix_time
     * d) found and mode == 'U' and mybbsmode == 'U' and updatetime > bbsrec.ix_time
     */
    
      if (!found
          || (found && gmode != 'U' && *mybbsmode == 'U')
          || (found && gmode != 'U' && *mybbsmode != 'U' && gtime < *updatetime)
          || (found && gmode == 'U' && *mybbsmode == 'U' && gtime < *updatetime)
         ) {
    
	Result	= true;
	
    /* check ob mybbs vorher auf eigenem Boxcall stand, wenn ja */
    /* und wenn readlock > 0 und passwort aktiv, dann userfile  */
    /* nicht hinterherschicken, stattdessen eine Warnung an den */
    /* User lokal und bei der neuen mybbs absetzen, dass das    */
    /* mybbs geaendert wurde.                                   */
     
	redirect	= true;
	if (found && !by_usercommand && gmode == 'U') {
	  unhpath(bbsrec.bbs, STR1);
	  if (!strcmp(STR1, Console_call)) {
	    unhpath(mybbs, STR1);
	    if (strcmp(STR1, Console_call)) {
	      if (!strcmp(call, Console_call))
	        redirect	= false;
	      else {
	        load_userfile(false, false, call, &rec);
	        if (rec.readlock > 0 || rec.pwmode > 1)
	          redirect	= false;
	      }
	    }
	  }
	}
	
	strcpy(bbsrec.call, call);
	strcpy(bbsrec.bbs, mybbs);
	bbsrec.ix_time	= *updatetime;
	bbsrec.mode	= *mybbsmode;
	if (k < minhandle)
	  k		= sfcreate(mybbs_box, FC_FILE);
	if (k >= minhandle) {
	  if (found)
	    sfseek(wpos, k, SFSEEKSET);
	  else
	    sfseek(0, k, SFSEEKEND);
	  sfwrite(k, sizeof(mybbstyp), (char *)(&bbsrec));
	  sfclose(&k);
	}
	k		= nohandle;
	clear_uf_cache(call);
	idxfname(STR1, call);
	if (exist(STR1)) {
	  has_mails	= resend_userfile(redirect, call, mybbs);
	  if (!redirect) {
	    redir_denied(has_mails, call, mybbs, rec.language); 
	  }
	} else {
	  if (!redirect) {
	    redir_denied(false, call, mybbs, rec.language);
	  }
	}
      }
    } else {
      sprintf(STR1, "corrupted M-forward-date : %s@%s stamp:%ld mytime:%ld",
	      call, mybbs, *updatetime, clock_.ixtime);
      append_profile(-1, STR1);
    }
  }


_STOP:
  sfclose(&k);
  return Result;
}

#undef rbsize


void dispose_mptr(void)
{
  mptrtype	*hp, *hp1;

  hp		= mptrroot;
  while (hp != NULL) {
    hp1		= hp;
    hp		= hp->next;
    free(hp1);
  }
  mptrroot	= NULL;
}


void load_mptr(void)
{
  short		x, k, l;
  mptrtype	*hp;
  indexstruct	ix;

  dispose_mptr();
  l		= sfsize(userinfos) / sizeof(indexstruct);
  if (l <= 0)
    return;

  k		= open_index("M", FO_READ, true, true);
  if (k < minhandle)
    return;

  for (x = 1; x <= l; x++) {
    read_index(k, x, &ix);
    if (check_hcs(ix) && !ix.deleted) {
      hp		= malloc(sizeof(mptrtype));
      if (hp != NULL) {
	hp->next	= NULL;
	hp->csum	= strcpycs(hp->call, ix.absender);
	hp->mpos	= x;
	if (mptrroot == NULL)
	  mptrroot	= hp;
	else {
	  hp->next	= mptrroot->next;
	  mptrroot->next= hp;
	}
      }
    }
  }
  close_index(&k);
}


static void del_mptr(char *call)
{
  mptrtype	*hp, *hp1;
  char		csum;

  hp		= mptrroot;
  hp1		= NULL;
  csum		= calccs(call);

  while (hp != NULL && (hp->csum != csum || strcmp(hp->call, call))) {
    hp1		= hp;
    hp		= hp->next;
  }

  if (hp == NULL)
    return;

  if (hp1 == NULL)
    mptrroot	= hp->next;
  else
    hp1->next	= hp->next;

  free(hp);
}


static void add_mptr(char *call, short mpos)
{
  mptrtype	*hp;
  short		i;

  i		= strlen(call);
  if (mpos <= 0 || i < 4 || i > LEN_CALL) return;

  if (mptrroot == NULL)
    load_mptr();
  del_mptr(call);

  hp		= malloc(sizeof(mptrtype));
  if (hp == NULL) return;

  hp->next	= NULL;
  hp->csum	= strcpycs(hp->call, call);
  hp->mpos	= mpos;
  if (mptrroot == NULL)
    mptrroot	= hp;
  else {
    hp->next	= mptrroot->next;
    mptrroot->next = hp;
  }
}


static short find_mptr(char *call)
{
  mptrtype	*hp;
  char		csum;

  csum		= calccs(call);
  hp		= mptrroot;
  while (hp != NULL && (hp->csum != csum || strcmp(hp->call, call)))
    hp		= hp->next;

  if (hp != NULL)
    return (hp->mpos);
  else
    return -1;
}


static boolean load_m(short mpos, char *call, indexstruct *ix)
{
  short		k;

  *ix->absender	= '\0';
  if (mpos <= 0)
    return false;

  k	= sfopen(userinfos, FO_READ);
  if (k < minhandle)
    return false;

  read_index(k, mpos, ix);
  sfclose(&k);

  if (check_hcs(*ix) && !strcmp(ix->absender, call) && ix->deleted == false)
    return true;

  return false;
}


static short load_m_call(char *call, indexstruct *ix)
{
  short		k;

  k		= find_mptr(call);
  if (k > 0) {
    if (!load_m(k, call, ix))
      k		= -1;
  }
  return k;
}


void init_ufcache(void)
{
  short		x;

  for (x = 0; x <= MAXMAXUFCACHE; x++) {
    if (ufcache[x] != NULL)
      free(ufcache[x]);
    ufcache[x]	= NULL;
  }
  ufcr		= 0;
}


void clear_uf_cache(char *call)
{
  short		x;
  char		csum;

  csum		= calccs(call);
  x		= 0;
  while (x <= maxufcache) {
    if (ufcache[x] != NULL && csum == ufcache[x]->callcsum && !strcmp(ufcache[x]->call, call)) {
      free(ufcache[x]);
      ufcache[x]= NULL;
    }
    x++;
  }
}


static void add_uf_cache(userstruct uf)
{
  clear_uf_cache(uf.call);
  ufcr++;
  if (ufcr > maxufcache)
    ufcr = 0;
  if (ufcache[ufcr] != NULL)
    free(ufcache[ufcr]);
  ufcache[ufcr] = malloc(sizeof(userstruct));
  if (ufcache[ufcr] != NULL) {
    *ufcache[ufcr]		= uf;
    ufcache[ufcr]->callcsum	= calccs(uf.call);
  }
}


static boolean load_uf_through_cache(char *call, userstruct *uf)
{
  short		x;
  boolean	hit;
  char		csum;

  csum		= calccs(call);
  hit		= false;
  x		= 0;

  while (!hit && x <= maxufcache) {
    if (ufcache[x] != NULL && ufcache[x]->callcsum == csum && !strcmp(ufcache[x]->call, call))
      hit	= true;
    else
      x++;
  }

  if (hit) {
    *uf		= *ufcache[x];
    ufchit++;
  } else
    ufcmiss++;

  return hit;
}


static void clear_userstruct(userstruct *rec)
{
  memset(rec, 0, sizeof(userstruct));
  rec->level		= 1;
  rec->plevel		= 1;
  rec->ttl		= gttl;
  rec->lock_here	= multi_master;
  rec->sendheader	= NULL;
}


void convert_ufil(boolean with_ext_strings, indexstruct header, userstruct *rec)
{
  char		*p1, *p2;
  short		x, y, k;
  char		w[256], hs[256], ds[256];
  short		FORLIM;
  char		STR1[256];

  clear_userstruct(rec);

  *ds		= '\0';

  strcpy(rec->call, header.absender);
  strcpy(rec->mybbs, header.verbreitung);
  p1		= (char *)(&header.size);
  p2		= rec->language;
  for (x = 0; x <= 3; x++)
    p2[x]	= p1[x];

  strcpy(hs, header.readby);
  strcpy(w, hs);
  x		= strpos2(hs, ";1", 1);
  if (x == 0) {
    cut(w, 80);
    strcpy(rec->name, w);
  } else {
    if (x <= 81)
      cut(w, x - 1);
    else
      cut(w, 80);
    strcpy(rec->name, w);
    strdelete(hs, 1, x + 1);
    x		= strpos2(hs, ";2", 1);
    if (x > 0) {
      sprintf(rec->promptmacro, "%.*s", x - 1, hs);
      strdelete(hs, 1, x + 1);
      strcpy(rec->logincommands, hs);

      x			= strpos2(hs, ";6", 1);
      if (x > 0) {
	rec->readlock		= hs[x + 1] - '0';
	if (rec->readlock > 2) {
	  rec->hidebeacon	= true;
	  rec->readlock		-= 3;
	}
	strdelete(hs, x, 3);
      }

      x			= strpos2(hs, ";7", 1);
      if (x > 0) {
	y		= hs[x + 1] - '0';
	rec->ttl	= ((y & 1) != 0);
	rec->lock_here	= ((y & 2) == 0);
	rec->login_priv	= (((y & 4) != 0) && (rec->readlock > 0));
	strdelete(hs, x, 3);
      }

      x			= strpos2(hs, ";8", 1);
      if (x > 0) {
        y		= strpos2(hs, ";", x + 1);
        if (y > x) {
          strcpy(ds, hs);
          cut(ds, y - 1);
          strdelete(ds, 1, x + 1);
          strdelete(hs, x, y - x); 
        } else {
	  strcpy(ds, hs);
	  cut(hs, x - 1);
	  strdelete(ds, 1, x + 1);
	}
      }

      x 		= strpos2(hs, ";9", 1);
      if (x > 0) {
	y = hs[x + 1] - '0';
/*	rec->sfmd2pw	= ((y & 1) != 0); */ /* nicht mehr ab v5.04.05 */
	rec->fbbmode	= ((y & 2) != 0);
	rec->unproto_ok	= ((y & 4) != 0);
	strdelete(hs, x, 3);
      }

      x			= strpos2(hs, ";5", 1);
      if (x > 0) {
	cut(rec->logincommands, x - 1);
	FORLIM		= strlen(rec->logincommands);
	for (y = 0; y < FORLIM; y++) {
	  if (rec->logincommands[y] == ',')
	    rec->logincommands[y] = ';';
	}

	if (with_ext_strings) {
	  sprintf(STR1, "%s%s%cEXT", extuserdir, rec->call, extsep);
	  k		= sfopen(STR1, FO_READ);
	  if (k >= minhandle) {
	    file2lstr(k, rec->checkboards, LEN_CHECKBOARDS);
	    if (*rec->checkboards != '\0')
	      rec->wantboards	= false;
	    else {
	      file2lstr(k, rec->checkboards, LEN_CHECKBOARDS);
	      if (*rec->checkboards != '\0')
	        rec->wantboards	= true;
	    }
	    file2str(k, ds);
	    sfclose(&k);
	  }
	}
      } else {
	x			= strpos2(hs, ";3", 1);
	if (x == 0) {
	  x			= strpos2(hs, ";4", 1);
	  rec->wantboards	= true;
	} else
	  rec->wantboards	= false;
	if (x > 0)
	  cut(rec->logincommands, x - 1);
	FORLIM			= strlen(rec->logincommands);
	for (y = 0; y < FORLIM; y++) {
	  if (rec->logincommands[y] == ',')
	    rec->logincommands[y]= ';';
	}
	if (x > 0) {
	  strdelete(hs, 1, x + 1);
	  if (*hs != '\0' && hs[0] == ',') {
	    strcpy(rec->checkboards, hs);
	  }
	}
      }

    }
  }

  strcpy(rec->password, header.betreff);
  rec->lastdate		= header.rxdate;
  rec->lastatime	= header.lastread;
  rec->level		= header.level;
  rec->plevel		= header.level;
  if (header.pmode < 255)
    rec->sf_level	= header.pmode;
  else
    rec->sf_level	= -1;
  rec->ssf_level	= rec->sf_level;
  rec->pwmode		= header.fwdct;
  if (rec->pwmode > MAXPWMODE)
    rec->pwmode		= 0;
  rec->mybbsupd		= header.txdate;
  rec->mybbsmode	= header.id[0];
  if (*rec->mybbs != '\0' && rec->mybbsmode != 'U' && rec->mybbsmode != 'G')
    rec->mybbsmode	= 'U';
    
  rec->sfmd2pw		= (short)header.id[1];
  rec->srbytes		= header.start;
  rec->ssbytes		= header.packsize;
  rec->sfstat_rx_p	= header.rxqrg;

  p1			= (char *)(&rec->sfstat_rx_b);
  p2			= header.dest;
  for (x = 0; x <= 3; x++)
    p1[x]		= p2[x];

  p1			= (char *)(&rec->sfstat_rx_s);
  p2			= (char *)(&p2[4]);
  for (x = 0; x <= 3; x++)
    p1[x]		= p2[x];

  p1			= (char *)(&rec->sfstat_tx_p);
  p2			= header.rxfrom;
  for (x = 0; x <= 3; x++)
    p1[x]		= p2[x];

  p1			= (char *)(&rec->sfstat_tx_b);
  p2			= (char *)(&p2[4]);
  for (x = 0; x <= 2; x++)
    p1[x]		= p2[x];
  p2			= &header.msgtype;
  p1[3]			= p2[0];

  p1			= (char *)(&rec->sfstat_tx_s);
  p2			= (char *)(&header.lifetime);
  for (x = 0; x <= 1; x++)
    p1[x]		= p2[x];
  p2			= (char *)(&header.txlifetime);
  for (x = 0; x <= 1; x++)
    p1[x + 2]		= p2[x];

  rec->ssrbytes		= header.erasetime;
  rec->sslogins		= header.readcount;

  p2			= (char *)(&header.infochecksum);
  p1			= (char *)(&rec->sssbytes);
  for (x = 0; x <= 1; x++)
    p1[x]		= p2[x];
  p2			= (char *)(&header.bcastchecksum);
  for (x = 0; x <= 1; x++)
    p1[x + 2]		= p2[x];

  p2			= header.sendbbs;
  p1			= (char *)(&rec->pwsetat);
  if (p2[0] == 1) {
    for (x = 0; x <= 3; x++) {
      p1[x]		= p2[x + 1];

    }
  } else { /* this is an update procedure. it is not used today */
    if (strlen(rec->password) >= MINPWLEN)
      rec->pwsetat	= clock_.ixtime - 7*SECPERDAY;
  }

  rec->logins		= header.msgflags;

  rec->maxread_day	= (long)header.firstbyte * MAXREADDIVISOR;
  if (clock_.ixtime - rec->lastatime > SECPERDAY)
    rec->read_today	= 0;
  else
    rec->read_today	= (long)header.eraseby * MAXREADDIVISOR;

  if (*ds != '\0' && in_real_sf(rec->call)) {
    get_word(ds, w);
    rec->dstat_rx_p	= atol(w);
    get_word(ds, w);
    rec->dstat_rx_b	= atol(w);
    get_word(ds, w);
    rec->dstat_rx_s	= atol(w);
    get_word(ds, w);
    rec->dstat_rx_bytes	= atol(w);
    get_word(ds, w);
    rec->dstat_tx_p	= atol(w);
    get_word(ds, w);
    rec->dstat_tx_b	= atol(w);
    get_word(ds, w);
    rec->dstat_tx_s	= atol(w);
    get_word(ds, w);
    rec->dstat_tx_bytes	= atol(w);
    get_word(ds, w);
    rec->dlogins	= (unsigned short)atol(w);
    return;
  }

  rec->dstat_rx_p	= rec->sfstat_rx_p;
  rec->dstat_rx_b	= rec->sfstat_rx_b;
  rec->dstat_rx_s	= rec->sfstat_rx_s;
  rec->dstat_rx_bytes	= rec->srbytes;
  rec->dstat_tx_p	= rec->sfstat_tx_p;
  rec->dstat_tx_b	= rec->sfstat_tx_b;
  rec->dstat_tx_s	= rec->sfstat_tx_s;
  rec->dstat_tx_bytes	= rec->ssbytes;
  rec->dlogins		= rec->logins;
}


void load_userfile(boolean only_m, boolean with_ext_strings, char *calls, userstruct *rec)
{
  indexstruct	header, *hp;
  calltype	mbbs;
  char		lan[256];
  time_t	time;

  debug(4, 0, 9, calls);

  if (load_uf_through_cache(calls, rec))
    return;

  with_ext_strings	= true;

  rec->M_pos		= load_m_call(calls, &header);

  if (rec->M_pos > 0) {
    hp			= &header;   /*alter eintrag ?*/
    if (hp->size == 0) {
      clear_userstruct(rec);
      strcpy(rec->call, hp->absender);
      strcpy(rec->mybbs, hp->verbreitung);
      strcpy(rec->language, hp->id);
      cut(hp->dest, 80);
      strcpy(rec->name, hp->dest);
      strcpy(rec->password, hp->betreff);
      rec->lastdate	= hp->rxdate;
      rec->level	= hp->level;
      rec->plevel	= hp->level;
      rec->sf_level	= hp->pmode;
      rec->ssf_level	= rec->sf_level;
      rec->pwmode	= hp->msgflags;
      if (rec->pwmode > MAXPWMODE)
	rec->pwmode	= 0;
      rec->mybbsupd	= hp->rxqrg;
      rec->mybbsmode	= hp->id[0];
    } else
      convert_ufil(with_ext_strings, *hp, rec);
  } else
    clear_userstruct(rec);

  if (*rec->call == '\0' && !only_m) {
    *mbbs		= '\0';
    update_mybbsfile(false, calls, &time, mbbs, &rec->mybbsmode);
	/*mal in den M - Meldungen suchen*/
    if (*mbbs != '\0') {
      strcpy(rec->call, calls);
      user_language(&whichlangmem, &whichlangsize, whotalks_lan, rec->call, lan);
      cut(lan, 3);
      strcpy(rec->language, lan);
      strcpy(rec->mybbs, mbbs);
      rec->mybbsupd	= time;
    }
  }
  
  if (rec->sf_level > 1) rec->sf_level = 1; /* keine Sonderrechte mehr fuer DieBox */
  if (rec->ssf_level > 1) rec->ssf_level = 1; /* dito */
  
  if (*rec->mybbs != '\0' && rec->mybbsmode != 'U' && rec->mybbsmode != 'G')
    rec->mybbsmode	= 'U';

  if (*rec->call != '\0')
    add_uf_cache(*rec);
}


void load_userinfo_for_change(boolean only_m, char *callx, userstruct *ufil)
{
  char lan[256];

  if (!callsign(callx))
    return;

  load_userfile(only_m, true, callx, ufil);
  if (*ufil->call != '\0' && *ufil->language != '\0')
    return;

  user_language(&whichlangmem, &whichlangsize, whotalks_lan, callx, lan);
  cut(lan, 3);
  strcpy(ufil->language, lan);
  strcpy(ufil->call, callx);
}


void code_ufil(userstruct *rec, indexstruct *header)
{
  char		*p1, *p2;
  short		x, k, i;
  char		hs[256], w[256], ds[256];
  char		STR7[256];
  short		FORLIM;

  memset(header, 0, sizeof(indexstruct));
  *ds = '\0';
  if (in_real_sf(rec->call)) {
    sprintf(ds, "%ld %ld %ld %ld %ld %ld %ld %ld %d",
		rec->dstat_rx_p, rec->dstat_rx_b, rec->dstat_rx_s, rec->dstat_rx_bytes,
		rec->dstat_tx_p, rec->dstat_tx_b, rec->dstat_tx_s, rec->dstat_tx_bytes,
		rec->dlogins);
  }

  strcpy(header->absender, rec->call);
  strcpy(header->verbreitung, rec->mybbs);
  p1		= (char *)(&header->size);
  p2		= rec->language;
  for (x = 0; x <= 3; x++)
    p1[x]	= p2[x];
  strcpy(hs, rec->name);
  cut(hs, 80);
  strcat(hs, ";1");
  cut(rec->promptmacro, 130 - strlen(hs));
  sprintf(hs + strlen(hs), "%s;2", rec->promptmacro);
  strcpy(w, rec->logincommands);
  cut(w, 132 - strlen(hs));
  FORLIM	= strlen(w);
  for (x = 0; x < FORLIM; x++) {
    if (w[x] == ';')
      w[x]	= ',';
  }

  if (strlen(rec->checkboards) + strlen(ds) +
      strlen(w) + strlen(hs) > 127) {
    sprintf(STR7, "%s%s%cEXT", extuserdir, rec->call, extsep);
    k = sfcreate(STR7, FC_FILE);
    if (k >= minhandle) {
      if (rec->wantboards)
        str2file(&k, "", true);
      str2file(&k, rec->checkboards, true);
      if (!rec->wantboards)
        str2file(&k, "", true);
      str2file(&k, ds, true);
      sfclose(&k);
      sprintf(hs + strlen(hs), "%s;5", w);
    } else
      strcat(hs, w);
    *w = '\0';
  } else {
    sprintf(STR7, "%s%s%cEXT", extuserdir, rec->call, extsep);
    sfdelfile(STR7);
    if (rec->wantboards && *rec->checkboards != '\0') {
      sprintf(hs + strlen(hs), "%s;4", w);
    } else {
      sprintf(hs + strlen(hs), "%s;3", w);
    }
    strcpy(w, rec->checkboards);
    if (*ds != '\0')
      sprintf(w + strlen(w), ";8%s", ds);
  }

  cut(w, 131 - strlen(hs));
  strcat(hs, w);

  i = rec->readlock;
  if (i > 2) i = 2; /* zur Sicherheit */
  if (rec->hidebeacon) /* und hide-beacon-flag einkodieren */
    i += 3;
  sprintf(w, "%d", i);
  sprintf(hs + strlen(hs), ";6%c", w[0]);

  x = 0;
  if (rec->ttl)
    x++;
  if (!rec->lock_here)
    x += 2;
  if (rec->login_priv)
    x += 4;
  sprintf(hs + strlen(hs), ";7%d", x);

  x = 0;
/* if (rec->sfmd2pw)
    x++; */ /* frei ab v5.04.05 */
  if (rec->fbbmode)
    x += 2;
  if (rec->unproto_ok)
    x += 4;
  sprintf(hs + strlen(hs), ";9%d", x);

  strcpy(header->readby, hs);

  strcpy(header->betreff, rec->password);
  header->rxdate	= rec->lastdate;
  if (rec->lastdate != 0 && rec->lastatime == 0)
    rec->lastatime	= rec->lastdate;
  header->lastread	= rec->lastatime;
  header->level		= rec->plevel;
  header->txdate	= rec->mybbsupd;
  header->deleted	= false;
  if (rec->ssf_level >= 0)
    header->pmode	= rec->ssf_level;
  else
    header->pmode	= 255;
  if (rec->pwmode > MAXPWMODE)
    rec->pwmode		= 0;
  header->fwdct		= rec->pwmode;

  rec->srbytes		+= rec->rbytes;
  rec->ssbytes		+= rec->sbytes;
  rec->ssrbytes		+= rec->rbytes;
  rec->sssbytes		+= rec->sbytes;
  rec->sfstat_rx_p	+= rec->fstat_rx_p;
  rec->fstat_rx_p	= 0;
  rec->sfstat_rx_b	+= rec->fstat_rx_b;
  rec->fstat_rx_b	= 0;
  rec->sfstat_rx_s	+= rec->fstat_rx_s;
  rec->fstat_rx_s	= 0;
  rec->sfstat_tx_p	+= rec->fstat_tx_p;
  rec->fstat_tx_p	= 0;
  rec->sfstat_tx_b	+= rec->fstat_tx_b;
  rec->fstat_tx_b	= 0;
  rec->sfstat_tx_s	+= rec->fstat_tx_s;
  rec->fstat_tx_s	= 0;

  for (x = 1; x <= MAXUSER; x++) {
    if (user[x] != NULL && !strcmp(user[x]->call, rec->call)) {
      user[x]->srbytes	= rec->srbytes;
      user[x]->ssbytes	= rec->ssbytes;
      user[x]->ssrbytes = rec->ssrbytes;
      user[x]->sssbytes = rec->sssbytes;
      user[x]->sfstat_rx_p = rec->sfstat_rx_p;
      user[x]->sfstat_rx_b = rec->sfstat_rx_b;
      user[x]->sfstat_rx_s = rec->sfstat_rx_s;
      user[x]->sfstat_tx_p = rec->sfstat_tx_p;
      user[x]->sfstat_tx_b = rec->sfstat_tx_b;
      user[x]->sfstat_tx_s = rec->sfstat_tx_s;
      user[x]->logins	= rec->logins;
      user[x]->sslogins	= rec->sslogins;
    }
  }

  header->start		= rec->srbytes;
  header->packsize	= rec->ssbytes;
  header->rxqrg		= rec->sfstat_rx_p;

  p1 = (char *)(&rec->sfstat_rx_b);
  p2 = header->dest;
  for (x = 0; x <= 3; x++)
    p2[x] = p1[x];

  p1 = (char *)(&rec->sfstat_rx_s);
  p2 = (char *)(&p2[4]);
  for (x = 0; x <= 3; x++)
    p2[x] = p1[x];

  p1 = (char *)(&rec->sfstat_tx_p);
  p2 = header->rxfrom;
  for (x = 0; x <= 3; x++)
    p2[x] = p1[x];

  p1 = (char *)(&rec->sfstat_tx_b);
  p2 = (char *)(&p2[4]);
  for (x = 0; x <= 2; x++)
    p2[x] = p1[x];
  p2 = &header->msgtype;
  p2[0] = p1[3];

  p1 = (char *)(&rec->sfstat_tx_s);
  p2 = (char *)(&header->lifetime);
  for (x = 0; x <= 1; x++)
    p2[x] = p1[x];
  p2 = (char *)(&header->txlifetime);
  for (x = 0; x <= 1; x++)
    p2[x] = p1[x + 2];

  header->msgflags	= rec->logins;
  header->erasetime	= rec->ssrbytes;
  header->readcount	= rec->sslogins;

  p2 = (char *)(&header->infochecksum);
  p1 = (char *)(&rec->sssbytes);
  for (x = 0; x <= 1; x++)
    p2[x] = p1[x];
  p2 = (char *)(&header->bcastchecksum);
  for (x = 0; x <= 1; x++)
    p2[x] = p1[x + 2];

  p2 = header->sendbbs;
  p1 = (char *)(&rec->pwsetat);
  p2[0] = 1;
  for (x = 0; x <= 3; x++)
    p2[x + 1] = p1[x];

  header->firstbyte	= ((rec->maxread_day + MAXREADDIVISOR - 1) / MAXREADDIVISOR) & 0xff;
  header->eraseby	= ((rec->read_today  + MAXREADDIVISOR - 1) / MAXREADDIVISOR) & 0xff;

  header->id[0]		= (char)rec->mybbsmode;
  header->id[1]		= (char)rec->sfmd2pw; /* neu ab v5.04.05 */
  for (x = 2; x < 12; x++)
    header->id[x]	= '\0';

}


void save_userfile(userstruct *rec)
{
  short		k, list, last;
  boolean	found, full;
  long		isize;
  indexstruct	header;
  userstruct	rec2;

  debug0(4, 0, 10);

  isize		= sfsize(userinfos);
  last		= isize / sizeof(indexstruct);
  full		= last > (SHORT_MAX - 30);
  list		= sfopen(userinfos, FO_RW);
  found		= false;
  k		= nohandle;

  if (list >= minhandle) {
    k		= rec->M_pos;
    if (k > 0 && k <= last) {
      read_index(list, k, &header);
      if (!header.deleted && !strcmp(header.absender, rec->call) && check_hcs(header))
        found	= true;
    }

    if (!found) {
      k		= find_mptr(rec->call);
      if (k > 0 && k <= last) {
        read_index(list, k, &header);
        if (!header.deleted && !strcmp(header.absender, rec->call) && check_hcs(header))
          found	= true;
      }
    }
  }
  
  if (list < minhandle)
    list	= sfcreate(userinfos, FC_FILE);
  if (list < minhandle) {
    clear_uf_cache(rec->call);
    return;
  }
  
  code_ufil(rec, &header);
  if (found)
    k		= write_index(list, k, &header);
  else if (full) {
    debug(0, 0, 10, "M.IDX full");
    k		= 0;
  } else
    k		= write_index(list, -1, &header);
  sfclose(&list);

  if (k <= 0) {
    clear_uf_cache(rec->call);
    return;
  }
  
  add_mptr(rec->call, k);

  convert_ufil(true, header, &rec2);
  if (*rec2.call == '\0')
    return;
  
  rec->M_pos	= k;
  rec2.M_pos	= k;
  add_uf_cache(rec2);
}


void user_mybbs(char *call, char *mbx)
{
  userstruct	ufil;

  *mbx		= '\0';
  load_userfile(false, false, call, &ufil);
  if (*ufil.call != '\0') {
    if (*ufil.mybbs != '\0')
      strcpy(mbx, ufil.mybbs);
  }
}


/* ********************************************************************************* */

/* Gibt an, welche Indexnummer der Rubrik die letzte gueltige fuer den       */
/* aktuellen User ist. Hidden-Flag und Level werden beachtet, auch ufilhide  */

short last_valid(short unr, char *brett)
{
  short		Result;
  indexstruct	header;
  userstruct	uf;
  long		isize;
  short		k, mct;
  boolean	hit, iscall, ufloaded, ownboard;
  unsigned	short lt, acc;
  char		hs[256];
  char		STR1[256];

  debug(3, unr, 18, brett);

  if (!boxrange(unr))
    return 0;

  lt		= 0;
  acc		= SHORT_MAX;

  if (strlen(brett) == 1) {
    if (user[unr]->supervisor || user[unr]->rsysop) {
      if (!may_sysaccess(unr, brett))
	return 0;
      check_lt_acc(brett, &lt, &acc);
    } else
      acc	= SHORT_MAX;
  } else
    check_lt_acc(brett, &lt, &acc);

  if (user[unr]->level < acc)
    return 0;

  hit		= false;
  iscall	= callsign(brett);
  ownboard	= (iscall && !strcmp(brett, user[unr]->call));
  ufloaded	= false;
  uf.readlock	= 2;

  idxfname(STR1, brett);
  isize		= sfsize(STR1);
  if (isize <= 0)
    return 0;

  mct		= isize / sizeof(indexstruct);
  k		= open_index(brett, FO_READ, user[unr]->hidden, false);
  if (k < minhandle)
    return 0;
    
  Result	= 0;

  while (mct > 0 && hit == false && k >= minhandle) {
    read_index(k, mct, &header);
    /* mit absicht kein check_hcs */
    if (header.deleted != user[unr]->hidden) {
      mct--;
      continue;
    }

    if (!(iscall || header.msgtype != 'B') || user[unr]->supervisor) {
      hit		= true;
      break;
    }
    
    if (iscall && !ufilhide && !ufloaded) {
      load_userfile(true, false, brett, &uf);
      if (*uf.call == '\0')
        uf.readlock	= 2;
      ufloaded		= true;
    }

    unhpath(header.verbreitung, hs);

    if (ufilhide || uf.readlock == 2 || strcmp(hs, Console_call)) {
      hit = (ownboard || !strcmp(header.absender, user[unr]->call) ||
	       !strcmp(header.dest, user[unr]->call));

      if (!hit)
        Result		= -1;

    } else if (uf.readlock == 1) {
      hit = (ownboard || !strcmp(header.absender, user[unr]->call) ||
	      !strcmp(header.dest, user[unr]->call) ||
      	      strstr(header.readby, brett) != NULL);

      if (!hit)
        Result		= -1;

    } else
      hit		= true;

    if (!hit)
    mct--;
  }

  close_index(&k);

  if (hit)
    return mct;

  return Result;
}


/* Diese Routine legt ein Zusatzfile an, welches beim naechsten Aufruf der   */
/* Forwardliste beruecksichtigt wird.                                        */

typedef struct erafwdtype {
  long		new_msgnum;
  char		what;
  bidtype	bid;
  calltype	sfcall;
  boardtype	new_board;
} erafwdtype;


void alter_fwd(char what, char *bid, long new_msgnum, char *new_board, char *sfcall)
{
  erafwdtype	elog;
  short		k;
  pathstr	hs;

  if (strlen(bid) > LEN_BID)
    return;

  if (strlen(new_board) > LEN_BOARD)
    return;

  if (strlen(sfcall) > LEN_CALL)
    return;

  strcpy(elog.bid, bid);
  elog.what		= what;
  elog.new_msgnum	= new_msgnum;
  strcpy(elog.new_board, new_board);
  strcpy(elog.sfcall, sfcall);
  
  sprintf(hs, "%salterfwd%cbox", boxstatdir, extsep);
  k			= sfopen(hs, FO_RW);
  if (k < minhandle)
    k			= sfcreate(hs, FC_FILE);
  if (k < minhandle)
    return;

  sfseek(0, k, SFSEEKEND);
  sfwrite(k, sizeof(erafwdtype), (char *)(&elog));
  sfclose(&k);
}

static boolean in_alterfwd_list(char *epuffer, long esize, char *bid, char *fwdto, long *epos)
{
  erafwdtype	*elog;

  if (esize <= 0)
    return false;

  *epos		= 0;
  while (*epos < esize) {
    elog	= (erafwdtype *)(&epuffer[*epos]);
    if (!strcmp(elog->bid, bid)) {
      if ((*elog->sfcall == '\0') || (!strcmp(elog->sfcall, fwdto)))
        return true;
      else
	*epos	+= sizeof(erafwdtype);
    }
    else
      *epos	+= sizeof(erafwdtype);
  }
  return false;
}


static void alter_fwdptr(char *epuffer, long esize, indexstruct *logptr, long epos)
{
  erafwdtype	*elog;
  
  while (epos < esize) {
    elog	= (erafwdtype *)(&epuffer[epos]);
    epos	+= sizeof(erafwdtype);
    if (strcmp(logptr->id, elog->bid))
      continue;
    if (*elog->sfcall != '\0' && strcmp(elog->sfcall, logptr->dest))
      continue;
    
    switch (elog->what) {

    case 'E':
      logptr->deleted	= true;
      break;

    case 'U':
      logptr->deleted	= false;
      break;

    case 'R':
      logptr->msgflags	&= ~(MSG_SFHOLD | MSG_LOCALHOLD);
      break;
    
    case 'T':
      strcpy(logptr->betreff, elog->new_board);
      logptr->msgnum	= elog->new_msgnum;
      break;
    }
  }
}


#define cpentries       150
#define cpsize          (cpentries * sizeof(indexstruct))

boolean recompile_fwd(void)
{
  long		err;
  short		log;
  indexstruct	*logptr;
  long		lv, ct;
  indexstruct	logheader;
  char		*epuffer;
  long		esize, retep;
  pathstr	efname;

  debug0(2, -1, 164);

  sprintf(efname, "%salterfwd%cbox", boxstatdir, extsep);
  if (!exist(efname))
    return false;
    
  sfbread(true, efname, &epuffer, &esize);
  if (esize > 0) {

    err		= sfsize(sflist);

    if (err <= 0) {
      free(epuffer);
      sfdelfile(efname);
      return false;
    }

    lv		= err / sizeof(indexstruct);

    log		= open_index("X", FO_RW, true, true);

    if (log >= minhandle && lv > 0) {
      ct	= 0;
      while (ct < lv) {
	ct++;
	if ((ct & 63) == 0)   /* Watchdog resetten */
	  dp_watchdog(2, 4711);

	read_index(log, ct, &logheader);
	logptr	= &logheader;

	if (check_hcs(*logptr) && in_alterfwd_list(epuffer, esize, logptr->id, logptr->dest, &retep)) {
	  alter_fwdptr(epuffer, esize, logptr, retep);
	  write_index(log, ct, logptr);
	}

      }
    }

    close_index(&log);
    free(epuffer);

  }
  sfdelfile(efname);
  return true;
}

#undef cpentries
#undef cpsize




/* Diese Routine legt ein Zusatzfile an, welches beim naechsten Aufruf der   */
/* Checkliste beruecksichtigt wird. Die hier gemachten Aenderungen werden    */
/* also erst bei CHECK wirklich in BOXLOG.DP uebernommen...                  */

typedef struct eralogtype {
  long			msgnum;
  char			what;
  unsigned short	msgflags;
  char			info[21];
} eralogtype;


void alter_log(boolean onram, long msgnumber, unsigned short msgflags,
	       char what, char *info)
{
  eralogtype	elog;
  short		k;

  debug(3, 0, 19, info);

  elog.msgnum	= msgnumber;
  elog.what	= what;
  elog.msgflags	= msgflags;
  if (strlen(info) <= 20)
    strcpy(elog.info, info);
  else
    *elog.info	= '\0';

  k		= sfopen(eralog, FO_RW);
  if (k < minhandle)
    k		= sfcreate(eralog, FC_FILE);
  if (k < minhandle) {
    debug(0, 0, 19, "open error");
    return;
  }
  sfseek(0, k, SFSEEKEND);
  sfwrite(k, sizeof(eralogtype), (char *)(&elog));
  sfclose(&k);
}


static boolean in_erase_list(char *epuffer, long esize, long msgnumber, short nr, long *epos)
{
  eralogtype	*elog;

  if (esize > 0) {
    *epos	= 0;
    while (*epos < esize) {
      elog	= (eralogtype *)(&epuffer[*epos]);
      if (elog->msgnum == msgnumber)
	return true;
      else
	*epos	+= sizeof(eralogtype);
    }
  }
  return false;
}


static void alter_logptr(char *epuffer, long esize, boxlogstruct **logptr, long epos)
{
  eralogtype	*elog;
  long	      	mnum;
  char		w2[256];

  while (epos < esize) {
    elog	= (eralogtype *)(&epuffer[epos]);
    epos	+= sizeof(eralogtype);
    if ((*logptr)->msgnum != elog->msgnum)
      continue;

    elog->msgnum = -1;
    switch (elog->what) {

    case 'K':
      mnum = (*logptr)->msgnum;
      memset(*logptr, 0, sizeof(boxlogstruct));
      (*logptr)->deleted	= true;
      (*logptr)->msgnum       	= mnum;
      break;

    case 'E':
      (*logptr)->deleted	= true;
      break;

    case 'U':
      (*logptr)->deleted	= false;
      break;

    case '#':
      get_word(elog->info, w2);
      (*logptr)->lifetime	= atoi(w2);
      break;

    case '@':
      get_word(elog->info, w2);
      cut(w2, 6);
      strcpy((*logptr)->verbreitung, w2);
      break;

    case '$':
      get_word(elog->info, w2);
      cut(w2, LEN_BID);
      strcpy((*logptr)->bid, w2);
      break;
    }
    if (elog->msgflags != 0)
      (*logptr)->msgflags	= elog->msgflags;
  }
}


#define cpentries       CHECKBLOCKSIZE
#define cpsize          (cpentries * sizeof(boxlogstruct))

void recompile_log(short unr)
{
  long		cpstart, err, seekpos;
  short		log;
  boxlogstruct	*logptr;
  long		lv, ct;
  boxlogstruct	logheader;
  char		*ipuffer, *epuffer;
  long		esize, retep;
  long		act;

  debug(2, unr, 21, boxlog);

  if (!exist(eralog))
    return;

  sfbread(true, eralog, &epuffer, &esize);
  if (esize > 0) {
    act		= esize / sizeof(eralogtype);
    err		= sfsize(boxlog);

    if (err <= 0) {
      free(epuffer);
      sfdelfile(eralog);
      return;
    }

    ipuffer	= malloc(cpsize);
    lv		= err / sizeof(boxlogstruct);
    log		= sfopen(boxlog, FO_RW);
    cpstart	= 0;

    if (log >= minhandle && lv > 0) {
      ct	= 0;
      while (ct < lv && act > 0) {
	ct++;
	if ((ct & 63) == 0)   /* Watchdog resetten */
	  dp_watchdog(2, 4711);

	if (ipuffer == NULL) {
	  read_log(log, ct, &logheader);
	  logptr	= &logheader;
	} else {
	  if (ct >= cpstart + cpentries || cpstart == 0) {
	    if (cpstart == 0)
	      cpstart	= 1;
	    else
	      cpstart	+= cpentries;
	    if (cpstart > lv)
	      break;
	    seekpos	= (cpstart - 1) * sizeof(boxlogstruct);
	    if (sfseek(seekpos, log, SFSEEKSET) != seekpos)
	      break;
	    if (sfread(log, cpsize, ipuffer) <= 0)
	      break;
	    if (ct >= cpstart + cpentries)
	      break;
	  }

	  logptr	= (boxlogstruct *)(&ipuffer[(ct - cpstart) * sizeof(boxlogstruct)]);
	}

	if (in_erase_list(epuffer, esize, logptr->msgnum, logptr->idxnr, &retep)) {
	  act--;
	  alter_logptr(epuffer, esize, &logptr, retep);
	  write_log(log, ct, logptr);
	}
      }
    }

    if (ipuffer != NULL)
      free(ipuffer);
    sfclose(&log);
    free(epuffer);
  }
  sfdelfile(eralog);
}


short check_held_messages(short unr, short display_if_less_than)
{
  long		cpstart, err, seekpos;
  short		log, held_messages, outhandle;
  boxlogstruct	*logptr;
  long		lv, ct;
  boxlogstruct	logheader;
  char		*ipuffer;
  char	      	hs[256];
  pathstr     	outfile;

  recompile_log(unr);
  held_messages = 0;
  err		= sfsize(boxlog);
  lv		= err / sizeof(boxlogstruct);
  if (lv <= 0) return 0;
  ipuffer	= malloc(cpsize);
  log		= sfopen(boxlog, FO_READ);
  if (log < minhandle) return 0;
  snprintf(outfile, 255, "%sHELD_XXXXXX", tempdir);
  mymktemp(outfile);
  cpstart	= 0;

  ct	= 0;
  while (ct < lv) {
    ct++;
    if ((ct & 2000) == 0)   /* Watchdog resetten */
    	dp_watchdog(2, 4711);

    if (ipuffer == NULL) {
      read_log(log, ct, &logheader);
      logptr	= &logheader;
    } else {
      if (ct >= cpstart + cpentries || cpstart == 0) {
	if (cpstart == 0)
	  cpstart	= 1;
	else
	  cpstart	+= cpentries;
	if (cpstart > lv)
	  break;
	seekpos	= (cpstart - 1) * sizeof(boxlogstruct);
	if (sfseek(seekpos, log, SFSEEKSET) != seekpos)
	  break;
	if (sfread(log, cpsize, ipuffer) <= 0)
	  break;
	if (ct >= cpstart + cpentries)
	  break;
      }
      logptr	= (boxlogstruct *)(&ipuffer[(ct - cpstart) * sizeof(boxlogstruct)]);
    }

    /* now the check for held messages */
    if ((logptr->msgflags & (MSG_SFHOLD|MSG_LOCALHOLD)) != 0) {
      if (   check_access_ok(unr, false, false, false, false, false, logptr)
      	  || check_access_ok(unr, true, false, false, false, false, logptr) ) {
      	disp_logptr(unr, ++held_messages, false, logptr, hs);
      	append(outfile, hs, true);
      }
    }

  }

  if (ipuffer != NULL)
    free(ipuffer);
  sfclose(&log);

  if (held_messages > 0 && held_messages < display_if_less_than) {
    snprintf(hs, 255, "%d messages on hold", held_messages);
    wlnuser(unr, hs);
    wlnuser0(unr);
    if (user[unr]->fbbmode)
      wln_btext(unr, 183);
    else
      wln_btext(unr, 17);
    wlnuser0(unr);
    outhandle = sfopen(outfile, FO_READ);
    if (outhandle >= minhandle) {    
      while (file2str(outhandle, hs)) wlnuser(unr, hs);
      sfclose(&outhandle);
    }
  }
  sfdelfile(outfile);

  return (held_messages);
}

#undef cpentries
#undef cpsize

void show_hold(short unr, short disp_if_less)
{
  short m;
  char	hs[256];

  m = check_held_messages(unr, disp_if_less);
  if (m >= disp_if_less) {
    snprintf(hs, 255, "%d messages on hold", m);
    wlnuser(unr, hs);
    wlnuser0(unr);
  } else if (m == 0 && disp_if_less != MAXDISPLOGINHOLD) {
    wln_btext(unr, 37);
  } else if (disp_if_less == MAXDISPLOGINHOLD) {
    wlnuser0(unr);
  }

}


short boxcheck(boolean all, char *callx)
{
  /* all wird nicht benutzt */
  short		list, msgct, ct, lv;
  indexstruct	header;
  char		hs[256];
  mbxtype	hmbx;
  pathstr	STR1;

  debug(3, 0, 22, callx);

  msgct		= 0;
  if (!(all || strcmp(callx, Console_call)))
    return msgct;

  idxfname(STR1, callx);
  lv		= sfsize(STR1) / sizeof(indexstruct);
  if (lv <= 0)
    return msgct;

  list		= sfopen(STR1, FO_READ);
  if (list < minhandle)
    return msgct;

  strcpy(hs, callx);
  rspacing(hs, LEN_CALL);
  ct		= lv;
  while (ct > 0) {
    read_index(list, ct, &header);
    if (!header.deleted && check_hcs(header)) {
      if (all)
	msgct++;
      else {
	if (clock_.ixtime - header.rxdate > 7*SECPERDAY)
	  break;
	else {
	  if (strstr(header.readby, hs) == NULL && header.fwdct == 0) {
	    unhpath(header.verbreitung, hmbx);
	    if (!strcmp(hmbx, Console_call)) {
	      msgct++;
	      break;
	    }
	  } else
	    break;
	}
      }
    }
    ct--;
  }
  sfclose(&list);
  return msgct;
}


void send_tcpip_protocol_frame(short unr)
{
  tcpiptype	*hp;
  short		chan;
  char		hs[256];

  chan		= user[unr]->pchan;
  if (chan <= 0)
    return;

  hp		= tcpiproot;
  while (hp != NULL) {
    if (!strcmp(hp->call, user[unr]->call)) {
      strcpy(hs, hp->txt);
      expand_macro(unr, hs);
      wlnuser(unr, hs);
      boxspoolfend(chan);
    }
    hp		= hp->next;
  }
}



/* ------------------------------------------------------------------------------ */


static unsigned short mfilter_sem;


static short call_m_filter(char **puffer, long *size, long *lesezeiger,
  char *frombox, char *ziel, char *mbx, char *absender, char *lifetime,
  char *bulletin_id, char *betreff, boolean *no_sf)
{
  short		Result, handle, ergebnis;
  long		rp;
  pathstr	tempname, filtername;
  char		hs[256];

  debug(2, 0, 25, ziel);
  *no_sf	= false;
  Result	= 0;
  mfilter_sem++;
  sprintf(tempname, "%sM_FILTER%c%d", tempdir, extsep, mfilter_sem);
  strcpy(filtername, m_filter_prg);
  handle	= sfcreate(tempname, FC_FILE);
  if (handle < minhandle)
    return Result;

  if (*frombox != '\0')
    str2file(&handle, frombox, true);
  else
    str2file(&handle, Console_call, true);
  str2file(&handle, absender, true);
  str2file(&handle, ziel, true);
  str2file(&handle, mbx, true);
  str2file(&handle, lifetime, true);
  str2file(&handle, bulletin_id, true);
  str2file(&handle, betreff, true);
  if (sfwrite(handle, *size - *lesezeiger, (char *)(&(*puffer)[*lesezeiger])) ==
      *size - *lesezeiger) {
    sfclose(&handle);
    ergebnis	= call_prg(filtername, tempname, "OUT.TXT");
    if (ergebnis > 4) ergebnis = 0;
    if (ergebnis > 2) {
      if (ergebnis == 4)
	*no_sf	= true;
      ergebnis	= 1;   /* erstmal als geloescht markieren */
      free(*puffer);
      rp	= 0;
      *size	= sfsize(tempname);
      if (*size <= maxram()) {
	handle	= sfopen(tempname, FO_READ);
	if (handle >= minhandle) {
	  *puffer	= malloc(2048);
	  if (*puffer != NULL) {
	    sfread(handle, 2048, *puffer);
	    get_line(*puffer, &rp, *size, hs);
	    get_line(*puffer, &rp, *size, hs);
	    get_line(*puffer, &rp, *size, hs);
	    cut(hs, 8);
	    strcpy(ziel, hs);
	    get_line(*puffer, &rp, *size, hs);
	    get_line(*puffer, &rp, *size, hs);
	    cut(hs, 3);
	    strcpy(lifetime, hs);
	    get_line(*puffer, &rp, *size, hs);
	    get_line(*puffer, &rp, *size, hs);
	    free(*puffer);
	    sfseek(rp, handle, SFSEEKSET);
	    *size	-= rp;
	    *puffer	= malloc(*size);
	    if (*puffer != NULL) {
	      if (sfread(handle, *size, *puffer) == *size) {
		*lesezeiger	= 0;
		ergebnis	= 0;   /* ok, gueltig */
	      } else {
		*size		= 0;
		free(*puffer);
		debug(0, 0, 25, "read error");
		ergebnis	= 0;
		m_filter	= false;
	      }
	    }
	  }
	  sfclose(&handle);
	}
      }
    } else if (ergebnis == 2) {
      *no_sf	= true;
      ergebnis	= 0;
    } else if (ergebnis < 0) {
      debug(0, 0, 25, "EXEC ERROR");
      ergebnis	= 0;
      m_filter	= false;
    }
    Result	= ergebnis;
  } else
    sfclose(&handle);
  sfdelfile(tempname);
  return Result;
}


#define blocksize       16384

static void new_entry(short unr, char *dname1_, char *status_, char *betreff1,
		      char *bulletin_id, long size1, long lesezeiger,
		      char *rcall_, char *frombox_, boolean forwarding,
		      boolean sfcut, boolean authentisch, boolean broadcast)
{
  short		k, k1, k2, k4, nr, packresult, fidx, loopct, ergebnis;
  unsigned short acc, lth, ics;
  boolean	fw1, looperr, server, dieboxsys, no_sf, is_7plus, is_binary;
  boolean	ack_requested, compress, is_html, dirty, part, db, ok, usersf;
  boolean	hmbxiscall, dirtys, ugzip, is_broken;
  long		nsize, psize, bct, lastpos1, lastpos2, sv, hsize, rs, li;
  long		chargedate, txdate1, lesezeiger1;
  boolean	wpupdate, outdated, reject_it, hold_it, dbimport, dbimpfilter;
  boolean     	wprotupdate, direct_sf;
  char		*nmem, *puffer;
  char 		TEMP, mtyp, mtyp7ext;
  indexstruct	header;
  calltype	hmbx;
  calltype	rcall, frombox;
  char		info[256], index[256], archiv[256], temp1[256], temp2[256], dname[256];
  char		dname1[256], status[256], hs[256], hs2[256];
  char		ziel[256], ziel2[256], absender1[256], mbx[256];
  char		datum[256], zeit[256], laenge[256], lifetime1[256], ackcall[256];
  char		dirtystring[256], lastvias[MAXLASTVIAS*(LEN_CALL+1)+1];
  char 		STR1[256], STR7[36], STR13[256];

  debug(2, unr, 26, dname1_);

  strcpy(dname1, dname1_);
  strcpy(status, status_);
  strcpy(rcall, rcall_);
  strcpy(frombox, frombox_);
  memset(&header, 0, sizeof(indexstruct));
  strcpy(dname, dname1);
  ack_requested	= false;
  is_binary	= false;
  is_7plus	= false;
  dirty		= false;
  dirtys	= false;
  is_html	= false;
  server	= false;
  looperr	= false;
  wpupdate	= false;
  wprotupdate 	= false;
  reject_it	= false;
  hold_it	= false;
  outdated	= false;
  direct_sf   	= false;
  dbimport	= (unr == -96 || unr == -97 ||unr == -98 || unr == -99);
  dbimpfilter	= (unr == -96 || unr == -98);
  loopct	= 0;
  txdate1	= 0;
  header.packsize=0;
  ics		= 0;
  mtyp		= '\0';
  ackcall[0]	= '\0';
  lastvias[0]	= '\0';
  dirtystring[0]= '\0';
  chargedate	= 0;
  lesezeiger1	= lesezeiger;
  compress	= (size1 - lesezeiger >= PACKMIN && packdelay == 0);

  if (sfcut)
    split_sline(status, ziel, absender1, mbx, bulletin_id, lifetime1, hs);
  else
    mtyp = separate_status(status, ziel, absender1, mbx, datum, zeit, laenge, lifetime1);

  if (!strcmp(lifetime1, "-"))
    strcpy(lifetime1, "0");

  if (mtyp == '\0')
    check_msgtype(&mtyp, ziel, betreff1);

  strcpy(header.dest, ziel);
  header.txlifetime = atoi(lifetime1);
  no_sf = false;

  if (dbimport) {
    if (unr == -96 || unr == -97)
      cbbtboard(ziel);
    if (unr == -98 || unr == -99)
      cdbtboard(ziel);
  }
  check_verteiler(ziel);
  if (!valid_boardname(ziel)) {
    if (*default_rubrik != '\0')
      strcpy(ziel, default_rubrik);
    else
      strcpy(ziel, "TEMP");
  }

  fw1 = (forwarding || callsign(frombox));
  unhpath(mbx, hmbx);
  hmbxiscall = callsign(hmbx);

  if (broadcast)   /* Der S&F... */
    forwarding = (strcmp(hmbx, frombox) && bcastforward);
  else if (callsign(frombox))
    forwarding = true;

  hs[0] = '\0';

  dieboxsys = (strlen(ziel) == 1 &&
	       (TEMP = upcase_(ziel[0]), TEMP == 'E' || TEMP == 'M'));

  usersf = (!dbimport && !charge_bbs && fw1 && !dieboxsys &&
	    *frombox != '\0' && sfinpdefault < 3 &&
	    strcmp(frombox, Console_call) && !in_real_sf(frombox) && !(direct_sf = direct_sf_bbs(frombox)));

  if (usersf) {  /* im user-sf hinter die letzte R:-Zeile gehen */
    k1 = sfopen(dname, FO_READ);
    if (k1 >= minhandle) {
      if (sfseek(lesezeiger, k1, SFSEEKSET) == lesezeiger) {
	while (file2str(k1, hs) && strstr(hs, "R:") == hs)
	  lesezeiger = sfseek(0, k1, SFSEEKCUR);
      }
      sfclose(&k1);
    }
  } else if (fw1 && (!strcmp(frombox, Console_call) || charge_bbs)) {
    k1 = sfopen(dname, FO_READ);
    if (k1 >= minhandle) {
      if (sfseek(lesezeiger, k1, SFSEEKSET) == lesezeiger) {
	if (file2str(k1, hs) && strstr(hs, "R:") == hs &&
	    (charge_bbs || uspos(Console_call, hs) > 0)) {
	  if (charge_bbs) {
	    if (strlen(hs) >= '\016' && hs[13] != 'Z')
	      hs[13] = 'Z';
	    chargedate = get_headerdate(hs);
	  }
	  lesezeiger = sfseek(0, k1, SFSEEKCUR);
	}
      }
      sfclose(&k1);
    }
  }


  if (fw1 && !broadcast && !dbimport && !charge_bbs) {
    if (hmbxiscall && !strcmp(hmbx, Console_call)) {
      if (!strcmp(header.dest, "WP"))
	wpupdate = (callsign(absender1) &&
		    strcmp(absender1, Console_call) &&
		    (scan_all_wp || in_wpservers(absender1)));
      else if (!strcmp(header.dest, "W"))
      	wprotupdate = (strcmp(absender1, Console_call)
	      	      	&& !strcmp(absender1, frombox) && in_real_sf(frombox)
			&& !strcmp(hmbx, Console_call));
      else if (!strcmp(header.dest, "PING") || !strcmp(header.dest, "P1NG")) {
      	if (do_ping(absender1, mtyp, betreff1, bulletin_id, dname, lesezeiger, clock_.ixtime)) {
	  if (*bulletin_id != '\0')
	    write_msgid(-1, bulletin_id);
	  server = true;
	}
      } else if ((!strcmp(header.dest, "LOCAL") && mtyp == 'P') ||
	       (!strcmp(header.dest, "LOCBBS") && mtyp == 'P') ||
	       !strcmp(header.dest, "REGION") ||
	       !strcmp(header.dest, "NATION")) {
	if (do_redist(absender1, header.dest, betreff1, dname, lesezeiger)) {
	  if (*bulletin_id != '\0')
	    write_msgid(-1, bulletin_id);
	  server = true;
	}
      } else if (!strcmp(header.dest, Console_call) || in_servers(header.dest)) {
	if (do_server(absender1, header.dest, betreff1, dname, lesezeiger)) {
	  if (*bulletin_id != '\0')
	    write_msgid(-1, bulletin_id);
	  server = true;
	} else if (strcmp(header.dest, Console_call)) {
	  sprintf(betreff1, "SERVER %s: %s",
		  header.dest, strcpy(STR1, betreff1));
	  cut(betreff1, 80);
	  strcpy(ziel, "Y");
	  sprintf(STR7, "Invalid request to server %s", header.dest);
	  append_profile(unr, STR7);
	}
      }
    }
  }

  /* ------------------------------------------------------------------ */

  part = true;

  if (!dieboxsys) {
    psize = size1 - lesezeiger;
    if (psize <= maxram()) {
      puffer = malloc(psize);
      if (puffer != NULL) {
	part = false;
	k1 = sfopen(dname, FO_READ);
	if (k1 >= minhandle) {
	  sfseek(lesezeiger, k1, SFSEEKSET);
	  size1 = sfread(k1, psize, puffer);
	  sfclose(&k1);
	  lesezeiger = 0;
	}
      }
    }
  }


  /* ----------------------------------------------------------------- */

  if (server) {  /* auch bei servermails die hroutes updaten! */
    if (!part) {
      if (ana_hpath && !usersf && !direct_sf) {
	if (strcmp(rcall, "-moni-")) {
	  if (*frombox != '\0' || !fw1) {
	    strcpy(hs, frombox);
	    scan_hierarchicals(hs, puffer, size1, &txdate1,
			       in_real_sf(hs) && mtyp != 'A' && strcmp(header.dest, "WP"),
			       mtyp, lastvias);
	  }
	}
      }
      free(puffer);
    }
  }

  /* ----------------------------------------------------------------- */

  if (!dieboxsys && !server) {
    strcpy(ziel2, ziel);

    if (!part) {
      /* tja, sorry, bei nur teilweise in den Speicher zu ladenden     */
      /* Files wird der Filter nicht aufgerufen. Sollte aber in der    */
      /* Praxis nicht auftreten.                                       */

      if (m_filter)
	ergebnis = call_m_filter(&puffer, &size1, &lesezeiger, frombox, ziel,
				 mbx, absender1, lifetime1, bulletin_id,
				 betreff1, &no_sf);
      else
	ergebnis = 0;
    } else
      ergebnis = 0;


    if (ergebnis == 0 && convtit && (!dbimport || dbimpfilter)) {
      /*local?*/
      do_convtit(frombox, mtyp, ziel, mbx, absender1, lifetime1, betreff1,
		 bulletin_id, size1, usersf || (fw1 && *frombox == '\0'),
		 authentisch, &reject_it, &hold_it);
    }

    if (!strcmp(ziel, ziel2))
      switch_to_default_board(ziel);
    if (reject_it)
      ergebnis = 1;

    /* ----------------------------------------------------------------- */
    if (ergebnis == 0) {

      if (ana_hpath && !usersf && !direct_sf) {
	if (strcmp(rcall, "-moni-")) {
	  if (*frombox != '\0' || !fw1) {
	    strcpy(hs, frombox);

	    if (part) {
	      /* So grosses Filefragment wie moeglich einlesen und dann abscannen   */
	      sfbread(true, dname, &puffer, &psize);
	      if (psize > 0) {
		loopct = scan_hierarchicals(hs,
		    (char *)(&puffer[lesezeiger]), psize - lesezeiger,
		    &txdate1, in_real_sf(hs) && mtyp != 'A' && strcmp(header.dest, "WP"),
		    mtyp, lastvias);
		free(puffer);
	      }
	    } else
	      /* gesamtes File scannen                                             */
	      loopct = scan_hierarchicals(hs, puffer, size1, &txdate1,
					  in_real_sf(hs) && mtyp != 'A'
					  && strcmp(header.dest, "WP"),
					  mtyp, lastvias);

	    if (hmbxiscall)   /* kommt sonst kein "Routing Error" */
	      *lastvias = '\0';

	  }
	}
      }

      /* --------------------------------------------------------------------- */

      if (loopct > 0 && fw1) {  /* nur stoppen, wenn im S&F */
	if (!hmbxiscall)
	      /* einfach loeschen ! doppelte Bulletins hat es nicht zu geben */
		return;
	if (loopct >= 3) {
	  no_sf = true;   /* sf stoppen. geht augenscheinlich schief */
	  looperr = true;
	}
      } else if (!hmbxiscall && txdate1 != 0 &&
		 clock_.ixtime - txdate1 > bullsfmaxage) {
	if (!dbimport && !charge_bbs) {
	  outdated = true;
	  no_sf = true;
	}
      }

      /* ---------------------------------------------------------------------- */

      if (private_dirtycheck || mtyp != 'P') {
        strcpyupper(hs, betreff1);
        if (check_for_dirty(hs)) {
	  strcpy(dirtystring, betreff1);
	  dirtys = true;
        }
      }

      /* ---------------------------------------------------------------------- */

      mtyp7ext = mtyp;
	  /* Keine Mails an privilegierte Rubriken extrahieren */
      if (mtyp7ext == 'B') {
	acc = 1;
	check_lt_acc(ziel, &lth, &acc);
	if (acc > 1 || ziel[1] == '\0') /* Einbuchstabenrubrik */
	  mtyp7ext = 'C';
      }

      if (part) {   /* und Test auf #BIN#-File */
	debug(2, unr, 26, "part");

	/* den ersten und den letzten Block des Files einlesen und scannen   */

	puffer = malloc(blocksize);
	if (puffer != NULL) {
	  k1 = sfopen(dname, FO_READ);
	  if (k1 >= minhandle) {
	    sv = lesezeiger;
	    psize = size1 - sv;
	    if (psize > blocksize)
	      psize = blocksize;
	    sfseek(sv, k1, SFSEEKSET);
	    psize = sfread(k1, psize, puffer);

	    /* scan nach BIN / 7PLUS */

	    ack_requested = scan_for_ack(puffer, psize, wpupdate, wprotupdate, part,
		absender1, header.dest, betreff1, mbx, bulletin_id, mtyp7ext, ackcall,
		&is_binary, &dirty, &is_html, dirtystring, &is_7plus, &is_broken);

	    if (sv + psize < size1) {
	      sv = size1 - blocksize;
	      if (sv < lesezeiger)
		sv = lesezeiger;
	      psize = size1 - sv;
	      sfseek(sv, k1, SFSEEKSET);
	      psize = sfread(k1, psize, puffer);

	      ack_requested = scan_for_ack(puffer, psize, wpupdate, wprotupdate, part,
		  absender1, header.dest, betreff1, mbx, bulletin_id, mtyp7ext, hs, &db, &dirty,
		  &is_html, dirtystring, &db, &db);
	    }
	    sfclose(&k1);
	  }
	  free(puffer);
	}


      } else

      /* das ganze File ist im RAM und wird in einem Rutsch gescannt (der Normalfall) */
	ack_requested = scan_for_ack(puffer, size1, wpupdate, wprotupdate, part, absender1,
	    header.dest, betreff1, mbx, bulletin_id, mtyp7ext, ackcall, &is_binary, &dirty,
	    &is_html, dirtystring, &is_7plus, &is_broken);

      if (!(((hmbxiscall && !strcmp(hmbx, Console_call)) || unr == -77) &&
	    fw1 && strstr(betreff1, "CP ") != betreff1))
	ack_requested = false;

      if (kill_wp_immediately && (wpupdate || wprotupdate)) return; /* bye then... */

      if ((dirtys || dirty || is_broken || !strcmp(ziel, "D")) && strcmp(absender1, Console_call)) {
	strcpy(ziel, "D");
	dirty = true;
	no_sf = true;
      } else
	dirty = false;

      /* ------------------------------------------------------------------- */

      sprintf(info, "%s%s%c%s", infodir, ziel, extsep, EXT_INF);
      sprintf(index, "%s%s%c%s", indexdir, ziel, extsep, EXT_IDX);
      if (sfsize(index) / sizeof(indexstruct) >= SHORT_MAX - 1) {
	sprintf(STR13, "board %s full. No more input possible.", ziel);
	debug(0, unr, 26, STR13);
	return;
      }
      sprintf(archiv, "%sDPINFO%cXXXXXX", tempdir, extsep);
      mymktemp(archiv);
      sprintf(temp1, "%sDPINFO%cXXXXXX", tempdir, extsep);
      mymktemp(temp1);
      sprintf(temp2, "%sDPINFO%cXXXXXX", tempdir, extsep);
      mymktemp(temp2);

      if ((strcmp(rcall, Console_call) && !sfcut && !broadcast) || usersf) {
	if (usersf) {
	  sprintf(hs, "X-Info: User S&F received from %s at %s\n",
		  frombox, Console_call);
	  if (!authentisch && authentinfo) {
	    sig_german_authinfo(frombox, mbx, hs2);
	    sprintf(hs + strlen(hs), "%s\n", hs2);
	  }
	} else {
	  strsub(hs, datum, 7, 2);
	  strsub(hs2, datum, 4, 2);
	  strcat(hs, hs2);
	  sprintf(hs2, "%.2s", datum);
	  sprintf(hs + strlen(hs), "%s/", hs2);
	  sprintf(hs2, "%.2s", zeit);
	  strcat(hs, hs2);
	  strsub(hs2, zeit, 4, 2);
	  sprintf(hs + strlen(hs), "%sz @:%s", hs2, rcall);
	  if (!strcmp(rcall, "-moni-"))
	    sprintf(hs, "I:%s [Monitor/Import at %s]",
		    strcpy(STR13, hs), Console_call);
	  else
	    sprintf(hs, "R:%s", strcpy(STR13, hs));
	}
      } else
	*hs = '\0';

      /* --------------------------------------------------------------------- */

      if (compress) {
	if (part) {
	  /* Leider liegt sie nicht komplett im RAM vor ...            */

	  puffer = malloc(blocksize);
	  if (puffer != NULL) {
	    k2 = sfcreate(temp1, FC_FILE);
	    if (k2 >= minhandle) {
	      k1 = sfopen(dname, FO_READ);
	      if (k1 >= minhandle) {
		if (strlen(hs) >= '\0')
		  str2file(&k2, hs, true);

		hsize = sfseek(lesezeiger, k1, SFSEEKSET);
		rs = size1 - hsize;
		ok = true;
		while (rs > 0 && ok) {
		  if (rs > blocksize)
		    rs = blocksize;
		  bct = sfread(k1, rs, puffer);
		  if (bct <= 0) {
		    debug(0, unr, 26, "read error (1)");
		    no_sf = true;
		    ok = false;
		    continue;
		  }

		  if (sfwrite(k2, bct, puffer) == bct) {
		    hsize += bct;
		    rs = size1 - hsize;
		  } else {
		    debug(0, unr, 26, "write error (1)");
		    no_sf = true;
		    ok = false;
		  }
		}

		sfclose(&k1);
		sfclose(&k2);

		size1 -= lesezeiger;
		lesezeiger = 0;
		strcpy(dname, temp1);

		header.size = size1;

		ugzip = use_gzip(size1);
		if (packer(ugzip, false, true, dname, temp2, false) == 0) {
		  sfdelfile(dname);
		  strcpy(dname, temp2);
		  if (ugzip) header.pmode = PM_GZIP;
		  else header.pmode = PM_HUFF2;
		} else
		  header.pmode = PM_NONE;

		header.packsize = sfsize(dname);
		size1 = header.packsize;

	      } else
		sfclose(&k2);


	    }
	    free(puffer);
	  } else
	    boxprotokoll("no mem");


	} else {
	  /* Datei komplett im RAM, dann passt auch das Ergebnis ins RAM   */

	  if (*hs != '\0') {
	    bct = size1;
	    add_line_to_buff(&puffer, &bct, 0, hs);
	    size1 = bct;
	  }

	  ugzip = use_gzip(size1);
	  packresult = mempacker(ugzip, true, puffer, size1, &nmem, &nsize,
				     archiv, false);
	  header.size = size1;
	  if (packresult != 0) {
	    boxprotokoll(TXTNOMEM);
	    header.packsize = size1;
	    header.pmode = PM_NONE;
	  } else {
	    free(puffer);
	    lesezeiger = 0;
	    if (nmem != NULL) {
	      puffer = nmem;
	      header.packsize = nsize;
	    } else {
	      puffer = malloc(sfsize(archiv));
	      if (puffer != NULL) {
		k4 = sfopen(archiv, FO_READ);
		header.packsize = sfread(k4, sfsize(archiv), puffer);
		sfclose(&k4);
	      } else {
		header.packsize = 0;
	      }
	      sfdelfile(archiv);
	    }

	    if (ugzip) header.pmode = PM_GZIP;
	    else header.pmode = PM_HUFF2;
	  }
	}

      } else
	header.packsize = size1 - lesezeiger;

      /* -------------------------------------------------------------------- */

      if (header.packsize > 0) {
	if (exist(info)) {
	  k1 = sfopen(info, FO_RW);
	  k2 = sfopen(index, FO_RW);
	} else {
	  k1 = sfcreate(info, FC_FILE);
	  k2 = sfcreate(index, FC_FILE);
	}

	if (k1 >= minhandle && k2 >= minhandle) {
	  lastpos1 = sfseek(0, k1, SFSEEKEND);
	  lastpos2 = sfseek(0, k2, SFSEEKEND);

	  if (!compress) {
	    header.pmode = PM_NONE;

	    if (*hs != '\0') {
	      checksum16_buf(hs, strlen(hs), &ics);
	      checksum16(10, &ics);
	      str2file(&k1, hs, true);
	    }
	  }

	  if (part) {

	    puffer = malloc(blocksize);
	    if (puffer != NULL) {
	      k = sfopen(dname, FO_READ);
	      if (k >= minhandle) {
		hsize = sfseek(lesezeiger, k, SFSEEKSET);
		rs = size1 - hsize;
		ok = true;
		while (rs > 0 && ok) {
		  if (rs > blocksize)
		    rs = blocksize;
		  bct = sfread(k, rs, puffer);
		  if (bct <= 0) {
		    debug(0, unr, 26, "read error (2)");
		    no_sf = true;
		    ok = false;
		    continue;
		  }
		  checksum16_buf(puffer, bct, &ics);
		  if (sfwrite(k1, bct, puffer) == bct) {
		    hsize += bct;
		    rs = size1 - hsize;
		  } else {
		    debug(0, unr, 26, "write error (2)");
		    no_sf = true;
		    ok = false;
		  }
		}

		sfclose(&k);

	      }
	      free(puffer);

	    } else
	      boxprotokoll("no mem");


	  } else {
	    if (puffer != NULL) {
	      checksum16_buf(puffer, header.packsize, &ics);
	      if (sfwrite(k1, header.packsize, puffer) != header.packsize) {
		debug(0, unr, 26, "write error (4)");
		no_sf = true;
	      }
	      free(puffer);
	    }
	  }



	  if (!compress) {
	    header.size = size1 - lesezeiger;
	    if (*hs != '\0')
	      header.size += strlen(hs) + 1;
	    header.packsize = header.size;
	  }

	  header.deleted = false;
	  header.msgnum = new_msgnum();
	  strcpy(header.id, bulletin_id);
	  strcpy(header.absender, absender1);
	  strcpy(header.verbreitung, mbx);
	  cut(betreff1, LEN_SUBJECT);
	  strcpy(header.betreff, betreff1);
	  if (dbimport)
	    header.rxdate = string2ixtime(datum, zeit);
	  else if (charge_bbs) {
	    if (chargedate > clock_.ixtime || chargedate == 0)
	      chargedate = clock_.ixtime;
	    header.rxdate = chargedate;
	  } else {
	    header.rxdate = messagerxtime();
	  }
	  header.txdate = txdate1;
	  header.rxqrg = boxaktqrg();
	  header.lifetime = atoi(lifetime1);
	  if (wprotupdate && (header.lifetime > 2 || header.lifetime == 0)) header.lifetime = 2;
	  if (wpupdate && (header.lifetime > 2 || header.lifetime == 0)) header.lifetime = 2;
	  if (outdated)
	    header.lifetime = 1;

	  if (reduce_lt_after_extract && (is_7plus || is_binary) && header.msgtype == 'B') {
	    li = bullsfmaxage / SECPERDAY;
	    if (li > 0) {
	      if (header.lifetime > li || header.lifetime == 0)
		header.lifetime = li;
	    }
	  }

	  header.start = lastpos1;
	  strcpy(header.rxfrom, frombox);
	  sfseek(header.start, k1, SFSEEKSET);
	  sfread(k1, 1, &header.firstbyte);
	  header.msgtype = mtyp;
	  header.fwdct = 0;
	  header.erasetime = 0;
	  header.lastread = 0;
	  header.readcount = 0;
	  *header.readby = '\0';
	  header.eraseby = 0;
	  strcpy(header.sendbbs, ackcall);

	  header.infochecksum = ics;
	  header.bcastchecksum = 0;

	  acc = 1;
	  check_lt_acc(ziel, &header.lifetime, &acc);
	  header.level = acc;

	  if (is_binary)
	    header.pmode |= TBIN;
	  else if (is_7plus)
	    header.pmode |= T7PLUS;
	  if (is_html)
	    header.pmode |= THTML;

	  if (broadcast)
	    header.msgflags = MSG_BROADCAST_RX;
	  else {
	    if (fw1) {
	      if (!usersf && *header.rxfrom != '\0')
		header.msgflags = MSG_SFRX;
	      else {
		header.msgflags = MSG_MINE;
		strcpy(header.sendbbs, Console_call);
		header.txdate = header.rxdate;
	      }
	    } else
	      header.msgflags = MSG_CUT;
	  }

	  if (hold_it && strcmp(header.absender, Console_call)) {
	    if (header.msgflags == MSG_MINE)
	      header.msgflags |= MSG_LOCALHOLD;
	    else
	      header.msgflags |= MSG_SFHOLD;
	  }

	  if (outdated)
	    header.msgflags |= MSG_OUTDATED;

	  if ((header.msgflags & MSG_SFRX) != 0) {
	    nr = actual_user(frombox);
	    if (nr > 0) {
	      switch (header.msgtype) {

	      case 'A':
	      case 'P':
		user[nr]->fstat_rx_p++;
		break;

	      default:
		user[nr]->fstat_rx_b++;
		break;
	      }
	    }
	  }

/*	  ack_requested = ((header.msgflags & MSG_SFRX) != 0 && ack_requested); */ /* why only in SFRX ? */

	  write_index(k2, -1, &header);
	  sfclose(&k2);
	  sfclose(&k1);

	  write_log_and_bid(ziel, lastpos2 / sizeof(indexstruct) + 1, header);

	  if (!dbimport && !charge_bbs && crawl_active && crawler_exists
	  	&& (header.msgtype == 'B' || crawl_private)
		&& ((header.pmode & (TBIN+T7PLUS)) == 0)
		&& strcmp("T", header.dest)
		&& strcmp("Y", header.dest)) {
				
	    if (!crawl_started) {
	      crawl_started = clock_.ixtime;
	      crawl_lastchecknum = sfsize(boxlog) / sizeof(boxlogstruct);
	      if (crawl_lastchecknum) crawl_lastchecknum--;
	    }
	    
	  }

	  if (!dbimport && !charge_bbs && forwarding) {
	    if (no_sf) {
	      if (looperr)
		fidx = -3;   /* das ist ein Flag */
	      else if (outdated)
		fidx = -5;   /* das auch */
	      else if (is_broken)
		fidx = -8;   /* das auch */
	      else if (dirty)
		fidx = -4;   /* das auch */
	      else
		fidx = -2;
	      if (is_broken || dirty) {
		sprintf(ziel + strlen(ziel), "(%s)", header.dest);
		*hs = '\0';
	      } else
		strcpy(hs, "fwd stopped for ");
	      sprintf(STR13, "%s%s@%s < %s $%s",
		      hs, ziel, header.verbreitung, header.absender,
		      header.id);
	      append_profile(fidx, STR13);
	    } else if (!dirty) {
	      fidx = sfsize(index) / sizeof(indexstruct);
	      *hs = '\0';
	      if (fidx > 0)
		set_forward(-1, unr, ziel, hs, fidx, fidx, "FORWARD", frombox,
			    lastvias);
	    }
	  }

	  if ((dirty || is_broken) && *dirtystring != '\0') {
	    if (is_broken)
	      append_profile(-8, dirtystring);
	    else
	      append_profile(-4, dirtystring);
	  }

	  /* Achtung, neue Nachricht fuer Dich ... */
	  if (!dbimport && callsign(ziel) && !dirty && !charge_bbs &&
	      strcmp(header.absender, ziel)) {
	    nr = actual_user(ziel);
	    if (nr > 0) {
	      if (!user[nr]->f_bbs) {
		if (user[nr]->action == 0 && user[nr]->lastprocnumber == 0) {
		  fidx = sfsize(index) / sizeof(indexstruct);
		  wlnuser(nr, "");
		  wlnuser(nr, "");
		  wln_btext(nr, 10);
		  *hs = '\0';
		  wlnuser0(nr);
		  list_brett(nr, ziel, fidx, fidx, 100, hs, hs);
		  show_prompt(nr);
		} else {
		  msgwuser(nr, false, "", true);
		  msgwuser(nr, false, "", true);
		  get_btext(nr, 10, hs);
		  msgwuser(nr, false, hs, true);
		}
	      }
	    }
	  }

	  if (create_acks && ack_requested && !dbimport && !dirty &&
	      !charge_bbs) {
	    if (unr == -77)
	      strcpy(hs, header.verbreitung);
	    else
	      *hs = '\0';
	    create_ack(header.absender, ackcall, header.dest, hs,
		       header.betreff);
	  }

	}  /* of k1 / k2 >= minhandle */
	else {
	  sfclose(&k1);
	  sfclose(&k2);
	  sprintf(hs, "file open error: %s", info);
	  debug(0, unr, 26, hs);
	}

      }

    }  /* of ergebnis = 0 */

  }  /* of not diebox_sys */

  else if (dieboxsys && !dbimport && !charge_bbs && check_double(bulletin_id)) {
    if (forwarding && !no_sf) {
      nr = actual_user(frombox);
      if (nr > 0) user[nr]->fstat_rx_s++;
      snprintf(hs, 200, "%c @ %s < %s $%s %s", upcase_(ziel[0]), mbx, absender1, bulletin_id, betreff1);
      sf_rx_emt(nr, hs);
    }
  }

  debug(5, unr, 26, "...done");
}

#undef blocksize


static void get_fbbdatime(char *zs, long *date)
{
  char	    d, m, y, h, min, s;
  short     x;
  char	    w[256], dw[21];

  upper(zs);
  x = strpos2(zs, ":", 1);
  if (x > 0)
    strdelete(zs, 1, x);
  get_word(zs, w);
  d = clock_.day;
  m = clock_.mon;
  y = clock_.year;
  x = strpos2(w, "-", 1);
  if (x > 0) {
    sprintf(dw, "%.*s", x - 1, w);
    d = atoi(dw);
    strdelete(w, 1, x);
    cut(w, 3);
    x = 0;
    do {
      x++;
    } while (x != 12 && strcmp(fbb_month[x - 1], w));
    m = x;
  }
  get_word(zs, w);
  h = clock_.hour;
  min = clock_.min;
  s = 0;
  x = strpos2(w, ":", 1);
  if (x > 0) {
    sprintf(dw, "%.*s", x - 1, w);
    h = atoi(dw);
    strdelete(w, 1, x);
    min = atoi(dw);
  }
  *date = calc_ixtime(d, m, y, h, min, s);
}


static void transform_boxheader(cutboxtyp cuttyp, char *status, char *z1,
  char *z2, char *z3, char *z4, char *z5, char *z6)
{
  indexstruct	header;
  short		x;
  boolean	nb;
  char		brett[256], dw[256], z0[256], w[256];

  debug(3, 0, 27, status);
  strcpy(z0, status);
  status[0]		= '\0';
  header.lifetime	= 0;
  header.msgtype	= '\0';
  header.size		= 999000L;
  strcpy(header.verbreitung, Console_call);

  switch (cuttyp) {

  case F6FBB_USER_514:
    strdelete(z0, 1, strlen(fbb514_fromfield));
    get_word(z0, header.absender);
    strdelete(z1, 1, strlen(fbb514_tofield));
    get_word(z1, w);
    x = strpos2(w, "@", 1);
    if (x > 0) {
      strsub(header.verbreitung, w, x + 1, strlen(w) - x);
      cut(w, x - 1);
      strcpy(brett, w);
    } else {
      strcpy(brett, w);
      if (strpos2(z1, "@", 1) == 1) {
	strdelete(z1, 1, 1);
	get_word(z1, header.verbreitung);
      }
    }

    x = strpos2(z6, ":", 1);
    if (x > 0)
      strdelete(z6, 1, x);
    del_leadblanks(z6);
    cut(z6, LEN_SUBJECT);
    strcpy(header.betreff, z6);

    get_fbbdatime(z3, &header.rxdate);

    x = strpos2(z4, ":", 1);
    if (x > 0)
      strdelete(z4, 1, x);
    del_leadblanks(z4);
    cut(z4, LEN_BID);
    strcpy(header.id, z4);
    create_status(0, false, brett, header, status);
    strcpy(z1, header.betreff);
    sprintf(z2, "* ID %s *", header.id);

    break;

  case F6FBB_USER:
    strdelete(z0, 1, strlen(fbb_fromfield));
    get_word(z0, header.absender);
    strdelete(z0, 1, strlen(fbb_tofield));
    get_word(z0, w);
    x = strpos2(w, "@", 1);
    if (x > 0) {
      strsub(header.verbreitung, w, x + 1, strlen(w) - x);
      cut(w, x - 1);
      strcpy(brett, w);
    } else {
      strcpy(brett, w);
      if (strpos2(z0, "@", 1) == 1) {
	strdelete(z0, 1, 1);

	get_word(z0, header.verbreitung);
      }
    }
    x = strpos2(z4, ":", 1);
    if (x > 0)
      strdelete(z4, 1, x);
    del_leadblanks(z4);
    cut(z4, LEN_SUBJECT);
    strcpy(header.betreff, z4);
    get_fbbdatime(z2, &header.rxdate);
    ixzeit2string(header.rxdate, dw);
    ixdatum2string(header.rxdate, w);
    create_userid(brett, header.absender, w, dw, header.betreff, header.id);
    create_status(0, false, brett, header, status);
    strcpy(z1, header.betreff);
    sprintf(z2, "* ID %s *", header.id);
    break;

  case AA4RE_USER:
    z1[0] = '\0';
    z2[0] = '\0';
    break;

  case W0RLI_USER:
    z1[0] = '\0';
    z2[0] = '\0';
    break;

  case RAW_IMPORT:
    if (strstr(z1, "From:") == z1)
      strdelete(z1, 1, 5);
    del_leadblanks(z1);
    upper(z1);
    if (callsign(z1)) {
      strcpy(header.absender, z1);
      if (strstr(z2, "To:") == z2)
        strdelete(z2, 1, 3);
      del_leadblanks(z2);
      upper(z2);
      if (valid_boardname(z2)) {
	strcpy(header.dest, z2);
      	if (strstr(z3, "MBX:") == z3)
	  strdelete(z3, 1, 4);
      	del_leadblanks(z3);
	upper(z3);
	unhpath(z3, dw);
	nb = false;
	if (*dw == '\0' || !strcmp(dw, Console_call)) {
	  if (callsign(header.dest)) {
	    user_mybbs(header.dest, z3);
	    if (*z3 == '\0')
	      strcpy(z3, ownhiername);
	    else {
	      unhpath(z3, dw);
	      if (strcmp(dw, Console_call))
		nb = true;
	    }
	  } else
	    strcpy(z3, ownhiername);
	}


	if (strchr(z3, '.') == NULL && callsign(z3))
	  complete_hierarchical_adress(z3);
	strcpy(header.verbreitung, z3);
	
      	if (strstr(z4, "Lifetime:") == z4)
	  strdelete(z4, 1, 9);
      	else if (strstr(z4, "LT:") == z4)
	  strdelete(z4, 1, 9);
      	del_leadblanks(z4);
	header.lifetime = atoi(z4);

      	if (strstr(z5, "BID:") == z5)
	  strdelete(z5, 1, 4);
      	del_leadblanks(z5);
	if (strlen(z5) <= LEN_BID) {
	  strcpy(header.id, z5);
	  if (*header.id == '\0')
	    new_bid(header.id);
	  else if (nb) {
	    write_msgid(-1, header.id);
	    new_bid(header.id);
	  }

	  if (strstr(z6, "Subject:") == z6)
	    strdelete(z6, 1, 8);
	  del_leadblanks(z6);
	  cut(z6, LEN_SUBJECT);
	  strcpy(header.betreff, z6);

	  header.rxdate = clock_.ixtime;

	  create_status(0, true, header.dest, header, status);

	  strcpy(z1, header.betreff);
	  sprintf(z2, "* ID %s *", header.id);

	}
      }
    }
    break;

  default:
    break;
  }
}


#define blocksize       16384

static boolean sort_new_mail3(short unr, char *pattern_, char *rcall1_)
{
  short		result, k;
  long		bct, lesezeiger, hlz, err, dsize;
  boolean	Result, imperr, origin, sfcut, okb, oka, had_no_bid;
  boolean	take_double, broadcast, authentisch, mycall_in_rlines;
  cutboxtyp	cuttyp;
  char		*puffer;
  pathstr	dname;
  char		frombox[256], hpath[256], rcall[256], betreff[256], STR1[256];
  char		datum[256], zeit[256], laenge[256], lifetime[256];
  char		pattern[256], rcall1[256], bulletin_id[256], zeile[256], ds[256];
  char		z1[256], z2[256], z3[256], z4[256], z5[256], z6[256], status[256];
  boardtype	ziel;
  calltype	absender;
  mbxtype	mbx;
  DTA		dirinfo;

  debug(2, unr, 28, pattern_);

  strcpy(pattern, pattern_);
  strcpy(rcall1, rcall1_);
  Result = false;

  puffer = malloc(blocksize);
  if (puffer == NULL)
    return Result;

  boxsetbmouse();
  strcpy(hpath, pattern);
  result = sffirst(pattern, 0, &dirinfo);

  if (result == 0 && dirinfo.d_length > 0) {
    Result = true;
    get_path(hpath);
    sprintf(dname, "%s%s", hpath, dirinfo.d_fname);

    debug(3, unr, 28, dname);
    strcpy(frombox, dname);
    broadcast = false;
    del_path(frombox);
    authentisch = true;
    if (frombox[0] == '&') {
      strdelete(frombox, 1, 1);
      del_ext(frombox);
    } else if (frombox[0] == '%') {
      strdelete(frombox, 1, 1);
      del_ext(frombox);
      authentisch = false;
    } else if (frombox[0] == '$') {
      strdelete(frombox, 1, 1);
      del_ext(frombox);
      broadcast = true;
    } else
      *frombox = '\0';
    if (frombox[0] == '_') {
      strdelete(frombox, 1, 1);
      had_no_bid = true;
    } else
      had_no_bid = false;
    cut(frombox, LEN_CALL);
    if (!callsign(frombox))
      *frombox = '\0';

    strcpyupper(ds, dname);
    del_path(ds);
    if (ds[0] == '%' || ds[0] == '&')
      strdelete(ds, 1, 1);
    if (ds[0] == '_')
      strdelete(ds, 1, 1);
    strcpy(rcall, rcall1);
    cut(rcall, LEN_CALL);
    sprintf(STR1, "SENDING%c", extsep);
    origin = ((strstr(ds, STR1) == ds ||
	       strstr(ds, (sprintf(STR1, "IMPORT%c", extsep), STR1)) == ds) &&
	      !strcmp(rcall, Console_call));
    sprintf(STR1, "SFBOX%c", extsep);
    sfcut = (strstr(ds, STR1) == ds);
    take_double = (strstr(ds, "BOXCUT") == ds);

    dsize = sfsize(dname);
    k = sfopen(dname, FO_READ);
    if (k >= minhandle) {
      bct = sfread(k, blocksize, puffer);
      sfclose(&k);
      *status = '\0';
      *betreff = '\0';
      imperr = false;
      lesezeiger = 0;
      do {
	get_line(puffer, &lesezeiger, bct, status);
      } while (*status == '\0' && lesezeiger < bct);

      if (strstr(status, DPREADOUTMAGIC) == status) {
	get_word(status, zeile);
	get_word(status, rcall);
	cut(rcall, LEN_CALL);
	*status = '\0';
	do {
	  get_line(puffer, &lesezeiger, bct, status);
	} while (*status == '\0' && lesezeiger < bct);
      }
      *bulletin_id = '\0';
      del_lastblanks(status);
      if (count_words(status) == 1 && callsign(status)) {
	cuttyp = RAW_IMPORT;
	strcpyupper(frombox, status);
      } else if (sfcut)
	cuttyp = W0RLI_SF;
      else
	cuttyp = boxheader(status);


      if (((1L << ((long)cuttyp)) & ((1L << ((long)W0RLI_USER)) |
	     (1L << ((long)AA4RE_USER)) | (1L << ((long)F6FBB_USER)) |
	     (1L << ((long)F6FBB_USER_514)) | (1L << ((long)RAW_IMPORT)))) != 0) {
	hlz = lesezeiger;
	get_line(puffer, &lesezeiger, bct, z1);
	get_line(puffer, &lesezeiger, bct, z2);
	get_line(puffer, &lesezeiger, bct, z3);
	get_line(puffer, &lesezeiger, bct, z4);
	if (((1L << ((long)cuttyp)) & ((1L << ((long)F6FBB_USER_514)) |
				       (1L << ((long)RAW_IMPORT)))) != 0) {
	  get_line(puffer, &lesezeiger, bct, z5);
	  get_line(puffer, &lesezeiger, bct, z6);
	} else {
	  *z5 = '\0';
	  *z6 = '\0';
	}
	del_lastblanks(z1);
	del_lastblanks(z2);
	del_lastblanks(z3);
	del_lastblanks(z4);
	del_lastblanks(z5);
	del_lastblanks(z6);
	transform_boxheader(cuttyp, status, z1, z2, z3, z4, z5, z6);
	if (*status != '\0') {
	  err = lesezeiger - strlen(z1) - strlen(z2) - 2;
	  if (err >= hlz) {
	    lesezeiger = err;
	    put_line(puffer, &err, z1);
	    put_line(puffer, &err, z2);
	    cuttyp = THEBOX_USER;
	  } else {
	    imperr = true;
	    if (cuttyp == RAW_IMPORT) {
	      debug(0, unr, 28,
		    "header conversion failed, import file deleted");
	    }
	  }
	} else
	  imperr = true;

      }

      if (!imperr) {
	if (((1L << ((long)cuttyp)) &
	     ((1L << ((long)THEBOX_USER)) | (1L << ((long)WAMPES_USER)) |
	      (1L << ((long)W0RLI_SF)) | (1L << ((long)NOP)))) != 0 ||
	    origin || sfcut) {
	  if (cuttyp == W0RLI_SF)
	    get_word(status, zeile);
	  get_line(puffer, &lesezeiger, bct, betreff);
	  if (cuttyp == WAMPES_USER)
	    strdelete(betreff, 1, 9);
	  hlz = lesezeiger;
	  get_line(puffer, &hlz, bct, zeile);
	  if (!sfcut) {
	    if (cuttyp == WAMPES_USER) {
	      if (count_words(zeile) == 3) {
		get_word(zeile, ds);
		get_word(zeile, ds);
		if (!strcmp(ds, "ID:")) {
		  get_word(zeile, bulletin_id);
		  lesezeiger = hlz;
		}
	      }
	    } else {
	      if (count_words(zeile) == 4) {
		get_word(zeile, ds);
		get_word(zeile, ds);
		if (strstr(ds, "ID") != NULL) {
		  get_word(zeile, bulletin_id);
		  lesezeiger = hlz;
		}
	      }
	      get_line(puffer, &hlz, bct, zeile);
	      if (strstr(zeile, "*** Received from") == zeile)
		lesezeiger = hlz;
	    }
	  }
	} else if (cuttyp == BAYCOM_USER) {
	  get_line(puffer, &lesezeiger, bct, zeile);
	  if (strstr(zeile, "BID :") == zeile) {
	    strdelete(zeile, 1, 6);
	    get_word(zeile, bulletin_id);
	    get_line(puffer, &lesezeiger, bct, zeile);
	  }
	  while (strstr(zeile, "X-Flags:") == zeile)
	    get_line(puffer, &lesezeiger, bct, zeile);
	  while (strstr(zeile, "Read:") == zeile)
	    get_line(puffer, &lesezeiger, bct, zeile);
	  if (strstr(zeile, "Subj:") == zeile) {
	    strcpy(betreff, zeile);
	    strdelete(betreff, 1, 6);
	  }
	}


	if (sfcut)
	  split_sline(status, ziel, absender, mbx, bulletin_id, lifetime,
		      zeile);
	else
	  separate_status(status, ziel, absender, mbx, datum, zeit, laenge,
			  lifetime);

	cut(betreff, LEN_SUBJECT);
	if (*betreff != '\0') {
	  if (*bulletin_id == '\0' && !sfcut)
	    create_userid(ziel, absender, datum, zeit, betreff, bulletin_id);

	  /* Wenn schonmal eingelesen, dann das alte File loeschen */
	  /* wenn es nicht aus dem SF kam                          */

	  if (take_double)
	    take_double = erase_by_bid(true, bulletin_id, Console_call);
	  else
	    take_double = check_double(bulletin_id);

	  if (take_double) {
	    hlz = dsize;

	    if (!sfcut) {
	      do {
		err = lesezeiger;
		get_line(puffer, &lesezeiger, bct, zeile);
		    /* Leerzeilen am Anfang ueberlesen */
		if (*zeile != '\0')
		  lesezeiger = err;
	      } while (*zeile == '\0' && lesezeiger < bct);
	    }

	    if (had_no_bid) {
	      mycall_in_rlines = false;
	      strcpy(z2, " @:");
	      strcat(z2, ownhiername);
	      strcat(z2, " ");
	      err = lesezeiger;
	      do {
		get_line(puffer, &err, bct, zeile);
		oka = (strstr(zeile, "R:") == zeile);
		if (oka) {
		  if (strstr(zeile, z2) != NULL)
		    mycall_in_rlines = true;
		  k = strpos2(zeile, " $:", 1);
		}
		okb = (oka && k > 1);
		if (okb) {
		  strdelete(zeile, 1, k + 2);
		  get_word(zeile, z1);
		  if (strlen(z1) <= LEN_BID) {
		    if (!check_double(z1)) { /*schon bekannt?*/
		      if (!mycall_in_rlines) {
		        do {
		          get_line(puffer, &err, bct, zeile);
			  oka = (strstr(zeile, "R:") == zeile);
			  if (oka) {
		  	    if (strstr(zeile, z2) != NULL)
		    	      mycall_in_rlines = true;
		          }
		        } while (err < bct && oka && !mycall_in_rlines);
		      }
		      take_double = mycall_in_rlines; /* passiert bei loops */
		    } else {
		      strcpy(bulletin_id, z1);
		    }
		  } else
		    okb = false;
		}
	      } while (err < bct && oka && !okb);
	    }

	    if (take_double) {
	      new_entry(unr, dname, status, betreff, bulletin_id, hlz,
			lesezeiger, rcall, frombox, origin, sfcut,
			authentisch, broadcast);
	    } else
	      boxprotokoll("double (recovered message ID)...");

	  } else
	    boxprotokoll("double...");

	} else
	  debug(0, unr, 28, "corrupted header (no subject)...");
	

      } else
	debug(0, unr, 28, "corrupted header...");

    }
    sfdelfile(dname);
    if (strchr(pattern, allquant) == NULL)
      Result = false;
  }
  boxsetamouse();
  free(puffer);
  return Result;
}

#undef blocksize


void sort_new_mail4(void)
{
  long		ticks;
  newmailtype	*hp;

  ticks		= get_cpuusage();
  while (newmailroot != NULL && get_cpuusage() - ticks < ttask * 2) {
    if (!sort_new_mail3(newmailroot->unr, newmailroot->pattern,
			newmailroot->rcall)) {
      hp	= newmailroot;
      newmailroot = newmailroot->next;
      free(hp);
    }
  }
}


void sort_new_mail2(short unr, char *pattern, char *rcall1)
{
  newmailtype	*newp, *hp;

  if (unr >= 0) {
    debug(0, -1, 28, "error: unr >= 0");
  }

  /* vorsicht bei unr = -99 / -97 (= diebox/baybox-import) */
  /* unr = -55 -> immediate, z.B. fuer boxcut */

  if (unr == -99 || unr == -97) {
    while (sort_new_mail3(unr, pattern, rcall1)) ;
    return;
  }
  if (unr == -55) {
    while (sort_new_mail3(-1, pattern, rcall1)) ;
    return;
  }
  newp		= malloc(sizeof(newmailtype));

  if (newp == NULL) {
    while (sort_new_mail3(unr, pattern, rcall1)) ;
    return;
  }
  newp->next	= NULL;
  newp->unr	= unr;
  strcpy(newp->pattern, pattern);
  strcpy(newp->rcall, rcall1);
  if (newmailroot == NULL) {
    newmailroot	= newp;
    return;
  }
  hp		= newmailroot;
  while (hp->next != NULL)
    hp		= hp->next;
  hp->next	= newp;
}


void _box_file_init(void)
{
  static int _was_initialized = 0;
  if (_was_initialized++)
    return;

  remote_erase	= true;
  with_rline	= true;
  maxbullids	= 0;
  shall_sem	= 0;
  mfilter_sem	= 0;
  ufcr		= 0;
  ufchit	= 0;
  ufcmiss	= 0;
  lastxreroute	= 0;
}

