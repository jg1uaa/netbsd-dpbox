#ifndef BOX_LOGS_H
#define BOX_LOGS_H

extern void append_rejectlog(short unr, char mtype, char *sender, char *board, char *mbx, char *bid,
      	      	      	     char *size, char *comment);
extern void append_profile(short unr, char *txt);
extern void append_readlog(short unr, char *txt);
extern void append_syslog(short unr);
extern void append_userlog(short unr);
extern void append_usersflog(short unr, char *sender, char *board, char *mbx, char *bid, char *size);
extern void append_protolog(short unr);
extern void append_sflog(short unr);
extern void append_convlog(short unr, char *txt);
extern boolean get_debug_func(short pn, char *p);
extern void debug_2(short level, short unr, short pn, char *txt);

#define debug(level, unr, pn, txt)	if (level <= debug_level) debug_2(level, unr, pn, txt); \
					else lastproc = pn
#define debug0(level, unr, pn)		if (level <= debug_level) debug_2(level, unr, pn, NULL); \
					else lastproc = pn

#endif
