/* German Atlantis PB(E)M host Copyright (C) 1995-1998   Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "atlantis.h"
 
int
wood (region * r)
{
  int i, m, w;

  /* m ist die bebaubare Flaeche, auf der auch Wald stehen koennte */

  m = production[r->terrain];
  if (!m)
    return 0;

  w = 100 * r->trees / m;

  for (i = MAXWOODS - 1; i; i--)
    if (w >= woodsize[i])
      break;

  /* i ist auf der entspr. woodsize */

  return i;
}

int
mainterrain (region * r)
{

  /* Falls neue Parteien/Monster an den Grenzen erschaffen werden, soll der
     Host nicht abstuerzen, wenn r->connect[] == 0 ist. */

  if (!r)
    return T_OCEAN;

  /* T_PLAIN kann als Wald gezeigt werden, wenn mehr als 60% Wald da sind. */

  if (r->terrain == T_PLAIN && wood (r) >= W_FOREST)
    return T_FOREST;
  else
    return r->terrain;
}



