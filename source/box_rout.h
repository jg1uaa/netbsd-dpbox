#ifndef BOX_ROUT_H
#define BOX_ROUT_H

#include "boxlocal.h"

/* handy defines for link tables */
#define add_lrow(row, add) row = (row + add) % LINKSPEEDS
#define sub_lrow(row, sub) row = (LINKSPEEDS + (row - sub)) % LINKSPEEDS
#define inc_lrow(row) add_lrow(row, 1)
#define dec_lrow(row) sub_lrow(row, 1)

extern void check_hpath(boolean reorg);
extern boolean complete_hierarchical_adress(char *mbx);
extern void add_hpath(char *mbx);
extern void find_neighbour(short mode, char *boxcall, char *nachbar);
extern void show_bbs_info(short unr, char *boxcall);
extern boolean is_bbs(char *callsign);
extern boolean direct_sf_bbs(char *callsign);
extern boolean add_wprot_box(char *hpath, time_t update,
  unsigned short status, char *connectcall, char *sysopcall);
extern short scan_hierarchicals(char *from1, char *puffer, long size,
  time_t *txdate, boolean sfpartner, char msgtyp, char *lastvias);
extern boolean add_wprot_routing(char *call, char *rxfrom, time_t timestamp, unsigned long quality, short hops);
extern void calc_linkspeed(routingtype *sfp, long starttime, long size);
extern void init_linkspeeds(sfdeftype *sfp, boolean file_forward);
extern boolean get_routing_table(short unr);
extern routingtype *find_routtable(char *call);
extern void compare_routing_and_sf_pointers(void);
extern void get_routing_targets(short unr, char *prefix);
extern unsigned long get_link_quality(char *call);
extern unsigned long get_link_quality_and_status(char *call);
extern time_t last_linkcheck(char *call);
extern boolean needs_linkcheck(char *call, boolean tryconnect);
extern void save_routing_table(void);
extern void calc_routing_table(void);
extern boolean load_routing_table(void);
extern boolean send_full_routing_bc(char *call);
extern boolean is_phantom(char *call);
extern boolean get_wprot_neighbour(char *call);
extern void set_wprot_neighbour(char *call, boolean yes);


#endif
