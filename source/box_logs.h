#ifndef BOX_LOGS_H
#define BOX_LOGS_H

void append_rejectlog(short unr, char mtype, char *sender, char *board, char *mbx, char *bid,
      	      	      	     char *size, char *comment);
void append_profile(short unr, char *txt);
void append_readlog(short unr, char *txt);
void append_syslog(short unr);
void append_userlog(short unr);
void append_usersflog(short unr, char *sender, char *board, char *mbx, char *bid, char *size);
void append_protolog(short unr);
void append_sflog(short unr);
void append_convlog(short unr, char *txt);
bool get_debug_func(short pn, char *p);
void debug_2(short level, short unr, short pn, char *txt);

#define debug(level, unr, pn, txt)	if (level <= debug_level) debug_2(level, unr, pn, txt); \
					else lastproc = pn
#define debug0(level, unr, pn)		if (level <= debug_level) debug_2(level, unr, pn, NULL); \
					else lastproc = pn

#endif
