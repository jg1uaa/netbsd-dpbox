/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/

/* Der Fileserver */


#define BOXFSERV_G
#include <unistd.h>
#include "boxfserv.h"
#include "boxglobl.h"
#include "pastrix.h"
#include "yapp.h"
#include "boxlocal.h"
#include "box_logs.h"
#include "tools.h"
#include "box.h"
#include "box_sub.h"
#include "box_sys.h"
#include "box_inou.h"
#include "shell.h"
#include "box_tim.h"
#include "box_file.h"


static void get_options(char *eingabe, char *options)
{
  char	w[256];

  *options	= '\0';
  while (*eingabe != '\0' && eingabe[0] == '-') {
    get_word(eingabe, w);
    sprintf(options + strlen(options), "%s ", w);
  }
}


static boolean expand_path(short unr, char *path)
{
  char	w[256];
  char	STR1[256];

  get_word(path, w);
  strcpy(path, w);

  if (insecure(path))
    return false;

  if (*path == '\0' || path[0] != '/')
    sprintf(path, "%s%s", user[unr]->spath, strcpy(STR1, path));
  strdelete(path, 1, 1);
  sprintf(path, "%s%s", fservdir, strcpy(STR1, path));
  return true;
}


static short call_7plus(boolean priv, short unr, char *fn, char *options, char *original1, char *board)
{
  short			k;
  pid_t			pid;
  time_t		tct;
  boolean     	      	changed_to_longname = false;
  unsigned short	date, time;
  char			hs[256], sd[256], ld[256], ofi[256], w[256], bd[256], resfile[256];
  char			original[256], splockfile[256], short_original[256];
  boardtype   	      	rubrik;

  if (*spluspath == '\0')
    return -1;

  pid		= fork();
  if (pid > 0) {
    add_zombie(pid, "", 0);
    return 0;
  }
  setsid();
  
  strcpy(splockfile, boxstatdir);
  strcat(splockfile, "7plock.box");
  
  tct		= clock_.ixtime;
  while ((utc_clock() - tct < 30) && (exist(splockfile))) {
    sleep(1);
  }
  
  k		= sfcreate(splockfile, FC_FILE);
  sfclose(&k);
  
  snprintf(hs, 255, "%s %s %s", spluspath, fn, options);
  /* add the -# option if applicable to force 7plus to provide a result file 7plus.fls */
  if (new_7plus)
    strcat(hs, " -#");

  del_lastblanks(hs);
  if (priv) {
    snprintf(sd, 255, "%s%stemp7pl%c", boxdir, pservdir, dirsep);
    snprintf(bd, 255, "%s%s", boxdir, pservdir);
  } else {
    snprintf(sd, 255, "%s%stemp7pl%c", boxdir, fservdir, dirsep);
    if (*expand_fservboard != '\0') {
      strcpy(rubrik, board);
      check_verteiler(rubrik);
      switch_to_default_fserv_board(rubrik);
      lower(rubrik);
      snprintf(bd, 255, "%s%s%s%c%s%c", boxdir, fservdir, expand_fservboard, dirsep, rubrik, dirsep);      
    } else {
      snprintf(bd, 255, "%s%snewbin%c", boxdir, fservdir, dirsep);
    }
  }
  sprintf(ofi, "%s7plus.out", sd);
  /* this result file only exists with newer 7plus versions >= 2.21 */
  sprintf(resfile, "%s7plus.fls", sd);
  sfdelfile(resfile);
  sfgetdir(0, ld);
  sfchdir(sd);
  my_system(hs);
  sfchdir(ld);

  sfdelfile(splockfile);
  sfdelfile(ofi);
  
  strcpy(short_original, original1);
  
  /* check for result file and change "original1" to long filename, if available */
  k = sfopen(resfile, FO_READ);
  if (k >= minhandle) {
    file2str(k, hs);
    sfclosedel(&k);
    get_word(hs, w);
    get_word(hs, w);
    if (*w) {
      /* ok, found valid result file with long filename */
      snprintf(original1, 255, "%s%s", sd, w);
      changed_to_longname = (strcmp(hs, short_original));
    }
  }
  
  if (exist(original1)) {

    get_ext(original1, w);
    upper(w);
    if (!strcmp(w, "COR") || !strcmp(w, "ERR")) {
      if (pid == 0)
	exit(0);
      else
	return 0;
    }

    ix2dostime(clock_.ixtime, &date, &time);
    sfsetdatime(original1, &date, &time); /* 7plus setzt selber das Originaldatum */
    strcpy(original, original1);
    del_path(original);
    *ofi	= '\0';
    *ld		= '\0';

    if (!priv && *expand_fservboard != '\0') {
      strcpy(hs, bd);
      hs[strlen(hs)-1] = '\0'; /* delete last slash */
      if (!exist(hs) && sfmakedir(hs) < 0) {
        snprintf(bd, 255, "cannot create directory %s", hs);
      	append_profile(unr, bd);
        snprintf(bd, 255, "%s%snewbin%c", boxdir, fservdir, dirsep);
      }
    }
    
    snprintf(ld, 255, "%s%s", sd, original);
    snprintf(ofi, 255, "%s%s", bd, original);
    sfrename(ld, ofi);
    chmod(ofi, 0644);

    new_ext(ld, "inf");
    new_ext(ofi, "inf");
    sfrename(ld, ofi);
    chmod(ofi, 0644);

    del_ext(ld);
    strdelete(ld, 1, strlen(boxdir));
    snprintf(w, 255, "%s%c%c", ld, extsep, allquant);
    file_delete(-1, w, wlnuser);

    if (changed_to_longname) {
      strcpy(ld, short_original);
      del_ext(ld);
      strdelete(ld, 1, strlen(boxdir));
      snprintf(w, 255, "%s%c%c", ld, extsep, allquant);
      file_delete(-1, w, wlnuser);

      strcpy(hs, short_original);
      del_path(hs);    
      snprintf(ld, 255, "%s.%s", sd, hs);
      snprintf(ofi, 255, "%s.%s", bd, original);
    } else {
      snprintf(ld, 255, "%s.%s", sd, original);
      snprintf(ofi, 255, "%s.%s", bd, original);
    }
    sfrename(ld, ofi);
    sfsetdatime(ofi, &date, &time);
    chmod(ofi, 0644);
    sfdelfile(ld);
  }

  if (pid == 0)
    exit(0);
  return 0;
}


static boolean expand_number(short unr, char *fn)
{
  short		nr;
  char		hs[256], s[256];
  char		STR7[256];

  if (!boxrange(unr))
    return false;
  if (!zahl(fn))
    return false;
  strcpy(hs, fn);
  nr = atoi(hs);
  if (nr <= 0)
    return false;
  sprintf(s, "%d", nr);
  sprintf(s, "-C%s -DATE", strcpy(STR7, s));
  *hs = '\0';
  if (!expand_path(unr, hs))
    return false;
  sprintf(hs, "%s%s%c%c%c", boxdir, strcpy(STR7, hs), allquant, extsep, allquant);
  file_dir(user[unr]->supervisor, hs, s, unr, wlnuser);
  if (*hs == '\0')
    return false;

  strcpy(fn, hs);
  return true;
}


static void create_dir(short unr, char *eingabe_)
{
  char	eingabe[256];
  char	STR1[256];

  strcpy(eingabe, eingabe_);
  if (!user[unr]->se_ok) {
    wln_btext(unr, 65);
    return;
  }

  if (!expand_path(unr, eingabe))
    return;

  sprintf(eingabe, "%s%s", boxdir, strcpy(STR1, eingabe));
  if (sfmakedir(eingabe) < 0)
    wlnuser(unr, "failed");
}


static void remove_dir(short unr, char *eingabe_)
{
  char	eingabe[256];
  char	STR1[256];

  strcpy(eingabe, eingabe_);
  if (!user[unr]->se_ok) {
    wln_btext(unr, 65);
    return;
  }

  if (!expand_path(unr, eingabe))
    return;

  sprintf(eingabe, "%s%s", boxdir, strcpy(STR1, eingabe));
  if (sfremovedir(eingabe) < 0)
    wlnuser(unr, "failed");
}


static void remove_file(short unr, char *eingabe_)
{
  char	eingabe[256];
  short	x, y;
  char	options[256], p[256];

  strcpy(eingabe, eingabe_);
  if (!user[unr]->se_ok) {
    wln_btext(unr, 65);
    return;
  }

  get_options(eingabe, options);
  upper(options);
  expand_number(unr, eingabe);
  if (!expand_path(unr, eingabe))
    return;

  strcpy(p, eingabe);

  if (strstr(options, "-Q") != NULL)
    x	= -1;
  else
    x	= unr;
  file_delete(x, eingabe, wlnuser);

  y	= strlen(p);
  while (y > 1 && p[y - 1] != dirsep)
    y--;
  if (p[y] == '.')
    return;

  strinsert(".", p, y + 1);
  file_delete(x, p, wlnuser);
}


static void show_shelp(short unr)
{
  short	x;

  wln_btext(unr, 150);
  wlnuser0(unr);
  for (x = 151; x <= 160; x++)
    wln_btext(unr, x);
  wlnuser0(unr);
  for (x = 161; x <= 170; x++)
    wln_btext(unr, x);
  wlnuser0(unr);
  for (x = 171; x <= 174; x++)
    wln_btext(unr, x);

  if (!user[unr]->supervisor)
    return;

  wlnuser0(unr);
  wln_btext(unr, 175);
  wlnuser0(unr);
  for (x = 176; x <= 181; x++)
    wln_btext(unr, x);
}


static void copy_file(short unr, char *eingabe_, boolean del_src)
{
  char	eingabe[256];
  short	y, u, ret;
  char	options[256], arg1[256], arg2[256], root[256], STR1[256];

  strcpy(eingabe, eingabe_);
  get_options(eingabe, options);
  upper(options);

  get_word(eingabe, arg1);
  get_word(eingabe, arg2);

  expand_number(unr, arg1);

  if (!strcmp(arg2, "."))
    *arg2 = '\0';

  if (!expand_path(unr, arg1) || !expand_path(unr, arg2))
    return;

  sprintf(arg1, "%s%s", boxdir, strcpy(STR1, arg1));
  sprintf(arg2, "%s%s", boxdir, strcpy(STR1, arg2));
  sprintf(root, "%s%s", boxdir, fservdir);
  if (strstr(options, "-Q") != NULL)
    u	= -1;
  else
    u	= unr;

  ret	= filecp(arg1, arg2, root, del_src, u, wlnuser);

  if (ret < 0) {
    wlnuser(unr, "failed");
    return;
  }

  y	= strlen(arg1);
  while (y > 1 && arg1[y - 1] != dirsep)
    y--;
  if (arg1[y] == '.')
    return;

  strinsert(".", arg1, y + 1);

  if (ret != 1) {  /* arg2 was no directory */
    y	= strlen(arg2);
    while (y > 1 && arg2[y - 1] != dirsep)
      y--;
    if (arg2[y] == '.')
      return;
    strinsert(".", arg2, y + 1);
  }

  filecp(arg1, arg2, root, del_src, u, wlnuser);
}


static void put_file2(short unr, char *eingabe, short mode);

static void add_ascii(short unr, char *eingabe, boolean return_)
{
  char	hs[256], w[256];
  short	x, ctrl_z;

  if (user[unr]->sendchan < minhandle) {
    user[unr]->action	= 0;
    return;
  }

  ctrl_z = strpos2(eingabe, "\032", 1);
  if (ctrl_z > 0 || (user[unr]->isfirstchar && return_ && !strcmp(eingabe, "."))) {
    if (*user[unr]->input2 != '\0') {
      str2file(&user[unr]->sendchan, user[unr]->input2, true);
      *user[unr]->input2 = '\0';
    }
    if (ctrl_z > 1) {
      cut(eingabe, ctrl_z - 1);
      str2file(&user[unr]->sendchan, eingabe, true);
    }
    if (sfseek(0, user[unr]->sendchan, SFSEEKEND) <= 0) {
      sfclosedel(&user[unr]->sendchan);
      wln_btext(unr, 11);
    } else {
      sfclose(&user[unr]->sendchan);
      wlnuser0(unr);
    }
    if (user[unr]->action != 94) {
      user[unr]->action = 0;
      wln_btext(unr, 6);
      return;
    }
    user[unr]->action = 0;
    wlnuser0(unr);
    put_file2(unr, user[unr]->sfilname, user[unr]->paging);
    return;
  }
  if (user[unr]->action != 95 && user[unr]->action != 94) {
    str2file(&user[unr]->sendchan, eingabe, return_);
    return;
  }
  del_blanks(eingabe);
  del_mulblanks(eingabe);
  if (strlen(eingabe) + strlen(user[unr]->input2) >= 255) {
    str2file(&user[unr]->sendchan, user[unr]->input2, true);
    *user[unr]->input2 = '\0';
  }
  sprintf(hs, "%s%s", user[unr]->input2, eingabe);
  if (return_)
    strcat(hs, " ");
  while (strlen(hs) > 72) {
    x = 72;
    while (x > 1 && hs[x - 1] != ' ')
      x--;
    if (x == 1)
      x = 72;
    sprintf(w, "%.*s", x, hs);
    strdelete(hs, 1, x);
    str2file(&user[unr]->sendchan, w, true);
  }
  strcpy(user[unr]->input2, hs);
}


static void put_file2(short unr, char *eingabe_, short mode)
{
  short	k;
  char	eingabe[256];
  char	fn[256], hs[256];
  char	STR1[256];

  strcpy(eingabe, eingabe_);
  if (!user[unr]->se_ok) {
    wln_btext(unr, 65);
    return;
  }

  get_word(eingabe, fn);

  if (*fn == '\0')
    return;
  if (!expand_path(unr, fn))
    return;

  if (!user[unr]->supervisor && strstr(fn, "/incoming/") == NULL) {
    wln_btext(unr, 96);
    return;
  }

  strcpy(hs, fn);
  sprintf(hs, "%s%s", boxdir, strcpy(STR1, hs));
  if (exist(hs)) {
    wln_btext(unr, 97);
    return;
  }

  if (mode == 0) {  /* ascii */
    k = sfcreate(hs, FC_FILE_RWALL);
    if (k < minhandle) {
      wln_btext(unr, 98);
      return;
    }
    wln_btext(unr, 99);
    user[unr]->sendchan = k;
    user[unr]->action = 92;
    return;
  }

  if (mode == 2)
    strcat(fn, " -Y");
  write_file(unr, fn);
}


static void put_file(short unr, char *eingabe_, short mode)
{
  char		eingabe[256], fn[256], hs[256], inp[256], STR1[256];
  short		k, x;
  boolean	is_upl, is_index;

  strcpy(eingabe, eingabe_);
  if (!user[unr]->se_ok) {
    wln_btext(unr, 65);
    return;
  }

  strcpy(inp, eingabe);

  get_word(eingabe, fn);
  if (*fn == '\0')
    return;
  if (!expand_path(unr, fn))
    return;

  if (!user[unr]->supervisor && strstr(fn, "/incoming/") == NULL) {
    wln_btext(unr, 96);
    return;
  }
  
  if (disk_full) {
    wln_btext(unr, 86);
    return;
  }

  is_upl	= false;
  is_index	= false;
  strcpy(hs, fn);
  sprintf(hs, "%s%s", boxdir, strcpy(STR1, hs));
  if (exist(hs)) {
    if (!user[unr]->supervisor) {
      wln_btext(unr, 97);
      return;
    }
    sfdelfile(hs);
  }

  strcpy(fn, hs);
  del_path(fn);
  if (fn[0] == '.') {
    if (!user[unr]->supervisor) {
      wln_btext(unr, 100);
      return;
    }
    sfdelfile(hs);
    is_upl	= (strcmp(fn, ".index") != 0);
    is_index	= !is_upl;
  }
  x	= strlen(hs);
  while (x > 1 && hs[x - 1] != dirsep)
    x--;
  if (hs[x] != '.')
    strinsert(".", hs, x + 1);

  if (is_index || exist(hs)) {
    put_file2(unr, inp, mode);
    return;
  }

  k	= sfcreate(hs, FC_FILE_RWALL);
  if (k < minhandle) {
    wln_btext(unr, 98);
    return;
  }

  wln_btext(unr, 101);
  strcpy(user[unr]->sfilname, inp);
  *user[unr]->input2	= '\0';
  user[unr]->paging	= mode;
  user[unr]->sendchan	= k;
  if (is_upl) {
    user[unr]->action	= 95;
    return;
  }
  user[unr]->action	= 94;
  if (!is_index) {
    sprintf(hs, "%s:", user[unr]->call);
    add_ascii(unr, hs, true);
  }

}


static void page_file(short unr, char *eingabe)
{
  short		k;
  boolean	abo;
  char		hs[256];

  k	= nohandle;
  abo	= (*eingabe != '\0' && upcase_(eingabe[0]) == 'C');

  if (!abo)
    k	= sfopen(user[unr]->sfilname, FO_READ);

  if (abo || k < minhandle) {
    user[unr]->sseekp	= 0;
    user[unr]->pagcount	= 0;
    user[unr]->paging	= 0;
    *user[unr]->sfilname = '\0';
    user[unr]->action	= 0;
    if (!abo)
      wlnuser(unr, "read error");
  }

  sfseek(user[unr]->sseekp, k, SFSEEKSET);

  while (file2str(k, hs)) {
    wlnuser(unr, hs);
    user[unr]->pagcount++;
    if (user[unr]->pagcount < user[unr]->paging)
      continue;
    user[unr]->sseekp	= sfseek(0, k, SFSEEKCUR);
    sfclose(&k);
    user[unr]->pagcount	= 0;
    wln_btext(unr, 102);
    return;
  }

  sfclose(&k);
  user[unr]->sseekp	= 0;
  user[unr]->pagcount	= 0;
  user[unr]->paging	= 0;

  *user[unr]->sfilname	= '\0';
  user[unr]->action	= 0;
}


static void get_file(short unr, char *eingabe_, short mode, boolean paging)
{
  char eingabe[256], fn[256], hs[256], s[256], STR1[256];

  strcpy(eingabe, eingabe_);
  if (!user[unr]->supervisor) {
    if ((user[unr]->maxread_day == 0 &&
	 all_maxread_day <= user[unr]->read_today) ||
	(user[unr]->maxread_day != 0 &&
	 user[unr]->maxread_day <= user[unr]->read_today)) {
      signalise_readlimit(unr, true, "", 0, 0);
      return;
    }
  }

  get_word(eingabe, fn);
  expand_number(unr, fn);
  if (*fn == '\0')
    return;
  if (!expand_path(unr, fn))
    return;

  sprintf(hs, "%s%s", boxdir, fn);

  if (!user[unr]->supervisor
      && uspos((sprintf(STR1, "%s%stemp7pl%c", boxdir, fservdir, dirsep), STR1), hs) == 1) {
    wln_btext(unr, 7);
    return;
  }

  strcpy(s, fn);
  del_path(s);
  if (!user[unr]->supervisor && s[0] == '.') {
    wln_btext(unr, 7);
    return;
  }

  if (!user[unr]->supervisor)
    add_readday(unr, sfsize(hs));

  if (paging && mode == 0) {
    user[unr]->sseekp	= 0;
    user[unr]->pagcount	= 0;
    user[unr]->paging	= 24;
    strcpy(user[unr]->sfilname, hs);
    user[unr]->action	= 93;
    page_file(unr, "");
    return;
  }

  switch (mode) {

  case 0:   /* ascii     */
    strcat(fn, " -a");
    break;

  case 1:   /* autobin   */
    break;

  case 2:   /* yapp      */
    strcat(fn, " -y");
    break;
    
  default:
    return;
    break;
  }

  if (read_file(false, unr, fn)) {
    if (read_filepart(unr))
      user[unr]->action	= 96;
  }

}


static void show_dir(short unr, char *eingabe_)
{
  char	eingabe[256];
  char	options[256];
  char	STR1[256];

  strcpy(eingabe, eingabe_);
  get_options(eingabe, options);
  upper(options);

  if (!expand_path(unr, eingabe)) {
    wln_btext(unr, 2);
    return;
  }

  sprintf(eingabe, "%s%s", boxdir, strcpy(STR1, eingabe));

  if (!user[unr]->supervisor
      && uspos((sprintf(STR1, "%s%stemp7pl%c", boxdir, fservdir, dirsep), STR1), eingabe) == 1) {
    wln_btext(unr, 7);
    return;
  }

  if (eingabe[strlen(eingabe) - 1] == dirsep)
    sprintf(eingabe + strlen(eingabe), "%c%c%c", allquant, extsep, allquant);

  if (strstr(options, "-X") != NULL)
    strcat(options, " -DATE");

  if (user[unr]->changed_dir) {
    user[unr]->changed_dir = false;
    strcat(options, " -I");
  }

  file_dir(user[unr]->supervisor, eingabe, options, unr, wlnuser);
}


static void change_spath(short unr, char *eingabe)
{
  short		x, y;
  char		oldp[256], newp[256];
  DTA		dirinfo;
  userstruct	*WITH;
  char		STR1[256], STR7[256];

  if (*eingabe == '\0')
    return;

  WITH	= user[unr];
  if (strcmp(eingabe, "..") && insecure(eingabe))
    return;

  strcpy(oldp, WITH->spath);
  if (!strcmp(eingabe, "."))
    return;

  if (!strcmp(eingabe, "./"))
    return;

  if (!strcmp(eingabe, "..")) {
    y	= strlen(WITH->spath);
    x	= y - 1;
    while (x > 1 && WITH->spath[x - 1] != '/')
      x--;
    if (x < 1)
      x	= 1;
    strdelete(WITH->spath, x, y - x + 1);
  } else if (eingabe[0] == '/')
    strcpy(WITH->spath, eingabe);
  else if (strlen(eingabe) + strlen(WITH->spath) < 254)
    strcat(WITH->spath, eingabe);
  if (WITH->spath[strlen(WITH->spath) - 1] != '/')
    strcat(WITH->spath, "/");
  if (WITH->spath[0] != '/') {
    sprintf(STR1, "/%s", WITH->spath);
    strcpy(WITH->spath, STR1);
  }
  strcpy(newp, WITH->spath);
  strdelete(newp, 1, 1);
  sprintf(newp, "%s%s%s", boxdir, fservdir, strcpy(STR7, newp));
  strdelete(newp, strlen(newp), 1);
  x	= sffirst(newp, 16, &dirinfo);
  if (x == 0 && dirinfo.d_attrib == 16) {
    WITH->changed_dir	= true;
    return;
  }

  get_btext(unr, 103, newp);
  sprintf(newp, "%s: %s", WITH->spath, strcpy(STR1, newp));
  wlnuser(unr, newp);
  strcpy(WITH->spath, oldp);
}

static void expand_first_arg(short unr, char *eingabe, char *fname)
{
  char	w[256], hs[256];
  
  strcpy(hs, eingabe);
  get_word(hs, w);
  if (expand_number(unr, w)) strcpy(fname, w);
  else *fname = '\0';
}

void analyse_smode_command(short unr, char *eingabe, boolean return_)
{
  boolean	onlysys;
  short		cnr;
  char		w[256], fname[256];
  userstruct	*WITH;
  char		STR1[256];

  if (!boxrange(unr))
    return;

  WITH	= user[unr];

  switch (WITH->action) {

  case 0:
    if (*eingabe != '\0')
      strcpy(WITH->lastcmd, eingabe);
    if (create_syslog && WITH->supervisor)
      append_syslog(unr);
    else if (create_userlog && !WITH->supervisor)
      append_userlog(unr);
    if (in_protocalls(WITH->call))
      append_protolog(unr);

    wlnuser0(unr);
    get_word(eingabe, w);
    upper(w);
    del_blanks(eingabe);

    if (compare(w, "HELP") || compare(w, "? "))
      show_shelp(unr);
    else if (compare(w, "TYPE"))
      get_file(unr, eingabe, 0, true);
    else if (compare(w, "CAT") || compare(w, "GET") || compare(w, "READ"))
      get_file(unr, eingabe, 0, false);
    else if (compare(w, "BGET"))
      get_file(unr, eingabe, 1, false);
    else if (compare(w, "YGET"))
      get_file(unr, eingabe, 2, false);
    else if (compare(w, "CD"))
      change_spath(unr, eingabe);
    else if (compare(w, "CD..")) /*typische Fehleingabe*/
      change_spath(unr, "..");
    else if (compare(w, "LIST")) {
      if (strlen(eingabe) < 250)
	sprintf(eingabe, "-X %s", strcpy(STR1, eingabe));
      show_dir(unr, eingabe);
    } else if (compare(w, "LS"))
      show_dir(unr, eingabe);
    else if (compare(w, "DIR")) {
      if (strlen(eingabe) < 250)
	sprintf(eingabe, "-L %s", strcpy(STR1, eingabe));
      show_dir(unr, eingabe);
    } else if (compare(w, "PUT") || compare(w, "WRITE"))
      put_file(unr, eingabe, 0);
    else if (compare(w, "BPUT"))
      put_file(unr, eingabe, 1);
    else if (compare(w, "YPUT"))
      put_file(unr, eingabe, 2);
    else if (WITH->supervisor) {
      if (compare(w, "MKDIR") || compare(w, "MD"))
	create_dir(unr, eingabe);
      else if (compare(w, "DEL") || compare(w, "RM") || compare(w, "ERASE"))
	remove_file(unr, eingabe);
      else if (compare(w, "RMDIR") || compare(w, "RD"))
	remove_dir(unr, eingabe);
      else if (compare(w, "CP") || compare(w, "COPY"))
	copy_file(unr, eingabe, false);
      else if (compare(w, "MV") || compare(w, "MOVE"))
	copy_file(unr, eingabe, true);
      else if (compare(w, "INDEX"))
	put_file(unr, ".index", 0);
      else if (compare(w, "EXIT") || compare(w, "QUIT") || compare(w, "BYE")) {
	wln_btext(unr, 104);
	WITH->smode = false;
      } else {
        cnr = (find_command(unr, w, &onlysys));

        switch (cnr) {
      
        case 98:
        case 99:
          expand_command(unr, w);
          lower(w);
          expand_first_arg(unr, eingabe, fname);
          if (*fname != '\0') sprintf(fname, "DPBOXSELECTEDFILE %s", strcpy(STR1, fname));
          call_runprg(false, unr, w, eingabe, cnr == 99, fname);
          break;
      
        default:
          wln_btext(unr, 105);
          break;
        }
      
      }
    } else if (compare(w, "EXIT") || compare(w, "QUIT") || compare(w, "BYE")) {
      wln_btext(unr, 104);
      WITH->smode = false;
      
    } else {
      
      cnr = (find_command(unr, w, &onlysys));

      switch (cnr) {
      
      case 98:
      case 99:
        expand_command(unr, w);
        lower(w);
        expand_first_arg(unr, eingabe, fname);
        if (*fname != '\0') sprintf(fname, "DPBOXSELECTEDFILE %s", strcpy(STR1, fname));
        call_runprg(false, unr, w, eingabe, cnr == 99, fname);
        break;
      
      default:
        wln_btext(unr, 105);
        break;
      }
      
    }
    
    break;

  case 80:
    talk_line(unr, eingabe);
    break;
    
  case 87:   /* wird auch in box.c benutzt */
    write_file2(unr, eingabe);
    break;

  case 92:
  case 94:
  case 95:
    add_ascii(unr, eingabe, return_);
    break;

  case 96:
    if (WITH->bin == NULL && WITH->yapp == NULL)
      WITH->action = 0;
    else if (*eingabe == '\0' || strstr(eingabe, "#ABORT#") != NULL) {
      wlnuser(unr, eingabe);
      abort_fileread(unr);
      abort_useroutput(unr);
      WITH->action = 0;
    }
    break;


  case 93:
    page_file(unr, eingabe);
    break;

  default:
    user[unr]->action = 0;
    break;
  }

  if (WITH->action == 0)
    show_prompt(unr);

}



void sig_7plus_incoming(boolean priv, char *name, char *original, char *board)
{
  char	hs[256], w[256];

  strcpy(hs, name);
  get_ext(hs, w);
  upper(w);
  if (!strcmp(w, "COR") || !strcmp(w, "7PL") || (strlen(w) == 3 && w[0] == 'P')) {
    del_ext(hs);
    call_7plus(priv, -1, hs, "-ka -y -q -sysop", original, board);
  }
}


static void clean_incoming(void)
{
  short	result, x;
  DTA	dirinfo;
  char	hs[256], w[256], STR7[256];

  x	= 0;
  do {
    sprintf(hs, "%s%s", boxdir, fservdir);
    switch (x) {

    case 0:
      sprintf(hs + strlen(hs), "temp7pl%c", dirsep);
      break;

    case 1:
      sprintf(hs + strlen(hs), "newbin%c", dirsep);
      break;

    case 2:
      sprintf(hs + strlen(hs), "incoming%c", dirsep);
      break;

    case 3:
      sprintf(hs, "%s%stemp7pl%c", boxdir, pservdir, dirsep);
      break;
    }

    sprintf(w, "%s%c%cerr", hs, allquant, extsep);
    strdelete(w, 1, strlen(boxdir));
    file_delete(-1, w, wlnuser);
    new_ext(w, "ERR");
    file_delete(-1, w, wlnuser);

    sprintf(STR7, "%s%c%c%c", hs, allquant, extsep, allquant);
    result = sffirst(STR7, 0, &dirinfo);
    while (result == 0) {
      if (strcmp(dirinfo.d_fname, ".index")) {
	if (clock_.ixtime - dos2ixtime(dirinfo.d_date, dirinfo.d_time) >
	    incominglifetime) {
	  snprintf(STR7, LEN_PATH, "%s%s", hs, dirinfo.d_fname);
	  sfdelfile(STR7);
	}
      }
      result	= sfnext(&dirinfo);
    }
    x++;
  } while (x <= 3);
}

static void clean_expanded_path(void)
{
  short     result, k;
  DTA 	    dirinfo;
  pathstr   hs, hs2, w, tmpf;

  if (*expand_fservboard == '\0') return;
  if (fservexpandlt <= 0) return;

  snprintf(hs, LEN_PATH, "%s%s%s%c%c%c%c", boxdir, fservdir, expand_fservboard, dirsep,
      	      	      	      	      	      	      	    allquant, extsep, allquant);
  result = sffirst(hs, 16, &dirinfo);
  if (result == 0) {

    mytmpnam(tmpf);
    k 	= sfcreate(tmpf, FC_FILE);
    if (k < minhandle) return;
        
    while (result == 0) {
      str2file(&k, dirinfo.d_fname, true);
      result	= sfnext(&dirinfo);
    }
    
    sfseek(0, k, SFSEEKSET);
    
    while (file2str(k, w)) {
      snprintf(hs, LEN_PATH, "%s%s%s%c%s", boxdir, fservdir, expand_fservboard, dirsep, w);
      snprintf(hs2, LEN_PATH, "%s%c%c%c%c", hs, dirsep, allquant, extsep, allquant);
      
      result = sffirst(hs2, 0, &dirinfo);
      if (result != 0) {
      	sfremovedir(hs); /* delete empty directory */
      } else {
	while (result == 0) {

          if (strcmp(dirinfo.d_fname, ".index")) {
	    if (clock_.ixtime - dos2ixtime(dirinfo.d_date, dirinfo.d_time) > fservexpandlt) {
	      snprintf(w, LEN_PATH, "%s%c%s", hs, dirsep, dirinfo.d_fname);
	      sfdelfile(w);
	    }
      	  }

          result	= sfnext(&dirinfo);
	}
      }
      
    } 
    
    sfclosedel(&k);
    
  }

}


void smode_timer(void)
{
  clean_incoming();
  clean_expanded_path();
}
