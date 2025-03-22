/* Header for module box_init */
#ifndef BOX_INIT_H
#define BOX_INIT_H

#include "pastrix.h"
#include "boxglobl.h"
#include "boxlocal.h"

extern void check_bullidseek(void);
extern void create_all_msgnums(boolean recalc);
extern void load_tcpipbuf(void);
extern void load_all_parms(short unr);
extern void init_boxvars(void);
extern void exit_boxvars(void);

#endif /*BOX_INIT_H*/
