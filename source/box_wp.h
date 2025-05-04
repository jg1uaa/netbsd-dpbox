#ifndef BOX_WP_H
#define BOX_WP_H

#include "pastrix.h"
#include "boxglobl.h"
#include "boxlocal.h"

/* implemented version of extended WP protocol */
#define WPROT_VERSION 10
/* used as dummy value if MyBBS info (in WP) did not contain a BID */
#define WPDUMMYBID "123456123456"
/* maximum extension of WP networks */
#define MAXWPHOPS 50
/* maximum age of WPROT messages */
#define MAXWPAGE (30 * SECPERDAY)
/* maximum age of route messages */
#define MAXRAGE (3 * SECPERDAY)
/* how often may B infos pass even if not changed in content? */
#define WPFREQ_B (7 * SECPERDAY)
/* how often will we create Route broadcasts? */
#define WPFREQ_R (5 * 60 * 60)
/* how often will we create dummy link check data? */
#define WPR_DUMMY (4 * 60 * 60)
/* after how many seconds of missing R broadcast of neighbours will we start to create phantoms? */
/* (but only if we have a more recent link check to that neighbour) */
#define WPRHASOWNBC SECPERDAY
/* when creating phantom broadcasts, lower the timestamp by this value */
#define WPRPHANTOMSUBST (WPFREQ_R * 3)
/* worst quality for a direct neighbour */
#define WPRMAXQUAL SHRT_MAX
/* best quality for a direct neighbour */
#define WPRMINQUAL 1
/* penalty for neighbours without interactive forward */
#define WPRADDWIRE (SHRT_MAX / 2)
/* aging per hop (absolute added) */
#define WPRHOPAGING 100
/* aging per hop (percent added) */
#define WPRPHOPAGING 10
/* invalid neighbour */
#define WPRINVALID (SHRT_MAX * 2)
/* how many seconds lives a link after last check ? */
#define WPRMAXUNCHECKED (WPR_DUMMY * 5 / 2)
/* how small is a small block ? */
#define WPRSMALLBLOCK 512
/* use small blocks after how many seconds? */
#define WPRSMALLBLOCKAFTER (30 * 60)
/* shift routing table after how many seconds? */
#define WPRLINKCHECKBLOCKS (30 * 60)
/* create wprot messages every n seconds */
#define WPROTCREATEINTERVAL (30 * 60)
/* base filename of routing stats */
#define WPRSTATNAME "routstat"
/* what is a valid routing timestamp */
#define valid_routing_timestamp(time) (time > clock_.ixtime - MAXRAGE && time < clock_.ixtime + 10*60*60)
/* what is a valid timestamp */
#define valid_wprot_timestamp(time) (time > clock_.ixtime - MAXWPAGE && time < clock_.ixtime + 10*60*60)
/* what is a valid version */
#define valid_wprot_version(version) (version >= 10 && version / 10 <= WPROT_VERSION / 10)
/* is this a phantom timestamp ? */
#define is_phantom_timestamp(time) ((time / WPFREQ_R) * WPFREQ_R == time)

typedef struct routstattype {
  calltype  call;
  calltype  rxfrom;
  time_t    timestamp;
  uint32_t  quality;
  short     hops;
} routstattype;

typedef struct wprottype {
  char	    which;
  calltype  call;
  calltype  rxfrom;
  calltype  sysop;
  calltype  origin;
  calltype  issuer;
  time_t    timestamp;
  int32_t   version;
  uint32_t  quality;
  mbxtype   bbs; /* also hadr (B) and flood (E) */
  bidtype   bid;
  bidtype   erasebid;
  short     hops;
  char	    zip[81];
  char	    name[81];
  char	    qth[81];
  char	    proto[81];
  char	    hwadr[81];
  char	    comment[81];
  unsigned short status;
  char	    routing[512];
} wprottype;

typedef void (*rsoutputproc)(const short unr, const char *s);
void cleanup_routing_stat(short days);
void show_routing_stat(short unr, char *call, char *options, rsoutputproc out);
void sf_rx_emt(short unr, char *eingabe);
void do_emt(int32_t *seekp);
void sf_rx_emt1(char *eingabe, char *actwpfilesender);
void write_routes(char *call, char *rxfrom, time_t timestamp, uint32_t quality, short hops);
void add_wprotline(wprottype *wpb, bool meta);
void add_wpline(char *call, char *bbs, time_t time, char *from,
		       char *zip, char *name, char *qth, bool is_wp);
void generate_wprot_files(void);
void generate_wp_files(void);
bool in_wpservers(char *call);
short process_wprotline(char *hs, char *actsender, bool meta);
short process_wpline(char *hs, char *actsender);
void create_own_routing_broadcasts(void);
void init_wp_timers(void);

#endif
