#ifndef BOX_GARB_H
#define BOX_GARB_H

extern void create_new_boxlog(short unr, bool bullids);
extern void delete_tempboxfiles(void);
extern void garbage_collection(bool xgar, bool fill_cbyte, bool check_all,
			       bool immediate, short unr);

#endif
