/* Output from p2c, the Pascal-to-C translator */
/* From input file "boxglobl.p" */


/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#define BOXGLOBL_G
#include "boxglobl.h"


void boxspoolfend(short chan)
{
  boxspoolread();
}


void boxpushmonstat(short tnc)
{
}


void boxpopmonstat(short tnc)
{
}


void boxpushunproto(short tnc)
{
}


void boxpopunproto(short tnc)
{
}


void boxsendallbufs(void)
{
}


void boxloadgreeting(void)
{
}


void boxsetamouse(void)
{
}


void boxsetbmouse(void)
{
}


void boxsetcompwait(short chan, short v)
{
}


void boxreadallbuffers(void)
{
}


void boxsaveallbuffers(void)
{
}


short boxmodemport(void)
{
  return -1;
}


long boxaktqrg(void)
{
  return 438300L;
}


long boxtncqrg(short i)
{
  return 438300L;
}


void boxsetbox(short i, short t)
{
}


void boxsetwasbox(short i, short t)
{
}


void boxsetnode(short i, boolean b)
{
}


void boxstartconnect(short i, char *s1, char *s2, boolean b1, boolean b2,
		     boolean b3)
{
}


short boxcallgemprg(char *s1, char *s2)
{
  return -1;
}


void boxsgdial1(void)
{
}


void boxegdial1(void)
{
}


void boxsgdial(void)
{
}


void boxegdial(void)
{
}


void boxwait(long l)
{
}


void boxrgdial1(char *s)
{
}


void boxendbusy(void)
{
}


void boxprprefix(short i, boolean b, char *s)
{
}


unsigned short boxruntime(void)
{
  return 1;
}


void boxfreemostram(boolean b)
{
}


void boxstartedit(char *s, char *p, long l1, long l2, boolean b1, boolean b2,
		  boolean b3)
{
}


void boxedit(char *s, char **p, long *l, long k)
{
}


void boxcancelmultibox(void)
{
}


void boxprreset(char *s)
{
}


void boxprkill(short i, char *s)
{
}


void boxprdir(short i, char *s)
{
}


boolean boxinitmodem(boolean b)
{
  return false;
}


boolean boxexitmodem(boolean b)
{
  return false;
}


void boxendmodem(void)
{
}
