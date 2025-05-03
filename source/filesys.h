
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
  int32_t d_length;
  int d_lastindex;
  int d_volumeID;
  int d_dirID;
  char d_fname[256];
} DTA;

typedef DTA mysearchrec;

int dup(int fd);
void mktemp(char *name);

#endif /* of __macos__ */










#if defined(__linux__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)

/* simply copied that widespread file access code of former dpbox code  */
/* in this single file. Not a real change to previous versions of the   */
/* code. lot of the code was written by Mark Wahl DL4YBG for the first  */
/* localisation of the dpbox code from Atari to Linux                   */

#include <sys/stat.h>
#if defined(__linux__)
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
  int32_t d_length;
  char d_fname[256];
} DTA;

typedef DTA mysearchrec;

/* only for linux version. called at programs end */
void delete_dirlist();

#endif /* of __linux__ */

/* put further localisations in here                                    */








typedef void (*dispfilelistproc)(const short x, const char *s);



/* these ones are coded different */

int32_t Diskfree(int dummy);
int32_t DFree(char *mount);
bool exist(char *name);
int32_t sfsize(char *name);
short sfrename(char *oldname, char *newname);
short sfgetdatime(char *name, unsigned short *date,
			 unsigned short *time);
short sfsetdatime(char *name, unsigned short *date,
			 unsigned short *time);
short sffirst(char *pfad, short attr, DTA *dirr);
short sfnext(DTA *dirr);
void sfgetdir(short d, char *p);
short sfchdir(char *p);
short sfmakedir(char *name);
short sfremovedir(char *name);
void del_ext(char *s);
void get_ext(char *s, char *sext);
void new_ext(char *s, char *ext);
void get_path(char *s);
void del_path(char *s);
void del_blanks(char *s);

/* these ones are coded common */

void del_dir(char *name);
void app_file2(char *filea, short k2, int32_t ab, bool del_source);
void validate(char *name);
void str2file(short *handle, const char *line, bool crlf);
bool file2lstr2(short handle, char *line, int32_t maxlen, bool *eol);
bool file2lstr(short handle, char *line, int32_t maxlen);
int32_t append(char *name, char *zeile, bool crlf);
void handle2name(short handle, char *name);
short dpsyscreate(char *fname, int flags, int mode);
short open_locked(bool create, char *name, short mode);
int32_t sfseek(int32_t count, short handle, short mode);
int32_t sfread(short handle, int32_t count, char *buf);
int32_t sfwrite(short handle, int32_t count, const char *buf);
void sfclose_x(short *handle, bool delete_it);
void sfdelfile(char *name);
void sfdispfilelist(short x, dispfilelistproc outproc);
void chkopenfiles(time_t maxopen, char *fn); /* maxopen in sekunden */
short fmv_x(char *filea, char *fileb, bool delete_source,
		       int32_t start, int32_t size, bool was_rename);
bool tas_lockfile(time_t waittime, time_t oldtime, char *name);
bool create_dirpath(char *dirpath);
bool mymktemp(char *name);
char *mytmpnam(char *name);

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
