
/* yapp.c
 *
 * Copyright (C) 1994 by Jonathan Naylor
 *
 * This module implements the YAPP file transfer protocol as defined by Jeff
 * Jacobsen WA7MBL in the files yappxfer.doc and yappxfer.pas.
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the license, or (at your option) any later version.
 */

/*
 * Yapp C and Resume support added by S N Henson.
 */

/*
 * Adapted for the use
 * in DPBOX by Joachim Schurig, DL8HBS 10.05.96
 * 
 * Fixed two errors of the original source
 * inhibiting resume mode on upload 16.05.96
 */

#include <ctype.h>
#include "yapp.h"

#define NUL             0
#define SOH             0x1
#define STX             0x2
#define ETX             0x3
#define EOT             0x4
#define ENQ             0x5
#define ACK             0x6
#define DLE             0x10
#define NAK             0x15
#define CAN             0x18

#define YAPPSTATE_S     0
#define YAPPSTATE_SH    1
#define YAPPSTATE_SD    2
#define YAPPSTATE_SE    3
#define YAPPSTATE_ST    4
#define YAPPSTATE_R     5
#define YAPPSTATE_RH    6
#define YAPPSTATE_RD    7


static void Write_Status(yapptype *yapp, char *s)
{
  yapp->statout(yapp->unr, s);
}


static void Send_RR(yapptype *yapp)
{
  yapp->chout(yapp->unr, ACK);
  yapp->chout(yapp->unr, 1);
}


static void Send_RF(yapptype *yapp)
{
  yapp->chout(yapp->unr, ACK);
  yapp->chout(yapp->unr, 2);
}


static void Send_RT(yapptype *yapp)
{
  yapp->chout(yapp->unr, ACK);
  yapp->chout(yapp->unr, ACK);
}


static void Send_AF(yapptype *yapp)
{
  yapp->chout(yapp->unr, ACK);
  yapp->chout(yapp->unr, 3);
}


static void Send_AT(yapptype *yapp)
{
  yapp->chout(yapp->unr, ACK);
  yapp->chout(yapp->unr, 4);
}


static void Send_NR(yapptype *yapp, char *reason)
{
  yapp->chout(yapp->unr, NAK);
  yapp->chout(yapp->unr, strlen(reason));
  yapp->lineout(yapp->unr, reason);
}


/* Send a Resume Sequence */

static void Send_RS(yapptype *yapp, long laenge)
{
  char buff[20];

  yapp->chout(yapp->unr, NAK);
  sprintf(buff, "%ld", laenge);
  yapp->chout(yapp->unr, strlen(buff) + 5);
  yapp->chout(yapp->unr, 'R');
  yapp->chout(yapp->unr, 0);
  yapp->lineout(yapp->unr, buff);
  yapp->chout(yapp->unr, 0);
  yapp->chout(yapp->unr, 'C');
  yapp->chout(yapp->unr, 0);
}


static void Send_SI(yapptype *yapp)
{
  yapp->chout(yapp->unr, ENQ);
  yapp->chout(yapp->unr, 1);
}


static void Send_CN(yapptype *yapp, char *reason)
{
  yapp->chout(yapp->unr, CAN);
  yapp->chout(yapp->unr, strlen(reason));
  yapp->lineout(yapp->unr, reason);
}


static void Send_HD(yapptype *yapp, char *filename, long laenge)
{
  unsigned short date, time;
  short len;
  char hs[20], w[256];
  char STR1[256];

  sprintf(hs, "%ld", laenge);
  yapp->chout(yapp->unr, SOH);
  sfgetdatime(filename, &date, &time);
  strcpy(w, filename);
  del_path(w);
  len = strlen(hs) + strlen(w) + 2;   /* include the NULs */
  if (date != 0)
    len += 9;
  yapp->chout(yapp->unr, len);
  yapp->lineout(yapp->unr, w);
  yapp->chout(yapp->unr, 0);
  yapp->lineout(yapp->unr, hs);

  yapp->chout(yapp->unr, 0);
  if (date == 0)
    return;
  int2hstr(date, hs);
  while (strlen(hs) < 4)
    sprintf(hs, "0%s", strcpy(STR1, hs));
  yapp->lineout(yapp->unr, hs);
  int2hstr(time, hs);
  while (strlen(hs) < 4)
    sprintf(hs, "0%s", strcpy(STR1, hs));
  yapp->lineout(yapp->unr, hs);
  yapp->chout(yapp->unr, 0);
}



static void Send_ET(yapptype *yapp)
{
  yapp->chout(yapp->unr, EOT);
  yapp->chout(yapp->unr, 1);
}


static void Send_DT(yapptype *yapp, short len)
{
  yapp->chout(yapp->unr, STX);
  if (len > 255)
    len = 0;
  yapp->chout(yapp->unr, len);
}


static void Send_EF(yapptype *yapp)
{
  yapp->chout(yapp->unr, ETX);
  yapp->chout(yapp->unr, 1);
}


static char checksum_(char *buf, short len)
{
  short i;
  char sum;

  sum = 0;
  for (i = 0; i < len; i++)
    sum += buf[i];
  return sum;
}


static boolean yapp_download_data(yapptype *yapp)
{
  boolean Result;
  char c;
  char *hptr;
  long i, x;
  char checksum;
  long seekh, len;
  char hfield[3][256];
  char hs[256];
  char STR1[256];

  Result = false;
  if (yapp == NULL)
    return Result;

  if (yapp->buffer[0] == (char)CAN || yapp->buffer[0] == (char)NAK) {
    Write_Status(yapp, "RcdABORT");
    return Result;
  }

  switch (yapp->state) {

  case YAPPSTATE_R:
    if (yapp->buffer[0] != (char)ENQ || yapp->buffer[1] != '\001') {
      Send_CN(yapp, "Unknown code");
      Write_Status(yapp, "SndABORT");
      return Result;
    }
    Send_RR(yapp);
    Write_Status(yapp, "YAPP reception started");
    yapp->state = YAPPSTATE_RH;
    break;

  case YAPPSTATE_RH:
    if (yapp->buffer[0] == (char)SOH) {

      /* Parse header: 3 fields == YAPP C */

      len = yapp->buffer[1];
      if (len == 0)
	len = 256;

      hptr = &yapp->buffer[2];

      for (x = 0; x <= 2; x++)
	*hfield[x] = '\0';
      yapp->fdate = 0;
      yapp->ftime = 0;

      while (len > 0 && yapp->yappc < 3) {
	c = hptr[0];
	if (c == '\0')
	  yapp->yappc++;
	else
	  sprintf(hfield[yapp->yappc] + strlen(hfield[yapp->yappc]), "%c", c);
	len--;
	hptr = (char *)(&hptr[1]);
      }

      if (yapp->filefd < minhandle && *yapp->fname == '\0') {
	del_path(hfield[0]);
	sprintf(STR1, "%s%s", yapp->yappdir, hfield[0]);
	strcpy(yapp->fname, STR1);
      }

      yapp->filelength = atol(hfield[1]);

      if (yapp->yappc < 3)
	yapp->yappc = 0;
      else {
	yapp->yappc = 1;
	if (strlen(hfield[2]) == 8) {
	  sprintf(STR1, "%.4s", hfield[2]);
	  yapp->fdate = hatoi(STR1);
	  yapp->ftime = hatoi(strsub(STR1, hfield[2], 4, 4));
	}
      }

      if (yapp->filefd < minhandle) {
	yapp->filefd = sfopen(yapp->fname, FO_RW);
	if (yapp->filefd < minhandle)
	  yapp->filefd = sfcreate(yapp->fname, FC_FILE);
	if (yapp->filefd < minhandle) {
	  sprintf(STR1, "Unable to open %s", yapp->fname);
	  Write_Status(yapp, STR1);
	  Send_NR(yapp, "Invalid filename");
	  return Result;
	}
      }

      sprintf(hs, "Receiving %s", hfield[0]);
      if (yapp->filelength > 0)
	sprintf(hs + strlen(hs), " (%s bytes)", hfield[1]);

      strcat(hs, ", mode = YAPP");

      if (yapp->yappc > 0)
	strcat(hs, "C");

      del_blanks(hs);
      Write_Status(yapp, hs);

      if (yapp->yappc > 0) {
	seekh = sfseek(0, yapp->filefd, SFSEEKEND);
	if (seekh > 0)
	  Send_RS(yapp, seekh);
	else
	  Send_RT(yapp);
      } else
	Send_RF(yapp);

      yapp->state = YAPPSTATE_RD;
    } else {
      if (yapp->buffer[0] != (char)ENQ || yapp->buffer[1] != '\001') {
	if (yapp->buffer[0] == (char)EOT && yapp->buffer[1] == '\001') {
	  Send_AT(yapp);
	  Write_Status(yapp, "YAPP reception ended");
	  return Result;
	}

	Send_CN(yapp, "Unknown code");
	Write_Status(yapp, "SndABORT");
	return Result;
      }
    }

    break;

  case YAPPSTATE_RD:
    if (yapp->buffer[0] == (char)STX) {
      len = yapp->buffer[1];
      if (len == 0)
	len = 256;
      yapp->total += len;
      if (yapp->yappc != 0) {
	checksum = 0;
	for (i = 2; i <= len + 1; i++)
	  checksum += yapp->buffer[i];
	if (checksum != yapp->buffer[len + 2]) {
	  Send_CN(yapp, "Bad Checksum");
	  Write_Status(yapp, "SndABORT: Bad Checksum");
	  return Result;
	}
      }

      sfwrite(yapp->filefd, len, &yapp->buffer[2]);
      yapp->seekpos += len;
    } else if (yapp->buffer[0] == (char)ETX && yapp->buffer[1] == '\001') {
      Send_AF(yapp);
      Write_Status(yapp, "RcvEof");
      yapp->state = YAPPSTATE_RH;
      sfclose(&yapp->filefd);
      if (yapp->fdate != 0)
	sfsetdatime(yapp->fname, &yapp->fdate, &yapp->ftime);
      yapp->delete_ = false;
    } else {
      Send_CN(yapp, "Unknown code");
      Write_Status(yapp, "SndABORT");
      return Result;
    }

    break;

  default:
    Send_CN(yapp, "Unknown state");
    Write_Status(yapp, "SndABORT");
    return Result;
    break;
  }
  return true;
}


boolean yapp_download(boolean init, boolean abort, yapptype *yapp,
		      char *buffp, long blen)
{
  boolean Result;
  long len, bminus;
  boolean used;

  Result = false;
  if (yapp == NULL)
    return Result;

  bminus = 0;

  if (init) {
    yapp->state = YAPPSTATE_R;
  } else if (abort) {
    Send_CN(yapp, "Cancelled");
    Write_Status(yapp, "SndABORT");
    return Result;
  }


  if (blen <= 0)
    return (bminus == 0);


  if (blen + yapp->buflen > 1024)
    bminus = 1024 - yapp->buflen;

  memcpy(&yapp->buffer[yapp->buflen], buffp, blen - bminus);
  yapp->buflen += blen - bminus;

  do {
    used = false;

    switch (yapp->buffer[0]) {

    case ACK:
    case ENQ:
    case ETX:
    case EOT:
      if (yapp->buflen >= 2) {
	if (!yapp_download_data(yapp))
	  return Result;
	yapp->buflen -= 2;
	memcpy(yapp->buffer, &yapp->buffer[2], yapp->buflen);
	used = true;
      }
      break;

    default:
      len = yapp->buffer[1];
      if (len == 0)
	len = 256;
      if (yapp->buffer[0] == (char)STX)
	len += yapp->yappc;
      if (yapp->buflen >= len + 2) {
	if (!yapp_download_data(yapp))
	  return Result;
	yapp->buflen += -len - 2;
	memcpy(yapp->buffer, &yapp->buffer[len + 2], yapp->buflen);
	used = true;
      }
      break;
    }
  } while (used);

  return (bminus == 0);
}


static boolean yapp_upload_data(yapptype *yapp)
{
  boolean Result;
  long len, x;
  char w[256];

  Result = false;

  if (yapp == NULL)
    return Result;


  if (yapp->buffer[0] == (char)CAN ||
      (yapp->buffer[0] == (char)NAK &&
       (yapp->buffer[1] <= '\003' || yapp->buffer[2] != 'R' ||
	yapp->buffer[3] != '\0'))) {
    Write_Status(yapp, "RcvABORT");
    return Result;
  }

  switch (yapp->state) {

  case YAPPSTATE_S:
    if (yapp->buffer[0] == (char)ACK && yapp->buffer[1] == '\001') {
      Write_Status(yapp, "SendHeader");
      Send_HD(yapp, yapp->fname, yapp->filelength);
      yapp->state = YAPPSTATE_SH;
    } else if (yapp->buffer[0] == (char)ACK && yapp->buffer[1] == '\002') {
      yapp->outlen = sfread(yapp->filefd, 255, yapp->outbuffer);
      yapp->outbufptr = 0;
      if (yapp->outlen > 0)
	Send_DT(yapp, yapp->outlen);
      if (yapp->yappc != 0) {
	yapp->outbuffer[yapp->outlen] = checksum_(yapp->outbuffer, yapp->outlen);
	yapp->outlen++;
      }
      yapp->state = YAPPSTATE_SD;
    } else {
      Send_CN(yapp, "Unknown code");
      Write_Status(yapp, "SndABORT");
      return Result;
    }

    break;

  case YAPPSTATE_SH:
    /* Could get three replies here:
     * ACK 02 : normal acknowledge.
     * ACK ACK: yappc acknowledge.
     * NAK ...: resume request.
     */
    if (yapp->buffer[0] == (char)NAK && yapp->buffer[2] == 'R') {
      len = yapp->buffer[1];
      if (yapp->buffer[len] == 'C')
	yapp->yappc = 1;

      w[0] = '\0';
      x = 4;
      while (x <= len && isdigit(yapp->buffer[x])) {
	sprintf(w + strlen(w), "%c", yapp->buffer[x]);
	x++;
      }

      yapp->seekpos = atol(w);

      if (sfseek(yapp->seekpos, yapp->filefd, SFSEEKSET) != yapp->seekpos) {
	Send_CN(yapp, "Invalid resume position");
	Write_Status(yapp, "Invalid resume position");
	Write_Status(yapp, "SndABORT");
	return Result;
      }

      yapp->buffer[0] = (char)ACK;

      if (yapp->yappc != 0)
	yapp->buffer[1] = (char)ACK;
      else
	yapp->buffer[1] = '\002';
    }

    if (yapp->buffer[0] != (char)ACK ||
	(yapp->buffer[1] != '\002' && yapp->buffer[1] != (char)ACK)) {
      Send_CN(yapp, "Unknown code");
      Write_Status(yapp, "SndABORT");
      return Result;
    }

    if (yapp->buffer[1] == (char)ACK)
      yapp->yappc = 1;

    if (yapp->yappc == 1)
      Write_Status(yapp, "mode = YAPPC");
    else
      Write_Status(yapp, "mode = YAPP");

    yapp->outlen = sfread(yapp->filefd, 255, yapp->outbuffer);

    yapp->outbufptr = 0;
    if (yapp->outlen > 0) {
      yapp->seekpos += yapp->outlen;
      Send_DT(yapp, yapp->outlen);
      if (yapp->yappc != 0) {
	yapp->outbuffer[yapp->outlen] = checksum_(yapp->outbuffer, yapp->outlen);
	yapp->outlen++;
      }
    }
    yapp->state = YAPPSTATE_SD;
    break;

  case YAPPSTATE_SD:
    Send_CN(yapp, "Unknown code");
    Write_Status(yapp, "SndABORT");
    return Result;
    break;

  case YAPPSTATE_SE:
    if (yapp->buffer[0] != (char)ACK || yapp->buffer[1] != '\003') {
      Send_CN(yapp, "Unknown code");
      Write_Status(yapp, "SndABORT");
      return Result;
    }

    Write_Status(yapp, "YAPP transmission ended");
    Send_ET(yapp);
    yapp->state = YAPPSTATE_ST;
    break;

  case YAPPSTATE_ST:
    if (yapp->buffer[0] == (char)ACK && yapp->buffer[1] == '\004')
      return Result;
    else {
      Send_CN(yapp, "Unknown code");
      Write_Status(yapp, "SndABORT");
      return Result;
    }
    break;
  }
  return true;
}


boolean yapp_upload(boolean init, boolean abort, yapptype *yapp, char *buffp,
		    long blen)
{
  boolean Result;
  long bminus, len, fill;
  boolean used;

  Result = false;
  if (yapp == NULL)
    return Result;

  bminus = 0;

  if (init) {
    Write_Status(yapp, "YAPP transmission started");
    yapp->state = YAPPSTATE_S;
    Send_SI(yapp);
    return true;
  }

  if (abort) {
    Write_Status(yapp, "SndABORT");
    Send_CN(yapp, "Cancelled by user");
    return Result;
  }

  if (blen > 0) {
    if (blen + yapp->buflen > 1024)
      bminus = 1024 - yapp->buflen;
    else
      bminus = 0;

    memcpy(&yapp->buffer[yapp->buflen], buffp, blen - bminus);
    yapp->buflen += blen - bminus;

    do {
      used = false;

      switch (yapp->buffer[0]) {

      case ACK:
      case ENQ:
      case ETX:
      case EOT:
	if (yapp->buflen >= 2) {
	  if (!yapp_upload_data(yapp))
	    return Result;

	  yapp->buflen -= 2;
	  memcpy(yapp->buffer, &yapp->buffer[2], yapp->buflen);
	  used = true;
	}
	break;

      default:
	len = yapp->buffer[1];
	if (len == 0)
	  len = 256;

	if (yapp->buflen >= len + 2) {
	  if (!yapp_upload_data(yapp))
	    return Result;

	  yapp->buflen += -len - 2;
	  memcpy(yapp->buffer, &yapp->buffer[len + 2], yapp->buflen);
	  used = true;
	}

	break;
      }

    } while (used);
  }


  fill = 0;
  while (fill < yapp->maxfill && yapp->state == YAPPSTATE_SD) {
    if (fill < yapp->maxfill && yapp->outlen == 0) {
      yapp->total -= yapp->yappc;

      yapp->outlen = sfread(yapp->filefd, 255, yapp->outbuffer);

      if (yapp->outlen > 0) {
	yapp->seekpos += yapp->outlen;
	yapp->outbufptr = 0;
	Send_DT(yapp, yapp->outlen);
	if (yapp->yappc != 0) {
	  yapp->outbuffer[yapp->outlen] = checksum_(yapp->outbuffer,
						    yapp->outlen);
	  yapp->outlen++;
	}
      } else {
	Write_Status(yapp, "SendEof");
	yapp->state = YAPPSTATE_SE;
	Send_EF(yapp);
      }
    }

    if (yapp->outlen <= 0)
      continue;
    yapp->buffout(yapp->unr, &yapp->outbuffer[yapp->outbufptr], yapp->outlen);
    yapp->outbufptr += yapp->outlen;
    yapp->total += yapp->outlen;
    fill += yapp->outlen;
    yapp->outlen = 0;
  }
  
  return (bminus == 0);
}
