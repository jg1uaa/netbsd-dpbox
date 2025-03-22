/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#define BOX_INOU_G
#include "box_inou.h"
#include "yapp.h"
#include "boxlocal.h"
#include "box_sub.h"
#include "box_sf.h"
#include "box.h"
#include "box_logs.h"
#include "box_file.h"
#include "box_send.h"
#include "box_tim.h"
#include "box_sys.h"
#include "box_mem.h"
#include "box_rout.h"
#include "tools.h"
#include "shell.h"


/* ========================================================================= */
/* Der eher dem Output zuzuordnende Teil der Box:                            */
/* ========================================================================= */

short create_outfile2(short unr)
{
  char s[256];
  
  if (user[unr]->fileout_name != NULL) {
    sfclosedel(&user[unr]->fileout_handle);
    free(user[unr]->fileout_name);
    user[unr]->fileout_name = NULL;
  }
  mytmpnam(s);
  if ((user[unr]->fileout_name = malloc(strlen(s)+1)) != NULL) {
    if ((user[unr]->fileout_handle = sfcreate(s, FC_FILE)) >= minhandle) {
      strcpy(user[unr]->fileout_name, s);
      return 1;
    } else {
      free(user[unr]->fileout_name);
      user[unr]->fileout_name = NULL;
      return 0;
    }
  } else
    return 0;
  
}

void close_outfile2(short unr)
{
  if (user[unr]->fileout_name != NULL) {
    sfclose(&user[unr]->fileout_handle);
    boxspool(user[unr]->tcon, user[unr]->pchan, boxprn2bios(user[unr]->print),
    		 user[unr]->fileout_name, true);    
    free(user[unr]->fileout_name);
    user[unr]->fileout_name = NULL;
  }
}


/* ************************** Die Trace-Rueckkopplung ************************ */

static void x_Vwuser(const short unr, const char *s, const boolean crlf, const boolean in_trace);

static void fill_traceheader(boolean userinp, short unr, char *hs, boolean cr)
{
  char direction;

  utc_clock();

  if (userinp)
    direction = '>';
  else
    direction = '<';

  sprintf(hs, "%2d%3d %s %s%c", unr, user[unr]->action, clock_.zeit, user[unr]->call, direction);
  rspacing(hs, 23);
}


void trace_string(boolean userinp, short unr, short trace, const char *s,
		  boolean cr)
{
  char hs[256], s2[256];

  box_ttouch(trace);

  if (user[trace]->fulltrace) {
    if (cr)
      x_Vwuser(trace, s, true, true);
    else
      x_Vwuser(trace, s, false, true);
    return;
  }

  if (!cr) {
    if (strlen(user[unr]->tracefract) + strlen(s) <= 255)
      strcat(user[unr]->tracefract, s);
    return;
  }

  if (userinp && *user[unr]->tracefract != '\0') {
    *hs = '\0';
    trace_string(false, unr, trace, hs, true);	/* loest tracefract-Aussendung aus */
  }

  fill_traceheader(userinp, unr, hs, cr);
  x_Vwuser(trace, hs, false, true);

  if (*user[unr]->tracefract == '\0' && strlen(s) <= 56) {
    x_Vwuser(trace, s, true, true);
    return;
  }

  if (strlen(user[unr]->tracefract) + strlen(s) <= 255)
    sprintf(s2, "%s%s", user[unr]->tracefract, s);
  else
    strcpy(s2, s);
  *user[unr]->tracefract = '\0';

  while (*s2 != '\0') {
    sprintf(hs, "%.56s", s2);
    strdelete(s2, 1, 56);
    x_Vwuser(trace, hs, true, true);
    if (*s2 == '\0')
      break;
    *hs = '\0';
    rspacing(hs, 23);
    x_Vwuser(trace, hs, false, true);
  }

}


static void trace_buf(boolean userinp, short unr, short trace, char *p1,
		      long s1)
{
  char hs[256], w[256];

  if (s1 <= 0 || p1 == NULL)
    return;

  box_ttouch(trace);

  *hs = '\0';
  if (*user[unr]->tracefract != '\0')
    trace_string(false, unr, trace, hs, true);

  if (!user[trace]->fulltrace) {
    fill_traceheader(userinp, unr, hs, false);
    sprintf(w, "%ld", s1);
    sprintf(hs + strlen(hs), "<%s>", w);
    x_Vwuser(trace, hs, true, true);
    return;
  }

  x_show_puffer(trace, p1, s1, true, false);
}


/* ************************** Die Ausgaberoutinen **************************** */


/* Loescht den Spooler    */

void abort_useroutput(short unr)
{
  userstruct *WITH;

  debug0(2, unr, 104);
  if (boxrange(unr)) {
    WITH = user[unr];
    boxspoolabort(WITH->tcon, WITH->pchan, boxprn2bios(WITH->print));
  }
}

/* gibt einen Speicherbereich auf den Spooler aus. Sucht nach dem #BIN#- */
/* Start, um den Wechsel von Strip-LF zu LF-Sendung mitzubekommen        */

void x_show_puffer(short unr, char *base, long size, boolean in_trace,
		   boolean transparent)
{
  long err;
  short k;
  char hs[256];
  userstruct *WITH;

  if (!boxrange(unr)) return;

  upd_statistik(unr, 0, size, 0, 0);

  WITH = user[unr];
  if (!in_trace && WITH->trace_to > 0 && WITH->trace_to != unr)
    trace_buf(false, unr, WITH->trace_to, base, size);

  if (WITH->tell >= minhandle) {
    sfwrite(WITH->tell, size, base);
    return;
  }

  if (WITH->umode == UM_FILEOUT || WITH->umode == UM_SINGLEREQ || WITH->umode == UM_FILESF) {
    if (WITH->fileout_handle >= minhandle)
      sfwrite(WITH->fileout_handle, size, base);
    return;
  }

  if (WITH->action == 76) { /* EXPORT */
    k = atoi(WITH->input2);
    sfwrite(k, size, base);
    return;
  }

  if (transparent) {
    boxmemspool(WITH->tcon, WITH->pchan, boxprn2bios(WITH->print), false, base, size);
    return;
  }

  err = get_binstart(base, size, hs);

  if (err <= 0) {
    boxmemspool(WITH->tcon, WITH->pchan, boxprn2bios(WITH->print), true, base, size);
    return;
  }

  boxmemspool(WITH->tcon, WITH->pchan, boxprn2bios(WITH->print), true, base, err);
  boxspoolfend(WITH->pchan);
  boxmemspool(WITH->tcon, WITH->pchan, boxprn2bios(WITH->print), false,
	      (char *) (&base[err]), size - err);

}


/* zeilenweise Ausgabe ueber den Spooler  */

void x_Vwuser(const short unr, const char *s, const boolean crlf, const boolean in_trace)
{
  short k;
  long l;
  userstruct *WITH;

  if (!boxrange(unr))
    return;
  
  l = strlen(s);
  if (crlf) l++;
  upd_statistik(unr, 0, l, 0, 0);

  WITH = user[unr];
  if (!in_trace && WITH->trace_to > 0 && WITH->trace_to != unr)
    trace_string(false, unr, WITH->trace_to, s, crlf);

  if (WITH->tell >= minhandle) {
    str2file(&WITH->tell, s, crlf);
    return;
  }

  if (WITH->umode == UM_FILEOUT || WITH->umode == UM_SINGLEREQ || WITH->umode == UM_FILESF) {
    if (WITH->fileout_handle >= minhandle) {
      str2file(&WITH->fileout_handle, s, crlf);
    } else if (WITH->umode == UM_FILESF) { /* this happens after forward conversion */
      if (strlen(WITH->spath) + strlen(s) < LEN_PATH) strcat(WITH->spath, s);
      else strcpy(WITH->spath, s);
      immediate_extcheck = WITH->spath != '\0';
    }
    return;
  }

  if (WITH->action == 76) {  /* export... */
    k = atoi(WITH->input2);
    str2file(&k, s, crlf);
  } else
    boxspool(WITH->tcon, WITH->pchan, boxprn2bios(WITH->print), s, crlf);
}

void wuser(const short unr, const char *s)
{
  x_Vwuser(unr, s, false, false);
}

void wlnuser(const short unr, const char *s)
{
  x_Vwuser(unr, s, true, false);
}

void wlnuser0(const short unr)
{
  x_Vwuser(unr, "", true, false);
}

void lwuser(const short unr, const long value)
{
  char s[50];

  sprintf(s, "%ld", value);
  x_Vwuser(unr, s, false, false);
}

void swuser(const short unr, const short value)
{
  char s[50];

  sprintf(s, "%d", value);
  x_Vwuser(unr, s, false, false);
}

/* funny, but it appears that, if sending only one char via   */
/* x_Vwuser, we have a problem in packed forward      	      */
/* Maybe because of boxspool <> boxmemspool. So, here again   */
/* unrolled...	      	      	      	      	      	      */

void chwuser(const short unr, char c)
{
  short     k;
  char	    s[256];
  userstruct *WITH;

  if (!boxrange(unr))
    return;

  upd_statistik(unr, 0, 1, 0, 0);   /* ein Byte mehr */

  WITH	= user[unr];
  if (WITH->trace_to > 0 && WITH->trace_to != unr) {
    sprintf(s, "%c", c);
    trace_string(false, unr, WITH->trace_to, s, false);
  }

  if (WITH->tell >= minhandle) {
    sfwrite(WITH->tell, 1, &c);
    return;
  }
  
  if (WITH->umode == UM_FILEOUT || WITH->umode == UM_SINGLEREQ || WITH->umode == UM_FILESF) {
    if (WITH->fileout_handle >= minhandle) {
      sfwrite(WITH->fileout_handle, 1, &c);
    } else if (WITH->umode == UM_FILESF) { /* this happens after forward conversion */
      sprintf(s, "%c", c);    
      if (strlen(WITH->spath) + 1 < LEN_PATH) strcat(WITH->spath, s);
      else strcpy(WITH->spath, s);
      immediate_extcheck = WITH->spath != '\0';
    }
    return;
  }

  if (WITH->action == 76) {  /* export... */
    k = atoi(WITH->input2);
    sfwrite(k, 1, &c);
  } else
    boxmemspool(WITH->tcon, WITH->pchan, boxprn2bios(WITH->print), false, &c, 1);
}



/* ========================================================================= */
/* Der eher dem Input zuzuordnende Teil der Box:                             */
/* ========================================================================= */


/* Die Abbruchroutine fuer die Box. Wird benutzt bei Disconnect oder  */
/* auch geschlossenem Terminal, aufgerufen vom PR-Terminal            */

void abort_box(short unr, boolean save)
{
  userstruct *WITH;

  debug0(2, unr, 105);
  if (!boxrange(unr))
    return;
  WITH = user[unr];
  abort_useroutput(unr);
  sfclosedel(&WITH->sendchan);
  WITH->input2[0] = '\0';
  melde_user_ab(unr, save);
}


void box_timing(long tct)
{
  box_timing2(tct);
}


void sort_new_mail(short unr, char *pattern, char *rcall)
{
  sort_new_mail2(unr, pattern, rcall);
}


void box_rawinput(short unr, unsigned short infosize,
		  unsigned short *infstart, char *info)
{
  char *p;
  long hsize;
  userstruct *WITH;

  current_unr = unr;
  strcpy(current_command, "BOX_RAWINPUT()");
  *current_user = '\0';

  if (infosize > 256 || infosize < *infstart || *infstart <= 0) {
    debug(0, unr, 83, "infosize/infstart invalid");
    *infstart = SHORT_MAX;
    return;
  }

  if (!boxrange(unr))
    return;
  strcpy(current_user, user[unr]->call);
  if ((unsigned)user[unr]->action < 32 &&
      ((1L << user[unr]->action) & 0xc000L) != 0) {
    *infstart = SHORT_MAX;
    return;
  }

  WITH = user[unr];

  if (WITH->trace_to > 0 && WITH->trace_to != unr) {
    p = info;
    trace_buf(true, unr, WITH->trace_to, (char *)(&p[*infstart - 1]),
	      infosize - *infstart + 1);
  }
  p = &info[*infstart - 1];
  hsize = infosize - *infstart + 1;

  upd_statistik(unr, hsize, 0, 0, 0);
  box_ttouch(unr);

  switch (user[unr]->action) {

  case 96:   /* YAPP - Upload */
    yapp_responses(unr, p, hsize);
    break;

  case 97:
  case 98:
    yapp_input(unr, p, hsize);
    break;

  case 99:
    if (write_pty(unr, hsize, p) == 0) {  /* SHELL und externe Kommandos */
      boxsetrwmode(user[unr]->pchan, RW_NORMAL);
      user[unr]->action = 0;
    }
    break;
  }

  *infstart = infosize + 1;

}


void fbbpack(short unr, unsigned short infosize, unsigned short *infstart,
	     char *info)
{
  char *p;
  userstruct *WITH;

  current_unr = unr;
  strcpy(current_command, "FBBPACK()");
  *current_user = '\0';

  if (infosize > 256 || infosize < *infstart || *infstart <= 0) {
    debug(0, unr, 83, "infosize/infstart invalid");
    *infstart = SHORT_MAX;
    return;
  }

  if (!boxrange(unr))
    return;

  strcpy(current_user, user[unr]->call);
  if ((unsigned)user[unr]->action < 32 &&
      ((1L << user[unr]->action) & 0xc000L) != 0) {
    *infstart = SHORT_MAX;
    return;
  }

  WITH = user[unr];

  if (WITH->trace_to > 0 && WITH->trace_to != unr) {
    p = info;
    trace_buf(true, unr, WITH->trace_to, (char *)(&p[*infstart - 1]),
	      infosize - *infstart + 1);
  }
  /* schuetzt sich selber gegen unsinnige Eingaben */
  fbbpack2(unr, infosize, infstart, info);
}



void fbb2pack(short unr, unsigned short infosize, unsigned short *infstart,
	      char *info)
{
  char *p;
  userstruct *WITH;

  current_unr = unr;
  strcpy(current_command, "FBB2PACK()");
  *current_user = '\0';

  if (infosize > 256 || infosize < *infstart || *infstart <= 0) {
    debug(0, unr, 84, "infosize/infstart invalid");
    *infstart = SHORT_MAX;
    return;
  }

  if (boxrange(unr)) {
    strcpy(current_user, user[unr]->call);
    if ((unsigned)user[unr]->action < 32 &&
	((1L << user[unr]->action) & 0xc000L) != 0) {
      *infstart = SHORT_MAX;
      return;
    }

    WITH = user[unr];
    if (WITH->trace_to > 0 && WITH->trace_to != unr) {
      p = info;
      trace_buf(true, unr, WITH->trace_to, (char *)(&p[*infstart - 1]),
		infosize - *infstart + 1);
    }
  }

  /* schuetzt sich selber gegen unsinnige Eingaben, trotzdem obige Sicherheitsabfragen */

  fbb2pack2(unr, infosize, infstart, info);
}


/* ------------------------ UNPROTO LIST REQUEST -------------------- */


void raw_unproto_request(short pid, short callcount, short heardfrom,
			 char *port, char (*calls)[10], long len, char *buf)
{
  short x;
  char path[256];
  char s[260];

  current_unr = 0;
  strcpy(current_command, "UNPROTO REQUEST: ");
  strcpy(current_user, calls[0]);

  if (len < 1)
    return;
  if (len > 256)
    return;
  if (callcount < 2)
    return;
  if (callcount > 2 && heardfrom < callcount)
    return;
  if (pid != 0xf0)
    return;
  if (boxboxssid() == 0) {
    if (strcmp(calls[1], Console_call))
      return;
  } else {
    sprintf(path, "%s-%d", Console_call, boxboxssid());
    if (strcmp(calls[1], path))
      return;
  }

  *path = '\0';
  if (callcount > 2)
    strcpy(path, calls[2]);
  for (x = 3; x < callcount; x++)
    sprintf(path + strlen(path), " %s", calls[x]);

  for (x = 0; x < len; x++)
    s[x] = (char)buf[x];
  s[len] = '\0';

  strcat(current_command, s);
  
  unproto_request1(calls[0], port, path, s);
}


/* ------------------------------------------------------------------ */


void box_get_next_input(void)
{
  boxintype *hpbi;
  short unr, olda;
  boolean was_server;
  userstruct *WITH;
  char STR1[256];

  unr = 1;

  do {

    if (user[unr] != NULL && user[unr]->inputroot != NULL &&
	user[unr]->wait_pid <= 0 &&
	(user[unr]->f_bbs ||
	 boxspoolstatus(user[unr]->tcon, user[unr]->pchan, -1) <= MAXUSERSPOOL)) {
      WITH = user[unr];
      WITH->cputime = get_cpuusage();

_L1:
      if ((unsigned)WITH->action < 32 && ((1L << WITH->action) & 0xc000L) != 0) {
	sfclosedel(&WITH->prochandle);
	WITH->lastprocnumber = 0;
	WITH->lastprocnumber2 = 0;
	while (WITH->inputroot != NULL) {
	  hpbi = WITH->inputroot;
	  WITH->inputroot = WITH->inputroot->next;
	  free(hpbi);
	}
      }

      if (WITH->inputroot != NULL) {
	olda = 0;
	was_server = false;
	if (WITH->smode || WITH->action == 90 || WITH->action == 80)
	{  /* server / talk / converse */
	  if (WITH->inputroot->line[0] == '!') {
	    olda = WITH->action;
	    WITH->action = 0;
	    was_server = WITH->smode;
	    WITH->smode = false;
	    strdelete(WITH->inputroot->line, 1, 1);
	    if (WITH->lastprocnumber == 0) {
	      if (was_server)
		wlnuser(unr, "[server off]");
	      else if (olda == 80)
		wlnuser(unr, "[talk off]");
	      else
		wlnuser(unr, "[converse off]");
	    }
	  }
	}

        current_unr = unr;
        strcpy(current_command, WITH->inputroot->line);
        strcpy(current_user, WITH->call);
	box_ttouch(unr);
	WITH->in_begruessung = WITH->inputroot->in_begruessung;
	box_command_fract(unr, WITH->inputroot->line,
			  WITH->inputroot->return_);
	if (user[unr] != NULL) {
		
	  if (*WITH->inputroot->line == '\0') {
	    if (WITH->inputroot->in_begruessung) {
	      WITH->in_begruessung = false;
	      if (WITH->action == 0)
	        wlnuser0(unr);
	    }
	    sfclosedel(&WITH->prochandle);
	    WITH->lastprocnumber = 0;
	    WITH->lastprocnumber2 = 0;
	    hpbi = WITH->inputroot;
	    WITH->inputroot = WITH->inputroot->next;
	    free(hpbi);

	    if (was_server || olda > 0) {
	      if (WITH->action != 0) {
		if (was_server)
		  was_server = false;
		else if (olda == 80)
		  end_talk_mode(WITH->talk_to, true);
		else if (olda == 90) {
		  send_conv(unr, "*** logging out");
		  WITH->convchan = -1;
		}
		olda = WITH->action;
	      } else {
		wlnuser0(unr);
		wlnuser0(unr);
		if (was_server)
		  wlnuser(unr, "[server on]");
		else if (olda == 80)
		  wlnuser(unr, "[talk on]");
		else
		  wlnuser(unr, "[converse on]");
	      }
	      WITH->action = olda;
	      WITH->smode = was_server;
	    }

	  } else if (was_server || olda > 0) {
	    sprintf(STR1, "!%s", WITH->inputroot->line);
	    strcpy(WITH->inputroot->line, STR1);
	    WITH->action = olda;
	    WITH->smode = was_server;
	  }


	  if (WITH->inputroot != NULL && get_cpuusage() - WITH->cputime < ttask) {
	    if (WITH->f_bbs ||
		boxspoolstatus(WITH->tcon, WITH->pchan, -1) <= MAXUSERSPOOL) {
	      if (WITH->wait_pid <= 0)
		goto _L1;
	    }
	  }
	}
      }


      if (user[unr] != NULL && WITH->inputroot == NULL &&
	  WITH->tell >= minhandle)
	stop_tell(unr);
      if (user[unr] != NULL) {
	upd_statistik(unr, 0, 0, WITH->cputime, get_cpuusage());
	WITH->cputime = 0;
      }

    }

    unr++;

  } while (unr <= MAXUSER);
}



void clear_immediately_input(short unr)
{
  boxintype *hpbi;
  short olda;

  boolean was_server;
  userstruct *WITH;
  char STR1[256];

  if (!boxrange(unr))
    return;

  WITH = user[unr];

  WITH->cputime = get_cpuusage();

  if ((unsigned)WITH->action < 32 && ((1L << WITH->action) & 0xc000L) != 0) {
    WITH->lastprocnumber = 0;
    WITH->lastprocnumber2 = 0;
    sfclosedel(&WITH->prochandle);
    while (WITH->inputroot != NULL) {
      hpbi = WITH->inputroot;
      WITH->inputroot = WITH->inputroot->next;
      free(hpbi);
    }
  }

  while (WITH->inputroot != NULL && WITH->wait_pid <= 0) {
    olda = 0;
    was_server = false;
    if (WITH->smode || WITH->action == 90 || WITH->action == 80)
    {  /* server / talk / converse */
      if (WITH->inputroot->line[0] == '!') {
	olda = WITH->action;
	WITH->action = 0;
	was_server = WITH->smode;
	WITH->smode = false;
	strdelete(WITH->inputroot->line, 1, 1);
	if (WITH->lastprocnumber == 0) {
	  if (was_server)
	    wlnuser(unr, "[server off]");
	  else if (olda == 80)
	    wlnuser(unr, "[talk off]");
	  else
	    wlnuser(unr, "[converse off]");
	}
      }
    }

    current_unr = unr;
    strcpy(current_command, WITH->inputroot->line);
    strcpy(current_user, WITH->call);
    box_ttouch(unr);
    WITH->in_begruessung = WITH->inputroot->in_begruessung;
    box_command_fract(unr, WITH->inputroot->line, WITH->inputroot->return_);
    if (user[unr] == NULL)
      continue;

    if (*WITH->inputroot->line != '\0') {
      if (WITH->smode || olda > 0) {
	sprintf(STR1, "!%s", WITH->inputroot->line);
	strcpy(WITH->inputroot->line, STR1);
	WITH->action = olda;
	WITH->smode = was_server;
      }
      continue;
    }


    if (WITH->inputroot->in_begruessung) {
      WITH->in_begruessung = false;
      if (WITH->action == 0)
        wlnuser0(unr);
    }
    sfclosedel(&WITH->prochandle);
    WITH->lastprocnumber = 0;
    WITH->lastprocnumber2 = 0;
    hpbi = WITH->inputroot;
    WITH->inputroot = WITH->inputroot->next;
    free(hpbi);

    if (!(was_server || olda > 0))
      continue;

    if (WITH->action != 0) {
      if (was_server)
	was_server = false;
      else if (olda == 80)
	end_talk_mode(WITH->talk_to, true);
      else if (olda == 90) {
	send_conv(unr, "*** logging out");
	WITH->convchan = -1;
      }
      olda = WITH->action;
    } else {
      wlnuser0(unr);
      wlnuser0(unr);
      if (was_server)
	wlnuser(unr, "[server on]");
      else if (olda == 80)
	wlnuser(unr, "[talk on]");
      else
	wlnuser(unr, "[converse on]");
    }
    WITH->action = olda;
    WITH->smode = was_server;
  }


  if (user[unr] != NULL && WITH->inputroot == NULL && WITH->tell >= minhandle)
    stop_tell(unr);
  if (user[unr] != NULL) {
    upd_statistik(unr, 0, 0, WITH->cputime, get_cpuusage());
    WITH->cputime = 0;
  }

}



/* Die Haupteingabe der Box. Zusaetzlich gibt es noch fbbpack und fbb2pack   */

void box_input(short unr, boolean inbegruessung, char *cmd,
	       boolean return_)
{
  char p[256];
  long l;
  boxintype *hpbi, *hpb2;
  userstruct *WITH;

  if (!boxrange(unr))
    return;


  WITH = user[unr];
  if ((unsigned)WITH->action < 32 && ((1L << WITH->action) & 0xc000L) != 0)
	/* user hatte QUIT eingegeben, dann soll er auch nicht senden */
	  return;

  l = strlen(cmd);
  if (l > 255) {
    debug(0, unr, 154, "len(cmd) > 255 -> abort");
    WITH->action = 15;
    return;
  }

  if (WITH->trace_to > 0 && WITH->trace_to != unr)
    trace_string(true, unr, WITH->trace_to, cmd, return_);

  if (WITH->action == 99) {
    strcpy(p, cmd);
    if (return_ && l < 255) {
      strcat(p, "\015");
      l++;
    }
    if (write_pty(unr, l, p) != 0) {  /* SHELL und externe Kommandos */
      upd_statistik(unr, l, 0, 0, 0);
      return;
    }
    boxsetrwmode(WITH->pchan, RW_NORMAL);
    WITH->action = 0;
  }

  if (*cmd == '\0' && return_ && WITH->action == 0 && WITH->errors >= 0 &&
      (WITH->lastprocnumber > 0 ||
       boxspoolstatus(WITH->tcon, WITH->pchan, -1) > 750)) {
    sfclosedel(&WITH->prochandle);
    WITH->lastprocnumber = 0;
    WITH->lastprocnumber2 = 0;
    while (WITH->inputroot != NULL) {
      hpbi = WITH->inputroot;
      WITH->inputroot = WITH->inputroot->next;
      free(hpbi);
    }
    abort_useroutput(unr);
  }

  hpb2 = malloc(sizeof(boxintype));
  if (hpb2 == NULL) {
    debug(0, unr, 154, "no mem for input stack -> abort");
    WITH->action = 15;
    return;
  }

  strcpy(hpb2->line, cmd);
  hpb2->return_ = return_;
  hpb2->in_begruessung = inbegruessung;
  hpb2->next = NULL;

  if (WITH->inputroot != NULL) {
    hpbi = WITH->inputroot;
    while (hpbi->next != NULL)
      hpbi = hpbi->next;
    hpbi->next = hpb2;
  } else
    WITH->inputroot = hpb2;

  if (return_)
    l++;
  upd_statistik(unr, l, 0, 0, 0);

  if (l <= 5)
    return;

  if (cmd[0] == '#') {
    if (strstr(cmd, "#BIN#") == cmd)
      clear_immediately_input(unr);
  }
  
}



/* MODE 0   = User
        1   = outgoing forward
        2   = Tell - Abarbeitung
        3   = system login
        4   = modem out
        5   = modem in
        6   = output in file
        7   = wie 6, aber typischerweise http-request
	8   = forward in file

UM_USER     =   0;
UM_SF_OUT   =   1;
UM_TELLREQ  =   2;
UM_SYSREQ   =   3;
UM_MODEM_OUT=   4;
UM_MODEM_IN =   5;
UM_FILEOUT  =   6;
UM_SINGLEREQ =  7;
UM_FILESF   =   8;
*/

short melde_user_an(char *calls1, short cons, short chan, short mode,
		    boolean reconnect)
{
  short Result;
  short x, y, z, newchan;
  boolean double_;
  short dct;
  long count;
  char calls[256];
  char hs[256], path[256];
  char lan[256];
  userstruct *WITH;
  char STR1[256];
  char STR13[30];

  debug(1, 0, 106, calls1);

  Result = 0;
  newchan = 0;
  if (cons < 0)
    cons = 0;
  if (chan < 0)
    chan = 0;
  if (cons == 0 && chan == 0 && mode != UM_SYSREQ && mode != UM_TELLREQ && mode != UM_FILESF)
    return Result;

  count = get_cpuusage();
  strcpy(hs, calls1);
  get_word(hs, path);
  upper(path);
  del_callextender(path, calls);
  strcpy(path, calls1);
  cut(path, 80);
  del_blanks(path);

  if (mode == UM_SF_OUT || mode == UM_FILESF)   /* nu isser da, der connect */
    abort_routing(calls);
  if (maxavail__() < 20000)
    boxfreemostram(true);
  if (memavail__() >= 20000 && callsign(calls)) {
    x = 1;
    while (x <= MAXUSER && user[x] != NULL)
      x++;
    if (x <= MAXUSER) {
      user[x] = malloc(sizeof(userstruct));
      if (user[x] != NULL) {
	user[x]->call[0] = '\0';
	load_userfile(false, true, calls, user[x]);

	dct = 1;

	if (user[x]->sf_level == 0) {
	  user[x]->direct_sf = direct_sf_bbs(user[x]->call);
	  if (in_real_sf(user[x]->call) || mode == UM_FILESF)
	    user[x]->sf_level = 1;
	} else user[x]->direct_sf = false;

	if (user[x]->sf_level < 1 && user[x]->pwmode < 10 && cons < 1 &&
	    mode != UM_TELLREQ && mode != UM_SYSREQ &&
	    mode != UM_SF_OUT && mode != UM_FILESF && !in_real_sf(calls)) {
	  for (y = 1; y <= MAXUSER; y++) {
	    if (user[y] != NULL) {
	      if (y != x) {
		if (!strcmp(user[y]->call, calls)) {
		  if (!user[y]->supervisor)
		    dct++;
		}
	      }
	    }
	  }
	}

	double_ = (dct > maxuserconnects);

	if ((!double_ && !(gesperrt && user[x]->pwmode < 10)) 
	    || (in_real_sf(calls) && sf_allowed) || (chan == 0)) {
	  WITH = user[x];
	  utc_clock();
	  if (cons > 1 && WITH->level < 1)
	    WITH->level = 1;
	  if (WITH->level > 0 &&
	      (mode != UM_MODEM_IN || strlen(WITH->password) >= 4)) {
	    WITH->magic1 = UF_MAGIC;
	    WITH->magic2 = UF_MAGIC;
	    WITH->magic3 = UF_MAGIC;

	    if (mode != UM_SYSREQ && mode != UM_TELLREQ) {
	      WITH->is_authentic = false;
	      z = actual_connects();
	      if (z > hiscore_connects) {
		hiscore_connects = z;
		flush_bidseek();
		sprintf(STR1, "new connect highscore: %d", hiscore_connects);
		append_profile(-1, STR1);
	      }
	    } else
	      WITH->is_authentic = true;

	    newchan = x;
	    WITH->umode = mode;
	    WITH->logindate = clock_.ixtime;
	    WITH->lastatime = clock_.ixtime;
	    WITH->lastcmdtime = clock_.ixtime;
	    WITH->supervisor = false;
	    WITH->rsysop = false;
	    WITH->hidden = false;
	    WITH->f_bbs = false;
	    WITH->console = (cons > 0);
	    WITH->sf_to = false;
	    WITH->undef = false;
	    WITH->inputroot = NULL;
	    WITH->lastprocnumber = 0;
	    WITH->lastprocnumber2 = 0;
	    WITH->prochandle = nohandle;
	    WITH->procbuf = NULL;
	    WITH->sendheader = NULL;
	    WITH->yapp = NULL;
	    WITH->bin = NULL;
	    WITH->lt_required = false;
	    WITH->pty = nohandle;
	    WITH->ptynum = -1;
	    WITH->ptyid[0] = '\0';
	    WITH->ptytouch = 0;
	    WITH->ptylfcrconv = true;
	    WITH->ptybuflen = 0;
	    WITH->wait_pid = -1;
	    WITH->wait_file[0] = '\0';
	    WITH->procbufsize = 0;
	    WITH->procbufseek = 0;
	    WITH->cputime = count;
	    WITH->in_begruessung = false;
	    WITH->newmsg = false;
	    WITH->fileout_name = NULL;
	    WITH->fileout_handle = nohandle;
	    WITH->sfspeedtime = 0;
	    WITH->sfspeedsize = 0;
	    WITH->sfspeedprops = 0;
      	    WITH->sp_input = 0;
	    if (mode == UM_SF_OUT || mode == UM_FILESF) {
	      WITH->f_bbs = true;
	      WITH->sf_to = true;
	    } else if (mode == UM_MODEM_OUT) {
	      WITH->f_bbs = true;
	      WITH->sf_to = true;
	    }

	    WITH->pchan = chan;
	    WITH->tcon = cons;
	    WITH->print = false;
	    if (!box_pw)
	      WITH->se_ok = (WITH->pwmode != PWM_MUST_PRIV &&
			     WITH->pwmode != PWM_SYSOPPRIV &&
			     WITH->pwmode != PWM_RSYSOPPRIV &&
			     mode != UM_SINGLEREQ);
	    else
	      WITH->se_ok = false;
	    if (!WITH->se_ok)
	      WITH->level = 1;

	    if (mode == UM_SYSREQ) {
	      WITH->supervisor = true;
	      WITH->se_ok = true;
	      WITH->level = 127;
	    }

	    strcpy(WITH->brett, calls);
	    WITH->reply_brett[0] = '\0';
	    WITH->reply_nr = 0;
	    WITH->in_reply = false;
	    WITH->no_reason = '\0';
	    WITH->lastroption[0] = '\0';
	    WITH->input[0] = '\0';
	    WITH->input2[0] = '\0';
	    strcpy(WITH->lastcmd, "***login");
	    *WITH->tellmbx = '\0';
	    WITH->action = 0;
	    WITH->force_priv = 0;
	    WITH->tell = nohandle;
	    WITH->sendchan = nohandle;
	    WITH->SID[0] = '\0';
	    WITH->MSID[0] = '\0';
	    WITH->FSID[0] = '\0';
	    WITH->sfpwdb[0] = '\0';
	    WITH->sfpwtn[0] = '\0';
	    WITH->tellfname[0] = '\0';
	    WITH->sf_ptr = NULL;
	    WITH->needs_new_sf_ptr = false;
	    WITH->binsfptr = NULL;
	    WITH->fwdmode = 0;
	    WITH->errors = 0;
	    WITH->processtime = 0;
	    WITH->talk_to = 0;
	    WITH->isfirstchar = true;
	    WITH->laterflag = false;
	    WITH->msgselection = 0;
	    WITH->sf_master = (mode == UM_SF_OUT || mode == UM_FILESF);
	    WITH->smode = false;
	    strcpy(WITH->spath, "/");
	    WITH->pagcount = 0;
	    WITH->paging = 0;   /* steht erstmal auf 0 */
	    WITH->changed_dir = false;
	    strcpy(WITH->conpath, path);

	    for (y = 0; y < MAXFBBPROPS; y++) {
	      WITH->fbbprop[y].line[0] = '\0';
	      WITH->fbbprop[y].brett[0] = '\0';
	      WITH->fbbprop[y].nr = 0;
	      WITH->fbbprop[y].crc = 0;
	      WITH->fbbprop[y].pack = false;
	      WITH->fbbprop[y].x_nr = 0;
	      WITH->fbbprop[y].bid[0] = '\0';
	      WITH->fbbprop[y].rname[0] = '\0';
	      WITH->fbbprop[y].unpacked = false;
	      WITH->fbbprop[y].mtype = '\0';
	    }

	    WITH->lastsfcall[0] = '\0';
	    WITH->tempbid[0] = '\0';
	    WITH->no_binpack = false;
	    WITH->emblockct = 0;
	    WITH->fsfinhandle = nohandle;
	    
	    del_blanks(WITH->call);
	    if (*WITH->call == '\0') {
	      strcpy(WITH->call, calls);
	      WITH->name[0] = '\0';
	      WITH->mybbs[0] = '\0';
	      WITH->lastdate = WITH->logindate;
	    } else if (WITH->lastdate == 0)
	      WITH->lastdate = WITH->logindate;
	    if (*WITH->language == '\0') {
	      user_language(&whichlangmem, &whichlangsize, whotalks_lan, WITH->call, lan);
	      cut(lan, 3);
	      strcpy(WITH->language, lan);
	    }
	    WITH->fulltrace = false;
	    WITH->trace_to = in_tracecalls(WITH->call);
	    *WITH->tracefract = '\0';
	    if (WITH->trace_to > 0 && WITH->trace_to != x) {
	      if (mode == UM_SF_OUT || mode == UM_FILESF)
		strcpy(hs, "s&f-connect successful");
	      else
		strcpy(hs, "logging in");

	      sprintf(hs, "*** %s %s", WITH->call, strcpy(STR1, hs));
	      trace_string(true, x, WITH->trace_to, hs, true);
	    }

	    if (in_protocalls(WITH->call))
	      append_protolog(x);

	    WITH->convchan = -1;
	    
	    if (mode == UM_FILESF) {
	      WITH->is_authentic = true;
	      WITH->se_ok = true;
	    }
	    else if (WITH->login_priv
	        && !WITH->is_authentic
	        && !WITH->console
	    	&& (WITH->sf_level == 0)
		&& (WITH->umode == UM_USER || WITH->umode == UM_FILEOUT
				|| WITH->umode == UM_TELLREQ)) {
	      strcpylower(STR1, WITH->call);
	      sprintf(hs, "%s%s%cpwd", boxsfdir, STR1, extsep);
	      if (strlen(WITH->password) >= MINPWLEN || exist(hs))
	        WITH->force_priv = 1;
	    }

	    if ((mode == UM_USER || mode == UM_FILEOUT) && !reconnect) {
	      if (create_userlog)
		append_userlog(x);
	      begruessung(x);
	      if (user[x] == NULL)
		return 0;
	    } else if (mode == UM_SF_OUT) {
	      send_tcpip_protocol_frame(x);
	      WITH->action = 100;
	    } else if (mode == UM_TELLREQ) {
	      sprintf(STR1, "%s%s%cTELXXXXXX", tempdir, WITH->call, extsep);
	      mymktemp(STR1);
	      strcpy(WITH->tellfname, STR1);
	      WITH->tell = sfcreate(WITH->tellfname, FC_FILE);
	    } else if (mode == UM_MODEM_IN) {
	      wuser(x, "Password: ");
	      WITH->action = 81;
	    } else if (mode == UM_SINGLEREQ) {
	    } else if (mode == UM_FILESF) {
	      WITH->action = 100;
	    } else if (reconnect)
	      show_prompt(x);

	    upd_statistik(x, 0, 0, WITH->cputime, get_cpuusage());
	    WITH->cputime = 0;

	  } else {
	    sprintf(STR13, "boxuser not permitted: %s", user[x]->call);
	    boxprotokoll(STR13);
	    strcpy(hs, "access denied");
	    boxspool(cons, chan, boxprn2bios(false), hs, true);
	  }

	} else {
	  if (!double_) {
	    if (is_german(calls))
	      strcpy(hs, "Box ist zu Wartungsarbeiten abgeschaltet. Bitte spaeter noch einmal versuchen.");
	    else
	      strcpy(hs, "Box is closed. Please try later");
	  } else {
	    if (is_german(calls))
	      strcpy(hs, "Sie haben zu viele gleichzeitige Verbindungen mit der Box");
	    else
	      strcpy(hs, "You have too many simultaneous connections with this bbs");
	  }
	  boxspool(cons, chan, boxprn2bios(false), hs, true);
	}

	if (newchan <= 0) {
	  if (user[x] != NULL) {
	    free(user[x]);
	    user[x] = NULL;
	  }
	}

      }
    }
  } else {
    if (!callsign(calls))
      sprintf(hs, "invalid login call %s", calls);
    else if (memavail__() < 20000)
      strcpy(hs, "not enough memory to create a user process");
    else
      strcpy(hs, "?");
    boxspool(cons, chan, boxprn2bios(false), hs, true);
  }
  if (newchan < 0)
    newchan = 0;

  return newchan;

}


void melde_user_ab(short unr, boolean sav)
{
  short x;
  boolean is_proto;
  char hs[256];
  boxintype *hpbi;
  userstruct *WITH;
  char STR7[256];

  if (boxrange(unr)) {
    WITH = user[unr];
    debug0(1, unr, 107);

    /* check if we have to clean up file forward */
    if (WITH->umode == UM_FILESF) {
      close_filesf_output(unr);
      close_filesf_input(unr);      
    }

    if (WITH->fileout_name != NULL) free(WITH->fileout_name);
    WITH->fileout_name = NULL;
    
    if (WITH->trace_to > 0 && WITH->trace_to != unr) {
      sprintf(hs, "*** %s logging out", WITH->call);
      trace_string(true, unr, WITH->trace_to, hs, true);
    }

    if (WITH->convchan > 0)
      send_conv(unr, "*** logging out");

    sfclosedel(&WITH->sendchan);

    if (WITH->sendheader != NULL)
      free(WITH->sendheader);

    if (WITH->yapp != NULL) {
      sfclose(&WITH->yapp->filefd);
      if (WITH->yapp->delete_)
        sfdelfile(WITH->yapp->fname);
      free(WITH->yapp);
    }

    if (WITH->bin != NULL) {
      sfclose(&WITH->bin->filefd);
      if (WITH->bin->write_ || WITH->bin->delete_)
	sfdelfile(WITH->bin->fname);
      free(WITH->bin);
    }

    sprintf(hs, "%sSHOWDIR%c%d", tempdir, extsep, unr);
    sfdelfile(hs);

    delete_trace_from(unr);

    if (WITH->emblockct > 0)
      del_emblocks(unr, WITH->emblockct);

    if (WITH->laterflag)
      reset_laterflag(unr);

    if (WITH->umode != UM_TELLREQ && WITH->umode != UM_SYSREQ
    	&& WITH->umode != UM_SINGLEREQ && !WITH->console)
      box_logbuch(unr);

    if ((WITH->supervisor || WITH->rsysop) && WITH->umode != UM_SYSREQ &&
	!WITH->console) {
      strcpy(WITH->lastcmd, "sysop");
      if (WITH->rsysop) {
	sprintf(STR7, "board-%s", WITH->lastcmd);
	strcpy(WITH->lastcmd, STR7);
      }
      strcat(WITH->lastcmd, " logged out");
      append_syslog(unr);
      *WITH->lastcmd = '\0';
    }

    is_proto = in_protocalls(WITH->call);
    if (is_proto || create_userlog) {
      strcpy(WITH->lastcmd, "***logout");
      if (is_proto)
        append_protolog(unr);
      if (create_userlog)
	append_userlog(unr);
      *WITH->lastcmd = '\0';
    }

    if (WITH->f_bbs)
      check_frag_sf(unr);
    if (WITH->newmsg) {
      sprintf(STR7, "%s%s%cMSG", boxstatdir, WITH->call, extsep);
      sfdelfile(STR7);
    }
    if (sav)
      WITH->lastdate = WITH->logindate;
    if (WITH->talk_to > 0)
      end_talk_mode(WITH->talk_to, true);
    if (WITH->binsfptr != NULL)
      free(WITH->binsfptr);
    if (*WITH->tellfname != '\0')
      sfdelfile(WITH->tellfname);

    sfclosedel(&WITH->prochandle);
    while (WITH->inputroot != NULL) {
      hpbi = WITH->inputroot;
      WITH->inputroot = WITH->inputroot->next;
      free(hpbi);
    }

    del_blanks(WITH->call);
    WITH->logins++;
    WITH->sslogins++;

    if (WITH->wait_pid > 0) {
      close_shell(unr);
      WITH->wait_pid = -1;   /* fuer den Atari/Mac */
    }

    if (WITH->umode >= 32 ||
	((1L << WITH->umode) & ((1L << UM_SYSREQ) | (1L << UM_TELLREQ))) == 0) {
      save_userfile(user[unr]);
      if (sav) {
	for (x = 1; x <= MAXUSER; x++) {
	  if (user[x] != NULL) {
	    if (!strcmp(user[x]->call, WITH->call))
	      user[x]->lastdate = WITH->lastdate;
	  }
	}
      }
    }

  }

  if (boxrange(unr)) {
    debug(5, unr, 107, "now deleting user");
    user[unr]->magic1 = 0;
    user[unr]->magic2 = 0;
    user[unr]->magic3 = 0;
    free(user[unr]);
    user[unr] = NULL;
  } else {
    sprintf(hs, "user %d was NULL", unr);
    debug(0, -1, 107, hs);
  }

  debug(5, -1, 107, "... done");
}
