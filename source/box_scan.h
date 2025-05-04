#ifndef BOX_SCAN_H
#define BOX_SCAN_H


#include "boxlocal.h"

void check_accepted_bad_rcalls_syntax(char *s);
void create_my_rline(time_t rxdate, char *bid, char *rline);
char *get_rtoken(char *rline, char *token, char *para, short maxlen);
char *get_w0rli_call(char *rline, char *para);
void get_rcall(char *rline, char *rcall);
time_t get_headerdate(char *timestr);
bool scan_for_ack(char *puffer, int32_t size, bool wpupdate,
		     bool wprotupdate, bool part,
                     char *absender, char *board, char *subject, char *mbx,
		     char *bid, char msgtype, char *ackcall, bool *is_binary,
		     bool *is_dirty, bool *is_html, char *dirtystring,
		     bool *is_7plus, bool *is_broken);

#endif
