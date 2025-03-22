/* dpbox: Mailbox
   include file for main (main.h)
   created: Joachim Schurig DL8HBS 99/01/15
   updated: Joachim Schurig DL8HBS 99/01/15
*/

#ifndef MAIN_H
#define MAIN_H

#include "pastrix.h"

#ifdef MAIN_G
 #define vextern
#else
 #define vextern extern
#endif

vextern void linux_watchdog(short what, short value);
vextern short statusconvert(int status);
vextern void bootinf(char *s);
vextern void list_ifaceusage(short unr);
vextern void list_qrgs(short unr);
vextern void connect_from_box(short unr,char *eingabe);
vextern void tnt_command(short unr,char *eingabe);
vextern int find_socket(char *qrg, short *socket);
vextern void blocking_on(void);
vextern void blocking_off(void);
vextern void boxisbusy(boolean busy);
vextern boolean bcast_file(char stnc,char sport,char *qrg,
                   long fid,unsigned short ftype,
                   char *name1,char *adress,char *bbs_source,
                   char *bbs_destination,char *bbs_ax25uploader,
                   time_t bbs_upload_time,time_t bbs_expire_time,
                   char bbs_compression,char *bbs_bid,char bbs_msgtype,
                   char *bbs_title,char *bbs_fheader,
                   unsigned short bodychecksum,boolean delete_after_tx);

#undef vextern

#endif /* MAIN_H */

