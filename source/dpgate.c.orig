/* dpgate: universal interface for DPBox
   Copyright (C) 1997 by Mark Wahl
   For license details see documentation
   Main procedure (dpgate.c)
   created: Mark Wahl DL4YBG 97/04/27
   updated: Mark Wahl DL4YBG 97/04/27
   updated: Joachim Schurig DL8HBS 99/09/27
*/

#define version_string "v0.3 99/09/27 (c) DL4YBG, changes DL8HBS"
#undef IFACE_DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <string.h>
#include <utime.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>
#include "ifacedef.h"
#include "iface.h"

static char iface_err_txt[80];
static char iface_open_txt[80];
static char sock_err_txt[80];
static char conn_err_txt[80];
static char end_err_txt[80];
char iface_active_txt[80];
char box_socket[80];
static struct iface_list if_list;
static struct iface_stat if_stat;
int box_active_flag;
int box_busy_flag;
int terminated;
int single_command;
int filemode;
int mute;
static char boxconsole_call[256];
static char outfilename[256];
static char thecommand[256];
static char thereturnname[256];
char kbdbuffer[256];
int kbdbufferlen;

static void deactivate_program();
void send_command_packet(short channel,short usernr,
                         unsigned short len,char *buf);
void flush_buf(int forget);
void write_iface(int len,char *str);

void keyboard_server(char ch)
{
  if (mute) return;
  if (ch == 0x08) {
    if (kbdbufferlen) {
      puts("\008 \008");
      kbdbufferlen--;
    }
    return;
  }
  if (kbdbufferlen > 255) {
    putchar(0x07);
    return;
  }
  if ((ch == '\r') || (ch == '\n')) {
    kbdbuffer[kbdbufferlen] = '\r';
    kbdbufferlen++;
    write_iface(kbdbufferlen,kbdbuffer);
    kbdbufferlen = 0;
    return;
  }
  kbdbuffer[kbdbufferlen] = ch;
  kbdbufferlen++;
}

void cmd_display(char *string,int crlf)
{
  if (mute) return;
  fputs(string,stderr);
  if (crlf) fputs("\n",stderr);
}

#define copybuflen 4096
static void move_file(char *from, char *to)
{
  long size;
  FILE *in, *out;
  char buf[copybuflen];
  
  if ((in = fopen(from, "rb")) == NULL) return;
  if (*to == '\0') out = stdout;
  else out = fopen(to, "wb");
  if (out != NULL) {
    while ((size = fread(buf, 1, copybuflen, in)) > 0)
      fwrite(buf, 1, size, out);
    if (out != stdout) {
      fclose(out);
      chmod(to, 0666);
    }
  }
  unlink(from);
}
#undef copybuflen

void display_buf(char *buf,int len)
{
  int i,l;
  
  if (single_command || filemode) {
    i = 0;
    l = strlen(thereturnname);
    while (i < len) {
      if (buf[i] == '\r' || buf[i] == '\n') {
        if (*outfilename != '\0' && l > 0) {
          move_file(thereturnname, outfilename);
        } else if (l > 0) {
          if (filemode) {
            chmod(thereturnname, 0666);
            fputs(thereturnname, stdout);
            fputs("\n", stdout);
            fflush(stdout);
          } else if (single_command) {
            move_file(thereturnname, "");
          }
        }
        *thereturnname = '\0';
        l = 0;
      } else {
        if (l < 255) {
          thereturnname[l++] = buf[i];
          thereturnname[l] = '\0';
        }
      }
      i++;
    }
    return;
  }
  
  i = 0;
  while (i < len) {
    if (buf[i] == '\r') {
      putchar('\n');
    }
    else if (isprint(buf[i]))
      putchar(buf[i]);
    else
      putchar('.');
    i++;
  }
  fflush(stdout);
}

/* close iface connection on channel */
void close_iface_con(deact)
int deact;
{
  if (if_stat.iface != -1) {
    if (deact) deactivate_program();
    flush_buf(1);
    if_stat.iface = -1;
    terminated = 1;
  }
}

/* close the iface-socket */
static void close_iface(deact)
int deact;
{
  struct queue_entry *oldq_ptr;
  
  if (if_list.active == IF_ACTIVE) {
    if (if_stat.iface != -1) {
      close_iface_con(deact);
    }
    box_active_flag = 0;
    box_busy_flag = 0;

    while (if_list.first_q != NULL) {
      oldq_ptr = if_list.first_q;
      free(oldq_ptr->buffer);
      if_list.first_q = oldq_ptr->next;
      free((char *)oldq_ptr);
    }
    close(if_list.sockfd);
    if_list.active = IF_NOTACTIVE;
  }
}

static void blocking_test(len)
int len;
{
  IFACE_CMDBUF command;
  
  if_list.sendlength += len;
  if (if_list.sendlength > BLOCKING_SIZE) {
    if_list.blocked = 1;
    command.command = CMD_BLOCK;
    send_command_packet(NO_CHANNEL,NO_USERNR,LEN_SIMPLE,
                        (char *)&command);
  }
}

static void unblocking()
{
  struct queue_entry *oldq_ptr;
  
  if (if_list.active != IF_ACTIVE) return;
  if_list.blocked = 0;
  if_list.sendlength = 0;
  while ((!if_list.blocked) &&
         (if_list.first_q != NULL)) {
    oldq_ptr = if_list.first_q;
    if (write(if_list.sockfd,oldq_ptr->buffer,
              oldq_ptr->len) < oldq_ptr->len) {
      close_iface(0);
      return;
    }
    blocking_test(oldq_ptr->len);
    free(oldq_ptr->buffer);
    if_list.first_q = oldq_ptr->next;
    free((char *)oldq_ptr);
  }
}

/* analysis of received packet via interface */
static void packet_analysis(indicator,channel,usernr,len,buf)
char indicator;
int channel;
int usernr;
int len;
char *buf;
{
  IFACE_CMDBUF *rec_command;
  IFACE_CMDBUF command;
  
  if ((channel != NO_CHANNEL) && (channel != 0)) return;
  if ((channel == 0) && (if_stat.iface == -1)) return;
  switch (indicator) {
  case IF_COMMAND:
    rec_command = (IFACE_CMDBUF *)buf;
    switch (rec_command->command) {
    case CMD_CONNECT:
      break;
    case CMD_DISCONNECT:
      break;
    case CMD_FINISH:
      if (channel == NO_CHANNEL) return;
      close_iface_con(0);
      break;
    case CMD_BLOCK:
      command.command = CMD_UNBLOCK;
      send_command_packet(NO_CHANNEL,NO_USERNR,LEN_SIMPLE,(char *)&command);
      break;
    case CMD_UNBLOCK:
      unblocking();
      break;
    case CMD_BOXPBOXSF:
      break;
    case CMD_BOXPRDISC:
      close_iface_con(1);
      break;
    case CMD_BOXPABORTSF:
      break;
    case CMD_ACT_RESP:
      if (channel == NO_CHANNEL) return;
      if ((len-LEN_SIMPLE) > 0) {
        if (!mute)
          display_buf(rec_command->data.buffer,len-LEN_SIMPLE);
      }
      if (usernr == NO_USERNR) {
        close_iface_con(0);
      }
      else {
        if_stat.usernr = usernr;
        flush_buf(0);
      }
      break;
    case CMD_HUFFSTAT:
      break;
    case CMD_BULLID:
      break;
    case CMD_BOXISBUSY:
      break;
    case CMD_STARTBOXBC:
      break;
    case CMD_SETUNPROTO:
      break;
    case CMD_ABORTCON:
      break;
    case CMD_TNTCOMMAND:
      break;
    default:
      break;
    }
    break;
  case IF_DATA:
    if (channel == NO_CHANNEL) return;
    display_buf(buf,len);
    break;
  case IF_UNPROTO:
    break;
  default:
    break;
  }
}

void send_command_packet(short channel,short usernr,
                         unsigned short len,char *buf)
{
  char buffer[MAX_LEN];
  int res;
  IFACE_HEADER header;

  if (if_list.active != IF_ACTIVE) return;
  if (len > IFACE_PACLEN) return;
  header.indicator = IF_COMMAND;
  header.channel = channel;
  header.usernr = usernr;
  header.len = len;
  memcpy(buffer,(char *)&header,HEAD_LEN);
  memcpy(buffer + HEAD_LEN,buf,len);
  if ((res = write(if_list.sockfd,buffer,len+HEAD_LEN)) < len+HEAD_LEN) {
    close_iface(0);
    return;
  }
}

static void activate_program()
{
  int len;
  IFACE_CMDBUF command;
  int um_type;
  int user_type;

  command.command = CMD_ACTIVATE;
  len = LEN_SIMPLE;
  if (single_command) um_type = 7;
  else if (filemode) um_type = 6;
  else um_type = 5;
  user_type = 0;
  if (*boxconsole_call != '\0') { /* fill callsign for boxlogin */
    strcpy(command.data.buffer,boxconsole_call);
    len += strlen(boxconsole_call);
    *(command.data.buffer + len - LEN_SIMPLE) = '\0';
    len++;
    memcpy(command.data.buffer + len - LEN_SIMPLE,(char *)&um_type,
           sizeof(int));
    len += sizeof(int);
    memcpy(command.data.buffer + len - LEN_SIMPLE,(char *)&user_type,
           sizeof(int));
    len += sizeof(int);
  }
  send_command_packet(0,NO_USERNR,len,(char *)&command);
}

static void deactivate_program()
{
  IFACE_CMDBUF command;
  
  command.command = CMD_DEACTIVATE;
  send_command_packet(0,if_stat.usernr,LEN_SIMPLE,(char *)&command);
}

/* write data to interface */
void write_iface(int len,char *str)
{
  char buffer[MAX_LEN];
  int res;
  char *ptr;
  struct buf_entry *b_ptr;
  struct queue_entry *q_ptr;
  IFACE_HEADER header;

  if (len > IFACE_PACLEN) return;

  if (if_list.active != IF_ACTIVE) return;  

  /* No response to activate program was received, queue all frames */  
  if (if_stat.usernr == NO_USERNR) {
    b_ptr = (struct buf_entry *)malloc(sizeof(struct buf_entry));
    if (b_ptr == NULL) {
      close_iface(0);
      return;
    }
    ptr = (char *)malloc(len);
    if (ptr == NULL) {
      free(b_ptr);
      close_iface(0);
      return;
    }
    memcpy(ptr,str,len);
    b_ptr->buf = ptr;
    b_ptr->len = len;
    b_ptr->next = NULL;
    if (if_stat.first_q == NULL) {
      if_stat.first_q = b_ptr;
    }
    else {
      if_stat.last_q->next = b_ptr;
    }
    if_stat.last_q = b_ptr;
    return;
  }

  header.indicator = IF_DATA;
  header.channel = 0;
  header.usernr = if_stat.usernr;
  header.len = len;
  memcpy(buffer,(char *)&header,HEAD_LEN);
  memcpy(&buffer[HEAD_LEN],str,len);
  if (if_list.blocked) {
    q_ptr = (struct queue_entry *)malloc(sizeof(struct queue_entry));
    if (q_ptr == NULL) {
      close_iface(0);
      return;
    }
    ptr = (char *)malloc(len+HEAD_LEN);
    if (ptr == NULL) {
      free(q_ptr);
      close_iface(0);
      return;
    }
    memcpy(ptr,buffer,len+HEAD_LEN);
    q_ptr->buffer = ptr;
    q_ptr->len = len+HEAD_LEN;
    q_ptr->next = NULL;
    if (if_list.first_q == NULL) {
      if_list.first_q = q_ptr;
    }
    else {
      if_list.last_q->next = q_ptr;
    }
    if_list.last_q = q_ptr;
  }
  else {
    if ((res = write(if_list.sockfd,
                   buffer,len+HEAD_LEN)) < len+HEAD_LEN) {
      close_iface(0);
      return;
    }
    blocking_test(len+HEAD_LEN);
  }
}

void flush_buf(forget)
int forget;
{
  struct buf_entry *b_ptr;
  
  if (if_stat.iface == -1) return;
   
  while (if_stat.first_q != NULL) {
    b_ptr = if_stat.first_q;
    if_stat.first_q = b_ptr->next;
    if (!forget && (if_stat.usernr != NO_USERNR)) {
      write_iface(b_ptr->len,b_ptr->buf);
    }
    free(b_ptr->buf);
    free(b_ptr);
  }
  if_stat.last_q = NULL;
}

/* read data from interface */
static int read_data_iface()
{
  char buffer[MAX_LEN];
  char *bufptr;
  int len;
  int remain;
#ifdef IFACE_DEBUG
  unsigned short tmplen;
#endif
  

  if (if_list.active != IF_ACTIVE) return(0);  
  len = read(if_list.sockfd,buffer,MAX_LEN);
  if (len == -1) {
    if (errno != EAGAIN) {
      close_iface(0);
    }
    return(0);
  }
  if (len == 0) {
    close_iface(0);
  }
  bufptr = buffer;
  remain = len;
  while (remain) {
    if (if_list.status < HEAD_LEN) {
      *(((char *)&if_list.header) + if_list.status) = *bufptr;
      if_list.status++;
      remain--;
      bufptr++;
    }
    else {
      /* packet data */
      if (if_list.header.len > 0) {
        if_list.buffer[if_list.status - HEAD_LEN] = *bufptr;
        if_list.status++;
      }
      if ((if_list.status - HEAD_LEN) == if_list.header.len) {
        if_list.status = 0;
#ifdef IFACE_DEBUG
        tmplen = 0;
        fprintf(stderr,"IF: %d, ind: %d, ch: %d, us: %d, len: %d\n",
                iface,
                if_list.header.indicator,
                if_list.header.channel,
                if_list.header.usernr,
                if_list.header.len);
        while (tmplen < if_list.header.len) {
          fprintf(stderr,"%x ",if_list.buffer[tmplen]);
          tmplen++;
        }
        fprintf(stderr,"\n");
#endif
        packet_analysis(if_list.header.indicator,
                        if_list.header.channel,
                        if_list.header.usernr,
                        if_list.header.len,
                        if_list.buffer);
      }
      remain--;
      bufptr++;
    }
  }
  return(1);
}

/* dummy proc for alarm */
void sigalarm()
{
}

/* install link to other program */
void cmd_iface()
{
  int sockfd;
  int servlen;
  struct sockaddr_un serv_addr;

  if (if_list.active == IF_ACTIVE) {
    cmd_display(iface_err_txt,1);
    return;
  }

  strcpy(if_list.name,box_socket);

  /* fill structure "serv_addr" */
  memset((char *) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path,if_list.name);
/*  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family); */
  servlen = sizeof(serv_addr); /* VK5ABN */
  /* open socket */
  if ((sockfd = socket(AF_UNIX,SOCK_STREAM,0)) < 0) {
    cmd_display(sock_err_txt,1);
    return;
  }
  signal(SIGALRM,sigalarm);
  alarm(2);
  /* connect other program */
  if (connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0) {
    close(sockfd);
    cmd_display(conn_err_txt,1);
    signal(SIGALRM,SIG_IGN);
    return;
  }
  signal(SIGALRM,SIG_IGN);

  if_list.active = IF_ACTIVE;
  if_list.sockfd = sockfd;
  if_list.status = 0;
  if_list.blocked = 0;
  if_list.sendlength = 0;
  if_list.unproto_address[0] = '\0';
  box_active_flag = 1;
  box_busy_flag = 0;

/*  cmd_display("Ok",1); */
}

/* cut link to other program */
void cmd_endiface()
{
  if (if_list.active == IF_ACTIVE) {
    close_iface(1);
  }
}

/* activate link to other program */
void cmd_actiface()
{
  if (if_stat.iface != -1) {
    cmd_display(iface_active_txt,1);
    return;
  }
  if (if_list.active == IF_ACTIVE) {
    if_stat.iface = 0;
    if_stat.usernr = NO_USERNR;
    if_stat.first_q = NULL;
    if_stat.last_q = NULL;

    activate_program();
/*    cmd_display("Activating program",1); */
    return;
  }
}

/* deactivate link to other program */
void cmd_deactiface()
{
  if (if_list.active == IF_ACTIVE) {
    close_iface_con(1);
    cmd_display("Ok",1);
    return;
  }
}

/* init of iface-variables */
void init_iface()
{
  if_list.active = IF_NOTACTIVE;
  if_stat.iface = -1;

  strcpy(iface_err_txt,"All interfaces in use");
  strcpy(iface_open_txt,"Interface already open");
  strcpy(sock_err_txt,"Can't open stream socket, will try later again");
  strcpy(conn_err_txt,"Can't connect to program, will try later again");
  strcpy(end_err_txt,"Interface not active");
  strcpy(iface_active_txt,"Interface already active");

  *boxconsole_call = '\0';
  box_active_flag = 0;
  box_busy_flag = 0;
  terminated = 0;
  *outfilename = '\0';
  *thecommand = '\0';
  *thereturnname = '\0';
  single_command = 0;
  filemode = 0;
  mute = 0;

  signal(SIGALRM,SIG_IGN);
}

/* exit of iface, close all sockets */
void exit_iface()
{
  close_iface(1);
  signal(SIGALRM,SIG_IGN);
}

static int in_args(int argc, char *argv[], char *searched, char *nextarg)
{
  int x;
  
  for (x = 1; x < argc; x++) {
    if (!strcmp(searched, argv[x])) {
      if (++x < argc) {
        strncpy(nextarg, argv[x], 255);
        nextarg[255] = '\0';
        return 2;
      }
      *nextarg = '\0';
      return 1;
    }
  } 
  return 0;
}

static void convert_cgi_command(char *s)
{
  int x, changed, r;
  char out[256], h[3], *o, *i;
  
  changed = 0;
  x = 0;
  o = out;
  i = s;
  while (*i) {
    if (*i == '%' && strlen(i) > 2 && i[1] != '%') {
      i++;
      h[0] = toupper(*i++);
      h[1] = toupper(*i++);
      h[2] = '\0';
      r = 32;
      sscanf(h, "%X", &r);
      *o++ = (char) r;
      changed = 1;
    } else {
      *o++ = *i++;
    }
  }
  if (changed) {
    *o = '\0';
    strcpy(s, out);
  }
}

int main(int argc,char *argv[])
{
  fd_set fdmask;
  struct timeval timevalue;
  int max_fd;
  int count;
  char ch;
  char buffer[256];
  char *ptr;
  
  if (argc > 1 && !strcmp(argv[1], "-v")) {
    fputs(version_string, stdout);
    fputs("\n", stdout);
    fflush(stdout);
    exit(0);
  }

  strcpy(box_socket,"/var/run/dpbox");
  init_iface();

  if (in_args(argc, argv, "-s", buffer) == 2) {
    buffer[79] = '\0';
    strcpy(box_socket, buffer);
  }
  
  if (in_args(argc, argv, "-u", buffer) == 2)
    strcpy(boxconsole_call, buffer);

  if (in_args(argc, argv, "-c", buffer) == 2) {
    strcpy(thecommand, buffer);
    if (*thecommand != '\0') {
      if (in_args(argc, argv, "-i", buffer) > 0)
        convert_cgi_command(thecommand);
      strcat(thecommand, "\r");
      single_command = 1;
      mute = 1;
    }
  }
  
  if (filemode && *boxconsole_call == '\0')
    exit(1);

  if (*boxconsole_call == '\0') {
    strcpy(boxconsole_call,"G0UEST");
    printf("Enter your callsign: ");
    fflush(stdout);
    ptr = fgets(buffer,256,stdin);
    if (ptr != NULL) {
      ptr = strchr(buffer,'\n');
      if (ptr != NULL) *ptr = '\0';
      if (buffer[0] != '\0') {
        strcpy(boxconsole_call,buffer);
      }
    }
  }

  cmd_iface();
  cmd_actiface();
  
  while (!terminated && (if_list.active == IF_ACTIVE)) {
    FD_SET(0,&fdmask);
    max_fd = 1;
    if (if_list.active == IF_ACTIVE) {
      FD_SET(if_list.sockfd,&fdmask);
      if (if_list.sockfd > ((max_fd) - 1)) {
        max_fd = if_list.sockfd + 1;
      }
    }
    timevalue.tv_usec = 0;
    timevalue.tv_sec = 60;
    count = select(max_fd,&fdmask,
                   (fd_set *) 0,(fd_set *) 0,&timevalue);
    if (count == -1) {
      continue;
    }
    
    if (single_command && *thecommand != '\0') {
      write_iface(strlen(thecommand), thecommand);
      *thecommand = '\0';
    }

    if (FD_ISSET(0,&fdmask)) {
      if (read(0,&ch,1) == 1) {
        keyboard_server(ch);
      }
    }
    if (if_list.active == IF_ACTIVE) {
      if (FD_ISSET(if_list.sockfd,&fdmask)) {
        read_data_iface();
      }
    }
    
  }
  
  cmd_endiface();
  exit_iface();
  return 0;
}
