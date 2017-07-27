/* German Atlantis PB(E)M host Copyright (C) 1995-1999  Alexander Schroeder
   
   based on:

   Atlantis v1.0 13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "atlantis.h"
 
/* ------------------------------------------------------------- */

int
finddirection (char *s)
{
  return findstr (directions, s, MAXDIRECTIONS);
}

int
getdirection (void)
{
  return finddirection (getstr ());
}

/* ------------------------------------------------------------- */

int
weight (unit * u)
{
  int i;
  int n;

  n = 0;

  for (i = 0; i != LASTLUXURY; i++)
    n += u->items[i] * itemweight[i];

  n += u->number * PERSONWEIGHT;

  return n;
}

int
ridingcapacity (unit * u)
{
  int w, n;

  n = 0;

  /* Man traegt sein eigenes Gewicht plus seine Kapazitaet! Die Menschen
     tragen nichts (siehe walkingcapacity). ein wagen zaehlt nur, wenn er von
     zwei pferden gezogen wird */

  w = min (u->items[I_HORSE] / HORSESNEEDED, u->items[I_WAGON]);
  n += w * WAGONCAPACITY;

  n += u->items[I_HORSE] * HORSECAPACITY;

  return n;
}

int
walkingcapacity (unit * u)
{
  int n;

  /* Das Gewicht, welches die Pferde tragen, plus das Gewicht, welches die
     Leute tragen */

  n = ridingcapacity (u) + u->number * carryingcapacity[u->type];

  return n;
}

int
canwalk (unit * u)
{
  return weight (u) - walkingcapacity (u) <= 0;
}

int
canride (region *r, unit *u)
{
  return effskill (u, SK_RIDING) 
    && weight (u) - ridingcapacity (u) <= 0
    && r->terrain != T_OCEAN;
}

int
capacity (region * r, ship * sh)
{
  int n;
  unit *u;

  n = 0;

  for (u = r->units; u; u = u->next)
    if (u->ship == sh)
      n += weight (u);

  return shipcapacity[sh->type] - n;
}

int
cansail (region * r, ship * sh)
{
  return capacity (r,sh) >= 0;
}

int
enoughsailors (region * r, ship * sh)
{
  int n;
  unit *u;

  n = 0;

  for (u = r->units; u; u = u->next)
    if (u->ship == sh)
      n += effskill (u, SK_SAILING) * u->number;

  return n >= sailors[sh->type];
}

/* ------------------------------------------------------------- */

region *
drift_to (region *r, ship *sh)
{

  /* Bestimmt ob ein Schiff treibt, und wohin es treibt. Kann das Schiff
     nicht treiben, wird 0 zurueck geliefert. */

  region *goal;
  unit *u;
  int i;

  /* Kapitaen bestimmen.  Falls kein Kapitaen da ist, driftet das
     Schiff so oder so.  */
  for (u = r->units; u; u = u->next)
    if (u->ship == sh && u->owner)
      break;
  if (u)
    {
      /* Falls der Kapitaen kein u->thisorder hat, dann muss er sich
	 schon bewegt haben. Auch in driftenden Schiffen haben alle
	 Einheiten thisorder[0]=0 gesetzt bekommen, weil die Einheiten
	 in der neuen Region sonst ihre thisorder nocheinmal
	 durchfuehren koennten.  */
      if (!u->thisorder[0])
	return 0;
      
      /* Kapitaen mit genuegend Talent da? Schiff nicht Beschaedigt?
	 Genuegend Matrosen? Genuegend leicht? Dann sind die
	 Bedingungen von ship_ready () erfuellt.  */
      if (effskill (u, SK_SAILING) >= sh->type
	  && !sh->left
	  && enoughsailors (r, sh)
	  && cansail (r, sh))
	return 0;

      /* Verzauberte Schiffe driften nicht, ausser sie wurden mit
	 einem Sturmwind verzaubert.  */
      if (sh->enchanted && sh->enchanted != SP_STORM_WINDS)
	return 0;
    }

  /* Auswahl einer Richtung: Zuerst auf Land, dann zufaellig.  Ich
     denke, dass es nicht moeglich ist, dass hier keine Richtung mehr
     moeglich ist.  */
  for (i = 0; i != MAXDIRECTIONS; i++)
    if (r->connect[i] && r->connect[i]->terrain != T_OCEAN)
      break;
  if (i == MAXDIRECTIONS)
    do
      {
	i = rand () % MAXDIRECTIONS;
      }
    while (!r->connect[i]);

  goal = r->connect[i];

  /* Wenn das Schiff aber driftet, und wenn ein Sturmwind blaest, dann
     soll das Schiff 2 Regionen weit segeln. */
  if (goal && goal->terrain == T_OCEAN && goal->connect[i]
      && sh->enchanted == SP_STORM_WINDS)
    goal = goal->connect[i];

  return goal;

}

void
drowning_men (region *r)
{
  unit *u;

  if (r->terrain != T_OCEAN)
    return;

  for (u = r->units; u; u = u->next)
    if (!u->ship && u->enchanted != SP_WATER_WALKING)
      {
	sprintf (buf, "%s ertrinkt in %s.", unitid (u), regionid (r));
	addwarning (u->faction, buf);
	u->number = 0;
      }
};

void
drifting_ships (region *r)
{
  region *goal;
  faction *f;
  ship *sh, *sh2;
  unit *u, *u2;

  /* Schiffe auf hoher See koennen driften. */

  if (r->terrain != T_OCEAN)
    return;

  for (sh = r->ships; sh;)
    {
      /* Da Schiffe sich bewegen koennen, kann sich sh->next aendern.
	 Deswegen vorher abspeichern. */
      sh2 = sh->next;

      goal = drift_to (r, sh);
      if (goal)
	{
	  for (f = factions; f; f = f->next)
	    f->dh = 0;

	  translist (&r->ships, &goal->ships, sh);
	  for (u = r->units; u;)
	    {
	      u2 = u->next;
	      
	      if (u->ship == sh)
		{
		  translist (&r->units, &goal->units, u);
		  u->thisorder[0] = 0;
		  u->faction->dh = 1;
		}
	      
	      u = u2;
	    }
	  
	  sprintf (buf, "Das Schiff %s wurde von Stuermen nach %s getrieben.",
		   shipid (sh), regionid (goal));
	  for (f = factions; f; f = f->next)
	    if (f->dh)
	      addwarning (f, buf);
	}
      
      /* naechstes Schiff */
      sh = sh2;
    }
}

/* ------------------------------------------------------------- */

region *
movewhere (region * r)
{

  region *r2;
  int i;

  i = getdirection ();

  if (i == -1)
    return 0;

  r2 = r->connect[i];

  /* Falls man neue Parteien genau an der Grenze zum Chaos ausgesetzt
     hat und diese Einheiten in der ersten Runde in Chaos schicken,
     dann ist r2 hier noch nicht gesetzt.  i ist die Richtung, in der
     eine neue Insel angefuegt werden soll.  makeblock() ruft
     connectregions() auf, so dass r->connect[i] nun definiert sein
     sollte!  */

  if (!r2)
    {
      printf ("   Neue Insel neben %s.\n", regionid (r));
      makeblock (r->x + delta_x[i], r->y + delta_y[i], r->z);
      seed_monsters (r->x + delta_x[i], r->y + delta_y[i], r->z);
      r2 = r->connect[i];
    }

  assert (r2);

  /* r2 enthaelt nun die existierende Zielregion - ihre Nachbaren
     sollen auch schon alle existieren.  Dies erleichtert das
     Umherschauen bei den Reports!  Bei automatisch generierten Inseln
     (hier!) werden Drachen ausgesetzt.  

     i zeigt nun in alle Richtungen, und fuer jede wird geprueft ob
     dort Chaos (Regionen existieren noch nicht) herrscht.  Die
     Herkunftsregion wird hier zwar mitgeprueft, aber das ist mir
     egal.  */

  for (i = 0; i != MAXDIRECTIONS; i++)
    if (!r2->connect[i])
      {
	printf ("   Neue Insel neben %s.\n", regionid (r));
	makeblock (r2->x + delta_x[i], r2->y + delta_y[i], r2->z);
	seed_monsters (r2->x + delta_x[i], r2->y + delta_y[i], r2->z);
      }
  return r2;
}

/* Diese Funktion uebertraegt die Rolle des Burgherren oder Kapitaens
   auf die naechste Einheit, welche Anrecht darauf hat, falls der
   Burgherren oder der Kapitaen seinen Besitz verlaesst.  */
void
leave (region * r, unit * u)
{
  unit *u2;
  building *b;
  ship *sh;

  if (u->building)
    {
      b = u->building;
      u->building = 0;

      if (u->owner)
        {
          u->owner = 0;

          for (u2 = r->units; u2; u2 = u2->next)
            if (u2->faction == u->faction && u2->building == b)
              {
                u2->owner = 1;
                return;
              }

          for (u2 = r->units; u2; u2 = u2->next)
            if (u2->building == b)
              {
                u2->owner = 1;
                return;
              }
        }
    }

  if (u->ship)
    {
      sh = u->ship;
      u->ship = 0;

      if (u->owner)
        {
          u->owner = 0;

          for (u2 = r->units; u2; u2 = u2->next)
            if (u2->faction == u->faction && u2->ship == sh)
              {
                u2->owner = 1;
                return;
              }

          for (u2 = r->units; u2; u2 = u2->next)
            if (u2->ship == sh)
              {
                u2->owner = 1;
                return;
              }
        }
    }
}

/* ------------------------------------------------------------- */

unit *
present (region * r, unit * u)
{
  unit *u2;

  for (u2 = r->units; u2; u2 = u2->next)
    if (u2 == u)
      break;

  return u2;
}

void
caught_target (region * r, unit * u)
{

  /* Verfolgungen melden */

  if (u->target)
    {

      /* Misserfolgsmeldung, oder bei erfolgreichem Verfolgen unter
         Umstaenden eine Warnung. */

      if (!present (r, u->target))
        {
          sprintf (buf, "%s hat uns in %s abgehaengt.",
                   unitid (u->target), regionid (r));
          addwarning (u->faction, buf);
        }
      else if (!isallied (u->target, u)
               && cansee (u->target->faction, r, u))
        {
          sprintf (buf, "%s ist uns bis nach %s gefolgt.",
                   unitid (u), regionid (r));
          addwarning (u->target->faction, buf);
        }
    }
}

/* ------------------------------------------------------------- */

void
travel (region * r, unit * u, region * r2)
{
  /* Tech: Zu fuss reist man 1 Region, zu Rferde 2 Regionen. Mit
     Strassen reist man Doppelt so weit. Berechnet wird das mit BP
     (Bewegungspunkten). Zu Fuss hat man 2 BPs, zu Pferde 4, ie. die
     BP entsprechen der max. Anzahl Regionen, die man reisen kann.
     Normalerweise verliert man 2 BP pro Region, bei Strassen nur 1
     BP. Strassen verbinden Regionen, wenn Start- und Zielregion 100%
     Strassen haben. */
  region *r3;
  int k, m, i, dh;
  region *rv[MAXSPEED];

  /* Auf hoher See ohne Wasserwandeln wird man spaehter noch
     ertrinken. */
  if (r->terrain == T_OCEAN && u->enchanted != SP_WATER_WALKING)
    return;

  if (!canwalk (u))
    {
      mistakeu (u, "Die Einheit traegt zuviel Gewicht, "
                "um sich bewegen zu koennen");
      return;
    }

  /* wir suchen so lange nach neuen Richtungen, wie es geht. Diese werden
     dann nacheinander ausgefuehrt. */

  /* im array rv[] speichern wir die regionen ab, durch die wir wandern. */

  /* BPs! */

  k = 2;
  if (canride (r, u))
    k = 4;

  m = 0;
  r3 = r;

  /* die naechste Region, in die man wandert, wird durch movewhere aus der
     letzten Region bestimmt.

     Anfangen tun wir bei r. r2 ist beim ersten Durchlauf schon gesetzt
     (Parameter!), das ziel des schrittes. m zaehlt die schritte, k sind die
     noch moeglichen schritte, r3 die letzte gueltige, befahrene Region. */

  while (r2 != 0)
    {
      if (roadto (r3, r2))
        k -= 1;
      else
        k -= 2;

      if (k < 0)
        break;

      if (present (r3, u->target))
        break;

      if (r3->blocked)
        {
          sprintf (buf, "%s konnte aus %s nicht ausreisen.",
                   unitid (u), regionid (r3));
          addwarning (u->faction, buf);
          break;

        }

      if (r2->terrain == T_OCEAN && u->enchanted != SP_WATER_WALKING)
        {
          sprintf (buf, "%s entdeckt, dass (%d,%d) Ozean ist.",
                   unitid (u), r2->x, r2->y);
          addwarning (u->faction, buf);
          break;
        }

      rv[m] = r2;
      m++;

      r3 = r2;
      r2 = movewhere (r2);
    }

  /* Nun enthaelt r3 die letzte Region, in die man gewandert ist. Wir
     generieren hier ein Ereignis fuer den Spieler, das ihm sagt, bis wohin
     er gewandert ist, falls er ueberhaupt vom Fleck gekommen ist. Das ist
     nicht der Fall, wenn er in den ozean zu wandern versuchte */

  if (r3 != r)
    {
      sprintf (buf, "%s ", unitid (u));

      if (canride (r, u))
        {
          scat ("reitet");
          u->skills[SK_RIDING] += u->number * PRODUCEEXP;
        }
      else
        scat ("wandert");
      scat (" von ");
      scat (regionid (r));
      scat (" nach ");
      scat (regionid (r3));
      scat (".");

      /* Ueber die letzte region braucht es keinen Bericht */

      m--;
      if (m > 0)
        {
          scat (" Dabei ");
          if (m > 1)
            scat ("wurden ");
          else
            scat ("wurde ");

          dh = 0;
          for (i = 0; i != m; i++)
            {
              if (dh)
                {
                  if (i == m - 1)
                    scat (" und ");
                  else
                    scat (", ");
                }
              dh = 1;

              scat (trailinto[rv[i]->terrain]);
              scat (" ");
              scat (regionid (rv[i]));
            }
          scat (" durchquert.");
        }

      addmovement (u->faction, buf);
      u->guard = 0;
      leave (r, u);
      translist (&r->units, &r3->units, u);

      /* Verfolgungen melden */

      caught_target (r3, u);

    }
}

int
ship_ready (region * r, unit * u)
{
  if (!u->owner)
    {
      mistakeu (u, "Wir sind nicht der Kapitaen des Schiffes");
      return 0;
    }

  if (effskill (u, SK_SAILING) < u->ship->type)
    {
      sprintf (buf, "Der Kapitaen muss mindestens Segeln %d haben, "
               "um %s zu befehligen ", u->ship->type,
               shiptypes[1][u->ship->type]);
      mistakeu (u, buf);
      return 0;
    }

  if (u->ship->left)
    {
      mistakeu (u, "Das Schiff ist noch nicht fertig gebaut");
      return 0;
    }

  if (!enoughsailors (r, u->ship))
    {
      mistakeu (u, "Auf dem Schiff befinden sich zuwenig erfahrene Seeleute");
      return 0;
    }

  if (!cansail (r, u->ship))
    {
      mistakeu (u, "Das Schiff ist zu schwer beladen, um in See zu stechen");
      return 0;
    }

  return 1;
}

void
sail (region * starting_point, unit * u, region * next_point)
{
  region *current_point;
  faction *f;
  unit *u2, *u3;
  int k, m, l, i;
  region *rv[MAXSPEED];

  /* u ist der Kapitaen.  */
  if (!ship_ready (starting_point, u))
    return;

  /* wir suchen so lange nach neuen Richtungen, wie es geht. Diese werden
     dann nacheinander ausgefuehrt. */

  /* im array rv[] speichern wir die kuestenregionen ab, durch die wir segeln
     (geht nur bei Halbinseln). */

  k = shipspeed[u->ship->type];
  if (u->ship->enchanted == SP_STORM_WINDS)                   /* Spell */
    k *= 2;
  if (u->ship->type == SH_BOAT && u->items[I_WINGED_HELMET])  /* Item */
    k *= 3;
  assert (k < MAXSPEED); /* zur Sicherheit */

  l = 0;
  m = 0;
  current_point = starting_point;

  /* Die naechste Region, in die man segelt, wird durch movewhere ()
     aus der letzten Region bestimmt.

     Anfangen tun wir bei starting_point.  next_point ist beim ersten
     Durchlauf schon gesetzt (Parameter!).  l zaehlt gesichtete
     Kuestenstreifen, rv[] speichert die gesichteten Kuestenstreifen,
     m zaehlt befahrene Felder, current_point ist die letzte gueltige,
     befahrene Region. */

  do
    {

      /* Man kann nur vom Ozean in den Hafen, oder von Hafen zum
         Ozean, oder von Ozean zu Ozean fahren - aber nicht von Hafen
         zu Hafen.  */
      if (current_point->terrain != T_OCEAN
          && next_point->terrain != T_OCEAN)
        {
          sprintf (buf, "Das Schiff %s entdeckt, dass (%d,%d) Festland ist.",
                   shipid (u->ship), next_point->x, next_point->y);
          addwarning (u->faction, buf);
          break;
        }

      /* Falls Blockade, endet die Seglerei hier.  */
      if (current_point->blocked)
        {
          sprintf (buf, "Das Schiff %s konnte (%d,%d) nicht verlassen.",
                   shipid (u->ship), current_point->x, current_point->y);
          addwarning (u->faction, buf);
          break;
        }

      /* Verfolgung erfolgreich? */
      if (present (current_point, u->target))
        break;

      /* Falls kein Problem, eines weiter ziehen */
      current_point = next_point;
      next_point = movewhere (current_point);
      m++;

      /* Falls eine Kuestenregion, dann in rv[] aufnehmen (die letzte
         Kueste wird nachher nicht aufgelistet werden, wenn das Schiff
         dort seine Runde beendent!).  */
      if (current_point->terrain != T_OCEAN)
        {
          rv[l] = current_point;
          l++;
        }

    }
  while (next_point != 0 && m < k);

  /* Nun enthaelt current_point die Region, in der das Schiff seine
     Runde beendet hat.  Wir generieren hier ein Ereignis fuer den
     Spieler, das ihm sagt, bis wohin er gesegelt ist, falls er
     ueberhaupt vom Fleck gekommen ist.  Das ist nicht der Fall, wenn
     er von der Kueste ins Inland zu segeln versuchte.  */
  if (starting_point != current_point)
    {
      sprintf (buf, "Das Schiff %s segelt", shipid (u->ship));
      scat (" von ");
      scat (regionid (starting_point));
      scat (" nach ");
      scat (regionid (current_point));
      u->skills[SK_SAILING] += u->number * PRODUCEEXP;

      /* Falls es Kuesten gibt, denen man entlang gefahren ist, sollen
         sie aufgefuehrt werden.  Falls wir aber unseren Zug in einer
         Kuestenregion beendet haben, fuehren wir sie nicht mit auf -
         sie wird sowieso in allen Details im Report erwaehnt werden.  */
      if (current_point->terrain != T_OCEAN)
        l--;

      if (l > 0)
        {
          scat (". Dabei segelte es entlang der ");
          if (l > 1)
            scat ("Kuesten");
          else
            scat ("Kueste");
          scat (" von ");

          for (i = 0; i != l; i++)
            {
              scat (regionid (rv[i]));

              /* die letzte region ist rv[l-1], denn bei l=4 geht i von 0 bis
                 3. Bsp. die 4 Regionen "A, B, C und D" - wobei A 0, B 1, C 2
                 und D 3 ist. Nach i < l-2 gibt es ein Komma, bei i = l-2 gibt
                 es ein und. */

              if (i < l - 2)
                scat (", ");
              else if (i == l - 2)
                scat (" und ");
            }
        }
      scat (".");

      for (f = factions; f; f = f->next)
        f->dh = 0;

      /* Das Schiff und alle Einheiten darin werden nun von
         starting_point nach current_point verschoben.  */
      translist (&starting_point->ships, &current_point->ships, u->ship);
      for (u2 = starting_point->units; u2;)
        {
          u3 = u2->next;
          if (u2->ship == u->ship)
            {
              translist (&starting_point->units, &current_point->units, u2);
              u2->thisorder[0] = 0;
              u2->faction->dh = 1;
	      u2->guard = 0;
            }
          u2 = u3;
        }

      /* Meldung ueber Bewegungen.  */
      for (f = factions; f; f = f->next)
        if (f->dh)
          addmovement (f, buf);

      /* Verfolgungen melden.  */
      caught_target (current_point, u);
    }
}

/* Segeln, Wandern, Reiten */

void
move (region * r, unit * u)
{
  region *r2;

  r2 = movewhere (r);

  if (!r2)
    mistakeu (u, "Die Richtung wurde nicht erkannt");
  else if (u->ship)
    sail (r, u, r2);
  else
    travel (r, u, r2);

  /* nachdem alle Richtungen abgearbeitet wurden, und alle Einheiten
     transferiert wurden, kann der aktuelle Befehl geloescht werden -
     sonst bewegt sich die Einheit in der neuen Region nochmals, wenn
     die neue Region erst nachher abgearbeitet wird. */

  u->thisorder[0] = 0;

}

/* Bewegung, Verfolgung */

void
movement (void)
{

  region *r;
  unit *u, *u2;

  puts ("- Bewegungen zu Land und zur See...");

  for (r = regions; r; r = r->next)
    {
      /* Ziele von Verfolgungen setzen. Dies muss geschehen, bevor sich die
         Ziele aus der Region absetzen. */
      for (u = r->units; u; u = u->next)
        if (igetkeyword (u->thisorder) == K_FOLLOW)
          {
            u2 = getunit (r, u);
            if (!u2)
              {
                mistakeu (u, "Die Einheit wurde nicht gefunden");
                continue;
              }
            if (igetkeyword (u2->thisorder) != K_MOVE)
              {
                mistakeu (u, "Die Einheit bewegte sich nicht");
                continue;
              }
            u->target = u2;
            mstrcpy (&u->thisorder, u2->thisorder);
          }

      /* Bewegungen: Zuerst muessen sich alle Einheiten ohne u->target
         bewegen (NACH), dann starten die Verfolger mit u->target
         (FOLGE) und hoehren erst auf, wenn sie ihr Ziel erreicht
         haben.  Wir verwenden hier *nicht* die Konstruktion:

          u2 = u->next;
          if (!u->target && igetkeyword (u->thisorder) == K_MOVE)
	    ...
          u = u2;

	  Dies bringt Probleme mit sich, wenn u ein Schiff
	  kommandiert, und u2 auch auf diesem Schiff reist.  Dann wird
	  naemlich u2 auf 0 oder auf sonst eine seltsame Einheit
	  gesetzt!  Deswegen nehmen wir lieber ein wenig Muehe auf uns
	  und fangen den ganzen Loop wieder von vorne an.  Diejenigen,
	  die sich bewegt haben, werden u->thisorder[0]=0 bekommen
	  haben, so dass diese Schlaufe keine Endlosschlaufe sein
	  wird!  */
      for (u = r->units; u;)
        {
          if (!u->target && igetkeyword (u->thisorder) == K_MOVE)
	    {
	      move (r, u);
	      u = r->units;
	    }
	  else 
	    u = u->next;
        }

      /* Verfolger starten */
      for (u = r->units; u;)
        {
          if (u->target && igetkeyword (u->thisorder) == K_MOVE)
	    {
	      move (r, u);
	      u = r->units;
	    }
	  else 
	    u = u->next;
        }

      /* Herrenlose Schiffe */
      drifting_ships (r);
      /* Einheiten ohne Schiff und Magie auf hoher See. */
      drowning_men (r);
    }
}

/* ------------------------------------------------------------- */
