#ifndef BOX_ROUT_H
#define BOX_ROUT_H

#include "boxlocal.h"

/* handy defines for link tables */
#define add_lrow(row, add) row = (row + add) % LINKSPEEDS
#define sub_lrow(row, sub) row = (LINKSPEEDS + (row - sub)) % LINKSPEEDS
#define inc_lrow(row) add_lrow(row, 1)
#define dec_lrow(row) sub_lrow(row, 1)

void check_hpath(bool reorg);
bool complete_hierarchical_adress(char *mbx);
void add_hpath(char *mbx);
void find_neighbour(short mode, char *boxcall, char *nachbar);
void show_bbs_info(short unr, char *boxcall);
bool is_bbs(char *callsign);
bool direct_sf_bbs(char *callsign);
bool add_wprot_box(char *hpath, time_t update,
  unsigned short status, char *connectcall, char *sysopcall);
short scan_hierarchicals(char *from1, char *puffer, int32_t size,
  time_t *txdate, bool sfpartner, char msgtyp, char *lastvias);
bool add_wprot_routing(char *call, char *rxfrom, time_t timestamp, uint32_t quality, short hops);
void calc_linkspeed(routingtype *sfp, int64_t starttime, int32_t size);
void init_linkspeeds(sfdeftype *sfp, bool file_forward);
bool get_routing_table(short unr);
routingtype *find_routtable(char *call);
void compare_routing_and_sf_pointers(void);
void get_routing_targets(short unr, char *prefix);
uint32_t get_link_quality(char *call);
uint32_t get_link_quality_and_status(char *call);
time_t last_linkcheck(char *call);
bool needs_linkcheck(char *call, bool tryconnect);
void save_routing_table(void);
void calc_routing_table(void);
bool load_routing_table(void);
bool send_full_routing_bc(char *call);
bool is_phantom(char *call);
bool get_wprot_neighbour(char *call);
void set_wprot_neighbour(char *call, bool yes);


#endif
