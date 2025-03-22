/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/

/* ** Revision Information *********************************
 * Last modified :   96/11/11 at 15:22:00
 * Revision      :   1.6
 * Filename      :   box_sub.c
 *
 * History : C.Porfiri    961110 Validated Unproto, found
 *                               the crash problem when
 *                               sending headers, implemented
 *                               the old message retrieval
 *                               but not tested yet.
 *
 * J. Schurig 961111 switched off newline flag in
 *                   boxsendpline() call. that was the
 *                   reason for the buffer overflow in main.c.
 *                   several changes concerning newline character
 *                   in unproto transmission. added security
 *                   exit in private_mail_exist() and check if
 *                   mail is deleted, changed call comparision
 *                   from header.obrett to header.brett.
 *                   we still have a problem with the port/path
 *                   check in unproto_request1(). It has to
 *                   be checked... in Germany, often Flexnet
 *                   digipeaters are used, they allow L2 transmissions
 *                   from every digipeater in the net to any other...
 *                   don't want a gambler requesting via 500 km.
 *                   BTW we have an inconsistency in handling private mails
 *                   in unproto output: if private is OFF in unproto defs,
 *                   they are blanked with # in normal list output, but
 *                   sent in private_mail_exist(). as I think it is useful
 *                   to send them when the user itself requests, I didn't
 *                   inhibit the transmission.
 * *********************************************************
 */

#define BOX_SUB_G
#include <ctype.h>
#include "main.h"
#include "box_sub.h"
#include "crc.h"
#include "box_logs.h"
#include "tools.h"
#include "status.h"
#include "box_mem.h"
#include "box_send.h"
#include "box_inou.h"
#include "box_file.h"
#include "shell.h"

/* the maximum frame length is 256 */
/* #define	MAX_UNPROTO_PACKET_SIZE	256	// This is now defined in boxlocal.h */


static void send_unproto_line(char *path, char *port, boolean fbb,
			      boolean dpbox, char *line)
{
  short iface, chan;
  char hs[256];

  if (!fbb && !dpbox)
    return;
  iface = -1;   /* wichtig */
  if (!find_socket(port, &iface))
    return;
  chan = boxgettncmonchan(0);
  if (fbb) {
    sprintf(hs, "FBB %s", path);
    del_lastblanks(hs);
    boxsetunproto(chan, hs, iface, port);
    boxsendpline(chan, line, false, iface); /* don't add CR */
  }
  if (!dpbox)
    return;
  sprintf(hs, "DPBOX %s", path);
  del_lastblanks(hs);
  boxsetunproto(chan, hs, iface, port);
  boxsendpline(chan, line, false, iface); /* don't add CR */
}

static void print_messageheader(boolean fbb, boxlogstruct header, char *buf)
{
  char w[256], w1[256], w2[256], w3[256];
  
  ixdatum2string(header.date, w);
    
  if (fbb) {
/*
12345 B   2053 TEST  @ALL    F6FBB  920325 This is the subject
                             ^^^^^^ six chars, fill up with spaces
                      ^^^^^^ six chars, fill up with spaces
               ^^^^^^ six chars, fill up with spaces
        ^^^^^^ six chars, fill up with spaces
*/
    strcpy(w1, header.obrett);
    rspacing(w1, 6);
    cut(w1, 6);
    sprintf(w2, "%ld", header.size);
    lspacing(w2, 6);
    unhpath(header.verbreitung, w3);
    rspacing(w3, 6);
    cut(w3, 6);
    sprintf(buf, "%ld %c %s %s@%s %-6s %c%c%c%c%c%c %s",
                 header.msgnum + MSGNUMOFFSET, header.msgtype,
    		 w2, w1, w3, header.absender,
    		 w[6], w[7], w[3], w[4], w[0], w[1], header.betreff);
    cut(buf, 79);
    strcat(buf, "\r"); /* we need the CR */
  } else {
/*
12345 12345_DL8HBS 365 B 2053 MODIFICA@ALL F6FBB 920325 This is the subject
                              ^^^^^^^^ up to 8 chars, no spaces appended
*/
    unhpath(header.verbreitung, w3);
    cut(w3, 6);
    sprintf(buf, "%ld %s %d %c %ld %s@%s %s %c%c%c%c%c%c %s",
    		  header.msgnum + MSGNUMOFFSET, header.bid, header.lifetime, header.msgtype,
    		  header.size, header.obrett, w3, header.absender,
    		  w[6], w[7], w[3], w[4], w[0], w[1], header.betreff);
    cut(buf, 79);
    strcat(buf, "\r"); /* we need the CR */
  }
}


/* signalise end of unproto list after a timeout (called after new messages arrived) */
/* (but only if we ever got a user request before) */
void send_final_unproto()
{
  unprotoportstype *hp;
  char hs[256];
  
  if (!unproto_list)
    return;
  if (unprotodefroot == NULL)
    return;

  hp = unprotodefroot->ports;
  sprintf(hs, "%ld !!\r", unproto_last + MSGNUMOFFSET);

  while (hp != NULL) {
    if (!hp->RequestActive)
      send_unproto_line(hp->path, hp->port, unprotodefroot->fbb, unprotodefroot->dpbox, hs);
    hp = hp->next;
  }
  
  unproto_final_update = 0;

}


/* this function gets called every time a new message was received */
void send_new_unproto_header(boxlogstruct header)
{
  long ct;
  unprotoportstype *hp;
  char hs[256], buff[MAX_UNPROTO_PACKET_SIZE+1], buff2[MAX_UNPROTO_PACKET_SIZE+1];
  
  debug0(3, -1, 165);

  if (!unproto_list)
    return;
  if (unprotodefroot == NULL)
    return;
  if (header.deleted)
    return;
  if (!unprotodefroot->priv && header.msgtype != 'B')
    return;
  if (header.msgtype != 'B') {
    unhpath(header.verbreitung, hs);
    if (strcmp(hs, Console_call))
      return;
  }
  if (!unprotodefroot->sys && strlen(header.obrett) == 1)
    return;

  hp = unprotodefroot->ports;

  while (hp != NULL) {
    if (!hp->RequestActive) {

      *buff = '\0';
      
      hp->CurrentSendPos = unproto_last; /* we may have several hp's */
      if (hp->CurrentSendPos > header.msgnum)
        hp->CurrentSendPos = header.msgnum;

      if (header.msgnum > hp->CurrentSendPos + 1) {
        if (header.msgnum - hp->CurrentSendPos > 200) /* really out of sync ? */
          hp->CurrentSendPos = header.msgnum - 200;
        for (ct = hp->CurrentSendPos + 1; ct < header.msgnum; ct++) {
	  if (ct % 50 == 0)
	    dp_watchdog(2, 4711);
	  sprintf(hs, "%ld #\r", ct + MSGNUMOFFSET);
	  if (strlen(buff) + strlen(hs) > MAX_UNPROTO_PACKET_SIZE) {
	    send_unproto_line(hp->path, hp->port, unprotodefroot->fbb, unprotodefroot->dpbox, buff);
	    *buff = '\0';
	  }
	  strcat(buff, hs);
        }
      }

      strcpy(buff2, buff);

      if (unprotodefroot->fbb) {
        print_messageheader(true, header, hs);
	if (strlen(buff) + strlen(hs) > MAX_UNPROTO_PACKET_SIZE) {
	  send_unproto_line(hp->path, hp->port, true, false, buff);
	  *buff = '\0';
	}
	strcat(buff, hs);
        send_unproto_line(hp->path, hp->port, true, false, buff);
      }

      if (unprotodefroot->dpbox) {
        print_messageheader(false, header, hs);
	if (strlen(buff2) + strlen(hs) > MAX_UNPROTO_PACKET_SIZE) {
	  send_unproto_line(hp->path, hp->port, false, true, buff2);
	  *buff2 = '\0';
	}
	strcat(buff2, hs);
        send_unproto_line(hp->path, hp->port, false, true, buff2);
      }
      
      hp->CurrentSendPos = header.msgnum;
      hp->LastTxTime = clock_.ixtime;
    }
    
    hp = hp->next;
  }

  unproto_last = header.msgnum;
  if (request_count != 0) /* if we ever got an unproto-user-request we will transmit */
    unproto_final_update = clock_.ixtime; /* a final flag after a timeout */
}

/*
 ****************************************************
 * Function : send_unproto_header_packed            *
 * ------------------------------------------------ * 
 * Params.  : unprotoportstype *hp, valid pointer   *
 *            short handle, an opened boxlog handler*
 *            long nr, the first MSG header to send *
 * ------------------------------------------------ * 
 * Descr.   : sends as much MSG headers as fit in   *
 *            a packet (less than 257 bytes)        *
 * ------------------------------------------------ *
 * Written by iw0fbb Claudio Porfiri 961104         *
 *                                                  *
 * V0.1 961104 - first draft                        *
 * modified 961107 J.Schurig                        *
 ****************************************************
 */ 
 
static void send_unproto_header_packed(unprotoportstype *hp, short handle, long nr)
{
  boxlogstruct header;                /* buffer for checklist data  */
  long ct;                            /* Counter for empty messages */
  char hs[256];                       /* buffers                    */
  char packed_h[MAX_UNPROTO_PACKET_SIZE+1]; /* The actual sent line */
  boolean invalid;                    /* hidden line                */
  boolean exit = false;               /* flag for the main loop     */
  
  debug0(5, -1, 166);

  /* Let's clean the buffer or add an existing fragment (alignment message) */

  strcpy(packed_h, hp->buff);
  *hp->buff = '\0';
  
  /* Main loop, it goes on until the exit condition is verified */

  while (exit == false) {
    header.msgnum = -167;                   /* initiate header for reading */
    read_log(handle, nr++, &header);          /* read the nr-th MSG header */
 
    if (header.msgnum <= 0)
      exit = true;

    else {
                            
      if (hp->CurrentSendPos > header.msgnum)
        hp->CurrentSendPos = header.msgnum;
                  
      /* We assume header.msgnum is greater than hp->CurrentSendPos +1
         when the messages between them have been removed, thus we
         replace the missing messages with "<nnnn> #" lines           */
         
      if (header.msgnum > hp->CurrentSendPos + 1) {
    /* newer gcc versions don't like modified for - loops. Changed to a while loop instead. */
        ct = hp->CurrentSendPos + 1;
        while (ct < header.msgnum) {
          sprintf(hs, "%ld #\r", ct + MSGNUMOFFSET);
          /* not enough buffer space? then flush it. */
          if (strlen(packed_h) + strlen(hs) > MAX_UNPROTO_PACKET_SIZE) {
            send_unproto_line(hp->path, hp->port, !hp->ReqDPBOX, hp->ReqDPBOX, packed_h);
            packed_h[0] = '\0';
          /* Set the exit conditions */
            header.msgnum = ct - 1; /* we will not send the actual nuber */
            ct = header.msgnum;
            exit = true;
	  } else {
            strcat(packed_h, hs);
            ct++;
          }
        } /* For all removed messages */
      } /* If there are removed messages */

      if (!exit) {

        /* Let's check if we can send the whole header or
           we must only send the "<nnnn> #" string       */

        /* never send deleted headers */
        invalid = header.deleted 
        /* do not send private mails if not enabled in unproto.box */
                  || (!unprotodefroot->priv && header.msgtype != 'B');
        
        /* if privates are enabled, only send those for local users, not
           throughpassing forward */
        if (!invalid && header.msgtype != 'B') {
          unhpath(header.verbreitung, hs);
          invalid = strcmp(hs, Console_call);
        }
        
        /* one-letter-boards in dpbox are sysop-only boards. only
           send them if enabled in unproto.box */
        if (!invalid)
          invalid = (!unprotodefroot->sys && strlen(header.obrett) == 1);


        if (invalid) {
          /* ok, send the "<nnnn> #", the header is not valid for transmission */    
          sprintf(hs, "%ld #\r", header.msgnum + MSGNUMOFFSET);
          /* check if the buffer would overflow */
          if (strlen(packed_h) + strlen(hs) > MAX_UNPROTO_PACKET_SIZE) {
            /* yes, flush it */
            send_unproto_line(hp->path, hp->port, !hp->ReqDPBOX, hp->ReqDPBOX, packed_h);
            packed_h[0] = '\0';
            /* set exit conditions */
            header.msgnum--; /* we didn't send that msgnum */
            exit = true;
          } else
            strcat(packed_h, hs);
        
        } else {
          /* finally we have a valid header. convert it in the required list format */
          print_messageheader(!hp->ReqDPBOX, header, hs);
          /* check if the buffer would overflow */
          if (strlen(packed_h) + strlen(hs) > MAX_UNPROTO_PACKET_SIZE) {
            /* yes, flush it */
            send_unproto_line(hp->path, hp->port, !hp->ReqDPBOX, hp->ReqDPBOX, packed_h);
            packed_h[0] = '\0';
            /* set exit conditions */
            header.msgnum--; /* we didn't send that msgnum */
            exit = true;
          } else
            strcat(packed_h, hs);
        }
                  
        hp->CurrentSendPos = header.msgnum;
          
        /* Let's try to send some more headers */

        if (hp->CurrentSendPos >= unproto_last) /* check for unproto_last */
          exit = true;
          
      } /* if (!exit) */
      
      hp->CurrentSendPos = header.msgnum; /* align the pointer with the last sent number */
 
      /* Let's try to send some more headers */

      if (!exit && hp->CurrentSendPos >= unproto_last) /* check for unproto_last */
        exit = true;
        
    } /* else */
    
  } /* while (!exit) */
  
  if (unproto_last == header.msgnum) { /* check for unproto_last */
    sprintf(hs, "%ld !!\r", header.msgnum + MSGNUMOFFSET);
    /* check if buffer would overflow */
    if (strlen(packed_h) + strlen(hs) > MAX_UNPROTO_PACKET_SIZE) {
      /* yes, flush it */
      send_unproto_line(hp->path, hp->port, !hp->ReqDPBOX, hp->ReqDPBOX, packed_h);
      packed_h[0] = '\0';
    } /* then send a next frame with the end-of-list - message */
    strcat(packed_h, hs);
    send_unproto_line(hp->path, hp->port, !hp->ReqDPBOX, hp->ReqDPBOX, packed_h);
    packed_h[0] = '\0';
    hp->RequestActive = false;
    hp->CurrentSendPos = unproto_last;
  }

  hp->LastTxTime = clock_.ixtime;
}

/*
 ****************************************************
 * Function : private_mail_exist                    *
 * ------------------------------------------------ *
 * Params.  : long From, first message to scan      *
 *            long To,   last message to scan       *
 *            char *User, user asking resync        *
 *            unprotoportstype *hp, valid pointer   *
 * Returns  : boolean, true if a mail exists        *
 *                     false otherwise || error     *
 * ------------------------------------------------ *
 * Descr.   : this function scans the message dbase *
 *            starting from the message number      *
 *            requested from the user up to the     *
 *            first message handled in normal ULIST *
 *            mode. If a private mail exists, the   *
 *            user will receive resync for the MSG  *
 *            immediately before and THE ONLY priv. *
 *            mail header will be sent.             *
 * ------------------------------------------------ *
 * Written by iw0fbb Claudio Porfiri 961107         *
 *                                                  *
 * V0.1 961107 - first draft                        *
 * 961111 J. Schurig   added exit if msglist empty  *
 *                     or damaged. changed cpos     *
 *                     calculation to a less cpu    *
 *                     intensive loop               *
 * 961113 J. Schurig   changed improper call com-   *
 *                     parison (did not work with   *
 *                     calls < 6 chars length)      *
 ****************************************************
 */

boolean private_mail_exist(long From, long To, char *User, unprotoportstype *hp)
{
  short k;
  long cpos;
  boxlogstruct header;
  boolean RetValue = false;
  char hs[256];
  char packed_hs[257];


  if (hp == NULL)
    return false;

  cpos = msgnum2centry(From, &header, true);
  if (cpos < 0)
    return false;
    
  k = sfopen(boxlog, FO_READ);
  if (k < minhandle)
      return false;

  header.msgnum = 1; /* to let the loop begin */
  
  /* For all old messages existing into the database */
  while ((header.msgnum > 0) && (header.msgnum <= To) && (RetValue == false)) {
  
      header.msgnum = -167;               /* initiate ibuf for reading */
      read_log(k, cpos++, &header);       /* read the nrth MSG header */

      /* If it's a valid header then check it */
      if (header.msgnum > 0){

          if (!header.deleted) {
            /* is this message for CALL? if so resync and send it */
            if (!strcmp(User, header.brett))
            {
                /* Prepare the resync string */
                sprintf(packed_hs, "%ld ! %s\r", (header.msgnum - 1) + MSGNUMOFFSET, User);
                /* Format the MSg header */
                print_messageheader(!hp->ReqDPBOX, header, hs);
                /* Build and send the unproto lines */
                strcat(packed_hs, hs);
                send_unproto_line(hp->path, hp->port, !hp->ReqDPBOX,
                                  hp->ReqDPBOX, packed_hs);
                RetValue = true;
            }
          }
      }

  } /* for all old messages */
  sfclose(&k);

  return RetValue;
}


void send_requested_unproto(unprotoportstype *hp)
{
  short k;
  long cpos, maxc;
  boxlogstruct log;

  if (hp == NULL)
    return;
  if (!hp->RequestActive)
    return;
  if (clock_.ixtime - hp->LastTxTime < unprotodefroot->TxInterval)
    return;

  if (hp->CurrentSendPos + 1 > unproto_last) {
    hp->RequestActive = false;
    hp->CurrentSendPos = unproto_last;
    return;
  }

  cpos = msgnum2centry(hp->CurrentSendPos + 1, &log, true);
  maxc = sfsize(boxlog) / sizeof(boxlogstruct);
  
  if (maxc - cpos <= 0) {
    hp->RequestActive = false;
    hp->CurrentSendPos = unproto_last;
    return;
  }

  k = sfopen(boxlog, FO_READ);
  if (k < minhandle) {
    hp->RequestActive = false;
    hp->CurrentSendPos = unproto_last;
    return;
  }
  
  send_unproto_header_packed(hp, k, cpos);

  sfclose(&k);

}


/* Die Verwaltung fuer Unproto-Requests:                                 */
/*                                                                       */
/* CALL ist das Absender-Call des Requesters, kann auch mit SSID sein    */
/* PORT ist der TNC-Port, auf dem der Request hereinkam                  */
/* PATH ist das dritte bis n-te call aus dem ax25-header,                */
/*      kann also auch leer sein                                         */
/* S    ist die empfangene Unproto-Zeile                                 */

void unproto_request1(char *call, char *port, char *path, char *s)
{
  boolean ok;
  userstruct ufil;
  long mnum;
  short x;
  unprotoportstype *hp;
  boolean dpboxmode, align;
  long cs, cs1;
  char hs[256], w[256], w1[256];
  char STR1[256];

  if (unprotodefroot == NULL)
    return;

  x = strlen(s);
  if (x < 12 || x > 13)
    return;

  strcpy(hs, path);
      /* senden wir ueberhaupt ueber diesen pfad? */
  get_word(hs, w);
  upper(w);
  ok = false;
  hp = unprotodefroot->ports;
  while (hp != NULL && !ok) {
    /* believe this one is right. */
    if (!strcmp(port, hp->port)) {
      /* we will have to check that. */
/*      if (*hp->path != '\0')
        strcpy(hs, hp->path);
      else
        strcpy(hs,w); */
      strcpy(hs, hp->path);
      *w1 = '\0';
      while (*hs != '\0')
        get_word(hs, w1);
      ok = (strcmp(w, w1) == 0);
    }
    if (!ok)
      hp = hp->next;
  }
  if (!ok)   /* nein -> und tschuess */
    return;
  
  /* removed the polling control. bbs controls transmission timed, so
     users can poll as often as they want */
/*  if (clock_.ixtime - hp->LastReqTime < unprotodefroot->PollInterval)
    return; */

  strcpy(hs, s);   /* nachfragemodus pruefen */
  get_word(hs, w);
  dpboxmode = (strcmp(w, "??") == 0);
  if (!dpboxmode && strcmp(w, "?"))
    return;
  if (dpboxmode && !unprotodefroot->dpbox)
    return;
  if (!dpboxmode && !unprotodefroot->fbb)
    return;
  hp->ReqDPBOX = dpboxmode;

  get_word(hs, w);   /* msgnum und checksumme pruefen */
  if (strlen(w) != 10)
    return;
  cs = hatoi(strsub(STR1, w, 9, 2));
  sprintf(STR1, "%.8s", w);
  mnum = hatoi(STR1);
  if (mnum < 1)
    return;
  mnum = mnum - MSGNUMOFFSET;
  if (mnum < 1) {
    mnum = 1;
    align = true;
  } else
    align = false;
    
  if (mnum < firstmsgnum) {
    mnum = firstmsgnum;
    align = true;
  }
  
  cs1 = 0;
  for (x = 0; x <= 3; x++)
    cs1 = (cs1 + hatoi(strsub(STR1, w, x * 2 + 1, 2))) & 0xff;
  if (cs1 != cs) {
    boxprotokoll("invalid unproto checksum");
    return;
  }
  request_count++; /* to make sure that we will send unproto-last-marks from now on */
  last_requesttime = clock_.ixtime;

  ok = allunproto;   /* ist dieser user zur nachfrage zugelassen? */
  if (!ok) {
    del_callextender(call, hs);
    load_userfile(false, true, hs, &ufil);
    ok = ufil.unproto_ok;
  }

  if (!ok) {  /* nein -> fehlermeldung auf entsprechendem pfad ausgeben */
    sprintf(w, "%ld / %s\r", mnum, call);
    send_unproto_line(hp->path, hp->port, !dpboxmode,
		      dpboxmode, w);
    return;
  }

    /* If the requested number is below the threshold, it's still possible
     that unread user files exist into the database, in this case they
     will be sent one at a time, the unproto list is stopped until the
     user knows all about its own messages                              */
  if (unproto_last - mnum > unprotodefroot->maxback) {
      /* OK, the user is unaligned, let's look for private mails */
      if (private_mail_exist(mnum, (unproto_last - unprotodefroot->maxback),
                             call, hp)) {
          hp->LastReqTime = clock_.ixtime;
          hp->LastTxTime = clock_.ixtime;
          hp->CurrentSendPos = unproto_last;
          hp->RequestActive = false;
          return;
      }
      else {
          mnum = unproto_last - unprotodefroot->maxback;
          align = true;
      }

  }
  
  *hp->buff = '\0';
  
  if (align)
    sprintf(hp->buff, "%ld ! %s\r", mnum + MSGNUMOFFSET, call);
    /* we will send that in one frame with the remaining answer */
  
  if (mnum >= unproto_last) { /* Last message reached. */
    sprintf(hs, "%ld !!\r", unproto_last + MSGNUMOFFSET);
    send_unproto_line(hp->path, hp->port, !dpboxmode,
		      dpboxmode, hs);
    hp->LastReqTime = clock_.ixtime;
    hp->LastTxTime = clock_.ixtime;
    hp->CurrentSendPos = unproto_last;
    hp->RequestActive = false; /* switch it off, may have interrupted a running request... */
    return;
  }
  
  hp->CurrentSendPos = mnum;
  hp->LastReqTime = clock_.ixtime;
  hp->RequestActive = true;
  
  send_requested_unproto(hp);
}


void clear_msgnumarr(void)
{
  short x;

  for (x = 0; x <= MAXMSGNUMARR; x++) {
    msgnumarr[x].msgnum = 0;
    msgnumarr[x].cpos = 0;
  }
  firstmsgnum = 0;
  msgnumarrct = 0;
  msgnumarrblock = 0;
}


void fill_msgnumarr(void)
{
  short k;
  long bsize, bct, mct, cpos, cct;
  boxlogstruct log;

  clear_msgnumarr();
  debug0(3, -1, 162);

  bsize = sfsize(boxlog);
  if (bsize <= 0)
    return;
  cct = bsize / sizeof(boxlogstruct);
  bct = (cct / MAXMSGNUMARR) + 1;

  k = sfopen(boxlog, FO_READ);
  if (k < minhandle)
    return;

  mct = 0;
  cpos = 0;

  do {

    log.msgnum = -176;
    read_log(k, cpos + 1, &log);
    if (log.msgnum == -176) {
      sfclose(&k);
      debug(0, -1, 162, "read error");
      return;
    }

    if (mct == 0)
      firstmsgnum = log.msgnum;
    msgnumarr[mct].msgnum = log.msgnum;
    msgnumarr[mct].cpos = cpos;

    mct++;
    cpos += bct;

  } while (mct <= MAXMSGNUMARR && cpos < cct);

  sfclose(&k);

  msgnumarrct = mct - 1;
  msgnumarrblock = bct;
}


static long msgnum2cposstart(long msgnum, long *end)
{
  short x;

  debug0(4, -1, 161);
  if (msgnumarrct <= 0) fill_msgnumarr();
  *end = -1;
  if (msgnum > actmsgnum)
    return -1;
  if (msgnumarrct == 1)
    return 0;

  x = 0;
  while (x <= msgnumarrct && msgnumarr[x].msgnum <= msgnum) x++;
  if (x > 0) x--;

  if (x < msgnumarrct)
    *end = msgnumarr[x+1].cpos;

  return (msgnumarr[x].cpos);
}


long msgnum2centry(long msgnum, boxlogstruct *log, boolean fuzzy)
{
  long start, seekp, end;
  short k;

  start = msgnum2cposstart(msgnum, &end);
  if (start < 0)
    return -1;

  k = sfopen(boxlog, FO_READ);
  if (k < minhandle)
    return -1;

  if (end < 0)
    end = sfseek(0, k, SFSEEKEND) / sizeof(boxlogstruct);
  
  seekp = start * sizeof(boxlogstruct);
  if (sfseek(seekp, k, SFSEEKSET) != seekp) {
    sfclose(&k);
    return -1;
  }
  
  while (start <= end
    		&& sfread(k, sizeof(boxlogstruct), (char *)log) == sizeof(boxlogstruct)
  		&& log->msgnum < msgnum) start++;
  
  seekp = sfseek(0, k, SFSEEKCUR);
  sfclose(&k);

  /* if "fuzzy" == true return the position of the first valid entry found */
  if (log->msgnum == msgnum || (fuzzy && log->msgnum >= msgnum))
    return (seekp / sizeof(boxlogstruct));
  return -1;
}


short msgnum2board(long msgnum, char *board)
{
  boxlogstruct log;

  if (msgnum2centry(msgnum, &log, false) > 0) {
    strcpy(board, log.brett);
    return (log.idxnr);
  } else {
    *board = '\0';
    return -1;
  }
}

/***************** end of msgnum - stuff *************************/

short actual_connects(void)
{
  short x, z;

  z = 0;
  for (x = 1; x <= MAXUSER; x++) {
    if (user[x] != NULL)
      z++;
  }
  return z;
}

long cpu_usage(short unr)
{
  long ll;
  
  if (!boxrange(unr)) return 0;
  if (user[unr]->cputime <= 0) ll = 0;
  else {
    ll = get_cpuusage() - user[unr]->cputime;
    if (ll < 0) ll = 0;
  }
  ll += user[unr]->processtime;
  return ll;
}


/* Makros:

   %7   CTRL-G (bell)
   %9   TAB

   %a   momentane Auslastung des Boxrechners in Prozent (letzte 40 Sekunden)
   %b   momentan aktiver Boardname
   %c   Call des eingelogten Benutzers
   %d   momentanes Datum
   %e   cpu-type
   %f   bogomips
   %g   Anzahl aktiver Nachrichten
   %h   hex-Zeichen (%hHH)
   %i   Loginzeitpunkt
   %l   Letzer Login (Datum und Uhrzeit)
   %m   Call der Mailbox
   %n   Name des Benutzers
   %o   Anzahl der eingelogten Benutzer (Logins)
   %p   vebrauchte Maschinenzeit seit dem Login
   %r   Zeilenumbruch (Return)
   %s   boxruntime (Tage, Stunden und Sekunden)
   %t   momentane Uhrzeit (mit Sekunden)
   %u   momentane Uhrzeit (ohne Sekunden)
   %v   Versionsnummer der Software
   %w	read/sent kBytes (aus Sicht der BBS)
   %x   boxruntime in Tagen
   %z   letztes Kommando
   %%   das % Zeichen
*/

void expand_macro(short unr, char *hs)
{
  short x, l;
  long ll;
  boolean brange;
  char w[256], r[256], h[256];

  *r = '\0';
  brange = boxrange(unr);
  l = strlen(hs);
  x = 1;
  while (x <= l && strlen(r) < 175) {
    if (hs[x - 1] == '%') {
      if (x < l) {
	*w = '\0';

	switch (hs[x]) {

	case '7':
	  strcpy(w, "\007");
	  break;

	case '9':
	  strcpy(w, "\t");
	  break;

	case 'a':
	  get_lastsysload(w);
	  break;

	case 'b':
	  if (brange)
	    strcpy(w, user[unr]->brett);
	  break;

	case 'c':
	  if (brange)
	    strcpy(w, user[unr]->call);
	  break;

	case 'd':
	  strcpy(w, clock_.datum);
	  break;
	  
	case 'e':
	  get_cpuinf(w, h);
	  break;
	  
	case 'f':
	  get_cpuinf(h, w);
	  break;
	  
	case 'g':
	  sprintf(w, "%ld", sfsize(boxlog) / sizeof(boxlogstruct));
	  break;

	case 'h':
	  if (l - x > 2) {
	    strsub(h, hs, x + 2, 2);
	    sprintf(w, "%c", (char)hatoi(h));
	    x += 2;
	  } else
	    strcpy(w, "%h");
	  break;

	case 'i':
	  if (brange)
	    ix2string(user[unr]->logindate, w);
	  break;

	case 'l':
	  ix2string(user[unr]->lastdate, w);
	  break;

	case 'm':
	  strcpy(w, Console_call);
	  break;

	case 'n':
	  if (brange)
	    strcpy(w, user[unr]->name);
	  break;

	case 'o':
	  sprintf(w, "%d", actual_connects());
	  break;

	case 'p':
	  if (brange) {
	    ll = cpu_usage(unr);
	    sprintf(w, "%ld.%.2ld", ll / 200, (ll % 200) >> 1);
	  }
	  break;

	case 'r':
	  strcpy(w, "\015");
	  break;

	case 's':
	  get_boxruntime_s(1, w);
	  break;

	case 't':
	  strcpy(w, clock_.zeit);
	  break;

	case 'u':
	  sprintf(w, "%.5s", clock_.zeit);
	  break;

	case 'v':
	  sprintf(w, "%s%s", dp_vnr, dp_vnr_sub);
	  break;
	
	case 'w':
	  sprintf(w, "%ld/%ld", user[unr]->rbytes / 1024, user[unr]->sbytes / 1024);
	  break;

	case 'x':
	  sprintf(w, "%ld", get_boxruntime_l() / SECPERDAY);
	  break;

	case 'z':
	  if (brange)
	    strcpy(w, user[unr]->lastcmd);
	  break;

	case '%':
	  strcpy(w, "%");
	  break;

	default:
	  sprintf(w, "%%%c", hs[x]);
	  break;
	}
	strcat(r, w);
	x++;
      } else
	sprintf(r + strlen(r), "%c", hs[x - 1]);
    } else
      sprintf(r + strlen(r), "%c", hs[x - 1]);
    x++;
  }
  strcpy(hs, r);
}


boolean in_rsysops_boards(char *call, char *board)
{
  rsysoptype *hp;
  boolean ok;
  unsigned short lt, acc;
  char w[256];

  ok = false;
  hp = rsysoproot;
  while (hp != NULL && !ok) {
    if (!strcmp(hp->call, call)) {
      if (!strcmp(hp->board, board))
	ok = true;
      else if (!strcmp(hp->board, "*"))
	ok = true;
      else if (!strcmp(hp->board, "!"))
	ok = !defined_board(board);
      else if (hp->board[0] == '#') {
	strcpy(w, hp->board);
	check_lt_acc(board, &lt, &acc);
	strdelete(w, 1, 1);
	ok = (lt <= atoi(w));
      }
    }
    hp = hp->next;
  }
  return ok;
}


short in_tracecalls(char *call)
{
  tracetype *hp;

  hp = traces;
  while (hp != NULL) {
    if (!strcmp(hp->call, call) || !strcmp(hp->call, "ALL"))
      return (hp->by);
    hp = hp->next;
  }
  return 0;
}


void delete_trace_from(short unr)
{
  tracetype *hp, *hpa;
  boolean found;
  short x;

  for (x = 1; x <= MAXUSER; x++) {
    if (user[x] != NULL && user[x]->trace_to == unr)
      user[x]->trace_to = 0;
  }

  do {
    found = false;
    hp = traces;
    hpa = hp;
    while (hp != NULL && hp->by != unr) {
      hpa = hp;
      hp = hp->next;
    }
    if (hp != NULL) {
      found = true;
      if (hp != traces) {
	hpa->next = hp->next;
	free(hp);
      } else {
	hpa = hp->next;
	free(hp);
	traces = hpa;
      }
    }
  } while (found);
}


static void add_tracecall(short unr, char *call)
{
  tracetype *hp;
  short x;

  if (in_tracecalls(call) != 0 || in_tracecalls("ALL") != 0) {
    wlnuser(unr, "TRACE set by another task. Abort");
    return;
  }
  hp = malloc(sizeof(tracetype));
  if (hp == NULL)
    return;
  hp->next = traces;
  strcpy(hp->call, call);
  hp->by = unr;
  traces = hp;
  for (x = 1; x <= MAXUSER; x++) {
    if (user[x] != NULL &&
	(!strcmp(user[x]->call, call) || !strcmp(call, "ALL")))
      user[x]->trace_to = unr;
  }
  wlnuser(unr, "added");
}


static void delete_tracecall(char *call)
{
  tracetype *hp, *hpa;
  short x;

  hp = traces;
  hpa = hp;
  while (hp != NULL && strcmp(hp->call, call)) {
    hpa = hp;
    hp = hp->next;
  }
  if (hp == NULL)
    return;
  for (x = 1; x <= MAXUSER; x++) {
    if (user[x] != NULL &&
	(!strcmp(user[x]->call, call) || !strcmp(call, "ALL")))
      user[x]->trace_to = 0;
  }
  if (hp != traces) {
    hpa->next = hp->next;
    free(hp);
    return;
  }
  hpa = hp->next;
  free(hp);
  traces = hpa;
}


void delete_all_tracecalls(void)
{
  tracetype *hp, *hpa;
  short x;

  hp = traces;
  while (hp != NULL) {
    hpa = hp;
    hp = hp->next;
    free(hpa);
  }
  traces = NULL;
  for (x = 1; x <= MAXUSER; x++) {
    if (user[x] != NULL)
      user[x]->trace_to = 0;
  }
}


void trace_cmd(short unr, char *w, char *w1)
{
  tracetype *pcp;

  if (*w == '\0') {
    if (traces == NULL) {
      wlnuser(unr, "Trace inactive");
      return;
    }
    wlnuser(unr, "Trace on: ");
    pcp = traces;
    while (pcp != NULL) {
      if (pcp->by == unr)
	wlnuser(unr, pcp->call);
      pcp = pcp->next;
    }
    return;
  }
  if ((!strcmp(w, "OFF") || !strcmp(w, "-")) &&
      (callsign(w1) || !strcmp(w1, "ALL") || *w1 == '\0')) {
    if (*w1 == '\0')
      delete_trace_from(unr);
    else
      delete_tracecall(w1);
    if (!strcmp(w, "OFF"))
      abort_useroutput(unr);
    wlnuser(unr, "OK");
    return;
  }
  if ((!strcmp(w, "ON") || !strcmp(w, "+")) &&
      (callsign(w1) || !strcmp(w1, "ALL")))
    add_tracecall(unr, w1);
  else
    wlnuser(unr, "USAGE: TRACE <+|-> <CALL|ALL>");
}


boolean in_protocalls(char *call)
{
  protocalltype *hp;
  char cs1;

  hp = protocalls;
  cs1 = calccs(call);
  while (hp != NULL) {
    if (hp->cs == cs1) {
      if (!strcmp(hp->call, call))
	return true;
    }
    hp = hp->next;
  }
  return false;
}


static void add_protocall(char *call)
{
  protocalltype *hp;

  if (in_protocalls(call))
    return;
  hp = malloc(sizeof(protocalltype));
  if (hp == NULL)
    return;
  hp->next = protocalls;
  strcpy(hp->call, call);
  hp->cs = calccs(call);
  protocalls = hp;
}


static void delete_protocall(char *call)
{
  protocalltype *hp, *hpa;

  hp = protocalls;
  hpa = hp;
  while (hp != NULL && strcmp(hp->call, call)) {
    hpa = hp;
    hp = hp->next;
  }
  if (hp == NULL)
    return;
  if (hp != protocalls) {
    hpa->next = hp->next;
    free(hp);
    return;
  }
  hpa = hp->next;
  free(hp);
  protocalls = hpa;
}


void delete_all_protocalls(void)
{
  protocalltype *hp, *hpa;

  hp = protocalls;
  while (hp != NULL) {
    hpa = hp;
    hp = hp->next;
    free(hpa);
  }
  protocalls = NULL;
}


void reload_proto(void)
{
  short k;
  char hs[256];
  char STR1[256];

  delete_all_protocalls();
  sprintf(STR1, "%sproto%clst", boxstatdir, extsep);
  k = sfopen(STR1, FO_READ);
  if (k < minhandle)
    return;
  while (file2str(k, hs)) {
    if (callsign(hs))
      add_protocall(hs);
  }
  sfclose(&k);
}


static void store_proto(void)
{
  short k;
  protocalltype *pcp;
  char STR1[256];

  if (protocalls == NULL) {
    sprintf(STR1, "%sproto%clst", boxstatdir, extsep);
    sfdelfile(STR1);
    return;
  }
  sprintf(STR1, "%sproto%clst", boxstatdir, extsep);
  k = sfcreate(STR1, FC_FILE);
  if (k < minhandle)
    return;
  pcp = protocalls;
  while (pcp != NULL) {
    str2file(&k, pcp->call, true);
    pcp = pcp->next;
  }
  sfclose(&k);
}


void proto_cmd(short unr, char *w, char *w1)
{
  protocalltype *pcp;

  if (*w == '\0') {
    if (protocalls == NULL) {
      wlnuser(unr, "Proto inactive");
      return;
    }
    wlnuser(unr, "Proto on: ");
    pcp = protocalls;
    while (pcp != NULL) {
      wlnuser(unr, pcp->call);
      pcp = pcp->next;
    }
    return;
  }
  if ((!strcmp(w, "OFF") || !strcmp(w, "-")) && (callsign(w1) || *w1 == '\0')) {
    if (*w1 == '\0') {
      wlnuser(unr, "Proto on: ");
      pcp = protocalls;
      while (pcp != NULL) {
        wlnuser(unr, pcp->call);
        pcp = pcp->next;
      }
      wlnuser(unr, "now deleted");
      delete_all_protocalls();
    }
    else
      delete_protocall(w1);
    store_proto();
    wlnuser(unr, "OK, ended");
    return;
  }
  if ((strcmp(w, "ON") && strcmp(w, "+")) || !callsign(w1)) {
    wlnuser(unr, "USAGE: PROTO <+|-> <CALL>");
    return;
  }
  add_protocall(w1);
  store_proto();
  wlnuser(unr, "OK, started");
}


/******************************************************************************/
static time_t startuptime;
static time_t totalruntime = 0;
static time_t lastruntimechanged = 0;
static time_t maxruntime = 0;
static time_t lastboxabort = 0;
static time_t lastgarbagetime = 0;
static short sane_shutdown_last = -1;
static boolean shutdown_value = false;

void reset_boxruntime(void)
{
  startuptime	= sys_ixtime();
  lastwpcreate	= startuptime;
  lastwprotcreate = startuptime;
  debug0(0, -1, 127);
}

static void set_lastwpcreate(time_t updatetime)
{
  if (updatetime != 0 && updatetime <= clock_.ixtime)
    lastwpcreate = updatetime;
}

static time_t get_lastwpcreate(void)
{
  return lastwpcreate;
}

static void set_lastwprotcreate(time_t updatetime)
{
  if (updatetime != 0 && updatetime <= clock_.ixtime)
    lastwprotcreate = updatetime;
}

static time_t get_lastwprotcreate(void)
{
  return lastwprotcreate;
}

static void set_last_wprot_r(time_t updatetime)
{
  if (updatetime != 0 && updatetime <= clock_.ixtime)
    last_wprot_r = updatetime;
}

static time_t get_last_wprot_r(void)
{
  return last_wprot_r;
}

void set_lastgarbagetime(time_t garbagetime)
{
  if (garbagetime != 0 && garbagetime <= clock_.ixtime)
    lastgarbagetime = garbagetime;
}

time_t get_lastgarbagetime(void)
{
  return lastgarbagetime;
}

time_t get_boxruntime_l(void)
{
  if (startuptime > clock_.ixtime)
    startuptime	= clock_.ixtime;
  return (clock_.ixtime - startuptime);
}

static time_t get_maxruntime_l(void)
{
  time_t  runtime;
  
  runtime = get_boxruntime_l();
  if (runtime > maxruntime) maxruntime = runtime;
  return maxruntime;
}

static void set_maxruntime(time_t runtime)
{
  if (runtime > maxruntime) maxruntime = runtime;
}

time_t get_totalruntime_l(void)
{
  totalruntime += get_boxruntime_l();
  if (lastruntimechanged != 0 ) {
    totalruntime -= (lastruntimechanged - startuptime);
  }
  lastruntimechanged = clock_.ixtime;
  return totalruntime;  
}

static void set_totalruntime(time_t runtime)
{
  if (totalruntime != 0) return;
  totalruntime = runtime + get_boxruntime_l();
  lastruntimechanged = clock_.ixtime;
  get_maxruntime_l();
}

static void set_lastboxabort(time_t runtime)
{
  if (lastboxabort != 0) return;
  if (runtime >= startuptime) return;
  lastboxabort = runtime;
}

static time_t get_lastboxabort_l(void)
{
  return lastboxabort;
}

static time_t get_startuptime_l(void)
{
  return startuptime;
}

static void set_sane_shutdown(boolean sane)
{
  char w[80], hs[80];

  if (sane_shutdown_last == -1) {
    if (sane) sane_shutdown_last = 1;
    else sane_shutdown_last = 0;
    if (lastboxabort == 0) return;
    ix2string4(lastboxabort, w);
    snprintf(hs, 79, "last shutdown was: %s", w);
    if (!sane_shutdown_last) nstrcat(hs, " (crashed)", 79); 
    debug(0, -1, 0, hs);
  }
}

boolean was_sane_shutdown(void)
{
  return (lastboxabort == 0 || sane_shutdown_last == 1);
}

void sane_shutdown(short magic)
{
  if (magic == 4711) shutdown_value = true;
}

void get_boxruntime_s(short which, char *hs)
{  
  switch (which) {
    case 2  : calc_ixsecs_to_string(get_totalruntime_l(), hs);
      	      break;
    case 3  : calc_ixsecs_to_string(get_maxruntime_l(), hs);
      	      break;
    case 4  : ix2string4(get_lastboxabort_l(), hs);
      	      if (!was_sane_shutdown()) strcat(hs, " (crashed)");
      	      break;
    case 5  : ix2string4(get_startuptime_l(), hs);
      	      break;
    case 6  : ix2string4(get_lastgarbagetime(), hs);
      	      break;
    case 7  : ix2string4(get_last_wprot_r(), hs);
      	      break;
    default : calc_ixsecs_to_string(get_boxruntime_l(), hs);
      	      break;
  }
}

/******************************************************************************/

static short updbidseekcounter;

void flush_bidseek(void)
{
  short k;
  time_t runtime;

  if (updbidseekcounter <= 0) return;
  updbidseekcounter = 0;

  k = sfcreate(bidseekfile, FC_FILE);
  if (k < minhandle) return;

  sfwrite(k, sizeof(long), (char *)(&bullidseek));
  sfwrite(k, sizeof(short), (char *)(&hiscore_connects));
  sfwrite(k, sizeof(long), (char *)(&actmsgnum));
  sfwrite(k, sizeof(boolean), (char *)(&crawl_active));
  sfwrite(k, sizeof(boolean), (char *)(&crawl_private));
  runtime = get_totalruntime_l();
  sfwrite(k, sizeof(time_t), (char *)(&runtime));
  runtime = get_maxruntime_l();
  sfwrite(k, sizeof(time_t), (char *)(&runtime));
  runtime = clock_.ixtime;
  sfwrite(k, sizeof(time_t), (char *)(&runtime));
  sfwrite(k, sizeof(boolean), (char *)(&shutdown_value));
  runtime = get_lastgarbagetime();
  sfwrite(k, sizeof(time_t), (char *)(&runtime));
  runtime = get_last_wprot_r();
  sfwrite(k, sizeof(time_t), (char *)(&runtime));
  runtime = get_lastwpcreate();
  sfwrite(k, sizeof(time_t), (char *)(&runtime));
  runtime = get_lastwprotcreate();
  sfwrite(k, sizeof(time_t), (char *)(&runtime));
  sfclose(&k);
}


void update_bidseek(void)
{
  updbidseekcounter++;
  flush_bidseek();
}


void flush_bidseek_immediately(void)
{
  updbidseekcounter = 1;
  flush_bidseek();
}


boolean load_bidseek(void)
{
  short k;
  long li;
  short i;
  time_t runtime;
  boolean b, result;

  result = false;
  bullidseek = sfsize(msgidlog) / 13;
  k = sfopen(bidseekfile, FO_READ);
  if (k < minhandle)
    return result;
  if (sfread(k, sizeof(long), (char *)(&li)) == sizeof(long)) {
    bullidseek = li;
    result = true;
    if (sfread(k, sizeof(short), (char *)(&i)) == sizeof(short)) {
      hiscore_connects = i;
      if (sfread(k, sizeof(long), (char *)(&li)) == sizeof(long)) {
	if (li > 0) actmsgnum = li;
	if (sfread(k, sizeof(boolean), (char *)(&b)) == sizeof(boolean)) {
	  crawl_active = b;
	  if (sfread(k, sizeof(boolean), (char *)(&b)) == sizeof(boolean)) {
	    crawl_private = b;
	    if (sfread(k, sizeof(time_t), (char *)(&runtime)) == sizeof(time_t)) {
	      set_totalruntime(runtime);
	      if (sfread(k, sizeof(time_t), (char *)(&runtime)) == sizeof(time_t)) {
	      	set_maxruntime(runtime);
	      	if (sfread(k, sizeof(time_t), (char *)(&runtime)) == sizeof(time_t)) {
	      	  set_lastboxabort(runtime);
	      	  if (sfread(k, sizeof(boolean), (char *)(&b)) == sizeof(boolean)) {
	      	    set_sane_shutdown(b);
	      	    if (sfread(k, sizeof(time_t), (char *)(&runtime)) == sizeof(time_t)) {
	      	      set_lastgarbagetime(runtime);
	      	      if (sfread(k, sizeof(time_t), (char *)(&runtime)) == sizeof(time_t)) {
	      	      	set_last_wprot_r(runtime);
	      	      	if (sfread(k, sizeof(time_t), (char *)(&runtime)) == sizeof(time_t)) {
	      	      	  set_lastwpcreate(runtime);
	      	      	  if (sfread(k, sizeof(time_t), (char *)(&runtime)) == sizeof(time_t)) {
	      	      	    set_lastwprotcreate(runtime);
			  }
			}
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
  sfclose(&k);
  return result;
}


void create_hcs(indexstruct *h)
{
  /* header-checksumme */
  unsigned short zs;

  zs = 0;
  checksum16_buf((char *)h, sizeof(indexstruct) - 4, &zs);  /* Alignment des GNU-C */
  h->headerchecksum = zs;
}


boolean check_hcs(indexstruct h)
{
  unsigned short zs;
  char STR1[36];

  zs = 0;
  checksum16_buf((char *)(&h), sizeof(indexstruct) - 4, &zs); /* Alignment des GNU-C */
  if (h.headerchecksum != zs) {
    sprintf(STR1, "%s : invalid header checksum", h.dest);
    debug(0, -1, 137, STR1);
    return false;
  }
  
  if (h.hver == HEADERVERSION)
    return true;
    
  sprintf(STR1, "%s : invalid header version", h.dest);
  debug(0, -1, 137, STR1);
  return false;
}


boolean boxrange(short unr)
{

  if (unr < 1)
    return false;
 
  if (unr > MAXUSER)
    return false;
 
  if (user[unr] == NULL)
    return false;
 
  if (user[unr]->magic1 == UF_MAGIC && user[unr]->magic2 == UF_MAGIC &&
      user[unr]->magic3 == UF_MAGIC)
    return true;
 
  debug(0, -1, 155, "userstruct violated");
  debug(0, -1, 155, "now trying to stop dpbox");
  ende = true;
  return false;
}


void upd_statistik(short unr, long txbytes, long rxbytes, long start,
		   long stop)
{
  userstruct *WITH;

  if (!boxrange(unr))
    return;
 
  WITH = user[unr];
  if (rxbytes > 0)   /* das ist etwas vertauscht */
    WITH->sbytes += rxbytes;
  if (txbytes > 0)   /* txbytes = die von der Gegenstation gesendeten Bytes */
    WITH->rbytes += txbytes;
  if (start > 0)
    WITH->processtime += stop - start;
}


void get_btext(short unr, short nr, char *s)
{
  userstruct uf;

  debug0(5, unr, 108);
  *s = '\0';
  if (boxrange(unr)) {
    get_language(nr, langmemroot, user[unr]->language, s);
    expand_macro(unr, s);
    return;
  }
  load_userfile(false, false, Console_call, &uf);
  get_language(nr, langmemroot, uf.language, s);
  expand_macro(unr, s);
}


void x_w_btext(short unr, short nr, boolean lf)
{
  char s[256];

  if (!boxrange(unr))
    return;
 
  get_btext(unr, nr, s);
  if (nr == 48) {
    if (s[0] == ',') {
      if (s[1] == ' ')
	strdelete(s, 1, 2);
    }
  }
  if (lf)
    wlnuser(unr, s);
  else
    wuser(unr, s);
}


/* 'there are no new messages for...' */

void no_files(short unr, char *name)
{
  char	    hs[256];

  w_btext(unr, 14);
  if (*name == '\0')
    wlnuser(unr, " ()");
  else {
    chwuser(unr, ' ');
    wuser(unr, name);
    if (callsign(name)) {
      user_mybbs(name, hs);
      if (*hs != '\0') {
        wuser(unr, " @ ");
        wuser(unr, hs);
      }
    }
    wlnuser0(unr);
  }
}


/* sucht die Boxusernummer eines durch tnr (= terminal) und pchan (= packet channel) */
/* definierten Benutzers */

short find_buser(short tnr, short pchan)
{
  short x;

  x   = 1;
  while (x <= MAXUSER) {
    if (user[x] != NULL) {
      if (user[x]->tcon == tnr && user[x]->pchan == pchan)
        return x;
    }
    x++;
  }
  return -1;
}


/* Ist der Nutzer z.Z. in der Box eingeloggt ? */

short actual_user(char *calls)
{
  short     x, first;
  boolean   ok;

  x   	    = 1;
  first     = 0;
  ok  	    = false;

  while (x <= MAXUSER && !ok) {
    if (user[x] != NULL) {
      ok = (!strcmp(user[x]->call, calls));
      if (ok) {
	ok = false;
	if (first == 0) {
	  first = x;
	  if (user[x]->action == 0 && !user[x]->f_bbs &&
	      user[x]->lastprocnumber == 0 &&
	      boxspoolstatus(user[x]->tcon, user[x]->pchan, -1) == 0)
	    ok = true;
	} else {
	  if (user[x]->action == 0 && !user[x]->f_bbs &&
	      user[x]->lastprocnumber == 0 &&
	      boxspoolstatus(user[x]->tcon, user[x]->pchan, -1) == 0) {
	    first = x;
	    ok = true;
	  }
	}
      }
    }
    x++;
  }
  
  return first;
}


void fill_logline(boolean full, short unr, char *hs)
{
  userstruct  *WITH;
  char	      hw1[256], hw2[256], hw3[256];

  hs[0] = '\0';
  if (!boxrange(unr))
    return;
  WITH = user[unr];
  *hw1 = '\0';
  strcpy(hw3, WITH->conpath);
  get_word(hw3, hs);
  if (hs[1] == ':')
    strdelete(hs, 1, 2);
  while (*hw3 != '\0')
    get_word(hw3, hw1);
  if (*hw1 != '\0') {
    cut(hs, 9);
    rspacing(hs, 9);
    cut(hw1, 9);
    rspacing(hw1, 9);
    sprintf(hs + strlen(hs), " v %s", hw1);
  } else
    rspacing(hs, 21);

  if (full) {
    ixdatum2string(WITH->logindate, hw1);
    ixzeit2string(WITH->logindate, hw2);
    sprintf(hs + strlen(hs), " %s %s - ", hw1, hw2);
    ixdatum2string(clock_.ixtime, hw3);
    ixzeit2string(clock_.ixtime, hw2);
    sprintf(hs + strlen(hs), "%s %s", hw3, hw2);
  } else {
    ixzeit2string(WITH->logindate, hw2);
    sprintf(hs + strlen(hs), " %s", hw2);
  }

  sprintf(hw2, "%ld", WITH->rbytes);
  lspacing(hw2, 9);
  strcat(hs, hw2);
  sprintf(hw2, "%ld", WITH->sbytes);
  lspacing(hw2, 9);
  strcat(hs, hw2);
}


void box_logbuch(short unr)
{
  char	hs[256], hw1[256], hw2[256];

  if (!boxrange(unr))
    return;
  fill_logline(true, unr, hs);
  sprintf(hw1, "%sboxlog%d%cbox", boxprotodir, clock_.mon, extsep);
  if (!exist(hw1)) {
    strcpy(hw2,
      "Call     via          Start (UTC)         End (UTC)          RxBytes  TxBytes");
    append(hw1, hw2, true);
    strcpy(hw2,
      "=============================================================================");
    append(hw1, hw2, true);
  }
  append(hw1, hs, true);
}


boolean wildcardcompare(short fangraster, char *w, char *t, char *wr)
{
  short     x, y, l, k, z, r, v, hz;
  boolean   first;
  char	    lastc;
  char	    wc[256], tc[256], t1[256], STR7[256];

  l = strlen(w);
  if (l == 0 || l > 255)
    return false;
    
  if (l == 1 && w[0] == '*')
    return true;

  k = strlen(t);
  if (k == 0 || k > 255)
    return false;

  if (l == k && !strcmp(w, t))
    return true;

  x = strpos2(w, "*", 1);
  if (x == 0)
    return false;

  strcpy(wc, w);
  strcpy(tc, t);
  t1[0] = '\0';
  hz  	= 0;
  lastc = '\0';
  r   	= strlen(wr);
  if (r > 255)
    r 	= 0;

_LOOP:
  first = true;
  if (fangraster == SHORT_MAX)
    fangraster--;

  while (l > 0) {
    if (x > 1) {   /* unmoeglich */
      sprintf(STR7, "%.*s", x - 1, wc);
      if (strstr(tc, STR7) != tc)
	return false;
	
      strdelete(tc, 1, x - 1);
      strdelete(wc, 1, x - 1);
      if (x == l)
	return true;
	
    } else if (x == 1 && l > 1) {
      strdelete(wc, 1, 1);
      l--;
      y = strpos2(wc, "*", 1);
      if (y > 1) {
	sprintf(STR7, "%.*s", y - 1, wc);
	z = strpos2(tc, STR7, 1);
	if (z < 1 || (!first && z > fangraster + 1))
	  return false;
	  
	if (first)
	  hz = z + y - 2;

	if (r > 0 && !first) {
	  for (v = 0; v <= z - 2; v++) {
	    sprintf(STR7, "%c", tc[v]);
	    if (strstr(wr, STR7) == NULL) {
	      if (tc[v] != lastc) {
		if (hz > 0) {
		  strcpy(wc, w);
		  if (*t1 == '\0')
		    strcpy(t1, t);
		  strdelete(t1, 1, hz);
		  strcpy(tc, t1);
		  lastc = '\0';
		  goto _LOOP;
		  
		} else
		  return false;
		  
	      }
	    }
	  }
	}

	lastc = wc[0];
	strdelete(tc, 1, z + y - 2);
	strdelete(wc, 1, y - 1);

      } else if (y == 0) {

	if (l > 0) {
	  z = strlen(tc) - l;
	  if (z < 0 || (!first && z > fangraster))
	    return false;

	  if (r > 0 && !first) {
	    for (v = 0; v < z; v++) {
	      sprintf(STR7, "%c", tc[v]);
	      if (strstr(wr, STR7) == NULL) {
		if (tc[v] != lastc)
		  return false;
		  
	      }
	    }
	  }

	  strdelete(tc, 1, z);
	  return (strcmp(wc, tc) == 0);
	  
	} else
	  return true;
	  
	return false;
      }

      /* else doppeltes ** - macht aber nichts, kommt im naechsten Durchlauf dran */
    } else if (x == 1 && l == 1)
      return true;

    else if (x == 0)
      return (strcmp(wc, tc) == 0);

    else
      return false;

    x = strpos2(wc, "*", 1);
    l = strlen(wc);
    first = false;
  }

  return false;
}


static char dwr_string[256];


static void setup_dwrstring(void)
{
  short x, y;

  y = 0;
  for (x =   1; x <=  63; x++) dwr_string[y++] = x;
  for (x =  91; x <=  96; x++) dwr_string[y++] = x;
  for (x = 123; x <= 127; x++) dwr_string[y++] = x;
  for (x = 168; x <= 175; x++) dwr_string[y++] = x;
  for (x = 185; x <= 224; x++) dwr_string[y++] = x;
  for (x = 226; x <= 255; x++) dwr_string[y++] = x;
  dwr_string[y] = 0;
}


boolean check_for_dirty(char *hs)
{
  dirtytype *hp;
  char	    w[256];

  debug0(6, -1, 143);
  
  hp = badwordroot;
  while (hp != NULL) {
    if (wildcardcompare(50, hp->bad, hs, dwr_string)) {
      sprintf(w, "dirty, keyword: %s", hp->bad);
      boxprotokoll(w);
      return true;
    }
    hp = hp->next;
  }
  return false;
}


boolean check_for_dirty2(char *bs, char *hs)
{
  return (wildcardcompare(50, bs, hs, dwr_string));
}

static char rnegcs1[2] = "";
static char rnegcs2[2] = "~";
static char *rnegc(boolean b)
{
  if (b) return rnegcs1;
  return rnegcs2;
}

static void print_rejectreason(rejecttype *hp, char *reason)
{
  if (hp == NULL) return;
  snprintf(reason, 100, "%c %s%c %s%s %s%s %s%s %s%s %s%ld",
      	    hp->what,
	    rnegc(hp->msgtypeneg), hp->msgtype,
	    rnegc(hp->fromneg), hp->from,
	    rnegc(hp->mbxneg), hp->mbx,
	    rnegc(hp->tobneg), hp->tob,
	    rnegc(hp->bidneg), hp->bid,
	    rnegc(hp->maxsizeneg), hp->maxsize);
}

static boolean private_distribution(char msgtype, char *ziel, char *mbx, char *absender, long size)
{
  if (msgtype != 'P') return false;
  if (!hcallsign(mbx)) return false;
  if (!callsign(absender)) return false;
  if (size > 40000) return false;
  return true;
}

void check_reject(char *frombox, char msgchar, char *ziel, char *mbx,
		  char *absender, char *lifetime, char *betreff, char *bid,
		  long laenge, boolean is_local, boolean is_privlocal,
		  boolean *reject_it, boolean *no_sf, char *rejectreason)
{
  rejecttype *hp;

  debug0(5, -1, 141);
  hp = rejectroot;
  rejectreason[0] = '\0';

  while (hp != NULL) {
    if ((hp->msgtype == msgchar) == hp->msgtypeneg || hp->msgtype == '*') {
      if ((hp->maxsize <= laenge) == hp->maxsizeneg) {
	if (wildcardcompare(SHORT_MAX, hp->from, absender, "") == hp->fromneg) {
	  if (wildcardcompare(SHORT_MAX, hp->mbx, mbx, "") == hp->mbxneg) {
	    if (wildcardcompare(SHORT_MAX, hp->tob, ziel, "") == hp->tobneg) {
	      if (wildcardcompare(SHORT_MAX, hp->bid, bid, "") == hp->bidneg) {
		switch (hp->what) {

		case 'R':
		  if (!do_wprot_routing
		      || !private_distribution(msgchar, ziel, mbx, absender, laenge)) {
		    *reject_it = true;
		    print_rejectreason(hp, rejectreason);
		    return;
		  }
		  break;

		case 'H':
		  if (!do_wprot_routing
		      || !private_distribution(msgchar, ziel, mbx, absender, laenge)) {
		    *no_sf = true;
		  }
		  break;

		case 'L':
		  if (is_local)
		    *no_sf = true;
		  break;

		case 'P':
		  if (is_local && !is_privlocal)
		    *no_sf = true;
		  break;
		}
	      }
	    }
	  }
	}
      }
    }

    hp = hp->next;
  }


}



static boolean convtit2(boolean iscall, boolean ltconv, convtittype *hp,
			char *ziel, char *mbx, char *absender1,
			char *lifetime1, char *betreff1)
{
  unsigned short  olt;
  boardtype   	  orub;
  boolean     	  ok, ok2, frag;
  short       	  k;
  boolean     	  einbuch;
  char	      	  s[256], s2[256], hs[256], w[256], w2[256];

  if (hp == NULL)
    return false;
  ok = false;
  frag = false;
  strcpy(orub, hp->fromboard);
  k = strlen(orub);
  if (k > 2) {
    if (strpos2(orub, "*", 1) == k) {
      frag = true;
      strdelete(orub, k, 1);
    }
  }

  einbuch = (strlen(ziel) == 1);

  if (strcmp(orub, ziel) && (strcmp(orub, "*") || iscall || einbuch) &&
      (strcmp(orub, "**") || einbuch) &&
      (!frag || einbuch || strstr(ziel, orub) != ziel) &&
      (strcmp(orub, "!") || iscall || einbuch || defined_board(ziel)))
    return ok;

  strcpyupper(s, betreff1);

  strcpy(hs, hp->title);
  ok = (*hs != '\0');
  while (*hs != '\0' && ok) {
    get_word(hs, w);
    if (*w == '\0')
      continue;
    switch (w[0]) {

    case '&':
      strdelete(w, 1, 1);
      ok = (ok && !strcmp(absender1, w));
      break;

    case '@':
      strdelete(w, 1, 1);
      ok = (ok && !strcmp(mbx, w));
      break;

    case '%':
      strdelete(w, 1, 1);
      ok = (ok && !strcmp(ziel, w));
      break;

    case '~':  /* quatsch, gestrichen */
      ok = false;
      break;

    case '$':  /* dito*/
      ok = false;
      break;

    case '^':
      ok2 = false;
      strdelete(w, 1, 1);
      if (strstr(s, w) != NULL) {
	strcpy(s2, s);
	while (*s2 != '\0' && !ok2) {
	  get_word(s2, w2);
	  ok2 = (strcmp(w2, w) == 0);
	}
      }
      ok = (ok && ok2);
      break;

    default:
      ok = (ok && strstr(s, w) != NULL);
      break;
    }
  }

  if (!ok)
    return ok;
  if (!ltconv)
    strcpy(ziel, hp->toboard);
  olt = (unsigned short)atoi(lifetime1);
  if (olt > hp->newlt || (olt == 0 && hp->newlt > 0))
    sprintf(lifetime1, "%d", hp->newlt);
  return ok;
}


void do_convtit(char *frombox, char msgchar, char *ziel, char *mbx,
		char *absender1, char *lifetime1, char *betreff1, char *bid,
		long laenge, boolean is_local, boolean is_privlocal,
		boolean *reject_it, boolean *no_sf)
{
  convtittype *hp;
  boolean ok, ok3, iscall;
  char ob[256], olt[256];
  char hs[256];

  debug0(5, -1, 142);
  ok = false;
  ok3 = false;
  iscall = callsign(ziel);

  if (create_convlog) {
    strcpy(ob, ziel);
    strcpy(olt, lifetime1);
  }

  if (rejectroot != NULL)
    check_reject(frombox, msgchar, ziel, mbx, absender1, lifetime1, betreff1,
		 bid, laenge, is_local, is_privlocal, reject_it, no_sf, hs);

  if (*reject_it)
    return;

  hp = convtitroot;
  while (hp != NULL && !ok) {
    ok = convtit2(iscall, false, hp, ziel, mbx, absender1, lifetime1,
		  betreff1);
    hp = hp->next;
  }

  hp = convltroot;
  while (hp != NULL && !ok3) {
    ok3 = convtit2(iscall, true, hp, ziel, mbx, absender1, lifetime1,
		   betreff1);
    hp = hp->next;
  }

  if (!(create_convlog && (ok || ok3) &&
	(strcmp(ob, ziel) || strcmp(olt, lifetime1))))
    return;

  if (!strcmp(ziel, ob))
    strcpy(ob, "-");
  rspacing(ob, LEN_BOARD);
  rspacing(olt, 3);
  sprintf(hs, "%s %s ", ob, olt);
  strcpy(ob, ziel);
  strcpy(olt, lifetime1);
  rspacing(ob, LEN_BOARD);
  rspacing(olt, 3);
  sprintf(hs + strlen(hs), "%s %s %s", ob, olt, betreff1);
  cut(hs, 54);
  append_convlog(-1, hs);
}


void show_rfile(short unr, char *base, long size, boolean mit_r, boolean sf,
		boolean only_headers)
{
  long lesezeiger, bodystart;
  short lct, k;
  boolean nop, is_r;
  char zeile[256];
  char hs[256], zl[256], lr[256];
  char STR7[256];

  debug0(4, unr, 109);

  if (sf) {
    show_puffer(unr, base, size);
    return;
  }

  if (mit_r) {
    if (!only_headers) {
      show_puffer(unr, base, size);
      return;
    }

/* = if only_headers ... */

    lesezeiger = 0;
    is_r = true;
    lct = 0;

    while (lesezeiger < size && is_r) {
      lct++;
      get_line(base, &lesezeiger, size, zeile);
      if (zeile[0] >= ' ' && lct < 15) {
      	wlnuser(unr, zeile);
	if (zeile[0] == 'R' && zeile[1] == ':') lct = 0;
      }
      else is_r = false;
    }

    return;
  }

  lesezeiger = 0;
  bodystart = 0;
  lct = 0;
  *hs = '\0';
  *zl = '\0';
  *lr = '\0';
  is_r = true;
  while (lesezeiger < size && is_r) {
    nop = false;
    get_line(base, &lesezeiger, size, zeile);
    bodystart = lesezeiger;
    lct++;
    if (strlen(zeile) > 18) {
      if (strstr(zeile, "I:") == zeile)
	zeile[0] = 'R';
      if (strstr(zeile, "R:") == zeile) {
	if (!mit_r)
	  strcpy(lr, zeile);
	k = strpos2(zeile, "@", 1);
	if (k > 0) {
	  strsub(hs, zeile, k + 1, strlen(zeile) - k);
	  if (hs[0] == ':')
	    strdelete(hs, 1, 1);
	  k = strpos2(hs, " ", 1);
	  if (k > 0)
	    strdelete(hs, k, strlen(hs) - k + 1);

	  if (!mit_r) {
	    unhpath(hs, hs);
	    *zeile = '\0';

	    if (*zl == '\0') {
	      if (lct == 1) {
		if (*hs != '\0')
		  sprintf(hs, "%s!%s", Console_call, strcpy(STR7, hs));
		else
		  strcpy(hs, Console_call);
		sprintf(zl, "Path: %s", hs);
	      } else
		sprintf(zl, "Path: !%s", hs);

	      nop = true;

	    } else if (strlen(zl) < 70) {
	      sprintf(zl + strlen(zl), "!%s", hs);
	      nop = true;

	    } else {
	      strcpy(zeile, zl);
	      sprintf(zl, "Path: !%s", hs);
	      nop = false;

	    }

	  }

	} else
	  is_r = false;
      } else if (*zl != '\0' && !mit_r) {
	wlnuser(unr, zl);
	*zl = '\0';
	if (strlen(lr) > 15) {
	  strdelete(lr, 1, 2);
	  cut(lr, 73);
	  wuser(unr, "Sent: ");
	  wlnuser(unr, lr);
	  *lr = '\0';
	}
	is_r = false;
      } else
	is_r = false;
    } else if (*zl != '\0' && !mit_r) {
      wlnuser(unr, zl);
      *zl = '\0';
      if (strlen(lr) > 15) {
	strdelete(lr, 1, 2);
	cut(lr, 73);
	wuser(unr, "Sent: ");
	wlnuser(unr, lr);
	*lr = '\0';
      }
      is_r = false;
    }
    if (!nop && !mit_r) {
      wlnuser(unr, zeile);
      *zeile = '\0';
    }
  }

  if (mit_r)
    return;

  if (*zl != '\0') {
    wlnuser(unr, zl);
    *zl = '\0';
  }
  if (*zeile != '\0') {
    wlnuser(unr, zeile);
    *zl = '\0';
  }
  if (only_headers) {
    lct = 0;
    while (lesezeiger < size && lct < 15) {
      lct++;
      get_line(base, &lesezeiger, size, zeile);
      if (zeile[0] > ' ') wlnuser(unr, zeile);
      else lct = 15;
    }
  }
  else show_puffer(unr, (char *)(&base[bodystart]), size - bodystart);

}


void show_textfile(short unr, char *name)
{
  long size, lz;
  char *rp;
  char hs[256];

  sfbread(true, name, &rp, &size);
  if (size <= 0) {
    if (!exist(name))
      wlnuser(unr, "file not found");
    return;
  }
  lz = 0;
  while (lz < size) {
    get_line(rp, &lz, size, hs);
    if (user[unr] != NULL && user[unr]->in_begruessung &&
	hs[strlen(hs) - 1] == '>')
      hs[strlen(hs) - 1] = ']';
    wlnuser(unr, hs);
  }
  free(rp);
}


static boolean show_allhelp(short unr, char *fn_, char *defhelp, char *hw)
{
  short k;
  char *tb;
  long tbs, rp;
  char hs[256], h2[256], fn[256];
  char STR7[256];

  sprintf(hw, " %s ", strcpy(STR7, hw));
  upper(hw);
  strcpy(fn, fn_);

_L0:
  sfbread(false, fn, &tb, &tbs);
  if (tb != NULL) {
    rp = 0;

_L1:
    while (rp < tbs) {
      get_line(tb, &rp, tbs, hs);
      if (strstr(hs, "---") != hs)
	continue;
      while (rp < tbs) {
	get_line(tb, &rp, tbs, hs);
	del_leadblanks(hs);
	if (*hs == '\0')
	  continue;
	sprintf(h2, " %s ", hs);
	if (strstr(h2, hw) != NULL) {
	  wlnuser(unr, hs);
	  while (rp < tbs) {
	    get_line(tb, &rp, tbs, hs);
	    if (strstr(hs, "---") == hs)
	      break;
	    wlnuser(unr, hs);
	  }
	  free(tb);
	  wlnuser0(unr);
	  return true;
	} else {
	  goto _L1;
	}
      }
    }
    free(tb);
    tb = NULL;
    if (*defhelp != '\0' && strcmp(defhelp, fn)) {
      strcpy(fn, defhelp);
      goto _L0;
    }
    return false;
  }

  k = sfopen(fn, FO_READ);
  if (k < minhandle) {
    wln_btext(unr, 4);
    return false;
  }
_L3:
  while (file2str(k, hs)) {
    if (strstr(hs, "---") != hs)
      continue;
    while (file2str(k, hs)) {
      del_leadblanks(hs);
      if (*hs == '\0')
	continue;
      sprintf(h2, " %s ", hs);
      if (strstr(h2, hw) != NULL) {
	wlnuser(unr, hs);
	while (file2str(k, hs) && strstr(hs, "---") != hs)
	  wlnuser(unr, hs);
	sfclose(&k);
	wlnuser0(unr);
	return true;
      } else {
	goto _L3;
      }
    }
  }
  sfclose(&k);
  if (*defhelp != '\0' && strcmp(defhelp, fn)) {
    strcpy(fn, defhelp);
    goto _L0;
  }
  return false;
}


void show_help(short unr, char *w_)
{
  char w[256], defhelp[256];
  DTA dirinfo;
  short result;
  char fname[256], hname[256];
  userstruct *WITH;

  strcpy(w, w_);
  debug(2, unr, 110, w);
  if (!boxrange(unr))
    return;
  WITH = user[unr];

  if (*w == '\0')
    sprintf(fname, "%sHELP%c%s", boxlanguagedir, extsep, WITH->language);

  else {

    expand_command(unr, w);
    sprintf(defhelp, "%sG%cALL", boxlanguagedir, extsep);

    if (!exist(defhelp))
      *defhelp = '\0';

    sprintf(hname, "%s%s%cALL", boxlanguagedir, WITH->language, extsep);
    if (exist(hname)) {
      if (show_allhelp(unr, hname, defhelp, w))
        return;
    }

    if (*defhelp != '\0') {
      if (show_allhelp(unr, defhelp, "", w))
        return;
    }

    /* DL3NEU: allow external help */
    sprintf(hname, "%s%s%cEXT", boxlanguagedir, WITH->language, extsep);
    if (exist(hname)) {
      if (show_allhelp(unr, hname, "", w_))
        return;
    }

    sprintf(hname, "%s%s%c%c%s", boxlanguagedir, w, allquant, extsep, WITH->language);
    result = sffirst(hname, 0, &dirinfo);
    if (result != 0)
      sprintf(dirinfo.d_fname, "ERROR%c%s", extsep, WITH->language);
    sprintf(fname, "%s%s", boxlanguagedir, dirinfo.d_fname);
  }

  if (!exist(fname))
    new_ext(fname, "G");
  if (exist(fname))
    show_textfile(unr, fname);
  else
    wln_btext(unr, 30);
  wlnuser0(unr);

  if (!(WITH->supervisor && *w == '\0'))
    return;

  sprintf(fname, "%sHELP_SYS%c%s", boxlanguagedir, extsep, WITH->language);
  if (!exist(fname))
    new_ext(fname, "G");
  if (exist(fname))
    show_textfile(unr, fname);
  else
    wln_btext(unr, 31);
  wlnuser0(unr);
}


/* boardnamen (und calls) duerfen nur aus 0..9 / A..Z - _ bestehen, und mindestens ein Buchstabe... */

boolean valid_boardname(char *rubrik)
{
  boolean   notnum;
  short     x, l;

  l   	    = strlen(rubrik);
  if (l < 1 || l > LEN_BOARD)
    return false;

  notnum    = false;
  for (x = 0; x < l; x++) {
    if (!(rubrik[x] == '_' || rubrik[x] == '-' || isupper(rubrik[x]) || isdigit(rubrik[x])))
      return false;
    else if (isupper(rubrik[x]))
      notnum  = true;
  }
  return notnum;
}


void extend_6_to_8(char *rubr)
{
  rubriktype *hp;
  boardtype hv;

  if (strlen(rubr) != 6 || callsign(rubr))
    return;

  hp = rubrikroot;
  while (hp != NULL) {
    if (strlen(hp->name) <= 6) {
      hp = hp->next;
      continue;
    }
    strcpy(hv, hp->name);
    cut(hv, 6);
    if (!strcmp(hv, rubr)) {
      strcpy(rubr, hp->name);
      hp = NULL;
    } else
      hp = hp->next;
  }
}


boolean defined_board(char *rubr)
{
  rubriktype *hp;
  boolean ok;

  ok = (callsign(rubr) || strlen(rubr) == 1);
  if (!ok) {
    hp = rubrikroot;
    while (hp != NULL) {
      if (!strcmp(hp->name, rubr)) {
	ok = true;
	hp = NULL;
      } else
	hp = hp->next;
    }
  }
  if (ok)
    ok = (strcmp(rubr, "X") && strcmp(rubr, "D"));
  return ok;
}


void switch_to_default_board(char *rubr)
{
  if (*default_rubrik == '\0')
    return;
  if (*rubr != '\0') {
    if (!defined_board(rubr))
      strcpy(rubr, default_rubrik);
  }
}

void switch_to_default_fserv_board(char *rubr)
{
  if (*rubr != '\0') {
    if (!defined_board(rubr)) {
      if (*default_rubrik == '\0') {
        strcpy(rubr, "div");
      } else {
        strcpy(rubr, default_rubrik);
      }
    }
  }
}

boolean strip_invalid_boardname_chars(char *rubrik)
{
  short x;
  char	*p, *p2, c;
  
  x   = 0;
  p   = rubrik;
  p2  = rubrik;
  while ((c = upcase_(*p++))) {
    if (c == '_' || c == '-' || isupper(c) || isdigit(c))
      *p2++ = c;
  }
  *p2 = '\0';
  cut(rubrik, LEN_BOARD);

  if (valid_boardname(rubrik)) return true;
  return false;
}

void check_transfers(char *rubr)
{
  transfertype *hp;

  hp = transferroot;
  while (hp != NULL && strcmp(hp->board1, rubr))
    hp = hp->next;

  if (hp != NULL)
    strcpy(rubr, hp->board2);
}

void check_verteiler(char *rubr)
{
  boolean loop = false;

  extend_6_to_8(rubr);   /*bei S&F-Namensbegrenzung auf 6 Bytes...*/

  while (true) {
    check_transfers(rubr);
    if (strcmp(rubr, "X") && strcmp(rubr, "D"))
      return;
    if (*default_rubrik != '\0') {
      strcpy(rubr, default_rubrik);
      return;
    }
    strcpy(rubr, "DIV");
    if (loop) return;
    loop = true;
  }
}


void chk_maxlt(unsigned short *lt1, unsigned short ltd)
{
  if (ltd != 0 && (ltd < *lt1 || *lt1 == 0))
    *lt1 = ltd;
}

void chk_minlt(unsigned short *lt1, unsigned short ltd)
{
  if (ltd == 0 || (ltd > *lt1 && *lt1 != 0))
    *lt1 = ltd;
}

void check_lt_acc(char *rubr, unsigned short *lt, unsigned short *acc)
{
  rubriktype *hp;

  hp = rubrikroot;
  while (hp != NULL) {
    if (!strcmp(hp->name, rubr)) {
      /* defined boards */
      chk_maxlt(lt, hp->lifetime);
      chk_minlt(lt, hp->min_lifetime);
      *acc = hp->access;
      return;
    }
    hp = hp->next;
  }

  if (callsign(rubr)) {
    /* userboards */
    chk_maxlt(lt, userlifetime);
    *acc = 1;
    return;
  }

  if (strlen(rubr) != 1) {
    /* miscellaneous boards */
    chk_maxlt(lt, default_lifetime);
    chk_minlt(lt, min_lifetime);
    *acc = 1;
    return;
  }
  /* system boards (len 1) */
  chk_maxlt(lt, default_lifetime);
/*  *lt = default_lifetime; */
  chk_minlt(lt, min_lifetime);
  *acc = 10;
}


boolean may_sysaccess(short unr, char *board)
{
  unsigned short lt, acc;

  if (!boxrange(unr))
    return false;

  if (user[unr]->supervisor ||
      (user[unr]->rsysop && in_rsysops_boards(user[unr]->call, board))) {
    check_lt_acc(board, &lt, &acc);
    return (user[unr]->level >= acc);
  }
  return false;
}


void check_msgtype(char *msgtype, char *ziel, char *betreff)
{
  if (*msgtype != '\0')
    return;
  if (!callsign(ziel)) {
    *msgtype = 'B';
    return;
  }
  if (*betreff == '\0') {
    *msgtype = 'P';
    return;
  }
  if (strstr(betreff, "ACK:") != betreff)
    *msgtype = 'P';
  else
    *msgtype = 'A';
}


long truesize(indexstruct rec)
{
  long sz;

  if (strcmp(rec.dest, "M")) {
    sz = rec.size + strlen(rec.betreff) + 64;   /* Laenge des Headers */
    /* CR LF * 3         */
    if (*rec.id != '\0')
      sz += 35;
    if (*rec.rxfrom != '\0')
      sz += strlen(rec.rxfrom) + 26;
    return sz;
  } else
    return 0;
}


/*
diebox:
DL8HBS @DB0GR.DEU.EU de:DL7WA  15.09.91 13:37  70    814 Bytes
SP @DB0GR        de:DL7WA  15.09.91 13:37  70    814 Bytes
wampes:
Msg# 19522   To: ALL @WW   From: N0NJY   Date: 04May93/1559
baycom:
DD6OQ  > MEINUN   04.05.93 13:38 39 Zeilen 1805 Bytes #30 @DL
DD6OQ  > MEINUN   04.05.93 13:38 39 Zeilen 1805 Bytes #30 DB0GV@DL
*/

char separate_status(char *status_, char *ziel, char *absender, char *mbx,
		     char *datum, char *zeit, char *laenge, char *lifetime)
{
  short       x, TEMP;
  cutboxtyp   typ;
  char	      mtype;
  char	      status[256], hs[256], STR7[256];

  strcpy(status, status_);
  cut(status, 200);
  debug(5, 0, 111, status);

  typ 	      = boxheader(status);
  mtype       = '\0';
  *ziel       = '\0';
  *absender   = '\0';
  *mbx	      = '\0';
  *datum      = '\0';
  *zeit       = '\0';
  *laenge     = '\0';
  *lifetime   = '\0';

  if (((1L << ((long)typ)) &
       ((1L << ((long)THEBOX_USER)) | (1L << ((long)NOP)))) != 0) {
    if (count_words(status) != 8)
      return mtype;
    get_word(status, hs);
    cut(hs, LEN_BOARD);
    strcpyupper(ziel, hs);
    get_word(status, hs);
    if (hs[0] == '@')
      strdelete(hs, 1, 1);
    cut(hs, LEN_MBX);
    strcpyupper(mbx, hs);
    get_word(status, hs);
    if (strstr(hs, "de:") == hs)
      strdelete(hs, 1, 3);
    cut(hs, LEN_CALL);
    strcpyupper(absender, hs);
    get_word(status, hs);
    cut(hs, 8);
    strcpy(datum, hs);
    get_word(status, hs);
    cut(hs, 5);
    strcpy(zeit, hs);
    get_word(status, hs);
    cut(hs, 3);
    strcpy(lifetime, hs);
    get_word(status, hs);
    cut(hs, 8);
    strcpy(laenge, hs);

    get_word(status, hs);
    if (strstr(hs, "Byte") != hs) {
      *ziel = '\0';
      *absender = '\0';
    } else {
      if (strpos2(hs, "~", 1) == 6)
	mtype = hs[6];
    }
    return mtype;
  }

  if (typ == BAYCOM_USER) {
    TEMP = count_words(status);
    if (!((unsigned)TEMP < 32 && ((1L << TEMP) & 0xc00) != 0))
      return mtype;
    get_word(status, absender);
    get_word(status, hs);
    get_word(status, ziel);
    get_word(status, datum);
    get_word(status, zeit);
    get_word(status, hs);
    get_word(status, hs);
    get_word(status, laenge);
    get_word(status, hs);
    if (strcmp(hs, "Bytes")) {
      *ziel = '\0';
      *absender = '\0';
      return mtype;
    }
    get_word(status, hs);
    if (*hs == '\0')
      return mtype;
    if (hs[0] == '#') {
      strcpy(lifetime, hs);
      strdelete(lifetime, 1, 1);
      get_word(status, hs);
    }
    if (*hs == '\0')
      return mtype;

    x = strpos2(hs, "@", 1);
    if (x > 0) {
      strdelete(hs, 1, x);
      strcpy(mbx, hs);
    }
    return mtype;
  }

  if (typ != WAMPES_USER)
    return mtype;
  if (strstr(status, "bbs>") == status)
    strdelete(status, 1, 4);
  if (count_words(status) != 9)
    return mtype;
  get_word(status, hs);
  get_word(status, hs);
  get_word(status, hs);
  if (strcmp(hs, "To:"))
    return mtype;
  get_word(status, ziel);
  get_word(status, mbx);
  strdelete(mbx, 1, 1);
  get_word(status, hs);
  get_word(status, absender);
  get_word(status, hs);
  get_word(status, hs);
  strsub(zeit, hs, 9, 4);
  cut(hs, 7);
  if (strlen(zeit) == 4)
    strinsert(":", zeit, 3);
  strcpy(datum, hs);
  strdelete(datum, 3, 3);
  strdelete(hs, 1, 2);
  cut(hs, 3);
  if (!strcmp(hs, "Jan"))
    strcpy(hs, "01");
  else if (!strcmp(hs, "Feb"))
    strcpy(hs, "02");
  else if (!strcmp(hs, "Mar"))
    strcpy(hs, "03");
  else if (!strcmp(hs, "Apr"))
    strcpy(hs, "04");
  else if (!strcmp(hs, "May"))
    strcpy(hs, "05");
  else if (!strcmp(hs, "Jun"))
    strcpy(hs, "06");
  else if (!strcmp(hs, "Jul"))
    strcpy(hs, "07");
  else if (!strcmp(hs, "Aug"))
    strcpy(hs, "08");
  else if (!strcmp(hs, "Sep"))
    strcpy(hs, "09");
  else if (!strcmp(hs, "Oct"))
    strcpy(hs, "10");
  else if (!strcmp(hs, "Nov"))
    strcpy(hs, "11");
  else if (!strcmp(hs, "Dec"))
    strcpy(hs, "12");
  else
    strcpy(hs, "01");
  sprintf(hs, ".%s.", strcpy(STR7, hs));
  strinsert(hs, datum, 3);
  strcpy(laenge, "999999");
  return mtype;
}


void create_status(long additional, boolean hierarchicals, char *name,
		   indexstruct header, char *status)
{
  char hs[256], w[256];

  strcpy(hs, header.verbreitung);
  if (!hierarchicals) {
    unhpath(hs, w);
    sprintf(hs, "%s @%s", name, w);
    rspacing(hs, 17);
  } else
    sprintf(hs, "%s @%s ", name, header.verbreitung);
  sprintf(hs + strlen(hs), "de:%s ", header.absender);
  rspacing(hs, 27);
  strcpy(status, hs);
  ix2string(header.rxdate, hs);
  cut(hs, 14);
  sprintf(status + strlen(status), "%s ", hs);
  sprintf(hs, "%d", header.lifetime);   /* war mal header.lifetime */
  lspacing(hs, 3);
  sprintf(w, "%ld", truesize(header) + additional);
  lspacing(w, 6);
  sprintf(hs + strlen(hs), " %s Bytes", w);
  if (hierarchicals && header.msgtype != '\0')
    sprintf(hs + strlen(hs), "~%c", header.msgtype);
  strcat(status, hs);
}


/* Dies ist speziell fuer den Broadcast - Empfang */

void create_status2(boolean hierarchicals, char *dest, char *absender,
		    long rxdate, long expire_time, long size, char msgtype,
		    char *status)
{
  short k, lifetime;
  long hl;
  char hs[256], w[256];

  debug(4, 0, 112, dest);
  *w = '\0';
  strcpy(hs, dest);
  k = strpos2(hs, "@", 1);
  if (k > 0) {
    strsub(w, hs, k + 1, strlen(hs) - k);
    cut(hs, k - 1);
  }
  del_blanks(hs);
  cut(hs, LEN_BOARD);
  del_blanks(w);
  cut(w, LEN_MBX);

  if (*w == '\0')
    strcpy(w, Console_call);

  if (!hierarchicals) {
    unhpath(w, w);
    sprintf(hs + strlen(hs), " @%s", w);
    rspacing(hs, 17);
  } else
    sprintf(hs + strlen(hs), " @%s ", w);

  strcpy(w, absender);
  k = strpos2(w, "@", 1);
  if (k > 0)   /* bei Pacsats sind auch Absender mit @bbs ueblich */
    cut(w, k - 1);
  del_blanks(w);
  cut(w, LEN_CALL);
  sprintf(hs + strlen(hs), "de:%s ", w);
  rspacing(hs, 27);

  strcpy(status, hs);
  ix2string(rxdate, hs);
  cut(hs, 14);
  sprintf(status + strlen(status), "%s ", hs);

  lifetime = 0;
  if (expire_time > 0) {
    hl = expire_time - clock_.ixtime;
    if (hl >= SECPERDAY)
      lifetime = (hl + SECPERDAY-1) / SECPERDAY;
    else
      lifetime = 1;
  }
  if (lifetime > 999)
    lifetime = 999;
  sprintf(hs, "%d", lifetime);
  lspacing(hs, 3);

  sprintf(w, "%ld", size + 180);   /* 180 Bytes fuer Header... */
  lspacing(w, 6);
  sprintf(hs + strlen(hs), " %s Bytes", w);

  if (hierarchicals && msgtype != '\0')
    sprintf(hs + strlen(hs), "~%c", msgtype);

  strcat(status, hs);
}


static void generate_it(char *s, unsigned short *crc)
{
  while(*s != '\0') crcthp(*s++, crc);
}


void create_userid(char *call, char *absender, char *datum, char *zeit,
		   char *betreff, char *userid)
{
  unsigned short  l;
  char	      	  hcall[256];

  strcpy(hcall, call);
  cut(hcall, LEN_CALL);
  l   	      	  = 0;
  generate_it(absender, &l);
  generate_it(datum, &l);
  generate_it(zeit, &l);
  generate_it(betreff, &l);
  sprintf(userid, "%.5d!", l);
  while (strlen(userid) + strlen(hcall) < LEN_BID) {
    strcat(userid, "_");
  }
  strcat(userid, hcall);
  upper(userid);
  cut(userid, LEN_BID);
}

static time_t lastmailixtime  = 0;

/* this function ensures that each message in the bbs has a different create time */
long messagerxtime(void)
{
  time_t      	  ti;
  
  ti  	      	  = clock_.ixtime;
  if (ti <= lastmailixtime) {
    /* but first check if the counter may have been corrupted by a wrong system date before.  */
    /* checking for +7200 means, that we could only import 7200 messages at once - should be  */
    /* sufficient because import needs some time, and time counter advances while import.     */
    /* The offset of 7200 is important to cover daylight saving time errors and corrections,  */
    /* as it _is_ desirable that all incoming messages use an increasing rx time.             */
    if (lastmailixtime < ti + 7200) {
      ti	  = lastmailixtime + 1;
    }
  }
  lastmailixtime  = ti;
  return ti;
}

long new_msgnum(void)
{
  actmsgnum++;
  if (actmsgnum < 1 || actmsgnum + MSGNUMOFFSET >= maxlonginteger - 10)
    actmsgnum = 1;
  return actmsgnum;
}


static time_t lastbidtick   = 0;

void new_bid(char *bid)
{
  short     x;
  boolean   ccl;
  char	    c1;
  long	    st, lb, nib;

  debug0(3, 0, 113);
  ccl 	    = (LEN_BID - strlen(Console_call) < 7);
  st  	    = clock_.ixtime;
  if (st < lastbidtick) {
    if (lastbidtick > st + 7200) {
      /* this happens with wrong set system times (after correction) */
      lastbidtick = st;
    } else {
      st    = lastbidtick;
    }
  }
  do {
    lb	    = st;
    for (x = 1; x <= 6; x++) {
      nib   = lb % 36;
      lb    /= 36;
      if (nib < 10)
	c1  = (char)(nib + '0');
      else if (nib < 37)
	c1  = (char)(nib - 10 + 'A');
      else
	c1  = '-';
      bid[6 - x] = c1;
    }
    bid[6]  = '\0';

    if (ccl)
      strinsert(Console_call, bid, 7 - sub_bid);
    else {
      if (sub_bid == 0)
	c1  = '_';
      else
	c1  = sub_bid + '0';
      sprintf(bid + strlen(bid), "%c%s", c1, Console_call);
    }

    cut(bid, LEN_BID);
    st++;
  } while (!check_double(bid));
  lastbidtick = st;
}




/* Split S(end)line ... Sucht aus der Send-Angabe des Users (oder der W0RLI- */
/* S&F - Mailbox) die noetigen Angaben heraus                                */

static boolean ordne_sparm_zu(char *inp_, char *inp1_, char *mbx, char *lt,
			      char *bid, char *call2)
{
  char inp[256], inp1[256];
  short k;

  strcpy(inp, inp_);
  strcpy(inp1, inp1_);
  if (*inp == '\0') return false;
  
  switch (inp[0]) {

  case '@':
    strdelete(inp, 1, 1);
    if (strlen(inp) >= 1 && strlen(inp) <= LEN_MBX)
      strcpy(mbx, inp);
    else {
      k = strpos2(inp, ".", 1);
      if (k >= 2 && k <= LEN_MBX) {
	cut(inp, k - 1);
	strcpy(mbx, inp);
      } else {
	cut(inp, LEN_MBX);
	strcpy(mbx, inp);
      }
    }

    if (*mbx != '\0') {
      if (mbx[strlen(mbx) - 1] == '.')
	mbx[strlen(mbx) - 1] = '\0';
    }
    return true;

  case '#':
    if ((unsigned long)strlen(inp) < 32 && /* !!!CHECK */
	((1L << strlen(inp)) & 0x1c) != 0 && isdigit(inp[1])) {
      strdelete(inp, 1, 1);
      strcpy(lt, inp);
      return true;
    } else
      return false;

  case '<':
    if ((unsigned long)strlen(inp) < 32 && ((1L << strlen(inp)) & 0xfc) != 0) { /* !!!CHECK */
      strdelete(inp, 1, 1);
      strcpy(call2, inp);
      return true;
    } else
      return false;

  case '$':
    strdelete(inp1, 1, 1);
    cut(inp, LEN_BID);
    if (*inp != '\0')
      strcpy(bid, inp1);
    return true;

  default:
    return false;
  }
}


static void normalisiere_send(char ch, char *inp, boolean correct, short *p)
{
  short k, l;

  k = *p;
  l = strlen(inp);
  if (l < 1 || l > 200)
    return;

  while (k < l && inp[k - 1] != ch)
    k++;

  if (inp[k - 1] != ch)
    k = 0;

  if (k <= 0)
    return;

  if (k > 1 && correct) {
    if (inp[k - 2] != ' ' && inp[k - 2] != '.') {
      strinsert(" ", inp, k);
      k++;
    }
  }

  if (k == 1 || inp[k - 2] == ' ') {
    while (strlen(inp) > k && inp[k] == ' ')
      strdelete(inp, k + 1, 1);
  }

  *p = k;
  while (*p < strlen(inp) && inp[*p - 1] != ' ')
    (*p)++;

}


void split_sline(char *eingabe_, char *call1, char *call2, char *mbx,
		 char *bid, char *lt, char *betreff)
{
  char eingabe[256];
  short p;
  char hs[256], hs1[256], hs2[256];

  strcpy(eingabe, eingabe_);
  debug(4, 0, 114, eingabe);
  *call1 = '\0';
  *call2 = '\0';
  *mbx = '\0';
  strcpy(lt, "-");
  strcpy(bid, "#NONE#");
  *betreff = '\0';

  p = strlen(eingabe);
  if (p < 1 || p > 200)
    return;

  p = 1;
  normalisiere_send('@', eingabe, true, &p);
  normalisiere_send('<', eingabe, false, &p);
  normalisiere_send('$', eingabe, false, &p);
  normalisiere_send('#', eingabe, false, &p);

  get_word(eingabe, hs);
  if ((unsigned long)strlen(hs) >= 32 || ((1L << strlen(hs)) & 0x1fe) == 0) /* !!!CHECK */
    return;
  upper(hs);
  if (hs[0] == '\032' || hs[0] == '$' || hs[0] == '<' || hs[0] == '#' ||
      hs[0] == '@' || hs[0] == '*')
    return;
  strcpy(call1, hs);

  do {

    strcpy(hs1, eingabe);
    get_word(eingabe, hs);
    strcpy(hs2, hs);
    upper(hs);

  } while (ordne_sparm_zu(hs, hs2, mbx, lt, bid, call2));

  strcpy(eingabe, hs1);
  del_leadblanks(eingabe);
  cut(eingabe, LEN_SUBJECT);
  strcpy(betreff, eingabe);
}


void create_ack(char *absender, char *ackcall, char *dest, char *umleitung,
		char *betreff_)
{
  char betreff[256];
  char hs[256];
  char STR1[80];

  strcpy(betreff, betreff_);
  cut(betreff, LEN_SUBJECT-4);
  sprintf(betreff, "ACK:%s", strcpy(STR1, betreff));
  snprintf(hs, 120, "%s@%s acknowledged (%s %s UTC)",
	  dest, Console_call, clock_.datum4, clock_.zeit);
  if (*umleitung != '\0')
    snprintf(hs + strlen(hs), 120, " - redirected to %s@%s", dest, umleitung);
  cut(hs, LEN_SUBJECT-1);
  send_sysmsg(absender, ackcall, betreff, hs, 0, 'A', 0);
}


void show_prompt(short unr)
{
  short x;
  char *p;
  char prompt[256];
  char flags[256];
  userstruct *WITH;
  char STR1[256];

  if (!boxrange(unr))
    return;
  WITH = user[unr];
  if (*WITH->call == '\0')
    return;

  close_outfile(unr); /* im zweifel doppelt, aber wichtig wegen externer kommandos */
  if (WITH->umode == UM_SINGLEREQ)
    WITH->action = 15;

  wlnuser0(unr);
  *prompt = '\0';
  *flags = '\0';
  
  if (*WITH->tempbid != '\0') { /* vorher SP eingegeben? */
    strcpy(WITH->brett, WITH->tempbid); /* dann Originalrubrik wiederherstellen */
    *WITH->tempbid = '\0';
  }

  if (WITH->smode) {
    strcpylower(prompt, Console_call);
    sprintf(prompt + strlen(prompt), "-server:%s > ", WITH->spath);
    wuser(unr, prompt);
    return;
  }

  if (WITH->hidden)
    strcat(flags, "H,");
  if (WITH->print)
    strcat(flags, "P,");
  if (WITH->supervisor && WITH->console)
    strcat(flags, "S,");
  if (WITH->undef)
    strcat(flags, "U,");
  if (WITH->supervisor && !sf_allowed)
    strcat(flags, "no s&f,");
  if (WITH->supervisor && gesperrt)
    strcat(flags, "no user,");
  if (WITH->supervisor && disk_full)
    strcat(flags, "disk full,");
  if (WITH->supervisor && wrong_clock)
    strcat(flags, "***wrong system time:BBS blocked:set system time***,");
  if (WITH->msgselection != 0)
    strcat(flags, "selection,");
  if (*flags != '\0')
    strdelete(flags, strlen(flags), 1);

  if (*WITH->promptmacro != '\0') {
    strcpy(prompt, WITH->promptmacro);
    expand_macro(unr, prompt);
  } else {
    strcpy(prompt, default_prompt);
    expand_macro(unr, prompt);
    if (WITH->in_begruessung) {
      p = strchr(prompt, 13);
      while (p != NULL) {
        *p = ' ';
        p = strchr(prompt, 13);
      }
      p = strchr(prompt, 10);
      while (p != NULL) {
        *p = ' ';
        p = strchr(prompt, 10);
      }
      x = strlen(prompt) - 1;
      if (x < 1) strcpy(prompt, ">");
      else if (prompt[x] != '>') strcat(prompt, ">"); 
    }
  }

  wuser(unr, prompt);

  if (*flags != '\0') {
    sprintf(flags, " [%s] ", strcpy(STR1, flags));
    wuser(unr, flags);
  }

}


void check_replytitle(char *reply)
{
  short x;
  char hs[256];
  char STR1[256];

  do {
    if ((x = strpos2(reply, "?", 1)) > 0) strdelete(reply, x, 1);
  } while (x > 0);
  strdelete(reply, 1, 3);
  strcpyupper(hs, reply);
  if (strstr(hs, "RE^") == hs) {
    x = strpos2(hs, ":", 1);
    if ((unsigned)x >= 32 || ((1L << x) & 0x70) == 0) {
      sprintf(reply, "RE:%s", strcpy(STR1, reply));
      return;
    }
    strdelete(reply, 1, 3);
    x -= 3;
    sprintf(hs, "%.*s", x - 1, reply);
    strdelete(reply, 1, x);
    x = atoi(hs);
    sprintf(hs, "%d", x + 1);
    sprintf(reply, "RE^%s:%s", hs, strcpy(STR1, reply));
    return;
  }
  if (strstr(hs, "RE:") == hs) {
    strinsert("^2", reply, 3);
    return;
  }
  if (strstr(hs, "RE ") == hs) {
    strdelete(reply, 1, 3);
    sprintf(reply, "RE^2:%s", strcpy(STR1, reply));
  } else
    sprintf(reply, "RE:%s", strcpy(STR1, reply));
}


void get_numbers(short unr, boolean plong, char *eingabe, long *start, long *ende, short *threshold,
		 char *option, char *search)
{
  boolean alphab, numbers, fbb;
  long li;
  short slashct, x, t, y, z;
  char *p;
  char ein[256], hs[256];
  char opt2[256];
  char STR1[256];
  short FORLIM;

  *start = 1;
  *ende = 1;
  *threshold = 100;
  *search = '\0';
  *opt2 = '\0';
  fbb = false;

  x = strpos2(eingabe, "$", 1);
  if (x > 0) {
    if (x == 1 || (x > 1 && eingabe[x - 2] == ' ') || eingabe[x - 2] == '<') {
      if (strlen(opt2) < 12)
	strcat(opt2, "$");
      strdelete(eingabe, x, 1);
    }
  }

  x = strpos2(eingabe, "\"", 1);
  if (x > 0) {
    strsub(hs, eingabe, x + 1, strlen(eingabe) - x + 2);
    t = strpos2(hs, "\"", 1);
    if (t > 0)
      cut(hs, t - 1);
    strcpyupper(search, hs);
    cut(eingabe, x - 1);
    del_lastblanks(eingabe);
  }

  x = strpos2(eingabe, "<", 1);
  if (x > 0) {
    if (*search == '\0') {
      strsub(hs, eingabe, x + 1, strlen(eingabe) - x + 2);
      strcpyupper(search, hs);
      del_leadblanks(search);
      del_lastblanks(search);
    }
    cut(eingabe, x - 1);
    del_lastblanks(eingabe);
  }
  
  x = strpos2(eingabe, "%", 1);
  if (x > 0) {
    if (*search == '\0') {
      *threshold = 50;
      strsub(hs, eingabe, x + 1, strlen(eingabe) - x + 2);
      strcpy(STR1, hs);
      get_word(hs, ein);
      if (!zahl(ein))
        strcpy(hs, STR1);
      else {
        *threshold = atoi(ein);
        if (*threshold > 100) *threshold = 100;
        else if (*threshold < 10) *threshold = 10;
      }
      strcpyupper(search, hs);
      del_leadblanks(search);
      del_lastblanks(search);
      cut(eingabe, x - 1);
      del_lastblanks(eingabe);
    }
  }

  if ((p = strchr(eingabe, '>')) != NULL)   /* Wegen TRANSFER ! */
    *p = ' ';

  for (z = 1; z <= 4; z++) {
    x = strpos2(eingabe, "+", 1);
    if (x > 0) {
      t = strlen(eingabe);
      *hs = '\0';
      y = x;
      y++;
      while (y <= t && eingabe[y - 1] != ' ' && isdigit(eingabe[y - 1])) {
	sprintf(hs + strlen(hs), "%c", eingabe[y - 1]);
	y++;
      }
      if ((unsigned long)strlen(hs) < 32 && ((1L << strlen(hs)) & 0xe) != 0) {
	strdelete(eingabe, x, strlen(hs) + 1);
	while (strlen(hs) < 3)
	  sprintf(hs, "0%s", strcpy(STR1, hs));
	sprintf(opt2 + strlen(opt2), "!&%s", hs);
      } else {
	y = 1;
	if (t > x) {
	  switch (eingabe[x]) {

	  case 'E':
	  case 'e':   /* DieBox-FOR-Befehl */
	    strcat(opt2, "(");
	    break;

	  case 'L':
	  case 'l':   /* dito              */
	    strcat(opt2, ")");
	    break;

	  case ' ':
	    strcat(opt2, "+");
	    break;
	  }
	  y = 2;
	} else
	  strcat(opt2, "+");
	strdelete(eingabe, x, y);
      }
    }
  }

  x = strpos2(eingabe, ":", 1);
  if (x > 0) {
    if (x == 1 || (x > 1 && eingabe[x - 2] == ' ') || eingabe[x - 2] == '<') {
      if (strlen(opt2) < 12)
	strcat(opt2, ":");
      strdelete(eingabe, x, 1);
    }
  }

  get_word(eingabe, ein);
  upper(ein);
  if (*ein != '\0') {   /*Loginsearch*/
    alphab = false;
    numbers = false;
    slashct = 0;

    FORLIM = strlen(ein);
    for (x = 0; x < FORLIM; x++) {
      if (isdigit(ein[x]))
	numbers = true;
      if (ein[x] == '_' || (ein[x] & 255) == 158 || (ein[x] & 255) == 154 ||
	  (ein[x] & 255) == 153 || (ein[x] & 255) == 142 || isupper(ein[x]))
	alphab = true;
      if (ein[x] == '-')
	slashct++;
    }

    if (alphab || (numbers && alphab) || slashct > 1) {
      cut(ein, 8);
      if (valid_boardname(ein)) {
        check_transfers(ein); /* LIST SAT-TV -> LIST SAT */
	strcpy(user[unr]->brett, ein);
      }
      else
	*user[unr]->brett = '\0';
      get_word(eingabe, ein);
      upper(ein);
    } else if (user[unr]->fbbmode && numbers && !alphab && slashct == 0) {
      li = atol(ein);
      if (li > MSGNUMOFFSET) {
	    /* sri, this is needed to distinguish msg numbers from board numbers */
	      li -= MSGNUMOFFSET;
	*start = msgnum2board(li, user[unr]->brett);
	if (*start < 1) {
	  *start = 1;
	  *user[unr]->brett = '\0';
	}
	*ende = *start;
	fbb = true;
	*ein = '\0';
      }
    }


    if (!fbb) {
      if (*ein != '\0') {
	if (strpos2(ein, "-", 1) > 1) {
	  sprintf(hs, "%.*s", strpos2(ein, "-", 1) - 1, ein);
	  *start = atol(hs);
	  if (*start < 1)
	    *start = 1;
	  if (strlen(ein) > strpos2(ein, "-", 1)) {
	    strsub(hs, ein, strpos2(ein, "-", 1) + 1,
		   strlen(ein) - strpos2(ein, "-", 1));
	    *ende = atol(hs);
	  } else
	    *ende = LONG_MAX;
	} else if (ein[0] == '-') {
	  strdelete(ein, 1, 1);
	  *ende = atol(ein);
	  if (*ende < 1)
	    *ende = 1;
	  *start = 1;
	} else {
	  *start = atol(ein);
	  if (*start < 1)
	    *start = 1;
	  *ende = *start;
	}


      } else
	strcat(opt2, "!");

    }

  } else
    strcat(opt2, "!");

  if (*ende < 1)
    *ende = LONG_MAX;
  if (*start < 1)
    *start = 1;
  if (*start > *ende)
    *start = *ende;

  if (*search != '\0') {  /* L ATARI < DP immer von 1- durchsuchen! */
    if (strchr(search, '*') != NULL) {
      if (search[0] != '*')
	sprintf(search, "*%s", strcpy(STR1, search));
      if (search[strlen(search) - 1] != '*')
	strcat(search, "*");
    }
    x = strpos2(opt2, "!", 1);
    if (x > 0) {
      strdelete(opt2, x, 1);
      *start = 1;
      *ende = LONG_MAX;
    }
  }
  cut(opt2, 12);
  strcpy(option, opt2);

  if (!plong) {
    if (*ende > SHORT_MAX) *ende = SHORT_MAX;
    if (*start > SHORT_MAX) *start = SHORT_MAX;
  }

}


static short find_command_x(short unr, char *kommando_, boolean *onlysys,
			    char *cname)
{
  short Result;
  char kommando[256];
  bcommandtype *hp;
  boolean found;
  short cnr, scnr;

  strcpyupper(kommando, kommando_);
  debug(4, unr, 115, kommando);
  Result = 0;
  *cname = '\0';
  *onlysys = false;

  if (*kommando == '\0')
    return Result;
  cnr = 0;
  found = false;

  if ((user[unr]->umode == UM_FILEOUT || user[unr]->umode == UM_SINGLEREQ) && zahl(kommando))
    scnr = atoi(kommando);
  else
    scnr = -1;

  if (kommando[0] == 'S') {
    if (!strcmp(kommando, "SP"))
      strcpy(kommando, "SEND");
    else if (!strcmp(kommando, "SB"))
      strcpy(kommando, "SEND");
    else if (!strcmp(kommando, "SR"))
      strcpy(kommando, "REPLY");
  }

  hp = bcommandroot;
  while (!found && hp != NULL) {
    if (scnr == hp->cnr || strstr(hp->command, kommando) == hp->command) {
      cnr = hp->cnr;
      if (hp->sysop && !user[unr]->supervisor)
	cnr = 0;
      else if (user[unr]->level < hp->ulev)
	cnr = 0;
/*      if (cnr == 99 || cnr == 98) {
	if (strcmp(kommando, hp->command))
	  cnr = 0;
      } */
      found = (cnr != 0);
      if (found) {
	*onlysys = hp->sysop;
	strcpy(cname, hp->command);
      }
    }
    hp = hp->next;
  }

  if (found)
    return cnr;

  *cname = '\0';
  if (user[unr]->sf_level >= 0 && strlen(kommando) > 4 &&
      kommando[0] == '[' && kommando[strlen(kommando) - 1] == ']') {
    if (kommando[strlen(kommando) - 2] == '$')
      cnr = 45;
    return cnr;
  }
  if (strstr(kommando, "#OK#") == kommando) {
    cnr = 77;
    return cnr;
  }
  if (strstr(kommando, "#NO#") == kommando)
    cnr = 77;
  else if (strstr(kommando, "<GP") == kommando)
    cnr = 112;
  return cnr;

}


short find_command(short unr, char *kommando, boolean *onlysys)
{
  char hs[256];

  return (find_command_x(unr, kommando, onlysys, hs));
}


void expand_command(short unr, char *kommando)
{
  char hs[256];
  boolean onlysys;

  find_command_x(unr, kommando, &onlysys, hs);
  strcpy(kommando, hs);
}


short str2disturb_chan(char *w)
{
  short xchan;

  if (callsign(w))
    xchan = actual_user(w);
  else
    xchan = atoi(w);

  if (boxrange(xchan)) {
    if (user[xchan]->action == 0 && !user[xchan]->f_bbs &&
	user[xchan]->lastprocnumber == 0 &&
	boxspoolstatus(user[xchan]->tcon, user[xchan]->pchan, -1) == 0)
      return xchan;
  }
  return 0;
}


short str2disturb_chan2(char *w)
{
  short xchan;

  if (callsign(w))
    xchan = actual_user(w);
  else
    xchan = atoi(w);

  if (boxrange(xchan)) {
    if (!user[xchan]->f_bbs)
      return xchan;
  }
  return 0;
}

static short gzip_ok = 0;

static void check_for_gzip(void)
{
  char tempname[256], tempname2[256], hs[256];
  
  if (gzip_ok != 0) return;
  mytmpnam(tempname);
  strcpy(tempname2, tempname);
  strcat(tempname2, ".gz");
  sfdelfile(tempname2);
  append(tempname, "abcdefghijklmnopqrstuvwxyz", true);
  sprintf(hs, "gzip -fnq5 %s", tempname);
  my_system(hs);
  if (exist(tempname2)) gzip_ok = 2;
  else gzip_ok = 1;
  sfdelfile(tempname);
  sfdelfile(tempname2);
}

boolean use_gzip(long size)
{
  if (gzip_ok == 0) check_for_gzip();
  return (gzip_ok == 2 && size > 2000);
}

/*
=====================Header DB0PIC (Baycom)==========================

DG9FDA > ATARI    18.03.93 16:55 29 Zeilen 1502 Bytes #90 @ALLE
BID : 0833DB0SIFH7
{Read: Callsigns...}
Subj: DP = DAS PR-Programm für ST
Path: !DB0PIC!DB0RBS!DB0LX!DB0AAA!DB0MWE!DB0KCP!OE9XPI!DB0CZ!DB0FRB!DB0GE!
      !DB0LJ!DB0SGL!DB0EAM!DB0SIF!
From: DG9FDA @ DB0SIF.DEU.EU
To  : ATARI @ ALLE

DK5SG:

Msg# 302784   To: HP @ALLE   From: DG1IY   Date: 14Sep92/1518
Subject: PACKET MIT HP48SX ???
Bulletin ID: 139258DB0GV
Path: DB0CZ!OE9XPI!HB9EAS!DB0GE!DB0IZ!DK0MWX!DB0EAM!DB0SIF!DB0GV
de DG1IY @ DB0GV

F6FBB:

Van : GB7BBS voor TPK   @EU
Type/Status : B$
Datum/tijd  : 21-Mrt 13:54
Bericht #   : 72617
Titel       : PERTON:G1DKI} Connected to BNOR71-1

Path: !PI8ZAA!PI8HWB!PI8GWO!PI8VNW!PI8MID!ON1CED!GB7TLH!GB7RUT!GB7BAD!GB7NOT!
      !GB7WRG!GB7YAX!GB7CHS!GB7SAM!GB7MAX!
HOW TO CONNECT USING A NODE


Von        : DK0NET
Nach       : DL8HBS@DB0HB.DEU.EU
Typ/Status : PF
Datum      : 17-Jun 08:23
BID/MID    : 166311DK0NET
Meldung #  : 85079
Titel      : Rubriken?! <dl5hbf

Path: !DK0NET!
 de DK0NET   @ DK0NET.DB0HBS.DEU.EU

 to DL8HBS   @ DL8HBS.DB0GR

 Moin Joachim,

Von        : DG8NBR
Nach       : YAESU @EU
Typ/Status : B$
Datum      : 18-Jun 06:44
BID/MID    : 17630BDB0BOX
Meldung #  : 85385
Titel      : info > FT 530

Path: !DB0HB!DB0HBS!DB0EAM!DB0SIF!DB0HOM!DB0GE!HB9EAS!HB9OS!DB0KCP!DB0AAB!
      !DB0FSG!DB0LNA!DB0RGB!DB0BOX!
de DG8NBR @ DB0BOX

hallo ft 530 user,                      tnx fürs lesen.ich beabsichtige den kauf

Von        : DG8NBR
Nach       : YAESU @EU
Typ/Status : B$
Datum      : 18-Jun 06:44
BID/MID    : 17630BDB0BOX
Meldung #  : 85385
Titel      : info > FT 530

R:930618/0427z @DB0HB  [NORD><LINK HAMBURG, JO43XP, OP:DF4HR/DL6HAZ]
R:930618/0535l @DB0HBS.#HH.DEU.EU [Ûý¤øBBS-Hamburg,JO43TN,DB-ST,OP:DL8XAWø¤ýÛ]
R:930617/2247z @DB0EAM.DEU.EU [Kassel JO41PI TheBox 1·9 OP:DB8AS]
R:930617/2244z @:DB0SIF.DEU.EU
R:930617/2244z @:DB0HOM.#SAR.DEU.EU
R:930617/2243z @DB0GE.#SL.DEU.EU [BBS Saarbruecken, DieBox 1.9]
R:930617/2017z @HB9EAS.CHE.EU [The Basel Area BBS]
R:930617/2015z @HB9OS  [Digital Radio Club Ost-Schweiz (HB9OS)  op:HB9CXN]
R:930617/2012z @:DB0KCP.#BAY.DEU.EU
R:930617/2012z @:DB0AAB.#BAY.DEU.EU
R:930617/2011z @:DB0FSG.#BAY.DEU.EU
R:930617/2011z @:DB0LNA.#BAY.DEU.EU
R:930617/2041z @:DB0RGB.#BAY.DEU.EU
R:930617/1841z @DB0BOX [DIE BOX in NUERNBERG JN59NJ, OP: DC3YC]
de DG8NBR @ DB0BOX

hallo ft 530 user,                      tnx fürs lesen.ich beabsichtige den kauf

*/

cutboxtyp boxheader(char *zeile)
{
  short zoffs, x, zl;
  cutboxtyp typ;
  short dpp;
  char hs[256], w[256];
  short TEMP;

  typ = NOP;
  zl = strlen(zeile);
  if (zl <= 18 || zl >= 255)
    return typ;


  dpp = strpos2(zeile, ":", 1);
  if (dpp <= 4)
    return typ;

  if (zeile[0] == ':' || zeile[0] == '.' || zeile[0] == '|' ||
      zeile[0] == '<' || zeile[0] == '>')
	/* Kommentarzeichen */
	  return typ;

  /*                           Das $#NONE# war ein Fehler in open_sendfile() ...
TEST @DL8HBS.DB0GR.DEU.EU de:DL8HBS 04.05.93 23:20   0 100008 Bytes$#NONE#
SP @DB0GR        de:DL7WA  15.09.91 13:37  70    814 Bytes
*/
  if (zl >= 58) {  /* DieBox */
    zoffs = zl - 58;
    if (zeile[zoffs + 19] == ':') {
      if (strpos2(zeile, "de:", 1) == zoffs + 18) {
	if (strpos2(zeile, "Bytes", 1) == zoffs + 54)
	  typ = THEBOX_USER;
      }
    }
  }

  /*
DD6OQ  > MEINUN   04.05.93 13:38 39 Zeilen 1805 Bytes #30 (DB0GV)@DL
BID : 045307DB0GR
Subj: Gebuehrenerhoehung ok?
Path: !DL0AGU!DB0BRB!DB0GR!
Sent: 930504/1151z @DB0GR.DEU.EU [BERLIN-BBS, TheBox 1.9a, OP: DL7QG]
de DD6OQ @ DB0GR.DEU.EU
to MEINUNG @ DL

DG9NFF > DG9NFF   06.06.94 03:10 11 Lines 631 Bytes #120
DG9NFF > DG9NFF   06.06.94 03:10 11 Lines 80 Bytes #120


*/
  /*
TEST @DL8HBS.DB0GR.DEU.EU de:DL8HBS 04.05.93 23:20   0 100008 Bytes$#NONE#
*/
  if (typ == NOP) {
    if (zl >= 50) {  /* Baycom */
      zoffs = dpp;
      if (zoffs >= 19) {
	if (strpos2(zeile, "Bytes", 1) >= zoffs + 15) {
	  x = strpos2(zeile, ".", 1);
	  if (x > 9 && x < zoffs - 7) {
	    if (strpos2(zeile, "de:", 1) == zoffs - 2)
	      typ = THEBOX_USER;
	    else
	      typ = BAYCOM_USER;
	  }
	}
      }
    }
  }

  /*
Msg# 19522   To: ALL @WW   From: N0NJY   Date: 04May93/1559
Subject: DONT READ MSG FRM YV5KWS!!!
Bulletin ID: 28099_KA0WIN
Path: DB0BBX!DB0ERF!DB0EAM!DB0GV!DK0MTV!DB0GE!DB0IZ!PI8HRL!ON1KGX!ON1KPU!ON4TOR!
7KLY!GB7YAX!GB7WRG!GB7NOT!GB7
BAD!GB7RUT!GB7PET!GB7MHD!GB7KHW!GB7ZPU!GB7SPV!GB7DAA!GB7HSN!WA2NDV!N2BQF!KA0WIN!
KA0WIN

From: N0NJY@KA0WIN.#SECO.CO.USA.NA
To  : ALL@WW
*/
  if (typ == NOP) {  /* WAMPES */
    if (zl >= 57) {
      zoffs = strpos2(zeile, "Msg#", 1);
      if (zoffs > 0) {
	if (strpos2(zeile, "To:", 1) >= zoffs + 13) {
	  if (strpos2(zeile, "From:", 1) > zoffs + 26) {
	    if (strpos2(zeile, "Date:", 1) > zoffs + 40)
	      typ = WAMPES_USER;
	  }
	}
      }
    }
  }

  /*
Date: 28 Oct 91 13:32
Message-ID: <0@EA2RCG>
From: EB2DJM@EA2RCG
To: ALL@EU
Subject: I need the schemas of FT-73R(yaesu)
*/
  /*
                  if typ = NOP then if dpp > 4 then begin { W0RLI }
                      if pos('Date:',zeile) = dpp-4 then begin
                          if zeile[14+dpp] = ':' then begin
                              if count_words(zeile) = 5 then typ := W0RLI_USER;
                          end;
                      end;
                  end;
  */
  /*
From    : PI8TMA @ PI8TMA.#GLD.NLD.EU
To      : SCIENC @ NLDNET
Date    : 911127/1645
Msgid   : B+ 2252@PI8TMA, 31885@PI8DAZ $SCIENC.220
Subject : Bewoners Biosphere bestrijden kooldioxyde.
Path    : PI8UTR!PI8TMA
*/
  /*
                  if typ = NOP then if dpp > 8 then begin { AA4RE }
                      if zl > 15 then begin
                          if zeile[dpp-5] = 'm' then begin
                              if pos('From    : ',zeile) = dpp-8 then begin
                                  if count_words(zeile) = 5 then begin
                                      if pos('@',zeile) > dpp+2 then typ := AA4RE_USER;
                                  end;
                              end;
                          end;
                      end;
                  end;
  */
  /*

Van : DC6OQ  voor IBM   @DL
Type/Status : B$
Datum/tijd  : 21-Mrt 13:55
Bericht #   : 72618
Titel       : hilfe aastor
*/
  if (typ == NOP) {  /* F6FBB */
    if (*fbb_fromfield != '\0') {
      if (fbb_fromfield[0] == zeile[0]) {
	if (fbb_fromfield[1] == zeile[1]) {
	  if (strpos2(zeile, fbb_fromfield, 1) == 1) {
	    x = strpos2(zeile, fbb_tofield, 1);
	    if (x > strlen(fbb_fromfield) + 4) {
	      strcpy(hs, zeile);
	      strdelete(hs, x, strlen(fbb_tofield));
	      strdelete(hs, 1, strlen(fbb_fromfield));
	      del_leadblanks(hs);
	      TEMP = count_words(hs);
	      if ((unsigned)TEMP < 32 && ((1L << TEMP) & 0xc) != 0) {
		get_word(hs, w);
		if (callsign(w))
		  typ = F6FBB_USER;
	      }
	    }
	  }
	}
      }
    }
  }

  /*
Von        : DG8NBR
Nach       : YAESU @EU
Typ/Status : B$
Datum      : 18-Jun 06:44
BID/MID    : 17630BDB0BOX
Meldung #  : 85385
Titel      : info > FT 530
*/
  if (typ != NOP)  /* F6FBB514 */
    return typ;

  if (*fbb514_fromfield == '\0')
    return typ;
  if (fbb514_fromfield[0] != zeile[0])
    return typ;
  if (fbb514_fromfield[1] != zeile[1])
    return typ;
  if (strpos2(zeile, fbb514_fromfield, 1) != 1)
    return typ;
  strcpy(hs, zeile);
  strdelete(hs, 1, strlen(fbb514_fromfield));
  del_leadblanks(hs);
  if (count_words(hs) != 1)
    return typ;
  get_word(hs, w);
  if (callsign(w))
    typ = F6FBB_USER_514;
  return typ;
}


/* ******** der Boxconvers ************** */

static boolean valid_convchan(short unr, long newchan)
{
  return (newchan >= 0 &&
	  (newchan < 100000L || (user[unr]->rsysop && newchan < 200000L) ||
	   user[unr]->supervisor));

}


static void send_convers(short unr, char *zeile)
{
  short x;
  long uch;
  char hs[256], hs2[256];
  char acall[256];
  char STR7[256];
  userstruct *WITH;

  strcpylower(acall, user[unr]->call);
  uch = user[unr]->convchan;
  hs2[0] = '\0';
  sprintf(acall, "<%s>:", strcpy(STR7, acall));
  rspacing(acall, LEN_CALL + 4);
  sprintf(hs, "%s%s", acall, zeile);
  if (strlen(hs) > 79) {
    x = 79;
    while (hs[x - 1] != ' ' && x > 1)
      x--;
    if (x == 1)
      x = 79;
    strcpy(hs2, hs);
    cut(hs, x);
    strdelete(hs2, 1, x);
    sprintf(hs2, "%s%s", acall, strcpy(STR7, hs2));
    cut(hs2, 120);
  }
  del_lastblanks(hs);
  del_lastblanks(hs2);
  for (x = 1; x <= MAXUSER; x++) {
    if (user[x] != NULL) {
      WITH = user[x];
      if (x != unr) {
	if (WITH->convchan >= 0 && WITH->convchan == uch && WITH->action == 90) {
	  wlnuser(x, hs);
	  if (*hs2 != '\0')
	    wlnuser(x, hs2);
	}
      }
    }
  }
}


static void conv_help(short unr)
{
  short x;

  for (x = 126; x <= 132; x++)
    wln_btext(unr, x);
  wlnuser(unr, "***");
}


static void conv_who(short unr, long cn)
{
  short x;
  char hs[256];
  char acall[256];
  userstruct *WITH;
  char STR1[256];

  if (cn >= 0)
    wln_btext(unr, 133);
  else
    wln_btext(unr, 134);
  for (x = 1; x <= MAXUSER; x++) {
    if (user[x] != NULL) {
      WITH = user[x];
      if (valid_convchan(unr, WITH->convchan)) {
	if (cn < 0 || cn == WITH->convchan) {
	  strcpylower(acall, WITH->call);
	  rspacing(acall, LEN_CALL+3);
	  if (cn < 0) {
	    sprintf(hs, "%ld", WITH->convchan);
	    lspacing(hs, 7);
	  } else
	    *hs = '\0';
	  sprintf(hs, "%s%s", acall, strcpy(STR1, hs));
	  wlnuser(unr, hs);
	}
      }
    }
  }
  wlnuser(unr, "***");
}


static void conv_quit(short unr)
{
  send_convers(unr, "*** logging off");
  user[unr]->convchan = -1;
  wln_btext(unr, 135);
  user[unr]->action = 0;
}


static void conv_channel(short unr, long newchan)
{
  char hs[256];
  char STR1[256];

  if (!valid_convchan(unr, newchan)) {
    wln_btext(unr, 137);
    return;
  }
  sprintf(hs, "%ld", newchan);
  if (newchan >= 100000L)
    strcpy(hs, "*** logging off");
  else
    sprintf(hs, "*** switching to chan %s", strcpy(STR1, hs));
  send_convers(unr, hs);
  user[unr]->convchan = newchan;
  send_convers(unr, "*** logging on");
  sprintf(hs, "%ld", newchan);
  w_btext(unr, 136);
  chwuser(unr, 32);
  wlnuser(unr, hs);
  conv_who(unr, newchan);
}


static boolean check_for_conv_command(short unr, char *zeile)
{
  char w[256];
  char hs[256];

  strcpyupper(hs, zeile);
  get_word(hs, w);
  if (strlen(w) < 1)
    return false;
  if (compare(w, "/HELP")) {
    conv_help(unr);
    return true;
  }
  if (compare(w, "/WHO")) {
    conv_who(unr, -1);
    return true;
  }
  if (compare(w, "/QUIT")) {
    conv_quit(unr);
    return true;
  }
  if (compare(w, "/EXIT")) {
    conv_quit(unr);
    return true;
  }
  if (!compare(w, "/CHANNEL"))
    return false;
  conv_channel(unr, atol(hs));
  return true;
}


void send_conv(short unr, char *s)
{
  if (!boxrange(unr))
    return;
  if (!check_for_conv_command(unr, s))
    send_convers(unr, s);
}


void box_convers(short unr, char *cmd)
{
  long i;
  char hs[256];

  if (!boxrange(unr))
    return;
  i = atol(cmd);
  if (!valid_convchan(unr, i)) {
    wln_btext(unr, 137);
    return;
  }
  user[unr]->convchan = i;
  user[unr]->action = 90;
  w_btext(unr, 138);
  chwuser(unr, 32);
  sprintf(hs, "%ld", i);
  wlnuser(unr, hs);
  conv_who(unr, i);
  send_conv(unr, "*** logging on");
}


void _box_sub_init(void)
{
  static int _was_initialized = 0;
  if (_was_initialized++)
    return;

  updbidseekcounter = 0;
  unproto_final_update = 0;
  request_count = 0;
  last_requesttime = 0;
  setup_dwrstring();
}



/* End. */
