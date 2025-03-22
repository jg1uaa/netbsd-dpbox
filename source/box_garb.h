#ifndef BOX_GARB_H
#define BOX_GARB_H

extern void create_new_boxlog(short unr, boolean bullids);
extern void delete_tempboxfiles(void);
extern void garbage_collection(boolean xgar, boolean fill_cbyte, boolean check_all,
      	      	      	       boolean immediate, short unr);

#endif
