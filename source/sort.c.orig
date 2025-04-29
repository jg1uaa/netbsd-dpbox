#define SORT_G
#include "sort.h"
#include "shell.h"

/* don't set sortstrsize lower than 256! */
#define sortstrsize     512


typedef struct strmem {
  struct strmem *left, *right;
  char *zeile;
  long hits;
} strmem;


  /*----------------------------------*/
static void fuege_ein(strmem **s, strmem *entry_, boolean single)
{
  /*Fuegt den String in den Baum ein  */
  /*----------------------------------*/
  int ret;

  if (*s == NULL) {
    *s = entry_;
    return;
  }
  if ((ret = strcmp(entry_->zeile, (*s)->zeile)) > 0)
    fuege_ein(&(*s)->left, entry_, single);
  else if (ret < 0 || !single)
    fuege_ein(&(*s)->right, entry_, single);
  else {
    (*s)->hits++;
    free(entry_->zeile);
    free(entry_);
  }
}


  /*-------------------------------------*/
static void ordne_ein(char *instr, strmem **lroot, boolean *no_mem,
			boolean single)
{
  /*Findet die richtige Stelle wo instr  */
  /*im Baum einzusortieren ist.          */
  /*-------------------------------------*/
  strmem *entry_;

  entry_ = malloc(sizeof(strmem));
  if (entry_ == NULL) {
    *no_mem = true;
    return;
  }
  entry_->zeile = malloc(strlen(instr) + 1);
  if (entry_->zeile == NULL) {
  	*no_mem = true;
  	free(entry_);
  	return;
  }
  entry_->left = NULL;
  entry_->right = NULL;
  entry_->hits = 1;
  strcpy(entry_->zeile, instr);
  fuege_ein(lroot, entry_, single);
}


static void lese_file_ein(char *name, strmem **lroot, boolean *no_mem,
				boolean single)
{
  char instr[sortstrsize+1];
  short workfile;

  workfile = sfopen(name, FO_READ);
  while (file2lstr(workfile, instr, sortstrsize) && !*no_mem) {
    if (*instr != '\0') {
      cut(instr, sortstrsize);
      ordne_ein(instr, lroot, no_mem, single);
    }
  }
  sfclose(&workfile);
}


  /*-----------------------------------*/
static void schreibe_file2(short *workfile, strmem *s)
{
  /*Rekursiver Schreibalgr.            */
  /*-----------------------------------*/
  if (s->right != NULL)
    schreibe_file2(workfile, s->right);
  str2file(workfile, s->zeile, true);
  if (s->left != NULL)
    schreibe_file2(workfile, s->left);
  free(s->zeile);
  free(s);
}


static void schreibe_file(char *name, strmem **lroot)
{
  short workfile;

  workfile = sfcreate(name, FC_FILE);
  if (*lroot != NULL)
    schreibe_file2(&workfile, *lroot);
  sfclose(&workfile);
}


static void lese_mem_ein(char *start, long size, strmem **lroot,
			 boolean *no_mem, boolean single)
{
  long lz;
  char instr[sortstrsize+1];

  lz = 0;
  while (lz < size && !*no_mem) {
    get_line(start, &lz, size, instr);
    if (*instr != '\0') {
      cut(instr, sortstrsize);
      ordne_ein(instr, lroot, no_mem, single);
    }
  }
}


  /*----------------------------------*/
static void schreibe_mem2(char *start, long *lz, strmem *s)
{
  /*Rekursiver Schreibalgr.           */
  /*----------------------------------*/
  if (s->right != NULL)
    schreibe_mem2(start, lz, s->right);
  put_line(start, lz, s->zeile);
  if (s->left != NULL)
    schreibe_mem2(start, lz, s->left);
  free(s->zeile);
  free(s);
}


static void schreibe_mem(char *start, long *size, strmem **lroot)
{
  long lz;

  lz = 0;
  if (*lroot != NULL)
    schreibe_mem2(start, &lz, *lroot);
  *size = lz;
}


static void sort_it(char *name, char *start, long *size, boolean single)
{
  strmem *lroot;
  boolean no_mem;

  no_mem = false;
  lroot = NULL;

  if (*name != '\0') {
    lese_file_ein(name, &lroot, &no_mem, single);
    if (!no_mem)
      schreibe_file(name, &lroot);
    return;
  }

  if (start == NULL || *size <= 0)
    return;
  lese_mem_ein(start, *size, &lroot, &no_mem, single);
  if (!no_mem)
    schreibe_mem(start, size, &lroot);
}

void gnusort(char *name, char *options1, char *options2)
{
  char s[256], tmp[256];

  strcpy(tmp, name);
  strcat(tmp, ".XXXXXX");
  mymktemp(tmp);
  snprintf(s, 256, "sort %s -o %s %s %s", options1, tmp, options2, name);
  my_system(s);
  sfdelfile(name);
  sfrename(tmp, name);
}

void sort_file(char *name, boolean single)
{
  if (single) gnusort(name, "-u", "");
  else gnusort(name, "", "");
/*  long h;

  sort_it(name, NULL, &h, single);
*/
}


void sort_mem(char *start, long *size, boolean single)
{
  sort_it("", start, size, single);
}
