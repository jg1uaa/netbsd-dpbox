/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#define BOX_SEND_G
#include "box_send.h"
#include "tools.h"
#include "crc.h"
#include "yapp.h"
#include "boxlocal.h"
#include "box_logs.h"
#include "box.h"
#include "box_sys.h"
#include "box_sub.h"
#include "box_file.h"
#include "box_mem.h"
#include "box_rout.h"
#include "box_sf.h"
#include "box_tim.h"
#include "box_inou.h"
#include "box_serv.h"



/* wird in main.c gebraucht, zum Abspeichern des Autobin */

short box_s_chan(short unr)
{
  if (boxrange(unr))
    return (user[unr]->sendchan);
  else
    return nohandle;
}


static void disp_sh(short unr)
{
  if (!boxrange(unr))
    return;
  if (user[unr]->sendheader != NULL)
    free(user[unr]->sendheader);
  user[unr]->sendheader = NULL;
}


#define blocksize       4096


boolean send_file1(short unr, short *kanal, boolean in_send, short force_bin)
/* force_bin == 1 -> keep as ascii text, force_bin == 2 -> keep as BIN, force_bin == 0 -> check type */
{
  boolean Result;
  long size, hsize, ct, ctold;
  char *puffer;
  long psize;
  boolean is_bin, werror;
  short x;
  unsigned short crc, date, time;
  char hs[256];
  char w[256];
  char STR1[256];

  debug0(3, unr, 29);
  Result = false;
  is_bin = (force_bin == 2);
  size = sfseek(0, *kanal, SFSEEKEND);
  sfseek(0, *kanal, SFSEEKSET);
  psize = blocksize;
  if (psize > size)
    psize = size;

  puffer = malloc(psize);
  if (puffer != NULL) {
    sfread(*kanal, psize, puffer);

    if (!is_bin && force_bin != 1) {
      for (ct = 0; ct < psize; ct++) {
	if (puffer[ct] > 168 && puffer[ct] != 225) {
	  is_bin = true;
	  break;
	}
      }

      if (is_bin) {  /* koennte auch ein 7PLUS-File sein */
	ct = 0;
	x = 0;
	while (ct < psize && x < 50 && is_bin) {
	  x++;
	  ctold = ct;
	  get_line(puffer, &ct, psize, hs);
	  if (ct - ctold > 80)
	    x = 100;
	  else if (strstr(hs, " go_7+. ") == hs)
	    is_bin = false;
	}
	if (x != 100)
	  is_bin = false;
      }

    }

    if (is_bin) {
      Result = true;
      str2file(&user[unr]->sendchan, "", true);
      str2file(&user[unr]->sendchan, "*** binary file ***", true);
      str2file(&user[unr]->sendchan, "", true);
      crc = 0;
      crcthp_buf(puffer, psize, &crc);
      hsize = psize;
      ct = 1;
      while (hsize < size && ct > 0) {
	ct = size - hsize;
	if (ct > psize)
	  ct = psize;
	ct = sfread(*kanal, ct, puffer);
	hsize += ct;
	crcthp_buf(puffer, ct, &crc);
      }
      handle2name(*kanal, w);
      sfgetdatime(w, &date, &time);
      int2hstr(date, w);
      while (strlen(w) < 4)
        sprintf(w, "0%s", strcpy(STR1, w));
      int2hstr(time, STR1);
      while (strlen(STR1) < 4)
        sprintf(STR1, "0%s", strcpy(hs, STR1));
      sprintf(hs, "#BIN#%ld#|%d#$%s%s#%s",
	      size, crc, w, STR1, user[unr]->tempbinname);
      str2file(&user[unr]->sendchan, hs, true);
      if (in_send)
	wlnuser(unr, hs);
    }

    sfseek(0, *kanal, SFSEEKSET);
    werror = false;
    hsize = 0;
    ct = 1;
    while (hsize < size && ct > 0) {
      ct = size - hsize;
      if (ct > psize)
	ct = psize;
      ct = sfread(*kanal, ct, puffer);
      if (ct <= 0)
	break;
      if (sfwrite(user[unr]->sendchan, ct, puffer) == ct) {
	if (in_send && !is_bin && user[unr]->umode != UM_SYSREQ && user[unr]->tcon > 0)
	      /* damit der Sysop auch sieht, was er einspielt... */
		show_puffer(unr, puffer, ct);
	hsize += ct;
      } else
	ct = 0;
      if (ct == 0)
	werror = true;
    }
    if (puffer != NULL) free(puffer);
    if (werror)
      wlnuser(unr, "write error, please delete imported file manually");
  } else
    wlnuser(unr, TXTNOMEM);

  sfclose(kanal);
  return Result;
}

#undef blocksize

/*---------------------------------------------------------------------------*/
/* The following 7plus error check is taken from baycom box by kind permission
   of Dietmar Zlabinger, OE3DZW. Please check for the original coding in 
   baycom box source code, as I had to modify some data types and function
   calls for dpbox.
*/

static unsigned char decode_7p[256]=
{
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0x0 , 0x1 , 0x2 , 0x3 , 0x4 , 0x5 , 0x6 , 
0x7 , 0x8 , 0xFF, 0x9 , 0xA , 0xB , 0xC , 0xD , 0xE , 0xF , 
0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 
0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 
0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 
0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 
0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 
0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 
0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 
0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0xFF, 0x5D, 0x5E, 
0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 
0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0xFF, 0x6E, 0xFF, 0x6F, 0x70, 
0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 
0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83, 0x84, 
0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 
0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 
0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2, 
0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 
0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 
0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 
0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 
0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 
0xD5, 0xD6, 0xD7, 0xFF, 0xFF, 0xFF};

/*---------------------------------------------------------------------------*/

static short line_7p(char * p)
/*************************************************************************
//
// Check the crc of a line 7plus line p, returns 1 if ok, else 0
//
// This part of code has been taken from 7plus by Axel, DG1BBQ 
// (modificiations by oe3dzw)
//
*************************************************************************/

/*
  ------------------------------------------------------
  DG1BBQ > OE3DZW   16.02.98 18:12 9 Lines 186 Bytes #999 @OE3XSR.#OE3.AUT.EU
  Subj: RE:Verwendung 7plus Source-Code fuer BCM?
  From: DG1BBQ @ DB0VER.#NDS.DEU.EU (Axel)
  To:   OE3DZW @ OE3XSR.#OE3.AUT.EU
  Hallo Dietmar,
  hab' ich nix gegen, solange im Quelltext sowie im Manual daruf hingewiesen
  wird.
  73s, Axel.
  ------------------------------------------------------
*/

{
/* Calculate CRC */
  unsigned short csequence;
  unsigned long cs;
  short i;
  unsigned short crc;
  short slen=strlen(p);

  if (slen < 64 || slen > 71) /* line must be at least 64 chars long */ 
     return (0);              /* usually 69 chars, +2 extra cr or lf */

  if (( (unsigned char) p[62])==(unsigned char) 0xb0 &&
      ( (unsigned char) p[63])==(unsigned char) 0xb1 &&
      ( (unsigned char) p[64])==(unsigned char) 0xb2)     /* header-line eg. filename */
     return (2);         

  csequence = 0;
  for (i=0; i<64; i++)
  crcthp(p[i],&csequence);
  csequence &= 0x3fff; /* strip calculated CRC to 14 bits */

  /* Get crc from code line */

   cs = 0xb640L * decode_7p[(unsigned char)p[66]] +
        0xd8L   * decode_7p[(unsigned char)p[65]] +
                  decode_7p[(unsigned char)p[64]];

   crc = (unsigned short) (cs & 0x3fffL);     /* lower 14 bits are the CRC */


  return (short) (csequence == crc);
}

/* end of 7plus error checking */
/*---------------------------------------------------------------------------*/


void send_text3(short unr, boolean first, char *eingabe_, boolean return_)
{
  char eingabe[256];
  short x;
  boolean invalid, endflag, delmsg, exitflag;
  short snmunr;
  char hs[256];
  userstruct *WITH;

  strcpy(eingabe, eingabe_);
  if (!boxrange(unr))
    return;

  WITH = user[unr];
  disp_sh(unr);

  invalid = (WITH->sendchan < minhandle);
  endflag = false;
  exitflag = false;
  snmunr = -1;

  if (!WITH->f_bbs) {
    if (strchr(eingabe, '\030') != NULL) {
      exitflag = true;   /* abort mit ctrl-x */
      endflag = true;
    } else if (strchr(eingabe, '\032') != NULL)
      endflag = true;
    else {
      if (uspos("***END", eingabe) > 0)
	endflag = true;
      else if (useq("/EX", eingabe))
	endflag = true;
      else
	endflag = (!strcmp("NNNN", eingabe));
    }
    if (!endflag) {
      if (WITH->sp_input == 1) {
      	if (strstr(eingabe, " stop_7+. ") == eingabe) WITH->sp_input = 0;
      	else if (!line_7p(eingabe)) WITH->sp_input = 2;
      }
      else if (WITH->sp_input == 0) {
        if (strlen(eingabe) == 69 && strstr(eingabe, " go_7+. ") == eingabe)
	  WITH->sp_input = 1;
      }
    }
  } else {
    if (strchr(eingabe, '\032') != NULL)
	  /* das '/EX' bei DieBox rausfiltern */
	    endflag = true;
    else if (WITH->sf_level == 1)
      endflag = useq("/EX", eingabe);
    else if (useq(eingabe, "/EX"))
      invalid = true;
  }

  if (endflag) {
    strcpy(WITH->lastcmd, "^Z");
    debug(3, unr, 30, eingabe);
    x = uspos("***END", eingabe);
    if (x == 0) {
      if (useq(eingabe, "/EX"))
	x = 1;
    }
    if (x == 0) {
      if (!strcmp(eingabe, "NNNN"))
	x = 1;
    }
    if (x == 0)
      x = strpos2(eingabe, "\032", 1);

    if (x > 1) {
      delmsg = false;
      strcpy(hs, eingabe);
      cut(hs, x - 1);
      if (!invalid)
	str2file(&WITH->sendchan, hs, true);
    } else
      delmsg = first;
      
    if (WITH->sp_input == 2) {
      wlnuser(unr, "Sorry, this 7+ file is not correct!");
      exitflag = true;
    }
    WITH->sp_input = 0;
    
    if (exitflag)
      delmsg = true;

    if (!invalid)
      sfclose(&WITH->sendchan);
    WITH->sendchan = nohandle;
    if (WITH->f_bbs) {
      if (*WITH->tempbid != '\0') {
	write_msgid(-1, WITH->tempbid);
	/*                   readressing := true; */
	snmunr = -77;
      }
      *WITH->tempbid = '\0';

      if (WITH->sf_level < 3) {
	if (!invalid)
	  sort_new_mail(snmunr, WITH->input2, Console_call);
	WITH->input2[0] = '\0';
	if (WITH->sf_level == 1 && WITH->sf_master)
	  wlnuser(unr, "F>");
	else {
	  chwuser(unr, '>');
	  wlnuser0(unr);
	}
	WITH->action = 5;
	return;
      }
      /*               readressing := false; */
      if (!invalid)
	sort_new_mail(snmunr, WITH->input2, Console_call);
      WITH->input2[0] = '\0';
      prepare_for_next_fbb(unr);
      return;
    }
    WITH->action = 0;
    if (delmsg) {  /* Leere Nachricht, loeschen. */
      sfdelfile(WITH->input2);
      wln_btext(unr, 11);
    } else {
      wln_btext(unr, 6);
      if (!invalid) {   /*unr*/
	sort_new_mail(-1, WITH->input2, Console_call);
	if (WITH->in_reply)
	  set_reply_flag(unr, WITH->reply_brett, WITH->reply_nr);
      }

      /* hier steht jetzt -1, damit der Router das Maul haelt. */
      /* beim manuellen SEND-Befehl wissen wir ja schon,       */
      /* wohin die Reise geht...                               */

      WITH->input2[0] = '\0';
    }
    WITH->in_reply = false;
    return;
  }

  if (invalid)
    return;
  if (!WITH->isfirstchar) {
    str2file(&WITH->sendchan, eingabe, return_);
    return;
  }
  if (true_bin(eingabe) >= 0) {
    if (WITH->pchan > 0) {
      WITH->errors = -5;
      boxprwprg2(WITH->pchan, eingabe);
    } else
      wlnuser(unr, "#BIN# not allowed in terminal mode...");
    return;
  }
  str2file(&WITH->sendchan, eingabe, return_);
}


static void open_sendfile(short unr)
{
  short k, handle;
  boolean ackmsg, had_no_bid, reject_it, hold_it;
  char mtyp;
  short sps;
  userstruct uf;
  bidtype bid;
  mbxtype mbx, umbx;
  char sname[256];
  char hw[256], toc[256];
  calltype fromcall;
  char hs[256], w[256];
  userstruct *WITH;
  char STR1[256], STR7[256];

  debug0(3, unr, 31);
  if (!(boxrange(unr) && user[unr]->sendheader != NULL))
    return;

  WITH = user[unr];
  ackmsg = (strstr(WITH->sendheader->betreff, "ACK:") == WITH->sendheader->betreff);

  had_no_bid = false;
  strcpy(bid, WITH->sendheader->id);
  if (!strcmp(bid, "#NONE#")) {
/*  if (WITH->f_bbs && !callsign(WITH->brett)) { */
    if (WITH->f_bbs && WITH->sendheader->msgtype != 'P') { /* change by DK2GO to allow "SP WP@XYZ" without BID */
      abort_sf(unr, false, "missing bulletin ID");
      return;
    }
    new_bid(bid);
    if (WITH->f_bbs)
      had_no_bid = true;
  }

  strcpy(sname, "001");
  if (WITH->f_bbs) {
    if (WITH->is_authentic) {
      if (had_no_bid)
	sprintf(sname, "%s&_%s%c%s",
		newmaildir, WITH->call, extsep, strcpy(STR7, sname));
      else
	sprintf(sname, "%s&%s%c%s",
		newmaildir, WITH->call, extsep, strcpy(STR1, sname));
    } else {
      if (had_no_bid)
	sprintf(sname, "%s%%_%s%c%s",
		newmaildir, WITH->call, extsep, strcpy(STR7, sname));
      else
	sprintf(sname, "%s%%%s%c%s",
		newmaildir, WITH->call, extsep, strcpy(STR1, sname));
    }
  } else {
    if (WITH->is_authentic)
      sprintf(sname, "%s&SENDING%c%s",
	      newmaildir, extsep, strcpy(STR1, sname));
    else
      sprintf(sname, "%s%%SENDING%c%s",
	      newmaildir, extsep, strcpy(STR1, sname));
  }
  validate(sname);
  WITH->sendchan = sfcreate(sname, FC_FILE);
  if (WITH->sendchan < minhandle) {
    WITH->sendchan = nohandle;
    debug(0, unr, 31, "cannot create file");
    if (WITH->f_bbs)
      abort_sf(unr, false, "file system error");
    else {
      WITH->action = 0;
      wln_btext(unr, 98);
    }
    return;
  }

  mtyp = WITH->sendheader->msgtype;
  strcpy(mbx, WITH->sendheader->verbreitung);
  strcpy(fromcall, WITH->sendheader->absender);

  handle = nohandle;
  k = strpos2(WITH->input2, "%", 1);
  if (k > 0) {
    sprintf(hs, "%c%c%c%c%c",
	    WITH->input2[k], WITH->input2[k + 1], WITH->input2[k + 2],
	    WITH->input2[k + 3], WITH->input2[k + 4]);
    del_leadblanks(hs);
    handle = atoi(hs);
  }


  create_status(0, true, WITH->sendheader->dest, *WITH->sendheader,
		WITH->input2);
  str2file(&WITH->sendchan, WITH->input2, true);
  str2file(&WITH->sendchan, WITH->sendheader->betreff, true);
  sprintf(hs, "*** Bulletin-ID: %s ***", bid);
  str2file(&WITH->sendchan, hs, true);

  str2file(&WITH->sendchan, "", true);
  if (WITH->f_bbs == false && ackmsg == false) {
    sps = 5;
    *toc = '\0';
    if (tpkbbs)
      sprintf(hs, "%s @ %s", WITH->call, default_tpkbbs);
    else
      sprintf(hs, "%s @ %s", WITH->call, ownhiername);
    upper(hs);
    strcpy(hw, "From:");
    rspacing(hw, sps);
    sprintf(hs, "%s %s", hw, strcpy(STR7, hs));
    if (*WITH->name != '\0')
      sprintf(hs + strlen(hs), " (%s)", WITH->name);
    cut(hs, 80);
    str2file(&WITH->sendchan, hs, true);

    k = strpos2(WITH->input2, "de:", 1);
    if (k > 1) {
      sprintf(hs, "%.*s", k - 1, WITH->input2);
      k = strpos2(hs, "@", 1);
      if (k > 0) {
	sprintf(toc, "%.*s", k - 1, hs);
	del_lastblanks(toc);   /* zur spaeteren Einfuegung des Namens */
	if (hs[k] != ' ')
	  strinsert(" ", hs, k + 1);
	if (hs[k - 2] != ' ')
	  strinsert(" ", hs, k);
      }
    } else
      *hs = '\0';
    upper(hs);
    strcpy(hw, "To:");
    rspacing(hw, sps);
    sprintf(hs, "%s %s", hw, strcpy(STR1, hs));
    str2file(&WITH->sendchan, hs, true);

    if (*WITH->mybbs != '\0' && strstr(WITH->mybbs, Console_call) != WITH->mybbs) {
      strcpy(w, WITH->mybbs);
      complete_hierarchical_adress(w);
      sprintf(hs, "%s @ %s", WITH->call, w);
      upper(hs);
      strcpy(hw, "Reply-To");
      rspacing(hw, sps);
      sprintf(hs, "%s: %s", hw, strcpy(STR7, hs));
      str2file(&WITH->sendchan, hs, true);
    }

    if (!WITH->is_authentic) {
      if (authentinfo) {
	sig_german_authinfo(WITH->call, mbx, hs);
	str2file(&WITH->sendchan, hs, true);
      }
    }

    str2file(&WITH->sendchan, "", true);

  }

  strcpy(WITH->input2, sname);

  unhpath(mbx, umbx);

  if (WITH->f_bbs && !strcmp(WITH->brett, "T")) {
    if (!strcmp(umbx, Console_call))
      tell_command(fromcall, bid, WITH->sendheader->betreff);
  }

  if (handle >= minhandle) {
    send_file1(unr, &handle, false, 0);
    strcpy(hs, "***END");
    send_text3(unr, false, hs, true);
    return;
  }
  if (WITH->f_bbs) {
    if (WITH->sf_level <= 2) {
      WITH->action = 7;
      return;
    }
    if (WITH->fwdmode > 0) {
      WITH->action = 28;
      set_packsf(unr);
    } else
      WITH->action = 28;
    return;
  }
  reject_it = false;
  hold_it = false;

  if (!strcmp(umbx, Console_call) && in_servers(WITH->brett))
    wlnuser(unr, "mailing list server");
  else {
    strcpy(hs, WITH->brett);
    check_verteiler(hs);
    switch_to_default_board(hs);
    *w = '\0';

    if (mtyp == '\0') {
      if (callsign(hs))
	mtyp = 'P';
      else
	mtyp = 'B';
    }

    /*local?*/
    do_convtit(w, mtyp, hs, mbx, WITH->call, w, WITH->sendheader->betreff,
	       bid, 50, true, WITH->is_authentic, &reject_it, &hold_it);

    if (!reject_it) {
      if (hold_it) {
	if (!WITH->is_authentic) {
	  if (holddelay > 0) {
	    w_btext(unr, 109);
	    sprintf(w, " %ld", holddelay / 3600);
	    wlnuser(unr, w);
	  }
	}
      }

      if (strcmp(WITH->brett, hs)) {
	w_btext(unr, 110);
	chwuser(unr, 32);
	wlnuser(unr, hs);
      }

    }


  }

  if (!reject_it) {
    w_btext(unr, 47);
    sprintf(hs, " %s @ %s", WITH->brett, mbx);
    if (callsign(WITH->brett)) {
      load_userfile(true, false, WITH->brett, &uf);
      if (*uf.name != '\0')
	sprintf(hs + strlen(hs), " (%s)", uf.name);
      if (uf.lastatime > 0 && !strcmp(umbx, Console_call)) {
	wlnuser(unr, hs);
	ix2string(uf.lastatime, hs);
	w_btext(unr, 54);
	sprintf(hs, " %s", strcpy(STR7, hs));
      } else if (uf.mybbsupd > 0 && *uf.mybbs != '\0') {
	wlnuser(unr, hs);
	w_btext(unr, 188);
	sprintf(hs, " %s ", uf.mybbs);
	wuser(unr, hs);
	w_btext(unr, 189);
	ix2string(uf.mybbsupd, hs);
	sprintf(hs, " %s", strcpy(STR7, hs));        
      }
    }
    wlnuser(unr, hs);

    wln_btext(unr, 48);
    wlnuser0(unr);
    WITH->action = 74;
    return;
  }


  WITH->action = 74;
  strcpy(w, "\030");
  send_text3(unr, true, w, true);


  /* reject */
}


static void lt_request(short unr)
{
  if (!boxrange(unr))
    return;
  w_btext(unr, 111);
  chwuser(unr, 32);
}


void enter_lifetime(short unr, char *eingabe)
{
  short k, handle;
  char hs[256];
  userstruct *WITH;

  if (!boxrange(unr) || user[unr]->sendheader == NULL)
    return;

  if (zahl(eingabe)) {
    user[unr]->errors = 0;
    user[unr]->sendheader->lifetime = atoi(eingabe);
    open_sendfile(unr);
    return;
  }

  if (strchr(eingabe, '\032') != NULL || strchr(eingabe, '\030') != NULL ||
      uspos(eingabe, "***END") > 0 ||
      useq(eingabe, "/EX")) {
    WITH = user[unr];
    k = strpos2(WITH->input2, "%", 1);   /* Import aktiv ? */
    if (k > 0) {
      sprintf(hs, "%c%c%c%c%c",
	      WITH->input2[k], WITH->input2[k + 1], WITH->input2[k + 2],
	      WITH->input2[k + 3], WITH->input2[k + 4]);
      del_leadblanks(hs);
      handle = atoi(hs);
      sfclose(&handle);
    }
    disp_sh(unr);
    WITH->action = 0;

    return;
  }
  wln_btext(unr, 39);
  lt_request(unr);
  user[unr]->errors++;
}


void box_txt2(boolean first, short unr, char *betreff1_)
{
  char		betreff1[256];
  short		k, handle;
  char		subject[256];
  char		hs[256];
  userstruct	*WITH;

  debug(3, unr, 32, betreff1_);

  strcpy(betreff1, betreff1_);
  if (!boxrange(unr)) {
    debug(0, unr, 32, "no such user!");
    return;
  }

  WITH		= user[unr];
  strcpy(subject, betreff1);
  del_blanks(subject);
  cut(subject, LEN_SUBJECT);

  if (WITH->f_bbs && !first && *subject == '\0')
    sprintf(subject, "<empty subject>");

  if (*subject == '\0') {
    if (WITH->f_bbs) {
      if (!first) {
	abort_sf(unr, true, "fatal error 34");
	return;
      }

      if (WITH->sf_level > 2)
	WITH->action	= 27;
      else
	WITH->action	= 6;
      if (WITH->fwdmode <= 0)
	return;

      WITH->binsfptr	= malloc(sizeof(binsftyp));
      if (WITH->binsfptr == NULL) {
	abort_sf(unr, false, "no binsfptr 1");
	return;
      }
      *WITH->binsfptr->fbbtitle		= '\0';
      *WITH->binsfptr->wname		= '\0';
      WITH->binsfptr->wchan		= nohandle;
      WITH->binsfptr->blockcounter	= BCMAGIC1; /* start flag */
      WITH->binsfptr->rxbytes		= 0;
      WITH->binsfptr->validbytes	= 0;
      WITH->binsfptr->fbbtitlelen	= 0;
      WITH->binsfptr->offset		= 0;
      boxsetrwmode(WITH->pchan, RW_FBB);
      return;
    }

    WITH->errors++;
    w_btext(unr, 50);
    WITH->action	= 73;
    return;
  }

  if (strchr(subject, '\032') != NULL || strchr(subject, '\030') != NULL ||
      uspos("***END", subject) > 0 ||
      !strcmp(subject, "/EX") ||
      *WITH->brett == '\0') {

    if (WITH->f_bbs) {
      write_msgid(-1, WITH->sendheader->id);
      abort_sf(unr, false, "invalid expression in <subject>");
      return;
    }

    WITH->errors	= 0;
    handle		= nohandle;
    k			= strpos2(WITH->input2, "%", 1);   /* Import aktiv ? */
    if (k > 0) {
      sprintf(hs, "%c%c%c%c%c",
	      WITH->input2[k], WITH->input2[k + 1], WITH->input2[k + 2],
	      WITH->input2[k + 3], WITH->input2[k + 4]);
      del_leadblanks(hs);
      handle		= atoi(hs);
      sfclose(&handle);
    }
    disp_sh(unr);
    WITH->action	= 0;
    return;

  }

  WITH->errors = 0;
  WITH->sp_input = 0;
  strcpy(WITH->sendheader->betreff, subject);
  if (!WITH->f_bbs && WITH->lt_required && WITH->sendheader->msgtype != 'P') {
    lt_request(unr);
    WITH->action	= 91;
  } else
    open_sendfile(unr);
}


static void send_text1(short unr, char *call1, char *mbx, char *call2,
		       char *lts, char *bid, char *betreff1, char mtype)
{
  short		handle;
  char		hs[256];
  indexstruct	*WITH;

  debug(3, unr, 33, call1);
  strcpy(user[unr]->brett, call1);

  disp_sh(unr);

  user[unr]->sendheader	= malloc(sizeof(indexstruct));

  if (user[unr]->sendheader == NULL) {
    user[unr]->action	= 0;
    return;
  }

  WITH			= user[unr]->sendheader;
  strcpy(WITH->id, bid);
  strcpy(WITH->verbreitung, mbx);
  strcpy(WITH->dest, call1);
  strcpy(WITH->absender, call2);
  WITH->lifetime	= atoi(lts);
  WITH->rxdate		= clock_.ixtime;
  WITH->msgtype		= mtype;
  WITH->size		= 999000L;
  if (*user[unr]->input2 != '\0') {
    handle		= sfopen(user[unr]->input2, FO_READ);
    if (handle >= minhandle) {
      sprintf(hs, "%d", handle);
      lspacing(hs, 5);
      sprintf(user[unr]->input2, "%%%s ", hs);
    } else
      *user[unr]->input2 = '\0';
  }

  box_txt2(true, unr, betreff1);
}


static void check_acc(short unr, char *b, unsigned short *ac)
{
  unsigned short	mlt;

  mlt		= 1;
  check_lt_acc(b, &mlt, ac);
  if (strlen(b) == 1) {
  
    if (!user[unr]->supervisor)
      *ac	= 1000;
  }
}


static boolean testtosys(char *call)
{
  userstruct	rec;

  if (!strcmp(call, Console_call))
    return true;
  else {
    load_userfile(true, false, call, &rec);
    return (rec.pwmode >= 10);
  }
}


void send_check(short unr, char *eingabe, boolean is_user, char msgtype)
{
  unsigned short	acc;
  boolean		c1isc, errok, ok, db, hadnolt;
  char			hs[256], hs2[256];
  boardtype		brett1, call1;
  calltype		call2;
  char			lt[256];
  mbxtype		mbx;
  bidtype		bid;
  char			mtyp = 'P';
  char			betreff[256];
  mbxtype		hmbx, hmbx2;
  userstruct		*WITH;

  debug(3, unr, 34, eingabe);
  if (!boxrange(unr))
    return;

  WITH			= user[unr];
  *WITH->tempbid	= '\0';
  WITH->lt_required	= false;

  strcpy(hs, eingabe);
  split_sline(eingabe, call1, call2, mbx, bid, lt, betreff);
  if (!strcmp(lt, "-")) {
    hadnolt		= true;
    strcpy(lt, "0");
  } else
    hadnolt		= false;
  if (is_user)
    upper(bid);

  if (is_user && !(WITH->supervisor || WITH->rsysop))
    strcpy(call2, WITH->call);
  else if (*call2 == '\0')
    strcpy(call2, WITH->call);
  if (strchr(call1, '+') != NULL && !WITH->f_bbs)
    *call1		= '\0';
  if (hs[0] == '\032' || (!WITH->f_bbs && useq(hs, "***END"))) {  /*Abbruch*/
    if (!WITH->f_bbs) {
      WITH->action	= 0;
      return;
    }

    if (WITH->sf_level == 1 && WITH->sf_master)
      wlnuser(unr, "F>");
    else {
      chwuser(unr, '>');
      wlnuser0(unr);
    }
    WITH->action	= 5;
    return;
  }

  if (*call1 == '\0') {

    if (!WITH->f_bbs) {
      w_btext(unr, 51);
      WITH->action	= 72;
      return;
    }

    /* eigentlich quatsch, hier sollte nie was ankommen... */
    if (WITH->sf_level > 2) {
      abort_sf(unr, false, "*** invalid destination");
      return;
    }

    chwuser(unr, '>');
    wlnuser0(unr);
    WITH->action	= 5;
    return;
  }

  c1isc			= callsign(call1);
  acc			= 1;
  errok			= false;
  if (is_user) {
    if (!c1isc) {
      WITH->lt_required = (hadnolt && request_lt);
      *hs		= '\0';
      ok		= false;

      if (msgtype == '\0') {
	if (c1isc)
	  mtyp		= 'P';
	else
	  mtyp		= 'B';
      } else
	mtyp		= msgtype;

      check_reject(hs, mtyp, call1, mbx, call2, hs, hs, bid, 0, false, false, &ok, &db, hs2);

      ok		= !ok;

      if (ok && valid_boardname(call1)) {
	strcpy(brett1, call1);
	check_acc(unr, brett1, &acc);
	if (WITH->level >= acc || may_sysaccess(unr, brett1)) { /* fuer die rsysops */
	  check_verteiler(brett1);
	  check_acc(unr, brett1, &acc);
	  if (WITH->level >= acc || may_sysaccess(unr, brett1)) {
	    switch_to_default_board(brett1);
	    check_acc(unr, brett1, &acc);
	    if (WITH->level >= acc || may_sysaccess(unr, brett1))
	      acc	= 1;
	  }
	}
      } else
	acc		= SHORT_MAX;
      if (WITH->level < acc) {
	wln_btext(unr, 7);
	errok		= true;   /* 'Fehlermeldung bereits ausgegeben' */
      }
    }
  }

  if (!is_user || (is_user && (WITH->se_ok || testtosys(call1)) && !errok)) {
    unhpath(mbx, hmbx);

    if (*hmbx == '\0' || (!strcmp(hmbx, Console_call) && WITH->f_bbs)) {
      if (c1isc) {
	user_mybbs(call1, mbx);
	unhpath(mbx, hmbx2);

	if (is_user && *mbx == '\0') {
	  WITH->action	= 0;
	  wln_btext(unr, 11);
	  wln_btext(unr, 112);
	  wln_btext(unr, 113);
	  return;
	}

	if (*mbx == '\0')
	  strcpy(mbx, Console_call);
	else if (WITH->f_bbs && strcmp(hmbx2, Console_call)) {
	  if (*bid != '\0')   /* alte BID sichern und nachher wegschreiben */
	    strcpy(WITH->tempbid, bid);
	  new_bid(bid); /* bei neuem MyBBS muss! eine neue BID vergeben werden */
	}
	if (is_user && *bid != '\0' && strcmp(bid, "#NONE#")) {
	  if (!check_double(bid)) {  /* BID schon vorhanden */
	    WITH->action	= 0;
	    wln_btext(unr, 2);
	    wln_btext(unr, 114);
	    wln_btext(unr, 11);
	    return;
	  }
	}

      } else
	strcpy(mbx, Console_call);

    } else if (c1isc && !callsign(hmbx) && !WITH->f_bbs)
      strcpy(mbx, Console_call);


    if (WITH->f_bbs && !strcmp(call1, "T")) {
      if (!strcmp(hmbx, Console_call))
	strcpy(lt, "1");
      else
	strcpy(lt, "7");
    }

    if (is_user || (strchr(mbx, '.') == NULL && callsign(mbx)))
      complete_hierarchical_adress(mbx);

    if (*mbx == '\0')
      strcpy(mbx, Console_call);

    if (is_user) {
      if (!gen_sftest2(unr, call1, mbx)) {
	WITH->action	= 0;
	wln_btext(unr, 11);
	return;
      }
      if (c1isc || mtyp == 'P') /* bei SP letztes Brett in tempbid sichern */
        strcpy(WITH->tempbid, WITH->brett); /* sichern	      */
    }

    send_text1(unr, call1, mbx, call2, lt, bid, betreff, msgtype);
    return;
  }

  WITH->action		= 0;
  if (!errok)
    wln_btext(unr, 65);
  /* wenn SEND und nicht privilegiert */
}


void send_file0(short unr, boolean in_send, char *fname)
{
  short kanal;
  boolean ok;
  char pfad[256], name[256];

  if (!boxrange(unr))
    return;
  sprintf(pfad, "%s%c%c%c", boxdir, allquant, extsep, allquant);
  *name = '\0';
  ok = false;

  if (*fname == '\0') {
    if (user[unr]->console) {
      if (select_file(unr, pfad, name, "import file"))
	ok = true;
    }
  } else {
    strcpy(name, fname);
    ok = true;
  }

  if (ok)
    ok = exist(name);

  if (!ok) {
    wln_btext(unr, 4);
    return;
  }
  if (in_send) {
    kanal = sfopen(name, FO_READ);
    del_path(name);
    strcpy(user[unr]->tempbinname, name);
    if (kanal >= minhandle) {
      if (send_file1(unr, &kanal, true, 0))  /* War ein Binaerfile */
	send_text3(unr, false, "***END", true);
    }
    return;
  }
  strcpy(user[unr]->input2, name);
  del_path(name);
  strcpy(user[unr]->tempbinname, name);
  *name = '\0';
  send_check(unr, name, false, '\0');
}


void send_sysmsg(char *tocall, char *tobbs_, char *betreff_, char *msg_, short lt, char msgtype, short binmsg)
/* binmsg == 1 -> keep as ascii text, binmsg == 2 -> keep as BIN, binmsg == 0 -> check type */
{
  char tobbs[256], betreff[256], msg[256];
  short unr, kanal;
  char hs[256];
  char w[256];
  char STR1[256];

  strcpy(tobbs, tobbs_);
  strcpy(betreff, betreff_);
  strcpy(msg, msg_);
  debug(3, -1, 35, tocall);
  unr = melde_user_an(Console_call, 0, 0, UM_SYSREQ, false);
  if (unr <= 0) {
    debug(0, -1, 35, "failed. no syslogin");
    return;
  }
  if (*tobbs != '\0')
    sprintf(tobbs, " @ %s", strcpy(STR1, tobbs));
  if (*betreff == '\0')
    strcpy(betreff, "Msg from SYSTEM");
  sprintf(hs, "%s%s < %s", tocall, tobbs, Console_call);
  if (lt > 0) {
    sprintf(w, "%d", lt);
    sprintf(hs + strlen(hs), " #%s", w);
  }
  sprintf(hs + strlen(hs), " %s", betreff);
  send_check(unr, hs, false, msgtype);

  if (binmsg == 1 || binmsg == 2) {
    if (exist(msg)) {
      kanal = sfopen(msg, FO_READ);
      del_path(msg);
      strcpy(user[unr]->tempbinname, msg);
      if (kanal >= minhandle)
	send_file1(unr, &kanal, true, binmsg);
      else
	send_text3(unr, false, "*** read error", true);
    }
  } else {
    *hs = '\0';
    send_text3(unr, false, hs, true);
    send_text3(unr, false, msg, true);
  }

  send_text3(unr, false, "***END", true);
  melde_user_ab(unr, false);
}
