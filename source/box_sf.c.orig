/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#define BOX_SF_G
#include <ctype.h>
#include "main.h"
#include "box_sf.h"
#include "shell.h"
#include "tools.h"
#include "crc.h"
#include "box_logs.h"
#include "box.h"
#include "box_sub.h"
#include "box_sys.h"
#include "box_mem.h"
#include "box_rout.h"
#include "box_file.h"
#include "box_send.h"
#include "box_tim.h"
#include "box_inou.h"
#include "boxbcast.h"
#include "box_wp.h"
#include "box_scan.h"

static boolean gen_sftest3(short unr, char *frombox, char *board, char *mbx);



/* these functions implement a link speed measurement */

static void start_linkcheck(short unr, short prop)
{

  debug0(3, unr, 231);
  if (!boxrange(unr)) return;
  if (user[unr]->sfspeedtime == 0 || user[unr]->sfspeedprops >= prop) {
    user[unr]->sfspeedtime = statclock();
    user[unr]->sfspeedsize = user[unr]->sbytes;
    user[unr]->sfspeedprops = 0;
  }
  user[unr]->sfspeedprops = prop;
}

static void stop_linkcheck(short unr, short prop)
{
  long	realsize;
  
  debug0(3, unr, 232);
  if (!boxrange(unr)) return;
  if (user[unr]->sfspeedtime != 0) {
    if (prop == user[unr]->sfspeedprops) {
      if (user[unr]->sf_ptr != NULL) {
      	realsize  = user[unr]->sbytes - user[unr]->sfspeedsize;
	if (user[unr]->fwdmode > 0) realsize = (realsize * 100) / 60;
      	calc_linkspeed(user[unr]->sf_ptr->routp, user[unr]->sfspeedtime, realsize);
      }
      user[unr]->sfspeedtime = 0;
      user[unr]->sfspeedsize = 0;
      user[unr]->sfspeedprops = 0;
    } else if (prop > user[unr]->sfspeedprops) { /* error -> should not happen */
      user[unr]->sfspeedtime = 0;
      user[unr]->sfspeedsize = 0;
      user[unr]->sfspeedprops = 0;
    }
  }
}



static boolean calc_prop_crc(short unr, char *crcstr)
{
  boolean		Result;
  short			x, z;
  unsigned short	crc16;
  boolean		true_crc;
  char			hs[256];
  userstruct		*WITH;
  short			FORLIM1;
  char			STR1[256];

  crcstr[0]		= '\0';
  crc16			= 0;
  Result		= false;

  if (!boxrange(unr))
    return Result;

  WITH			= user[unr];
  true_crc		= (strchr(WITH->SID, 'D') != NULL);

  for (x = 0; x < MAXFBBPROPS; x++) {
    if (*WITH->fbbprop[x].line != '\0') {
      Result		= true;
      if (true_crc) {  /*HBS-Checksumme*/
	FORLIM1		= strlen(WITH->fbbprop[x].line);
	for (z = 0; z < FORLIM1; z++)
	  crcthp(WITH->fbbprop[x].line[z], &crc16);
      } else {  /*FBB-Checksumme*/
	FORLIM1		= strlen(WITH->fbbprop[x].line);
	for (z = 0; z < FORLIM1; z++)
	  crc16		= (crc16 + WITH->fbbprop[x].line[z]) & 0xff;
	crc16		= (crc16 + 13) & 0xff;
      }
    }
  }

  if (true_crc) {
    int2hstr(crc16, hs);
    while (strlen(hs) < 4)
      sprintf(hs, "0%s", strcpy(STR1, hs));
    sprintf(crcstr, "%c%c%c%c", hs[2], hs[3], hs[0], hs[1]);
    return Result;
  }

  crc16		= (-crc16) & 0xff;
  int2hstr(crc16, hs);
  while (strlen(hs) < 2)
    sprintf(hs, "0%s", strcpy(STR1, hs));
  sprintf(crcstr, "%c%c", hs[0], hs[1]);
  return Result;
}

/* send a roughly 2000 bytes linkcheck message */
static void send_ascii_linkcheck_data(short unr)
{
  char	  hs[256];
  short   x;
  
  if (!boxrange(unr)) return;
  wlnuser(unr, "Linkcheck data");
  create_my_rline(clock_.ixtime, "", hs);
  wlnuser(unr, hs);
  wlnuser0(unr);
  for (x = 0; x < 8; x++) {
    wlnuser(unr, "This is a linkcheck. Delete this message, if you like to, but do not reject it.");
    wlnuser(unr, "Ceci est un essay de liason. Effacez ce message, mais ne le rejetez pas, SVP.");
    wlnuser(unr, "Dies ist ein Linkcheck. Er kann geloescht werden, aber bitte nicht zurueckweisen.");
  }
  chwuser(unr, 0x1a);
  wlnuser0(unr);
}

/* these data are generated for a link test, if we had no tests for a longer time */
static boolean generate_linkcheck_data(short unr)
{
  short   x;
  bidtype bid;
  char	  hs[256];
  mbxtype bcall;

  debug0(3, unr, 233);
  if (!boxrange(unr)) return false;
  strcpy(bcall, user[unr]->call);
  complete_hierarchical_adress(bcall);

  if (user[unr]->sf_level < 3) {
    new_bid(bid);
    write_msgid(-1, bid);
    snprintf(hs, 255, "SP W @ %s < %s $%s", bcall, Console_call, bid);
    if (user[unr]->sf_level == 2) strcat(hs, " #1");
    start_linkcheck(unr, 0);
    wlnuser(unr, hs);
    user[unr]->action = 557;
    return true;
  }

  switch (user[unr]->fwdmode) {
    case 1:
    case 3:   strcpy(hs, "FA");
      	      break;

    case 2:
    case 4:
    case 5:   strcpy(hs, "FD");
      	      break;

    default:  strcpy(hs, "FB");
	      break;
  }

  for (x = 0; x < MAXFBBPROPS; x++) {
    new_bid(bid);
/* please uncomment the following line, if a real file will be sent after this proposal.  */
/* in DPBOX v6.00, only the proposal is sent, and for this purpose, we do not need to     */
/* waste space in the bulletin id buffer.      	      	      	      	      	      	  */
/*
    write_msgid(-1, bid);
*/
    snprintf(user[unr]->fbbprop[x].line, 255, "%s P %s %s %s %s 1000",
      	      hs, Console_call, bcall, user[unr]->call, bid);
  }
  calc_prop_crc(unr, hs);
  start_linkcheck(unr, 0);
  for (x = 0; x < MAXFBBPROPS; x++)
    wlnuser(unr, user[unr]->fbbprop[x].line);
  wuser(unr, "F> ");
  wlnuser(unr, hs);
  user[unr]->action = 555;
  return true;
}


/* Loescht die verketteten Pointer der Resume-Liste   */

void del_resume_list(void)
{
  resumemem	*hr, *hl;

  hr		= resume_root;
  while (hr != NULL) {
    hl		= hr->next;
    free(hr);
    hr		= hl;
  }
  resume_root	= NULL;
}


/* Laedt die Resume-List von Disk ins RAM    */

void load_resume(void)
{
  short		k;
  resumemem	*hr, *hl;
  short		ct, x;

  del_resume_list();
  ct		= sfsize(resume_inf) / sizeof(resumemem);
  if (ct <= 0)
    return;

  k		= sfopen(resume_inf, FO_READ);
  if (k < minhandle)
    return;

  hl		= NULL;
  hr		= NULL;
  for (x = 1; x <= ct; x++) {
    hr		= malloc(sizeof(resumemem));
    if (hr != NULL) {
      sfread(k, sizeof(resumemem), (char *)hr);
      hr->next	= hl;
      hl	= hr;
    }
  }
  sfclose(&k);
  resume_root	= hr;
}


/* Schreibt die im Speicher gehaltene Resume-Liste auf Disk  */

static void write_resume(void)
{
  short		k;
  resumemem	*hr;

  if (resume_root == NULL) {
    sfdelfile(resume_inf);
    return;
  }
  k		= sfcreate(resume_inf, FC_FILE);
  if (k < minhandle)
    return;

  hr		= resume_root;
  while (hr != NULL) {
    sfwrite(k, sizeof(resumemem), (char *)hr);
    hr		= hr->next;
  }
  sfclose(&k);
}


/* Loescht File und Eintrag in resume-liste identifiziert durch bid und call  */

static void del_resume(char *bid, char *call)
{
  resumemem	*hr, *hl;
  pathstr	STR1;

  hr		= resume_root;
  hl		= hr;
  while (hr != NULL) {
    if (strcmp(hr->rbid, bid)) {
      hl	= hr;
      hr	= hr->next;
      continue;
    }
    if (strcmp(hr->rcall, call)) {
      hl	= hr;
      hr	= hr->next;
      continue;
    }
    sprintf(STR1, "%s%s", boxstatdir, hr->rfname);
    sfdelfile(STR1);
    if (hl == hr)
      resume_root = hr->next;
    else
      hl->next	= hr->next;
    free(hr);
    hr		= NULL;
    write_resume();   /* Liste aktualisiert auf Disk schreiben */
  }
}


/* loescht nach Ablauf der Lifetime die Resume-Files  */
/* wird aus Garbage aufgerufen                        */

void kill_resume(void)
{
  resumemem	*hr, *hl;
  pathstr	STR1;

  debug0(1, 0, 69);
  hr		= resume_root;
  hl		= hr;
  while (hr != NULL) {
    if (clock_.ixtime - hr->rdate <= resume_lifetime * SECPERDAY &&
	clock_.ixtime >= hr->rdate) {
      hl	= hr;
      hr	= hr->next;
      continue;
    }

    sprintf(STR1, "%s%s", boxstatdir, hr->rfname);
    sfdelfile(STR1);
    if (hl == hr) {
      resume_root = hr->next;
      hl	= resume_root;
    } else
      hl->next	= hr->next;
    free(hr);
    if (hl != NULL)
      hr	= hl->next;
    else
      hr	= NULL;
  }

  write_resume();   /* Liste aktualisiert auf Disk schreiben */
}


/* sucht in der resume-file-liste nach einem Eintrag mit derselben BID       */
/* gesendet von demselben Call. Wenn gefunden, dann werden Filename und      */
/* Offset zurueckgeliefert. Sollte da was inkonsistent sein (File geloescht  */
/* oder andere Laenge) werden Eintrag und File geloescht.                    */

static long check_resume(char *bid, char *call, char *fname)
{
  long		Result;
  resumemem	*hr;
  pathstr	STR1;

  debug(3, 0, 70, call);

  fname[0]	= '\0';
  Result	= 0;
  hr		= resume_root;

  while (hr != NULL) {
    if (strcmp(hr->rbid, bid)) {
      hr	= hr->next;
      continue;
    }
    if (strcmp(hr->rcall, call)) {
      hr	= hr->next;
      continue;
    }
    sprintf(STR1, "%s%s", boxstatdir, hr->rfname);
    if (sfsize(STR1) == hr->rsize) {
      Result	= hr->rsize;
      strcpy(fname, hr->rfname);
      hr	= NULL;
      debug(3, 0, 70, "fragment found");
    } else {
      debug(3, 0, 70, "unmatching size");
      del_resume(bid, call);
      hr	= NULL;
    }
  }

  return Result;
}


short bbs_pack(short unr)
{
  userstruct	*WITH;

  if (!boxrange(unr))
    return 0;

  WITH		= user[unr];
  if (!((strchr(WITH->SID, 'F') != NULL || strchr(WITH->SID, 'D') != NULL)
      && strchr(WITH->SID, 'B') != NULL && WITH->sf_level > 3 && packed_sf))
    return 0;

  if (strstr(WITH->SID, "B1") != NULL) { 
    if (strchr(WITH->SID, 'D') != NULL) {
      if (strstr(WITH->SID, "D1") != NULL)
	return 5;   /* + CRC im Block        */
      else
	return 4;   /* HBS' neuer BIN-Modus  */
    } else
      return 3;     /* FBBs neuer BIN-Modus  */
  } else {
    if (strchr(WITH->SID, 'D') != NULL)
      return 2;     /* HBS' alter BIN-Modus  */
    else
      return 1;     /* FBBs alter BIN-Modus  */
  }
  return 0;
}

#define chwuser1(unr, ch, chksum) chksum = (chksum + ch) & 0xff; chwuser(unr, ch)

static void boxmemspool1(short unr, short tcon, short pchan, short prn,
      	      	      	  boolean strip_lf, boolean add_bcrc, char *puffer, long blen,
      	      	      	  unsigned short *chksum, unsigned short *bcrc)
{
  checksum8_buf(puffer, blen, chksum);

  if (!add_bcrc) {
    boxmemspool(tcon, pchan, prn, strip_lf, puffer, blen);
    return;
  }
  
  crcfbb_buf(puffer, blen, bcrc);
  boxmemspool(tcon, pchan, prn, strip_lf, puffer, blen);
  chwuser(unr, (*bcrc) & 0xff);
  chwuser(unr, ((*bcrc) >> 8) & 0xff);
}


/* 'Send packed fbb' in RAM */

void send_pfbbram(short mode, short unr, char *puffer, long size,
		  long offset, char *betreff)
{
  char			b1, b2;
  short			x, FORLIM;
  unsigned short	sf_crc, bcrc, sf_chksum;
  boolean		blockcrc;
  long			blen, ct;
  userstruct		*WITH;
  char			hs[256];

  debug(2, unr, 71, betreff);

  if (!boxrange(unr))
    return;

  b1		= 0;
  b2		= 0;
  sf_crc	= 0;

  if (mode < 3)
    offset	= 0;
  else if (offset < 10)
    offset	= 0;

  blockcrc	= (mode == 5);

  sprintf(hs, "%6ld", offset);

  chwuser(unr, 1);
  chwuser(unr, strlen(betreff) + strlen(hs) + 2);
  wuser(unr, betreff);
  chwuser(unr, 0);
  wuser(unr, hs);
  chwuser(unr, 0);

  if (offset > 1)
    offset	-= 2;

  switch (mode) {

  case 3:
  case 4:
  case 5:  /* FBB 2 */
    if ((unsigned)mode < 32 && ((1L << mode) & 0x30) != 0) {   /* HBS neu */
      FORLIM	= strlen(betreff);
      /* HBS berechnet die CRC auch ueber den Titel */
      for (x = 0; x < FORLIM; x++)
	crcfbb(betreff[x], &sf_crc);
    }
    crcfbb_buf(puffer, size, &sf_crc);
    b1		= sf_crc & 0xff;
    b2		= (sf_crc >> 8) & 0xff;
    break;

  case 2:  /* HBS */
    FORLIM	= strlen(betreff);
    for (x = 0; x < FORLIM; x++)
      crcthp(betreff[x], &sf_crc);
    crcthp(0, &sf_crc);
    FORLIM	= strlen(hs);
    for (x = 0; x < FORLIM; x++)
      crcthp(hs[x], &sf_crc);
    crcthp(0, &sf_crc);
    crcthp_buf(puffer, size, &sf_crc);
    b1		= sf_crc & 0xff;
    b2		= (sf_crc >> 8) & 0xff;
    break;
  }

  ct		= offset;
  sf_chksum	= 0;

  while (ct < size) {

    /* Nach Moeglichkeit blen auf max. 250 lassen ! Nebeneffekte  */
    /* siehe weiter unten                                         */

    if (size - ct > 250)
      blen	= 250;
    else
      blen	= size - ct;

    bcrc	= 0;
    chwuser(unr, 2);

    if (ct == offset && (unsigned)mode < 32 && ((1L << mode) & 0x38) != 0) {
      
      /* erster Datenblock, bei FBB 2  */
      /* CRC einfuegen                 */

      /* Achtung: Das funktioniert SO nur, wenn blen 3..255 ist, also      */
      /* VORSICHT bei Modifikation des Sources !   Nicht auf 0 setzen!     */

      if (offset == 0) {

	if (blen > 248) {
	  chwuser(unr, blen & 0xff);
	  blen	-= 2;
	} else
	  chwuser(unr, (blen + 2) & 0xff);

	chwuser1(unr, b1, sf_chksum);   /* CRC Byte 1    */
	chwuser1(unr, b2, sf_chksum);   /* CRC Byte 2    */

	if (blockcrc) {
	  crcfbb(b1, &bcrc);   /* BlockCRC auch updaten */
	  crcfbb(b2, &bcrc);
	}

      } else {

      /* FBB fuegt einen ganzen YAPP - Block ein, wenn der Offset > 0 ist   */

	chwuser(unr, 6);
	chwuser1(unr, b1, sf_chksum);   /* CRC Byte 1    */
	chwuser1(unr, b2, sf_chksum);   /* CRC Byte 2    */

	chwuser1(unr, puffer[0], sf_chksum);   /* Originallaenge */
	chwuser1(unr, puffer[1], sf_chksum);
	chwuser1(unr, puffer[2], sf_chksum);
	chwuser1(unr, puffer[3], sf_chksum);

	if (blockcrc) {  /* BlockCRC erzeugen und anhaengen   */
	  crcfbb(b1, &bcrc);   /* CRC Byte 1    */
	  crcfbb(b2, &bcrc);   /* CRC Byte 2    */

	  crcfbb(puffer[0], &bcrc);   /* Originallaenge */
	  crcfbb(puffer[1], &bcrc);
	  crcfbb(puffer[2], &bcrc);
	  crcfbb(puffer[3], &bcrc);

	  chwuser(unr, bcrc & 0xff);
	  chwuser(unr, (bcrc >> 8) & 0xff);

	  bcrc	= 0;
	}

	chwuser(unr, 2);
	chwuser(unr, blen & 0xff);
      }

    } else
      chwuser(unr, blen & 0xff);

    WITH	= user[unr];

    boxmemspool1(unr, WITH->tcon, WITH->pchan, boxprn2bios(WITH->print),
		 false, blockcrc, &puffer[ct], blen, &sf_chksum, &bcrc);

    upd_statistik(unr, 0, blen, 0, 0);
    ct		+= blen;
  }

  chwuser(unr, 4);

  if (mode == 2) {   /* HBS */
    chwuser(unr, b1);
    chwuser(unr, b2);
  } else {
    sf_chksum = (-sf_chksum) & 0xff;
    chwuser(unr, sf_chksum);
  }

}


/* 'Send packed fbb' from disk */

void send_pfbbdisk(short mode, short unr, long offset, char *tempname,
		   char *betreff)
{
  char			b1, b2;  
  short			x, FORLIM, fhandle;
  unsigned short	sf_crc, bcrc, sf_chksum;
  long			blen, size, ct;
  boolean		blockcrc;
  char			*puffer;
  userstruct		*WITH;
  char			hs[256];
  char			firstfour[4];
  char			pufferarr[256];

  debug(2, unr, 72, tempname);

  if (!boxrange(unr))
    return;

  puffer	= pufferarr;
  b1		= 0;
  b2		= 0;
  sf_crc	= 0;

  if (mode < 3)
    offset	= 0;
  else if (offset < 10)
    offset	= 0;

  blockcrc	= (mode == 5);

  sprintf(hs, "%6ld", offset);

  chwuser(unr, 1);
  chwuser(unr, strlen(betreff) + strlen(hs) + 2);
  wuser(unr, betreff);
  chwuser(unr, 0);
  wuser(unr, hs);
  chwuser(unr, 0);

  if (offset > 1)
    offset	-= 2;

  fhandle = sfopen(tempname, FO_READ);

  if (fhandle < minhandle)
    return;

  /* Die ersten vier Bytes... */

  sfread(fhandle, 4, firstfour);
  sfclose(&fhandle);

  switch (mode) {

  case 3:
  case 4:
  case 5:  /* FBB 2 */
    if ((unsigned)mode < 32 && ((1L << mode) & 0x30) != 0) {   /* HBS neu */
      FORLIM	= strlen(betreff);
      /* HBS berechnet die CRC auch ueber den Titel */
      for (x = 0; x < FORLIM; x++)
	crcfbb(betreff[x], &sf_crc);
    }

    for (x = 0; x <= 3; x++)
      crcfbb(firstfour[x], &sf_crc);
    file_crc(3, tempname, sf_crc, 4, 0);
    b1		= sf_crc & 0xff;
    b2		= (sf_crc >> 8) & 0xff;
    break;

  case 2:  /* HBS */
    FORLIM	= strlen(betreff);
    for (x = 0; x < FORLIM; x++)
      crcthp(betreff[x], &sf_crc);
    crcthp(0, &sf_crc);
    FORLIM	= strlen(hs);
    for (x = 0; x < FORLIM; x++)
      crcthp(hs[x], &sf_crc);
    crcthp(0, &sf_crc);
    for (x = 0; x <= 3; x++)
      crcfbb(firstfour[x], &sf_crc);
    file_crc(2, tempname, sf_crc, 4, 0);
    b1		= sf_crc & 0xff;
    b2		= (sf_crc >> 8) & 0xff;
    break;
  }

  size		= sfsize(tempname);
  ct		= offset;
  sf_chksum	= 0;
  fhandle	= sfopen(tempname, FO_READ);

  if (fhandle < minhandle)
    return;

  sfseek(offset, fhandle, SFSEEKSET);

  while (ct < size) {

    /* Nach Moeglichkeit blen auf max. 250 lassen ! Nebeneffekte  */
    /* siehe weiter unten                                         */

    if (size - ct > 250)
      blen	= 250;
    else
      blen	= size - ct;

    chwuser(unr, 2);
    bcrc	= 0;

    if (ct == offset && (unsigned)mode < 32 && ((1L << mode) & 0x38) != 0) {

      /* erster Datenblock, bei FBB 2  */
      /* CRC einfuegen                 */

      /* Achtung: Das funktioniert SO nur, wenn blen 3..255 ist, also      */
      /* VORSICHT bei Modifikation des Sources !   Nicht auf 0 setzen!     */

      if (offset == 0) {

	chwuser(unr, blen & 0xff);
	chwuser1(unr, b1, sf_chksum);   /* CRC Byte 1    */
	chwuser1(unr, b2, sf_chksum);   /* CRC Byte 2    */

	chwuser1(unr, firstfour[0], sf_chksum);
	chwuser1(unr, firstfour[1], sf_chksum);
	chwuser1(unr, firstfour[2], sf_chksum);
	chwuser1(unr, firstfour[3], sf_chksum);

	if (blockcrc) {
	  crcfbb(b1, &bcrc);   /* BlockCRC auch updaten */
	  crcfbb(b2, &bcrc);

	  crcfbb(firstfour[0], &bcrc);
	  crcfbb(firstfour[1], &bcrc);
	  crcfbb(firstfour[2], &bcrc);
	  crcfbb(firstfour[3], &bcrc);
	}

	blen	-= 6;
	ct	+= 4;

      } else {

      /* FBB fuegt einen ganzen YAPP - Block ein, wenn der Offset > 0 ist   */

	chwuser(unr, 6);
	chwuser1(unr, b1, sf_chksum);   /* CRC Byte 1    */
	chwuser1(unr, b2, sf_chksum);   /* CRC Byte 2    */

	chwuser1(unr, firstfour[0], sf_chksum);
	chwuser1(unr, firstfour[1], sf_chksum);
	chwuser1(unr, firstfour[2], sf_chksum);
	chwuser1(unr, firstfour[3], sf_chksum);

	if (blockcrc) {
	  crcfbb(b1, &bcrc);   /* BlockCRC auch updaten */
	  crcfbb(b2, &bcrc);

	  crcfbb(firstfour[0], &bcrc);
	  crcfbb(firstfour[1], &bcrc);
	  crcfbb(firstfour[2], &bcrc);
	  crcfbb(firstfour[3], &bcrc);

	  chwuser(unr, bcrc & 0xff);
	  chwuser(unr, (bcrc >> 8) & 0xff);

	  bcrc	= 0;
	}

	chwuser(unr, 2);
	chwuser(unr, blen & 0xff);
      }

    } else
      chwuser(unr, blen & 0xff);

    blen	= sfread(fhandle, blen, puffer);
    if (blen <= 0)
      break;

    WITH	= user[unr];

    boxmemspool1(unr, WITH->tcon, WITH->pchan, boxprn2bios(WITH->print),
		 false, blockcrc, puffer, blen, &sf_chksum, &bcrc);

    upd_statistik(unr, 0, blen, 0, 0);
    ct		+= blen;
  }

  sfclose(&fhandle);
  chwuser(unr, 4);

  if (mode == 2) {   /* HBS */
    chwuser(unr, b1);
    chwuser(unr, b2);
  } else {
    sf_chksum	= (-sf_chksum) & 0xff;
    chwuser(unr, sf_chksum);
  }

}

#undef chwuser1

boolean resend_userfile(boolean redirect, char *brett, char *newmbx)
{
  pathstr	index;
  indexstruct	header;
  short		k, list, lv;
  boolean	has_mails;
  mbxtype	umbx;
  char		hs[256];

  debug(2, 0, 73, brett);
  
  has_mails	= false;
  
  if (!strcmp(brett, Console_call))
    return false;

  add_hpath(newmbx);
  unhpath(newmbx, umbx);

  idxfname(index, brett);

  lv		= sfsize(index) / sizeof(indexstruct);
  if (lv <= 0)
    return false;

  list		= sfopen(index, FO_RW);
  if (list >= minhandle) {
    for (k = 1; k <= lv; k++) {
      read_index(list, k, &header);
      if (!header.deleted && check_hcs(header)) {
	if ( strcmp(header.verbreitung, newmbx)
	    || (clock_.ixtime - header.rxdate > 5 * SECPERDAY) ) {
	  if ((header.msgflags & (MSG_SFRX | MSG_MINE)) != 0 && header.fwdct == 0) {
	      has_mails	= true;
	    if (redirect) {
	      if ((header.msgflags & MSG_OWNHOLD) == 0) {
	        new_bid(header.id);   /* wie immer, wenn wir userfiles */
	                              /* redirecten wollen             */
	        strcpy(header.verbreitung, newmbx);
	        write_index(list, k, &header);
	        sfclose(&list);
	        write_msgid(-1, header.id);
	        alter_log(true, header.msgnum, header.msgflags, '$', header.id);
	        alter_log(true, header.msgnum, header.msgflags, '@', newmbx);
	        *hs	= '\0';
	        if (strcmp(umbx, Console_call))
	          set_forward(-1, -1, brett, hs, k, k, "FORWARD", hs, hs);
	        list	= sfopen(index, FO_RW);
	      }
	    } else { /* zukuenftige MyBBS-Spielereien unterbinden */
	      header.msgflags |= MSG_OWNHOLD;
	      write_index(list, k, &header);
	      alter_log(true, header.msgnum, header.msgflags, '!', "");
	    }
	  }
	}
      }
    }
  }

  sfclose(&list);
  
  return has_mails;
}


static boolean not_while_connected(char *absender, char *ziel, long date)
{
  short	y;

  if (!strcmp("T", ziel)) return true;
  if (!strcmp("W", ziel)) return true;
  if (!strcmp("PING", ziel)) return true;
  if (strstr(ziel, "M ") == ziel) return true;
  
  for (y = 1; y <= MAXUSER; y++) {
    if (user[y] != NULL) {
      if (!user[y]->f_bbs) {
	if (!user[y]->console) {
	  if (!strcmp(user[y]->call, absender))
	    return (clock_.ixtime - date > SFDELAYIFLOGIN);
	}
      }
    }
  }
  return true;
}


static boolean not_multiprivprop(short unr, char *bid)
{
  short	z, y;

  for (y = 1; y <= MAXUSER; y++) {
    if (user[y] != NULL) {
      if (user[y]->f_bbs) {
	if (strcmp(user[y]->call, user[unr]->call)) {
	  for (z = 0; z < MAXFBBPROPS; z++) {
	    if (!strcmp(user[y]->fbbprop[z].bid, bid))
	      return false;
	  }
	}
      }
    }
  }
  return true;
}


static boolean not_in_prop_list(short unr, short x, char *bid)
{
  short	z, y;

  for (y = 1; y <= MAXUSER; y++) {
    if (user[y] != NULL) {
      if (user[y]->f_bbs) {
	if (!strcmp(user[y]->call, user[unr]->call)) {
	  for (z = 0; z < MAXFBBPROPS; z++) {
	    if (user[y]->fbbprop[z].x_nr == x)
	      return false;
	    if (!strcmp(user[y]->fbbprop[z].bid, bid))
	      return false;
	  }
	}
      }
    }
  }
  return true;
}


static boolean not_double_same_sender(char *absender, char *lastsender,
				      boolean *dflag)
{
  if (strcmp(absender, lastsender)) {
    strcpy(lastsender, absender);
    return true;
  }
  *dflag = true;
  return false;
}


static void binmsg(short unr, short *list, char *tocall, char *tombx,
		   char *absender)
{
  pathstr	hs;
  short		k;

  if (tocall[0] == Console_call[0] || absender[0] == Console_call[0]) {
    sprintf(hs, "%simport%c001", newmaildir, extsep);
    validate(hs);
    k		= sfcreate(hs, FC_FILE);
    if (k >= minhandle) {
      if (tocall[0] == Console_call[0]) {
	str2file(&k, Console_call, true);
	str2file(&k, Console_call, true);
	str2file(&k, tocall, true);
	str2file(&k, tombx, true);
	strcpy(hs, "14");   /* Lifetime */
	rspacing(hs, 80);
	str2file(&k, hs, true);
	*hs	= '\0';   /* BID */
	str2file(&k, hs, true);
	sprintf(hs, "Undeliverable mail from %s at %s", absender, Console_call);
	str2file(&k, hs, true);
	*hs	= '\0';
	str2file(&k, hs, true);
	sprintf(hs, "binary file for %s at %s, please read out and delete!", tocall, Console_call);
	str2file(&k, hs, true);
      } else if (absender[0] == Console_call[0]) {
	str2file(&k, Console_call, true);
	str2file(&k, Console_call, true);
	str2file(&k, absender, true);
	str2file(&k, Console_call, true);
	strcpy(hs, "14");   /* Lifetime */
	rspacing(hs, 80);
	str2file(&k, hs, true);
	*hs	= '\0';   /* BID */
	str2file(&k, hs, true);
	sprintf(hs, "Undeliverable mail at %s", Console_call);
	str2file(&k, hs, true);
	*hs	= '\0';
	str2file(&k, hs, true);
	sprintf(hs, "binary file for %s at %s, can't forward.", tocall, Console_call);
	str2file(&k, hs, true);
	sprintf(hs, "no binary forward with %s", user[unr]->call);
	str2file(&k, hs, true);
      }
    }

    sfclose(&k);
  }

  sprintf(hs, "bin-mail held for %s@%s via %s (14 days)", tocall, tombx, user[unr]->call);
  append_profile(-1, hs);
}


static void setbinlt(indexstruct *hpointer, char *rubrik)
{
  pathstr	fname;
  short		rl, ct, lv;
  indexstruct	rp;

  if ((hpointer->pmode & 4) != 0)
    return;

  idxfname(fname, rubrik);
  lv	= sfsize(fname) / sizeof(indexstruct);
  rl	= open_index(rubrik, FO_RW, true, false);
  ct	= lv + 1;

  if (rl < minhandle)
    return;

  while (ct > 1) {
    ct--;
    read_index(rl, ct, &rp);
    if (!strcmp(rp.id, hpointer->id) && check_hcs(rp)) {
      if (rp.lifetime > 14) {
	rp.lifetime	= 14;
	write_index(rl, ct, &rp);
      }
    }
  }
  close_index(&rl);
}


static boolean conv_to_7plus(short unr, char *board, char *bid)
{
  short		lv2, x, y, k;
  long		pid;
  indexstruct	header;
  pathstr	fname, STR1;
  char		w[256], w1[256], hs1[256];
  char		ofi[256], sd[256], ld[256], hs[256];
  char		hsendbbs[256];

  if (*spluspath == '\0')
    return false;
  
  if (!new_7plus) /* we need the -# option */
    return false;

  lv2		= search_by_bid(board, bid, false);
  if (lv2 < 1)
    return false;

  sprintf(fname, "%stmp7pl%cXXXXXX", tempdir, extsep);
  mymktemp(fname);
  export_brett(unr, board, lv2, lv2, 100, "", "", fname);
  if (!exist(fname))
    return false;

  idxfname(STR1, board);
  k		= sfopen(STR1, FO_RW);
  if (k < minhandle)
    return false;

  read_index(k, lv2, &header);
  header.deleted	= true;
  header.eraseby	= EB_BIN27PLUS;
  header.erasetime	= clock_.ixtime;
  alter_log(false, header.msgnum, header.msgflags, 'E', "");
  write_index(k, lv2, &header);
  sfclose(&k);

  sprintf(STR1, "bin>7plus: converting mail from %s to %s@%s",
	  header.absender, header.dest, header.verbreitung);
  append_profile(-1, STR1);
  dp_watchdog(2, 4711);

  /* pid = fork(); */
  pid		= -1;
  /* denn eigentlich ist es unsinn, hier mit mehreren instanzen zu arbeiten */
  if (pid > 0) {
    add_zombie(pid, "", 0);
    return true;
  }
  /* setsid(); */

  strcpy(sd, fname);
  get_path(sd);
  sprintf(w, "%s %s -k -y -q -#", spluspath, fname);
  sfgetdir(0, ld);
  sfchdir(sd);
  my_system(w);
  sfchdir(ld);

  sfdelfile(fname);

  sprintf(ofi, "%s7plus.out", sd);
  sfdelfile(ofi);

  sprintf(ofi, "%s7plus.fls", sd);
  k	= sfopen(ofi, FO_READ);
  if (k < minhandle) {
    if (pid == 0)
      exit(1);
    else
      return false;
  }

  *ld	= '\0';
  file2str(k, ld);
  sfclosedel(&k);

  if (*ld == '\0') {
    if (pid == 0)
      exit(1);
    else
      return false;
  }

  get_word(ld, w);	/* Anzahl    */
  get_word(ld, w1);	/* Name      */
  y = atoi(w);

  if (y == 0) {
    if (pid == 0)
      exit(1);
    else
      return false;
  }

  strcpy(ofi, fname);
  del_path(ofi);

  strcpy(hsendbbs, header.sendbbs);
  complete_hierarchical_adress(hsendbbs);

  for (x = 1; x <= y; x++) {
    if (y == 1)
      strcpy(w, "7pl");
    else {
      int2hstr(x, w);
      lower(w);
      while (strlen(w) < 2)
	sprintf(w, "0%s", strcpy(STR1, w));
      sprintf(w, "p%s", strcpy(STR1, w));
    }
    sprintf(fname, "%s%s%c%s", sd, w1, extsep, w);
    
    if (!exist(fname)) { /* passiert immer dann, wenn 7plus als BIN eingespielt war */
      if (pid == 0)
        exit(1);
      else
        return false;
    }

    sprintf(hs, "%simport%cXXXXXX", newmaildir, extsep);
    mymktemp(hs);
    k		= sfcreate(hs, FC_FILE);
    if (k >= minhandle) {
      if (*header.rxfrom == '\0')
	str2file(&k, Console_call, true);
      else
	str2file(&k, header.rxfrom, true);
      str2file(&k, header.absender, true);
      str2file(&k, header.dest, true);
      str2file(&k, header.verbreitung, true);
      sprintf(hs, "%d", header.txlifetime);
      rspacing(hs, 80);
      str2file(&k, hs, true);
      *hs	= '\0';   /* BID */
      str2file(&k, hs, true);
      sprintf(hs, "%s%c%s", w1, extsep, w);
      upper(hs);
      sprintf(hs, "bin>7plus: %s > %s %s", ofi, strcpy(STR1, hs), header.betreff);
      cut(hs, 79);
      str2file(&k, hs, true);
      /* Start der Message */
      if (strcmp(header.sendbbs, Console_call)) {
	ixdatum2string(header.txdate, hs);
	sprintf(hs1, "%c%c%c%c%c%c/", hs[6], hs[7], hs[3], hs[4], hs[0], hs[1]);
	ixzeit2string(header.txdate, hs);
	sprintf(hs1 + strlen(hs1), "%c%c%c%cz", hs[0], hs[1], hs[3], hs[4]);
	sprintf(hs, "R:%s @:", hs1);
	strcat(hs, hsendbbs);
	str2file(&k, hs, true);
	*hs	= '\0';
	str2file(&k, hs, true);
	sprintf(hs, "Original message from %s@%s", header.absender, hsendbbs);
	str2file(&k, hs, true);
      } else {
	*hs	= '\0';
	str2file(&k, hs, true);
      }
      str2file(&k, "Converted from full binary format to 7plus", true);
      sprintf(hs, "at %s %s", ownhiername, ownfheader);
      str2file(&k, hs, true);
      sprintf(hs, "Send error reports to %s@%s, NOT TO %s !", header.absender, hsendbbs, Console_call);
      str2file(&k, hs, true);
      *hs	= '\0';
      str2file(&k, hs, true);
      app_file2(fname, k, 0, true);
      sfclose(&k);
    }
  }

  sprintf(hs, "%simport%c001", newmaildir, extsep);
  validate(hs);
  k	= sfcreate(hs, FC_FILE);
  if (k >= minhandle) {
    str2file(&k, Console_call, true);
    str2file(&k, Console_call, true);
    str2file(&k, header.absender, true);
    str2file(&k, hsendbbs, true);
    sprintf(hs, "%d", header.txlifetime);
    rspacing(hs, 80);
    str2file(&k, hs, true);
    *hs	= '\0';   /* BID */
    str2file(&k, hs, true);
    sprintf(hs, "bin>7plus: %s > %s %s", ofi, header.dest, header.betreff);
    cut(hs, LEN_SUBJECT);
    str2file(&k, hs, true);
    *hs	= '\0';
    str2file(&k, hs, true);
    strcpy(hs, "Converted your mail from full binary format to 7plus at");
    str2file(&k, hs, true);
    sprintf(hs, "%s %s", ownhiername, ownfheader);
    str2file(&k, hs, true);
    sprintf(hs, "Wait for error reports of %s@%s", header.dest, header.verbreitung);
    str2file(&k, hs, true);
    *hs	= '\0';
    str2file(&k, hs, true);
    strcpy(hs, "To respond a returning error report, put it in the same directory as");
    str2file(&k, hs, true);
    strcpy(hs, "the original file and start 7plus with the error report as option.");
    str2file(&k, hs, true);
    strcpy(hs, "7plus returns a .COR file, send it to");
    str2file(&k, hs, true);
    sprintf(hs, "%s@%s", header.dest, header.verbreitung);
    str2file(&k, hs, true);
    *hs	= '\0';
    str2file(&k, hs, true);
    sprintf(hs, "73 from %s %s", ownhiername, ownfheader);
    str2file(&k, hs, true);
    *hs	= '\0';
    str2file(&k, hs, true);
    sfclose(&k);
  }

  if (pid == 0)
    exit(0);
  return true;
}


static boolean valid_entry(indexstruct hpointer, char *rubrik,
			   boolean spec_sf, long maxu, long maxb,
			   boolean send_pack, short *ct, char *origrub)
{
  pathstr	fname;
  short		rlist, lv2;
  indexstruct	header1, *rpointer;
  boolean	valid;

  valid		= false;

  if ((hpointer.pmode & 4) != 0) {
    valid = true;
    return valid;
  }

  idxfname(fname, rubrik);
  lv2		= sfsize(fname) / sizeof(indexstruct);
  rlist		= open_index(rubrik, FO_READ, true, false);
  *ct		= lv2 + 1;
  if (rlist >= minhandle) {
    header1.deleted = false;
    rpointer	= &header1;   /* bleibt gleich wenn isize <= 0 */

    while (*ct > 1 && !valid) {
      (*ct)--;
      read_index(rlist, *ct, &header1);
      if (check_hcs(*rpointer) &&
	  (hpointer.msgnum == 0 || hpointer.msgnum == rpointer->msgnum) &&
	  !strcmp(rpointer->id, hpointer.id))
	valid	= true;
    }

    close_index(&rlist);

    if (rpointer->deleted)
      valid	= false;
    if (valid)
      valid	= (!strcmp(hpointer.verbreitung, rpointer->verbreitung) || spec_sf);
    if (valid) {
      if (rpointer->msgtype != 'B')
	valid	= (rpointer->size <= maxu || (rpointer->packsize <= maxu && send_pack));
      else
	valid	= (rpointer->size <= maxb || (rpointer->packsize <= maxb && send_pack));
    }

    if (valid)
      strcpy(origrub, rpointer->dest);
  }

  return valid;
}


/* local consts/types fuer propose_sf_sending */

#define msg_nil         0
#define msg_conscall    1
#define msg_user        2
#define msg_bulletin    3
#define msg_system      4
#define msg_system2     5
#define maxsyscount     50

typedef struct proprectype {
  boolean	filled, is_sys, specsf;
  indexstruct	pheader;
  short		x_nr, r_nr;
  boardtype	porigrubrik, pmyrubrik;
} proprectype;

typedef proprectype	parrtyp[msg_system - msg_nil + 1][MAXFBBPROPS];
typedef short		pcttyp[msg_system - msg_nil + 1];
typedef char		lastsendertyp[msg_system - msg_nil + 1][LEN_CALL+1];

static short smaller_than_in_props(boolean is_system, indexstruct *hpointer, boolean send_pack,
      	      	      	      	   short msg_type, proprectype (*pa)[MAXFBBPROPS], short *pct)
{
  short		a, b;
  long		sizea, sizeb, sizec;
  short		FORLIM;

  sizec		= 0;
  b		= -1;

  if (send_pack) {
    if (is_system)
      sizeb	= 3000;
    else
      sizeb	= hpointer->packsize;
  } else {
    if (is_system)
      sizeb	= maxsyscount * 80;
    else
      sizeb	= hpointer->size;
  }

  FORLIM	= pct[msg_type - msg_nil];
  for (a = 1; a <= FORLIM; a++) {
    if (send_pack && pa[msg_type - msg_nil][a - 1].pheader.packsize > 0) {
      if (pa[msg_type - msg_nil][a - 1].is_sys)
	sizea	= 3000;
      else
	sizea	= pa[msg_type - msg_nil][a - 1].pheader.packsize;
    } else {
      if (pa[msg_type - msg_nil][a - 1].is_sys)
	sizea	= maxsyscount * 80;
      else
	sizea	= pa[msg_type - msg_nil][a - 1].pheader.size;
    }

    if (sizeb < sizea) {
      if (sizea > sizec) {
	b	= a;
	sizec	= sizea;
      }
    }

  }

  return b;
}


static void sort_pa(short maxprop, short msg_type, proprectype (*pa)[MAXFBBPROPS])
{
  short		a, b, c, d, loopc;
  long		s1, s2;
  proprectype	p1;

  for (d = 1; d <= 2; d++) {
    loopc	= 0;
    b		= maxprop;
    do {
      loopc++;
      a		= b;
      b--;
      while (a > 2 && !pa[msg_type - msg_nil][a - 1].filled)
	a--;
      if (a > 1 && pa[msg_type - msg_nil][a - 1].filled) {
	if (pa[msg_type - msg_nil][a - 1].is_sys)
	  s1		= 3000;
	else {
	  s1		= pa[msg_type - msg_nil][a - 1].pheader.packsize;
	  if (s1 == 0)
	    s1		= pa[msg_type - msg_nil][a - 1].pheader.size;
	}
	c		= a;
	while (c > 1 && pa[msg_type - msg_nil][c - 2].filled) {
	  c--;
	  if (pa[msg_type - msg_nil][c - 1].is_sys)
	    s2		= 3000;
	  else {
	    s2		= pa[msg_type - msg_nil][c - 1].pheader.packsize;
	    if (s2 == 0)
	      s2	= pa[msg_type - msg_nil][c - 1].pheader.size;
	  }
	  if (s1 < s2) {
	    p1					= pa[msg_type - msg_nil][c - 1];
	    pa[msg_type - msg_nil][c - 1]	= pa[msg_type - msg_nil][a - 1];
	    pa[msg_type - msg_nil][a - 1]	= p1;
	    c					= 0;
	  }
	}
      }
    } while (a != 1 && d != 2 && loopc <= 30);

    if (loopc > 30) {
      boxprotokoll("failure in sort_pa()");
    }
  }
}


static boolean not_in_own_props(short x, char *bid, short msg_type,
				proprectype (*pa)[MAXFBBPROPS], short *pct)
{
  short		a, FORLIM;

  FORLIM	= pct[msg_type - msg_nil];
  for (a = 0; a < FORLIM; a++) {
    if (pa[msg_type - msg_nil][a].x_nr == x)
      return false;
    if (!strcmp(pa[msg_type - msg_nil][a].pheader.id, bid))
      return false;
  }
  return true;
}


static char fwtype(char pmode, char *rubrik, char *absender)
{
  if ((pmode & 1) != 0) {
    if (!strcmp(absender, Console_call))
      return msg_conscall;
    else
      return msg_user;
  } else if ((pmode & 2) != 0) {
    if (!strcmp(rubrik, "F"))
      return msg_conscall;
    else
      return msg_bulletin;
  } else if ((pmode & 4) != 0) {
    return msg_system;

    /* spezielles Flag fuer Systemmails, die im blockweisen Senden stehen */

  } else if ((pmode & 16) != 0) {
    return msg_system2;

  } else
    return msg_nil;
}


/* Dies ist eine der komplexesten Routinen der Box, die Steuerung des    */
/* ausgehenden S&F. Bitte Vorsicht bei kleinen Aenderungen, sie haben    */
/* GROSSE Wirkung...                                                     */

static short propose_sf_sending(short unr)
{
  short			Result;
  boolean		doublesamesenderflag;
  lastsendertyp		lastsender;
  indexstruct		*hpointer, header;
  short			k, lv, sel, sel2, list, ct, is_what, newlt;
  boolean		is_ack, send_pack, valid, delete_it, spec_sf, boxbin;
  long			selsize;
  short			prop, x, y, maxprop;
  long			maxb, maxu, maxp;
  boolean		loopex, nothing, dpflag, theboxflag, invalid, again;
  boolean		is_system, flood, file_forward;
  short			is_smaller;
  long			TotalSize;
  parrtyp		pa;
  pcttyp		pct;
  char			msgchar;
  short			msg_type;
  unsigned short	sct;
  short			fct, fsl, fmax, loopct;
  char			hs[256], hs2[256], hb[256];
  char			to_box[256];
  char			w[256];
  boardtype		myrubrik, brubrik, rubrik, fbs, origrub;
  mbxtype		hmbx;
  pathstr		xname;
  userstruct		*WITH;
  char			STR1[256];
  sfdeftype		*WITH1;
  fbbproptype		*WITH2;
  proprectype		*WITH3;

  debug0(2, unr, 74);

  WITH       	= NULL;
  WITH1       	= NULL;
  WITH2       	= NULL;
  WITH3       	= NULL;
  
  recompile_fwd();
  
  Result	= 0;

  if (boxrange(unr)) {
    WITH	= user[unr];
    WITH->emblockct = 0;
    boxbin	= false;
    file_forward= (WITH->umode == UM_FILESF);
    dpflag	= (strchr(WITH->SID, 'D') != NULL);
    theboxflag	= (!strcmp(WITH->FSID, "THEBOX")); /* wenn ich das auskommentiere, stuerzt diebox ab */
    boxbin	= dpflag;
    send_pack	= (WITH->fwdmode > 0);

    if (WITH->sf_ptr != NULL) {
      WITH1	= WITH->sf_ptr;
      maxb	= WITH1->maxbytes_b;
      maxu	= WITH1->maxbytes_u;
      maxp	= WITH1->maxbytes_p;
    } else {
      maxb	= 50000L;
      maxu	= 50000L;
      maxp	= 50000L;
    }

    Result	= 0;
    strcpy(to_box, WITH->call);
    TotalSize	= sfsize(sflist);
    lv		= TotalSize / sizeof(indexstruct);
    is_what	= 0;
    selsize	= 0;
    sel		= 0;

    for (x = 0; x < MAXFBBPROPS; x++) {
      WITH2		= &WITH->fbbprop[x];
      WITH2->line[0]	= '\0';
      WITH2->brett[0]	= '\0';
      WITH2->nr		= 0;
      WITH2->x_nr	= 0;
      WITH2->crc	= 0;
      WITH2->pack	= false;
      WITH2->bid[0]	= '\0';
      WITH2->mtype	= '\0';
    }

    for (x = msg_nil; x <= msg_system; x++) {
      pct[x - msg_nil]	= 0;
      for (y = 0; y < MAXFBBPROPS; y++)
	pa[x - msg_nil][y].filled	= false;
    }

    nothing	= true;

    if (WITH->sf_level > 2)
      maxprop	= 5;
    else
      maxprop	= 1;

    if (lv > 0)
      list	= open_index("X", FO_RW, true, true);
    else
      list	= nohandle;

    if (list >= minhandle) {
      again	= false;

_L4:
      if (again) {
	is_what			= 0;
	selsize			= 0;
	sel			= 0;

	for (x = 0; x < MAXFBBPROPS; x++) {
	  WITH2			= &WITH->fbbprop[x];
	  WITH2->line[0]	= '\0';
	  WITH2->brett[0]	= '\0';
	  WITH2->nr		= 0;
	  WITH2->x_nr		= 0;
	  WITH2->crc		= 0;
	  WITH2->pack		= false;
	  WITH2->bid[0]		= '\0';
	  WITH2->mtype		= '\0';
	}

	for (x = msg_nil; x <= msg_system; x++) {
	  pct[x - msg_nil]	= 0;
	  for (y = 0; y < MAXFBBPROPS; y++)
	    pa[x - msg_nil][y].filled	= false;
	}
      } else
	again			= true;

      hpointer			= &header;   /* bleibt gleich, wenn xsize <= 0 */
      loopct			= 0;
      doublesamesenderflag	= false;
      do {
	if (doublesamesenderflag)   /* zweiter Schleifendurchlauf */
	  *WITH->lastsfcall	= '\0';
	doublesamesenderflag	= false;
	for (x = msg_conscall; x <= msg_system; x++) {
	  if (maxprop == 1)
	    strcpy(lastsender[x - msg_nil], WITH->lastsfcall);
	  else
	    lastsender[x - msg_nil][0] = '\0';
	}

	k			= 0;

	loopct++;

	do {
	  k++;
	  delete_it		= false;

	  read_index(list, k, &header);

	  if (!hpointer->deleted) {
	    if (!strcmp(hpointer->dest, to_box) && check_hcs(*hpointer) &&
		(hpointer->pmode & 128) == 0) /* laterflag */
	    {
	      msg_type		= fwtype(hpointer->pmode, hpointer->betreff, hpointer->absender);
	      is_smaller	= -1;
	      is_system		= (msg_type == msg_system);
	      if (is_system)
		msg_type	= msg_bulletin;

	      if (small_first
	      	  && !file_forward
		  && msg_type != msg_system2
		  && pct[msg_type - msg_nil] >= maxprop ) {
	      	is_smaller	= smaller_than_in_props(is_system, hpointer, send_pack, msg_type, pa, pct);
	      }

	      if (  msg_type != msg_system2
      	      	  && (pct[msg_type - msg_nil] < maxprop || is_smaller > 0)
		  && ((hpointer->msgflags & (MSG_LOCALHOLD | MSG_SFHOLD)) == 0
		    || clock_.ixtime - hpointer->rxdate > holddelay)
		  && not_in_own_props(k, hpointer->id, msg_type, pa, pct)
		  && not_in_prop_list(unr, k, hpointer->id)
		  && (is_system
		    || not_double_same_sender(hpointer->absender, lastsender[msg_type - msg_nil],
					      &doublesamesenderflag))
		  && not_while_connected(hpointer->absender, hpointer->betreff, hpointer->rxdate)) {
		if ((hpointer->pmode & 1) == 0
		    || multiprivate
		    || not_multiprivprop(unr, hpointer->id))
		    /* ist kein PrivateFile */
		    {  /* doch, aber noch nicht in Aussendung */
		  spec_sf	= ((hpointer->pmode & 8) != 0);
		  strcpy(hs, hpointer->betreff);
		  get_word(hs, rubrik);
		  unhpath(hpointer->verbreitung, hs);
		  flood		= !callsign(hs);
		  hs[0]		= '\0';
		  
		  valid		= !(theboxflag && flood && !callsign(hpointer->absender));
		  /* SB BLABLA@WW < SYSOP nicht an DieBox schicken */

		  if (valid && (hpointer->pmode & TBIN) != 0)
		  {  /* nur bei THEBOX1.9/DP Binfiles senden  */
		    /* und auch bei baycombox > 1.30         */
		    if (boxbin)
		      valid	= true;
		    else {
		      if (!flood) {
			if (!conv_to_7plus(unr, rubrik, hpointer->id)) {
			  binmsg(unr, &list, rubrik, hpointer->verbreitung, hpointer->absender);
			  setbinlt(hpointer, rubrik);
			}
		      }
		      valid	= false;
		      delete_it	= true;
		    }
		  } else
		    valid	= true;

		  if (valid) {
		    if (msg_type == msg_nil) {
		      debug(0, unr, 74, "invalid pmode in X -> deleted");
		      valid	= false;   /* was nie vorkommen sollte */
		      delete_it	= true;
		    }
		  }

		  if (valid) {
		    origrub[0]		= '\0';

		    if (is_smaller <= 0) {
		      pct[msg_type - msg_nil]++;
		      is_smaller	= pct[msg_type - msg_nil];
		    }

		    WITH3		= &pa[msg_type - msg_nil][is_smaller - 1];
		    WITH3->filled	= true;
		    WITH3->specsf	= spec_sf;
		    WITH3->pheader	= *hpointer;
		    WITH3->x_nr		= k;
		    WITH3->is_sys	= is_system;
		  }


		  if (delete_it) {
		    hpointer->deleted	= true;	/* Loeschen da nicht mehr gueltig */
		    write_index(list, k, hpointer);
		  }

		}
		/* doch, aber soll doppelt gesendet werden */

	      }

	    }

	  }

	} while (k < lv);

      } while (loopct <= maxprop && doublesamesenderflag);

      if (!file_forward && small_first && sort_props && maxprop > 1) {
	for (x = msg_conscall; x <= msg_bulletin; x++)   /* msg_system */
	  sort_pa(maxprop, x, pa);
      }

      prop		= 0;
      loopex		= false;
      invalid		= false;

      while (prop < maxprop && !loopex) {
	sel		= 0;
	sel2		= 0;
	is_what		= 0;

	for (x = msg_conscall; x <= msg_bulletin; x++) {   /* msg_system */
	  for (y = 0; y < maxprop; y++) {
	    WITH3		= &pa[x - msg_nil][y];
	    if (WITH3->filled) {
	      WITH3->filled	= false;
	      valid		= true;
	      strcpy(hs, WITH3->pheader.betreff);
	      get_word(hs, rubrik);
	      hs[0] = '\0';
	      if (!WITH3->is_sys)
		valid = valid_entry(WITH3->pheader, rubrik, WITH3->specsf,
				    maxu, maxb, send_pack, &ct, origrub);

	      if (valid) {
		if (WITH3->is_sys) {
		  WITH3->r_nr	= 0;
		  sprintf(STR1, "%c", WITH3->pheader.betreff[0]);
		  strcpy(WITH3->porigrubrik, STR1);
		  /*NUR EINBUCHSTABIGE SYSTEMRUBRIKEN*/
		  strcpy(WITH3->pmyrubrik, WITH3->porigrubrik);
		} else {
		  WITH3->r_nr	= ct;
		  strcpy(WITH3->porigrubrik, origrub);
		  strcpy(WITH3->pmyrubrik, rubrik);
		}

		sel		= WITH3->x_nr;
		sel2		= WITH3->r_nr;
		header		= WITH3->pheader;

		if (x == msg_conscall) {
		  if (!strcmp(WITH3->pmyrubrik, "F"))
			/* 'F' haben wir ja unter      */
			/* msg_conscall bevorrangt. Das*/
			/* muessen wir jetzt aber      */
			/* wieder auf typ=bulletin     */
			/* stellen, sonst kommt die    */
			/* Senderoutine aus dem        */
			/* Tritt                       */
		    is_what	= msg_bulletin;
		  else
		    is_what	= msg_user;
		} else
		  is_what	= x;
		if (WITH3->is_sys)
		  is_what	= msg_system;

		strcpy(myrubrik, WITH3->pmyrubrik);
		strcpy(brubrik, WITH3->porigrubrik);
		goto _L2;
	      }

	      invalid		= true;
	      WITH3->pheader.deleted = true;	/* Loeschen da nicht mehr gueltig */
	      write_index(list, WITH3->x_nr, &WITH3->pheader);
	    }
	  }
	}

_L2:
	if (sel <= 0) {
	  loopex		= true;
	  if (invalid && prop == 0) {
	    dp_watchdog(2, 4711);
	    goto _L4;
	  }
	} else {
	  is_ack		= (strchr(WITH->SID, 'A') != NULL && header.msgtype == 'A');
	  if (!strcmp(brubrik, "T") && !strcmp(WITH->FSID, "FBB")) {
	    msgchar		= 'P';
	    header.size		= 80;
	    header.packsize	= 80;
	  } else if (WITH->sf_level > 2 && is_what == msg_system) {
	    header.size		= 80;
	    header.packsize	= 80;
	    msgchar		= header.msgtype;
	    if (msgchar == '\0')
	      msgchar		= 'B';
	  } else if (header.msgtype != '\0')
	    msgchar		= header.msgtype;
	  else {
	    switch (is_what) {

	    case msg_user:
	      msgchar		= 'P';
	      break;

	    default:
	      msgchar		= 'B';
	      break;
	    }

	  }
	  if (msgchar == 'A' && !is_ack) {
	    msgchar		= 'P';
	  }

	  nothing		= false;
	  prop++;
	  strcpy(fbs, brubrik);
	  if (!dpflag && WITH->sf_level != 2) cut(fbs, 6);

	  unhpath(header.verbreitung, hmbx);

	  if (WITH->sf_level > 2) {
	    *WITH->lastsfcall	= '\0';

	    switch (WITH->fwdmode) {

	    case 1:
	    case 3:
	      strcpy(hs, "FA");
	      break;

	    case 2:
	    case 4:
	    case 5:
	      strcpy(hs, "FD");
	      break;

	    default:
	      strcpy(hs, "FB");
	      break;
	    }

	    sprintf(hs + strlen(hs), " %c", msgchar);

	    if (strchr(WITH->SID, 'H') != NULL)
	      sprintf(hs + strlen(hs), " %s %s %s %s %ld",
		      header.absender, header.verbreitung, fbs, header.id, header.size);
	    else
	      sprintf(hs + strlen(hs), " %s %s %s %s %ld",
		      header.absender, hmbx, fbs, header.id, header.size);
		      
	    if (is_what != msg_system) {
	      if (WITH->sf_level > 3 && send_pack)
		selsize		+= header.packsize;
	      else
		selsize		+= header.size;
	    }

	    if (dpflag && header.txlifetime > 0) {
	      newlt		= header.txlifetime - (clock_.ixtime - header.rxdate) / SECPERDAY;
	      if (newlt < 1)
		newlt		= 1;
	      sprintf(hs + strlen(hs), " %d", newlt);
	    }

	    if (is_what == msg_system)
	      sel2 = 0;

	    upper(hs);

	  } else {

	    if (is_what == msg_system && WITH->sf_level == 2) {
	      /* Blockweises E/M - Forwarding */

	      *WITH->lastsfcall	= '\0';
	      sct		= 0;
	      boxspoolfend(user[unr]->pchan);
	      fmax		= boxgetpaclen(user[unr]->pchan);
	      fct		= 0;

	      while (sel <= lv && sct < maxsyscount) {

		read_index(list, sel, &header);
		
		if (!hpointer->deleted) {
		  if (!strcmp(hpointer->dest, to_box) && check_hcs(*hpointer)) {
		    if ((hpointer->pmode & 4) != 0) {
		      sct++;
		      strcpy(hb, hpointer->betreff);
		      get_word(hb, hs2);   /* M/E loeschen */
		      sprintf(hs, "S %s @ %s < %s $%s",
			      hs2, hpointer->verbreitung, hpointer->absender,
			      hpointer->id);

		      if (!strcmp(hs2, "M") && theboxflag) {
			get_word(hb, w);
			unhpath(w, w);
		      } else
			get_word(hb, w);

		      sprintf(hs + strlen(hs), " %s", w);
		      if (!strcmp(hs2, "M"))
			get_word(hb, w);
		      else
			*w	= '\0';

      	      	      if (file_forward) {
		      	sprintf(hs + strlen(hs), " %s /EX", w);
			if (sct > 1) { /* we get a first OK from the calling function (look_for_mail) */
       	      	      	  box_input(unr, false, "OK", true);
      	      	      	  box_input(unr, false, ">", true);
			}
		      } else {
		      	sprintf(hs + strlen(hs), " %s \032", w);
		      }
		      fsl	= strlen(hs) + 1;

		      if (theboxflag || fct + fsl > fmax) {
			boxspoolfend(user[unr]->pchan);
			fct	= 0;
		      }

		      wlnuser(unr, hs);
		      fct	+= fsl;

		      hpointer->pmode = 16;
		      write_index(list, sel, hpointer);
		    }
		  }
		}
		sel++;
	      }
	      if (sct > 0) {
		WITH->action	= 40;
		WITH->emblockct	= sct;
		nothing		= false;
		goto _L3;
	      } else {
		nothing		= true;
		sel		= 0;
		goto _L3;
	      }
	    }

	    strcpy(WITH->lastsfcall, header.absender);

	    sprintf(hs, "S%c", msgchar);

	    /*die letzte DieBox schaut in die Userliste...*/
	    if (  is_what == msg_user
	      	  && !strcmp(hmbx, to_box)
		  && WITH->sf_level == 2
		  && callsign(fbs)
		) {
	      sprintf(hs + strlen(hs), " %s < %s", fbs, header.absender);
	    } else {
	      if (strchr(WITH->SID, 'H') != NULL)
		sprintf(hs + strlen(hs), " %s @ %s < %s", fbs, header.verbreitung, header.absender);
	      else
		sprintf(hs + strlen(hs), " %s @ %s < %s", fbs, hmbx, header.absender);
	    }

	    if ( (is_what != msg_user
	      	  || msgchar == 'B'
		  || strchr(user[unr]->SID, 'M') != NULL
		  || !callsign(fbs)
		  || !callsign(hmbx) )
		 && *header.id != '\0'
		)
	      sprintf(hs + strlen(hs), " $%s", header.id);

	    if (  (is_what == msg_bulletin || dpflag)
	      	  && header.txlifetime > 0
		) {
	      if (WITH->sf_level == 2) {
		newlt	= header.txlifetime - (clock_.ixtime - header.rxdate) / SECPERDAY;
		if (newlt < 1)
		  newlt	= 1;
		sprintf(hs + strlen(hs), " #%d", newlt);
	      }
	    }

	    if (is_what == msg_system)
	      sel2	= 0;
	  }

	  strcpy(WITH->fbbprop[prop - 1].line, hs);
	  strcpy(WITH->fbbprop[prop - 1].brett, myrubrik);
	  WITH->fbbprop[prop - 1].nr	= sel2;
	  WITH->fbbprop[prop - 1].x_nr	= sel;
	  strcpy(WITH->fbbprop[prop - 1].bid, header.id);

	  switch (is_what) {

	  case msg_system:
	    WITH->fbbprop[prop - 1].mtype = 'S';
	    break;

	  case msg_user:
	    WITH->fbbprop[prop - 1].mtype = 'P';
	    break;

	  default:
	    WITH->fbbprop[prop - 1].mtype = 'B';
	    break;
	  }

	}

	if (selsize >= maxp)
	  loopex	= true;
	if (nothing)
	  loopex	= true;
      }

_L3:
      close_index(&list);
    }

    if (nothing && *WITH->fbbprop[0].line == '\0' && strcmp(WITH->call, Console_call)) {
      idxfname(xname, WITH->call);
      if (exist(xname)) {
	lv		= sfsize(xname) / sizeof(indexstruct);
	list		= open_index(WITH->call, FO_RW, true, true);
	if (list >= minhandle) {
	  k		= 1;
	  prop		= 1;
	  selsize	= 0;
	  do {
	    read_index(list, k, &header);

	    if (!header.deleted && check_hcs(header) && header.fwdct == 0 &&
		(header.msgflags & MSG_CUT) == 0) {
	      if (header.msgtype != '\0')
		msgchar	= header.msgtype;
	      else
		msgchar	= 'P';

	      if (msgchar == 'A') {
		if (strchr(WITH->SID, 'A') == NULL)
		  msgchar = 'P';
	      }

	      if (WITH->sf_level > 2) {

		switch (WITH->fwdmode) {

		case 1:
		case 3:
		  strcpy(hs, "FA");
		  break;

		case 2:
		case 4:
		case 5:
		  strcpy(hs, "FD");
		  break;

		default:
		  strcpy(hs, "FB");
		  break;
		}

		sprintf(hs + strlen(hs), " %c %s %s %s %s %ld",
			msgchar, header.absender, WITH->call, user[unr]->call,
			header.id, header.size);

		if (WITH->sf_level > 3 && send_pack)
		  selsize	+= header.packsize;
		else
		  selsize	+= header.size;

		if (dpflag && header.txlifetime > 0) {
		  newlt		= header.txlifetime - (clock_.ixtime - header.rxdate) / SECPERDAY;
		  if (newlt < 1)
		    newlt	= 1;
		  sprintf(hs + strlen(hs), " %d", newlt);
		}
		*WITH->lastsfcall = '\0';
	      } else {
		strcpy(WITH->lastsfcall, header.absender);
		sprintf(hs, "S%c %s < %s", msgchar, WITH->call, header.absender);
		if ((strchr(WITH->SID, 'M') != NULL || msgchar == 'B') && *header.id != '\0')
		  sprintf(hs + strlen(hs), " $%s", header.id);
	      }

	      strcpy(WITH->fbbprop[prop - 1].line, hs);
	      strcpy(WITH->fbbprop[prop - 1].brett, user[unr]->call);
	      WITH->fbbprop[prop - 1].nr	= k;
	      WITH->fbbprop[prop - 1].x_nr	= 0;
	      strcpy(WITH->fbbprop[prop - 1].bid, header.id);
	      WITH->fbbprop[prop - 1].mtype	= 'P';
	      prop++;
	      sel				= 1;
	      if (msgchar == 'B')
		is_what				= msg_bulletin;
	      else
		is_what				= msg_user;
	    }

	    k++;
	  } while (k <= lv && prop <= maxprop && selsize < maxp);
	  close_index(&list);
	}
      }
    }

    if (WITH->sf_level < 3) {
      if (WITH->action != 40) {
	if (sel > 0) {
	  wlnuser(unr, WITH->fbbprop[0].line);

	  switch (is_what) {

	  case msg_system:
	    if (WITH->sf_level == 2)
	      Result	= 2;
	    else
	      Result	= 1;
	    break;

	  default:
	    Result	= 1;
	    break;
	  }

	} else {
	  *WITH->lastsfcall	= '\0';
	  Result		= 0;
	}
      } else
	Result			= 4;

      boxspoolread();
    }
  }

  return Result;
}


static boolean sf_for(char *box)
{
  boolean	mail;
  indexstruct	*hpointer, header;
  short		list, k, lv;

  debug(4, 0, 126, box);

  mail		= false;

  lv		= sfsize(sflist) / sizeof(indexstruct);

  if (lv <= 0)
    return mail;

  hpointer	= &header;   /* bleibt im Zweifelsfall gleich */
  list		= open_index("X", FO_READ, true, false);

  if (list < minhandle)
    return mail;

  k		= lv;
  while (k > 0 && !mail) {
    read_index(list, k, &header);
    if (check_hcs(*hpointer)) {
      mail	= (!hpointer->deleted && !strcmp(hpointer->dest, box));
      if (mail)
	mail	= ((hpointer->pmode & 128) == 0 &&
		    not_while_connected(hpointer->absender, hpointer->betreff,
					hpointer->rxdate) &&
		   ((hpointer->msgflags & (MSG_LOCALHOLD | MSG_SFHOLD)) == 0 ||
		    clock_.ixtime - hpointer->rxdate > holddelay));
    }
    k--;
  }
  close_index(&list);
  return mail;
}


void change_sfentries(short von, short bis, char *forcall, char typ, char *newcall)
{
  indexstruct	header;
  short		list, k, lv;
  char		mtyp;
  char		w[256], w1[256];
  boolean	delet;

  debug(2, 0, 75, forcall);

  delet		= (strcmp(newcall, "NIL") == 0);
  lv		= sfsize(sflist) / sizeof(indexstruct);
  if (lv <= 0)
    return;

  list		= open_index("X", FO_RW, true, true);
  if (list >= minhandle) {
    k		= von;
    if (bis > lv)
      bis	= lv;

    while (k <= bis) {
      read_index(list, k, &header);
      if (!check_hcs(header))
	continue;

      strcpy(w, header.betreff);
      get_word(w, w1);

      mtyp	= header.msgtype;

      if (strlen(w1) == 1 && (w1[0] == 'M' || w1[0] == 'E'))
	mtyp	= 'S';
      else if (mtyp == 'A')
	mtyp	= 'P';
      else if (mtyp != 'P')
	mtyp	= 'B';

      if (typ == 'A' || typ == mtyp) {
	if (*forcall == '\0' || !strcmp(header.dest, forcall)) {
	  if (delet) {
	    header.deleted	= true;
	    x_garbage_waiting	= true;
	  } else
	    strcpy(header.dest, newcall);
	  write_index(list, k, &header);
	}
      }

      k++;
    }

  }
  close_index(&list);
}


static void routing_error(short unr, char condition, indexstruct header,
			  char *board, char *comment)
{
  char		hs[256], hs2[256], w[256];
  calltype	ucall;
  char		STR7[256];
  userstruct	*WITH;

  *hs		= '\0';
  if (*header.dest != '\0') {
    if (strcmp(header.dest, board))
      sprintf(hs, "(%s)", header.dest);
  }

  if (boxrange(unr))
    strcpy(ucall, user[unr]->call);
  else
    strcpy(ucall, "SYSTEM");

  sprintf(hs, "%s: %s%s@%s < %s $%s %s",
	  comment, board, strcpy(STR7, hs), header.verbreitung,
	  header.absender, header.id, header.betreff);
  cut(hs, 120);
  append_profile(unr, hs);

  if (condition == 'f')
    return;

  if (!boxrange(unr))
    return;

  WITH	= user[unr];
  if (!in_real_sf(WITH->call))
    return;

  sprintf(hs2, "%s%s%cERR", boxprotodir, WITH->call, extsep);
  if (!exist(hs2)) {
    get_btext(unr, 123, w);
    sprintf(w + strlen(w), " %s:", Console_call);
    append(hs2, w, true);
    get_btext(unr, 124, w);
    append(hs2, w, true);
    append(hs2, "---", true);
  }
  append(hs2, hs, true);
}


/* Rueckwirkend blockweise E/M - Eintraege loeschen oder ruecksetzen */

void del_emblocks(short unr, unsigned short undelete)
{
  short		list, lv;
  indexstruct	header;

  if (!boxrange(unr))
    return;

  lv		= sfsize(sflist) / sizeof(indexstruct);
  list		= open_index("X", FO_RW, true, true);
  if (list >= minhandle) {
    while (lv > 0) {
      read_index(list, lv, &header);
      if (check_hcs(header)) {
	if (!strcmp(header.dest, user[unr]->call)) {
	  if ((header.pmode & 16) != 0) {
	    if (undelete > 0) {
	      undelete--;
	      header.pmode	= 4;
	    } else
	      header.deleted	= true;
	    write_index(list, lv, &header);
	  }
	}
      }
      lv--;
    }

    close_index(&list);
    x_garbage_waiting	= true;
  }
  user[unr]->emblockct	= 0;
}


void reset_laterflag(short unr)
{
  short		list, lv, x;
  indexstruct	header;

  if (!boxrange(unr))
    return;

  lv		= sfsize(sflist) / sizeof(indexstruct);
  list		= open_index("X", FO_RW, true, true);
  if (list < minhandle)
    return;

  for (x = 1; x <= lv; x++) {
    read_index(list, x, &header);
    if (check_hcs(header)) {
      if ((header.pmode & 128) != 0) {
        if (!strcmp(header.dest, user[unr]->call)) {
	  if ((header.pmode & 128) != 0) {
	    header.pmode	&= ~128;
	    write_index(list, x, &header);
	  }
	}
      }
    }
  }
  user[unr]->laterflag	= false;
  close_index(&list);
}


static void set_reject_call(indexstruct *header, char *rc_)
{
  short		x;
  char		STR1[256];
  char		rc[256];

  strcpy(rc, rc_);
  rspacing(rc, LEN_CALL);
  sprintf(rc, "%%%s", strcpy(STR1, rc));
  if (strstr(header->readby, rc) != NULL)
    return;

  if (strlen(header->readby) < 134) {
    strcat(header->readby, rc);
    return;
  }

  x	= strpos2(header->readby, "!", 1);
  if (x > 0) {
    strdelete(header->readby, x, LEN_CALL+1);
    strcat(header->readby, rc);
  }
}


static void received_sf_sending(char condition, short unr, short prop, boolean abgelehnt)
{
  short		inr;
  indexstruct	header;
  short 	lv, list, rindex, newlt;
  boolean	inc_fwd, spec_sf, abg2, rerr;
  userstruct	*WITH;
  fbbproptype	*WITH1;
  pathstr	index;
  char		hs[256];
  char		uh[256];
  char		w1[256], w2[256];

  debug0(3, unr, 76);

  if (!boxrange(unr))
    return;

  WITH		= user[unr];
  abg2		= false;
  inc_fwd	= !abgelehnt;
  spec_sf	= false;
  rerr		= (condition == 'f');

  /* this is for the link test */
  stop_linkcheck(unr, prop);

  if (!abgelehnt) {

    switch (WITH->fbbprop[prop - 1].mtype) {

    case 'P':
      WITH->fstat_tx_p++;
      break;

    case 'S':
      WITH->fstat_tx_s++;
      break;

    default:
      WITH->fstat_tx_b++;
      break;
    }
  }

  rindex	= WITH->fbbprop[prop - 1].x_nr;
  if (rindex > 0) {
    lv		= sfsize(sflist) / sizeof(indexstruct);
    if (rindex <= lv) {
      list	= sfopen(sflist, FO_RW);
      if (list >= minhandle) {
	read_index(list, rindex, &header);
	if (check_hcs(header)) {
	  spec_sf	= ((header.pmode & 8) != 0);
	  header.deleted = true;
	  write_index(list, rindex, &header);
	}
	sfclose(&list);
	x_garbage_waiting = true;
      }
    }
  }

  strcpy(w1, WITH->fbbprop[prop - 1].brett);
  inr		= WITH->fbbprop[prop - 1].nr;
  if (inr > 0) {
    idxfname(index, w1);
    lv		= sfsize(index) / sizeof(indexstruct);
    if (inr > 0 && inr <= lv) {
      list	= sfopen(index, FO_RW);
      if (list >= minhandle) {
	read_index(list, inr, &header);
	if (check_hcs(header)) {
	  unhpath(header.verbreitung, uh);

	  if (rerr || (callsign(uh) && !spec_sf)) {
	    if (inc_fwd)
	      header.msgflags |= MSG_SFTX;

	    if (abgelehnt || rerr) {
	      if ((header.msgflags & MSG_PROPOSED) > 0 && condition == '-') {
		inc_fwd		= true;
		header.msgflags	&= ~MSG_PROPOSED;
	      } else if (condition == '-' &&
			 !strcmp(WITH->call, Console_call) &&
			 !strcmp(uh, Console_call)) {
		inc_fwd		= false;
		    /* doppelte Boxinstallation am selben Ort */
		header.msgflags	&= ~MSG_PROPOSED;
	      } else if (!rerr && WITH->fbbprop[prop - 1].x_nr == 0)
		inc_fwd		= false;   /*das sind die Userfile-Vorschlaege...*/
	      else {
		inc_fwd		= false;
		if (header.fwdct == 0 || condition != '-') {
		  abg2		= true;
		  if (condition != 'f') {
		    header.msgflags |= MSG_SFERR;
		    set_reject_call(&header, WITH->call);
		  }

		  switch (condition) {

		  case 'E':
		    strcpy(w2, "Syntax Error");
		    break;

		  case 'R':
		  case '\0':
		    header.msgflags |= MSG_REJECTED;
		    strcpy(w2, "Rejected");
		    break;

		  case '-':
		  case 'N':
		    header.msgflags |= MSG_DOUBLE;
		    strcpy(w2, "Double");
		    break;

		  case 'f':
		    strcpy(w2, "Read Error (file deleted)");
		    break;

		  default:
		    sprintf(w2, "Unknown answer (%c)", condition);
		    break;
		  }

		  switch (condition) {

		  case 'E':
		  case 'R':
		  case '-':
		  case 'N':
		    switch (condition) {

		    case 'E':
		      newlt = 3;
		      break;

		    case 'R':
		      newlt = returntime / SECPERDAY + 1;
		      break;

		    case '-':
		    case 'N':
		      newlt = 1;
		      break;

		    default:
		      newlt = 1;
		      break;
		    }
		    if (header.lifetime == 0 || header.lifetime > newlt) {
		      header.lifetime	= newlt;
		      sprintf(hs, "%d", header.lifetime);
		      alter_log(false, header.msgnum, header.msgflags, '#', hs);
		    }
		    break;

		  case 'f':
		    header.lifetime	= 1;
		    header.deleted	= true;
		    header.erasetime	= clock_.ixtime;
		    header.eraseby	= EB_SFERR;
		    add_eraseby(user[unr]->call, &header);
		    alter_log(false, header.msgnum, header.msgflags, 'E', "");
		    break;
		  }
		  routing_error(unr, condition, header, w1, w2);
		}
	      }
	    } else {
	      inc_fwd		= true;
	      header.msgflags	&= ~MSG_PROPOSED;
	      alter_log(false, header.msgnum, header.msgflags, '!', "");
	    }

	    if (!multiprivate) {
	      /*tocalls nur einmal forwarden, auch wenn mehrere Eintraege in der SF-Liste sind*/
	      strcpy(hs, "X");
	      delete_brett_by_bid(hs, "", header.id, false, true);
	    }

	    if (multiprivate) {
	      strcpy(hs, "X");
	      if (search_by_bid(hs, header.id, false) == 0) {
		if (!abg2) {
		  header.msgflags	&= ~MSG_SFWAIT;
		  if (!holdownfiles || strcmp(header.absender, Console_call)) {
		    if (strcmp(WITH->call, Console_call) ||
			strcmp(uh, Console_call)) {
		      header.deleted	= true;
		      header.erasetime	= clock_.ixtime;
		      header.eraseby	= EB_SF;
		      add_eraseby(user[unr]->call, &header);
		    }
		    alter_log(false, header.msgnum, header.msgflags, 'E', "");
		  } else {
		    header.msgflags	|= MSG_OWNHOLD;
		    alter_log(false, header.msgnum, header.msgflags, '!', "");
		  }
		}
	      }
	    } else {
	      if (!abg2) {
		header.msgflags		&= ~MSG_SFWAIT;
		if (!holdownfiles || strcmp(header.absender, Console_call)) {
		  if (strcmp(WITH->call, Console_call) ||
		      strcmp(uh, Console_call)) {
		    header.deleted	= true;
		    header.erasetime	= clock_.ixtime;
		    header.eraseby	= EB_SF;
		    add_eraseby(user[unr]->call, &header);
		  }
		  alter_log(false, header.msgnum, header.msgflags, 'E', "");
		} else {
		  header.msgflags	|= MSG_OWNHOLD;
		  alter_log(false, header.msgnum, header.msgflags, '!', "");
		}
	      }
	    }
	  } else {
	    header.msgflags	&= ~MSG_PROPOSED;
	    header.msgflags	|= MSG_SFTX;
	    strcpy(hs, "X");
	    if (search_by_bid(hs, header.id, false) == 0)
	      header.msgflags	&= ~MSG_SFWAIT;
	    alter_log(false, header.msgnum, header.msgflags, '!', "");
	  }

	  if (inc_fwd)
	    header.fwdct++;

	  write_index(list, inr, &header);
	}
	sfclose(&list);
      }
    }
  }

  WITH1			= &WITH->fbbprop[prop - 1];
  WITH1->crc		= 0;
  WITH1->pack		= false;
  WITH1->nr		= 0;
  WITH1->x_nr		= 0;
  WITH1->brett[0]	= '\0';
  WITH1->line[0]	= '\0';
  WITH1->bid[0]		= '\0';
  WITH1->mtype		= '\0';
  debug(3, unr, 76, "end");
}


static void set_msgflag(char *brett, short nr)
{
  short		k;
  indexstruct	header;
  pathstr	STR1;

  idxfname(STR1, brett);
  k	= sfopen(STR1, FO_RW);
  if (k < minhandle)
    return;

  read_index(k, nr, &header);
  if (check_hcs(header)) {
    header.msgflags	|= MSG_PROPOSED;
    write_index(k, nr, &header);
    alter_log(false, header.msgnum, header.msgflags, '!', "");
  }
  sfclose(&k);
}


static void get_title(char *brett, short nr, char *title)
{
  short		k;
  indexstruct	idx;
  pathstr	STR1;

  title[0]	= '\0';
  idxfname(STR1, brett);
  k		= sfopen(STR1, FO_READ);
  if (k < minhandle)
    return;

  read_index(k, nr, &idx);

  if (check_hcs(idx))
    strcpy(title, idx.betreff);
  else
    *title	= '\0';

  sfclose(&k);
}


void ok_sf_sending(short unr, short prop, long offset)
{
  short		kx;
  long		rps, nsize;
  char		*rp, *nmem;
  fbbproptype	*WITH;
  indexstruct	lheader;
  char		hs[256], title[256], rpa[301], temp2[256];

  debug0(3, unr, 77);

  if (!boxrange(unr))
    return;

  /* this is for the link check feature */
  start_linkcheck(unr, prop);

  WITH		= &user[unr]->fbbprop[prop - 1];
  WITH->line[0]	= '\0';

  if (*WITH->brett != '\0' && WITH->nr > 0) {
    set_msgflag(WITH->brett, WITH->nr);
    
    if (read_brett(unr, nohandle, WITH->brett, WITH->nr, WITH->nr, 100, "", "", offset, &lheader)
      	      	    != ERR_READBOARD) {
      return;
    }
    if (user[unr] != NULL) {
      received_sf_sending('f', unr, prop, true);
      abort_sf(unr, false, "file error");
    }
    return;
  }

  if (strcmp(WITH->brett, "E") && strcmp(WITH->brett, "M")) {
    abort_sf(unr, false, "msg not found 1");
    return;
  }
  
  title[0]	= '\0';
  if (user[unr]->sf_level < 2 || (user[unr]->sf_level > 2 && !packed_sf)) {
    get_title("X", WITH->x_nr, title);
    get_word(title, hs);
    wlnuser(unr, title);
    wlnuser(unr, "***sysfile");
    if (user[unr]->umode == UM_FILESF) {
      wlnuser(unr, "/EX");
    } else {
      chwuser(unr, 0x1a);
      wlnuser0(unr);
    }
    return;
  }
  
  if (user[unr]->sf_level <= 2) {
    abort_sf(unr, false, "msg not found 2");
    return;
  }
  
  /* zur Unterdrueckung der zu sendenden Daten im Verbindungsfenster */
  boxsetboxbin(user[unr]->pchan, true);
 
  get_title("X", WITH->x_nr, title);
  get_word(title, hs);	/* delete first word  */

  rp	= rpa;
  rps	= 0;
  nmem	= NULL;
  nsize	= 0;

  sprintf(temp2, "%sPCKEM657", tempdir);
  
  put_line(rp, &rps, " ");
  put_line(rp, &rps, "***sysfile");
  put_line(rp, &rps, " ");

  if (mempacker(false, true, rp, rps, &nmem, &nsize, temp2, true) != 0) {
    abort_sf(unr, false, "*** packerror");
    return;
  }
  
  if (nsize != 0) {
    rp	= nmem;
    rps	= nsize;
  } else {
    rps	= sfsize(temp2);
    kx	= sfopen(temp2, FO_READ);
    if (kx >= minhandle) {
      sfread(kx, rps, rp);
      sfclose(&kx);
      sfdelfile(temp2);
    }
  }

  send_pfbbram(user[unr]->fwdmode, unr, rp, rps, offset, title);

  if (nsize != 0)
    free(nmem);
}

static void change_filesf_direction(short unr);

void look_for_mail(short unr, boolean not_last, boolean immediate)
{
  short		x;
  boolean	nobbs;
  userstruct	*WITH;

  if (!boxrange(unr))
    return;

  WITH		= user[unr];

  switch (propose_sf_sending(unr)) {
  case 0:
    if (WITH->umode == UM_FILESF) {

      change_filesf_direction(unr);
      if (!boxrange(unr)) return;
      WITH->sf_to	= false;
      WITH->action	= 5;
      wlnuser(unr, "F>");

    } else if (needs_linkcheck(WITH->call, false)) {

      generate_linkcheck_data(unr);
      return;

    } else if (!not_last) {

      abort_sf(unr, false, "***done");

    } else {

      if (immediate || (WITH->sf_ptr != NULL && WITH->sf_ptr->no_sfpropdelay)) {
	WITH->sf_to	= false;
	WITH->action	= 5;
	wlnuser(unr, "F>");

      } else {

	switch (WITH->action) {

	case 205:
	  WITH->sf_to	= false;
	  WITH->action	= 5;
	  wlnuser(unr, "F>");
	  break;

	case 200:
	case 201:
	case 202:
	case 203:
	case 204:
	  WITH->action++;
	  break;

	default:   /* 30 Sekunden warten, dann nochmal probieren */
	  WITH->action	= 202;
/* wait for maximum of 2 minutes, if you want 3 minutes, uncomment the following */
/*	  WITH->action	= 200; */
	  nobbs		= true;
	  if (WITH->sf_ptr != NULL) {
	    for (x = 1; x <= MAXUSER; x++) {
	      if (x != unr && user[x] != NULL && user[x]->f_bbs)
		nobbs	= false;
	    }
	  }

	  if (nobbs) {
	    WITH->sf_to	= false;
	    WITH->action = 5;
	    wlnuser(unr, "F>");
	  }
	  break;
	}
      }
    }
    break;

  case 1:  /*bulletin*/
    WITH->sf_to		= true;
    WITH->action	= 3;
    break;

  case 2:  /*system*/
    WITH->sf_to		= true;
    WITH->action	= 30;
    break;

  case 3:  /*private diebox*/
    WITH->sf_to		= true;
    WITH->action	= 4;
    ok_sf_sending(unr, 1, 0);
    break;

  case 4:
    WITH->sf_to		= true;
    break;
  }

  if (boxrange(unr) && WITH->umode == UM_FILESF && WITH->sf_to == true) {
    box_input(unr, false, "OK", true);
    box_input(unr, false, ">", true);
  }
  
}



void send_fbb_proposals(short unr, boolean disc_if_none)
{
  short		x;
/*  boolean	nobbs; */
  userstruct	*WITH;
  char		crcstr[256];
  char		STR1[256];

  do {
  
    if (!boxrange(unr))
      return;

    WITH		= user[unr];
    boxsetboxbin(WITH->pchan, false);
    propose_sf_sending(unr);

    if (calc_prop_crc(unr, crcstr)) {
      for (x = 0; x < MAXFBBPROPS; x++) {
	if (*WITH->fbbprop[x].line != '\0')
	  wlnuser(unr, WITH->fbbprop[x].line);
      }
      sprintf(STR1, "F> %s", crcstr);
      wlnuser(unr, STR1);
      WITH->action	= 11;
      WITH->sf_to		= true;
      return;
    }

    if (tell_waiting) tell_check();
    
  } while (tell_waiting);

  if (needs_linkcheck(WITH->call, false)) {
    generate_linkcheck_data(unr);
    return;
  }

  if (disc_if_none) {
    switch (WITH->action) {

    case 305:
      wlnuser(unr, "FQ");
      abort_sf(unr, false, "");
      return;
      break;

    case 300:
    case 301:
    case 302:
    case 303:
    case 304:
      WITH->action++;
      break;

    default:   /* 30 sekunden warten, dann nochmal probieren */
/* the idle stuff is a mess at all for fbb forward. lets kick it off */
      wlnuser(unr, "FQ");
      abort_sf(unr, false, "");
      return;
      
/*
      WITH->action    = 300;

      nobbs		= true;
      if (WITH->sf_ptr != NULL && !WITH->sf_ptr->no_sfpropdelay) {
	for (x = 1; x <= MAXUSER; x++) {
	  if (x != unr && user[x] != NULL && user[x]->f_bbs) {
	    nobbs	= false;
	    break;
	  }
	}
      }

      if (nobbs) {
	wlnuser(unr, "FQ");
	abort_sf(unr, false, "");
	return;
      }
      break;
*/
    }

    return;
  }
  
  wlnuser(unr, "FF");
  WITH->action		= 20;
  WITH->sf_to		= false;
}


void prepare_for_next_fbb(short unr)
{
  short		x, hit;
  char	      	*p;
  char		msgtype;
  pathstr	hs, STR7;
  char		hs2[256];
  char		w[256], hw[256];

  debug0(3, unr, 78);

  if (!boxrange(unr))
    return;

  hs[0]		= '\0';
  hit		= 0;

  for (x = MAXFBBPROPS; x >= 1; x--) {
    if (*user[unr]->fbbprop[x - 1].line != '\0')
      hit	= x;
  }

  if (hit <= 0) {
    send_fbb_proposals(unr, false);
    return;
  }

  strcpy(hs, user[unr]->fbbprop[hit - 1].line);
  user[unr]->fbbprop[hit - 1].line[0] = '\0';
  get_word(hs, w);   /*FA/FB/FD*/
  get_word(hs, w);   /*B/P/A*/
  msgtype	= w[0];
  get_word(hs, w);   /*Absender*/
  sprintf(hs2, " < %s", w);
  get_word(hs, w);   /*mbx*/
  strcpy(hw, w);
  p   	      	= strchr(hw, '.');
  if (p != NULL)
    *p	      	= '\0';
  /*letzte Box prueft MyBBS*/
  if (strcmp(hw, Console_call))
    sprintf(hs2, " @ %s%s", w, strcpy(STR7, hs2));
  get_word(hs, w);   /*Ziel*/
  sprintf(hs2, "%s%s", w, strcpy(STR7, hs2));
  get_word(hs, w);   /*BID/MID*/
  strcpy(user[unr]->resumebid, w);
  /* damit ich hinterher beim Abbruch auch weiss,  */
  /* welche BID dieses File hatte                  */
  user[unr]->no_binpack = user[unr]->fbbprop[hit - 1].unpacked;
  sprintf(hs2 + strlen(hs2), " $%s", w);
  get_word(hs, w);   /*laenge*/
  get_word(hs, w);   /*Lifetime wenn vorhanden*/
  if (*w != '\0')
    sprintf(hs2 + strlen(hs2), " #%s", w);
  user[unr]->input2[0] = '\0';
  send_check(unr, hs2, false, msgtype);

  /* sind wir etwa im Resume-Mode am empfangen ? */

  if (*user[unr]->fbbprop[hit - 1].rname == '\0')
    return;

  /* ja, also File kopieren und unter dem Namen ablegen, wie   */
  /* er in SET_PACKSF erwartet wird                            */

  sprintf(hs, "%sBOXSF%c%d", tempdir, extsep, unr);
  sprintf(STR7, "%s%s", boxstatdir, user[unr]->fbbprop[hit - 1].rname);
  filecopy(STR7, hs);
}


/* FB B DL8HBS ALL ATARI $123456123456 9124 */

boolean check_prop_crc(short unr, char *eingabe_)
{
  char	eingabe[256];
  char	crcstr[256];

  strcpy(eingabe, eingabe_);
  if (calc_prop_crc(unr, crcstr)) {
    del_lastblanks(eingabe);
    if (*eingabe != '\0')
      return (strcmp(crcstr, eingabe) == 0);
    else
      return true;
  } else
    return false;
}


static boolean routing_ok(char *call, char *board, char *mbx)
{
  char	hs[256];

  unhpath(mbx, hs);
  if (callsign(hs) && strcmp(hs, Console_call))
    return (gen_sftest3(-1, call, board, mbx));
  else
    return true;
}


/* (Sx) DL8HBS @ DB0GR < DL7XYZ */

static boolean allowed_rlisf(short unr, char msgtype, char *fline_)
{
  short		k;
  boolean	from, b, p, ok, insfp, reject_it, db, tome;
  char	      	*p1;
  char		fline[256], sender[256], hs[256];
  char		board[256], mbx[256], bid[256], hmbx[256];
  char	      	rejectreason[256];

  strcpy(fline, fline_);
  get_word(fline, board);   /* to */
  upper(board);
  p1   	= strchr(board, '@');
  if (p1 != NULL) *p1 = '\0';
  if (!valid_boardname(board))
    return false;

  k	= strpos2(fline, "@", 1);
  if (k > 0) {
    strdelete(fline, 1, k);
    get_word(fline, mbx);
    upper(mbx);
  } else
    strcpy(mbx, Console_call);
  strdelete(fline, 1, strpos2(fline, "<", 1));
  get_word(fline, sender);
  upper(sender);
  get_word(fline, bid);
  if (*bid != '\0' && bid[0] == '$')
    strdelete(bid, 1, 1);
  else
    *bid = '\0';
    
  if (strlen(bid) > LEN_BID)
    return false;

  *hs	= '\0';

  reject_it = false;

  check_reject(user[unr]->call, msgtype, board, mbx, sender, hs, hs, bid, 0,
	       false, false, &reject_it, &db, rejectreason);

  if (reject_it)
    return false;

  unhpath(mbx, hmbx);
  tome	= !strcmp(Console_call, hmbx);
  ok	= false;
  insfp = user[unr]->sf_ptr != NULL && user[unr]->sf_ptr->usersf == false;

  if (sfinpdefault == SFI_ALL || insfp) {
  
    if (!strcmp(board, "W")) {
      ok  = (strchr(user[unr]->SID, 'W') != NULL && !strcmp(sender, user[unr]->call) && tome);
    } else ok = true;

  } else {

    if (sfinpdefault != SFI_NONE) {
      b	= false;
      p	= false;

      if (board[1] != '\0') { /* == if strlen board > 1 */
	if (callsign(board))
	  p = true;
	else
	  b = true;
      } else if (!strcmp(board, "T") && usersftellmode != SFT_NONE) {
	if ((unsigned)usersftellmode < 32 &&
	    ((1L << usersftellmode) & ((1L << SFT_OWN) | (1L << SFT_ALL))) != 0 &&
	    tome)
	  p = true;
	else if (usersftellmode == SFT_ALL)
	  b = true;
      }

      from = (strcmp(sender, user[unr]->call) == 0);
      switch (sfinpdefault) {

      case SFI_FROMPB:
	ok = ((p || b) && from);
	if (!ok && p && tome && user[unr]->direct_sf) {
	  user_mybbs(board, hs);
	  unhpath(hs, hs);
	  ok = !strcmp(Console_call, hs);
	}
	break;

      case SFI_FROMP:
	ok = (p && from);
	if (!ok && p && tome && user[unr]->direct_sf) {
	  user_mybbs(board, hs);
	  unhpath(hs, hs);
	  ok = !strcmp(Console_call, hs);
	}
	break;
      }
      if (ok)
	ok = routing_ok(user[unr]->call, board, mbx);
      if (create_usersflog && ok && !insfp)
	append_usersflog(unr, sender, board, mbx, bid, "  ");
    }
  }

  if (ok)
    ok = routing_ok(user[unr]->call, board, mbx);

  return ok;
}


/* FB B DL8HBS ALL ATARI $123456123456 9124 */

static boolean allowed_fbbsf(short unr, char *sender, char *board, char *mbx,
			     char *bid, char *size)
{
  boolean	from, b, tome;
  boolean	p, ok, insfp;
  char		hs[256], hmbx[256];

  if (!valid_boardname(board))
    return false;
  
  if (strlen(bid) > LEN_BID)
    return false;
    
  ok = false;
  unhpath(mbx, hmbx);
  tome	= !strcmp(Console_call, hmbx);
  insfp = user[unr]->sf_ptr != NULL && user[unr]->sf_ptr->usersf == false;

  if (sfinpdefault == SFI_ALL || insfp) {

    if (!strcmp(board, "W")) {
      ok  = (strchr(user[unr]->SID, 'W') != NULL && !strcmp(sender, user[unr]->call) && tome);
    } else ok = true;

  } else {

    if (sfinpdefault != SFI_NONE) {
      b = false;
      p = false;
      from = (strcmp(sender, user[unr]->call) == 0);

      if (board[1] != '\0') { /* == if strlen board > 1 */
	if (callsign(board))
	  p = true;
	else
	  b = true;
      } else if (!strcmp(board, "T") && usersftellmode != SFT_NONE) {
	if ((unsigned)usersftellmode < 32 &&
	    ((1L << usersftellmode) & ((1L << SFT_OWN) | (1L << SFT_ALL))) != 0 && tome)
	  p = true;
	else if (usersftellmode == SFT_ALL)
	  b = true;
      }


      switch (sfinpdefault) {

      case SFI_FROMPB:
	ok = ((p || b) && from);
	if (!ok && p && tome && user[unr]->direct_sf) {
	  user_mybbs(board, hs);
	  unhpath(hs, hs);
	  ok = !strcmp(Console_call, hs);
	}
	break;

      case SFI_FROMP:
	ok = (p && from);
	if (!ok && p && tome && user[unr]->direct_sf) {
	  user_mybbs(board, hs);
	  unhpath(hs, hs);
	  ok = !strcmp(Console_call, hs);
	}
	break;
      }
    }
  }
  if (ok)
    ok = routing_ok(user[unr]->call, board, mbx);

  if (create_usersflog && ok && !insfp)
    append_usersflog(unr, sender, board, mbx, bid, size);

  return ok;
}


/* FB B DL8HBS ALL ATARI $123456123456 9124 */

void send_fbb_answer(short unr)
{
  char		answer[256];
  char		bid[256];
  char		hs[256], hs2[256];
  char		tmbx[256];
  char		tboard[256];
  char		tsender[256], tsize[256];
  char	      	rejectreason[256];
  short		propct, x, y, w, z;
  long		roffset;
  boolean	ok, no_sf, double_prop, disk_full_abort;
  bidchecktype	bidcheck;
  bidarrtype	bidarr;
  boolean	extended_proto, reject_it;
  char		mtype;
  userstruct	*WITH;

  debug0(3, unr, 79);
  disk_full_abort = false;

  if (!boxrange(unr))
    return;

  WITH	= user[unr];

  extended_proto = ((unsigned)WITH->fwdmode < 32 && ((1L << WITH->fwdmode) & 0x38) != 0);

  ok	= false;
  strcpy(answer, "FS ");

  propct = 0;
  for (x = 0; x < MAXFBBPROPS; x++) {
    bidarr[x][0] = '\0';
    if (*WITH->fbbprop[x].line != '\0') {
      propct++;
      strcpy(hs, WITH->fbbprop[x].line);
      for (y = 1; y <= 6; y++)
	get_word(hs, hs2);
      cut(hs2, LEN_BID);   /* 6. Wort ist die BID */
      strcpy(bidarr[x], hs2);
      strcpy(WITH->fbbprop[x].bid, hs2);
    }
  }

  multiple_bullcheck(propct, bidcheck, bidarr);

  for (x = 1; x <= propct; x++) {
    WITH->fbbprop[x - 1].rname[0] = '\0';

    if (count_words(WITH->fbbprop[x - 1].line) == 7 ||
	(count_words(WITH->fbbprop[x - 1].line) == 8 &&
	 strchr(WITH->SID, 'D') != NULL)) {
      if (bidcheck[x - 1]) {
	strcpy(hs, WITH->fbbprop[x - 1].line);
	for (y = 1; y <= 2; y++)
	  get_word(hs, hs2);
	mtype = hs2[0];
	get_word(hs, tsender);   /* 3. Wort ist der Absender */
	cut(tsender, LEN_CALL);
	get_word(hs, tmbx);   /* 4. Wort ist @mbx */
	cut(tmbx, LEN_MBX);
	add_hpath(tmbx);
	get_word(hs, tboard);
	cut(tboard, LEN_BOARD);   /* 5. Wort ist die Zielrubrik */

	get_word(hs, hs2);
	cut(hs2, LEN_BID);
	strcpy(bid, hs2);
	get_word(hs, tsize);

	*hs = '\0';
	reject_it = false;
	no_sf = false;
	check_reject(WITH->call, mtype, tboard, tmbx, tsender, hs, hs, bid,
		     atol(tsize), false, false, &reject_it, &no_sf, rejectreason);

	if (!reject_it) {
	  double_prop = false;

	  for (w = 1; w <= MAXUSER; w++) {
	    if (user[w] != NULL) {
	      if (user[w]->f_bbs && !user[w]->sf_to) {
		for (z = 0; z < MAXFBBPROPS; z++) {
		  if (w != unr || z + 1 != x) {
		    if (double_prop == false)
		      double_prop = (strcmp(bid, user[w]->fbbprop[z].bid) == 0);
		  }
		}
	      }
	    }
	  }


	  if (double_prop) {
	    strcat(answer, "=");
	    WITH->fbbprop[x - 1].line[0]	= '\0';
	    WITH->fbbprop[x - 1].bid[0]		= '\0';
	  } else {
	    if (disk_full) {
	      strcat(answer, "=");
	      WITH->fbbprop[x - 1].line[0]	= '\0';
	      disk_full_abort			= true;

	    } else {

	      /* Testen, ob die Mail an den Sender zurueckgeroutet werden muesste, */
	      /* ob wir ueberhaupt einen Weg zum Ziel kennen und ob der Einspieler */
	      /* berechtigt ist, an das Ziel zu senden                             */

	      if (allowed_fbbsf(unr, tsender, tboard, tmbx, bid, tsize)) {

		/* Nachschauen, ob ein Resume-Fragment vorliegt */

		if (extended_proto) {  /* resume !? */
		  roffset	= check_resume(bidarr[x - 1], WITH->call,
						WITH->fbbprop[x - 1].rname);
		  if (roffset > 0) {  /* ja! */
		    sprintf(hs2, "%ld", roffset);
		    sprintf(answer + strlen(answer), "!%s", hs2);
		  } else {
		    if (no_sf)
		      strcat(answer, "H");
		    else
		      strcat(answer, "+");
		  }
		} else {
		  if (no_sf)
		    strcat(answer, "H");
		  else
		    strcat(answer, "+");
		}
		ok	= true;

	      } else {
      	      	append_rejectlog(unr, mtype, tsender, tboard, tmbx, bid, tsize, "no routing");
		if (extended_proto)
		  strcat(answer, "R");
		else
		  strcat(answer, "-");
		WITH->fbbprop[x - 1].line[0] = '\0';
	      }

	    }

	  }

	} else {
      	  append_rejectlog(unr, mtype, tsender, tboard, tmbx, bid, tsize, rejectreason);
	  if (extended_proto)
	    strcat(answer, "R");
	  else
	    strcat(answer, "-");
	  WITH->fbbprop[x - 1].line[0]	= '\0';
	}


      } else {  /*schon vorhanden*/
	strcat(answer, "-");
	WITH->fbbprop[x - 1].line[0]	= '\0';
      }

    } else if (*WITH->fbbprop[x - 1].line != '\0') {
      if (extended_proto)
	strcat(answer, "E");
      else
	strcat(answer, "-");
      WITH->fbbprop[x - 1].line[0]	= '\0';
    }
  }

  if (disk_full_abort) {
    abort_sf(unr, false, "DISK FULL");
    return;
  }

  if (!boxrange(unr))
    return;

  wlnuser(unr, answer);
  boxspoolread();

  /* nachschauen, ob eventuell von hier auch eine msg mit dieser BID an    */
  /* den Partner gehen soll und dann sofort loeschen...                    */

  strcpy(hs, "X");
  for (y = 0; y < propct; y++) {
    if (!bidcheck[y])   /* diese hier war doppelt       */
      delete_brett_by_bid(hs, WITH->call, bidarr[y], false, true);
  }

  if (!ok)
    send_fbb_proposals(unr, false);
  else
    prepare_for_next_fbb(unr);
}

static void set_laterflag(short unr, short x_nr)
{
  short		k;
  indexstruct	header;
  
  if (x_nr > 0) {
    k	= sfopen(sflist, FO_RW);
    if (k >= minhandle) {
      read_index(k, x_nr, &header);
      header.pmode	|= 128;   /* laterflag */
      write_index(k, x_nr, &header);
      sfclose(&k);
      user[unr]->laterflag = true;
    }
  }
}

void analyse_fbb_answer(short unr, char *eingabe_)
{
  short		z;
  userstruct	*WITH;
  char		eingabe[256], w[256];

  debug(3, unr, 80, eingabe_);

  strcpy(eingabe, eingabe_);
  
  if (!boxrange(unr))
    return;
    
  WITH	      	= user[unr];
  z		= 0;
  
  while (user[unr] != NULL && z < MAXFBBPROPS && *eingabe != '\0') {
    z++;

    switch (eingabe[0]) {

    case 'L':
    case '=':   /* take out of the current list...    */
      set_laterflag(unr, WITH->fbbprop[z - 1].x_nr);
      WITH->fbbprop[z - 1].nr		= 0;
      WITH->fbbprop[z - 1].x_nr	= 0;
      WITH->fbbprop[z - 1].brett[0]	= '\0';
      WITH->fbbprop[z - 1].line[0]	= '\0';
      WITH->fbbprop[z - 1].bid[0]	= '\0';
      break;

    case 'Y':
    case 'H':   /* other system will hold the file    */
    case '+':   /* send the file    	      	      */
      ok_sf_sending(unr, z, 0);
      break;

    case '!':   /* send with resume offset  	      */
      w[0]  	= '\0';
      while (strlen(eingabe) > 1 && isdigit(eingabe[1])) {
	sprintf(w + strlen(w), "%c", eingabe[1]);
	strdelete(eingabe, 1, 1);
      }
      ok_sf_sending(unr, z, atol(w));
      break;

    /* 'E','R','-','N' : */
    default:	  /* delete proposal  	      	      */
      received_sf_sending(eingabe[0], unr, z, true);
      break;

    }
    strdelete(eingabe, 1, 1);
  }

  if (user[unr] != NULL)
    WITH->action = 20;
}



void set_packsf(short unr)
{
  userstruct	*WITH;
  binsftyp	*WITH1;
  pathstr	STR1;

  if (!boxrange(unr))
    return;

  WITH = user[unr];
  if (WITH->binsfptr == NULL) {
    abort_sf(unr, false, "no binsfptr 2");
    return;
  }
  boxsetrwmode(WITH->pchan, RW_FBB2);
  WITH->binsfptr->blockcounter = 0;

  WITH1 = WITH->binsfptr;

  sprintf(WITH1->wname, "%d", unr);
  sprintf(STR1, "%sBOXSF%c%s", tempdir, extsep, WITH1->wname);
  strcpy(WITH1->wname, STR1);

  WITH1->wchan = nohandle;

  /* Wenn alles geklappt hat, dann haben wir ein Resume-Fragment   */
  /* hier liegen, wenn der TX-Offset > 0 ist                       */

  if (WITH1->offset > 0)
    WITH1->wchan = sfopen(WITH1->wname, FO_RW);

  /* Entweder war da nix oder wir sind nicht im Resume-Modus. Dann */
  /* wird das File neu erzeugt. Wenn wir im Resume sind, ist das   */
  /* ein Fehler, der wird aber in CHECK_FIRSTSIX abgefangen        */

  if (WITH1->wchan < minhandle)
    WITH1->wchan = sfcreate(WITH1->wname, FC_FILE);
  if (WITH1->wchan < minhandle) {
    abort_sf(unr, true, "no handle");
    return;
  }

  WITH1->checksum = 0;
  WITH1->rxbytes = 0;
  WITH1->validbytes = 0;
}


static boolean check_firstsix(short unr)
{
  long		err;
  firstsixtype	fs2;
  short		x;
  binsftyp	*WITH;

  if (!boxrange(unr))
    return false;

  if (user[unr]->binsfptr == NULL)
    return false;

  WITH = user[unr]->binsfptr;
  sfseek(0, WITH->wchan, SFSEEKSET);   /* an den Anfang des Files */
  sfread(WITH->wchan, sizeof(firstsixtype), fs2);
  err = sfseek(WITH->offset, WITH->wchan, SFSEEKSET);
      /* an die aktuelle Schreibmarke */
  if (err != WITH->offset)
    return false;

  for (x = 0; x <= 5; x++) {
    if (fs2[x] != WITH->firstsix[x])
	return false;
  }
  return true;
}



static void del_binsfptr(short unr)
{

  if (!boxrange(unr))
    return;

  if (user[unr]->binsfptr != NULL)
    free(user[unr]->binsfptr);
  user[unr]->binsfptr = NULL;
}


static unsigned short dpi16integer(unsigned short l)
{
  return l;
}


static void add_bpacksf(short unr)
{
  boolean crc_ok;
  short mode, k;
  unsigned short soll_crc;
  short x;
  char w[256], hs[256];
  boolean abort;
  char *puffer;
  long size;
  char *nmem;
  long nsize;
  userstruct *WITH;
  char STR1[256];
  short FORLIM;

  debug0(4, unr, 82);
  puffer = NULL;
  abort = false;
  if (!boxrange(unr))
    return;

  WITH = user[unr];
  /* nachschauen, ob da ein Fragment rumliegt, wenn ja, dann weg damit! */
  del_resume(WITH->resumebid, WITH->call);

  if (WITH->binsfptr == NULL)
    return;
  mode = WITH->fwdmode;
  crc_ok = (WITH->binsfptr->checksum == 0);

  /* quasi die Grundvoraussetzung */

  if (crc_ok) {
    sprintf(w, "%d", unr);
    sprintf(w, "%sDECSF%c%s", tempdir, extsep, strcpy(STR1, w));
    size = sfsize(WITH->binsfptr->wname);

    if (size > 0) {
      if ((unsigned)mode < 32 && ((1L << mode) & 0x38) != 0)
	size -= 2;

      if (size <= maxram())
	puffer = malloc(size);


      if (puffer != NULL) {
	k = sfopen(WITH->binsfptr->wname, FO_READ);
	if (k >= minhandle) {
	  if ((unsigned)mode < 32 && ((1L << mode) & 0x38) != 0) {
	    sfread(k, 2, (char *)(&soll_crc));
	    /* CRC umdrehen */
	    soll_crc = dpi16integer(soll_crc);
	  }
	  sfread(k, size, puffer);
	  sfclose(&k);
	  sfdelfile(WITH->binsfptr->wname);

	  /* CRC testen    */
	  if ((unsigned)mode < 32 && ((1L << mode) & 0x38) != 0) {
	    WITH->binsfptr->crcsum = 0;

	    if ((unsigned)mode < 32 && ((1L << mode) & 0x30) != 0) {
	      FORLIM = strlen(WITH->binsfptr->fbbtitle);
	      for (x = 0; x < FORLIM; x++)
		crcfbb(WITH->binsfptr->fbbtitle[x], &WITH->binsfptr->crcsum);
	    }

	    crcfbb_buf(puffer, size, &WITH->binsfptr->crcsum);
	    crc_ok = (WITH->binsfptr->crcsum == soll_crc);
	  }


	  if (crc_ok) {
	    nmem = NULL;
	    if (WITH->no_binpack) {
	      sfwrite(WITH->sendchan, size, puffer);
	      free(puffer);
	      del_binsfptr(unr);
	      strcpy(w, "\032");
	      send_text3(unr, false, w, true);
	    } else {
	      if (mempacker(false, false, puffer, size, &nmem, &nsize, w, true) == 0) {
		free(puffer);
		puffer = NULL;
		if (nmem == NULL)
		  app_file(w, WITH->sendchan, true);
		else {
		  sfwrite(WITH->sendchan, nsize, nmem);
		  free(nmem);
		  nmem = NULL;
		}
		del_binsfptr(unr);
		strcpy(w, "\032");
		send_text3(unr, false, w, true);
	      } else
		abort = true;
	    }
	  } else
	    abort = true;
	} else
	  abort = true;
	/* Soll-CRC auslesen */

      } else {
	crc_ok = false;
	if ((unsigned)mode < 32 && ((1L << mode) & 0x38) != 0) {
	  k = sfopen(WITH->binsfptr->wname, FO_READ);
	  if (k >= minhandle) {
	    sfread(k, 2, (char *)(&soll_crc));
	    sfclose(&k);
	    /* CRC umdrehen */
	    soll_crc = dpi16integer(soll_crc);
	    strcpy(hs, w);
	    validate(hs);
	    filecut(WITH->binsfptr->wname, hs, 2, 0);
	    strcpy(WITH->binsfptr->wname, hs);
	    if (mode == 3)
	      crc_ok = (file_crc(3, hs, 0, 0, 0) == soll_crc);
	    else
	      crc_ok = (file_crc(3, hs, WITH->binsfptr->crcsum, 0, 0) == soll_crc);
	  }
	} else
	  crc_ok = true;

	if (crc_ok) {
	  if (WITH->no_binpack) {
	    app_file(WITH->binsfptr->wname, WITH->sendchan, true);
	    strcpy(w, "\032");
	    send_text3(unr, false, w, true);
	  } else {
	    if (packer(false, false, false, WITH->binsfptr->wname, w, true) == 0) {
	      sfdelfile(WITH->binsfptr->wname);
	      del_binsfptr(unr);
	      app_file(w, WITH->sendchan, true);
	      strcpy(w, "\032");
	      send_text3(unr, false, w, true);
	    } else
	      abort = true;
	  }
	} else
	  abort = true;
      }

    } else {  /* leeres File (nur Titel) */
      sfdelfile(WITH->binsfptr->wname);
      del_binsfptr(unr);
      strcpy(w, "\032");
      send_text3(unr, false, w, true);
    }


    if (!abort)
      return;

    if (puffer != NULL)
      free(puffer);
    if (crc_ok)
      strcpy(w, "*** Decode-Error");
    else
      strcpy(w, "*** CRC-Error");
    sfdelfile(WITH->binsfptr->wname);
    del_binsfptr(unr);
    str2file(&WITH->sendchan, w, true);
    abort_sf(unr, false, w);
    return;
  }

  strcpy(w, "*** Checksum-Error");
  sfdelfile(WITH->binsfptr->wname);
  del_binsfptr(unr);
  str2file(&WITH->sendchan, w, true);
  abort_sf(unr, false, w);
}


/* Hier kommt der Titel von FBB an */

static void title_ok(short unr)
{
  short       	    x, mode;
  binsftyp    	    *WITH;
  char	      	    hs[256];

  mode	      	    = user[unr]->fwdmode;
  WITH	      	    = user[unr]->binsfptr;
  WITH->offset	    = 0;
  WITH->checksum    = 0;
  WITH->crcsum	    = 0;
  WITH->validbytes  = 0;
  WITH->blockcrc    = 0;

  if (mode == 2) {   /* HBS alt */
    for (x = 0; x < WITH->fbbtitlelen; x++)
      crcthp(WITH->fbbtitle[x], &WITH->crcsum);
  }

  if ((unsigned)mode < 32 && ((1L << mode) & 0x38) != 0)
  {  /* Offset extrahieren */
    x 	      	    = 1;
    while (WITH->fbbtitle[x - 1] != '\0' && x < WITH->fbbtitlelen)
      x++;
    if (WITH->fbbtitle[x - 1] == '\0') {
      hs[0]   	    = '\0';
      x++;
      while (WITH->fbbtitle[x - 1] != '\0' && x < WITH->fbbtitlelen) {
	if (WITH->fbbtitle[x - 1] != ' ')
	  sprintf(hs + strlen(hs), "%c", WITH->fbbtitle[x - 1]);
	x++;
      }

      WITH->offset  = atol(hs);
    }
  }
  
  box_txt2(false, unr, WITH->fbbtitle);
}

void fbbpack2(short unr, unsigned short infosize, unsigned short *infstart, char *info)
{
  short       	  ct, hx, hy, hlen, oinfs;
  unsigned short  hsize;
  long	      	  count;
  binsftyp    	  *WITH;

  debug0(4, unr, 83);
  hsize       	      	      = infosize - *infstart + 1;
  if (!boxrange(unr)) {
    *infstart 	      	      = SHORT_MAX;
    abort_sf(unr, true, "invalid unr");
    return;
  }
  count       	      	      = get_cpuusage();
  oinfs       	      	      = *infstart;
  if (user[unr]->binsfptr == NULL) {
    *infstart 	      	      = SHORT_MAX;
    abort_sf(unr, true, "no binsfptr");
    return;
  }
  WITH	      	      	      = user[unr]->binsfptr;
  box_ttouch(unr);
  if (hsize > 0) {
  
    if (WITH->blockcounter == BCMAGIC2) {
      hlen    	      	      = info[*infstart - 1];
      hx      	      	      = hsize;
      if (hx >= hlen + 1)
	hy    	      	      = hlen;
      else
	hy    	      	      = hx - 1;
      for (ct = 1; ct <= hy; ct++)
	WITH->fbbtitle[ct - 1] = info[*infstart - 1 + ct];
      WITH->fbbtitle[hy]      = '\0';
      WITH->fbbtitlelen       = hy;
      if (hx >= hlen + 1) {
	*infstart     	      += hlen + 1;
	WITH->blockcounter    = 0;
	title_ok(unr);
      } else {
	*infstart     	      += hsize;
	WITH->blockcounter    = hlen - hx + 1;
      }
    }
    
    else if (WITH->blockcounter == BCMAGIC1) {
      if (info[*infstart - 1] != 1) {
	*infstart     	      = SHORT_MAX;
	abort_sf(unr, false, "invalid block identifier");
	return;
      }
      if (hsize == 1) {
        *infstart     	      += 1;
      	WITH->blockcounter    = BCMAGIC2;
      }
      else {
	hlen  	      	      = info[*infstart];
	hx    	      	      = hsize;
	if (hx >= hlen + 2)
	  hy  	      	      = hlen;
	else
	  hy  	      	      = hx - 2;
	for (ct = 1; ct <= hy; ct++)
	  WITH->fbbtitle[ct - 1] = info[*infstart + ct];
	WITH->fbbtitle[hy]    = '\0';
	WITH->fbbtitlelen     = hy;
	if (hx >= hlen + 2) {
	  *infstart   	      += hlen + 2;
	  WITH->blockcounter  = 0;
	  title_ok(unr);
	} else {
	  *infstart   	      += hsize;
	  WITH->blockcounter  = hlen - hx + 2;
	}
      }
    }
    
    else if (WITH->blockcounter > 0) {
      hy      	      	      = WITH->blockcounter;
      if (hy > hsize)
	hy    	      	      = hsize;
      hx      	      	      = WITH->fbbtitlelen;
      for (ct = 1; ct <= hy; ct++)
	WITH->fbbtitle[ct + hx - 1] = info[*infstart + ct - 2];
      WITH->fbbtitle[hx + hy] = '\0';
      WITH->fbbtitlelen       = hx + hy;
      WITH->blockcounter      -= hy;
      *infstart       	      += hy;
      if (WITH->blockcounter == 0)
	title_ok(unr);
    }
    
    else {
      *infstart       	      = SHORT_MAX;
      abort_sf(unr, true, "invalid bcount");
      return;
    }

  }
  upd_statistik(unr, *infstart - oinfs, 0, count, get_cpuusage());
}


static void put_data(short unr, unsigned short infosize,
		     unsigned short *infstart, char *info)
{
  short       	  z;
  boolean     	  error;
  unsigned short  y;
  unsigned short  hsize;
  char	      	  *buff;
  binsftyp    	  *WITH;

  buff	      	  = (char *)(&info);
  error       	  = false;
  WITH	      	  = user[unr]->binsfptr;
  hsize       	  = infosize - *infstart + 1;
  if (hsize > WITH->blockcounter)
    hsize     	  = WITH->blockcounter;

  /* Die ersten 6 Bytes muessen gleich den bereits empfangenen */
  /* sein, wenn es sich um eine RESUME - Aussendung handelt    */

  if (WITH->offset > 0 && WITH->rxbytes < 6) {
    y 	      	  = 6 - WITH->rxbytes;
    if (y > hsize)
      y       	  = hsize;

    for (z = 1; z <= y; z++)
      WITH->firstsix[z + WITH->rxbytes - 1] = info[z + *infstart - 2];

    if (y + WITH->rxbytes == 6) {
      error   	  = !check_firstsix(unr);
      if (!error) {
	buff  	  = &info[*infstart - 1];
	WITH->rxbytes = WITH->offset - y;
	if (hsize - y > 0) {
	  if (WITH->wchan < minhandle) {
	    abort_sf(unr, true, "no handle");
	    *infstart = SHORT_MAX;
	    return;

	  }
	  sfwrite(WITH->wchan, hsize - y, (char *)(&buff[y]));
	}
      }

    }

  } else {
    buff      	  = &info[*infstart - 1];
    if (buff != NULL && hsize > 0) {
      if (WITH->wchan < minhandle) {
	abort_sf(unr, true, "no handle");
	*infstart = SHORT_MAX;
	return;
      }
      sfwrite(WITH->wchan, hsize, buff);
    }
  }

  if (!error) {
    switch (user[unr]->fwdmode) {

    case 1:
    case 3:
    case 4:
      checksum8_buf(buff, hsize, &WITH->checksum);
      WITH->blockcounter -= hsize;
      break;

    case 5:
      checksum8_buf(buff, hsize, &WITH->checksum);
      crcfbb_buf(buff, hsize, &WITH->blockcrc);
      WITH->blockcounter -= hsize;
      if (WITH->blockcounter == 0)
	WITH->blockcounter = BCMAGIC4;
      break;

    case 2:
      crcthp_buf(buff, hsize, &WITH->crcsum);
      WITH->blockcounter -= hsize;
      break;
    }

    *infstart 	  += hsize;
    WITH->rxbytes += hsize;
    return;
  }

  *infstart   	  = SHORT_MAX;
  del_resume(user[unr]->resumebid, user[unr]->call);
  abort_sf(unr, false, "*** unmatching CRC in resume, file deleted");
}


static void add_it(short unr)
{
  binsftyp   *WITH;

  WITH	= user[unr]->binsfptr;
  sfclose(&WITH->wchan);
  WITH->blockcounter = 0;
  boxsetrwmode(user[unr]->pchan, RW_NORMAL);
  add_bpacksf(unr);
}


/* Hier kommen die Daten von FBB an */

void fbb2pack2(short unr, unsigned short infosize, unsigned short *infstart, char *info)
{
  short       	  oinfs, mode;
  unsigned short  isize;
  long	      	  count;
  binsftyp    	  *WITH;

  debug0(4, unr, 84);
  
  if (!boxrange(unr)) {
    *infstart 	      	    = SHORT_MAX;
    return;
  }
  
  if (user[unr]->binsfptr == NULL)
    return;
    
  WITH	      	      	    = user[unr]->binsfptr;
  count       	      	    = get_cpuusage();
  oinfs       	      	    = *infstart;
  box_ttouch(unr);
  isize       	      	    = infosize - *infstart + 1;

  if (isize > 0) {
    mode      	      	    = user[unr]->fwdmode;

    if (WITH->blockcounter == 0) {
      if (info[*infstart - 1] == 2) {
	WITH->blockcrc	    = 0;
	WITH->validbytes = WITH->rxbytes;
	if (isize > 1) {
	  WITH->blockcounter = info[*infstart];
	  if (WITH->blockcounter == 0)
	    WITH->blockcounter = 256;
	  *infstart   	    += 2;
	  put_data(unr, infosize, infstart, info);
	  if (*infstart == SHORT_MAX)
	    return;
	} else {
	  WITH->blockcounter = BCMAGIC1;
	  (*infstart)++;
	}
      } else if (info[*infstart - 1] == 4) {
	if (isize > 1) {
	  if (mode == 2) {
	    if (isize > 2) {
	      WITH->crc2    = 0;
	      WITH->crc2    = info[*infstart + 1];
	      WITH->crc2    = (WITH->crc2 << 8) + info[*infstart];
	      if (WITH->crcsum == WITH->crc2)
		WITH->checksum = 0;
	      else
		WITH->checksum = 1;
	      *infstart     += 3;
	      add_it(unr);
	    } else {
	      WITH->crc2    = 0;
	      WITH->crc2    = info[*infstart];
	      *infstart     += 2;
	      WITH->blockcounter = BCMAGIC3;
	    }
	  } else {
	    WITH->checksum  = (WITH->checksum + info[*infstart]) & 0xff;
	    *infstart += 2;
	    add_it(unr);
	  }
	} else {
	  WITH->blockcounter = BCMAGIC2;
	  (*infstart)++;
	}
      } else {
	*infstart     	    = SHORT_MAX;
	abort_sf(unr, false, "invalid block identifier");
	return;
      }


    } else if (WITH->blockcounter == BCMAGIC1) {
      WITH->blockcounter    = info[*infstart - 1];
      if (WITH->blockcounter == 0)
	WITH->blockcounter  = 256;
      (*infstart)++;
      put_data(unr, infosize, infstart, info);
      if (*infstart == SHORT_MAX)
	return;
    } else if (WITH->blockcounter == BCMAGIC2) {
      if (mode == 2) {
	WITH->crc2    	    = 0;
	WITH->crc2    	    = info[*infstart];
	WITH->crc2    	    = (WITH->crc2 << 8) + info[*infstart - 1];
	if (WITH->crcsum == WITH->crc2)
	  WITH->checksum    = 0;
	else
	  WITH->checksum    = 1;
	(*infstart)++;
	(*infstart)++;
	add_it(unr);
      } else {
	WITH->checksum	    = (WITH->checksum + info[*infstart - 1]) & 0xff;
	(*infstart)++;
	add_it(unr);

      }
    } else if (WITH->blockcounter == BCMAGIC3) {
      WITH->crc2      	    += info[*infstart - 1] << 8;
      if (WITH->crcsum == WITH->crc2)
	WITH->checksum	    = 0;
      else
	WITH->checksum	    = 1;
      (*infstart)++;
      add_it(unr);
    } else if (WITH->blockcounter == BCMAGIC4) {
      if (isize > 1) {
	WITH->crc2    	    = 0;
	WITH->crc2    	    = info[*infstart];
	WITH->crc2    	    = (WITH->crc2 << 8) + info[*infstart - 1];
	if (WITH->crc2 != WITH->blockcrc) {
	  *infstart   	    = SHORT_MAX;
	  abort_sf(unr, false, "invalid block crc");
	  return;
	}
	WITH->blockcounter  = 0;
	WITH->blockcrc	    = 0;
	*infstart     	    += 2;
      } else {
	WITH->crc2    	    = 0;
	WITH->crc2    	    = info[*infstart - 1];
	(*infstart)++;
	WITH->blockcounter  = BCMAGIC5;
      }
    } else if (WITH->blockcounter == BCMAGIC5) {
      WITH->crc2      	    += info[*infstart - 1] << 8;
      if (WITH->crc2 != WITH->blockcrc) {
	*infstart     	    = SHORT_MAX;
	abort_sf(unr, false, "invalid block crc");
	return;
      }
      WITH->blockcounter  = 0;
      WITH->blockcrc  	  = 0;
      (*infstart)++;
    } else {
      put_data(unr, infosize, infstart, info);
      if (*infstart == SHORT_MAX)
	return;
    }

  }
  upd_statistik(unr, *infstart - oinfs, 0, count, get_cpuusage());
}


/* Schaut nach, ob das ein abgebrochener S&F war, ob der im FBB/DP - Resume -*/
/* Modus war, ob da mehr als 250 gueltige Daten gesendet wurden und legt     */
/* dann einen Pointer mit den entsprechenden Daten zum Wiederaufnehmen des   */
/* s&f an nachdem die empfangenen Daten gesichert worden sind                */

void check_frag_sf(short unr)
{
  resumemem   	  *hr;
  userstruct  	  *WITH;
  binsftyp    	  *WITH1;
  char	      	  hfragname[256];

  debug0(3, unr, 85);
  
  if (!boxrange(unr))
    return;
  
  WITH        = user[unr];
  if (!WITH->f_bbs)
    return;
    
  if (!((unsigned)WITH->fwdmode < 32 && ((1L << WITH->fwdmode) & 0x38) != 0))
    return;
    
  if (WITH->binsfptr == NULL)
    return;
    
  WITH1       = WITH->binsfptr;
  if (WITH1->wchan < minhandle)
    return;
    
  sfclose(&WITH1->wchan);
  if (WITH1->validbytes > 500) {
    WITH1->validbytes -= 500;
    hr	      = malloc(sizeof(resumemem));
    if (hr != NULL) {
      /* Eventuell vorhandenes altes Resume-Fragment   */
      /* loeschen, gleichfalls den Pointer im Speicher */
      del_resume(WITH->resumebid, WITH->call);
      sprintf(hfragname, "%sresume%c001", boxstatdir, extsep);
      validate(hfragname);
      filecut(WITH1->wname, hfragname, 0, WITH1->validbytes);
      del_path(hfragname);
      cut(WITH->resumebid, LEN_BID);
      strcpy(hr->rbid, WITH->resumebid);
      strcpy(hr->rcall, WITH->call);
      cut(hfragname, 12);
      strcpy(hr->rfname, hfragname);
      hr->rsize   = WITH1->validbytes;
      hr->rdate   = clock_.ixtime;
      hr->next	  = NULL;
      if (resume_root != NULL) {
	hr->next  = resume_root->next;
	resume_root->next = hr;
      } else
	resume_root = hr;
      write_resume();
    }

  }
  sfdelfile(WITH1->wname);
}


void abort_sf(short unr, boolean immediate, char *txt)
{
  char	      STR1[256];

  debug(1, unr, 86, txt);
  
  if (*txt != '\0') {
    sprintf(STR1, "S&F: %s", txt);
    boxprotokoll(STR1);
  }
  
  if (!boxrange(unr))
    return;

  sfclosedel(&user[unr]->sendchan);
  if (!immediate) {
    if (*txt != '\0')
      wlnuser(unr, txt);
    boxspoolread();
  } else
    abort_useroutput(unr);

  if (!boxrange(unr))
    return;

  user[unr]->action	= 15;   /* ABORT */
}


static void sfproterr2(short unr)
{
  abort_sf(unr, false, "***error");
}


void sfproterr(short unr)
{
  abort_sf(unr, true, "unexpected answer");
}


sfdeftype *find_sf_pointer(char *call)
{
  sfdeftype *sftptr;

  sftptr = sfdefs;
  while (sftptr != NULL) {
    if (!strcmp(sftptr->call, call))
      return sftptr;
    sftptr = sftptr->next;
  }
  return NULL;
}

boolean in_real_sf(char *call)
{
 sfdeftype *sftptr;

 sftptr = find_sf_pointer(call);
 if (sftptr == NULL) return false;
 return !sftptr->usersf;
}

static long calc_aclock(void)
{
  long h, m, s;

  utc_clock();   /* Uhr stellen */
  h = clock_.hour;
  m = clock_.min;
  s = clock_.sec;
  return (h * 3600 + m * 60 + s);
}


static void set_sf_timer(boolean routing, char *call, long timeout)
{
  sfdeftype *sftptr;

  sftptr = find_sf_pointer(call);
  if (sftptr == NULL)
    return;
  sftptr->lasttry = calc_aclock();
  if (routing)
    sftptr->in_routing = true;
  if (timeout < 1) timeout = 900;
  sftptr->timeout = clock_.ixtime + timeout;
}


void check_routing_timeouts(void)
{
  sfdeftype *sftptr;
  
  sftptr = sfdefs;
  while (sftptr != NULL) {
    if (sftptr->timeout < clock_.ixtime)
      sftptr->in_routing = false;
    sftptr = sftptr->next;
  }
}

/* Der Box wird vom Packet-Router mitgeteilt, dass der Connectversuch    */
/* erfolglos verlief bzw. beendet ist, da er zustande gekommen ist       */

void abort_routing(char *call_)
{
  sfdeftype *sftptr;
  char	    *p;
  char	    call[256];

  strcpy(call, call_);
  p   	  = strchr(call, '-');
  if (p != NULL) *p = '\0';
  sftptr  = find_sf_pointer(call);
  if (sftptr != NULL)
    sftptr->in_routing = false;
}


void kill_all_routing_flags(void)
{
  sfdeftype *sftptr;

  sftptr = sfdefs;
  while (sftptr != NULL) {
    sftptr->in_routing = false;
    sftptr = sftptr->next;
  }

  load_boxbcastparms(bcast_box);
}


static void unr_and_protokoll(short unr, char *s_)
{
  char	  s[256];

  strcpy(s, s_);
  debug(1, unr, 87, s);
  if (unr > 0)
    wlnuser(unr, s);
  boxprotokoll(s);
}


void close_filesf_output(short unr)
{
  char	      hs[256];

  if (!boxrange(unr)) return;

  if (user[unr]->fileout_handle >= minhandle) {
    /* this results in the output file name */
    handle2name(user[unr]->fileout_handle, hs);
    /* now close the output file (and don't delete) */
    sfclose(&user[unr]->fileout_handle);
    /* check if it only contains 0 bytes */
    if (sfsize(hs) < 1)
    /* then delete it... */
      sfdelfile(hs);
    /* this in the corresponding lock file */
    strcat(hs, ".lock");
    /* now delete the output lock handle */
    sfdelfile(hs);
  }
}

void close_filesf_input(short unr)
{
  char	      hs[256];

  if (!boxrange(unr)) return;

  if (user[unr]->sfilname[0] != '\0') {
    sfclose(&user[unr]->fsfinhandle);
    /* this is the input file name */
    strcpy(hs, user[unr]->sfilname);
    /* now delete the input file */
    sfdelfile(hs);
    /* and this the corresponding lock file */
    strcat(hs, ".lock");
    /* now delete the input lock file */
    sfdelfile(hs);
  }
}

static boolean check_for_singleliner(char *hs)
{
  char *p;
  
  if ((p = strstr(hs, "/EX")) != NULL && ((strstr(hs, "S M @ ") == hs) || strstr(hs, "S E @ ") == hs)) {
    *p++ = '\032';
    *p	 = '\0';
    return true;
  } else return false;
}

#define binbuf 1000
static long bincopy(short inh, short outh, long size, boolean dummy_write)
{ 
  long	count;
  char	buf[binbuf];
  
  while (size > 0) {
    count = size;
    if (count > binbuf) count = binbuf;
    if (sfread(inh, count, buf) != count) break;
    if (!dummy_write) {
      if (sfwrite(outh, count, buf) != count) break;
    }
    size -= count;
  }
  return size;
}
#undef binbuf

void do_filesf_input(short unr)
{
  boolean   eol;
  boolean   formaterr = false;
  boolean   got_ex = false;
  long	    binsize;
  pathstr   inp;
  char	    hs[256];

/* in this function, "changed_dir" is only used as a flag for a single line e/m message */

  if (!boxrange(unr)) return;
  
  strcpy(inp, user[unr]->spath);
  user[unr]->spath[0] = '\0';

  if (!user[unr]->changed_dir && strstr(inp, "OK") != NULL) {

    while (!got_ex && file2lstr2(user[unr]->fsfinhandle, hs, 254, &eol)) {
      if (!strcmp(hs, "/EX")) {
      	strcpy(hs, "\032");
	got_ex = true;
      }
      binsize = true_bin(hs);
      if (binsize >= 0) {
        clear_immediately_input(unr);
	if (!boxrange(unr)) return;
      	str2file(&user[unr]->sendchan, hs, true);
	if (bincopy(user[unr]->fsfinhandle, user[unr]->sendchan, binsize, false) == 0) {      
      	  strcpy(hs, "\032");
	  eol = true;
	  got_ex = true;
	} else {
	  *hs = '\0';
	  break;
	}
      }
      box_input(unr, false, hs, eol);
      if (!boxrange(unr)) return;
    }
    if (!got_ex) abort_sf(unr, false, "end of infile");

  } else if (!user[unr]->changed_dir && strstr(inp, "NO") != NULL) {

    while (!got_ex && file2lstr2(user[unr]->fsfinhandle, hs, 254, &eol)) {
      if (!strcmp(hs, "/EX")) {
	got_ex = true;
      }
      binsize = true_bin(hs);
      if (binsize >= 0) {
	if (bincopy(user[unr]->fsfinhandle, nohandle, binsize, true) == 0) {      
	  got_ex = true;
	} else {
	  break;
	}
      }
    }
    if (!got_ex) abort_sf(unr, false, "end of infile");
    else if (file2lstr2(user[unr]->fsfinhandle, hs, 254, &eol)) {
      formaterr = true;
      if (eol && hs[0] == 'S') {
      	formaterr = false;
      } else {
      	while ((!strcmp(hs, "/EX") || hs[0] == '\0') && eol) { /* sometimes, "/EX" is at end of file itself */
	  if (!file2lstr2(user[unr]->fsfinhandle, hs, 254, &eol)) {
	    eol = false;
	  } else if (eol) {
            if (hs[0] == 'S') {
	      formaterr = false;
	    } else {
	      del_lastblanks(hs);
	    }
	  }
	}
      }
      if (!formaterr) {
      	user[unr]->changed_dir = check_for_singleliner(hs);
      	box_input(unr, false, hs, true);
      } else {
      	append_profile(unr, "invalid format of sf input file");
      	abort_sf(unr, false, "invalid format of sf input file");
      }
    } else abort_sf(unr, false, "end of infile");

  } else if (strchr(inp, '>') != NULL) {
    user[unr]->changed_dir = false; /* this is a flag for single line messages, and it is now cleared */
    if (file2lstr2(user[unr]->fsfinhandle, hs, 254, &eol)) {
      formaterr = true;
      if (eol && hs[0] == 'S') {
      	formaterr = false;
      } else {
      	while ((!strcmp(hs, "/EX") || hs[0] == '\0') && eol) { /* sometimes, "/EX" is at end of file itself */
	  if (!file2lstr2(user[unr]->fsfinhandle, hs, 254, &eol)) {
	    eol = false;
	  } else if (eol) {
            if (hs[0] == 'S') {
	      formaterr = false;
	    } else {
	      del_lastblanks(hs);
	    }
	  }
	}
      }
      if (!formaterr) {
      	user[unr]->changed_dir = check_for_singleliner(hs);
      	box_input(unr, false, hs, true);
      } else {
      	append_profile(unr, "invalid format of sf input file");
      	abort_sf(unr, false, "invalid format of sf input file");
      }
    } else abort_sf(unr, false, "end of infile");
    
  } else abort_sf(unr, false, "invalid response");
}

static void change_filesf_direction(short unr)
{
  short       inhandle;
  
  close_filesf_output(unr);

  if (!boxrange(unr)) return;
  
  inhandle = sfopen(user[unr]->sfilname, FO_READ);
  
  user[unr]->spath[0] 	= '\0';
  user[unr]->fsfinhandle = inhandle;

  if (inhandle <= minhandle) {
    abort_sf(unr, false, "no input file");
  }  
}

static void start_filesf(short unr, char *call, char *args_)
{
  short       filesf, handle, fwdmode;
  pathstr     outfname, infname, lockinfile, lockoutfile;
  char	      hs[256], args[256], w[256];
  
  strcpy(args, args_);
  fwdmode = 0;
  get_word(args, w);
  upper(w);
  if (!strcmp(w, "FBB")) fwdmode = 1;
  get_word(args, outfname);
  get_word(args, infname);
  
  sprintf(hs, "opening forward to %s via files >%s <%s", call, outfname, infname);
  unr_and_protokoll(unr, hs);

  /* check and set a lockfile for output */
  strcpy(lockoutfile, outfname);
  strcat(lockoutfile, ".lock");
  if (!tas_lockfile(0, 30*60, lockoutfile)) {
    sprintf(hs, "have a lock file for %s", outfname);
    unr_and_protokoll(unr, hs);
    return;
  }
  
  /* check and set a lockfile for input */
  strcpy(lockinfile, infname);
  strcat(lockinfile, ".lock");
  if (!tas_lockfile(0, 30*60, lockinfile)) {
    sprintf(hs, "have a lock file for %s", infname);
    unr_and_protokoll(unr, hs);
    sfdelfile(lockoutfile);
    return;
  }
  
  /* now try to open/create the output forward file */
  handle = sfopen(outfname, FO_RW);
  if (handle < minhandle)
    handle = sfcreate(outfname, FC_FILE_RWGROUP);

  /* failure? -> exit */
  if (handle < minhandle) {
    sprintf(hs, "cannot open file %s", outfname);
    unr_and_protokoll(unr, hs);
    sfdelfile(lockoutfile);
    sfdelfile(lockinfile);
    return;
  }

  /* seek to end of file */
  sfseek(0, handle, SFSEEKEND);

  /* logon this user */
  filesf      = melde_user_an(call, 0, 0, UM_FILESF, false);
  if (!boxrange(filesf)) {
    sprintf(hs, "file forward to %s via file %s failed", call, outfname);
    unr_and_protokoll(unr, hs);
    sfclose(&handle);
    sfdelfile(lockoutfile);
    sfdelfile(lockinfile);
    return;
  }
  
  /* assign the filehandle */
  user[filesf]->fileout_handle = handle;
  
  /* note the input file name */
  strcpy(user[filesf]->sfilname, infname);
  
  /* kickstart the forward */
  if (fwdmode == 1) {
    box_input(filesf, false, "[DPFILESF-1.0-AHM$]", true);
  } else {
    box_input(filesf, false, "[DPFILESF-1.0-AD1HMW$]", true);
  }
  box_input(filesf, false, ">", true);
  box_input(filesf, false, ">", true);
}


void start_sf(short unr, char *call_, char *parameter_)
{
  sfcpathtype 	*hp;
  boolean     	fchk, tofile, toexec;
  short       	outtnc, k;
  sfdeftype   	*sfp;
  char	      	call[256], parameter[256], hs[256], w[256], fpath[256], qrgstr[256];

  debug(2, unr, 88, call_);
  
  strcpyupper(call, call_);
  strcpy(parameter, parameter_);
  fchk	    = false;
  cut(call, LEN_CALL);
  *qrgstr   = '\0';
  tofile    = false;
  toexec    = false;
  outtnc    = -1;
  *fpath    = '\0';
  hp  	    = NULL;

  sfp 	    = find_sf_pointer(call);
  if (sfp != NULL) {

    if (sfp->in_routing) {
      get_btext(unr, 125, hs);
      sprintf(w, "%s: %s", call, hs);
      unr_and_protokoll(unr, w);
      return;
    }

    for (k = 1; k <= 2; k++) {
      get_word(parameter, hs);
      if (*hs != '\0') {
	if (hs[0] == '#') {
	  if (hs[1] >= '0' && hs[1] <= '3')
	    outtnc  = hs[1] - '0';
	} else {
	  fchk	    = true;
	  strcpy(qrgstr, hs);
	}
      }
    }

    hp	      	    = sfp->cpathp;
    while (hp != NULL) {
      if (fchk) {
	if (strcmp(hp->qrg, qrgstr)) {
	  hp  	    = hp->next;
	  continue;
	}
      }
      if (!strcmp(hp->qrg, "FILE")) {
	tofile	    = true;
	fchk  	    = true;
	break;
      }
      if (!strcmp(hp->qrg, "EXTERNAL")) {
	toexec	    = true;
	fchk  	    = true;
	break;
      }
      if (find_socket(hp->qrg, &outtnc)) {
	fchk  	    = true;
	break;
      }
      hp      	    = hp->next;
    }
  }

  if (hp == NULL) {
    sprintf(hs, "%s: no path found", call);
    unr_and_protokoll(unr, hs);
    return;
  }
  
  sprintf(hs, "S&F: %s", hp->ccall);
  unr_and_protokoll(unr, hs);

  if (tofile) {
    set_sf_timer(false, call, atol(hp->timeout));
    start_filesf(unr, call, hp->extended_args);
    return;
  }

  else if (toexec) {

    strcpy(hs, hp->extended_args);
    get_word(hs, w);

    if (!exist(w)) {
      sprintf(hs, "%s: cannot open connection, <%s> not existing", hp->ccall, w);
      unr_and_protokoll(unr, hs);
      return;
    }

    set_sf_timer(true, call, atol(hp->timeout));
    sprintf(hs, "%s %s %s %s", w, hp->ccall, sf_connect_call, hp->timeout);
    add_zombie(my_exec(hs), "", 0);
    return;
  }
  
  strcpy(hs, hp->ccall);
  if (hp->timeout[0] != '\0') {
    strcat(hs, " ");
    strcat(hs, hp->timeout);
  }

  if (boxpboxsf(outtnc, sf_connect_call, hs))
    set_sf_timer(true, call, atol(hp->timeout));
  else {
    sprintf(hs, "%s: cannot open connection", hp->ccall);
    unr_and_protokoll(unr, hs);
  }

}


static short total_connects(char *call)
{
  short i, x;

  i = 0;
  for (x = 1; x <= MAXUSER; x++) {
    if (user[x] != NULL) {
      if (!strcmp(user[x]->call, call))
	i++;
    }
  }
  return i;
}


static long last_sftimer    = 0;


void check_sftimer(void)
{
  sfdeftype *sftptr;
  long aclock, ival, ival2, zsum1, zsum2;
  boolean has_mail, try, try1, try2, do_linkcheck;
  short x;
  char w[256];
  boolean tcheck;
  sfdeftype *WITH;

  aclock = calc_aclock();
  if (labs(aclock - last_sftimer) <= 30)
    return;
  debug0(4, 0, 125);
  last_sftimer = aclock;
  has_mail = false;
  sftptr = sfdefs;

  while (sftptr != NULL) {
    WITH = sftptr;
    try = true;
    x = 1;
    while (x <= MAXUSER && try) {  /* Box schon eingeloggt? */
      if (user[x] != NULL) {
	if (user[x]->f_bbs) {
	  if (!strcmp(user[x]->call, WITH->call)) {
	    if (user[x]->sf_level >= 3) {
	      try = (total_connects(WITH->call) < 2 &&
		     clock_.ixtime - user[x]->lastcmdtime >= sftimeout * 60);
	      break;
	    }
	    if (user[x]->sf_to)
	      try = false;
	  }
	}
      }
      x++;
    }

    if (try) {
      ival = WITH->intervall * 60;
      ival2 = WITH->pollifnone * 60;
      if (WITH->startutc < WITH->endutc)
	tcheck = (aclock >= WITH->startutc && aclock <= WITH->endutc);
      else
	tcheck = (aclock <= WITH->endutc || aclock >= WITH->startutc);

      if (tcheck && !WITH->in_routing) {
	try1 = false;
	try2 = false;
	if ((ival > 0 || ival2 > 0) && WITH->bedingung > 0) {
	  if (WITH->intervall == 1)
	    try1 = (WITH->bedingung < 32 &&
		    ((1L << WITH->bedingung) & 0x6) != 0);
		/* sofort forwarden */
	  else {
	    if (WITH->lasttry > aclock)
	      zsum1 = WITH->lasttry + ival - SECPERDAY;
	    else
	      zsum1 = WITH->lasttry + ival;
	    if (zsum1 <= aclock)
	      try1 = (WITH->bedingung < 32 &&
		      ((1L << WITH->bedingung) & 0x6) != 0);
	  }

	  if (WITH->lasttry > aclock)
	    zsum2 = WITH->lasttry + ival2 - SECPERDAY;
	  else
	    zsum2 = WITH->lasttry + ival2;
	  if (zsum2 <= aclock)
	    try2 = (WITH->bedingung == 2);

	  if (try1 || try2) {
	    has_mail = sf_for(WITH->call);
	    if (has_mail || try2)
	      WITH->lasttry = aclock;
	    else if (WITH->bedingung < 2)
	      WITH->lasttry = aclock;
	    try = ((has_mail && try1) || try2);
	  } else
	    try = false;
      	  
	  do_linkcheck  = needs_linkcheck(WITH->call, true);
	  if (do_linkcheck) try = true;

	  if (try) {
	    if (WITH->bedingung == 1 && !do_linkcheck)
	      try = has_mail;

	    if (try) {
	      *w = '\0';
	      start_sf(-1, WITH->call, w);
	    }
	  }
	}

      }
    } else if (WITH->in_routing)
      WITH->lasttry = aclock;
    sftptr = sftptr->next;
  }

}


void set_sftimer(char *box, short min, short stnc, short sfcase, long maxb,
		 long maxu, long maxp, long pifnone, long sutc, long eutc)
{
  sfdeftype   *sfptr;

  sfptr       	      	= find_sf_pointer(box);
  if (sfptr == NULL)
    return;

  if (sfptr->maxbytes_p == 0) {
    sfptr->intervall  	= 0;
    sfptr->pollifnone 	= 0;
    sfptr->tnc	      	= 0;
    sfptr->bedingung  	= 0;
    sfptr->maxbytes_b 	= 20000;
    sfptr->maxbytes_p 	= 20000;
    sfptr->maxbytes_u 	= 20000;
    sfptr->lasttry    	= calc_aclock();
    sfptr->startutc   	= 0;
    sfptr->endutc     	= SECPERDAY-1;
    sfptr->in_routing 	= false;
  }
  if (min >= 0)
    sfptr->intervall  	= min;
  if (stnc >= 0)
    sfptr->tnc	      	= stnc;
  if (sfcase >= 0)
    sfptr->bedingung  	= sfcase;
  if (maxb >= 0)
    sfptr->maxbytes_b 	= maxb;
  if (maxu >= 0)
    sfptr->maxbytes_u 	= maxu;
  if (maxp >= 0)
    sfptr->maxbytes_p 	= maxp;
  if (pifnone >= 0)
    sfptr->pollifnone 	= pifnone;
  if (sutc >= 0)
    sfptr->startutc   	= sutc;
  if (eutc >= 0)
    sfptr->endutc     	= eutc;

  if (sfptr->tnc > 3)
    sfptr->tnc	      	= 0;
  if (sfptr->startutc > SECPERDAY-1)
    sfptr->startutc   	= 0;
  if (sfptr->endutc > SECPERDAY-1)
    sfptr->endutc     	= SECPERDAY-1;
  if (sfptr->bedingung > 2)
    sfptr->bedingung  	= 0;
  if (sfptr->bedingung <= 0) {
    sfptr->bedingung  	= 0;
    return;
  }
  if (sfptr->intervall < 1)
    sfptr->intervall  	= 1;
  if (sfptr->pollifnone == 0)
    sfptr->bedingung  	= 1;
  if (sfptr->bedingung == 2) {
    if (sfptr->pollifnone < 5)
      sfptr->pollifnone = 5;
  }

}


static long gwli(char *h)
{
  char w[256];

  get_word(h, w);
  return (atol(w));
}


static short gwi(char *h)
{
  return (gwli(h));
}


static long gwutc(char *h)
{
  long	  hl, erg;
  char	  w[256], w2[256];

  get_word(h, w);
  if (zahl(w) && strlen(w) == 4) {
    sprintf(w2, "%.2s", w);
    hl	  = atol(w2);
    strsub(w2, w, 3, 2);
    erg   = atol(w2);
    return ((hl * 60 + erg) * 60);
  } else
    return -1;
}


void set_sfparms(char *box, char *hs_)
{
  short     min, tnc, sfcase, which, wc;
  long	    pifnone, sutc, eutc, hl, maxb, maxu, maxp, val;
  char	    hs[256], w[256], w2[256];

  strcpy(hs, hs_);
  wc  	    = count_words(hs);
  if ((unsigned)wc < 32 && ((1L << wc) & 0x240) != 0) {
    min     = gwi(hs);
    tnc     = gwi(hs);
    sfcase  = gwi(hs);
    if (sfcase > 2)
      sfcase = 0;
    maxb    = gwli(hs);
    maxu    = gwli(hs);
    maxp    = gwli(hs);
    pifnone = gwli(hs);
    sutc    = gwutc(hs);
    eutc    = gwutc(hs);
    set_sftimer(box, min, tnc, sfcase, maxb, maxu, maxp, pifnone, sutc, eutc);
    return;
  }
  
  if (wc != 2) {
    set_sftimer(box, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    return;
  }
  
  get_word(hs, w);
  if (!zahl(w))
    return;
    
  which     = atoi(w);
  if ((unsigned)which >= 32 || ((1L << which) & 0x3fe) == 0)
    return;
  
  get_word(hs, w);
  if (!zahl(w))
    return;
  
  val 	    = atol(w);
  maxb	    = -1;
  maxu	    = -1;
  maxp	    = -1;
  sfcase    = -1;
  min 	    = -1;
  tnc 	    = -1;
  pifnone   = -1;
  sutc	    = -1;
  eutc	    = -1;
  
  switch (which) {

  case 1:
    min     = val;
    break;

  case 2:
    tnc     = val;
    break;

  case 3:
    sfcase  = val;
    break;

  case 4:
    maxb    = val;
    break;

  case 5:
    maxu    = val;
    break;

  case 6:
    maxp    = val;
    break;

  case 7:
    pifnone = val;
    break;

  case 8:
  case 9:
    if (zahl(w) && strlen(w) == 4) {
      sprintf(w2, "%.2s", w);
      hl    = atol(w2);
      strsub(w2, w, 3, 2);
      val   = atol(w2);
      val   = (hl * 60 + val) * 60;
      if (which == 8)
	sutc  = val;
      else
	eutc  = val;
    }
    break;
  }
  
  set_sftimer(box, min, tnc, sfcase, maxb, maxu, maxp, pifnone, sutc, eutc);
}


static void form_utc(time_t time, char *out)
{
  time /= 60;
  sprintf(out, "%.2ld%.2ld", time / 60, time % 60);
}


static void form_nextutc(time_t time, time_t start, time_t ende, char *out)
{
  if (time > SECPERDAY)
    time -= SECPERDAY;
  if (start < ende) {
    if (time > ende || time < start)
      time = start;
  } else {  /* start ist groesser als ende... ,zb s&f von 2200-0400 */
    if (time > ende && time < start)
      time = start;
  }
  form_utc(time, out);
}


static void add_hs(long v, short spc, short blanks, char *hs)
{
  short     x;
  char	    w[256];

  sprintf(w, "%*ld", spc, v);
  strcat(hs, w);
  for (x = 1; x <= blanks; x++)
    strcat(hs, " ");
}


void show_sfparms(short unr, char *box)
{
  boolean     	ok;
  sfdeftype   	*WITH, *sfptr;
  sfcpathtype 	*cp1;
  char	      	hs[256], sutcs[256], eutcs[256], nutcs[256];

  upper(box);
  ok  	  = false;
  sfptr   = sfdefs;
  while (sfptr != NULL) {

    WITH  = sfptr;

    if (*box == '\0' || !strcmp(WITH->call, box)) {

      switch (WITH->bedingung) {

      case 1:
	if (sf_for(WITH->call))
	  form_nextutc(WITH->lasttry + WITH->intervall * 60 + 60,
		       WITH->startutc, WITH->endutc, nutcs);
	else
	  strcpy(nutcs, "-");
	break;

      case 2:
	if (sf_for(WITH->call))
	  form_nextutc(WITH->lasttry + WITH->intervall * 60 + 60,
		       WITH->startutc, WITH->endutc, nutcs);
	else
	  form_nextutc(WITH->lasttry + WITH->pollifnone * 60 + 60,
		       WITH->startutc, WITH->endutc, nutcs);
	break;

      default:
	strcpy(nutcs, "-");
	break;

      }

      form_utc(WITH->startutc, sutcs);
      form_utc(WITH->endutc, eutcs);
      if (!ok)
	wlnuser(unr, "Box  1:Min 2:TNC 3:if 4:Info 5:User  6:Proposals 7:Min2 8:sUTC 9:eUTC   Next");
      ok    = true;

      strcpy(hs, WITH->call);
      rspacing(hs, LEN_CALL);
      add_hs(WITH->intervall, 3, 3, hs);
      add_hs(WITH->tnc, 3, 3, hs);
      add_hs(WITH->bedingung, 2, 1, hs);
      add_hs(WITH->maxbytes_b, 6, 1, hs);
      add_hs(WITH->maxbytes_u, 6, 5, hs);
      add_hs(WITH->maxbytes_p, 6, 2, hs);
      add_hs(WITH->pollifnone, 3, 5, hs);
      sprintf(hs + strlen(hs), "%s   %s   %s ", sutcs, eutcs, nutcs);
      if (WITH->usersf)
      	strcat(hs, "u");
      if (WITH->in_routing)
	strcat(hs, "*");
      cp1     	= WITH->cpathp;
      while (cp1 != NULL) {
	if (!strcmp(cp1->qrg, "FILE")) {
	  strcat(hs, "!");
	  cp1 	= NULL;
	} else
	  cp1 	= cp1->next;
      }
      wlnuser(unr, hs);
    }
    sfptr     	= sfptr->next;
  }
  if (!ok)
    wln_btext(unr, 45);
}

static void clear_sfinfos_in_userstruct(void)
{
  short x;

  for (x = 1;x <= MAXUSER; x++) {
    if (user[x] != NULL) {
      user[x]->needs_new_sf_ptr = user[x]->sf_ptr != NULL;
      user[x]->sf_ptr = NULL;
    }
  }
}

static void reassign_sfinfos_in_userstruct(void)
{
  short x;
  
  for (x = 1;x <= MAXUSER; x++) {
    if (user[x] != NULL) {
      if (user[x]->needs_new_sf_ptr) {
      	user[x]->sf_ptr = find_sf_pointer(user[x]->call);
	user[x]->needs_new_sf_ptr = false;
      }
    }
  }  
}

void dispose_sfinfos(void)
{
  sfdeftype   	  *hp;
  sfcpathtype 	  *cp1, *cp2;
  sffortype   	  *fp1, *fp2;
  sfrubtype   	  *rp1, *rp2;
  sfnotfromtype   *nf1, *nf2;

  debug0(2, 0, 135);
  
  if (sfdefs != NULL) clear_sfinfos_in_userstruct();
  
  while (sfdefs != NULL) {
    hp	    = sfdefs;
    sfdefs  = sfdefs->next;

    cp1     = hp->cpathp;
    while (cp1 != NULL) {
      cp2   = cp1;
      cp1   = cp1->next;
      free(cp2);
    }

    fp1     = hp->forp;
    while (fp1 != NULL) {
      fp2   = fp1;
      fp1   = fp1->next;
      free(fp2);
    }

    fp1     = hp->notforp;
    while (fp1 != NULL) {
      fp2   = fp1;
      fp1   = fp1->next;
      free(fp2);
    }

    rp1     = hp->rubrikp;
    while (rp1 != NULL) {
      rp2   = rp1;
      rp1   = rp1->next;
      free(rp2);
    }

    rp1     = hp->notrubrikp;
    while (rp1 != NULL) {
      rp2   = rp1;
      rp1   = rp1->next;
      free(rp2);
    }

    nf1     = hp->notfromp;
    while (nf1 != NULL) {
      nf2   = nf1;
      nf1   = nf1->next;
      free(nf2);
    }

    free(hp);
  }

  sfdefs    = NULL;
}


void load_sfinfos(void)
{
  short       	  k, result;
  boolean     	  ok, owncall, file_forward;
  sfdeftype   	  *hp, *hp2;
  sfcpathtype 	  *cp1;
  sffortype   	  *fp1;
  sfrubtype   	  *rp1;
  sfnotfromtype   *nf1;
  DTA 	      	  dirinfo;
  calltype    	  lastcall;
  char	      	  hs[256], hs1[256], w[256], sfps[256], STR1[256];

  dispose_sfinfos();

  debug0(2, 0, 90);

  hp2 	      	      	= NULL;
  sprintf(STR1, "%s%c%csf", boxsfdir, allquant, extsep);
  result      	      	= sffirst(STR1, 0, &dirinfo);
  while (result == 0) {
    ok	      	      	= false;
    owncall   	      	= false;
    *sfps     	      	= '\0';

    hp	      	      	= malloc(sizeof(sfdeftype));
    if (hp != NULL) {
      file_forward    	= false;
      hp->next	      	= NULL;
      hp->forp	      	= NULL;
      hp->notforp     	= NULL;
      hp->rubrikp     	= NULL;
      hp->notrubrikp  	= NULL;
      hp->notfromp    	= NULL;
      hp->cpathp      	= NULL;
      hp->routp       	= NULL;
      hp->no_sfpropdelay = nosfpropdelaydefault;
      hp->no_bullbin  	= false;
      hp->no_bull7plus	= false;
      hp->usersf      	= false;
      hp->routing_guest = false;
      hp->send_em     	= false;
      hp->em  	      	= EM_UNKNOWN;

      strcpy(hs, dirinfo.d_fname);
      del_ext(hs);
      cut(hs, LEN_CALL);
      upper(hs);
      strcpy(hp->call, hs);
      strcpy(lastcall, hs);
      
      if (get_wprot_neighbour(hp->call)) hp->em = EM_WPROT;

      hp->maxbytes_p  	= 0;   /* ist ein Flag */

      sprintf(STR1, "%s%s", boxsfdir, dirinfo.d_fname);
      k       	      	= sfopen(STR1, FO_READ);
      if (k >= minhandle) {
	ok    	      	= true;

	while (file2str(k, hs)) {
	  
	  del_blanks(hs);
	  if (*hs == '\0' || hs[0] == '#')
	    continue;
	    
	  strcpy(hs1, hs);
	  upper(hs);
	  get_word(hs, w);

	  if (!strcmp(w, "NOBULLBIN")) {
	    hp->no_bullbin = true;
	    continue;
	  }

	  if (!strcmp(w, "NOBULL7PLUS")) {
	    hp->no_bull7plus = true;
	    continue;
	  }

	  if (!strcmp(w, "USERSF")) {
	    hp->usersf = true;
	    continue;
	  }

	  if (!strcmp(w, "ROUTING_GUEST")) {
	    hp->routing_guest = true;
	    continue;
	  }

	  if (!strcmp(w, "MYBBS")) {
	    hp->send_em = true;
      	    if (hp->em == EM_WPROT) continue;
	    hp->em = EM_WP;
	    get_word(hs, w);
	    if (!*w) continue;
	    if (!strcmp(w, "EM")) hp->em = EM_EM;
	    continue;
	  }

	  if (!*hs) continue;

	  if (!strcmp(w, "IFQRG")) {
	    cp1       	= malloc(sizeof(sfcpathtype));
	    if (cp1 == NULL)
	      continue;
	      
	    get_word(hs, w);
	    cut(w, LEN_TNTPORT);
	    if (!strcmp(w, "FILE")) file_forward = true;
	    strcpy(cp1->qrg, w);
	    get_word(hs, w);
	    cut(w, 20);
	    strcpy(cp1->ccall, w); /* to call */
	    get_word(hs, w);
	    cut(w, 20);
	    strcpy(cp1->timeout, w); /* timeout */
	    get_word(hs1, w);
	    get_word(hs1, w);
	    get_word(hs1, w);	    
	    get_word(hs1, w);
	    strcpy(cp1->extended_args, hs1);   /* filename / execname */
	    cp1->next 	= hp->cpathp;
	    hp->cpathp	= cp1;
	    continue;
	  }

	  if (!strcmp(w, "SFIDLE")) {
	    hp->no_sfpropdelay = !positive_arg(hs);
	    continue;
	  }

	  if (!strcmp(w, "SFPARMS")) {
	    strcpy(sfps, hs);
	    continue;
	  }

	  if (!strcmp(w, "FOR")) {
	    while (*hs != '\0') {
	      fp1     	= malloc(sizeof(sffortype));
	      if (fp1 == NULL)
		continue;
		
	      get_word(hs, w);
	      cut(w, LEN_MBX);
	      strcpy(fp1->pattern, w);
	      if (!owncall) owncall = strstr(fp1->pattern, hp->call) == fp1->pattern; 
	      fp1->next = hp->forp;
	      hp->forp	= fp1;
	    }
	    continue;
	  }

	  if (!strcmp(w, "NOT") || !strcmp(w, "NOTFOR")) {
	    while (*hs != '\0') {
	      fp1     	= malloc(sizeof(sffortype));
	      if (fp1 == NULL)
		continue;
		
	      get_word(hs, w);
	      cut(w, LEN_MBX);
	      strcpy(fp1->pattern, w);
	      fp1->next = hp->notforp;
	      hp->notforp = fp1;
	    }
	    continue;
	  }

	  if (!strcmp(w, "RUBRIK") || !strcmp(w, "BOARD")) {
	    while (*hs != '\0') {
	      rp1     	= malloc(sizeof(sfrubtype));
	      if (rp1 == NULL)
		continue;
		
	      get_word(hs, w);
	      cut(w, LEN_BOARD);
	      strcpy(rp1->pattern, w);
	      rp1->next = hp->rubrikp;
	      hp->rubrikp = rp1;

	    }
	    continue;
	  }

	  if (!strcmp(w, "NOTRUBRIK") || !strcmp(w, "NOTBOARD")) {
	    while (*hs != '\0') {
	      rp1     	= malloc(sizeof(sfrubtype));
	      if (rp1 == NULL)
		continue;
		
	      get_word(hs, w);
	      cut(w, LEN_BOARD);
	      strcpy(rp1->pattern, w);
	      rp1->next = hp->notrubrikp;
	      hp->notrubrikp = rp1;
	    }
	    continue;
	  }

	  if (strcmp(w, "NOTFROM"))
	    continue;
	    
	  while (*hs != '\0') {
	    nf1       	= malloc(sizeof(sfnotfromtype));
	    if (nf1 == NULL)
	      continue;
	      
	    get_word(hs, w);
	    cut(w, LEN_CALL);
	    strcpy(nf1->pattern, w);
	    nf1->next = hp->notfromp;
	    hp->notfromp = nf1;
	  }
	}
	
	if (!owncall) {
	  /* Call of neighbour not found in FOR defs */
	  fp1 	  = malloc(sizeof(sffortype));
	  if (fp1 != NULL) {
	      /* add call manually */
      	      strcpy(fp1->pattern, hp->call);
	      fp1->next = hp->forp;
	      hp->forp	= fp1;
	  }	
	}

	sfclose(&k);
      	init_linkspeeds(hp, file_forward);
	
	if (hp2 == NULL) {
	  hp2 	      	= hp;
	  sfdefs      	= hp;
	} else {
	  hp2->next   	= hp;
	  hp2 	      	= hp;
	}
      } else  
	free(hp);

    }

    if (ok) set_sfparms(lastcall, sfps);

    result    	      	= sfnext(&dirinfo);
  }

  reassign_sfinfos_in_userstruct();
}


/* Diese Routine durchsucht die S&F-DEFINITIONEN auf Gueltigkeit zu den aktuellen     	      	    */
/* Parametern einer Nachricht. Wenn look_FOR false ist, dann wird nur getestet, ob die aktuelle     */
/* Nachricht auf gar keinen Fall an den entsprechenden Partner zu senden ist. Dies ist notwendig    */
/* im Zusammenhang mit dem automatischen Router, welcher immer dann in Aktion tritt, wenn die fest  */
/* vorgegebenen Definitionen keine Partnerbox fuer eine Zielmailbox angeben.          	      	    */

static boolean check_sfdeffile(boolean look_FOR, boolean only_rubrik,
  boolean is_7plus, boolean is_bin,
  sfdeftype *sfp, char *rubrik, char *orubrik, char *mbx, char *hiermbx, char *frombox,
  char *bid, boolean *rubrik_transfer, char *lastvias)
{
  boolean Result, ok, flood, direct_neighbour, check_rubriken, is_wp;
  char s[256];
  short k;
  sffortype *fp1;
  sfrubtype *rp1;
  sfnotfromtype *nf1;

  *rubrik_transfer = false;
  Result = false;
  if (sfp == NULL)
    return Result;

  debug(5, -1, 149, sfp->call);

  k = strpos2(lastvias, sfp->call, 1);
      /* lief das BULLETIN schon ueber diese Partnerbox ? */
  if (k > 0) {
    if (lastvias[k + strlen(sfp->call) - 1] == ' ') {
      return false;
    }
  }

  s[0] = '\0';

  flood = !callsign(mbx);
  if (!flood) {
    direct_neighbour = !strcmp(sfp->call, mbx);
  } else {
    direct_neighbour = false;
  }
  is_wp = (!strcmp(rubrik, "W") || !strcmp(rubrik, "WP"));
  check_rubriken = flood || ((!do_wprot_routing || is_wp) && direct_neighbour);

  if (!only_rubrik) {
    if (look_FOR) {
      ok = (strcmp(sfp->call, mbx) == 0);   /* @zielbox ist immer gueltig */

      if (strcmp(mbx, Console_call) || !strcmp(sfp->call, Console_call)) {
	fp1 = sfp->forp;
	while (fp1 != NULL && !ok) {
	  ok = wildcardcompare(SHORT_MAX, fp1->pattern, mbx, s);
	  fp1 = fp1->next;
	}

	if (*hiermbx != '\0' && !ok) {
	  fp1 = sfp->forp;
	  while (fp1 != NULL && !ok) {
	    ok = wildcardcompare(SHORT_MAX, fp1->pattern, hiermbx, s);
	    fp1 = fp1->next;
	  }
	}

      }

    } else
      ok = true;

    fp1 = sfp->notforp;
    while (fp1 != NULL && ok) {
      ok = !wildcardcompare(SHORT_MAX, fp1->pattern, mbx, s);
      fp1 = fp1->next;
    }

    if (*hiermbx != '\0' && ok) {
      fp1 = sfp->notforp;
      while (fp1 != NULL && ok) {
	ok = !wildcardcompare(SHORT_MAX, fp1->pattern, hiermbx, s);
	fp1 = fp1->next;
      }
    }

    if (check_rubriken) {
      rp1 = sfp->notrubrikp;
      while (rp1 != NULL && ok) {
      	ok = !wildcardcompare(SHORT_MAX, rp1->pattern, rubrik, s);
      	if (ok && *orubrik) ok = !wildcardcompare(SHORT_MAX, rp1->pattern, orubrik, s);
      	rp1 = rp1->next;
      }
    }

    if (flood) {
      nf1 = sfp->notfromp;
      while (nf1 != NULL && ok) {
	ok = !wildcardcompare(SHORT_MAX, nf1->pattern, frombox, s);
	nf1 = nf1->next;
      }
    }

  } else
    ok = false;


  rp1 = sfp->rubrikp;
  while (rp1 != NULL && !ok) {
    ok = wildcardcompare(SHORT_MAX, rp1->pattern, rubrik, s);
    if (!ok && *orubrik) ok = wildcardcompare(SHORT_MAX, rp1->pattern, orubrik, s);
    rp1 = rp1->next;
    if (ok) {
      if (callsign(mbx))
	*rubrik_transfer = true;
    }
  }

  if (ok && *bid && strcmp(sfp->call, Console_call)) {
    if (strlen(sfp->call) == LEN_CALL)
      ok = (strstr(bid, sfp->call) == NULL);
    /* bei kurzen BoxCalls kann man diesen Test nicht machen ... */
  }

  if (flood && ok) {
    if (sfp->no_bullbin && is_bin) ok = false;
    else if (sfp->no_bull7plus && is_7plus) ok = false;
  }

  /* send thebox-EM-Files only to neighbours without WP or WPROT flag */
  if (ok && rubrik[1] == '\0' && (rubrik[0] == 'E' || rubrik[0] == 'M')) {
    ok = (sfp->send_em && sfp->em == EM_EM);
  }

  return ok;
}


/* checks if a partner bbs should get a specific bulletin */
boolean forward_ok(char *box, char *rubrik, char *hiermbx, char *frombox, char *bid,
      	      	    boolean splus, boolean bin)
{
  boolean db, ok;
  sfdeftype *sfp;
  mbxtype mbx;
  
  sfp = find_sf_pointer(box);
  if (sfp == NULL) return false;
  unhpath(hiermbx, mbx);
  ok  = check_sfdeffile(true, false, splus, bin, sfp, rubrik, "", mbx, hiermbx, frombox, bid, &db, "");
  /* "W" only to direct neighbour */
  if (ok && (!strcmp(rubrik, "W") || (!strcmp(rubrik, "WP"))) && strcmp(mbx, box)) ok = false; 
  return ok;
}

/* checks if a partner bbs should _not_at_all_ get a specific bulletin */
static boolean valid_partner(boolean is_7plus, boolean is_bin,
			     char *box, char *rubrik, char *orubrik, char *mbx,
			     char *hiermbx, char *frombox, char *bid)
{
  boolean ok, db;
  sfdeftype *sfp;

  debug(5, -1, 148, box);
  sfp = find_sf_pointer(box);
  if (sfp != NULL) {
    ok = check_sfdeffile(false, false, is_7plus, is_bin, sfp, rubrik, orubrik, mbx, hiermbx, frombox, bid, &db, "");
    /* "W" only to direct neighbour */
    if (ok && (!strcmp(rubrik, "W") || (!strcmp(rubrik, "WP"))) && strcmp(mbx, box)) ok = false; 
    return ok;
  }
  if (strcmp(box, Console_call) && strlen(box) == LEN_CALL)
    ok = (strstr(bid, box) == NULL);
  else  /* bei kurzen BoxCalls kann man diesen Test nicht machen ... */
    ok = true;
  /* "W" only to direct neighbour */
  if (ok && (!strcmp(rubrik, "W") || (!strcmp(rubrik, "WP"))) && strcmp(mbx, box)) ok = false; 
  return ok;
}


/* vermerke_sf gibt nun auch Informationen ueber die gefundenen Wege an den  */
/* User aus. Da dies schon bei Beginn der Eingabe (also direkt nach Eingabe  */
/* des SEND-Befehles) passieren soll, gibt es einen quasi zweigeteilten      */
/* Modus: einmal wird vermerke_sf mit gesetztem Flag routtest aufgerufen     */
/* (zu Beginn), dabei wird NUR der Router getestet und die Informationen     */
/* ausgegeben, dann wird NACH dem CTRL-Z nochmal vermerke_sf aufgerufen,     */
/* diesmal allerdings ohne unr_msg, was dazu fuehrt, dass nichts ausgegeben  */
/* wird, aber geroutet wird, da routtest = false ist.                        */

static void new_sfentry(indexstruct *xheader)
{
  short			k2;
  unsigned short	acc;
  boardtype		ziel;

  debug0(5, -1, 140);
  strcpy(ziel, "X");
  if (exist(sflist)) {
    k2 = sfopen(sflist, FO_RW);
    /* ans Ende der Datei */
    if (sfseek(0, k2, SFSEEKEND) >= ((SHORT_MAX * sizeof(indexstruct)) - 100)) {
      sfclose(&k2);
      debug(0, -1, 140, "forward list overflow (board X)");
      return;
    }
  } else
    k2 = sfcreate(sflist, FC_FILE);
  if (k2 < minhandle) {
    debug(0, -1, 140, "can't open forward list (board X)");
    return;
  }
  xheader->deleted = false;
  xheader->start = 0;

  if (xheader->rxdate == 0)
    xheader->rxdate = clock_.ixtime;
  if (!callsign(xheader->rxfrom))
    xheader->rxfrom[0] = '\0';
  acc = 1;
  check_lt_acc(ziel, &xheader->lifetime, &acc);
  xheader->level = acc;

  xheader->erasetime = 0;
  xheader->lastread = 0;
  xheader->readcount = 0;
  *xheader->readby = '\0';
  xheader->eraseby = 0;
  *xheader->sendbbs = '\0';
  xheader->infochecksum = 0;
  xheader->bcastchecksum = 0;
  write_index(k2, -1, xheader);
  sfclose(&k2);
}


static void set_fwd(short unr_msg, indexstruct header1, short *routct,
		    boolean routtest, boolean rubrik_transfer,
		    boolean spec_sf, char *to_box, char *rubrik)
{
  char w[256];
  indexstruct header;

  debug0(5, unr_msg, 139);
  if (!routtest) {
    header = header1;
    if (header.msgtype == '\0')
      check_msgtype(&header.msgtype, header.dest, header.betreff);
    if (rubrik_transfer)
      strcpy(header.verbreitung, to_box);
    if (*header.verbreitung != '\0') {
      upper(header.verbreitung);
      strcpy(header.dest, to_box);
      header.fwdct = 0;
      strcpy(header.betreff, rubrik);
      if (strlen(rubrik) == 1 && (rubrik[0] == 'M' || rubrik[0] == 'E')) {
	header.pmode = 4;
	sprintf(header.betreff + strlen(header.betreff), " %s", header1.betreff);
      } else if (header.msgtype == 'P')
	header.pmode = 1;
      else
	header.pmode = 2;
      if (spec_sf || rubrik_transfer)
	header.pmode |= 8;
      if ((header1.pmode & TBIN) != 0)
	header.pmode |= TBIN;
      else if ((header1.pmode & T7PLUS) != 0)
	header.pmode |= T7PLUS;
      new_sfentry(&header);
    }
  }

  if (!boxrange(unr_msg))
    return;

  if (!in_real_sf(to_box) && !user[unr_msg]->supervisor)
    return;
    
  if (*routct == 0)
    wuser(unr_msg, "Routing  : ");
  (*routct)++;
  if (*routct % 7 == 0) {
    wlnuser0(unr_msg);
    wuser(unr_msg, "         ");
  }
  strcpy(w, to_box);
  if (ring_bbs && !strcmp(w, Console_call)) {
    strcat(w, " (swap)");
    rspacing(w, LEN_CALL+1+LEN_CALL+1);
    (*routct)++;
  } else
    rspacing(w, LEN_CALL+1);
  wuser(unr_msg, w);
}


static void check_if_err(short unr_msg, char *rubrik, char *mbx,
			 boolean routtest, indexstruct header1, short routct, char *ret)
{
  if (	     (!routtest || !strcmp(mbx, Console_call))
	 && !(callsign(mbx) && strcmp(mbx, Console_call))) {
    *ret	= 0;
    return;
  }

  *ret		= MSG_SFNOFOUND;

  if (!routtest)
    routing_error(-1, '\0', header1, rubrik, "routing error");

  if (!boxrange(unr_msg))
    return;

  if (routct == 0) {
    wuser(unr_msg, "routing error (@");
    wuser(unr_msg, mbx);
    wuser(unr_msg, ") : ");
  }
  wln_btext(unr_msg, 45);
}


unsigned short vermerke_sf(short unr_msg, boolean routtest, char *rubrik,
			   char *from_box, char *to_box_, indexstruct header1,
			   char *lastvias)
{
  short			k, routct;
  boolean		fout, fchk, rubrik_transfer, spec_sf, iscall, mirror, check_defs;
  boolean		hierarchical_mess, was_local, is_7plus, is_bin;
  sfdeftype		*sfp;
  userstruct		ufil;
  char			ret;
  char			to_box[256], hiermbx[256], mbx[256], hiermbx2[256], mbx2[256], box2[256];



  debug(3, unr_msg, 91, rubrik);

  strcpy(to_box, to_box_);
  routct		= 0;
  ret			= 0;
  fout			= false;
  rubrik_transfer	= false;
  hierarchical_mess	= false;
  was_local		= false;
  add_hpath(header1.verbreitung);
  unhpath(header1.verbreitung, mbx);
  spec_sf		= (strcmp(to_box, "*") && callsign(to_box));
  is_bin		= ((header1.pmode & TBIN) != 0);
  is_7plus		= ((header1.pmode & T7PLUS) != 0);

  /* forward W@NEIGHBOUR only to W@NEIGHBOUR */
  if ((!strcmp(rubrik, "W") || !strcmp(rubrik, "WP")) && !strcmp(Console_call, header1.absender)
      && find_routtable(mbx) != NULL) {
    if (valid_partner(is_7plus, is_bin, mbx, rubrik, header1.dest, mbx, header1.verbreitung, from_box, header1.id)) {
      set_fwd(unr_msg, header1, &routct, routtest, rubrik_transfer, spec_sf, mbx, rubrik);
    }
    
  } else if (!spec_sf && to_box[0] == '!') {

    /*So kann man einem nicht definierten Partner Eintraege forwarden (mit FOR + )*/
    spec_sf		= (!callsign(mbx) ||
			  (!strcmp(mbx, Console_call) && header1.msgtype == 'P'));
    strdelete(to_box, 1, 1);
    if (callsign(to_box))
      set_fwd(unr_msg, header1, &routct, routtest, rubrik_transfer, spec_sf, to_box, rubrik);

  } else {

    if (spec_sf) {
      spec_sf		= (!callsign(mbx) ||
			  (!strcmp(mbx, Console_call) && header1.msgtype == 'P') || header1.msgtype == 'B');
      strcpy(header1.verbreitung, to_box);
    }

/* was ist das denn hier? */
    strcpy(mbx, header1.verbreitung);
    hiermbx[0]		= '\0';
    k = strpos2(mbx, ".", 1);
    if (k > 0) {
      strsub(hiermbx, mbx, k, strlen(mbx) - k + 1);
      cut(mbx, k - 1);
    }
    cut(mbx, LEN_CALL);

_L1:
    if (sfdefs != NULL) {

      iscall		= callsign(mbx);
      was_local		= iscall && !strcmp(mbx, Console_call);
      mirror		= false;

      if (iscall && !strcmp(mbx, Console_call) && callsign(header1.dest)) {
	if (in_sfp(Console_call)) {
	  load_userfile(true, true, header1.dest, &ufil);
	  check_defs	= !ufil.lock_here;
	  mirror	= check_defs;
	} else
	  check_defs	= false;
      } else
	check_defs	= true;

      if (check_defs) {

      	/* auto routing with active auto router (if activated) */
	if (do_wprot_routing && !mirror && !fout && iscall && (callsign(header1.dest) || strcmp(Console_call, mbx))) {
	  find_neighbour(-1, mbx, box2);
	  if (*box2 != '\0') {
	    if (unr_msg == -77 || strcmp(from_box, box2) || (ring_bbs && !strcmp(from_box, Console_call))) {
	      if (valid_partner(is_7plus, is_bin, box2, rubrik, header1.dest, "", "", "", "")) {
	      	set_fwd(unr_msg, header1, &routct, routtest, rubrik_transfer, spec_sf, box2, rubrik);
	      	fout	= true;
	      }
	    }
	  }
	}

      	/* normal routing with sf definition files (if auto routing (passive) is off) */
	if (!fout && (!(smart_routing && iscall) || mirror)) {
	  sfp		= sfdefs;
	  while (sfp != NULL) {
	    if (unr_msg == -77 || strcmp(from_box, sfp->call) || (ring_bbs && !strcmp(from_box, Console_call))) {
	      fchk	= check_sfdeffile(true, false, is_7plus, is_bin, sfp, rubrik, header1.dest, mbx, hiermbx,
					  from_box, header1.id, &rubrik_transfer, lastvias);
	      if (fchk) {
		set_fwd(unr_msg, header1, &routct, routtest, rubrik_transfer, spec_sf, sfp->call, rubrik);
		fout	= true;
	      }
	    }
	    sfp		= sfp->next;
	  }
	}

      	/* auto routing with passive auto router */
	if (!fout && iscall && (callsign(header1.dest) || strcmp(Console_call, mbx))) {
	  find_neighbour(-2, mbx, box2);
	  if (*box2 != '\0') {
	    if (unr_msg == -77 || strcmp(from_box, box2) || (ring_bbs && !strcmp(from_box, Console_call))) {
	      if (valid_partner(is_7plus, is_bin, box2, rubrik, header1.dest, mbx, hiermbx, from_box, header1.id)) {
		set_fwd(unr_msg, header1, &routct, routtest, rubrik_transfer, spec_sf, box2, rubrik);
		fout	= true;
	      }
	    }
	  }
	}

      	/* normal routing with sf definition files, if auto routing failed */
	if (!fout && (smart_routing || do_wprot_routing) && iscall) {
	  sfp		= sfdefs;
	  while (sfp != NULL) {
	    if (unr_msg == -77 || strcmp(from_box, sfp->call) || (ring_bbs && !strcmp(from_box, Console_call))) {
	      fchk	= check_sfdeffile(true, false, is_7plus, is_bin, sfp, rubrik, header1.dest, mbx, hiermbx,
					  from_box, header1.id, &rubrik_transfer, lastvias);
	      if (fchk) {
		set_fwd(unr_msg, header1, &routct, routtest, rubrik_transfer, spec_sf, sfp->call, rubrik);
		fout	= true;
	      }
	    }
	    sfp		= sfp->next;
	  }
	}
	
      	/* special routing for subdomains with sf definition files for upper level domain */
	if (!fout && iscall && !was_local) { /* dg5lk.db0hes.#slh.deu.eu */
          strcpy(mbx2, hiermbx); /* hiermbx = .db0hes.#slh.deu.eu */
          hiermbx2[0]	= '\0';
          k		= strpos2(mbx2, ".", 1);
          if (k == 1) {
            strdelete(mbx2, 1, 1);
            k		= strpos2(mbx2, ".", 1);
          }
          if (k > 0) {
            strsub(hiermbx2, mbx2, k, strlen(mbx2) - k + 1);
            cut(mbx2, k - 1);
          }
          cut(mbx2, LEN_CALL);

	  if (callsign(mbx2)) {
	    if (strcmp(Console_call, mbx2)) {
	      strcpy(mbx, mbx2);
	      strcpy(hiermbx, hiermbx2);
	      goto _L1;
	    } else hierarchical_mess = true;
	  }
	}

	if (fout) {
	  if (iscall)
	    ret		= MSG_SFWAIT;
	}
	else if (!hierarchical_mess)
	  check_if_err(unr_msg, rubrik, mbx, routtest, header1, routct, &ret);
      }

    } else
      check_if_err(unr_msg, rubrik, mbx, routtest, header1, routct, &ret);

  }

  if (routct > 0) {
    if (boxrange(unr_msg))
      wlnuser0(unr_msg);
  }

  debug(5, unr_msg, 91, "...done");
  return (unsigned short)ret;
}


static boolean gen_sftest3(short unr, char *frombox, char *board, char *mbx)
{
  indexstruct		header;

  strcpy(header.verbreitung, mbx);
  *header.id		= '\0';
  strcpy(header.dest, board);
  *header.absender	= '\0';
  *header.betreff	= '\0';
  strcpy(header.rxfrom, frombox);
  header.deleted	= false;
  header.pmode		= TTEXT;
  header.msgflags	= MSG_SFRX;
  *header.readby	= '\0';
  *header.sendbbs	= '\0';

  if (callsign(board))
    header.msgtype	= 'P';
  else
    header.msgtype	= 'B';

  return (vermerke_sf(unr, true, board, frombox, "*", header, "") != MSG_SFNOFOUND);
}


boolean gen_sftest2(short unr, char *board, char *mbx)
{
  char		frombox[256];

  frombox[0]	= '\0';
  return (gen_sftest3(unr, frombox, board, mbx));
}


boolean gen_sftest(short unr, char *eingabe)
{
  boardtype	rubrik;
  char		hs[256];
  mbxtype	mbx;
  char		STR7[100];

  split_sline(eingabe, rubrik, hs, mbx, hs, hs, hs);
  if (*mbx == '\0') {
    if (callsign(rubrik))
      user_mybbs(rubrik, mbx);
  }
  add_hpath(mbx);
  snprintf(STR7, 255, "%s @ %s", rubrik, mbx);
  wlnuser(unr, STR7);
  return (gen_sftest2(unr, rubrik, mbx));
}


static boolean check_tpk_request(short unr, char *eingabe)
{
  long msgnum;
  short idx;
  indexstruct header;
  boardtype board;
  char w[256];

  get_word(eingabe, w);
  msgnum = atol(w) - MSGNUMOFFSET;
  idx = msgnum2board(msgnum, board);
  if (idx > 0) {
    if (read_brett(unr, nohandle, board, idx, idx, 100, "T", "", 0, &header) != ERR_READBOARD) {
      wlnuser(unr, ">");
      return true;
    }
  }
  sprintf(w, "Msg #%ld does not exist !\r", msgnum + MSGNUMOFFSET);
  chwuser(unr, 24); /* CANCEL */
  chwuser(unr, strlen(w));
  wuser(unr, w);
  wlnuser(unr, ">");
  return false;
}


static void ok_master(short unr)
{
  userstruct *WITH;

  if (!boxrange(unr))
    return;
  WITH = user[unr];
  if (WITH->sf_level == 1 && WITH->sf_master)
    wlnuser(unr, "F>");
  else {
    chwuser(unr, '>');
    wlnuser0(unr);
  }

  if (!strcmp(WITH->FSID, "THEBOX")) /* ausnahmsweise drinlassen... */
    boxspoolfend(WITH->pchan);
  boxspoolread();
}


void create_my_sid(char *sid, char *password)
{
  char hs[30];
  
  if (packed_sf) {
    strcpy(hs, "AB1D1FHMRW");
  } else {
    strcpy(hs, "ADFHMRW");
  }
  snprintf(sid, 200, "[DP-%s-%s$]%s", dp_vnr, hs, password);
}


void analyse_sid(boolean request, short unr, char *w)
{
  short       k, x;
  userstruct  *WITH;
  char	      w2[256], w3[256];

  if (!boxrange(unr))
    return;
    
  WITH	      	      = user[unr];
  WITH->sf_ptr	      = find_sf_pointer(WITH->call);
  WITH->f_bbs 	      = true;
  WITH->sf_level      = 1;
  boxsetboxsf(WITH->pchan, true);
  strcpyupper(w2, w);
  k   	      	      = strpos2(w2, "-", 1);
  x   	      	      = 0;
  while (k > 0) {
    x++;
    if (x == 1) {
      strsub(w3, w2, 2, k - 2);
      cut(w3, LEN_SID);
      strcpy(WITH->FSID, w3);
    } else if (x == 2) {
      sprintf(w3, "%.*s", k - 1, w2);
      cut(w3, LEN_SID);
      strcpy(WITH->MSID, w3);
    }
    strdelete(w2, 1, k);
    k 	      	      = strpos2(w2, "-", 1);
  }
  if (strlen(w2) > 1)   /* $] */
    strdelete(w2, strlen(w2) - 1, 2);
  cut(w2, LEN_SID);
  strcpy(WITH->SID, w2);

  if (strchr(WITH->SID, 'F') != NULL) {
    if (strstr(WITH->SID, "B1") != NULL)
      WITH->sf_level  = 5;
    else if (strchr(WITH->SID, 'B') != NULL)
      WITH->sf_level  = 4;
    else
      WITH->sf_level  = 3;
  } else if (strchr(WITH->SID, 'D') != NULL)
    WITH->sf_level    = 2;
  else
    WITH->sf_level    = 1;

  if (WITH->sf_level < 2) {
    if (!strcmp(WITH->FSID, "THEBOX"))
      WITH->sf_level  = 2;
    else if (!strcmp(WITH->FSID, "BCM"))
      WITH->sf_level  = 2;
  }
  
  set_wprot_neighbour(WITH->call, strchr(WITH->SID, 'W') != NULL);
  
  WITH->fwdmode       = bbs_pack(unr);

  if (!request) {
    WITH->action      = 1;
    return;
  }

  WITH->sf_to 	      = false;
  if (WITH->sf_level > 2)
    WITH->action      = 20;
  else {
    WITH->action      = 5;
    ok_master(unr);
  }
}


static boolean check_for_double_connect(short unr)
{
  short     x;

  if (!boxrange(unr))
    return false;

  for (x = 1; x <= MAXUSER; x++) {
    if (user[x] != NULL) {
      if (x != unr) {
	if (!strcmp(user[x]->call, user[unr]->call)) {
	  if (user[x]->sf_to)
	    return true;
	}
      }
    }
  }
  return false;
}


static short errorcondition(char *eingabe)
{
  if (strstr(eingabe, "CONNECTED TO") != NULL)
    return 2;
  else if (strstr(eingabe, "FAILURE WITH") != NULL)
    return 2;
  else if (strstr(eingabe, "BUSY FROM") != NULL)
    return 2;
  else if (!strcmp(eingabe, "***DONE"))
    return 1;
  else
    return 0;
}


void analyse_sf_command(short unr, char *eingabe, boolean return_)
{
  short       	k, x, ec, ct;
  boolean     	has_bid, plus;
  userstruct  	*WITH;
  char	      	*p;
  char	      	msgtype;
  char	      	w[256], w1[256], w2[256], w3[256], search[256], pwresp[256];
  char	      	STR1[256], STR13[256];

  debug(5, unr, 92, eingabe);
 
  if (!boxrange(unr))
    return;
    
  WITH = user[unr];
  box_ttouch(unr);

  if (*eingabe != '\0') {
    switch (WITH->action) {

    case 7:
    case 28:
      /* blank case */
      break;

    case 6:
    case 8:
    case 9:
    case 27:
      if (strlen(WITH->lastcmd) + strlen(eingabe) <= 254)
	sprintf(WITH->lastcmd + strlen(WITH->lastcmd), " %s", eingabe);
      else {
	sprintf(STR13, "%s %.*s", WITH->lastcmd, (int)(254 - strlen(WITH->lastcmd)), eingabe);
	strcpy(WITH->lastcmd, STR13);
      }
      break;

    default:
      strcpy(WITH->lastcmd, eingabe);
      break;
    }
  }

  if ((unsigned)WITH->action >= 32 ||
      ((1L << WITH->action) & 0x10000080L) == 0) {
    if (debug_level < 5) {
      debug(1, unr, 92, eingabe);
    }

    if (create_sflog)
      append_sflog(unr);
    if (in_protocalls(user[unr]->call))
      append_protolog(unr);
  }

  if (WITH->action == 0)
    return;
    
  del_lastblanks(eingabe);

  switch (WITH->action) {

/* these action values are used for the dummy link tests */

  case 555: /* linktest dummy FBB */
    stop_linkcheck(unr, 0);
    abort_sf(unr, true, "");
    break;

  case 556: /* linktest dummy FBB end */
    stop_linkcheck(unr, 0);
    abort_sf(unr, true, "");
    break;

  case 557: /* linktest dummy ASCII */
    if (strstr(eingabe, "OK") == eingabe) {
      start_linkcheck(unr, 0);
      send_ascii_linkcheck_data(unr);
      WITH->action = 558;
    } else {
      stop_linkcheck(unr, 0); /* use the value from the proposal for link check */
      abort_sf(unr, true, "");
    }
    break;

  case 558: /* linktest dummy ASCII end */
    if (strchr(eingabe, '>') != NULL) {
      stop_linkcheck(unr, 0);
      abort_sf(unr, true, "");
    } else {
      upper(eingabe);
      if (errorcondition(eingabe) > 0)
	sfproterr(unr);
    }
    break;

/* end dummy link tests ******************************** */

  case 1:
    WITH->sf_to = true;
    if (eingabe[strlen(eingabe) - 1] == '>') {
    
      *pwresp = '\0';
      if (*WITH->sfpwdb != '\0')
        calc_pwdb(WITH->call, WITH->sfpwdb, pwresp);
      else if (*WITH->sfpwtn != '\0') {
        if (count_words(WITH->sfpwtn) == 1) {
	  if (WITH->sfmd2pw == 1) /* MD2 */
	    calc_MD2_pw(WITH->sfpwtn, WITH->password, pwresp);
	  else /* MD5 */
	    calc_MD5_pw(WITH->sfpwtn, WITH->password, pwresp);
        } else {
	  calc_pwtn(WITH->call, WITH->sfpwtn, pwresp);
	  jitter_pwtn(pwresp);
        }
      }
      if (*pwresp != '\0')
        sprintf(pwresp, " %s", strcpy(STR13, pwresp));
        
      WITH->sf_ptr = find_sf_pointer(WITH->call);
      if (WITH->umode != UM_FILESF) {
      	create_my_sid(w2, pwresp);
	wlnuser(unr, w2);
      }
      if (WITH->sf_level > 2)
	send_fbb_proposals(unr, false);
      else
	user[unr]->action = 110;
    } else {
      upper(eingabe);
      if (errorcondition(eingabe) > 0)
	sfproterr(unr);
    }
    break;

  case 110:
    if (eingabe[strlen(eingabe) - 1] == '>') {
      if (user[unr]->umode == UM_FILESF) {
	look_for_mail(unr, true, true);
      }
      else if (check_for_double_connect(unr)) {
	wlnuser(unr, "F>");
	WITH->sf_to = false;
	WITH->action = 5;
      } else {
	look_for_mail(unr, true, true);
      }
    } else
      sfproterr(unr);
    break;

  case 2:
    if (eingabe[strlen(eingabe) - 1] == '>')
      look_for_mail(unr, true, false);
    else
      sfproterr(unr);
    break;

  case 3:
  case 4:   /* bulletin */
    /* diebox private */
    k = strlen(eingabe);
    if (k > 0) {
      upper(eingabe);
      if (errorcondition(eingabe) > 0)
	sfproterr(unr);
      else if (eingabe[0] == 'O'
		|| (eingabe[0] == 'H' && strchr(WITH->SID, 'R') != NULL)) {
	if (WITH->action == 3)
	  ok_sf_sending(unr, 1, 0);
	WITH->action = 30;
      } else if (eingabe[k - 1] == '>' && (k < 2 || eingabe[k - 2] != '|')) {
	/* die komische DieBox-Signalisation einer fehlerhaften BIN-CRC: */
	received_sf_sending('\0', unr, 1, false);
	look_for_mail(unr, true, false);
      } else {
	WITH->action = 35;
	if (strchr(WITH->SID, 'R') != NULL) {
	  switch (eingabe[0]) {
	  case 'R':
	    WITH->no_reason = 'R';
	    break;
	  case 'E':
	    WITH->no_reason = 'E';
	    break;
	  case 'N':
	    WITH->no_reason = '-';
	    break;
	  case 'L':
	    set_laterflag(unr, WITH->fbbprop[0].x_nr);
	    WITH->no_reason = 'L';
	    break;
	  default:
	    WITH->no_reason = '\0';
	    break;
	  }
	} else if (strchr(WITH->SID, 'M') != NULL) {
	  WITH->no_reason = '-';
	} else
	  WITH->no_reason = '\0';
      }
    } else
      sfproterr(unr);

    break;

  case 5:
    if (strlen(eingabe) > 100) {
      abort_sf(unr, true, "messy input");
      break;
    }
    msgtype = '\0';
    get_word(eingabe, w);
    if ((unsigned long)strlen(w) < 32 && ((1L << strlen(w)) & 0x6) != 0) { /* !!!CHECK */
      if (!strcmp(w, "F<")) {
	if (!check_tpk_request(unr, eingabe))
	  abort_sf(unr, false, "***invalid message number");
      } else if (w[0] == 'S') {
	if (disk_full)
	  abort_sf(unr, false, "DISK FULL");
	else {
	  if (strlen(w) == 2)
	    msgtype = w[1];
	  strcpy(w1, eingabe);
	  get_word(w1, w2);   /*Rubrik*/
	  p   	= strchr(w2, '@');
	  if (p != NULL) *p = '\0';

	  if (msgtype == '\0') {
	    *w3 = '\0';
	    check_msgtype(&msgtype, w2, w3);
	  }


	  if (allowed_rlisf(unr, msgtype, eingabe)) {
	    if (!strcmp(w2, "M") || !strcmp(w2, "E")) {
	      for (x = 1; x <= 5; x++)   /* Die BID */
		get_word(w1, search);
	      if (*search != '\0') {
		if (search[0] == '$') {
		  strdelete(search, 1, 1);
		  if (check_double(search)) {
		    WITH->fstat_rx_s++;
		    if (count_words(eingabe) == 6) {
		      wlnuser(unr, "OK");
		      strcpy(WITH->input2, eingabe);
		      WITH->action = 8;

		    } else {
		      wlnuser(unr, "OK");
		      chwuser(unr, '>');
		      wlnuser0(unr);
		      if (!strcmp(WITH->FSID, "THEBOX")) /* ausnahmsweise drinlassen... */
			boxspoolfend(WITH->pchan);
		      boxspoolread();
      	      	      ct  = strlen(eingabe);
      	      	      if (eingabe[ct - 1] == '\032')
      	      	      	strdelete(eingabe, ct, 1);
      	      	      del_lastblanks(eingabe);
		      sf_rx_emt(unr, eingabe);
		    }
		  } else {
                    delete_brett_by_bid("X", WITH->call, search, false, true);
		    wlnuser(unr, "NO");
		    ok_master(unr);
		  }
		} else {
		  wlnuser(unr, "NO");
		  ok_master(unr);
		}
	      }

	    } else {
	      has_bid = false;
	      plus = true;
	      do {
		get_word(w1, w3);
		if (strlen(w3) > 1) {
		  if (w3[0] == '$') {
		    strdelete(w3, 1, 1);
		    cut(w3, LEN_BID);
		    plus = check_double(w3);
		    has_bid = true;
		  }
		}
	      } while (*w1 != '\0' && plus != false);

	      if (!has_bid) {
/* 		if (!callsign(w2) || msgtype == 'B') */
		if (msgtype == 'B') /* change by DK2GO to allow "SP WP@XYZ" without BID */
		  plus = false;
	      }

	      if (plus) {
		wlnuser(unr, "OK");
		send_check(unr, eingabe, false, msgtype);
	      } else {
		if (has_bid) {
                  delete_brett_by_bid("X", WITH->call, w3, false, true);
		  wlnuser(unr, "NO - double");
		}
		else
		  wlnuser(unr, "NO - no BID");
		ok_master(unr);
	      }

	    }

	  } else {
	    if (strchr(WITH->SID, 'R')) wlnuser(unr, "REJECTED");
	    else wlnuser(unr, "NO - rejected");
	    ok_master(unr);
	  }

	}

      } else if (!strcmp(w, "F>")) {
	if (check_for_double_connect(unr))
	  abort_sf(unr, false, "*** double connect");
	else
	  look_for_mail(unr, false, false);
      } else {
        upper(eingabe);
        ec = errorcondition(eingabe);
        if (ec == 1)
          abort_sf(unr, true, "");
	else
	  sfproterr2(unr);
      }

    } else {
      upper(eingabe);
      if (errorcondition(eingabe) > 0)
	abort_sf(unr, true, "");
      else
	sfproterr2(unr);
    }

    break;

  case 6:
    box_txt2(false, unr, eingabe);
    break;

  case 7:
    send_text3(unr, false, eingabe, return_);
    break;

  case 8:
    if (strlen(eingabe) > 80) {
      abort_sf(unr, true, "messy input");
      break;
    }
    sprintf(eingabe, "%s %s", WITH->input2, strcpy(STR1, eingabe));
    sf_rx_emt(unr, eingabe);
    WITH->input2[0] = '\0';
    WITH->action = 9;
    break;

  case 9:
    if (strchr(eingabe, '\032') != NULL) {
      ok_master(unr);
      WITH->action = 5;
    } else {
      upper(eingabe);
      ec = errorcondition(eingabe);
      if (ec == 1)
        abort_sf(unr, true, "");
      else if (ec > 1)
	sfproterr(unr);
    }
    break;

  case 11:
    get_word(eingabe, w);
    if (!strcmp(w, "FS"))
      analyse_fbb_answer(unr, eingabe);
    else
      sfproterr2(unr);
    break;

  /* ACHTUNG: 14-15 sind reserviert! */

  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
    boxsetboxbin(WITH->pchan, false);   /* damit wir wieder was sehen... */
    get_word(eingabe, w);
    if (!strcmp(w, "FF") || !strcmp(w, "F>") || !strcmp(w, "FQ") ||
	!strcmp(w, "FB") || !strcmp(w, "FA") || !strcmp(w, "FD") ||
	!strcmp(w, "FU") || !strcmp(w, "F<")) {
      WITH->sf_to = false;

      if (WITH->action == 20) {
	for (x = 1; x <= MAXFBBPROPS; x++) {
	  if (WITH->fbbprop[x - 1].nr > 0 || WITH->fbbprop[x - 1].x_nr > 0)
	    received_sf_sending('\0', unr, x, false);
	}
      }

      if (!strcmp(w, "FF"))
	send_fbb_proposals(unr, true);
      else if (!strcmp(w, "FQ"))
	abort_sf(unr, false, "");
      else if (!strcmp(w, "F>") && user[unr]->action > 20) {
	if (check_prop_crc(unr, eingabe))
	  send_fbb_answer(unr);
	else
	  sfproterr2(unr);
      } else if (!strcmp(w, "F<")) {
	if (WITH->action == 20)
	  check_tpk_request(unr, eingabe);
	else
	  abort_sf(unr, false, "*** F< - command mixed with other...");
      } else if (!strcmp(w, "FB")) {
	if ((unsigned)WITH->fwdmode < 32 && ((1L << WITH->fwdmode) & 0xa) != 0)
	      /* ist ein FBB-Bin-FILE */
		abort_sf(unr, false, "*** file transfer not supported");
	else {
	  sprintf(WITH->fbbprop[WITH->action - 20].line, "%s %s", w, eingabe);
	  WITH->fbbprop[WITH->action - 20].unpacked = false;
	  WITH->action++;
	}
      } else if ((!strcmp(w, "FA") &&
		  ((unsigned)WITH->fwdmode < 32 &&
		  ((1L << WITH->fwdmode) & 0xa) != 0)) ||
		 (!strcmp(w, "FD") &&
		  ((unsigned)WITH->fwdmode < 32 &&
		  ((1L << WITH->fwdmode) & 0x34) != 0))) {
	sprintf(WITH->fbbprop[WITH->action - 20].line, "%s %s", w, eingabe);
	WITH->fbbprop[WITH->action - 20].unpacked = false;
	WITH->action++;
      } else if (!strcmp(w, "FU") &&
		 ((unsigned)WITH->fwdmode < 32 &&
		 ((1L << WITH->fwdmode) & 0x3e) != 0)) {
	sprintf(WITH->fbbprop[WITH->action - 20].line, "%s %s", w, eingabe);
	WITH->fbbprop[WITH->action - 20].unpacked = true;
	WITH->action++;
      } else
	sfproterr(unr);
    } else
      sfproterr(unr);

    break;

  case 25:
    get_word(eingabe, w);
    if (!strcmp(w, "F>")) {
      if (check_prop_crc(unr, eingabe))
	send_fbb_answer(unr);
      else
	sfproterr2(unr);
    } else
      sfproterr(unr);
    break;

  case 27:   /*hier kommt der Titel an*/
    box_txt2(false, unr, eingabe);
    break;

  case 28:   /*hier kommt der Text an*/
    send_text3(unr, false, eingabe, return_);
    break;

  case 30:
  case 31:
  case 32:
  case 33:
  case 35:
  case 36:
  case 37:
  case 38:
    upper(eingabe);
    boxsetboxbin(WITH->pchan, false);   /*damit wir wieder was sehen...*/
    if (errorcondition(eingabe) > 0)
      sfproterr(unr);
    else if (eingabe[strlen(eingabe) - 1] == '>') {
      if (WITH->no_reason != 'L')
        received_sf_sending(WITH->no_reason, unr, 1, WITH->action >= 35);
      WITH->no_reason = '\0';
      look_for_mail(unr, true, false);
    } else {
      WITH->action++;
      if (WITH->action == 34 || WITH->action == 39)
	sfproterr(unr);
    }
    break;

  case 34:
  case 39:
    sfproterr(unr);
    break;


  case 40:
  case 41:
  case 42:
  case 43:  /* Bestaetigungen im blockweisen E/M */
    upper(eingabe);
    if (errorcondition(eingabe) > 0)
      sfproterr(unr);
    else if (eingabe[strlen(eingabe) - 1] == '>') {
      WITH->fstat_tx_s++;
      WITH->emblockct--;
      if (WITH->emblockct <= 0) {
	WITH->action = 0;
	del_emblocks(unr, 0);
	look_for_mail(unr, true, false);
      } else
	WITH->action = 40;
    } else
      WITH->action++;
    break;

  case 44:
    sfproterr(unr);
    break;

  /* Achtung: 70-99 reserviert ! */

  case 100:
  case 101:
  case 102:
  case 103:
  case 104:
  case 105:
    strcpy(w1, eingabe);   /* fuer MD2 brauchen wir auch lowercase */
    upper(eingabe);
    if (errorcondition(eingabe) > 0)
      sfproterr(unr);
    else {
      get_word(eingabe, w);
      if (strlen(w) > 4) {
	if (w[0] == '[' && w[strlen(w) - 1] == ']' && w[strlen(w) - 2] == '$') {
	  analyse_sid(false, unr, w);
	  
	  del_lastblanks(eingabe);
	  switch (count_words(eingabe)) {   /* Passwordanfrage ? */
	  case 1:
	    if (eingabe[0] == '[') {  /* MD2 */
	      if (strlen(eingabe) >= 12) {
		get_word(w1, w); /* SID entfernen */
		strsub(WITH->sfpwtn, w1, 2, 10);
	      }
	    } else if (strlen(eingabe) == 10)
	      strcpy(WITH->sfpwdb, eingabe);
	    else if ((strlen(eingabe) == 11) && (eingabe[10] == 'N')) { /* neues DieBox-PW */
	      nstrcpy(WITH->sfpwdb, eingabe, 10);
	    }
	    break;
	    
	  case 2: /* diebox kann sich nicht entscheiden... */
	    get_word(eingabe, w);
	    if (strlen(w) == 10)
	      strcpy(WITH->sfpwdb, w);
	    break;

	  case 5:
	    if (strlen(eingabe) < 15)
	      strcpy(WITH->sfpwtn, eingabe);
	    break;
	  }
	} else
	  WITH->action++;
      } else
	WITH->action++;
    }
    break;

  default:
    if (useq(eingabe, "***done"))
      abort_sf(unr, true, "");
    else
      sfproterr(unr);
    break;
  }

}
