/* dpbox: Mailbox
   Procedures for initialization (init.c)
   created: Mark Wahl DL4YBG 94/07/29
   updated: Mark Wahl DL4YBG 96/01/13
   updated: Joachim Schurig DL8HBS 99/09/26
*/

#if defined(__linux__) || defined(__NetBSD__)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include "boxglobl.h"
#include "boxlocal.h"
#include "init.h"
#include "main.h"
#endif

#ifdef __macos__
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stat.h>
#include <string.h>
#include <time.h>
#include <types.h>
#include <unistd.h>
#include "boxglobl.h"
#include "boxlocal.h"
#include "init.h"
#include "main.h"
#endif

extern char box_socket[];
extern char dpbox_user[];
char prgdir[256];
int dpbox_debug;
char dpbox_debugfile[256];
char dpbox_initfile[256];
char proc_file[256];
char watch_file[256];
int box_paclen;
extern short box_ssid;
extern short node_ssid;

/* local variables */
static int analyse_value(stri1,stri2)
char stri1[];
char stri2[];
{
  int tmp;
  
  if (strcmp(stri1,"box_paclen") == 0) {
    if (sscanf(stri2,"%d",&tmp) != 1) return(1);
    if ((tmp < 20) || (tmp > 255)) return(1);
    box_paclen = tmp;
    return(0);
  }
  if (strcmp(stri1,"box_ssid") == 0) {
    if (sscanf(stri2,"%d",&tmp) != 1) return(1);
    if ((tmp < 0) || (tmp > 15)) return(1);
    box_ssid = (short)tmp;
    return(0);
  }
  if (strcmp(stri1,"node_ssid") == 0) {
    if (sscanf(stri2,"%d",&tmp) != 1) return(1);
    if ((tmp < 0) || (tmp > 15)) return(1);
    node_ssid = (short)tmp;
    return(0);
  }
  else if (strcmp(stri1,"Console_call") == 0) {
    if (strlen(stri2) > 6) return(1);
    upper(stri2);
    strcpy(Console_call,stri2);
    return(0);
  }
  else if (strcmp(stri1,"myqthwwloc") == 0) {
    if (strlen(stri2) > 6) return(1);
    upper(stri2);
    strcpy(myqthwwloc,stri2);
    return(0);
  }
  else if (strcmp(stri1,"boxdir") == 0) {
    strcpy(boxdir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"tempdir") == 0) {
    strcpy(tempdir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"proc_file") == 0) {
    strcpy(proc_file,stri2);
    return(0);
  }
  else if (strcmp(stri1,"watch_file") == 0) {
    strcpy(watch_file,stri2);
    return(0);
  }
  else if (strcmp(stri1,"dp_prg") == 0) {
    strcpy(dp_prg,stri2);
    return(0);
  }
  else if (strcmp(stri1,"dp_new_prg") == 0) {
    strcpy(dp_new_prg,stri2);
    return(0);
  }
  else if (strcmp(stri1,"box_socket") == 0) {
    strcpy(box_socket,stri2);
    return(0);
  }
  else if (strcmp(stri1,"dpbox_user") == 0) {
    strcpy(dpbox_user,stri2);
    return(0);
  }
  else if (strcmp(stri1,"boxprocdir") == 0) {
    strcpy(boxprocdir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"boxsockdir") == 0) {
    strcpy(boxsockdir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"boxsysdir") == 0) {
    strcpy(boxsysdir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"boxlanguagedir") == 0) {
    strcpy(boxlanguagedir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"boxsfdir") == 0) {
    strcpy(boxsfdir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"boxrundir") == 0) {
    strcpy(boxrundir,stri2);
    return(0);
  }

/*

  else if (strcmp(stri1,"newmaildir") == 0) {
    strcpy(newmaildir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"impmaildir") == 0) {
    strcpy(impmaildir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"infodir") == 0) {
    strcpy(infodir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"indexdir") == 0) {
    strcpy(indexdir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"boxstatdir") == 0) {
    strcpy(boxstatdir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"boxprotodir") == 0) {
    strcpy(boxprotodir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"savedir") == 0) {
    strcpy(savedir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"prgdir") == 0) {
    strcpy(prgdir,stri2);
    return(0);
  }
  else if (strcmp(stri1,"serverdir") == 0) {
    strcpy(prgdir,stri2);
    return(0);
  } 
  else if (strcmp(stri1,"msgidlog") == 0) {
    strcpy(msgidlog,stri2);
    return(0);
  }
  else if (strcmp(stri1,"mybbs_box") == 0) {
    strcpy(mybbs_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"hpath_box") == 0) {
    strcpy(hpath_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"config_box") == 0) {
    strcpy(config_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"badnames_box") == 0) {
    strcpy(badnames_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"transfer_box") == 0) {
    strcpy(transfer_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"bcast_box") == 0) {
    strcpy(bcast_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"beacon_box") == 0) {
    strcpy(beacon_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"commands_box") == 0) {
    strcpy(commands_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"ctext_box") == 0) {
    strcpy(ctext_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"debug_box") == 0) {
    strcpy(debug_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"f6fbb_box") == 0) {
    strcpy(f6fbb_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"rubriken_box") == 0) {
    strcpy(rubriken_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"tcpip_box") == 0) {
    strcpy(tcpip_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"r_erase_box") == 0) {
    strcpy(r_erase_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"profile_box") == 0) {
    strcpy(profile_box,stri2);
    return(0);
  }
  else if (strcmp(stri1,"whotalks_lan") == 0) {
    strcpy(whotalks_lan,stri2);
    return(0);
  }
  else if (strcmp(stri1,"m_filter_prg") == 0) {
    strcpy(m_filter_prg,stri2);
    return(0);
  }
  else if (strcmp(stri1,"resume_inf") == 0) {
    strcpy(resume_inf,stri2);
    return(0);
  }
  
*/ 

  else {
    return(1);
  }
}

int init_proc(void)
{
#ifndef __macos__
  FILE *fp;
  char file[256];
  pid_t pid;

  strcpy(file,boxprocdir);
  strcat(file,proc_file);
  fp = fopen(file,"w+");
  if (fp == NULL) {
    printf("ERROR: Can't create process file\n\n");
    return(1);
  }
  pid = getpid();
  fprintf(fp,"%d\n",pid);
  fclose(fp);
#endif
  return(0);
}

void init_watch(void)
{
  FILE *fp;
  char file[256];

  strcpy(file,boxstatdir);
  strcat(file,watch_file);
  fp = fopen(file,"w+");
  if (fp != NULL)
    fclose(fp);
}

void exit_proc(void)
{
  char file[256];

  strcpy(file,boxprocdir);
  strcat(file,proc_file);
  unlink(file);
}

void exit_watch(void)
{
  char file[256];

  strcpy(file,boxstatdir);
  strcat(file,watch_file);
  unlink(file);
}

boolean check_watch(void)
{
  char file[256], hs[256];
  struct stat buf;
  long t1, tct;

  strcpy(file,boxstatdir);
  strcat(file,watch_file);
  if (stat(file, &buf) != 0)
    return false; /* no watchdog -> exit */

  t1 = time(NULL);
  if ((tct = (t1 - buf.st_mtime)) > 45)
    return false; /* watchdog older than 45 seconds -> exit */

  if (buf.st_mtime - t1 > 300)
    return false; /* watchdog more than 300 seconds in the future -> exit */

  sprintf(hs, "\nfound fresh lock file (%ld seconds old) of another dpbox\nnow checking 30 seconds for activity", tct);
  bootinf(hs);
  
  tct = 30;
  do {
    tct--;
    sleep(1);
    bootinf(".");
    if (stat(file, &buf) != 0) {
      bootinf("\nlock file was deleted - now resuming\n");
      return false;
    }
    if (t1 < buf.st_mtime) {
      bootinf("\nlock file was updated - still active!\n");
      return true;
    } 
  } while (tct >= 0);

  bootinf("\nlock file is old - now resuming\n");
  return false;
}

int read_init_file(int argc,char *argv[])
{
  FILE *init_file_fp;
  int file_end;
  int file_corrupt;
  char line[258];
  char stri1[258];
  char stri2[258];
  char tmp_str[256];
  int rslt;
  int wrong_usage;
  char *str_ptr;
  int scanned;
  int index = 0;

  char *dpbox_initdir[] = {
        "/etc/dpbox",
        "/usr/etc/dpbox",
        "/usr/local/etc/dpbox",
        "/usr/local/share/dpbox/conf",
        "/usr/share/dpbox/conf",
        "/usr/pkg/share/dpbox/conf",
        NULL
  };

  box_paclen = DEF_BOX_PACLEN;
  box_ssid = DEF_BOX_SSID;
  node_ssid = DEF_NODE_SSID;
  strcpy(Console_call,DEF_CONSOLE_CALL);
  strcpy(myqthwwloc,DEF_MYQTHWWLOC);
  strcpy(ownhiername,DEF_OWNHIERNAME);
  strcpy(boxdir,DEF_BOXDIR);
  strcpy(tempdir,DEF_TEMPDIR);
  strcpy(boxsysdir,DEF_BOXSYSDIR);
  strcpy(newmaildir,DEF_NEWMAILDIR);
  strcpy(impmaildir,DEF_IMPMAILDIR);
  strcpy(infodir,DEF_INFODIR);
  strcpy(indexdir,DEF_INDEXDIR);
  strcpy(boxstatdir,DEF_BOXSTATDIR);
  strcpy(boxprocdir,DEF_BOXPROCDIR);
  strcpy(boxsockdir,DEF_BOXSOCKDIR);
  strcpy(boxprotodir,DEF_BOXPROTODIR);
  strcpy(boxsfdir,DEF_BOXSFDIR);
  strcpy(boxrundir,DEF_BOXRUNDIR);
  strcpy(boxlanguagedir,DEF_BOXLANGUAGEDIR);
  strcpy(savedir,DEF_SAVEDIR);
  strcpy(prgdir,DEF_PRGDIR);
  strcpy(serverdir,DEF_SERVERDIR);
  strcpy(proc_file,DEF_PROC_FILE);
  strcpy(watch_file,DEF_WATCH_FILE);
  strcpy(msgidlog,DEF_MSGIDLOG);
  strcpy(mybbs_box,DEF_MYBBS_BOX);
  strcpy(hpath_box,DEF_HPATH_BOX);
  strcpy(config_box,DEF_CONFIG_BOX);
  strcpy(badnames_box,DEF_BADNAMES_BOX);
  strcpy(transfer_box,DEF_TRANSFER_BOX);
  strcpy(bcast_box,DEF_BCAST_BOX);
  strcpy(beacon_box,DEF_BEACON_BOX);
  strcpy(commands_box,DEF_COMMANDS_BOX);
  strcpy(ctext_box,DEF_CTEXT_BOX);
  strcpy(debug_box,DEF_DEBUG_BOX);
  strcpy(f6fbb_box,DEF_F6FBB_BOX);
  strcpy(rubriken_box,DEF_RUBRIKEN_BOX);
  strcpy(tcpip_box,DEF_TCPIP_BOX);
  strcpy(r_erase_box,DEF_R_ERASE_BOX);
  strcpy(profile_box,DEF_PROFILE_BOX);
  strcpy(whotalks_lan,DEF_WHOTALKS_LAN);
  strcpy(m_filter_prg,DEF_M_FILTER_PRG);
  strcpy(resume_inf,DEF_RESUME_INF);
  strcpy(dp_prg,DEF_DP_PRG);
  strcpy(dp_new_prg,DEF_DP_NEW_PRG);
  strcpy(box_socket,DEF_BOX_SOCKET);  
  strcpy(dpbox_user,DEF_DPBOX_USER);  
  strcpy(dpbox_initfile,INIT_FILE);
  dpbox_debug = 0;
  wrong_usage = 0;
  scanned = 1;
  while ((scanned < argc) && (!wrong_usage)) {
    if (strcmp(argv[scanned],"-i") == 0) {
      scanned++;
      if (scanned < argc) {
        strcpy(dpbox_initfile,argv[scanned]);
      }
      else wrong_usage = 1;
    }
    else if (strcmp(argv[scanned],"-f") == 0) {
      scanned++;
      if (scanned < argc) {
        strcpy(dpbox_debugfile,argv[scanned]);
        dpbox_debug = 2;
      }
      else wrong_usage = 1;
    }
    else if (strcmp(argv[scanned],"-d") == 0) {
      scanned++;
      dpbox_debug = 1;
    }
    else {
      wrong_usage = 1;
    }
    scanned++;
  }
  if (wrong_usage) {
    printf("Usage : dpbox [-i <init-file>] [-d] [-f <debug-file>]\n");
    return(1);
  }
  
  if (!(init_file_fp = fopen(dpbox_initfile,"r"))) {
    str_ptr = getenv("HOME");
    if (str_ptr != NULL) {
      snprintf(tmp_str,255,"%s/%s",str_ptr,dpbox_initfile);
      init_file_fp = fopen(tmp_str,"r");
    }

    if (!(init_file_fp)) {

      while(dpbox_initdir[index]!=NULL) {
        snprintf(tmp_str,255,"%s/%s",dpbox_initdir[index],dpbox_initfile);
        if ((init_file_fp = fopen(tmp_str,"r")))
          break;
        index++;
      }
    }
  }

  if (!(init_file_fp)) {
    printf("ERROR: %s not found\n\n",dpbox_initfile);
    return(1);
  }

  file_end = 0;
  file_corrupt = 0;
  while (!file_end) {
    if (fgets(line,256,init_file_fp) == NULL) {
      file_end = 1;
    }
    else {
      if (strlen(line) == 256) {
        file_end = 1;
        file_corrupt = 1;
      }
      else {
        if (line[0] != '#') { /* ignore comment-lines */
          rslt = sscanf(line,"%s %s",stri1,stri2);
          switch (rslt) {
          case EOF: /* ignore blank lines */
            break;
          case 2:
            if (analyse_value(stri1,stri2)) {
              file_end = 1;
              file_corrupt = 1;
            }
            break;
          default:
            file_end = 1;
            file_corrupt = 1;
            break;
          }
        }
      }
    }
  }
  fclose(init_file_fp);
  if (file_corrupt) {
    printf("ERROR: dpbox.ini is in wrong format\n\n");
    return(1);
  }
  else {
    return(0);
  }
}
