
/* file system access of dpbox */


#ifndef FILESYS_H
#define FILESYS_H

#include <fcntl.h>
#include "pastrix.h"




#ifdef __macos__
#include <stat.h>

/* this is the macintosh localisation for file/folder/path - resolving  */
/* a macintosh does not keep track of files by explicit access paths    */
/* but volume and directory reference IDs, resolved at access time     */
/* lets see if we get that resolved in an efficient way so that unix    */
/* access conventions can be used with not too much overhead in         */
/* execution time                                                       */

#define allquant        '*'   /* *.*                           */
#define singlequant     '?'   /* *.HL?                         */
#define extsep          '.'   /* FILE.EXT                      */
#define drivesep        ':'   /* D:\...                        */
#define dirsep		':'
#define minhandle       1   /* kleinste gueltige Dateihandlenummer */
#define maxhandle       32767
#define nohandle        (-1)   /* ungueltiges Handle                 */

#define SFSEEKSET       SEEK_SET
#define SFSEEKCUR       SEEK_CUR
#define SFSEEKEND       SEEK_END

#define FO_READ O_RDONLY
#define FO_WRITE O_WRONLY
#define FO_RW O_RDWR
#define FO_CREATE O_CREAT|O_RDWR|O_TRUNC

#define FC_FILE         0600   /* PARAMETER FUER FCREATE (NORMAL FILE) */
#define FC_FILE_RALL   	0644
#define FC_FILE_RWGROUP	0660
#define FC_FILE_RWALL 	0666

typedef struct DTA {
  char d_attrib;
  int d_time;
  int d_date;
  long d_length;
  int d_lastindex;
  int d_volumeID;
  int d_dirID;
  char d_fname[256];
} DTA;

typedef DTA mysearchrec;

extern int dup(int fd);
extern void mktemp(char *name);

#endif /* of __macos__ */










#if defined(__linux__) || defined(__NetBSD__)

/* simply copied that widespread file access code of former dpbox code  */
/* in this single file. Not a real change to previous versions of the   */
/* code. lot of the code was written by Mark Wahl DL4YBG for the first  */
/* localisation of the dpbox code from Atari to Linux                   */

#include <sys/stat.h>
#ifndef __NetBSD__
#include <sys/vfs.h>
#endif

#define allquant        '*'   /* *.*                           */
#define singlequant     '?'   /* *.HL?                         */
#define extsep          '.'   /* FILE.EXT                      */
#define drivesep        ':'   /* D:\...                        */
#define dirsep          '/'
#define minhandle       0   /* kleinste gueltige Dateihandlenummer */
#define maxhandle       255
#define nohandle        (-1)   /* ungueltiges Handle                 */

#define SFSEEKSET       SEEK_SET
#define SFSEEKCUR       SEEK_CUR
#define SFSEEKEND       SEEK_END

#define FO_READ O_RDONLY
#define FO_WRITE O_WRONLY
#define FO_RW O_RDWR
#define FO_CREATE O_CREAT|O_RDWR|O_TRUNC

#define FC_FILE         S_IRUSR|S_IWUSR   /* PARAMETER FOR FCREATE (NORMAL FILE) */
#define FC_FILE_RALL   	S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
#define FC_FILE_RWGROUP S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP
#define FC_FILE_RWALL   S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH

typedef struct DTA {
  char d_attrib;
  int d_time;
  int d_date;
  long d_length;
  char d_fname[256];
} DTA;

typedef DTA mysearchrec;

/* only for linux version. called at programs end */
extern void delete_dirlist();

#endif /* of __linux__ */

/* put further localisations in here                                    */








typedef void (*dispfilelistproc)(const short x, const char *s);



/* these ones are coded different */

extern long Diskfree(int dummy);
extern long DFree(char *mount);
extern boolean exist(char *name);
extern long sfsize(char *name);
extern short sfrename(char *oldname, char *newname);
extern short sfgetdatime(char *name, unsigned short *date,
			 unsigned short *time);
extern short sfsetdatime(char *name, unsigned short *date,
			 unsigned short *time);
extern short sffirst(char *pfad, short attr, DTA *dirr);
extern short sfnext(DTA *dirr);
extern void sfgetdir(short d, char *p);
extern short sfchdir(char *p);
extern short sfmakedir(char *name);
extern short sfremovedir(char *name);
extern void del_ext(char *s);
extern void get_ext(char *s, char *sext);
extern void new_ext(char *s, char *ext);
extern void get_path(char *s);
extern void del_path(char *s);
extern void del_blanks(char *s);
extern boolean myeof(short handle);

/* these ones are coded common */

extern void del_dir(char *name);
extern void app_file2(char *filea, short k2, long ab, boolean del_source);
extern void validate(char *name);
extern void str2file(short *handle, const char *line, boolean crlf);
extern boolean file2lstr2(short handle, char *line, long maxlen, boolean *eol);
extern boolean file2lstr(short handle, char *line, long maxlen);
extern long append(char *name, char *zeile, boolean crlf);
extern void handle2name(short handle, char *name);
extern short dpsyscreate(char *fname, int flags, int mode);
extern short open_locked(boolean create, char *name, short mode);
extern long sfseek(long count, short handle, short mode);
extern long sfread(short handle, long count, char *buf);
extern long sfwrite(short handle, long count, const char *buf);
extern void sfclose_x(short *handle, boolean delete_it);
extern void sfdelfile(char *name);
extern void sfdispfilelist(short x, dispfilelistproc outproc);
extern void chkopenfiles(long maxopen, char *fn); /* maxopen in sekunden */
extern short fmv_x(char *filea, char *fileb, boolean delete_source,
		       long start, long size, boolean was_rename);
extern boolean tas_lockfile(long waittime, long oldtime, char *name);
extern boolean create_dirpath(char *dirpath);
extern boolean mymktemp(char *name);
extern char *mytmpnam(char *name);

#define app_file(filea, k2, del_source) app_file2(filea, k2, 0, del_source)
#define drv2num(c) 0
#define file2str(handle, line) file2lstr(handle, line, 255)
#define filecut(filea, fileb, start, size) fmv_x(filea, fileb, true, start, size, false)
#define filecut_nodel(filea, fileb, start, size) fmv_x(filea, fileb, false, start, size, false)
#define filecopy(filea, fileb) fmv_x(filea, fileb, false, 0, 0, false)
#define filemove(filea, fileb) fmv_x(filea, fileb, true, 0, 0, false)
#define sfcreate(name, mode) open_locked(true, name, mode)
#define sfopen(name, mode) open_locked(false, name, mode)
#define sfclose(handle) sfclose_x(handle, false)
#define sfclosedel(handle) sfclose_x(handle, true)
#define idxfname(fname, board) sprintf(fname, "%s%s%c%s", indexdir, board, extsep, EXT_IDX)
#define inffname(fname, board) sprintf(fname, "%s%s%c%s", infodir, board, extsep, EXT_INF)

#endif
