/* codeconv.c : simple filter to strip html control chars */
/* added with umlaut conversion (ISO Latin-1 / ISO 8859-1) */
/* created 97/05/19 Joachim Schurig, DL8HBS */
/* changed 99/12/08 Joachim Schurig, DL8HBS */

#include <stdio.h>

int main(int argc, char *argv[])
{
  int c;
  
  while ((c = getchar()) != EOF) {

    if ((c < 32 && c != 10 && c != 13) || c > 127) {
      
      switch (c) {
      	
	case 0x8e : fputs("&Auml;", stdout);
	      	    break;
	case 0x99 : fputs("&Ouml;", stdout);
	      	    break;
	case 0x9a : fputs("&Uuml;", stdout);
	      	    break;
	case 0x84 : fputs("&auml;", stdout);
	      	    break;
	case 0x94 : fputs("&ouml;", stdout);
	      	    break;
	case 0x81 : fputs("&uuml;", stdout);
	      	    break;
	case 0xE1 : fputs("&szlig;", stdout);
	      	    break;
	case 0x9E : fputs("&szlig;", stdout);
	      	    break;
      	default   : fprintf(stdout, "&#%d;", c);
	      	    break;
      }
    }
    
    else switch (c) {

      case '>': fputs("&gt;", stdout); break;
      case '<': fputs("&lt;", stdout); break;

      default: putchar(c);
    }

  }
  
  return 0;
}
