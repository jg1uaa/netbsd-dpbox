/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/

#include "filesys.h"

#include <unistd.h>
#ifdef __NetBSD__
#include <sys/param.h>
#include <sys/mount.h>
#endif
#ifdef __macos__
#include <time.h>
#include <stat.h>
#include <Files.h>
#else
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#endif
#include <utime.h>

#include "boxlocal.h"
#include "pastrix.h"
#include "boxglobl.h"
#include "box_file.h"
#include "tools.h"
#include "box_logs.h"
#include "box_sub.h"





#ifdef __macos__

/* this is the macintosh localisation for file/folder/path - resolving  */
/* a macintosh does not keep track of files by explicit access paths    */
/* but volume and directory reference id½s, resolved at access time     */
/* lets see if we get that resolved in an efficient way so that unix    */
/* access conventions can be used with not too much overhead in         */
/* execution time                                                       */

static short myVolume;
static long myWorkDir;
static char apName[256];
static short apRefNum;

void StartupFilesysInit(void)
{
  Handle apParam;
  GetAppParms(&apName, &apRefNum, &apParam);
  if (GetVRefNum(apRefNum, &myVolume) < 0)
    myVolume = 0;
}



#define DiskFreeDefault 80000000L

long Diskfree(int dummy)
{
  short retVId;
  long retval;
  char hs[256];
  
  if (GetVInfo(myVolume, &hs, &retVId, &retval) < 0)
    return DiskFreeDefault;
  if (retVId != myVolume)
    return DiskFreeDefault;
  return retval;
}

long DFree(char *mount)
{
  return Diskfree(0) / 1024;
}

static int namecomp(file_str,wild_str)
char *file_str;
char *wild_str;
{
  if (strcmp(file_str,"..") == 0) return 1;
  if (strcmp(file_str,".") == 0) return 1;  
  if (strcmp(wild_str,"*") == 0) return 0;
  if (strcmp(wild_str,"*.*") == 0) return 0;
  if (strchr(wild_str,'*') == NULL) {
    return strcmp(file_str,wild_str);
  }
  if (wildcardcompare(SHORT_MAX, wild_str, file_str, "")) return 0;
  return 1; 
}

void convert_time(time_t time, DTA *dirr)
{
  char h,min,s,d,m,y;
  
  decode_ixtime(time, &d, &m, &y, &h, &min, &s);
  dirr->d_time = Make_DTime(h,min,s);
  dirr->d_date = Make_DDate(d,m,y);
}

static short dir_attribut;
// static struct dirlist *nextptr;

#define RES_NOFILE 2
#define RES_NOMORE 18

short sffirst(char *pfad, short attr, DTA *dirr)
{
#ifndef __macos__
  struct dirlist *actptr;
  struct stat buf;
  char tempstr[255];
  
  dir_attribut = attr;
  if (nextptr != NULL) delete_dirlist();
  nextptr = dir_scan(pfad);
  while (nextptr != NULL) {
    actptr = nextptr;
    strcpy(dirr->d_fname,actptr->filename);
    sprintf(tempstr,"%s%s",dirstr,actptr->filename);
    stat(tempstr,&buf);
    dirr->d_length = buf.st_size;
    nextptr = actptr->next;
    free(actptr);
    switch (buf.st_mode & S_IFMT) {
    case S_IFREG:
      if (dir_attribut == 0) {
        dirr->d_attrib = 0;
        convert_time(buf.st_mtime,dirr);
        return 0;
      }
      break;
    case S_IFDIR:
      if (dir_attribut == 16) {
        dirr->d_attrib = 16;
        convert_time(buf.st_mtime,dirr);
        return 0;
      }
      break;
    default:
      break;
    }
  }
#endif
  return RES_NOFILE;
}

short sfnext(DTA *dirr)
{
#ifndef __macos__
  struct dirlist *actptr;
  struct stat buf;
  char tempstr[255];

  while (nextptr != NULL) {
    actptr = nextptr;
    strcpy(dirr->d_fname,actptr->filename);
    sprintf(tempstr,"%s%s",dirstr,actptr->filename);
    stat(tempstr,&buf);
    dirr->d_length = buf.st_size;
    nextptr = actptr->next;
    free(actptr);
    switch (buf.st_mode & S_IFMT) {
    case S_IFREG:
      if (dir_attribut == 0) {
        dirr->d_attrib = 0;
        convert_time(buf.st_mtime,dirr);
        return 0;
      }
      break;
    case S_IFDIR:
      if (dir_attribut == 16) {
        dirr->d_attrib = 16;
        convert_time(buf.st_mtime,dirr);
        return 0;
      }
      break;
    default:
      break;
    }
  }
#endif
  return RES_NOMORE;
}

int dup(int fd)
{
    return -1;
}

void mktemp(char *name)
{
  validate(name);
}



/* ------------------------------------------------------------------ */


#define alldirsep(c) (c == dirsep || c == '/' || c == '\\' || c == drivesep)

/* C:\TEST\TEST.PAS -> C:\TEST\TEST */

void del_ext(char *s)
{
  short i;

  i = strlen(s);
  while (i > 0 && s[i - 1] != extsep && !alldirsep(s[i - 1]))
    i--;
  if (s[i - 1] == extsep)
    s[i - 1] = '\0';
}


/* C:\TEST\TEST.PAS -> PAS */

void get_ext(char *s, char *sext)
{
  short i;

  sext[0] = '\0';
  i = strlen(s);
  while (i > 0 && s[i - 1] != extsep && !alldirsep(s[i - 1]))
    i--;
  if (s[i - 1] == extsep)
    strsub(sext, s, i + 1, strlen(s) - i);
}


/* C:\TEST\TEST.PAS + TXT -> C:\TEST\TEST.TXT */

void new_ext(char *s, char *ext)
{
  del_ext(s);
  sprintf(s + strlen(s), "%c%s", extsep, ext);
}


/* C:\TEST\TEST.PAS -> TEST.PAS */

void del_path(char *s)
{
  short x;

  x = strlen(s);
  while (x > 0 && !alldirsep(s[x - 1]))
    x--;
  strdelete(s, 1, x);
  cut(s, 80);
}


/* C:\TEST\TEST.PAS -> C:\TEST\ */

void get_path(char *s)
{
  short x;

  x = strlen(s);
  while (x > 0 && !alldirsep(s[x - 1]))
    x--;
  cut(s, x);
}



/* sind wir am Ende der Datei ? */

boolean myeof(short handle)
{
  long apos;

  if (handle < minhandle)
    return true;
  apos = sfseek(0, handle, SFSEEKCUR);
  if (sfseek(1, handle, SFSEEKCUR) > apos) {
    sfseek(-1, handle, SFSEEKCUR);
    return false;
  }
  return true;
}


short sfgetdatime(char *name, unsigned short *date, unsigned short *time)
{
  time_t ixt;

  struct stat buf;
  if (stat(name,&buf) != 0) {
    *date = 0;
    *time = 0;
    return -1;
  }
  ixt = buf.st_mtime;
  ix2dostime(ixt, date, time);
  return 0;
}


short sfsetdatime(char *name, unsigned short *date, unsigned short *time)
{
  short Result;
  time_t ixt;

  struct utimbuf utim;
  Result = -1;
  ixt = dos2ixtime(*date, *time);
  utim.modtime = ixt;
  if (utime ((unsigned char *)name,&utim) >= 0)
    Result = 0;
  return Result;
}


long sfsize(char *name)
{
  struct stat buf;
  if (stat(name,&buf) != 0) {
    return 0;
  }
  return buf.st_size;
}


boolean exist(char *name)
{
  struct stat buf;
  if (stat(name,&buf) != 0) {
    return false;
  }
  return true;
}


short sfrename(char *oldname, char *newname)
{
  if (!rename(oldname, newname))
    return 0;
  return fmv_x(oldname, newname, true, 0, 0, true);
}


void sfdelfile(char *name)
{
  unlink(name);
}


void sfgetdir(short d, char *p)
{
  /* Get Directory */
  /* und zwar als Ergebnis = 'C:\DP\TEST\' */
  /* d 0 = aktuelles Drive */
  *p = '\0';
  getcwd(p, 255);
  if (p[strlen(p) - 1] != dirsep)
    sprintf(p + strlen(p), "%c", dirsep);
}


short sfchdir(char *p)
{
  return chdir(p);
}


short sfmakedir(char *name)
{
  return mkdir(name, 0x1ff);
}


short sfremovedir(char *name)
{
  return rmdir(name);
}



#endif /* of __macos__ */











#if defined(__linux__) || defined(__NetBSD__)

/* simply copied that widespread file access code of former dpbox code  */
/* in this single file. Not a real change to previous versions of the   */
/* code. lot of the code was written by Mark Wahl DL4YBG for the first  */
/* localisation of the dpbox code from Atari to Linux                   */

long Diskfree(int dummy)
{
  return 80000000L;
}

long DFree(char *mount)
{
  struct statfs mystatfs;
  
  statfs(mount, &mystatfs);
  if (mystatfs.f_bsize % 1024 == 0)
    return (mystatfs.f_bsize / 1024) * mystatfs.f_bavail;
  else
    return mystatfs.f_bsize * (mystatfs.f_bavail / 1024);
}


typedef struct dirlist {
  pathstr filename;
  struct dirlist *next;
} dirlist;

static int namecomp(char *file_str, char *wild_str)
{
  if (strcmp(file_str, "..") == 0) return 1;
  if (strcmp(file_str, ".") == 0) return 1;
  if (strcmp(wild_str, "*") == 0) return 0;
  if (strcmp(wild_str, "*.*") == 0) return 0;
  if (strchr(wild_str, '*') == NULL) {
    return strcmp(file_str, wild_str);
  }
  if (wildcardcompare(SHORT_MAX, wild_str, file_str, "")) return 0;
  return 1; 
}

static pathstr filestr;
static pathstr dirstr;

static struct dirlist *dir_scan(char *path)
{
  char *ptr;
  DIR *dirp;
  struct dirent *dp;
  struct dirlist *startptr;
  struct dirlist *actptr;
  struct dirlist *workptr;
  
  ptr = strrchr(path, '/');
  if (ptr == NULL) {
    strcpy(filestr, path);
    strcpy(dirstr, ".");
  }
  else {
    strcpy(filestr, ptr+1);
    nstrcpy(dirstr, path, strlen(path) - strlen(filestr));
  }
  if ((dirp = opendir(dirstr)) == NULL) return NULL;
  startptr = NULL;
  actptr = NULL;
  while ((dp = readdir(dirp)) != NULL) {
    if (namecomp(dp->d_name, filestr) == 0) {
      workptr = malloc(sizeof(struct dirlist));
      nstrcpy(workptr->filename, dp->d_name, LEN_PATH);
      workptr->next = NULL;
      if (startptr == NULL) startptr = workptr;
      if (actptr != NULL) {
        actptr->next = workptr;
      }
      actptr = workptr;
    }
  }
  closedir(dirp);
  return startptr;
}

static short dir_attribut;
static struct dirlist *nextptr;
#define RES_NOFILE 2
#define RES_NOMORE 18

void delete_dirlist()
{
  struct dirlist *actptr;
  
  while (nextptr != NULL) {
    actptr = nextptr;
    nextptr = actptr->next;
    free(actptr);
  }
}

void convert_time(time_t time, DTA *dirr)
{
  short h, min, s, d, m, y;
  
  decode_ixtime(time, &d, &m, &y, &h, &min, &s);
  dirr->d_time = Make_DTime(h, min, s);
  dirr->d_date = Make_DDate(d, m, y);
}

short sffirst(char *pfad, short attr, DTA *dirr)
{
  struct dirlist *actptr;
  struct stat buf;
  pathstr tempstr;
  
  dir_attribut = attr;
  if (nextptr != NULL) delete_dirlist();
  nextptr = dir_scan(pfad);
  while (nextptr != NULL) {
    actptr = nextptr;
    strcpy(dirr->d_fname, actptr->filename);
    sprintf(tempstr, "%s%s", dirstr, actptr->filename);
    stat(tempstr, &buf);
    dirr->d_length = buf.st_size;
    nextptr = actptr->next;
    free(actptr);
    switch (buf.st_mode & S_IFMT) {
    case S_IFREG:
      if (dir_attribut == 0) {
        dirr->d_attrib = 0;
        convert_time(buf.st_mtime, dirr);
        return 0;
      }
      break;
    case S_IFDIR:
      if (dir_attribut == 16) {
        dirr->d_attrib = 16;
        convert_time(buf.st_mtime, dirr);
        return 0;
      }
      break;
    default:
      break;
    }
  }
  return RES_NOFILE;
}

short sfnext(DTA *dirr)
{
  struct dirlist *actptr;
  struct stat buf;
  pathstr tempstr;

  while (nextptr != NULL) {
    actptr = nextptr;
    strcpy(dirr->d_fname, actptr->filename);
    sprintf(tempstr,"%s%s", dirstr, actptr->filename);
    stat(tempstr, &buf);
    dirr->d_length = buf.st_size;
    nextptr = actptr->next;
    free(actptr);
    switch (buf.st_mode & S_IFMT) {
    case S_IFREG:
      if (dir_attribut == 0) {
        dirr->d_attrib = 0;
        convert_time(buf.st_mtime, dirr);
        return 0;
      }
      break;
    case S_IFDIR:
      if (dir_attribut == 16) {
        dirr->d_attrib = 16;
        convert_time(buf.st_mtime, dirr);
        return 0;
      }
      break;
    default:
      break;
    }
  }
  return RES_NOMORE;
}



/* ------------------------------------------------------------------ */

#define alldirsep(c) (c == dirsep || c == '/' || c == '\\' || c == drivesep)

/* C:\TEST\TEST.PAS -> C:\TEST\TEST */

void del_ext(char *s)
{
  short i;

  i = strlen(s);
  while (i > 0 && s[i - 1] != extsep && !alldirsep(s[i - 1]))
    i--;
  if (s[i - 1] == extsep)
    s[i - 1] = '\0';
}

/* C:\TEST\TEST.PAS -> PAS */

void get_ext(char *s, char *sext)
{
  short i;

  sext[0] = '\0';
  i = strlen(s);
  while (i > 0 && s[i - 1] != extsep && !alldirsep(s[i - 1]))
    i--;
  if (s[i - 1] == extsep)
    strsub(sext, s, i + 1, strlen(s) - i);
}

/* C:\TEST\TEST.PAS + TXT -> C:\TEST\TEST.TXT */

void new_ext(char *s, char *ext)
{
  del_ext(s);
  sprintf(s + strlen(s), "%c%s", extsep, ext);
}

/* C:\TEST\TEST.PAS -> TEST.PAS */

void del_path(char *s)
{
  short x;

  x = strlen(s);
  while (x > 0 && !alldirsep(s[x - 1]))
    x--;
  strdelete(s, 1, x);
  cut(s, 80);
}

/* C:\TEST\TEST.PAS -> C:\TEST\ */

void get_path(char *s)
{
  short x;

  x = strlen(s);
  while (x > 0 && !alldirsep(s[x - 1]))
    x--;
  cut(s, x);
}

/* sind wir am Ende der Datei ? */

boolean myeof(short handle)
{
  long apos;

  if (handle < minhandle)
    return true;
  apos = sfseek(0, handle, SFSEEKCUR);
  if (sfseek(1, handle, SFSEEKCUR) > apos) {
    sfseek(-1, handle, SFSEEKCUR);
    return false;
  }
  return true;
}


short sfgetdatime(char *name, unsigned short *date, unsigned short *time)
{
  struct stat buf;

  if (stat(name, &buf) != 0) {
    *date = 0;
    *time = 0;
    return -1;
  }
  ix2dostime(buf.st_mtime, date, time);
  return 0;
}


short sfsetdatime(char *name, unsigned short *date, unsigned short *time)
{
  struct utimbuf utim;

  utim.modtime = dos2ixtime(*date, *time);
  if (utime((unsigned char *)name, &utim) >= 0)
    return 0;
  return -1;
}


long sfsize(char *name)
{
  struct stat buf;
  if (stat(name, &buf) != 0) {
    return 0;
  }
  return buf.st_size;
}


boolean exist(char *name)
{
  struct stat buf;
  if (stat(name, &buf) != 0) {
    return false;
  }
  return true;
}


short sfrename(char *oldname, char *newname)
{
  if (!rename(oldname, newname))
    return 0;
  return fmv_x(oldname, newname, true, 0, 0, true);
}


void sfdelfile(char *name)
{
  unlink(name);
}

/* ----------------------------------------------------------------- */


void sfgetdir(short d, char *p)
{
  /* Get Directory */
  /* und zwar als Ergebnis = '/DP/TEST/' */
  /* d 0 = aktuelles Drive */
  *p = '\0';
  getcwd(p, 255);
  if (p[strlen(p) - 1] != dirsep)
    sprintf(p + strlen(p), "%c", dirsep);
}


short sfchdir(char *p)
{
  return chdir(p);
}


short sfmakedir(char *name)
{
  return mkdir(name, 0x1ff);
}


short sfremovedir(char *name)
{
  return rmdir(name);
}


#endif /* of __linux__ */








/* put further localisations in here                                    */















/* These are common routines of all localisations */


void del_dir(char *name)
{
  short result;
  DTA dirinfo;
  pathstr STR1;

  result = sffirst(name, 0, &dirinfo);
  get_path(name);
  while (result == 0) {
    sprintf(STR1, "%s%s", name, dirinfo.d_fname);
    sfdelfile(STR1);
    result = sfnext(&dirinfo);
  }
}



#define bsize           8192


/* haengt ein File an ein bereits geoeffnetes an */

void app_file2(char *filea, short k2, long ab, boolean del_source)
{
  char *puffer;
  long psize;
  long done, bc, cstep, hli, fsize;
  short k1;

  if (k2 < minhandle)
    return;
  k1 = sfopen(filea, FO_READ);
  if (k1 < minhandle)
    return;
  fsize = sfseek(0, k1, SFSEEKEND);
  sfseek(0, k1, SFSEEKSET);
  if (ab > 0 && ab <= fsize)
    fsize -= ab;
  else
    ab = 0;
  done = 0;
  hli = 0;
  psize = bsize;
  if (psize > fsize)
    psize = fsize;
  puffer = malloc(psize);
  if (puffer != NULL) {
    if (ab > 0)
      sfseek(ab, k1, SFSEEKSET);
    bc = 1;
    while (done < fsize && bc > 0) {
      cstep = fsize - done;
      if (cstep > psize)
	cstep = psize;
      bc = sfread(k1, cstep, puffer);
      if (bc <= 0)
	break;
      done += bc;
      bc = sfwrite(k2, cstep, puffer);
      if (bc > 0)
	hli += bc;
    }
    free(puffer);
  }
  sfclose(&k1);
  if (hli == fsize && done == fsize && del_source)
    sfdelfile(filea);
}

#undef bsize


#define copysize        8192
#define copysize2       128


short fmv_x(char *filea, char *fileb, boolean delete_source,
		       long start, long size, boolean was_rename)
{
  long err;
  char *puffer;
  long psize;
  long done, bc, cstep, hli, fsize;
  short k1, k2, dtime, ddate;
  boolean save_date;


  if (!was_rename && delete_source && size == 0 && start == 0) {
    sfdelfile(fileb);
    return (sfrename(filea, fileb));
  }
  
  if (size == 0 && start == 0) {
    save_date = true;
    sfgetdatime(filea, &ddate, &dtime);
  } else
    save_date = false;

  k1 = sfopen(filea, FO_READ);
  if (k1 < minhandle)
    return -1;

  fsize = sfseek(0, k1, SFSEEKEND);
  sfseek(0, k1, SFSEEKSET);

  if (fsize <= 0) {
    sfclose(&k1);
    return -1;
  }

  if (size > 0) {
    if (fsize > size - start)
      fsize = size - start;
  } else if (start > 0) {
    if (start <= fsize)
      fsize -= start;
  }

  if (fsize <= 0) {
    sfclose(&k1);
    return -1;
  }

  psize = copysize;
  if (psize > fsize)
    psize = fsize;
  puffer = malloc(psize);

  if (puffer == NULL) {
    psize = copysize2;
    if (psize > fsize)
      psize = fsize;
    puffer = malloc(psize);
  }

  if (puffer == NULL) {
    sfclose(&k1);
    return -1;
  }
  done = 0;
  hli = 0;
  if (start > 0)
    err = sfseek(start, k1, SFSEEKSET);
  else
    err = 0;
  if (err == start) {
    k2 = sfcreate(fileb, FC_FILE);
    if (k2 >= minhandle) {
      bc = 1;
      while (done < fsize && bc > 0) {
	cstep = fsize - done;
	if (cstep > psize)
	  cstep = psize;
	bc = sfread(k1, cstep, puffer);
	if (bc <= 0)
	  break;
	done += bc;
	bc = sfwrite(k2, cstep, puffer);
	if (bc > 0)
	  hli += bc;
      }
      sfclose(&k2);
    }
  }
  sfclose(&k1);
  if (hli == fsize && done == fsize) {
    if (delete_source)
      sfdelfile(filea);
  } else
    sfdelfile(fileb);
  free(puffer);
  if (save_date)
    sfsetdatime(fileb, &ddate, &dtime);
  return 0;
}

#undef copysize
#undef copysize2


/* validate sucht solange eine neue .extension, bis sichergestellt ist, */
/* dass keine bestehende Datei ueberschrieben wird */

void validate(char *name)
{
  char ext[40];
  short extnum;
  pathstr try;

  if (!exist(name))
    return;
  get_ext(name, ext);
  del_ext(name);
  extnum = atoi(ext) + 1;
  while (exist((sprintf(try, "%s%c%.3d", name, extsep, extnum++), try)));
  strcpy(name, try);
}


/*fuegt eine Zeile an eine bereits geoeffnete Datei an*/

void str2file(short *handle, const char *line, boolean crlf)
{
  static char crlfarr = 10;
  long l;

  if (*handle < minhandle)
    return;
  if ((l = strlen(line)) > 0) {
    if (sfwrite(*handle, l, line) != l) {
      sfclose(handle);
      return;
    }
  }
  if (crlf) {
    if (sfwrite(*handle, sizeof(crlfarr), &crlfarr) != sizeof(crlfarr))
      sfclose(handle);
  }
}


/* funktioniert nur korrekt, wenn Eingabezeilen maximal maxlen-2 Bytes lang sind */


boolean file2lstr2(short handle, char *line, long maxlen, boolean *eol)
{
  long x, ct, s2, l;

  *line = '\0';

  *eol = false;
  if (handle < minhandle)
    return false;
  ct = sfread(handle, maxlen, line);
  if (ct <= 0)
    return false;
  line[ct] = '\0';
  l = ct;
  for (x = 1; x <= l; x++) {
    if (line[x - 1] == '\015' || line[x - 1] == '\n') {
      *eol = true;
      if (line[x - 1] == '\015') {
	if (line[x] == '\n')
	  s2 = x + 1;
	else
	  s2 = x;
      } else
	s2 = x;
      ct -= s2;
      if (ct > 0) {
	ct = -ct;
	sfseek(ct, handle, SFSEEKCUR);
      }
      line[x - 1] = '\0';
      return true;
    }
  }
  return true;
}

boolean file2lstr(short handle, char *line, long maxlen)
{
  boolean eol;
 
  return file2lstr2(handle, line, maxlen, &eol);
}

/* Fuegt eine Zeile an eine Datei an, gibt neue Laenge zurueck */

long append(char *name, char *zeile, boolean crlf)
{
  short k;
  long sz;

  sz = -1;
  k = sfopen(name, FO_RW);
  if (k < minhandle)
    k = sfcreate(name, FC_FILE);
  if (k < minhandle)
    return sz;
  sz = sfseek(0, k, SFSEEKEND) + strlen(zeile);
  if (crlf)
    sz++;
  str2file(&k, zeile, crlf);
  sfclose(&k);
  return sz;
}


/* ----------------------------------------------------------------- */


/* File oeffnen / schliessen im Multitasking */

typedef struct flocktype {
  struct flocktype *next;
  char cs;
  short handle;
  boolean write;
  time_t attime;
  pathstr name;  
} flocktype;


static flocktype *flockroot;


void handle2name(short handle, char *name)
{
  flocktype *hp;

  hp = flockroot;
  while (hp != NULL && hp->handle != handle)
    hp = hp->next;
  if (hp != NULL)
    strcpy(name, hp->name);
  else
    *name = '\0';
}


static void flockp2s(flocktype *hp, char *hs)
{
  time_t t2;
  char c;

  if (hp == NULL) {
    strcpy(hs, "???");
    return;
  }

  t2 = (statclock() - hp->attime) / TICKSPERSEC;

  if (hp->write)
    c = 'w';
  else
    c = 'r';

  sprintf(hs, "%3d %c %.2ld:%.2ld %s", hp->handle, c, t2 / 60, t2 % 60, hp->name);
}


static void flockprotocole(short i, flocktype *hp, char *name)
{
  static char crlfarr = 10;

  pathstr hs, w, STR1;
  short k;

  if (hp != NULL || i <= 2)
    return;

  switch (i) {

  case 0:
  case 3:
    strcpy(w, "open  ");
    break;

  case 1:
  case 4:
    strcpy(w, "create");
    break;

  case 2:
  case 5:
    strcpy(w, "close ");
    break;

  default:
    strcpy(w, "???   ");
    break;
  }

  if (hp != NULL)
    flockp2s(hp, hs);
  else
    sprintf(hs, "%s failed!", name);

  sprintf(hs, "%s %s %s: %s", clock_.datum4, clock_.zeit, w, strcpy(STR1, hs));
  if (lastproc != 0) {
    get_debug_func(lastproc, STR1);
    strcat(hs, " - possibly called by ");
    strcat(hs, STR1);
  }

  sprintf(w, "%sfile_err%ctxt", boxprotodir, extsep);
  k = open(w, FO_RW);
  if (k < minhandle)
    k = dpsyscreate(w, FO_CREATE, FC_FILE);
  if (k < minhandle)
    return;
  lseek(k, 0, 2);
  write(k, hs, strlen(hs));
  write(k, &crlfarr, sizeof(char));
  close(k);
}

short dpsyscreate(char *fname, int flags, int mode)
{
#ifdef __macos__
   return open(fname, flags);
#else
   return open(fname, flags, mode);
#endif
}


short open_locked(boolean create, char *name, short mode)
{
  short Result, handle;
  flocktype *hp;
  boolean nowrite, wantwrite;
  char tcs;

  Result = nohandle;
  tcs = calccs(name);
  handle = nohandle;
  wantwrite = (create || mode != FO_READ);
  hp = flockroot;
  nowrite = true;
  /* ist das File bereits in Benutzung? */

  while (hp != NULL && nowrite) {
    if (hp->cs == tcs) {
      if (!strcmp(hp->name, name)) {
	if (wantwrite)
	  nowrite = false;
	if (hp->write)
	  nowrite = false;
      }
    }
    hp = hp->next;
  }

  hp = NULL;
  if (nowrite) {  /* ok, wir koennen zugreifen */
    if (create)
      handle = dpsyscreate(name, FO_CREATE, mode);
    else
      handle = open(name, mode);

    if (handle < minhandle)  /* File geoeffnet */
      return handle;

    hp = malloc(sizeof(flocktype));
    if (hp != NULL) {
      hp->next = flockroot;
      hp->cs = tcs;
      strcpy(hp->name, name);
      hp->handle = handle;
      hp->write = wantwrite;
      hp->attime = statclock();
      flockroot = hp;
      if (create)
	flockprotocole(1, hp, name);
      else
	flockprotocole(0, hp, name);
      return handle;
    }
    close(handle);
    if (create)
      sfdelfile(name);
    handle = nohandle;
    if (create)
      flockprotocole(1, hp, name);
    else
      flockprotocole(0, hp, name);
    return handle;
  }

  if (create)
    flockprotocole(4, hp, name);
  else
    flockprotocole(3, hp, name);
  return handle;

  /* kein Verwaltungsspeicher - dann koennen wir auch nicht oeffnen */
}


static short lastactivehandle;
static flocktype *lastactiveflock;

static flocktype *valid_handle(short handle)
{
  flocktype *hp;

  if (handle == lastactivehandle && lastactiveflock != NULL)
    return lastactiveflock;

  hp = flockroot;
  while (hp != NULL && hp->handle != handle)
    hp = hp->next;

  if (hp != NULL) {
    lastactivehandle = handle;
    lastactiveflock = hp;
    return hp;
  } else
    return NULL;
}


long sfseek(long count, short handle, short mode)
{ 
  if (valid_handle(handle) != NULL)
    return lseek(handle, count, mode);
  else
    return -1;
}


long sfread(short handle, long count, char *buf)
{
  if (valid_handle(handle) != NULL)
    return read(handle, buf, count);
  else
    return -1;
}


long sfwrite(short handle, long count, const char *buf)
{
  if (valid_handle(handle) != NULL)
    return write(handle, buf, count);
  else
    return -1;
}

void sfclose_x(short *handle, boolean delete_it)
{
  flocktype *hp, *hpa;
  pathstr dname, w;

  hp = valid_handle(*handle);
  if (hp != NULL) {
    lastactivehandle = nohandle;
    lastactiveflock = NULL;

    hp = flockroot;
    hpa = NULL;
    while (hp != NULL && hp->handle != *handle) {
      hpa = hp;
      hp = hp->next;
    }

    *dname = '\0';
    *w = '\0';
    if (hp != NULL) {
      strcpy(dname, hp->name);
      flockprotocole(2, hp, w);
      if (hpa != NULL) {
	hpa->next = hp->next;
	free(hp);
      } else {
	hpa = hp->next;
	free(hp);
	flockroot = hpa;
      }
    } else
      flockprotocole(5, hp, w);
    close(*handle);
    if (delete_it && *dname != '\0')
      sfdelfile(dname);
  }

  *handle = nohandle;
}


void sfdispfilelist(short x, dispfilelistproc outproc)
{
  char hs[256];
  flocktype *hp;

  hp = flockroot;
  while (hp != NULL) {
    flockp2s(hp, hs);
    outproc(x, hs);
    hp = hp->next;
  }
}


void chkopenfiles(time_t maxopen, char *fn)
{
  /* maxopen in sekunden */
  flocktype *hp;
  time_t t2, ta;
  short handle;
  char hs[256];

  t2 = maxopen * TICKSPERSEC;
  ta = statclock();
  hp = flockroot;
  while (hp != NULL) {
    if (hp->attime + t2 >= ta) {  /* Zeit abgelaufen ... */
      hp = hp->next;
      continue;
    }
    /* protokoll schreiben ... */
    sprintf(hs, "%s timed out", hp->name);
    append(fn, hs, true);
    handle = hp->handle;
    hp = hp->next;
    sfclose(&handle);
  }
}

boolean tas_lockfile(long waittime, long oldtime, char *name)
{
  time_t ixt, time1;
  struct stat buf;

  time1 = time(NULL);
  ixt 	= time1;

  while (true) {
    if (stat(name, &buf) == 0) {
      if (time1 - buf.st_mtime >= oldtime) break;
      else {
      	if (ixt - time1 >= waittime) return(false);
	sleep(1);
      }
      ixt = time(NULL);
    }
    else break;
  }

  sfdelfile(name);
  close( dpsyscreate(name, FO_CREATE, FC_FILE_RALL) );

  return(true);
}

/* creates a directory including all parent directories */
boolean create_dirpath(char *dirpath)
{
  boolean result;
  char	  *p;
  pathstr curdir, hs;

  if (*dirpath == '\0') return false;

  if (sfmakedir(dirpath) == 0) return true;
  
  result = false;
  strcpy(hs, dirpath);
  if (*hs == '/') strcpy(curdir, "/");
  else sfgetdir(0, curdir);

  p = strtok(hs, "/"); 
  while (p != NULL) {
    if (*p != '\0') {
      strcat(curdir, p);
      result = sfmakedir(curdir) == 0;
      strcat(curdir, "/");
    }
    p = strtok(NULL, "/");
  }
  return result;
}

static void usintasc(short digits, unsigned int num, char *asc)
{
  char	c1;
  short x;
  unsigned int lb, nib;

  lb  = num;
  for (x = 1; x <= digits; x++) {
    nib   = lb % (10+26+26);
    lb    /= (10+26+26);
    if (nib < 10)
      c1  = (char)(nib + '0');
    else if (nib < 10+26)
      c1  = (char)(nib - 10 + 'A');
    else if (nib < 10+26+26)
      c1  = (char)(nib - (10+26) + 'a');
    else
      c1  = '-';
    asc[digits - x] = c1;
  }
  asc[digits]  = '\0';
}

/* create a unique filename. This function overcomes some limitations of  */
/* current implementations of mktemp()	      	      	      	      	  */
/* this function requires a case sensitive file system	      	      	  */
/* it offers 238328 ((26+26+10)^3) different filenames per pid       	  */

boolean mymktemp(char *name)
{
  static unsigned int tempcount = 0;
  static pid_t mypid = 0;
  short x;
  unsigned int count = 0;
  char *p, *s;
  pathstr hs, ts, pids;
  
  strcpy(hs, name);
  p = strstr(hs, "XXXXXX");
  if (p == NULL) return false;
  if (mypid == 0) mypid = getpid();
  usintasc(3, mypid, pids);
  do {
    if (++count > 62*62*62) return false;
    if (++tempcount >= 62*62*62) tempcount = 0;
    usintasc(3, tempcount, ts);
    s = p;
    for (x = 0; x < 3; x++) *s++ = pids[x];
    for (x = 0; x < 3; x++) *s++ = ts[x];
  } while (exist(hs));
  strcpy(name, hs);
  return true;
}

char *mytmpnam(char *name)
{
  strcpy(name, tempdir);
  strcat(name, "dpXXXXXX");
  if (mymktemp(name))
    return name;
  else
    return NULL;
}

void _filesys_init(void)
{
  static int _was_initialized = 0;
  if (_was_initialized++)
    return;

  lastactivehandle = nohandle;
  lastactiveflock = NULL;
  flockroot = NULL;
#ifdef __macos__
  StartupFilesysInit();
#endif
#if defined(__linux__) || defined(__NetBSD__)
  nextptr = NULL;
#endif
}
