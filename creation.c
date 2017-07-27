/* German Atlantis PB(E)M host Copyright (C) 1995-1999  Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "atlantis.h"

/* Liste von Regionennamen, wird nur hier eingebaut. */
#include "names.inc"
 
#define INIT_PASSWORD_LENGTH     5

/* Eine Insel pro 9x9 Feld.  Das erste Feld von (0,0) bis (8,8).  */
#define BLOCKSIZE       9

/* Entweder eine grosse Insel (Chance 2/3) mit 31 bis 40 Felder oder eine
   kleine Insel (Chance 1/3) mit 11 bis 39 Feldern. */
#define ISLANDSIZE      ((rand()%3)?(31+rand()%10):(11+rand()%20))

/* Ozean und Grasland wird automatisch generiert (ein Ozean, darin
   eine Insel aus Grasland). Jedes Seed generiert ein Feld des
   entsprechenden Terrains, und max. 3 angrenzende Felder. Details in
   der Funktion seed () und transmute () in REGIONS.C.  Die Anzahl
   Inseln auf der Welt (falls sie sphaerisch ist) bestimmt man in
   spericalx () und sphericaly ().  Dabei climate (/) nicht vergessen,
   dort wird bestimmt, zu welcher y Koordinate welches Klima gehoert.
   Das Klima bestimmt das Verhaeltniss der einzelnen Terrains auf der
   Insel.  */

/* ------------------------------------------------------------- */

enum
  {
    C_PACIFIC,
    C_TROPIC,
    C_DRY,
    C_TEMPERATE,
    C_COOL,
    C_ARCTIC,
    MAXCLIMATES,
  };

/* OCEAN, PLAIN, FOREST, SWAMP, DESERT, HIGHLAND, MOUNTAIN, GLACIER,
   MAXTERRAINS */

char maxseeds[MAXCLIMATES][MAXTERRAINS] =
{
  {3, 0, 4, 0, 0, 1, 0, 0,},
  {0, 0, 6, 4, 0, 2, 2, 0,},
  {0, 0, 1, 0, 6, 2, 2, 0,},
  {0, 0, 3, 2, 0, 2, 3, 1,},
  {0, 0, 3, 3, 0, 1, 4, 5,},
  {0, 0, 0, 2, 0, 0, 5,12,},
};

int
climate (int y)
{

  /* Abfolge:    y/BLOCKSIZE    +2  abs %10 -5 abs

        TEMPERATE        -4     -2   2   2  -3  3
        COOL             -3     -1   1   1  -4  4
      > ARCTIC           -2      0   0   0  -5  5
        COOL             -1      1   1   1  -4  4
        TEMPERATE  (0,0)  0      2   2   2  -3  3
        DRY               1      3   3   3  -2  2
        TROPIC            2      4   4   4  -1  1
      < PACIFIC           3      5   5   5   0  0
        TROPIC            4      6   6   6   1  1 
        DRY               5      7   7   7   2  2 */

  return abs( abs( (y/BLOCKSIZE) + 2) % 10 - 5);
}

int random_monster[MAXTYPES] =
{
  0,
  10, /* U_UNDEAD */
  0,
  30, /* U_FIREDRAGON */
  20, /* U_DRAGON */
  10, /* U_WYRM */
  30, /* U_GUARDS */
};

/* Transformiert die Koordinaten in Koordinaten auf einer runden Welt.
   Soll Atlantis nicht rund sein, dann kann man hier einfach ein
   inline `return coord;' machen.  Die Klimazonen Verteilung im
   Origignal GA hatte Grenzen von -4 bis 5, dh. die Ecken lagen bei
   (-36,-36) und (53,53).  In diesem Fall sind lb=-36 (-4x9) und ub=54
   ((5+1)x9).  */
int
sphericalx (int coord)
{
  int lb=-4 * BLOCKSIZE;          /* lower boundary.  */
  int ub=(5 +1) * BLOCKSIZE;     /* upper boundary.  */

  if (coord < lb) 
    /* coord=-37 -> (54)-[(-36)-(-37)]=53.  */
    /* Falls lb=0 und ub=9, coord=-1 -> (9)-[(0)-(-1)]=8.  */
      coord = ub - (lb - coord);
  else if (coord >= ub) 
     /* coord=54 -> (-36)+[(54)-(54)]=-36.  */
     /* coord=55 -> (-36)+[(55)-(54)]=-35.  */
     /* Falls lb=0 und ub=9, coord=9 -> (0)+[(9)-(9)]=0.  */
     /* Falls lb=0 und ub=9, coord=10 -> (0)+[(10)-(9)]=1.  */
      coord = lb + (coord - ub);
  return coord;
}

int
sphericaly (int coord)
{
  /* Die Welt ist torsoidal (dh. wenn man oben ueber den Rand faehrt
     kommt man unten wieder heraus, dasselbe mit links und rechts),
     und die gesamte Karte ist quadratisch, also sind sphericalx ()
     und sphericaly () im Moment noch gleich.  */
  return sphericalx (coord);
}

/* ------------------------------------------------------------- */

region *
inputregion (void)
{
  region *r;
  int x, y, z;

  printf ("X? ");
  gets (buf);
  if (buf[0] == 0)
    return 0;
  x = atoi (buf);

  printf ("Y? ");
  gets (buf);
  if (buf[0] == 0)
    return 0;
  y = atoi (buf);

  printf ("Z? ");
  gets (buf);
  if (buf[0] == 0)
    return 0;
  z = atoi (buf);

  r = findregion (x, y, z);

  if (!r)
    {
      puts ("Diese Region existiert nicht.");
      return 0;
    }

  return r;
}

/* ------------------------------------------------------------- */

void
createmonsters (void)
{
  faction *f;

  if (findfaction (0))
    {
      puts ("Die Monster Partei gibt es schon.");
      return;
    }

  f = cmalloc (sizeof (faction));
  memset (f, 0, sizeof (faction));

  /* alles ist auf null gesetzt, ausser dem folgenden. achtung - partei no 0
     muss keine orders einreichen! */

  mnstrcpy (&f->name, "Monster", NAMESIZE);
  f->alive = 1;
  f->options = pow (2, O_REPORT);
  addlist (&factions, f);
}

/* ------------------------------------------------------------- */

char 
random_letter (void)
{
  return 'a' + random () % 26;
}

void 
generate_password (faction *f)
{
  int i;
  for (i = 0; i != INIT_PASSWORD_LENGTH; i++)
    buf[i] = random_letter ();
  buf[i] = 0;
  mnstrcpy (&f->passw, buf, INIT_PASSWORD_LENGTH);
}

void
addaplayerat (region * r)
{
  faction *f;
  unit *u;

  f = cmalloc (sizeof (faction));
  memset (f, 0, sizeof (faction));

  /* Die Adresse steht in buf (vgl. fkt. addplayers) */

  mnstrcpy (&f->addr, buf, DISPLAYSIZE);

  /* Passwort auf ein Zufallscode setzen, letzter Befehl diese Runde
     und am leben */

  generate_password (f);
  f->lastorders = turn;
  f->alive = 1;
  f->newbie = 1;
  f->options = pow (2, O_REPORT) + pow (2, O_STATISTICS)
    + pow (2, O_ZINE) + pow (2, O_COMMENTS);

  /* Setze die Nummer und den Namen fuer die Partei auf die erste freie
     Nummer */

  do
    f->no++;
  while (findfaction (f->no));

  sprintf (buf, "Nummer %d", f->no);
  mnstrcpy (&f->name, buf, NAMESIZE);
  addlist (&factions, f);

  /* erzeuge zugehoerige Einheit */

  u = createunit (r);
  u->number = 1;
  u->money = STARTMONEY;
  u->faction = f;
}

void
addplayers (void)
{
  region *r;

  r = inputregion ();

  /* Die Spieler erscheinen auf dieser Stelle im Spiel */
  /* falls keine Region eingegeben wurde */
  if (!r)
    return;

  /* falls die Region ein Ozean ist */
  if (r->terrain == T_OCEAN)
    {
      puts ("Diese Gegend liegt unter dem Meeresspiegel!");
      return;
    }

  printf ("Datei mit den Spieleradressen? ");
  gets (buf);

  /* wenn buf nirgendswohin zeigt, wurde keine Datei angegeben. */
  if (!buf[0])
    {
      /* und deswegen fragen wir nun einfach direkt nach dem Namen. */
      printf ("OK. Name bzw. Adresse des Spielers? ");
      gets (buf);

      /* wieder ohne Antwort, beenden wir die Funktion */
      if (!buf[0])
        return;
      else
        addaplayerat (r);
    }
  else
    {
      if (!cfopen (buf, "r"))
        return;
      for (;;)
        {
          getbuf ();

          if (buf[0] == 0 || buf[0] == EOF)
            {
              fclose (F);
              break;
            }

          addaplayerat (r);
        }
      puts ("Es empfiehlt sich, hier noch Wachen aufzustellen!");
      addunit (U_GUARDS);
    }
}

/* ------------------------------------------------------------- */

void
connecttothis (region * r, int x, int y, int z, int from, int to)
{
  region *r2;

  r2 = findregion (x, y, z);

  if (r2)
    {
      r->connect[from] = r2;
      r2->connect[to] = r;
    }
}

void
connectregions (void)
{
  region *r;
  int i;

  for (r = regions; r; r = r->next)
    for (i = 0; i != MAXDIRECTIONS; i++)
      if (!r->connect[i])
        connecttothis (r, sphericalx (r->x + delta_x[i]), 
		       sphericaly (r->y + delta_y[i]),
		       r->z,
                       i, back[i]);
}

/* ------------------------------------------------------------- */

void
listnames (void)
{
  region *r;
  int i;

  puts ("Die Liste der benannten Regionen ist:");

  i = 0;
  for (r = regions; r; r = r->next)
    if (r->terrain != T_OCEAN)
      {
        printf ("%s,", r->name ? r->name : "Das unbenannte Land");
        i += strlen (r->name) + 1;

        while (i % 15)
          {
            printf (" ");
            i++;
          }

        if (i > 60)
          {
            printf ("\n");
            i = 0;
          }
      }
  printf ("\n");
}

int
regionnameinuse (char *s)
{
  region *r;

  for (r = regions; r; r = r->next)

    /* hier darf es case-insensitive sein, da die Namen nie vom Benutzer
       eingegeben werden. */

    if (r->name && !strcmp (r->name, s))
      return 1;

  return 0;
}

void
nameregion (region * r)
{
  /* Zaehlt die benutzten Namen */

  int i, n;
  region *r2;

  n = 0;
  for (r2 = regions; r2; r2 = r2->next)
    if (r2->name)
      n++;

  /* Vergibt einen Namen */

  i = rand () % (sizeof regionnames / sizeof (char *));

  /* Falls es noch genug unbenutzte Namen gibt, wird solange ein weiterer
     genommen, bis einer der unbenutzten gefunden wurde. */

  if (n < 0.8 * sizeof regionnames / sizeof (char *))
    while (regionnameinuse (regionnames[i]))
        i = rand () % (sizeof regionnames / sizeof (char *));

  mnstrcpy (&r->name, regionnames[i], NAMESIZE);
}

void
terraform (region * r, int terrain)
{
  /* defaults: */

  r->terrain = terrain;
  r->trees = 0;
  r->horses = 0;
  r->peasants = 0;
  r->money = 0;

  if (terrain != T_OCEAN)
    {
      /* jede r ausser ozean hat einen namen. */

      nameregion (r);

      switch (terrain)
        {
        case T_PLAIN:

          /* 0-200 pferde in Ebenen */

          r->horses = (rand () % (production[terrain] / 5));
          break;

        case T_FOREST:

          /* waelder per se gibt es nicht, das ist eine Ebene mit vielen
             baeumen. 600-900 baueme max. */

          r->terrain = T_PLAIN;
          r->trees = production[T_PLAIN] * (60 + rand () % 30) / 100;
          break;

        }

      /* 50-80% der maximalen Anzahl an Bauern, in einer Ebene also
         5000-8000. */

        r->peasants = MAXPEASANTS_PER_AREA * production[terrain] *
                      (50 + rand () % 30) / 100;
    }
}

/* ------------------------------------------------------------- */

char newblock[BLOCKSIZE][BLOCKSIZE];

void
transmute (int from, int to, int n, int count)
{
  int i, x, y;

  /* insgesamt werden also n felder geaendert */

  do
    {
      i = 0;

      do
        {
          x = rand () % BLOCKSIZE;
          y = rand () % BLOCKSIZE;
          i += count;
        }
      while (i <= 10 && !(newblock[x][y] == from &&
                          ((x != 0 && newblock[x - 1][y] == to) ||
                         (x != BLOCKSIZE - 1 && newblock[x + 1][y] == to) ||
                           (y != 0 && newblock[x][y - 1] == to) ||
                        (y != BLOCKSIZE - 1 && newblock[x][y + 1] == to))));

      /* Wenn von seed aus gerufen: zehn versuche, um ein feld zu
         finden, welches =from ist, und neben einem feld liegt, das
         =to ist.  Wenn von makeblock aus gerufen, hat man unendlich
         lange zeit, um danach zu suchen.  Aufpassen, dass ISLANDSIZE
         n nie groesser als BLOCKSIZE^2 wird! */

      /* Wurde 10 mal erfolglos probiert, ein seed zu machen, brechen
         wir ab.  */

      if (i > 10)
        break;

      /* ansonsten aendern wir ein feld */

      newblock[x][y] = to;
    }
  while (--n);
}

void
seed (int to, int n)
{
  int x, y, i = 0;

  /* hier setzen wir ein T_PLAIN feld =to (20 Versuche), und rufen
     nachher transmute auf, um neben eines der =to felder n weitere
     =to felder zu setzen. Wird hier also ein wald gesaeht, wird ein
     neues wald-feld gesezt, und entweder an diesem oder an anderen
     werden n neue waldfelder angehaengt. um diese n zu setzten, hat
     man nur 10 versuche (4. parameter 1) */

  do
    {
      x = rand () % BLOCKSIZE;
      y = rand () % BLOCKSIZE;
      i++;
    }
  while (newblock[x][y] != T_PLAIN && i < 20);

  newblock[x][y] = to;
  transmute (T_PLAIN, to, n, 1);
}

int
blockcoord (int x)
{
  return (x / BLOCKSIZE) * BLOCKSIZE;
}

void
upper_left (int *x1, int *y1)
{
  int x = *x1, y = *y1;

  /* negative Koordinaten: Runterzaehlen */

  if (x < 0)
    while (x != blockcoord (x))
      x--;

  if (y < 0)
    while (y != blockcoord (y))
      y--;

  /* positive Koordinaten: Runden.  Negative Koord. stimmen schon.  */

  *x1 = blockcoord (x);
  *y1 = blockcoord (y);
}

void
makeblock (int x1, int y1, int z)
{
  /* Die z-Koordinate ist insofern speziell, als dass sie konstant
     ist.  Die x und y Koordinaten werden benoetigt, um einen block
     auf der Landkarte zu bezeichnen.  Auf diesem Block werden dann
     Regionen erzeugt -- alle auf derselben z-Ebene.  */

  int i, j, x, y, p1, p2, local_climate;
  region *r;

  /* Zuerst werden die Koordinaten auf eine Sphaere projeziert.  */
  x1 = sphericalx (x1);
  y1 = sphericaly (y1);

  local_climate = climate (y1);

  /* Links-Oben Koordinate des Quadrates, in dem sich (x1,y1)
     befinden.  */
  upper_left (&x1, &y1);

  /* ein Kontinent wird erzeugt und in newblock abgespeichert die Mitte von
     newblock wird zu Weideland. Diese Insel wird vergroessert, und dann
     werden Terrains "gesaeht" */
  memset (newblock, T_OCEAN, sizeof newblock);
  newblock[BLOCKSIZE / 2][BLOCKSIZE / 2] = T_PLAIN;
  transmute (T_OCEAN, T_PLAIN, ISLANDSIZE, 0);

  for (i = 0; i != MAXTERRAINS; i++)
    for (j = 0; j != maxseeds[local_climate][i]; j++)
      seed (i, 3);

  /* newblock wird innerhalb der Rahmen in die Karte kopiert, die Landstriche
     werden benannt und bevoelkert, und die produkte p1 und p2 des kontinentes
     werden gesetzt. */
  p1 = rand () % MAXLUXURIES;
  do
    p2 = rand () % MAXLUXURIES;
  while (p2 == p1);

  for (x = 0; x != BLOCKSIZE; x++)
    for (y = 0; y != BLOCKSIZE; y++)
      {
        r = cmalloc (sizeof (region));
        memset (r, 0, sizeof (region));

        r->x = sphericalx (x1 + x);
        r->y = sphericaly (y1 + y);
	r->z = z;

        terraform (r, newblock[x][y]);

        for (i = 0; i != MAXLUXURIES; i++)
          r->demand[i] = MINDEMAND + rand () % 500;
	r->produced_good = (rand () & 1) ? p1 : p2;
        r->demand[r->produced_good] = MINDEMAND;

        addlist (&regions, r);
      }

  connectregions ();
}

/* Drachen werden ausgesetzt, wenn die Region automatisch (durch
   einreisende Spieler) erzeugt wurde.  Dies geschieht bei Bewegungen.
   Wenn der Spielleiter Inseln erzeugt, werden Drachen *nicht*
   automatisch ausgesetzt.  */
void 
seed_monsters (int x1, int y1, int z) 
{
  region *r;
  int i, m, x, y;

  /* Zuerst werden die x und y Koordinaten auf eine Sphaere
     projeziert.  */
  x1 = sphericalx (x1);
  y1 = sphericaly (y1);

  upper_left (&x1, &y1);

  for (i=0; i != MAXTYPES; i++)
    {
      if (rand () % 100 < random_monster[i])
	{
	  do
	    {
	      x = rand () % BLOCKSIZE;
	      y = rand () % BLOCKSIZE;
	      r = findregion (x1 + x, y1 + y, z);
	    }
	  while (r->terrain == T_OCEAN);
	  printf ("   %s in %s ausgesetzt.\n", 
		  strings[typenames[1][i]][0], regionid(r));
	  switch (i)
	    {
	    case U_UNDEAD:
	      make_undead_unit (r, findfaction (0), lovar (20));
	      break;
	    case U_FIREDRAGON:
	      make_firedragon_unit (r, findfaction (0), 1);
	      break;
	    case U_DRAGON:
	      make_dragon_unit (r, findfaction (0), 1);
	      break;
	    case U_WYRM:
	      make_wyrm_unit (r, findfaction (0), 1);
	      break;
	    case U_GUARDS:
	      m = lovar (20);
	      make_guarded_tower_unit (r, findfaction (0), m, m);
	      break;
	    default:
	      assert (0);
	    }
	  break;
	}
    }
}

/* ------------------------------------------------------------- */

char
factionsymbols (region * r)
{
  faction *f;
  unit *u;
  int i = 0;

  for (f = factions; f; f = f->next)
    for (u = r->units; u; u = u->next)
      if (u->faction == f)
        {
          i++;
          break;
        }
  assert (i);

  if (i > 9)
    return 'x';
  else
    return '1' - 1 + i;
}

char
armedsymbols (region * r)
{
  unit *u;
  int i = 0;

  for (u = r->units; u; u = u->next)
    if (armedmen (u))
      return 'X';
    else
      i += u->number;
  assert (i);

  if (i > 9)
    return 'x';
  else
    return '1' - 1 + i;
}

void
output_map (int mode)
{
  int x, y, z, minx, miny, maxx, maxy, minz, maxz;
  region *r;

  minz = INT_MAX;
  maxz = INT_MIN;

  for (r = regions; r; r = r->next)
    {
      minz = min (minz, r->z);
      maxz = max (maxz, r->z);
    }

  for (z = minz; z <= maxz; z++)
    {

      minx = INT_MAX;
      maxx = INT_MIN;
      miny = INT_MAX;
      maxy = INT_MIN;

      for (r = regions; r; r = r->next)
	if (r->z == z)
	  {
	    minx = min (minx, r->x);
	    maxx = max (maxx, r->x);
	    miny = min (miny, r->y);
	    maxy = max (maxy, r->y);
	  }

      fprintf (F, "Koordinaten von (%d,%d) bis (%d,%d) der Ebene %d:\n\n",
	       minx, miny, maxx, maxy, z);

      for (y = miny; y <= maxy; y++)
	{
	  memset (buf, ' ', sizeof buf);
	  buf[maxx - minx + 1] = 0;

	  for (r = regions; r; r = r->next)
	    if (r->y == y && r->z == z)
	      {
		if (r->units && mode == M_FACTIONS)
		  buf[r->x - minx] = factionsymbols (r);
		else if (r->units && mode == M_UNARMED)
		  buf[r->x - minx] = armedsymbols (r);
		else                /* default: terrain */
		  buf[r->x - minx] = terrainsymbols[mainterrain (r)];
	      }

	  for (x = 0; buf[x]; x++)
	    {
	      if (y == 0 && x == -minx)
		fputc ('(', F);
	      else if (y == 0 && x == -minx + 1)
		fputc (')', F);
	      else
		fputc (' ', F);

	      fputc (buf[x], F);
	    }

	  fputs ("\n", F);
	}
      fputs ("\n", F);
    }

  fputs ("\nLegende:\n\n", F);
  switch (mode)
    {
    case M_TERRAIN:
      for (y = 0; y != MAXTERRAINS; y++)
        fprintf (F, "%c - %s\n", terrainsymbols[y], strings[terrainnames[y]][DEFAULT_LANGUAGE]);
      break;

    case M_FACTIONS:
      for (y = 0; y != MAXTERRAINS; y++)
        fprintf (F, "%c - %s\n", terrainsymbols[y], strings[terrainnames[y]][DEFAULT_LANGUAGE]);
      fputs ("x - mehr als 9 Parteien\n", F);
      fputs ("1-9 - Anzahl Parteien\n", F);
      break;

    case M_UNARMED:
      for (y = 0; y != MAXTERRAINS; y++)
        fprintf (F, "%c - %s\n", terrainsymbols[y], strings[terrainnames[y]][DEFAULT_LANGUAGE]);
      fputs ("X - mindestens eine bewaffnete Person\n", F);
      fputs ("x - mehr als 9 Personen\n", F);
      fputs ("1-9 - Anzahl unbewaffneter Personen\n", F);
      break;

    }
}

void
showmap (int mode)
{
  F = stdout;
  output_map (mode);
}

void
writemap (int mode)
{
  if (!cfopen ("karte", "w"))
    return;
  puts ("Schreibe Karte (karte)...");
  output_map (mode);
  fclose (F);
}

/* ------------------------------------------------------------- */

void
createcontinent (void)
{
  int x, y, z;

  /* Kontinente werden automatisch generiert! Dies ist nur so zum Testen
     gedacht. Da man die X und Y Koordinaten braucht, kann man leider nicht
     inputregion () verwenden. */

  printf ("X? ");
  gets (buf);
  if (buf[0] == 0)
    return;
  x = atoi (buf);

  printf ("Y? ");
  gets (buf);
  if (buf[0] == 0)
    return;
  y = atoi (buf);

  printf ("Z? ");
  gets (buf);
  if (buf[0] == 0)
    return;
  z = atoi (buf);

  if (!findregion (x, y, z))
    makeblock (x, y, z);
  else
    puts ("Dort gibt es schon Regionen.");
}

/* ------------------------------------------------------------- */

void
longgets (char *s)
{
  int l;
  char q = 0;

  /* so lange gets, bis nichts mehr eingegeben wird, oder die moegliche
     laenge ueberschritten wurde. */

  printf ("$ ");
  gets (s);
  while (!q && (l = strlen (s)) < DISPLAYSIZE)
    {
      s[l] = ' ';
      printf ("$ ");
      gets (s + l + 1);
      q = !((s + l + 1)[0]);
    }

  /* wurde verlaengert und mit einer leerzeile abgeschlossen, so wurde
     unguterweise ein ' ' angehaengt */

  if (q)
    s[strlen (s) - 1] = 0;

}

/* ------------------------------------------------------------- */

void
changeterrain (void)
{
  region *r;
  int i;

  r = inputregion ();
  if (!r)
    return;

  puts ("Terrain?");
  for (i = 0; i != MAXTERRAINS; i++)
    printf ("%d - %s\n", i, strings[terrainnames[i]][DEFAULT_LANGUAGE]);
  gets (buf);
  if (buf[0])
    {
      i = atoi (buf);
      if (i >= 0 && i < MAXTERRAINS)
        terraform (r, i);
    }
  F = stdout;
  describe (r, findfaction(0));
}

/* ------------------------------------------------------------- */

void
addbuilding (void)
{
  region *r;
  building *b;

  r = inputregion ();
  if (!r)
    return;

  b = cmalloc (sizeof (building));
  memset (b, 0, sizeof (building));

  do
    b->no++;
  while (findbuilding (b->no));

  printf ("Groesse: ");
  gets (buf);
  b->size = max (atoi (buf), 1);

  printf ("Name: ");
  gets (buf);
  if (buf[0] == 0)
    sprintf (buf, "Burg %d", b->no);
  mnstrcpy (&b->name, buf, NAMESIZE);

  printf ("Beschreibung: ");
  longgets (buf);
  if (buf[0])
    mnstrcpy (&b->display, buf, DISPLAYSIZE);

  addlist (&r->buildings, b);

  printf ("\n%s, Groesse %d, %s", buildingid (b), b->size,
          buildingtype (b));
  if (b->display)
    printf ("; %s", b->display);

  putc ('\n', stdout);
}

/* ------------------------------------------------------------- */

/* Der type kann -1 sein, dann wird neu nach dem type gefragt. */
void
addunit (int type)
{
  region *r;
  unit *u;
  faction *f;
  int i, n;

  F = stdout;

  r = inputregion ();
  if (!r)
    return;

  /* typ */

  if (type == -1)
    {
      puts ("Typ?");
      for (i = 0; i != MAXTYPES; i++)
	printf ("%d - %s\n", i, strings[typenames[0][i]][0]);
      gets (buf);
      if (buf[0])
	{
	  i = atoi (buf);
	  if (i >= 0 && i < MAXTYPES)
	    type = i;
	}
    }
  if (type == -1)
    {
      puts ("Diesen Typ gibt es nicht.");
      return;
    }
  
  /* anzahl - vor make_illsionary_unit, damit die Anzahl erschaffener
     Gegenstaende pro Unit auch stimmt */
  
  printf ("Anzahl: ");
  gets (buf);
  n = atoi (buf);
  if (!n)
    return;

  /* partei - auch fuer die funktionen (fakultativ vorher schon) vorher schon
     pruefen, ob es die Monster (0) scho gibt, wenn nicht, erschaffen */

  if (!(f = findfaction (0)))
    {
      createmonsters ();
      if (!(f = findfaction (0)))
        puts ("* Fehler! Die Monsterpartei konnte nicht erzeugt "
              "werden");
    }

  printf ("Partei: ");
  gets (buf);
  f = findfaction (atoi (buf));
  if (!f)
    {
      puts ("Diese Partei gibt es nicht.");
      return;
    }

  /* erschaffe unit und setze ein paar defaults */

  switch (type)
    {
    case U_FIREDRAGON:
      u = make_firedragon_unit (r, f, n);
      break;
    case U_DRAGON:
      u = make_dragon_unit (r, f, n);
      break;
    case U_WYRM:
      u = make_wyrm_unit (r, f, n);
      break;
    case U_UNDEAD:
      u = make_undead_unit (r, f, n);
      break;
    case U_ILLUSION:
      u = make_illsionary_unit (r, f, n);
      break;
    case U_GUARDS:
      u = make_guards_unit (r, f, n);
      break;
    default:
      u = createunit (r);
      u->type = type;
      u->number = n;
      u->money = income[u->type] * u->number;
      u->faction = f;
      break;
    }

  puts ("Bis jetzt:");
  rpunit (f, r, u, 4);

  printf ("Neuer Name:\n"
	  "$ ");
  gets (buf);
  if (buf[0])
    mnstrcpy (&u->name, buf, NAMESIZE);

  puts ("Neue Beschreibung: ");
  longgets (buf);
  if (buf[0])
    mnstrcpy (&u->display, buf, DISPLAYSIZE);

  rpunit (f, r, u, 4);

  printf ("Akzeptieren? ");
  gets (buf);
  if (!(buf[0] == 'j' ||
        buf[0] == 'J' ||
        buf[0] == 'y' ||
        buf[0] == 'Y'))
    u->number = 0;              /* automatisch geloescht */
  return;

}

/* ------------------------------------------------------------- */

void
writeinfo (region * r, FILE * F)
{
  unit *u;
  building *b;
  ship *sh;
  faction *f;
  int d;

  fprintf (F, "%s\n", regionid (r));
  fprintf (F, "%s, %d Baeume, %d Bauern, $%d, %d Pferde\n",
           strings[terrainnames[mainterrain (r)]][DEFAULT_LANGUAGE], r->trees, r->peasants, r->money,
           r->horses);

  fprintf (F, "Burgen:\n");

  if (!r->buildings)
    fprintf (F, " keine\n");
  else
    {
      for (b = r->buildings; b; b = b->next)
        {
          fprintf (F, "%s, Groesse %d, %s\n",
                   buildingid (b), b->size, buildingtype (b));
          fprintf (F, " (Besitzer: ");
          for (u = r->units; u; u = u->next)
            if (u->building == b && u->owner)
              {
                fprintf (F, "%s, Partei %d",
                         unitid (u), u->faction->no);
                break;
              }
          if (!u)
            fprintf (F, "niemand");
          fprintf (F, ")\n");
        }

      b = largestbuilding (r);
      fprintf (F, "Groesste Burg: %s\n", buildingid (b));
    }

  fprintf (F, "Schiffe:\n");

  if (!r->ships)
    fprintf (F, " keine\n");
  else
    for (sh = r->ships; sh; sh = sh->next)
      {
        fprintf (F, "%s, %s", shipid (sh), shiptypes[0][sh->type]);
        if (sh->left)
          fprintf (F, ", im Bau");
        fprintf (F, "\n (Besitzer: ");
        for (u = r->units; u; u = u->next)
          if (u->ship == sh && u->owner)
            {
              fprintf (F, "%s, Partei %d",
                       unitid (u), u->faction->no);
              break;
            }
        if (!u)
          fprintf (F, "niemand");
        fprintf (F, ")\n");
      }

  fprintf (F, "Parteien:\n");

  if (!r->units || !factions)
    fprintf (F, " keine\n");
  else
    for (f = factions; f; f = f->next)
      for (u = r->units; u; u = u->next)
        if (u->faction == f)
          {
            fprintf (F, " %s\n", factionid (u->faction));
            break;
          }

  for (d = 0; d != MAXDIRECTIONS; d++)
    {
      fprintf (F, "Im %s: %s, %s\n", directions[d], 
	       regionid (r->connect[d]), 
	       strings[terrainnames[mainterrain (r->connect[d])]][DEFAULT_LANGUAGE]);

      if (!r->connect[d] || !r->connect[d]->units || !factions)
        fprintf (F, " keine\n");
      else
        for (f = factions; f; f = f->next)
          for (u = r->connect[d]->units; u; u = u->next)
            if (u->faction == f)
              {
                fprintf (F, " %s\n", factionid (u->faction));
                break;
              }
    }
}

void
regioninfo (void)
{
  region *r;

  r = inputregion ();
  if (!r)
    return;

  writeinfo (r, stdout);

  if (!cfopen ("info", "w"))
    return;
  puts ("Schreibe Info...");

  writeinfo (r, F);

  fclose (F);
}







