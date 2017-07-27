/* German Atlantis PB(E)M host Copyright (C) 1995-1998  Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "atlantis.h"
 
#define ILLUSIONMAX                 6   /* ein halbes Jahr */
#define UNDEAD_REPRODUCTION         10  /* vermehrung */
#define UNDEAD_MIN                  90  /* mind. zahl vor weg gehen */
#define UNDEAD_BREAKUP              20  /* chance dafuer */
#define UNDEAD_BREAKUP_FRACTION     40  /* anteil der weg geht */
#define UNDEAD_MOVE                 15  /* chance fuer bewegung */

static int scared_of[MAXTYPES] =
{
  0,
  1,
  0,
  20,
  30,
  40,
};

/* ------------------------------------------------------------- */

#define MAXUNDEAD_TEXTS     3

static char *undead_name[MAXUNDEAD_TEXTS] =
{
  "Ausgemergelte Skelette",
  "Zerfallende Zombies",
  "Keuchende Ghoule",
};

static char *undead_display[MAXUNDEAD_TEXTS] =
{
  "in ihren Augenhoehlen flackert ein unheiliges Feuer, ihr irres "
  "Grinsen erscheint im fahlen Mondlicht der diabolischen Freude "
  "am Toeten zu entspringen. Schon manche Bauernfamilie verschwand, "
  "als in einsamen Naechten ihr knirschenden Gang zu hoeren war.",

  "in groben Fetzen faellt ihnen das Fleisch vom Leibe, ihre Haut "
  "ist grau und faulig gruen, ihre Augen weiss wie blinde Steine, "
  "und ihr Atem bringt die Pest. Gar mancher Wanderer war eines "
  "Nachts von einer Gruppe Zombies umzingelt, die sich leise wie "
  "der Tod angeschlichen hatten, und nie mehr ward von ihm gehoert.",

  "schwarz und kalt scheinen diese Schatten in der Nacht, nur der "
  "fahle Glanz ihrer Augen und ihr roechelnder, verpesteter Atem "
  "warnen vor dem nahenden Tod durch die kinderfressenden Ghoule.",
};

#define MAXILLUSION_TEXTS   3

static char *illusion_name[MAXUNDEAD_TEXTS] =
{
  "Dunkle Ritter in grauen Umhaengen",
  "Finstere Ritter, in schwarze Maentel gehuellt",
  "Prachtvolle Schar von Rittern",
};

static char *illusion_display[MAXUNDEAD_TEXTS] =
{
  "diese dunklen Ritter sind in der Abenddaemmerung fast nicht zu "
  "sehen. Still wie Steine sitzen sie auf ihren Pferden und starren "
  "mit ihren kalten Augen in die Leere.",

  "unter ihren Kapuzen funkeln stechenden Augen. Unbeweglich und "
  "drohend erblickt man sie des Nachts auf ihren Rappen sitzen und "
  "ins Dunkle starren, als wenn sie schon bald einen Ueberfall "
  "erwarten.",

  "stolz sitzen die glaenzenden Ritter mit geschlossenen Visiren auf "
  "ihren Schimmeln im funkelnden Sonnenlicht. Eine Aura der "
  "Erhabenheit umgibt sie, und nie sieht man die Bauern sich einem "
  "der Ritter naehern."
};

/* Beschuetzen Newbies in neuen Regionen.  */

#define MAXGUARDS_TEXTS   4

static char *guards_name[MAXGUARDS_TEXTS] =
{
  "Eine Gemeinschaft von Bergmoenchen",
  "Eine Gemeinschaft von Kriegermoenchen",
  "Eine Gemeinschaft von Bettelmoenchen",
  "Eine Gemeinschaft von alten Moenchen",
};

static char *guards_display[MAXGUARDS_TEXTS] =
{
  "Mit einer dunkelroten Robe umhuellt und ihrem langen, gebogenen "
  "Schwert auf dem Ruecken gehen sie gebueckt und bescheiden ihrer "
  "Arbeit nach. Im Laufe eines Monats kommen die Moenche in der "
  "ganzen Region herum, versuchen mit Wanderern Kontakte zu knuepfen "
  "und betonen immer wieder die Wichtigkeit des Friedens.",

  "Die braune Robe der Moenche verhuellt nur knapp ihren schwarzen "
  "Saebel, den sie an der Seite tragen. Sie scheinen friedlich ihrer "
  "Arbeit nachzugehen - Holz hacken, Acker pfluegen - doch am fruehen "
  "Morgen sieht man sie vor ihrem Kloster sich im Kampf ueben. Die "
  "Saebelkaempfe muessen eine rituelle Bedeutung fuer die Moenche "
  "haben.",

  "Eine milde Gabe, der Herr! Eine milde Gabe, gnae' Frau! Mit einer "
  "kleinen Blechschuessel ziehen die Bettelmoenche jeden Morgen durch "
  "die Quartiere und bitten um Almosen, welche die Bevoelkerung ihnen "
  "auch gerne gewaehrt. Jeder der Bettelmoenche traegt einen langen, "
  "rostigen Dolch bei sich, mit dem er Leib und Leben verteidigen kann.",

  "Die alten Maenner scheinen alle einen geschwaechten Eindruck zu machen. "
  "Mit zittriger Stimme bitten sie die Bauern um Almosen, beten stundenlang "
  "und hoffen auf die Gnade der Goetter. Seltsamerweise besitzt jeder der "
  "Moenche ein altes, schartiges Schwert. Wenn man sie darauf anspricht, sagen "
  "sie, dass man auch als Moench vor Strolchen nicht sicher sei.",
};

/* Leben moeglicherweise in diesen Gebaeuden */

#define MAXBUILDING_TEXTS   3

static char *building_name[MAXBUILDING_TEXTS] =
{
  "Steinernes Kloster",
  "Befestigtes Kloster",
  "Aermliches Kloster",
};

static char *building_display[MAXBUILDING_TEXTS] =
{
  "Klein und eng ist es in diesem Kloster. Die Waende sind nicht" 
  "verputzt und die Einrichtung ist karg. Stille herrscht.",

  "Hinter dicken Mauern und draengen sich die kleinen Klausen des "
  "Klosters in die Schatten des Gemaeuers. Das Leben hier ist hart und "
  "karg.",

  "Das krude Bauwerk ist schon halb verfallen. Die niedrige Decke "
  "ist an manchen Orten eingefallen, die Ecken der Zimmer voller feuchtem "
  "Stroh.",
};

/* Diese Botschaften schicken sie an die Regionen, in denen sie leben.  */

#define MAXGUARDS_MESSAGES  6

static char *guards_message[MAXGUARDS_MESSAGES] =
{
  "Liebe Mitbewohner! Habt keine Angst, wir werden euch gegen alle "
  "Angriffe schuetzen. Wir sind Waechter des Friedens. Wir werden "
  "keinen Ueberfall dulden.",

  "Wir bitten euch, uns so wenig wie moeglich zu beachten. Wir sind "
  "eine geschlossene Klostergemeinschaft und beten taeglich drei "
  "Stunden fuer den Frieden.",

  "Helft mit, Krieg und Leiden zu verhindert. Gruendet auch ihr "
  "kleine Gemeinschaften, die sich dem Frieden verschrieben haben. "
  "Glaubt uns: Mit Gewalt erzeugt man nur weitere Gewalt, unter der "
  "man selber auch zu leiden hat.",

  "Friede sei mit euch, liebe Mitmenschen. Lasst euch nicht zu Gewalt "
  "und Totschlag verleiten. Kommt Zeit kommt Rat. Wir glauben fest, "
  "dass es nicht zu Blutvergiessen kommen darf!",

  "Friede und Einheiten fuer alle! Betet, liebe Mitbewohner, fuer "
  "das Glueck eurer Nachbarn.",

  "Seid gesegnet, Nachbarn. Moegen eure Ernten reich und eure Reisen "
  "erfolgreich sein.",
};

/* Im Unterschied zu obigen Beschreibungen werden diese Texte
   untereinander per Zufall kombiniert!  */

#define MAXDRAGON_TEXTS   11

static char *dragon_name[MAXDRAGON_TEXTS] =
{
  "Echthelion",
  "Ghrauthlin",
  "Althilorgh",
  "Uulonarchg",
  "Snauplirtan",
  "Zhecchellar",
  "Nolithiin",
  "Khaurranthor",
  "Tharrganthil",
  "Viithalgarr",
  "Ylnarghashorg",
};

static char *dragon_suffix[MAXDRAGON_TEXTS] =
{
  "der Schreckliche",
  "der Grausame",
  "der Fuerchterliche",
  "der Hinterlistige",
  "der Blutruenstige",
  "der Gierige",
  "der Hinterhaeltige",
  "der Engel des Todes",
  "die Nemesis",
  "der Zerstoerer",
  "der dreimal Verfluchte",
};

static char *dragon_display[MAXDRAGON_TEXTS] =
{
  "seine gelben Augen glitzern vor Goldgier und Mordlust. Um ihn "
  "weht der Hauch des Todes. Blut trocknet an seinen Krallen. Seine "
  "Stimme verkuendet das Ende. Sein Atem ist das Fegefeuer. Verflucht "
  "sind seine Opfer, und verzweifelt seine Gegner.",

  "seine Stimme schwingt in der Luft wie der Nachhall eines suessen "
  "Versprechens. Verlorene Liebe, dunkles Kerkerleben und dunkle Lueste "
  "flimmern am Rande des Bewusstseins. Betaeubend wie der verbotene Duft "
  "fleischiger Blueten versetzt sein Verderben den Verstand mit "
  "Daemmertraeumen der Liebe und des Todes.",

  "sein Schrei ist ein Donnerbeben, ein Zerreisen der Erde, ein "
  "Zerstruemmern der Berge. Sein naechtlicher Zorn zerfetzt die "
  "Dunkelheit und laesst Schlaf und Tod einen fuerchterlichen "
  "Tanz toben, dessen Ende ein kochendes Flammenmeer des Wahnsinns ist.",

  "sein Atem bringt feurigen Tod. Wenn sein Kopf sich aus den Schatten "
  "erhebt, schweigt die Welt fuer einen Augenblick. Stumm oeffnet sich "
  "der todbringende Rachen, und dann erhebt sich ein Tosen, ein Brennen, "
  "ein wildes Sprengen von Holz und Stein, ein elendes, wimmerndes "
  "Schreien der Toten und ein seufzen des gepeinigten Bodens.",

  "guetig und freundlich klingt seine Stimme. Seine Worten verweben sich "
  "zu einem Netz von Versprechungen. Trost und Ermutigung faerbt seinen "
  "Zuspruch. Macht und Erfuellung schimmern durch seine Rede. Tiefe "
  "Freundschaft wird hier angedeutet, wahrhaftiges Vertrauen geweckt. "
  "Nur seine unergruendlichen Augen scheinen ein eigenes Geheimnis zu hueten.",

  "sein schlurfender Schritt hoert der einsame Wanderer nur in tiefster "
  "Nacht. Der drohende Tod legt sich wie eine kalte Klammer ums Herz, wenn "
  "die unnatuerliche Stille sich bald wieder ausbreitet. Die Lauer will "
  "nicht enden, erst wenn die Angst die Luft fast zu dick zum Atmen macht, "
  "hoert man noch einen Ton, ein dumpfes, lauter werdendes Brausen. Der "
  "Beginn des Fegefeuers.",

  "jeder Bauer hier weiss, dass nur Silber das Vieh besaenftigen kann. "
  "Riecht er noch Silber im Dorf, welches nicht auf dem klaeglichen Haufen "
  "des Marktplatzes liegt, haben die Bewohner ihr leben verspielt. Sein "
  "Zorn laesst nur noch einen stillen Haufen glimmender Ruinen und ein "
  "paar verkohlte Leichnahme zurueck.",

  "hypnotisch funkeln die Augen der Bestie, wenn sie sich langsam "
  "naehert. Leise summt sie ein Lied steigender Panik, welches jedem "
  "Gegner das Herz verstummen laesst. Aus dem tiefen Schlund grollt "
  "das Hoellenfeuer hervor, welches dort brodelt. So einem Feind ist "
  "kein Mensch gewachsen.",

  "seine gelben Augen glitzern vor Hass und Vorfreude. Ein Dunst von "
  "Verwesung umgiebt ihn. Die bleichen Knochen seiner Opfer finden "
  "sich ueberall. Die Zeit um ihn herum scheint aelter, langsamer zu "
  "sein. Dies ist kein Ort fuer Menschen! Die schwere Stille wird nur "
  "vom zischenden Atem der Bestie unterbrochen.",

  "seine Klauen sind der eisige Tod. Sein Atem der Hauch der sieben "
  "Hoellen. Sein Blick verflucht, seine Stimme laehmt. Sein Panzer "
  "ist ein Werk von 100 Teufeln, der Glanz seiner Zaehne durchbohrt 1000 "
  "Herzen. Sein Name ist Urteil, sein Urteil der Tod.",

  "zuckendes Fleisch und aufquellendes, dunkelrotes Blut sind sein Genuss. "
  "Kreischender Tod und flammenbringender Wahnsinn sind seine Genossen. "
  "Seine Klauen aus Stahl zerfetzen die Opfer, zertruemmern die Stadtmauern, "
  "und vernichten jegliche Hoffnung. Es gibt keinen Widerstand gegen "
  "Ihn.",
};


/* ------------------------------------------------------------- */

void
attacked_by_monster (unit * u, unit * u2)
{
  strlist *S;

  sprintf (buf, "%s %d",
           keywords[K_ATTACK], u2->no);
  S = makestrlist (buf);
  addlist (&u->orders, S);
}

void
taxed_by_monster (unit * u)
{
  strlist *S;

  S = makestrlist (keywords[K_TAX]);
  addlist (&u->orders, S);
}

int
get_money_for_monster (region * r, unit * u, int wanted)
{
  unit *u2;
  int n;

  /* attackiere bewachende Einheiten. */
  for (u2 = r->units; u2; u2 = u2->next)
    if (u2 != u && u2->guard)
      attacked_by_monster (u, u2);

  /* Falls das genug Geld ist, bleibt das Monster hier. */
  if (r->money >= wanted)
    return 1;

  /* Attackiere so lange die fremden, alteingesessenen Einheiten mit Geld
     (einfach die Reihenfolge der Einheiten), bis das Geld da ist. n zaehlt
     das so erhaltene Geld. */
  n = 0;
  for (u2 = r->units; u2; u2 = u2->next)
    if (u2->faction != u->faction && u2->money)
      {
        attacked_by_monster (u, u2);
        n += u2->money;
	/* Falls die Einnahmen erreicht werden, bleibt das Monster
	   noch eine Runde hier.  */
        if (n + r->money >= wanted)
          return 1;
      }

  /* Falls das Geld uns nicht genuegt, es aber zum Leben reicht,
     koennen wir uns fortbewegen.  Wir duerfen Steuereinnahmen nicht
     beruecksichtigen - wir bewegen uns ja fort.  */
  if (u->money + n >= MAINTENANCE)
    return 0;

  /* Falls wir auch mit Angriffe nicht genug Geld zum wandern haben,
     muessen wir wohl oder uebel hier bleiben.  Vielleicht wurden ja
     genug Steuern eingetrieben.  */
  return 1;
}

int
money (region * r)
{
  unit *u;
  int m;

  m = r->money;
  for (u = r->units; u; u = u->next)
    m += u->money;
  return m;
}

int
richest_neighbour (region * r)
{

  /* m - maximum an Geld, d - Richtung, i - index, t = Geld hier */

  int m, d = -1, i, t;

  m = money (r);

  /* finde die region mit dem meisten geld */

  for (i = 0; i != MAXDIRECTIONS; i++)
    if (r->connect[i] && r->connect[i]->terrain != T_OCEAN)
      {
        t = money (r->connect[i]);
        if (t > m)
          {
            m = t;
            d = i;
          }
      }

  return d;
}

int
random_walk (region * r)
{

  int d=-1, i, m=0;

  /* Ueberpruefen, ob wir nicht auf einer Insel sind.  */
  for (i=0; i != MAXDIRECTIONS; i++)
    if (r->connect[i] && r->connect[i]->terrain != T_OCEAN)
      m++;

  if (m)
    while (d == -1)
      {
	i = rand () % MAXDIRECTIONS;
	if (r->connect[i] && r->connect[i]->terrain != T_OCEAN)
	  d = i;
      }
      
  return d;
}

void
move_monster (region * r, unit * u)
{
  int d;
  strlist *S;

  d = richest_neighbour (r);

  /* Falls kein Geld gefunden wird, zufaellig verreisen, aber nicht in den
     Ozean. */
  if (d == -1)
    d = random_walk (r);
  if (d == -1)
    return; /* Offensichtlich sind wir auf einer Insel...  */

  sprintf (buf, "%s %s", keywords[K_MOVE], directions[d]);

  S = makestrlist (buf);
  addlist (&u->orders, S);

}

void
monster_work (unit * u)
{
  strlist *S;

  S = makestrlist (keywords[K_WORK]);
  addlist (&u->orders, S);
}

void
monster_message (unit * u)
{
  strlist *S;
  int i;

  switch (u->type)
    {
    case U_GUARDS:
      i = rand () % MAXGUARDS_MESSAGES;
      sprintf (buf, "%s %s %s",
        keywords[K_MAIL], parameters[P_REGION], guards_message[i]);
      S = makestrlist (buf);
      addlist (&u->orders, S);
    }
}
/* ------------------------------------------------------------- */

void
scared_by_monster (region * r, unit * u, int p)
{
  int d;

  d = wanderoff (r, p);
  sprintf (buf, "%s hat %d %s verscheucht!",
           unitid (u), d, d == 1 ? "Bauer" : "Bauern");
  addmessage (u->faction, buf);
}

void
plan_monsters (void)
{
  region *r;
  faction *f;
  unit *u;

  /* Finde die Monster (0). */
  f = findfaction (0);
  if (!f)
    return;

  /* Verhindere loeschen der Monster. */
  f->lastorders = turn;

  /* Befehle werden fuer alle Monster generiert.  Sollten die Spieler auch Einheiten eines nicht-menschlichen
     Typs haben, so werden unausweichliche Folgen dort bei age_unit () behandelt.  Die Spieler koennen
     ansonsten ihren Einheiten aber durchaus normale Befehle geben! Ab und zu sollen die Monster nicht
     arbeiten, haben aber sonst auch keinen Default Befehl.  Das wird in setdefaults behandelt.  */
  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      if (u->faction == f)
        switch (u->type)
          {
          case U_GUARDS:
	    /* Arbeiten friedlich. */
            if (!u->faction->no)
	      {
		monster_work (u);
		monster_message (u);
	      }
            break;

          case U_FIREDRAGON:
          case U_DRAGON:
          case U_WYRM:
            /* Treiben Steuern ein und verscheuchen Bauern.  Attackieren
	       bewachende Einheiten und Einheiten, die viel Geld haben. */
            if (!u->faction->no)
	      {
		if (get_money_for_monster (r, u, income[u->type]))
		  taxed_by_monster (u);
		else
		  move_monster (r, u);
	      }
	    scared_by_monster (r, u, u->number
                               * scared_of[u->type]);
            break;

          case U_UNDEAD:
            /* Untote treiben keine Steuern ein.  Sie wandern zufaellig
               umher. */
	    if ((!u->faction->no) && rand () % 100 < UNDEAD_MOVE)
	      move_monster (r, u);
	    scared_by_monster (r, u, u->number
			       * scared_of[u->type]);
            break;

          }

}

/* ------------------------------------------------------------- */

unit *
make_undead_unit (region * r, faction * f, int n)
{
  unit *u;
  int i;

  u = createunit (r);

  u->type = U_UNDEAD;
  u->number = n;
  u->faction = f;

  u->lastorder[0] = 0;

  i = rand () % MAXUNDEAD_TEXTS;
  mnstrcpy (&u->name, undead_name[i], NAMESIZE);
  mnstrcpy (&u->display, undead_display[i], DISPLAYSIZE);

  u->status = ST_FIGHT;

  return u;
}

unit *
make_illsionary_unit (region * r, faction * f, int n)
{
  unit *u;
  int i;

  u = createunit (r);

  u->type = U_ILLUSION;
  u->number = n;
  u->faction = f;

  u->lastorder[0] = 0;

  i = rand () % MAXILLUSION_TEXTS;
  mnstrcpy (&u->name, illusion_name[i], NAMESIZE);
  mnstrcpy (&u->display, illusion_display[i], DISPLAYSIZE);

  /* Kein Problem, da die Ritter ja nie an Kaempfen teilnehmen! */

  u->status = ST_AVOID;
  u->items[I_SPEAR] = n;
  u->items[I_HORSE] = n;
  u->items[I_PLATE_ARMOR] = n;
  u->skills[SK_RIDING] = n * 30;

  return u;
}

unit *
make_guards_unit (region * r, faction * f, int n)
{
  unit *u;
  int i;

  u = createunit (r);

  u->type = U_GUARDS;
  u->number = n;
  u->faction = f;
  u->skills[SK_OBSERVATION]= n * 180;
  u->lastorder[0] = 0;
  u->money = income[u->type] * u->number;

  i = rand () % MAXGUARDS_TEXTS;
  mnstrcpy (&u->name, guards_name[i], NAMESIZE);
  mnstrcpy (&u->display, guards_display[i], DISPLAYSIZE);

  /* Kein Problem, da die Ritter ja nie an Kaempfen teilnehmen! */

  u->status = ST_FIGHT;
  u->items[I_SWORD] = n;
  u->skills[SK_SWORD] = n * 180;

  return u;
}

unit *
make_guarded_tower_unit (region *r, faction *f, int n, int m)
{
  unit *u;
  building *b;
  int i;

  u = make_guards_unit (r, f, n);
  b = create_building (r, u);
  b->size = m;
  i = rand () % MAXBUILDING_TEXTS;
  mnstrcpy (&b->name, building_name[i], NAMESIZE);
  mnstrcpy (&b->display, building_display[i], DISPLAYSIZE);

  return u;
}

void
name_dragon (unit *u)
{
  int i;
  
  i = rand () % MAXDRAGON_TEXTS;
  strcpy (buf, dragon_name[i]);
  i = rand () % MAXDRAGON_TEXTS;
  scat (" ");
  scat (dragon_suffix[i]);
  mnstrcpy (&u->name, buf, NAMESIZE);

  i = rand () % MAXDRAGON_TEXTS;
  mnstrcpy (&u->display, dragon_display[i], DISPLAYSIZE);
}

unit *
make_firedragon_unit (region * r, faction * f, int n)
{
  unit *u;

  u = createunit (r);

  u->type = U_FIREDRAGON;
  u->number = n;
  u->faction = f;
  u->skills[SK_OBSERVATION] = n * 180;
  u->skills[SK_TACTICS] = n * 30;
  u->money = income[u->type] * u->number;
  u->lastorder[0] = 0;
  u->items[I_DRAGON_PLATE] = n;
  u->status = ST_FIGHT;
  name_dragon (u);
  return u;
}

unit *
make_dragon_unit (region * r, faction * f, int n)
{
  unit *u;

  u = createunit (r);

  u->type = U_DRAGON;
  u->number = n;
  u->faction = f;
  u->skills[SK_OBSERVATION] = n * 450;
  u->skills[SK_TACTICS] = n * 180;
  u->money = income[u->type] * u->number;
  u->lastorder[0] = 0;
  u->items[I_DRAGON_PLATE] = n;
  u->status = ST_FIGHT;
  name_dragon (u);
  return u;
}

unit *
make_wyrm_unit (region * r, faction * f, int n)
{
  unit *u;

  u = createunit (r);

  u->type = U_WYRM;
  u->number = n;
  u->faction = f;
  u->skills[SK_OBSERVATION] = n * 630;
  u->skills[SK_TACTICS] = n * 630;
  u->money = income[u->type] * u->number;
  u->lastorder[0] = 0;
  u->items[I_DRAGON_PLATE] = n;
  u->status = ST_FIGHT;
  name_dragon (u);
  return u;
}

/* ------------------------------------------------------------- */

void
age_unit (region * r, unit * u)
{
  int n, m;

  switch (u->type)
    {
      /* Bestimmte Sprueche wirken nur eine bestimmte Zeit.  Dies wird
	 hier geprueft.  Werden sie gezaubert beenden die Einheiten
	 die Runde mit u->effect == 0, also wird von 0 aus gezaehlt.
	 Deswegen wird hier beim Vergleich auch >= statt > verwendet.  */
    case U_MAN:
      if (!u->enchanted)
	break;
      switch (u->enchanted)
	{
	case SP_NIGHT_EYES:
	  u->effect++;
	  if (u->effect >= NIGHT_EYES_MAX)
	    u->enchanted = 0;
	  break;

	case SP_WATER_WALKING:
	  u->effect++;
	  if (u->effect >= WATERWALK_MAX)
	    u->enchanted = 0;
	  break;
	}
      break;
	
      /* Illusionen verschwinden nach einer bestimmten Zeit.  Das
         Alter der Illsusion steht in u->effect.  */
    case U_ILLUSION:
      u->effect++;
      if (u->effect == ILLUSIONMAX)
        {
          sprintf (buf, "%s wird sich bald verfluechtigen.",
                   unitid (u));
          addevent (u->faction, buf);
        }
      else if (u->effect > ILLUSIONMAX)
        {
          u->number = 0;
          sprintf (buf, "%s hat sich unbemerkt verfluechtigt.", unitid (u));
          addevent (u->faction, buf);
        }
      break;

      /* Untote vermehren und verselbststaendigen sich mit einer
         bestimmten Wahrscheinlichkeit. */
    case U_UNDEAD:
      n = 0;
      for (m = u->number; m; m--)
        if (rand () % 100 < UNDEAD_REPRODUCTION)
          n++;
      n = min (n, r->peasants);
      r->peasants -= n;
      u->number += n;

      /* Untote, die einer Partei (dh. nicht der Partei Monster (0))
         angehoeren, koennen absplittern und eigene Einheiten
         bilden. */
      if (u->number > UNDEAD_MIN &&
          u->faction->no &&
          rand () % 100 < UNDEAD_BREAKUP)
        {
          n = 0;
          for (m = u->number; m; m--)
            if (rand () % 100 < UNDEAD_BREAKUP_FRACTION)
              n++;
          u->number -= n;
          make_undead_unit (r, findfaction (0), n);
        }
    }
}

/* -- Erschaffung neuer Einheiten ------------------------------ */

/* Hoechste Einheit-Nummer, wird in save.c:readgame () zum ersten Mal gesetzt. */
int highest_unit_no; 
/* Sobald die aktuelle Nummer groesser als highest_unit_no ist, wird nicht mehr getested ob eine betreffende
   Einheit schon existiert, die Nummer wird einfach um 1 erhoeht und verwendet (beschleunigtes finden von
   Nummern!) */
int last_new_unit_no; 

unit *
createunit (region *r1)
{
  unit *u;

  u = cmalloc (sizeof (unit));
  memset (u, 0, sizeof (unit));

  mstrcpy (&u->lastorder, keywords[K_WORK]);
  u->combatspell = -1;

  /* Falls noch nicht die hoechste Zahl erreicht wurde, mache findunitglobal (), ansonsten erhoehe
     last_new_unit_no einfach um eins.  */
  do
    last_new_unit_no++;
  while (last_new_unit_no <= highest_unit_no && findunitglobal (last_new_unit_no));
  u->no = last_new_unit_no;

  sprintf (buf, "Nummer %d", u->no);
  mnstrcpy (&u->name, buf, NAMESIZE);
  addlist (&r1->units, u);
  return u;
}

void
new_units (void)
{
  region *r;
  unit *u, *u2;
  strlist *S, *S2;

  /* Neue Einheiten werden gemacht und ihre Befehle (bis zum "ende" zu ihnen rueberkopiert, damit diese
     Einheiten genauso wie die alten Einheiten verwendet werden koennen.  */
  printf ("- neue Einheiten erschaffen... ");

  last_new_unit_no = 0;
  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      for (S = u->orders; S;)
        {
          if ((igetkeyword (S->s) == K_MAKE) && (getparam () == P_TEMP))
            {
              u2 = createunit (r);

              u2->alias = geti ();
              if (u2->alias == 0) /* zweite Chance falls jemand TEMP EINHEIT 2 statt TEMP 2 schreibt...  */
                u2->alias = geti ();
#ifdef DEBUG_TEMP_UNITS
	      printf ("\n%d macht %d (TEMP %d)", u->no, u2->no, u2->alias);
#endif
#ifndef DEBUG_TEMP_UNITS
	      indicator_tick ();
#endif
              u2->faction = u->faction;
              u2->building = u->building;
              u2->ship = u->ship;

              S = S->next;

              while (S)
                {
                  if (igetkeyword (S->s) == K_END)
                    break;
                  S2 = S->next;
                  translist (&u->orders, &u2->orders, S);
                  S = S2;
                }
            }
          if (S)
            S = S->next;
        }
  /* im for-loop wuerde S = S->next ausgefuehrt, bevor S geprueft wird. Wenn S aber schon 0x0 ist, fuehrt das
     zu einem Fehler. Und wenn wir den while (S) ganz durchlaufen, wird S = 0x0 sein!  Dh. wir muessen
     sicherstellen, dass S != 0, bevor wir S = S->next auszufuehren! */

  putchar ('\n');

  /* Damit die Sache auch bei einem zweiten Durchgang funktioniert, muss highest_unit_no meist neu gesetzt
     werden.  */
  if (last_new_unit_no > highest_unit_no)
    highest_unit_no = last_new_unit_no;
}






