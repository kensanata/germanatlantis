/* German Atlantis PB(E)M host Copyright (C) 1995-1999  Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "atlantis.h"
 
#define xisdigit(c)     ((c) == '-' || ((c) >= '0' && (c) <= '9'))
#define COMMENT_CHAR		';'
#define SUMMARY_BASENAME	"parteien"
#define ADDRESS_BASENAME	"adressen"

/* Konstanten, die man #definieren kann:

   DEBUG_SAVED_GAME: Die Daten-Datei wird beim Laden auf
   Doppeleintraege ueberprueft und alle Namen werden auf dem
   Bildschirm ausgegeben.

   DEBUG_MEMORY_USAGE: Der ungefaehre Speicherverbraucht wird mit der
   Zusammenfassung zusammen geschrieben.  */

int data_version;

/* ------------------------------------------------------------- */

int
cfopen (char *filename, char *mode)
{
  F = fopen (filename, mode);

  if (F == 0)
    {
      printf ("Ich kann die Datei %s nicht im %s-Modus oeffnen.\n", filename, mode);
      return 0;
    }
  return 1;
}

int
skipline (void)
{
  int c=0;

  while (c != EOF && c != '\n')
    c = fgetc (F);
  return c;
}

void
getbuf (void)
{
  int i=0, j=0, c=0, old_c=0, quote=0;

  /* buf vorsichtshalber auf 0, i: position in buf, j: position in
     quote, quote: 1 wenn innerhalb eines Textes in "Gaensefuesschen",
     c: aktueller, gelesener char */
  buf[0] = 0;                   
  for (;;)
    {
      /* old_c wird verwendet, weil in buf anstelle von tabs, spaces
         und newlines immer ein SPACE_REPLACEMENT steht (wird am ende
         der Schlaufe gesetzt).  */
      old_c = c;
      c = fgetc (F);

      /* Nach einem Semikolon folgen Kommentare bis zum Zeilenende.  */
      if (c == COMMENT_CHAR && !quote)
        {
          buf[i] = 0;
          c = skipline ();
        }

      /* Falls die Zeile zu lange ist, wird der Text abgebrochen, und
         die Zeile zuende gelesen.  Ein ge-quoteter Text war
         wahrscheinlich zu lang (Gaensefuesschen vergessen oder
         zuviel).  Es wird wieder auf normalen Zeilenmodus
         umgeschalten (quote=0).  Nun wird in c entweder '\n' oder EOF
         stehen.  Bei Funktionen, welche ihren eigenen buf[]
         definieren, muss selber auf zu grosse strings geprueft werden
         -- hier werden strings bis zur Maximallaenge eingelesen.  */
      if (i == sizeof buf - 1)
        {
          buf[i] = 0;
          c = skipline ();
	  quote = 0;
        }

      /* Bricht die Datei mitten in der Zeile ab, wird die Zeile
         ignoriert.  Dieser Test muss nach allen skipline () Aufrufen
         stehen, da diese auch mit EOF enden koennen.  */
      if (c == EOF)
        {
          buf[0] = EOF;
          break;
        }

      /* Befehle werden durch Zeilenende beendet.  Quotes durch eine
         Leerzeile (dh. zwei Zeilenenden hintereinander).  Die Zeile
         wird - mit oder ohne quote - durch 0 oder ';' abgebrochen
         (normalerweise werden so Kommentare ignoriert).  */
      if ((c == '\n' && !quote) ||
          (c == '\n' && quote && old_c == '\n'))
        {
          buf[i] = 0;
          if (i > 0 && buf[i - 1] == SPACE_REPLACEMENT)
            buf[i - 1] = 0;
          break;
        }

      /* quotes werden mit `"' begonnen oder beenden */
      if (c == '"')
        {
          quote = !quote;
          if (quote)
            j = 0;
          continue;
        }

      /* whitespace - alle spaces und tabs nach spaces und tabs oder
         am Anfang der Zeile oder am Anfang von strings werden
         ignoriert.  */
      if ((isspace (c) && isspace (old_c)) ||
          (j == 0 && isspace (c)))
        continue;

      j++;          /* Textlaenge seit Zeilen oder string beginn.  */

      /* In quotes werden alle spaces und tabs durch SPACE_REPLACEMENT
         ersetzt, ansonsten wird c eingesetzt.  Ausserhalb von quotes
         werden tabs durch spaces ersetzt, ansonsten wird auch c
         eingesetzt.  getstr etc operieren nur mit spaces. */
      if (isspace (c))
        buf[i++] = quote ? SPACE_REPLACEMENT : ' ';
      else
        buf[i++] = c;
    }
}

/* ------------------------------------------------------------- */

void
readunit (faction * f)
{
  int i;
  unit *u;
  strlist *S, **SP;

  if (!f)
    return;

  i = geti ();
  u = findunitglobal (i);

  if (u && u->faction == f)
    {
      /* SP zeigt nun auf den Pointer zur ersten StrListe.  Auf die
         erste StrListe wird mit u->orders gezeigt.  Deswegen also
         &u->orders, denn SP zeigt nicht auf die selbe stelle wie
         u->orders, sondern auf die stelle VON u->orders!  */
      SP = &u->orders;

      for (;;)
        {
          getbuf ();

          /* Erst wenn wir sicher sind, dass kein Befehl eingegeben
             wurde, checken wir, ob nun eine neue Einheit oder ein
             neuer Spieler drankommt.  */
          if (igetkeyword (buf) < 0)
            switch (igetparam (buf))
              {
              case P_UNIT:
              case P_FACTION:
              case P_NEXT:

                return;
              }

          if (buf[0] == EOF)
            return;

          if (buf[0])
            {
              /* Nun wird eine StrListe S mit dem Inhalt buf2
                 angelegt.  S->next zeigt nirgends hin.  */
              S = makestrlist (buf);

              /* Nun werden zwei Operationen ausgefuehrt (addlist2 ist ein
                 #definiertes macro!):

                 *SP = S -- Der Pointer, auf den SP zeigte, zeigt nun
                 *auf S! Also entweder u->orders oder S(alt)->next
                 *zeigen nun auf das neue S!

                 SP = &S->next -- SP zeigt nun auf den Pointer
                 S->next.  Nicht auf dieselbe Stelle wie S->next,
                 sondern auf die Stelle VON S->next! */

              addlist2 (SP, S);

              /* Und das letzte S->next darf natuerlich nirgends mehr
                 hinzeigen, es wird auf null gesetzt.  Warum das nicht
                 bei addlist2 getan wird, ist mir schleierhaft.  So
                 wie es jetzt implementiert ist, kann man es (hier auf
                 alle Faelle) hinter addlist2 schreiben, oder auch nur
                 am Ende einmal aufrufen - das waere aber bei allen
                 returns, und am ende des for (;;) blockes.  Grmpf.
                 Dann lieber hier, immer, und dafuer sauberer... */

              *SP = 0;
            }
        }
    }
  else
    {
      sprintf (buf, "Die Einheit %d gehoert uns nicht.", i);
      addstrlist (&f->mistakes, buf);
    }
}

/* ------------------------------------------------------------- */

faction *
readfaction (void)
{
  int i;
  faction *f;
  region *r;
  unit *u;

  i = geti ();
  f = findfaction (i);

  if (f)
    {
      /* Kontrolliere, ob das Passwort richtig eingegeben wurde.  Es
         muss in "Gaensefuesschen" stehen!!  */
      if (f->passw && strcmp (f->passw, getstr ()))
        {
          addstrlist (&f->mistakes, "Das Passwort wurde falsch eingegeben");
          return 0;
        }

      /* Loesche alle alten Befehle, falls schon welche eingeben
         worden sind.  */
      for (r = regions; r; r = r->next)
        for (u = r->units; u; u = u->next)
          if (u->faction == f)
            {
              freelist (u->orders);
              u->orders = 0;
            }

      /* Die Partei hat sich zumindest gemeldet, so dass sie noch
         nicht als untaetig gilt.  */
      f->lastorders = turn;
    }
  else
    printf (" -- Befehle fuer die ungueltige Partei Nr. %d.\r\n"
	     "    Befehle:     ", i);

  return f;
}

/* ------------------------------------------------------------- */

int
readorders (void)
{
  faction *f;
  struct stat fs;

  /* Der Name der Befehlsdatei muss in buf drinnen stehen */

  if (!cfopen (buf, "r"))
    return 0;
  stat (buf, &fs);

  puts ("- lese Befehlsdatei...");

  getbuf ();

  /* Auffinden der ersten Partei, und danach abarbeiten bis zur
     letzten Partei.  Gleichzeitig wird ein Indicator angezeigt.  Wie
     immer kann der Indicator problemlos entfernt werden.  */
  printf ("    Befehle: ");
  indicator_reset (fs.st_size);

  f = 0;
  while (buf[0] != EOF)
    switch (igetparam (buf))
      {
	/* In readfaction wird nur eine Zeile gelesen: Diejenige mit
           dem Passwort.  Die befehle der units werden geloescht, und
           die Partei wird als aktiv vermerkt. */
      case P_FACTION:
        f = readfaction ();
        getbuf ();
        break;

        /* Falls in readunit abgebrochen wird, steht dort entweder
           eine neue Partei, eine neue Einheit oder das Fileende.  Das
           switch wird erneut durchlaufen, und die entsprechende
           Funktion aufgerufen.  Man darf buf auf alle Faelle nicht
           ueberschreiben!  Bei allen anderen Eintraegen hier, muss
           buf erneut gefuellt werden, da die betreffende Information
           in nur einer Zeile steht, und nun die naechste gelesen
           werden muss.  */
      case P_UNIT:
        if (f)
          readunit (f);
        else
          getbuf ();
        indicator_count_up (ftell (F));
	break;

      case P_NEXT:
        f = 0;
      default:
        getbuf ();
      }
  indicator_done ();
  putchar ('\n');

  fclose (F);
  return 1;
}

/* ------------------------------------------------------------- */

int nextc;

void
rc (void)
{
  nextc = fgetc (F);
}

/* Read a string from file F into array s.  No memory allocation!  */
void
rs (char *s)
{
  while (nextc != '"')
    {
      if (nextc == EOF)
        {
          puts ("Der Datenfile bricht vorzeitig ab.");
          exit (1);
        }

      rc ();
    }

  rc ();

  while (nextc != '"')
    {
      if (nextc == EOF)
        {
          puts ("Der Datenfile bricht vorzeitig ab.");
          exit (1);
        }

      *s++ = nextc;
      rc ();
    }

  rc ();
  *s = 0;
}

/* Read a string from file F to pointer s.  Allocate memory for s.  */
void
mrs (char **s)
{
  rs (buf);
  if (buf[0])
    mstrcpy (s, buf);
}

/* Read a, int from file F and return it.  */
int
ri (void)
{
  int i;
  char buf[20];

  i = 0;

  while (!xisdigit (nextc))
    {
      if (nextc == EOF)
        {
          puts ("Der Datenfile bricht vorzeitig ab.");
          exit (1);
        }

      rc ();
    }

  while (xisdigit (nextc))
    {
      buf[i++] = nextc;
      rc ();
    }

  buf[i] = 0;
  return atoi (buf);
}

/* Liest eine strlist von Diskette (zB. die Meldungen fuer eine
   Partei) und speichert sie am Ende der strliste SP ab.  */
void
rstrlist (strlist ** SP)
{
  int n;
  strlist *S;

  n = ri ();

  while (--n >= 0)
    {
      rs (buf);
      S = makestrlist (buf);
      addlist2 (SP, S);
    }

  *SP = 0;
}

/* Liest eine strlist und vergisst sie sofort wieder (lean mode).  */
void
skipstrlist ()
{
  int n;
  n = ri ();
  while (--n >= 0)
    rs (buf);
}

void
number_space_free ()
{
  int i, m;
  
  puts ("Freie Parteien:");
  m=0;
  for (i=0; i!=5; i++)
    {
      do
	m++;
      while (findfaction (m));
      printf (" %d", m);
    }

  puts ("\nFreie Burgen:");
  m=0;
  for (i=0; i!=5; i++)
    {
      do
	m++;
      while (findbuilding (m));
      printf (" %d", m);
    }

  puts ("\nFreie Schiffe:");
  m=0;
  for (i=0; i!=5; i++)
    {
      do
	m++;
      while (findship (m));
      printf (" %d", m);
    }

  puts ("\nFreie Einheiten:");
  m=0;
  for (i=0; i!=5; i++)
    {
      do
	m++;
      while (findunitglobal (m));
      printf (" %d", m);
    }
  putchar ('\n');
}
      
void
readgame (int lean)
{
  int i, n, p, error=0, region_will_be_deleted=0;
  faction *f, **fp;
  rfaction *rf, **rfp;
  region *r, *r2, **rp;
  building *b, **bp;
  ship *sh, **shp;
  unit *u, **up;

  sprintf (buf, "data/%d", turn);
  if (!cfopen (buf, "r"))
    exit (1);
  rc ();

  /* Globale Variablen.  */

  data_version = ri ();
  turn = ri ();
  printf ("- Version: %d.%d, Runde %d.\n",
          data_version / 10, data_version % 10, turn);
  highest_unit_no = 0;

  /* Read factions.  */

  n = ri ();
  fp = &factions;

  /* fflush (stdout); */
  printf ("    Parteien: ");
#ifndef DEBUG_SAVED_GAME
  indicator_reset (n);
#endif      

  while (--n >= 0)
    {
#ifndef DEBUG_SAVED_GAME
      indicator_count_down (n);
#endif      
#ifdef DEBUG_SAVED_GAME
      indicator_tick ();
#endif      

      f = cmalloc (sizeof (faction));
      memset (f, 0, sizeof (faction));

      f->no = ri ();
      mrs (&f->name);
      mrs (&f->addr);
      mrs (&f->passw);
      f->lastorders = ri ();
      if (data_version > V24)
	{
	  f->newbie = ri ();
	  f->old_value = ri ();
	}
      if (data_version > V50)
	f->language = ri ();
      f->options = ri ();

      for (i = 0; i != MAXSPELLS; i++)
	if (data_version >= V60 || i <= SP_12)
	  /* Ab 6.0 gibt es SP_13 bis SP_17 neu. */
	  f->showdata[i] = ri ();

      p = ri ();
      rfp = &f->allies;

      while (--p >= 0)
        {
          rf = cmalloc (sizeof (rfaction));
          rf->factionno = ri ();
          addlist2 (rfp, rf);
        }

      *rfp = 0;

      if (lean)
	{
	  skipstrlist (/* &f->mistakes */);
	  skipstrlist (/* &f->warnings */);
	  skipstrlist (/* &f->messages */);
	  skipstrlist (/* &f->battles */);
	  skipstrlist (/* &f->events */);
	  skipstrlist (/* &f->income */);
	  skipstrlist (/* &f->commerce */);
	  skipstrlist (/* &f->production */);
	  skipstrlist (/* &f->movement */);
	  skipstrlist (/* &f->debug */);
	}
      else
	{
	  rstrlist (&f->mistakes);
	  rstrlist (&f->warnings);
	  rstrlist (&f->messages);
	  rstrlist (&f->battles);
	  rstrlist (&f->events);
	  rstrlist (&f->income);
	  rstrlist (&f->commerce);
	  rstrlist (&f->production);
	  rstrlist (&f->movement);
	  rstrlist (&f->debug);
	}
#ifdef DEBUG_SAVED_GAME
      if (findfaction (f->no))
	{
	  printf ("\nPartei Nr. %d kommt doppelt vor.\n-", f->no);
	  error=1;
	}
#endif      
      addlist2 (fp, f);
    }
  putchar ('\n');

  *fp = 0;

  /* Regionen */
  n = ri ();
  assert (n);
  rp = &regions;

  printf ("    Regionen: ");
#ifndef DEBUG_SAVED_GAME
  indicator_reset (n);
#endif      

  while (--n >= 0)
    {

#ifndef DEBUG_SAVED_GAME
      indicator_count_down (n);
#endif      

      r = cmalloc (sizeof (region));
      memset (r, 0, sizeof (region));

      r->x = ri ();
      r->y = ri ();
      if (data_version < V60)
	r->z = 0;
      else
	r->z = ri ();
      mrs (&r->name);
      mrs (&r->display);
      r->terrain = ri ();
      r->trees = ri ();
      r->horses = ri ();
      r->peasants = ri ();
      r->money = ri ();
      r->road = ri ();

      assert (r->x == sphericalx (r->x));
      assert (r->y == sphericaly (r->y));
      assert (r->terrain >= 0);
      assert (r->trees >= 0);
      assert (r->horses >= 0);
      assert (r->peasants >= 0);
      assert (r->money >= 0);
      assert (r->road >= 0);

      /* Test ob die Region doppelt vorkommt.  Falls ja: Einheiten,
         Burgen und Schiffe dieser Region mit der bestehenden Region
         zusammenlegen.  Dies wurde eingebaut, um den Bug mit dem
         unnoetigen Aufrufen von makeblock aus show_map heraus
         auszubuegeln.  */
      region_will_be_deleted = 0;
      r2 = findregion (r->x, r->y, r->z);
      if (r2)
	{
	  printf ("%s (%d) kommt doppelt vor.\n", regionid(r), r->z);
	  printf ("Sie wird mit %s zusammengelegt.\n", regionid(r2));
	  /* Das uebertragen von Dingen von einer Region in die andere
	     waerend dem Lesen wuerde einige Aenderungen am
	     bestehenden code erfordern.  Deswegen dieses unsaubere
	     Loesung mit dem region_will_be_deleted.  Dafuer lassen
	     sich auch alle Aenderungen leicht lokalisieren.  */
	  region_will_be_deleted = 1;
	}
      else
	{
	  /* Die Region kommt nicht in die Liste aller Regionen, damit
             sie noch in dieser Funktion wieder entfernt werden kann.
             Nach dem Lesen aller nachfolgender Burgen, Schiffe und
             Einheiten werden nachdem zB. alle Burgen der Region r
             gelesen wurden diese von r zu r2 transferiert, so dass
             ganz am Ende die Region r ohne Zugemuese dasteht und
             geloescht werden kann.  */
	  addlist2 (rp, r);
	}

      for (i = 0; i != MAXLUXURIES; i++)
	{
	  r->demand[i] = ri ();
	  if (data_version < V40)
	    {
	      if (r->demand[i])
		r->demand[i] *= 100;
	      else
		{
		  r->demand[i] = 100;
		  r->produced_good = i;
		}
	    }
	}
      if (data_version >= V40)
	r->produced_good = ri ();

      rstrlist (&r->comments);
      rstrlist (&r->debug);

      /* Burgen */
      p = ri ();
      bp = &r->buildings;

      while (--p >= 0)
        {
          b = cmalloc (sizeof (building));
          memset (b, 0, sizeof (building));

          b->no = ri ();
          mrs (&b->name);
          mrs (&b->display);
          b->size = ri ();

#ifdef DEBUG_SAVED_GAME
	  if (findbuilding (b->no))
	    {
	      printf ("\nBurg Nr. %d kommt doppelt vor.\n-", b->no);
	      error=1;
	    }
#endif      
          addlist2 (bp, b);
        }

      *bp = 0;

      /* Burgen von r in die urspruengliche Region r2 bewegen.  */ 
      if (region_will_be_deleted)
	for (;r->buildings;)
	  translist (&r->buildings, &r2->buildings, r->buildings);

      /* Schiffe */
      p = ri ();
      shp = &r->ships;

      while (--p >= 0)
        {
          sh = cmalloc (sizeof (ship));
          memset (sh, 0, sizeof (ship));

          sh->no = ri ();
          mrs (&sh->name);
          mrs (&sh->display);
          sh->type = ri ();
          sh->left = ri ();

#ifdef DEBUG_SAVED_GAME
	  if (findship (sh->no))
	    {
	      printf ("\nSchiff Nr. %d kommt doppelt vor.\n-", sh->no);
	      error=1;
	    }
#endif      
          addlist2 (shp, sh);
        }

      *shp = 0;

      /* Schiffe von r in die urspruengliche Region r2 bewegen.  */ 
      if (region_will_be_deleted)
	for (;r->ships;)
	  translist (&r->ships, &r2->ships, r->ships);

      /* Einheiten */
      p = ri ();
      up = &r->units; /* up zeigt immer auf die letzte Einheit.  */

      while (--p >= 0)
        {
#ifdef DEBUG_SAVED_GAME
	  indicator_tick ();
#endif 
          u = cmalloc (sizeof (unit));
          memset (u, 0, sizeof (unit));

          u->no = ri ();
	  if (u->no > highest_unit_no)
	    highest_unit_no = u->no;
          mrs (&u->name);
          mrs (&u->display);
          u->number = ri ();
          u->type = ri ();
          u->money = ri ();
	  u->effect = ri ();
          if (data_version < V24 && u->type != U_ILLUSION)
	    u->effect = 0;
          if (data_version >= V24)
	    u->enchanted = ri ();
          u->faction = findfaction (ri ());
          u->building = findbuilding (ri ());
          u->ship = findship (ri ());
          u->owner = ri ();
          u->status = ri ();
          u->guard = ri ();

	  /* Default Befehle gibt es nur fuer Menschen!  rs (buf)
             bedeuted, dass dass der gelesene String einfach
             weggeworfen wird.  Dort sollte sowieso nichts stehen (bei
             Monstern).  */
	  if (u->type == U_MAN)
	    mrs (&u->lastorder);
	  else
	    rs (buf);
	  if (data_version >= V32)
	    {
	      if (u->type == U_MAN)
		mrs (&u->thisorder2);
	      else
		rs (buf);
	    }

          u->combatspell = ri ();

#ifdef DEBUG_SAVED_GAME
	  if (findunitglobal (u->no))
	    {
	      printf ("\nEinheit Nr. %d kommt doppelt vor.\n-", u->no);
	      error=1;
	    }
#endif      

          assert (u->number >= 0);
          assert (u->money >= 0);
          assert (u->type >= 0);
          assert (u->effect >= 0);

          for (i = 0; i != MAXSKILLS; i++)
	    u->skills[i] = ri ();

          for (i = 0; i != MAXITEMS; i++)
	    u->items[i] = ri ();

          for (i = 0; i != MAXSPELLS; i++)
	    if (data_version >= V60 || i <= SP_12)
	      /* Ab 6.0 gibt es SP_13 bis SP_17 neu. */
	      u->spells[i] = ri ();

          if (data_version < V20 && u->owner && r->units)
            {
              /* Eigentuemer am Anfang, falls sie nicht am Anfang sind */

              u->next = r->units;
              r->units = u;
            }
          else
            addlist2 (up, u);
        }

      *up = 0;

      /* Einheiten von r in die urspruengliche Region r2 bewegen.  */ 
      if (region_will_be_deleted)
	for (;r->units;)
	  translist (&r->units, &r2->units, r->units);

      /* Speicher fuer die unbenoetigte Region freigeben.  */
      if (region_will_be_deleted)
	{
	  assert (r->units == 0);
	  assert (r->buildings == 0);
	  assert (r->ships == 0);
	  printf("%s wurde wieder entfernt.\n", regionid(r));
	  free (r);
	}
  
    }
  putchar ('\n');

  *rp = 0;

  /* Link rfaction structures */

  puts ("- Daten der Parteien durchgehen...");

  for (f = factions; f; f = f->next)
    for (rf = f->allies; rf; rf = rf->next)
      rf->faction = findfaction (rf->factionno);

  for (r = regions; r; r = r->next)
    {
      /* Initialize faction seendata values */

      for (u = r->units; u; u = u->next)
        for (i = 0; i != MAXSPELLS; i++)
          if (u->spells[i])
            u->faction->seendata[i] = 1;

      /* Check for alive factions */

      for (u = r->units; u; u = u->next)
        u->faction->alive = 1;

    }

  connectregions ();
  fclose (F);

#ifdef DEBUG_SAVED_GAME
  number_space_free ();
#endif      
  assert (!error);
}

/* ------------------------------------------------------------- */

int
value (faction *f)
{
  return (f->number * 50 + f->money);
}

int
growth (faction *f)
{
  int n;

  n = 100 * (value (f) - f->old_value);
  if (f->old_value)
    return (n / f->old_value);
  else if (value (f))
    return (n / value (f));
  else
    return n;
  }
/* ------------------------------------------------------------- */

void
wc (int c)
{
  fputc (c, F);
}

void
wsn (char *s)
{
  while (*s)
    wc (*s++);
}

void
wnl (void)
{
  wc ('\n');
}

void
wspace (void)
{
  wc (' ');
}

void
ws (char *s)
{
  wc ('"');
  if (s)
    wsn (s);
  wc ('"');
}

void
wi (int n)
{
  sprintf (buf, "%d", n);
  wsn (buf);
}

void
wstrlist (strlist * S)
{
  wi (listlen (S));
  wnl ();

  while (S)
    {
      ws (S->s);
      wnl ();
      S = S->next;
    }
}

/* Vor dem Aufruf von writegame muss writesummary () aufgerufen
   werden, damit der Wert value () der Partei berechnet werden kann
   (unter Verwendung von f->money etc).  */
void
writegame (void)
{
  int i;
  faction *f;
  rfaction *rf;
  region *r;
  building *b;
  ship *sh;
  unit *u;

  sprintf (buf, "data/%d", turn);
  if (!cfopen (buf, "w"))
    return;

  printf ("Schreibe die %d. Runde...\n", turn);

  /* globale Variablen */
  wi (RELEASE_VERSION);
  wnl ();
  wi (turn);
  wnl ();

  /* Parteien */
  wi (listlen (factions));
  wnl ();

  for (f = factions; f; f = f->next)
    {
      wi (f->no);
      wspace ();
      ws (f->name);
      wspace ();
      ws (f->addr);
      wspace ();
      ws (f->passw);
      wspace ();
      wi (f->lastorders);
      wspace ();
      wi (f->newbie);
      wspace ();
      wi (value(f));
      wspace ();
      wi (f->language);
      wspace ();
      wi (f->options);
      wnl ();

      for (i = 0; i != MAXSPELLS; i++)
        {
          wspace ();
          wi (f->showdata[i]);
        }
      wnl ();

      wi (listlen (f->allies));
      for (rf = f->allies; rf; rf = rf->next)
        {
          wspace ();
          wi (rf->faction->no);
        }
      wnl ();

      wstrlist (f->mistakes);
      wstrlist (f->warnings);
      wstrlist (f->messages);
      wstrlist (f->battles);
      wstrlist (f->events);
      wstrlist (f->income);
      wstrlist (f->commerce);
      wstrlist (f->production);
      wstrlist (f->movement);
      wstrlist (f->debug);

      wnl (); /* plus leerzeile */
    }

  /* Regionen */

  wi (listlen (regions));
  wnl ();

  for (r = regions; r; r = r->next)
    {
      wnl (); /* plus leerzeile */

      wi (r->x);
      wspace ();
      wi (r->y);
      wspace ();
      wi (r->z);
      wspace ();
      ws (r->name);
      wspace ();
      ws (r->display);
      wspace ();
      wi (r->terrain);
      wspace ();
      wi (r->trees);
      wspace ();
      wi (r->horses);
      wspace ();
      wi (r->peasants);
      wspace ();
      wi (r->money);
      wspace ();
      wi (r->road);
      wnl ();

      assert (r->terrain >= 0);
      assert (r->trees >= 0);
      assert (r->horses >= 0);
      assert (r->peasants >= 0);
      assert (r->money >= 0);
      assert (r->road >= 0);

      for (i = 0; i != MAXLUXURIES; i++)
        {
          wspace ();
          wi (r->demand[i]);
        }
      wspace ();
      wi (r->produced_good);
      wnl ();

      wstrlist (r->comments);
      wstrlist (r->debug);

      wi (listlen (r->buildings));
      wnl ();
      for (b = r->buildings; b; b = b->next)
        {
          wspace ();
          wi (b->no);
          wspace ();
          ws (b->name);
          wspace ();
          ws (b->display);
          wspace ();
          wi (b->size);
          wnl ();
        }

      wi (listlen (r->ships));
      wnl ();
      for (sh = r->ships; sh; sh = sh->next)
        {
          wspace ();
          wi (sh->no);
          wspace ();
          ws (sh->name);
          wspace ();
          ws (sh->display);
          wspace ();
          wi (sh->type);
          wspace ();
          wi (sh->left);
          wnl ();
        }

      wi (listlen (r->units));
      wnl ();
      for (u = r->units; u; u = u->next)
        {
          wspace ();
          wi (u->no);
          wspace ();
          ws (u->name);
          wspace ();
          ws (u->display);
          wspace ();
          wi (u->number);
          wspace ();
          wi (u->type);
          wspace ();
          wi (u->money);
          wspace ();
          wi (u->effect);
          wspace ();
          wi (u->enchanted);
          wspace ();
          wi (u->faction->no);
          wspace ();
          if (u->building)
            wi (u->building->no);
          else
            wi (0);
          wspace ();
          if (u->ship)
            wi (u->ship->no);
          else
            wi (0);
          wspace ();
          wi (u->owner);
          wspace ();
          wi (u->status);
          wspace ();
          wi (u->guard);
          wspace ();
          ws (u->lastorder);
          wspace ();
          ws (u->thisorder2);
          wspace ();
          wi (u->combatspell);
          wnl ();

          assert (u->number >= 0);
          assert (u->money >= 0);
          assert (u->type >= 0);
          assert (u->effect >= 0);

          for (i = 0; i != MAXSKILLS; i++)
            {
              wspace ();
              wi (u->skills[i]);
            }
          wnl ();

          for (i = 0; i != MAXITEMS; i++)
            {
              wspace ();
              wi (u->items[i]);
            }
          wnl ();

          for (i = 0; i != MAXSPELLS; i++)
            {
              wspace ();
              wi (u->spells[i]);
            }
          wnl ();
        }
    }
  fclose (F);
}

/* ------------------------------------------------------------- */

void
output_addresses (void)
{
  faction *f;

  /* adressen liste */

  for (f = factions; f; f = f->next)
    {
      rparagraph (factionid (f), 0, 0);
      if (f->addr)
	rparagraph (f->addr, 4, 0);
      fputs ("\n", F);
    }
}

void
showaddresses ()
{
  F = stdout;
  output_addresses ();
}

void
writeaddresses ()
{
  sprintf (buf, "%s.%d", ADDRESS_BASENAME, turn);
  if (!cfopen (buf, "w"))
    return;
  printf ("Schreibe Liste der Adressen (%s)...\n", buf);
  output_addresses ();
  fclose (F);
}

long int
mem_str (char *s)
{
  return s ? strlen (s) : 0;
}

long int
mem_strlist (strlist *S)
{
  long int m=0;
  while (S)
    {
      m += mem_str (S->s);
      S = S->next;
    }
  return m;
}

void
writesummary (void)
{
  int inhabitedregions=0;
  int peasants=0;
  int peasantmoney=0;
  int nunits=0;
  int playerpop=0;
  int playermoney=0;
  int armed_men=0;
  int i, nmrs[ORDERGAP], newbies=0;
  faction *f;
  region *r, *r2;
  unit *u;
#ifdef DEBUG_MEMORY_USAGE
  long int mem_factions=0;
  long int mem_regions=0;
  long int mem_buildings=0;
  long int mem_ships=0;
  long int mem_units=0;
  long int mem_orders=0;
  runit *ru;
  building *b;
  ship *sh;
  rfaction *rf;
#endif

  sprintf (buf, "%s.%d", SUMMARY_BASENAME, turn);
  if (!cfopen (buf, "w"))
    return;
  printf ("Schreibe Zusammenfassung (%s)... ", buf);

  for (f = factions; f; f = f->next)
    {
      indicator_tick ();
      f->nregions = 0;
      f->nunits = 0;
      f->number = 0;
      f->money = 0;
#ifdef DEBUG_MEMORY_USAGE
      mem_factions += sizeof (faction);
      mem_factions += mem_str (f->name);
      mem_factions += mem_str (f->addr);
      mem_factions += mem_str (f->passw);
      for (rf = f->allies; rf; rf = rf->next)
	mem_factions += sizeof (rfaction);
      mem_factions += mem_strlist (f->mistakes);
      mem_factions += mem_strlist (f->warnings);
      mem_factions += mem_strlist (f->messages);
      mem_factions += mem_strlist (f->battles);
      mem_factions += mem_strlist (f->events);
      mem_factions += mem_strlist (f->income);
      mem_factions += mem_strlist (f->commerce);
      mem_factions += mem_strlist (f->production);
      mem_factions += mem_strlist (f->movement);
      mem_factions += mem_strlist (f->debug);
#endif
    }
      
  /* Regionen durchgehen */
  for (r = regions; r; r = r->next)
    {

      /* Kleiner Test um zu kontrollieren, dass keine falschen
	 Regionen entstanden sind.  */
      
      r2 = findregion (r->x, r->y, r->z);
      if (r2 != r)
	printf ("\n%s (%d,%d,%d) kommt doppelt vor.",
		regionid(r), r->x, r->y, r->z);

      /* Bauern und Einheiten zaehlen. */ 

      if (r->peasants || r->units)
	{
	  indicator_tick ();
	  inhabitedregions++;
	  peasants += r->peasants;
	  peasantmoney += r->money;

#ifdef DEBUG_MEMORY_USAGE
	  mem_regions += sizeof (region);
	  mem_regions += mem_str (r->name);
	  mem_regions += mem_str (r->display);
	  mem_regions += mem_strlist (r->comments);
	  mem_regions += mem_strlist (r->debug);
	  for (sh = r->ships; sh; sh = sh->next)
	    {
	      mem_ships += sizeof (ship);
	      mem_ships += mem_str (sh->name);
	      mem_ships += mem_str (sh->display);
	    }
	  for (b = r->buildings; b; b = b->next)
	    {
	      mem_buildings += sizeof (building);
	      mem_buildings += mem_str (b->name);
	      mem_buildings += mem_str (b->display);
	    }
#endif

	  /* nregions darf nur einmal pro Partei per Region
	     incrementiert werden. */
	  for (f = factions; f; f = f->next)
	    f->dh = 0;
	  for (u = r->units; u; u = u->next)
	    {
	      nunits++;
	      playerpop += u->number;
	      playermoney += u->money;
	      armed_men += armedmen (u);

	      u->faction->nunits++;
	      u->faction->number += u->number;
	      u->faction->money += u->money;

	      if (!u->faction->dh)
		u->faction->nregions++;
	      u->faction->dh = 1;

#ifdef DEBUG_MEMORY_USAGE
	  mem_units += sizeof (unit);
	  mem_units += mem_str (u->name);
	  mem_units += mem_str (u->display);
	  for (ru = u->contacts; ru; ru = ru->next)
	    mem_units += sizeof (runit);
	  mem_orders += mem_strlist (u->orders);
#endif
	    }
	}
    }

  fprintf (F, "Zusammenfassung fuer Atlantis %s\n\n",
           gamedate (findfaction (0)));

  fprintf (F, "Regionen:\t\t%d\n", listlen (regions));
  fprintf (F, "Bewohnte Regionen:\t%d\n\n", inhabitedregions);

  fprintf (F, "Parteien:\t\t%d\n", listlen (factions));
  fprintf (F, "Einheiten:\t\t%d\n\n", nunits);

  fprintf (F, "Spielerpopulation:\t%d\n", playerpop);
  fprintf (F, " davon bewaffnet:\t%d\n", armed_men);
  fprintf (F, "Bauernpopulation:\t%d\n", peasants);
  fprintf (F, "Population gesamt:\t%d\n\n", playerpop + peasants);

  fprintf (F, "Reichtum Spieler:\t$%d\n", playermoney);
  fprintf (F, "Reichtum Bauern:\t$%d\n", peasantmoney);
  fprintf (F, "Reichtum gesamt:\t$%d\n\n", playermoney + peasantmoney);

#ifdef DEBUG_MEMORY_USAGE
  fputs ("Belegter Speicher\n\n", F);
  fprintf (F, "fuer Parteien:\t\t%ld\n", mem_factions);
  fprintf (F, "fuer Regionen:\t\t%ld\n", mem_regions);
  fprintf (F, "fuer Burgen:\t\t%ld\n", mem_buildings);
  fprintf (F, "fuer Schiffe:\t\t%ld\n", mem_ships);
  fprintf (F, "fuer Einheiten:\t\t%ld\n", mem_units);
  fprintf (F, "fuer Befehle:\t\t%ld\n\n", mem_orders);
#endif

  /* NMRs und Newbies zaehlen. */  
  for (i = 0; i != ORDERGAP; i++)
    nmrs[i] = 0;
  newbies = 0;
  for (f = factions; f; f = f->next)
    {
      if (f->newbie)
	newbies++;
      else
	nmrs[turn - f->lastorders]++;
    }
  for (i = 0; i != ORDERGAP; i++)
    fprintf (F, "%d %s\t\t\t%d\n", i,
             i != 1 ? "NMRs:" : "NMR: ", nmrs[i]);
  fprintf (F, "Newbies:\t\t%d\n", newbies);

  if (factions)
    fprintf (F, "\nNr.\tReg.\tEinh.\tPers.\tSilber\tWert\tZuwachs\tNMRs\n\n");
  for (f = factions; f; f = f->next)
    fprintf (F, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
             f->no, f->nregions, f->nunits, f->number,
	     f->money, value (f), growth (f),
	     f->newbie ? -1 : (turn - f->lastorders));

  fclose (F);
  putchar ('\n');
}

/* ------------------------------------------------------------- */

void
initgame (int lean)
{
  int max_turn = -1;
  DIR *dp;
  struct dirent *ep;

  /* Hier werden alle Dateien im DATA directory untersucht: Ihr Name
     wird in eine Nummer umgewandelt und die Variable max_turn enthaelt
     die hoechste dieser Nummern. In readgame () wird im Normalfall die
     Datei mit diesem Namen (der hoechsten Nummer!) geoeffnet und
     verwendet. Wird turn schon gesetzt (turn != -1), dann gibt es nur
     eine Warnung, falls es noch hoehere Nummern gibt. */
  dp = opendir ("data/");
  if (dp != NULL)
    {
      while ((ep = readdir (dp)))
	{
	  if (atoi (ep->d_name) > max_turn && ep->d_name[0] >= '0' && ep->d_name[0] <= '9')
	    max_turn = atoi (ep->d_name);
	}
      closedir (dp);
    }
  else
    mkdir ("data/", S_IRWXU);

  /* Falls turn schon gesetzt wurde (turn != -1), wird versucht, den
     file mit der Nr. turn zu lesen, ansonsten wird file Nr. max_turn
     verwendet, also der Letzte. Bei Fehlern waehrend dem Lesen des
     Spieles in readgame () wird mit exit (1) beendet. Vgl. auch
     Funktionen rs () und ri (), deswegen brechen wir hier bei einem
     Fehler auch mit exit (1) ab. */

  /* Test, ob es ueberhaupt die verlangete Nr. turn geben kann */
  if (turn != -1 && turn > max_turn)
    {
      printf ("Es gibt nur Datenfiles bis Runde %d.\n", max_turn);
      exit (1);
    }

  /* Wird keine Datei gefunden, wird ein neues Spiel erzeugt, und keine
     Datei gelesen. */
  if (max_turn == -1)
    {
      turn = 0;

      puts ("Keine Spieldaten gefunden, erzeuge neues Spiel...\n");
      mkdir ("data", 0);

      createmonsters ();
      makeblock (0, 0, 0);

      /* Die neu erschaffenen Strukturen werden abgespeichert. */
      writesummary ();
      writegame ();
      return;
    }

  /* Das file Nr. turn wird gelesen, aber es gibt hoehere Nummern, also
     warnen wir. */
  if (turn != -1 && max_turn > turn)
    printf ("Lese Datenfile %d, obwohl %d der Letzte ist.\n",
            turn, max_turn);

  /* Nun wird das Datenfile Nr. turn bzw. max_turn gelesen. */
  if (turn == -1)
    turn = max_turn;

  printf ("Lese Datenfile %d ...\n", turn);
  readgame (lean);

  /* Initialisieren des random number generators. */
  srand (time (0));
}

