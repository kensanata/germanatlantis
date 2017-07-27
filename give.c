/* German Atlantis PB(E)M host Copyright (C) 1995-1998   Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed. It may not be
   sold or used commercially without prior written permission from the
   author.

 */

#include "atlantis.h"
 
int
canlearn (unit *u, int i)
{
  /* Eine Einheit kann einen Spruch lernen, wenn ihr halbes Magietalent (aufgerundet) gleich gross
     oder groesser wie die Stufe des Spruches ist.  */
  return ((effskill (u, SK_MAGIC) + 1) / 2 >= spelllevel[i]);
}

void
givespell (int i, unit *u, unit *u2, strlist *S)
{
  /* Nocheinmal, da man weder an Bauern noch an Niemand einen Spruch geben
     kann. */

  if (!u2)
    if (getunitpeasants)
      {
        mistake2 (u, S, "Die Bauern koennen nicht zaubern");
        return;
      }
    else
      {
        mistake2 (u, S, "Die Einheit wurde nicht gefunden");
        return;
      }

  if (!u->spells[i])
    {
      mistake2 (u, S, "Die Einheit beherrscht diesen Spruch gar nicht");
      return;
    }

  if (!canlearn (u2, i))
    {
      mistake2 (u, S, "Die Einheit kann nicht gut genug zaubern, um diesen Spruch zu lernen");
      return;
    }

  u2->spells[i] = 1;

  addevent (u->faction, translate (ST_GIVE_SPELL, u->faction->language, unitid (u), unitid2 (u2),
				   strings[spellnames[i]][u->faction->language]));
  if (u->faction != u2->faction)
    addevent (u2->faction, translate (ST_GIVE_SPELL, u2->faction->language, unitid (u), unitid2 (u2),
				      strings[spellnames[i]][u->faction->language]));

  if (!u2->faction->seendata[i])
    {
      u2->faction->seendata[i] = 1;
      u2->faction->showdata[i] = 1;
    }
}

void
giveallspells (unit  *u, unit *u2, strlist *S)
{
  int i;

  sprintf (buf, "%s leiht ", unitid (u));
  scat (unitid (u2));
  scat (" das Zauberbuch aus.");
  addevent (u->faction, buf);
  if (u->faction != u2->faction)
    addevent (u2->faction, buf);

  /* Dieser code steht aehnlich auch in givemen. */
  for (i = 0; i != MAXSPELLS; i++)
    if (u->spells[i] && !u2->spells[i] && canlearn (u2, i))
      givespell (i, u, u2, S);
}

void
giveitem (int n, int i, unit *u, region *r, unit *u2, strlist *S)
{
  if (n > u->items[i])
    n = u->items[i];

  if (n == 0)
    {
      mistake2 (u, S, "Die Einheit hat diesen Gegenstand nicht");
      return;
    }

  u->items[i] -= n;

  if (!u2)
    {
      if (i == I_HORSE)
        r->horses += n;

      if (getunitpeasants)
	addcommerce (u->faction, translate (ST_GIVE_PEASANTS_ITEMS, u->faction->language, unitid (u), n, 
					    strings[itemnames[n != 1][i]][u->faction->language]));
      else
	addcommerce (u->faction, translate (ST_GIVE_NOBODY_ITEMS, u->faction->language, unitid (u), n, 
					    strings[itemnames[n != 1][i]][u->faction->language]));
      return;
    }

  u2->items[i] += n;

  if (u->faction != u2->faction)
    {
      addcommerce (u->faction, translate (ST_GIVE_UNIT_ITEMS, u->faction->language, unitid (u), n, 
					  strings[itemnames[n != 1][i]][u->faction->language], unitid2 (u2)));
      addcommerce (u2->faction, translate (ST_GIVE_UNIT_ITEMS, u2->faction->language, unitid (u), n, 
					   strings[itemnames[n != 1][i]][u2->faction->language], unitid2 (u2)));
    }
  else
    addevent (u->faction, translate (ST_GIVE_UNIT_ITEMS, u->faction->language, unitid (u), n, 
				     strings[itemnames[n != 1][i]][u->faction->language], unitid2 (u2)));
}

void
givemen (int n, unit * u, region * r, unit * u2, strlist * S)
{
  int i, j, k;

  if (n > u->number)
    n = u->number;

  if (n == 0)
    {
      mistake2 (u, S, strings[ST_NO_MEN_TO_GIVE][u->faction->language]);
      return;
    }

  /* Falls Magier transferiert werden: */
  if (u->skills[SK_MAGIC] && u2)
    {

      /* Zaehle Magier in der Zielpartei: */
      k = magicians (u2->faction);

      /* Falls die Zieleinheit keine Magier sind, werden sie nun welche. */
      if (!u2->skills[SK_MAGIC])
        k += u2->number;

      /* Falls sie in eine neue Partei kommen, zaehlen sie selber auch mit
         zum Kontigent an Magier der Zielpartei. */
      if (u2->faction != u->faction)
        k += n;

      /* Falls dies nun zuviele sind gibt es einen Fehler */
      if (k > MAXMAGICIANS)
        {
          mistake2 (u, S, translate (ST_MAX_MAGICIANS, u->faction->language,
				     MAXMAGICIANS));
          return;
        }
    }

  k = u->number - n;

  /* Transfer Talente */
  for (i = 0; i != MAXSKILLS; i++)
    {
      j = distribute (u->number, k, u->skills[i]);
      if (u2)
        u2->skills[i] += u->skills[i] - j;
      u->skills[i] = j;
    }

  u->number = k;

  if (u2)
    {
      u2->number += n;

      /* Transfer der Sprueche ohne Meldung, aehnlich wie
         giveallspells.  */
      for (i = 0; i != MAXSPELLS; i++)
        if (u->spells[i] && !u2->spells[i] && canlearn (u2, i))
          u2->spells[i] = 1;

      addevent (u->faction, 
		translate (ST_GIVE_MEN, u->faction->language,
			   unitid (u), n, unitid2 (u2)));
      if (u->faction != u2->faction)
	addevent (u2->faction, 
		  translate (ST_GIVE_MEN, u2->faction->language,
			     unitid (u), n, unitid2 (u2)));
    }
  else
    {
      if (getunitpeasants)
        {
          r->peasants += n;
          if (k)
	    addevent (u->faction, 
		      translate (ST_GIVE_MEN_TO_PEASANTS, 
				 u->faction->language, unitid (u)));
          else
	    addevent (u->faction, 
		      translate (ST_GIVE_ALL_MEN_TO_PEASANTS, 
				 u->faction->language, unitid (u)));
        }
      else
        {
          if (k)
	    addevent (u->faction, 
		      translate (ST_GIVE_MEN_TO_NOTHING, 
				 u->faction->language, unitid (u), n));
          else
	    addevent (u->faction, 
		      translate (ST_GIVE_ALL_MEN_TO_NOTHING, 
				 u->faction->language, unitid (u), n));
        }
    }
}

void
givesilver (int n, unit * u, region * r, unit * u2, strlist * S)
{
  if (n > u->money)
    n = u->money;

  if (n == 0)
    {
      mistake2 (u, S, strings[ST_NO_MONEY][u->faction->language]);
      return;
    }

  u->money -= n;

  if (u2)
    {
      u2->money += n;

      sprintf (buf, "%s zahlte $%d an ", unitid (u), n);
      scat (unitid (u2));
      scat (".");
      if (u->faction != u2->faction)
	{
	  addcommerce (u->faction, buf);
	  addcommerce (u2->faction, buf);
	}
      else
	addevent (u->faction, buf);
    }
  else
    {
      if (getunitpeasants)
        {
          r->money += n;

          sprintf (buf, "%s verteilt $%d unter den Bauern.", unitid (u), n);
        }
      else
        sprintf (buf, "%s wirft $%d weg.", unitid (u), n);
      addcommerce (u->faction, buf);
    }
}

void
giveall (unit * u, region * r, unit * u2, strlist * S)
{
  int i;
  if (u->number)
    givemen (u->number, u, r, u2, S);
  if (u->money)
    givesilver (u->money, u, r, u2, S);
  for (i = 0; i != MAXITEMS; i++)
    if (u->items[i])
      giveitem (u->items[i], i, u, r, u2, S);
}

void
giveunit (unit * u, region * r, unit * u2, strlist * S)
{
  if (!u2)
    {
      giveall (u, r, u2, S);
      return;
    }
      
  if (u->faction == u2->faction)
    return;

  if (u->skills[SK_MAGIC]
      && magicians (u2->faction) + u->number > MAXMAGICIANS)
    {
      sprintf (buf, "Es kann maximal %d Magier pro Partei geben", MAXMAGICIANS);
      mistake2 (u, S, buf);
      return;
    }

  sprintf (buf, "%s wechselt zu %s",
           unitid (u), factionid (u2->faction));
  addevent (u->faction, buf);
  addevent (u2->faction, buf);
  u->faction = u2->faction;
}

/* ------------------------------------------------------------- */

/* Diese Funktion wird auch verwendet, um das fuer den Lebensunterhalt
   noetige Geld (meist $10) einzusammeln.  Dies ist der side-effect.
   Der return Wert wird in der naechsten Funktion collectsomemoney
   gebraucht, um den Text der Message zu machen.  */
int
collectmoney (region * r, unit * collector, int n)
{
 int i, sum=0;
  unit *u;

  /* n gibt an, wieviel Silber noch eingesammelt werden soll.  */

  for (u = r->units; u && n >= 0; u = u->next)
    if (u->faction == collector->faction &&
        u != collector &&
        u->type == U_MAN &&    /* nur menschen geben geld her */
        can_contact (r, collector, u))
      {
        i = min (u->money, n);
	/* u->money wird immer 0, ausser bei der letzten Einheit,
           welche nicht mehr alles hergeben muss.  */
        u->money -= i;
        sum += i;
        n -= i;
      }
  collector->money += sum;
  return sum;
}

void
collectsomemoney (region * r, unit * collector, int n)
{
  sprintf (buf, "%s sammelte $%d ein.", unitid (collector), collectmoney (r, collector, n));
  addevent (collector->faction, buf);
}

void
collectallmoney (region * r, unit *collector)
{
  int sum=0;
  unit *u;

  for (u = r->units; u; u = u->next)
    if (u->faction == collector->faction && 
	u != collector &&
	u->type == U_MAN &&    /* nur menschen geben geld her */
	can_contact (r, collector, u))
      {
	sum += u->money;
	u->money = 0;
      }
  collector->money += sum;
  sprintf (buf, "%s sammelte $%d ein.", unitid (collector), sum);
  addevent (collector->faction, buf);
}

/* ------------------------------------------------------------- */

void
giving (void)
{
  region *r;
  unit *u, *u2;
  strlist *S;
  char *s;
  int i, n;

  puts ("- geben...");

  /* GIB vor STIRB!  */
  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      for (S = u->orders; S; S = S->next)
	switch (igetkeyword (S->s))
	  {
	  case K_COLLECT:
	    
	    s = getstr ();
	    if (findparam (s) == P_ALL)
	      {
		collectallmoney (r, u);
		continue;
	      }
	    
	    n = atoip (s);
	    if (n)
	      {
		collectsomemoney (r, u, n);
		continue;
	      }
	    
	    mistake2 (u, S, "Die Silbermenge wurde nicht angegeben");
	    break;
	    
	  case K_DELIVER:
	  case K_GIVE:
	    
	    /* Drachen, Illusionen etc geben nichts her */
	    if (u->type != U_MAN)
	      {
		mistake2 (u, S, translate (ST_MONSTERS_DONT_GIVE, u->faction->language,
					   strings[typenames[1][u->type]][u->faction->language]));
		continue;
	      }

	    u2 = getunit (r, u);
	    if (u==u2)
	      {
		mistake2 (u, S, "Das ist die selbe Einheit");
		continue;
	      }

	    /* Ziel auffinden */
	    if (!u2 && !getunit0 && !getunitpeasants)
	      {
		mistake2 (u, S, "Die Einheit wurde nicht gefunden");
		continue;
	      }

	    if (u2 && !contacts (r, u2, u))
	      {
		mistake2 (u, S, "Die Einheit hat keinen Kontakt mit uns aufgenommen");
		continue;
	      }

	    /* Drachen, Illusionen etc nehmen auch nichts an (va. keine neuen Leute!).  */
	    if (u2 && u2->type != U_MAN)
	      {
		mistake2 (u, S, translate (ST_MONSTERS_DONT_TAKE, u->faction->language,
					   strings[typenames[1][u2->type]][u->faction->language]));
		continue;
	      }

	    s = getstr ();
	    switch (findparam (s))
	      {
	      case P_CONTROL:
		/* KONTROLLE in enter ()! */
		continue;

	      case P_ALL:
		/* ALLES: Mit einer anderen Einheit mischen.  */
		giveall (u, r, u2, S);
		continue;
		  
	      case P_SPELLBOOK:
		/* ZAUBERBUCH: uebergibt alle Sprueche.  */
		giveallspells (u, u2, S);
		continue;
      
	      case P_UNIT:
		/* EINHEIT: Einheiten als ganzes an eine andere Partei uebergeben.  */
		giveunit (u, r, u2, S);
		continue;
	      }

	    /* Falls nicht, dann steht in s vielleicht ein Zauberspruch */
	    i = findspell (s);
	    if (i >= 0)
	      {
		givespell (i, u, u2, S);
		continue;
	      }

	    /* Falls nicht, dann steht darin eine Anzahl */
	    n = atoip (s);
	    /* und nun folgt ein Gegenstand... */
	    s = getstr ();

	    switch (findparam (s))
	      {
		/* PERSONEN */
	      case P_MAN:
	      case P_PERSON:
		givemen (n, u, r, u2, S);
		continue;
		  
		/* SILBER */
	      case P_SILVER:
		givesilver (n, u, r, u2, S);
		continue;
	      }
	      
	    /* oder ein allgemeiner Gegenstand */
	    i = finditem (s);
	    if (i >= 0)
	      {
		giveitem (n, i, u, r, u2, S);
		continue;
	      }
	      
	    /* Wenn es bis hierhin nichts war, dann hat die Einheit soetwas nicht.  */
	    mistake2 (u, S, "So etwas hat die Einheit nicht");
	  }
}
