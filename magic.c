/* German Atlantis PB(E)M host Copyright (C) 1995-1999   Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "atlantis.h"
 
/* ------------------------------------------------------------- */

int
magicians (faction * f)
{
  int n;
  region *r;
  unit *u;

  n = 0;

  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      if (u->skills[SK_MAGIC] && u->faction == f)
        n += u->number;

  return n;
}

int
cancast (unit * u, int i)
{
  if (u->spells[i])
    return u->number;
  return 0;
}

/* ------------------------------------------------------------- */

int
findspell (char *s)
{
  return _findstr (spellnames, s, MAXSPELLS);
}

int
getspell (void)
{
  return findspell (getstr ());
}

/* ------------------------------------------------------------- */

/* Zeichnet eine Karte.  Die Einheit magician fuehrt den Spruch aus
   (Traumadler).  Die Karte ist um die Region r zentriert.  magician
   muss sich nicht zwingend in r befinden!  */
void
show_map (region *r, unit *magician, char *type)
{
  int minx, miny, maxx, maxy;
  int spminx, spminy, spmaxx, spmaxy;
  int i, dx, width, length, size;
  region *r2;
  char *map;

  minx = r->x - MAP_RANGE;
  maxx = r->x + MAP_RANGE;
  miny = r->y - MAP_RANGE;
  maxy = r->y + MAP_RANGE;

  length = maxy - miny + 1;
  width = maxx - minx + 1;
  size = length * width;

  /* Fuer den Test, ob alle Regionen existieren, und fuer die Angaben
     auf der Karte, brauchen wir Koordinaten, welche in sphaerische
     Koordinaten transformiert wurden, denn minx kann durchaus
     ausserhalb vom Torus der Welt liegen.  */

  spminx = sphericalx (minx);
  spmaxx = sphericalx (maxx);
  spminy = sphericaly (miny);
  spmaxy = sphericaly (maxy);

  /* Wenn das Adlerauge ueber die Grenzen des bekannten Gebietes
     schaut, muss diese Region halt einfach hinzuerfunden werden.  */

  if (!findregion (spminx, spminy, r->z))
    makeblock (spminx, spminy, r->z);
  if (!findregion (spminx, spmaxy, r->z))
    makeblock (spminx, spmaxy, r->z);
  if (!findregion (spmaxx, spminy, r->z))
    makeblock (spmaxx, spminy, r->z);
  if (!findregion (spmaxx, spmaxy, r->z))
    makeblock (spmaxx, spmaxy, r->z);

  sprintf (buf, "%s konnte %s folgende Karte von (%d,%d) bis (%d,%d) zeichnen:",
           type, unitid (magician), spminx, spminy, spmaxx, spmaxy);
  addevent (magician->faction, buf);

  /* map enthaelt eine Zeile der Karte mit einem bestimmten y.  Ab
     hier werden minx, maxx, miny und maxy nicht mehr verwendet, denn
     hier muss man sehr aufpassen mit den sphaerischen Koordinaten:
     Angenommen es gaebe die Koordinaten zwischen 0 und 8.  -1
     entspricht also 8, -2 entspricht 7, etc.  Falls r nun (1,1) ist,
     muessen die Koordinaten von (-2,-2) bis (4,4) angezeigt werden
     (MAP_RANGE sei 3).  Statt (-2,-2) muss allerdings (7,7) verwendet
     werden!  */

  map = cmalloc (width * 2 +1);
  for (i = 0; i < max (length, MAXTERRAINS); i++)
    {
      memset (map, ' ', width * 2);
      map[width * 2] = 0;

      if (i < length)
        for (r2 = regions; r2; r2 = r2->next)
	  if (((abs (r2->x - r->x) <= MAP_RANGE)
	       || (abs (r2->x - sphericalx (r->x - MAP_RANGE) - MAP_RANGE) <= MAP_RANGE)
	       || (abs (r2->x - sphericalx (r->x + MAP_RANGE) + MAP_RANGE) <= MAP_RANGE))
	      && r2->y == sphericaly (miny + i)
	      && r2->z == r->z)
	    {
	      dx = r2->x - r->x;
	      if (abs (dx) > MAP_RANGE)
		dx = r2->x - sphericalx (r->x - MAP_RANGE) - MAP_RANGE;
	      if (abs (dx) > MAP_RANGE)
		dx = r2->x - sphericalx (r->x + MAP_RANGE) + MAP_RANGE;
	      dx += MAP_RANGE;

	      assert (dx >= 0);
      
	      map[dx * 2] = terrainsymbols[mainterrain (r2)];
	    }

      if (i < MAXTERRAINS)
        sprintf (buf, "%s  %c - %s", map, terrainsymbols[i], 
		 strings[terrainnames[i]][magician->faction->language]);
      else
        strcpy (buf, map);

      addevent (magician->faction, buf);
    }
  free (map);
}

/* Zeigt eine Karte fuer den Standort einer anderen Einheit (nicht des
   Zauberers!).  Wir brauchen also eine neue target_region.  */
void
show_dream_map (region * r, unit * magician)
{
  unit *u, *target_unit;
  region *target_region;

  target_unit = getunitglobal (r, magician);
  if (!target_unit)
    {
      mistakeu (magician, 
		strings[ST_GOAL_UNIT_NOT_FOUND][magician->faction->language]);
      return;
    }
  if (!contacts (r, target_unit, magician))
    {
      mistakeu (magician, 
		strings[ST_GOAL_UNIT_HAS_NOT_CONTACTED_US]
		       [magician->faction->language]);
      return;
    }

  /* target_region: Durchforste alle Regionen nach der angegebenen
     Ziel-Einheit. */
  for (target_region = regions; target_region; target_region = target_region->next)
    {
      for (u = target_region->units; u; u = u->next)
        if (u == target_unit)
          break;
      if (u)
        break;
    }
  
  /* Da getunitglobal () eine unit hergab, musss dies funktionieren.  */
  assert (target_region);

  show_map (target_region, magician, "Mit dem Auge eines Traumadlers");
}

/* Zeigt eine Karte fuer beliebige Koordinaten, falls der Ort
   innerhalb einer gewissen Limite liegt.  */
void
show_remote_map (region * r, unit *magician)
{
  region *target_region;
  int x, y, distance;

  x = geti ();
  y = geti ();

  distance = abs (x - (r->x)) + abs (y - (r->y));
  if (distance > DIRECTED_MAP_DISTANCE)
    {
      mistakeu (magician, "Diese Region liegt zu weit entfernt");
      return;
    }

  target_region = findregion (x, y, r->z);
  if (!target_region)
    {
      makeblock (x, y, r->z);
      target_region = findregion (x, y, r->z);
    }

  assert (target_region);
  show_map (target_region, magician, "Mit dem Geisterauge");
}

/* ------------------------------------------------------------- */

int
docast (unit * u, int spell)
{
  int n;

  n = cancast (u, spell);

  if (!n)
    {
      mistakeu (u, translate (ST_CANNOT_CAST, u->faction->language, unitid (u), 
			      strings[spellnames[spell]][u->faction->language]));
      return 0;
    }

  n = min (n, u->money / 50);

  if (!n)
    {
      sprintf (buf, "%s hat nicht genug Silber", unitid (u));
      mistakeu (u, buf);
      return 0;
    }

  /* wenn n == 0 ist, wird im Folgenden nichts veraendert. n ist entweder die
     Anzahl Leute (Magier) oder die Anzahl magischer Gegenstaende
     (Zauberstaebe etc.) die bezahlt werden koennen. */

  u->money -= n * 50;
  u->skills[SK_MAGIC] += min (u->number, n) * PRODUCEEXP;

  return n;
}

/* ------------------------------------------------------------- */

void
plague (region * r, unit * magician, int n)
{
  faction *f;
  unit *u;
  int i, p, d=0;

  if (!n)
    return;

  sprintf (buf, "%s beschwoert einen Pesthauch.", unitid (magician));
  addevent (magician->faction, buf);

  /* Fuer jeden Magier werden 40% getoetet.  */

  for (i=0; i!=n; i++)
    for (p = r->peasants; p; p--)
      if (rand () % 100 > PLAGUE_SURVIVAL)
	d++;

  r->peasants -= d;
      
  for (f = factions; f; f = f->next)
    f->dh = 0;
  for (u = r->units; u; u = u->next)
    if (!u->faction->dh)
      {
	u->faction->dh = 1;
	sprintf (buf, "In %s wuetete die Pest und %d %s.",
		 regionid (r), d,
		 d == 1 ? "Bauer starb" : "Bauern starben");
	addwarning (u->faction, buf);
      }
}

/* ------------------------------------------------------------- */

void
rust (region * r, unit * magician, int n)
{
  unit *u;
  int i;

  if (!n)
    return;

  n = lovar (n * 50);

  sprintf (buf, "%s ruft einen Rosthauch.", unitid (magician));
  addevent (magician->faction, buf);

  /* fuer jede Einheit */

  for (;;)
    {
      if (!n)
        break;

      u = getunit (r, magician);

      if (getunit0)
        break;

      if (!u)
        {
          mistakeu (magician, "Die Einheit wurde nicht gefunden");
          continue;
        }

      i = min (u->items[I_SWORD], n);

      if (!i)
        {
          sprintf (buf, "%s hatte keine Schwerter.", unitid (u));
          addevent (magician->faction, buf);
          continue;
        }
      
      u->items[I_SWORD] -= i;
      n -= i;
      
      sprintf (buf, "%s muss %d Schwerter wegwerfen, weil sie "
               "rostig und schartig geworden sind.", unitid (u), i);
      addevent (u->faction, buf);
      if (u->faction != magician->faction)
        addevent (magician->faction, buf);
    }
}

/* ------------------------------------------------------------- */

int
teleport (region *r, unit *magician, int teleportable_weight)
{
  region *target_region;
  unit *u, *target_unit;
  int unit_weight, distance;

  /* In dieser Funktion werden bestimmte Befehle der teleportierten
     Einheiten geloescht (zB. weitere Teleportation Zauber).  */

  /* Bestimme teleportierbares Gewicht */

  if (!teleportable_weight)
    return 0;
  teleportable_weight *= TELEPORT_MAX_WEIGHT;

  /* Ziel-Einheit */

  target_unit = getunitglobal (r, magician);
  if (!target_unit)
    {
      mistakeu (magician, 
		strings[ST_GOAL_UNIT_NOT_FOUND][magician->faction->language]);
      return 0;
    }
  if (!contacts (r, target_unit, magician))
    {
      mistakeu (magician, 
		strings[ST_GOAL_UNIT_HAS_NOT_CONTACTED_US]
		       [magician->faction->language]);
      return 0;
    }

  /* Zielregion... durchforste alle Regionen nach der angegebenen
     Ziel-Einheit */

  for (target_region = regions; 
       target_region; 
       target_region = target_region->next)
    {
      for (u = target_region->units; u; u = u->next)
        if (u == target_unit)
          break;

      if (u)
        break;
    }

  /* Da getunitglobal () eine unit hergab, musss dies funktionieren. */

  assert (target_region);

  /* Test ob die Zieleinheit auch wirklich auf derselben Ebene
     liegt. */

  if (r->z != target_region->z)
    {
      mistakeu (magician, 
		strings[ST_GOAL_UNIT_ON_ANOTHER_PLANE]
		       [magician->faction->language]);
      return 0;
    }

  /* Einfluss der Reichweite: Distanz 0 kann auftreten, wenn Einheiten
     in eine belagerte Burg hineinteleportieren wollen. */

  distance = abs (target_region->x - r->x)
    + abs (target_region->y - r->y);
  if (distance > 1)
    teleportable_weight /= distance;

  /* Meldung - nur einmal regionid pro sprintf ()! */

  sprintf (buf, "%s zaubert Teleport von %s nach ",
           unitid (magician), regionid (r));
  scat (regionid (target_region));
  scat (". Das teleportierbare Gewicht ist ");
  icat (teleportable_weight);
  scat (".");
  addevent (magician->faction, buf);

  /* Teleportiere angegebene Einheiten */

  for (;;)
    {
      u = getunit (r, magician);

      if (getunit0)
        break;

      if (!u)
        {
          mistakeu (magician, "Die Einheit wurde nicht gefunden");
          continue;
        }

      if (!contacts (r, u, magician))
        {
          sprintf (buf, "Einheit %d hat keinen Kontakt mit uns aufgenommen",
                   u->no);
          mistakeu (magician, buf);
          continue;
        }

      unit_weight = weight (u);
      if (unit_weight > teleportable_weight)
        {
          sprintf (buf, "Einheit %d ist zu schwer", u->no);
          mistakeu (magician, buf);
          continue;
        }

      teleportable_weight -= unit_weight;

      leave (r, u);
      translist (&r->units, &target_region->units, u);
      u->building = target_unit->building;
      u->ship = target_unit->ship;
      u->guard = 0;
      
      /* Folgende Befehle sind bei teleportierten Einheiten verboten: NACH, FOLGE, und ZAUBERE TELEPORTATION.
         Zudem muessen alle Einheiten, welche einen ZAUBERE Befehl haben, diesen nach erfolgter Ausfuehrung
         auf 0 setzen, damit die Einheit am neuen Ort den Zauber nicht nocheinmal zaubern kann!  */
      switch (findkeyword (aigetstr (u->thisorder)))
	{
	case K_CAST:
	  if (findspell (agetstr ()) != SP_TELEPORT)
	    break;
	  /* else fall through */
	case K_MOVE:
	case K_FOLLOW:
	  u->thisorder[0] = 0;
	  break;

	default:
	  break;
	}

      sprintf (buf, "%s wurde teleportiert.", unitid (u));
      addevent (u->faction, buf);
      if (u->faction != magician->faction)
        addevent (magician->faction, buf);
    }
  magician->thisorder[0] = 0;
  return 1;
}

/* ------------------------------------------------------------- */

void
fog (region * r, unit * magician)
{
  r->blocked = 1;

  sprintf (buf, "%s zaubert Nebelnetze.", unitid (magician));
  addevent (magician->faction, buf);

  addcomment (r, "Dichte Nebel bedeckten diesen Monat die Region. "
              "Keine Einheit schaffte es, diese Nebel zu durchdringen und "
              "die Region zu verlassen.");

}

/* ------------------------------------------------------------- */

void
tremmor (region * r, unit * magician, int n)
{
  faction *f;
  building *b;
  unit *u;
  int d, t=0, p;

  if (!n)
    return;

  b = getbuilding (r);
  if (!b)
    {
      mistakeu (magician, "Die Burg wurde nicht gefunden");
      return;
    }

  if (b->size)
    {
      for (; n; n--)
        {
          /* Zerstoere pro Magier 20%, aber nie die ganze Burg.  */

          p = lovar (20);
          d = min (b->size - 1, b->size * p / 100);
          b->size -= d;
          t += d;
        }
    }

  /* Melde das Beben, 1x pro Partei.  */

  for (f = factions; f; f = f->next)
    f->dh = 0;
  for (u = r->units; u; u = u->next)
    if (!u->faction->dh)
      {
        u->faction->dh = 1;
        if (cansee (f, r, magician))
          sprintf (buf, "%s zauberte ein Donnerbeben und "
                   "beschaedigte %s in %s um %d.",
                   unitid (magician), buildingid (b), regionid (r), t);
        else
          sprintf (buf, "Durch ein Donnerbeben wurde %s in %s "
                   "um %d beschaedigt.",
                   buildingid (b), regionid (r), t);
        addevent (u->faction, buf);
      }
}

/* ------------------------------------------------------------- */

void
grow_wood (region * r, unit * magician, int i)
{
  int n;
  faction *f;
  unit *u;

  if (!i)
    return;
  n = lovar (i * 50);
  r->trees += n;

  /* Melden, 1x pro Partei.  */

  for (f = factions; f; f = f->next)
    f->dh = 0;
  for (u = r->units; u; u = u->next)
    if (!u->faction->dh)
      {
        u->faction->dh = 1;
        if (cansee (f, r, magician))
          sprintf (buf, "%s erschuf in %s einen heiligen Hain "
                   "von %d erhabenen, 1000 jaehrigen Eichen.",
                   unitid (magician), regionid (r), n);
        else
          sprintf (buf, "In %s wurde ein heiliger Hain von %d "
                   "erhabenen, 1000 jaehrigen Eichen geschaffen.",
                   regionid (r), n);
        addevent (u->faction, buf);
      }
}

/* ------------------------------------------------------------- */

void
golem_service (region * r, unit * magician, int i)
{
  faction *f;
  unit *u;
  int n;

  if (!i)
    return;

  /* 2 Steinquader pro Magier */
  n = i * 2;
  magician->items[I_STONE] += n;

  /* Melden, 1x pro Partei.  */

  for (f = factions; f; f = f->next)
    f->dh = 0;
  for (u = r->units; u; u = u->next)
    if (!u->faction->dh)
      {
        u->faction->dh = 1;
        if (cansee (f, r, magician))
          sprintf (buf, "%s rief %d Steingolems nach %s, welche %d %s brachten.",
                   unitid (magician), i, regionid (r), n, strings[itemnames[1][I_STONE]][u->faction->language]);
        else
          sprintf (buf, "%d Steingolems sind durch %s gewandert.", i, regionid (r));
        addevent (u->faction, buf);
      }
}

/* ------------------------------------------------------------- */

void
claws_of_the_deep (region * r, unit * magician, int i)
{
  int n;
  faction *f;
  unit *u;

  if (!i)
    return;
  n = lovar (i * 10);
  magician->items[I_IRON] += n;

  /* Melden, 1x pro Partei.  */

  for (f = factions; f; f = f->next)
    f->dh = 0;
  for (u = r->units; u; u = u->next)
    if (!u->faction->dh)
      {
        u->faction->dh = 1;
        if (cansee (f, r, magician))
          sprintf (buf, "%s erregt den Zorn der Erdgeister in %s und luchste ihnen %d %s ab.",
                   unitid (magician), regionid (r), n, strings[itemnames[1][I_IRON]][u->faction->language]);
        else
          sprintf (buf, "Die Erdgeister in %s wurden erzürnt.", regionid (r));
        addevent (u->faction, buf);
      }
}

/* ------------------------------------------------------------- */

void
nameUnit (unit * u, char *s)
{
  int i;

  if (!s[0])
    return;

  for (i = 0; s[i]; i++)
    if (s[i] == '(')
      break;

  if (s[i])
    {
      mistakeu (u, "Namen duerfen keine Klammern enthalten");
      return;
    }

  mnstrcpy (&u->name, s, NAMESIZE);
}

void
displayUnit (unit * u, char *s)
{

  if (!s[0])
    return;

  mnstrcpy (&u->display, s, DISPLAYSIZE);
}

void
summon_undead (region * r, unit * magician, int n)
{
  unit *u;

  if (!n)
    return;

  u = make_undead_unit (r, magician->faction,
                        n * 8 + lovar (n * 42));        /* 10 - 50 */
  nameUnit (u, getstr ());
  displayUnit (u, getstr ());

  addevent (magician->faction, 
	    translate (ST_CASTS, magician->faction->language, unitid (magician), 
		       strings[spellnames[SP_SUMMON_UNDEAD]][magician->faction->language]));
}

void
conjure_knights (region * r, unit * magician, int n)
{
  unit *u;

  if (!n)
    return;

  u = make_illsionary_unit (r, magician->faction,
                            lovar (n * 20));    /* 2 - 20 */
  nameUnit (u, getstr ());
  displayUnit (u, getstr ());

  addevent (magician->faction, 
	    translate (ST_CASTS, magician->faction->language, unitid (magician), 
		       strings[spellnames[SP_CONJURE_KNIGHTS]][magician->faction->language]));
}

/* ------------------------------------------------------------- */

void
storm_winds (region * r, unit * magician)
{
  faction *f;
  unit *u;

  if (!magician->ship)
    {
      mistakeu (magician, "Der Magier befindet sich gar nicht "
                "an Bord eines Schiffes");
      return;
    }

  magician->ship->enchanted = SP_STORM_WINDS;

  /* melden, 1x pro partei an bord */

  for (f = factions; f; f = f->next)
    f->dh = 0;
  for (u = r->units; u; u = u->next)
    if (u->ship == magician->ship && !u->faction->dh)
      {
        u->faction->dh = 1;
        sprintf (buf, "Die von %s gerufenen Stuerme jagen die "
                 "%s mit doppelter Geschwindigkeit ueber den Ozean.",
                 unitid (magician), shipid (magician->ship));
        addevent (u->faction, buf);
      }
}

/* ------------------------------------------------------------- */

void
waterwalk (unit *magician)
{
  magician->enchanted = SP_WATER_WALKING;
  addevent (magician->faction, 
	    translate (ST_CASTS, magician->faction->language, unitid (magician), 
		       strings[spellnames[SP_WATER_WALKING]][magician->faction->language]));
}
		  
/* ------------------------------------------------------------- */

int
percentage_dead (int population, int percentage)
{
  int i, dead;

  dead = 0;
  for (i = population; i; i--)
    if (rand () % 100 <= percentage)
      dead++;
  return dead;
}

void
burn (region * r, unit * magician)
{
  faction *f;
  unit *u;
  building *b;
  int i, dead_peasants, dead_men, survivors;

  if (r->terrain == T_PLAIN)
    r->terrain = T_SWAMP;

  r->trees = 0;
  r->horses = 0;
  r->money = 0;

  dead_peasants = percentage_dead (r->peasants, 90);
  r->peasants -= dead_peasants;

  for (i = 0; i != MAXLUXURIES; i++)
    r->demand[i] = MINDEMAND;

  /* reduziere alle gebaeude auf 1% ihrer urspruenglichen groesse, ohne sie
     vollstaendig aus zu radieren. von einer zitadelle bleibt so noch ein
     turm stehen. */

  for (b = r->buildings; b; b = b->next)
    b->size = max (b->size / 100, 1);

  /* melde das inferno, 1x pro partei und reduziere alle einheiten */

  for (f = factions; f; f = f->next)
    f->dh = 0;

  for (u = r->units; u; u = u->next)
    {
      if (!u->faction->dh)
        {
          u->faction->dh = 1;
          sprintf (buf, "In %s wurde durch %s ein Inferno entfacht. "
                   "In den Flammen starben %d Bauern. "
                   "Alle Gebaeude brannten bis auf ihre Grundmauern "
                   "nieder. In den folgenden Tagen "
                   "fiel ein dreckiger Nieselregen, der das Feuer "
                   "loeschte und die Region "
                   "in einen schwarzen Sumpf verwandelt hat.",
                   regionid (r), unitid (magician), dead_peasants);
          addevent (u->faction, buf);
        }

      /* nur der magier und einheiten an bord von schiffen ueberleben
         unbeschadet */

      if (u == magician || u->ship)
        dead_men = 0;
      else
        dead_men = percentage_dead (u->number, 90);

      if (dead_men)
        {
          survivors = u->number - dead_men;

          for (i = 0; i != MAXSKILLS; i++)
            u->skills[i] = distribute (u->number, survivors, u->skills[i]);

          u->number = survivors;

          sprintf (buf, "%s verlor %d Mann im Flammenmeer.",
                   unitid (u), dead_men);
          addevent (u->faction, buf);
        }

    }
}

/* ------------------------------------------------------------- */

void
hammer (region * r, unit * magician)
{
  faction *f;
  unit *u;
  building *b;
  int i, dead_peasants;

  /* Die Region aendert sich. */

  switch (r->terrain)
    {
    case T_OCEAN:
      sprintf (buf, "In %s beschwor %s den Hammer der Goetter und loeste eine Flutwelle aus.",
	       regionid (r), unitid (magician));
      addevent (magician->faction, buf);
      return;

    case T_PLAIN:
    case T_DESERT:
      r->terrain = T_SWAMP;
      break;
      
    case T_SWAMP:
      r->terrain = T_OCEAN;
      
      /* Alles wird vernichtet. */

      r->trees = 0;
      r->horses = 0;
      r->money = 0;
      r->peasants = 0;
      for (i = 0; i != MAXLUXURIES; i++)
	r->demand[i] = MINDEMAND;
      
      /* Alle Einheiten ertrinken, wenn sie nicht in einem Schiff sind. */
      
      drowning_men (r);
      break;
      
    case T_GLACIER:
      r->terrain = T_MOUNTAIN;
      break;
      
    case T_MOUNTAIN:
      r->terrain = T_HIGHLAND;
      break;
      
    case T_HIGHLAND:
      r->terrain = T_DESERT;
      break;
      
    default:
      /*Region nicht implementiert.  */
      assert (0);
    }

  /* Meldung aufbauen.  */

  sprintf (buf, "In %s beschwor %s den Hammer der Goetter und der Boden senkte sich permanent.",
	   regionid (r), unitid (magician));

  /* Bauern sterben */

  if (r->peasants)
    {
      dead_peasants = percentage_dead (r->peasants, 20);
      r->peasants -= dead_peasants;

      if (dead_peasants)
	{
	  scat (" ");
	  icat (dead_peasants);
	  scat (" ");
	  if (dead_peasants == 1)
	    scat ("Bauer");
	  else
	    scat ("Bauern");
	  scat (" starben in der Katastrophe.");
	}
    }

  /* Reduziere alle Gebaeude auf 50% ihrer urspruenglichen Groesse, ohne sie vollstaendig auszuradieren. */

  for (b = r->buildings; b; b = b->next)
    b->size = max (b->size / 2, 1);

  if (r->buildings)
    scat (" Alle Gebaeude nahmen betraechtlichen Schaden.");
    
  /* Meldung 1x pro Partei mit anwesenden Einheiten. */

  for (f = factions; f; f = f->next)
    f->dh = 0;

  for (u = r->units; u; u = u->next)
    if (!u->faction->dh)
      {
	u->faction->dh = 1;
	addevent (u->faction, buf);
      }
}


void
provocation (region * r, unit * magician)
{
  faction *f;
  unit *u;

  /* Die Region aendert sich. */

  switch (r->terrain)
    {
    case T_GLACIER:
      /* Es passiert nichts, der Gletscher bleibt Gletscher.  */
      sprintf (buf, "In %s provozierte %s die Titanen und loeste einige Lawinen aus.",
	       regionid (r), unitid (magician));
      addevent (magician->faction, buf);
      return;
      
    case T_OCEAN:
      r->terrain = T_SWAMP;
      break;

    case T_SWAMP:
      r->terrain = T_DESERT;
      break;

    case T_PLAIN:
    case T_DESERT:
      r->terrain = T_HIGHLAND;
      break;
	
    case T_HIGHLAND:
      r->terrain = T_MOUNTAIN;
      break;
	
    case T_MOUNTAIN:
      r->terrain = T_GLACIER;
      break;

    default:
      /* Region wurde nicht programmiert.  */
      assert (0);
    }

  /* Meldung 1x pro Partei mit anwesenden Einheiten. */

  sprintf (buf, "In %s provozierte %s die Titanen und der Boden hob sich permanent.",
	   regionid (r), unitid (magician));

  for (f = factions; f; f = f->next)
    f->dh = 0;

  for (u = r->units; u; u = u->next)
    if (!u->faction->dh)
      {
	u->faction->dh = 1;
	addevent (u->faction, buf);
      }
}

void
blessing (region * r, unit * magician)
{
  faction *f;
  unit *u;

  /* Meldung 1x pro Partei mit anwesenden Einheiten. */

  sprintf (buf, "In %s beschwor %s den Segen der Goettinen der Fruchtbarkeit.",
	   regionid (r), unitid (magician));

  for (f = factions; f; f = f->next)
    f->dh = 0;

  for (u = r->units; u; u = u->next)
    if (!u->faction->dh)
      {
	u->faction->dh = 1;
	addevent (u->faction, buf);
      }

  /* Die Region aendert sich. */

  if (r->terrain == T_DESERT)
    {
      r->terrain = T_PLAIN;
    }
  else
    {
      sprintf (buf, "Leider befindet sich %s nicht in einer Wueste.", unitid (magician));
      addevent (magician->faction, buf);
    }
}

/* ------------------------------------------------------------- */

void
makeitem (unit * u, int spell, int item)
{
  int n;

  if (u->money < ITEMCOST * spelllevel[spell])
    {
      mistakeu (u, "Die Einheit kann die Herstellungskosten nicht bezahlen");
      return;
    }

  n = min (u->number, u->money / (ITEMCOST * spelllevel[spell]));
  u->items[item] += n;
  u->money -= n * ITEMCOST * spelllevel[spell];

  /* Falls fuer Gegenstaende nicht bezahlt werden kann, gibts auch keine
     PRODUCEEXP. */

  u->skills[SK_MAGIC] += min (u->number, n) * PRODUCEEXP;

  strcpy (buf, translate (ST_CASTS, u->faction->language, unitid (u), 
			  strings[spellnames[spell]][u->faction->language]));
  addevent (u->faction, buf);
}

/* ------------------------------------------------------------- */

void domagic (void)
{
  /* Fuehrt alle ZAUBER Befehle aus, ausser Teleportation. */

  region *r;
  unit *u;
  int i, j;

  puts ("- Magie, ohne Teleport...");

  /* u->thisorder wird bei allen Magiern geloescht, sobald sie gezaubert haben, so treten keine Doppelzauber
     auf.  Dies muss auch geschehen wenn ein Fehler auftritt (sonst gibt es zT. zwei Fehlermeldungen, wenn
     magische Gegenstaende produziert werden).  */

  for (r = regions; r; r = r->next)
      for (u = r->units; u; u = u->next)
	if (igetkeyword (u->thisorder) == K_CAST)
	  {
	    i = getspell ();

	    /* Eigentlich darf man auf Ozeanen nicht zaubern.  Nur
               falls man Provokation der Titanen zaubert, ist dies
               auch auf dem Ozean erlaubt! */
	    if ( r->terrain == T_OCEAN && i != SP_PROVOCATION )
	      continue;

	    if (i < 0 || !cancast (u, i))
	      {
		mistakeu (u, "Diesen Zauber kennt die Einheit nicht");
		u->thisorder[0] = 0;
		continue;
	      }
	    
	    /* Fuer die Erschaffung von magic items */
	    j = spellitem[i];
	    if (j >= 0)
	      {
		makeitem (u, i, j);
		u->thisorder[0] = 0;
		continue;
	      }
	    
	    if (u->money < 50)
	      {
		mistakeu (u, "Die Einheit kann die Ingredienzien nicht bezahlen");
		u->thisorder[0] = 0;
		continue;
	      }

	    if (i==SP_TELEPORT)
	      /* Wird anschliessend in do teleport ausgefuehrt.  */
	      continue;  
	    
	    switch (i)
	      {
		/* docast () wird bei jedem case aufgerufen. Der return value ist wichtig, wenn die Anzahl der
		   Magier relevant fuer den Spruch ist. Jeder Spruch muss irgendeine message - addevent () -
		   absetzen, um ueber das Geschehen zu informieren. */
	      case SP_FOG_WEBS:
		if (docast (u, SP_FOG_WEBS))
		  fog (r, u);
		break;
		
	      case SP_PLAGUE:
		plague (r, u, docast (u, SP_PLAGUE));
		break;
		
	      case SP_RUST:
		rust (r, u, docast (u, SP_RUST));
		break;
		
	      case SP_MAP:
		if (docast (u, SP_MAP))
		  show_map (r, u, "Mit seinen Adleraugen");
		break;
		  
	      case SP_REMOTE_MAP:
		if (docast (u, SP_REMOTE_MAP))
		  show_remote_map (r, u);
		break;
		  
	      case SP_DREAM_MAP:
		if (docast (u, SP_MAP))
		  show_dream_map (r, u);
		break;
		
	      case SP_HOLY_WOOD:
		grow_wood (r, u, docast (u, SP_HOLY_WOOD));
		break;
		
	      case SP_GOLEM_SERVICE:
		golem_service (r, u, docast (u, SP_GOLEM_SERVICE));
		break;
		
	      case SP_CLAWS_OF_THE_DEEP:
		claws_of_the_deep (r, u, docast (u, SP_CLAWS_OF_THE_DEEP));
		break;
		
	      case SP_INFERNO:
		if (docast (u, SP_INFERNO))
		  burn (r, u);
		break;
		
	      case SP_TREMMOR:
		tremmor (r, u, docast (u, SP_TREMMOR));
		break;
		
	      case SP_SUMMON_UNDEAD:
		summon_undead (r, u,
				 docast (u, SP_SUMMON_UNDEAD));
		break;
		
	      case SP_CONJURE_KNIGHTS:
		conjure_knights (r, u,
				 docast (u, SP_CONJURE_KNIGHTS));
		break;
		
	      case SP_STORM_WINDS:
		if (docast (u, SP_STORM_WINDS))
		  storm_winds (r, u);
		break;
		
	      case SP_NIGHT_EYES:
		if (docast (u, SP_NIGHT_EYES))
		  {
		    u->enchanted = SP_NIGHT_EYES;
		    addevent (u->faction, 
			      translate (ST_CASTS, u->faction->language, unitid (u), 
					 strings[spellnames[SP_NIGHT_EYES]][u->faction->language]));
		  }
		break;
		
	      case SP_WATER_WALKING:
		if (docast (u, SP_WATER_WALKING))
		  waterwalk (u);
		break;
		
	      case SP_HAMMER:
		if (docast (u, SP_HAMMER))
		  hammer (r, u);
		break;
		
	      case SP_PROVOCATION:
		if (docast (u, SP_PROVOCATION))
		  provocation (r, u);
		break;
		
	      case SP_BLESSING:
		if (docast (u, SP_BLESSING))
		  blessing (r, u);
		break;

	      case SP_HEAL:
		mistakeu (u, "Heilung ist nur nach einem "
			  "Kampf noetig, um die Sterbenden zu "
			  "retten");
		break;
		
	      default:
		puts ("* Fehler! Ein Spruch wurde als Ritual "
		      "definiert und nicht ausprogrammiert.");
	      }
	    u->thisorder[0] = 0;
	  }
}

void doteleport (void)
{
  /* Fuehrt alle Teleportations-Zauber aus. */

  region *r;
  unit *u;
  int i;

  puts ("- Teleport...");

  /* u->thisorder wird auch hier bei allen Magiern geloescht, sobald sie gezaubert haben, so treten keine
     Doppelzauber auf.  Fehlermeldungen sind keine noetig, da alles schon in domagic ausgefuehrt wurde.  */

  for (r = regions; r; r = r->next)
    if (r->terrain != T_OCEAN)
      for (u = r->units; u; u = u->next)
	if (igetkeyword (u->thisorder) == K_CAST)
	  {
	    i = getspell ();

	    /* Alle Tests wurden schon in domagic durchgefuehrt! Alle Ausgaben sind schon abgezogen.  */

	    assert (i == SP_TELEPORT);

	    /* Einheiten, welche sich mittels Teleportation aus der Region bewegen, koennen die Liste
	       durcheinander bringen.  Dies geschieht zum Beispiel, wenn die erste Einheit in der Region sich
	       selber wegteleportiert.  Nach dem Aufruf von teleport wissen wir also nicht mehr, wo sich die
	       Einheit u befindet.  In der Funktion teleport wurde r->units aber richtig abgespeichert.  Also
	       setzen wir u auf r->units und fangen einfach wieder von vorne an.  */
	    
	    if (teleport (r, u, docast (u, SP_TELEPORT)))
	      {
		u->thisorder[0] = 0;
		u = r->units;
	      }
	  }
}

void
doforget (void)
{
  region *r;
  unit *u;
  int i;

  /* Vergessen von zu schweren Spruechen, falls noetig.  */

  puts ("- zu schwere Sprueche vergessen...");

  for (r = regions; r; r = r->next)
    {
      /* Vergesse alle sprueche, fuer die man nicht genug talent mehr hat (v.a. nach neuen rekrutierungen) */

      for (u = r->units; u; u = u->next)
        {
          for (i = 0; i != MAXSPELLS; i++)
            if (u->spells[i] && spelllevel[i] > (effskill (u, SK_MAGIC) + 1) / 2)
              u->spells[i] = 0;

          if (u->combatspell >= 0 && !cancast (u, u->combatspell))
            u->combatspell = -1;
        }
    }

}

void
magic (void)
{
  /* Rituale werden kurz vor den Bewegungen ausgefuehrt, damit man keine Rituale in fremden Regionen machen
     kann.  Magier sind also eine Runde lang verletzlich in einem fremden Gebiet, da sie es betreten, und
     angegriffen werden koennen bevor sie ein Ritual machen koennen.  Teleport erst nachdem alle anderen
     Magier gezaubert haben. */

  doforget ();
  domagic ();
  doteleport ();
}

/* ------------------------------------------------------------- */

void
research (unit * u)
{
  int i, j, n;

  if (effskill (u, SK_MAGIC) < 2)
    {
      mistakeu (u, "Hierzu muss man ein Magietalent von mindestens 2 haben");
      return;
    }

  /* Es wird auf Stufe i geforscht.  */
  i = geti ();
  if (i > effskill (u, SK_MAGIC) / 2)
    {
      sprintf (buf, "%s kann maximal auf Stufe %d forschen",
               unitid (u), effskill (u, SK_MAGIC) / 2);
      mistakeu (u, buf);
      i = 0;
    }

  /* Falls kein i oder kein gueltiges i gegeben wurde, wird i auf das Maximum gesetzt.  i muss groesser als 0
     sein, da es vielleicht "slots" in der Spruchliste gibt, die geloescht worden sind.  Dann hat der Spruch
     keinen Namen und keine Beschreibung, sowies Stufe 0 -- diese Sprueche duerfen nie erforscht werden; i
     darf nie 0 bleiben.  */
  if (i == 0)
    i = effskill (u, SK_MAGIC) / 2;
  assert (i);

  /* Falls es einen unbekannten Spruch gibt, ist j >= 0 und < MAXSPELLS.  */
  for (j = 0; j != MAXSPELLS; j++)
    if (spelllevel[j] == i && !u->spells[j])
      break;
  assert (j<=MAXSPELLS);

  if (j == MAXSPELLS)
    {
      if (u->money < STUDYCOST)
        {
          mistakeu (u, "Die Einheit kann fuer die Nebenkosten nicht aufkommen");
          return;
        }

      /* Fuer jeden Magier STUDYCOST Silber.  Auf erfolgloses Forschen kostet Silber und bringt Erfahrung.  */
      for (n = u->number; n; n--)
        if (u->money >= STUDYCOST)
          {
            u->money -= STUDYCOST;

            /* Und PRODUCEEXP Tage ueben, trotz allem - also wird hier
               automatisch fuer jeden der Geld hat min (n, u->number) *
               PRODUCEEXP gemacht. */

            u->skills[SK_MAGIC] += PRODUCEEXP;
          }

      sprintf (buf, "%s realisiert, dass es keine weiteren Zauber "
               "der %d. Stufe gibt.", unitid (u), i);
      addevent (u->faction, buf);
      return;
    }

  /* Fuer jeden Magier STUDYCOST Silber, PRODUCEEXP Lernerfahrung und einen Spruch.  Wenn mehrere Magier
     gleichzeitig forschen, koennen sie durchaus in der gleichen Runde denselben Spruch erforschen!  */
  for (n = u->number; n; n--)
    {
      if (u->money < STUDYCOST)
        {
          mistakeu (u, "Die Einheit kann fuer die Nebenkosten nicht aufkommen");
          return;
        }
      u->money -= STUDYCOST;

      /* Vorher haben wir nur ueberprueft, *dass* ein moeglicher Spruch existiert.  Falls es mehrere gibt,
         soll davon einer zufaellig gewaehlt werden.  Also waehlen wir so lange zufaellige Sprueche, bis wir
         einen der richtigen Stufe gefunden haben.  */
      do
        j = rand () % MAXSPELLS;
      while (spelllevel[j] != i || u->spells[j] == 1);

      /* Wird beim Schreiben der reports verwendet, um das Anzeigen der Beschreibungen zu kontrollieren */
      if (!u->faction->seendata[j])
        {
          u->faction->seendata[j] = 1;
          u->faction->showdata[j] = 1;
        }

      if (u->spells[j] == 0)
	addevent (u->faction, translate (ST_RESEARCHES, u->faction->language, 
					 unitid (u), strings[spellnames[j]][u->faction->language]));
      
      /* Falls eine Einheit mehrere Magier hat, und der erste den Spruch j entdeckt, kann ihn der zweite
         Magier auch entdecken (und somit vergebens geforscht haben). Dies vereinfacht einiges, so zB. die
         Suche nach noch unbekannten Spruechen: sobald es einen unbekannten Spruch gibt, koennen auch drei
         Magier danach forschen (sie entdecken halt alle drei den selben Spruch). */
      u->spells[j] = 2;
      u->skills[SK_MAGIC] += PRODUCEEXP;
    }

  for (j = 0; j != MAXSPELLS; j++)
    if (u->spells[j] == 2)
      u->spells[j] = 1;
}

