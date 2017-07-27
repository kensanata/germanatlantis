/* German Atlantis PB(E)M host Copyright (C) 1995-1998   Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed. It may not be
   sold or used commercially without prior written permission from the
   author.

 */

#include "atlantis.h"
 
int norders;
order *oa;

/* ------------------------------------------------------------- */

int
findshiptype (char *s)
{
  return findstr (shiptypes[0], s, MAXSHIPS);

}

/* ------------------------------------------------------------- */

int
scramblecmp (const void *p1, const void *p2)
{
  return *((long *) p1) - *((long *) p2);
}

void
scramble (void *v1, int n, int width)
{
  int i;
  void *v;

  v = cmalloc (n * (width + 4));

  for (i = 0; i != n; i++)
    {
      *(long *) addptr (v, i * (width + 4)) = rand ();
      memcpy (addptr (v, i * (width + 4) + 4), addptr (v1, i * width), width);
    }

  qsort (v, n, width + 4, scramblecmp);

  for (i = 0; i != n; i++)
    memcpy (addptr (v1, i * width), addptr (v, i * (width + 4) + 4), width);

  free (v);
}

void
expandorders (region * r, order * orders)
{
  int i, j;
  unit *u;
  order *o;

  /* Alle Units ohne order haben ein -1, alle units mit orders haben ein 0
     hier stehen */

  for (u = r->units; u; u = u->next)
    u->n = -1;

  norders = 0;

  for (o = orders; o; o = o->next)
    norders += o->qty;

  oa = cmalloc (norders * sizeof (order));

  i = 0;

  for (o = orders; o; o = o->next)
    for (j = o->qty; j; j--)
      {
        oa[i].unit = o->unit;
        oa[i].type = o->type;
        oa[i].unit->n = 0;
        i++;
      }

  freelist (orders);

  scramble (oa, norders, sizeof (order));
}

/* ------------------------------------------------------------- */

void
expandrecruit (region * r, order * recruitorders)
{
  /* Rekrutierung */

  int i, n;
  unit *u;

  expandorders (r, recruitorders);

  for (i = 0, n = r->peasants / RECRUITFRACTION; i != norders && n; i++, n--)
    {
      oa[i].unit->number++;
      r->peasants--;
      oa[i].unit->money -= RECRUITCOST;
      r->money += RECRUITCOST;
      oa[i].unit->n++;
    }

  free (oa);

  for (u = r->units; u; u = u->next)
    if (u->n >= 0)
      {
        sprintf (buf, "%s rekrutiert %d.", unitid (u), u->n);
        addevent (u->faction, buf);

        if (u->n < u->wants_recruits)
          {
            sprintf (buf, "%s in %s rekrutierte %d statt %d.",
                     unitid (u), regionid (r), u->n, u->wants_recruits);
            addwarning (u->faction, buf);
          }
	assert (u->n <= u->wants_recruits);
      }
}

void
recruit (unit * u, strlist * S,
         order ** recruitorders, int *availmoney)
{
  int new_people, new_mages;
  order *o, *o2;

  if (*availmoney < RECRUITCOST)
    {
      sprintf (buf, "Die Einheit braucht mindestens $%d, um Bauern zu "
               "rekrutieren", RECRUITCOST);
      mistake2 (u, S, buf);
      return;
    }

  new_people = geti ();

  if (u->type != U_MAN)
    {
      mistake2 (u, S, translate (ST_MONSTERS_REFUSE_PEASANTS, u->faction->language,
				 strings[typenames[1][u->type]][u->faction->language]));
      return;
    }

  /* Da die Rekrutierungsbefehle erst am Schluss ausgefuehrt werden, ist es
     moeglich, dass 1 Magier zuerst 2 Personen rekrutiert (macht total 3
     Magier und ist legal), und dann noch einmal 2 Personen rekrutiert (macht
     total 5 Magier und ist illegal). Das Problem ist, dass beim zweiten
     Befehl nicht klar ist, dass gleichzeitig schon 2 Magier rekrutiert
     wurden. Deswegen muss gezaehlt werden, wieviele Rekrutierungsbefehle die
     Magiereinheit schon abgesetzt hat, um zu entscheiden, ob weiter
     rekrutiert werden darf. */

  if (u->skills[SK_MAGIC])
    {

      /* new_mages zaehlt die Magier, die diese Runde auch schon von anderen
         Einheiten o2->unit rekrutiert werden, inkl. der Magier, die von der
         aktuellen Einheit u rekrutiert werden. In o2 sind alle
         Rekrutierungsbefehle drinnen. o2->qty sind die neuen Magier, die
         o2->unit rekrutiert. Diese werden Magier werden, wenn
         o2->unit->skills[SK_MAGIC] hat. */

      new_mages = new_people;
      for (o2 = *recruitorders; o2; o2 = o2->next)
        if (o2->unit->faction == u->faction
            && o2->unit->skills[SK_MAGIC])
          new_mages += o2->qty;

      if (magicians (u->faction) + new_mages > MAXMAGICIANS)
        {
          sprintf (buf, "Es kann maximal %d Magier pro Partei geben", MAXMAGICIANS);
          mistake2 (u, S, buf);
          return;
        }
    }

  u->wants_recruits += new_people;        /* In u->wants wird das Gewuenschte angegeben */

  new_people = min (new_people, *availmoney / RECRUITCOST);

  if (!new_people)
    {
      sprintf (buf, "Der Einheit fehlen $%d um eine Person zu rekrutieren",
               RECRUITCOST);
      mistake2 (u, S, buf);
      return;
    }

  o = cmalloc (sizeof (order));
  o->qty = new_people;
  o->unit = u;
  addlist (&recruitorders, o);

  *availmoney -= o->qty * RECRUITCOST;
  return;
}

void
recruiting (void)
{
  region *r;
  order *recruitorders;
  unit *u;
  strlist *S;
  int availmoney;

  puts ("- rekrutieren...");

  /* REKRUTIERE.  Rekrutieren vor allen Einnahmequellen. */
  for (r = regions; r; r = r->next)
    {
      recruitorders = 0;
      for (u = r->units; u; u = u->next)
        {
          availmoney = u->money;
	  u->wants_recruits = 0;
          for (S = u->orders; S; S = S->next)
            if (igetkeyword (S->s) == K_RECRUIT)
              recruit (u, S, &recruitorders, &availmoney);
        }
      expandrecruit (r, recruitorders);
    }
}

/* ------------------------------------------------------------- */

void
manufacture (region *r, unit * u, int i, int m, order * produceorders[LASTRESSOURCE])
{
  /* u ist die produzierende Einheit, i der Index des Gegenstandes, m
     die verlangte Anzahl Gegenstaende (falls 0, sollen soviele wie
     moeglich produziert werden), produceorders ist die Liste der
     Produktionsbefehle, in welcher die Befehle abgespeichert werden.
     r ist die Region, in der produziert wird; sie wird nur bei einer
     Warnung gebraucht.  */
  int n;
  order *o;

  n = effskill (u, itemskill[i]);

  if (n == 0)
    {
      sprintf (buf, "Keiner hier hat das Talent %s",
               skillnames[itemskill[i]]);
      mistakeu (u, buf);
      return;
    }

  if (n < itemquality[i])
    {
      mistakeu (u, translate (ST_SKILL_NEEDED_FOR_ITEM, u->faction->language,
			      strings[itemnames[1][i]][u->faction->language], 
			      skillnames[itemskill[i]], itemquality[i]));
      return;
    }

  if (isressource (i) && besieged (u))
    {
      mistakeu (u, translate (ST_IS_BESIEGED, u->faction->language, buildingid (u->building)));
      return;
    }

  /* n /= itemquality[i]; zB. Bogenbau 3 macht nur 1 Bogen! */

  n *= u->number;

  /* In u->wants wird das Gewuenschte angegeben */
  u->wants = n;

  /* Limitierung durch Parameter m.  */
  if (m)
    {
      n = min (n, m);
      u->wants = m;
    }

  if (isressource (i))
    {
      o = cmalloc (sizeof (order));
      o->unit = u;
      o->qty = n;
      addlist (&produceorders[i], o);

      /* ressourcen muessen vielleicht aufgeteilt werden, da es ein limite
         dafuer gibt */

    }
  else
    {

      n = min (n, u->items[rawmaterial[i]] / rawquantity[i]);

      if (n == 0)
        {
          mistakeu (u, translate (ST_THIS_REQUIRES_ITEMS, u->faction->language, rawquantity[i],
				  strings[itemnames[(rawquantity[i] > 1)][rawmaterial[i]]][u->faction->language]));
          return;
        }

      u->items[i] += n;
      u->items[rawmaterial[i]] -= n * rawquantity[i];

      addproduction (u->faction, translate (ST_PRODUCES, u->faction->language, unitid (u), n, 
					    strings[itemnames[(n != 1)][i]][u->faction->language]));

      if (n < u->wants)
	addwarning (u->faction, translate (ST_PRODUCES_LESS, u->faction->language,
					   unitid (u), regionid (r), n, u->wants,
					   strings[itemnames[(u->wants != 1)][i]][u->faction->language]));
      assert (u->n <= u->wants);
    }

  /* Nur soviel PRODUCEEXP wie auch tatsaechlich gemacht wurde */

  u->skills[itemskill[i]] += min (n, u->number) * PRODUCEEXP;
}

void
make (region * r, unit * u, order * produceorders[LASTRESSOURCE])
{
  char *s;
  int i, m;

  s = getstr ();

  i = findparam (s);

  /* MACHE TEMP kann hier schon gar nicht auftauchen, weil diese nicht in
     thisorder abgespeichert werden - und auf den ist getstr() beim aufruf
     von make geeicht */

  if (i == P_BUILDING)
    {
      build_building (r, u);
      return;
    }

  if (i == P_ROAD)
    {
      build_road (r, u);
      return;
    }

  if (i == P_SHIP)
    {
      continue_ship (r, u);
      return;
    }

  i = findshiptype (s);

  if (i >= 0)
    {
      create_ship (r, u, i);
      return;
    }

  i = finditem (s);

  if (isproduct (i))
    {
      /* qty 0 bedeuted so viele Gueter wie moeglich.  */
      manufacture (r, u, i, 0, produceorders);
      return;
    }

  /* Letzter Versuch mit den Parametern: falls die Anzahl zu
     produzierender Gueter angegeben wurde, bezeichnet der naechste
     Parameter einen Gegenstand.  Das jetztige i ist also kein
     Gegenstand sondern eine Anzahl.  */

  m = atoip (s);
  if (m)
    {
      i = getitem ();

      if (isproduct (i))
	{
	  manufacture (r, u, i, m, produceorders);
	  return;
	}
    }

  /* Falls ohne parameter angegeben, kann man versuchen, beim aktuellen
     schiff bzw. der aktuellen burg weiterzubauen (ist wohl meistens
     unnoetig, da der befehl einmal korrekt eingegeben, als default bleibt,
     aber trotzdem...) */

  if (!s[0] && u->building)
    {
      build_building (r, u);
      return;
    }

  if (!s[0] && u->ship)
    {
      continue_ship (r, u);
      return;
    }

  mistakeu (u, "So etwas kann man nicht machen");
}

/* ------------------------------------------------------------- */

void
expandbuying (region * r, order * buyorders)
{
  /* i ist das gekaufte Gut.  j zaehlt die Befehle.  count[] zaehlt,
     wieviele Gueter eines Types schon gekauft worden sind.  money ist
     eine Kopie des alten r->money Wertes und wird fuer die debug
     messages gebraucht.  p ist der Preis der gekauften Produktes.
     dh, m, n wird fuer das Formatieren von messages gebraucht.  */
  int i, j, money, p, count[MAXLUXURIES], m, n, dh;
  unit *u;

  /* Kaufen kann man nur bei einem Stuetzpunkt.  */
  if (!r->buildings)
    return;

  /* Initialisation.  */
  memset (count, 0, MAXLUXURIES * sizeof (int));
  money = r->money;

  expandorders (r, buyorders);
  if (r->peasants) /* Kauf nur moeglich, falls es Bauern gibt.  */
    for (j = 0; j != norders; j++)
      {
	/* Die units welche orders zum kaufen haben, bekommen u->n >= 0.  */
	if (oa[j].unit->n < 0)
	  oa[j].unit->n = 0;

	i = oa[j].type; 

	/* Folgende Dokumentation stammt aus einem Kommentar.  Im code
	   wurde es genauso programmiert, mit der Ausnahme, dass die
	   Nachfrage mit 100 multiplizert sind, und der Faktor n
	   jedesmal neu berechnet wird.

	   Die Nachfrage ist der Quotien Verkaufspreis/Basispreis.
	   Beispiel: Der Basispreis fuer Juwelen ist $7.  Wenn in einer
	   Region die Juwelen fuer $77 verkauft wurden, dann ist die
	   Nachfrage 77 / 7 = 11.00.

	   Der Faktor n sei 25 geteilt durch die Anzahl Bauern.  Die
	   Nachfrage sinkt um n, sobald ein Luxusgut verkauft wird, sie
	   steigt um n sobald ein Luxusgut gekauft wird.  Der aktuelle
	   Preis ist der Basispreis mal diese korrigierte Nachfrage.

	   In Formeln ausgedrueckt ist der Preis eines Luxusgutes,
	   nachdem i Luxusgueter desselben Typus gehandelt worden sind,
	   der folgende:

	   Verkaufspreis = Basispreis * (Nachfrage - i * n)
	   Kaufspreis = Basispreis * (Nachfrage + i * n)

	   Am Ende der Runde wird die Nachfrage in allen Regionen fuer
	   alle nicht-produzierten Handelsgueter um 0.25 erhoeht.  Die
	   Nachfrage fuer das produzierte Gut in der Region sinkt um
	   3.00.

	   Es folgen zwei Beispiele.  Eines mit 2000 Bauern und eine
	   kleine Umrechnung auf die Situation mit 10000 Bauern.  In
	   beiden Faellen werden Juwelen verkauft, nach denen am Anfang
	   der Runde eine Nachfrage von 11.00 besteht.

	   Bei 2000 Bauern ist der n Faktor = 25 / 2000 = 0.0125

	   In der Tabelle wird aufgelistet, wie hoch der Verkaufspreis
	   VP des Gutes Nummer # ist, nachdem schon i Luxusgueter
	   desselben Typus verkauft worden sind.  Zuletzt wird
	   aufgefuehrt, auf welchen Wert die Nachfrage N gesunken waere,
	   keine weiteren Luxusgueter dieses Types verkauft wuerden.

	   #         i 	VP	   N
	   1         0 	77	10.9875
	   2         1 	77	10.975 
	   3         2 	77	10.9625
	      etc...
	   20        19	75	 10.75 

	   Am Ende der Runde wird die Nachfrage wieder um 0.25 steigen
	   und 11.00 erreichen.  In diesem Fall koennen also 20 Gueter
	   pro Runde verkauft werden.  Der Erloes betraegt $1524.

	   Bei 10000 Bauern ist der n Faktor = 25 / 10000 = 0.0025.
	   Dieser n Faktor ist genau 5x kleiner als der n Faktor bei
	   2000 Bauern.  Deshalb kann man genau 5x mehr Gueter verkaufen
	   als beim Beispiel mit 2000 Bauern, wenn die Nachfrage nur um
	   0.25 sinken darf.  Durch den Verkauf von 100 Juwelen laesst
	   sich somit $7620 pro Runde verdienen.  */

	p = itemprice[i] * (r->demand[i] + count[i] * DEMANDFACTOR / r->peasants) / 100;
	/* Der Kauf wird nur weiter verfolgt, wenn die Einheit auch das
	   noetige Geld auf sich traegt.  */
	if (oa[j].unit->money >= p)
	  {
	    /* litems zaehlt die Gueter die gekauft wurden, u->n das
	       ausgegebene Geld.  Spaehter muss litems summiert werden
	       und mit u->wants2 verglichen werden (die gewuenschte
	       Anzahl umgesetzter Gueter).  Man muss sich die Anzahl
	       gehandelter Gueter und das ausgegebene Geld separat
	       merken, weil der Preis sich staendig aendert.  */
	    if (!oa[j].unit->litems)
	      {
		oa[j].unit->litems = cmalloc (MAXLUXURIES * sizeof (int));
		memset (oa[j].unit->litems, 0, MAXLUXURIES * sizeof (int));
	      }
	    oa[j].unit->items[i + FIRSTLUXURY]++;
	    oa[j].unit->litems[i]++;
	    oa[j].unit->money -= p;
	    oa[j].unit->n += p;
	    r->money += p;

	    /* Der Zaehler bestimmt, dass das naechste Mal der Preis
	       fuer das Gut hoeher liegt.  */
	    count[i]++;
	  }
      }
  free (oa);

  /* debug messages und Anpassung von r->demand[].  */
  sprintf (buf, "Es wurden Luxusgueter im Wert von $%d gekauft", r->money - money);
  dh = 0;
  for (i = 0; i != MAXLUXURIES; i++)
    if (count[i] > 1)
      {
	if (!dh)
	  scat (": ");
	else
	  scat (", ");
	dh = 1;
	icat (count[i]);
	scat (" ");
	scat (strings[itemnames[1][i + FIRSTLUXURY]][0]);
      }
  scat (". Nachfrage vorher: ");
  dh = 0;
  for (i = 0; i != MAXLUXURIES; i++)
    {
	if (dh)
	  scat (", ");
	dh = 1;
	scat (strings[itemnames[1][i + FIRSTLUXURY]][0]);
	scat (" ");
	icat (r->demand[i]);
      }
  scat (". Nachfrage jetzt: ");
  dh = 0;
  if (r->peasants) /* Verkauf nur moeglich, falls es Bauern gibt.  */
    for (i = 0; i != MAXLUXURIES; i++)
      {
	/* Wichtig:  Hier wird die Nachfrage effektiv erhoeht.  */
	r->demand[i] = r->demand[i] + count[i] * DEMANDFACTOR / r->peasants;
	if (dh)
	  scat (", ");
	dh = 1;
	scat (strings[itemnames[1][i + FIRSTLUXURY]][0]);
	scat (" ");
	icat (r->demand[i]);
      }
  else
    scat ("gleich, da keine Bauern hier leben.");
  scat (".");
  adddebug (r, buf);

  /* Ausgabe an Einheiten.  */
  for (u = r->units; u; u = u->next)
    {
      /* m zaehlt, wieviele Arten von Produkten gekauft wurden, n
         zaehlt die genaue Anzahl, und u->n zaehlt, wieviel Geld
         ausgegeben wurde.  */
      m = 0;
      n = 0;

      /* Ausgabe wenn Gueter gekauft werden konnten, dh.  u->n > 0!
         Fehler werden danach behandelt.  */
      if (u->n > 0)
        {
	  /* buf enthaelt die Liste gehandelter Gueter.  */
          buf[0] = 0;

          for (j = 0; j != MAXLUXURIES; j++)
            if (u->litems[j])
              {
                m++;
                n += u->litems[j];
              }

          /* m muss > 0 sein, sonst gab es auch kein u->n!  */
          assert (m);

          /* i zaehlt, welches der m Produkte gerade gekauft wird, um
             die Aufzaehlung richtig zu formatieren.  */
          i = 0;
          for (j = 0; j != MAXLUXURIES; j++)
	    if (u->litems[j])
	      {
		i++;
		if (i == 1)
		  scat (translate (ST_FIRST_QUANTITY_IN_LIST, u->faction->language,
				   u->litems[j], 
				   strings[itemnames[u->litems[j] != 1][j + FIRSTLUXURY]][u->faction->language]));
		else if (i > 1 && i < m)
		  scat (translate (ST_QUANTITY_IN_LIST, u->faction->language,
				   u->litems[j], 
				   strings[itemnames[u->litems[j] != 1][j + FIRSTLUXURY]][u->faction->language]));
		else if (i > 1 && i == m)
		  scat (translate (ST_LAST_QUANTITY_IN_LIST, u->faction->language,
				   u->litems[j], 
				   strings[itemnames[u->litems[j] != 1][j + FIRSTLUXURY]][u->faction->language]));
	      }
          addcommerce (u->faction, translate (ST_BUYS, u->faction->language, unitid (u), u->n, buf));

          free (u->litems);
          u->litems = 0;
        }

      /* Warnungen - hier wird auch festgestellt, wenn weniger Gueter
	 als geplant gekauft wurden.  In der Warnung wird nicht mehr
	 nach einzelnen Produkten aufgeschluesselt.  Achtung: Es wird
	 keine commerce Meldung ausgegeben, wenn 0 Produkte gekauft
	 wurden.  u->n == -1 fuer alle units, die keinen order
	 plaziert haben.  */
      if (u->n >= 0 && n < u->wants2)
        {
          sprintf (buf, "%s in %s kauft %d %s statt %d.", 
		   unitid (u), regionid (r), n, n != 1 ? "Luxusgueter" : "Luxusgut", u->wants2);
          addwarning (u->faction, buf);
        }
    }
}

/* ------------------------------------------------------------- */

void
expandselling (region *r, order *sellorders)
{
  /* i ist das verkaufte Gut.  j zaehlt die Befehle.  count[] zaehlt,
     wieviele Gueter eines Types schon verkauft worden sind.  money
     ist die Menge des Geldes, welches die Bauern einsetzen, um den
     Spielern die Gueter abzukaufen.  old_money wird fuer die debug
     messages gebraucht.  p ist der Preis der verkauften Produktes.
     dh, m, n wird fuer das Formatieren von messages gebraucht.  */
  int i, j, money, old_money, p, count[MAXLUXURIES], dh, n, m;
  unit *u;

  /* Verkaufen kann man nur bei einem Stuetzpunkt.  */
  if (!r->buildings)
    return;

  /* Initialisation: Die Region muss genug Geld haben, um die Produkte
     kaufen zu koennen.  Abgezogen wird das Geld jedoch nicht, denn
     die Produkte koennten ja als Steuern durchaus eingetrieben
     werden.  Diese Abstraktion soll foerdern, dass die
     Steuereintreiber die Haendler Handel treiben lassen.  Die
     Haendler koennen nun zusaetzlich zu den Steuern Abgaben an die
     Steuereintreiber zahlen.  Die Bauern versuchen genug Geld zu
     sparen, um nicht zu verhungern!  */
  memset (count, 0, MAXLUXURIES * sizeof (int));
  money = r->money - r->peasants * MAINTENANCE;
  old_money = money = max (money, 0);

  expandorders (r, sellorders);
  if (r->peasants) /* Verkauf nur moeglich, falls es Bauern gibt.  */
    for (j = 0; j != norders; j++)
      {
	/* Die units welche orders zum verkaufen haben, bekommen u->n >= 0.  */
	if (oa[j].unit->n < 0)
	  oa[j].unit->n = 0;
      
      /* Der Preis berechnet sich wie in buyorders () beschrieben, nur
         dass der Preis hier mit der Stueckzahl sinkt.  Der Preis
         sinkt nie unter den Basispreis.  */
	i = oa[j].type; 
	p = itemprice[i] * (r->demand[i] - count[i] * DEMANDFACTOR / r->peasants) / 100;
	p = max (itemprice[i], p);
	/* Der Verkauf wird nur weiter verfolgt, wenn die Bauern auch
         das noetige Geld haben.  */
	if (money >= p && oa[j].unit->items[i + FIRSTLUXURY])
	  {
	    /* litems zaehlt die Gueter die verkauft wurden, u->n das
	       verdiente Geld.  Spaehter muss litems summiert werden und
	       mit u->wants verglichen werden (die gewuenschte Anzahl
	       umgesetzter Gueter).  Man muss sich die Anzahl
	       gehandelter Gueter und das verdiente Geld separat merken,
	       weil der Preis sich staendig aendert.  */
	    if (!oa[j].unit->litems)
	      {
		oa[j].unit->litems = cmalloc (MAXLUXURIES * sizeof (int));
		memset (oa[j].unit->litems, 0, MAXLUXURIES * sizeof (int));
	      }
	    oa[j].unit->items[i + FIRSTLUXURY]--;
	    oa[j].unit->litems[i]++;
	    oa[j].unit->money += p;
	    oa[j].unit->n += p;
	    money -= p;

          /* Der Zaehler bestimmt, dass das naechste Mal der Preis
             fuer das Gut tiefer liegt.  */
	    count[i]++;
	  }
      }
  free (oa);

  /* debug messages und Anpassung von r->demand[].  */
  sprintf (buf, "Die Bauern konnten fuer $%d Gueter kaufen. "
	   "Es wurden Luxusgueter im Wert von $%d an sie verkauft", 
	   old_money,  old_money - money);
  dh = 0;
  for (i = 0; i != MAXLUXURIES; i++)
    if (count[i] > 1)
      {
	if (!dh)
	  scat (": ");
	else
	  scat (", ");
	dh = 1;
	icat (count[i]);
	scat (" ");
	scat (strings[itemnames[1][i + FIRSTLUXURY]][0]);
      }
  scat (". Nachfrage vorher: ");
  dh = 0;
  for (i = 0; i != MAXLUXURIES; i++)
    {
	if (dh)
	  scat (", ");
	dh = 1;
	scat (strings[itemnames[1][i + FIRSTLUXURY]][0]);
	scat (" ");
	icat (r->demand[i]);
      }
  scat (". Nachfrage jetzt: ");
  dh = 0;
  if (r->peasants) /* Verkauf nur moeglich, falls es Bauern gibt.  */
    for (i = 0; i != MAXLUXURIES; i++)
      {
	/* Wichtig:  Hier wird die Nachfrage effektiv reduziert!  */
	r->demand[i] = (r->demand[i] - count[i] * DEMANDFACTOR / r->peasants);
	if (dh)
	  scat (", ");
	dh = 1;
	scat (strings[itemnames[1][i + FIRSTLUXURY]][0]);
	scat (" ");
	icat (r->demand[i]);
      }
  else
    scat ("gleich, da keine Bauern hier leben.");
  scat (".");
  adddebug (r, buf);

  /* Ausgabe an Einheiten.  */
  for (u = r->units; u; u = u->next)
    {
      /* m zaehlt, wieviele Arten von Produkten verkauft wurden, n
         zaehlt die genaue Anzahl, und u->n zaehlt, wieviel Geld
         verdient wurde.  */
      m = 0;
      n = 0;

      /* Ausgabe wenn Gueter gekauft werden konnten, dh.  u->n > 0!
         Fehler werden danach behandelt.  */
      if (u->n > 0)
        {
	  /* buf enthaelt die Liste gehandelter Gueter.  */
	  buf[0] = 0;

          for (j = 0; j != MAXLUXURIES; j++)
            if (u->litems[j])
              {
                m++;
                n += u->litems[j];
              }

          /* m muss > 0 sein, sonst gab es auch kein u->n!  */
          assert (m);

          /* i zaehlt, welches der m Produkte gerade verkauft wird, um
             die Aufzaehlung richtig zu formatieren.  */
          i = 0;
          for (j = 0; j != MAXLUXURIES; j++)
	    if (u->litems[j])
	      {
		i++;
		if (i == 1)
		  scat (translate (ST_FIRST_QUANTITY_IN_LIST, u->faction->language,
				   u->litems[j], 
				   strings[itemnames[u->litems[j] != 1][j + FIRSTLUXURY]][u->faction->language]));
		else if (i > 1 && i < m)
		  scat (translate (ST_QUANTITY_IN_LIST, u->faction->language,
				   u->litems[j], 
				   strings[itemnames[u->litems[j] != 1][j + FIRSTLUXURY]][u->faction->language]));
		else if (i > 1 && i == m)
		  scat (translate (ST_LAST_QUANTITY_IN_LIST, u->faction->language,
				   u->litems[j], 
				   strings[itemnames[u->litems[j] != 1][j + FIRSTLUXURY]][u->faction->language]));
	      }
          addcommerce (u->faction, translate (ST_SELLS, u->faction->language, unitid (u), u->n, buf));

          free (u->litems);
          u->litems = 0;
        }

      /* Warnungen - hier wird auch festgestellt, wenn weniger Gueter
	 als geplant verkauft wurden.  In der Warnung wird nicht mehr
	 nach einzelnen Produkten aufgeschluesselt.  Achtung: Es wird
	 keine commerce Meldung ausgegeben, wenn 0 Produkte gekauft
	 wurden.  u->n == -1 fuer alle units, die keinen order
	 plaziert haben.  */
      if (u->n >= 0 && n < u->wants)
        {
          sprintf (buf, "%s in %s verkauft %d %s statt %d.", 
		   unitid (u), regionid (r), n, n != 1 ? "Luxusgueter" : "Luxusgut", u->wants);
          addwarning (u->faction, buf);
        }
    }
}

/* ------------------------------------------------------------- */

void
trade (region *r, unit *u, int type, order **buyorders, order **sellorders)
{
  /* n ist die gewuenschte Menge eines bestimmten Gutes i, welches
     gehandelt werden soll.  p ist der Preis eines bestimmten
     Luxusgutes in der Region.  m ist die maximale Anzahl Gueter,
     welche gehandelt werden kann, u->wants ist die Anzahl Gueter,
     welche verkauft werden sollte, u->wants2 die Anzahl, die gekauft
     werden sollte, und t ist die wirkliche Anzahl Gueter, fuer die
     wirklich gehandelt wird.  Ob am Ende tatsaechlich auch t Gueter
     gehandelt werden, wird in expandbuying () und expandselling ()
     bestimmt.  p ist der Preis eines Luxusgutes (fuer den Fall, dass
     die Einheit viel zu wenig Geld hat).  */
  int i, n, m, t=0, new_type;
  order *o;
  char *s;

  if (!r->peasants)
    {
      mistakeu (u, "Hier gibt es keine Bauern, mit denen man handeln koennte");
      return;
    }
  if (!r->buildings)
    {
      mistakeu (u, "Ohne eine Burg gibt es keinen Markt");
      return;
    }
  if (besieged (u))
    {
      sprintf (buf, "%s wird belagert", buildingid (u->building));
      mistakeu (u, buf);
      return;
    }
  /* Ein Haendler kann nur 10 Gueter pro Talentpunkt umsetzen.  */
  m = u->number * 10 * effskill (u, SK_TRADE);
  if (!m)
    {
      mistakeu (u, "Keiner hier kann handeln");
      return;
    }
  
  /* Moegliche Konstrukte:

     KAUFE JUWELEN
     KAUFE 10 JUWELEN
     KAUFE 10 JUWELEN UND 10 GEWUERZ
     KAUFE 10 JUWELEN 10 GEWUERZ
     KAUFE 5 JUWELEN GEWUERZ
     VERKAUFE JUWELEN UND GEWUERZ
     VERKAUFE 10 JUWELEN KAUFE 50 GEWUERZ
     VERKAUFE 10 JUWELEN UND KAUFE 50 GEWUERZ

     Ungewollt, aber auch moeglich:

     KAUFE VERKAUFE 10 JUWELEN
     
     Umoegliches:

     KAUFE JUWELEN UND GEWUERZ -- es werden zuviele Juwelen gekauft,
     so dass man kein Gewuerz mehr kriegt!

     */

  u->wants = 0;
  u->wants2 = 0;

  while (1)
    {
      /* Naechstes UND/KAUFE/VERKAUFE Kommando.  Die while Schlaufe
         wird wiederholt, wenn type == K_BUY oder K_SELL ist.  Wenn
         ein P_AND folgt, bleibt der type einfach unveraendert.  */
      s = getstr ();
      if (findparam (s) == P_AND)
	s = getstr ();
      new_type = findkeyword (s);
      if (new_type == K_BUY || new_type == K_SELL)
	{
	  type = new_type;
	  s = getstr ();
	}
      /* Vielleicht steht in s die Anzahl Gegenstaende.  */
      n = atoip (s);
      if (n)
	{
	  /* Falls s die Anzahl Gegenstaende war, folgt nun der
             Gegenstand.  */
	  i = getitem ();
	}
      else
	{
	  /* Falls s nicht die Anzahl war, dann ist wird die Anzahl
             auf das Maximum (m beim Kaufen, Anzahl Gegenstaende beim
             Verkaufen) gesetzt, und s enthaelt den Gegenstand.  */
	  i = finditem (s);
	  if (i >= 0)
	    switch (type)
	      {
	      case K_BUY:
		n = m;
		break;
	      case K_SELL:
		n = u->items[i];
		break;
	      default:
		assert (0);
		break;
	      }
	}
      /* Falls kein Gegenstand erkannt wurde, brechen wir hier ab.  Da
         wir keine Ahnung haben was hier folgen koennte, wird dieser
         Fehler nicht weiter behandelt...  */
      if (i < 0)
	{
	  /* Falls kein Wort mehr gefolgt ist, dann beenden wir den
             while loop.  */
	  if (s[0] == 0)
	    break;
	  /* Falls es ein Wort war, dann kennen wir es nicht...  */
	  sprintf (buf, "'%s' ist ein unbekanntes Luxusgut (Bearbeitung abgebrochen)", s);
	  mistakeu (u, buf);
	  return;
	}
      if (!isluxury (i))
	{
	  mistakeu (u, translate (ST_PEASANTS_DONT_TRADE_IN_IT, u->faction->language, 
				  strings[itemnames[1][i]][u->faction->language]));
	  continue;
	}
      switch (type)
	{
	case K_SELL:
	  u->wants += n;
	  break;
	case K_BUY:
	  u->wants2 += n;
	  break;
	}

      /* m - t ist die Anzahl Gueter welche noch gehandelt werden koennen.  */
      n = min (m - t, n);
      if (!n)
	{
	  sprintf (buf, "Die Einheit kann nicht mit mehr als %d Luxusgueter pro Monat handeln", m);
	  mistakeu (u, buf);
	  return;
	  /* Hier wird nur mit return abgebrochen, wenn alle weiteren Befehle sowieso keine Wirkung mehr haben
             koennen.  */
	}

      switch (type)
	{
	case K_SELL:
	  n = min (n, u->items[i]);
	  if (!n)
	    {
	      mistakeu (u, translate (ST_UNIT_HASNT_ANY, u->faction->language,
				      strings[itemnames[1][i]][u->faction->language]));
	      continue;
	      /* Dieser Fehler kann und wird gerettet.  */
	    }
	  break;
	  
	case K_BUY:
	  break;
	  
	default:
	  assert (0);
	  break;
	}

      o = cmalloc (sizeof (order));
      o->unit = u;
      o->qty = n;
      o->type = i - FIRSTLUXURY;

      switch (type)
	{
	case K_SELL:
	  addlist (&sellorders, o);
	  break;
	case K_BUY:
	  addlist (&buyorders, o);
	  break;
	}

      /* Effektiv gehandelte Menge.  */
      t += n;
    }
  
  /* Nur soviel PRODUCEEXP wie auch tatsaechlich gehandelt wird.  */
  u->skills[SK_TRADE] += min (t, u->number) * PRODUCEEXP;
}

/* ------------------------------------------------------------- */

void
expandstealing (region * r, order * stealorders)
{
  int i;
  faction *f;
  unit *u, *main_target=0;

  expandorders (r, stealorders);

  /* f->money wird missbraucht um das der Partei in dieser Region
     geklaute Geld zu zaehlen.  */
  for (f = factions; f; f = f->next)
    f->money = 0;

  /* i zaehlt durch alle zu klauenden Silbertaler durch.  Der Dieb
     oa[i].unit versucht nun bei allen Einheiten u, ob er dort seine
     Muenze klauen kann.  */
  for (i = 0; i != norders; i++)
    {
      /* oa[i].type (expanded stealorders) enthaelt die Einheit, die
	 beklaut werden soll.  Unter allen Einheiten u suchen wir also
	 die Zieleinheit.  */
      for (u = r->units; u; u = u->next)
	if (oa[i].type == u->no)
	  {
	    main_target = u;
	    break;
	  }
      assert (main_target);
      
      /* Nun wissen wir die Zieleinheit, aber es koennten auch noch
	 andere Einheiten beklaut werden.  Moegliche Opfer u gehoeren
	 zur selben Partei wie die Zieleinheit, dh. die beiden
	 Parteien sind gleich und bekannt.  Der Test (u ==
	 main_target) wurde nicht weggelassen, weil main_target's
	 Partei vielleicht unbekannt ist.  Der Diebstahl muss dann
	 wenigstens fuer main_target funktionieren!  */
      for (u = r->units; u; u = u->next)
	if (u->money
	    && ((u == main_target)
		|| (u->faction == main_target->faction
		    && can_contact (r, oa[i].unit, u)
		    && cansee (oa[i].unit->faction, r, u) == 2
		    && cansee (oa[i].unit->faction, r, main_target) == 2)))
	  {
	    /* Jeder Order ist nur $1 wert, deswegen muss nach dem
               Klau des $1 ein break eingefuegt werden.  */
	    oa[i].unit->money++;
	    oa[i].unit->n++;
	    u->money--;
	    u->faction->money++;
	    break;
	  }
    }
  free (oa);

  /* Nun die Erfolgs-Meldungen fuer die Taeter.  */
  for (u = r->units; u; u = u->next)
    if (u->n >= 0)
      {
        sprintf (buf, "%s klaute $%d.", unitid (u), u->n);
        addevent (u->faction, buf);

        if (u->n < u->wants)
          {
            sprintf (buf, "%s in %s klaute $%d statt $%d.",
                     unitid (u), regionid (r), u->n, u->wants);
            addwarning (u->faction, buf);
          }
	assert (u->n <= u->wants);
      }
  /* Und die Schadensmeldung fuer die Opfer.  */
  for (f = factions; f; f = f->next)
    if (f->money)
      {
	sprintf (buf, "In %s wurden $%d geklaut!", 
		 regionid (r), f->money);
	addwarning (f, buf);
      }
}

void
steal (region * r, unit * u, order ** stealorders)
{
  int n;
  order *o;
  unit *u2;

  u2 = getunit (r, u);

  if (!u2)
    {
      mistakeu (u, "Die Einheit wurde nicht gefunden");
      return;
    }

  if (!can_contact (r, u, u2))
    {
      mistakeu (u, "Der Belagerungszustand verunmoeglicht die "
                "Kontaktaufnahme");
      return;
    }

  /* Die beste Wahrnehmung der Partei des Opfers bestimmt, ob der Dieb
      ertappt wird oder nicht.  */

  switch (cansee (u2->faction, r, u))
    {
    case 1:

      sprintf (buf, "%s gelang es nicht, sich nahe genug an ",
               unitid (u));
      scat (unitid (u2));
      scat (" in ");
      scat (regionid (r));
      scat (" heran zu schleichen.");
      addwarning (u->faction, buf);

      sprintf (buf, "%s in %s fuehlt sich beobachtet.",
               unitid (u2), regionid (r));
      addwarning (u2->faction, buf);

      return;

    case 2:

      sprintf (buf, "%s wurde in %s beim versuchten Diebstahl ertappt!",
               unitid (u), regionid (r));
      addwarning (u->faction, buf);

      sprintf (buf, "%s ertappte ", unitid (u2));
      scat (unitid (u));
      scat (" in ");
      scat (regionid (r));
      scat (" beim versuchten Diebstahl!");

      addwarning (u2->faction, buf);

      return;

    }

  n = max (1, effskill (u, SK_STEALTH) - effskill (u2, SK_OBSERVATION));
  n *= u->number * STEALINCOME;

  u->wants = n;         /* In u->wants wird das Gewuenschte angegeben */

  /* Wer dank Unsichtbarkeitsringen klauen kann, muss nicht unbedingt ein
     guter Dieb sein, schliesslich macht man immer noch sehr viel Laerm */

  o = cmalloc (sizeof (order));
  o->unit = u;
  o->qty = n;
  o->type = u2->no;
  addlist (&stealorders, o);

  /* Nur soviel PRODUCEEXP wie auch tatsaechlich gemacht wurde */

  u->skills[SK_STEALTH] += min (n, u->number) * PRODUCEEXP;
}

void
stealing (void)
{
  region *r;
  order *stealorders;
  unit *u;

  puts ("- klauen...");

  for (r = regions; r; r = r->next)
    {
      if (r->terrain == T_OCEAN)
        continue;
      stealorders = 0;
      for (u = r->units; u; u = u->next)
        if (igetkeyword (u->thisorder) == K_STEAL)
	  steal (r, u, &stealorders);

      expandstealing (r, stealorders);
    }
}

/* ------------------------------------------------------------- */

void
expandentertainment (region * r, order * entertainorders)
{
  int i, n, m;
  unit *u;

  m = r->money;
  expandorders (r, entertainorders);
  for (i = 0, n = r->money / ENTERTAINFRACTION; i != norders && n; i++, n--)
    {
      oa[i].unit->money++;
      r->money--;
      oa[i].unit->n++;
    }
  free (oa);

  sprintf (buf, "Unterhaltung: $%d (max. $%d), neu: $%d.",
           m - r->money, m / ENTERTAINFRACTION, r->money);
  adddebug (r, buf);

  for (u = r->units; u; u = u->next)
    if (u->n >= 0)
      {
        sprintf (buf, "%s verdient $%d als Entertainer.", unitid (u), u->n);
        addincome (u->faction, buf);

        /* Nur soviel PRODUCEEXP wie auch tatsaechlich gemacht wurde */

        u->skills[SK_ENTERTAINMENT] += PRODUCEEXP
          * min (u->n, u->number);

        if (u->n < u->wants)
          {
            sprintf (buf, "%s in %s verdient $%d statt $%d als Entertainer.",
                     unitid (u), regionid (r), u->n, u->wants);
            addwarning (u->faction, buf);
          }
	assert (u->n <= u->wants);
      }
}

void
entertain (region *r, unit * u, order ** entertainorders)
{
  order *o;

  if (!r->peasants)
    {
      mistakeu (u, strings[ST_NO_PEASANTS][u->faction->language]);
      return;
    }
  if (besieged (u))
    {
      mistakeu (u, translate (ST_IS_BESIEGED, u->faction->language,
			      buildingid (u->building)));
      return;
    }

  /* In u->wants wird das Gewuenschte angegeben */
  u->wants = u->number * effskill (u, SK_ENTERTAINMENT) * ENTERTAININCOME;
  if (!u->wants)
    {
      mistakeu (u, strings[ST_NO_ENTERTAINERS][u->faction->language]);
      return;
    }

  o = cmalloc (sizeof (order));
  o->unit = u;
  o->qty = u->number * effskill (u, SK_ENTERTAINMENT) * ENTERTAININCOME;
  addlist (entertainorders, o);
}

/* ------------------------------------------------------------- */

void
expandwork (region * r, order * workorders, int wage)
{
  int i, n, m, earnings;
  unit *u;

  /* m: maximale einnahmen */

  m = max (production[r->terrain] - r->trees, 0) 
    * MAXPEASANTS_PER_AREA * wage;
  sprintf (buf, "Arbeitsplaetze: %d, Lohn: $%d, max. Einnahmen: $%d.",
           (production[r->terrain] - r->trees) * MAXPEASANTS_PER_AREA,
           wage, m);
  adddebug (r, buf);

  expandorders (r, workorders);
  for (i = 0, n = m; i != norders && n; i++, n--)
    {
      oa[i].unit->money++;
      oa[i].unit->n++;
    }
  free (oa);

  sprintf (buf, "Arbeit durch Einheiten: $%d, bleiben $%d.", i, n);
  adddebug (r, buf);

  for (u = r->units; u; u = u->next)
    if (u->n >= 0)
      {
        sprintf (buf, "%s verdient $%d mit arbeiten.", unitid (u), u->n);
        addincome (u->faction, buf);

        if (u->n < u->wants)
          {
            sprintf (buf, "%s in %s verdient $%d statt $%d mit arbeiten.",
                     unitid (u), regionid (r), u->n, u->wants);
            addwarning (u->faction, buf);
          }
	assert (u->n <= u->wants);
      }

  /* Der Rest wird von den Bauern verdient: n ist das uebrig gebliebene
     Geld. */

  earnings = min (n, r->peasants * wage);
  r->money += earnings;

  sprintf (buf, "Arbeit durch Bauern: $%d.", earnings);
  adddebug (r, buf);

}

void
work (unit * u, order ** workorders, int wage)
{
  order *o;

  if (besieged (u))
    {
      sprintf (buf, "%s wird belagert", buildingid (u->building));
      mistakeu (u, buf);
      return;
    }

  u->wants = u->number * wage;  /* In u->wants wird das Gewuenschte angegeben */

  o = cmalloc (sizeof (order));
  o->unit = u;
  o->qty = u->number * wage;
  addlist (workorders, o);
}

/* ------------------------------------------------------------- */

void
expandtax (region * r, order * taxorders)
{
  unit *u;
  int i, m;

  m = r->money;
  expandorders (r, taxorders);
  for (i = 0; i != norders && r->money > TAXFRACTION; i++, r->money -= TAXFRACTION)
    {
      oa[i].unit->money += TAXFRACTION;
      oa[i].unit->n += TAXFRACTION;
    }
  free (oa);

  sprintf (buf, "Silber: $%d, Steuern: $%d, neu: $%d.",
           m, m - r->money, r->money);
  adddebug (r, buf);

  for (u = r->units; u; u = u->next)
    if (u->n >= 0)
      {
        sprintf (buf, "%s treibt $%d an Steuern ein.", unitid (u), u->n);
        addincome (u->faction, buf);

        if (u->n < u->wants)
          {
            sprintf (buf, "%s in %s treibt $%d statt $%d an Steuern ein.",
                     unitid (u), regionid (r), u->n, u->wants);
            addwarning (u->faction, buf);
          }
	assert (u->n <= u->wants);
      }
}

void
tax (region * r, unit * u, order ** taxorders)
{
  /* Steuern werden noch vor der Forschung eingetrieben */

  unit *u2;
  int n, m;
  order *o;

  if (!r->peasants)
    {
      mistakeu (u, "Hier gibt es keine Bauern, die man besteuern koennte");
      return;
    }
  if (besieged (u))
    {
      sprintf (buf, "%s wird belagert", buildingid (u->building));
      mistakeu (u, buf);
      return;
    }
  for (u2 = r->units; u2; u2 = u2->next)
    if (u2->guard && u2->number && !contacts (r, u2, u) &&
        !besieged (u2))
      {
        sprintf (buf, "%s bewacht die Region", unitid (u2));
        mistakeu (u, buf);
        return;
      }
  n = armedmen (u);
  if (!n)
    {
      mistakeu (u, "Einheit ist nicht bewaffnet und kampffaehig");
      return;
    }

  /* Man kann ein Maximum (m) an gewuenschten Steuern angeben. Als
     Default gilt: Jede Person in der Einheit soll das maximale
     Steuergeld eintreiben. */
  m = geti ();
  m = max (m, u->number * income[u->type]);

  /* In u->wants wird das Gewuenschte angegeben. */
  u->wants = m;

  /* Tatsaechlich ist das Maximum aber abhaengig von der Anzahl
     bewaffneter Maenner (n). */
  m = min (m, n * income[u->type]);

  /* die einnahmen werden in fraktionen von 10 silber eingeteilt: diese
     fraktionen werden dann bei eintreiben unter allen eintreibenden
     einheiten aufgeteilt. */
  o = cmalloc (sizeof (order));
  o->qty = m / TAXFRACTION;
  o->unit = u;
  addlist (taxorders, o);
  return;
}

/* ------------------------------------------------------------- */

void
expandproduction (region * r, order * produceorders[LASTRESSOURCE])
{
  int i, j, n, ntrees=0;
  unit *u;

  for (i = 0; i != LASTRESSOURCE; i++)
    {
      expandorders (r, produceorders[i]);

      switch (i)
        {
        case I_IRON:
        case I_STONE:

          for (j = 0, n = mines[r->terrain]; j != norders && n; j++, n--)
            {
              oa[j].unit->items[i]++;
              oa[j].unit->n++;
            }
          break;

        case I_WOOD:

          for (j = 0, n = r->trees; j != norders && n; j++, n--)
            {
              oa[j].unit->items[i]++;
              oa[j].unit->n++;
              r->trees--;
              ntrees++;
            }
          break;

        case I_HORSE:

          for (j = 0, n = r->horses; j != norders && n; j++, n--)
            {
              oa[j].unit->items[i]++;
              oa[j].unit->n++;
              r->horses--;
            }
          break;
        }

      free (oa);

      for (u = r->units; u; u = u->next)
        if (u->n >= 0)
          {
            addproduction (u->faction, translate (ST_PRODUCES, u->faction->language, unitid (u), u->n, 
						  strings[itemnames[u->n != 1][i]][u->faction->language]));
	    if (u->n < u->wants)
	      addwarning (u->faction, translate (ST_PRODUCES_LESS, u->faction->language, 
						 unitid (u), regionid (r), u->n, u->wants,
						 strings[itemnames[u->n != 1][i]][u->faction->language]));
	    assert (u->n <= u->wants);
          }
    }
  if (ntrees)
    {
      sprintf (buf, "%d Baeume gefaellt.", ntrees);
      adddebug (r, buf);
    }
}

void
produce (void)
{
  region *r;
  order *entertainorders, *workorders, *taxorders, *sellorders, *buyorders;
  order *produceorders[LASTRESSOURCE];
  unit *u;
  building *b;
  int wage, i, regions_count;

  /* Das sind alles Befehle, die 30 Tage brauchen, und die in thisorder stehen! von allen 30-Tage Befehlen wird
     einfach der Letzte verwendet (siehe setdefaults).  Kaufen vor Einnahmequellen.  Lehren vor Lernen. */
  puts   ("- forschen, lehren, produzieren, unterhalten, verkaufen, arbeiten, kaufen, Steuern eintreiben...");
  printf ("    Regionen: ");
  regions_count = listlen (regions);
  indicator_reset (regions_count);

  for (r = regions; r; r = r->next)
    {
      indicator_count_down (--regions_count);

      assert (r->money >= 0);
      assert (r->peasants >= 0);

      if (r->terrain == T_OCEAN)
        continue;

      memset (&produceorders, 0, sizeof produceorders);
      buyorders = 0;
      sellorders = 0;
      entertainorders = 0;
      workorders = 0;
      taxorders = 0;

      /* gehalt zw 11 und 16, falls WAGE = 11, BONUS = 1, denn dann ist das
         gehalt = WAGE + BONUS * building->type (zw. 1 und 5) */
      wage = WAGE;
      b = largestbuilding (r);
      if (b)
        wage += buildingeffsize (b) * BONUS;

      /* Zuerst werden alle MACHE Befehle ausgefuehrt.  Hier werden
         die produceorders gefuellt, und alle Burgen (!) werden
         gemacht.  */
      for (u = r->units; u; u = u->next)
        if (igetkeyword (u->thisorder) == K_MAKE)
	  make (r, u, produceorders);

      /* Die Burgen muessen schon bestehen, bevor man die ARBEITE,
         KAUFE und VERKAUFE Befehle ausfuehrt.  */
      for (u = r->units; u; u = u->next)
        switch (i = igetkeyword (u->thisorder))
          {
          case K_ENTERTAIN:
            entertain (r, u, &entertainorders);
            break;

          case K_WORK:
            if (u->type == U_MAN || u->type == U_GUARDS)
              work (u, &workorders, wage);
            else
              mistakeu (u, "Nur Menschen koennen auf dem Feld "
                        "arbeiten");
            break;

          case K_RESEARCH:
            research (u);
            break;

          case K_TEACH:
            teach (r, u);
            break;

          case K_TAX:
            tax (r, u, &taxorders);
            break;

          case K_BUY:
          case K_SELL:
            trade (r, u, i, &buyorders, &sellorders);
            break;

          }

      /* debug messages */
      sprintf (buf, "Bauern: %d, Pferde: %d, Baeume: %d, Geld: $%d",
               r->peasants, r->horses, r->trees, r->money);
      for (i = 0; i != MAXLUXURIES; i++)
        if (r->demand[i])
          {
            scat (", ");
            scat (strings[itemnames[1][FIRSTLUXURY + i]][0]);
            scat (" $");
            icat (itemprice[i] * r->demand[i] / 100);
          }
      scat (".");
      adddebug (r, buf);

      /* Ausfuehren aller Befehle, bei denen knappe Ressourcen verteilt
         werden. */
      expandproduction (r, produceorders);

      /* Zuerst verdienen die Bauern Geld.  In einer Region mit Lohn $14 und 10000 Bauern verdienen sie hier
         $140000.  */
      expandwork (r, workorders, wage);

      /* Davon wird 1/20 fuer Unterhaltung ausgegeben.  Bauern: $133000, Spieler $7000.  */
      expandentertainment (r, entertainorders);

      /* Nun verkaufen die Spieler Waren. Auch hier kaufen die Bauern nur soviel, dass sie nicht verhungern
         muessen.  Das Geld wird ihnen aber nicht abgezogen: Bauern immer noch: $133000, Spieler +33000 = $40000.  */
      expandselling (r, sellorders);

      /* Nun kaufen die Spieler Waren. Angenommen es ist fuer 10x weniger Geld ($3300): Bauern $136300, Spieler
         $36700.  */
      expandbuying (r, buyorders);

      /* Das Geld der Bauern kann als Steuern abgeknoepft werden. Die Bauern gehen entsprechend zugrunde, wenn
         zuviele Steuern eingetrieben werden.  Bauern $100000, Spieler +$36300 = 73000.  */
      expandtax (r, taxorders);

      /* Tests */
      assert (r->money >= 0);
      assert (r->peasants >= 0);
      for (u = r->units; u; u = u->next)
	{
	  assert (u->money >= 0);
	  assert (u->number >= 0);
	  for (i = 0; i != MAXITEMS; i++)
	    assert (u->items[i] >= 0);
	}
    }
  putchar ('\n');
}
