/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/

#define BOX_INIT_G
#include "main.h"
#include "box_init.h"
#include "box_file.h"
#include "tools.h"
#include "box_garb.h"
#include "box_logs.h"
#include "box_inou.h"
#include "box_sub.h"
#include "box_mem.h"
#include "box_rout.h"
#include "box_wp.h"
#include "box_sf.h"
#include "boxbcast.h"
#include "box_tim.h"
#include "box_scan.h"
#include "shell.h"
#include "sort.h"


static void append_profile_unr(short unr, char *s)
{
  append_profile(-1, s);
  if (boxrange(unr)) wlnuser(unr, s);
}

static void dispose_tcpip(void)
{
  tcpiptype	*hp, *hph;

  hp		= tcpiproot;   /* alte Liste loeschen */
  while (hp != NULL) {
    hph		= hp;
    hp		= hp->next;
    free(hph);
  }
  tcpiproot	= NULL;
}


void load_tcpipbuf(void)
{
  short		inf;
  tcpiptype	*hp, *hph;
  char		hs[256];
  char		w[256];

  dispose_tcpip();
  hph		= NULL;
  inf		= sfopen(tcpip_box, FO_READ);
  if (inf < minhandle)
    return;

  while (file2str(inf, hs)) {
    if (hs[0] == '#')
      continue;
      
    if (count_words(hs) <= 0)
      continue;
    
    hp		= malloc(sizeof(tcpiptype));
    if (hp == NULL)
      continue;

    if (tcpiproot == NULL)
      tcpiproot	= hp;
    get_word(hs, w);
    upper(w);
    cut(w, LEN_CALL);
    strcpy(hp->call, w);
    cut(hs, LEN_TCPIPBBSTXT);
    strcpy(hp->txt, hs);
    hp->next	= NULL;
    if (hph != NULL)
      hph->next	= hp;
    hph		= hp;
  }
  sfclose(&inf);
}


static void dispose_badwords(void)
{
  dirtytype	*hp, *hph;

  hp		= badwordroot;   /* alte Liste loeschen */
  while (hp != NULL) {
    hph		= hp;
    hp		= hp->next;
    free(hph);
  }
  badwordroot	= NULL;
}


static void load_badwords(void)
{
  short		inf, x, l;
  dirtytype	*hp, *hph;
  pathstr	hs;

  dispose_badwords();
  hph		= NULL;
  sprintf(hs, "%sbadwords%cbox", boxsysdir, extsep);
  inf		= sfopen(hs, FO_READ);

  if (inf < minhandle)
    return;

  while (file2str(inf, hs)) {
    l	= strpos2(hs, "#", 1);
    if (l > 0)
      cut(hs, l - 1);
    if (count_words(hs) <= 0)
      continue;

    upper(hs);

    hp		= malloc(sizeof(dirtytype));
    if (hp == NULL)
      continue;

    if (badwordroot == NULL)
      badwordroot	= hp;

    cut(hs, LEN_BADWORD / 2 - 2);
    l = strlen(hs);
    if (hs[0] != '*' /* && hs[0] != ' ' */)
      strcpy(hp->bad, "*");
    else
      *hp->bad	= '\0';

    for (x = 0; x < l; x++)
      sprintf(hp->bad + strlen(hp->bad), "%c*", hs[x]);

    while ((x = strpos2(hp->bad, "**", 1)) > 0)
      strdelete(hp->bad, x, 1);
/*
    if (l > 1 && hs[l - 1] == ' ')
      strdelete(hp->bad, strlen(hp->bad), 1);
*/
    hp->next	= NULL;

    if (hph != NULL)
      hph->next	= hp;
    hph		= hp;
  }
  sfclose(&inf);
}


static void dispose_rejectinfos(void)
{
  rejecttype	*hp, *hph;

  hp		= rejectroot;   /* alte Liste loeschen */
  while (hp != NULL) {
    hph		= hp;
    hp		= hp->next;
    free(hph);
  }
  rejectroot	= NULL;
}


static boolean checkneg(char *w)
{
  if (w[0] != '~')
    return true;

  strdelete(w, 1, 1);
  return false;
}


static void load_rejectinfos(void)
{
  short		inf;
  rejecttype	*hp, *hph;
  pathstr 	hs;
  char		w[256];

  dispose_rejectinfos();
  hph		= NULL;
  snprintf(hs, LEN_PATH, "%sreject%cbox", boxsysdir, extsep);
  inf		= sfopen(hs, FO_READ);
  if (inf < minhandle)
    return;

  while (file2str(inf, hs)) {
    if (hs[0] == '#')
      continue;

    if (count_words(hs) <= 6)
      continue;
      
    upper(hs);

    hp		= malloc(sizeof(rejecttype));
    if (hp == NULL)
      continue;

    if (rejectroot == NULL)
      rejectroot	= hp;

    convtit		= true;

    get_word(hs, w);
    hp->what		= upcase_(w[0]);
    get_word(hs, w);
    hp->msgtypeneg	= checkneg(w);
    hp->msgtype		= upcase_(w[0]);
    get_word(hs, w);
    hp->fromneg		= checkneg(w);
    cut(w, LEN_CALL);
    strcpy(hp->from, w);
    get_word(hs, w);
    hp->mbxneg		= checkneg(w);
    cut(w, LEN_MBX);
    strcpy(hp->mbx, w);
    get_word(hs, w);
    hp->tobneg		= checkneg(w);
    cut(w, LEN_BOARD);
    strcpy(hp->tob, w);
    get_word(hs, w);
    hp->bidneg		= checkneg(w);
    cut(w, LEN_BID);
    strcpy(hp->bid, w);
    get_word(hs, w);
    hp->maxsizeneg	= checkneg(w);
    hp->maxsize		= atol(w) * 1024;
    hp->next		= NULL;

    if (hph != NULL)
      hph->next		= hp;
    hph			= hp;
  }
  sfclose(&inf);
}


static void dispose_rubrikinfos(void)
{
  rubriktype	*hp, *hph;

  debug0(2, 0, 136);
  hp		= rubrikroot;
  while (hp != NULL) {
    hph		= hp;
    hp		= hp->next;
    free(hph);
  }
  rubrikroot	= NULL;
}


/* format of rubriken.box:    	      	      */
/* NAME Max_Lifetime Userlevel Min_Lifetime   */
/* ALL 90 1 30	      	      	      	      */

static void load_rubrikinfos(short unr)
{
  short		inf, k, acc;
  unsigned short mlt;
  rubriktype	*hp, *hph;
  char		hs[256];
  char		w[256], w1[256], w2[256], w3[256];
  short		i;

  dispose_rubrikinfos();
  debug0(2, 0, 23);
  hph		= NULL;

  inf		= sfopen(rubriken_box, FO_READ);
  if (inf < minhandle)
    return;

  while (file2str(inf, hs)) {
    if (hs[0] == '#')
      continue;

    if (count_words(hs) > 1)
      strcpy(w, hs);
    else {
      get_word(hs, w);
      do {
	k	= strpos2(w, ":", 1);
	if (k > 0)
	  w[k - 1] = ' ';
      } while (k != 0);
    }
    get_word(w, w1);
    get_word(w, w2);
    get_word(w, w3);
    i	= strlen(w1);
    if (i < 1 || i > LEN_BOARD)
      continue;

    hp	= malloc(sizeof(rubriktype));
    if (hp == NULL)
      continue;

    if (rubrikroot == NULL)
      rubrikroot= hp;

    upper(w1);
    strcpy(hp->name, w1);

    hp->lifetime= atoi(w2);
    acc		= atoi(w3);
    if (acc < 1)
      acc	= 1;
    hp->access	= acc;
    
    get_word(w, w3);
    if (*w3 && zahl(w3)) {
      mlt = atoi(w3);
    } else {
      mlt = min_lifetime;
    }
    if (mlt > 999) mlt = 0;
    if (hp->lifetime > 0 && (mlt > hp->lifetime || mlt == 0))
      hp->lifetime = mlt;
    hp->min_lifetime = mlt;
    
    hp->next	= NULL;
    if (hph != NULL)
      hph->next = hp;
    hph		= hp;
  }
  sfclose(&inf);
}

static boolean valid_target_board(char *w)
{
  if (!strcmp(w, "E")) return false;
  if (!strcmp(w, "M")) return false;
  if (!strcmp(w, "T")) return false;
  if (!strcmp(w, "W")) return false;
  if (!strcmp(w, "X")) return false;
  return true;
}
static void dispose_verteilinfos(void)
{
  transfertype	*hp, *hph;

  hp		= transferroot;
  while (hp != NULL) {
    hph		= hp;
    hp		= hp->next;
    free(hph);
  }
  transferroot	= NULL;
}


static void load_verteilinfos(short unr)
{
  short		inf;
  transfertype	*hp, *hph;
  pathstr	hs;
  char		w[256], w2[256];

  dispose_verteilinfos();
  hph		= NULL;
  snprintf(hs, LEN_PATH, "%stransfer%cbox", boxsysdir, extsep);
  inf		= sfopen(hs, FO_READ);
  if (inf < minhandle)
    return;

  while (file2str(inf, hs)) {
    if (hs[0] == '#')
      continue;

    if (count_words(hs) != 2)
      continue;

    upper(hs);
    hp		= malloc(sizeof(transfertype));
    if (hp == NULL)
      continue;

    if (transferroot == NULL)
      transferroot = hp;
    get_word(hs, w);
    cut(w, LEN_BOARD);
    strcpy(hp->board1, w);
    get_word(hs, w);
    cut(w, LEN_BOARD);
    if (valid_target_board(w)) {
      strcpy(hp->board2, w);
      hp->next	= NULL;
      if (hph != NULL)
      	hph->next	= hp;
      hph		= hp;
    } else {
      snprintf(w2, 200, "invalid target board in transfer.box: %s", w);
      append_profile_unr(unr, w2);
      if (transferroot == hp)
      	transferroot = NULL;
      free(hp);
    }
  }
  sfclose(&inf);
}


static void dispose_rsysops(void)
{
  rsysoptype	*hp, *hph;

  hp		= rsysoproot;
  while (hp != NULL) {
    hph		= hp;
    hp		= hp->next;
    free(hph);
  }
  rsysoproot	= NULL;
}


static void load_rsysops(void)
{
  short		inf;
  rsysoptype	*hp, *hph;
  pathstr	hs;
  char		w[256];

  dispose_rsysops();
  hph		= NULL;
  snprintf(hs, LEN_PATH, "%sprvcalls%cbox", boxsysdir, extsep);
  inf		= sfopen(hs, FO_READ);
  if (inf < minhandle)
    return;

  while (file2str(inf, hs)) {
    if (hs[0] == '#')
      continue;

    if (count_words(hs) != 2)
      continue;

    upper(hs);
    hp		= malloc(sizeof(rsysoptype));
    if (hp == NULL)
      continue;

    if (rsysoproot == NULL)
      rsysoproot = hp;
    get_word(hs, w);
    cut(w, LEN_CALL);
    strcpy(hp->call, w);
    get_word(hs, w);
    cut(w, LEN_BOARD);
    strcpy(hp->board, w);
    hp->next	= NULL;
    if (hph != NULL)
      hph->next	= hp;
    hph		= hp;
  }
  sfclose(&inf);
}


static void dispose_convtit(void)
{
  convtittype	*hp, *hph;

  hp		= convtitroot;
  while (hp != NULL) {
    hph		= hp;
    hp		= hp->next;
    free(hph);
  }
  convtitroot	= NULL;
}


static void load_convtit(short unr)
{
  short		inf;
  convtittype	*hp, *hph;
  pathstr	hs;
  char		w[256], w2[256];

  dispose_convtit();
  hph		= NULL;
  snprintf(hs, LEN_PATH, "%sconvtit%cbox", boxsysdir, extsep);
  inf		= sfopen(hs, FO_READ);
  if (inf < minhandle)
    return;

  while (file2str(inf, hs)) {
    if (hs[0] == '#')
      continue;

    if (count_words(hs) <= 3)
      continue;

    upper(hs);
    hp		= malloc(sizeof(convtittype));
    if (hp == NULL)
      continue;
    convtit	= true;
    if (convtitroot == NULL)
      convtitroot = hp;
    get_word(hs, w);
    cut(w, LEN_BOARD);
    strcpy(hp->fromboard, w);
    get_word(hs, w);
    cut(w, LEN_BOARD);
    if (valid_target_board(w)) {
      strcpy(hp->toboard, w);
      get_word(hs, w);
      hp->newlt = (unsigned short)atoi(w);
      cut(hs, LEN_SUBJECT);
      strcpy(hp->title, hs);
      hp->next	= NULL;
      if (hph != NULL)
      	hph->next	= hp;
      hph		= hp;
    } else {
      snprintf(w2, 200, "invalid target board in convtit.box: %s", w);
      append_profile_unr(unr, w2);
      if (convtitroot == hp)
      	convtitroot = NULL;
      free(hp);
    }
  }
  sfclose(&inf);
}


static void dispose_convlt(void)
{
  convtittype	*hp, *hph;

  hp		= convltroot;   /* alte Liste loeschen */
  while (hp != NULL) {
    hph		= hp;
    hp		= hp->next;
    free(hph);
  }
  convltroot	= NULL;
}


static void load_convlt(void)
{
  short		inf;
  convtittype	*hp, *hph;
  pathstr	hs;
  char		w[256];

  dispose_convlt();
  hph		= NULL;
  snprintf(hs, LEN_PATH, "%sconvlt%cbox", boxsysdir, extsep);
  inf		= sfopen(hs, FO_READ);
  if (inf < minhandle)
    return;

  while (file2str(inf, hs)) {
    if (hs[0] == '#')
      continue;

    if (count_words(hs) <= 2)
      continue;

    upper(hs);
    hp		= malloc(sizeof(convtittype));
    if (hp == NULL)
      continue;

    convtit	= true;
    if (convltroot == NULL)
      convltroot = hp;
    get_word(hs, w);
    cut(w, LEN_BOARD);
    strcpy(hp->fromboard, w);
    get_word(hs, w);
    hp->newlt	= (unsigned short)atoi(w);
    cut(hs, LEN_SUBJECT);
    strcpy(hp->title, hs);
    hp->next	= NULL;
    hp->toboard[0] = '\0';
    if (hph != NULL)
      hph->next	= hp;
    hph		= hp;
  }
  sfclose(&inf);
}


static void load_balise(void)
{
  long		rp;
  boolean	ok;
  char		hs[256], w[256];

  debug0(2, 0, 24);
  balisetime	= 0;
  ok		= false;
  if (balisesize != 0)
    free(balisebuf);
  sfbread(true, beacon_box, &balisebuf, &balisesize);
  if (balisesize <= 0)
    return;

  rp		= 0;
  do {
    get_line(balisebuf, &rp, balisesize, hs);
    upper(hs);
    get_word(hs, w);
    if (!strcmp(w, "INTERVAL")) {
      get_word(hs, w);
      balisetime= atol(w) * 60;
      ok	= true;
    }
  } while (!(ok || rp >= balisesize));
}


static void free_unprotodefs(void)
{
  unprotoportstype	*hpp, *hpp1;

  if (unprotodefroot == NULL)
    return;

  hpp		= unprotodefroot->ports;
  while (hpp != NULL) {
    hpp1	= hpp;
    hpp		= hpp->next;
    free(hpp1);
  }
  free(unprotodefroot);
  unprotodefroot= NULL;
}


static void load_unprotodefs(void)
{
  short			k;
  unprotoportstype	*hp;
  char			hs[256], w[256];
  pathstr		STR1;

  free_unprotodefs();
  snprintf(STR1, LEN_PATH, "%sunproto%cbox", boxsysdir, extsep);
  k			= sfopen(STR1, FO_READ);
  if (k < minhandle)
    return;

  unprotodefroot	= malloc(sizeof(unprotodeftype));
  if (unprotodefroot == NULL)
    return;

  unprotodefroot->ports		= NULL;
  unprotodefroot->maxback	= 2000;
  unprotodefroot->PollInterval	= 20;
  unprotodefroot->TxInterval	= 5;
  unprotodefroot->priv		= false;
  unprotodefroot->sys		= false;
  unprotodefroot->fbb		= true;
  unprotodefroot->dpbox		= true;

  while (file2str(k, hs)) {
    del_comment(hs, '#');
    get_word(hs, w);
    if (*w == '\0' || w[0] == '#')
      continue;

    upper(w);
    del_lastblanks(hs);
    if (!strcmp(w, "POLLINTERVAL")) {
      unprotodefroot->PollInterval	= atol(hs);
      continue;
    }
    if (!strcmp(w, "TXINTERVAL") || !strcmp(w, "INTERVAL")) {
      unprotodefroot->TxInterval	= atol(hs);
      continue;
    }
    if (!strcmp(w, "PRIVATES")) {
      unprotodefroot->priv		= positive_arg(hs);
      continue;
    } 
    if (!strcmp(w, "SYSOP")) {
      unprotodefroot->sys		= positive_arg(hs);
      continue;
    }  
    if (!strcmp(w, "MAXBACK")) {
      unprotodefroot->maxback		= atol(hs);
      continue;
    }
    if (!strcmp(w, "FBB")) {
      get_word(hs, w);
      unprotodefroot->fbb		= positive_arg(w);
      continue;
    }
    if (!strcmp(w, "DPBOX")) {
      get_word(hs, w);
      unprotodefroot->dpbox		= positive_arg(w);
      continue;
    }
    if (strcmp(w, "QRG"))
      continue;

    get_word(hs, w);
    if (*w == '\0')
      continue;

    hp			= malloc(sizeof(unprotoportstype));
    if (hp == NULL)
      continue;

    cut(w, LEN_TNTPORT);
    strcpy(hp->port, w);
    upper(hs);
    strcpy(hp->path, hs);
    hp->RequestActive	= false;
    hp->CurrentSendPos	= unproto_last;
    hp->LastReqTime	= 0;
    hp->LastTxTime	= 0;
    hp->next		= unprotodefroot->ports;
    unprotodefroot->ports= hp;
  }
  sfclose(&k);
  if (unprotodefroot->maxback < 200)
    unprotodefroot->maxback	= 200;
  else if (unprotodefroot->maxback > 4000)
    unprotodefroot->maxback	= 4000;
  if (unprotodefroot->PollInterval < 0)
    unprotodefroot->PollInterval= 0;
  if (unprotodefroot->TxInterval < 0)
    unprotodefroot->TxInterval	= 0;

  if (!(unprotodefroot->fbb || unprotodefroot->dpbox))
    free_unprotodefs();
}


static void dispose_boxcommands(void)
{
  bcommandtype	*hp, *hph;

  hp		= bcommandroot;
  while (hp != NULL) {
    hph		= hp;
    hp		= hp->next;
    free(hph);
  }
  bcommandroot	= NULL;
}


static void load_boxcommands(void)
{
  short		inf;
  bcommandtype	*hp, *hph;
  pathstr	hs;
  char		w[256];

  dispose_boxcommands();
  hph		= NULL;
  snprintf(hs, LEN_PATH, "%scommands%cbox", boxsysdir, extsep);
  inf		= sfopen(hs, FO_READ);
  if (inf < minhandle) {
    boxalert2("No COMMANDS.BOX !", true);
    return;
  }

  while (file2str(inf, hs)) {
    if (hs[0] == '#')
      continue;

    if (count_words(hs) != 4)
      continue;

    upper(hs);
    hp		= malloc(sizeof(bcommandtype));
    if (hp == NULL)
      continue;

    if (bcommandroot == NULL)
      bcommandroot = hp;
    get_word(hs, w);
    cut(w, LEN_BOXCOMMAND);
    strcpy(hp->command, w);
    get_word(hs, w);
    hp->cnr	= atoi(w);
    get_word(hs, w);
    hp->sysop	= (atoi(w) != 0);
    get_word(hs, w);
    hp->ulev	= atoi(w);
    hp->next	= NULL;
    if (hph != NULL)
      hph->next	= hp;
    hph		= hp;
  }
  sfclose(&inf);
}


static void load_fbbheader(void)
{
  short		x, fbb, zct;
  char		zl[256];


  for (x = 0; x <= 11; x++)
    *fbb_month[x]	= '\0';
  fbb_fromfield[0]	= '\0';
  fbb514_fromfield[0]	= '\0';
  fbb_tofield[0]	= '\0';
  fbb514_tofield[0]	= '\0';

  if (!exist(f6fbb_box))
    return;

  fbb			= sfopen(f6fbb_box, FO_READ);

  zct			= 0;
  while (file2str(fbb, zl)) {
    del_leadblanks(zl);
    if (*zl == '\0')
      continue;

    if (zl[0] == '#')
      continue;

    zct++;
    switch (zct) {

    case 1:
      cut(zl, 20);
      strcpy(fbb_fromfield, zl);
      break;

    case 2:
      cut(zl, 20);
      strcpy(fbb_tofield, zl);
      break;

    case 3:
      cut(zl, 20);
      strcpy(fbb514_fromfield, zl);
      break;

    case 4:
      cut(zl, 20);
      strcpy(fbb514_tofield, zl);
      break;

    default:
      if ((unsigned)zct < 32 && ((1L << zct) & 0x1ffe0L) != 0) {
	cut(zl, 3);
	upper(zl);
	strcpy(fbb_month[zct - 5], zl);
      }
      break;
    }
  }
  sfclose(&fbb);
}


static void load_boxconfig(short unr)
{
  short		x, inf, outf;
  char	      	*p;
  char		hs[256], w[256], w1[256], w2[256], h2[256], lasttoken[256];
  pathstr     	tmpfile;

  strcpy(sf_connect_call, Console_call);
  y_lifetime		= 3;
  resume_lifetime	= 3;
  min_lifetime	      	= 1; /* minimum lifetime for all boards, if not specified in rubriken.box */
  *e_m_verteiler	= '\0';
  *pacsrv_verteiler	= '\0';
  tpkbbs		= false;
  *mustcheckboards    	= '\0';
  *default_rubrik	= '\0';
  *default_tpkbbs	= '\0';
  *other_callsyntax	= '\0';
  *expand_fservboard   	= '\0';
  *accepted_bad_rcalls	= '\0';
  bcastforward		= false;
  multiprivate		= false;
  sfinpdefault		= SFI_FROMPB;
  debug_level		= 0;
  debug_size		= 99999999L;
  usertimeout		= 20;
  sftimeout		= 5;
  max_lt_inc		= 0;
  ufilhide		= false;
  scan_all_wp		= false;
  create_syslog		= true;
  create_userlog	= false;
  create_usersflog	= false;
  holdownfiles		= false;
  create_readlog	= false;
  create_sflog		= false;
  create_convlog	= false;
  remoteerasecheck	= false;
  authentinfo		= false;
  mail_beacon		= false;
  gttl			= false;
  add_ex		= false;
  smart_routing		= false;
  route_by_private    	= false;
  request_lt		= false;
  small_first		= true;
  sort_props		= true;
  create_acks		= true;
  packed_sf		= true;
  forwerr		= true;
  xcheck_in_ram		= true;
  valid_in_ram		= false;
/* send_bbsbcast	= false; */
  unproto_list		= false;
  ring_bbs		= false;
  multi_master		= true;
  charge_bbs		= false;
  auto7plusextract	= false;
  autobinextract	= false;
  reduce_lt_after_extract=false;
  check_sffor_in_ram	= false;
  no_rline_if_exterior	= false;
  allunproto		= false;
  guess_mybbs		= true;
  login_check		= false; /* DL5HBF */
  md2_only_numbers	= false;
  private_dirtycheck	= false;
  usersftellmode	= SFT_NONE;
  holddelay		= SECPERDAY*1;	/* 1 day	*/
  erasewait		= SECPERDAY*3;	/* 3 days	*/
  wpcreateinterval	= SECPERDAY*1;
  wprotcreateinterval	= WPROTCREATEINTERVAL; /* don't change */
  returntime		= SECPERDAY*7;
  bullsfwait		= SECPERDAY*7;
  bullsfmaxage		= SECPERDAY*30;
  incominglifetime	= SECPERDAY*7;
  fservexpandlt       	= SECPERDAY*7;
  maxuserconnects	= 2;
  all_maxread_day	= 999999999L;
  sprintf(connecttext, "%s BBS", Console_call);
  *xeditor    	      	= '\0';
  *redlocal		= '\0';
  *redregion		= '\0';
  *rednation		= '\0';
  *reddefboard		= '\0';
  redlifetime		= -1;
  strcpy(default_prompt, "(%b) %c de %m>");
  sub_bid		= 0;
  retmailmaxbytes	= 1500;
  maxsubscriberdisp	= 50;
  maxufcache		= 100;
  import_logs		= true;
  nosfpropdelaydefault	= true;
  show_readers_to_everyone=false;
  stop_invalid_rcalls  	= true;
  stop_broken_rlines  	= true;
  stop_invalid_rdates 	= true;
  stop_changed_bids   	= true;
  stop_changed_boards 	= false;
  stop_changed_mbx    	= true;
  stop_changed_senders	= true;
  ping_allowed	      	= true;
  log_ping	      	= false;
  profile_to_syslog   	= true;
  allow_direct_sf     	= true;
  do_wprot_routing    	= true;
  do_routing_stats    	= true;
  kill_wp_immediately 	= true;

  if (exist(config_box)) {

    snprintf(tmpfile, LEN_PATH, "%sdpboxcfg.XXXXXX", tempdir);
    mymktemp(tmpfile);
    inf		= sfopen(config_box, FO_READ);
    if (inf < minhandle) {
      append_profile_unr(unr, "cannot open config.box. DPBOX will stop now.");
      ende    	= true;
      return;
    }

    sfdelfile(tmpfile);
    outf      	= sfcreate(tmpfile, FC_FILE);
    if (outf < minhandle) {
      append_profile_unr(unr, "cannot create temporary config file. DPBOX will stop now.");
      sfclose(&inf);
      ende    	= true;
      return;
    }
    
    /* extract all tokens in the temp file */
    while (file2str(inf, hs)) {
      del_blanks(hs);
      if (*hs == '\0' || *hs == '#') continue;
      str2file(&outf, hs, true);
    }
    sfclose(&inf);
    sfclose(&outf);
    
    /* sort the token temp file */
    sort_file(tmpfile, false);
    
    /* reopen the token temp file */
    inf       = sfopen(tmpfile, FO_READ);
    if (inf < minhandle) {
      append_profile_unr(unr, "cannot open temporary config file. DPBOX will stop now.");
      ende    	= true;
      return;
    }
    
    *lasttoken	= '\0';
    while (file2str(inf, hs)) {    
      get_word(hs, w);
      if (!*w) continue;
      strcpy(h2, hs);
      get_word(hs, w1);
      upper(w);
      if (!strcmp(w, lasttoken)) {
      	snprintf(lasttoken, 255, "warning: multiple appearances of keyword %s in config.box", w);  
      	append_profile_unr(unr, lasttoken);
      }
      strcpy(lasttoken, w);
      strcpy(w2, w1);
      upper(w1);

      if (!strcmp(w, "DEFAULTPROMPT")) {
	cut(h2, 80);
	strcpy(default_prompt, h2);
	continue;
      }

      if (!strcmp(w, "UNDEFBOARD")) {
	cut(w1, LEN_BOARD);
	strcpy(default_rubrik, w1);
	continue;
      }

      if (!strcmp(w, "SFCONNECTCALL")) {
	cut(w1, LEN_CALL);
	if (*w1 != '\0')
	  del_callextender(w1, sf_connect_call);
	continue;
      }

      if (!strcmp(w, "REDIST_LOCAL")) {
	cut(w1, 8);
	strcpy(redlocal, w1);
	continue;
      }

      if (!strcmp(w, "REDIST_REGION")) {
	cut(w1, 8);
	strcpy(redregion, w1);
	continue;
      }

      if (!strcmp(w, "REDIST_NATION")) {
	cut(w1, 8);
	strcpy(rednation, w1);
	continue;
      }

      if (!strcmp(w, "REDIST_DEFBOARD")) {
	cut(w1, 8);
	if (valid_boardname(w1))
	  strcpy(reddefboard, w1);
	continue;
      }

      if (!strcmp(w, "REDIST_LIFETIME")) {
	redlifetime = atoi(w1);
	if (redlifetime > 999)
	  redlifetime = -1;
	continue;
      }

      if (!strcmp(w, "INCOMING_LIFETIME")) {
	incominglifetime = atoi(w1) * SECPERDAY;
	continue;
      }

      if (!strcmp(w, "SYSFORWARD")) {
	cut(w1, 8);
	strcpy(e_m_verteiler, w1);
	continue;
      }

      if (!strcmp(w, "PACSRV")) {
	cut(w1, 8);
	strcpy(pacsrv_verteiler, w1);
	continue;
      }

      if (!strcmp(w, "RESUMELT")) {
	resume_lifetime = atoi(w1);
	continue;
      }
      
      if (!strcmp(w, "SUB_BID")) {
	sub_bid = atoi(w1);
	continue;
      }
      
      if (!strcmp(w, "YLT")) {
	y_lifetime = atoi(w1);
	continue;
      }
      
      if (!strcmp(w, "MIN_LIFETIME")) {
	min_lifetime = atoi(w1);
	continue;
      }
      
      if (!strcmp(w, "TTASK")) {
	ttask = atoi(w1) / 5;
	continue;
      }
      
      if (!strcmp(w, "SFIDLEDEFAULT")) {
        nosfpropdelaydefault = !positive_arg(w1);
        continue;
      }
      
      if (!strcmp(w, "STOP_BAD_RCALLS")) {
	stop_invalid_rcalls = positive_arg(w1);
	continue;
      }

      if (!strcmp(w, "STOP_BAD_RLINES")) {
	stop_broken_rlines = positive_arg(w1);
	continue;
      }

      if (!strcmp(w, "STOP_BAD_RDATES")) {
	stop_invalid_rdates = positive_arg(w1);
	continue;
      }

      if (!strcmp(w, "STOP_CHANGED_SENDERS")) {
	stop_changed_senders = positive_arg(w1);
	continue;
      }

      if (!strcmp(w, "STOP_CHANGED_BIDS")) {
	stop_changed_bids = positive_arg(w1);
	continue;
      }

      if (!strcmp(w, "STOP_CHANGED_BOARDS")) {
	stop_changed_boards = positive_arg(w1);
	continue;
      }

      if (!strcmp(w, "STOP_CHANGED_DIST")) {
	stop_changed_mbx = positive_arg(w1);
	continue;
      }

      if (!strcmp(w, "PRIVATE_BADWORDFILTER")) {
	private_dirtycheck = positive_arg(w1);
	continue;
      }

      if (!strcmp(w, "LAZY_MD2")) {
	md2_only_numbers = positive_arg(w1);
	continue;
      }

      if (!strcmp(w, "LOGINCHECK")) {
	login_check = positive_arg(w1);
	continue;
      }

      if (!strcmp(w, "IMPORT_LOGS")) {
	import_logs = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "GUESS_MYBBS")) {
	guess_mybbs = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "MULTI_BBS")) {
	ring_bbs = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "MULTI_MASTER")) {
	multi_master = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "CHARGE_BBS")) {
	charge_bbs = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "FORWARDERRORS")) {
	forwerr = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "BCASTFORWARD")) {
	bcastforward = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "REDUCEEXTRACTLT")) {
	reduce_lt_after_extract = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "AUTO7PLUSEXTRACT")) {
	auto7plusextract = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "AUTOBINEXTRACT")) {
	autobinextract = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "UNPROTO")) {
	unproto_list = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "ALL_UNPROTO")) {
	allunproto = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "TTL")) {
	gttl = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "PROFILE_TO_SYSLOG")) {
	profile_to_syslog = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "ADDEX")) {
	add_ex = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "SMALL_FIRST")) {
	small_first = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "SMART_ROUTING")) {
	smart_routing = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "ACTIVE_ROUTING")) {
	do_wprot_routing = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "PRIV_ROUTER")) {
	route_by_private = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "SEND_BBSBCAST")) {
	send_bbsbcast = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "REQUEST_LT")) {
	request_lt = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "SORT_PROPS")) {
	sort_props = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "SYSLOG")) {
	create_syslog = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "USERLOG")) {
	create_userlog = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "USERSFLOG")) {
	create_usersflog = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "READLOG")) {
	create_readlog = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "SFLOG")) {
	create_sflog = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "CREATE_ACKS")) {
	create_acks = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "CONVLOG")) {
	create_convlog = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "REMOTEERASECHECK")) {
	remoteerasecheck = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "MULTIPRIVATE")) {
	multiprivate = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "HOLDOWNFILES")) {
	holdownfiles = positive_arg(w1);
	continue;
      }
            
      if (!strcmp(w, "SFINPUT")) {
	x = atoi(w1);
	if ((x <= SFI_ALL) && (x >= SFI_NONE))
	  sfinpdefault = x;
	continue;
      }

      if (!strcmp(w, "USERSFTELLMODE")) {
	x = atoi(w1);
	if ((x <= SFT_ALL) && (x >= SFT_NONE))
	  usersftellmode = x;
	continue;
      }

      if (!strcmp(w, "MAXUFCACHE")) {
	maxufcache = atoi(w1);
	continue;
      }
      
      if (!strcmp(w, "MAXSUBSCRIBERDISP")) {
	maxsubscriberdisp = atol(w1);
	continue;
      }
      
      if (!strcmp(w, "RETMAILSIZE")) {
	retmailmaxbytes = atol(w1);
	continue;
      }
      
      if (!strcmp(w, "DEBUGLEVEL")) {
	debug_level = atoi(w1);
	continue;
      }
      
      if (!strcmp(w, "DEBUGSIZE")) {
	debug_size = atol(w1);
	continue;
      }
      
      if (!strcmp(w, "USERTIMEOUT")) {
	usertimeout = atoi(w1);
	continue;
      }
      
      if (!strcmp(w, "SFTIMEOUT")) {
	sftimeout = atoi(w1);
	continue;
      }
      
      if (!strcmp(w, "MAXLTINC")) {
	max_lt_inc = atoi(w1);
	continue;
      }
      
      if (!strcmp(w, "MAXUSERCONNECTS")) {
	maxuserconnects = atoi(w1);
	continue;
      }
            
      if (!strcmp(w, "UFILHIDE")) {
	ufilhide = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "XCHECKINRAM")) {
	xcheck_in_ram = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "VALIDINRAM")) {
	valid_in_ram = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "SFCHECKINRAM")) {
	check_sffor_in_ram = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "WPSCANALL")) {
	scan_all_wp = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "KILL_WP_IMMEDIATELY")) {
	kill_wp_immediately = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "DIRECT_SF")) {
	allow_direct_sf = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "ROUTING_STATS")) {
	do_routing_stats = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "PING")) {
	ping_allowed = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "LOG_PING")) {
	log_ping = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "RETURNTIME")) {
	returntime = atol(w1) * SECPERDAY;
	continue;
      }
      
      if (!strcmp(w, "BULLSFWAIT")) {
	bullsfwait = atol(w1) * SECPERDAY;
	continue;
      }
      
      if (!strcmp(w, "BULLSFMAXAGE")) {
	bullsfmaxage = atol(w1) * SECPERDAY;
	continue;
      }
      
      if (!strcmp(w, "ERASEDELAY")) {
	erasewait = atol(w1) * SECPERDAY;
	continue;
      }
      
      if (!strcmp(w, "UNDEFBOARDSLT")) {
	default_lifetime = atoi(w1);
	continue;
      }
      
      if (!strcmp(w, "FSERVEXPANDLT")) {
	fservexpandlt = atol(w1) * SECPERDAY;
	continue;
      }
      
      if (!strcmp(w, "MAXPERDAY")) {
	all_maxread_day = atol(w1);
	continue;
      }
      
      if (!strcmp(w, "WPCREATEINTERVAL")) {
	wpcreateinterval = atol(w1) * 3600;
	continue;
      }
      
      if (!strcmp(w, "HOLDDELAY")) {
	holddelay = atol(w1) * 3600;
	continue;
      }
      
      if (!strcmp(w, "CONNECTTEXT")) {
	strcpy(connecttext, h2);
	cut(connecttext, 30);
	continue;
      }
      
      if (!strcmp(w, "XEDITOR")) {
	strcpy(xeditor, h2);
	continue;
      }      

      if (!strcmp(w, "FSERVEXPAND")) {
	strcpy(expand_fservboard, w2);
      	if (*expand_fservboard == '/')
	  strdelete(expand_fservboard, 1, 1);
	x = strlen(expand_fservboard);
	if (x > 0 && expand_fservboard[x-1] == '/')
	  expand_fservboard[x-1] = '\0';
	if (*expand_fservboard != '\0') {
	  snprintf(h2, 255, "%s%s%s", boxdir, fservdir, expand_fservboard);
	  if (!exist(h2)) {
	    if (!create_dirpath(h2)) {
      	      append_profile_unr(unr, "config.box: invalid fservexpand definition:");
	      append_profile_unr(unr, h2);
	    }
	  } 
	}
	continue;
      }

      if (!strcmp(w, "CALLSYNTAX")) {
	strcpy(other_callsyntax, h2);
        if (!check_other_callsyntax(other_callsyntax)) {
	  append_profile_unr(unr, "config.box: invalid callsyntax definition:");
	  append_profile_unr(unr, h2);
	}
	continue;
      }

      if (!strcmp(w, "ACCEPTED_BAD_RCALLS")) {
	check_accepted_bad_rcalls_syntax(h2);
	continue;
      }

      if (!strcmp(w, "MUSTCHECK")) {
      	del_blanks(h2);
	del_mulblanks(h2);
	upper(h2);
	p = h2;
	while ((*p)) {
	  if (*p == ' ') *p = ',';
	  p++;
	}
	sprintf(mustcheckboards, ",%s,", h2);
	continue;
      }

      if (!strcmp(w, "OWNHIERNAME")) {
	if (*w1 == '\0') {
	  strcpy(ownhiername, Console_call);
	  continue;
	}
	if (strstr(w1, Console_call) == w1)
	  strcpy(h2, w1);
	else
	  sprintf(h2, "%s.%s", Console_call, w1);
	cut(h2, LEN_MBX);
	strcpy(ownhiername, h2);
	continue;
      }

      if (!strcmp(w, "MAXBULLIDS")) {
	maxbullids = atol(w1);
	continue;
      }
      
      if (!strcmp(w, "WITH_RLINE")) {
	with_rline = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "NO_EXT_RLINE")) {
	no_rline_if_exterior = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "ANA_HPATH")) {
	ana_hpath = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "AUTO_GARBAGE")) {
	auto_garbage = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "PACKDELAY")) {
	packdelay = atoi(w1);
	continue;
      }
      
      if (!strcmp(w, "BOX_PW")) {
	box_pw = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "USERLIFETIME")) {
	userlifetime = atoi(w1);
	continue;
      }
      
      if (!strcmp(w, "GARBAGETIME")) {
	garbagetime = atoi(w1);
	continue;
      }
      
      if (!strcmp(w, "FHEADER")) {
	cut(h2, 107 - strlen(ownhiername));
	strcpy(ownfheader, h2);
	continue;
      }

      if (!strcmp(w, "REMOTE_ERASE")) {
	remote_erase = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "PACKED_SF")) {
	packed_sf = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "SHOW_READERS")) {
	show_readers_to_everyone = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "MAIL_BEACON")) {
	mail_beacon = positive_arg(w1);
	continue;
      }
      
      if (!strcmp(w, "AUTHENTIFICATION") || !strcmp(w, "AUTHENTICATION")) {
	authentinfo = positive_arg(w1);
      	continue;
      }

      if (!strcmp(w, "HOMEBBS")) {
	unhpath(w1, default_tpkbbs);
	if (callsign(default_tpkbbs)) {
	  tpkbbs = true;
	} else {
	  tpkbbs = false;
	  *default_tpkbbs = '\0';
	  sprintf(w2, "invalid HOMEBBS: %s", w1);
	  append_profile_unr(unr, w2);
	}
	continue;
      }
      
      sprintf(w2, "unknown keyword in system/config.box: %s", w);
      append_profile_unr(unr, w2);
      
    }
    sfclosedel(&inf);
  } else {
    append_profile_unr(unr, "system/config.box is not existing. DPBOX will stop now.");
    ende = true;
  }

  if (ttask < 1)
    ttask	= 1;
  else if (ttask > 400)
    ttask	= 400;
  if (*ownhiername == '\0')
    strcpy(ownhiername, Console_call);
  if (maxbullids == 0)
    maxbullids	= 30000;
  if (sftimeout > 45) /* nicht hoeher ! */
    sftimeout	= 45;
  if (usertimeout > 60 * 12) /* 12 h */
    usertimeout	= 60 * 12;
  if (wpcreateinterval < 3600)
    wpcreateinterval = 3600;
  if (holddelay < 3600)
    holddelay	= 3600;
  if (returntime < SECPERDAY*1)
    returntime	= SECPERDAY*1;
  if (bullsfwait < SECPERDAY*1)
    bullsfwait	= SECPERDAY*1;
  if (bullsfmaxage < SECPERDAY*1)
    bullsfmaxage= SECPERDAY*1;
  if (debug_size < 10000)
    debug_size	= 10000;
  if (all_maxread_day <= 0 || all_maxread_day >= 261120)
    all_maxread_day = 999999999L;
  else if (all_maxread_day < 10000)
    all_maxread_day = 10000;
  if (incominglifetime < SECPERDAY*1)
    incominglifetime = SECPERDAY*1;
  if (fservexpandlt < 0)
    fservexpandlt = SECPERDAY*1;
  if ((unsigned)sub_bid > 6)
    sub_bid = 0;
  if (min_lifetime > 999)
    min_lifetime = 0;
  if (retmailmaxbytes < 1000)
    retmailmaxbytes = 1000;
  if (maxufcache > MAXMAXUFCACHE)
    maxufcache = MAXMAXUFCACHE;
  else if (maxufcache < 25)
    maxufcache = 25;
    
  check_rheaders  = (stop_invalid_rcalls || stop_broken_rlines 
      	      	  || stop_invalid_rdates || stop_changed_bids
		  || stop_changed_boards || stop_changed_mbx
		  || stop_changed_senders );
}

static void check_for_7plus(void)
{
  pathstr	cp;
  char		STR1[20], STR2[20], STR3[20];
  
  sfgetdir(0, cp);
  sfchdir(boxdir);
  strcpy(STR1, "7plus.out");
  sfdelfile(STR1);
  strcpy(spluspath, "7plus");
  my_system("7plus -Y -Q");

  if (!exist(STR1))
    *spluspath = '\0';

  sfdelfile(STR1);

  if (*spluspath) {
    strcpy(STR2, "7plus.fls");
    sfdelfile(STR2);
    strcpy(STR3, "7ptest");
    sfdelfile(STR3);
    append(STR3, "abcdefghijklmnopqrstuvwxyz", true);
    my_system("7plus 7ptest -Y -Q -#");
    sfdelfile(STR1);
    new_7plus = exist(STR2);
    sfdelfile(STR2);
    sfdelfile(STR3);
    strcat(STR3, ".7pl");
    sfdelfile(STR3);
  }

  sfchdir(cp);
}

void load_all_parms(short unr)
{
  pathstr	STR1;

  dispose_mptr();
  init_ufcache();
  clear_hboxhash();
  clear_bidhash();
  free_languages(&langmemroot);
  dispose_boxcommands();
  dispose_sfinfos();
  dispose_rubrikinfos();
  dispose_verteilinfos();
  dispose_rejectinfos();
  dispose_badwords();
  dispose_rsysops();
  dispose_convtit();
  dispose_convlt();
  dispose_tcpip();
  if (balisesize != 0)
    free(balisebuf);
  balisesize = 0;
  del_resume_list();
  convtit = false;
  free_boxbcastdesc();
  free_unprotodefs();

  load_boxconfig(unr);
  load_boxcommands();
  sprintf(STR1, "%s%c%cLAN", boxlanguagedir, allquant, extsep);
  load_languages(&langmemroot, STR1);
  load_sfinfos();
  load_rubrikinfos(unr);
  load_verteilinfos(unr);
  load_rejectinfos();
  load_badwords();
  load_fbbheader();
  load_balise();
  load_resume();
  load_tcpipbuf();
  load_boxbcastparms(bcast_box);
  load_convtit(unr);
  load_convlt();
  load_rsysops();
  load_mptr();
  boxloadgreeting();
  reload_proto();
  load_unprotodefs();
  check_for_7plus();
  if (*spluspath && !new_7plus) {
    strcpy(STR1, "You are using an outdated 7plus version. Check for a new version >= v2.21");
    append_profile(-1, STR1);
    if (boxrange(unr)) wlnuser(unr, STR1);
  }
  m_filter = exist(m_filter_prg);
  crawler_exists = exist(crawlname);
  lastxreroute = 0;
  garbage_collection(true, false, false, false, -1);
  if (*default_tpkbbs) {
    complete_hierarchical_adress(default_tpkbbs);
    if (strchr(default_tpkbbs, '.') == NULL) {
      sprintf(STR1, "warning: no hierarchical definition known for HOMEBBS %s", default_tpkbbs);
      append_profile(-1, STR1);
      if (boxrange(unr)) wlnuser(unr, STR1);
    }
  }
}


static void check_indexes(void)
{
  DTA		dirinfo;
  short		result;
  long		sz;
  pathstr	STR1;

  snprintf(STR1, LEN_PATH, "%s%c%c%s", indexdir, allquant, extsep, EXT_IDX);
  result	= sffirst(STR1, 0, &dirinfo);
  if (result != 0)
    return;

  sprintf(STR1, "%s%s", indexdir, dirinfo.d_fname);
  sz		= sfsize(STR1);

  if (sz % sizeof(indexstruct) != 0) {
    boxalert2("The format of the box-indicies has changed. Please delete the content of the bbs manually", false);
    ende	= true;
  }
}


/* local fuer check_bullidseek */

static long lastbid(void)
{
  short		log, lognr;
  boxlogstruct	logheader;

  lognr		= sfsize(boxlog) / sizeof(boxlogstruct);
  log		= sfopen(boxlog, FO_READ);
  if (log < minhandle)
    return -1;
  read_log(log, lognr, &logheader);
  if (actmsgnum <= 0)
    actmsgnum	= logheader.msgnum;
  sfclose(&log);
  return (bull_mem(logheader.bid, false));
}


void check_bullidseek(void)
{
  /* falls das bidseek-file fehlt... */
  long		fpos, bs;

  fpos		= bullidseek;
  bs		= sfsize(msgidlog) / sizeof(bidtype);
  if ((bullidseek < bs && bs < maxbullids) || bullidseek > bs) {
    fpos	= bs;

  } else if (bs >= maxbullids) {
    if (sfsize(boxlog) % sizeof(boxlogstruct) != 0)
      create_new_boxlog(0, false);

    fpos	= lastbid();

    /* die folgenden Zeilen sind wegen der ' BULLID + ' - Funktion da */
    if (fpos >= 0) {
      if (fpos < bullidseek) {
	if (bullidseek - fpos < 50)
	  fpos	= bullidseek;
      }
    }
  }

  if (bullidseek == fpos)
    return;

  if (fpos > 0) {
    bullidseek	= fpos;
    update_bidseek();
  }
}

static boolean msgnumsem = false;

void create_all_msgnums(boolean recalc)
{
  long		bsize, xsize;
  short		k, h;
  long		ct, cct, l;
  boxlogstruct	log;
  indexstruct	header;
  pathstr	fn, lfn;
  char		hs[256], STR1[100];
  
  if (msgnumsem)
    return;

  msgnumsem	= true;
  actmsgnum	= 0;
  bsize		= sfsize(boxlog);
  if (!recalc && (bsize % sizeof(boxlogstruct) != 0 || bsize == 0)) {
    boxbusy("creating checklist");
    create_new_boxlog(-1, false);
    boxendbusy();
    bsize	= sfsize(boxlog);
  }

  if (bsize <= 0) {	/* leere bbs */
    msgnumsem	= false;
    return;
  }
  
  append_profile(-1, "creating msgnums");
  
  xsize		= sfsize(sflist);	/* delete msgnums in X */
  k		= open_index("X", FO_RW, true, true);
  if (k >= minhandle) {
    l	= xsize / sizeof(indexstruct);
    for (ct = 1; ct <= l; ct++) {
      read_index(k, ct, &header);
      if (check_hcs(header)) {
        header.msgnum	= 0;
        write_index(k, ct, &header);
      }
    }
    close_index(&k);
  }
  
  cct		= bsize / sizeof(boxlogstruct);
  k		= sfopen(boxlog, FO_RW);
  if (k < minhandle) {   /* ? */
    msgnumsem	= false;
    return;
  }

  boxbusy("creating msgnums");

  *lfn		= '\0';
  h		= nohandle;
  for (ct = 1; ct <= cct; ct++) {
    if (ct % 25 == 0) {
      dp_watchdog(2, 4711);   /* reset watchdog */
      sprintf(hs, "msgnums: %d%%", calc_prozent(ct, cct));
      boxendbusy();
      boxbusy(hs);
    }

    *log.brett	= '\0';
    read_log(k, ct, &log);

    idxfname(fn, log.brett);
    if (strcmp(fn, lfn)) {
      strcpy(lfn, fn);
      sfclose(&h);
      h		= sfopen(fn, FO_RW);
    }

    if (h >= minhandle) {
      *header.absender	= '\0';
      read_index(h, log.idxnr, &header);
      if (check_hcs(header)) {
	header.msgnum	= new_msgnum();
	log.msgnum	= header.msgnum;
	write_index(h, log.idxnr, &header);
	write_log(k, ct, &log);
      } else {
	sprintf(STR1, "checksum error (index %s)", log.brett);
	debug(0, -1, 163, STR1);
      }
    } else {
      sprintf(STR1, "open error (index %s)", log.brett);
      debug(0, -1, 163, STR1);
    }
  }

  sfclose(&h);
  sfclose(&k);
  boxendbusy();
  unproto_last	= actmsgnum;
  start_crawl(-1, "-q");
  sprintf(hs, "%s -d %s", crawlname, crawldir);
  my_system(hs);
  crawl_active	= false;
  flush_bidseek_immediately();
  append_profile(-1, "msgnums created");
  msgnumsem	= false;
}


static long check_msgnum(void)
{
  long		Result;
  short		k;
  long		size;
  boxlogstruct	log;

  Result	= actmsgnum;
  size		= sfsize(boxlog);
  if (size <= 0)
    return Result;

  k		= sfopen(boxlog, FO_READ);
  if (k < minhandle)
    return Result;

  log.msgnum	= -157;
  read_log(k, size / sizeof(boxlogstruct), &log);
  sfclose(&k);
  if (log.msgnum > 0 && actmsgnum < log.msgnum)
    actmsgnum	= log.msgnum;

  return Result;
}

static void init_filenames(void)
{
  snprintf(boxlog, LEN_PATH, "%smsglist%cbox", boxstatdir, extsep);
  snprintf(eralog, LEN_PATH, "%smsglist%cupd", boxstatdir, extsep);
  snprintf(bidseekfile, LEN_PATH, "%sbidseek%cbox", boxstatdir, extsep);
  snprintf(extuserdir, LEN_PATH, "%sextusers%c", boxstatdir, dirsep);
  snprintf(fservdir, LEN_PATH, "fileserv%c", dirsep);
  snprintf(pservdir, LEN_PATH, "privserv%c", dirsep);
  snprintf(crawldir, LEN_PATH, "%scrawler%c", boxdir, dirsep);
  snprintf(crawlname,LEN_PATH, "%scrawler", crawldir);
  snprintf(temperase, LEN_PATH, "%srerase%ctmp", boxstatdir, extsep);
  idxfname(sflist, "X");
  idxfname(userinfos, "M");
}

void init_boxvars(void)
{
  short	x;
  long	l;
  char	dbold[256], w[256];

  profile_to_syslog   	= true;
  wrong_clock 	      	= false;

  boxisbusy(true);
  strcpy(dbold, debug_box);
  new_ext(dbold, "001");
  validate(dbold);
  sfdelfile(dbold);
  sfrename(debug_box, dbold);

  reset_boxruntime();
  quick_speed();

  init_filenames();
  
  /* this one is no longer used (v5.08.44) */
  snprintf(w, 255, "%swpfor%cbox", boxsysdir, extsep);
  sfdelfile(w);

  delete_tempboxfiles();

  strcpy(w, boxstatdir);
  strcat(w, "7plock.box");
  sfdelfile(w); /* 7plus-Lockfile loeschen */
  
  crawl_active		= false;
  crawler_exists	= false;
  crawl_private       	= false;
  
  check_disk_full();

  for (x = 0; x <= MAXUSER; x++)
    user[x]	= NULL;
  for (x = 0; x <= MAXMAXUFCACHE; x++)
    ufcache[x]	= NULL;

  clear_msgnumarr();
  actmsgnum		= 0;
  indexcacheroot	= NULL;
  indexcaches		= 0;
  zombieroot		= NULL;
  immediate_extcheck	= false;
  newmailroot		= NULL;
  protocalls		= NULL;
  traces		= NULL;
  mptrroot		= NULL;
  routing_root	      	= NULL;
  badtimecount		= 0;
  ttask			= 20;
  maxerrors		= 5;
  tell_waiting		= true;
  x_garbage_waiting	= false;
  new_mybbs_data	= true;
  mybbsseekp		= 0;
  new_remote_erases	= true;
  remerseekp		= 0;
  rubrikroot		= NULL;
  transferroot		= NULL;
  boxtimect		= 0;
  sfdefs		= NULL;
  balisesize		= 0;
  balisetime		= 0;
  lastbalise		= 0;
  debug_level		= 0;
  bcommandroot		= NULL;
  sf_allowed		= true;
  gesperrt		= false;
  langmemroot		= NULL;
  resume_root		= NULL;
  tcpiproot		= NULL;
  convtitroot		= NULL;
  convltroot		= NULL;
  rsysoproot		= NULL;
  badwordroot		= NULL;
  bcommandroot		= NULL;
  rejectroot		= NULL;
  unprotodefroot	= NULL;
  whichlangmem	      	= NULL;
  hiscore_connects	= 0;
  convtit		= false;
  *other_callsyntax   	= '\0';
  *expand_fservboard   	= '\0';
  init_wp_timers(); /* only initial setup. is overwritten with stored values in load_bidseek() */
  
  if (calc_qth(myqthwwloc, &mylaenge, &mybreite))
    myqthwwlocvalid	= true;
  else
    myqthwwlocvalid	= false;
  
  load_all_parms(-1);
  load_routing_table(); /* only one time ! */
  calc_routing_table();
  check_indexes();
  check_hpath(false);

  if (!load_bidseek())
    check_bullidseek();
  l	= sfsize(msgidlog) / sizeof(bidtype);
  if (bullidseek > l)
    bullidseek	= l;
  if (l < maxbullids)
    bullidseek	= l;

  if (crawler_exists) { /* unlock crawler database */
    sprintf(w, "%s -u %s", crawlname, crawldir);
    my_system(w);
  }

  actmsgnum	= check_msgnum();
  unproto_last	= actmsgnum;

  if (actmsgnum < 1) {
    create_all_msgnums(false);   /* recreate msgnums... */
  }
  fill_msgnumarr();

  boxisbusy(false);
}


void exit_boxvars(void)
{
  short	x;

  save_routing_table(); /* save the last measurements */
  kill_zombies();
  append_profile(-1, "*** shutdown ***");
  sane_shutdown(4711);
  flush_bidseek_immediately();
  delete_all_protocalls();
  delete_all_tracecalls();
  for (x = 1; x <= MAXUSER; x++) {
    if (boxrange(x)) {
      user[x]->action	= 15;
      abort_useroutput(x);
      end_boxconnect(x);
    }
  }
}

