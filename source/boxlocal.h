/* Header for module boxlocal */

#ifndef BOXLOCAL_H
#define BOXLOCAL_H

#include "pastrix.h"
#include "boxglobl.h"
#include "yapp.h"

#ifdef BOXLOCAL_G
# define vextern
#else
# define vextern extern
#endif

/* general settings      	*/

#define HEADERVERSION	20	/* indexstruct header version				*/
#define HPATHVERSION  	6     	/* hpath.box header version   	      	      	      	*/
#define UF_MAGIC	25431	/* used to lock userstruct against corruption		*/
#define MAXUSER		200	/* max. simultaneous users in the bbs			*/
#define MAXMAXUFCACHE	250	/* max. cached userinfos				*/
#define MAXMSGNUMARR	512	/* must be power of 2					*/
#define MAXFBBPROPS	5	/* don't change it...					*/
#define CHECKBLOCKSIZE	500	/* n entries at one round (don't change)		*/
#define MAXREADDIVISOR	10000   /* factor for maxread computation			*/
#define MINDISKFREE	5000	/* min. free kbytes on filesystem			*/
#define SFDELAYIFLOGIN	30	/* wait delay for msgs if sender is logged in (seconds) */
#define INDEXLOOKAHEAD	150	/* cache for read/write_index()				*/
#define DPBOXCPUINTERVAL 15	/* measure cpu load in interval <seconds>		*/
#define DPBOXCPUARRSIZE (15*60) / DPBOXCPUINTERVAL /* track 15 minutes			*/
#define MINPWLEN	21	/* at least 21 chars for a valid PW			*/
#define MAXBOXINPLINE	256	/* maximum length of one bbs input line			*/
#define MSGNUMOFFSET	50000	/* to distinguish between msgnums and LIST-nums		*/
#define MAXLANGUAGELINEDEFS 200	/* how many lines are in a language definition		*/
#define MAXSPOOLBYTES	30000	/* pause reading if write queue filled	 		*/
#define MAXUSERSPOOL	10000   /* max. bytes in output buffer (MAXSPOOLBYTES???)	*/
#define MAXLASTVIAS	50	/* rescan n calls in the forward header			*/
#define SECPERDAY	86400L	/* You should talk to some higher instance if you plan to change this */
#define	MAX_UNPROTO_PACKET_SIZE	256 /* the packet size for unproto transmissions	*/
#define PACKMIN		200	/* minimum size of a file to be compressed		*/
#define MAXDISPLOGINHOLD 21   	/* show max n held messages to sysops at login	      	*/
#define MAX_RLINELEN  	79    	/* maximum length of own R:line       	      	      	*/
#define LINKSPEEDS    	10    	/* how many linkchecks for averaging? 10 is enough!    	*/

/* consts for msgflags */

#define MSG_SFWAIT	1
#define MSG_PROPOSED	2
#define MSG_SFRX	4
#define MSG_SFTX	8
#define MSG_SFERR	16
#define MSG_SFNOFOUND	32
#define MSG_CUT		64
#define MSG_MINE	128
#define MSG_REJECTED	256
#define MSG_DOUBLE	512
#define MSG_BROADCAST	1024
#define MSG_BROADCAST_RX 2048
#define MSG_OWNHOLD	4096
#define MSG_LOCALHOLD	8192
#define MSG_SFHOLD	16384
#define MSG_OUTDATED	32768L

/* consts for pmode  */
/* take care, coding is not clean. 1,2,4,8,16,128 are used in propose_sf_sending() in a */
/* special manner                                                                     	*/

#define PM_NONE		0
#define PM_GZIP		1
#define PM_HUFF2	4
#define TTEXT		0
#define TBIN		32
#define T7PLUS		64
#define THTML		128

/* consts for pwmode */

#define PWM_MODEM_ACCESS 0
#define PWM_SEND_ERASE	1
#define PWM_MUST_PRIV	2	/* must always priviledge for send/erase */
#define PWM_RSYSOP	8   	/* board sysop */
#define PWM_RSYSOPPRIV	9   	/* must always priviledge for send/erase */
#define PWM_SYSOP	10
#define PWM_SYSOPPRIV	11   	/* must always priviledge for send/erase */
#define MAXPWMODE	PWM_SYSOPPRIV

/* who deleted a file ? */

#define EB_NONE		0
#define EB_SENDER	1
#define EB_RECEIVER	2
#define EB_SYSOP	4
#define EB_RSYSOP	5	/* not a bit value... 	*/
#define EB_DOUBLESYSOP	6	/* not a bit value... 	*/
#define EB_BIN27PLUS	7	/* not a bit value... 	*/
#define EB_LIFETIME	8
#define EB_SF		16
#define EB_REMOTE	32
#define EB_SFERR	64
#define EB_RETMAIL	66
#define EB_TRANSFER	128

/* consts for umode */

#define UM_USER		0
#define UM_SF_OUT	1
#define UM_TELLREQ	2
#define UM_SYSREQ	3
#define UM_MODEM_OUT	4
#define UM_MODEM_IN	5
#define UM_FILEOUT	6
#define UM_SINGLEREQ	7
#define UM_FILESF	8

/* consts for sfinpmode (user-sf-login) */

#define SFI_NONE	0
#define SFI_FROMP	1
#define SFI_FROMPB	2
#define SFI_ALL		3

/* consts for usersftellmode (user-sf-login) */

#define SFT_NONE	0
#define SFT_OWN		1
#define SFT_ALL		2

/* states of rwmode for routing of input data */

#define RW_NORMAL     	0
#define RW_ABIN_WAIT  	5
#define RW_ABIN_RCV   	6
#define RW_FBB	      	7
#define RW_FBB2       	8
#define RW_RAW	      	9
#define RW_LINKED      -1

/* consts for packed forward (special flags for blockcount) */

#define BCMAGIC1        (SHORT_MAX - 0)
#define BCMAGIC2        (SHORT_MAX - 1)
#define BCMAGIC3        (SHORT_MAX - 2)
#define BCMAGIC4        (SHORT_MAX - 3)
#define BCMAGIC5        (SHORT_MAX - 4)

/* error codes	*/

#define ERR_READBOARD 	0xFFFF

/* file extensions  */

#define EXT_IDX		"IDX"
#define EXT_IDX2	"IDL"
#define EXT_INF		"IFO"
#define EXT_GIDX	"IDG"
#define EXT_GINF	"IFG"

/* some hard coded text output	*/

#define DPREADOUTMAGIC	"#DPRDOUT66521A#"
#define TXTDISKFULL	"*** out of disk-memory"
#define TXTDPLZHBIN	"#*DPLZHDP*#"
#define TXTNOMEM	"*** out of memory"
#define TXTUNAUTHUPGER	"X-Info: Einspielung ohne Passwortschutz"
#define TXTUNAUTHUPENG	"X-Info: Upload without password authentication"

/* size of strings	*/

#define LEN_PATH	255
#define LEN_BADWORD	40
#define LEN_SID		20
#define LEN_MULSEARCH	255
#define LEN_CHECKBOARDS	999
#define LEN_PTYBUF	256	/* not lower than size of packet frames	*/
#define LEN_TCPIPBBSTXT	80
#define LEN_BOXCOMMAND	10
#define LEN_TNTPORT	20

typedef char		pathstr[LEN_PATH+1];

/****************** FIXED DATA DEFINITIONS ******************************/
/* Following are data structures that represent file records.		*/
/* You should NEVER change anything of the definitions, neither		*/
/* the order nor the type nor the amount of elements,			*/
/* as this will invalidate all your stored data.			*/
/* If for some reason you decide to change the definitions		*/
/* or plan to add some elements, you should be aware of that		*/
/* you would have to delete all data files referring to these		*/
/* definitions manually (thus losing all data inside). Some of the 	*/
/* data files may even get automatically deleted, as the bbs checks	*/
/* the data integrity at startup.					*/
/* Best would be to start from scratch with an empty mailbox.		*/
/* OK., you are warned...						*/
/************************************************************************/

#define LEN_BID		12
#define LEN_MBX		40
#define LEN_CALL	6
#define LEN_SSID      	2
#define LEN_CALLSSID   	LEN_CALL+1+LEN_SSID
#define LEN_BOARD	8
#define LEN_CHECKSUBJECT 40
#define LEN_SUBJECT	80

typedef char		msgidmem[LEN_BID+1];
typedef msgidmem	bidtype;
typedef char		mbxtype[LEN_MBX+1];
typedef char		calltype[LEN_CALL+1];
typedef char		callssidtype[LEN_CALLSSID+1];
typedef char		boardtype[LEN_BOARD+1];
typedef char		checksubjecttype[LEN_CHECKSUBJECT+1];
typedef char		subjecttype[LEN_SUBJECT+1];

typedef struct indexstruct {
  char			hver;
  bidtype		id;
  boardtype		dest;
  calltype		absender;
  mbxtype		verbreitung;
  subjecttype		betreff;
  time_t		rxdate;
  time_t		txdate;
  unsigned short	lifetime;
  unsigned short	txlifetime;
  char			msgtype;
  char			firstbyte;
  long			start;
  long			packsize;
  long			size;
  long			rxqrg;
  calltype		rxfrom;
  boolean		deleted;
  char			pmode;
  char			level;
  char			fwdct;
  unsigned short	msgflags;
  time_t		erasetime;
  time_t		lastread;
  unsigned short	readcount;
  char			readby[141];
  char			eraseby;
  calltype		sendbbs;
  char			reserved;
  long			msgnum;
  unsigned short	bcastchecksum;
  unsigned short	infochecksum;
  unsigned short	headerchecksum;
} indexstruct;

typedef struct boxlogstruct {
  long			msgnum;
  time_t		date;
  unsigned short	idxnr;
  unsigned short	lifetime;
  unsigned short	msgflags;
  long			size;
  boolean		deleted;
  char			msgtype;
  char			pmode;
  char			level;
  boardtype		brett;
  boardtype		obrett;
  bidtype		bid;
  calltype		absender;
  calltype		verbreitung;
  checksubjecttype	betreff;
} boxlogstruct;

typedef struct mybbstyp {
  calltype		call;
  calltype 		bbs;	 /* MyBBS				*/
  char			mode;	 /* starting with v5.03: guessed entry?	*/
  time_t		ix_time; /* changed at				*/
} mybbstyp;


typedef struct hboxtyp {

  /* struct version */

  char	      	      	version;

  /* general info */

  calltype		call;
  mbxtype		hpath;
  char			desc[41];

  /* direct forward */

  time_t      	      	last_connecttry;
  time_t      	      	last_connectsuccess;
  time_t      	      	last_incoming_connect;
  unsigned long       	incoming_connects;
  unsigned long       	sent_direct;
  unsigned long       	received_direct;

  /* wprot */

  time_t      	      	wprot_update;
  unsigned short       	wprot_status;
  callssidtype 	      	connectcall;
  calltype    	      	sysopcall;

  /* wprot routing */

  time_t      	      	wprot_routing_update;
  time_t      	      	last_direct_routing_update;
  calltype    	      	routing_neighbour;
  unsigned long       	routing_quality;
  short      	      	routing_hops;
  /* second set of routing parms (those ones actually used) */
  time_t      	      	cur_wprot_routing_update;
  calltype    	      	cur_routing_neighbour;
  unsigned long       	cur_routing_quality;
  short       	      	cur_routing_hops;

  /* autorouting all by R: lines */

  calltype		bestfrom_bp;
  calltype		rxfrom_bp;
  time_t		lasttx_bp;
  unsigned long		msgct_bp;
  unsigned long		best_bp;
  unsigned long		aver_bp;
  unsigned long		bytect_bp;
  time_t		at_bp;

  /* autorouting private by R: lines */

  calltype    	      	bestfrom_p;
  calltype    	      	rxfrom_p;
  time_t      	      	lasttx_p;
  unsigned long	       	msgct_p;
  unsigned long	      	best_p;
  unsigned long	      	aver_p;
  unsigned long	      	bytect_p;
  time_t      	      	at_p;
} hboxtyp;


/* hpath.box for DPBOX <= v5.08.18 */
typedef struct hboxtyp_old {
  calltype		call;
  mbxtype		hpath;
  calltype		bestfrom;
  calltype		rxfrom;
  char			desc[41];
  time_t		lasttx;
  long			msgct;
  long			best;
  long			aver;
  long			bytect;
  time_t		at;
} hboxtyp_old;

typedef struct resumemem {
  struct resumemem	*next;
  bidtype		rbid;
  calltype		rcall;
  char			rfname[13];
  long			rsize;
  time_t		rdate;
} resumemem;



/**************************************************************************/
/* Following are definitions that are only used dynamically (in RAM).     */
/* This means you can change them, as long as you cause no conflicts with */
/* the functions that use these definitions. In general, adding some      */
/* elements to the structs should not cause side effects. Changing types  */
/* of the existing elements could cause serious problems.                 */
/**************************************************************************/

typedef enum {
  nix,
  diebox,
  baycom,
  f6fbb,
  wampes,
  diebox_bid
} boxtype;

typedef enum {
  NOP,
  THEBOX_USER,
  W0RLI_SF,
  F6FBB_SF,
  F6FBB_USER,
  BAYCOM_USER,
  WAMPES_USER,
  W0RLI_USER,
  AA4RE_USER,
  F6FBB_USER_514,
  RAW_IMPORT
} cutboxtyp;

typedef struct bintype {
  short			filefd;
  boolean		write_;
  boolean		delete_;
  boolean		ascii;
  long			maxfill;
  long			posi;
  long			total;
  time_t		touch;
  pathstr		fname;
} bintype;

typedef struct local_msgnumarrtype {
  long			msgnum;
  long			cpos;
} local_msgnumarrtype;

typedef local_msgnumarrtype	msgnumarrtype[MAXMSGNUMARR + 1];

typedef struct mptrtype {
  char			csum;
  calltype		call;
  short			mpos;
  struct mptrtype	*next;
} mptrtype;

typedef struct dirtytype {
  struct dirtytype	*next;
  char			bad[LEN_BADWORD+1];
} dirtytype;

typedef struct rejecttype {
  struct rejecttype	*next;
  char			what;
  char			msgtype;
  calltype		from;
  boardtype		tob;
  mbxtype		mbx;
  bidtype		bid;
  long			maxsize;
  boolean		msgtypeneg;
  boolean		fromneg;
  boolean		tobneg;
  boolean		mbxneg;
  boolean		bidneg;
  boolean		maxsizeneg;
} rejecttype;

typedef struct sffortype {
  struct sffortype	*next;
  mbxtype		pattern;
} sffortype;

typedef struct sfrubtype {
  struct sfrubtype	*next;
  boardtype		pattern;
} sfrubtype;

typedef struct sfnotfromtype {
  struct sfnotfromtype	*next;
  calltype		pattern;
} sfnotfromtype;

typedef struct sfcpathtype {
  struct sfcpathtype	*next;
  char			qrg[LEN_TNTPORT+1];
  char			ccall[21];
  char			timeout[21];
  char			extended_args[256];
} sfcpathtype;

typedef struct routingtype {
  struct routingtype  	*next;
  char	      	      	csum;
  calltype    	      	call;
  unsigned short      	kspeed;
  time_t      	      	lastspeed;
  time_t      	      	lasttry;
  time_t      	      	lastconnecttry;
  unsigned long       	tempspeeds;
  unsigned long         tempsizes;
  unsigned long      	speeds[LINKSPEEDS];
  unsigned long      	sizes[LINKSPEEDS];
  short       	      	speedsrow;
  unsigned short      	last_measured;
  boolean     	      	small_blocks;
  boolean     	      	file_forward;
  boolean     	      	send_phantom;
  boolean     	      	sends_route_bc;
  boolean     	      	full_partner;
  boolean     	      	routing_guest;
} routingtype;

typedef enum {
  EM_UNKNOWN,
  EM_EM,
  EM_WP,
  EM_WPROT
} em_type;

typedef struct sfdeftype {
  struct sfdeftype	*next;
  calltype		call;
  boolean		in_routing;
  boolean		no_sfpropdelay;
  boolean		no_bullbin;
  boolean		no_bull7plus;
  boolean     	      	usersf;
  boolean     	        routing_guest;
  boolean     	      	send_em;
  em_type       	em; /* EM_UNKNOWN, EM_EM, EM_WP, EM_WPROT */
  time_t		lasttry;
  time_t		timeout;
  unsigned short	tnc;
  unsigned short	bedingung;
  long			intervall;
  long			pollifnone;
  long			maxbytes_b;
  long			maxbytes_u;
  long			maxbytes_p;
  long			startutc;
  long			endutc;
  sffortype		*forp;
  sffortype		*notforp;
  sfrubtype		*rubrikp;
  sfrubtype		*notrubrikp;
  sfnotfromtype		*notfromp;
  sfcpathtype		*cpathp;
  routingtype 	      	*routp;
} sfdeftype;

typedef struct convtittype {
  struct convtittype	*next;
  boardtype		fromboard;
  boardtype		toboard;
  unsigned short	newlt;
  subjecttype		title;
} convtittype;

typedef struct tcpiptype {
  struct tcpiptype	*next;
  calltype		call;
  char			txt[LEN_TCPIPBBSTXT+1];
} tcpiptype;

typedef struct rsysoptype {
  struct rsysoptype	*next;
  calltype		call;
  boardtype		board;
} rsysoptype;

typedef struct bcommandtype {
  struct bcommandtype	*next;
  char			command[LEN_BOXCOMMAND+1];
  short			cnr;
  boolean		sysop;
  short			ulev;
} bcommandtype;

typedef struct transfertype {
  struct transfertype	*next;
  boardtype		board1;
  boardtype		board2;
} transfertype;

typedef struct rubriktype {
  struct rubriktype	*next;
  boardtype		name;
  unsigned short	lifetime;
  unsigned short	access;
  unsigned short      	min_lifetime;
} rubriktype;

typedef struct blogmem {
  struct blogmem	*vor;
  struct blogmem	*nach;
  time_t		date;
  long			logidxct;
} blogmem;

typedef char firstsixtype[6];

typedef struct binsftyp {
  long			blockcounter;
  long			rxbytes;
  long			offset;
  long			validbytes;
  short			wchan;
  short			fbbtitlelen;
  subjecttype		fbbtitle;
  pathstr		wname;
  firstsixtype		firstsix;
  unsigned short	checksum;	/* laeuft als Checksumme mit		*/
  unsigned short	crcsum;		/* laeuft mit CRC mit			*/
  unsigned short	crc2;		/* die CRC des aktuellen FBB-Blocks	*/
  unsigned short	blockcrc;
} binsftyp;

typedef struct fbbproptype {
  char			line[81];
  boardtype		brett;
  bidtype		bid;
  char			rname[13];
  unsigned short	nr;
  unsigned short	x_nr;
  boolean		pack;
  unsigned short	crc;
  boolean		unpacked;
  char			mtype;
} fbbproptype;

typedef fbbproptype	fbbproparrtype[MAXFBBPROPS];
typedef char		fbb_montype[12][4];
typedef boolean		bidchecktype[MAXFBBPROPS];
typedef char		bidarrtype[MAXFBBPROPS][LEN_BID+1];

typedef struct newmailtype {
  struct newmailtype	*next;
  short			unr;
  pathstr		pattern;
  char			rcall[256];
} newmailtype;

typedef struct boxintype {
  struct boxintype	*next;
  boolean		return_;
  boolean		in_begruessung;
  char			line[MAXBOXINPLINE];
} boxintype;

typedef struct protocalltype {
  struct protocalltype	*next;
  calltype		call;
  char			cs;
} protocalltype;

typedef struct tracetype {
  struct tracetype	*next;
  calltype		call;
  short			by;
} tracetype;

typedef struct unprotoportstype {
  struct unprotoportstype *next;
  boolean		RequestActive;
  boolean		ReqDPBOX;
  long			CurrentSendPos;
  time_t		LastReqTime;
  time_t		LastTxTime;
  char			port[LEN_TNTPORT+1];
  char			path[256];
  char			buff[MAX_UNPROTO_PACKET_SIZE+1];
} unprotoportstype;

typedef struct unprotodeftype {
  unprotoportstype	*ports;
  long			maxback;
  time_t		PollInterval;
  time_t		TxInterval;
  boolean		fbb;
  boolean		dpbox;
  boolean		priv;
  boolean		sys;
} unprotodeftype;

typedef struct zombietype {
  struct zombietype	*next;
  pid_t			pid;
  char			*killfile;
  short       	      	exitaction;
} zombietype;

typedef struct indexcachetype {
  struct indexcachetype	*next;
  struct indexcachetype	*prev;
  char			*start;
  long			size;
  long			CurBufOffs;
  short			handle;
  boolean		vorwaerts;
} indexcachetype;

typedef struct mulsearchtype {
  struct mulsearchtype	*next;
  char			wort[LEN_MULSEARCH+1];
  short			len;
  short			fn1;
  short			fn2;
  short			threshold;
} mulsearchtype;

typedef struct clocktype {
  short			day;  	      	/* 01..31     	      	      	      	      	    */
  short			mon;  	      	/* 01..12     	      	      	      	      	    */
  short			year; 	      	/* 70..137    	      	      	      	      	    */
  short       	      	year4;	      	/* 1970..2037 	      	      	      	      	    */
  short			hour; 	      	/* 00..23     	      	      	      	      	    */
  short			min;  	      	/* 00..59     	      	      	      	      	    */
  short			sec;  	      	/* 00..59     	      	      	      	      	    */
  time_t		ixtime;       	/* seconds since 01.01.1970   	      	      	    */
  time_t		korrektur;	/* diff between system clock and utc (0 with linux) */
  time_t		daystart;     	/* ixtime of current day 00:00:00     	      	    */
  short			weekday;	/* 1..7 -> Monday..Sunday	      	      	    */
  long	      	      	ticks;	      	/* 200 Hz counter     	      	      	      	    */
  char			zeit[9];      	/* 23:59:59   	      	      	      	      	    */
  char			datum[9];     	/* 17.01.99   	      	      	      	      	    */
  char			datum4[11];   	/* 17.01.1999 	      	      	      	      	    */
} clocktype;

typedef struct lmtype { 
  char			*p;
  long			s;
} lmtype;

typedef struct languagetyp {
  struct languagetyp	*next;
  char			sprache[9];
  char			*puffer;
  long			psize;
  lmtype		lct[MAXLANGUAGELINEDEFS];
} languagetyp;


/****************************************************************/
/* This is the data struct created for each current user of the */
/* bbs. Parts of it are stored on disk, but not in a 1:1        */
/* representation. So, don´t delete elements, but you are free  */
/* to add some (at any place).                                  */
/****************************************************************/

typedef struct userstruct {
  short			magic1;
  char			callcsum;
  calltype		call;
  mbxtype		mybbs;
  char			language[4];
  char			name[81];
  char			SID[LEN_SID+1];
  char			MSID[LEN_SID+1];
  char			FSID[LEN_SID+1];
  char			no_reason;
  char			sfpwdb[11];
  char			sfpwtn[33];
  boardtype		brett;
  boardtype		reply_brett;
  short			reply_nr;
  boolean		in_reply;
  pathstr		tempbinname;
  char			password[81];
  char			input[256];
  char			input2[256];
  char			lastcmd[256];	/* for box activity display				*/
  bidtype		resumebid;	/* for creation of resume index				*/
  bidtype		tempbid;	/* for redirection of private mail in FBB mode		*/
  calltype		lastsfcall;	/* w0rli sf, send only one mail of this sender at time	*/
  char			promptmacro[256];
  char			logincommands[256];
  boolean		wantboards;
  char			checkboards[LEN_CHECKBOARDS+1];
  char			tracefract[256];
  char			lastroption[256];
  fbbproparrtype	fbbprop;
  time_t		lastdate;
  unsigned short	level;
  unsigned short	plevel;
  time_t		logindate;
  time_t		lastatime;
  short			sfmd2pw;	/* using MD2/MD5 authentication for SF ?		*/
  short			magic2;
  boolean		login_priv;
  short			force_priv;
  boolean		se_ok;
  boolean		hidden;
  boolean		console;
  boolean		supervisor;
  boolean		rsysop;
  boolean		ttl;
  boolean		undef;
  time_t		processtime;
  long			rbytes;
  long			sbytes;
  short			pchan;
  unsigned short	pwmode;
  short			fwdmode;	/* 0..5 */
  indexstruct		*sendheader;
  boolean		lt_required;
  short			tcon;
  short			action;
  short			sendchan;
  short			tell;
  short			sf_level;
  sfdeftype		*sf_ptr;
  boolean     	      	needs_new_sf_ptr; /* this is only needed for a reload while a running connection, */
      	      	      	      	      	  /* see reassign_sfinfos_in_userstruct in box_sf.c   	      	  */
  boolean		f_bbs;
  boolean		sf_master;
  boolean		sf_to;
  short			errors;
  short     	      	sp_input;     	/* for error check on 7plus user send 	      	*/
  boolean		laterflag;
  binsftyp		*binsfptr;
  boolean		print;
  boolean		lock_here;
  boolean		isfirstchar;	/* was there a RETURN before this line ?	*/
  unsigned short	umode;
  char			mybbsmode;	/* guessed MyBBS ?				*/
  time_t		mybbsupd;	/* timestamp for MyBBS				*/
  time_t		lastcmdtime;	/* timestamp last input				*/
  time_t		pwsetat;	/* timestamp password activation		*/
  unsigned short	emblockct;	/* for E/M blocks in THEBOX format		*/
  boolean		no_binpack;
  short			M_pos;		/* index position of userfile in M.IDX		*/
  boolean		fulltrace;
  short			trace_to;
  long			convchan;
  short			talk_to;
  boolean		is_authentic;
  boolean		newmsg;
  unsigned short	msgselection;
  boolean		in_begruessung;
  boolean		fbbmode;
  boolean		unproto_ok;
  boolean		hidebeacon;
  short			readlock;
  short			paging;		/* paging n lines				*/
  short			pagcount;	/* counter for current line			*/
  long			fstat_rx_p, fstat_rx_b, fstat_rx_s;
  long			fstat_tx_p, fstat_tx_b, fstat_tx_s;
  unsigned short	logins;
  long			sfstat_rx_p, sfstat_rx_b, sfstat_rx_s;
  long			sfstat_tx_p, sfstat_tx_b, sfstat_tx_s;
  long			srbytes, ssbytes, ssrbytes, sssbytes;
  unsigned short	sslogins;
  short			ssf_level;
  long			dstat_rx_p, dstat_rx_b, dstat_rx_s, dstat_rx_bytes;
  long			dstat_tx_p, dstat_tx_b, dstat_tx_s, dstat_tx_bytes;
  unsigned short	dlogins;
  long			maxread_day;
  long			read_today;
  short			fileout_handle;
  char			*fileout_name;
  boxintype		*inputroot;	/* pseudo multitasking administration	*/
  long			cputime;	/*   "                                  */
  long			lastprocnumber;	/*   "                                  */
  long			lastprocnumber2; /*  "                                  */
  long			lastprocnumber3; /*  "                                  */
  long			lastprocnumber4; /*  "                                  */
  long			lastprocnumber5; /*  "                                  */
  long			lastprocnumber6; /*  "                                  */
  long			lastprocnumber7; /*  "                                  */
  short			prochandle;	/*   "                                  */
  char			*procbuf;	/*   "                                  */
  long			procbufsize;	/*   "                                  */
  long			procbufseek;	/*   "                                  */
  mbxtype		tellmbx;	/*   "                                  */
  pathstr		tellfname;
  boolean		smode;		/* User is in server mode		*/
  pathstr		spath;		/* current path in server mode		*/
  short       	      	fsfinhandle;   	/* file input handle for file sf      	*/
  pathstr		sfilname;	/* file in paging mode			*/
      	      	      	      	      	/* (also filename of input file in file sf) */
  long			sseekp;		/* seek position in paged file		*/
  boolean		changed_dir;	/* show .index file ?			*/
  yapptype		*yapp;
  bintype		*bin;
  short			pty;
  short			ptynum;
  char			ptyid[4];
  time_t		ptytouch;
  boolean		ptylfcrconv;
  short			ptybuflen;
  char			ptybuffer[LEN_PTYBUF];
  pid_t			wait_pid;	/* pid of child process			*/
  pathstr		wait_file;	/* output file of child process		*/
  char			conpath[81];
  long      	      	sfspeedtime; /* in TICKSPERSEC */
  long	      	      	sfspeedsize;
  short       	      	sfspeedprops;
  boolean     	      	direct_sf;
  short			magic3;
} userstruct;

typedef userstruct	*ufcachetyp[MAXMAXUFCACHE + 1];
typedef userstruct	*userptrarrtyp[MAXUSER + 1];

vextern ufcachetyp	ufcache;
vextern userptrarrtyp	user;
vextern languagetyp	*langmemroot;
vextern clocktype	clock_;
vextern boolean		fast_machine;
vextern double		mylaenge, mybreite;
vextern boolean		myqthwwlocvalid;
vextern short		current_unr;
vextern unsigned short	wd_while_ext_prg;
vextern boolean		disk_full;
vextern long		mindiskavail;
vextern char		*whichlangmem;
vextern long		whichlangsize;
vextern boolean		wd_active;
vextern boolean		ende;
vextern char		laufwerk;
vextern boolean		dpboxcpufilled;
vextern long		dpboxcpu[DPBOXCPUARRSIZE];
vextern short		dpboxuserct[DPBOXCPUARRSIZE];
vextern indexcachetype	*indexcacheroot;
vextern zombietype	*zombieroot;
vextern boolean		immediate_extcheck;
vextern unsigned short	maxerrors;
vextern rubriktype	*rubrikroot;
vextern rejecttype	*rejectroot;
vextern dirtytype	*badwordroot;
vextern boolean		tell_waiting;
vextern unsigned short	boxtimect;
vextern time_t		lastbalise, balisetime, laststartbalise;
vextern long		balisenumber;
vextern char		*balisebuf;
vextern long		balisesize;
vextern short		baliseh1;
vextern long		ttask;
vextern sfdeftype	*sfdefs;
vextern routingtype 	*routing_root;
vextern boolean		tpkbbs;
vextern transfertype	*transferroot;
vextern bcommandtype	*bcommandroot;
vextern boolean		sf_allowed, m_filter, x_garbage_waiting;
vextern resumemem	*resume_root;
vextern unsigned short	resume_lifetime, y_lifetime;
vextern short		debug_level;
vextern long		debug_size;
vextern short		usertimeout, sftimeout;
vextern tcpiptype	*tcpiproot;
vextern boolean		ufilhide;
vextern unsigned short	max_lt_inc;
vextern boolean		scan_all_wp, gesperrt;
vextern long		erasewait, indexcaches;
vextern boolean		create_syslog, create_userlog, create_usersflog;
vextern boolean		create_readlog, create_sflog, create_convlog;
vextern boolean		remoteerasecheck, holdownfiles, authentinfo, forwerr, gttl;
vextern boolean		add_ex, small_first, sort_props, create_acks;
vextern boolean		new_remote_erases, new_mybbs_data, valid_in_ram;
vextern boolean		xcheck_in_ram, check_sffor_in_ram, no_rline_if_exterior;
vextern boolean		smart_routing, request_lt, multi_master, route_by_private;
vextern boolean		show_readers_to_everyone, new_7plus;
vextern long		remerseekp, mybbsseekp;
vextern short		usersftellmode, maxuserconnects, hiscore_connects, sub_bid;
vextern boolean		ring_bbs, charge_bbs;
vextern boolean		convtit, allow_direct_sf;
vextern convtittype	*convltroot, *convtitroot;
vextern rsysoptype	*rsysoproot;
vextern protocalltype	*protocalls;
vextern tracetype	*traces;
vextern mptrtype	*mptrroot;
vextern time_t		wpcreateinterval, wprotcreateinterval, lastwpcreate, lastwprotcreate;
vextern time_t	      	holddelay, returntime;
vextern time_t		bullsfwait, bullsfmaxage, incominglifetime, fservexpandlt;
vextern time_t	      	lastxreroute, last_wprot_r;
vextern long		all_maxread_day, badtimecount, retmailmaxbytes;
vextern short		ufcr;
vextern long		ufchit, ufcmiss;
vextern newmailtype	*newmailroot;
vextern boolean		reduce_lt_after_extract, auto7plusextract, autobinextract;
vextern boolean		allunproto, guess_mybbs, login_check, nosfpropdelaydefault;
vextern unprotodeftype	*unprotodefroot;
vextern boolean		unproto_list, md2_only_numbers, private_dirtycheck;
vextern boolean		import_logs, profile_to_syslog, do_wprot_routing;
vextern boolean       	log_ping, kill_wp_immediately;
vextern long		unproto_last, maxsubscriberdisp;
vextern long		actmsgnum, firstmsgnum, unproto_final_update, request_count;
vextern time_t		last_requesttime;
vextern short		msgnumarrct, maxufcache;
vextern long		msgnumarrblock;
vextern msgnumarrtype	msgnumarr;
vextern time_t		crawl_started;
vextern long		crawl_lastchecknum;
vextern char		crawl_args[256];
vextern pid_t		crawl_pid;
vextern boolean		crawl_active, crawler_exists, crawl_private;
vextern boolean		ana_hpath, check_rheaders, stop_invalid_rcalls, stop_broken_rlines;
vextern boolean       	stop_invalid_rdates, stop_changed_bids, stop_changed_boards;
vextern boolean       	stop_changed_mbx, stop_changed_senders;
vextern boolean		with_rline, box_pw, do_routing_stats, wrong_clock;
vextern unsigned short	sfinpdefault;
vextern boolean		bcastforward, multiprivate, send_bbsbcast, user_moncut, bulletin_moncut;
vextern unsigned short	default_lifetime;
vextern boolean		auto_garbage, auto_block, remote_erase;
vextern boolean		mail_beacon, packed_sf, hold_view, ping_allowed;
vextern long		maxbullids, bullidseek;
vextern unsigned short	garbagetime, userlifetime, packdelay, min_lifetime;
vextern fbb_montype	fbb_month;
vextern char		e_m_verteiler[9], pacsrv_verteiler[9];
vextern char		redlocal[9], redregion[9], rednation[9], reddefboard[9];
vextern short		redlifetime;
vextern short		lastproc;
vextern char		lastproctxt[256];
vextern char		current_command[300];
vextern char		current_user[20];
vextern mbxtype		ownhiername;
vextern char		ownfheader[31];
vextern char		dp_vnr[5];
vextern char		dp_vnr_sub[4];
vextern char		dp_date[11];
vextern char		myqthwwloc[7];
vextern calltype	Console_call;
vextern char		default_prompt[81];
vextern calltype	sf_connect_call;
vextern char		connecttext[81];
vextern boardtype	default_rubrik;
vextern mbxtype       	default_tpkbbs;
vextern char  	      	mustcheckboards[256];
vextern char		fbb514_fromfield[21], fbb_fromfield[21], fbb514_tofield[21], fbb_tofield[21];
vextern char  	      	other_callsyntax[256], expand_fservboard[256], xeditor[256];
vextern char  	      	accepted_bad_rcalls[256];
vextern pathstr		savedir, boxdir, fservdir, pservdir, tempdir, editprg;
vextern pathstr		spluspath, impmaildir, whotalks_lan;
vextern pathstr		eralog, boxlog, mybbs_box, hpath_box;
vextern pathstr		config_box, boxrundir, boxlanguagedir;
vextern pathstr		boxstatdir, boxprotodir, boxsfdir;
vextern pathstr		boxprocdir, boxsockdir;
vextern pathstr		badnames_box, transfer_box, autobox_dir;
vextern pathstr		bcast_box, beacon_box, commands_box;
vextern pathstr		ctext_box, debug_box, f6fbb_box, modem_box;
vextern pathstr		rubriken_box, bidseekfile, tcpip_box;
vextern pathstr		r_erase_box, profile_box, m_filter_prg;
vextern pathstr		resume_inf, dp_prg, dp_new_prg;
vextern pathstr		newmaildir, serverdir, infodir, indexdir;
vextern pathstr		extuserdir, boxsysdir, msgidlog, crawldir;
vextern pathstr		crawlname, temperase, sflist, userinfos;

#undef vextern

#endif /*BOXLOCAL_H*/

