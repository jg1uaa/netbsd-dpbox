/*********************************************************/
/* Plain text search					 */
/* Copyright 1997 Joachim Schurig, DL8HBS		 */
/* created: 97/03/18 Joachim Schurig			 */
/* modified: 00/01/12 Joachim Schurig			 */
/*                                                       */
/* This file is under the GPL.				 */
/*********************************************************/

#define vnum "0.2"

/* undefine macos for unix */
#undef macos
/* create a list of all known words? */
#undef wordlist
/* use an own file buffer for encoding? */
#undef setbuf
/* add newly detected "stopwords"? */
#define addstopwords
/* use compressed header format? */
#define compress
/* skip input file for encoding until first empty line? */
#define headerskip
/* create debugging output */
#undef debug

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef macos
#include <console.h>
#include <stat.h>
#else
#include <sys/stat.h>
#include <sys/wait.h>
#endif

#ifdef macos
  #define BENDIAN
#else
  #undef BENDIAN
#endif

#define minwordlen 2
#define maxwordlen 20
#define maxnumwordlen 5
#define maxvalidwordlen 40
#define mindocthreshold 2000
#define maxdocpercentage 17
#define minfoundforsubstitution 2
#define maxsubstitutiondiff 3
#define maxlinesize 256
#ifdef setbuf
#define streambufsize 4096
#endif

typedef struct dictentry {
	unsigned char thischar;
	unsigned char stopword;
	unsigned long doccount;
	long matchseek;
	long unmatchseek;
	long liststart;
} dictentry;

typedef struct messages {
	unsigned long msgnum;
	long next;
} messages;

#ifdef compress

typedef unsigned char diskentry[2 + 3*4];
typedef unsigned char diskmsg[3*2];

#define dictentrysize sizeof(diskentry)
#define messageentrysize sizeof(diskmsg)
#define nilp 256*256*256 - 1

#else

#define dictentrysize sizeof(dictentry)
#define messageentrysize sizeof(messages)
#define nilp 256*256*256*256 - 1

#endif

static char dbasedir[256];
static int totaldocs = 0;
static int lastscannedmsg = 0;

#ifdef debug
static void debug(char *s)
{
	FILE *deb;
	
	if ((deb = fopen("debug.crawl", "a+")) == NULL) return;
	fprintf(deb, "%s\n", s);
	fclose(deb);
}
#endif

#define maxrunargs 50
static void my_system(char *s)
{
	int wpid;
	int handle;
	int ct;
	int li;
	char *args[maxrunargs];
	char command[256];
	
#ifdef macos
	system(s);
#else
	if ((wpid = fork())) {
    	waitpid(wpid, &li, 0);
		return;
    }
    
    for (handle = 0; handle <= 255; handle++) close(handle);
    setsid();
    dup(0);
    dup(0);

	strcpy(command, s);
    args[0] = strtok(command, " \t");
    ct = 1;
    do {
      args[ct] = strtok(NULL, " \t");
    } while (args[ct++] != NULL && ct < maxrunargs);
    execvp(args[0], args);
    exit(1);
#endif
}
#undef maxrunargs

static void sort_file(char *name, int only_one, int reverse_order)
{
	char add_option[20] = "";
	char s[256], tmp[256];
	
#ifdef dpmacos
#else
	tmpnam(tmp);
	if (only_one) {
		strcat(add_option, "-u");
		if (reverse_order)
			strcat(add_option, "r");
	} else if (reverse_order)
		strcat(add_option, "-r");
	sprintf(s, "sort %s -o %s %s", add_option, tmp, name);
	my_system(s);
	remove(name);
	rename(tmp, name);
#endif
}

static int is_valid_char(unsigned char c)
{
	if (c > 96 && c < 123) return 1;
	if (c == 45) return 1;
	if (c < 48) return 0;
	if (c < 58) return 1;
	if (c < 65) return 0;
	if (c < 91) return 1;
	if (c < 128) return 0;
	if (c < 155) return 1;
	if (c == 158) return 1;
	if (c < 160) return 0;
	if (c < 166) return 1;
	if (c < 176) return 0;
	if (c < 185) return 1;
	if (c == 225) return 1;
	return 0;
}

static int split_the_word(char *outw, char **inw, int *wlen)
{
	char c;
	char *p, *n;
	
	p = *inw;
	while ((*outw++ = c = *p++) && c != '-');
	if (!c) {
		*wlen = (--p - *inw);
		*inw = p;
		return (*wlen > 0);
	} else {
		n = p;
		while ((c = *n++) && isdigit(c));
		if (!c) {
			strcpy(--outw, p);
			*wlen = (--n - *inw) - 1;
			*inw = n;
			return (*wlen > 0);
		} else {
			*--outw = '\0';
			*wlen = (p - *inw) - 1;
			*inw = p;
			return 1;
		}
	}
}

static int convert_the_word(char *outw, char *inw)
{
	int ct = 0;
	int wlen = 0;
	int nonum = 0;
	char c;
	
	while ((c = *inw++) && (ct < maxwordlen)) {
	
		if (c == '-')
			continue;

		wlen++;
		c = tolower(c);
		
		switch ((unsigned char)c) {

		case 132:
		case 142:
 			*outw++ = 'a';
 			*outw++ = 'e';
 			ct += 2;
 			nonum = 1;
			break;

		case 148:
		case 153:
 			*outw++ = 'o';
 			*outw++ = 'e';
			ct += 2;
			nonum = 1;
			break;

		case 129:
		case 154:
			*outw++ = 'u';
			*outw++ = 'e';
			ct += 2;
			nonum = 1;
			break;

		case 158:
		case 225:
			*outw++ = 's';
			*outw++ = 's';
			ct += 2;
			nonum = 1;
			break;

		default:
		  	*outw++ = c;
		  	ct++;
		  	if (nonum == 0)
		  		if (!isdigit(c))
		  			nonum = 1;
		  	break;
		}
	}
	
	if (nonum != 0 && c != '\0') /* Eingangswortlaenge bestimmen */
		while ((*inw++) && wlen <= maxvalidwordlen) wlen++;
	
	if (ct > maxwordlen) {
		*--outw = '\0';
		if ((nonum == 0 && wlen > maxnumwordlen) || wlen > maxvalidwordlen)
			return 0;
		else
			return --ct;
	} else {
		*outw = '\0';
		if ((nonum == 0 && wlen > maxnumwordlen) || wlen > maxvalidwordlen)
			return 0;
		else
			return ct;
	}
}

#ifdef compress
static long char24long(unsigned char *b)
{
	long l;
	unsigned char *p;

	p = (unsigned char *)&l;
#ifndef BENDIAN
	*p++ = *b++;
	*p++ = *b++;
	*p++ = *b;
	*p   = '\0';
#else
	*p++ = '\0';
	*p++ = *b++;
	*p++ = *b++;
	*p   = *b;
#endif
	if (l == nilp) return -1;
  	return l;
}

static void long24char(long l, unsigned char *b)
{
	unsigned char *p;

	if (l < 0) l = nilp;
	p = (unsigned char *)&l;
#ifndef BENDIAN
	*b++ = *p++;
	*b++ = *p++;
	*b   = *p;
#else
	p++;
	*b++ = *p++;
	*b++ = *p++;
	*b   = *p;
#endif
}
#endif

static long msgread(int handle, messages *unpacked_data)
#ifdef compress
{
	long result;
	diskmsg packed_data;
	
	result = read(handle, (char *)packed_data, sizeof(diskmsg));
	if (result != sizeof(diskmsg))
		return 0;
	else {
		unpacked_data->msgnum = char24long(packed_data);
		unpacked_data->next = char24long(packed_data+3);
		return result;
	}
}
#else
{
	long result;
	
	result = read(handle, (char *)unpacked_data, sizeof(messages));
	if (result != sizeof(messages))
		return 0;
	else
		return result;
		
}
#endif

static long msgwrite(int handle, messages *unpacked_data)
#ifdef compress
{
	long result;
	diskmsg packed_data;
	
	long24char(unpacked_data->msgnum, packed_data);
	long24char(unpacked_data->next, packed_data+3);
	
	result = write(handle, (char *)packed_data, sizeof(diskmsg));
	if (result != sizeof(diskmsg))
		return 0;
	else 
		return result;
}
#else
{
	long result;
	
	result = write(handle, (char *)unpacked_data, sizeof(messages));
	if (result != sizeof(messages))
		return 0;
	else
		return result;
		
}
#endif


static long dictread(int handle, dictentry *unpacked_data)
#ifdef compress
{
	long result;
	diskentry packed_data;
	
	result = read(handle, (char *)&packed_data, sizeof(diskentry));
	if (result != sizeof(diskentry))
		return 0;
	else {
		unpacked_data->thischar = packed_data[0];
		unpacked_data->stopword = packed_data[1];
		unpacked_data->doccount = char24long(&packed_data[2]);
		unpacked_data->matchseek = char24long(&packed_data[5]);
		unpacked_data->unmatchseek = char24long(&packed_data[8]);
		unpacked_data->liststart = char24long(&packed_data[11]);
		return result;
	}
}
#else
{
	long result;
	
	result = read(handle, (char *)unpacked_data, sizeof(dictentry));
	if (result != sizeof(dictentry))
		return 0;
	else
		return result;
		
}
#endif


static long dictwrite(int handle, dictentry *unpacked_data)
#ifdef compress
{
	long result;
	diskentry packed_data;
	
	packed_data[0] = unpacked_data->thischar;
	packed_data[1] = unpacked_data->stopword;
	long24char(unpacked_data->doccount, &packed_data[2]);
	long24char(unpacked_data->matchseek, &packed_data[5]);
	long24char(unpacked_data->unmatchseek, &packed_data[8]);
	long24char(unpacked_data->liststart, &packed_data[11]);
	
	result = write(handle, (char *)&packed_data, sizeof(diskentry));
	if (result != sizeof(diskentry))
		return 0;
	else 
		return result;
}
#else
{
	long result;
	
	result = write(handle, (char *)unpacked_data, sizeof(dictentry));
	if (result != sizeof(dictentry))
		return 0;
	else
		return result;
		
}
#endif




/*
  Uebergabe:
  	k:
  		<0 oder Filehandle
  	word:
  		gesuchtes Wort
  	wordlen:
  		Laenge des Wortes
  		
  Rueckgabewerte:
	Funktionswert:
		wenn >= 0: Zeiger auf Liste, die alle msgnums enthaelt in denen
					dieses Wort vorkommt
			oder
			   -1: Wort nur zum Teil abgearbeitet, Kette war zuende
			   -2: Wort vorhanden aber keine msgnum-Liste vorhanden
			   -3: Wort nur zum Teil abgearbeitet
			   -4: Fehler in der Kette
			   -5: wie -1
			 -100: Datei nicht vorhanden (andere Rueckgabewerte dann ungueltig)
			 -300: Wort ist ein "Stopword" -> besitzt keinen relevanten
			 	   Informationsgehalt und ist von der Suche ausgeschlossen
	complen:
		Anzahl erfolgreich verglichener Buchstaben
	doccount:
		Anzahl Dokumente, in denen dieses Wort vorkommt
*/

static long find_dictentry(int f, char *word, int wordlen,
							int *complen, int *doccount)
{
	int k;
	int bct = 0;
	int wlen;
	int hit = 0;
	int close_file = 0;
	long seek = 0;
	int comp;
	int step = 0;
	struct dictentry dentry;
	char dictname[256];
	
	k = f;
	if (k < 0) {
#ifdef macos
		sprintf(dictname, "%d.aix", (unsigned char)word[0]);
#else 
		sprintf(dictname, "%s%d.aix", dbasedir, (unsigned char)word[0]);
#endif
		if ((k = open(dictname, O_RDONLY)) < 0)
			return (-100);

		close_file = 1;
	} else
		lseek(k, 0, SEEK_SET);
	
	wlen = wordlen - 1;
	
	while (hit == 0
			&& dictread(k, &dentry)
			&& step <= 256) {
	
		comp = (unsigned char)word[bct] - (unsigned char)dentry.thischar;
		if (comp > 0) {
			step++;
			seek = dentry.unmatchseek;
		} else if (comp < 0) {
			hit = 2;
			bct--;
		} else {
			if (bct == wlen) {
				hit = 1;
			} else {
				if ((seek = dentry.matchseek) >= 0) {
					bct++;
					step = 0;
				}
			}
		}
		
		if (!hit) {
			if (seek < 0)
				hit = 4; /* Kette zuende */
			else {
				if (lseek(k, seek, SEEK_SET) != seek)
					hit = 3; /* Fehler */
			}
		}
	
	}
	
	if (close_file)
		close(k);

	*complen = bct+1;
	
	if (hit == 1) {
		*doccount = dentry.doccount;
		if (dentry.stopword != '\0')
			return -300;
		else
			return dentry.liststart;
	} else {
		*doccount = 0;
		return ((hit + 1) * (-1));
	}
}

#ifdef wordlist
static FILE *of;

static void add_to_wordlist(char *word, int wordlen)
{
	if (of == NULL) return;
	fprintf(of, "%.*s\n", wordlen, word);
}
#endif

static int open_dictionary(char firstchar)
{
	int k;
	struct stat statbuf;
	char dictname[256], lockname[256];
	
	sprintf(dictname, "%s%d.aix", dbasedir, (unsigned char)firstchar);
	sprintf(lockname, "%s.lock", dictname);
	while (stat(lockname, &statbuf) == 0);	

#ifdef macos
	close(open(lockname, O_RDWR+O_CREAT));
	k = open(dictname, O_RDWR+O_CREAT);
#else
	close(open(lockname, O_RDWR+O_CREAT, 0x180));
	k = open(dictname, O_RDWR+O_CREAT, 0x180);
#endif

	if (k < 0)
		remove(lockname);
		
	return k;
}

static void close_dictionary(char firstchar, int k)
{
	char lockname[256];

	if (k >= 0)
		close(k);
		
	sprintf(lockname, "%s%d.aix.lock", dbasedir, (unsigned char)firstchar);
	remove(lockname);	
}

static void get_totaldocs(int increase, int cur_message)
{
	int handle;
	int wert, lmsg;
	char name[256];

	sprintf(name, "%sstatus", dbasedir);
#ifdef macos
	handle = open(name, O_RDWR+O_CREAT);
#else
	handle = open(name, O_RDWR+O_CREAT, 0x180);
#endif
	
	wert = 0;
	lmsg = 0;
	read(handle, (char *)&wert, sizeof(wert));
	read(handle, (char *)&lmsg, sizeof(lmsg));
	if (wert <= 0)
		wert = 0;
	if (lmsg <= 0)
		lmsg = 0;
	if ((cur_message <= 0 || (cur_message > lmsg)) && increase > 0) {
		wert += increase;
		lseek(handle, 0, SEEK_SET);
		write(handle, (char *)&wert, sizeof(wert));
		if (cur_message >= 0)
			write(handle, (char *)&cur_message, sizeof(cur_message));
	}
	close(handle);
	totaldocs = wert;
	lastscannedmsg = lmsg;
}

static int open_msglist(int readwrite, char firstchar, long *listsize)
{
	int handle, mode;
	char listname[256];
	
	if (readwrite)
		mode = O_RDWR+O_CREAT;
	else
		mode = O_RDONLY;

	sprintf(listname, "%s%d.msg", dbasedir, (unsigned char)firstchar);
#ifdef macos
	handle = open(listname, mode);
#else
	handle = open(listname, mode, 0x180);
#endif

	if (handle >= 0)
		*listsize = lseek(handle, 0, SEEK_END);
	else
		*listsize = -1;
	
	return handle;
}

static void close_msglist(int handle)
{
	if (handle >= 0)
		close(handle);
}


static long add_dictentry(int f, char *word, int wordlen, long listsize, int stop)
{
	int k;
	int bct = 0;
	int step = 0;
	int wlen;
	int hit = 0;
	int close_file = 0;
	long seek = 0;
	long lastseek = 0;
	long retval = -2;
	long fsize;
	int comp;
	struct dictentry dentry, newentry;
#ifdef addstopwords
	FILE *stopw;
	char swname[256];
#endif
	
	if (wordlen < minwordlen || wordlen > maxwordlen)
		return -1;
	
	k = f;
	if (k < 0) {
		k = open_dictionary(word[0]);
		if (k < 0) {
			close_dictionary(word[0], k);
			return (-100);
		}
		close_file = 1;
	}
	
	if ((fsize = lseek(k, 0, SEEK_END)) == 0) { /*neues Dictionary */
		dentry.thischar = word[0]; /* leeren Eintrag erzeugen */
		dentry.stopword = '\0';
		dentry.doccount = 0;
		dentry.matchseek = -1;
		dentry.unmatchseek = -1;		
		dentry.liststart = -1;
		dictwrite(k, &dentry);
		fsize = dictentrysize;
	}
	
	lseek(k, 0, SEEK_SET);
	wlen = wordlen - 1;
	
	while (!hit 
			&& (dictread(k, &dentry))
			&& step <= 256) {
	
		comp = (unsigned char)word[bct] - (unsigned char)dentry.thischar;

		if (comp > 0) {
			step++;
			lastseek = seek;
			if (dentry.unmatchseek < 0)
				hit = 5;
			else 
				seek = dentry.unmatchseek;
		} else if (comp < 0) {
			hit = 2;
		} else {
			if (bct == wlen) {
				hit = 1;
			} else {
				bct++;
				step = 0;
				lastseek = seek;
				if (dentry.matchseek >= 0)
					seek = dentry.matchseek;
				else
					hit = 4;
			}
		}
		
		if (!hit) {
			if (lseek(k, seek, SEEK_SET) != seek)
				hit = 3; /* Fehler */
		}
	
	}
	
	if (step > 256) { /* fatal error */
		if (close_file)
			close_dictionary(word[0], k);
		return -200;
	}
	
	if (hit == 1) {
		/* Wort schon vorhanden */
		if (stop)
			dentry.stopword = '\1';
		else if (dentry.stopword == '\0' && totaldocs >= mindocthreshold) {
			if (dentry.doccount * 100 / totaldocs > maxdocpercentage) {
				dentry.stopword = '\1';
#ifdef addstopwords
				sprintf(swname, "%sstopwords", dbasedir);
				if ((stopw = fopen(swname, "a+")) != NULL) {
					fprintf(stopw, "%.*s\n", wordlen, word);
					fclose(stopw);
				}
#endif
			}
		}
		
		if (dentry.stopword != '\0')
			retval = -300;
		else {
			if (dentry.liststart < 0) {
				dentry.liststart = listsize;
				retval = listsize;
#ifdef wordlist
				add_to_wordlist(word, wordlen);
#endif
			} else
				retval = dentry.liststart;
		}

		if (!stop)
			dentry.doccount++;

		lseek(k, dictentrysize * (-1), SEEK_CUR);
		dictwrite(k, &dentry);

	} else if (hit != 3 && hit != 0 && bct <= wlen) {
	
		if (hit == 2) {
			lseek(k, lastseek, SEEK_SET);
			dictread(k, &dentry);
			if (step == 0) {
				newentry.unmatchseek = dentry.matchseek;
				dentry.matchseek = fsize;
			} else {
				newentry.unmatchseek = dentry.unmatchseek;
				dentry.unmatchseek = fsize;
			}
		} else if (hit == 4) {
			newentry.unmatchseek = -1;
			dentry.matchseek = fsize;
		} else if (hit == 5) {
			newentry.unmatchseek = -1;
			dentry.unmatchseek = fsize;
		}

		lseek(k, fsize, SEEK_SET);
		while (bct <= wlen) {
			fsize += dictentrysize;
			newentry.stopword = '\0';
			if (bct < wlen) {
				newentry.matchseek = fsize;
				newentry.liststart = -1;
				newentry.doccount = 0;
			} else {
				newentry.matchseek = -1;
				newentry.doccount = 1;
				if (stop) {
					newentry.liststart = -1;
					newentry.stopword = '\1';
					retval = -300;
				} else {
					newentry.liststart = listsize;
					newentry.stopword = '\0';
					retval = listsize;
#ifdef wordlist
					add_to_wordlist(word, wordlen);
#endif
				}
			}
			newentry.thischar = word[bct];
			dictwrite(k, &newentry);
			newentry.unmatchseek = -1;
			bct++;
		}
		
		lseek(k, lastseek, SEEK_SET);
		dictwrite(k, &dentry);
	}
	
	if (close_file)
		close_dictionary(word[0], k);
	
	if (hit == 3)
		return -2;
	else
		return retval;
}



static void add_listentry(int handle, long liststart, long *listsize,
							unsigned long msgnum)
{
	struct messages lentry, newentry;
	
	if (handle < 0)
		return;
	
	lseek(handle, liststart, SEEK_SET);
	
	if (liststart == *listsize) { /* Eintrag anhaengen */
		newentry.msgnum = msgnum;
		newentry.next = -1;
		msgwrite(handle, &newentry);
		*listsize += messageentrysize;
		return;
	}
	
	msgread(handle, &lentry);
	
	lseek(handle, *listsize, SEEK_SET);
	newentry.msgnum = msgnum;
	newentry.next = lentry.next;
	msgwrite(handle, &newentry);
	lseek(handle, liststart, SEEK_SET);
	lentry.next = *listsize;
	msgwrite(handle, &lentry);
	*listsize += messageentrysize;
}


static void add_token(int dhandle, int lhandle,
						long *listsize,
						char *word, int wordlen,
						unsigned long msgnum)
{
	int liststart;
	
	if ((liststart = add_dictentry(dhandle, word, wordlen, *listsize, 0)) >= 0) {
		add_listentry(lhandle, liststart, listsize, msgnum);
	}
}

static void get_database_info(char *outfile)
{
}

static void follow_wordlist(FILE *out, int k, long seek, char *word_,
							int wlen, int mlen, int rct, char sepchar)
{
	struct dictentry dentry;
	char word[maxwordlen+1];

	if (++rct > maxwordlen*2) return;
	if (seek < 0) return;
	if (wlen > mlen) return;
	if (wlen >= maxwordlen) return;
	if (lseek(k, seek, SEEK_SET) != seek) return;
	if (!dictread(k, &dentry)) return;
	
	strcpy(word, word_);
	strncat(word, (char *)&dentry.thischar, 1);
	wlen++;
	if (dentry.liststart >= 0 && dentry.stopword == '\0' && dentry.doccount > 0)
		fprintf(out, "%.*s:%ld%c", wlen, word, dentry.doccount, sepchar);
	follow_wordlist(out, k, dentry.matchseek, word, wlen, mlen, rct, sepchar);
	follow_wordlist(out, k, dentry.unmatchseek, word_, --wlen, mlen, --rct, sepchar);
}

static void get_wordlist(FILE *out, char *word, int wordlen, int maxdepth,
							char sepchar)
{
	int k;
	int wlen;
	int hit = 0;
	int bct = 0;
	long seek = 0;
	int comp;
	int step = 0;
	struct dictentry dentry;
	char dictname[256];
	
#ifdef macos
	sprintf(dictname, "%d.aix", (unsigned char)word[0]);
#else 
	sprintf(dictname, "%s%d.aix", dbasedir, (unsigned char)word[0]);
#endif
	if ((k = open(dictname, O_RDONLY)) < 0)
		return;
	
	wlen = wordlen - 1;
	
	while (hit == 0
			&& dictread(k, &dentry)
			&& step <= 256) {
	
		comp = (unsigned char)word[bct] - (unsigned char)dentry.thischar;
		if (comp > 0) {
			step++;
			seek = dentry.unmatchseek;
		} else if (comp < 0) {
			hit = 2;
		} else {
			if (bct == wlen) {
				hit = 1;
			} else {
				if ((seek = dentry.matchseek) >= 0) {
					bct++;
					step = 0;
				}
			}
		}
		
		if (!hit) {
			if (seek < 0)
				hit = 4; /* Kette zuende */
			else {
				if (lseek(k, seek, SEEK_SET) != seek)
					hit = 3; /* Fehler */
			}
		}
	
	}
	
	if (hit == 1) {
		if (dentry.liststart >= 0 && dentry.stopword == '\0' && dentry.doccount > 0)
			fprintf(out, "%.*s:%ld%c", wordlen, word, dentry.doccount, sepchar);
		follow_wordlist(out, k, dentry.matchseek, word, wordlen, maxdepth, 0, sepchar);
		fprintf(out, "\n");
	}
	
	close(k);

}

static void get_substitutes(FILE *slist, FILE *out)
{
	char *token;
	int foundlen, wordlen;
	char word[256];
	
	while (feof(slist) == 0 && fgets(word, 255, slist) != NULL) {
		token = strchr(word, ':');
		if (token == NULL) continue;
		*token++ = '\0';
		token[strlen(token)] = '\0';
		foundlen = atoi(token);
		if (foundlen < minfoundforsubstitution) continue;
		wordlen = strlen(word);
		if (wordlen - foundlen > maxsubstitutiondiff) continue;
		fprintf(out, "\n\"%s\" not found. Nearby words:\n", word);
		word[foundlen] = '\0';
		get_wordlist(out, word, foundlen, foundlen + maxsubstitutiondiff, ' ');
	}
}

static void search_database(int argoffs, char *outfile, int argc, char *argv[])
{
	int ct;
	int doccount;
	int ret1;
	long ret;
	FILE *out, *sort, *tminus, *tsubst;
	int lhandle;
	long lsize;
	int foundwords = 0;
	int minuswords = 0;
	int founddocs = 0;
	int subst = 0;
	int wordlen;
	int tlen;
	int minus, is_minus;
	int lastmsgnum, msgnum;
	struct messages lentry;
	char c;
	char *token, *lastpos;
	char hs[256];
	char word[maxwordlen + 5];
	char nword[maxlinesize + 1];
	char tmp[256];
	char tmpminus[256];
	char tmpsubst[256];

	if ((out = fopen(outfile, "w")) == NULL) return;
	tmpnam(tmp);
	if ((sort = fopen(tmp, "w")) == NULL) {
		fprintf(out, "cannot create sort file 1");
		fclose(out);
		return;
	}

	tmpnam(tmpminus);
	if ((tminus = fopen(tmpminus, "w")) == NULL) {
		fprintf(out, "cannot create sort file 2");
		fclose(out);
		fclose(sort);
		remove(tmp);
		return;
	}
	
	tmpnam(tmpsubst);
	
	get_totaldocs(0, 0);

	fprintf(out, "FTR v%s, full text retrieval, (c) 1997-1999 Joachim Schurig DL8HBS\n", vnum);
	fprintf(out, "documents:%d ", totaldocs);

	for (ct = argoffs; ct < argc; ct++) {

		strncpy(hs, argv[ct], 255);
		hs[255] = '\0';
		
		token = hs;
		lastpos = token;
		do {
			while ((c = *lastpos++) && (is_valid_char(c)));
			if (lastpos > token + minwordlen) {
				lastpos[(-1)] = '\0';
				if (*token == '-') {
					minus = 1;
					token++;
				} else {
					minus = 0;
					if (*token == '+')
						token++;
				}
				
				while (split_the_word(nword, &token, &tlen)) {
					if ((wordlen = convert_the_word(word, nword)) >= minwordlen)
						ret = find_dictentry(-1, word, wordlen, &ret1, &doccount);
					else {
						ret = -300;
						doccount = 0;
						ret1 = 0;
					}
		
					if (ret == -300) {
						fprintf(out, "(%s:%d) ", word, doccount);
					} else if (ret >= 0 && foundwords >= 0) {
						if (!minus)
							fprintf(out, "%s:%d ", word, doccount);
						lhandle = open_msglist(0, word[0], &lsize);
						if (lhandle >= 0) {
							if (!minus)
								foundwords++;
							else
								minuswords++;
					
							ret = lseek(lhandle, ret, SEEK_SET);
							while (ret >= 0 && msgread(lhandle, &lentry)) {
								if (minus)
									fprintf(tminus, "%8ld -\n", lentry.msgnum);
								else
									fprintf(sort, "%8ld\n", lentry.msgnum);
								ret = lentry.next;
								if (ret > 0)
									ret = lseek(lhandle, ret, SEEK_SET);
							}
							close_msglist(lhandle);
						}
					} else {
						fprintf(out, "%s:%d ", word, doccount);
						foundwords = -1;
						if (ret < 0
							&& ret1 >= minfoundforsubstitution
							&& wordlen - ret1 <= maxsubstitutiondiff) {
							if ((tsubst = fopen(tmpsubst, "a+")) != NULL) {
								fprintf(tsubst, "%s:%d\n", word, ret1);
								fclose(tsubst);
								subst = 1;
							}
						}
					}
				}
			}
			
			token = lastpos;
				
		} while (c);

	}
	
	fprintf(out, "\n");
	
	fclose(sort);
	fclose(tminus);
	if (minuswords) {
		sort_file(tmpminus, 1, 0);
		tminus = fopen(tmpminus, "a+");
	} else {
		sort_file(tmp, 0, 1);
	}

	if ((foundwords > 0) && (sort = fopen(tmp, "r")) != NULL) {
		lastmsgnum = -1;
		msgnum = -1;
		ct = 0;
		while (feof(sort) == 0 && fgets(hs, 255, sort) != NULL) {
			token = strchr(hs, '\n');
			if (token != NULL)
				*token = '\0';
			msgnum = atoi(hs);
			if (msgnum != lastmsgnum) {
				if (lastmsgnum >= 0) {
					if (ct >= foundwords) {
						if (minuswords)
							fprintf(tminus, "%8d\n", lastmsgnum);
						else {
							founddocs++;
							fprintf(out, "%8d ", lastmsgnum);
						}
					}
				}
				lastmsgnum = msgnum;
				ct = 1;
			} else {
				ct++;
			}
		}
		
		if (lastmsgnum >= 0) {
			if (ct >= foundwords) {
				if (minuswords)
					fprintf(tminus, "%8d\n", lastmsgnum);
				else {
					founddocs++;
					fprintf(out, "%8d ", lastmsgnum);
				}
			}
		}
		
		fclose(sort);
	}
	remove(tmp);
	
	if (minuswords) {
		fclose(tminus);
		sort_file(tmpminus, 0, 1);
		tminus = fopen(tmpminus, "r");
		
		lastmsgnum = -1;
		msgnum = -1;
		is_minus = 0;
		ct = 0;
		while (feof(tminus) == 0 && fgets(hs, 255, tminus) != NULL) {
			token = strchr(hs, ' ');
			if (token != NULL) {
				*token = '\0';
				is_minus = 1;
			} else {
				token = strchr(hs, '\n');
				if (token != NULL)
				  *token = '\0';
			}
			msgnum = atoi(hs);
			if (msgnum != lastmsgnum) {
				if (lastmsgnum >= 0) {
					if (ct == 1 && !is_minus) {
						founddocs++;
						fprintf(out, "%8d ", lastmsgnum);
					}
				}
				lastmsgnum = msgnum;
				is_minus = 0;
				ct = 1;
			} else {
				if (!is_minus)
					ct++;
			}
		}
		
		if (lastmsgnum >= 0) {
			if (ct == 1 && !is_minus) {
				founddocs++;
				fprintf(out, "%8d ", lastmsgnum);
			}
		}

		fclose(tminus);
	}
	
	remove(tmpminus);

	fprintf(out, "\nfound: %d document", founddocs);
	if (founddocs == 1)
		fprintf(out, "\n");
	else
		fprintf(out, "s\n");
		
	if (subst) {
		if ((tsubst = fopen(tmpsubst, "r")) != NULL) {
			get_substitutes(tsubst, out);
			fclose(tsubst);
		}
	}
	
 	fclose(out);

	if (subst) remove(tmpsubst);
}

static void dispose_database(void)
{
	char s[256];
#ifdef dpmacos
#else
	sprintf(s, "rm -f %s*.aix", dbasedir);
	system(s);
	sprintf(s, "rm -f %s*.aix.lock", dbasedir);
	system(s);
	sprintf(s, "rm -f %s*.msg", dbasedir);
	system(s);
	sprintf(s, "rm -f %sstatus", dbasedir);
	system(s);
	sprintf(s, "rm -f %swordlist", dbasedir);
	system(s);
#endif
}

static void unlock_database(void)
{
	char s[256];
#ifdef dpmacos
#else
	sprintf(s, "rm -f %s*.aix.lock", dbasedir);
	system(s);
#endif
}

static void show_known_words(unsigned char startalpha, unsigned char stopalpha, char *outfile)
{
	unsigned char ct;
	FILE *out;
	char word[maxwordlen+1];
	
	if ((out = fopen(outfile, "w")) == NULL) return;
	for (ct = startalpha; ct < stopalpha; ct++) {
		word[0] = ct;
		word[1] = '\0';
		get_wordlist(out, word, 1, maxwordlen, '\n');		
	}
	fclose(out);
/*	sort_file(outfile, 0, 0); */
}

#ifdef headerskip
static void skip_header(FILE *k)
{
	char line[256];
	
	while (feof(k) == 0 && fgets(line, 255, k) != NULL && line[1] != '\0');
}
#endif

static void add_stopwords(void)
{
	int wordlen;
	int tlen;
	FILE *in;
	char *token;
	char name[256];
	char nword[256];
	char line[256];
	char word[maxwordlen + 5];

	strcpy(name, dbasedir);
	strcat(name, "stopwords");

	if ((in = fopen(name, "r")) == NULL) return;

	while (feof(in) == 0 && fgets(line, 255, in) != NULL) {
		token = strtok(line, " \t\n\r.,:!?");
		while (token != NULL) {
			while (split_the_word(nword, &token, &tlen)) {
				if ((wordlen = convert_the_word(word, nword)) >= minwordlen) {
					add_dictentry(-1, word, wordlen, -1, 1);
				}
			}
			token = strtok(NULL, " \t\n\r.,:!?");
		}
	}

	fclose(in);
}

static void scan_new_file(char *msgnums, char *filename)
{
	unsigned long msgnum;
	int wordlen;
	int ct;
	int fraglen;
	int tlen;
	int dhandle = -1;
	int lhandle = -1;
	long lsize = 0;
	FILE *in, *out;
	register char c;
	char act_dictionary;
	char line[maxlinesize + 1];
	char nword[maxlinesize + 1];
	char fragment[(maxlinesize + 1) * 2];
	char word[maxwordlen + 5];
	char *token, *lastpos;
	long wordcount = 0;
	char crawltemp[256];
#ifdef setbuf
	char inbuf[streambufsize], outbuf[streambufsize];
#endif
#ifdef wordlist
	char wlname[256];
#endif
	
	msgnum = atol(msgnums);
	get_totaldocs(1, msgnum);
	if (msgnum <= lastscannedmsg) return;
	
	if ((in = fopen(filename, "r")) == NULL) return;

	if (totaldocs <= 1)
		add_stopwords();
	
	tmpnam(crawltemp);
	if ((out = fopen(crawltemp, "w")) == NULL) {
		fclose(in);
		return;
	}

#ifdef setbuf	
	setvbuf(in, inbuf, _IOFBF, streambufsize);
	setvbuf(out, outbuf, _IOFBF, streambufsize);
#endif

#ifdef headerskip
	skip_header(in);
#endif

	*fragment = '\0';
	fraglen = 0;
	while (feof(in) == 0 && fgets(line, maxlinesize, in) != NULL) {
		token = line;
		ct = 0; /* um ascii-dumps rauszuhalten */
		while (*token == ' ') token++;
		while ((c = *token++) && c != ' ' && c != '\t' && ct <= maxvalidwordlen)
			ct++;
		if (ct <= maxvalidwordlen) {
			lastpos = line;
			do {
				while ((c = *lastpos++) && (c == ' ' || c == '\t'));
				token = --lastpos;
				while ((c = *lastpos++) && (is_valid_char(c)));
				if (c == '\0') {
					lastpos[(-1)] = '\0';
					if (fraglen < maxlinesize)
						strcat(fragment, token);
					else
						strcpy(fragment, token);
					fraglen = strlen(fragment);
				} else {
					if ((tlen = (lastpos - 1) + fraglen - token) >= minwordlen) {
						lastpos[(-1)] = '\0';
						if (fraglen > 0) {
							strcat(fragment, token);
							token = fragment;
							fraglen = 0;
						}
						if (token[tlen - 1] == '-' && c == '\n') {
							if (token == fragment)
								fraglen = tlen - 1;
							else {
								strcpy(fragment, token);
								fraglen = strlen(fragment) - 1;
							}
							fragment[fraglen] = '\0';
						} else {
							while (split_the_word(nword, &token, &tlen)) {
								if (tlen <= maxvalidwordlen) {
									if ((wordlen = convert_the_word(word, nword))
											>= minwordlen) {
										wordcount++;
										strcat(word, "\n");
										fputs(word, out);
									}
								}
							}
							fraglen = 0;
							*fragment = '\0';
						}
					} else {
						fraglen = 0;
						*fragment = '\0';
					}
				}
			} while (c);
		} else {
			fraglen = 0;
			*fragment = '\0';
		}
	}
	fclose(out);
	fclose(in);
	
	sort_file(crawltemp, 1, 0);
	
	if ((out = fopen(crawltemp, "r")) == NULL) {
		remove(crawltemp);
		return;
	}
#ifdef setbuf
	setvbuf(out, outbuf, _IOFBF, streambufsize);
#endif
#ifdef wordlist
	sprintf(wlname, "%swordlist", dbasedir);
	of = fopen(wlname, "a+");
#endif

	act_dictionary = '\0';
	while (feof(out) == 0 && fgets(word, maxwordlen + 5, out) != NULL) {
		if (word[0] != act_dictionary) {
			if (dhandle >= 0)
				close_dictionary(act_dictionary, dhandle);
			if (lhandle >= 0)
				close_msglist(lhandle);
			act_dictionary = word[0];
			dhandle = open_dictionary(act_dictionary);
			lhandle = open_msglist(1, act_dictionary, &lsize);
			if (dhandle < 0 || lhandle < 0) {
				close_msglist(lhandle);
				close_dictionary(act_dictionary, dhandle);
			}
		}
		add_token(dhandle, lhandle, &lsize,
						word, strlen(word) - 1, msgnum);
	}

	close_msglist(lhandle);
	close_dictionary(act_dictionary, dhandle);
	
	fclose(out);
	remove(crawltemp);

#ifdef wordlist
	if (of != NULL)
		fclose(of);
/*	sort_file(wlname, 0, 0); */
#endif
}




/* arguments:
   0   = own name
   1   = -a -s -d -i -w -u
   2   = crawler base directory
   3   = name of input/output file
   4   = if -a: message number
	   = else add. argument
   5.. = add. arguments
*/

int main(int argc, char *argv[])
{
	int ret, len;

#ifdef macos
	argc = ccommand(&argv);
#endif

	ret = -1;
	if (argc > 2) {
		ret = 0;
		
#ifdef macos
		*dbasedir = '\0';
#else
		strcpy(dbasedir, argv[2]);
		if ((len = strlen(dbasedir)) > 0 && dbasedir[len-1] != '/')
			strcat(dbasedir, "/");
/*		strcat(dbasedir, "crawler/"); */
#endif
		
		if (argc > 4 && !strcmp(argv[1], "-a") && *argv[3] != '\0')
			scan_new_file(argv[4], argv[3]);
		else if (argc > 4 && !strcmp(argv[1], "-s"))
			search_database(4, argv[3], argc, argv);
		else if (argc > 3 && !strcmp(argv[1], "-i"))
			get_database_info(argv[3]);
		else if (argc > 2 && !strcmp(argv[1], "-d"))
			dispose_database();
		else if (argc > 2 && !strcmp(argv[1], "-u"))
			unlock_database();
		else if (argc > 3 && !strcmp(argv[1], "-w"))
			show_known_words(0, 255, argv[3]);
		else ret = -1;
	}
	
	if (ret < 0) { /* Fehlermeldung */
	}
	
	return ret;
}
