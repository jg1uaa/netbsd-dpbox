/* Header for module box_init */
#ifndef BOX_INIT_H
#define BOX_INIT_H

#include "pastrix.h"
#include "boxglobl.h"
#include "boxlocal.h"

void check_bullidseek(void);
void create_all_msgnums(bool recalc);
void load_tcpipbuf(void);
void load_all_parms(short unr);
void init_boxvars(void);
void exit_boxvars(void);

#endif /*BOX_INIT_H*/
