/* created 13.05.97 Joachim Schurig DL8HBS */
/* modified 19.05.97 Joachim Schurig DL8HBS */
 
/* filter for DPBOX Check- and List- lists, fills them with html links */
/* reads from stdin, writes to stdout */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define dpcgi "/cgi-bin/dpcmd" /* script for dpbox commands */
#define dplist "/cgi-bin/dplist" /* script for LIST after DIR */

int mode;

int get_line(char *s)
{
  if (feof(stdin) == 0 && fgets(s, 255, stdin) != NULL) return 1;
  else return 0;
}

int put_line(char *s)
{
  fputs(s, stdout);
  return 1;
}


	
void parse_checklist(void)
/*
    1 DL8HBS &gt; DIGI.......4 12.05.97 DL8HBS 10436 181 (BIN) pastrix.Z
    2 DL8HBS &gt; DIGI.......3 26.04.97 DL8HBS  9381   1 test
*/
{
  int modified;
  int l,x;
  int num;
  char board[9], nums[5], link[80], *token;
  char s[256], o[256];
  
  while (get_line(s)) {
    modified = 0;
    *o = '\0';
    l = strlen(s);
    if (l > 55 && s[13] == '&' && s[5] == ' ' && s[12] == ' '
       && s[17] == ' ' && s[30] == ' ' && s[56] == ' ' && isdigit(s[55])
       && isdigit(s[29])) {
     
      for (x = 18; x <= 25; x++) {
        if (s[x] != '.') board[x-18] = toupper(s[x]);
        else board[x-18] = '\0';
      }
      board[8] = '\0';
     
      token = nums;
      for (x = 26; x <= 29; x++) {
        if (s[x] != '.') *token++ = s[x];
      }  
      *token = '\0';
      num = atoi(nums);
      
      if (strlen(board) > 0 && num > 0) {
        sprintf(link, "<A HREF=\042%s?R%%20%s%%20%d\042>", dpcgi, board, num);
        strncpy(o, s, 18);
        o[18] = '\0';
        strcat(o, link);
        l = strlen(o);
        strncat(o, &s[18], 12);
        o[l+12] = '\0';
        strcat(o, "</A>");
        strcat(o, &s[30]);
        modified = 1;
      }
    }
    
    if (modified) put_line(o);
    else put_line(s);
  }
}

void parse_listlist(void)
/*
   1 DL8HBS 01.03.97 12:26    288  test
   2 DL8HBS 26.04.97 12:22    369  test
   3 DL8HBS 26.04.97 14:46   9413  test
   4 DL8HBS 12.05.97 14:58  10468  (BIN) pastrix.Z
*/
{
  int modified;
  int l,x;
  int num;
  char board[9], nums[5], *token, *t2;
  char s[256], o[256];
  
  /* first line must be of this kind:
     Info-File: DIGI
     first word after ": " is parsed as the actual board */
  
  *board = '\0';
  t2 = board;
  if (get_line(s)) {
    token = strstr(s, ": ");
    if (token != NULL) {
      token++;token++;
      for (x = 1; x < 9; x++) {
        if (isalnum(*token) || *token == '_' || *token == '-' || *token == '/')
          *t2++ = toupper(*token++);
        else
          x = 9;
      }
      *t2 = '\0';
    }
    put_line(s);
  }  
  
  while (get_line(s)) {
    modified = 0;
    *o = '\0';
    l = strlen(s);
    if (l > 33 && s[23] == ':' && s[4] == ' ' && s[11] == ' '
       && s[20] == ' ' && s[26] == ' ' && s[33] == ' ' && isdigit(s[3])
       && isdigit(s[12])) {

      token = nums;
      for (x = 0; x <= 3; x++) {
        if (isdigit(s[x])) *token++ = s[x];
      }  
      *token = '\0';
      num = atoi(nums);
      
      if (strlen(board) > 0 && num > 0) {
        sprintf(o, "<A HREF=\042%s?R%%20%s%%20%d\042>", dpcgi, board, num);
        l = strlen(o);
        strncat(o, s, 11);
        o[l+11] = '\0';
        strcat(o, "</A>");
        strcat(o, &s[11]);
        modified = 1;
      }
    }
    
    if (modified) put_line(o);
    else put_line(s);
  }
}

void parse_dirlist(void)
/*
Files:

DIGI........4 DIVERSES....1 MANUAL......6 STATISTI....2
*/
{
  int x;
  char link[80], s[256], o[256], board[9], hs[256], *t, *s1;
  
  if (get_line(s)) put_line(s);
  if (get_line(s)) put_line(s);
  
  while (get_line(s)) {
    *o = '\0';
    s1 = s;
    while (*s1 != '\0') {
      t = hs;
      x = 0;
      while (*s1 != ' ' && *s1 != '\n' && *s1 != '\r' && *s1 != '\0') {
        *t++ = *s1++;
      }
      *t = '\0';
      if (strlen(hs) > 10) {
        strncpy(board, hs, 8);
        board[9] = '\0';
        t = strchr(board, '.');
        if (t != NULL) *t = '\0';
        sprintf(link, "<A HREF=\042%s?%s%%201-\042>", dplist, board);
        strcat(o, link);
        strcat(o, hs);
        strcat(o, "</A> ");
        if (*s1 != ' ') strcat(o, "\n");
      } else if (*s1 != ' ') strcat(o, "\n");
      if (*s1 != '\0') s1++;
    } 
    
    put_line(o);
  }
}

int main(int argc, char *argv[])
{
  if (argc <= 1) mode = 1; /* check list */
  else if (!strcmp("-check", argv[1])) mode = 1;
  else if (!strcmp("-list", argv[1])) mode = 2;
  else if (!strcmp("-dir", argv[1])) mode = 3;
  else mode = 0;

  if (mode == 1) parse_checklist();
  else if (mode == 2) parse_listlist();
  else if (mode == 3) parse_dirlist();
  else exit(1);
  exit(0);
}
