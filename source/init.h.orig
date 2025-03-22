/* dpbox: Mailbox
   include file for initialization (init.h)
   created: Mark Wahl DL4YBG 94/07/29
   updated: Mark Wahl DL4YBG 96/01/13
   updated: Joachim Schurig DL8HBS 99/09/24
*/

/* files used for init */
#define INIT_FILE "dpbox.ini"

/* default-values if init-file not found or corrupt */
#define DEF_BOX_PACLEN 235

#define DEF_CONSOLE_CALL "AN0NYM"
#define DEF_MYQTHWWLOC "XX00XX"
#define DEF_BOX_SSID 8
#define DEF_NODE_SSID 0

#define DEF_OWNHIERNAME "NOCALL"
#define DEF_BOXDIR "./box/"
#define DEF_TEMPDIR "/tmp/"

#define DEF_BOXSYSDIR "system/"
#define DEF_NEWMAILDIR "newmail/"
#define DEF_IMPMAILDIR "import/"
#define DEF_INFODIR "infofiles/"
#define DEF_INDEXDIR "indexes/"
#define DEF_BOXSTATDIR "stat/"
#define DEF_BOXPROTODIR "proto/"
#define DEF_BOXSFDIR "sf/"
#define DEF_BOXRUNDIR "run/"
#define DEF_BOXLANGUAGEDIR "language/"
#define DEF_SAVEDIR "save/"
#define DEF_SERVERDIR "server/"

#ifdef __NetBSD__
  #define DEF_BOXPROCDIR "/var/run/"
  #define DEF_BOXSOCKDIR "/var/run/"
#else /* Have to preserve the old setting for running installations */
      /* They might have incomplete config files (dpbox.ini)        */
  #define DEF_BOXPROCDIR "stat/"
  #define DEF_BOXSOCKDIR "stat/"
#endif

#define DEF_PRGDIR "./"
#define DEF_PROC_FILE "dpbox.pid"
#define DEF_WATCH_FILE "watchdog.dp"
#define DEF_MSGIDLOG "msgidlog.dp"
#define DEF_MYBBS_BOX "mybbs.box"
#define DEF_HPATH_BOX "hpath.box"
#define DEF_CONFIG_BOX "config.box"
#define DEF_BADNAMES_BOX "badnames.box"
#define DEF_TRANSFER_BOX "transfer.box"
#define DEF_BCAST_BOX "bcast.box"
#define DEF_BEACON_BOX "beacon.box"
#define DEF_COMMANDS_BOX "commands.box"
#define DEF_CTEXT_BOX "ctext.box"
#define DEF_DEBUG_BOX "debug.box"
#define DEF_F6FBB_BOX "f6fbb.box"
#define DEF_RUBRIKEN_BOX "rubriken.box"
#define DEF_TCPIP_BOX "tcpip.box"
#define DEF_R_ERASE_BOX "r_erase.box"
#define DEF_PROFILE_BOX "profile.box"
#define DEF_WHOTALKS_LAN "whotalks.lan"
#define DEF_M_FILTER_PRG "m_filter.prg"
#define DEF_RESUME_INF "resume.inf"
#define DEF_DP_PRG "dpbox"
#define DEF_DP_NEW_PRG "dpbox.new"
#define DEF_BOX_SOCKET "socket"
#define DEF_DPBOX_USER ""

extern int box_paclen;
extern char dpbox_initfile[256];

extern int read_init_file(int argc,char *argv[]);
extern void init_watch(void);
extern int init_proc(void);
extern void exit_proc(void);
extern void exit_watch(void);
extern boolean check_watch(void);

