/* German Atlantis PB(E)M host Copyright (C) 1995-1998  Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed. It may
   not be sold or used commercially without prior written permission
   from the author.  */

/* Definitions for all strings that players have contact with for the
   atlantis host. Since a lot of data types are thus defined here,
   associated information is also defined here. */

#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#include "language.h"

/* Siehe info:(cpp)Swallow Semicolon fuer eine Erklärung der do {...}
   while 0 Schlaufe.  */

#define die(TEXT)                                          \
do                                                         \
{                                                          \
  printf ("\n\n%s:%d:%s\n", __FILE__, __LINE__, TEXT);     \
  exit(255);                                               \
}                                                          \
while (0)

char *translate (int index, int language, ...);

#endif /* TRANSLATE_H */
