/************************************************************************/
/* Simple merger for mybbs.box                                          */
/* Merges the content of a couple of mybbs.box files into one output    */
/* file, and takes only the newest entries. This tool could be useful   */
/* if you have a new installation running and want to update the mybbs  */
/* data with the mybbs file(s) of another bbs.                          */
/*                                                                      */
/* created: Joachim Schurig, 28.09.1999                                 */
/* modified:                                                            */
/************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#ifndef true
  #define true 1
  #define false 0
#endif

#define LEN_BID		12
#define LEN_MBX		40
#define LEN_CALL	6
#define LEN_BOARD	8
#define LEN_CHECKSUBJECT 40
#define LEN_SUBJECT	80

typedef char		msgidmem[LEN_BID+1];
typedef msgidmem	bidtype;
typedef char		mbxtype[LEN_MBX+1];
typedef char		calltype[LEN_CALL+1];
typedef char		boardtype[LEN_BOARD+1];
typedef char		checksubjecttype[LEN_CHECKSUBJECT+1];
typedef char		subjecttype[LEN_SUBJECT+1];

typedef struct mybbstyp {
  calltype		call;
  calltype 		bbs;	 /* MyBBS				*/
  char			mode;	 /* starting with v5.03: guessed entry?	*/
  time_t		ix_time; /* changed at				*/
} mybbstyp;



static long sfsize(char *name)
{ 
  struct stat buf;
  
  if (stat(name, &buf) != 0) return -1;
  return buf.st_size;
}

static int merge(char *outfile, char *name, long recs, int quiet)
{
  FILE *outhandle, *inhandle;
  int write_mode; /* 0 = append, 1 = overwrite, 2 = don't write */
  long oldsize, oldrecs;
  long ct, oldct;
  long outct = 0;
  mybbstyp mybbs, mybbs_o;

  oldsize = sfsize(outfile);
  if (oldsize <= 0) {
    oldsize = 0;
    oldrecs = 0;
    if ((outhandle = fopen(outfile, "w+b")) == NULL) return (-1);  
  } else {
    if (oldsize % sizeof(mybbstyp) != 0) return (-1);
    oldrecs = oldsize / sizeof(mybbstyp);
    if ((outhandle = fopen(outfile, "r+b")) == NULL) return (-1);
  }
  
  if ((inhandle = fopen(name, "rb")) < 0) {
    fclose(outhandle); return (-1);
  }
  
  if (!quiet) fprintf(stderr, " %6d of %6ld", 0, recs);
  for (ct = 0; ct < recs; ct++) {
    if (!quiet) {
      if (ct % 10 == 0) {
      	fprintf(stderr, "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%6ld of %6ld", ct, recs);
      }
    }
    write_mode = 0;
    if (fread(&mybbs, sizeof(mybbstyp), 1, inhandle) != 1) {
      fclose(inhandle); fclose(outhandle); return (-1);
    }
    
    if (oldrecs > 0) {
      fseek(outhandle, 0, SEEK_SET);
    
      for (oldct = 0; oldct < oldrecs; oldct++) {
        if (fread(&mybbs_o, sizeof(mybbstyp), 1, outhandle) != 1) {
          fclose(inhandle); fclose(outhandle); return (-1);	
        }

	if (!strcmp(mybbs_o.call, mybbs.call)) {
	  if (mybbs_o.mode == mybbs.mode) {
	    if (mybbs_o.ix_time >= mybbs.ix_time) write_mode = 2;
	    else write_mode = 1;
	  } else if (mybbs_o.mode == 'G' && mybbs.mode == 'U') {
	    write_mode = 1;
	  }
	  
	  if (write_mode == 1) fseek(outhandle, oldct * sizeof(mybbstyp), SEEK_SET);
	  break;
	}
	
      }

      if (write_mode == 0) fseek(outhandle, 0, SEEK_END);
    }

    if (write_mode < 2) {
      outct++;
      if (fwrite(&mybbs, sizeof(mybbstyp), 1, outhandle) != 1) {
        fclose(inhandle); fclose(outhandle); return (-1);
      }
    }

  }
  fclose(inhandle); fclose(outhandle); return (outct);
}



static void show_help(void)
{
  fprintf(stderr, "usage: dpmybbs [-q] <mybbsfile1> <mybbsfile2> [mybbsfile3..n] -o <mybbsfile.out>\n");
  fprintf(stderr, "                -q = do not display counter while adding records\n\n");
}

int main(int argc, char *argv[])
{
  int op_ok = false;
  int files = 0;
  int ct = 0;
  int firstarg = 0;
  int quiet = false;
  long size, recs, recs2, tsize, trecs;
  char outfile[256], hs[256];
  
  fprintf(stderr, "\nDPBOX mybbs merge utility - v0.1 28.09.1999 J.Schurig, DL8HBS\n\n");

  *outfile = '\0';
    
  if (argc > 2) {
    if (!strcmp("-q", argv[1])) {
      quiet = true;
      firstarg = 1;
      ct++;
    }
    while (ct < argc && !op_ok) {
      if (!strcmp("-o", argv[ct])) {
      	if (ct+2 == argc) {
	  strcpy(outfile, argv[ct+1]);
	  op_ok = true;
	  files = ct-firstarg;
	}
      }
      ct++;
    }
  }
  
  if (op_ok) {
    tsize = 0;
    trecs = 0;
    fprintf(stderr, "Size      Records Name\n");
    fprintf(stderr, "------------------------------------------\n");
    for (ct = 1; ct < files; ct++) {
      strcpy(hs, argv[ct+firstarg]);
      size  = sfsize(hs);
      if (size > 0) recs = size / sizeof(mybbstyp);
      else recs = -1;
      fprintf(stderr, "%8ld %8ld %s\n", size, recs, hs);
      if (recs <= 0 || size % sizeof(mybbstyp) != 0) op_ok = false;
      if (op_ok) {
      	tsize += size;
	trecs += recs;
      }
    }
    fprintf(stderr, "------------------------------------------\n");
    fprintf(stderr, "%8ld %8ld\n", tsize, trecs);
  }
  
  if (op_ok) {
   trecs = 0;
   fprintf(stderr, "\nnow merging into output file: %s\n", outfile);
   for (ct = 1; ct < files; ct++) {
      strcpy(hs, argv[ct+firstarg]);
      size = sfsize(hs);
      recs = size / sizeof(mybbstyp);
      fprintf(stderr, "\nFile: %s", hs);
      if ((recs2 = merge(outfile, hs, recs, quiet)) < 0) {
      	fprintf(stderr, "\nfile error\n");
	return 1;
      }
      fprintf(stderr, " - added %6ld of %6ld records", recs2, recs);
      trecs += recs2;     
    }
    fprintf(stderr, "\n%ld total records\n", trecs);
  }
  
  if (!op_ok) show_help();
  return(0);
}
