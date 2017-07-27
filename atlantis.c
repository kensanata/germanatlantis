/* German Atlantis PB(E)M host Copyright (C) 1995-1999  Alexander Schroeder
   
   based on:

   Atlantis v1.0 13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include    "atlantis.h"

/* #define DEBUG_TEMP_UNITS zeigt auf dem Bildschirm jede gemachte TEMP Einheit an. */

int turn;
char buf[MAXLINE];
char zat[NAMESIZE] = NOZAT;
FILE *F;

region *regions;
faction *factions;

/* Daten in buf schreiben */

void
scat (char *s)
{
  strcat (buf, s);
}

void
icat (int n)
{
  char s[20];

  sprintf (s, "%d", n);
  scat (s);
}

/* Speicher */

void *
cmalloc (int n)
{
  void *p;

  if (n == 0)
    n = 1;

  p = malloc (n);

  if (p == 0)
    {
      puts ("Kein freier Speicher mehr.");
      exit (1);
    }

  return p;
}

/* Konversionen */

int
atoip (char *s)
{
  int n;

  n = atoi (s);

  if (n < 0)
    n = 0;

  return n;
}

void
nstrcpy (char *to, char *from, int n)
{
  do
    if ((*to++ = *from++) == 0)
      return;
  while (--n) ;
  *to = 0;
}

void
mnstrcpy (char **to, char *from, int n)
{
  int l;

  assert (n);
  if (*to)
    free (*to);
  l = strlen (from);
  n = min (l, n);
  *to = cmalloc (n +1);    /* Das +1 soll Platz fuer \0 am Ende bieten.  */
  nstrcpy (*to, from, n);
}

void
mstrcpy (char **to, char *from)
{
  int l;

  if (*to)
    free (*to);
  l = strlen (from);
  *to = cmalloc (l +1);    /* Das +1 soll Platz fuer \0 am Ende bieten.  */
  strcpy (*to, from);
}

/* LIST Operationen */

void
addlist (void *l1, void *p1)
{

  /* add entry p to the end of list l */

  list **l;
  list *p, *q;

  l = l1;
  p = p1;

  p->next = 0;

  if (*l)
    {
      for (q = *l; q->next; q = q->next)
        assert (q);
      q->next = p;
    }
  else
    *l = p;
}

void
choplist (list **l, list *p)
{

  /* remove entry p from list l - when called, a pointer to p must be
     kept in order to use (and free) p; if omitted, this will be a
     memory leak */

  list *q;

  if (*l == p)
    *l = p->next;
  else
    {
      for (q = *l; q->next != p; q = q->next)
        assert (q);
      q->next = p->next;
    }
}

void
translist (void *l1, void *l2, void *p)
{

  /* remove entry p from list l1 and add it at the end of list l2 */

  choplist (l1, p);
  addlist (l2, p);
}

/* void */
/* insertlist (list **l, list *p) */
/* { */

  /* insert entry p at the beginning of list l */

/*   p->next = *l; */
/*   *l = p; */

/* } */

/* void */
/* promotelist (void *l, void *p) */
/* { */

/*   remove entry p from list l; insert p again at the beginning of l */

/*   choplist (l, p); */
/*   insertlist (l, p); */
/* } */

void
removelist (void *l, void *p)
{

  /* remove entry p from list l; free p */

  choplist (l, p);
  free (p);
}

void
freelist (void *p1)
{

  /* remove all entries following and including entry p from a listlist */

  list *p, *p2;

  p = p1;

  while (p)
    {
      p2 = p->next;
      free (p);
      p = p2;
    }
}

int
listlen (void *l)
{

  /* count entries p in list l */

  int i;
  list *p;

  for (p = l, i = 0; p; p = p->next, i++)
    ;
  return i;
}

/* Funktionen, die den Status von Einheiten abklaeren */

int
effskill (unit *u, int i)
{
  int n, j, result;

  n = 0;
  if (u->number)
    n = u->skills[i] / u->number;
  j = 30;
  result = 0;

  while (j <= n)
    {
      n -= j;
      j += 30;
      result++;
    }

  return result;
}

int
cansee (faction *f, region *r, unit *u)
{
  int n, o;
  int cansee;
  unit *u2;

  if (u->faction == f)
    return 2;

  cansee = 0;
  if (u->guard || u->besieging || u->building || u->ship)
    cansee = 1;

  n = effskill (u, SK_STEALTH);

  for (u2 = r->units; u2; u2 = u2->next)
    {
      if (u2->faction != f)
        continue;

      if (u->items[I_RING_OF_INVISIBILITY] >= u->number &&
          !u2->items[I_AMULET_OF_TRUE_SEEING])
        continue;

      o = effskill (u2, SK_OBSERVATION);
      if (u2->enchanted == SP_NIGHT_EYES && o < NIGHT_EYE_TALENT)
        o = NIGHT_EYE_TALENT;

      if (o > n)
        return 2;
      if (o >= n)
        cansee = 1;             /* vielleicht koennte es noch 2 werden */
    }

  return cansee;
}

/* igetstr () [init getstr] -- falls der Parameter 0 ist, wird an der
   letzten Stelle weiter gelesen.  Wenn man getstr () aufruft, wird
   einfach igetstr (0) aufgerufen.  Soll das Parsen eines neuen
   Strings initialisiert werden, ruft man igetstr () mit einem
   Parameter !0 auf.  Wenn man eine Alternative braucht, zB. wird ein
   laengerer Befehl wie "LEHRE 12 13 14 15" oder "ZAUBER TELEPORT 15
   101 102 103" geparst und man moechte die Befehle der angegebenen
   Einheiten am liebsten auch mit einem igetstr pruefen, dann soll man
   aigetstr () [alternate init getstr] verwenden.  aigetstr hat einen
   eigenen *s, so dass man mit getstr () weiterliest, wo man mit
   igetstr () und getstr () aufgehoert hat.  aigetstr () und agetstr
   () sind equivalent zu igetstr () und getstr (), es ist allerdings
   Konvention, dass man wo immer moeglich igetstr () und getstr ()
   verwendet.  igetstr () und aigetstr () verwenden beide die Funktion
   bgetstr () [basic getstr].  Alle normalen get-Funktionen verwenden
   igetstr () und getstr (), wenn man also aigetstr () und agetstr ()
   verwenden will, muss man den Rest alles mit den find-Funktionen
   erledigen.  */
char *
bgetstr (char **s, char *init)
{
  int i;
  static char buf[MAXSTRING];

  /* Verwende neuen String init, falls angegeben.  */
  if (init)
    *s = init;

  /* skip whitespace */
  while (**s == ' ')
    (*s)++;

  i = 0;
  while (**s && **s != ' ' && i != sizeof buf - 1)
    {
      buf[i] = (**s);

      /* Hier wird space_replacement wieder in space zurueck
         verwandelt, ausser wenn es nach einem escape_char kommt. Im
         letzteren Fall wird escape_char durch space_replacement
         ersetzt, statt den aktuellen char einfach dran zu haengen.  */

      if (**s == SPACE_REPLACEMENT)
        {
          if (i > 0 && buf[i - 1] == ESCAPE_CHAR)
            buf[--i] = SPACE_REPLACEMENT;
          else
            buf[i] = SPACE;
        }

      i++;
      (*s)++;
    }

  buf[i] = 0;
  return buf;
}

char *
igetstr (char *init)
{
  static char *s;

  return bgetstr (&s, init);
}

char *
getstr (void)
{
  return igetstr (0);
}

char *
aigetstr (char *init)
{
  static char *s;

  return bgetstr (&s, init);
}

char *
agetstr (void)
{
  return aigetstr (0);
}

int
geti (void)
{
  return atoi (getstr ());
}

/* GET KEYWORD, SKILL, ITEM, SPELL benutzen FINDSTR - welche Item um
   Item eine Liste durchsucht.

   FIND wird immer von GET aufgerufen. GET braucht keine Parameter,
   IGET braucht einen String aus dem gelesen wird. In FIND stehen dann
   listen etc drinnen. FIND kann man auch allein verwenden, wenn der
   string _nur_ noch das gesuchte object enthaelt. Steht noch weitere
   info darin, sollte man GET verwenden, bzw. GETI wenn die info am
   Anfang eines neuen Stringes steht.  */

int
findstr (char **v, char *s, int n)
{
  /* strnicmp ignoriert Gross/Kleinschreibung, und vergleicht nur
     soviele Charakter, wie angegeben wurden. Dh. "o" passt auf
     "osten", aber "osterei" nicht! Aber vorsicht, wenn s = "", dann
     wuerde immer der erste Eintrag herauskommen!! */

  int i;

  if (!s[0])
    return -1;

  for (i = 0; i != n; i++)
    if (!strncasecmp (v[i], s, strlen (s)))
      return i;

  return -1;
}

/* Internationale Version -- soll spaeter anstelle von findstr
   verwendet werden.  */
int
_findstr (int v[], char *s, int n)
{
  /* strnicmp ignoriert Gross/Kleinschreibung, und vergleicht nur
     soviele Charakter, wie angegeben wurden. Dh. "o" passt auf
     "osten", aber "osterei" nicht! Aber vorsicht, wenn s = "", dann
     wuerde immer der erste Eintrag herauskommen!! */

  /* findstr geht immer davon aus, dass der Array v indexe enthaelt,
     welche in die Sprachentabelle strings zeigen.  */

  int l, i;

  if (!s[0])
    return -1;

  for (i = 0; i != n; i++)
    for (l = 0; l != MAXLANGUAGES; l++)
      if (!strncasecmp (strings[v[i]][l], s, strlen (s)))
	return i;

  return -1;
}

int
findkeyword (char *s)
{
  return findstr (keywords, s, MAXKEYWORDS);
}

int
igetkeyword (char *s)
{
  return findkeyword (igetstr (s));
}

int
getkeyword (void)
{
  return findkeyword (getstr ());
}

int
findparam (char *s)
{
  return findstr (parameters, s, MAXPARAMS);
}

int
igetparam (char *s)
{
  return findparam (igetstr (s));
}

int
getparam (void)
{
  return findparam (getstr ());
}

int
finditem (char *s)
{
  int i;


  i = _findstr (itemnames[0], s, MAXITEMS);
  if (i >= 0)
    return i;

  return _findstr (itemnames[1], s, MAXITEMS);
}

int
getitem (void)
{
  return finditem (getstr ());
}

/* Zeugs finden, entweder aus files oder aus strings.  */

region *
findregion (int x, int y, int z)
{
  region *r;

  for (r = regions; r; r = r->next)
    if (r->x == x && r->y == y && r->z == z)
      return r;

  return 0;
}

faction *
findfaction (int n)
{
  faction *f;

  for (f = factions; f; f = f->next)
    if (f->no == n)
      return f;

  return 0;
}

faction *
getfaction (void)
{
  return findfaction (atoip (getstr ()));
}

unit *
findunitglobal (int n)
{
  region *r;
  unit *u;

  /* findunit global! */

  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      if (u->no == n)
	return u;
  return 0;
}

int getunit0;
int getunitpeasants;
char getunit_text[20];

unit *
getnewunit (region *r, unit *u)
{
  int n;
  unit *u2;
  char *s;

  s = getstr ();
  n = atoip (s);
  if (n == 0)
    {
      getunit_text[0] = 0;
      return 0;
    }
  sprintf (getunit_text, "%s %d", parameters[P_TEMP], n);
  assert (strlen (getunit_text) < 20);

  for (u2 = r->units; u2; u2 = u2->next)
    if (u2->faction == u->faction && u2->alias == n)
      return u2;
  return 0;
}

unit *
getunitglobal (region *r, unit *u)
{
  int n;
  char *s;

  s = getstr ();

  /* Da s nun nur einen string enthaelt, suchen wir ihn direkt in der
     keywordliste. machen wir das nicht, dann wird getnewunit in s nach der
     nummer suchen, doch dort steht nur "temp" drinnen! */
  switch (findparam (s))
    {
    case P_TEMP:
      return getnewunit (r, u);
    case P_UNIT:
      s = getstr ();            /* Verbesserung von Syntax Fehlern */
      break;
    }
  /* Da findunitglobal auch 0 zurueckliefert, falls nichts gefunden
     wurde, koennen wir hier auch 0 zurueck liefern.  */
  n = atoip (s);
  if (n == 0)
    {
      getunit_text[0] = 0;
      return 0; 
    }
  sprintf (getunit_text, "%d", n);
  assert (strlen (getunit_text) < 20);
  /* findunit global! r und u wurden nur fuer getnewunit gebraucht */
  return findunitglobal (atoip (s));
}

unit *
getunit (region *r, unit *u)
{
  int n;
  char *s;
  unit *u2;

  getunit0 = 0;
  getunitpeasants = 0;

  s = getstr ();

  /* Da s nun nur einen string enthaelt, suchen wir ihn direkt in der
     paramliste. machen wir das nicht, dann wird getnewunit in s nach
     der nummer suchen, doch dort steht bei temp-units nur "temp"
     drinnen! */
  switch (findparam (s))
    {
    case P_TEMP:
      return getnewunit (r, u);
    case P_UNIT:
      s = getstr ();            /* Verbesserung von Syntax Fehlern */
      break;
    }
  /* Auf dem Ozean werden Bauern als 0 interpretiert.  Die
     resultierende unit kann dann als "nicht gefunden" gemeldet
     werden.  */
  if (r->terrain != T_OCEAN && findparam (s) == P_PEASANT)
    {
      strcpy (getunit_text, s);
      getunitpeasants = 1;
      return 0;
    }
  n = atoip (s);
  /* getunit_text[0] == 0 gilt, falls n keine Zahl oder 0 ist.  */
  getunit_text[0] = 0; 
  if (n == 0)
    {
      getunit0 = 1;
      return 0; 
    }
  /* Lokale units - contacts () erlaubt es unit u auch units u2, die u
     nicht sehen kann, anzusprechen.  Dies ist immer noch lokal! */
  for (u2 = r->units; u2; u2 = u2->next)
    if (u2->no == n && (cansee (u->faction, r, u2) || contacts (r, u2, u)))
      {
	sprintf (getunit_text, "%d", u2->no);
	assert (strlen (getunit_text) < 20);
	return u2;
      }
  /* Falls n eine Zahl war aber keine Einheit gefunden wurde.  */
  sprintf (getunit_text, "%d", n);
  assert (strlen (getunit_text) < 20);
  return 0;
}

/* - String Listen --------------------------------------------- */

strlist *
makestrlist (char *s)
{
  strlist *S;

  S = cmalloc (sizeof (strlist) + strlen (s));
  strcpy (S->s, s);
  return S;
}

void
addstrlist (strlist **SP, char *s)
{
  addlist (SP, makestrlist (s));
}

void
catstrlist (strlist **SP, strlist *S)
{
  strlist *S2;

  while (*SP)
    SP = &((*SP)->next);

  while (S)
    {
      S2 = makestrlist (S->s);
      addlist2 (SP, S2);
      S = S->next;
    }

  *SP = 0;
}

/* - Fehler ---------------------------------------------------- */

void
mistake (faction *f, unit *u, char *s, char *comment)
{
  static char buf[2 * DISPLAYSIZE];
  unsigned int i;

  if (strlen (s) > DISPLAYSIZE)
    strcpy (s + DISPLAYSIZE - 4, "...");
  /* snprintf () schreibt nicht mehr als die angegebene Anzahl bytes!
     Sollte sprintf () an vielen Stellen ersetzen.  */
  i = snprintf (buf, sizeof buf, "(%d) '%s' - %s.", u->no, s, comment);
  assert (i < sizeof buf);
  addstrlist (&f->mistakes, buf);
}

void
mistake2 (unit *u, strlist *S, char *comment)
{
  mistake (u->faction, u, S->s, comment);
}

void
mistakeu (unit *u, char *comment)
{
  mistake (u->faction, u, u->thisorder, comment);
}

/* - Meldungen ------------------------------------------------- */

void
addwarning (faction *f, char *s)
{
  addstrlist (&f->warnings, s);
}

void
addevent (faction *f, char *s)
{
  addstrlist (&f->events, s);
}

void
addmessage (faction *f, char *s)
{
  addstrlist (&f->messages, s);
}

void
addbattle (faction *f, char *s)
{
  addstrlist (&f->battles, s);
}

void
addcomment (region *r, char *s)
{
  addstrlist (&r->comments, s);
}

void
adddebug (region *r, char *s)
{
  addstrlist (&r->debug, s);
}

void
addmovement (faction *f, char *s)
{
  addstrlist (&f->movement, s);
}

void
addincome (faction *f, char *s)
{
  addstrlist (&f->income, s);
}

void
addcommerce (faction *f, char *s)
{
  addstrlist (&f->commerce, s);
}

void
addproduction (faction *f, char *s)
{
  addstrlist (&f->production, s);
}

/* - Namen der Strukturen -------------------------------------- */

char *
factionid (faction *f)
{
  static char buf[NAMESIZE + 20];

  sprintf (buf, "%s (%d)", f->name ? f->name : "", f->no);
  return buf;
}

char *
regionid (region *r)
{
  static char buf[NAMESIZE + 20];

  if (!r)
    strcpy (buf, "(Chaos)");
  else if (r->terrain == T_OCEAN)
    sprintf (buf, "(%d,%d)", r->x, r->y);
  else
    sprintf (buf, "%s (%d,%d)", r->name ? r->name : "", 
	     r->x, r->y);
  return buf;
}

char *
buildingid (building *b)
{
  static char buf[NAMESIZE + 20];

  sprintf (buf, "%s (%d)", b->name ? b->name : "", b->no);
  return buf;
}

building *
largestbuilding (region *r)
{
  building *b, *b2;

  /* durch die verw. von '>' statt '>=' werden die aelteren burgen
     bevorzugt.  */

  b2 = 0;

  for (b = r->buildings; b; b = b->next)
    if (!b2 || b->size > b2->size)
      b2 = b;

  return b2;
}

char *
shipid (ship *sh)
{
  static char buf[NAMESIZE + 20];

  sprintf (buf, "%s (%d)", sh->name ? sh->name : "", sh->no);
  return buf;
}

/* Diese Funktion gibt es zwei mal, damit man in einem Funktionsaufruf
   zwei units als Parameter angeben kann, zB. "A gibt B etwas".  Dies
   waere sonst nicht moeglich, da beide Funktionsaufrufe den gleichen
   buf pointer zurueckliefern wuerden, die Meldung dann also hiesse "B
   gibt B etwas"! */
char *
unitid (unit *u)
{
  static char buf[NAMESIZE + 20];

  sprintf (buf, "%s (%d)", u->name ? u->name : "", u->no);
  return buf;
}

char *
unitid2 (unit *u)
{
  static char buf[NAMESIZE + 20];

  sprintf (buf, "%s (%d)", u->name ? u->name : "", u->no);
  return buf;
}

/* -- Kampf! --------------------------------------------------- */

int
distribute (int old, int new, int n)
{
  int i;
  int t;

  assert (new <= old);

  if (old == 0)
    return 0;

  t = (n / old) * new;
  for (i = (n % old); i; i--)
    if (rand () % old < new)
      t++;

  return t;
}

int
isallied (unit *u, unit *u2)
{
  rfaction *rf;

  /* u ist mit den Bauern alliiert, wenn u die Region bewacht.  */
  if (!u2)
    return u->guard;

  /* Alle Einheiten der eigenen Partei.  */
  if (u->faction == u2->faction)
    return 1;

  for (rf = u->faction->allies; rf; rf = rf->next)
    if (rf->faction == u2->faction)
      return 1;

  return 0;
}

int
lovar (int n)
{
  n /= 2;
  return (rand () % n + 1) + (rand () % n + 1);
}

