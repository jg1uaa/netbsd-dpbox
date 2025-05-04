/* dpbox: Mailbox
   include file for main (main.h)
   created: Joachim Schurig DL8HBS 99/01/15
   updated: Joachim Schurig DL8HBS 99/01/15
*/

#ifndef MAIN_H
#define MAIN_H

#include "pastrix.h"

void linux_watchdog(short what, short value);
short statusconvert(int status);
void bootinf(char *s);
void list_ifaceusage(short unr);
void list_qrgs(short unr);
void connect_from_box(short unr,char *eingabe);
void tnt_command(short unr,char *eingabe);
int find_socket(char *qrg, short *socket);
void blocking_on(void);
void blocking_off(void);
void boxisbusy(bool busy);
bool bcast_file(char stnc,char sport,char *qrg,
                   int32_t fid,unsigned short ftype,
                   char *name1,char *adress,char *bbs_source,
                   char *bbs_destination,char *bbs_ax25uploader,
                   time_t bbs_upload_time,time_t bbs_expire_time,
                   char bbs_compression,char *bbs_bid,char bbs_msgtype,
                   char *bbs_title,char *bbs_fheader,
                   unsigned short bodychecksum,bool delete_after_tx);

#endif /* MAIN_H */

