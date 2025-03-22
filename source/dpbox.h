/* dpbox: Mailbox
   include file for main (dpbox.h)
   created: Mark Wahl DL4YBG 94/11/26
   updated: Mark Wahl DL4YBG 97/05/04
   updated: Joachim Schurig DL8HBS 96/05/30
*/

#define IF_TIMEOUT 1
#define MAX_SND_FRMS 7
#define MAX_SOCKET 50

typedef struct unproto_rxheader {
  int pid;
  int callcount;
  int heardfrom;
  char qrg[20];
  char calls[10][10];
} UNPROTO_RXHEADER;

typedef struct unproto_queue {
  int queue_len;
  struct queue_entry *first_q;
  struct queue_entry *last_q;
} UNPROTO_QUEUE;

typedef struct iface_list {
  int active;
  int status;
  int newsockfd;
  IFACE_HEADER header;
  char buffer[256];
  int sendlength;
  int blocked;
  char qrg[MAXQRGS+1][20];
  UNPROTO_QUEUE unproto_queue;
  UNPROTO_RXHEADER unproto_rxheader;
} IFACE_LIST;

typedef struct iface_data {
  short iface;
  short channel;
  short unr;
  short um_type;
  short user_type;
  short nl_type;
  int rwmode;
  char call[256];
  int linked;
  int from_box;
  char dest_call[10];
  boolean boxsf;
  char buffer[256];
  int buflen;
  time_t bufupdate;
  char rxbuffer[256];
  int rxbuflen;
  int snd_frms;
  boolean huffcod;
  char abin_filename[256];
  int fd;
  unsigned short crc;
  unsigned short crc_tmp;
  int len;
  int len_tmp;
  int queue_len;
  struct queue_entry *first_q;
  struct queue_entry *last_q;
} IFACE_DATA;

typedef struct iface_user {
  int active;
  IFACE_DATA *data;
} IFACE_USER;

