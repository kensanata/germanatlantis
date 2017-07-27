/* German Atlantis PB(E)M host Copyright (C) 1995-1998   Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "atlantis.h"

/* ------------------------------------------------------------- */

int
armedmen (unit * u)
{
  int n;

  n = 0;

  switch (u->type)
    {
    case U_MAN:

      /* alle Waffen werden gezaehlt, und dann wird auf die Anzahl Personen
         minimiert */

      if (effskill (u, SK_SWORD))
        n += u->items[I_SWORD];
      if (effskill (u, SK_SPEAR))
        n += u->items[I_SPEAR];
      if (effskill (u, SK_CROSSBOW))
        n += u->items[I_CROSSBOW];
      if (effskill (u, SK_LONGBOW))
        n += u->items[I_LONGBOW];
      n = min (n, u->number);
      break;

    case U_ILLUSION:

      /* eigentlich unnoetig, da sie sowieso keine skills haben */

      break;

    default:
      n = u->number;            /* fuer untote und drachen */
      break;

    }
  return n;
}

/* ------------------------------------------------------------- */

void
reportevent (region * r, char *s)
{
  faction *f;
  unit *u;

  for (f = factions; f; f = f->next)
    for (u = r->units; u; u = u->next)
      if (u->faction == f && u->number)
        {
          addevent (f, s);
          break;
        }
}

/* ------------------------------------------------------------- */

building *
getbuilding (region * r)
{
  int n;
  building *b;

  n = geti ();

  for (b = r->buildings; b; b = b->next)
    if (b->no == n)
      return b;

  return 0;
}

ship *
getship (region * r)
{
  int n;
  ship *sh;

  n = geti ();

  /* Die Schiffsnummer kommt vom Input, und nach dieser Nummer wird nun in
     der aktuellen Region gesucht */

  for (sh = r->ships; sh; sh = sh->next)
    if (sh->no == n)
      return sh;

  return 0;
}

building *
findbuilding (int n)
{
  region *r;
  building *b;

  for (r = regions; r; r = r->next)
    for (b = r->buildings; b; b = b->next)
      if (b->no == n)
        return b;

  return 0;
}

ship *
findship (int n)
{
  region *r;
  ship *sh;

  for (r = regions; r; r = r->next)
    for (sh = r->ships; sh; sh = sh->next)
      if (sh->no == n)
        return sh;

  return 0;
}

/* ------------------------------------------------------------- */

void
siege (void)
{
  faction *f;
  region *r;
  unit *u, *u2;
  building *b;
  int d;

  puts ("- belagern...");

  for (r = regions; r; r = r->next)
    if (r->terrain != T_OCEAN)
      for (u = r->units; u; u = u->next)
        if (igetkeyword (u->thisorder) == K_BESIEGE)
          {
            /* gibt es ueberhaupt Burgen? */

            b = getbuilding (r);

            if (!b)
              {
                mistakeu (u, strings[ST_BUILDING_NOT_FOUND][u->faction->language]);
                continue;
              }

            /* keine Drachen, Illusionen, Untote etc */

            if (u->type != U_MAN)
              {
                mistakeu (u, translate (ST_MONSTERS_CANT_LAY_SIEGE, u->faction->language,
					strings[typenames[1][u->type]][u->faction->language]));
                continue;
              }

            /* Schaden durch Katapulte */

            d = min (u->number, u->items[I_CATAPULT]);
            d *= effskill (u, SK_CATAPULT);

            /* Abbruch, falls unbewaffnet oder unfaehig, Katapulte zu
               benutzen.  */

            if (!armedmen (u) && !d)
              {
                mistakeu (u, strings[ST_UNIT_UNARMED][u->faction->language]);
                continue;
              }

            /* Abbruch, wenn die Einheit nicht vorher die Region
               bewacht - als Warnung fuer alle anderen! */

            if (!u->guard)
              {
                mistakeu (u, strings[ST_UNIT_MUST_GUARD][u->faction->language]);
                continue;
              }

            /* Einheit und Burg markieren - spart Zeit beim behandeln
               der Einheiten in der Burg, falls die Burg auch markiert
               ist und nicht alle Einheiten wieder abgesucht werden
               muessen! */

            u->besieging = b;
            b->besieged = 1;

            /* Definitiver Schaden eingeschraenkt */

            d = min (d, u->besieging->size - 1);
	    u->besieging->size -= d;

            /* Meldung fuer Belagerer und Burginsassen verteilen.  */

            for (f = factions; f; f = f->next)
              f->dh = 0;

            u->faction->dh = 1;
            for (u2 = r->units; u2; u2 = u2->next)
              if (u2->building == u->besieging && !u2->faction->dh)
		u2->faction->dh = 1;

            for (f = factions; f; f = f->next)
              if (f->dh)
		{
		  if (d)
		    strcpy (buf, translate (ST_IS_BESIEGED_WITH_CATAPULTS_BY, u->faction->language, 
					    unitid (u), buildingid (u->besieging), 
					    100 * d / u->besieging->size));
		  else
		    strcpy (buf, translate (ST_IS_BESIEGED_BY, u->faction->language,
					    unitid (u), buildingid (u->besieging)));
		  addevent (u->faction, buf);
		}
          }
}

/* ------------------------------------------------------------- */

void
destroy (void)
{
  region *r;
  unit *u, *u2;
  strlist *S;
  building *b;
  ship *sh;
  int n;

  puts ("- zerstoeren...");

  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      for (S = u->orders; S; S = S->next)
	if (igetkeyword (S->s) == K_DESTROY)
	  {
	    if (!u->owner)
	      {
		mistake2 (u, S, strings[ST_CANNOT_DESTROY][u->faction->language]);
		continue;
	      }
	    
	    if (u->building)
	      {
		b = u->building;
		
		/* vergib die Steine */
		
		for (n = b->size; n; n--)
		  if (rand () % 100 < STONERECYCLE)
		    u->items[I_STONE]++;
		
		/* befoerdere alle Einheiten hinaus */
		for (u2 = r->units; u2; u2 = u2->next)
		  if (u2->building == b)
		    {
		      u2->building = 0;
		      u2->owner = 0;
		    }

		reportevent (r, translate (ST_IS_BURNED_DOWN, 
					   u->faction->language, 
					   unitid (u), buildingid (b)));
		removelist (&r->buildings, b);
	      }
	    else if (u->ship)
	      {
		if (r->terrain == T_OCEAN)
		  {
		    mistake2 (u, S, strings[ST_SHIP_NOT_ON_SHORE][u->faction->language]);
		    continue;
		  }
		
		sh = u->ship;
		
		for (u2 = r->units; u2; u2 = u2->next)
		  if (u2->ship == sh)
		    {
		      u2->ship = 0;
		      u2->owner = 0;
		    }

		reportevent (r, translate (ST_IS_SUNK, 
					   u->faction->language, 
					   unitid (u), shipid (sh)));
		removelist (&r->ships, sh);
	      }
	    else
	      printf ("Die Einheit %s von %s war owner eines objects, "
		      "war aber weder in einer Burg noch in einem Schiff.\n",
		      unitid (u), factionid (u->faction));
	  }
}

/* ------------------------------------------------------------- */

void
build_road (region * r, unit * u)
{
  int n;

  if (!effskill (u, SK_ROAD_BUILDING))
    {
      mistakeu (u, "Keiner hier kann Strassen bauen");
      return;
    }

  if (besieged (u))
    {
      sprintf (buf, "%s wird belagert", buildingid (u->building));
      mistakeu (u, buf);
      return;
    }

  if (!roadreq[r->terrain])
    {
      sprintf (buf, "Niemand kann durch %s Strassen oder Bruecken "
               "bauen", strings[terrainnames[r->terrain]][u->faction->language]);
      mistakeu (u, buf);
      return;
    }

  if (!u->items[I_STONE])
    {
      mistakeu (u, "Zum Strassenbau braucht man Steinquader");
      return;
    }

  /* n kann man noch bauen */

  n = roadreq[r->terrain] - r->road;

  /* hoffentlich ist r->road <= roadreq[r->terrain], n also >= 0 */

  if (n <= 0)
    {
      sprintf (buf, "In %s gibt es keine Bruecken und Strassen "
               "mehr zu bauen", regionid (r));
      mistakeu (u, buf);
      return;
    }

  /* max bauten anhand des talentes */
  n = min (n, u->number * effskill (u, SK_ROAD_BUILDING));

  /* und anhand der rohstoffe */
  n = min (n, u->items[I_STONE]);

  r->road += n;
  u->items[I_STONE] -= n;

  /* Nur soviel PRODUCEEXP wie auch tatsaechlich gemacht wurde */

  u->skills[SK_ROAD_BUILDING] += min (n, u->number) * PRODUCEEXP;

  addevent (u->faction, translate (ST_BUILDS_ROAD, u->faction->language, unitid (u), regionid (r), n, 
				   strings[itemnames[n > 1][I_STONE]][u->faction->language]));
}

/* ------------------------------------------------------------- */

int
buildingeffsize (building *b)
{
  int i;

  if (!b)
    return 0;

  for (i = MAXBUILDINGS - 1; i; i--)
    if (b->size >= buildingcapacity[i])
      break;
  return i;
}

/* ------------------------------------------------------------- */

building *
create_building (region *r, unit *u)
{
  building *b;

  b = cmalloc (sizeof (building));
  memset (b, 0, sizeof (building));
  
  do
    b->no++;
  while (findbuilding (b->no));
  
  sprintf (buf, "Burg %d", b->no);
  mnstrcpy (&b->name, buf, NAMESIZE);
  addlist (&r->buildings, b);
  
  /* Die Einheit befindet sich automatisch im Inneren der neuen Burg. */
  
  leave (r, u);
  u->building = b;
  u->owner = 1;

  return b;
}

void
build_building (region *r, unit *u)
{
  building *b;
  int newbuilding, l, n, m;

  l = effskill (u, SK_BUILDING);
  if (!l)
    {
      mistakeu (u, strings[ST_NOBODY_CAN_BUILD][u->faction->language]);
      return;
    }

  if (!u->items[I_STONE])
    {
      mistakeu (u, strings[ST_BUILDING_REQUIRES_STONES][u->faction->language]);
      return;
    }

  /* Baulimite gemäss Talent und Rohstoffe.  n > 0 ist garantiert
     wegen den obigen Tests.  */
  n = u->number * l;
  n = min (n, u->items[I_STONE]);
  assert (n > 0);

  newbuilding = 0;

  b = getbuilding (r);

  /* Falls eine Nummer angegeben worden ist, und ein Gebaeude mit der
     betreffenden Nummer existiert, ist b nun gueltig. Wenn keine Burg
     gefunden wurde, dann wird einfach eine neue erbaut, falls man in keiner
     burg ist. Ansonsten baut man an der eigenen burg weiter. */

  if (!b)
    b = u->building;

  /* Falls man belagert ist und eine neue Burg erbauen will.  */
  if (!b && besieged (u))
    {
      mistakeu (u, translate (ST_IS_BESIEGED, u->faction->language,
			      buildingid (u->building)));
      return;
    }

  /* Falls an einer belagerten Burg von aussen angebaut werden will.  */
  if (b && b->besieged && u->building != b)
    {
      mistakeu (u, translate (ST_IS_BESIEGED, u->faction->language,
			      buildingid (b)));
      return;
    }

  /* Neubau falls noetig */
  if (!b)
    {
      b = create_building (r, u);
      assert (b);
      newbuilding = 1;
    }
  
  /* Bis zu Schwierigkeit l+1 kann man bauen.  Hat man ein Talent von
     2, kann man "an 2 weiterbauen", aber 3 nicht erreichen.  n
     Quader-Einheiten kann man noch dazu bauen, dh. wenn man 3 nicht
     erreichen kann, kann man mit 2 genause soviele Quader bauen, dass
     die Groesse eines unter der Groesse von 3 bleiben wuerde.  Diese
     Limite gibt es für Zitadellen natürlich nicht.  */
  if (l+1 < MAXBUILDINGS)
    {
      /* Nun rechnen wir aus, wieviel noch fehlt bis zur maximalen
         Grösse, welche die Einheit erreichen kann.  */
      m = buildingcapacity[l+1] - b->size - 1;

      /* Falls zuwenig Talent um weiterzubauen.  */
      if (m < 0)
	{
	  mistakeu (u, translate (ST_CONTINUING_NEEDS_MORE_TALENT, u->faction->language,
				  buildingid (b), buildingeffsize (b)));
	  return;
	}

      /* Falls Grenze genau erreicht.  */
      if (m == 0)
	{
	  mistakeu (u, translate (ST_TALENT_IS_INSUFFICIENT, u->faction->language,
				  buildingid (b)));
	  return;
	}
      n = min (n, m);
    }

  b->size += n;
  u->items[I_STONE] -= n;

  /* Nur soviel PRODUCEEXP wie auch tatsaechlich gemacht wurde */

  u->skills[SK_BUILDING] += min (n, u->number) * PRODUCEEXP;

  if (newbuilding)
    addevent (u->faction, translate (ST_STARTS_TO_BUILD, u->faction->language,
				     unitid (u), buildingid (b), n));
  else
    addevent (u->faction, translate (ST_CONTINUES_TO_BUILD, u->faction->language,
				     unitid (u), buildingid (b), n));
}

int
build_ship (unit * u, ship * sh)
{
  int n;

  n = u->number * effskill (u, SK_SHIPBUILDING);
  n = min (n, sh->left);
  n = min (n, u->items[I_WOOD]);
  sh->left -= n;
  u->items[I_WOOD] -= n;

  /* Nur soviel PRODUCEEXP wie auch tatsaechlich gemacht wurde */

  u->skills[SK_SHIPBUILDING] += min (n, u->number) * PRODUCEEXP;

  return n;
}

void
create_ship (region * r, unit * u, int newtype)
{
  ship *sh;
  int n;

  if (!effskill (u, SK_SHIPBUILDING))
    {
      mistakeu (u, "Keiner hier ist gelernter Schiffsbauer");
      return;
    }

  if (besieged (u))
    {
      sprintf (buf, "%s wird belagert", buildingid (u->building));
      mistakeu (u, buf);
      return;
    }

  if (!u->items[I_WOOD])
    {
      mistakeu (u, "Fuer den Schiffsbau braucht man Holz");
      return;
    }

  if (effskill (u, SK_SHIPBUILDING) < newtype)
    {
      sprintf (buf, "Um %s zubauen, braucht man ein Talent von mindestens %d",
	       shiptypes[1][newtype], newtype);
      mistakeu (u, buf);
      return;
    }

  sh = cmalloc (sizeof (ship));
  memset (sh, 0, sizeof (ship));

  sh->type = newtype;
  sh->left = shipcost[newtype];

  do
    sh->no++;
  while (findship (sh->no));
  sprintf (buf, "Schiff %d", sh->no);
  mnstrcpy (&sh->name, buf, NAMESIZE);

  addlist (&r->ships, sh);

  leave (r, u);
  u->ship = sh;
  u->owner = 1;

  /* Effektiv verbaute Holzstaemme abziehen etc., und eine Meldung.  */
  n = build_ship (u, sh);
  sprintf (buf, "%s beginnt fuer %d an %s zu bauen.", unitid (u), n, shipid (sh));
  addevent (u->faction, buf);

  /* Den default Befehl aendern!  */
  sprintf (buf, "%s %s %d", keywords[K_MAKE], parameters[P_SHIP], sh->no);
  mstrcpy (&u->lastorder, buf);
}

void
continue_ship (region * r, unit * u)
{
  ship *sh;
  int n;

  if (!effskill (u, SK_SHIPBUILDING))
    {
      mistakeu (u, "Keiner hier ist gelernter Schiffsbauer");
      return;
    }

  if (!u->items[I_WOOD])
    {
      mistakeu (u, "Fuer den Schiffsbau braucht man Holz");
      return;
    }

  /* Die Schiffsnummer bzw der Schiffstyp wird eingelesen */

  sh = getship (r);

  if (!sh)
    sh = u->ship;

  if (!sh)
    {
      mistakeu (u, "Das Schiff wurde nicht gefunden");
      return;
    }

  if (!sh->left)
    {
      mistakeu (u, "Das Schiff ist schon fertig");
      return;
    }

  if (effskill (u, SK_SHIPBUILDING) < sh->type)
    {
      sprintf (buf, "Um %s zu bauen, braucht man ein Talent von "
               "mindestens %d", shiptypes[1][sh->type], sh->type);
      mistakeu (u, buf);
      return;
    }

  n = build_ship (u, sh);
  sprintf (buf, "%s baut fuer %d an %s weiter.", unitid (u), n, shipid (sh));
  addevent (u->faction, buf);
}

/* ------------------------------------------------------------- */

unit *
buildingowner (region * r, building * b)
{
  unit *u;

  for (u = r->units; u; u = u->next)
    if (u->building == b && u->owner)
      return u;

  return 0;
}

unit *
shipowner (region * r, ship * sh)
{
  unit *u;

  for (u = r->units; u; u = u->next)
    if (u->ship == sh && u->owner)
      return u;

  return 0;
}

int
mayenter (region * r, unit * u, building * b)
{
  unit *u2;

  u2 = buildingowner (r, b);

  return (!u2 || contacts (r, u2, u));
}

int
mayboard (region * r, unit * u, ship * sh)
{
  unit *u2;

  u2 = shipowner (r, sh);

  return (!u2 || contacts (r, u2, u));
}

/* ------------------------------------------------------------- */

void
enter (void)
{
  region *r;
  unit *u, *u2;
  strlist * S;
  ship *sh;
  building *b;

  /* Betreten von Schiffen und Burgen vor der Kommando uebergabe. */

  puts ("- betreten...");

  for (r = regions; r; r = r->next)
    for (u = r->units; u;)
      {

        /* 'u = u->next' darf nicht in der for () Schlaufe stehen, da sich
           u->next aendern kann. Deswegen der Umweg ueber u2 - analog zu
           movement (). Genauso wie in move () dann thisorder[0]=0 gesetzt
           wird, muss hier S->s[0]=0 gesetzt werden, damit beim
           vertauschen der Reihenfolge nicht Fehler o.ae. zweimal
           auftreten. */

        u2 = u->next;

        for (S = u->orders; S; S = S->next)
          if (igetkeyword (S->s) == K_ENTER)
            {
              switch (getparam ())
                {
                  case P_BUILDING:
                    b = getbuilding (r);
                    if (!b)
                      {
                        mistake2 (u, S, "Die Burg wurde nicht gefunden");
                        break;
                      }
                    if (!mayenter (r, u, b))
                      {
                        mistake2 (u, S, "Der Eintritt wurde verwehrt");
                        break;
                      }
                    if (!slipthru (r, u, b))
                      {
                        sprintf (buf, "%s wird belagert",
                                 buildingid (b));
                        mistake2 (u, S, buf);
                        break;
                      }
                    leave (r, u);
                    u->building = b;
                    u->owner = 0;
                    if (buildingowner (r, b) == 0)
                      {
                        u->owner = 1;
                        /* u kommt wird zur "aeltesten" Einheit in der
                           Region.  Burgherren werden somit auch nicht
                           so schnell verhungern, weil sie ploetzlich
                           so weit oben in der Hierarchie stehen.
                           Hier aendert sich auch u->next.  */
			/* Neu: Das Verhalten in hungernden Regionen
                           ist undefiniert!  */
                        /* promotelist (&r->units, u); */
                      }
                    break;

                  case P_SHIP:
                    sh = getship (r);
                    if (!sh)
                      {
                        mistake2 (u, S, "Das Schiff wurde nicht gefunden");
                        break;
                      }
                    if (!mayboard (r, u, sh))
                      {
                        mistake2 (u, S, "Die Einheit darf nicht an Bord "
				  "kommen");
                        break;
                      }
                    leave (r, u);
                    u->ship = sh;
                    u->owner = 0;
                    if (shipowner (r, sh) == 0)
                      u->owner = 1;
                    break;

                  default:
                    mistake2 (u, S, "Ein Schiff oder eine Burg muss "
			      "angegeben werden");

                }
              S->s[0] = 0;
            }
        u = u2;
      }
}

/* ------------------------------------------------------------- */

void
leaving (void)
{
  region *r;
  unit *u;
  strlist * S;

  /* Verlassen von Schiffen und Burgen nachdem Kommando uebergeben
     wurde. */

  puts ("- verlassen...");

  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      for (S = u->orders; S; S = S->next)
        if (igetkeyword (S->s) == K_LEAVE)
          {
            if (r->terrain == T_OCEAN && u->ship)
              {
                mistake2 (u, S, "Das Schiff befindet sich auf hoher See");
                continue;
              }

            if (!slipthru (r, u, u->building))
              {
                sprintf (buf, "%s wird belagert",
                         buildingid (u->building));
                mistake2 (u, S, buf);
                continue;
              }

            leave (r, u);
          }
}

/* ------------------------------------------------------------- */

void
givecommand (void)
{
  region *r;
  unit *u, *u2, *new_commander;
  strlist * S;

  /* Kommando Uebergabe nach betreten und vor verlassen der Objekte */

  puts ("- Kommando Uebergabe...");

  for (r = regions; r; r = r->next)
    for (u = r->units; u;)
      {

        /* 'u = u->next' darf nicht in der for () Schlaufe stehen, da
           sich u->next aendern kann. Deswegen der Umweg ueber u2 -
           analog zu movement (). Genauso wie in move () dann
           thisorder[0]=0 gesetzt wird, muss hier S->s[0]=0 gesetzt
           werden, damit beim vertauschen der Reihenfolge nicht Fehler
           o.ae. zweimal auftreten. S->s[0]=0 darf aber nicht am Anfang
           gesetzt werden, deswegen die lange "else if" Konstruktion, da
           man auch kein "continue" verwenden kann (sonst wuerde
           S->s[0]=0 am Ende uebersprungen werden). */

        u2 = u->next;

        for (S = u->orders; S; S = S->next)
          if (igetkeyword (S->s) == K_GIVE)
            {
              new_commander = getunit (r, u);

              if (getparam () == P_CONTROL)
                {
                  if (!new_commander)
                    mistake2 (u, S, "Die Einheit wurde nicht gefunden");
                  else if (!u->building && !u->ship)
                    mistake2 (u, S, "Wir befinden uns weder in einer Burg noch in einem Schiff");
                  else if (!u->owner)
                    mistake2 (u, S, "Wir haben weder das Kommando ueber eine Burg noch ueber ein Schiff");
                  else if (!contacts (r, new_commander, u))
                    mistake2 (u, S, "Die Einheit hat keinen Kontakt mit uns aufgenommen");
                  else if (u->building && new_commander->building != u->building)
                    mistake2 (u, S, "Die Einheit befindet sich nicht in unserer Burg");
                  else if (u->ship && new_commander->ship != u->ship)
                    mistake2 (u, S, "Die Einheit befindet sich nicht an Bord unseres Schiffes");
                  else
                    {
                      u->owner = 0;
                      new_commander->owner = 1;

                      /* new_commander wird zur "aeltesten" Einheit in der
                         Region - da nun an erster Stelle. Damit ist sie auch
                         automatisch die die erste Einheit in der Burg.
                         Burgherren werden somit auch nicht so schnell
                         verhungern, weil sie ploetzlich so weit oben in der
                         Hierarchie stehen.

                         Obwohl promotelist () nur fuer Burgen noetig waere
                         (dort koennen mehr Leute in der Burg sein, als diese
                         gross ist, so dass nicht mehr klar ist, wer von der
                         Burg geschuetzt wird), passiert dies hier auch fuer
                         Schiffe, damit der Effekt immer gleich ist. */
		      
		      /* Neu: Verhalten undefiniert.  */

/*                       promotelist (&r->units, new_commander); */

                      sprintf (buf, "%s uebergibt das Kommando ",
                               unitid (u));
                      if (u->building)
                        scat ("der Burg");
                      if (u->ship)
                        scat ("des Schiffes");
                      scat (" an ");
                      scat (unitid (new_commander));
                      scat (".");

                      addevent (u->faction, buf);
                      if (u->faction != new_commander->faction)
                        addevent (new_commander->faction, buf);
                    }
                  S->s[0] = 0;
                }
            }
        u = u2;
      }
}

/* ------------------------------------------------------------- */










