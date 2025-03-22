/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#include <ctype.h>
#include "boxlocal.h"
#include "filesys.h"
#include "box_logs.h"
#include "box_mem.h"
#include "tools.h"
#include "box_inou.h"
#include "box_file.h"
#include "box_sf.h"
#include "box_sub.h"
#include "box_sys.h"
#include "boxfserv.h"
#include "sort.h"
#include "box_wp.h"

#define hashsize	1023

typedef struct hashrecord {
  long			offset;
  struct hashrecord	*next;
} hashrecord;

typedef hashrecord	*hasharr[hashsize + 1];

static hashrecord	**hboxhash  = NULL;
static hashrecord     	**bidhash   = NULL;


/* ---------------------------------------------------------------- */

static void clear_hash(hashrecord ***hash)
{
  long		x;
  hashrecord	*p1, *p2;

  debug0(5, -1, 128);
  if (*hash == NULL)
    return;
  for (x = 0; x <= hashsize; x++) {
    p1		= (*hash)[x];
    while (p1 != NULL) {
      p2	= p1->next;
      free(p1);
      p1	= p2;
    }
  }
  free(*hash);
  *hash		= NULL;
}


static long calc_hashcs(char *token)
{
  short		x, y;
  long		cs;

  cs		= 0;
  y		= strlen(token);
  if (y > 12) y = 12;	/* This algorithm is optimized for 12 char alphanumericals.	*/
			/* Using more digits would imbalance the dislocation of nodes.	*/
			/* Leave it at a max of 12 even for longer token or change	*/
			/* the computation below to a more clever one...		*/

  for (x = 1; x <= y; x++) {
    cs		+= ((token[x - 1] & 0x7f) - 33) * x * 3;
  }

  if (y < 7 && y > 1) {
    /* for lower token lengths, mix the result up with first and last chars (BBS-HPATH-HASH) */
    cs += (((token[0] & 0x7f) - 33) + (token[y - 1] & 0x7f) - 33);
    if (y > 2) {
      cs += ((token[y - 2] & 0x7f) - 33);
    }
  }

  cs 		%= hashsize + 1;

  return cs;
}


static boolean add_hash(hashrecord **hash, long offs, char *token, boolean deb)
{
  long		cs;
  hashrecord	*p1, *p2;

  if (deb) {
    debug0(5, 0, 129);
  }
  if (hash == NULL)
    return true;

  cs		= calc_hashcs(token);
  if (hash[cs] == NULL) {
    p1		= malloc(sizeof(hashrecord));
    if (p1 == NULL) {
      debug(0, 0, 129, "no mem (1)");
      return false;
    }
    p1->next	= NULL;
    p1->offset	= offs;
    hash[cs]	= p1;
    return true;
  }

  p1		= hash[cs];

  while (p1->next != NULL) p1 = p1->next;

  p2		= malloc(sizeof(hashrecord));
  if (p2 == NULL) {
    debug(0, 0, 129, "no mem (2)");
    return false;
  }

  p2->next	= NULL;
  p2->offset	= offs;
  p1->next	= p2;
  return true;
}


static void delete_hash_offset(hashrecord **hash, long offset)
{
  long		ct;
  hashrecord	*p1, *p2;

  debug0(5, 0, 131);

  if (hash == NULL)
    return;

  for (ct = 0; ct <= hashsize; ct++) {
    p1		= hash[ct];
    p2		= NULL;
    while (p1 != NULL && p1->offset != offset) {
      p2	= p1;
      p1	= p1->next;
    }
    if (p1 != NULL) {
      if (p2 != NULL)
	p2->next	= p1->next;
      else
	hash[ct]	= p1->next;
      free(p1);
      return;
    }
  }
}


static void update_hash(hashrecord **hash, char *token, long offset)
{
  debug0(5, 0, 132);

  if (hash != NULL) {
    delete_hash_offset(hash, offset);
    add_hash(hash, offset, token, true);
  }
}


static void disp_hash(hashrecord **hash, short unr)
{
  long		x, z;
  hashrecord	*p;
  long		ct;
  char		w[256];

  debug0(5, unr, 144);

  if (hash == NULL) {
    wlnuser(unr, "hash not active");
    return;
  }

  for (x = 0; x <= hashsize; x++) {
    if (x % 17 == 0) {
      wlnuser0(unr);
      wlnuser0(unr);
      wuser(unr, "hashpos: ");
      for (z = 0; z <= 16; z++) {
	if (x + z <= hashsize) {
	  sprintf(w, "%4ld", x + z);
	  wuser(unr, w);
	}
      }
      wlnuser0(unr);
      wuser(unr, "nodes  : ");
    }
    ct		= 0;
    p		= hash[x];
    while (p != NULL) {
      ct++;
      p		= p->next;
    }
    sprintf(w, "%4ld", ct);
    wuser(unr, w);
  }
  wlnuser0(unr);
}


/* ---------------------------------------------------------------------- */

long bidhash_active(void)
{
  if (bidhash != NULL)
    return (sfsize(msgidlog) / sizeof(bidtype) * sizeof(hashrecord) + sizeof(hasharr));
  else
    return 0;
}


long hboxhash_active(void)
{
  if (hboxhash != NULL)
    return (sfsize(hpath_box) / sizeof(hboxtyp) * sizeof(hashrecord) + sizeof(hasharr));
  else
    return 0;
}


void disp_bidhash(short unr)
{
  disp_hash(bidhash, unr);
}


void disp_hboxhash(short unr)
{
  disp_hash(hboxhash, unr);
}


void clear_bidhash(void)
{
  clear_hash(&bidhash);
}


static boolean	bptr_loaded  = false;

void clear_hboxhash(void)
{
  clear_hash(&hboxhash);
  bptr_loaded	= false;
}

#define bsize 500*sizeof(bidtype)

static void load_bidhash(void)
{
  char		*buf, *p;
  long		filled;
  boolean     	error = false;
  short		k;
  long		ct, a, b, c, needed_mem;
  bidtype	bid;

  clear_bidhash();
  debug0(5, 0, 130);

  c		= sfsize(msgidlog);
  a		= c / sizeof(bidtype);
  needed_mem	= sizeof(hasharr) + a * sizeof(hashrecord);

  if (needed_mem > memavail__() - 100000L)
    return;

  bidhash = malloc(sizeof(hasharr));
  if (bidhash == NULL)
    return;

  buf		= malloc(bsize);
  p		= buf;
  filled	= 0;

  boxbusy("loading bid hash table");
  for (ct = 0; ct <= hashsize; ct++)
  bidhash[ct]	= NULL;

  if (exist(msgidlog)) {

    k		= sfopen(msgidlog, FO_READ);
    if (k >= minhandle) {
      for (b = 1; b <= a; b++) {
	if (b % 20 == 0)
	  dp_watchdog(2, 4711);
	if (buf != NULL) {
	  if (filled <= 0) {
            filled	= sfread(k, bsize, buf);
            p		= buf;
          }
          if (filled >= sizeof(bidtype)) {
	    strncpy(bid, p, sizeof(bidtype));
	    p		= &p[sizeof(bidtype)];
	  }
	  filled	-= sizeof(bidtype);
	} else {
	  if (sfread(k, sizeof(bidtype), bid) != sizeof(bidtype)) {
	    error = true;
	    break;
	  }
	}
	cut(bid, LEN_BID); /* nur zur Sicherheit */
	if (!add_hash(bidhash, (b - 1) * sizeof(bidtype), bid, false)) {
	  error = true;
	  break;
	}
      }
      sfclose(&k);
    } else error = true;
    
    if (error) {
      debug(0, 0, 130, "no mem for list (or read error in input file)");
      clear_bidhash();
    }

  }

  if (buf != NULL) free(buf);
  boxendbusy();
}

#undef bsize;

static long find_bidhash(char *bid)
{
  long		cs;
  hashrecord	*p1;
  short		k;
  bidtype	bid2;
  short		l2;

  debug0(5, 0, 133);

  if (bidhash != NULL) {
    l2		= strlen(bid);
    if (l2 > 0 && l2 <= LEN_BID) {
      cs	= calc_hashcs(bid);
      p1	= bidhash[cs];
      if (p1 != NULL) {
	k	= sfopen(msgidlog, FO_READ);
	if (k >= minhandle) {
	  while (p1 != NULL) {
	    sfseek(p1->offset, k, SFSEEKSET);
	    sfread(k, sizeof(bidtype), bid2);
	    if (!strcmp(bid, bid2)) {
	      sfclose(&k);
	      return p1->offset;
	    }
	    p1	= p1->next;
	  }
	  sfclose(&k);
	}
      }
    }
  }
  return -1;
}


void write_msgid(long nr, char *ibuf)
{
  long		seek;
  short		k;

  debug(2, 0, 62, ibuf);

  k		= strlen(ibuf);

  if (k < 1 || k > LEN_BID || !strcmp(ibuf, "#NONE#"))
    return;

  k		= sfopen(msgidlog, FO_RW);
  if (k < minhandle)
    k		= sfcreate(msgidlog, FC_FILE);
  if (k < minhandle)
    return;

  if (nr < 0) {
    if (bullidseek >= maxbullids)
      bullidseek	= 0;
    seek		= bullidseek * sizeof(bidtype);
    if (sfseek(seek, k, SFSEEKSET) != seek) {
      debug(0, 0, 62, "invalid bullidseek, readjusted (1)");
      bullidseek	= sfseek(0, k, SFSEEKEND) / sizeof(bidtype);
    }
    bullidseek++;
    update_bidseek();
  } else {
    seek		= (nr - 1) * sizeof(bidtype);
    if (sfseek(seek, k, SFSEEKSET) != seek) {
      debug(0, 0, 62, "invalid bullidseek, readjusted (2)");
      bullidseek	= sfseek(0, k, SFSEEKEND) / sizeof(bidtype);
    }
  }
  sfwrite(k, sizeof(bidtype), ibuf);

  sfclose(&k);

  /* und falls vorhanden den hash updaten */

  update_hash(bidhash, ibuf, seek);
}


/* local for bid_mem ****************************************************** */

static void kill_id(short *k, boolean openfile, long at)
{
  bidtype id;

  if (!openfile)
    *k = sfopen(msgidlog, FO_RW);
  if (*k < minhandle)
    return;
  sfseek(at, *k, SFSEEKSET);
  *id = '\0';
  sfwrite(*k, sizeof(bidtype), id);
  if (!openfile)
    sfclose(k);

  /* falls vorhanden auch im hash loeschen */
  delete_hash_offset(bidhash, at);
}


static long search_in_ram(boolean mul, short lnid, char *new_id,
  bidchecktype bidcheck, bidarrtype bidarr, short panz, boolean *ok,
  boolean delet, short *k, char *id, boolean openfile, char *puf, long psiz,
  long offset)
{
  long Result, rpos, ct;
  short x, y;
  char *mp;

  debug0(5, -1, 145);
  rpos = 0;
  Result = -1;

  if (mul) {
    while (rpos < psiz && !*ok) {
      mp = (char *)(&puf[rpos]);
      for (x = 0; x < panz; x++) {
	if (!strcmp(mp, bidarr[x])) {
	  bidcheck[x] = false;
	  *ok = true;
	  for (y = 0; y < panz; y++) {
	    if (bidcheck[y] == true)
	    *ok = false;
	  }
	}
      }
      rpos += sizeof(bidtype);
    }
    return Result;
  }

  ct = offset / sizeof(bidtype);

  while (rpos < psiz && !*ok) {
    ct++;
    mp = (char *)(&puf[rpos]);
    if (!strcmp(mp, new_id)) {
      Result = ct;
      *ok = true;
      if (delet) {
	*mp = '\0';
	kill_id(k, openfile, offset + rpos);
      }
    }

    rpos += sizeof(bidtype);
  }
  return Result;
}


static long search_in_hash(boolean mul, char *new_id, short panz,
			   bidarrtype bidarr, bidchecktype bidcheck,
			   boolean delet, short *k)
{
  long Result;
  short x;
  long ct;

  debug0(5, -1, 146);
  Result = -1;
  if (mul) {
    for (x = 0; x < panz; x++)
      bidcheck[x] = (find_bidhash(bidarr[x]) < 0);
    return Result;
  }
  ct = find_bidhash(new_id);
  if (delet && ct >= 0)
    kill_id(k, false, ct);
  if (ct >= 0)
    return (ct / sizeof(bidtype) + 1);
  return Result;
}


#define blocksize       sizeof(bidtype)*315

static long bid_mem(boolean mul, boolean delet, short panz, bidchecktype bidcheck,
		    bidarrtype bidarr_, char *new_id_)
{
  long Result;
  bidarrtype bidarr;
  bidtype new_id;
  short k, x, y;
  long anz, err, ct, dsize, bct;
  char *puffer;
  bidtype id;
  boolean ok;
  short l1, l2, lnid;

  memcpy(bidarr, bidarr_, sizeof(bidarrtype));
  strcpy(new_id, new_id_);
  debug(4, 0, 63, new_id);

  if (mul) {
    for (x = 0; x < MAXFBBPROPS; x++)
      bidcheck[x] = true;
  }
  ok = false;
  Result = -1;
  lnid = strlen(new_id);
  if (!(mul || (lnid > 1 && lnid <= LEN_BID)))
    return Result;
    
  if (bidhash == NULL)
    load_bidhash();

  if (bidhash != NULL) {
    Result = search_in_hash(mul, new_id, panz, bidarr, bidcheck, delet, &k);

  } else {
    dsize = sfsize(msgidlog);
    if (dsize > 0) {
      if (delet)
	k = sfopen(msgidlog, FO_RW);
      else
	k = sfopen(msgidlog, FO_READ);
      if (k >= minhandle) {
	puffer = malloc(blocksize);
	if (puffer != NULL) {
	  bct = 0;
	  err = dsize - bct;
	  while (err > 0 && !ok) {
	    if (err > blocksize)
	      err = blocksize;
	    err = sfread(k, err, puffer);
	    if (err > 0) {
              Result = search_in_ram(mul, lnid, new_id, bidcheck, bidarr,
				       panz, &ok, delet, &k, id, true, puffer,
				       err, bct);
	      bct += err;
	      err = dsize - bct;
	      continue;
	    }
	    if (err < 0) {
	      debug(0, 0, 63, "read error 2");
	      break;
	    }
	    if (err == 0)
	      break;
	  }
	} else {
	  debug(3, 0, 63, "Disk");

	  anz = dsize / sizeof(bidtype);
	  for (ct = 1; ct <= anz; ct++) {
	    sfread(k, sizeof(bidtype), id);
	    l1 = strlen(id);

	    if (mul) {
	      for (x = 0; x < panz; x++) {
		l2 = strlen(bidarr[x]);
		if (l1 == l2) {
		  if (id[l1 - 1] == bidarr[x][l2 - 1]) {
		    if (id[1] == bidarr[x][1]) {
		      if (!strcmp(id, bidarr[x])) {
			bidcheck[x] = false;
			ok = true;
			for (y = 0; y < panz; y++) {
			  if (bidcheck[y] == true)
			    ok = false;
			}
			if (ok) break;
		      }
		    }
		  }
		}
	      }
	      if (ok) break;
	    } else {
	      if (l1 == lnid) {
		if (id[l1 - 1] == new_id[lnid - 1]) {
		  if (id[1] == new_id[1]) {
		    if (!strcmp(id, new_id)) {
		      Result = ct;
		      if (delet)
			kill_id(&k, true, (ct - 1) * sizeof(bidtype));
		      break;
		    }
		  }
		}
	      }
	    }
	  }
	}

	sfclose(&k);
      }

    }
  }

  return Result;
}

#undef blocksize


boolean check_double(char *new_id)
{
  bidchecktype bidcheck;
  bidarrtype bidarr;

  if (bidhash != NULL)
    return (find_bidhash(new_id) < 0);
  else
    return (bid_mem(false, false, 1, bidcheck, bidarr, new_id) < 0);
}


long bull_mem(char *new_id, boolean delet)
{
  bidchecktype bidcheck;
  bidarrtype bidarr;
  long ct;

  if (bidhash != NULL && !delet) {
    ct = find_bidhash(new_id);
    if (ct >= 0)
      return (ct / sizeof(bidtype) + 1);
    else
      return -1;
  } else
    return (bid_mem(false, delet, 1, bidcheck, bidarr, new_id));
}


void multiple_bullcheck(short ct, bidchecktype bidcheck, bidarrtype bidarr)
{
  bidtype nid;
  short x;

  if (bidhash != NULL) {
    for (x = 0; x < ct; x++)
      bidcheck[x] = (find_bidhash(bidarr[x]) < 0);
  } else {
    nid[0] = '\0';
    bid_mem(true, false, ct, bidcheck, bidarr, nid);
  }
}


/* *************************************************************************** */

static void load_bptr(void)
{
  short		k;
  long		l, x, rps, rpr, rpb;
  char		*rb;
  hboxtyp	hbox, *hboxp;

  debug0(1, -1, 158);
  clear_hboxhash();

  hboxhash	= malloc(sizeof(hasharr));
  if (hboxhash == NULL)
    return;

  for (x = 0; x <= hashsize; x++)
    hboxhash[x]	= NULL;
  l		= sfsize(hpath_box) / sizeof(hboxtyp);
  if (l > 0) {
    k		= sfopen(hpath_box, FO_READ);
    if (k >= minhandle) {

      boxbusy("loading hbox hash");

      rpb	= sizeof(hboxtyp) * 300;
      rb	= malloc(rpb);

      if (rb == NULL) {
	for (x = 0; x < l; x++) {
	  if (sfread(k, sizeof(hboxtyp), (char *)(&hbox)) == sizeof(hboxtyp)) {
	    if (hbox.version == HPATHVERSION && callsign(hbox.call))
	      add_hash(hboxhash, x * sizeof(hboxtyp), hbox.call, false);
	  }
	}
      } else {
	rps	= 0;
	rpr	= 0;

	for (x = 0; x < l; x++) {
	  if (rpr + sizeof(hboxtyp) - 1 > rps) {
	    rps	= sfread(k, rpb, rb);
	    rpr	= 0;
	  }

	  hboxp	= (hboxtyp *)(&rb[rpr]);
	  rpr	+= sizeof(hboxtyp);

	  if (callsign(hboxp->call))
	    add_hash(hboxhash, x * sizeof(hboxtyp), hboxp->call, false);

	}

	free(rb);
      }

      boxendbusy();
    }
    sfclose(&k);
  }
  bptr_loaded	= true;
}


void load_initial_hbox(void)
{
  if (bptr_loaded)
    return;
    
  load_bptr();
}


void add_bptr(char *call, long bpos)
{
  debug(4, -1, 159, call);
  if (bpos >= 0 && (unsigned long)strlen(call) < 32 &&
      ((1L << strlen(call)) & 0x7e) != 0) {
    /*   if hboxhash = NIL then load_bptr; doesn´t work, file is already open for write */
    add_hash(hboxhash, bpos, call, true);
  }
}


long load_hbox(short hboxhandle, char *call, hboxtyp *hbox)
{
  short		k;
  long		Result, cs;
  hashrecord	*p1;

  debug(4, -1, 160, call);

  Result	= -1;
  *hbox->call	= '\0';

  if (hboxhandle < minhandle)
    load_initial_hbox();

  if (hboxhash == NULL)
    return Result;

  if (hboxhandle >= minhandle)
    k		= hboxhandle;
  else
    k		= sfopen(hpath_box, FO_READ);

  if (k < minhandle)
    return Result;

  cs		= calc_hashcs(call);
  p1		= hboxhash[cs];
  if (p1 != NULL) {
    while (p1 != NULL) {
      if (sfseek(p1->offset, k, SFSEEKSET) == p1->offset) {
	if (sfread(k, sizeof(hboxtyp), (char *)hbox) == sizeof(hboxtyp)) {
	  if (hbox->version == HPATHVERSION && !strcmp(hbox->call, call)) {
	    Result	= p1->offset;
	    if (hboxhandle < minhandle)
	      sfclose(&k);
	    return Result;
	  }
	}
      }
      p1	= p1->next;
    }
  }
  if (hboxhandle < minhandle)
    sfclose(&k);

  return Result;
}


/*************************************************************************/


long true_bin(char *zeile)
{
  short k, ct;
  char	c;
  long	size;

  if (zeile[0] != '#')
    return -1;
    
  if (zeile[1] != 'B')
    return -1;

  if (zeile[2] != 'I')
    return -1;

  if (zeile[3] != 'N')
    return -1;

  if (zeile[4] != '#')
    return -1;
    
  k   = strlen(zeile);
  if (k < 6)
    return -1;

  size	= 0;
  ct  	= 5;
  while (ct < k) {
    ct++;
    c = zeile[ct - 1];
    if (!isdigit(c)) {
      if (c == '#') {
	if (ct > 6)
	  return size;
      }
      return -1;
    } else size = size*10 + (long)(c - '0');
  }

  return size;
}


long get_binstart(char *puffer, long size, char *fname)
{

  /* unfortunately, memmem() can´t be used because of erroneus library	*/
  /* implementations in most linux libs (at least up till today)	*/
  /* so we use memchr and then a less sophisticated algorithm		*/

  char		*rp1, *rp2;
  long		remainder, i, ct, binstart, eol;

  debug0(4, 0, 60);

  rp2		= puffer;
  remainder	= size;

  while (remainder > 6) {	/* we search at least #BIN#1#	*/

    /* Scan. If rp1 == NULL, go away...		*/
    rp1	= memchr(rp2, '#', remainder);
    if (rp1 == NULL) {
      *fname = '\0';
      return 0;
    }

    remainder = size - (rp1 - puffer);

    /* is this the beginning of a line?		*/
    if (remainder > 6 && (rp1 == puffer || rp1[-1] == '\n' || rp1[-1] == '\015')) {

	/* yes, now check for #BIN#sizearg#		*/
	/* max headerlen is #BIN#1234567890# (9.9 GB)	*/
	/*                  0123456789012345		*/
  	if (rp1[1] == 'B' && rp1[2] == 'I' && rp1[3] == 'N' && rp1[4] == '#') {
	  i	= 4;
	  while (++i <= remainder && i < 16) {
	    if (!isdigit(rp1[i])) {
	      if (rp1[i] == '#') {
		if (i > 5) {		/* valid header	*/
		  binstart	= size;	/* this is a fallback in case of truncated input buffer */
		  *fname	= '\0';
		  /* find EOL	*/		  
		  while (++i <= remainder && i < 256) {
		    if (rp1[i] == '\015' || rp1[i] == '\n') { /* carriage return or linefeed */
		      eol	= i;
		      binstart	= (rp1+i+1) - puffer;
		      if (i < remainder && rp1[i] == '\015') {
			if (rp1[i+1] == '\n')
			  binstart++;
		      }
		      /* step back to LAST '#' (not necessarily the one found above) */
		      while (rp1[--i] != '#');
		      i++;
		      ct	= 0;
		      while (i < eol) fname[ct++] = rp1[i++];
		      fname[ct]	= '\0';
      	      	      break;
		    }
		  }
		  debug(4, 0, 60, fname);
		  return binstart;
		}
	      }
	      break;	/* exit loop	*/
            }
	  }
	}

    }

    if (remainder > 6) { 	/* next run	*/
	rp2	= rp1+1;
	--remainder;
    }

  }

  *fname	= '\0';
  return 0;
}


void add_line_to_buff(char **buf1, long *size1, long inspos, char *srline)
{
  char	        *buf2;
  long	        size2, offset, err;
  short       	k;
  long	      	ll;
  char	      	tname[256];
 
  debug0(5, -1, 147);
  ll 	      	= strlen(srline);
  size2       	= *size1 + ll + 1;
  buf2	      	= malloc(size2);
  if (inspos > *size1 || inspos < 0)
    inspos    	= 0;

  if (buf2 != NULL) {  /* everything fits in memory */
  
    if (inspos > 0) memcpy(buf2, *buf1, inspos);
    memcpy(buf2+inspos, srline, ll);
    buf2[ll + inspos]  = 10;
    offset    	= ll + inspos + 1;
    memcpy(buf2+offset, *buf1+inspos, *size1 - inspos);
    free(*buf1);
    *buf1     	= buf2;
    *size1    	= size2;
    return;
  
  }

  debug(3, 0, 61, "Disk");
  sprintf(tname, "%sTEMPR%cXXXXXX", tempdir, extsep);
  mymktemp(tname);
  k   	    = sfcreate(tname, FC_FILE);
  if (k < minhandle)
    return;
    
  err 	      	= sfwrite(k, *size1, *buf1);
  if (err == *size1) {
    free(*buf1);
    sfseek(0, k, SFSEEKSET);   /* back to start */
    *buf1     	= malloc(size2);
    if (*buf1 != NULL) {
      if (inspos > 0) sfread(k, inspos, *buf1);
      memcpy(*buf1+inspos, srline, ll);
      *buf1[ll + inspos]  = 10;
      offset  	= ll + inspos + 1;
      sfread(k, *size1 - inspos, *buf1+offset);
      *size1  	= size2;
    } else {  /*hat alles nichts genuetzt*/
      *buf1   	= malloc(*size1);
      if (*buf1 != NULL)
	sfread(k, *size1, *buf1);
      else {
	*buf1 	= NULL;
	*size1	= 0;
      }
    }
  }
  sfclose(&k);
  sfdelfile(tname);
}



void pack_entry(char **puffer, long *size, short *pmode)
{
  short		h1, packresult;
  boolean	ugzip;
  long		nasize, nsize;
  char		*nmem;
  pathstr	archiv;

  debug0(3, 0, 68);

  if (*size <= 0 || *puffer == NULL)
    return;

  sprintf(archiv, "%sDPXXXXXX%cLZH", tempdir, extsep);
  mymktemp(archiv);
  ugzip		= use_gzip(*size);
  packresult	= mempacker(ugzip, true, *puffer, *size, &nmem, &nsize, archiv, false);
  if (packresult == 0) {
    free(*puffer);
    *puffer	= NULL;
    *size	= 0;
    if (nsize == 0) {
      nasize	= sfsize(archiv);
      *puffer	= malloc(nasize);
      if (*puffer != NULL) {
	h1	= sfopen(archiv, FO_READ);
	if (h1 >= minhandle) {
	  *size	= sfread(h1, nasize, *puffer);
	  sfclose(&h1);
	  if (nasize != *size) {
	    free(*puffer);
	    *puffer	= NULL;
	    *size	= 0;
	    debug(0, 0, 68, "read error");
	  } else {
	    if (ugzip) *pmode |= PM_GZIP;
	    else *pmode |= PM_HUFF2;
	  }
	} else
	  debug(0, 0, 68, "open error");
      } else
	debug(0, 0, 68, "malloc error");
    } else {
      *puffer	= nmem;
      *size	= nsize;
      if (ugzip) *pmode |= PM_GZIP;
      else *pmode |= PM_HUFF2;
    }
  }
  sfdelfile(archiv);
  if (*size >= 0)
    return;

  *puffer	= NULL;
  *size		= 0;
  debug(0, 0, 68, "packerror: size = 0");
}
