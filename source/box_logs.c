/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/

#include <syslog.h>
#include "boxlocal.h"
#include "boxglobl.h"
#include "filesys.h"
#include "pastrix.h"
#include "tools.h"
#include "box_sub.h"

/* liefert neue Laenge des Files zurueck */

static long ap_sysfile(boolean to_syslog, short unr, char *txt, char *fname)
{
  long		Result;
  short		x;
  boolean	first;
  char		hs[256];
  char		d2[256], t2[256], c2[256];
  char		s[256], w[256], ww[256];
  char		STR7[256];

  Result = 0;
  if (boxrange(unr))
    sprintf(hs, "%.6s", user[unr]->call);
  else if (unr == -2)
    strcpy(hs, "FILTER");
  else if (unr == -3)
    strcpy(hs, "LOOP");
  else if (unr == -4)
    strcpy(hs, "DIRTY");
  else if (unr == -5)
    strcpy(hs, "OLD");
  else if (unr == -6)
    strcpy(hs, "REDIST");
  else if (unr == -7)
    strcpy(hs, "LSTSRV");
  else if (unr == -8)
    strcpy(hs, "BROKEN");
  else if (unr == -9)
    strcpy(hs, "PING");
  else
    strcpy(hs, "SYSTEM");
  strcat(hs, ":");
  rspacing(hs, 8);

  if (to_syslog && profile_to_syslog) {
    openlog("dpbox", LOG_NDELAY+LOG_PID, LOG_DAEMON);
    syslog(LOG_NOTICE, "%s%s", hs, txt);
    closelog();
  }

  utc_clock();
  sprintf(d2, "%.5s", clock_.datum);
  sprintf(t2, "%.8s", clock_.zeit);

  x = unr;
  if (x < 0)
    x = 0;
  sprintf(c2, "%2d", x);
  if (strlen(txt) < 52) {
    snprintf(w, 255, "%s %s %s %s%s", d2, t2, c2, hs, txt);
    boxprotokoll(w);
    return (append(fname, w, true));
  }

  first = true;
  strcpy(s, txt);
  while (*s != '\0') {
    x = strlen(s);
    if (x > 51) {
      x = 51;
      while (x > 8 && s[x - 1] != ' ')
	x--;
      if (s[x - 1] != ' ')
	x = 51;
    }
    sprintf(w, "%.*s", x, s);
    strdelete(s, 1, x);
    if (first)
      sprintf(ww, "%s %s %s %s", d2, t2, c2, hs);
    else
      strcpy(ww, "                          ");
    sprintf(STR7, "%s%s", ww, w);
    boxprotokoll(STR7);
    sprintf(STR7, "%s%s", ww, w);
    Result = append(fname, STR7, true);
    first = false;
  }
  return Result;
}


void append_profile(short unr, char *txt)
{
  ap_sysfile(true, unr, txt, profile_box);
}


void append_readlog(short unr, char *txt)
{
  char rlog[256];

  sprintf(rlog, "%sreadlog%cbox", boxprotodir, extsep);
  ap_sysfile(false, unr, txt, rlog);
}

void append_rejectlog(short unr, char mtype, char *sender, char *board, char *mbx, char *bid,
      	      	      char *size, char *comment)
{
  char	rlog[256];
  char	hs[256];
  
  if (!boxrange(unr)) return;
  snprintf(rlog, 255, "%srejlog%cbox", boxprotodir, extsep);
  snprintf(hs, 255, "S%c %s@%s < %s $%s %s reason:%s", mtype, board, mbx, sender, bid, size, comment);
  ap_sysfile(false, unr, hs, rlog);
}

static void append_log(short unr, char *hs)
{
  char txt[300];

  if (!boxrange(unr)) return;
  if (user[unr]->smode)
    sprintf(txt, "fileserver: %s", user[unr]->lastcmd);
  else if (*user[unr]->brett != '\0')
    sprintf(txt, "(%s) %s", user[unr]->brett, user[unr]->lastcmd);
  else
    strcpy(txt,user[unr]->lastcmd);
  ap_sysfile(false, unr, txt, hs);
}


void append_syslog(short unr)
{
  char hs[256];

  sprintf(hs, "%ssyslog%cbox", boxprotodir, extsep);
  append_log(unr, hs);
}


void append_userlog(short unr)
{
  char hs[256];

  sprintf(hs, "%suserlog%cbox", boxprotodir, extsep);
  append_log(unr, hs);
}


void append_usersflog(short unr, char *sender, char *board, char *mbx,
		      char *bid, char *size)
{
  char hs[256], txt[256];

  sprintf(txt, "%s@%s <%s", board, mbx, sender);
  if (*bid != '\0')
    sprintf(txt + strlen(txt), " $%s", bid);
  sprintf(txt + strlen(txt), " %s", size);
  del_lastblanks(txt);
  sprintf(hs, "%susrsflog%cbox", boxprotodir, extsep);
  ap_sysfile(false, unr, txt, hs);
}


void append_protolog(short unr)
{
  char hs[256];

  sprintf(hs, "%s%s%cPRO", boxprotodir, user[unr]->call, extsep);
  append_log(unr, hs);
}


void append_sflog(short unr)
{
  char hs[256];

  sprintf(hs, "%ssflog%cbox", boxprotodir, extsep);
  append_log(unr, hs);
}


void append_convlog(short unr, char *txt)
{
  char hs[256];

  sprintf(hs, "%sconvlog%cbox", boxprotodir, extsep);
  ap_sysfile(false, unr, txt, hs);
}

#ifdef DPDEBUG
#ifdef __GLIBC__
#include <malloc.h>
static void print_memusage(char *hs)
{
  struct mallinfo mallinfos;

  mallinfos = mallinfo();
  sprintf(hs, "%7d%8d%8d%8d", mallinfos.uordblks, mallinfos.fordblks,
		mallinfos.arena, mallinfos.keepcost);
}
#endif
#endif

boolean get_debug_func(short pn, char *p)
{
  switch (pn) {

  case 0:
    strcpy(p, "");
    break;

  case 1:
    strcpy(p, "read_index");
    break;

  case 2:
    strcpy(p, "write_index");
    break;

  case 3:
    strcpy(p, "read_log");
    break;

  case 4:
    strcpy(p, "write_log");
    break;

  case 5:
    strcpy(p, "write_log_and_bid");
    break;

  case 6:
    strcpy(p, "check_mybbsfile");
    break;

  case 7:
    strcpy(p, "show_all_user_at");
    break;

  case 8:
    strcpy(p, "update_mybbsfile");
    break;

  case 9:
    strcpy(p, "load_userfile");
    break;

  case 10:
    strcpy(p, "save_userfile");
    break;

  case 11:
    strcpy(p, "get_bbsinfo");
    break;

  case 12:
    strcpy(p, "check_hpath");
    break;

  case 13:
    strcpy(p, "create_new_boxlog");
    break;

  case 14:
    strcpy(p, "garbage error");
    break;

  case 15:
    strcpy(p, "garbage_collection");
    break;

  case 16:
    strcpy(p, "garbage, board");
    break;

  case 17:
    strcpy(p, "garbage, no mem");
    break;

  case 18:
    strcpy(p, "last_valid");
    break;

  case 19:
    strcpy(p, "alter_log");
    break;

  case 20:
    strcpy(p, "alter_x_and_log_entry");
    break;

  case 21:
    strcpy(p, "recompile_log");
    break;

  case 22:
    strcpy(p, "boxcheck");
    break;

  case 23:
    strcpy(p, "load_rubrikinfos");
    break;

  case 24:
    strcpy(p, "load_balise");
    break;

  case 25:
    strcpy(p, "call_mfilter");
    break;

  case 26:
    strcpy(p, "new_entry");
    break;

  case 27:
    strcpy(p, "transform_boxheader");
    break;

  case 28:
    strcpy(p, "sort_new_mail");
    break;

  case 29:
    strcpy(p, "send_file1");
    break;

  case 30:
    strcpy(p, "send_text3");
    break;

  case 31:
    strcpy(p, "open_sendfile");
    break;

  case 32:
    strcpy(p, "box_txt2");
    break;

  case 33:
    strcpy(p, "send_text1");
    break;

  case 34:
    strcpy(p, "send_check");
    break;

  case 35:
    strcpy(p, "send_sysmsg");
    break;

  case 36:
    strcpy(p, "search_by_bid");
    break;

  case 37:
    strcpy(p, "erase_by_bid");
    break;

/* deleted. was "set_sys_fwd"
  case 38:
    break;
*/    
  case 39:
    strcpy(p, "delete_brett_by_bid");
    break;

  case 40:
    strcpy(p, "erase_brett");
    break;

  case 41:
    strcpy(p, "readlimit");
    break;

  case 42:
    strcpy(p, "read_brett");
    break;

  case 43:
    strcpy(p, "list_brett");
    break;

  case 44:
    strcpy(p, "change_mbx");
    break;

  case 45:
    strcpy(p, "change_lifetime");
    break;

  case 46:
    strcpy(p, "check");
    break;

  case 47:
    strcpy(p, "show_dir");
    break;

  case 48:
    strcpy(p, "show_statistik");
    break;

  case 49:
    strcpy(p, "set_forward");
    break;

  case 50:
    strcpy(p, "transfer");
    break;

  case 51:
    strcpy(p, "export_brett");
    break;

  case 52:
    strcpy(p, "read_for_view");
    break;

  case 53:
    strcpy(p, "read_for_bcast");
    break;

  case 54:
    strcpy(p, "begruessung");
    break;

  case 55:
    strcpy(p, "enter_password");
    break;

  case 56:
    strcpy(p, "analyse_boxcommand");
    break;

  case 57:
    strcpy(p, "run_batch");
    break;

  case 58:
    strcpy(p, "run_sysbatch");
    break;

  case 59:
    strcpy(p, "box_command_fract");
    break;

  case 60:
    strcpy(p, "get_binstart");
    break;

  case 61:
    strcpy(p, "add_line_to_buff");
    break;

  case 62:
    strcpy(p, "write_msgid");
    break;

  case 63:
    strcpy(p, "bid_mem");
    break;

  case 64:
    strcpy(p, "bbspath");
    break;

/* deleted...
    case 65:
    break;
*/

  case 66:
    strcpy(p, "scan_for_ack");
    break;

  case 67:
    strcpy(p, "scan_hierarchicals");
    break;

  case 68:
    strcpy(p, "pack_entry");
    break;

  case 69:
    strcpy(p, "kill_resume");
    break;

  case 70:
    strcpy(p, "check_resume");
    break;

  case 71:
    strcpy(p, "send_pfbbram");
    break;

  case 72:
    strcpy(p, "send_pfbbdisk");
    break;

  case 73:
    strcpy(p, "resend_userfile");
    break;

  case 74:
    strcpy(p, "propose_sf");
    break;

  case 75:
    strcpy(p, "change_sfentries");
    break;

  case 76:
    strcpy(p, "received_sf");
    break;

  case 77:
    strcpy(p, "ok_sf_sending");
    break;

  case 78:
    strcpy(p, "prepare_for_next_fbb");
    break;

  case 79:
    strcpy(p, "send_fbb_answer");
    break;

  case 80:
    strcpy(p, "analyse_fbb_answer");
    break;

  case 81:
    strcpy(p, "sf_rx_emt");
    break;

  case 82:
    strcpy(p, "add_bpacksf");
    break;

  case 83:
    strcpy(p, "fbbpack");
    break;

  case 84:
    strcpy(p, "fbb2pack");
    break;

  case 85:
    strcpy(p, "check_frag_sf");
    break;

  case 86:
    strcpy(p, "abort_sf");
    break;

  case 87:
    strcpy(p, "_protokoll");
    break;

  case 88:
    strcpy(p, "start_sf");
    break;

  case 89:
    strcpy(p, "start_modem_sf");
    break;

  case 90:
    strcpy(p, "load_sfinfos");
    break;

  case 91:
    strcpy(p, "vermerke_sf");
    break;

  case 92:
    strcpy(p, "analyse_sf_command");
    break;

  case 93:
    strcpy(p, "show_user");
    break;

  case 94:
    strcpy(p, "change_mybbs");
    break;

  case 95:
    strcpy(p, "change_levels");
    break;

  case 96:
    strcpy(p, "change_name");
    break;

  case 97:
    strcpy(p, "change_language");
    break;

  case 98:
    strcpy(p, "disc_user");
    break;

  case 99:
    strcpy(p, "write_ctext");
    break;

  case 100:
    strcpy(p, "reset_to_term");
    break;

  case 101:
    strcpy(p, "call_runprg");
    break;

  case 102:
    strcpy(p, "read_file");
    break;

  case 103:
    strcpy(p, "write_file");
    break;

  case 104:
    strcpy(p, "abort_useroutput");
    break;

  case 105:
    strcpy(p, "abort_box");
    break;

  case 106:
    strcpy(p, "melde_user_an");
    break;

  case 107:
    strcpy(p, "melde_user_ab");
    break;

  case 108:
    strcpy(p, "get_btext");
    break;

  case 109:
    strcpy(p, "show_rfile");
    break;

  case 110:
    strcpy(p, "show_help");
    break;

  case 111:
    strcpy(p, "separate_status");
    break;

  case 112:
    strcpy(p, "create_status2");
    break;

  case 113:
    strcpy(p, "new_bid");
    break;

  case 114:
    strcpy(p, "split_sline");
    break;

  case 115:
    strcpy(p, "find_command");
    break;

  case 116:
    strcpy(p, "load_boxbcastparms");
    break;

  case 117:
    strcpy(p, "get_nextvalidlog");
    break;

  case 118:
    strcpy(p, "find_firstvalidlog");
    break;

  case 119:
    strcpy(p, "send_lognr");
    break;

  case 120:
    strcpy(p, "boxbcasttimer");
    break;

  case 121:
    strcpy(p, "tell_processing");
    break;

  case 122:
    strcpy(p, "timeout_check");
    break;

  case 123:
    strcpy(p, "show_mailbeacon");
    break;

  case 124:
    strcpy(p, "write_file2");
    break;

  case 125:
    strcpy(p, "check_sftimer");
    break;

  case 126:
    strcpy(p, "sf_for");
    break;

  case 127:
    strcpy(p, "*** (re)start ***");
    break;

  case 128:
    strcpy(p, "clear_hash");
    break;

  case 129:
    strcpy(p, "add_hash");
    break;

  case 130:
    strcpy(p, "load_bidhash");
    break;

  case 131:
    strcpy(p, "delete_hash_offset");
    break;

  case 132:
    strcpy(p, "update_hash");
    break;

  case 133:
    strcpy(p, "find_bidhash");
    break;

  case 135:
    strcpy(p, "dispose_sfinfos");
    break;

  case 136:
    strcpy(p, "dispose_rubrikinfos");
    break;

  case 137:
    strcpy(p, "check_hcs");
    break;

  case 138:
    strcpy(p, "del_first_emblock");
    break;

  case 139:
    strcpy(p, "set_forward");
    break;

  case 140:
    strcpy(p, "new_sfentry");
    break;

  case 141:
    strcpy(p, "check_reject");
    break;

  case 142:
    strcpy(p, "do_convtit");
    break;

  case 143:
    strcpy(p, "check_dirty");
    break;

  case 144:
    strcpy(p, "disp_hash");
    break;

  case 145:
    strcpy(p, "search_in_ram");
    break;

  case 146:
    strcpy(p, "search_in_hash");
    break;

  case 147:
    strcpy(p, "add_line_to_buff");
    break;

  case 148:
    strcpy(p, "valid_partner");
    break;

  case 149:
    strcpy(p, "check_sfdeffile");
    break;

  case 150:
    strcpy(p, "end_boxconnect");
    break;

  case 151:
    strcpy(p, "find_sfdef");
    break;

  case 152:
    strcpy(p, "wildcardcompare");
    break;

  case 153:
    strcpy(p, "calc_bidhash");
    break;

  case 154:
    strcpy(p, "box_input");
    break;

  case 155:
    strcpy(p, "boxrange");
    break;

  case 157:
    strcpy(p, "set_password");
    break;

  case 158:
    strcpy(p, "load_bptr");
    break;

  case 159:
    strcpy(p, "add_bptr");
    break;

  case 160:
    strcpy(p, "load_hbox");
    break;

  case 161:
    strcpy(p, "msgnum2cposstart");
    break;

  case 162:
    strcpy(p, "fill_msgnumarr");
    break;

  case 163:
    strcpy(p, "create_all_msgnums");
    break;
  
  case 164:
    strcpy(p, "recompile_fwd");
    break;
    
  case 165:
    strcpy(p, "send_new_unproto");
    break;
    
  case 166:
    strcpy(p, "send_packed_unproto");
    break;

  case 200:
    strcpy(p, "open_iface");
    break;

  case 201:
    strcpy(p, "close_iface");
    break;

  case 202:
    strcpy(p, "send_command_packet");
    break;

  case 203:
    strcpy(p, "send_command_iface");
    break;

  case 204:
    strcpy(p, "send_command_channel_if");
    break;

  case 205:
    strcpy(p, "unblocking");
    break;

  case 206:
    strcpy(p, "packet_analysis");
    break;

  case 207:
    strcpy(p, "write_iface_packet2");
    break;

  case 208:
    strcpy(p, "main");
    break;
    
  case 209:
    strcpy(p, "process_crawl");
    break;
    
  case 210:
    strcpy(p, "crawl_message");
    break;
    
  case 211:
    strcpy(p, "enchuf");
    break;
  
  case 212:
    strcpy(p, "dechuf");
    break;
  
  case 213:
    strcpy(p, "enchufmem");
    break;
  
  case 214:
    strcpy(p, "dechufmem");
    break;

  case 215:
    strcpy(p, "memusage");
    break;

  case 218:
    strcpy(p, "check_sanity");
    break;

  case 219:
    strcpy(p, "close_shell");
    break;

  case 220:
    strcpy(p, "cmd_shell");
    break;

  case 221:
    strcpy(p, "my_exec1");
    break;

  case 222:
    strcpy(p, "cmd_run");
    break;

  case 223:
    strcpy(p, "write_pty");
    break;

  case 224:
    strcpy(p, "add_zombie");
    break;

  case 225:
    strcpy(p, "kill_zombies");
    break;

  case 226:
    strcpy(p, "process_wprotline");
    break;

  case 227:
    strcpy(p, "get_wprot_b");
    break;

  case 228:
    strcpy(p, "get_wprot_m");
    break;

  case 229:
    strcpy(p, "get_wprot_e");
    break;

  case 230:
    strcpy(p, "calc_linkspeed");
    break;

  case 231:
    strcpy(p, "start_linkcheck");
    break;

  case 232:
    strcpy(p, "stop_linkcheck");
    break;

  case 234:
    strcpy(p, "calc_linktable");
    break;

  case 235:
    strcpy(p, "calc_linkspeed_result");
    break;

  case 236:
    strcpy(p, "link_aging");
    break;

  case 237:
    strcpy(p, "needs_linkcheck");
    break;

  default:
    *p = '\0';
    return false;
  }

  if (pn != 127 && pn != 0)
    strcat(p, "()");

  return true;
}

void debug_2(short level, short unr, short pn, char *txt)
{
  short		k, l;
  char		hs[256], p[256];
  pathstr	dbold;

  if (level > debug_level || disk_full)
    return;

  lastproc = pn;
  get_debug_func(pn, p);

  if (txt != NULL) {

    nstrcpy(lastproctxt, txt, 255);
    k = strlen(p) + 2;
    if (strlen(txt) + k > 255) {
      if (k > 253) l = 1;
      else l = 254 - k;
      if (*p)
        snprintf(hs, 255, "%s %.*s>", p, l, txt);
      else
        snprintf(hs, 255, "%.*s>", l, txt);
    } else {
      if (*p)
        snprintf(hs, 255, "%s %s", p, txt);
      else
        snprintf(hs, 255, "%s", txt);
    } 
    del_lastblanks(hs);

  } else {

    *lastproctxt = '\0';
    snprintf(hs, 255, "%s", p);

  }

  if (debug_level > 0) {
#ifdef DPDEBUG
#ifdef __GLIBC__
    if (debug_level >= 5 || pn == 215) {
      print_memusage(dbold);
      ap_sysfile(false, unr, dbold, debug_box); /* this is the meminfo... */
      if (pn == 215) return;
    }
#endif
#endif
    if (ap_sysfile(false, unr, hs, debug_box) > debug_size) {
      strcpy(dbold, debug_box);
      new_ext(dbold, "bak");
      sfdelfile(dbold);
      sfrename(debug_box, dbold);
    }
  }
  
  if (level == 0) {
    ap_sysfile(true, unr, hs, profile_box);
  }
}

