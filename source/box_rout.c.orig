/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#include <ctype.h>
#include <math.h>
#include "box_rout.h"
#include "boxlocal.h"
#include "pastrix.h"
#include "filesys.h"
#include "box_logs.h"
#include "box_mem.h"
#include "box_scan.h"
#include "tools.h"
#include "box_file.h"
#include "box_sub.h"
#include "box_sys.h"
#include "box_wp.h"
#include "box_inou.h"
#include "box_sf.h"
#include "sort.h"





/* -------------------------------------------------------------------- */

static void print_bbs_extended(short unr, char *hcall, char *desc_)
{
  short		waz, itu;
  boolean	lochit;
  long		dist;
  double	lon, lat, dl1, db1;
  double	entfernung, richtung, gegenrichtung;
  char		subst[256], name[256], continent[256], lsysop[256];
  char		call[256], STR1[256], qthloc[256], qthname[256], disploc[7];
  char		mbsysop[20], allsysops[256], mbsystem[80], digisystem[80], remarks[256];
  char		updatetime[20];
  
  if (unr < 0)
    return;

  *qthname	= '\0';
  *lsysop 	= '\0';
  dl1		= 0;
  db1		= 0;

  unhpath(hcall, call);
  
  if (prefix(call, subst, name, &lon, &lat, &waz, &itu, continent)) {
    sprintf(STR1, "Count(r)y: %s (%s)  WAZ %d ITU %d", name, continent, waz, itu);
    wlnuser(unr, STR1);
    dl1		= lon;
    db1		= lat;
  }

  lochit      	= get_wwloc(desc_, disploc);
  if (lochit) {
    calc_qth(disploc, &dl1, &db1);
  }

  get_digimap_data(true, call, allsysops, mbsysop, qthloc,
	      	   qthname, mbsystem, digisystem, remarks, updatetime);

  if (!lochit) {
    lochit	= calc_qth(qthloc, &dl1, &db1);
    if (lochit)
      strcpy(disploc, qthloc);
  }
  
  if (myqthwwlocvalid && (dl1 != 0 || db1 != 0)) {
    loc_dist(mylaenge, mybreite, dl1, db1,
             &entfernung, &richtung, &gegenrichtung);
    dist	= entfernung;
    if (lochit || dist > 500) {
      if (!lochit)
        dist	= (dist / 100) * 100; /* Genauigkeit verschlechtern */
      sprintf(STR1, "Distance : %ld km (%.0f deg)", dist, richtung);
      if (!lochit)
        strcat(STR1, " (estimated)");
      wlnuser(unr, STR1);
    }
  }
      
  if (*qthname != '\0') {
    sprintf(STR1, "QTH      : %s", qthname);
    wlnuser(unr, STR1);
  }
  
  if (*disploc != '\0') {
    sprintf(STR1, "QTHLOC   : %s", disploc);
    wlnuser(unr, STR1);
    wuser(unr, "Latitude : ");
    calc_latstr(db1, STR1);
    wlnuser(unr, STR1);
    wuser(unr, "Longitude: ");
    calc_lonstr(dl1, STR1);
    wlnuser(unr, STR1);
  }
  
  if (*mbsysop != '\0' || *allsysops != '\0') {
    if (*mbsysop != '\0')
      *allsysops	= '\0';
    sprintf(STR1, "Sysop    : %s%s", mbsysop, allsysops);
    wlnuser(unr, STR1);
  }
  
  if (*remarks != '\0') {
    sprintf(STR1, "Remarks  : %s", remarks);
    wlnuser(unr, STR1);
  }
  
}

/* -------------------------------------------------------------------- */

static void ct8(short unr, char *w, short ct)
{
  if (unr <= 0)
    return;

  rspacing(w, LEN_CALL+1);
  wuser(unr, w);

  if (ct % (short)((79-11)/(LEN_CALL+1)) == 0) {
    wlnuser0(unr);
    wuser(unr, "           ");
  }
}

#define MAXBBSPATHLAST 15
#define MAXBBSPATHSTREAM 150
static void bbspath(short unr, boolean privates, char *box_, char *nachbar)
{
  short		ct, k, row, x;
  boolean	first, lp1;
  calltype	box, rxfrom, last[MAXBBSPATHLAST];
  hboxtyp	hbox;
  char		w[256];

  load_initial_hbox();
  debug(2, unr, 64, box_);

  strcpy(box, box_);

  ct		= 0;
  row		= 0;
  for (x = 0; x < MAXBBSPATHLAST; x++) {
    *last[x]	= '\0';
  }
  *nachbar	= '\0';
  *rxfrom	= '\0';
  first		= true;
  lp1		= false;

  k		= sfopen(hpath_box, FO_READ);
  if (k >= minhandle) {
    if (unr > 0) {
      if (privates)
        wuser(unr, "best Path: ");
      else
        wuser(unr, "Best path: ");
    }
    do {

      ct++;
      load_hbox(k, box, &hbox);
      if (!strcmp(hbox.call, box)) {
	if (first) {
	  if (privates)
	    strcpy(rxfrom, hbox.rxfrom_p);
	  else
	    strcpy(rxfrom, hbox.rxfrom_bp);
	  first	= false;
	}
	if (privates)
	  strcpy(box, hbox.bestfrom_p);
	else
	  strcpy(box, hbox.bestfrom_bp);
	strcpy(w, hbox.call);
	if (++row >= MAXBBSPATHLAST)
	  row = 0;
	strcpy(last[row], w);
	ct8(unr, w, ct);
      }
      
      for (x = 0; x < MAXBBSPATHLAST; x++) {
	if (!strcmp(box, last[x])) {
	  if (x == row)
	    lp1	= true;
	  else
	    ct	= MAXBBSPATHSTREAM;
	  break;
	}
      }

    } while (!lp1 && ct < MAXBBSPATHSTREAM && *hbox.call != '\0' && strcmp(box, Console_call));

    if (ct >= MAXBBSPATHSTREAM)
      strcpy(w, "**loop");
    else if (*hbox.call == '\0') {
      if (in_sfp(box)) {
	strcpy(w, box);
	strcpy(nachbar, box);
      } else
	strcpy(w, "***???");
    } else {
      strcpy(w, Console_call);
      strcpy(nachbar, last[row]);
    }

    if (unr > 0) {
      ct++;
      ct8(unr, w, ct);
      wlnuser0(unr);
    }

    sfclose(&k);
  }

  if (*nachbar == '\0')
    strcpy(nachbar, rxfrom);
}
#undef MAXBBSPATHLAST
#undef MAXBBSPATHSTREAM


/* get_real_neighbour() untersucht, ob die nachbarbox als sf-partner definiert ist.      */
/* wenn nicht, dann wird geschaut, ob der nachbar der nachbarbox definiert ist...        */
/* dies ist nur eine notloesung, wenn man zuhause den router mit mitschnitten fuettert   */

static void get_real_neighbour(char *n1_, char *rn)
{
  boolean	ok;
  hboxtyp	hbox;
  char		n1[256];

  if (in_sfp(n1_)) {
    strcpy(rn, n1_);
    return;
  }

  strcpy(n1, n1_);

  load_hbox(nohandle, n1, &hbox);

  ok		= (strcmp(hbox.call, n1) == 0);
  if (ok) {
    strcpy(n1, hbox.rxfrom_bp);
    ok		= in_sfp(n1);
  }

  if (ok)
    strcpy(rn, hbox.rxfrom_bp);
  else
    strcpy(rn, n1);
}

static long age_local_partners_quality_for_display(long oquality)
{
  long	    quality;
  
  if (oquality >= WPRMAXQUAL) return oquality;
  quality   = oquality * 2 + WPRHOPAGING;
  quality   += (quality * WPRPHOPAGING / 100);
  return quality;
}

#define psize_          16384

static void get_bbs_info(short unr, char *boxcall, char *hname, char *nachbar,
			 long *average, long *min, long *cts)
{
  short		ct;
  boolean	found, multi, namesearch, is_call;
  long 		size, rp, minutes, days, hours, x, sz, lz;
  char		*puffer, *buff;
  hboxtyp	*hbox, hb;
  mbxtype	w, wcp;
  char		hs[256], hs1[256], hs2[256];

  load_initial_hbox();
  debug(3, unr, 11, boxcall);

  puffer	= NULL;
  *average	= 0;
  *min		= 0;
  *cts		= 0;
  *hname	= '\0';
  *nachbar	= '\0';
  is_call	= callsign(boxcall);
  multi		= (!is_call && unr > 0);
  if (unr <= 0 && !is_call) return;
  namesearch	= false;
  wcp[0]	= '\0';   /* muss 0 bleiben */

  if (multi) {
    if (*boxcall != '\0') {
      if (boxcall[0] == '<') {
	strdelete(boxcall, 1, 1);
	del_leadblanks(boxcall);
	if (*boxcall == '\0') {
	  wln_btext(unr, 13);
	  return;
	}
	namesearch = true;
	if (boxcall[0] != '*')
	  sprintf(boxcall, "*%s", strcpy(w, boxcall));
	if (boxcall[strlen(boxcall) - 1] != '*')
	  strcat(boxcall, "*");
      } else if (strchr(boxcall, '*') == NULL)
	strcat(boxcall, "*");
    }
  }

  if (!strcmp(boxcall, Console_call)) {
    if (unr > 0) {
      wuser(unr, "Call    : ");
      wlnuser(unr, ownhiername);
      wlnuser(unr, "Local ;-)");
      return;
    }
    strcpy(hname, ownhiername);
    *cts	= 0;
    *min	= 0;
    *average	= 0;
    strcpy(nachbar, Console_call);
    return;
  }
  buff		= NULL;
  size		= 0;
  
  if (multi) {
    sfbread(false, hpath_box, &buff, &size);
    if (size <= 0) return;
  }
  
  ct		= 0;
  rp		= 0;
  found		= false;
  if (multi) {
    puffer	= malloc(psize_);
    sz		= 0;
  }

  do {

    if (multi) {
      hbox		= (hboxtyp *)(&buff[rp]);
      rp		+= sizeof(hboxtyp);
      if (puffer != NULL) {
	if (*boxcall == '\0')
	  found		= true;
	else {
	  if (namesearch) {
	    strcpyupper(hs, hbox->desc);
	    found	= wildcardcompare(SHORT_MAX, boxcall, hs, wcp);
	  } else {
	    strcpy(hs, hbox->call);
	    if (*hbox->hpath != '\0')
	      sprintf(hs + strlen(hs), ".%s", hbox->hpath);
	    found	= wildcardcompare(SHORT_MAX, boxcall, hs, wcp);
	  }
	}
	if (found) {
	  if (sz < psize_ - 8)
	    put_line(puffer, &sz, hbox->call);
	}
      } else
	wlnuser(unr, "no mem");
    } else {
      load_hbox(nohandle, boxcall, &hb);
      hbox		= &hb;
      found		= (strcmp(hbox->call, boxcall) == 0);
    }

    if (found) {
      if (unr > 0) {
	if (!multi) {

	  wuser(unr, "Call     : ");
	  strcpy(hs, hbox->call);   /* we need hs for gen_sftest2() below...  */
	  if (*hbox->hpath != '\0') {
	    strcat(hs, ".");
	    strcat(hs, hbox->hpath);
      	  }
	  wlnuser(unr, hs);
      	  if (hbox->wprot_update > 0) {
	    ix2string4(hbox->wprot_update, hs1);
	    cut(hs1, 16); /* no seconds */
	    wuser(unr, "Last WP  : ");
	    wlnuser(unr, hs1);
	    if (*hbox->sysopcall) {
      	      wuser(unr, "Sysop    : ");
	      wlnuser(unr, hbox->sysopcall);
	    }
	    if (*hbox->connectcall) {
      	      wuser(unr, "SF-Call  : ");
	      wlnuser(unr, hbox->connectcall);
	    }
	    if (hbox->last_incoming_connect > 0) {
	      wuser(unr, "DirectCon: ");
	      lwuser(unr, hbox->incoming_connects);
	      wlnuser0(unr);
	      wuser(unr, "LastCon  : ");
	      ix2string4(hbox->last_incoming_connect, hs1);
	      cut(hs1, 16); /* no seconds */
	      wlnuser(unr, hs1);
	    }
	  }
      	  if (do_wprot_routing && hbox->cur_wprot_routing_update > 0) {
	    ix2string4(hbox->cur_wprot_routing_update, hs2);
	    snprintf(hs1, 200, "ActivRout: %s Quality:%ld Hops:%d Update:%s",
	      	     hbox->cur_routing_neighbour, hbox->cur_routing_quality, hbox->cur_routing_hops, hs2);
	    if (!valid_routing_timestamp(hbox->cur_wprot_routing_update)) strcat(hs1, " (outdated)");
	    wlnuser(unr, hs1);
	    if (valid_wprot_timestamp(hbox->last_direct_routing_update)) {
	      ix2string4(hbox->last_direct_routing_update, hs2);
	      snprintf(hs1, 200, "LastDirUp: %s", hs2);
	      if (!valid_routing_timestamp(hbox->last_direct_routing_update)) strcat(hs1, " (outdated)");
	      wlnuser(unr, hs1);
      	    }
	  }
	  print_bbs_extended(unr, hs, hbox->desc);
	  wuser(unr, "Info     : ");
	  wlnuser(unr, hbox->desc);
	  ix2string4(hbox->lasttx_bp, hs1);
	  cut(hs1, 16); /* no seconds */
	  wuser(unr, "Last Tx  : ");
	  wlnuser(unr, hs1);
	  if (hbox->best_bp < maxlonginteger) {
	    x		= hbox->best_bp;
	    days	= x / 1440;
	    hours	= x % 1440;
	    minutes	= hours % 60;
	    hours	/= 60;
	    sprintf(w, "MinTimeB : %ld days %.2ld:%.2ld", days, hours, minutes);
	    wlnuser(unr, w);
	  }
	  if (hbox->aver_bp < maxlonginteger) {
	    x		= hbox->aver_bp;
	    days	= x / 1440;
	    hours	= x % 1440;
	    minutes	= hours % 60;
	    hours	/= 60;
	    sprintf(w, "AverTimeB: %ld days %.2ld:%.2ld", days, hours, minutes);
	    wlnuser(unr, w);
	  }
	  if (hbox->lasttx_p > 0) {
	    if (hbox->best_p < maxlonginteger) {
	      x		= hbox->best_p;
	      days	= x / 1440;
	      hours	= x % 1440;
	      minutes	= hours % 60;
	      hours	/= 60;
	      sprintf(w, "MinTimeP : %ld days %.2ld:%.2ld", days, hours, minutes);
	      wlnuser(unr, w);
	    }
	    if (hbox->aver_p < maxlonginteger) {
	      x		= hbox->aver_p;
	      days	= x / 1440;
	      hours	= x % 1440;
	      minutes	= hours % 60;
	      hours	/= 60;
	      sprintf(w, "AverTimeP: %ld days %.2ld:%.2ld", days, hours, minutes);
	      wlnuser(unr, w);
	    }
	  }
	  wuser(unr, "MsgCtB   : ");
	  lwuser(unr, hbox->msgct_bp - hbox->msgct_p);
	  wlnuser0(unr);
	  if (hbox->msgct_p > 0) {
	    wuser(unr, "MsgCtP   : ");
	    lwuser(unr, hbox->msgct_p);
	    wlnuser0(unr);
	  }
	  wuser(unr, "Rx Bytes : ");
	  lwuser(unr, hbox->bytect_bp);
	  wlnuser0(unr);
	  gen_sftest2(unr, "DL1XYZ", hs);
	  bbspath(unr, false, hbox->call, hs1);
	  if (hbox->msgct_p > 0) {
	    bbspath(unr, true, hbox->call, hs1);
	  }
	  if (!smart_routing) {
	    wuser(unr, "SmartRout: ");
	    wlnuser(unr, hs1);
	  }
	  if (!do_wprot_routing && valid_routing_timestamp(hbox->cur_wprot_routing_update)) {
	    wuser(unr, "ActivRout: ");
	    wlnuser(unr, hbox->cur_routing_neighbour);
	  }
	}

      } else { /* this is for the sf autorouter */

	strcpy(hname, hbox->call);
	if (*hbox->hpath != '\0') sprintf(hname + strlen(hname), ".%s", hbox->hpath);
	*nachbar = '\0';

      	/* active router */
	if ((unr == -1 || unr == -3)
	    && do_wprot_routing
	    && valid_routing_timestamp(hbox->cur_wprot_routing_update)
	    && *hbox->cur_routing_neighbour) {

	  *cts		= hbox->msgct_bp;
	  *min		= hbox->best_bp;
	  *average	= hbox->aver_bp;
	  strcpy(nachbar, hbox->cur_routing_neighbour);

      	/* passive router by private mails */
	} else if ((unr == -2 || unr == -3)
	      	    && route_by_private
		    && hbox->msgct_p > 0) {

	  *cts		= hbox->msgct_p;
	  *min		= hbox->best_p;
	  *average	= hbox->aver_p;
	  if (!strcmp(hbox->bestfrom_p, boxcall))
	    strcpy(nachbar, boxcall);
	  else
	    bbspath(-1, true, boxcall, nachbar);
	  get_real_neighbour(nachbar, nachbar);

      	/* passive router by all mails */
      	} else if (unr == -2 || unr == -3) {

	  *cts		= hbox->msgct_bp;
	  *min		= hbox->best_bp;
	  *average	= hbox->aver_bp;
	  if (!strcmp(hbox->bestfrom_bp, boxcall))
	    strcpy(nachbar, boxcall);
	  else
	    bbspath(-1, false, boxcall, nachbar);
	  get_real_neighbour(nachbar, nachbar);

      	}
      }
    }
    if (multi)
      found		= false;

  } while (multi && !found && rp < size);

  if (multi) {
    if (puffer != NULL && sz > 0) {
      sort_mem(puffer, &sz, false);
      ct		= 1;
      lz		= 0;
      while (lz < sz) {
	get_line(puffer, &lz, sz, hs);
	rspacing(hs, LEN_CALL+1);
	wuser(unr, hs);
	if (ct % (short)(79/(LEN_CALL+1)) == 0)
	  wlnuser0(unr);
	ct++;
      }
    }
    if (puffer != NULL)
      free(puffer);
  }

  if (ct > 0)
    wlnuser0(unr);

  if (!multi) {
    if (!found) {
      wln_btext(unr, 13);
      *hname		= '\0';
      *nachbar		= '\0';
    }
  }

  if (buff != NULL)
    free(buff);
}

#undef psize_


static void show_sfdefs(short unr, char *call)
{
  boolean     	  all;
  sfdeftype   	  *hp;
  sffortype   	  *fp1;
  char	      	  hs[256];
  
  if (!boxrange(unr)) return;

  all 	= !strcmp(call, "*") || !strcmp(call, "ALL");
  
  if (all) hp = sfdefs;
  else hp = find_sf_pointer(call);
  if (hp == NULL) {
    wuser(unr, call);
    wuser(unr, ": ");
    wln_btext(unr, 45);
    return;
  }

  while (hp != NULL) {

    if (!hp->usersf || user[unr]->supervisor) {

      wuser(unr, "Forward definition for: ");
      wlnuser(unr, hp->call);

      fp1     = hp->forp;
      if (fp1 != NULL) {
	wlnuser0(unr);
	strcpy(hs, "FOR:     ");
	while (fp1 != NULL) {
	  if (strlen(hs) + strlen(fp1->pattern) + 1 < 75) {
            strcat(hs, " ");
            strcat(hs, fp1->pattern);
	  } else {
            wlnuser(unr, hs);
            strcpy(hs, "          ");
            strcat(hs, fp1->pattern);
	  }
	  fp1   = fp1->next;
	}
	wlnuser(unr, hs);
      }

      fp1     = hp->notforp;
      if (fp1 != NULL) {
	wlnuser0(unr);
	strcpy(hs, "NOTFOR:  ");
	while (fp1 != NULL) {
	  if (strlen(hs) + strlen(fp1->pattern) + 1 < 75) {
            strcat(hs, " ");
            strcat(hs, fp1->pattern);
	  } else {
      	    wlnuser(unr, hs);
      	    strcpy(hs, "          ");
      	    strcat(hs, fp1->pattern);
	  }
	  fp1   = fp1->next;
	}
	wlnuser(unr, hs);
      }
      if (all) wlnuser0(unr);
    }
    
    if (all) hp = hp->next;
    else hp = NULL;
  }
  
}


void show_bbs_info(short unr, char *boxcall)
{
  long		aver, min, cts;
  char		nachbar[80], hname[256];

  if (strstr(boxcall, "-CONFIG ") == boxcall) {
    get_word(boxcall, nachbar); /* delete first word */
    show_sfdefs(unr, boxcall);
  } else {
    get_bbs_info(unr, boxcall, hname, nachbar, &aver, &min, &cts);
  }
}


/*
  mode == -1 -> active router
  mode == -2 -> passive router
  mode == -3 -> both
*/
void find_neighbour(short mode, char *boxcall, char *nachbar)
{
  long		aver, min, cts;
  char		hname[256];

  *nachbar	= '\0';
  if (mode == -1 || mode == -2 || mode == -3)
    get_bbs_info(mode, boxcall, hname, nachbar, &aver, &min, &cts);
}


/* Diese Funktion komplettiert ein Rufzeichen auf eine volle hierarchische   */
/* Adresse. Achtung: eine eventuell mit uebergebene hierarchische Angabe     */
/* wird abgetrennt und falls gefunden durch die neue ersetzt. User machen    */
/* viele falsche Angaben...                                                  */

boolean complete_hierarchical_adress(char *mbx)
{
  hboxtyp	hbox;
  mbxtype 	hmbx;

  if (*mbx == '\0') return false;

  unhpath(mbx, hmbx);
  if (!callsign(hmbx)) return false;

  if (!strcmp(hmbx, Console_call)) {
    strcpy(mbx, ownhiername);
    return true;
  }

  load_hbox(nohandle, hmbx, &hbox);
  if (!strcmp(hbox.call, hmbx)) {
    if (*hbox.hpath != '\0') {
      snprintf(mbx, LEN_MBX, "%s.%s", hbox.call, hbox.hpath);
    }
    return true;
  }
  return false;
}

/* only add a hierarchical address if none was existant */

void add_hpath(char *mbx)
{
  if (strchr(mbx, '.') != NULL) return;
  complete_hierarchical_adress(mbx);
}

/* Prüft, ob das Callsign eine bekannte Mailbox ist */

boolean is_bbs(char *callsign)
{
  hboxtyp    	hbox;
  mbxtype     	hmbx;
  
  unhpath(callsign, hmbx);
  if (!strcmp(hmbx, Console_call)) return true;
  load_hbox(nohandle, hmbx, &hbox);
  if (!strcmp(hbox.call, hmbx)) return true;
  return false;
}

/* Prüft, ob das Callsign eine bekannte Mailbox fuer direkten S&F ist */

boolean direct_sf_bbs(char *callsign)
{
  hboxtyp    	hbox;
  mbxtype     	hmbx;
  
  if (!allow_direct_sf) return false;
  unhpath(callsign, hmbx);
  if (!strcmp(hmbx, Console_call)) return false;
  load_hbox(nohandle, hmbx, &hbox);
  if (strcmp(hbox.call, hmbx)) return false;
  if (!valid_wprot_timestamp(hbox.wprot_update)) return false;
  if (hbox.wprot_status < 1) return false;
  return true;
}

/* konvertiert altes hpath-format (<5.08.19) in neues Format um */

static void convert_hpath(void)
{
  short		kin_index, kout_index;
  long		ct, dsize;
  hboxtyp	nheader;
  hboxtyp_old 	oheader;
  pathstr	oidxname, nidxname;

  strcpy(oidxname, hpath_box);
  strcpy(nidxname, oidxname);
  nidxname[strlen(nidxname) - 1] = 'N';
  dsize		= sfsize(oidxname);
  if (dsize == 0) return; /* empty hpath.box */

  if (DFree(oidxname) < (2 * dsize / 1024) + MINDISKFREE) {
    debug(0, 0, 12, "disk full");
    return;
  }

  kin_index	= sfopen(oidxname, FO_READ);
  kout_index	= sfcreate(nidxname, FC_FILE);
  if (kin_index >= minhandle && kout_index >= minhandle) {
    for (ct = 1; ct <= dsize / sizeof(hboxtyp_old); ct++) {
      if (sfread(kin_index, sizeof(hboxtyp_old), (char *)(&oheader)) != sizeof(hboxtyp_old)) {
	debug(0, 0, 12, "read error");
	break;
      }
      if (callsign(oheader.call)) {
	if (clock_.ixtime - oheader.lasttx < 180*SECPERDAY) {
	  memset(&nheader, 0, sizeof(hboxtyp));
      	  nheader.version = HPATHVERSION;
	  strcpy(nheader.call, oheader.call);
      	  strcpy(nheader.hpath, oheader.hpath);
	  strcpy(nheader.desc, oheader.desc);
	  strcpy(nheader.bestfrom_bp, oheader.bestfrom);
	  strcpy(nheader.rxfrom_bp, oheader.rxfrom);
	  nheader.lasttx_bp = oheader.lasttx;
	  nheader.msgct_bp  = oheader.msgct;
	  nheader.best_bp   = oheader.best;
	  nheader.aver_bp   = oheader.aver;
	  nheader.bytect_bp = oheader.bytect;
	  nheader.at_bp	    = oheader.at;
	  sfwrite(kout_index, sizeof(hboxtyp), (char *)(&nheader));
	}
      }
    }
  }

  sfclose(&kin_index);
  sfclose(&kout_index);
  sfdelfile(oidxname);
  filemove(nidxname, oidxname);
  sfdelfile(nidxname);
}


/* sortiert zu alte Eintraege aus */

void check_hpath(boolean reorg)
{
  short		kin_index, kout_index;
  long		ct, dsize;
  hboxtyp	nheader;
  pathstr	oidxname, nidxname;

  debug0(2, 0, 12);

  if (sfsize(hpath_box) % sizeof(hboxtyp) != 0) {
    if (sfsize(hpath_box) % sizeof(hboxtyp_old) != 0) {
      append_profile(-1, "Cannot convert old HPATH.BOX to newer format. File renamed.");
      strcpy(nidxname, hpath_box);
      strcat(nidxname, ".old_format");
      sfdelfile(nidxname);
      sfrename(hpath_box, nidxname);
    } else {
      convert_hpath();
    }
    clear_hboxhash();
    return;
  }

  if (!reorg)
    return;

  clear_hboxhash();
  strcpy(oidxname, hpath_box);
  strcpy(nidxname, oidxname);
  nidxname[strlen(nidxname) - 1] = 'N';
  dsize		= sfsize(oidxname);
  if (dsize == 0) return; /* empty hpath.box */

  if (DFree(oidxname) < (dsize / 1024) + MINDISKFREE) {
    debug(0, 0, 12, "disk full");
    return;
  }

  kin_index	= sfopen(oidxname, FO_READ);
  kout_index	= sfcreate(nidxname, FC_FILE);
  if (kin_index >= minhandle && kout_index >= minhandle) {
    for (ct = 1; ct <= dsize / sizeof(hboxtyp); ct++) {
      if (sfread(kin_index, sizeof(hboxtyp), (char *)(&nheader)) != sizeof(hboxtyp)) {
	debug(0, 0, 12, "read error");
	break;
      }
      if ((nheader.version == HPATHVERSION) && callsign(nheader.call)) {
	if (clock_.ixtime - nheader.lasttx_bp < 365*SECPERDAY 
	 || clock_.ixtime - nheader.wprot_update < 365*SECPERDAY
         || clock_.ixtime - nheader.wprot_routing_update < 365*SECPERDAY )
	  sfwrite(kout_index, sizeof(hboxtyp), (char *)(&nheader));
      }
    }
  }

  sfclose(&kin_index);
  sfclose(&kout_index);
  sfdelfile(oidxname);
  filemove(nidxname, oidxname);
  sfdelfile(nidxname);
}


boolean add_wprot_box(char *hpath, time_t update, unsigned short status,
      	      	      char *connectcall, char *sysopcall)
{
  long	    seekp;
  short     hbh;
  boolean   found, newer;
  char	    *p;
  calltype  call;
  hboxtyp   hbox;
  
  upper(hpath);
  unhpath(hpath, call);
  upper(sysopcall);
  upper(connectcall);

  if (!callsign(call)) return false;
  if (!valid_wprot_timestamp(update)) return false;
  if (strlen(connectcall) > LEN_CALLSSID) return false;
  if (strlen(sysopcall) > LEN_CALL) return false;
  if (strlen(hpath) > LEN_MBX) return false;

  if (!exist(hpath_box))
    hbh		= sfcreate(hpath_box, FC_FILE);
  else
    hbh		= sfopen(hpath_box, FO_RW);
  if (hbh < minhandle) return false;

  seekp	= load_hbox(hbh, call, &hbox);
  found	= (!strcmp(hbox.call, call));
  newer = true;
  
  if (found) {
    if (update > hbox.wprot_update) {
      if ( clock_.ixtime - hbox.wprot_update > WPFREQ_B
      	|| status != hbox.wprot_status
      	|| strcmp(connectcall, hbox.connectcall)
	|| strcmp(sysopcall, hbox.sysopcall) ) {

      	hbox.wprot_update = update;
      	hbox.wprot_status = status;
      	strcpy(hbox.connectcall, connectcall);
      	strcpy(hbox.sysopcall, sysopcall);
      	if (sfseek(seekp, hbh, SFSEEKSET) == seekp)
          sfwrite(hbh, sizeof(hboxtyp), (char *)(&hbox));

      } else newer = false;
    } else newer = false;
  } else {
    memset(&hbox, 0, sizeof(hboxtyp));
    hbox.version    	= HPATHVERSION;
    hbox.wprot_update   = update;
    hbox.wprot_status   = status;
    strcpy(hbox.connectcall, connectcall);
    strcpy(hbox.sysopcall, sysopcall);
    strcpy(hbox.call, call);
    upper(hpath);
    if (strstr(hpath, call) == hpath) {
      p = &hpath[strlen(call)];
      if (*p == '.') {
      	p++;
	strcpy(hbox.hpath, p);
      } else {
      	*hbox.hpath   	= '\0';
      }
    } else {
      strcpy(hbox.hpath, hpath);
    }
    hbox.msgct_bp	= 1;
    hbox.best_bp	= SHORT_MAX;
    hbox.aver_bp	= SHORT_MAX;
    hbox.bytect_bp	= 1;
  
    seekp		= sfseek(0, hbh, SFSEEKEND);
    if (seekp >= 0) {
      if (sfwrite(hbh, sizeof(hboxtyp), (char *)(&hbox)) == sizeof(hboxtyp))
      	add_bptr(call, seekp);
    }
  }

  sfclose(&hbh);
  return newer;
}



/* ***************************************************************************** */

boolean add_wprot_routing(char *callin, char *rxfromin, time_t timestamp, unsigned long quality, short hops)
{
  long	    seekp, rquality;
  short     hbh;
  boolean   found, newer, write_back;
  mbxtype   call, rxfrom;
  hboxtyp   hbox;
  
  unhpath(callin, call);
  upper(call);
  if (!callsign(call)) return false;
  unhpath(rxfromin, rxfrom);
  upper(rxfrom);
  if (!callsign(rxfrom)) return false;
  if (!valid_wprot_timestamp(timestamp)) return false;

  if (!exist(hpath_box))
    hbh		= sfcreate(hpath_box, FC_FILE);
  else
    hbh		= sfopen(hpath_box, FO_RW);
  if (hbh < minhandle) return false;

  seekp	= load_hbox(hbh, call, &hbox);
  found	= (!strcmp(hbox.call, call));
  newer = false;
  
  if (found) {

    write_back = false;
    if ((timestamp > hbox.last_direct_routing_update || hbox.last_direct_routing_update > clock_.ixtime)
      	&& hops == 1) {
      hbox.last_direct_routing_update = timestamp;
      write_back = true;
    }
    
    if (timestamp > hbox.wprot_routing_update 
    || (timestamp == hbox.wprot_routing_update && quality < hbox.routing_quality) ) {
      if (timestamp > hbox.wprot_routing_update) {
      	hbox.cur_wprot_routing_update = hbox.wprot_routing_update;
	nstrcpy(hbox.cur_routing_neighbour, hbox.routing_neighbour, LEN_CALL);
	hbox.cur_routing_quality = hbox.routing_quality;
	hbox.cur_routing_hops = hbox.routing_hops;
	rquality = hbox.cur_routing_quality;
	if (hbox.cur_routing_hops < 2) {
      	  rquality = age_local_partners_quality_for_display(hbox.cur_routing_quality);
	} else {
	  rquality = hbox.cur_routing_quality;
	}
	write_routes(call, hbox.cur_routing_neighbour, hbox.cur_wprot_routing_update,
	      	      	  rquality, hbox.cur_routing_hops);
      }
      newer = true;
      hbox.wprot_routing_update = timestamp;
      hbox.routing_hops = hops;
      nstrcpy(hbox.routing_neighbour, rxfrom, LEN_CALL);
      hbox.routing_quality = quality;
      write_back = true;
    }
    
    if (write_back && sfseek(seekp, hbh, SFSEEKSET) == seekp) {
      sfwrite(hbh, sizeof(hboxtyp), (char *)(&hbox));
    }

  } else { /* unknown target */
    memset(&hbox, 0, sizeof(hboxtyp));
    hbox.version    	= HPATHVERSION;
    hbox.wprot_routing_update = timestamp;
    hbox.routing_hops   = hops;
    nstrcpy(hbox.routing_neighbour, rxfrom, LEN_CALL);
    hbox.routing_quality = quality;
    strcpy(hbox.call, call);
    hbox.msgct_bp	= 1;
    hbox.best_bp	= SHORT_MAX;
    hbox.aver_bp	= SHORT_MAX;
    hbox.best_p	      	= SHORT_MAX;
    hbox.aver_p 	= SHORT_MAX;
    hbox.bytect_bp	= 1;
      
    seekp		= sfseek(0, hbh, SFSEEKEND);
    if (seekp >= 0) {
      if (sfwrite(hbh, sizeof(hboxtyp), (char *)(&hbox)) == sizeof(hboxtyp)) {
      	add_bptr(call, seekp);
	newer = true;
      }
    }
  }

  sfclose(&hbh);
  return newer;
}


routingtype *find_routtable(char *call)
{
  char	      csum;
  routingtype *rp;
  
  csum	= calccs(call);
  rp  	= routing_root;
  while (rp != NULL) {
    if (rp->csum == csum && !strcmp(rp->call, call)) break;
    rp	= rp->next;
  }
  return rp;
}


/* the last measurement is read from disk,   */
/* this will be used as an initial value     */
/* after restart of the bbs 	      	     */
boolean load_routing_table(void)
{
  short       k, x, y, z;
  long	      l;
  time_t      thattime;
  unsigned short i;
  routingtype *sfp;
  pathstr     w;
  char	      hs[256], w1[256];

  snprintf(w, LEN_PATH, "%srouttab%cbox", boxstatdir, extsep);
  k = sfopen(w, FO_READ);
  if (k < minhandle) return false;
  while (file2str(k, hs)) {
    get_word(hs, w1); /* call */
    if (!*w1 || *w1 == '#') continue;
    if (!strcmp(w1, "LRB")) {
      get_word(hs, w1);
      if (!zahl(w1)) continue;
      l = atol(w1);
      if (l > clock_.ixtime) continue;
      last_wprot_r = l;
      get_word(hs, w1);
      continue;
    }
    if (!strcmp(w1, "LMV")) {
      get_word(hs, w1);
      if (!callsign(w1)) continue;
      sfp = find_routtable(w1);
      if (sfp == NULL) continue;
      get_word(hs, w1);
      if (!zahl(w1)) continue;
      thattime = atol(w1);
      if (thattime > clock_.ixtime) continue;
      if (thattime < clock_.ixtime - LINKSPEEDS * WPRLINKCHECKBLOCKS) continue;
      get_word(hs, w1);
      if (!zahl(w1)) continue;
      l = atol(w1);
      if (l != WPRLINKCHECKBLOCKS) continue;
      x = (clock_.ixtime - thattime) / WPRLINKCHECKBLOCKS;
      if (x < 0 || x >= LINKSPEEDS) continue;
      if (count_words(hs) != LINKSPEEDS * 2) continue;
      z = sfp->speedsrow;
      sub_lrow(z, x);
      for (y = x; y < LINKSPEEDS; y++) {
      	get_word(hs, w1);
	if (!zahl(w1)) continue;
	l = atol(w1);
	if (l > WPRMAXQUAL) continue;
	sfp->speeds[z] = l;
      	get_word(hs, w1);
	if (!zahl(w1)) continue;
	l = atol(w1);
	sfp->sizes[z] = l;
	dec_lrow(z);
      }
      continue;
    }
    if (!callsign(w1)) continue;
    sfp = find_routtable(w1);
    if (sfp == NULL) continue;
    get_word(hs, w1); /* timestamp */
    if (!zahl(w1)) continue;
    l = atol(w1);
    if (l < WPRMINQUAL) continue;
    if (sfp->lastspeed >= l) continue;
    get_word(hs, w1);
    if (!zahl(w1)) continue;
    i = atoi(w1);
    if (i < 1) continue;
    sfp->lastspeed = l;
    sfp->last_measured = i;
    get_word(hs, w1); /* timestamp lasttry */
    if (!zahl(w1)) continue;
    l = atol(w1);
    if (sfp->lasttry >= l) continue;
    sfp->lasttry = l;
    if (sfp->lasttry > clock_.ixtime) sfp->lasttry = clock_.ixtime;
    get_word(hs, w1); /* timestamp lastconnecttry */
    if (!zahl(w1)) continue;
    l = atol(w1);
    if (sfp->lastconnecttry >= l) continue;
    sfp->lastconnecttry = l;
    if (sfp->lastconnecttry > clock_.ixtime) sfp->lastconnecttry = clock_.ixtime;
  }
  sfclose(&k);
  return true;
}

/* the last measurement is written to disk,   	      	  */
/* this will be used as an initial value at a further 	  */
/* restart of the bbs 	      	      	      	      	  */
void save_routing_table(void)
{
  short       x, y;
  routingtype *sfp;
  pathstr     w;
  char	      hs[256];

  snprintf(w, LEN_PATH, "%srouttab%cbox", boxstatdir, extsep);
  sfdelfile(w);

  append(w, "# Do not touch this file!", true);
  append(w, "# !!!", true);
  snprintf(hs, 255, "LRB %ld", last_wprot_r);
  append(w, hs, true);  
  
  sfp 	= routing_root;
  while (sfp != NULL) {
    snprintf(hs, 255, "%s %ld %d %ld %ld", sfp->call, sfp->lastspeed, sfp->last_measured,
      	      	      	    sfp->lasttry, sfp->lastconnecttry);
    append(w, hs, true);
    snprintf(hs, 255, "LMV %s %ld %d", sfp->call, clock_.ixtime, WPRLINKCHECKBLOCKS);
    append(w, hs, false);
    y   = sfp->speedsrow;
    for (x = 0; x < LINKSPEEDS; x++) {
      sprintf(hs, " %ld %ld", sfp->speeds[y], sfp->sizes[y]);
      append(w, hs, false);
      dec_lrow(y);
    }
    append(w, "", true);
    sfp = sfp->next;
  }
}

/* aging of link quality  */
static unsigned short link_aging(unsigned short speed, time_t measured, boolean file_forward)
{
  time_t  diff;
  long	  z, h;

  debug0(3, 0, 236);
  diff	  = clock_.ixtime - measured;
  if (diff <= 0) diff = 1;
  /* for each hour since last measurement, link speed is increased by 20 % of the measured value */
  /* (but counting starts after WPR_DUMMY seconds have passed (dummy link test interval) ) */
  if (diff > WPR_DUMMY) {
    /* mark as broken if more than WPRMAXUNCHECKED seconds have passed since last link check */
    if (diff > WPRMAXUNCHECKED && !file_forward) return WPRMAXQUAL;
    z = (speed / 5); /* 20% */
    if (z < 1) z = 1;
    /* hours since last check */
    h = (diff - WPR_DUMMY) / 3600;
    speed += (h * z);
  }
  if (speed > WPRMAXQUAL) speed = WPRMAXQUAL;
  else if (speed < WPRMINQUAL) speed = WPRMINQUAL;
  return speed;
}

/* finally, here, the overall speed of all link checks is computed  */
/* depending on "next", the current entry is only updated, or the complete table is shifted one round */
static void calc_linkspeed_result(unsigned long newspeed, unsigned long newsize,
      	      	      	      	  routingtype *sfp, boolean next)
{
  short   x, v, percent;
  long	  tsize;
  long	  minspeed;
  double  tspeed;

  debug0(3, 0, 235);
  if (sfp == NULL) return;
  
  if (sfp->speedsrow < 0 || sfp->speedsrow >= LINKSPEEDS) {
    debug(3, 0, 236, "speedsrow out of range");
    sfp->speedsrow = 0;
  }
  sfp->speeds[sfp->speedsrow] = newspeed;
  sfp->sizes[sfp->speedsrow] = newsize;

  if (next) {
    inc_lrow(sfp->speedsrow);
    sfp->speeds[sfp->speedsrow] = 0;
    sfp->sizes[sfp->speedsrow] = 0;
  }
  
  v   	      = 0;
  sfp->kspeed = 0;
  tspeed      = 0.0;
  tsize       = 0;
  minspeed    = WPRMAXQUAL;
  for (x = 0; x < LINKSPEEDS; x++) {
    tsize += sfp->sizes[x];
  } 
  for (x = 0; x < LINKSPEEDS; x++) {
    if (sfp->speeds[x] != 0) {
      v++;
      if (sfp->speeds[x] < minspeed) minspeed = sfp->speeds[x];
      percent = calc_prozent(sfp->sizes[x], tsize);
      if (percent < 1) percent = 1;
      tspeed += sfp->speeds[x] * percent / 100;
    }
  }
  if (v == 0) {
    if (sfp->last_measured > 0) {
      sfp->kspeed = sfp->last_measured;
    } else sfp->kspeed = WPRMAXQUAL;
  } else {
    sfp->kspeed = rint(tspeed);
    if (minspeed > sfp->kspeed) sfp->kspeed = minspeed;
  }
  
  sfp->kspeed = link_aging(sfp->kspeed, sfp->lastspeed, sfp->file_forward);

  if (sfp->file_forward) {
    /* if this link goes via file forward, age it with SHORT_MAX/2 */
    if (sfp->kspeed >= WPRADDWIRE) sfp->kspeed = WPRMAXQUAL;
    else sfp->kspeed += WPRADDWIRE;
  }
}

/* this function computes the "real world" value for the link speed: seconds per 100 kBytes   */
/* if "clear" is set, the so far collected data is cleared and the table shifted one round    */
/* this should be done all WPRLINKCHECKBLOCKS seconds  	      	      	      	      	      */
static void calc_linktable(routingtype *sfp, boolean clear)
{
  long	    speed;

  debug0(3, 0, 234);
  if (sfp == NULL) return;
  if (sfp->tempspeeds > 0 && sfp->tempsizes > 0) {
    speed = (sfp->tempsizes * TICKSPERSEC) / sfp->tempspeeds; /* == bytes / sec */
    if (speed < 1) speed = 1;
    speed = 100*1024 / speed; /* == seconds for 100 kBytes */
    if (speed < WPRMINQUAL) speed = WPRMINQUAL;
    else if (speed > WPRMAXQUAL-10) speed = WPRMAXQUAL-10;
    sfp->lastspeed  = clock_.ixtime;
    sfp->last_measured = speed;
  } else speed = 0;
  calc_linkspeed_result((unsigned long)speed, sfp->tempsizes, sfp, clear);
  if (clear) {
    sfp->tempsizes  = 0;
    sfp->tempspeeds = 0;
  }
}

/* here we get single results of linktests, no matter */
/* how often and for how many data    	      	      */
void calc_linkspeed(routingtype *sfp, long starttime, long size)
{
  long ticks;

  debug0(3, 0, 230);
  if (sfp == NULL) return;
  if (size < 30) return; /* the linkcheck data for unused links is very small in size: */
      	      	      	 /* about 39 bytes for ASCII forward */
  ticks = statclock() - starttime;
  if (ticks < 0) return;
  if (ticks == 0) ticks = 1;
  /* only use small amounts of data if last measurement is older than WPRSMALLBLOCKAFTER minutes */
  if (size < WPRSMALLBLOCK
    && !sfp->small_blocks
    && clock_.ixtime - sfp->lastspeed < WPRSMALLBLOCKAFTER) return;
  /* IF we use small amounts of data, we will use them consecutively until we get a bigger block */
  sfp->small_blocks = size < WPRSMALLBLOCK;
  sfp->tempspeeds += ticks;
  sfp->tempsizes  += size;
  calc_linktable(sfp, false);
}

static routingtype *new_routtable(char *call)
{
  short       x;
  routingtype *hp;
  
  hp  = malloc(sizeof(routingtype));
  if (hp == NULL) return NULL;
  nstrcpy(hp->call, call, LEN_CALL);
  hp->csum = calccs(hp->call);

  for (x = 0; x < LINKSPEEDS; x++) {
    hp->speeds[x] = 0;
    hp->sizes[x] = 0;
  }
  hp->speedsrow      	= 0;
  hp->tempspeeds     	= 0;
  hp->tempsizes      	= 0;
  hp->lastspeed       	= 0;
  hp->lasttry       	= 0;
  hp->lastconnecttry    = 0;
  hp->last_measured   	= 0;
  hp->small_blocks   	= false;
  hp->file_forward    	= false;
  hp->send_phantom    	= false;
  hp->sends_route_bc  	= false;
  hp->full_partner    	= false;
  hp->routing_guest   	= false;
  calc_linkspeed_result(0, 0, hp, false);

  if (routing_root == NULL) {
    hp->next  	  = NULL;
    routing_root  = hp;
  } else {
    hp->next  	  = routing_root->next;
    routing_root->next = hp;
  }
  return hp;
}

/* this function checks if routing pointers exist with no corresponding sf pointer -> delete  	*/
/* (may happen after a redefinition of sf partners and a reload of the configuration)         	*/
/* check all current definitions and set the flags send_phantom and sends_route_bc accordingly	*/
void compare_routing_and_sf_pointers(void)
{
  short       hbh;
  routingtype *rp, *hp;
  sfdeftype   *sfp;
  hboxtyp     hbox;
  
  hbh 	= sfopen(hpath_box, FO_READ);

  hp  = NULL;
  rp  = routing_root;
  while (rp != NULL) {
  
    if ((sfp = find_sf_pointer(rp->call)) == NULL) {
      if (rp == routing_root) {
      	routing_root = rp->next;
      } else {
      	hp->next = rp->next;
      }
      free(rp);
      rp = routing_root;
      continue;
    }
    
    /* OK, we still have an SF-Pointer. Now check for the flags... */
    rp->sends_route_bc	= false;
    rp->full_partner  	= false;    
    rp->send_phantom	= false;
    rp->routing_guest 	= sfp->routing_guest;
    
    if (!sfp->usersf && sfp->bedingung > 0 && hbh >= minhandle) {
      rp->full_partner	= true;
      load_hbox(hbh, rp->call, &hbox);
      if (!strcmp(hbox.call, rp->call)) { /* OK, found in hpath.box */
      	if (clock_.ixtime - hbox.last_direct_routing_update < WPRHASOWNBC) {
      	  rp->sends_route_bc = true;
      	}
      }
    
    }
    
    /* closer check of phantoms */
    if (rp->full_partner && !rp->sends_route_bc) {
      /* if last measurement is less than WPRHASOWNBC seconds ago */
      if (   rp->lastspeed >= clock_.ixtime - WPRHASOWNBC
      	  /* and last measurement is less than WPRMAXUNCHECKED seconds ago */
      	  && rp->lastspeed >= clock_.ixtime - WPRMAXUNCHECKED ) {
	  /* then we will create phantom broadcasts for this neighbour */
      	rp->send_phantom  = true;
      }
    }
    
    /* prepare for next round */
    hp	= rp;
    rp	= rp->next;
  }

  sfclose(&hbh);
}

/* init a linktable */
void init_linkspeeds(sfdeftype *sfp, boolean file_forward)
{  
  if (sfp == NULL) return;

  sfp->routp  = find_routtable(sfp->call);
  if (sfp->routp == NULL) sfp->routp = new_routtable(sfp->call);
  if (sfp->routp != NULL) sfp->routp->file_forward = file_forward;  
}

/* this function updates all linktables and is typically  */
/* called in a regular interval, eg 1 minute. it times    */
/* operation itself	      	      	      	      	  */
void calc_routing_table(void)
{ 
  static time_t last_update = 0;
  routingtype *sfp;

  if (last_update > clock_.ixtime) last_update = clock_.ixtime;
  if (clock_.ixtime - last_update < WPRLINKCHECKBLOCKS) return;
  last_update = clock_.ixtime;
  sfp 	= routing_root;
  while (sfp != NULL) {
    calc_linktable(sfp, true);
    sfp = sfp->next;
  }
  save_routing_table();
}

unsigned long get_link_quality(char *call)
{
  routingtype *sfp;

  sfp  = find_routtable(call);
  if (sfp == NULL) return WPRINVALID;
  return sfp->kspeed;
}

unsigned long get_link_quality_and_status(char *call)
{
  routingtype *sfp;

  sfp  = find_routtable(call);
  if (sfp == NULL) return WPRINVALID;
  if (!sfp->full_partner) return WPRINVALID;
  return sfp->kspeed;
}

time_t last_linkcheck(char *call)
{
  routingtype *sfp;

  sfp  = find_routtable(call);
  if (sfp == NULL) return 0;
  return sfp->lastspeed;
}

boolean needs_linkcheck(char *call, boolean tryconnect)
{
  routingtype *rp;

  debug(3, 0, 237, call);
  if (!do_wprot_routing) return false;
  rp = find_routtable(call);
  if (rp == NULL) return false;
  if (!rp->full_partner) return false;
  if (clock_.ixtime - rp->lastspeed < WPR_DUMMY) return false;
  if (tryconnect && clock_.ixtime - rp->lastconnecttry < WPR_DUMMY) return false;
  if (clock_.ixtime - rp->lasttry < WPR_DUMMY) return false;
  if (rp->file_forward) return false;

  if (tryconnect)
    rp->lastconnecttry = clock_.ixtime;
  else
    rp->lasttry = clock_.ixtime;

  return true;
}

boolean send_full_routing_bc(char *call)
{
  routingtype *rp;
  
  if ((rp = find_routtable(call)) == NULL) return false;
  return (rp->sends_route_bc || rp->routing_guest);
}

boolean is_phantom(char *call)
{
  routingtype *rp;
  
  if ((rp = find_routtable(call)) == NULL) return false;
  return (rp->send_phantom);
}

boolean is_full_sf_partner_for_routing(char *call)
{
  routingtype *rp;
  
  if ((rp = find_routtable(call)) == NULL) return false;
  return (rp->full_partner);
}

/* does this neighbour presents a "W" in its SID ? */
boolean get_wprot_neighbour(char *call)
{
  short       handle;
  pathstr     fname;
  char	      hs[256];
  
  snprintf(fname, LEN_PATH, "%swprotnbr%cbox", boxstatdir, extsep);
  handle  = sfopen(fname, FO_READ);
  if (handle < minhandle) return false;
  while (file2str(handle, hs)) {
    if (!strcmp(hs, call)) {
      sfclose(&handle);
      return true;
    }
  }
  sfclose(&handle);
  return false;  
}

/* does this neighbour presents a "W" in its SID ? */
void set_wprot_neighbour(char *call, boolean yes)
{
  short       handle, handle2;
  sfdeftype   *sfp;
  pathstr     fname, fname2;
  char	      hs[256];
  
  sfp = find_sf_pointer(call);
  if (sfp == NULL) return;
  if (yes && sfp->em == EM_WPROT) return;
  if (!yes && sfp->em != EM_WPROT) return;

  if (yes) sfp->em = EM_WPROT;
  else if (!yes) sfp->em = EM_WP; /* this is only a fallback, it should not happen */

  snprintf(fname, LEN_PATH, "%swprotnbr%cbox", boxstatdir, extsep);
  snprintf(fname2, LEN_PATH, "%s%ctemp", fname, extsep);
  handle  = sfopen(fname, FO_READ);

  if (handle < minhandle) { /* empty list */
    if (yes) append(fname, call, true);
    return;
  }  
  
  sfdelfile(fname2);
  handle2 = sfcreate(fname2, FC_FILE);
  if (handle2 < minhandle) return;
  
  while (file2str(handle, hs)) {
    if (strcmp(hs, call)) {
      if (find_sf_pointer(hs) != NULL) {
      	str2file(&handle2, hs, true);
      }
    }
  }
  if (yes) str2file(&handle2, call, true);

  sfclosedel(&handle);
  sfclose(&handle2);
  sfrename(fname2, fname);
}

/* this function prints the current linktables */
boolean get_routing_table(short unr)
{
  short       	  x, y, z;
  boolean     	  have_r, have_p;
  char	      	  action;
  routingtype     *rp;
  pathstr     	  sortname;
  char	      	  hs[256], w[256];

  compare_routing_and_sf_pointers();
  
  if (!boxrange(unr)) return false;

  wlnuser(unr, "Quality = Seconds / 100 KBytes TX");
  wlnuser0(unr);
  snprintf(hs, 255, "%-*s   Total   0-%2dm  %3dm  %3dm  %3dm  %3dm  %3dm  %3dm  %3dm  %3dm  %3dm",
    	    LEN_CALL, "Call",
	    WPRLINKCHECKBLOCKS/60 * 1,
	    WPRLINKCHECKBLOCKS/60 * 2,
	    WPRLINKCHECKBLOCKS/60 * 3,
	    WPRLINKCHECKBLOCKS/60 * 4,
	    WPRLINKCHECKBLOCKS/60 * 5,
	    WPRLINKCHECKBLOCKS/60 * 6,
	    WPRLINKCHECKBLOCKS/60 * 7,
	    WPRLINKCHECKBLOCKS/60 * 8,
	    WPRLINKCHECKBLOCKS/60 * 9,
	    WPRLINKCHECKBLOCKS/60 *10   );
  wlnuser(unr, hs);

  snprintf(sortname, LEN_PATH, "%sDPXXXXXX", tempdir);
  mymktemp(sortname);
  sfdelfile(sortname);
  have_r = false;
  have_p = false;
  rp  = routing_root;
  while (rp != NULL) {
    if (rp->full_partner) {
      if (rp->sends_route_bc) {
      	action = 'r';
	have_r = true;
      } else if (rp->send_phantom) {
      	action = 'p';
      	have_p = true;
      } else action = ' ';
      snprintf(hs, 40, "%-*s %c %5d -", LEN_CALL, rp->call, action, rp->kspeed);
      y   = rp->speedsrow;
      z   = LINKSPEEDS;
      if (z > 10) z = 10;
      for (x = 0; x < z; x++) {
      	snprintf(w, 20, " %5ld", rp->speeds[y]);
	strcat(hs, w);
	dec_lrow(y);
      }
      append(sortname, hs, true);
    }
    rp = rp->next;
  }
  sort_file(sortname, false);
  show_textfile(unr, sortname);
  sfdelfile(sortname);
  if (have_r || have_p) {
    wlnuser0(unr);
    if (have_r) {
      wuser(unr, "r = sends route bc");
      if (have_p) wuser(unr, ", ");
    }
    if (have_p) wuser(unr, "p = create phantom bc");
    wlnuser0(unr);
  }
  return true;
}

void get_routing_targets(short unr, char *prefix)
{
  short     hbh, sf, x;
  hboxtyp   hbox;
  char	    boxtype;
  char	    w[256];
  pathstr   fn;

  if (!exist(hpath_box)) return;
  hbh 	= sfopen(hpath_box, FO_READ);
  if (hbh < minhandle) return;
  
  snprintf(fn, LEN_PATH, "%sdptargetsXXXXXX", tempdir);
  mymktemp(fn);
  sf  = sfcreate(fn, FC_FILE);
  if (sf < minhandle) return;
  
  while (sfread(hbh, sizeof(hboxtyp), (char *)&hbox) == sizeof(hboxtyp)) {
    if (hbox.version != HPATHVERSION) continue;
    if (!valid_routing_timestamp(hbox.cur_wprot_routing_update)) continue;
    if (hbox.cur_routing_quality <= 0) continue;
    if (!*hbox.cur_routing_neighbour) continue;
    if (*prefix && strstr(hbox.call, prefix) != hbox.call) continue;
    if (!callsign(hbox.call)) continue;
    if (hbox.cur_routing_hops == 0) hbox.cur_routing_hops = 1;
    if (hbox.cur_routing_hops == 1) {
      hbox.cur_routing_quality = age_local_partners_quality_for_display(hbox.cur_routing_quality);
    }
    lower(hbox.cur_routing_neighbour);
    if (is_phantom_timestamp(hbox.cur_wprot_routing_update)) boxtype = '-';
    else boxtype = '+';
    snprintf(w, 50, "%-*s %6ld%c%2d>%-*s  ", LEN_CALL, hbox.call, hbox.cur_routing_quality,
      	      	      	      	      	     boxtype, hbox.cur_routing_hops,
      	      	      	      	      	     LEN_CALL, hbox.cur_routing_neighbour);
    str2file(&sf, w, true);
  }
  
  sfclose(&sf);
  sfclose(&hbh);
  sort_file(fn, true);
  sf  = sfopen(fn, FO_READ);
  if (sf >= minhandle) {
    wlnuser(unr, "BBSCALL Quality-Hops>Neighbour");
    wlnuser0(unr);
    x = 0;
    while (file2str(sf, w)) {
      x++;
      if (x == 3) {
      	wlnuser(unr, w);
        x = 0;
      } else {
      	wuser(unr, w);
      }
    }
    if (x != 0) wlnuser0(unr);
    sfclose(&sf);
  }
  sfdelfile(fn);
}





/* ************************************************************************************* */




/* Diese Routine untersucht die Nachrichtenkoepfe neu eingehender Mails.     */
/* Mit den daraus gewonnenen Informationen wird der BBS-Router gefuettert.   */
/* Ausserdem wird das Absenderdatum der Nachricht ermittelt.                 */

short scan_hierarchicals(char *from1, char *puffer, long size, time_t *txdate,
			 boolean sfpartner, char msgtyp, char *lastvias)
{
  short		k, i, btc, hops, lct, lastrl, hbh, loops, lviact;
  boolean	found;
  long		lastminutes, minutes, alterfakt, hmsgct, seekp, rp;
  hboxtyp	hbox;
  calltype	neighbour, lastfrom;
  char		hs[256], hcall[256], htime[256], w1[256], adesc[256];

  /* R:940424/0945z @:DL8HBS.#BLN.DEU.EU [] #:MSG_NR []    */
  /* R:940424/0945z MSG_NR@DL8HBS.#BLN.DEU.EU              */

  load_initial_hbox();

  debug(2, 0, 67, from1);

  if (*from1 != '\0')
    strcpy(lastfrom, from1);
  else
    strcpy(lastfrom, Console_call);

  *lastvias	= '\0';
  lviact	= 0;
  strcpy(neighbour, from1);
  *htime	= '\0';
  loops		= 0;
  hops		= 0;
  rp		= 0;
  lct		= 0;
  lastrl	= 0;
  *txdate	= 0;
  btc		= 0;
  hbh		= nohandle;

  if (!exist(hpath_box))
    hbh		= sfcreate(hpath_box, FC_FILE);
  else
    hbh		= sfopen(hpath_box, FO_RW);

  if (puffer != NULL && size > 0 && hbh >= minhandle) {
    lastminutes	= 1;
    do {
      get_line(puffer, &rp, size, hs);
      cut(hs, 120);
      lct++;
      if (strlen(hs) > 6) {
	if (hs[0] == 'R' && hs[1] == ':') {
	  lastrl	= lct;
	  get_word(hs, htime);   /*Uhrzeit*/
	  *txdate	= get_headerdate(htime);
	  if (*txdate > 0) {
	    if (btc < 3 && badtimecount >= 0 && clock_.ixtime + 600 < *txdate) {
	      btc++;
	      if (btc == 3)
		badtimecount++;
	    } else
	      btc	= 4;

	    minutes = (clock_.ixtime - *txdate) / 60;
	    if (minutes < 0) {
	      if (minutes >= -120)
		minutes		= lastminutes;
	    }
	    get_word(hs, hcall);
		/* @:Call.at.con oder @Call.at.con oder 12345@Call.at.con */
	    if (strlen(hcall) > 3 && minutes >= 0) {
	      if (minutes < lastminutes)
		minutes		= lastminutes;
	      lastminutes	= minutes + 1;
	      k 		= strpos2(hcall, "@", 1);
	      if (k > 0) {
		strdelete(hcall, 1, k);
		if (hcall[0] == ':')
		  strdelete(hcall, 1, 1);
		upper(hcall);

		*adesc		= '\0';
		i		= strpos2(hs, "[", 1);
		if (i > 0) {
		  k		= strpos2(hs, "]", 1);
		  if (k > i + 1) {
		    strsub(adesc, hs, i + 1, k - i - 1);
		    strdelete(hs, 1, k);
		    get_word(hs, w1);
		    if (*w1 != '\0' && isalpha(w1[0]))
		      sprintf(adesc + strlen(adesc), " %s", w1);
		  }
		}

		if (*adesc == '\0') {
		  strcpy(adesc, hs);
		  k		= strpos2(adesc, "#:", 1);
		  if (k > 0) {
		    cut(adesc, k - 1);
		    del_blanks(adesc);
		  } else
		    *adesc	= '\0';
		}

		if (*adesc == '\0') {
		  strcpy(adesc, hs);
		  k		= strpos2(adesc, "#:", 1);
		  if (k > 0) {
		    strdelete(adesc, 1, k);
		    get_word(adesc, w1);   /* MSGNUMBER */
		    k		= strpos2(adesc, "$:", 1);
		    if (k > 0)
		      cut(adesc, k - 1);
		  }
		  del_blanks(adesc);
		}
		cut(adesc, LEN_MBX);

		*w1		= '\0';
		k		= strpos2(hcall, ".", 1);
		if (k > 0 && k < LEN_CALL+2) {
		  if (k < strlen(hcall))
		    strsub(w1, hcall, k + 1, strlen(hcall) - k);
		  cut(hcall, k - 1);
		  cut(w1, LEN_MBX);
		} else
		  cut(hcall, LEN_CALL);

		if (callsign(hcall)) {
		  if (lviact < MAXLASTVIAS) {
		    sprintf(lastvias + strlen(lastvias), "%s ", hcall);
		    lviact++;
		  }

		  if (!strcmp(hcall, Console_call))
		    loops++;
		  else {
		    seekp	= load_hbox(hbh, hcall, &hbox);
		    found	= (strcmp(hbox.call, hcall) == 0);
		    if (found) {
		      if (hbox.aver_bp < 1)
			hbox.aver_bp	= 1;
		      else if (hbox.aver_bp > minutes * 30)
			hbox.aver_bp	= minutes;
		      hbox.bytect_bp	+= size - rp;
		      hbox.msgct_bp++;
		      if (sfpartner) {
			hmsgct		= hbox.msgct_bp;
			if (hmsgct > 2000)
			  hmsgct	= 2000;
			hbox.aver_bp	= (hbox.aver_bp * (hmsgct - 1) + minutes) * 4 / (hmsgct * 4);
			if (hbox.aver_bp < 1)
			  hbox.aver_bp	= 1;
		      }
		      if (*txdate > hbox.lasttx_bp) {
			if (*from1 != '\0' && sfpartner) {
			  alterfakt	= (clock_.ixtime - hbox.at_bp) / 60;
			  if (alterfakt < 0)
			    alterfakt	= 0;
			  if (minutes + hops * 3 <= hbox.best_bp || hbox.best_bp == 0 ||
			      alterfakt > hbox.best_bp + 2000) {
			    hbox.best_bp= minutes + hops * 3;
			    if (hbox.aver_bp < hbox.best_bp)
			      hbox.aver_bp= hbox.best_bp;
			    strcpy(hbox.bestfrom_bp, lastfrom);
			    strcpy(hbox.rxfrom_bp, neighbour);
			    hbox.at_bp	= clock_.ixtime;
			  }
			}
			hbox.lasttx_bp	= *txdate;
			if (*adesc != '\0')
			  strcpy(hbox.desc, adesc);
			strcpy(hbox.hpath, w1);			
		      }
		      
		      if (msgtyp == 'P') {

			if (hbox.aver_p < 1)
			  hbox.aver_p	= 1;
			else if (hbox.aver_p > minutes * 30)
			  hbox.aver_p	= minutes;
			hbox.bytect_p	+= size - rp;
			hbox.msgct_p++;
			if (sfpartner) {
			  hmsgct	= hbox.msgct_p;
			  if (hmsgct > 2000)
			    hmsgct	= 2000;
			  hbox.aver_p	= (hbox.aver_p * (hmsgct - 1) + minutes) * 4 / (hmsgct * 4);
			  if (hbox.aver_p < 1)
			    hbox.aver_p	= 1;
			}
			if (*txdate > hbox.lasttx_p) {
			  if (*from1 != '\0' && sfpartner) {
			    alterfakt	= (clock_.ixtime - hbox.at_p) / 60;
			    if (alterfakt < 0)
			      alterfakt	= 0;
			    if (minutes + hops * 3 <= hbox.best_p || hbox.best_p == 0 ||
				alterfakt > hbox.best_p + 2000) {
			      hbox.best_p= minutes + hops * 3;
			      if (hbox.aver_p < hbox.best_p)
				hbox.aver_p= hbox.best_p;
			      strcpy(hbox.bestfrom_p, lastfrom);
			      strcpy(hbox.rxfrom_p, neighbour);
			      hbox.at_p	= clock_.ixtime;
			    }
			  }
			  hbox.lasttx_p	= *txdate;
			}

		      }

		      if (sfseek(seekp, hbh, SFSEEKSET) == seekp)
			sfwrite(hbh, sizeof(hboxtyp), (char *)(&hbox));
		    } else {
      	      	      memset(&hbox, 0, sizeof(hboxtyp));
		      hbox.version    	      	  = HPATHVERSION;
		      strcpy(hbox.call, hcall);
		      strcpy(hbox.hpath, w1);
		      strcpy(hbox.bestfrom_bp, lastfrom);
		      strcpy(hbox.rxfrom_bp, neighbour);
		      strcpy(hbox.desc, adesc);
		      hbox.at_bp	= clock_.ixtime;
		      hbox.lasttx_bp	= *txdate;
		      hbox.msgct_bp	= 1;
		      if (sfpartner) {
			hbox.best_bp	= minutes + hops * 3;
			hbox.aver_bp	= hbox.best_bp;
		      } else {
			hbox.best_bp	= SHORT_MAX;
			hbox.aver_bp	= SHORT_MAX;
		      }
		      hbox.bytect_bp	= size - rp;
		      
		      if (msgtyp == 'P') {
		      	strcpy(hbox.bestfrom_p, lastfrom);
			strcpy(hbox.rxfrom_p, neighbour);
			hbox.lasttx_p 	= *txdate;
			hbox.msgct_p  	= 1;
			if (sfpartner) {
			  hbox.best_p   = minutes + hops * 3;
			  hbox.aver_p   = hbox.best_p;
			} else {
			  hbox.best_p	= SHORT_MAX;
			  hbox.aver_p	= SHORT_MAX;
			}
			hbox.bytect_p 	= size - rp;
			hbox.at_p     	= clock_.ixtime;
		      }
		      
		      seekp		= sfseek(0, hbh, SFSEEKEND);
		      if (seekp >= 0) {
			if (sfwrite(hbh, sizeof(hboxtyp), (char *)(&hbox)) == sizeof(hboxtyp))
			  add_bptr(hcall, seekp);
		      }
		    }
		  }
		}
	      }
	    }
	    if (callsign(hcall))
	      strcpy(lastfrom, hcall);
	    hops++;
	  }
	}
      }
    } while (rp < size && lct - lastrl <= 0);

    *txdate	= get_headerdate(htime);
    if (*txdate == 0)
      *txdate	= clock_.ixtime;
  }

  sfclose(&hbh);
  return loops;
}

