#ifndef BOX_SCAN_H
#define BOX_SCAN_H


#include "boxlocal.h"

extern void check_accepted_bad_rcalls_syntax(char *s);
extern void create_my_rline(time_t rxdate, char *bid, char *rline);
extern char *get_rtoken(char *rline, char *token, char *para, short maxlen);
extern char *get_w0rli_call(char *rline, char *para);
extern void get_rcall(char *rline, char *rcall);
extern time_t get_headerdate(char *timestr);
extern boolean scan_for_ack(char *puffer, long size, boolean wpupdate,
      	      	     boolean wprotupdate, boolean part,
                     char *absender, char *board, char *subject, char *mbx,
		     char *bid, char msgtype, char *ackcall, boolean *is_binary,
		     boolean *is_dirty, boolean *is_html, char *dirtystring,
		     boolean *is_7plus, boolean *is_broken);

#endif
