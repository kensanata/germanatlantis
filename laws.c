/* German Atlantis PB(E)M host Copyright (C) 1995-1998   Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "atlantis.h"
 
/* ------------------------------------------------------------- */

int
findoption (char *s)
{
  return findstr (options, s, MAXOPTIONS);
}

int
getoption (void)
{
  return findoption (getstr ());
}

/* ------------------------------------------------------------- */

void
remove_empty_factions (void)
{
  faction *f, *f2, *f3;
  rfaction *rf, *rf2;

  puts ("- beseitigen leerer Parteien...");

  for (f = factions; f;)
    {
      f2 = f->next;

      /* Monster (0) werden nicht entfernt.  alive kann beim readgame
         () auf 0 gesetzt werden, wenn Monsters keine Einheiten mehr
         haben.  */

      if (!f->alive && f->no)
        {
	  /* Die tote Partei f wird aus allen Alliertenlisten der
             Parteien f3 genommen.  */
          for (f3 = factions; f3; f3 = f3->next)
            for (rf = f3->allies; rf;)
              {
                rf2 = rf->next;
                if (rf->faction == f)
                  removelist (&f3->allies, rf);
                rf = rf2;
              }
	  /* Eine ganze Liste von freelist () function calls wurde
             hier geloescht -- vielleicht macht dies das Auswerten
             schneller?  Normalerweise wird ja auch nicht darauf
             geschaut, dass der Speicher wieder frei wird!  */
          removelist (&factions, f);
        }
      f = f2;
    }
}

void
remove_empty_units (void)
{
  int i;
  region *r;
  unit *u, *u2, *u3;

  puts ("- beseitigen leerer Einheiten...");

  /* Einheiten u werden beseitigt, wenn sie keine Personen haben.
     Ihre Sachen werden an eine Einheit u3 uebergeben, wenn sich eine
     Einheit u3 derselben Partei und mit Personen finden laesst.  */

  for (r = regions; r; r = r->next)
    for (u = r->units; u;)
      {
        u2 = u->next;

        if (!u->number)
          {
            leave (r, u);

            if (u->type != U_ILLUSION)
	      {
		for (u3 = r->units; u3; u3 = u3->next)
		  if (u3->faction == u->faction && u3->number)
		    {
		      u3->money += u->money;
		      u->money = 0;
		      for (i = 0; i != MAXITEMS; i++)
			u3->items[i] += u->items[i];
		      break;
		    }
		if (!u3 && r->terrain != T_OCEAN)
		  r->money += u->money;
	      }
	    
            freelist (u->orders);
            removelist (&r->units, u);
          }
	
        u = u2;
      }
}

void
destroyfaction (faction * f)
{
  region *r;
  unit *u;

  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      if (u->faction == f)
        {
          if (r->terrain != T_OCEAN)
            {
              r->peasants += u->number;
              r->money += u->money;
            }

          u->number = 0;
        }

  f->alive = 0;
}

void
quit (void)
{
  region *r;
  unit *u;
  strlist *S;
  faction *f;

  /* sterben erst nachdem man allen anderen gegeben hat - bzw. man kann alles
     machen, was nicht ein dreissigtaegiger befehl ist. */

  puts ("- beseitige Spieler, die Aufhoeren wollen...");

  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      for (S = u->orders; S; S = S->next)
        if (igetkeyword (S->s) == K_QUIT)
          {
	    if (u->faction->passw && strcmp (u->faction->passw, getstr ()))
	      mistake2 (u, S, "Das Passwort wurde falsch eingegeben");
            else
              destroyfaction (u->faction);
          }
  
  puts ("- beseitige Spieler, "
	 "die sich zu lange nicht mehr gemeldet haben...");

  for (f = factions; f; f = f->next)
      if (turn - f->lastorders >= ORDERGAP)
	destroyfaction (f);

  /* Clear away debris of destroyed factions */

  puts ("- beseitige leere Einheiten und leere Parteien...");

  remove_empty_units ();
  remove_empty_factions ();
}

/* ------------------------------------------------------------- */

void
checkorders (void)
{
  faction *f;

  /* Warn players who haven't sent in orders */

  puts ("- Warne spaehte Spieler...");
  for (f = factions; f; f = f->next)
    if (f->no && turn - f->lastorders == ORDERGAP - 1)
      addstrlist (&f->messages,
                  "Bitte sende die Befehle naechste Runde ein, "
                  "wenn du weiterspielen moechtest.");
}

/* ------------------------------------------------------------- */

void
feed (region * r, unit * u)
{
  int satiated, dead, i;

  /* Die Einheiten sammeln genug Geld von Einheiten in derselben Region, um zu ueberleben.  i ist das Geld,
     dass die Einheit zusaetzlich benoetigt -- hat sie mehr Geld als benoetigt, sammelt sie kein Geld von
     anderen Einheiten.  */

  i = max (0, u->number * MAINTENANCE - u->money);
  if (i)
    collectmoney (r, u, i);

  /* Alle werden satt, oder halt soviele fuer die es auch Geld gibt */

  satiated = min (u->number, u->money / MAINTENANCE);
  u->money -= satiated * MAINTENANCE;
  assert (u->money >= 0);

  /* Von denjenigen, die nicht satt geworden sind, verhungert der Grossteil */

  dead = 0;
  for (i = u->number - satiated; i; i--)
    if (rand () % STARVATION_SURVIVAL)
      dead++;
  if (dead)
    {
      if (u->number > dead)
        sprintf (buf, "%s in %s verliert %d %s durch Unterernaehrung.",
                 unitid (u), regionid (r), dead,
                 dead != 1 ? "Mitglieder" : "Mitglied");
      else
        sprintf (buf, "%s in %s verhungert.", unitid (u), regionid (r));
      addwarning (u->faction, buf);

      for (i = 0; i != MAXSKILLS; i++)
        u->skills[i] = distribute (u->number, u->number - dead,
                                   u->skills[i]);

      u->number -= dead;
    }
}

void
live (region * r)
{
  unit *u;

  for (u = r->units; u; u = u->next)
    {

      /* Einheiten ernaehren sich: illusionen und untote brauchen nichts */
      if (u->type != U_ILLUSION && u->type != U_UNDEAD)
        feed (r, u);

      /* Einheiten werden aelter */
      age_unit (r, u);

      /* Nach diesem Punkt werden Einheiten nicht mehr mit TEMP X angesprochen.  Damit man den Host aber nicht
         neu starten muss (zB. waehrend einem Test Spiel), muessen alle u->alias Hinweise geloescht werden!  */
      u->alias = 0;
    }
}

/* ------------------------------------------------------------- */

int
wanderoff (region * r, int p)
{
  int n, i, d = 0, emigrants[MAXDIRECTIONS];

  /* Wandern am Ende des Monats (normal) oder wegen Monster. Die Wanderung
     wird erst am Ende von demographics () ausgefuehrt. emigrants[] ist
     local, weil r->newpeasants durch die Monster vielleicht schon
     hochgezaehlt worden ist. */

  for (i = 0; i != MAXDIRECTIONS; i++)
    emigrants[i] = 0;

  for (n = r->peasants; n; n--)
    if (rand () % 100 < p)
      {
        i = rand () % MAXDIRECTIONS;

        if (r->connect[i] && r->connect[i]->terrain != T_OCEAN)
          {
            d++;
            r->peasants--;
            r->connect[i]->newpeasants++;
            emigrants[i]++;
          }
      }

  sprintf (buf, "Chance: %d%%.", p);

  for (i = 0; i != MAXDIRECTIONS; i++)
    if (r->connect[i] && r->connect[i]->terrain != T_OCEAN)
      {
        scat (" Nach ");
        scat (regionid (r->connect[i]));
        scat (" ");
        icat (emigrants[i]);
        scat (".");
      }
  scat (" Bleiben: ");
  icat (r->peasants);
  scat (".");
  adddebug (r, buf);

  assert (r->peasants >= 0);
  return d;
}

void
follow_money (region * r)
{
  int n, d = 0, favored;

  /* Die Bauern wandern in Nachbarregionen, falls diese reicher sind */

  favored = richest_neighbour (r);

  if (favored < 0)
    strcpy (buf, "Keine Region wurde bevorzugt.");
  else
    {
      for (n = r->peasants; n; n--)
        if (rand () % 100 < PEASANTGREED)
          {
            d++;
            r->peasants--;
            r->connect[favored]->newpeasants++;
          }

      sprintf (buf, "%d Bauern ins bevorzugte %s, bleiben: %d.",
               d, regionid (r->connect[favored]), r->peasants);

    }
  adddebug (r, buf);

  assert (r->peasants >= 0);
}

void
peasants (region * r)
{
  int old_population, new_population, money, n, dead, satiated;

  /* Bauern vermehren sich.  */
  old_population = r->peasants;
  for (n = r->peasants; n; n--)
    if (rand () % 100 < PEASANTGROWTH)
      r->peasants++;
  new_population = r->peasants;

  /* Bauern ernaehren sich.  */
  money = r->money;
  satiated = min (r->peasants, r->money / MAINTENANCE);
  r->money -= satiated * MAINTENANCE;

  /* Von denjenigen, die nicht satt geworden sind, verhungert der Grossteil.
     dead kann nie groesser als r->peasants - satiated werden, so dass
     r->peasants >= 0 bleiben muss. */
  dead = 0;
  for (n = r->peasants - satiated; n; n--)
    if (rand () % STARVATION_SURVIVAL)
      dead++;
  r->peasants -= dead;

  /* Debug Info.  */
  sprintf (buf, "Bauern: %d, vermehrten sich auf: %d.",
           old_population, new_population);
  adddebug (r, buf);

  sprintf (buf, "Silber: $%d, verbraucht: $%d, neu: $%d.",
           money, money - r->money, r->money);
  adddebug (r, buf);

  sprintf (buf, "Bauern satt: %d, verhungert: %d, neu: %d.",
           satiated, dead, r->peasants);
  adddebug (r, buf);

  assert (r->peasants >= 0);
}

/* ------------------------------------------------------------- */

void
horses (region * r)
{
  int m, n, i, horses, dead, emigrants[MAXDIRECTIONS];

  for (i = 0; i != MAXDIRECTIONS; i++)
    emigrants[i] = 0;

  /* Pferde vermehren sich. */

  horses = r->horses;
  for (n = r->horses; n; n--)
    if (rand () % 100 < HORSEGROWTH)
      r->horses++;

  /* m ist die Anzahl Pferde, fuer die das Land reicht. Alle anderen sterben
     an Hunger. Pferde leben im Schutz der Bauern. Wenn das Land ohne Bauern
     ist, wird der Wald wachsen und die Pferde zurueckdraengen, weil sie im
     wald verhungern. Sollte das sich das Terrain schlagartig veaendern
     (Magie), dann darf m keinen negativen Wert annehmen. */

  m = max (0, production[r->terrain] - r->trees);
  dead = max (0, r->horses - m);
  r->horses -= dead;

  sprintf (buf, "Pferde vorher: %d, jetzt: %d (max.: %d), neu: %d; %d starben.",
           horses, r->horses, m, r->horses - horses, dead);

  /* Pferde wandern, werden als Immigranten abgespeichert */

  for (n = r->horses; n; n--)
    if (rand () % 100 < HORSEMOVE)
      {
        i = rand () % MAXDIRECTIONS;

        if (r->connect[i] && r->connect[i]->terrain != T_OCEAN)
          {
            r->horses--;
            r->connect[i]->newhorses++;
            emigrants[i]++;
          }
      }

  for (i = 0; i != MAXDIRECTIONS; i++)
    if (r->connect[i] && r->connect[i]->terrain != T_OCEAN)
      {
        scat (" Nach ");
        scat (regionid (r->connect[i]));
        scat (" ");
        icat (emigrants[i]);
        scat (".");
      }
  adddebug (r, buf);

  assert (r->horses >= 0);
}

/* ------------------------------------------------------------- */

void
trees (region * r)
{
  int n, m, i, trees, emigrants[MAXDIRECTIONS];

  for (i = 0; i != MAXDIRECTIONS; i++)
    emigrants[i] = 0;

  /* Baeume vermehren sich. m ist die Anzahl Baeume, fuer die es Land gibt.
     Der Wald besiedelt keine bebauten Gebiete, wird den Pferden aber Land
     wegnehmen. gibt es zuviele bauern, werden sie den wald nicht faellen,
     sondern verhungern. der wald kann nur gefaellt werden! der wald wandert
     nicht. Auch bei magischen Terrainveraenderungen darf m nicht negativ
     werden! */

  m = max (0, production[r->terrain] -
           (r->peasants / MAXPEASANTS_PER_AREA));

  /* Solange es noch freie Plaetze gibt, darf jeder Baum versuchen, sich
     fortzupflanzen. Da Baeume nicht sofort eingehen, wenn es keinen Platz
     hat (wie zB. die Pferde), darf nicht einfach drauflos vermehrt werden
     und dann ein min () gemacht werden, sondern es muss auf diese Weise
     vermehrt werden. */

  trees = r->trees;
  for (n = r->trees; n && r->trees < m; n--)
    if (rand () % 100 < FORESTGROWTH)
      r->trees++;

  sprintf (buf, "Baeume vorher: %d, jetzt: %d (max.: %d), neu: %d.",
           trees, r->trees, m, r->trees - trees);

  /* Baeume breiten sich in Nachbarregionen aus */

  for (n = r->trees; n; n--)
    if (rand () % 100 < FORESTSPREAD)
      {
        i = rand () % MAXDIRECTIONS;

        if (!(r->connect[i] && r->connect[i]->terrain != T_OCEAN))
          continue;

        m = max (0, production[r->connect[i]->terrain] -
                 (r->connect[i]->peasants / MAXPEASANTS_PER_AREA));
        if (m > r->connect[i]->trees + r->connect[i]->newtrees)
          {
            r->connect[i]->newtrees++;
            emigrants[i]++;
          }
      }

  for (i = 0; i != MAXDIRECTIONS; i++)
    if (r->connect[i] && r->connect[i]->terrain != T_OCEAN)
      {
        scat (" Nach ");
        scat (regionid (r->connect[i]));
        scat (" ");
        icat (emigrants[i]);
        scat (".");
      }
  adddebug (r, buf);

  assert (r->trees >= 0);
}

/* ------------------------------------------------------------- */

void
demographics (void)
{
  region *r;
  int n, rmax = 0;

  puts ("- Unterhaltskosten, steigende Nachfrage, Seuchen, Vermehrung, Auswanderung,\n"
        "  Pferde, Baeume...");

  for (r = regions; r; r = r->next)
    {
      rmax++;
      r->newpeasants = 0;
      r->newhorses = 0;
      r->newtrees = 0;
    }

  printf ("    Regionen: ");
  indicator_reset (rmax);

  for (r = regions; r; r = r->next)
    {
      indicator_count_down (--rmax);

      live (r);

      if (r->terrain != T_OCEAN)
        {
          /* Die Preise fuer nicht-produzierte Gueter steigt.  */
          for (n = 0; n != MAXLUXURIES; n++)
            if (n == r->produced_good)
	      {
		/* Der Preis fuer produzierte Gueter faellt.  */
		r->demand[n] -= DEMANDFALL;
		r->demand[n] = max (r->demand[n], MINDEMAND);
	      }
	    else
	      {
		/* Der Preis fuer die restlichen Gueter steigt.  */
		r->demand[n] += DEMANDRISE;
		r->demand[n] = min (r->demand[n], MAXDEMAND);
	      }

          /* Seuchen erst nachdem die Bauern sich vermehrt haben und
             gewandert sind */
          peasants (r);
          wanderoff (r, PEASANTMOVE);
          follow_money (r);

          horses (r);
          trees (r);
        }
    }
  putchar ('\n');

  remove_empty_units ();

  puts ("- Einwanderung...");
  for (r = regions; r; r = r->next)
    {
      r->peasants += r->newpeasants;
      r->horses += r->newhorses;
      r->trees += r->newtrees;

      if (r->newpeasants || r->newhorses || r->newtrees)
        {
          sprintf (buf, "Einwanderung: %d Bauern, %d Pferde, %d Baeume.",
                   r->newpeasants, r->newhorses, r->newtrees);
          adddebug (r, buf);
        }
    }

  checkorders ();
}

/* ------------------------------------------------------------- */

void
set_ally (unit * u, strlist * S)
{
  rfaction *rf;
  faction *f;

  f = getfaction ();

  if (f == 0)
    {
      mistake2 (u, S, "Die Partei wurde nicht gefunden");
      return;
    }

  if (f == u->faction)
    return;

  if (getparam () == P_NOT)
    {
      for (rf = u->faction->allies; rf; rf = rf->next)
        if (rf->faction == f)
          {
            removelist (&u->faction->allies, rf);
            break;
          }
    }
  else
    {
      for (rf = u->faction->allies; rf; rf = rf->next)
        if (rf->faction == f)
          break;

      if (!rf)
        {
          rf = cmalloc (sizeof (rfaction));
          rf->faction = f;
          addlist (&u->faction->allies, rf);
        }
    }
}

/* ------------------------------------------------------------- */

void
set_display (region * r, unit * u, strlist * S)
{
  char **s, *s2;

  switch (getparam ())
    {
    case P_BUILDING:
      if (!u->building)
        {
          mistake2 (u, S, "Wir sind in keiner Burg");
          return;
        }
      if (!u->owner)
        {
          mistake2 (u, S, "Das Gebaeude gehoert uns nicht");
          return;
        }
      s = &u->building->display;
      break;

    case P_SHIP:
      if (!u->ship)
        {
          mistake2 (u, S, "Wir sind in keinem Schiff");
          return;
        }
      if (!u->owner)
        {
          mistake2 (u, S, "Das Schiff gehoert uns nicht");
          return;
        }
      s = &u->ship->display;
      break;

    case P_UNIT:
      s = &u->display;
      break;

    case P_REGION:
      if (!u->building)
        {
          mistake2 (u, S, "Wir sind in keiner Burg");
          return;
        }
      if (!u->owner)
        {
          mistake2 (u, S, "Wir sind nicht die Burgherren");
          return;
        }
      if (u->building != largestbuilding (r))
        {
          mistake2 (u, S, "Wir sind nicht die Burgherr der groessten Burg in der Region");
          return;
        }
      s = &r->display;
      break;

    default:
      mistake2 (u, S, "Man muss angeben, ob eine Burg, ein Schiff, "
                "eine Region oder eine Einheit beschrieben "
                "werden soll");
      return;
    }

  s2 = getstr ();
  mnstrcpy (s, s2, DISPLAYSIZE);

}

void
set_name (region * r, unit * u, strlist * S)
{
  char **s, *s2;
  int i;

  switch (getparam ())
    {
    case P_BUILDING:
      if (!u->building)
        {
          mistake2 (u, S, "Wir sind in keiner Burg");
          return;
        }
      if (!u->owner)
        {
          mistake2 (u, S, "Wir sind nicht die Burgherren");
          return;
        }
      s = &u->building->name;
      break;

    case P_FACTION:
      s = &u->faction->name;
      break;

    case P_SHIP:
      if (!u->ship)
        {
          mistake2 (u, S, "Wir sind auf keinem Schiff");
          return;
        }
      if (!u->owner)
        {
          mistake2 (u, S, "Das Schiff gehoert uns nicht");
          return;
        }
      s = &u->ship->name;
      break;

    case P_UNIT:
      s = &u->name;
      break;

    case P_REGION:
      if (!u->building)
        {
          mistake2 (u, S, "Wir sind in keiner Burg");
          return;
        }
      if (!u->owner)
        {
          mistake2 (u, S, "Wir sind nicht die Burgherren");
          return;
        }
      if (u->building != largestbuilding (r))
        {
          mistake2 (u, S, "Wir sind nicht die Burgherr "
                    "der groessten Burg in der Region");
          return;
        }
      s = &r->name;
      break;

    default:
      mistake2 (u, S, "Man muss angeben, ob eine Burg, ein Schiff, "
                "eine Einheit, eine Region oder eine Partei benannt "
                "werden soll");
      return;
    }

  s2 = getstr ();
  if (!s2[0])
    {
      mistake2 (u, S, "Es wurde kein Name angegeben");
      return;
    }

  for (i = 0; s2[i]; i++)
    if (s2[i] == '(')
      break;
  if (s2[i])
    {
      mistake2 (u, S, "Namen duerfen keine Klammern enthalten");
      return;
    }

  mnstrcpy (s, s2, NAMESIZE);
}

/* ------------------------------------------------------------- */

void
deliverMail (faction *recipient, region *r, unit *sender, char *s)
{
  char message[DISPLAYSIZE+1];

  nstrcpy (message, s, DISPLAYSIZE);

  switch (recipient->dh)
    {
    case P_FACTION:
      sprintf (buf, translate (ST_MESSAGE_FROM_FACTION, recipient->language,
			       factionid (sender->faction), message));
      break;

      /* Einheit P_UNIT oder Region P_REGION: die Botschaft ist
         spezifisch für die Region (die Empfänger-Einheit der eigenen
         Partei bleibt unbekannt). */
    default:
      if (cansee (recipient, r, sender))
	sprintf (buf, translate (ST_MESSAGE_FROM_UNIT, recipient->language,
				 unitid (sender), regionid (r), message));
      else
	sprintf (buf, translate (ST_ANONYMOUS_MESSAGE_FROM_UNIT, recipient->language,
				 unitid (sender), regionid (r), message));
      break;
    }
  addmessage (recipient, buf);
}

void
mailunits (region * r, unit * u, strlist *S, int n, char **s)
{
  unit *u2;

  /* wenn n == 0 ist, dann muss *s die Botschaft sein. Bis dahin sind alle
     n's Einheiten, deren Partei die Botschaft bekommen werden. */

  for (;;)
    if (n)
      {
        for (u2 = r->units; u2; u2 = u2->next)
          if (u2->no == n && cansee (u->faction, r, u2))
            {
              u2->faction->dh = P_UNIT;
              break;
            }
        /* falls Einheit nicht gefunden */
        if (!u2)
	  mistake2 (u, S, translate (ST_CANNOT_FIND_UNIT_FOR_MAIL, u->faction->language, 
				     n, regionid (r)));
        /* naechste Zahl */
        *s = getstr ();
        n = atoip (*s);
      }
    else
      break;
}

void
mailfaction (region *r, unit * u, strlist *S, int n, char **s)
{
  /* Die Region r wird nur fuer die Warnung gebraucht.  */

  faction *f;

  /* wenn n == 0 ist, dann muss *s die Botschaft sein. Bis dahin sind alle
     n's Parteien die Botschaft bekommen werden. */
  for (;;)
    if (n)
      {
        f = findfaction (n);
        if (f)
          f->dh = P_FACTION;
        else
	  mistake2 (u, S, translate (ST_CANNOT_FIND_FACTION_FOR_MAIL, u->faction->language, 
				     n, regionid (r)));
        /* naechste Zahl */
        *s = getstr ();
        n = atoip (*s);
      }
    else
      break;
}

void
distributeMail (region *r, unit *u, strlist *S)
{
  faction *f;
  unit *u2;
  char *s;
  int n;

  /* Botschaft an eine andere unit: Die Empfaenger Parteien werden
     markiert.  f->dh == P_REGION oder P_UNIT bedeuted, dass die
     Botschaft an eine Einheit in der betreffenden Region gesendet
     wurde.  P_FACTION bedeuted, dass die Botschaft unabhaengig von
     der Region gelten soll.  Hierfuer ist es nicht noetig, Einheiten
     in derselben Region stationiert zu haben (primitive email).  */
  for (f = factions; f; f = f->next)
    f->dh = 0;
  s = getstr ();

  /* Falls kein Parameter und keine Zahl gegeben, lese den naechsten
     Text (das war dann wohl das Fuellwort "AN"). */
  if (!atoip (s) && findparam (s) == -1)
    s = getstr ();

  /* Allen Parteien mit f->dh > 0 bekommen die Botschaft. Ist f->dh ==
     P_UNIT, kann die Botschaft anonym sein. */
  switch (findparam (s))
    {

    case P_UNIT:
      n = geti ();
      mailunits (r, u, S, n, &s);
      break;

    case P_REGION:
      s = getstr ();
      if (!s[0])
        break;

      for (u2 = r->units; u2; u2 = u2->next)
        if (cansee (u->faction, r, u2))
          u2->faction->dh = P_REGION;
      break;

    case P_FACTION:
      n = geti ();
      mailfaction (r, u, S, n, &s);
      break;

    default:
      /* Falls es eine Zahl ist, soll es als Einheit interpretiert werden. */
      n = atoip (s);
      if (!n)
        {
          mistake2 (u, S, strings[ST_CANNOT_DETERMINE_RECIPIENT][u->faction->language]);
          return;
        };
      mailunits (r, u, S, n, &s);
      break;
    }
  if (!s[0])
    {
      mistake2 (u, S, strings[ST_MESSAGE_EMPTY][u->faction->language]);
      return;
    }
  u->faction->dh = P_UNIT;  /* Die eigene Partei erhaelt immer eine Rueckmeldung.  */
  for (f = factions; f; f = f->next)
    if (f->dh)
      deliverMail (f, r, u, s);
}

void
mail (void)
{
  region *r;
  unit *u;
  strlist *S;

  puts ("- verschicke Botschaften...");

  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      for (S = u->orders; S; S = S->next)
        if (igetkeyword (S->s) == K_MAIL)
          distributeMail (r, u, S);
}

/* ------------------------------------------------------------- */

void
set_passw (void)
{

  /* diese meldungen sollen zuoberst stehen! va. nicht mit den adressen des
     FINDE befehles gemischt. */

  region *r;
  unit *u;
  strlist *S;
  char *s;
  int o, i;

  puts ("- setze Passwoerter, Adressen, und Format...");

  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      for (S = u->orders; S; S = S->next)
        switch (igetkeyword (S->s))
          {
          case -1:
            mistake2 (u, S, "Der Befehl wurde nicht erkannt");
            break;

          case K_ADDRESS:
            s = getstr ();

            if (!s[0])
              {
                mistake2 (u, S, "Es wurde keine Adresse angegeben");
                break;
              }

            mnstrcpy (&u->faction->addr, s, DISPLAYSIZE);
	    if (u->faction->addr)
	      sprintf (buf, "Die Adresse wurde auf '%s' geaendert.", 
		       u->faction->addr);
	    else
	      strcpy (buf, "Die Adresse wurde geloescht.");
            addmessage (u->faction, buf);
            break;

          case K_PASSWORD:
            s = getstr ();
	    mnstrcpy (&u->faction->passw, s, NAMESIZE);
	    if (u->faction->passw)
	      sprintf (buf, "Das Passwort wurde auf '%s' geaendert.", 
		       u->faction->passw);
	    else
	      strcpy (buf, "Das Passwort wurde geloescht.");
            addmessage (u->faction, buf);
            break;

          case K_SEND:
            o = getoption ();
            i = pow (2, o);
            if (getparam () == P_NOT)
              u->faction->options = u->faction->options & ~i;
            else
              u->faction->options = u->faction->options | i;
            break;

          }
}

/* ------------------------------------------------------------- */

void
instant_orders (void)
{
  region *r;
  unit *u, *u2;
  strlist *S;
  char *s;
  int i;

  /* Instant orders - diplomacy etc. */

  puts ("- Hilfe, Status, Kampfzauber, Texte, Bewachen (aus), Zeigen...");

  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      for (S = u->orders; S; S = S->next)
        switch (igetkeyword (S->s))
          {
          case K_ALLY:
            set_ally (u, S);
            break;

          case K_STATUS:
	    if (u->type != U_MAN)
	      {
		mistake2 (u, S, translate (ST_MONSTERS_DONT_CHANGE_STATUS, u->faction->language,
					   strings[typenames[1][u->type]][u->faction->language]));
		break;
	      }
            switch (getparam ())
              {
              case P_NOT:
                u->status = ST_AVOID;
                break;

              case P_BEHIND:
                u->status = ST_BEHIND;
                break;

              default:
                u->status = ST_FIGHT;
              }
            break;

	  case K_COLLECT:
	    if (getparam () == P_LOOT)
	      {
		/* Der letzte SAMMEL BEUTE Befehl gilt; er ueberschreibt alle Vorgaenger.  */
		for (u2 = r->units; u2; u2 = u2->next)
		  if (u2->faction == u->faction)
		    u2->collector = u;
		/* Damit beim normalen SAMMEL Befehl kein Fehler generiert wird, wird dieser SAMMEL BEUTE
                   Befehl hier geloescht.  */
		S->s[0] = 0;
	      }
            break;

          case K_COMBAT:
            s = getstr ();

            if (!s[0])
              {
                u->combatspell = -1;
                break;
              }

            i = findspell (s);

            if (i < 0 || !cancast (u, i))
              {
                mistake2 (u, S, "Diesen Zauber kennt die Einheit nicht");
                break;
              }

            if (!iscombatspell[i])
              {
                mistake2 (u, S, "Das ist kein Kampfzauber");
                break;
              }

            u->combatspell = i;
            break;

          case K_DISPLAY:
            set_display (r, u, S);
            break;

          case K_NAME:
            set_name (r, u, S);
            break;

          case K_GUARD:
            if (getparam () == P_NOT)
              u->guard = 0;
            break;

          case K_RESHOW:
            s = getstr ();
	    if (findparam (s) == P_ALL)
	      {
		for (i = 0; i != MAXSPELLS; i++)
		  u->faction->showdata[i] = u->faction->seendata[i];
		break;
	      }
	    i = findspell (s);
	    if (i < 0 || !u->faction->seendata[i])
	      {
		mistake2 (u, S, "Der Zauber wurde nicht gefunden");
		break;
	      }
	    u->faction->showdata[i] = 1;
	    break;
          }
}

/* ------------------------------------------------------------- */

void
last_orders (void)
{
  region *r;
  unit *u;
  strlist *S;

  /* letzte schnellen befehle - bewache */

  puts ("- Bewache (an)...");

  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      if (u->type != U_ILLUSION)
	for (S = u->orders; S; S = S->next)
	  if (igetkeyword (S->s) == K_GUARD)
	    if (getparam () != P_NOT)
	      u->guard = 1;
}

/* Setzt Default Befehle -------------------------------------- */

void
setdefaults (void)
{
  region *r;
  unit *u;
  strlist *S;
  int i;
  int new_delivery;

  puts ("- setze neue Default-Befehle...");

  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      {
	new_delivery = 0;
        for (S = u->orders; S; S = S->next)
	  {
	    i = igetkeyword (S->s);
	    switch (i)
	      {
		/* Geladen wird der alte Default Befehl in lastorder.  Wenn unter den Befehlen der Einheit ein
		   neuer langer Befehl ist, wird dieser nach thisorder kopiert.  thisorder ist der
		   diesmonatige Default.  Falls thisorder tatsaechlich gesetzt wurde, wird thisorder nach
		   lastorder kopiert und ueberschreibt den alten Default.  Dies wird nur unterlassen, falls
		   thisorder der NACH Befehl ist.  Falls thisorder nicht gesetzt wurde, wird lastorder nach
		   thisorder kopiert.  lastorder wird abgespeichert.  */

		/* Falls wir MACHE TEMP haben, zaehlt MACHE nicht als neuer Default (es ist ein kurzer Befehl).
		   Alle anderen Variationen von MACHE zaehlen als Defaults und die Bearbeitung wird nicht mittels
		   'break' abgebrochen.  */
	      case K_MAKE:
		if (getparam () == P_TEMP)
		  break;
	      case K_BESIEGE:
	      case K_BUY:
	      case K_CAST:
	      case K_ENTERTAIN:
	      case K_MOVE:
	      case K_FOLLOW:
	      case K_RESEARCH:
	      case K_SELL:
	      case K_STEAL:
	      case K_STUDY:
	      case K_TAX:
	      case K_TEACH:
	      case K_WORK:
		mstrcpy (&u->thisorder, S->s);

		/* thisorder wird nun nach lastorder kopiert, wenn thisorder nicht der NACH Befehl war.  Da
                   Monster ihre Befehle nicht abspeichern, ist dies fuer sie unnoetig.  Monster setzen ihre
                   Befehle in plan_monster () jede Runde neu.  */
		if (i != K_MOVE && u->type == U_MAN)
		  mstrcpy (&u->lastorder, u->thisorder);

		break;
		/* Wird je diese Ausschliesslichkeit aufgehoben, muss man aufpassen mit der Reihenfolge von
		   Kaufen, Verkaufen etc., damit es Spielern nicht moeglich ist, Schulden zu machen.  */

		/* Gleichzeitig ist der LIEFERE Befehl der 2. Default, falls es einen gibt.  LIEFERE NICHT
                   beendet den LIEFERE Befehl.  thisorder2 wird nur verwendet, um den letzten LIEFERE Befehl
                   zu speichern.  Ansonsten wird LIEFERE genauso wie GIB behandelt!  LIEFERE kann deshalb
                   sogar mehrmals vorkommen (der letzte LIEFERE Befehl wird gespeichert) -- dann werden
                   einfach mehrere GIB Befehle ausgefuehrt.  Falls allerdings kein neue LIEFERE Befehl gesetzt
                   wurde, wird aus thisorder2 ein normaler Befehl generiert (spaeter).  */
	      case K_DELIVER:
		new_delivery = 1;
		if (getparam () == P_NOT)
		  {
		    free (u->thisorder2);
		    u->thisorder2 = 0;
		    /* Dies erlaubt es uns spaeter nicht mehr danach testen zu muessen.  */
		    S->s[0] = 0;
		  }
		else
		  /* Dies ist nur eine Kopie zum Speichern, ausgefuehrt wird der Befehl in S->s.  Gibt es
                     keinen Befehl in S->s und keine new_delivery, dann wird u->thisorder2 nach S->s kopiert.
                     Das folgt weiter unten.  */
		  mstrcpy (&u->thisorder2, S->s);
		break;
	      }
	  }
	
	/* Falls kein thisorder und ein lastorder existiert, verwenden wir lastorder als thisorder.  Das
	   bedeuted, dass kein neuer Default gesetzt wurde, und somit der alte verwendet werden muss.  */
	if (!u->thisorder && u->lastorder)
	  mstrcpy (&u->thisorder, u->lastorder);

	/* Falls noch immer kein thisorder existiert -- zB. bei Monstern mit eingeschraenkten Moeglichkeiten
           -- wird hier ein leerer thisorder generiert (damit kein Konflikt mit igetstr (u->thisorder)
           auftritt -- wenn dort naemlich u->thisorder==0x0 ist, funktioniert igetstr wie getstr (liest also
           am alten String weiter).  Bei thisoder2 ist diese Vorsicht nicht angebracht, weil thisorder2 ausser
           beim report, beim laden und speichern, sowie im Rest dieser Funktion hier gebraucht wird.  */
	if (!u->thisorder)
	  mstrcpy (&u->thisorder, "");

	/* Falls kein neuer LIEFERE Befehl gegeben worden ist, ein alter thisorder2 Befehl aber existiert,
           dann wird ein neuer Befehl generiert und in orders abgespeichert.  */
	if (!new_delivery && u->thisorder2)
	  addstrlist (&u->orders, u->thisorder2);
      }
}




