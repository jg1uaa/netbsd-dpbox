/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#define STATUS_G
#include "status.h"
#include "boxglobl.h"
#include "box_sys.h"
#include "yapp.h"
#include "boxlocal.h"
#include "boxbcast.h"
#include "box_mem.h"
#include "box_file.h"
#include "box_tim.h"
#include "box_sub.h"
#include "tools.h"
#include "linpack.h"

#ifdef __macos__
#include <utsname.h>
#endif

#ifdef __NetBSD__
#include <sys/utsname.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#endif

#define statbufsize     32768


void free_statp(char **p)
{
  if (*p != NULL) {
    free(*p);
    *p	= NULL;
  }
}


static char	*pp;
static long	szz;
static short	lcc;
static char	tl[256];


static void pl(const char *line)
{ char	hs[256];

  nstrcpy(hs, line, 255);
  del_lastblanks(hs);
  if (szz < statbufsize - strlen(hs) - 2) {
    put_line(pp, &szz, hs);
    lcc++;
  }
}

static void ap(const short i, const char *line)
{
  pl(line);
}


static void spp2(char *s)
{
  rspacing(s, 39);
  if (strlen(tl) + strlen(s) > 79) {
    pl(tl);
    strcpy(tl, s);
  } else
    strcat(tl, s);
}

static void sppi(char *s, long l, char *r)
{
  char hs[256];
  
  sprintf(hs, "%-17s: %ld%s", s, l, r);
  spp2(hs);
}

static void sppb(char *s, boolean b)
{
  char hs[256];
  
  if (b)
    sprintf(hs, "%-17s: on", s);
  else
    sprintf(hs, "%-17s: off", s);
  spp2(hs);
}

static void spps(char *s, char *p, char *r)
{
  char hs[256];
  
  sprintf(hs, "%-17s: %s%s", s, p, r);
  spp2(hs);
}

long get_sysruntime(void)
{
  static time_t lastsysrunt	= 0;
  static time_t lastsysrunres	= 0;

#ifndef __NetBSD__
  short k;
  char hs[256], w[256];
  short x, y;
#endif

  if (clock_.ixtime - lastsysrunt < 15)
    return lastsysrunres;

  lastsysrunres = get_boxruntime_l();

#ifdef __linux__
  k = sfopen("/proc/uptime", FO_READ);
  if (k < minhandle)
    return lastsysrunres;
  file2str(k, hs);
  sfclose(&k);
  get_word(hs, w);
  x = strlen(w);
  y = strpos2(w, ".", 1);
  if (y > 0)
    strdelete(w, y, x - y + 1);
  lastsysrunres = atol(w);
#endif

  lastsysrunt = clock_.ixtime;
  return lastsysrunres;
}


void get_sysload(char *s)
{
  static time_t lastsysltime	= 0;
  static char lastsysload[21]	= "";

#ifdef __NetBSD__
  double loadavg[3];
#else
  short k;
  char hs[256], w[256];
  short x, y;
#endif

  if (clock_.ixtime - lastsysltime < 10) {
    strcpy(s, lastsysload);
    return;
  }

#ifdef __linux__
  k = sfopen("/proc/loadavg", FO_READ);
  if (k >= minhandle) {
    *s = '\0';
    file2str(k, hs);
    sfclose(&k);

    for (x = 1; x <= 3; x++) {
      get_word(hs, w);

      y = strpos2(w, ".", 1);
      if (y > 0)
	strdelete(w, y, 1);
      cut(w, 3);
      while (w[0] == '0')
	strdelete(w, 1, 1);

      if (*w == '\0')
	strcpy(w, "0");

      sprintf(s + strlen(s), "%s%% ", w);
    }

    del_lastblanks(s);

  } else
#endif
#ifdef __NetBSD__
  if (getloadavg(loadavg, 3))
    sprintf(s,"load averages: %.2f%%, %.2f%%, %.2f%%\n",
            loadavg[0], loadavg[1], loadavg[2]);
  else
#endif
    strcpy(s, "100% 100% 100%");

  lastsysltime = clock_.ixtime;
  strcpy(lastsysload, s);
}

void get_cpuinf(char *cpu, char *bmips)
{
  static time_t lastcputime	= 0;
  static char lastcputype[81]	= "unknown";
  static char lastbogomips[81]	= "unknown";

  short k;
  char *hp;
#ifdef __NetBSD__
  int mib[2];
  size_t len;
#else
  char hs[256], w[256];
#endif

  if (clock_.ixtime - lastcputime < 100000) {
    strcpy(cpu, lastcputype);
    strcpy(bmips, lastbogomips);
    return;
  }
  
  k		= nohandle;

#ifdef __linux__
  k		= sfopen("/proc/cpuinfo", FO_READ);
  if (k >= minhandle) {
    hs[0]	= '\0';
    hs[1]	= '\0';
    *w		= '\0';

    while (file2str(k, w)) {
	if (uspos("cpu", w) == 1 && w[3] == '\t') {
	  hp	= strchr(w, ':');
	  if (hp != NULL) nstrcat(hs, ++hp, 25);
	}
	if (uspos("model", w) == 1 && w[5] == '\t') {
	  hp	= strchr(w, ':');
	  if (hp != NULL) nstrcat(hs, ++hp, 25);
	}
	if (uspos("vendor_id", w) == 1 && w[9] == '\t') {
	  hp	= strchr(w, ':');
	  if (hp != NULL) nstrcat(hs, ++hp, 25);
	}
	if (uspos("bogomips", w) == 1 && w[8] == '\t') {
	  hp	= strchr(w, ':');
	  if (hp != NULL) nstrcpy(lastbogomips, hp+2, 80);
	}
    }

    cut(hs, 80);
    strcpy(lastcputype, &hs[1]);
  } 
#endif
#ifdef __NetBSD__
    mib[0] = CTL_HW;
    mib[1] = HW_MODEL;
    sysctl(mib, 2, NULL, &len, NULL, 0);
    if ((hp = malloc(len)) != NULL) {
      sysctl(mib, 2, hp, &len, NULL, 0);
      snprintf(lastcputype,80,"%s",hp);
      free(hp);
    }
#endif
  sfclose(&k);
  lastcputime	= clock_.ixtime;
  strcpy(cpu, lastcputype);
  strcpy(bmips, lastbogomips);
}


void get_lastsysload(char *s)
{
  char hs[256];

  get_sysload(hs);
  get_word(hs, s);
}


void get_linpack(char *s)
{
  static int check_linpack = 2;
  static time_t lastlinpack = 0;
  static char linpackresult[60] = "";

  if (check_linpack == 2) {
    if (get_cpu_speed(2) < 10000)
      check_linpack = 0;
    else
      check_linpack = 1;
  }

  if (check_linpack == 0) {
    strcpy(s, linpackresult);
    return;
  }
  
  if (*linpackresult == '\0' || clock_.ixtime-lastlinpack > SECPERDAY) {
    linpack(linpackresult);
    lastlinpack = clock_.ixtime;
  }
  strcpy(s, linpackresult);
}

void get_sysversion(char *s)
{
#if defined(__macos__) || defined(__NetBSD__)
  struct utsname name;

  strcpy(s, "?");
  if (!uname(&name))
    sprintf(s, "%s v%s.%s", name.sysname, name.release, name.version);

#else
  short k, x, y;
  boolean nok;

  strcpy(s, "?");
  k = sfopen("/proc/version", FO_READ);
  if (k < minhandle)
    return;
  *s = '\0';
  file2str(k, s);
  sfclose(&k);
  do {
    x = strpos2(s, "(", 1);
    y = strpos2(s, ")", x);
    nok = false;
    if (x > 0 && y > 0 && y > x) {
      strdelete(s, x, y - x + 1);
      x++;
      do {
	y = strpos2(s, ")", x);
	if (y > 0 && y > x) {
	  if (s[y] == ')') y++;
	  strdelete(s, x, y - x + 1);
	} else y = 0;
      } while (y > 0);
    } else
      nok = true;
  } while (!nok);
#endif
  del_mulblanks(s);
  del_lastblanks(s);
}

#ifdef DPDEBUG
#ifdef __GLIBC__
#include <malloc.h>
static void get_mallocstats(int *total, int *free, int *release)
{
  struct mallinfo mallinfos;

  mallinfos	= mallinfo();
  *total	= mallinfos.uordblks;
  *free		= mallinfos.fordblks;
  *release	= mallinfos.keepcost;
/*  malloc_trim(0); */
}
#endif
#endif

void get_status(boolean sysop, char **p, long *sz)
{
  char hs[256], w[256], STR1[256];
  short s1, s2, s3;
  boolean owner;

#ifdef DPDEBUG
#ifdef __GLIBC__
  int mtotal, mfree, mrelease;

  get_mallocstats(&mtotal, &mfree, &mrelease);
#endif
#endif

  if (*p == NULL) {
    *p = malloc(statbufsize);
    owner = true;
  } else
    owner = false;

  if (*p == NULL)
    return;
  *sz = 0;

  pp = *p;
  szz = *sz;
  *tl = '\0';
  lcc = 0;

  sprintf(hs,"dpbox v%s%s, %s, (c) DL8HBS, Joachim Schurig                       ",
              dp_vnr, dp_vnr_sub, dp_date);
  pl(hs);
#ifdef __macos__
  pl("parts of code (c) DL4YBG, Mark Wahl");
#endif
#ifdef __linux__
  pl("Linux porting (c) DL4YBG, Mark Wahl");
#endif
#ifdef __NetBSD__
  pl("NetBSD porting (c) VK5ABN, Berndt Josef Wulf");
#endif
  pl("");

  sprintf(hs, "installed by     : %s", Console_call);
  pl(hs);
  sprintf(hs, "compiled         : %s %s", __DATE__, __TIME__);
#ifdef __GLIBC__
  strcat(hs, " for GLIBC");
#endif
  pl(hs);
  pl("");
  sprintf(hs, "date             : %s %s", clock_.datum4, clock_.zeit);
  pl(hs);
  sprintf(hs, "ixtime           : %ld sec. since 1.1.1970", clock_.ixtime);
  pl(hs);
  get_boxruntime_s(1, STR1);
  sprintf(hs, "dpbox runtime    : %s", STR1);
  pl(hs);
  get_boxruntime_s(2, STR1);
  sprintf(hs, "total box runtime: %s", STR1);
  pl(hs);
  get_boxruntime_s(3, STR1);
  sprintf(hs, "max. box runtime : %s", STR1);
  pl(hs);
  get_boxruntime_s(4, STR1);
  sprintf(hs, "last box shutdown: %s", STR1);
  pl(hs);
  get_boxruntime_s(5, STR1);
  sprintf(hs, "last box startup : %s", STR1);
  pl(hs);
  get_boxruntime_s(6, STR1);
  sprintf(hs, "last garbage     : %s", STR1);
  pl(hs);
  if (do_wprot_routing) {
    get_boxruntime_s(7, STR1);
    sprintf(hs, "last route bc    : %s", STR1);
    pl(hs);
  }
  get_dpboxusage(&s1, &s2, &s3);
  sprintf(hs, "dpbox sysload    : %d%% %d%% %d%%", s1, s2, s3);
  pl(hs);
  calc_ixsecs_to_string(get_sysruntime(), STR1);
  sprintf(hs, "system runtime   : %s", STR1);
  pl(hs);
  get_sysload(STR1);
  sprintf(hs, "system load      : %s", STR1);
  pl(hs);
  get_sysversion(STR1);
  sprintf(hs, "system version   : %s", STR1);
  pl(hs);
  pl("");
  
  get_cpuinf(w, STR1);
  sprintf(hs, "cpu type         : %s", w);
  pl(hs);
/* bogomips says nothing (or little) about the system speed. from now on, we will use linpack
   (whow, a BBS is a real floating point cruncher :) )
  sprintf(hs, "bogomips         : %s", STR1);
  pl(hs);
*/
  sprintf(hs, "cpu speed index1 : %ld%% (68000/8Mhz = 100%%)", get_cpu_speed(1));
  pl(hs);
  sprintf(hs, "cpu speed index2 : %ld%% (68000/8Mhz = 100%%)", get_cpu_speed(2));
  pl(hs);
  get_linpack(w);
  if (*w) {
    sprintf(hs, "linpack bench    : %s", w);
    pl(hs);
  }
  pl("");

  if (sysop) {
    pl("open files:");
    sfdispfilelist(0, ap);
    pl("");
  }

  if (sysop) {
    pl("bbs connection streams:");
    calc_boxactivity(0, ap);
    pl("");
  }

  if (send_bbsbcast) {
    pl("broadcast transmission streams:");
    show_bcastactivity(0, ap);
  } else
    pl("no bbs broadcast active");
  pl("");

  pl("parameters:");
  pl("");

  sppi("free in infodir", DFree(infodir) / 1024, " MB");
  if (disk_full) spps("disk full", "YES", "");
  sppi("bidhash", bidhash_active(), " bytes");
  sppi("maxbullids", maxbullids, "");
  sppi("bullidseek", bullidseek, "");
  sppi("bids", sfsize(msgidlog) / 13, "");
  if (sub_bid != 0) sppi("sub_bid", sub_bid, "");
  sppi("actmsgnum", actmsgnum + MSGNUMOFFSET, "");
  sppi("hboxhash", hboxhash_active(), " bytes");
  sppi("ttask", ttask * 5, " ms");
  sppb("ufilhide", ufilhide);
  sppb("active_routing", do_wprot_routing);
  sppb("smart_routing", smart_routing);  
  sppb("priv_router", route_by_private);  
  sppb("small_msgs_first", small_first);
  sppb("gzip", use_gzip(1000000));
  sppb("7plus", *spluspath != '\0');
  sppb("7plus/long names", new_7plus);
  sppi("debug_level", debug_level, "");
  sppi("erasedelay", erasewait / SECPERDAY, " days");
  sppi("packdelay", packdelay, " days");
  sppi("bullsfwait", bullsfwait / SECPERDAY, " days");
  sppi("bullsfmaxage", bullsfmaxage / SECPERDAY, " days");
  sppi("returntime", returntime / SECPERDAY, " days");
  sppi("retmailsize", retmailmaxbytes, " bytes");
  sppb("auto7plus", auto7plusextract);
  sppb("autobin", autobinextract);
  sppb("request_lt", request_lt);
  sppb("create_acks", create_acks);
  sppb("ping server", ping_allowed);
  sppb("authentication", authentinfo);
  sppb("scan_all_wp", scan_all_wp);
  sppb("direct_sf", allow_direct_sf);
  sppb("m_filter", m_filter);
  sppb("lazy_MD2", md2_only_numbers);
  
  if (!multi_master) sppb("multi_master", multi_master);
  
  sppb("guess_mybbs", guess_mybbs);

  if (indexcaches != 0) sppi("indexcaches", indexcaches, " PANIC!");
  sppi("maxufcache", maxufcache, "");
  sppi("userinfocachehit", calc_prozent(ufchit, ufchit + ufcmiss), "%");
  sppi("usersftellmode", usersftellmode, "");
  sppi("connect hiscore", hiscore_connects, "");
  sppb("allunproto", allunproto);
  sppi("unproto requests", request_count, "");
  
  if (last_requesttime != 0) {
    ixdatum2string(last_requesttime, w);
    spps("last request at", w, ""); 
  }
  
  sppi("usertimeout", usertimeout, " min");
  sppi("userlifetime", userlifetime, " days");
  sppi("maxuserconnects", maxuserconnects, "");
  sppi("maxread per day", all_maxread_day, " bytes");
  spps("sf_connect_call", sf_connect_call, "");

  sppb("crawler", crawl_active && crawler_exists);
  if (crawl_private && crawl_active && crawler_exists) {
    sppb("crawl privmails", crawl_private);
  }

  if (*other_callsyntax != '\0') {
    spps("callsyntax", other_callsyntax, "");
  }
  
  if (*expand_fservboard != '\0') {
    spps("fservexpand", expand_fservboard, "");
    sppi("fservexpandlt", fservexpandlt / SECPERDAY, " days");
  }
  
#ifdef DPDEBUG
#ifdef __GLIBC__
  get_mallocstats(&mtotal, &mfree, &mrelease);
  sppi("malloced", mtotal, " bytes");
  sppi("reserved (unused)", mfree, " bytes");
  sppi("releasable (top)", mrelease, " bytes");
#endif
#endif

  if (*tl != '\0')
    pl(tl);

  *sz = szz;

}
