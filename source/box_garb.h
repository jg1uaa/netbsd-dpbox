#ifndef BOX_GARB_H
#define BOX_GARB_H

void create_new_boxlog(short unr, bool bullids);
void delete_tempboxfiles(void);
void garbage_collection(bool xgar, bool fill_cbyte, bool check_all,
			       bool immediate, short unr);

#endif
