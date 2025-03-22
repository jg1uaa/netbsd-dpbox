/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/

/* 09.02.97 DL3NEU: added some features and updated Source  */
/* 06.01.2000 DL8HBS: added ping server       	      	    */

#define BOX_SERV_G
#include "box_serv.h"
#include "sort.h"
#include "tools.h"
#include "boxglobl.h"
#include "box_send.h"
#include "box_logs.h"
#include "yapp.h"
#include "boxlocal.h"
#include "box_sub.h"
#include "box_inou.h"
#include "box_file.h"
#include "box_mem.h"
#include "box_rout.h"
#include "box_scan.h"


static boolean valid_servername(char *n_)
{
  short i;
  calltype n;
  
  i = strlen(n_);
  if (i < 1 || i > LEN_CALL)
    return false;
  strcpy(n,n_);
  check_verteiler(n);
  if (callsign(n) || !defined_board(n))
    return true;
  return false;
}


static void strip_call(char *c1_, char *c2, char *c)
{
  char	*p;
  char	c1[256];

  strcpy(c1, c1_);
  if (c1[0] == '!') {
    *c = '!';
    strdelete(c1, 1, 1);
  } else if (c1[0] == '#') {
    *c = '#';
    strdelete(c1, 1, 1);
  } else
    *c = '\0';
  cut(c1, LEN_CALL);
  p = strchr(c1, '@');
  if (p != NULL) *p = '\0';
  del_lastblanks(c1);
  strcpy(c2, c1);
}


boolean in_servers(char *board)
{
  char s[256];

  if (!strcmp(board, "LSTSRV"))
    return true;
  sprintf(s, "%s%s%cSRV", serverdir, board, extsep);
  return (exist(s));
}


static short in_abonnees(char *sender, char *board)
{
  short ifl, unr;
  boolean ok, sflag;
  char c;
  char hs[256], c1[256];
  char STR1[256];

  ok = false;
  sflag = false;

  if (in_servers(board)) {
    sprintf(STR1, "%s%s%cSRV", serverdir, board, extsep);
    ifl = sfopen(STR1, FO_READ);
    while (file2str(ifl, hs) && !ok) {
      strip_call(hs, c1, &c);
      sflag = (c == '!');
      ok = ((strcmp(c1, sender) == 0) && (c != '#'));
    }
    sfclose(&ifl);

    /* DL5HBF: An Console_call darf JEDER schreiben */
    if (!ok && !strcmp(board, Console_call))
      ok = true;
  
    /* DL8HBS 20000424: Local connected and authenticated Sysops may send to every server  */
    if (!ok && (unr = actual_user(sender)) > 0) {
      if (user[unr]->supervisor && user[unr]->se_ok && user[unr]->pwmode >= 10) {
      	ok  = true;
      }
    }
  
    /* DL5HBF: Bloss der Absender darf nicht der Servername sein, */
    /*         sonst gibt's 'ne huebsche Schleife :-))            */
    if (!strcmp(board, sender))
      ok = false;
  }

  if (!ok)
    return 0;
  if (sflag)
    return 2;
  return 1;
}

static boolean is_public(char *board)
{
  short ifl;
  boolean ok;
  char c;
  char hs[256], c1[256];
  char STR1[256];

  ok = false;

  if (!strcmp(board, "LSTSRV")) return true;
  if (in_servers(board)) {
    sprintf(STR1, "%s%s%cSRV", serverdir, board, extsep);
    ifl = sfopen(STR1, FO_READ);
    while (file2str(ifl, hs) && !ok) {
      strip_call(hs, c1, &c);
      ok = ((strcmp(c1, "PUBLIC") == 0) && (c == '#'));
    }
    sfclose(&ifl);
  }

  return ok;
}

static boolean u_subscribe(char *sender_, char *board_, char *betreff_);

#define blocksize       16384

boolean do_server(char *sender_, char *board_, char *betreff_, char *tname_,
		  long offset)
{
  boolean Result;
  short ifl, ifs, ifc, k, x, y, kanal, pmh, ct;
  char *tb;
  long tbs, rp, lrp, lrp2;
  boolean ok;
  long cntmsg, cntabonnees;
  char c;
  char svname[256];
  char sublist[256];
  char svcnt[256];
  char svsysop[256];
  char hs[256], name[256];
  char tobbs[256];
  char w[256], nname[256];
  char betreff1[256];
  char STR7[256];
  char protomsg[256];
  char sender[256];
  char board[256], betreff[256], tname[256], replyto[256];

  strcpy(sender, sender_);
  strcpy(board, board_);
  strcpy(betreff, betreff_);
  strcpy(tname, tname_);
  Result = false;
  if (exist(tname) && callsign(sender)) {
    if (!u_subscribe(sender, board, betreff)) {
      if (in_abonnees(sender, board) > 0) {
        
        mytmpnam(protomsg);
        mytmpnam(nname);
        mytmpnam(sublist);
	sprintf(svname, "%s%s%cSRV", serverdir, board, extsep);
	sprintf(svcnt, "%s%s%cCNT", serverdir, board, extsep);
	*svsysop = '\0';

	if ((kanal = sfopen(tname, FO_READ)) >= minhandle) {
	  sfseek(offset, kanal, SFSEEKSET);
	  tb = malloc(blocksize);
	  if (tb != NULL) {
	    tbs = sfread(kanal, blocksize, tb);
	    sfclose(&kanal);
	    rp = 0;
	    lrp = 0;
	    lrp2 = 0;
	    ok = true;
	    while (rp + 10 < tbs && ok) {
	      get_line(tb, &rp, tbs, hs);
	      if (strstr(hs, "R:") != hs)
		ok = false;
	      else {
		lrp2 = lrp;
		lrp = rp;
	      }
	    }
	    offset += lrp2;
	  } else
	    sfclose(&kanal);
	}

	filecut_nodel(tname, nname, offset, 0);

	if (exist(svcnt)) {
	  ifc = sfopen(svcnt, FO_READ);
	  file2str(ifc, w);
	  sfclose(&ifc);
	  cntmsg = atol(w);
	} else
	  cntmsg = 0;
	cntmsg++;
	ifc = sfcreate(svcnt, FC_FILE);
	sprintf(w, "%ld", cntmsg);
	str2file(&ifc, w, true);
	sfclose(&ifc);

	ifl = sfopen(svname, FO_READ);
	ifs = sfcreate(sublist, FC_FILE);
	*hs = '\0';
	cntabonnees = 0;
	while (file2str(ifl, w)) {
	  strip_call(w, name, &c);
	  if (c != '#') {
            if (c == '!') {
              if (*svsysop == '\0')
                strcpy(svsysop, name);
              else if (strlen(svsysop) < 70)
		sprintf(svsysop + strlen(svsysop), " %s", name);
	    }
	    if (*hs == '\0')
              strcpy(hs, name);
	    else if (strlen(hs) < 70)
	      sprintf(hs + strlen(hs), " %s", name);
	    else {
	      str2file(&ifs, hs, true);
	      strcpy(hs, name);
	    }
            cntabonnees++;
          }
	}
	if (*hs != '\0')
	  str2file(&ifs, hs, true);
	sfclose(&ifl);
	sfclose(&ifs);

	sprintf(STR7, "(%s ", board);
	strcpy(betreff1, betreff);
	x = strpos2(betreff1, STR7, 1);
	if (x > 0) {
	  y = strpos2(betreff1, ") ", 1);
	  if (y > x)
	    strdelete(betreff1, x, y - x + 2);
	}
	sprintf(betreff1, "(%s %ld) %s", board, cntmsg, strcpy(STR7, betreff1));


	if ((pmh = sfcreate(protomsg, FC_FILE)) >= minhandle) {

	  sprintf(STR7, "Mail processed by dpbox mailing list server %s@%s",
			 board, ownhiername);
	  str2file(&pmh, STR7, true);
	  str2file(&pmh, "", true);
	  if (cntabonnees <= maxsubscriberdisp) {
            str2file(&pmh, "Subscriber list:", true);
            app_file(sublist, pmh, true);
	  } else {
            sprintf(STR7, "There are %ld users on the list", cntabonnees);
	    str2file(&pmh, STR7, true);
          }
          str2file(&pmh, "", true);
	  sprintf(STR7, "For server redistribution reply to %s@%s", board, ownhiername);
	  str2file(&pmh, STR7, true);
	  if (*svsysop != '\0') {
	    sprintf(STR7, "Server is maintained by %s", svsysop);
	    str2file(&pmh, STR7, true);
	  }
	  str2file(&pmh, "", true);
	  str2file(&pmh,
	    "-----------------------------------------------------------------------------",
	    true);
	  app_file(nname, pmh, true);
	  sfclose(&pmh);
	}

	Result = true;
	*replyto = '\0';
	user_mybbs(sender, hs);
	if (*hs != '\0') {
	  complete_hierarchical_adress(hs);
	  if (strcmp(hs, ownhiername))
	    sprintf(replyto, "Reply-To: %s @ %s", sender, hs);
	}
	  
	if ((ifl = sfopen(svname, FO_READ)) >= minhandle) {
	  ct = 0;
	  while (file2str(ifl, hs)) {

	    if (ct % 10 == 0)
	      dp_watchdog(2, 4711);   /* Watchdog resetten         */

	    if (*hs == '\0')
	      continue;
	    strip_call(hs, name, &c);
	    if (c == '#')
              continue;
            if (useq(name, board))
              continue;
            k = strpos2(hs, "@", 1);
            if (k > 0)
	      strdelete(hs, 1, k);
	    else
	      *hs = '\0';
	    del_leadblanks(hs);
	    strcpy(tobbs, hs);
	    
	    if (*tobbs == '\0') {
	      user_mybbs(name, hs);
	      if (*hs != '\0')
	        strcpy(tobbs, hs);
	    }
	    if (*tobbs == '\0')
	      strcpy(tobbs, Console_call);
	    complete_hierarchical_adress(tobbs);

	    sprintf(w, "%simport%cXXXXXX", newmaildir, extsep);
	    mymktemp(w);
	    if ((k = sfcreate(w, FC_FILE)) >= minhandle) {

	      str2file(&k, Console_call, true); /* frombox */
	      str2file(&k, sender, true); /* absender */
	      str2file(&k, name, true);   /* an */
	      str2file(&k, tobbs, true);  /* mbx */
	      *hs = '\0';
	      rspacing(hs, 80);
	      str2file(&k, hs, true);      /* lt */
	      str2file(&k, hs, true);      /* BID */
	      str2file(&k, betreff1, true);/* subject */
	      sprintf(hs, "From: %s @ %s", sender, ownhiername);
	      str2file(&k, hs, true);
	      if (*tobbs != '\0')
	        sprintf(hs, "To:   %s @ %s", name, tobbs);
	      else
	        sprintf(hs, "To:   %s", name);
	      str2file(&k, hs, true);
              if (*replyto != '\0')
                str2file(&k, replyto, true);
	      str2file(&k, " ", true);
	      app_file(protomsg, k, false);
	      sfclose(&k);

 	    }
	  }

	  sfclose(&ifl);
	  sprintf(STR7, "%simport%c%c", newmaildir, extsep, allquant);
	  sort_new_mail(-1, STR7, Console_call);

	}

	sfdelfile(nname);
	sfdelfile(sublist);
	sfdelfile(protomsg);
      }

    } else
      Result = true;
  }
  return Result;
}

#undef blocksize


static void list_abonnees(short unr, char *sname, char *call_)
{
  char call[256];
  short ifl;
  char svname[256];
  char hs[256], w[256], w0[256];
  char STR7[256];

  strcpy(call, call_);
  if (in_servers(sname)) {
    sprintf(svname, "%s%s%cSRV", serverdir, sname, extsep);
    w_btext(unr, 115);
    sprintf(STR7, " %s:", sname);
    wlnuser(unr, STR7);
    ifl = sfopen(svname, FO_READ);
    *w0 = '\0';
    if (*call != '\0' && call[strlen(call) - 1] != '*')
      strcat(call, "*");
    while (file2str(ifl, hs)) {
      if (hs[0] == '#')
        continue;
      upper(hs);
      if (*call == '\0') {
        wlnuser(unr, hs);
        continue;
      }
      strcpy(w, hs);
      if (w[0] == '!')
        strdelete(w, 1, 1);
      if (wildcardcompare(SHORT_MAX, call, w, w0))
        wlnuser(unr, hs);
    }
    sfclose(&ifl);
  } else
    wln_btext(unr, 45);
}


static boolean add_abonnee(short unr, char *sname, char *call, boolean public)
{
  boolean Result;
  short ifl, ofl;
  char c;
  char svname[256];
  char ovname[256];
  char hs[256], c1[256], c2[256];
  char STR1[256], STR7[256];

  Result = false;
  sprintf(svname, "%s%s%cSRV", serverdir, sname, extsep);
  strip_call(call, c1, &c);
  if (strcmp(sname, "LSTSRV") && (callsign(c1) || public)) {
    if (!in_servers(sname)) {
      w_btext(unr, 116);
      sprintf(STR1, " %s", sname);
      wlnuser(unr, STR1);
      w_btext(unr, 117);
      sprintf(STR7, " %s ", c1);
      wlnuser(unr, STR7);
      w_btext(unr, 118);
      chwuser(unr, 32);
      wlnuser(unr, sname);
      ofl = sfcreate(svname, FC_FILE);
      if (ofl >= minhandle) {
	str2file(&ofl, call, true);
	sfclose(&ofl);
        Result = true;
      } else
	wln_btext(unr, 119);
    } else {
      sprintf(ovname, "%s%s%cSRX", serverdir, sname, extsep);
      w_btext(unr, 117);
      sprintf(STR1, " %s ", c1);
      wlnuser(unr, STR1);
      w_btext(unr, 118);
      chwuser(unr, 32);
      wlnuser(unr, sname);
      ifl = sfopen(svname, FO_READ);
      ofl = sfcreate(ovname, FC_FILE);
      while (file2str(ifl, hs)) {
	strip_call(hs, c2, &c);
	if (strcmp(c1, c2))
	  str2file(&ofl, hs, true);
      }
      str2file(&ofl, call, true);
      sfclose(&ifl);
      sfclose(&ofl);
      sfdelfile(svname);
      sfrename(ovname, svname);
      sort_file(svname, true);
      Result = true;
    }
  }
  return Result;
}


static boolean sub_abonnee(short unr, char *sname, char *call, boolean public)
{
  boolean Result;
  short ifl, ofl, ct;
  char c;
  char svname[256];
  char ovname[256];
  char hs[256];
  char c1[256], c2[256];
  char STR7[256];

  Result = false;
  if (in_servers(sname)) {
    sprintf(svname, "%s%s%cSRV", serverdir, sname, extsep);
    sprintf(ovname, "%s%s%cSRX", serverdir, sname, extsep);
    strip_call(call, c1, &c);
    if (callsign(c1) || public) {
      w_btext(unr, 120);
      sprintf(STR7, " %s ", c1);
      wlnuser(unr, STR7);
      w_btext(unr, 118);
      chwuser(unr, 32);
      wlnuser(unr, sname);
      ct = 0;
      ifl = sfopen(svname, FO_READ);
      ofl = sfcreate(ovname, FC_FILE);
      while (file2str(ifl, hs)) {
	strip_call(hs, c2, &c);
	if (strcmp(c1, c2)) {
	  str2file(&ofl, hs, true);
	  ct++;
	} else
          wlnuser(unr, "OK");
      }
      sfclose(&ifl);
      sfclose(&ofl);
      sfdelfile(svname);
      if (ct == 0) {
	sfdelfile(ovname);
	new_ext(ovname, "CNT");
	sfdelfile(ovname);
        w_btext(unr, 121);
        chwuser(unr, 32);
        wlnuser(unr, sname);
      }
      sfrename(ovname, svname);
      Result = true;
    }
  } else {
    wln_btext(unr, 45);
  }
  return Result;
}

/* Bestaetigungsmail generieren */
static void gen_svackmail(char *sender_,char *server_, short Subscribe)
{

  short result;
  short k, ifl, l;
  char hs[256], infoname[256];
  char STR7[256];
  char sender[256], server[256];
  char svname[256], ivname[256];
  char svsysop[256];
  char w[256], w1[256], c, c1[256];
  char infotxt[256]; 
  unsigned short date, time;
  DTA dirinfo;

  strcpyupper(sender, sender_);
  strcpyupper(server, server_);
  
  sprintf(hs, "%simport%cXXXXXX", newmaildir, extsep);
  mymktemp(hs);
  k = sfcreate(hs, FC_FILE);
  if (k >= minhandle) {
    str2file(&k, Console_call, true); /* frombox */
    str2file(&k, Console_call, true); /* absender */
    str2file(&k, sender, true);       /* an */
    *hs = '\0';
    rspacing(hs, 80);
    str2file(&k, hs, true);    /* mbx */
    str2file(&k, hs, true);    /* lt */
    str2file(&k, hs, true);    /* BID */

    if (Subscribe == 0)
      sprintf(hs, "LSTSRV: Error occured at your request to %s", server);
    else if (Subscribe == 1)
      sprintf(hs, "LSTSRV: Subscribed to list server %s", server);
    else if (Subscribe == 2)
      sprintf(hs, "LSTSRV: Unsubscribed from list server %s", server);
    else if (Subscribe == 3)
      sprintf(hs, "LSTSRV: Subscriber list of server %s", server);
    else if (Subscribe == 4)
      sprintf(hs, "LSTSRV: Public mailing list servers at %s", Console_call);

    str2file(&k, hs, true);   /* subject */
    sprintf(hs, "From: %s @ %s", Console_call, ownhiername);
    str2file(&k, hs, true);
    sprintf(hs, "To:   %s", sender);
    str2file(&k, hs, true);
    str2file(&k, " ", true);

    strcpy(STR7, "Mail generated by dpbox mailing list server:");
    str2file(&k, STR7, true);
    str2file(&k, " ", true);

    if (Subscribe == 0) {
      str2file(&k, "Executing your command an error occured", true);
      str2file(&k, "Try again or write a mail to the sysop", true);
    } else if (Subscribe == 1) {
      sprintf(STR7, "You are now subscribed to list server %s.", server);    
      str2file(&k, STR7, true);
      sprintf(STR7, 
              "Send a mail to LSTSRV@%s with subject  UNSUBSCRIBE %s ",
              ownhiername, server);
      str2file(&k, STR7, true);
      str2file(&k, "if you want to unsubscribe from this server.", true);
/* wenn vorhanden, Infomail des Sysops ausgeben... */
      sprintf(infoname, "%s%s%cINF", serverdir, server, extsep); 
      if (exist(infoname)) {
        str2file(&k, " ", true);
        str2file(&k, "Server info: ", true);
        strcpy(STR7, 
    "----------------------------------------------------------------------");
        str2file(&k, STR7, true);
        app_file(infoname, k, false); 
        str2file(&k, STR7, true);
      }


    } else if (Subscribe == 2) {
      sprintf(STR7, "You are unsubscribed from list server %s.",
               server); 
      str2file(&k, STR7, true);
    

    } else if (Subscribe == 3) {
      sprintf(svname, "%s%s%cSRV", serverdir, server, extsep);
      sprintf(STR7, "Subscriber list of server %s:", server);
      str2file(&k, STR7, true);
      ifl = sfopen(svname, FO_READ);
      *hs = '\0';
      *svsysop = '\0';
      while (file2str(ifl, w)) {
        strip_call(w, c1, &c);
	if (c != '#') {
          if (c == '!') {
            if (*svsysop == '\0')
              strcpy(svsysop, c1);
            else if (strlen(svsysop) < 70)
              sprintf(svsysop + strlen(svsysop), " %s", c1);
          }
	  if (*hs == '\0')
            strcpy(hs, c1);
	  else if (strlen(hs) < 70)
            sprintf(hs + strlen(hs), " %s", c1);
          else {
            str2file(&k, hs, true);
            strcpy(hs, c1);
	  }
        }
      }
      if (*hs != '\0')
      str2file(&k, hs, true);
      if (*svsysop != '\0') {
        str2file(&k, " ", true);
        sprintf(STR7, "Server is maintained by %s", svsysop);
        str2file(&k, STR7, true);
      }
      sfclose(&ifl);

    } else if (Subscribe == 4) {
      str2file(&k, "Installed public listservers:", true);
      str2file(&k, " ", true);
      str2file(&k, "Name   Count LastAccess        Infotext", true);
      str2file(&k,
      "-----------------------------------------------------------", true);

      sprintf(svname, "%s%c%cSRV", serverdir, allquant, extsep);
      result = sffirst(svname, 0, &dirinfo);   /* alle normalen Files suchen */
      while (result == 0) {
        strcpy(w, dirinfo.d_fname);
        del_ext(w);
        if (!is_public(w)) {
          result = sfnext(&dirinfo);
          continue;
        }
        sprintf(ivname, "%s%s%cSRV", serverdir, w, extsep);
        sprintf(STR7, "%s%s%cCNT", serverdir, w, extsep);
        l = sfopen(STR7, FO_READ);
        rspacing(w, 8);
        str2file(&k, w, false);
        strcpy(w, "0");
        strcpy(w1, "-");
        if (l >= minhandle) {
          file2str(l, w);
          sfclose(&l);
          sfgetdatime(STR7, &date, &time);
          dosdt2string(date, time, w1);
        }
        lspacing(w, 4);
        str2file(&k, w, false);
        str2file(&k, " ", false);
        str2file(&k, w1, false);
        str2file(&k, " ", false);
        ifl = sfopen(ivname, FO_READ);
        while (file2str(ifl, hs)) {
          strip_call(hs, c1, &c);
          if (!strcmp("PUBLIC", c1) && (c == '#')) {
            strdelete(hs, 1, 8);
            strcpy(infotxt, hs);
          }
        }
        sfclose(&ifl);
        str2file(&k, infotxt, true);
        result = sfnext(&dirinfo);
      }
      str2file(&k, 
        "-----------------------------------------------------------", true);
    }
    str2file(&k, " ", true);
    sprintf(STR7, "73 de %s", Console_call);
    str2file(&k, STR7, true);
  }
  sfclose(&k);
}

/* DL3NEU: Subscribe bzw. Unsubscribe per Mail */
static boolean u_subscribe(char *sender_, char *board_, char *betreff_)
{
  boolean ok;
  char betreff1[256];
  char sender1[256];
  char board1[256];
  char STR1[256];
  short what;


  strcpyupper(betreff1, betreff_);
  get_word(betreff1, STR1);
  strcpyupper(board1, board_);

  if (!strcmp(STR1, "SUBSCRIBE"))
    what = 1;
  else if (!strcmp(STR1, "UNSUBSCRIBE"))
    what = 2;
  else if (!strcmp(STR1, "LISTABONNEES"))
    what = 3;
  else if (!strcmp(STR1, "SUBSCRIBERS"))
    what = 3;
  else if (!strcmp(STR1, "LISTSERV"))
    what = 4;
  else if (!strcmp(STR1, "LISTSERVER"))
    what = 4;
  else if (!strcmp(STR1, "LISTSERVERS"))
    what = 4;
  else if (!strcmp(board1, "LSTSRV")) {
    sprintf(STR1, "invalid command: %s - from: %s", betreff_, sender_);
    append_profile(-7, STR1);
    return true;
  } else
    return false;

  strcpy(sender1, sender_);
  
  if (!strcmp(board1, "LSTSRV") || !strcmp(board1, Console_call))
    get_word(betreff1, board1);

  if (!is_public(board1) && !(what == 3 || what == 4))
    return true;

  switch (what) {
  
  case 1:
    if (in_abonnees(sender1, board1) <= 0) {
      ok = (add_abonnee(-7, board1, sender1, false)); 
      if (ok) {
        gen_svackmail(sender1, board1, 1);
        sprintf(STR1, "%s subscribed at server %s", sender1, board1);
        append_profile(-7, STR1);
      } else {
        gen_svackmail(sender1, board1, 0);
        sprintf(STR1, "Subscribe of %s at server %s failed", sender1, board1);
        append_profile(-7, STR1);
      }
    }
    return true;
    break;

  case 2:
    if (in_abonnees(sender1, board1) > 0) {
      ok = sub_abonnee(-7, board1, sender1, false);
      if (ok) {
        gen_svackmail(sender1, board1, 2);
        sprintf(STR1, "%s unsubscribed from server %s", sender1, board1);
        append_profile(-7, STR1);
      } else {
        gen_svackmail(sender1, board1, 0);
        sprintf(STR1, "Unsubscribe of %s from server %s failed", sender1,
           	board1);
        append_profile(-7, STR1);
      }
    }
    return true;
    break;
    
  case 3:
    /* list abonnees of a pubserv */
    if (in_abonnees(sender1, board1) > 0)
      gen_svackmail(sender1, board1, 3);
    return true;
    break;
        
  case 4:
    /* List all public listservers */
    gen_svackmail(sender1, board1, 4);
    return true;
    break;

  default:
    return false;
    
  }

}


static void list_servers(short unr, boolean All, boolean Subscribed)
{
  DTA dirinfo;
  short result, k, ifl;
  char w[256], w1[256];
  char STR1[256];
  char svname[256], ivname[256];
  char c;
  char c1[256], hs[256], infotxt[256];
  unsigned short date, time;

  if (All && !Subscribed)
    wln_btext(unr, 122);
  else if (!All && !Subscribed)
    wlnuser(unr, "Installed public listservers:");
  else if (Subscribed)
    wlnuser(unr, "Listservers you are subscribed to:");

  wlnuser0(unr);
  if (!All) {
    wlnuser(unr, "Name   Count LastAccess        Infotext");
    wlnuser(unr, "-----------------------------------------------------------");
  } else {
    wlnuser(unr, "Name   Count LastAccess");
    wlnuser(unr, "------------------------------");
  }

  sprintf(svname, "%s%c%cSRV", serverdir, allquant, extsep);
  result = sffirst(svname, 0, &dirinfo);   /* alle normalen Files suchen */
  while (result == 0) {
    strcpy(w, dirinfo.d_fname);
    del_ext(w);
    if (!All && !is_public(w)) {
      result = sfnext(&dirinfo);
      continue;
    } else if ((Subscribed && !strcmp(w, Console_call))
               || (Subscribed && !in_abonnees(user[unr]->call, w))) {
      result = sfnext(&dirinfo);
      continue;
    }
    sprintf(ivname, "%s%s%cSRV", serverdir, w, extsep);
    sprintf(STR1, "%s%s%cCNT", serverdir, w, extsep);
    k = sfopen(STR1, FO_READ);
    rspacing(w, 8);
    wuser(unr, w);
    strcpy(w, "0");
    strcpy(w1, "-");
    if (k >= minhandle) {
      file2str(k, w);
      sfclose(&k);
      sfgetdatime(STR1, &date, &time);
      dosdt2string(date, time, w1);
    }
    lspacing(w, 4);
    wuser(unr, w);
    chwuser(unr, 32);
    if (!All) {
      wuser(unr, w1);
      chwuser(unr, 32);
      ifl = sfopen(ivname, FO_READ);
      while (file2str(ifl, hs)) {
        strip_call(hs, c1, &c);
        if (!strcmp("PUBLIC", c1) && (c == '#')) {
          strdelete(hs, 1, 8);
          strcpy(infotxt, hs);
        }
      }
      sfclose(&ifl);
      wlnuser(unr, infotxt);
    } else
      wlnuser(unr, w1); 
    result = sfnext(&dirinfo);
  }
  if (!All)
    wlnuser(unr, "-----------------------------------------------------------");
  else
    wlnuser(unr, "------------------------------");
}

static boolean lock_abonnee(short unr, char *sname, char *call)
{
  boolean Result;
  char call1[256];
  char STR1[256];
  Result = false;

  if ((in_abonnees(call, sname)) > 0)
    sub_abonnee(-7, sname, call, false);
  sprintf(call1, "#%s", call);
  if (add_abonnee(-7, sname, call1, false)) {
    sprintf(STR1, 
            "OK, User %s now may not subscribe himself any more to server %s",
            call, sname);
    wlnuser(unr, STR1);
    Result = true;
  }
  return Result;
}


/* set public flag and infotext of a listserver */
static boolean toggle_pubserv(short unr, char *sname, char mode,
                	      char *eingabe)
{
  boolean ok, Result;
  char svname[256], ausgabe[256];
  char hs[256], c1[256];
  char c;
  short ifl;
  Result = false;

  sprintf(svname, "%s%s%cSRV", serverdir, sname, extsep);

  ok = false;
  ifl = sfopen(svname, FO_READ);
  while (file2str(ifl, hs)) {
    strip_call(hs, c1, &c);
    if (!strcmp("PUBLIC", c1) && (c == '#'))
      ok = true;
  }
  sfclose(&ifl);

  switch (mode) {

  case 1:
    if (!ok) {
      if (add_abonnee(-7, sname, "#PUBLIC@", true))
        Result = true;
    }
    break;

  case 2:      
    if (ok) {
      if (sub_abonnee(-7, sname, "#PUBLIC", true))
        Result = true;
    }
    break;
    
  case 3:
    if (ok) {
      ifl = sfopen(svname, FO_READ);
      while (file2str(ifl, hs)) {
        strip_call(hs, c1, &c);
        if (!strcmp("PUBLIC", c1) && (c == '#')) {
          strdelete(hs, 1, 8);
          strcpy(eingabe, hs);
          Result = true;
          
        }
      }
      sfclose(&ifl);
    }
    break;

  case 4:
    if (ok) {
      if (sub_abonnee(-7, sname, "#PUBLIC", true)) {
        sprintf(ausgabe, "#PUBLIC@%s", eingabe);
        if (add_abonnee(-7, sname, ausgabe, true))
          Result = true;
      }
    }
    break;
  }
  return Result;
}


boolean config_server(short unr, char *eingabe_)
{
  boolean Result;
  char eingabe[256];
  char sname[256];
  char w[256], w1[256];
  char STR1[256];

  strcpy(eingabe, eingabe_);
  Result = false;
  if (!boxrange(unr))
    return Result;

  switch (eingabe[0]) {
  
  case '\0':
    list_servers(unr, user[unr]->supervisor, false);
    Result = true;
    break;

  case '?':
    list_servers(unr, true, true);
    Result = true;
    break;

  case '!':
    if (!user[unr]->supervisor) {
      list_servers(unr, false, false);
      Result = true;
    } else {
     get_word(eingabe, w);
     get_word(eingabe, sname);
     if (*sname == '\0' && w[1] != '\0')
       break;
     upper(sname);

     switch (w[1]) {

     case '+':
       if (toggle_pubserv(unr, sname, 1, '\0')) {
         sprintf(STR1, "OK, server %s is now public", sname);
         wlnuser(unr, STR1); 
       } else {
         sprintf(STR1, "Server %s is already public", sname);
         wlnuser(unr, STR1);
       }
       Result = true;
       break;

     case '-':
       if (toggle_pubserv(unr, sname, 2, '\0')) {
         sprintf (STR1, "OK, server %s is now private", sname);
         wlnuser(unr, STR1);      
       } else {
         sprintf(STR1, "Server %s is already private", sname);
         wlnuser(unr, STR1);
       }
       Result = true;
       break;
   
    case '=':
      if (eingabe[0] == '\0') {
        if (toggle_pubserv(unr, sname, 3, eingabe)) {
          sprintf(STR1, "Infotext of server %s:", sname);
          wlnuser(unr, STR1);
          wlnuser(unr, eingabe);
        } else
          wlnuser(unr, "Server is not public or infotext unset");
      } else {
        if (toggle_pubserv(unr, sname, 4, eingabe)) {
          sprintf(STR1, "OK, infotext of server %s set to", sname);
          wlnuser(unr, STR1);
          wlnuser(unr, eingabe);
        } else
          wlnuser(unr, "Server is not public");
      }
      Result = true;
      break;
   
    default:
      /* list only public servers */
      list_servers(unr, false, false);
      Result = true;
      break;
    }
    break;
  }

  default:
    get_word(eingabe, sname);
    upper(sname);
    if (!valid_servername(sname))
      return Result;
    if (!(user[unr]->supervisor || in_abonnees(user[unr]->call, sname) == 2
          || is_public(sname)))
      return Result;
    get_word(eingabe, w);
    get_word(eingabe, w1);
    upper(w1);
    if (strlen(w) != 1)
      return Result;
    Result = true;

    switch (w[0]) {

    case '+':
      add_abonnee(unr, sname, w1, false);
      break;

    case '-':
      sub_abonnee(unr, sname, w1, false);
      break;

    case '=':
      list_abonnees(unr, sname, w1);
      break;

    case '#':
      lock_abonnee(unr, sname, w1);
      break;

    break;

    default:
      Result = false;
      break;
    }
  }
  return Result;
}



/* ------------------------------------------------------------------------------------------ */

/* der REDIST - Server...                        */
/* SP LOCBBS, SP LOCAL, S REGION, S NATION       */
/* Wenn in der Titelzeile zu Beginn ein # steht, */
/* ist das angefuegte Wort als Rubrikangabe      */
/* zu interpretieren, ansonsten gehts an ALL     */
/* Der Absender bekommt eine Bestaetigung        */
/* Die Absender-R:-Line wird an die neue Mail    */
/* angehaengt                                    */

boolean do_redist(char *absender, char *dest, char *betreff_, char *tname, long offset)
{
  boolean Result;
  char betreff[256];

  short k, h, x;
  long seekp;
  char hs[256], newb[256], newmbx[256];
  char lastr[256], sendbbs[256];
  char STR7[256];

  strcpy(betreff, betreff_);
  Result = false;
  if (exist(tname)) {
    if (callsign(absender)) {
      *newmbx = '\0';
      if (!strcmp(dest, "LOCBBS"))
	strcpy(newmbx, Console_call);
      else if (!strcmp(dest, "LOCAL"))
	strcpy(newmbx, redlocal);
      else if (!strcmp(dest, "REGION"))
	strcpy(newmbx, redregion);
      else if (!strcmp(dest, "NATION"))
	strcpy(newmbx, rednation);

      if (*newmbx != '\0') {
	strcpy(newb, "ALL");
	if (*reddefboard != '\0')
	  strcpy(newb, reddefboard);
	if (betreff[0] == '#') {
	  get_word(betreff, hs);
	  strdelete(hs, 1, 1);
	  cut(hs, LEN_BOARD);
	  upper(hs);
	  if (valid_boardname(hs) && !callsign(hs))
	    strcpy(newb, hs);
	}

	if (*betreff == '\0')
	  strcpy(betreff, "REDIST MSG");

	sprintf(hs, "%simport%cXXXXXX", newmaildir, extsep);
	mymktemp(hs);
	k = sfcreate(hs, FC_FILE);
	if (k >= minhandle) {
	  str2file(&k, Console_call, true);   /* frombox */
	  str2file(&k, absender, true);   /* absender */
	  str2file(&k, newb, true);   /* an */
	  str2file(&k, newmbx, true);   /* mbx */
	  if (redlifetime >= 0)
	    sprintf(hs, "%d", redlifetime);
	  else
	    strcpy(hs, "30");
	  rspacing(hs, 30);
	  str2file(&k, hs, true);   /* lt */
	  strcpy(hs, "                   ");
	  str2file(&k, hs, true);   /* BID */
	  str2file(&k, betreff, true);   /* subject */

	  h = sfopen(tname, FO_READ);
	  if (h >= minhandle) {
	    Result = true;
	    seekp = sfseek(offset, h, SFSEEKCUR);
	    *lastr = '\0';
	    *sendbbs = '\0';
	    while (file2str(h, hs) && strstr(hs, "R:") == hs) {
	      strcpy(lastr, hs);
	      seekp = sfseek(0, h, SFSEEKCUR);
	    }

	    sfclose(&h);

	    x = strpos2(lastr, "]", 1);
	    if (x > 0)
	      cut(lastr, x + 1);
	    x = strpos2(lastr, "$:", 1);
		/* sollte eigentlich nicht vorkommen */
	    if (x > 0)
	      cut(lastr, x - 1);

	    x = strpos2(lastr, "@", 1);
	    if (x > 0) {
	      strsub(sendbbs, lastr, x + 1, 7);
	      if (sendbbs[0] == ':')
		strdelete(sendbbs, 1, 1);
	      unhpath(sendbbs, sendbbs);
	      if (!callsign(sendbbs))
		*sendbbs = '\0';
	    }

	    str2file(&k, lastr, true);

	    app_file2(tname, k, seekp, false);

	    sfclose(&k);

	    strcpy(hs, absender);
	    if (*sendbbs != '\0')
	      sprintf(hs + strlen(hs), "@%s", sendbbs);
	    sprintf(hs + strlen(hs), " used redist server %s (%s@%s)",
		    dest, newb, newmbx);
	    append_profile(-6, hs);

	    if (*sendbbs != '\0') {
	      sprintf(hs, "%simport%cXXXXXX", newmaildir, extsep);
	      mymktemp(hs);
	      k = sfcreate(hs, FC_FILE);
	      if (k >= minhandle) {
		str2file(&k, Console_call, true);   /* frombox */
		str2file(&k, Console_call, true);   /* absender */
		str2file(&k, absender, true);   /* an */
		str2file(&k, sendbbs, true);   /* mbx */
		*hs = '\0';
		rspacing(hs, 30);
		str2file(&k, hs, true);   /* lt */
		strcpy(hs, "                   ");
		str2file(&k, hs, true);   /* BID */
		strcpy(hs, "ACK:REDIST server acknowledged");
		str2file(&k, hs, true);   /* subject */
		sprintf(hs, "Mail delivered to %s@%s", newb, newmbx);
		str2file(&k, hs, true);
		sfclose(&k);
	      }

	    }

	    sprintf(STR7, "%simport%c%c", newmaildir, extsep, allquant);
	    sort_new_mail(-1, STR7, Console_call);

	  } else
	    sfclosedel(&k);


	}

      }
    }

  }
  return Result;
}


/* ------------------------------------------------------------------------------------------ */

/* der PING - Server...                          */
/* Sx PING@CONSOLE_CALL, Sx P1NG@CONSOLE_CALL    */

boolean do_ping(char *absender, char msgtype, char *subject, char *bid, char *tname,
      	      	long offset, time_t rxtime)
{
  short   k, l;
  long	  filesize;
  boolean has_eol, was_eol;
  char	  hs[256], lastr[256];
  mbxtype tobbs, atbbs;

  if (!ping_allowed) return false;
  if (!exist(tname)) return false;
  if (!callsign(absender)) return false;
  if (!strcmp(absender, "P1NG")) return false; /* this would create a wonderful loop! 	      	      */
  if (!strcmp(absender, "PING")) return false; /* just in case someone defined PING as a callsign ... */
  l = sfopen(tname, FO_READ);
  if (l < minhandle) return false;
  sfseek(offset, l, SFSEEKSET);
  *lastr = '\0';
  while (file2str(l, hs) && strstr(hs, "R:") == hs) strcpy(lastr, hs);
  sfclose(&l);
  get_rcall(lastr, atbbs);
  if (*atbbs == '\0') strcpy(atbbs, ownhiername);
  unhpath(atbbs, hs);
  if (!callsign(hs)) return false;
  strcpy(tobbs, atbbs);
  if (*subject == '@' && count_words(subject) == 1 && strlen(subject) <= LEN_MBX+1) {
    unhpath(&subject[1], hs);
    upper(hs);
    if (callsign(hs) && is_bbs(hs)) {
      complete_hierarchical_adress(hs);
      strcpy(tobbs, hs);
    }
  }
  snprintf(hs, 255, "%simport%cXXXXXX", newmaildir, extsep);
  mymktemp(hs);
  k = sfcreate(hs, FC_FILE);
  if (k < minhandle) return false;
  str2file(&k, Console_call, true);   	      /* frombox */
  str2file(&k, Console_call, true);   	      /* sender */
  str2file(&k, absender, true);       	      /* to */
  str2file(&k, tobbs, true);          	      /* mbx */
  *hs 	= '\0';
  rspacing(hs, 50);
  str2file(&k, hs, true);     	      	      /* lt */
  str2file(&k, hs, true);     	      	      /* BID */
  snprintf(hs, 255, "%s PING server reply", Console_call);
  str2file(&k, hs, true);     	      	      /* subject */
  sprintf(hs, "From: %s @ %s", Console_call, ownhiername);
  str2file(&k, hs, true);
  snprintf(hs, 255, "To:   %s @ %s", absender, tobbs);
  str2file(&k, hs, true);
  str2file(&k, "", true);
  snprintf(hs, 255, "Hi, this is the ping server at %s %s", ownhiername, ownfheader);
  str2file(&k, hs, true);  
  snprintf(hs, 255, "Pinged at %s %s UTC by DPBOX v%s%s", clock_.datum4, clock_.zeit, dp_vnr, dp_vnr_sub);
  str2file(&k, hs, true);
  str2file(&k, "", true);
  snprintf(hs, 255, "Your message arrived with type: %c", msgtype);
  str2file(&k, hs, true);
  snprintf(hs, 255, "Your subject was: %s", subject);
  str2file(&k, hs, true);
  if (!*bid) str2file(&k, "Your message arrived without BID/MID", true);
  str2file(&k, "Transcript of message follows:", true);
  str2file(&k, "", true);

  l = sfopen(tname, FO_READ);
  if (l < minhandle) return false;
  sfseek(offset, l, SFSEEKSET);
  create_my_rline(rxtime, bid, lastr);
  snprintf(hs, 255, ">%s", lastr);
  str2file(&k, hs, true);
  was_eol = true;
  strcpy(lastr, ">");
  filesize = 0;
  while (file2lstr2(l, hs, 250, &has_eol)) {
    if (was_eol && strstr(hs, "#BIN#") == hs) break;
    filesize += strlen(hs);
    if (filesize > 10000) {
      str2file(&k, "", true);
      if (!was_eol) str2file(&k, "", true);
      snprintf(hs, 255, "%s: File too long for complete echo", Console_call);
      str2file(&k, hs, true);
      break;
    }
    nstrcat(lastr, hs, 255);
    str2file(&k, lastr, has_eol);
    was_eol = has_eol;
    if (has_eol) {
      strcpy(lastr, ">");
    } else {
      *lastr = '\0';
    }
  }
  sfclose(&l);
  sfclose(&k);

  if (log_ping) {
    snprintf(hs, 255, "%s@%s used ping server", absender, atbbs);
    append_profile(-9, hs);
  }
  
  sprintf(hs, "%simport%c%c", newmaildir, extsep, allquant);
  sort_new_mail(-1, hs, Console_call);
  return true;
}
