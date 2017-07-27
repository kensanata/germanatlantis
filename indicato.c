/* Progress Indicator Copyright (C) 1995-1998  Alexander Schroeder

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "indicato.h"

#define INDICATOR_INCREMENT     5
#define INDICATOR_SYMBOL_COUNT  4
long max_position, last_position;
char progress_symbol[INDICATOR_SYMBOL_COUNT] = "|/-\\";

/* Diese Funktion muss aufgerufen werden, sobald bekannt ist, wieviele
   Eintraege es gibt.  Danach muss in ab und zu entweder
   indicator_count_up () oder indicator_count_down () verwendet
   werden.  */
void
indicator_reset (long new_max_position)
{
  max_position = new_max_position;
  last_position = 0;
  fputs ("  0%", stdout);
}

/* Diese Funktion ist nach aussen nicht bekannt.  Sie wird von
   indicator_count_up () oder indicator_count_down () verwendet.  */
void
indicator_update (long current_position)
{
  long position;

  assert (max_position);
  position = 100 * current_position / max_position;

  if (position >= last_position)
    {
      printf ("\b\b\b\b%3ld%%", position);
      fflush (stdout);
      last_position += INDICATOR_INCREMENT;
    }
}

/* Der Parameter zeigt an, am wievielten Eintrag man ist.  Dies
   verwendet man mit Vorteil, wenn man die gesamte Anzahl einmal
   bestimmt hat, danach aber keine Referenz mehr auf dieses Total hat
   (zB. beim Lesen von Dateien).  */
void 
indicator_count_up (long current_position)
{
  indicator_update (current_position);
}

/* Die Verwendung von indicator_count_up () kann zu Ungenauigkeiten fuehren, so dass (zB. beim Lesen von Dateien)
   nicht ganz bis 100% hochgezaehlt wird.  Damit diese Unschoenheit beseitigt wird, kann man folgende Funktion
   aufrufen, welche ein ansprechendes 100% hinschreibt.  ;) */
void 
indicator_done ()
{
  indicator_update (max_position);
}

/* Der Parameter zeigt an, wieviele Eintraege noch fehlen.  Dies
   verwendet man mit Vorteil, wenn man von der gesamten Anzahl
   Eintraege zurueckzaehlt (zB. in linked lists).  */
void
indicator_count_down (long current_position)
{
  indicator_update (max_position - current_position);
}

/* Dies Funktion kann aufgerufen werden, wenn die Anzahl Schritte
   unbekannt ist.  */
void 
indicator_tick (void)
{
  static int i;
  printf ("\b%c", progress_symbol[i++]);
  fflush (stdout);
  if (i >= INDICATOR_SYMBOL_COUNT)
    i = 0;
}











