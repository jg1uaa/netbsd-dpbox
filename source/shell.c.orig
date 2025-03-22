/* ---- shell functs, stolen from TNT / Mark Wahl, DL4YBG ----- */

#if defined(__linux__) || defined(__NetBSD__)
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include "pastrix.h"
#include "filesys.h"
#include "boxglobl.h"
#include "boxlocal.h"
#include "box_logs.h"
#include "ifacedef.h"
#include "dpbox.h"
#include "init.h"
#include "status.h"
#include "tools.h"
#include "box_inou.h"
#include "box_tim.h"
#include "box_file.h"
#include "box_sub.h"
#endif

#ifdef __macos__
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stat.h>
#include <string.h>
#include <time.h>
#include <types.h>
#include <signal.h>
#include <ctype.h>
#include "pastrix.h"
#include "filesys.h"
#include "boxglobl.h"
#include "boxlocal.h"
#include "box_logs.h"
#include "ifacedef.h"
#include "dpbox.h"
#include "init.h"
#include "status.h"
#include "tools.h"
#include "box_inou.h"
#include "box_tim.h"
#include "box_file.h"
#include "box_sub.h"
#endif

#define NUMPTY		256
#define MASTERPREFIX    "/dev/pty"
#define SLAVEPREFIX     "/dev/tty"
#define pty_timeout     1
#define maxrunargs	50

#define pty_name(name, prefix, num) \
  sprintf(name, "%s%c%x", prefix, 'p' + (num >> 4), num & 0xf)


static void restore_pty(const char *id)
{
#ifndef __macos__
  char filename[80];

  sprintf(filename, "%s%s", MASTERPREFIX, id);
  chown(filename, 0, 0);
  chmod(filename, 0666);
  sprintf(filename, "%s%s", SLAVEPREFIX, id);
  chown(filename, 0, 0);
  chmod(filename, 0666);
#endif
}

static short find_pty(short *numptr, char *slave)
{
#ifndef __macos__
  char master[80];
  int fd;
  short num;

  for (num = 0; num < NUMPTY; num++) {
    pty_name(master, MASTERPREFIX, num);
    fd = dpsyscreate(master, O_RDWR, FC_FILE);
    if (fd >= minhandle) {
      *numptr = num;
      pty_name(slave, SLAVEPREFIX, num);
      return (short) fd;
    }
  }
#endif
  return (-1);
}

static void dat_input_pty(short unr, char *buffer, int len)
{
  if (user[unr]->ptylfcrconv)
    show_puffer(unr,buffer,len);
  else
    trans_show_puffer(unr,buffer,len);
}

static void pty_send_data(short unr ,char *buffer, int len)
{
  int tmp_len;
  
  if (len > box_paclen) return; /* sanity check */
  if ((user[unr]->ptybuflen == 0) && (len == box_paclen)) {
    dat_input_pty(unr,buffer,len);
  }
  else if ((tmp_len = user[unr]->ptybuflen + len) > box_paclen) {
    if (user[unr]->ptybuflen) {
      dat_input_pty(unr,user[unr]->ptybuffer,user[unr]->ptybuflen);
    }
    memcpy(user[unr]->ptybuffer,buffer,len);
    user[unr]->ptybuflen = len;
    user[unr]->ptytouch = clock_.ixtime;
  }
  else {
    memcpy((user[unr]->ptybuffer + user[unr]->ptybuflen),
           buffer,len);
    user[unr]->ptybuflen = tmp_len;       
    user[unr]->ptytouch = clock_.ixtime;
  }
}

static void conv_crtolf(char *str, int len)
{
  int i;

  for (i = 0; i < len; i++) {
    if (str[i] == 13) str[i] = 10;
  }
}

static void conv_lftocr(char *str, int len)
{
  int i;

  for (i = 0; i < len; i++) {
    if (str[i] == 10) str[i] = 13;
  }
}


static void pty_check_timeout(short unr)
{
  if (!user[unr]->ptybuflen) return;
  if ((clock_.ixtime - user[unr]->ptytouch) >= pty_timeout) {
    dat_input_pty(unr,user[unr]->ptybuffer,user[unr]->ptybuflen);
    user[unr]->ptybuflen = 0;
  }
}

static int read_data_pty(unr, buffer)
short unr;
char buffer[];
{

#ifndef __macos__

  int len;
  int fc;

  fcntl(user[unr]->pty,F_SETFL,O_NONBLOCK);
  len = read(user[unr]->pty,buffer,box_paclen);
  fc  = fcntl(user[unr]->pty,F_GETFL,NULL);
  fcntl(user[unr]->pty,F_SETFL,fc & ~O_NONBLOCK);

  if (len > 0) {
    if (user[unr]->ptylfcrconv) {
      conv_lftocr(buffer,len);
    }
    pty_send_data(unr,buffer,len);
    return(1);
  }
  else {
    return(0);
  }
#else
    return(0);
#endif
}

void shell_receive(fd_set *fdmask)
{
#ifndef __macos__
  short unr;
  char buffer[256];

  for (unr=0;unr<=MAXUSER;unr++) {
    if ((user[unr] != NULL) && (user[unr]->pty >= minhandle)) {
      pty_check_timeout(unr);
      if (FD_ISSET(user[unr]->pty,fdmask)) {
        read_data_pty(unr,buffer);
      }
    }
  }
#endif
}

boolean close_shell(short unr)
{
  char buffer[256];

  debug0(1, unr, 219);
  
  if (user[unr] == NULL) return false;

#ifndef __macos__  
  if (user[unr]->pty >= minhandle) {
  
    /* read all remaining data from pty */
    fcntl(user[unr]->pty,F_SETFL,O_NONBLOCK);
    while (read_data_pty(unr,buffer) == 1);
    if (user[unr]->ptybuflen) {
      if (user[unr]->ptylfcrconv)
      	show_puffer(unr,user[unr]->ptybuffer,user[unr]->ptybuflen);
      else
      	trans_show_puffer(unr,user[unr]->ptybuffer,user[unr]->ptybuflen);
    }
    if (user[unr]->pty >= minhandle) {
#ifdef __NetBSD__
      ioctl(user[unr]->pty, TCIOFLUSH, 2);
#else
      ioctl(user[unr]->pty, TCFLSH, 2);
#endif
      close(user[unr]->pty);
      user[unr]->pty = nohandle;
    
      restore_pty(user[unr]->ptyid);

    }
  }
#endif
  
  if (user[unr]->wait_pid > 0) {
    add_zombie(user[unr]->wait_pid, "", 0);
    kill(-user[unr]->wait_pid, DP_SIGHUP);
  }
  user[unr]->wait_pid = -1;
  return true;
}

void set_dpbox_environment(short unr)
{
    userstruct *WITH;
    char w[256];
    
    WITH = user[unr];
    if (WITH == NULL) return;
    setenv("TERM", "dumb", true);
    setenv("LOGNAME", WITH->call, true);
    setenv("CALLSIGN", WITH->call, true);
    setenv("DPBOXUSERCALL", WITH->call, true);
    setenv("DPBOXUSERMYBBS", WITH->mybbs, true);
    setenv("DPBOXUSERNAME", WITH->name, true);
    setenv("DPBOXUSERLANGUAGE", WITH->language, true);
    setenv("DPBOXBOARD", WITH->brett, true);
    ix2string(WITH->lastdate, w);
    setenv("DPBOXLASTDATE", w, true);
    sprintf(w, "%ld", WITH->lastdate);
    setenv("DPBOXIXLASTDATE", w, true);
    if (WITH->se_ok)
      strcpy(w, "1");
    else
      strcpy(w, "0");
    setenv("DPBOXSENDERASEOK", w, true);
    if (WITH->hidden)
      strcpy(w, "1");
    else
      strcpy(w, "0");
    setenv("DPBOXHIDDEN", w, true);
    if (WITH->is_authentic)
      strcpy(w, "1");
    else
      strcpy(w, "0");
    setenv("DPBOXPWOK", w, true);
    if (WITH->rsysop)
      strcpy(w, "1");
    else
      strcpy(w, "0");
    setenv("DPBOXBOARDSYSOP", w, true);
    if (WITH->supervisor)
      strcpy(w, "1");
    else
      strcpy(w, "0");
    setenv("DPBOXSYSOP", w, true);
    sprintf(w, "%d", WITH->level);
    setenv("DPBOXUSERLEVEL", w, true);
    setenv("DPBOXBOXCALL", Console_call, true);
    if (WITH->smode)
      strcpy(w, "1");
    else
      strcpy(w, "0");
    setenv("DPBOXSERVERMODE", w, true);
    strcpy(w, boxdir);
    strcat(w, fservdir);
    setenv("DPBOXSERVERROOT", w, true);
    if (*WITH->spath == '\0') *w = '\0';
    else strcpy(w, &WITH->spath[1]);
    setenv("DPBOXSERVERPATH", w, true);
    setenv("DPBOXCONNECTPATH", WITH->conpath, true);
    if (WITH->unproto_ok)
      strcpy(w, "1");
    else
      strcpy(w, "0");
    setenv("DPBOXUNPROTOOK", w, true);
    if (WITH->wantboards) {
      setenv("DPBOXNOCHECK", "", true);
      setenv("DPBOXWANTCHECK", WITH->checkboards, true);
    } else {
      setenv("DPBOXNOCHECK", WITH->checkboards, true);
      setenv("DPBOXWANTCHECK", "", true);
    }
    setenv("DPBOXLASTCMD", WITH->lastcmd, true);
    sprintf(w, "%d", debug_level);
    setenv("DPBOXDEBUGLEVEL", w, true);
    sprintf(w, "%d", usertimeout);
    setenv("DPBOXUSERTIMEOUT", w, true);
    strcpy(w, dp_vnr);
    strcat(w, dp_vnr_sub);
    setenv("DPBOXVERSION", w, true);
    sprintf(w, "%d", unr);
    setenv("DPBOXUSERNUMBER", w, true);
    sprintf(w, "%d", actual_connects());
    setenv("DPBOXUSERCOUNT", w, true);
    setenv("DPBOXBOXDIR", boxdir, true);
}

int my_exec1(char *s, int hang)
{
#ifdef __macos__
    system(s);
    return 0;
#else
    int wpid;
    int handle;
    int ct;
    int li;
    char *token;
    char *args[maxrunargs];
    char command[256];
    char fname[256];

    debug(1, 0, 221, s);

    if ((wpid = fork())) {
    	if (hang) waitpid(wpid, &li, 0);
	return wpid;
    }
    
    for (handle = 0; handle <= 255; handle++) close(handle);
    setsid();
    dup(0);
    dup(0);

    strcpy(command, s);
    token = strtok(command, " \t");
    if (token != NULL) {
    	strcpy(fname, token);
	args[0] = fname;
    	ct = 1;
	while ((args[ct] = strtok(NULL, " \t")) != NULL && ct++ < maxrunargs);
    	execvp(fname, args);
        sprintf(command, "exec: %s not found", fname);
        append_profile(-1, command);
    	exit(1);
    }
    return 0;
#endif
}


/* open a shell */
boolean cmd_shell(short unr, boolean transparent)
{

#ifndef NO_SHELL

  char slave[80];
  int i;
  struct termios termios;
  
  debug0(1, unr, 220);

  if (user[unr] == NULL) return false;

  if ((user[unr]->pty = find_pty(&user[unr]->ptynum, slave)) < 0) {
    wlnuser(unr,"Can't open pseudo terminal");
    return false;
  }
  
  user[unr]->ptybuflen = 0;
  user[unr]->ptylfcrconv = !transparent;
  strcpy(user[unr]->ptyid, slave + strlen(slave) - 2);
  if (!(user[unr]->wait_pid = fork())) {
    for (i = minhandle; i <= maxhandle; i++) close(i);
    setsid();
    dpsyscreate(slave, O_RDWR, FC_FILE_RWALL);
    dup(0);
    dup(0);
    chmod(slave, 0622);
    if (user[unr]->ptylfcrconv) {
      memset((char *) &termios, 0, sizeof(termios));
      termios.c_iflag = ICRNL | IXOFF;
#ifdef __NetBSD__
      termios.c_oflag = OPOST | OXTABS | ONLRET;
#else
      termios.c_oflag = OPOST | TAB3 | ONLRET;
#endif
      termios.c_oflag &= ~(OCRNL | ONLCR | ONOCR);
      termios.c_cflag = CS8 | CREAD | CLOCAL;
      termios.c_lflag = ISIG | ICANON;
      termios.c_cc[VINTR]  = 127;
      termios.c_cc[VQUIT]  =  28;
      termios.c_cc[VERASE] =   8;
      termios.c_cc[VKILL]  =  24;
      termios.c_cc[VEOF]   =   3;
      cfsetispeed(&termios, B1200);
      cfsetospeed(&termios, B1200);
    }
    else {
      tcgetattr(0,&termios);
    }
    tcsetattr(0, TCSANOW, &termios);
    set_dpbox_environment(unr);
    printf("\n"); /* leading newline nun ueber tty */
    execl("/bin/sh","/bin/sh","-login",(char *) 0);
    sprintf(slave, "shell (/bin/sh) not found (!?)\n");
    printf(slave);
    append_profile(-1, slave);
    exit(1);
  }
  return true;
#else
  return false;
#endif
}


/* run a program on the current channel */
boolean cmd_run(short unr, boolean transparent, char *command, char *ofi, char *add_environment)
{
#ifndef __macos__
  char slave[80];
  char execfile[81];
  char fname[81];
  char w[256], hs[256];
  int i, ct;
  struct termios termios;
  char *args[maxrunargs];

  debug(1, unr, 222, command);

  if (user[unr] == NULL) return false;

  if ((user[unr]->pty = find_pty(&user[unr]->ptynum, slave)) < 0) {
    wlnuser(unr,"Can't open pseudo terminal");
    return false;
  }
  user[unr]->ptybuflen = 0;

  user[unr]->ptylfcrconv = !transparent;
  
  strcpy(user[unr]->ptyid, slave + strlen(slave) - 2);
  if (!(user[unr]->wait_pid = fork())) {
    for (i = minhandle; i <= maxhandle; i++) close(i);
    setsid();
    dpsyscreate(slave, O_RDWR, FC_FILE_RWALL);
    dup(0);
    dup(0);
    chmod(slave, 0622);
    if (user[unr]->ptylfcrconv) {
      memset((char *) &termios, 0, sizeof(termios));
      termios.c_iflag = ICRNL | IXOFF;
#ifdef __NetBSD__
      termios.c_oflag = OPOST | OXTABS | ONLRET;
#else
      termios.c_oflag = OPOST | TAB3 | ONLRET;
#endif
      termios.c_oflag &= ~(OCRNL | ONLCR | ONOCR);
      termios.c_cflag = CS8 | CREAD | CLOCAL;
      termios.c_lflag = ISIG | ICANON;
      termios.c_cc[VINTR]  = 127;
      termios.c_cc[VQUIT]  =  28;
      termios.c_cc[VERASE] =   8;
      termios.c_cc[VKILL]  =  24;
      termios.c_cc[VEOF]   =   3;
      cfsetispeed(&termios, B1200);
      cfsetospeed(&termios, B1200);
    }
    else {
      tcgetattr(0,&termios);
    }
    tcsetattr(0, TCSANOW, &termios);

    set_dpbox_environment(unr);

    setenv("DPBOXTEXTOUT", ofi, true);
    new_ext(ofi, "trn");
    setenv("DPBOXTRANSOUT", ofi, true);
    new_ext(ofi, "bin");
    setenv("DPBOXBINOUT", ofi, true);
    new_ext(ofi, "yap");
    setenv("DPBOXYAPPOUT", ofi, true);
    
    while (*add_environment != '\0') {
      get_word(add_environment, w);
      get_word(add_environment, hs);
      setenv(w, hs, true);
    }
    
    get_word(command, execfile);
    setenv("_", execfile, true);

    strcpy(fname, execfile);
    
    for (ct = 0; ct < maxrunargs; ct++)
      args[ct] = NULL;
    
    args[0] = fname;
    args[1] = strtok(command, " \t");
    ct = 2;
    do {
      args[ct] = strtok(NULL, " \t");
    } while (args[ct++] != NULL && ct < maxrunargs);
    
    printf("\n"); /* leading newline nun ueber tty */
    execv(execfile, args);
    printf("%s: command not found\n", fname);
    exit(1);
  }
  return true;
#else
  return false;
#endif
}

#define maxbuf 300
boolean write_pty(short unr, int len, char *str)
{
  char buffer[maxbuf];

  debug0(4, unr, 223);

  if (user[unr] == NULL) return false;
  if (user[unr]->pty < minhandle) return false;
  if (len < 1) return true;
  if (len > maxbuf) len = maxbuf;

  memcpy(buffer,str,len);
  if (user[unr]->ptylfcrconv) {
    conv_crtolf(buffer,len);
  }
  if (write(user[unr]->pty,buffer,len) < len) {
    close_shell(unr);
    return false;
  }
  return true;
}
#undef maxbuf

void shell_fdset(int *max_fd, fd_set *fdmask)
{
#ifndef __macos__
  int unr;
  int fd;
  
  for (unr=1;unr<=MAXUSER;unr++) {
    if ((user[unr] != NULL) && (user[unr]->pty >= minhandle)) {
      fd = user[unr]->pty;
      FD_SET(fd,fdmask);
      if (fd > ((*max_fd) - 1)) {
        *max_fd = fd + 1;
      }
    }
  }
#endif
}
