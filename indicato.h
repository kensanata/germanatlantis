/* Progress Indicator Copyright (C) 1995-1998  Alexander Schroeder

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#ifndef INDICATOR_H
#define INDICATOR_H

#include <stdio.h>
#include <assert.h>

extern char progress_symbol[4];

void indicator_reset (long new_max_position);
void indicator_count_up (long current_position);
void indicator_count_down (long current_position);
void indicator_done ();
void indicator_tick (void);
#endif
