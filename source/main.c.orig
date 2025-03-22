
/* dpbox: Mailbox
   Main procedure (main.c)
   created: Mark Wahl DL4YBG 94/06/20
   updated: Mark Wahl DL4YBG 97/05/04
   updated: Joachim Schurig DL8HBS 00/01/14
*/

/* definition of version-number */

#define DP_DATE		"27.04.2000"
#define DP_VNR		"6.00"
#define DP_VNR_SUB	".00"


#ifdef __macos__
#undef IFACE_DEBUG
#endif
#ifdef __linux__
#undef IFACE_DEBUG
#endif
#ifdef __NetBSD__
#undef IFACE_DEBUG
#endif

#define MAIN_G

#if defined(__linux__) || defined(__NetBSD__)
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "main.h"
#include "init.h"
#include "box_init.h"
#include "boxglobl.h"
#include "boxlocal.h"
#include "box_logs.h"
#include "ifacedef.h"
#include "dpbox.h"
#include "status.h"
#include "tools.h"
#include "shell.h"
#include "box.h"
#include "crc.h"
#include "box_sys.h"
#include "box_sub.h"
#include "box_send.h"
#include "box_inou.h"
#include "box_file.h"
#include "box_sf.h"
#include "box_mem.h"
#include "pastrix.h"
#include "filesys.h"
#include "boxbcast.h"
#include "box_wp.h"
#endif

#ifdef __macos__
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stat.h>
#include <unix.h>
#include <string.h>
#include <time.h>
#include <types.h>
#include <signal.h>
#include <unistd.h>
#include "main.h"
#include "init.h"
#include "box_init.h"
#include "boxglobl.h"
#include "boxlocal.h"
#include "box_logs.h"
#include "ifacedef.h"
#include "dpbox.h"
#include "status.h"
#include "tools.h"
#include "shell.h"
#include "box.h"
#include "crc.h"
#include "box_sys.h"
#include "box_sub.h"
#include "box_send.h"
#include "box_inou.h"
#include "box_file.h"
#include "box_sf.h"
#include "box_mem.h"
#include "pastrix.h"
#include "filesys.h"
#include "boxbcast.h"
#include "box_wp.h"
#endif

char filestring[256];
static int finish_box;
static int reload_configs;
extern int dpbox_debug;
extern char dpbox_debugfile[];
static FILE *error_fp;
extern char prgdir[];
static IFACE_LIST iface_list[MAX_SOCKET];
static IFACE_USER iface_user[MAXUSER+1];
char box_socket[80];
char dpbox_user[80];
static int sockfd;
extern int box_paclen;
static int no_blocking_flag;
short box_ssid;
short node_ssid;

void write_iface();
void write_iface_packet();
static void write_iface_unproto();
static void send_command_packet();
static void send_command_iface();
static void exit_iface_user();
static void close_iface();
static void queue_iface_cmd(short usernr, char *cmdbuffer, int cmdKulen);
static void queue_iface_unproto();

/* convert result of 'system()' to exit-value */
short statusconvert(int status)
{
#ifdef __macos__
  return(status);
#else
  short Result;
  
  if (WIFEXITED(status)) {
    Result = WEXITSTATUS(status);
  }
  else {
    Result = -1;
  }
  return(Result);
#endif
}

#define BUFSIZE 16384
static int add_cr(char *filename,unsigned short *bodychecksum)
{
  char *srcbuffer;
  char *destbuffer;
  char *destptr;
  int srclen;
  int destlen;
  char tmpname[80];
  int srcfd;
  int destfd;
  int result;
  char ch;
  int binpos;
  int prebin;
  int bin;
  int end;
  int error;
  int i;
  
  srcbuffer = (char *)malloc(BUFSIZE);
  destbuffer = (char *)malloc(2*BUFSIZE);
  if ((srcbuffer == NULL) || (destbuffer == NULL)) return(0);

  strcpy(tmpname,filename);
  strcat(tmpname,"XXXXXX");
  mymktemp(tmpname);
  
  srcfd = open(filename,O_RDONLY);
  if (srcfd == -1) {
    free(srcbuffer);
    free(destbuffer);
    return(0);
  }
  destfd = dpsyscreate(tmpname,O_RDWR|O_CREAT|O_TRUNC,FC_FILE_RWALL);
  if (destfd == -1) {
    free(srcbuffer);
    free(destbuffer);
    close(srcfd);
    return(0);
  }
  end = 0;
  error = 0;
  binpos = 0;
  prebin = 0;
  bin = 0;
  *bodychecksum = 0;
  while ((!end) && (!error)) {
    srclen = read(srcfd,srcbuffer,BUFSIZE);
    if (srclen == -1) error = 1;
    if (!error) {
      destlen = 0;
      destptr = destbuffer;
      if (srclen < BUFSIZE) end = 1;
      for (i=0;i<srclen;i++) {
        ch = srcbuffer[i];
        if ((!bin) && (ch == '\n')) {
          *destptr = '\r';
          *bodychecksum += *destptr;
          destlen++;
          destptr++;
          *destptr = ch;
          *bodychecksum += *destptr;
          destlen++;
          destptr++;
          if (prebin) bin = 1;
          else binpos = 1;
        }
        else {
          *destptr = ch;
          *bodychecksum += *destptr;
          destlen++;
          destptr++;
          if (!bin) {
            switch (binpos) {
            case 0:
              break;
            case 1:
              if (ch == '#') binpos = 2;
              else binpos = 0;
              break;
            case 2:
              if (ch == 'B') binpos = 3;
              else binpos = 0;
              break;
            case 3:
              if (ch == 'I') binpos = 4;
              else binpos = 0;
              break;
            case 4:
              if (ch == 'N') binpos = 5;
              else binpos = 0;
              break;
            case 5:
              if (ch == '#') {
                prebin = 1;
                binpos = 0;
              }
              else binpos = 0;
              break;
            default:
              binpos = 0;
              break;
            }
          }
        }
      }
      result = write(destfd,destbuffer,destlen);
      if ((result == -1) || (result < destlen)) error = 1;
    }
  }
  free(srcbuffer);
  free(destbuffer);
  close(srcfd);
  close(destfd);
  if (error) {
    sfdelfile(tmpname);
    return(0);
  }
  else {
    sfdelfile(filename);
    sfrename(tmpname,filename);
    return(1);
  }
}

short calc_usernr(console,channel)
{
  short usernr;
  
  if (channel > 0) {
    if (console == 0) {
      usernr = channel;
    }
    else {
      return(0);
    }
  }
  else {
    if (console > 0 ) {
      usernr = console;
    }
    else {
      return(0);
    }
  }
  if (iface_user[usernr].active) return (usernr);
  else return(0);
}



/* reception of an autobin-file */
void abin_rcv(usernr,pos,len,buf)
short usernr;
unsigned short *pos;
unsigned short len;
char *buf;
{
  int end_abin;
  int wrklen;
  int buflen;
  char *bufptr;
  int tmp;
  short unr;
  char ans_str[80];
  char binheader[256];
  long stc;
  long filelen;
  int error;
  char binheadrest[259];
  
  if (iface_user[usernr].active) {
    unr = iface_user[usernr].data->unr;
    box_ttouch(unr);
    end_abin = 0;
    buflen = len - *pos;
    bufptr = buf + *pos;
    wrklen = buflen;
    tmp = iface_user[usernr].data->len - iface_user[usernr].data->len_tmp;
    if (tmp <= buflen) {
      wrklen = tmp;
      end_abin = 1;
    }
    *pos += wrklen;
    iface_user[usernr].data->len_tmp += wrklen;
    crcthp_buf(bufptr,wrklen,&(iface_user[usernr].data->crc_tmp));
    tmp = write(iface_user[usernr].data->fd,bufptr,wrklen);
    if (tmp < wrklen) {
      /* write error on file */
      close(iface_user[usernr].data->fd);
      sfdelfile(iface_user[usernr].data->abin_filename);
      iface_user[usernr].data->rwmode = RW_NORMAL;
      strcpy(ans_str,"*** write error\r");
      write_iface(usernr,strlen(ans_str),ans_str);
      user[unr]->action = 15; /* ABORT */
    }
    else if (end_abin) {
      close(iface_user[usernr].data->fd);
      error = 1;
      if ((iface_user[usernr].data->crc == iface_user[usernr].data->crc_tmp) ||
          (iface_user[usernr].data->crc == 0)) {
        error = 0;
      }
      switch (user[unr]->action) {
      case 0:
        /* binary file sent to prompt, ignore */
        break;
      case 87: /* write file */
      case 88: /* update DP */
      case 89:
        if (error) {
          sfdelfile(iface_user[usernr].data->abin_filename);
          iface_user[usernr].data->rwmode = RW_NORMAL;
          strcpy(ans_str,"*** invalid crc\r");
          write_iface(usernr,strlen(ans_str),ans_str);
          user[unr]->action = 15; /* ABORT */
        }
        else {
          if ((user[unr]->action == 88) ||
              (user[unr]->action == 89)) {
            update_dp2(unr);
          }
          iface_user[usernr].data->rwmode = RW_NORMAL;
          user[unr]->errors = -5;
          user[unr]->action = 0;
          show_prompt(unr);
        }
        break;      
      default: /* binary file to box */
        iface_user[usernr].data->rwmode = RW_NORMAL;
        user[unr]->errors = -5;
        if (error) {
          sfdelfile(iface_user[usernr].data->abin_filename);
          /* pipe-character for DIEBOX */
          strcpy(ans_str,"|>\r");
          write_iface(usernr,strlen(ans_str),ans_str);
          strcpy(ans_str,"*** invalid crc\r");
          write_iface(usernr,strlen(ans_str),ans_str);
          user[unr]->action = 15; /* ABORT */
        }
        else {
          box_rtitle(false,unr,binheadrest);
          stc = get_cpuusage();
          sprintf(binheader,"#BIN#%d#|%d%s",
                  iface_user[usernr].data->len_tmp,
                  iface_user[usernr].data->crc_tmp,
                  binheadrest);
          str2file(&(user[unr]->sendchan),binheader,1);
          filelen = sfsize(iface_user[usernr].data->abin_filename);
          app_file(iface_user[usernr].data->abin_filename,
                   box_s_chan(unr),true);
          upd_statistik(unr,filelen+strlen(binheader)+1,0,stc,get_cpuusage());
          box_input(unr,false,"\032",true);
        }
        break;
      }
    }
  }
}

/* interface-procedures */
void iboxendbox(short console)
{
  IFACE_CMDBUF command;

  command.command = CMD_FINISH;
  send_command_packet(console,LEN_SIMPLE,(char *)&command);
  exit_iface_user(console);
}

void iboxpacketendbox(short channel)
{
  IFACE_CMDBUF command;
  
  command.command = CMD_FINISH;
  send_command_packet(channel,LEN_SIMPLE,(char *)&command);
  exit_iface_user(channel);
}

/* in boxglobl.c */
long iboxaktqrg()
{
  return 438300;
}

/* in boxglobl.c */
unsigned short iboxruntime()
{
  return 1;
}

void iboxcheckdiskspace()
{
}

short iboxunackframes(short channel)
{
  return 0;
}

/* number of tncs */
short iboxtnccount()
{
  return 1;
}

void iboxsetunproto(short tnc,char *str,short iface,char *qrg)
{
  int len;
  int hlp;
  IFACE_CMDBUF command;
      
  if (!iface_list[iface].active) return;

  command.command = CMD_SETUNPROTO;
  command.data.setunproto.qrg[0] = '\0';
  if (strlen(qrg) < 20)
    strcpy(command.data.setunproto.qrg,qrg);
  len = LEN_SETUNPROTO + 1;
  command.data.setunproto.address[0] = '\0';
  hlp = strlen(str);
  if (hlp < (256-LEN_SETUNPROTO)) {
    strcpy(command.data.setunproto.address,str);
    len += hlp;
  }
  queue_iface_unproto(iface,(char *)&command,len,1);
}

void iboxsendpline(short channel,char *str,boolean withcr,short iface)
{
  char *buf;
  
  buf = (char *)malloc(strlen(str)+2);
  if (buf == NULL)
    return;
  strcpy(buf,str);
  if (withcr) strcat(buf,"\r");
  write_iface_unproto(iface,buf,strlen(buf));
  free(buf);
}

/* get monitor channel of specified tnc */
short iboxgettncmonchan(short tnc)
{
  return 0;
}

/* in boxglobl.c */
void iboxsetmonstat(short tnc,char *str)
{
}

void iboxsetunattmode(short tnc,char *str)
{
}

unsigned short iboxgetpaclen(short channel)
{
  return((unsigned short)box_paclen);
}

void iboxprwprg2(short usernr,char *str)
{
  int umw;
  unsigned int crc;
  int len;
  char reststring[257];
  char binheadrest[259];
  char filename[256];
  int fd;
  char ans_str[80];
  
  if (iface_user[usernr].active) {
    if (strncmp(str,"#BIN#",5) == 0) {
      umw = sscanf(str,"#BIN#%d#|%d#$%s",&len,&crc,reststring);
      /* do not allow negative length */
      if (umw >= 1) {
        if (len < 0) umw = 0;
      }
      /* if #BIN#-string was not successfully parsed */
      if (umw < 0) umw = 0;
      /* if no crc received, set it to 0 */
      if (umw == 1) crc = 0;
      /* set date and filename of binheader */
      binheadrest[0] = '\0';
      if (umw == 3) {
        sprintf(binheadrest,"#$%s",reststring);
      }
      /* now open file and init of autobin-data */
      if ((umw >= 1) && (umw <= 3)) {
        if (iface_user[usernr].data->rwmode == RW_NORMAL) {
          strcpy(filename,tempdir);
          strcat(filename,"abinboxXXXXXX");
          mymktemp(filename);
          strcpy(iface_user[usernr].data->abin_filename,filename);
          iface_user[usernr].data->rwmode = RW_ABIN_WAIT;
        }
        box_rtitle(true,iface_user[usernr].data->unr,binheadrest);
        fd = dpsyscreate(iface_user[usernr].data->abin_filename,
                  O_RDWR|O_CREAT|O_TRUNC,FC_FILE);
        if (fd == -1) {
          if (iface_user[usernr].data->boxsf) {
            abort_sf(iface_user[usernr].data->unr,
                     false,"*** cannot create file");
          }
          else {
            strcpy(ans_str,"\r#ABORT# cannot create file");
            wlnuser(iface_user[usernr].data->unr,ans_str);
            iface_user[usernr].data->rwmode = RW_NORMAL;
          }
        }
        else {
          if (!iface_user[usernr].data->boxsf) {
            strcpy(ans_str,"#OK#");
            wlnuser(iface_user[usernr].data->unr,ans_str);
          }
          iface_user[usernr].data->fd = fd;
          iface_user[usernr].data->len = len;
          iface_user[usernr].data->crc = crc;
          iface_user[usernr].data->len_tmp = 0;
          iface_user[usernr].data->crc_tmp = 0;
          iface_user[usernr].data->rwmode = RW_ABIN_RCV;
        }
      }
    }
  }
}

void iboxsetinpfile(short usernr,char *str)
{
  if (iface_user[usernr].active) {
    strcpy(iface_user[usernr].data->abin_filename,str);
  }
}

/* in boxglobl.c */
void iboxprdir(short channel,char *str)
{
}

void iboxsetrwmode(short usernr,short mode)
{
  if (iface_user[usernr].active) {
    iface_user[usernr].data->rwmode = mode;
  }
}

void iboxsetboxbin(short channel,boolean nodisplay)
{
}

void iboxpabortsf(short channel,boolean flag)
{
  IFACE_CMDBUF command;
  
  command.command = CMD_BOXPABORTSF;
  command.data.immediate = (int)flag;
  send_command_packet(channel,LEN_BOXPABORTSF,(char *)&command);
  exit_iface_user(channel);
}

boolean iboxpboxsf(short tnc,char *console_call,char *path)
{
  int len;
  int timeout;
  char call[256];
  char *bufptr;
  IFACE_CMDBUF command;
  int res;
  
  if (!iface_list[tnc].active) return false;
  res = sscanf(path,"%s %d",call,&timeout);
  if ((strlen(call) > 9) || (res == 0)) return false;
  if (res == 1) timeout = 120;
  
  /* fill command */
  command.command = CMD_BOXPBOXSF;
  /* fill timeout */
  command.data.boxpboxsf.timeout = timeout;
  /* fill console_call */
  len = LEN_BOXPBOXSF;
  bufptr = command.data.boxpboxsf.buffer;
  strcpy(bufptr,console_call);
  len += strlen(console_call);
  *(bufptr + len - LEN_BOXPBOXSF) = '\0';
  len++;
  /* fill call */
  strcpy(bufptr + len - LEN_BOXPBOXSF,call);
  len += strlen(call);
  *(bufptr + len - LEN_BOXPBOXSF) = '\0';
  len++;
  send_command_iface(tnc,len,(char *)&command);
  
  return true;
}

/* in boxglobl.c */
long iboxtncqrg(short tnc)
{
  switch (tnc) {
  case 0:
    return 438300;
  case 1:
    return 438400;
  default:
    return 438450;
  }
}

void iboxprdisc(short channel)
{
  IFACE_CMDBUF command;
    
  command.command = CMD_BOXPRDISC;
  send_command_packet(channel,LEN_SIMPLE,(char *)&command);
  exit_iface_user(channel);
}

/* in boxglobl.c */ 
short iboxboxssid()
{
  return(box_ssid);
}

/* in boxglobl.c */
short iboxnodessid()
{
  return(node_ssid);
}

boolean iboxgetcompmode(short channel)
{
  if (iface_user[channel].active)
    return (iface_user[channel].data->huffcod);
  else
    return(0);
}

void iboxsetcompmode(short channel,boolean mode)
{
  IFACE_CMDBUF buffer;
  
  if (iface_user[channel].active) {
    iface_user[channel].data->huffcod = mode;
    buffer.command = CMD_HUFFSTAT;
    buffer.data.huffstat = (int)mode;
    queue_iface_cmd(channel,(char *)&buffer,LEN_HUFFSTAT);
  }
}

void iboxsetboxsf(short usernr,boolean flag)
{
  if (iface_user[usernr].active) {
    iface_user[usernr].data->boxsf = flag;
  }
}

void iboxsetbox(short channel,short value)
{
}

void iboxsetwasbox(short channel,short value)
{
}

void iboxsetnode(short channel,boolean flag)
{
}

void iboxstartconnect(short channel,char *stri1,char *stri2,
                      boolean *flag1,boolean *flag2,boolean *flag3)
{
}

void iboxcancelmultibox()
{
}

boolean iboxhoststarted()
{
  return false;
}

void iboxprreset(char *str)
{
}

void iboxprkill(short channel,char *str)
{
}

void iboxprprefix(short channel,boolean flag,char *str)
{
}

void iboxsetamouse()
{
}

void iboxsetbmouse()
{
}

void iatarisetamouse()
{
}

void iatarisetbmouse()
{
}

boolean iboxgbstop()
{
  return false;
}

short iboxcallgemprg(char *stri1,char *stri2)
{
  return 0;
}

void iboxsgdial1()
{
}

void iboxegdial1()
{
}

void iboxsetgdial(boolean flag,char *stri1,short wert1,short wert2,
                  long lwert1,long lwert2,long lwert3,char *stri2)
{
  if (dpbox_debug) {
    fprintf(error_fp,"%s\n",stri2);
    fflush(error_fp);
  }
}

void iboxrgdial1(char *str)
{
}

void iboxsgdial()
{
}

void iboxegdial()
{
}

void iboxwait(long wert)
{
}
 
void iboxalert2(char *str,boolean flag)
{
  if (dpbox_debug) {
    fprintf(error_fp,"%s\n",str);
    fflush(error_fp);
  }
}

void iboxbusy(char *str)
{
  if (dpbox_debug) {
    fprintf(error_fp,"%s\n",str);
    fflush(error_fp);
  }
}

void iboxendbusy()
{
}

boolean iboxgetfilename(char *pfad,char *name,char *dummy)
{
  
  if (filestring[0] == '\0') return false;
  if (strchr(filestring,'/') == NULL) {
    get_path(pfad);
    strcpy(name,pfad);
    strcat(name,filestring);
    return(true);
  }
  else {
    if (filestring[0] != '/') return false;
    strcpy(name,filestring);
    return(true);
  }
  return false;
}

void iboxfreemostram()
{
}

long iboxspoolstatus(short console,short channel,short printer)
{
  short usernr;

  usernr = calc_usernr(console,channel);
  if (!usernr) return 0;  
  return (iface_user[usernr].data->buflen + 
          iface_user[usernr].data->queue_len);
}

void iboxspoolabort(short console,short channel,short printer)
{
  struct queue_entry *oldq_ptr;
  short usernr;
  
  usernr = calc_usernr(console,channel);
  if (!usernr) return;  
  if (iface_user[usernr].active) {
    while (iface_user[usernr].data->first_q != NULL) {
      oldq_ptr = iface_user[usernr].data->first_q;
      iface_user[usernr].data->queue_len -= (oldq_ptr->len-HEAD_LEN);
      free(oldq_ptr->buffer);
      iface_user[usernr].data->first_q = oldq_ptr->next;
      free(oldq_ptr);
    }
    iface_user[usernr].data->last_q = NULL;
    iface_user[usernr].data->buflen = 0;
  }
}

void iboxmemspool(short console,short channel,short printer,
                  boolean strip_lf,char *start,long length)
{
  short usernr;
  int i;
  
  usernr = calc_usernr(console,channel);
  if (!usernr) return;  
  if (strip_lf && iface_user[usernr].data->nl_type == 0) {
    for (i=0;i<length;i++) {
      if (start[i] == '\n') start[i] = '\r';
/*      else if (start[i] == '\r') start[i] = ' '; */
    }
  }
  write_iface(usernr,length,start);
}

void iboxspoolread()
{
  short i;
  
  for(i=1;i<=MAXUSER;i++) {
    if (iface_user[i].active) {
      if (iface_user[i].data->buflen != 0) {
        write_iface_packet(i);
      }
    }
  }
}

void iboxspool(short console,short channel,short printer,const char *str,boolean cr)
{
  short usernr;
  char *buf;
  
  usernr = calc_usernr(console,channel);
  if (!usernr) return;  
  buf = (char *)malloc(strlen(str)+2);
  if (buf == NULL) return;
  strcpy(buf,str);
  if (cr) {
    if (iface_user[usernr].data->nl_type != 0)
      strcat(buf,"\n");
    else
      strcat(buf,"\r");
  }
  write_iface(usernr,strlen(buf),buf);
  free(buf);
}

void iboxedit(char *str,void **dummy,long wert1,long wert2)
{
}

void iboxstartedit(char *str,void *dummy,long wert1,long wert2,
                   boolean flag1,boolean flag2,boolean flag3)
{
  pathstr w2;
  char	  w1[256];
  
  if (!*xeditor) return;
  if (insecure(str) || !*str) return;
  if (*str != '/') strcpy(w2, boxdir);
  else *w2 = '\0';
  snprintf(w1, 255, "%s %s%s", xeditor, w2, str);
  my_exec(w1);
}

void iboxhardwatchdog(short wert1,short wert2)
{
}

void iboxprotokoll(char *str)
{
  if (dpbox_debug) {
    fprintf(error_fp,"%s\n",str);
    fflush(error_fp);
  }
}

void bcast_timing()
{
}

boolean bcast_file(char stnc,char sport,char *qrg,
                   long fid,unsigned short ftype,
                   char *name1,char *adress,char *bbs_source,
                   char *bbs_destination,char *bbs_ax25uploader,
                   time_t bbs_upload_time,time_t bbs_expire_time,
                   char bbs_compression,char *bbs_bid,char bbs_msgtype,
                   char *bbs_title,char *bbs_fheader,
                   unsigned short bodychecksum,boolean delete_after_tx)
{
  IFACE_CMDBUF command;
  BCAST_HEADINFO bh;
  int fd;
  int result;
  unsigned short len;
  short iface;
  unsigned short calc_bodychecksum;
  
  if (!add_cr(name1,&calc_bodychecksum)) return(false);
  bh.magic = BHMAGIC;
  bh.tnc = stnc;
  bh.port = sport;
  bh.qrg[0] = '\0';
  if (strlen(qrg) < 19)
    strcpy(bh.qrg,qrg);
  bh.file_id = fid;
  bh.file_type = ftype;
  strcpy(bh.filename,name1);
  strcpy(bh.address,adress);
  strcpy(bh.bbs_source,bbs_source);
  strcpy(bh.bbs_destination,bbs_destination);
  strcpy(bh.bbs_ax25uploader,bbs_ax25uploader);
  bh.bbs_upload_time = bbs_upload_time;
  bh.bbs_expire_time = bbs_expire_time;
  bh.bbs_compression = bbs_compression;
  strcpy(bh.bbs_bid,bbs_bid);
  bh.bbs_msgtype = bbs_msgtype;
  strcpy(bh.bbs_title,bbs_title);
  strcpy(bh.bbs_fheader,bbs_fheader);
  bh.bodychecksum = calc_bodychecksum;
  bh.delete_after_tx = delete_after_tx;

  iface = -1;
  if (!find_socket(qrg,&iface)) {
    iboxprotokoll("BC:QRG not found");
    return(false);
  }
  strcpy(command.data.buffer, tempdir);
  strcat(command.data.buffer,"bcheadXXXXXX");
  mymktemp(command.data.buffer);
  fd = dpsyscreate(command.data.buffer,O_RDWR|O_CREAT,FC_FILE_RWALL);
  if (fd == -1) return(false);
  result = write(fd,(char *)&bh,sizeof(bh));
  close(fd);
  if (result != sizeof(bh)) {
    sfdelfile(command.data.buffer);
    iboxprotokoll("BC:Schreiben war nicht erfolgreich");
    return(false);
  }
  command.command = CMD_STARTBOXBC;
  len = LEN_SIMPLE + strlen(command.data.buffer) + 1;
  send_command_iface(iface,len,(char *)&command);
  return(true);     
}

void boxisbusy(boolean busy)
{
  IFACE_CMDBUF command;
  int iface;

  command.command = CMD_BOXISBUSY;
  if (busy == true)
    command.data.boxisbusy = 1;
  else
    command.data.boxisbusy = 0;
  for (iface=0;iface<MAX_SOCKET;iface++) {
    if (iface_list[iface].active) {
      send_command_iface(iface,LEN_BOXISBUSY,(char *)&command);
    }
  }
}

/* end of interface-procedures */

static int init_box(argc,argv)
int argc;
char *argv[];
{
  int i;
  char hs[80];

  filestring[0] = '\0';
  for (i=0;i<=MAXUSER;i++) {
    iface_user[i].active = 0;
  }
  strcpy(dp_date,DP_DATE);
  strcpy(dp_vnr,DP_VNR);
  strcpy(dp_vnr_sub,DP_VNR_SUB);

  if (read_init_file(argc,argv))
    return(1);

  sprintf(ownfheader,"%s OP: %s",myqthwwloc,Console_call);
  if (boxsysdir[0] != '/') { 
    strcpy(hs,boxsysdir);
    sprintf(boxsysdir,"%s%s",boxdir,hs);
  }
  strcpy(hs,newmaildir);
  sprintf(newmaildir,"%s%s",boxdir,hs);
  strcpy(hs,impmaildir);
  sprintf(impmaildir,"%s%s",boxdir,hs);
  strcpy(hs,infodir);
  sprintf(infodir,"%s%s",boxdir,hs);
  strcpy(hs,indexdir);
  sprintf(indexdir,"%s%s",boxdir,hs);
  if (boxsfdir[0] != '/') { 
    strcpy(hs,boxsfdir);
    sprintf(boxsfdir,"%s%s",boxdir,hs);
  }
  if (boxrundir[0] != '/') {
    strcpy(hs,boxrundir);
    sprintf(boxrundir,"%s%s",boxdir,hs);
  }
  if (boxlanguagedir[0] != '/') { 
    strcpy(hs,boxlanguagedir);
    sprintf(boxlanguagedir,"%s%s",boxdir,hs);
  }
  strcpy(hs,savedir);
  sprintf(savedir,"%s%s",boxdir,hs);
  strcpy(hs,serverdir);
  sprintf(serverdir,"%s%s",boxdir,hs);
  strcpy(hs,boxstatdir);
  sprintf(boxstatdir,"%s%s",boxdir,hs);
  strcpy(hs,boxprotodir);
  sprintf(boxprotodir,"%s%s",boxdir,hs);
  strcpy(hs,msgidlog);
  sprintf(msgidlog,"%s%s",boxstatdir,hs);
  strcpy(hs,mybbs_box);
  sprintf(mybbs_box,"%s%s",boxstatdir,hs);
  strcpy(hs,hpath_box);
  sprintf(hpath_box,"%s%s",boxstatdir,hs);
  strcpy(hs,config_box);
  sprintf(config_box,"%s%s",boxsysdir,hs);
  strcpy(hs,badnames_box);
  sprintf(badnames_box,"%s%s",boxsysdir,hs);
  strcpy(hs,transfer_box);
  sprintf(transfer_box,"%s%s",boxsysdir,hs);
  strcpy(hs,bcast_box);
  sprintf(bcast_box,"%s%s",boxsysdir,hs);
  strcpy(hs,beacon_box);
  sprintf(beacon_box,"%s%s",boxsysdir,hs);
  strcpy(hs,commands_box);
  sprintf(commands_box,"%s%s",boxsysdir,hs);
  strcpy(hs,ctext_box);
  sprintf(ctext_box,"%s%s",boxsysdir,hs);
  strcpy(hs,debug_box);
  sprintf(debug_box,"%s%s",boxprotodir,hs);
  strcpy(hs,f6fbb_box);
  sprintf(f6fbb_box,"%s%s",boxsysdir,hs);
  strcpy(hs,rubriken_box);
  sprintf(rubriken_box,"%s%s",boxsysdir,hs);
  strcpy(hs,tcpip_box);
  sprintf(tcpip_box,"%s%s",boxsysdir,hs);
  strcpy(hs,r_erase_box);
  sprintf(r_erase_box,"%s%s",boxprotodir,hs);
  strcpy(hs,profile_box);
  sprintf(profile_box,"%s%s",boxprotodir,hs);
  strcpy(hs,whotalks_lan);
  sprintf(whotalks_lan,"%s%s",boxsysdir,hs);
  strcpy(hs,m_filter_prg);
  sprintf(m_filter_prg,"%s%s",boxrundir,hs);
  strcpy(hs,resume_inf);
  sprintf(resume_inf,"%s%s",boxstatdir,hs);
  strcpy(hs,dp_prg);
  sprintf(dp_prg,"%s%s",prgdir,hs);
  strcpy(hs,dp_new_prg);
  sprintf(dp_new_prg,"%s%s",prgdir,hs);
  if (box_socket[0] != '/') {
    strcpy(hs,box_socket);
    sprintf(box_socket,"%s%s",boxsockdir,hs);
  }
  return(0);
}

static void send_command_packet(usernr,len,buf)
short usernr;
unsigned short len;
char *buf;
{
  char buffer[MAX_LEN];
  IFACE_HEADER header;
  int nsockfd;
  short iface;
  
  if (len > IFACE_PACLEN) return;
  if (!iface_user[usernr].active) return;
  iface = iface_user[usernr].data->iface;
  if (!iface_list[iface].active) return;
  nsockfd = iface_list[iface].newsockfd;
  header.indicator = IF_COMMAND;
  header.channel = iface_user[usernr].data->channel;
  header.usernr = usernr;
  header.len = len;
  memcpy(buffer,(char *)&header,HEAD_LEN);
  memcpy(&buffer[HEAD_LEN],buf,len);
  if (write(nsockfd,buffer,len+HEAD_LEN) < len+HEAD_LEN) {
    debug(0,iface_user[usernr].data->unr,202,"Write error");
    close_iface(iface);
    return;
  }
}

static void send_command_iface(iface,len,buf)
short iface;
unsigned short len;
char *buf;
{
  char buffer[MAX_LEN];
  IFACE_HEADER header;
  int nsockfd;
  
  if (len > IFACE_PACLEN) return;
  if (!iface_list[iface].active) return;
  nsockfd = iface_list[iface].newsockfd;
  header.indicator = IF_COMMAND;
  header.channel = NO_CHANNEL;
  header.usernr = NO_USERNR;
  header.len = len;
  memcpy(buffer,(char *)&header,HEAD_LEN);
  memcpy(&buffer[HEAD_LEN],buf,len);
  if (write(nsockfd,buffer,len+HEAD_LEN) < len+HEAD_LEN) {
    debug(0,-1,203,"Write error");
    close_iface(iface);
    return;
  }
}

static void send_command_channel_if(channel,len,buf,iface)
short channel;
unsigned short len;
char *buf;
short iface;
{
  char buffer[MAX_LEN];
  IFACE_HEADER header;
  int nsockfd;
  
#ifdef IFACE_DEBUG
  if (dpbox_debug)
    fprintf(error_fp,"channel_if: %d %d %s %d\n",channel,len,buf,iface);
#endif
  if (len > IFACE_PACLEN) return;
  if (!iface_list[iface].active) return;
  nsockfd = iface_list[iface].newsockfd;
  header.indicator = IF_COMMAND;
  header.channel = channel;
  header.usernr = NO_USERNR;
  header.len = len;
  memcpy(buffer,(char *)&header,HEAD_LEN);
  memcpy(&buffer[HEAD_LEN],buf,len);
  if (write(nsockfd,buffer,len+HEAD_LEN) < len+HEAD_LEN) {
    debug(0,-1,204,"Write error");
    close_iface(iface);
    return;
  }
}

static void send_command_all(usernr,channel,len,buf,iface)
short usernr;
short channel;
unsigned short len;
char *buf;
short iface;
{
  char buffer[MAX_LEN];
  IFACE_HEADER header;
  int nsockfd;
  
  if (len > IFACE_PACLEN) return;
  if (!iface_list[iface].active) return;
  nsockfd = iface_list[iface].newsockfd;
  header.indicator = IF_COMMAND;
  header.channel = channel;
  header.usernr = usernr;
  header.len = len;
  memcpy(buffer,(char *)&header,HEAD_LEN);
  memcpy(&buffer[HEAD_LEN],buf,len);
  if (write(nsockfd,buffer,len+HEAD_LEN) < len+HEAD_LEN) {
    debug(0,-1,204,"Write error");
    close_iface(iface);
    return;
  }
}

static void blocking_test(size,iface)
int size;
short iface;
{
  IFACE_CMDBUF command;

  if (no_blocking_flag) return;
  iface_list[iface].sendlength += size;
  if (iface_list[iface].sendlength > BLOCKING_SIZE) {
    iface_list[iface].blocked = 1;
    command.command = CMD_BLOCK;
    send_command_iface(iface,LEN_SIMPLE,(char *)&command);
  }
}

static void unblocking(iface)
short iface;
{
  struct queue_entry *oldq_ptr;
  int usernr;
  
  if (!iface_list[iface].active) return;
  iface_list[iface].blocked = 0;
  iface_list[iface].sendlength = 0;
  while ((!iface_list[iface].blocked) &&
         (iface_list[iface].unproto_queue.first_q != NULL)) {
    oldq_ptr = iface_list[iface].unproto_queue.first_q;
    if (write(iface_list[iface].newsockfd,
        oldq_ptr->buffer,oldq_ptr->len) < oldq_ptr->len) {
      debug(0,-1,205,"Write error");
      close_iface(iface);
      return;
    }
    iboxprotokoll("Unproto unqueued");
    blocking_test(oldq_ptr->len,iface);
    iface_list[iface].unproto_queue.queue_len -= (oldq_ptr->len-HEAD_LEN);
    free(oldq_ptr->buffer);
    iface_list[iface].unproto_queue.first_q = oldq_ptr->next;
    if (iface_list[iface].unproto_queue.first_q == NULL)
      iface_list[iface].unproto_queue.last_q = NULL;
    free(oldq_ptr);
  }
  usernr = 1;
  while ((usernr<=MAXUSER) && (!iface_list[iface].blocked)) {
    /* only if user active and on specified interface */
    if (iface_user[usernr].active) {
      if (iface_user[usernr].data->iface == iface) {
        /* as long as interface not blocked again */
        while ((!iface_list[iface].blocked) &&
               (iface_user[usernr].data->snd_frms < MAX_SND_FRMS) &&
               (iface_user[usernr].data->first_q != NULL)) {
          oldq_ptr = iface_user[usernr].data->first_q;
          if (write(iface_list[iface].newsockfd,
                    oldq_ptr->buffer,oldq_ptr->len) < oldq_ptr->len) {
            debug(0,-1,205,"Write error");
            close_iface(iface);
            return;
          }
          blocking_test(oldq_ptr->len,iface);
          if ((iface_user[usernr].data->channel != 0) &&
              (oldq_ptr->buffer[0] != IF_COMMAND))
            iface_user[usernr].data->snd_frms++;
          iface_user[usernr].data->queue_len -= (oldq_ptr->len-HEAD_LEN);
          free(oldq_ptr->buffer);
          iface_user[usernr].data->first_q = oldq_ptr->next;
          if (iface_user[usernr].data->first_q == NULL)
            iface_user[usernr].data->last_q = NULL;
          free(oldq_ptr);
        }
      }
    }
    usernr++;
  }
}


/* find a free entry in iface_user */
static short find_iface_user(iface,channel)
short iface;
short channel;
{
  IFACE_DATA *dataptr;
  short usernr;
  
  /* find free usernumber */
  usernr = 1;
  while ((usernr <= MAXUSER) && (iface_user[usernr].active)) {
    usernr++;
  }
  if (usernr > MAXUSER) return(0); /* nothing found, return */
  
  dataptr = (IFACE_DATA *)malloc(sizeof(IFACE_DATA));
  if (dataptr == NULL) return(0); /* no memory, return */
  
  iface_user[usernr].active = 1;
  iface_user[usernr].data = dataptr;
  iface_user[usernr].data->channel = channel;
  iface_user[usernr].data->iface = iface;
  iface_user[usernr].data->unr = 0;
  iface_user[usernr].data->buflen = 0;
  iface_user[usernr].data->rxbuflen = 0;
  iface_user[usernr].data->rwmode = RW_NORMAL;
  iface_user[usernr].data->boxsf = false;
  iface_user[usernr].data->snd_frms = 0;
  iface_user[usernr].data->huffcod = false;
  iface_user[usernr].data->queue_len = 0;
  iface_user[usernr].data->first_q = NULL;
  iface_user[usernr].data->last_q = NULL;
  iface_user[usernr].data->call[0] = '\0';
  iface_user[usernr].data->linked = -1;
  iface_user[usernr].data->from_box = 1;
  iface_user[usernr].data->um_type = UM_USER;
  iface_user[usernr].data->user_type = 0;
  iface_user[usernr].data->nl_type = 0;
  return(usernr);
}

/* delete entry iface_user */
static void exit_iface_user(usernr)
short usernr;
{
  struct queue_entry *oldq_ptr;
  
  if (iface_user[usernr].active) {
    if (iface_user[usernr].data->rwmode == RW_ABIN_RCV) {
      close(iface_user[usernr].data->fd);
      sfdelfile(iface_user[usernr].data->abin_filename);
      iface_user[usernr].data->rwmode = RW_NORMAL;
    }
    while (iface_user[usernr].data->first_q != NULL) {
      oldq_ptr = iface_user[usernr].data->first_q;
      iface_user[usernr].data->queue_len -= (oldq_ptr->len-HEAD_LEN);
      free(oldq_ptr->buffer);
      iface_user[usernr].data->first_q = oldq_ptr->next;
      free(oldq_ptr);
    }
    free(iface_user[usernr].data);
    iface_user[usernr].active = 0;
  }
}

static void cancel_connect(short usernr)
{
  IFACE_CMDBUF command;
  short iface;
    
  if (!iface_user[usernr].active) return;
  iface = iface_user[usernr].data->iface;
  command.command = CMD_ABORTCON;
  send_command_all(usernr,NO_CHANNEL,LEN_SIMPLE,(char *)&command,iface);
}

static void start_connect(short usernr,short iface,int timeout,
                          char *call,char *qrg)
{
  int len;
  char *bufptr;
  IFACE_CMDBUF command;
  char hcall[256];
  char *ptr;
    
  if (!iface_user[usernr].active) return;

  /* fill command */
  command.command = CMD_CONNECT;
  /* fill timeout */
  command.data.connect.timeout = timeout;
  /* fill qrg */
  strcpy(command.data.connect.qrg,qrg);
  /* fill sourcecall */
  len = LEN_CONNECT;
  bufptr = command.data.connect.buffer;
  nstrcpy(hcall,iface_user[usernr].data->call,9);
  ptr = strchr(hcall,' ');
  if (ptr != NULL) *ptr = '\0';
  ptr = strchr(hcall,'-');
  if (ptr != NULL) *ptr = '\0';
  strcpy(bufptr,hcall);
  len += strlen(hcall);
  *(bufptr + len - LEN_CONNECT) = '\0';
  len++;
  /* fill call */
  strcpy(bufptr + len - LEN_CONNECT,call);
  len += strlen(call);
  *(bufptr + len - LEN_CONNECT) = '\0';
  len++;
  send_command_all(usernr,NO_CHANNEL,len,(char *)&command,iface);
}

static void box_reconnect(usernr,message)
short usernr;
int message;
{
  int errlen;
  char error_text[256-LEN_SIMPLE];
  short newunr;
  short newusernr;
  IFACE_CMDBUF command;
  unsigned short comlen;
  short iface;
  short channel;
  char hs[256];
  char bssid[256];
  
  if (!iface_user[usernr].active) return;
  iface_user[usernr].data->rwmode = RW_NORMAL;
  iface = iface_user[usernr].data->iface;
  channel = iface_user[usernr].data->channel;
  strcpy(error_text,"Boxconnect not successful\r");

  switch (message) {
  case 1:
    if (iboxboxssid() > 0) {
      sprintf(bssid, "-%d", iboxboxssid());
    }
    else
      *bssid = '\0';
    sprintf(hs,"*** reconnected to %s%s\r",Console_call, bssid);
    write_iface(usernr,strlen(hs),hs);
    break;
  case 2:
    sprintf(hs,"*** failure with %s\r",iface_user[usernr].data->dest_call);
    write_iface(usernr,strlen(hs),hs);
    sprintf(hs,"*** reconnected to %s%s\r",Console_call, bssid);
    write_iface(usernr,strlen(hs),hs);
    break;
  }

  if (iface_user[usernr].data->user_type == 1)
    newunr = melde_user_an(iface_user[usernr].data->call,usernr,0,
                           iface_user[usernr].data->um_type,0);
  else
    newunr = melde_user_an(iface_user[usernr].data->call,0,usernr,
                           iface_user[usernr].data->um_type,0);

  if (newunr == 0) {
    debug(3,-1,206,"User not reconnected");
    /* fetch reason, if any */
    if ((errlen = iface_user[usernr].data->buflen) != 0) {
      if (errlen >= (256-LEN_SIMPLE)) errlen = 256 - LEN_SIMPLE - 1;
      memcpy(error_text,iface_user[usernr].data->buffer,errlen);
      error_text[errlen] = '\0';
    }
    exit_iface_user(usernr);
    newusernr = 0;
  }
  else {
    debug(3,-1,206,"User reconnected");
    iface_user[usernr].data->unr = newunr;
    iface_user[usernr].data->rwmode = RW_NORMAL;
    iface_user[usernr].data->linked = -1;
    iface_user[usernr].data->dest_call[0] = '\0';
    newusernr = usernr;
  }
  if (!newusernr) {
    command.command = CMD_ACT_RESP;
    strcpy(command.data.buffer,error_text);
    comlen = LEN_SIMPLE + strlen(command.data.buffer);
    send_command_channel_if(channel,comlen,(char *)&command,iface);
    return;
  }
}

/* analysis of received packet via interface */
static void packet_analysis(iface,indicator,channel,usernr,len,buf)
short iface;
char indicator;
short channel;
short usernr;
unsigned short len;
char *buf;
{
  unsigned short i,ix;
  char *bufptr;
  int buflen;
  struct queue_entry *oldq_ptr;
  short unr, ct;
  char pattern[256];
  char rcall[256];
  IFACE_CMDBUF *rec_command;
  IFACE_CMDBUF command;
  short newusernr;
  short newunr;
  short own_usernr;
  short other_usernr;
  short um_type;
  short user_type;
  short nl_type;
  int call_len;
  unsigned short comlen;
  boolean result;
  int errlen;
  char leflag;
  char error_text[256-LEN_SIMPLE];
  char hs[256];
  short found;
  
  switch (indicator) {
  
  case IF_COMMAND:
#ifdef IFACE_DEBUG
    if (dpbox_debug)
     fprintf(error_fp,"IF_COMMAND\n");
#endif
    rec_command = (IFACE_CMDBUF *)buf;
    switch (rec_command->command) {
    case CMD_ACTIVATE:
#ifdef IFACE_DEBUG
      if (dpbox_debug)
        fprintf(error_fp,"CMD_ACTIVATE\n");
#endif
      strcpy(error_text,"Boxconnect not successful\r");

      um_type = UM_USER;
      user_type = 0;
      nl_type = 0;
      call_len = 0;
      if (len != LEN_SIMPLE) call_len = strlen(rec_command->data.buffer);
      if (channel == 0) user_type = 1;
      if (len >= (LEN_SIMPLE + call_len + 1 + sizeof(int))) {
        um_type = *(int *)(&rec_command->data.buffer[call_len+1]);
      }
      if (len >= (LEN_SIMPLE + call_len + 1 + sizeof(int) + sizeof(int))) {
        user_type = *(int *)(&rec_command->data.buffer[call_len+1+sizeof(int)]);
      }
      if (len == (LEN_SIMPLE + call_len + 1 + 3 * sizeof(int))) {
        nl_type = *(int *)(&rec_command->data.buffer[call_len+1+2*sizeof(int)]);
      }
      
      if ((newusernr = find_iface_user(iface,channel)) != 0) {
        command.command = CMD_ACT_RESP;
        send_command_packet(newusernr,LEN_SIMPLE,(char *)&command);
        iface_user[newusernr].data->um_type = um_type;
        iface_user[newusernr].data->user_type = user_type;
        iface_user[newusernr].data->nl_type = nl_type;
        if (user_type == 1) {
          if (call_len == 0) {
            newunr = melde_user_an(Console_call,newusernr,0,
                                   um_type,0);
       	    strcpy(iface_user[newusernr].data->call,Console_call);
       	  }
       	  else {
            newunr = melde_user_an(rec_command->data.buffer,newusernr,0,
                                   um_type,0);
            strcpy(iface_user[newusernr].data->call,rec_command->data.buffer);
       	  }
        }
        else {
          newunr = melde_user_an(rec_command->data.buffer,0,newusernr,
                                 um_type,0);
          strcpy(iface_user[newusernr].data->call,rec_command->data.buffer);
        }
        if (newunr == 0) {
          debug(3,-1,206,"User not accepted");
          /* fetch reason, if any */
          if ((errlen = iface_user[newusernr].data->buflen) != 0) {
            if (errlen >= (256-LEN_SIMPLE)) errlen = 256 - LEN_SIMPLE - 1;
            memcpy(error_text,iface_user[newusernr].data->buffer,errlen);
            error_text[errlen] = '\0';
          }
          exit_iface_user(newusernr);
          newusernr = 0;
        }
        else {
          debug(3,-1,206,"User accepted");
          iface_user[newusernr].data->unr = newunr;
        }
      }
      if (!newusernr) {
        command.command = CMD_ACT_RESP;
        strcpy(command.data.buffer,error_text);
        comlen = LEN_SIMPLE + strlen(command.data.buffer);
        send_command_channel_if(channel,comlen,(char *)&command,iface);
      }
      break;
    case CMD_DEACTIVATE:
#ifdef IFACE_DEBUG
      if (dpbox_debug)
        fprintf(error_fp,"CMD_DEACTIVATE\n");
#endif
      own_usernr = usernr;
      if (own_usernr == NO_USERNR) {
        own_usernr = 1;
        found = 0;
        while (!found && (own_usernr <= MAXUSER)) {
          if (iface_user[own_usernr].active) {
            if ((iface_user[own_usernr].data->iface == iface) &&
                (iface_user[own_usernr].data->channel == channel)) {
              found = 1;
            }
          }
          if (!found) own_usernr++;
        }
        if (!found) own_usernr = 0;
      }
      if ((own_usernr != NO_USERNR) && (own_usernr) && 
          (own_usernr <= MAXUSER)) {
        if (iface_user[own_usernr].active) {
          if (iface_user[own_usernr].data->rwmode == RW_LINKED) {
            other_usernr = iface_user[own_usernr].data->linked;
            if (other_usernr != -1) {
              if (iface_user[other_usernr].data->from_box) {
                box_reconnect(other_usernr,1);
              }
              else {
                iboxpacketendbox(other_usernr);
              }
            }
          }
          else {
            melde_user_ab(iface_user[own_usernr].data->unr,0);
            debug(3,-1,206,"User canceled");
          }
          exit_iface_user(own_usernr);
        }
      }
      break;
    case CMD_CSTATUS:
      if ((usernr) && (usernr != NO_USERNR) && (usernr <= MAXUSER)) {
        if (iface_user[usernr].active) {
          iface_user[usernr].data->snd_frms = rec_command->data.snd_frames;
          while ((!iface_list[iface].blocked) &&
                 (iface_user[usernr].data->snd_frms < MAX_SND_FRMS) &&
                 (iface_user[usernr].data->first_q != NULL)) {
            oldq_ptr = iface_user[usernr].data->first_q;
            if (write(iface_list[iface_user[usernr].data->iface].newsockfd,
                      oldq_ptr->buffer,oldq_ptr->len) < oldq_ptr->len) {
               debug(0,iface_user[usernr].data->unr,206,"Write error");
               close_iface(iface_user[usernr].data->iface);
               return;
            }
            blocking_test(oldq_ptr->len,iface);
            if (oldq_ptr->buffer[0] != IF_COMMAND)
              iface_user[usernr].data->snd_frms++;
            iface_user[usernr].data->queue_len -= (oldq_ptr->len-HEAD_LEN);
            free(oldq_ptr->buffer);
            iface_user[usernr].data->first_q = oldq_ptr->next;
            if (iface_user[usernr].data->first_q == NULL)
              iface_user[usernr].data->last_q = NULL;
            free(oldq_ptr);
          }
        }
      }
      break;
    case CMD_BLOCK:
      command.command = CMD_UNBLOCK;
      send_command_iface(iface,LEN_SIMPLE,(char *)&command);
      break;
    case CMD_UNBLOCK:
      unblocking(iface);
      break;
    case CMD_SORT_NEW_MAIL:
      unr = usernr;
      bufptr = rec_command->data.buffer;
      strcpy(pattern,bufptr);
      bufptr += strlen(pattern) + 1;
      strcpy(rcall,bufptr);
      sort_new_mail(unr,pattern,rcall);
      break;
    case CMD_SF_RX_EMT:
      unr = usernr;
      bufptr = rec_command->data.buffer;
      strcpy(pattern,bufptr);
      ct  = strlen(pattern);
      if (pattern[ct - 1] == '\032')
      strdelete(pattern, ct, 1);
      del_lastblanks(pattern);
      sf_rx_emt(unr,pattern);
      break;
    case CMD_ABORT_ROUTING:
      abort_routing(rec_command->data.buffer);
      break;
    case CMD_START_SF:
      strcpy(error_text,"Boxconnect not successful\r");
      if ((newusernr = find_iface_user(iface,channel)) != 0) {
        command.command = CMD_ACT_RESP;
        send_command_packet(newusernr,LEN_SIMPLE,(char *)&command);
        iface_user[newusernr].data->um_type = UM_SF_OUT;
        iface_user[newusernr].data->user_type = 0;
        iface_user[newusernr].data->nl_type = 0;
        newunr = melde_user_an(rec_command->data.buffer,0,newusernr,UM_SF_OUT,0);
        strcpy(iface_user[newusernr].data->call,rec_command->data.buffer);
        if (newunr == 0) {
          debug(0,-1,206,"SF not accepted");
          /* fetch reason, if any */
          if ((errlen = iface_user[newusernr].data->buflen) != 0) {
            if (errlen >= (256-LEN_SIMPLE)) errlen = 256 - LEN_SIMPLE - 1;
            memcpy(error_text,iface_user[newusernr].data->buffer,errlen);
            error_text[errlen] = '\0';
          }
          exit_iface_user(newusernr);
          newusernr = 0;
        }
        else {
          debug(3,-1,206,"SF accepted");
          iface_user[newusernr].data->unr = newunr;
        }
      }
      if (!newusernr) {
        command.command = CMD_ACT_RESP;
        strcpy(command.data.buffer,error_text);
        comlen = LEN_SIMPLE + strlen(command.data.buffer);
        send_command_channel_if(channel,comlen,(char *)&command,iface);
      }
      break;
    case CMD_FINISH_PRG:
      finish_box = 1;
      break;
    case CMD_HUFFSTAT:
      if ((usernr) && (usernr != NO_USERNR) && (usernr <= MAXUSER)) {
        if (iface_user[usernr].active) {
          iface_user[usernr].data->huffcod = 
                               (boolean)(rec_command->data.huffstat);
        }
      }
      break;
    case CMD_QRGINFO:
      memcpy(iface_list[iface].qrg,rec_command->data.qrg,DAT_QRGINFO);
      break;
    case CMD_BULLID:
#ifdef IFACE_DEBUG
      if (dpbox_debug)
        fprintf(error_fp,"CMD_BULLID: %s\n",rec_command->data.bullid.bullid);
#endif
      result = !check_double(rec_command->data.bullid.bullid);
      command.command = CMD_BULLID;
      command.data.bullid.found = (int)result;
      strcpy(command.data.bullid.bullid,rec_command->data.bullid.bullid);
      comlen = LEN_BULLID;
      send_command_channel_if(channel,comlen,(char *)&command,iface);
      break;
    case CMD_BCCALLBACK:
#ifdef IFACE_DEBUG
      if (dpbox_debug)
        fprintf(error_fp,"CMD_BCCALLBACK: %x\n",rec_command->data.file_id);
#endif
      bccallback(rec_command->data.file_id);
      break;
    case CMD_NOSUCCESSCON:
      box_reconnect(usernr,2);
      break;
    case CMD_SUCCESSCON:
      newusernr = 0;
      if ((usernr) && (usernr != NO_USERNR) && (usernr <= MAXUSER)) {
        if (iface_user[usernr].active) {
          if ((iface_user[usernr].data->rwmode == RW_LINKED) &&
             (iface_user[usernr].data->linked == -1)) {
            if ((newusernr = find_iface_user(iface,channel)) != 0) {
              command.command = CMD_ACT_RESP;
              send_command_packet(newusernr,LEN_SIMPLE,(char *)&command);
              strcpy(iface_user[newusernr].data->call,
                      rec_command->data.buffer);
              iface_user[newusernr].data->rwmode = RW_LINKED;
              iface_user[newusernr].data->linked = usernr;
              iface_user[newusernr].data->from_box = 0;
              iface_user[usernr].data->linked = newusernr;
              sprintf(hs,"*** connected to %s\r",
                      iface_user[usernr].data->dest_call);
              write_iface(usernr,strlen(hs),hs);
            }
          }
        }
      }
      if (!newusernr) {
        command.command = CMD_ACT_RESP;
        command.data.buffer[0] = '\0';
        comlen = LEN_SIMPLE + strlen(command.data.buffer);
        send_command_channel_if(channel,comlen,(char *)&command,iface);
      }
      break;
    case CMD_TNTRESPONSE:
      if ((usernr) && (usernr != NO_USERNR) && (usernr <= MAXUSER)) {
        if (iface_user[usernr].active) {
          if (iface_user[usernr].data->rwmode != RW_LINKED) {
            strcpy(hs,"<TNT>:");
            strcat(hs,rec_command->data.tntresponse.buffer);
            write_iface(usernr,strlen(hs),hs);
          }
        }
      }
      break;
    case CMD_RXUNPROTOHEAD:
      iface_list[iface].unproto_rxheader.pid =
          rec_command->data.rxunprotohead.pid;
      iface_list[iface].unproto_rxheader.callcount =
          rec_command->data.rxunprotohead.callcount;
      iface_list[iface].unproto_rxheader.heardfrom =
          rec_command->data.rxunprotohead.heardfrom;
      nstrcpy(iface_list[iface].unproto_rxheader.qrg,
          rec_command->data.rxunprotohead.qrg,20);
      memcpy(iface_list[iface].unproto_rxheader.calls,
          rec_command->data.rxunprotohead.calls,DAT_PATHINFO);
      break;
    default:
      break;
    }
    break;
    
  case IF_DATA:
#ifdef IFACE_DEBUG
    if (dpbox_debug)
      fprintf(error_fp,"IF_DATA\n");
#endif
    if ((!usernr) || (usernr == NO_USERNR)) return;
    if (iface_user[usernr].active == 0) return;
    if (iface_user[usernr].data->rwmode == RW_LINKED) {
      other_usernr = iface_user[usernr].data->linked;
      if (other_usernr == -1) {
        box_reconnect(usernr,0);
        cancel_connect(usernr);
      }
      else if (iface_user[other_usernr].active) {
        write_iface(other_usernr,len,buf);
      }
      return;
    }
    if (iface_user[usernr].data->unr == 0) return;
    bufptr = iface_user[usernr].data->rxbuffer;
    buflen = iface_user[usernr].data->rxbuflen;
    bufptr += buflen;
    if (iface_user[usernr].data->nl_type == 0) leflag = '\r';
    else leflag = '\n';
    i = 0;
    while (i < len) {
      switch (iface_user[usernr].data->rwmode) {
      case RW_NORMAL:
      case RW_ABIN_WAIT:
        if ((*(buf + i)) == leflag) {
          *bufptr = '\0';
          box_input(iface_user[usernr].data->unr,false,
                    iface_user[usernr].data->rxbuffer,true);
          if (iface_user[usernr].active) {
            bufptr = iface_user[usernr].data->rxbuffer;
            buflen = 0;
            i++;
          }
          else {
            i = len;
          }
        }
        else {
          *bufptr = *(buf + i);
          i++;
          bufptr++;
          buflen++;
          if (buflen == 255) {
            *bufptr = '\0';
            box_input(iface_user[usernr].data->unr,false,
                      iface_user[usernr].data->rxbuffer,false);
            if (iface_user[usernr].active) {
              bufptr = iface_user[usernr].data->rxbuffer;
              buflen = 0;
            }
            else {
              i = len;
            }
          }
        }
        break;
      case RW_ABIN_RCV:
        abin_rcv(usernr,&i,len,buf);
        break;
      case RW_RAW:
      	ix = i+1;
      	box_rawinput(iface_user[usernr].data->unr,len,&ix,buf);
      	i = ix-1;
      	if (!iface_user[usernr].active) {
      	  i = len;
      	}
      	break;
      case RW_FBB:
        ix = i+1;
        fbbpack(iface_user[usernr].data->unr,len,&ix,buf);
        i = ix-1;
        if (!iface_user[usernr].active) {
          i = len;
        }
        break;
      case RW_FBB2:
        ix = i+1;
        fbb2pack(iface_user[usernr].data->unr,len,&ix,buf);
        i = ix-1;
        if (!iface_user[usernr].active) {
          i = len;
        }
        break;
      }
    }
    if (iface_user[usernr].active) {
      iface_user[usernr].data->rxbuflen = buflen;
    }
    break;
    
  case IF_UNPROTO:
    if (iface_list[iface].unproto_rxheader.pid >= 0) {
      raw_unproto_request(iface_list[iface].unproto_rxheader.pid,
                          iface_list[iface].unproto_rxheader.callcount,
                          iface_list[iface].unproto_rxheader.heardfrom,
                          iface_list[iface].unproto_rxheader.qrg,
                          iface_list[iface].unproto_rxheader.calls,
                          len,
                          buf);
    }
    break;
    
  }
}

/* send unproto packet via interface */
static void queue_iface_unproto(iface,cmdbuffer,cmdlen,isaddress)
long iface;
char *cmdbuffer;
int cmdlen;
int isaddress;
{
  char buffer[MAX_LEN];
  int len;
  char *ptr;
  struct queue_entry *q_ptr;
  IFACE_HEADER header;
  
  if (!iface_list[iface].active) return;
  if (cmdlen > IFACE_PACLEN) return;
  len = cmdlen;
  if (isaddress) {
    header.indicator = IF_COMMAND;
  }
  else {
    header.indicator = IF_UNPROTO;
  }
  header.channel = NO_CHANNEL;
  header.usernr = NO_USERNR;
  header.len = len;
  memcpy(buffer,(char *)&header,HEAD_LEN);
  memcpy(&buffer[HEAD_LEN],cmdbuffer,len);
  if (iface_list[iface].blocked) {
    q_ptr = (struct queue_entry *)malloc(sizeof(struct queue_entry));
    if (q_ptr == NULL) {
      debug(0,-1,207,"Malloc error");
      return;
    }
    ptr = (char *)malloc(len+HEAD_LEN);
    if (ptr == NULL) {
      debug(0,-1,207,"Malloc error");
      return;
    }
    memcpy(ptr,buffer,len+HEAD_LEN);
    q_ptr->buffer = ptr;
    q_ptr->len = len+HEAD_LEN;
    q_ptr->next = NULL;
    iface_list[iface].unproto_queue.queue_len += (q_ptr->len-HEAD_LEN);
    if (iface_list[iface].unproto_queue.first_q == NULL) {
      iface_list[iface].unproto_queue.first_q = q_ptr;
      iface_list[iface].unproto_queue.last_q = q_ptr;
    }
    else {
      iface_list[iface].unproto_queue.last_q->next = q_ptr;
    }
    iface_list[iface].unproto_queue.last_q = q_ptr;
    iboxprotokoll("Unproto queued");
  }
  else {
    if (write(iface_list[iface].newsockfd,buffer,
              len+HEAD_LEN) < len+HEAD_LEN) {
      debug(0,-1,207,"Write error");
      close_iface(iface);
      return;
    }
    blocking_test(len+HEAD_LEN,iface);
    iboxprotokoll("Unproto sent");
  }
}

/* send unproto packet via interface */
static void write_iface_unproto(iface,cmdbuffer,cmdlen)
long iface;
char *cmdbuffer;
int cmdlen;
{
  queue_iface_unproto(iface,cmdbuffer,cmdlen,0);
}

/* send packet on channel via interface */
static void write_iface_packet2(usernr,cmdflag,cmdbuffer,cmdlen)
short usernr;
short cmdflag;
char *cmdbuffer;
int cmdlen;
{
  char buffer[MAX_LEN];
  int len;
  char *ptr;
  struct queue_entry *q_ptr;
  IFACE_HEADER header;
  short iface;
  
  if (cmdlen > IFACE_PACLEN) return;
  if (!iface_user[usernr].active) return;
  iface = iface_user[usernr].data->iface;
  if (!iface_list[iface].active) return;
  if (!cmdflag) {
    len = iface_user[usernr].data->buflen;
    header.indicator = IF_DATA;
    header.channel = iface_user[usernr].data->channel;
    header.usernr = usernr;
    header.len = len;
    memcpy(buffer,(char *)&header,HEAD_LEN);
    memcpy(&buffer[HEAD_LEN],iface_user[usernr].data->buffer,len);
  }
  else {
    len = cmdlen;
    header.indicator = IF_COMMAND;
    header.channel = iface_user[usernr].data->channel;
    header.usernr = usernr;
    header.len = len;
    memcpy(buffer,(char *)&header,HEAD_LEN);
    memcpy(&buffer[HEAD_LEN],cmdbuffer,len);
  }
  if ((iface_user[usernr].data->snd_frms >= MAX_SND_FRMS) ||
      (iface_list[iface_user[usernr].data->iface].blocked)) {
    q_ptr = (struct queue_entry *)malloc(sizeof(struct queue_entry));
    if (q_ptr == NULL) {
      debug(0,iface_user[usernr].data->unr,207,"Malloc error");
      return;
    }
    ptr = (char *)malloc(len+HEAD_LEN);
    if (ptr == NULL) {
      debug(0,iface_user[usernr].data->unr,207,"Malloc error");
      return;
    }
    memcpy(ptr,buffer,len+HEAD_LEN);
    q_ptr->buffer = ptr;
    q_ptr->len = len+HEAD_LEN;
    q_ptr->next = NULL;
    iface_user[usernr].data->queue_len += (q_ptr->len-HEAD_LEN);
    if (iface_user[usernr].data->first_q == NULL) {
      iface_user[usernr].data->first_q = q_ptr;
      iface_user[usernr].data->last_q = q_ptr;
    }
    else {
      iface_user[usernr].data->last_q->next = q_ptr;
    }
    iface_user[usernr].data->last_q = q_ptr;
  }
  else {
    if (write(iface_list[iface].newsockfd,buffer,
              len+HEAD_LEN) < len+HEAD_LEN) {
      debug(0,iface_user[usernr].data->unr,207,"Write error");
      close_iface(iface);
      return;
    }
    blocking_test(len+HEAD_LEN,iface);
    if ((iface_user[usernr].data->channel != 0) &&
        (buffer[0] != IF_COMMAND))
      iface_user[usernr].data->snd_frms++;
  }
  iface_user[usernr].data->buflen = 0;
}

/* send packet on channel via interface */
void write_iface_packet(usernr)
short usernr;
{
  write_iface_packet2(usernr,0,NULL,0);
}

/* put command in queue for channel via interface */
static void queue_iface_cmd(short usernr, char *cmdbuffer, int cmdlen)
{
  if (iface_user[usernr].active) {
    if (iface_user[usernr].data->buflen != 0) {
      write_iface_packet(usernr);
    }
  }
  write_iface_packet2(usernr,1,cmdbuffer,cmdlen);
}

/* check timeout on channel at interface */
void write_iface_timeout()
{
  short i;
  long timeout;
  
  for(i=1;i<=MAXUSER;i++) {
    if (iface_user[i].active) {
      if (iface_user[i].data->buflen != 0) {
        timeout = time(NULL) - iface_user[i].data->bufupdate;
        if ((timeout >= IF_TIMEOUT) || (timeout < 0)) {
          write_iface_packet(i);
        }
      }
    }
  }
}

void write_iface_seg(usernr,len,str)
short usernr;
unsigned short len;
char *str;
{
  char *bufptr;
  int max;
  int remain;
  
  if (len > box_paclen) return;
  if (!iface_user[usernr].active) return;
  bufptr = iface_user[usernr].data->buffer;
  max = len;
  remain = 0;
  if (iface_user[usernr].data->buflen) {
    bufptr = bufptr + iface_user[usernr].data->buflen;
    if ((iface_user[usernr].data->buflen + len) > box_paclen) {
      max = box_paclen - iface_user[usernr].data->buflen;
      remain = len - max;
    }
  }
  memcpy(bufptr,str,max);
  iface_user[usernr].data->buflen += max;
  if (remain || (iface_user[usernr].data->buflen == box_paclen)) {
    write_iface_packet(usernr);
    if (remain) {
      memcpy(iface_user[usernr].data->buffer,str+max,remain);
      iface_user[usernr].data->buflen = remain;
      iface_user[usernr].data->bufupdate = time(NULL);
    }
  }
  else {
    iface_user[usernr].data->bufupdate = time(NULL);
  }
}

/* write data to interface */
void write_iface(usernr,len,str)
short usernr;
long len;
char *str;
{
  long len_cur;
  char *ptr;

  if (!iface_user[usernr].active) return;
  ptr = str;
  len_cur = len;
  while (len_cur > box_paclen) {
    write_iface_seg(usernr,box_paclen,ptr);
    len_cur -= box_paclen;
    ptr += box_paclen;
  }
  if (len_cur > 0) {
    write_iface_seg(usernr,len_cur,ptr);
    if ((iface_user[usernr].data->user_type == 1) &&
        (iface_user[usernr].data->buflen != 0))
      write_iface_packet(usernr);
  }
}

/* read data from interface */
static int read_data_iface(iface)
short iface;
{
  char buffer[MAX_LEN];
  char *bufptr;
  int len;
  int remain;
#ifdef IFACE_DEBUG
  unsigned short tmplen;
#endif

  /* iface already verified */
  len = read(iface_list[iface].newsockfd,buffer,MAX_LEN);
  if ((len == -1) || (len == 0)) {
    return(1);
  }
  bufptr = buffer;
  remain = len;
  while (remain) {
    if (iface_list[iface].status < HEAD_LEN) {
      *((char *)&iface_list[iface].header + iface_list[iface].status) = *bufptr;
      iface_list[iface].status++;
      remain--;
      bufptr++;
    }
    else {
      /* packet data */
      if (iface_list[iface].header.len > 0) {
        iface_list[iface].buffer[iface_list[iface].status - HEAD_LEN] = *bufptr;
        iface_list[iface].status++;
      }
      if ((iface_list[iface].status - HEAD_LEN) == iface_list[iface].header.len) {
        iface_list[iface].status = 0;
        /* packet complete, handle it */
#ifdef IFACE_DEBUG
        if (dpbox_debug) {
          tmplen = 0;
          fprintf(error_fp,"IF: %d, ind: %d, ch: %d, us: %d, len: %d\n",
                  iface,
                  iface_list[iface].header.indicator,
                  iface_list[iface].header.channel,
                  iface_list[iface].header.usernr,
                  iface_list[iface].header.len);
          while (tmplen < iface_list[iface].header.len) {
            fprintf(error_fp,"%x ",iface_list[iface].buffer[tmplen]);
            tmplen++;
          }
          fprintf(error_fp,"\n");
        }
#endif 
        packet_analysis(iface,
                        iface_list[iface].header.indicator,
                        iface_list[iface].header.channel,
                        iface_list[iface].header.usernr,
                        iface_list[iface].header.len,
                        iface_list[iface].buffer);
      }
      remain--;
      bufptr++;
    }
  }
  return(0);
}

static int find_qrg(qrg,socket)
char *qrg;
short socket;
{
  int found;
  int qrgnr;
  
  if (socket >= MAX_SOCKET) return(0);
  if (!iface_list[socket].active) return(0);
  found = 0;
  qrgnr = 0;
  while ((!found) && (iface_list[socket].qrg[qrgnr][0] != '\0')) {
    found = (strcmp(qrg,iface_list[socket].qrg[qrgnr]) == 0);
    if (!found) qrgnr++;
  }
  return(found);
}

int find_socket(char *qrg, short *socket)
{
  short iface;
  int found;
  
  if (qrg[0] == '\0') return(0);
  if (*socket < 0) {
    /* check for qrg on all sockets */
    iface = 0;
    found = 0;
    while ((!found) && (iface < MAX_SOCKET)) {
      found = find_qrg(qrg,iface);
      if (!found) iface++;
    }
    if (found) *socket = iface;
    return(found);
  }
  else {
    /* check for qrg on specified socket */
    return(find_qrg(qrg,*socket));
  }
}

static void clear_iface_qrg(short iface)
{
  int i;
  
  if (iface >= 0 && iface < MAX_SOCKET) {
    for (i=0;i<=MAXQRGS;i++) {
      iface_list[iface].qrg[i][0] = '\0';
    }
  }
}

static void init_iface()
{
  short iface;
  
  for (iface=0;iface<MAX_SOCKET;iface++) {
    iface_list[iface].active = 0;
    clear_iface_qrg(iface);
  }
}


static void open_iface(nsockfd)
int nsockfd;
{
  short iface;
  char helpstr[80];
  
  iface = 0;
  while ((iface < MAX_SOCKET) && (iface_list[iface].active)) {
    iface++;
  }
  if (iface == MAX_SOCKET) {
    close(nsockfd);
    debug(0,-1,200,"No interface left");
    return;
  }
  iface_list[iface].active = 1;
  iface_list[iface].newsockfd = nsockfd;
  iface_list[iface].status = 0;
  iface_list[iface].blocked = 0;
  iface_list[iface].sendlength = 0;
  clear_iface_qrg(iface);
  iface_list[iface].unproto_queue.queue_len = 0;
  iface_list[iface].unproto_queue.first_q = NULL;
  iface_list[iface].unproto_queue.last_q = NULL;
  iface_list[iface].unproto_rxheader.pid = -1;
  sprintf(helpstr,"%s %d","Connection accepted, interface",iface);
  debug(1,-1,200,helpstr);
}


static void close_iface(iface)
short iface;
{
  short usernr;
  char helpstr[80];
  int found;
  struct queue_entry *oldq_ptr;

  if (iface_list[iface].active == 0) return;  
  while (iface_list[iface].unproto_queue.first_q != NULL) {
    oldq_ptr = iface_list[iface].unproto_queue.first_q;
    iface_list[iface].unproto_queue.queue_len -= (oldq_ptr->len-HEAD_LEN);
    free(oldq_ptr->buffer);
    iface_list[iface].unproto_queue.first_q = oldq_ptr->next;
    free(oldq_ptr);
  }
  iface_list[iface].unproto_queue.last_q = NULL;
  found = 0;
  for (usernr=1;usernr<=MAXUSER;usernr++) {
    if (iface_user[usernr].active) { 
      if (iface_user[usernr].data->iface == iface) {
        if (iface_user[usernr].data->unr) {
          found++;
          melde_user_ab(iface_user[usernr].data->unr,0);
          if (iface_user[usernr].active)
            iface_user[usernr].data->unr = 0;
        }
        exit_iface_user(usernr);
      }
    }
  }
  kill_all_routing_flags();
  close(iface_list[iface].newsockfd);
  sprintf(helpstr,"%d %s",found,"active users found");
  debug(1,-1,201,helpstr);
  sprintf(helpstr,"%s %d","Closing connection, interface",iface);
  debug(1,-1,201,helpstr);
  iface_list[iface].active = 0;
  clear_iface_qrg(iface);
}


static void exit_iface()
{
  short iface;
  
  for (iface=0;iface<MAX_SOCKET;iface++) {
    if (iface_list[iface].active) {
      close_iface(iface);
    }
  }
}

void list_qrgs(short unr)
{
  short iface;
  char hs[256];
  int i;
  
  sprintf(hs,"Iface QRGs");
  wlnuser(unr,hs);
  for (iface=0;iface<MAX_SOCKET;iface++) {
    if (iface_list[iface].active) {
      sprintf(hs,"%2d    ",iface);
      for (i=0;i<MAXQRGS;i++) {
        if (iface_list[iface].qrg[i][0] != '\0') {
          strcat(hs,iface_list[iface].qrg[i]);
          strcat(hs," ");
        }
      }
      wlnuser(unr,hs);
    }
  }
}

void list_ifaceusage(short unr)
{
  int i;
  char hs[256];
  short iface;
  short channel;
  short unr2;
  short link;
  char qrg[20];
  char call[256];
  char *ptr;
  char mode[10];
  
  sprintf(hs,"Call      Nbr  Ch Mode Unr Lnk Iface QRG");
  wlnuser(unr,hs);
  for(i=1;i<=MAXUSER;i++) {
    if (iface_user[i].active) {
      iface = iface_user[i].data->iface;
      channel = iface_user[i].data->channel;
      unr2 = iface_user[i].data->unr;
      link = 0;
      strcpy(mode,"BOX");
      if (iface_user[i].data->rwmode == RW_LINKED) {
        link = iface_user[i].data->linked;
        strcpy(mode,"LINK");
      }
      strcpy(call,iface_user[i].data->call);
      ptr = strchr(call,' ');
      if (ptr != NULL) *ptr = '\0';
      qrg[0] = '\0';
      if (iface_list[iface].active) {
        strcpy(qrg,iface_list[iface].qrg[0]);
      }
      sprintf(hs,"%-9.9s %3d %3d %-4.4s %3d %3d %2d    %-20.20s",
              call,i,channel,mode,unr2,link,iface,qrg);
      wlnuser(unr,hs);
    }
  }
}

void connect_from_box(short unr,char *eingabe)
{
  char str1[256];
  char str2[256];
  char str3[256];
  int result;
  int error;
  char call[10];
  char qrg[20];
  short iface;
  short owniface = -1;
  short usernr = -1;
  int timeout;
  int i,found;
  char hs[256];
  
  i = 1;
  found = 0;
  while ((i<=MAXUSER) && (!found)) {
    if (iface_user[i].active) {
      if (iface_user[i].data->rwmode != RW_LINKED) {
        if (unr == iface_user[i].data->unr) {
          found = 1;
          owniface = iface_user[i].data->iface;
          usernr = i;
        }
      }
    }
    i++;
  }
  if (!found) {
    iboxprotokoll("Connect: own connection not found!");
    return;
  }
  error = 1;
  iface = -1;
  if (eingabe[0] != '\0') {
    result = sscanf(eingabe,"%s %s %s",str1,str2,str3);
    switch (result) {
    case 1:
      if (strlen(str1) < 10) {
        strcpy(call,str1);
        upper(call);
        iface = owniface;
        strcpy(qrg,iface_list[iface].qrg[0]);
        timeout = 120;
        error = 0;
      }
      break;
    case 2:
      if (strlen(str1) < 10) {
        strcpy(call,str1);
        upper(call);
        if (strlen(str2) < 20) {
          upper(str2);
          if (find_socket(str2,&iface)) {
            strcpy(qrg,str2);
            timeout = 120;
            error = 0;
          }
        }
        if (error) {
          if (sscanf(str2,"%d",&timeout) == 1) {
            iface = owniface;
            strcpy(qrg,iface_list[iface].qrg[0]);
            error = 0;
          }
        }
      }
      break;
    case 3:
      if ((strlen(str1) < 10) && (strlen(str2) < 20) &&
          (sscanf(str3,"%d",&timeout) == 1)) {
        upper(str2);
        if (find_socket(str2,&iface)) {
          strcpy(call,str1);
          upper(call);
          strcpy(qrg,str2);
          error = 0;
        }
      }
      break;
    default:
      break;
    }
  }
  if (!error) {
    melde_user_ab(unr,0);
    iface_user[usernr].data->unr = 0;
    strcpy(hs,"link setup...\r");
    write_iface(usernr,strlen(hs),hs);
    start_connect(usernr,iface,timeout,call,qrg);
    iface_user[usernr].data->rwmode = RW_LINKED;
    iface_user[usernr].data->linked = -1;
    strcpy(iface_user[usernr].data->dest_call,call);
  }
  else {
    strcpy(hs,"Usage: connect <call> [<qrg>] [<timeout>]");
    wlnuser(unr,hs);
  }
}

void tnt_command(short unr,char *eingabe)
{
  char str1[256];
  char str2[256];
  int result;
  int error;
  short iface;
  short usernr = -1;
  int i,found;
  char hs[256];
  int len;
  char *bufptr;
  IFACE_CMDBUF command;
  int tmp;
  short channel = 0;
  char *ptr = NULL;
  
  i = 1;
  found = 0;
  while ((i<=MAXUSER) && (!found)) {
    if (iface_user[i].active) {
      if (iface_user[i].data->rwmode != RW_LINKED) {
        if (unr == iface_user[i].data->unr) {
          found = 1;
          usernr = i;
        }
      }
    }
    i++;
  }
  if (!found) {
    iboxprotokoll("Connect: own connection not found!");
    return;
  }
  error = 1;
  iface = -1;
  if (eingabe[0] != '\0') {
    ptr = eingabe;
    get_word(ptr,str1);
    if (strlen(str1) < 20) {
      upper(str1);
      if (find_socket(str1,&iface)) {
        get_word(ptr,str2);
        result = sscanf(str2,"%d",&tmp);
        if (result == 1) {
          channel = tmp;
          if (ptr[0] != '\0') {
            error = 0;
          }
        }
      }
    }
  }
  if (!error) {
    strcpy(hs,"command issued, result follows without boxprompt\r");
    write_iface(usernr,strlen(hs),hs);
    
    /* fill command */
    command.command = CMD_TNTCOMMAND;
    /* fill command */
    len = LEN_SIMPLE;
    bufptr = command.data.buffer;
    strcpy(bufptr,ptr);
    len += strlen(ptr);
    *(bufptr + len - LEN_SIMPLE) = '\0';
    len++;
    send_command_all(usernr,channel,len,(char *)&command,iface);
    user[unr]->action = 86;
  }
  else {
    strcpy(hs,"Usage: tntcomm <qrg> <tnt-channel> <command>");
    wlnuser(unr,hs);
  }
}

void blocking_on(void)
{
  no_blocking_flag = 0;
}

void blocking_off(void)
{
  no_blocking_flag = 1;
}

static void get_dpbox_ugid(uid,gid)
int *uid;
int *gid;
{
#ifndef __macos__
  struct passwd *pstp;
  
  *uid = geteuid();
  pstp = getpwuid(*uid);
  if ((pstp->pw_uid == 0) && (dpbox_user[0] != '\0')) {
    pstp = getpwnam(dpbox_user);
    if (pstp == NULL) {
      *uid = -1;
      return;
    }
    *uid = pstp->pw_uid;
    *gid = pstp->pw_gid;
  }
  else {
    *uid = -1;
  }
#endif
}

static void drop_priv(uid,gid)
int *uid;
int *gid;
{
#ifndef __macos__
  struct passwd *pstp;
  int nuid,ngid;
  
  *uid = geteuid();
  *gid = getegid();
  pstp = getpwuid(*uid);
  if ((pstp->pw_uid == 0) && (dpbox_user[0] != '\0')) {
    pstp = getpwnam(dpbox_user);
    if (pstp == NULL) {
      *uid = -1;
      return;
    }
    nuid = pstp->pw_uid;
    ngid = pstp->pw_gid;
    setgid(ngid);
    setegid(ngid);
    setuid(nuid);
    seteuid(nuid);
  }
  else {
    *uid = -1;
  }
#endif
}

static void rest_priv(uid,gid)
int uid;
int gid;
{
#ifndef __macos__
  if (uid == -1) return;
  seteuid(uid);
  setegid(gid);
#endif
}

void linux_watchdog(short what, short value)
{
  if (value == 4711) {
    init_watch();
  }
}

void init_funcs()
{
  boxgetstatus=get_status;
  boxfreestatp=free_statp;
  boxprotokoll=iboxprotokoll;
  boxendbox=iboxendbox;
  boxpacketendbox=iboxpacketendbox;
  boxunackframes=iboxunackframes;
  boxtnccount=iboxtnccount;
  boxsetunproto=iboxsetunproto;
  boxsendpline=iboxsendpline;
  boxgbstop=iboxgbstop;
  boxgettncmonchan=iboxgettncmonchan;
  boxsetmonstat=iboxsetmonstat;
  boxsetunattmode=iboxsetunattmode;
  boxgetpaclen=iboxgetpaclen;
  boxsetrwmode=iboxsetrwmode;
  boxsetboxbin=iboxsetboxbin;
  boxpabortsf=iboxpabortsf;
  boxpboxsf=iboxpboxsf;
  boxprdisc=iboxprdisc;
  boxgetcompmode=iboxgetcompmode;
  boxsetcompmode=iboxsetcompmode;
  boxsetgdial=iboxsetgdial;
  boxhoststarted=iboxhoststarted;
  boxcheckdiskspace=iboxcheckdiskspace;
  boxalert2=iboxalert2;
  boxbusy=iboxbusy;
  boxgetfilename=iboxgetfilename;
  boxspoolstatus=iboxspoolstatus;
  boxspoolabort=iboxspoolabort;
  boxmemspool=iboxmemspool;
  boxspoolread=iboxspoolread;
  boxspool=iboxspool;
  boxsetinpfile=iboxsetinpfile;
  boxprwprg2=iboxprwprg2;
  boxsetboxsf=iboxsetboxsf;
  boxboxssid=iboxboxssid;
  boxnodessid=iboxnodessid;
}

extern void _box_init();
extern void _box_file_init();
extern void _box_sub_init();
extern void _box_sys_init();
extern void _box_tim_init();
extern void _boxlocal_init();
extern void _filesys_init();

void init_modules()
{
  _box_init();
  _box_file_init();
  _box_sub_init();
  _box_sys_init();
  _box_tim_init();
  _boxlocal_init();
  _filesys_init();
}

static void sigterm()
{
  finish_box = 1;
  signal(DP_SIGTERM, SIG_IGN);
}

static void sighup()
{
  reload_configs = 1;
}

static void sigsegv()
{
  FILE *out;
  char hs[256];
  
  if ((out = fopen(profile_box, "a+")) != NULL) {
    fprintf(out, "SIGSEGV: unr:%d call:%s cmd:%s\n",
    	current_unr, current_user, current_command);
    fflush(out);
    get_debug_func(lastproc, hs);
    fprintf(out, "SIGSEGV: lastproc:%s\n", hs);
    fflush(out);
    if (*lastproctxt != '\0') {
      fprintf(out, "SIGSEGV: error(?):%s\n", lastproctxt);
      fflush(out);
    }
    fclose(out);
  }
  exit(1);
}

static boolean show_bootinf;

void bootinf(char *s)
{
  if (!show_bootinf) return;
  printf(s);
  fflush(0);
}

int 
main(argc,argv)
int argc;
char *argv[];
{
  int clilen;
  int servlen;
#ifndef __macos__
  struct sockaddr_un serv_addr;
  struct sockaddr_un cli_addr;
  fd_set fdmask;
#endif
  struct timeval timevalue;
  int max_fd;
  int count;
  int newsockfd;
  short iface;
  pathstr hs;
  int uid, gid, suid, sgid;

  show_bootinf = true;
  bootinf("loading dpbox ...");
  finish_box = 0;
  reload_configs = 0;
  ende = 0;
  blocking_on();
  init_funcs();
  if (init_box(argc,argv))
    exit(1);

  if (check_watch()) {
    bootinf("now aborting...\n");
    exit(1);
  }

  init_iface();
  bootinf("\n");
  show_bootinf = false;

#ifndef __macos__
  if ((sockfd = socket(AF_UNIX,SOCK_STREAM,0)) < 0) {
    printf("Can't open stream socket\n");
    exit(1);
  }
  memset((char *) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path,box_socket);
  servlen =  sizeof(serv_addr);       /* VK5ABN */
/*  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family); */

  unlink(serv_addr.sun_path);
  
  if (bind(sockfd,(struct sockaddr *) &serv_addr,servlen) < 0) {
    printf("Can't bind socket\n");
    exit(1);
  }

  /* change the permissions for the socket */
  chmod(serv_addr.sun_path, 0660);
  get_dpbox_ugid(&suid, &sgid);  
  if (suid != -1) {
    chown(serv_addr.sun_path, suid, sgid);
    /* and try to change them for the crawler exe as well */
    strcpy(hs, boxdir);
    strcat(hs, "crawler/crawler");
    chown(hs, suid, sgid);
  }
  
  listen(sockfd,5);

  /* fcntl(sockfd,F_SETFL,O_NONBLOCK); */
  
 
#ifdef __NetBSD__ 
  printf("DigiPoint Box v%s%s %s (NetBSD) successfully started\n",
          dp_vnr, dp_vnr_sub, dp_date);
#define __dp_version_done
#endif
#ifdef __linux__
  printf("DigiPoint Box v%s%s %s (Linux) successfully started\n",
          dp_vnr, dp_vnr_sub, dp_date);
#define __dp_version_done
#endif

#endif
#ifdef __macos__
  printf("DigiPoint Box v%s%s %s (MacOS) successfully started\n",
          dp_vnr, dp_vnr_sub, dp_date);
#define __dp_version_done
#endif

#ifndef __dp_version_done
  printf("DigiPoint Box v%s%s %s successfully started\n",
          dp_vnr, dp_vnr_sub, dp_date);
#endif
#undef __dp_version_done

#ifndef __macos__
  switch (dpbox_debug) {
  case 1:
    if (init_proc()) {
      close(sockfd);
      exit(1);
    }
    close(0);
    close(1);
    error_fp = stderr;
    /* set stdout and stdin to /dev/null */
    dpsyscreate("/dev/null",O_RDWR,FC_FILE_RWALL);
    dup(0);
    signal(DP_SIGHUP, sighup);
    signal(DP_SIGSEGV, sigsegv);
    signal(DP_SIGTSTP, SIG_IGN);
    signal(DP_SIGTTIN, SIG_IGN);
    signal(DP_SIGTTOU, SIG_IGN);
    signal(DP_SIGTERM, sigterm);
    break;
  case 2:
    if (init_proc()) {
      close(sockfd);
      exit(1);
    }
    error_fp = fopen(dpbox_debugfile,"w+");
    if (error_fp == NULL) {
      printf("Can't create debugfile\n");
      sleep(2);
      dpbox_debug = 0;
    }
  case 0:
    if (fork() != 0)
      exit(0);
    if (init_proc()) {
      close(sockfd);
      exit(1);
    }
    close(0);
    close(1);
    close(2);
    chdir("/");
    setsid();
    /* set stdout,stdin and stderr to /dev/null */
    dpsyscreate("/dev/null",O_RDWR,FC_FILE_RWALL);
    dup(0);
    dup(0);
    signal(DP_SIGPIPE, SIG_IGN);
    signal(DP_SIGHUP, sighup);
    signal(DP_SIGSEGV, sigsegv);
    signal(DP_SIGTSTP, SIG_IGN);
    signal(DP_SIGTTIN, SIG_IGN);
    signal(DP_SIGTTOU, SIG_IGN);
    signal(DP_SIGTERM, sigterm);
    break;
  }

  drop_priv(&uid,&gid);
  init_watch();
#endif
  init_modules();
  init_boxvars();
  utc_clock();

  sprintf(hs, "%s%c%c%c", newmaildir, allquant, extsep, allquant);
  sort_new_mail(-1, hs, Console_call);
  sprintf(hs, "%s%c%c%c", impmaildir, allquant, extsep, allquant);
  sort_new_mail(-1, hs, "-moni-");
                   
  for (;;) {

#ifndef __macos__
    FD_ZERO(&fdmask);
    max_fd = 0;
    FD_SET(sockfd,&fdmask);
    if (sockfd > max_fd - 1) {
      max_fd = sockfd + 1;
    }

    for (iface=0;iface<MAX_SOCKET;iface++) {
      if (iface_list[iface].active) {
        FD_SET(iface_list[iface].newsockfd,&fdmask);
        if (iface_list[iface].newsockfd > max_fd - 1) {
          max_fd = iface_list[iface].newsockfd + 1;
        }
      }
    }
    
    shell_fdset(&max_fd,&fdmask);

    timevalue.tv_usec = 1000000 / 20;	/* call dpbox 20 times a second	*/
    timevalue.tv_sec = 0;
    count = select(max_fd,&fdmask,(fd_set *) 0,
                                  (fd_set *) 0,
                                  &timevalue);

    if (reload_configs) {
      load_all_parms(-1);
      reload_configs = 0;
    }
    
    if (count == -1) {
      continue;
    }
    
    if (FD_ISSET(sockfd,&fdmask)) {
      clilen = sizeof(cli_addr);
      newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
      if (newsockfd < 0) {
        debug(0,-1,208,"Accept error");
      }
      else {
        open_iface(newsockfd);
      }
    }
    
    for (iface=0;iface<MAX_SOCKET;iface++) {
      if (iface_list[iface].active) {
        if (FD_ISSET(iface_list[iface].newsockfd,&fdmask)) {
          if (read_data_iface(iface)) {
            close_iface(iface);
          }
        }
      }
    }
    
    shell_receive(&fdmask);
    write_iface_timeout();

#endif

    box_timing(statclock());
        
    if ((finish_box) || (ende)) {
      exit_boxvars();
      exit_iface();
      close(sockfd);
      exit_watch();
      rest_priv(uid,gid); /* this won't work anymore, as drop_priv() now also sets the real UID */
      exit_proc();
      if (dpbox_debug == 2)
        fclose(error_fp);
#if defined(__linux__) || defined(__NetBSD__)
      delete_dirlist();
      unlink(serv_addr.sun_path);
#endif
      exit(0);
    }
  }
}

