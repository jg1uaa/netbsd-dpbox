/************************************************************/
/*                                                          */
/* DigiPoint SourceCode                                     */
/*                                                          */
/* Copyright (c) 1991-2000 Joachim Schurig, DL8HBS, Berlin  */
/*                                                          */
/* For license details see documentation                    */
/*                                                          */
/************************************************************/


#define BOXLOCAL_G
#include "boxlocal.h"


void _boxlocal_init(void)
{
  static int _was_initialized = 0;
  if (_was_initialized++)
    return;

  lastproc = 0;
  *lastproctxt = '\0';
  *current_command = '\0';
  *current_user = '\0';
  current_unr = 0;
  hold_view = false;
}


/* End. */
