/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#include <unistd.h>

#define BOX_TIM_G
#include "main.h"
#include "box_tim.h"
#include "tools.h"
#include "box_garb.h"
#include "box_logs.h"
#include "sort.h"
#include "boxbcast.h"
#include "yapp.h"
#include "boxlocal.h"
#include "box_send.h"
#include "box_sub.h"
#include "box_sys.h"
#include "box_sf.h"
#include "box.h"
#include "box_file.h"
#include "box_inou.h"
#include "box_mem.h"
#include "boxfserv.h"
#include "shell.h"
#include "status.h"
#include "box_rout.h"
#include "box_wp.h"


/* Wird angesprungen, wenn endlich der Spooler nach Eingabe von QUIT */
/* leergelaufen ist (unter anderem wegen //COMP ON). Beendet den     */
/* Connect zur Box endgueltig.                                       */

void end_boxconnect(short unr)
{
  short s, e;
  boolean bbs;

  if (!boxrange(unr))
    return;
  debug0(3, -1, 150);
  s = user[unr]->tcon;
  e = user[unr]->pchan;
  bbs = user[unr]->f_bbs;
  melde_user_ab(unr, user[unr]->action == 14);
  if (bbs) {
    if (s > 0) {
      boxendbox(s);
      return;
    }
    if (e > 0)
      boxpabortsf(e, true);
    return;
  }
  if (s > 0) {
    boxendbox(s);
    return;
  }
  if (e > 0)
    boxpacketendbox(e);
}


void spoolendcheck(short unr)
{
  userstruct *WITH;

  if (!boxrange(unr))
    return;

  WITH = user[unr];
  if (boxspoolstatus(WITH->tcon, WITH->pchan, -1) <= 0)
    end_boxconnect(unr);
}


void tell_command(char *call, char *bid, char *frage)
{
  short		k;
  char		tellname[256];

  if (!callsign(call))
    return;

  k = strlen(bid);
  if (k < 1 || k > LEN_BID)
    return;

  if (count_words(frage) < 3)
    return;

  sprintf(tellname, "%sTELLREQ%cXXXXXX", boxstatdir, extsep);
  mymktemp(tellname);
  k = sfcreate(tellname, FC_FILE);
  if (k < minhandle)
    return;

  str2file(&k, call, true);
  str2file(&k, bid, true);
  str2file(&k, frage, true);
  tell_waiting = true;
  sfclose(&k);
  write_msgid(-1, bid);
}


void stop_tell(short unr)
{
  char hs[256], mbx[256];

  if (!boxrange(unr))
    return;

  sfclose(&user[unr]->tell);

  if (sfsize(user[unr]->tellfname) <= 300000L) {
    strcpy(mbx, user[unr]->tellmbx);
    if (!gen_sftest2(-1, user[unr]->call, mbx)) {
      user_mybbs(user[unr]->call, hs);
      if (*hs == '\0')
	strcpy(hs, Console_call);
      strcpy(mbx, hs);
    }

    sprintf(hs, "%s @ %s < %s tell reply from %s",
	    user[unr]->call, mbx, Console_call, Console_call);
    strcpy(user[unr]->input2, user[unr]->tellfname);
    send_check(unr, hs, false, 'P');
  }

  sfdelfile(user[unr]->tellfname);
  user[unr]->tellfname[0] = '\0';
  melde_user_ab(unr, false);
}


boolean tell_processing(char *tellfile)
{
  boolean Result;
  short unr, tf, x;
  char telluser[256];
  char request[256];
  char mbx[256];
  char bid[256];

  debug(1, -1, 121, tellfile);
  boxsetbmouse();
  Result = true;

  tf = sfopen(tellfile, FO_READ);
  if (tf < minhandle)
    return Result;
  file2str(tf, telluser);
  file2str(tf, bid);
  file2str(tf, request);
  sfclosedel(&tf);
  sfdelfile(tellfile);
      /* eigentlich ueberfluessig... aber es gab mal Probleme */

  x = strlen(bid);
  if (!(callsign(telluser) && x > 0 && x <= LEN_BID))
    return Result;

  delete_brett_by_bid("T", "", bid, false, true);

  x = strlen(request);
  while (x > 0 && request[x - 1] != '@')
    x--;
  if (x > 0) {
    strsub(mbx, request, x + 1, strlen(request) - x);
    cut(request, x - 1);
    del_leadblanks(mbx);
    cut(mbx, LEN_MBX);
    del_lastblanks(request);

    unr = melde_user_an(telluser, 0, 0, UM_TELLREQ, false);
    if (unr > 0) {
      Result = true;
      strcpy(user[unr]->tellmbx, mbx);
      box_input(unr, false, request, true);
    } else
      Result = false;
  }
  boxsetamouse();
  debug(1, -1, 121, "ended (may be continued in background)");
  return Result;
}


void tell_check(void)
{
  short result;
  DTA dirinfo;
  boolean okproc;
  char STR1[256];

  okproc = true;
  do {
    sprintf(STR1, "%sTELLREQ%c%c", boxstatdir, extsep, allquant);
    result = sffirst(STR1, 0, &dirinfo);
    if (result == 0 && okproc) {
      sprintf(STR1, "%s%s", boxstatdir, dirinfo.d_fname);
      okproc = tell_processing(STR1);
      sprintf(STR1, "%s%s", boxstatdir, dirinfo.d_fname);
      sfdelfile(STR1);
    }
  } while (result == 0 && okproc != false);
  tell_waiting = (result == 0);
}


void do_quit(short unr, boolean abort, boolean verbose)
{
  char STR1[256];

  if (!boxrange(unr))
    return;

  if (verbose) {
    sprintf(STR1, "%scookies%cdoc", boxsysdir, extsep);
    if (give_cookie(unr, wlnuser, STR1))
      wlnuser0(unr);
    show_stat(unr, unr);
    wlnuser(unr, "73 !");
  }

  boxspoolread();
  if (abort)
    user[unr]->action = 15;
  else
    user[unr]->action = 14;
}


void timeout_check(short unr)
{
  char hs[256];
  userstruct *WITH;

  if (!boxrange(unr))
    return;
  WITH = user[unr];
  if (!callsign(WITH->call))
    return;
  if (user[unr]->console != false)
    return;
  if (WITH->pchan > 0) {
    if (boxspoolstatus(0, WITH->pchan, -1) > 0)
      WITH->lastatime = clock_.ixtime;
    else if (boxunackframes(WITH->pchan) > 0)
      WITH->lastatime = clock_.ixtime;
  }
  if (WITH->f_bbs || in_real_sf(WITH->call)) {
    if (clock_.ixtime - WITH->lastatime < sftimeout * 60 &&
	clock_.ixtime - WITH->lastcmdtime < 3600)
      return;
    if ((unsigned)WITH->action < 32 && ((1L << WITH->action) & 0xc000L) != 0)
    {   /* endgueltig ende */
      if (clock_.ixtime - WITH->lastatime >= sftimeout * 120)
	abort_useroutput(unr);
      return;
    }
    debug(1, unr, 122, "s&f");
    sprintf(hs, "%s: Timeout", user[unr]->call);
    abort_sf(unr, false, hs);
    return;
  }
  if (clock_.ixtime - WITH->lastatime < usertimeout * 60 &&
      clock_.ixtime - WITH->lastcmdtime < 12 * 3600) /* 12 Stunden */
    return;
  if ((unsigned)WITH->action < 32 && ((1L << WITH->action) & 0xc000L) != 0)
      /* rote Karte */
      {  /* dann kam schon ein QUIT/ABORT */
    if (clock_.ixtime - WITH->lastcmdtime >= 13 * 3600) /* 13 Stunden */
      abort_useroutput(unr);
    return;
  }
  WITH->action = 0;
  debug(1, unr, 122, "user");
  wlnuser(unr, "TIMEOUT");
  do_quit(unr, true, false);
}


static void show_mailbeacon2(char *ofiname)
{
  short tnc, x;
  long rp;
  char *btbuff;
  long btsize;

  boolean found;

  short chan, plen;
  char hs[256], w[256], hs2[256];
  char statustext[256];
  char mailtext[256];
  char adress[256];
  long rpx;
  short iface;
  char qrg[256];

  debug(1, 0, 123, "transmitting...");
  boxsetbmouse();
  sprintf(statustext, "< %%u > dpbox v%%v - %s", Console_call);
  strcpy(mailtext, "Mail for :%r");
  strcpy(adress, "MAIL");
  
  tnc = 0;

  if (balisesize > 0) {
    rp = 0;
    do {
      get_line(balisebuf, &rp, balisesize, hs);
      get_word(hs, w);
      upper(w);
      if (!strcmp(w, "STATUSTEXT"))
	strcpy(statustext, hs);
      else if (!strcmp(w, "MAILTEXT"))
	strcpy(mailtext, hs);
    } while (rp < balisesize);
  }

  expand_macro(-1, statustext);
  expand_macro(-1, mailtext);

  sfbread(true, ofiname, &btbuff, &btsize);
  if (btbuff == NULL) {
    btbuff = malloc(4);
    if (btbuff != NULL) {
      nstrcpy(btbuff, "NONE", 4);
      btsize = 4;
    }
  }
  if (btbuff != NULL) {
    rp = 0;
    *qrg = '\0';
    while (rp < balisesize) {
      found = false;
      get_line(balisebuf, &rp, balisesize, hs);
      upper(hs);
      get_word(hs, w);
      if (!strcmp(w, "QRG")) {
	get_word(hs, w);
	upper(w);
	iface = -1;
	if (find_socket(w, &iface)) {
	  strcpy(adress, hs);
	  strcpy(qrg, w);
	  found = true;
	}
      }
      if (!found)  /* jetzt wird's ernst */
	continue;
      boxpushunproto(tnc);
      boxsetunproto(tnc, adress, iface, qrg);
      chan = boxgettncmonchan(tnc);
      strcpy(hs, statustext);
      if (*hs != '\0')
	boxsendpline(chan, hs, true, iface);
      if (btsize > 0) {  /* Calls senden */
	plen = boxgetpaclen(chan);
	rpx = 0;
	while (rpx < btsize) {
	  *hs = '\0';
	  strcpy(hs2, mailtext);
	  x = strlen(hs2);
	  while (x > 0 && hs2[x - 1] != '\015')
	    x--;
	  if (hs2[x - 1] == '\015') {
	    sprintf(hs, "%.*s", x, hs2);
	    strdelete(hs2, 1, x);
	  }

	  while (strlen(hs) + strlen(hs2) < plen - 9 && rpx < btsize) {
	    if (strlen(hs2) >= 72) {
	      sprintf(hs + strlen(hs), "%s \015", hs2);
	      *hs2 = '\0';
	    }
	    get_line(btbuff, &rpx, btsize, w);
	    if (*hs2 != '\0')
	      sprintf(hs2 + strlen(hs2), " %s", w);
	    else
	      strcpy(hs2, w);
	  }
	  strcat(hs, hs2);
	  boxsendpline(chan, hs, true, iface);
	}

      }
      boxpopunproto(tnc);
    }
    free(btbuff);
  }

  boxsetamouse();
}


void show_mailbeacon(void)
{
  short x, ofi, result;
  boolean noch;
  userstruct uf;
  char ofiname[256];
  char ofiname2[256];
  char hs[256];
  DTA dirinfo;
  char STR1[256];

  debug0(3, 0, 123);
  boxsetbmouse();

  sprintf(ofiname2, "%sBALISE%cTX", tempdir, extsep);

  if (balisenumber == 0) {
    laststartbalise = clock_.ixtime;

    sprintf(ofiname, "%sBALISE%cDIR", tempdir, extsep);

    sfdelfile(ofiname2);

    ofi = sfcreate(ofiname, FC_FILE);
    if (ofi >= minhandle) {
      sprintf(STR1, "%s%c%c%s", indexdir, allquant, extsep, EXT_IDX);
      result = sffirst(STR1, 0, &dirinfo);
      while (result == 0) {
	strcpy(hs, dirinfo.d_fname);
	del_ext(hs);
	cut(hs, 8);
	upper(hs);
	if (callsign(hs))
	  str2file(&ofi, hs, true);
	result = sfnext(&dirinfo);
      }
      sfclose(&ofi);
      baliseh1 = sfopen(ofiname, FO_READ);
    }
  }

  ofi = baliseh1;
  if (ofi >= minhandle) {
    if (file2str(ofi, hs)) {
      balisenumber++;

      noch = false;
      for (x = 1; x <= MAXUSER; x++) {   /* user ist grade im sf eingeloggt */
	if (user[x] != NULL) {
	  if (!strcmp(user[x]->call, hs)) {
	    if (user[x]->f_bbs)
	      noch = true;
	  }
	}
      }

      if (!noch && boxcheck(false, hs) > 0) {
	load_userfile(true, false, hs, &uf);
	if (!uf.hidebeacon)
	  append(ofiname2, hs, true);
      }

    } else {
      sfclosedel(&baliseh1);
      sort_file(ofiname2, false);
      show_mailbeacon2(ofiname2);
      sfdelfile(ofiname2);
      lastbalise = laststartbalise;
      balisenumber = 0;
    }

  } else {
    lastbalise = laststartbalise;
    balisenumber = 0;
    sfdelfile(ofiname2);
  }

  boxsetamouse();
}


void balise_check(void)
{
  if (balisenumber == 0 &&
      (clock_.ixtime - lastbalise >= balisetime ||
       clock_.ixtime - lastbalise < 0))
    show_mailbeacon();
}


void start_mailbeacon_manually(short unr)
{
  time_t sec;
  char hs[256];

  if (balisenumber == 0) {
    show_mailbeacon();
    wlnuser(unr, "OK, processing mail beacon");
    return;
  }
  sec = clock_.ixtime - laststartbalise;
  sprintf(hs, "mail beacon generation in process since %ld seconds", sec);
  wlnuser(unr, hs);
}

void start_crawl(short unr, char *par)
{
  int status;
  time_t stime;
  char w[256];
  
  crawler_exists  = exist(crawlname);
  
  if (!crawler_exists) {
    crawl_active  = false;
    crawl_private = false;
    crawl_started = 0;
    wlnuser(unr, "no crawler installed");
    return;
  }
  
  if (crawl_started != 0) {
    wlnuser(unr, "crawler currently running");
    if (!strcmp(par, "-q")) {
      crawl_started = 0;
      wlnuser(unr, "waiting for stop");
      boxspoolread();
      stime = time(NULL);
      while (crawl_pid > 0
		&& waitpid(crawl_pid, &status, DP_WNOHANG) == 0
		&& time(NULL) - stime < 20) {
      	sleep(1);
      }
      wlnuser(unr, "now stopped");
      crawl_active = false;
    } else {
      wlnuser(unr, "stop with <crawl -q>");
    }
    return;
  } else if (!strcmp(par, "-q")) {
    wlnuser(unr, "crawl not running");
    crawl_active = false;
    update_bidseek();
    wlnuser(unr, "stopped scanning of new files");
    return;
  } else if (!strcmp(par, "-a1")) {
    if (crawl_active) {
      wlnuser(unr, "scanning of new files already active");
      return;
    }
    crawl_active = true;
    update_bidseek();
    wlnuser(unr, "started scanning of new files");
    return;
  } else if (!strcmp(par, "-d")) {
    wlnuser(unr, "OK, will stop crawler and delete database");
    crawl_active = false;
    crawl_started = 0;
    sprintf(w, "%s -d %s", crawlname, crawldir);
    add_zombie(my_exec(w), "", 0);
    return;
  }
  
  if (!strcmp(par, "-p")) {
    crawl_private = true;
    *par = '\0';
    wlnuser(unr, "OK, will include private mails in crawler database"); 
  } else crawl_private = false;

  sprintf(w, "%s -d %s", crawlname, crawldir);
  my_system(w);
  crawl_active = false;
  update_bidseek();
  crawl_started = clock_.ixtime;
  crawl_lastchecknum = 0;
  crawl_pid = 0;
  strcpy(crawl_args, par);
  wlnuser(unr, "OK, will start crawling");
}

static boolean immediate_zombiecheck;

#define maxrunargs 50
static void process_crawl(void)
{
  boxlogstruct logheader;
  long last, seek;
  int status;
  short handle;
  char vname[256];
  char command[256];
  char hs[256], hs2[10];
  short ct;
  indexstruct header;
  char *args[maxrunargs];
  
  if (!crawler_exists) {
    crawl_started = 0;
    crawl_active = false;
    crawl_pid = 0;
    return;
  }
  
  if (crawl_pid > 0) {
    if (waitpid(crawl_pid, &status, DP_WNOHANG) != 0) {
      crawl_pid = 0;
      immediate_extcheck = true;
    } else {
      return;
    }
  }
  
  if (crawl_pid == 0) {
    if (disk_full) {
      crawl_started = 0;
      crawl_active = false;
      update_bidseek();
      return;
    }
    
    debug0(4, -1, 209); 

    last = sfsize(boxlog) / sizeof(boxlogstruct);
    if (crawl_lastchecknum >= last) {
      crawl_started = 0;
      crawl_active = true;
      update_bidseek();
      return;
    }

    handle = sfopen(boxlog, FO_READ);
    if (handle < minhandle) {
      crawl_started = 0;
      return;
    }
    
    seek = (crawl_lastchecknum) * sizeof(boxlogstruct);
    if (sfseek(seek, handle, SFSEEKSET) != seek) {
      crawl_started = 0;
      crawl_active = false;
      sfclose(&handle);
      return;
    }
    
    do {
      seek = sfread(handle, sizeof(boxlogstruct), (char *)&logheader);
    } while (crawl_lastchecknum++ <= last
             && seek == sizeof(boxlogstruct)
             && ((logheader.msgtype != 'B' && crawl_private != true)
             	 || (logheader.deleted == true)
             	 || (!strcmp("Y", logheader.brett))
             	 || (!strcmp("T", logheader.brett))
                 || ((logheader.pmode & (TBIN+T7PLUS)) != 0)));
                 
    sfclose(&handle);
    
    if (seek != sizeof(boxlogstruct)) {
      crawl_started = 0;
      if (crawl_lastchecknum >= last) {
        crawl_active = true;
        update_bidseek();
      }
      return;
    }

    mytmpnam(vname);
    handle = sfcreate(vname, FC_FILE);
    if (handle < minhandle)
      return;

    read_brett(-2, handle, logheader.brett, logheader.idxnr, logheader.idxnr,
    		 100, "+BC", "", 0, &header);
    str2file(&handle, "", true);
    str2file(&handle, logheader.betreff, true);
    str2file(&handle, logheader.absender, true);
    str2file(&handle, logheader.obrett, true);
    str2file(&handle, logheader.brett, true);
    sfclose(&handle);

    if ((crawl_pid = fork())) {
      add_zombie(crawl_pid, vname, 0);
      return;
    }
    
    for (handle = minhandle; handle <= maxhandle; handle++) close(handle);
    setsid();
    dup(0);
    dup(0);
        
    for (ct = 0; ct < maxrunargs; ct++)
      args[ct] = NULL;
   
    sprintf(hs, "%ld", header.msgnum);
    strcpy(command, crawl_args);
    
    args[0] = crawlname;
    strcpy(hs2, "-a");
    args[1] = hs2;
    args[2] = crawldir;
    args[3] = vname;
    args[4] = hs;
    args[5] = strtok(command, " \t");
    ct = 6;
    do {
      args[ct] = strtok(NULL, " \t");
    } while (args[ct++] != NULL && ct < maxrunargs);
    
    execv(crawlname, args);
    exit(1);
  }
}
#undef maxrunargs

void query_crawl(short unr, char *query)
{
  char s[256], fname[256];
  
  if (!crawler_exists) {
    wlnuser(unr, "no database");
    return;
  }
    
  mytmpnam(fname);
  strcpy(user[unr]->wait_file, fname);
  user[unr]->action = 500;
  sprintf(s, "%s -s %s %s %s", crawlname, crawldir, fname, query);
  user[unr]->wait_pid = my_exec(s);
}

static time_t lastxgartime;


static void chk_xgar(void)
{
  if (lastxgartime == 0) {
    lastxgartime = clock_.ixtime;
    return;
  }

  if (!x_garbage_waiting)
    return;

  if (clock_.ixtime - lastxgartime <= 300) /* 5 Minuten */
    return;

  garbage_collection(true, false, false, false, -1);
  /* Rubrik 'X' aufraeumen */
  lastxgartime	= clock_.ixtime;
}



static void retarded_msg(short unr)
{
  short	x, k;
  char	hs[256];

  x	= str2disturb_chan(user[unr]->call);
  if (x <= 0)
    return;

  user[unr]->newmsg	= false;
  sprintf(hs, "%s%s%cMSG", boxstatdir, user[unr]->call, extsep);
  k	= sfopen(hs, FO_READ);
  if (k < minhandle)
    return;

  while (file2str(k, hs))
    wlnuser(x, hs);
  sfclosedel(&k);
  show_prompt(x);
}



void add_zombie(pid_t pid, char *fname, short exitaction)
{
  zombietype	*zz;
  short		slen;

  debug(2, 0, 224, fname);

  if (pid <= 0) return;

  zz	= malloc(sizeof(zombietype));
  if (zz == NULL) return;

  zz->pid	= pid;
  zz->exitaction= exitaction;
  if ((slen = strlen(fname)) == 0)
    zz->killfile= NULL;
  else {
    if ((zz->killfile = malloc(slen+1)) != NULL)
      strcpy(zz->killfile, fname);
  }
  if (zombieroot == NULL) {
    zz->next	= NULL;
    zombieroot	= zz;
  } else {
    zz->next	= zombieroot->next;
    zombieroot->next = zz;
  }
  immediate_zombiecheck = true;
}


void kill_zombies(void)
{
  zombietype	*zz, *hz, *hz1;
  char	      	*p;
  pathstr      	hs;
  long		ret;
  int		status;
  char	      	crcs[40];

  zz		= zombieroot;
  hz		= NULL;
  while (zz != NULL) {
    ret		= waitpid(zz->pid, &status, DP_WNOHANG);
    if (ret != -1 && ret != zz->pid) {
      hz	= zz;
      zz	= zz->next;
      continue;
    }

    switch (zz->exitaction) {
      case 1  : if (zz->killfile != NULL) {
      	      	  p = strstr(zz->killfile, " CRC:");
      	      	  if (p != NULL) {
		    *p = '\0';
		    p++;
      	      	    snprintf(crcs, 39, "CRC:%d %ld", file_crc(0, zz->killfile, 0xFFFF, 0, 0), sfsize(zz->killfile));
		    if (strcmp(p, crcs)) p = NULL; /* == CRC is changed, file is edited */
		  }
		  if (p == NULL) { /* == CRC is changed, or no CRC was available -> import reply */
      	      	    conv_file_from_local(zz->killfile); /* umlaut conversion */
      	      	    snprintf(hs, LEN_PATH, "%simport%cXXXXXX", newmaildir, extsep);
		    mymktemp(hs);
		    sfrename(zz->killfile, hs);
      	      	    sort_new_mail(-1, hs, Console_call);
		  }
      	      	}
		break;
      default :	break;
    }
    
    if (zz->killfile != NULL) {
      debug(2, 0, 225, zz->killfile);
      sfdelfile(zz->killfile);
      free(zz->killfile);
    } else {
      debug0(2, 0, 225);
    }
    
    hz1		= zz;
    zz		= zz->next;
    free(hz1);

    if (hz != NULL)
      hz->next	= zz;
    else
      zombieroot= zz;
  }
}

static void check_unproto_requests(void)
{
  unprotoportstype	*hp;

  hp	= unprotodefroot->ports;
  if (hp == NULL)
    return;

  if (unproto_final_update > 0) {
    if (clock_.ixtime - unproto_final_update > unprotodefroot->TxInterval) {
      send_final_unproto();		/* append a "<n> !!"  - message */
      unproto_final_update	= 0;	/* redundant, just for security */
    }
  }
  
  while (hp != NULL) {
    if (hp->RequestActive)
      send_requested_unproto(hp);
    hp	= hp->next;
  }
}

void print_dpboxusage(short unr, dpuaproc outputproc)
{
  short		scaley = 10;
  short		usage[59];
  short		users[59];
  short		maxusage;
  short		maxusers;
  short		x, y, ch, uh, uhl;
  double	interval, step, faktor_cpu, faktor_users, cmul, umul;
  char		s[256];
  
  if (dpboxcpufilled == false) return;
  
  interval	= DPBOXCPUARRSIZE / 60;
  step		= 0.0;
  maxusage	= 0;
  maxusers	= 0;
  
  for (x = 0; x < 59; x++) {
    y		= step;
    usage[x]	= calc_prozent((dpboxcpu[y + 1] - dpboxcpu[y]) / TICKSPERSEC,
    				interval*DPBOXCPUINTERVAL);
    users[x]	= dpboxuserct[y + 1];
    if (usage[x] > maxusage) maxusage = usage[x];
    if (users[x] > maxusers) maxusers = users[x];
    step	+= interval;
  }
  
  if (maxusage > 100) maxusage = 100;
  faktor_cpu	= (double)maxusage / 100.0;
  faktor_users	= (double)maxusers / (double)MAXUSER;
  cmul		= scaley*faktor_cpu;
  umul		= scaley*(MAXUSER/100)*faktor_users;
  if (cmul < 1.0) cmul = 1.0;
  
  for (x = scaley; x >= 0; x--) {

    ch		= (double)x*cmul + 0.5;
    uh		= (double)x*umul + 0.5;
    uhl		= (double)(x-1)*umul + 0.5;

    if (x == scaley)
      strcpy(s, "%CPU ^");
    else
      sprintf(s, "  %.2d +", ch);

    for (y = 0; y < 59; y++) {
      if (users[y] > uhl && users[y] <= uh)
        strcat(s, ".");
      else {
        if (usage[y] >= ch)
          strcat(s, "*");
        else if (users[y] > uhl)
          strcat(s, ".");
        else
          strcat(s, " ");
      }
    }

    if (x == scaley)
      strcat(s, "^ User");
    else {
      if (uh != uhl)
        sprintf(s + strlen(s), "+ %d", uh);
      else
        strcat(s, "|");
    }
    outputproc(unr, s);

  }
  
  strcpy(s, "     +");
  for (x = 1; x < 60; x++) {
    if (x % 4 == 0)
      strcat(s, "+");
    else
      strcat(s, "-");
  }

  strcat(s, "+");
  outputproc(unr, s);

  strcpy(s, "     15m");
  for (x = 0; x < 56; x++)
    strcat(s, " ");
  strcat(s, "0m");
  outputproc(unr, s);

  outputproc(unr, "                        * = dpbox cpu, . = user");
}


void get_dpboxusage(short *u1, short *u5, short *u15)
{
  long		m1, m5, m15, last;

  if (dpboxcpufilled == false) {
    *u1		= 100;
    *u5		= 100;
    *u15	= 100;
    return;
  }
  
  last		= dpboxcpu[DPBOXCPUARRSIZE - 1];
  m15		= dpboxcpu[0];
  *u15		= calc_prozent((last - m15) / TICKSPERSEC, 15*60);
  m5		= dpboxcpu[DPBOXCPUARRSIZE - (60 / DPBOXCPUINTERVAL * 5) - 1];
  *u5		= calc_prozent((last - m5) / TICKSPERSEC, 5*60);
  m1		= dpboxcpu[DPBOXCPUARRSIZE - (60 / DPBOXCPUINTERVAL) - 1];
  *u1		= calc_prozent((last - m1) / TICKSPERSEC, 1*60);
}

void block_mailbox(boolean block, char *reason)
{
  if (block) {
    append_profile(-1, "DPBOX blocked");
    append_profile(-1, reason);
    gesperrt = true;
    sf_allowed = false;
    disc_user(-1, "USER maintenance works, sri. try later.");
    disc_user(-1, "SF maintenance works, sri. try later.");
    kill_all_routing_flags();
  } else {
    append_profile(-1, "DPBOX unblocked");
    gesperrt = false;
    sf_allowed = true;
  }
}

static boolean test_correct_clock(void)
{
  static boolean blocked = false;
  
  if (clock_.year4 >= 2000) {
    if (blocked) { /* if date was wrong before... */
      wrong_clock = false;
      blocked 	  = false;
      block_mailbox(false, "");
    }
    return true;
  } else {
    if (!blocked) { /* if date was good before... */
      wrong_clock = true;
      blocked 	  = true;
      block_mailbox(true, "invalid system time");
    }
    return false;
  }
}

/* tct is a counter with TICKSPERSEC Hz (at least 200)	*/

void box_timing2(long tct)
{
  static short		lastbatchhour	= -1;
  static long	 	ltc1		= 0;
  static long		ltc2		= 0;
  static long		ltc3		= 0;
  static long		ltc4		= 0;
  static long		ltc5		= 0;
  static long		ltc6		= 0;
  static boolean	startup		= true;

  short			unr, x;
  pid_t			ret;
  int			status;
  time_t      	      	tt;
  char			w[256];

  clock_.ticks = tct;

  if (tct - ltc6 > DPBOXCPUINTERVAL*TICKSPERSEC || tct - ltc6 < 0) {  /* alle 15 sec */
    ltc6	= tct;
    if (dpboxcpufilled == false) {
      dpboxcpu[0]			= get_cpuusage();
      for (x = 1; x < DPBOXCPUARRSIZE; x++) {
        dpboxcpu[x]			= dpboxcpu[0];
        dpboxuserct[x - 1]		= 0;
      }
      dpboxuserct[DPBOXCPUARRSIZE - 1]	= actual_connects();
      dpboxcpufilled			= true;
    } else {
      for (x = 1; x < DPBOXCPUARRSIZE; x++) {
        dpboxcpu[x - 1]			= dpboxcpu[x];
        dpboxuserct[x - 1]		= dpboxuserct[x];
      }
      dpboxcpu[DPBOXCPUARRSIZE - 1]	= get_cpuusage();
      dpboxuserct[DPBOXCPUARRSIZE - 1]	= actual_connects();
    }

#ifdef DPDEBUG
#ifdef __GLIBC__
  debug0(5, -1, 215);
#endif
#endif

  }

  if (ltc1 == 0) {
    ltc1	= tct;
    ltc2	= tct;
    ltc3	= tct;
    ltc4	= tct;
    ltc5	= tct;
    return;
  }

  if ((tct - ltc5 > 41*60*TICKSPERSEC || tct - ltc5 < 0) && !wrong_clock) {  /* alle 41 Minuten */
    fill_msgnumarr();
    ltc5	= statclock();
  }

  if ((tct - ltc1 > 10*60*TICKSPERSEC || tct - ltc1 < 0) && !wrong_clock) {  /* alle 10 Minuten */
    recompile_log(-1);
    recompile_fwd();

    if (clock_.ixtime - lastwprotcreate >= wprotcreateinterval)
      generate_wprot_files();

    if (clock_.ixtime - lastwpcreate >= wpcreateinterval)
      generate_wp_files();

    sprintf(w, "%slostfile%cbox", boxprotodir, extsep);
    chkopenfiles(12*60*60, w);   /* nach 12 Stunden schliessen */

    ltc1	= statclock();

    check_disk_full();
    boxisbusy(false);
  }


  if ((tct - ltc2 > 61*TICKSPERSEC || tct - ltc2 < 0) && !wrong_clock) {  /* alle 61 Sekunden */
    
    if (startup) {
      startup	= false;
      sprintf(w, "%sstartup%cbat", boxsysdir, extsep);
      run_sysbatch(w);
    }
    
    calc_routing_table();
    create_own_routing_broadcasts();
    get_linpack(w);

    if (!disk_full) {
      sprintf(w, "%simport%c%c", newmaildir, extsep, allquant);
      sort_new_mail(-1, w, Console_call);
      sprintf(w, "%s%c%c%c", impmaildir, allquant, extsep, allquant);
      sort_new_mail(-1, w, "-moni-");
    }

    check_routing_timeouts();

    if (sf_allowed)
      check_sftimer();

    if (mail_beacon && balisetime > 0)
      balise_check();

    if (clock_.hour != lastbatchhour) {
      if (lastbatchhour == -1)
	lastbatchhour	= clock_.hour;
      else {
	lastbatchhour	= clock_.hour;
	if (lastbatchhour == 0) {
	  badtimecount	= 0;
	  readjust_dayly_sfcount();
	}
	sprintf(w, "%shour_%d%cbat", boxsysdir, lastbatchhour, extsep);
	run_sysbatch(w);
	sprintf(w, "%shour%cbat", boxsysdir, extsep);
	run_sysbatch(w);
      }
    }

    if (tell_waiting)
      tell_check();

    tt	= clock_.ixtime - get_lastgarbagetime();
    if ( (auto_garbage && (clock_.hour == garbagetime) && (tt > SECPERDAY / 4))
         || ((tt > SECPERDAY * 5) && (get_boxruntime_l() > 60*15) && (get_totalruntime_l() > SECPERDAY)) ) {
      if (!boxgbstop()) {
      	garbage_collection(false, false, false, false, -1);
      }
    }

    flush_bidseek_immediately();
    ltc2	= statclock();

  }

  if (immediate_extcheck || tct - ltc4 > 1*TICKSPERSEC || tct - ltc4 < 0) {	/* alle 1 Sekunde */
    dp_watchdog(2, 4711);

    immediate_extcheck	= false;
    ltc4		= tct;

    utc_clock();
    test_correct_clock();
    
    if (send_bbsbcast && !wrong_clock) {
      if (get_boxruntime_l() > 60)
        boxbcasttxtimer();
    }
    
    if (unproto_list && unprotodefroot != NULL && !wrong_clock)
      check_unproto_requests();

    for (unr = 1; unr <= MAXUSER; unr++) {
      if (boxrange(unr)) {

	if (user[unr]->trace_to > 0) {
	  if (*user[unr]->tracefract != '\0') {
	    *w		= '\0';
	    trace_string(false, unr, user[unr]->trace_to, w, true);
	  }
	}

      	if (user[unr]->action != 15 && user[unr]->umode == UM_FILESF
	    && user[unr]->sf_to == false && user[unr]->spath[0] != '\0') {
	  do_filesf_input(unr);
      	  if (!boxrange(unr)) continue;
	}
	
	if (user[unr]->newmsg)
	  retarded_msg(unr);

	if (user[unr]->action == 96) {
	  if (user[unr]->yapp != NULL) {
	    if (boxspoolstatus(user[unr]->tcon, user[unr]->pchan, -1) <= MAXUSERSPOOL) {
	      if (!read_filepart(unr))
		user[unr]->action	= 0;
	    } else if (clock_.ixtime - user[unr]->yapp->touch > 10)
	      sfclose(&user[unr]->yapp->filefd);
	  } else {
	    if (user[unr]->bin == NULL)
	      user[unr]->action		= 0;
	    else if (boxspoolstatus(user[unr]->tcon, user[unr]->pchan, -1) <= MAXUSERSPOOL) {
	      if (!read_filepart(unr))
		user[unr]->action	= 0;
	    } else if (clock_.ixtime - user[unr]->bin->touch > 10)
	      sfclose(&user[unr]->bin->filefd);
	  }
	}

	if (user[unr]->wait_pid > 0) {
	  ret = waitpid(user[unr]->wait_pid, &status, DP_WNOHANG);
	  if (ret == -1 || ret == user[unr]->wait_pid) {
	    user[unr]->cputime	= get_cpuusage();
	    if (ret == user[unr]->wait_pid) {
	      debug(2, unr, 219, "closing from box_timing2()");
	      close_shell(unr);
	    }

	    user[unr]->wait_pid	= -1;
	    boxsetrwmode(user[unr]->pchan, RW_NORMAL);

	    switch (user[unr]->action) {

	    case 71:
	      user[unr]->action	= 0;
	      show_sortcheck(unr);
	      show_prompt(unr);
	      break;

	    case 99:
	      user[unr]->action	= 0;
	      show_ext_command_result(unr);
	      if (user[unr]->action == 0)
	        show_prompt(unr);
	      break;

	    case 500:
	      user[unr]->action	= 0;
	      show_fullcheck(unr);
	      show_prompt(unr);
	      break;

	    }
	    upd_statistik(unr, 0, 0, user[unr]->cputime, get_cpuusage());
	    user[unr]->cputime	= 0;

	  }
	}

	if ((unsigned)user[unr]->action < 32 && ((1L << user[unr]->action) & 0xc000L) != 0)
	  spoolendcheck(unr);

      }

    }
  }

  if ((tct - ltc3 > 31*TICKSPERSEC || tct - ltc3 < 0) && !wrong_clock) {  /* alle 31 Sekunden */

    for (unr = 1; unr <= MAXUSER; unr++) {
      if (boxrange(unr)) {
	if (user[unr]->f_bbs && (user[unr]->action >= 200 && user[unr]->action <= 205))
	  look_for_mail(unr, true, false);
	else if (user[unr]->f_bbs && (user[unr]->action >= 300 && user[unr]->action <= 305))
	  send_fbb_proposals(unr, true);
	else
	  timeout_check(unr);
      }
    }

    kill_zombies();
    immediate_zombiecheck = false;
    
    chk_xgar();
    ltc3	= statclock();
  }

  if (immediate_zombiecheck) {
    kill_zombies();
    immediate_zombiecheck = false;
  }

  box_get_next_input();
  if (newmailroot != NULL)
    sort_new_mail4();
  if (balisenumber > 0)
    show_mailbeacon();
  if (new_mybbs_data)
    do_emt(&mybbsseekp);
  if (new_remote_erases)
    check_remote_erase(&remerseekp);
  if (crawl_started != 0)
    process_crawl();
}


void _box_tim_init(void)
{
  static int _was_initialized = 0;
  if (_was_initialized++)
    return;

  baliseh1		= nohandle;
  balisenumber		= 0;
  lastxgartime		= 0;
  immediate_zombiecheck	= false;
  dpboxcpufilled	= false;
  crawl_started		= 0;
  crawl_pid		= 0;
}
