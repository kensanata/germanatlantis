/* German Atlantis PB(E)M host Copyright (C) 1995-1998   Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "translate.h"

#define MAXSTRING 5000

/* Dies passt gerade so, dass die Platzhalter im Formatstring die Form
   "%[sd][0-9]" haben.  */
#define MAXARGS_TO_TRANSLATE 10

char * 
translate (int index, int language, ...)
{
  /* Siehe GNU libc manual zum Thema "variadic functions".  p zeigt
     immer auf das aktuelle Argument der variablen Argumente.  index
     und language sind fixe Argumente.  */
  va_list p;
  char *start, *pos, *message;
  char typ;
  int order;
  static char buf[MAXSTRING];

  /* Je nach typ wird entweder s_arg[i] oder i_arg[i] gefuellt.  Damit
     koennen wir alle Argumente abspeichern und spaeter in der
     richtigen Reihenfolge wieder abrufen.  */

  /* Das ganze besteht aus drei Durchgaengen.  Im ersten Durchgang
     wird bestimmt, welcher Platzhalter im string welchen Typ hat.
     Dies wird in arg_typ[i] gespeichert.  In maxargs wird die Anzahl
     Argumente abgespeichert.  Im zweiten Durchgang wird jedes
     Argument der translate Funktion entsprechend arg_typ[i]
     eingelesen und in s_arg[i] oder i_arg[i] gefuellt.  Im letzten
     scan wird der konstante Teil nach message kopiert, und die
     Platzhalter werden ersetzt.  */
  int i, maxargs;
  char arg_typ[MAXARGS_TO_TRANSLATE];
  char *s_arg[MAXARGS_TO_TRANSLATE];
  int i_arg[MAXARGS_TO_TRANSLATE];

  /* Erster Durchgang: Typ der Platzhalter feststellen.  */

  start = strings[index][language];
  maxargs = 0;

  /* Jetzt identifizieren wir zuerst alle Parameter, indem wir durch
     den string gehen.  start zeigt auf den Anfang des strings oder
     auf das letzte pos.  pos soll immer auf das naechste '%' Zeichen
     zeigen.  */

  while ((pos = (char *) strchr (start, '%')))
    {
      /* Der Typ und die Reihenfolge des Argumentes werden eruiert.
	 typ ist [sd] und order ist [0-9] (falls MAXARGS_TO_TRANSLATE
	 10).  */
      typ = pos[1];
      if (typ == 'd' || typ == 's')
	{
	  order = pos[2] - '0'; /* da order ein int und kein char ist! */
	  if (order < 0 || order >= MAXARGS_TO_TRANSLATE) die ("order falsch");
	  arg_typ[order] = typ;
	  maxargs++;
	  
	  /* Beim erneuten Durchlauf der for Schleife soll das
             naechste % Zeichen gefunden werden.  */
	  start = pos + 3;
	}
      else
	start = pos + 1;
    }

  /* Zweiter Durchgang: Argumente abspeichern.  */

  /* Das erste Argument kommt nach 'language' und wird in p
     gespeichert.  */
  va_start (p, language);
  i = 0;

  while (i < maxargs)
    {
      if (arg_typ[i] == 'd')
	i_arg[i++] = va_arg (p, int);
      else
	s_arg[i++] = va_arg (p, char *);
    }
  /* Leider können wir nicht kontrollieren, ob zuviele oder zuwenig
     Argumente geliefert wurden.  */

  /* Nochmals durch den String durchgehen, allen Text nach buf
     kopieren und alle Argumente in der richtigen Reihenfolge
     einsetzen.  Das Resultat dieser Funktion wird in buf gespeichert.
     Das Ende von buf wird in message gespeichert.  So kann man immer
     die Differenz zwischen start und pos an message anhaengen.  */
  start = strings[index][language];
  buf[0] = 0;
  message = buf;

  while ((pos = (char *) strchr (start, '%')))
    {
      /* Gefundener Teil ohne '%' nach Position message in buf
         kopieren, message erhoehen.  Da wir memcpy verwenden, koennen
         wir nicht strlen verwenden; das \0 am Ende fehlt.  Dieses
         wollen wir nicht in die Vorlage anstelle des % schreiben.  */
      if (strlen (buf) + (pos - start) >= sizeof buf) die ("buf zu klein");
      memcpy (message, start, pos - start);
      message += pos - start;
		  
      /* Der Typ und die Reihenfolge des Argumentes werden eruiert.  */
      typ = *(pos+1);
      if (typ == 'd' || typ == 's')
	{
	  order = *(pos+2) - '0'; /* da order ein int und kein char ist! */

	  /* Das entsprechende Argument wird nach message kopiert und
	     message wird entsprechend erhoeht.  In diesem Fall
	     verwenden wir Funktionen, welche \0 message anhaengen, so
	     dass wir message nachher mit strlen erhoehen duerfen.
	     Dies ist noetig, da wir die Laenge der Argumente nicht
	     kennen, sie also so oder so zaehlen muessen.  */
	  if (typ == 'd')
	    sprintf (message, "%d", i_arg[order]);
	  else
	    strcpy (message, s_arg[order]);
	  message += strlen (message);

	  /* Beim erneuten Durchlauf der for Schleife soll das
             naechste % Zeichen gefunden werden.  */
	  start = pos + 3;
	}
      else
	{
	  /* Falls kein gueltiger Typ angegeben, kopieren wir einfach
	     das Prozentzeichen und zaehlen start hoch.  start muss
	     hochgezaehlt werden, damit das Suchen nach dem naechsten
	     Prozentzeichen in der while-Schlaufe weiterhin
	     funktioniert.  */
	  strcpy (message, "%");
	  message++;
	  start = pos + 1;
	}
  }
  
  /* Am Ende wollen wir den verbleibenden String auch nach message
     kopieren.  */
  strcpy (message, start);

  va_end (p);
  return buf;
}











