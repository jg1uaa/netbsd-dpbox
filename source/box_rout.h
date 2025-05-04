#ifndef BOX_ROUT_H
#define BOX_ROUT_H

#include "boxlocal.h"

/* handy defines for link tables */
#define add_lrow(row, add) row = (row + add) % LINKSPEEDS
#define sub_lrow(row, sub) row = (LINKSPEEDS + (row - sub)) % LINKSPEEDS
#define inc_lrow(row) add_lrow(row, 1)
#define dec_lrow(row) sub_lrow(row, 1)

extern void check_hpath(bool reorg);
extern bool complete_hierarchical_adress(char *mbx);
extern void add_hpath(char *mbx);
extern void find_neighbour(short mode, char *boxcall, char *nachbar);
extern void show_bbs_info(short unr, char *boxcall);
extern bool is_bbs(char *callsign);
extern bool direct_sf_bbs(char *callsign);
extern bool add_wprot_box(char *hpath, time_t update,
  unsigned short status, char *connectcall, char *sysopcall);
extern short scan_hierarchicals(char *from1, char *puffer, int32_t size,
  time_t *txdate, bool sfpartner, char msgtyp, char *lastvias);
extern bool add_wprot_routing(char *call, char *rxfrom, time_t timestamp, uint32_t quality, short hops);
extern void calc_linkspeed(routingtype *sfp, int64_t starttime, int32_t size);
extern void init_linkspeeds(sfdeftype *sfp, bool file_forward);
extern bool get_routing_table(short unr);
extern routingtype *find_routtable(char *call);
extern void compare_routing_and_sf_pointers(void);
extern void get_routing_targets(short unr, char *prefix);
extern uint32_t get_link_quality(char *call);
extern uint32_t get_link_quality_and_status(char *call);
extern time_t last_linkcheck(char *call);
extern bool needs_linkcheck(char *call, bool tryconnect);
extern void save_routing_table(void);
extern void calc_routing_table(void);
extern bool load_routing_table(void);
extern bool send_full_routing_bc(char *call);
extern bool is_phantom(char *call);
extern bool get_wprot_neighbour(char *call);
extern void set_wprot_neighbour(char *call, bool yes);


#endif
