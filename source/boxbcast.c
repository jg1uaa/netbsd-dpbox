/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/

#define BOXBCAST_G
#include "boxbcast.h"
#include "tools.h"
#include "boxglobl.h"
#include "box_logs.h"
#include "yapp.h"
#include "boxlocal.h"
#include "box.h"
#include "box_file.h"
#include "main.h"


#define maxages         20

typedef long agearrtype[maxages];
typedef long logcttype[maxages];
typedef long fileidarrtype[maxages];
typedef char statinfarrtype[maxages][81];

typedef struct bcastdesctype {
  struct bcastdesctype	*next;
  char			tnc, port;
  boolean		UsePidF0;
  long			hfbaud;
  unsigned short	msgsel;
  char			level;
  unsigned short	filetype;
  unsigned short	timeslices;
  agearrtype		maxage, starttime;
  logcttype		firstlognr, lastlognr;
  fileidarrtype		lastfileid;
  statinfarrtype	statinf;
  char			qrg[LEN_TNTPORT+1];
  char			TxPath[121];
} bcastdesctype;


static time_t		lastbcastok	= 0;
static bcastdesctype	*bcastdesc	= NULL;
static boolean		boxbcastsemaphore= false;
static long 		bcasttfsize	= 0;
static long		bcasttfct	= 0;

static void increase_filecount(void)
{
  short	f;
  char	fn[256];
  char	w[256];

  sprintf(fn, "%sbcstat%cbox", boxstatdir, extsep);
  if (exist(fn)) {
    f	= sfopen(fn, FO_READ);
    if (f < minhandle)
      return;
    *w	= '\0';
    file2str(f, w);
    bcasttfsize	+= atol(w);
    *w	= '\0';
    file2str(f, w);
    bcasttfct	+= atol(w);
    sfclose(&f);
  }
  f	= sfcreate(fn, FC_FILE);
  if (f < minhandle)
    return;

  sprintf(w, "%ld", bcasttfsize);
  str2file(&f, w, true);
  sprintf(w, "%ld", bcasttfct);
  str2file(&f, w, true);
  sfclose(&f);
  bcasttfsize	= 0;
  bcasttfct	= 0;
}


void free_boxbcastdesc(void)
{
  bcastdesctype	*hp, *hp2;

  hp	= bcastdesc;

  while (hp != NULL) {
    hp2	= hp;
    hp	= hp->next;
    free(hp2);
  }

  bcastdesc		= NULL;
  boxbcastsemaphore	= true;
  increase_filecount();
}


static void add_bcastdesc(char *TxPath_, boolean UsePidF0, short tnc,
  short port, char *qrg, long hfbaud, unsigned short msgsel, char level,
  unsigned short filetype, unsigned short timeslices, long *ages)
{
  char			TxPath[256];
  bcastdesctype		*hp, *hp2;
  unsigned short	x;

  strcpy(TxPath, TxPath_);
  hp = malloc(sizeof(bcastdesctype));
  if (hp == NULL)
    return;

  if (timeslices > maxages)
    timeslices = maxages;
  else if (timeslices == 0)
    timeslices = 1;
  hp->timeslices = timeslices;
  memcpy(hp->maxage, ages, sizeof(agearrtype));
  for (x = timeslices; x < maxages; x++)
    hp->maxage[x] = 0;
  for (x = 0; x < maxages; x++)
    hp->starttime[x] = 0;
  for (x = 0; x < maxages; x++)
    hp->firstlognr[x] = 0;
  for (x = 0; x < maxages; x++)
    hp->lastlognr[x] = 0;
  for (x = 0; x < maxages; x++)
    hp->lastfileid[x] = 0;
  for (x = 0; x < maxages; x++)
    *hp->statinf[x] = '\0';
  hp->tnc = tnc;
  hp->port = port;
  hp->next = NULL;
  strcpy(hp->qrg, qrg);
  cut(TxPath, 120);
  strcpy(hp->TxPath, TxPath);
  hp->UsePidF0 = UsePidF0;
  hp->hfbaud = hfbaud;
  hp->msgsel = msgsel;
  hp->level = level;
  hp->filetype = filetype;

  if (bcastdesc == NULL) {
    bcastdesc = hp;
    return;
  }

  hp2 = bcastdesc;
  while (hp2->next != NULL)
    hp2 = hp2->next;
  hp2->next = hp;
}


void load_boxbcastparms(char *name)
{
  char			*puffer;
  long			psize;
  char			hs[256], w[256], w1[256];
  long			rp;
  boolean		startdef;
  char			TxPath[256];
  char			qrg[LEN_TNTPORT+1];
  boolean		UsePidF0;
  short			tnc, port;
  unsigned short	timeslices;
  long			hfbaud;
  unsigned short	msgsel, filetype;
  char			level;
  agearrtype		ages;

  debug(2, 0, 116, name);
  free_boxbcastdesc();

  lastbcastok	= clock_.ixtime;
  sfbread(true, name, &puffer, &psize);
  if (psize > 0) {
    UsePidF0	= true;
    *TxPath	= '\0';
    *qrg	= '\0';
    tnc		= 0;
    port	= 0;
    timeslices	= 0;
    level	= 1;
    filetype	= 0;
    ages[0]	= 1800;
    hfbaud	= 9600;
    msgsel	= MSG_BROADCAST;
    startdef	= false;

    rp		= 0;
    while (rp < psize) {
      get_line(puffer, &rp, psize, hs);
      del_comment(hs, '#');
      del_blanks(hs);

      if (*hs != '\0' && hs[0] == '{') {
	if (!startdef) {
	  startdef	= true;
	  UsePidF0	= true;
	  *TxPath	= '\0';
	  *qrg		= '\0';
	  tnc		= 0;
	  port		= 0;
	  timeslices	= 0;
	  level		= 1;
	  filetype	= 0;
	  ages[0]	= 1800;
	  hfbaud	= 9600;
	  msgsel	= MSG_BROADCAST;
	}
	strdelete(hs, 1, 1);
      }

      if (*hs != '\0' && hs[0] == '}') {
	if (startdef)
	  add_bcastdesc(TxPath, UsePidF0, tnc, port, qrg, hfbaud, msgsel,
			level, filetype, timeslices, ages);
	startdef	= false;
	strdelete(hs, 1, 1);
      }

      get_word(hs, w);
      upper(w);
      get_word(hs, w1);

      if (!strcmp(w, "TXPATH")) {
	sprintf(TxPath, "%s %s", w1, hs);
	del_blanks(TxPath);
	upper(TxPath);
	continue;
      }

      if (!strcmp(w, "QRG")) {
	cut(w1, LEN_TNTPORT);
	upper(w1);
	strcpy(qrg, w1);
	del_blanks(qrg);
	continue;
      }
      if (!strcmp(w, "PIDF0")) {
	UsePidF0	= (strcmp(w1, "0") != 0);
	continue;
      }
      if (!strcmp(w, "TNC")) {
	tnc		= atoi(w1);
	continue;
      }
      if (!strcmp(w, "PORT")) {
	port		= atoi(w1);
	continue;
      }
      if (!strcmp(w, "HFBAUD")) {
	hfbaud		= atol(w1);
	continue;
      }
      if (!strcmp(w, "MSGSEL")) {
	msgsel		= (unsigned short)atol(w1);
	continue;
      }
      if (!strcmp(w, "LEVEL")) {
	level		= atoi(w1);
	continue;
      }
      if (!strcmp(w, "FILETYPE"))
	filetype	= atoi(w1);
      else if (!strcmp(w, "SLICE")) {
	if (timeslices < maxages) {
	  timeslices++;
	  ages[timeslices - 1]	= atol(w1);
	}
      }
    }

    free(puffer);
  }

  boxbcastsemaphore	= false;
}


static boolean true_sel(unsigned short flags, unsigned short sel)
{
  if (sel != 0)
    return ((flags & sel) != 0);
  else
    return true;
}


static boolean get_nextvalidlog(unsigned short msgsel, char level, long *nr, boxlogstruct *log)
{
  long		logct;
  short		handle;

  debug0(4, 0, 117);

  log->deleted	= true;
  logct		= sfsize(boxlog) / sizeof(boxlogstruct);

  if (*nr > 1 && *nr <= logct) {
    handle	= sfopen(boxlog, FO_READ);
    if (handle >= minhandle) {
      while ((log->deleted || !true_sel(log->msgflags, msgsel) ||
	      strlen(log->brett) < '\002' || log->msgtype != 'B' ||
	      log->level > level) && *nr > 1) {
	(*nr)--;
	read_log(handle, *nr, log);
      }

      sfclose(&handle);
    }
  }

  if (*nr > 1)
    return true;
  else
    return false;
}


static long find_firstvalidlog(long *firstlognr, unsigned short msgsel,
			       char level, boxlogstruct *log)
{
  short handle;
  long nr;

  debug0(4, 0, 118);

  log->deleted	= true;
  nr		= sfsize(boxlog) / sizeof(boxlogstruct);
  if (nr > 0) {
    if (*firstlognr <= 0)
      *firstlognr	= nr;
    else if (*firstlognr > nr)
      *firstlognr	= nr;
    else
      nr		= *firstlognr;

    handle	= sfopen(boxlog, FO_READ);
    if (handle >= minhandle) {
      while ((log->deleted || !true_sel(log->msgflags, msgsel) ||
	      strlen(log->brett) < '\002' || log->msgtype != 'B' ||
	      log->level > level) && nr > 1) {
	read_log(handle, nr, log);
	nr--;
      }

      sfclose(&handle);
    }

  }


  if (nr > 1)
    return (nr + 1);
  else
    return 0;
}


static void send_lognr(unsigned short ct, bcastdesctype *hp, short nr,
		       boxlogstruct log, boolean rekursion)
{
  indexstruct		header;
  long			fid, h;
  time_t		exptime;
  unsigned short	bodychecksum;
  char			fheader[256], hs[256], bname[256];

  debug0(2, 0, 119);

  if (hp->lastlognr[nr - 1] <= 0)
    return;

  dp_watchdog(2, 4711);
  sprintf(hs, "%d", nr + ct * 50);
  sprintf(bname, "%sBCASTBOX%c%s", tempdir, extsep, hs);
  bodychecksum	= read_for_bcast(log.brett, log.idxnr, bname, &header);

  if (bodychecksum == ERR_READBOARD) {
    sfdelfile(bname);
    return;
  }

  if (header.deleted) {
    sfdelfile(bname);
    return;
  }

  fid	= (header.rxdate - header.size) & 0xfffffffL;
  h	= ct;
  h	&= 15;
  fid	+= h << 28;
  hp->lastfileid[nr - 1] = fid;
  if (header.txlifetime > 0)
    exptime = header.rxdate + header.txlifetime * SECPERDAY;
  else
    exptime = 0;

  fheader[0] = '\0';

  sprintf(hs, "slice:%d board:%s %d size:%ld %s",
		nr, log.brett, log.idxnr, header.size, header.betreff);
  cut(hs, 80);
  strcpy(hp->statinf[nr - 1], hs);
  if (debug_level > 1) boxprotokoll(hs);
  boxbcastsemaphore	= true;
  bcasttfsize		+= header.size;
  bcasttfct++;
  if (bcasttfct >= 25) increase_filecount();
  sprintf(hs, "%s @ %s", header.dest, header.verbreitung);
  bcast_file(hp->tnc, hp->port, hp->qrg, fid, hp->filetype, bname, hp->TxPath,
	     header.absender, hs, header.rxfrom, header.rxdate, exptime, 0,
	     header.id, header.msgtype, header.betreff, fheader, bodychecksum,
	     true);


}


static void bcastlist(bcastdesctype *hp, unsigned short x, unsigned short ct,
		      boolean rekursion)
{
  boxlogstruct	log;

  if (hp->starttime[x - 1] + hp->maxage[x - 1] > clock_.ixtime) {
    if (get_nextvalidlog(hp->msgsel, hp->level, &hp->lastlognr[x - 1], &log))
      send_lognr(ct, hp, x, log, rekursion);
    else {
      /* ansonsten eine Runde aussetzen */
      hp->starttime[x - 1] = 0;
    }
    return;
  }


  if (rekursion)
    return;

  if (hp->firstlognr[x - 1] <= 0 && x != 1)
    return;

  hp->starttime[x - 1] = clock_.ixtime;
  if (hp->lastlognr[x - 1] > 0)
    hp->firstlognr[x] = hp->lastlognr[x - 1] - 1;
  else
    hp->firstlognr[x] = 0;
  if (x == 1)   /* sonst werden neue Eintraege nie gesendet */
    hp->firstlognr[x - 1] = 0;
  hp->lastlognr[x - 1] = find_firstvalidlog(&hp->firstlognr[x - 1], hp->msgsel, hp->level, &log);
  send_lognr(ct, hp, x, log, rekursion);
}


void bccallback(long l)
{
  /* der TX teilt uns mit, dass das File mit FID l komplett gesendet worden ist	*/

  bcastdesctype		*hp;
  unsigned short	x, ct;

  lastbcastok	= clock_.ixtime;
  ct		= 0;
  hp		= bcastdesc;
  while (hp != NULL) {
    ct++;
    for (x = 1; x <= hp->timeslices; x++) {
      if (hp->lastfileid[x - 1] == l) {
	hp->lastfileid[x - 1] = 0;
	boxbcastsemaphore = false;
	if (send_bbsbcast)
	  bcastlist(hp, x, ct, true);
	return;
      }
    }
    hp		= hp->next;
  }
}


void boxbcasttxtimer(void)
{
  bcastdesctype		*hp;
  unsigned short	x, ct;

  debug0(4, 0, 120);

  hp		= bcastdesc;
  ct		= 0;
  while (hp != NULL) {
    ct++;
    for (x = 1; x <= hp->timeslices; x++) {
      if (hp->lastfileid[x - 1] == 0)
	bcastlist(hp, x, ct, false);
    }
    hp		= hp->next;
  }
  if (send_bbsbcast && bcastdesc != NULL && clock_.ixtime - lastbcastok > 1800) {
    load_boxbcastparms(bcast_box);
  } 
}


void show_bcastactivity(short unr, cbbproc outputproc)
{
  bcastdesctype	*hp;
  short		x;

  hp		= bcastdesc;
  while (hp != NULL) {
    for (x = 0; x < hp->timeslices; x++) {
      if (*hp->statinf[x] != '\0')
	outputproc(unr, hp->statinf[x]);
    }
    hp		= hp->next;
  }
}
