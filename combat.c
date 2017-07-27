/* German Atlantis PB(E)M host Copyright (C) 1995-1998  Alexander Schroeder
   
   based on:

   Atlantis v1.0 13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "atlantis.h"
 
#define COMBATEXP               10

#define WEAPON_BONUS             2
#define BUILDING_BONUS           2

#define CATAPULT_HITS           20

/* Mit #define DEBUG_COMBAT kann man das Schreiben von Kampf-Debugs ermoeglichen. Diese erhalten alle Spieler,
   welche SENDE DEBUG gesetzt haben. Der Debug enthaelt auch die Kampftalente der Gegner, so dass damit Infos,
   welche missbraucht werden koennen, an die Spieler abgegeben werden.  */

enum
{
  A_NONE,
  A_CHAIN,
  A_PLATE,
  A_DRAGON_PLATE,
  A_INVULNERABLE,
  A_FIREDRAGON,
  A_DRAGON,
  A_WYRM,
  MAXARMOR
};

/* Der Wert ist ein Teil von 10000.  */
#define MAX_ARMOR_VALUE      10000
int armor_value[MAXARMOR] =
{
  0,
  3333,   /* 1/3        */
  6666,   /* 2/3        */
  9500,   /* 19/20      */
  9999,   /* 9999/10000 */
  9500,   /* 19/20      */
  9800,   /* 49/50      */
  9900,   /* 99/100     */
};

char attack_default[MAXTYPES] =
{
  -2,                           /* mensch */
  0,                            /* untote */
  0,                            /* illusionen - kaempfen nie! */
  3,                            /* feuerdrachen */
  5,                            /* drachen */
  7,                            /* wyrm */
};

#define MAX_ADVANTAGE           5

char attack_table[MAX_ADVANTAGE] =
{
   1,
  10,
  25,
  40,
  49,
};

enum
  {
    SI_DEFENDER,
    SI_ATTACKER,
  };

void
battlerecord (char *s)
{
  faction *f;

  for (f = factions; f; f = f->next)
    if (f->seesbattle)
      addstrlist (&f->battles, s);
      /*sparagraph (&f->battles, s, 0, 0);*/
}

#ifdef DEBUG_COMBAT
#define battledebug(a) _battledebug(a)
#else
#define battledebug(a)
#endif

void
_battledebug (char *s)
{
  faction *f;

  for (f = factions; f; f = f->next)
    if (f->seesbattle)
      addstrlist (&f->debug, s);
}

void
battlepunit (region * r, unit * u)
{
  faction *f;

  for (f = factions; f; f = f->next)
    if (f->seesbattle)
      addstrlist (&f->battles, spunit (f, r, u, 1));
}

typedef struct troop
  {
    struct troop *next;
    unit *unit;
    int lmoney;
    char status;
    int side;
    int weapon;
    char attacked;
    char missile;
    char skill;
    char armor;
    char behind;
    char inside;
    char riding;
    char reload;
    char canheal;
    char runesword;
    char power;
    char shieldstone;
    char demoralized;
    char dazzled;
  }
troop;

char buf2[NAMESIZE + 20];
int ntroops;
troop *ta;
troop attacker, defender;
int initial[2];
int left[2];
int infront[2];
int toattack[2];
int shields[2];
int runeswords[2];

troop **
maketroops (troop ** tp, unit * u, int mainterrain)
{
  int i;
  troop *t;
  static int skills[MAXSKILLS];
  static int items[MAXITEMS];

  /* Kopiere Daten der unit. skills ist nun immer effskill(), und items
     koennen nun auf jede einzelne troop verteilt werden */
  for (i = 0; i != MAXSKILLS; i++)
    skills[i] = effskill (u, i);
  memcpy (items, u->items, sizeof items);

  /* Einteilung der Einheit in der Schlacht */
  left[u->side] += u->number;
  if (u->status == ST_FIGHT)
    infront[u->side] += u->number;

  /* Erzeugung von jedem einzelnen troop (ein Kaempfer) */
  for (i = u->number; i; i--)
    {
      t = cmalloc (sizeof (troop));
      memset (t, 0, sizeof (troop));
      addlist2 (tp, t);

      /* Einheitsinformation wird kopiert. Als Kampftalent wird der Wert aus
         der attack_default[] Tabelle genommen. Dies ist das Kampftalent aller
         nicht-menschlichen Einheiten (Zombies, Drachen etc).  */
      t->unit = u;
      t->side = u->side;
      t->behind = (u->status != ST_FIGHT);
      t->skill = attack_default[u->type];
      t->armor = A_NONE;

      /* Verteidiger in BURGEN setzen bekommen keinen Pferdebonus, sondern einen Burgenbonus. Greifen die
         Einwohner einer Burg selber an, bekommen sie keinen Burgenbonus, sondern den Pferdebonus.  Dies ist
         wichtig, weil der Pferdebonus wiederum bestimmt, was mit den Speeren geschieht - vgl. hits
         (). t->inside wird dort gebraucht, um zu sehen, ob die jeweiligen Gegner den Pferdebonus bekommen
         oder nicht.  */
      if (u->building && u->building->sizeleft
          && u->side == SI_DEFENDER)
        {
          t->inside = 1;
          u->building->sizeleft--;
        }

      /* PFERDE kann man gebrauchen, wenn man ein Pferd hat, ein Reiten Talent von 2 hat, in einer Ebene
         (vgl. mainterrain) kaempft, und sich nicht in einer Burg verteidigt (!t->inside).  Mit Pferden kann
         man Runenschwerter, Schwerter und Speere verwenden!  Als Angreifer aus einer Burg heraus ist
         t->inside==0 und somit kann ein Pferd verwendet werden!  */

      /* WAFFEN - in der Reihenfolge ihrer Prioritaeten. Nur eine Waffe pro Mann! Also werden Schuetzen nicht
         mit Schwertern ausgeruestet.  Hier wird t->skill neu gesetzt, dh. alle Boni darf man erst nachher
         addieren! */
      if (u->combatspell >= 0)
        {
          /* Magier ohne Waffen! */
          t->missile = 1;
        }
      else if (items[I_RUNESWORD] && skills[SK_SWORD])
        {
          /* Runenschwerter */
          t->weapon = I_SWORD;
          t->skill = skills[SK_SWORD] + 2;      /* Bonus fuer Runenschwert */
          t->runesword = 1;
          items[I_RUNESWORD]--;
          runeswords[u->side]++;
	  /* ... mit Pferd */
          if (items[I_HORSE] && skills[SK_RIDING] >= 2
              && mainterrain == T_PLAIN
              && !t->inside)
            {
              t->riding = 1;
              items[I_HORSE]--;
            }
        }
      else if (items[I_CATAPULT] && skills[SK_CATAPULT])
        {
          /* Katapult */
          t->weapon = I_CATAPULT;
          t->missile = 1;
          t->skill = skills[SK_CATAPULT];
          items[I_CATAPULT]--;
        }
      else if (items[I_LONGBOW] && skills[SK_LONGBOW])
        {
          /* Langbogen */
          t->weapon = I_LONGBOW;
          t->missile = 1;
          t->skill = skills[SK_LONGBOW];
          items[I_LONGBOW]--;
        }
      else if (items[I_CROSSBOW] && skills[SK_CROSSBOW])
        {
          /* Armbrust */
          t->weapon = I_CROSSBOW;
          t->missile = 1;
          t->skill = skills[SK_CROSSBOW];
          items[I_CROSSBOW]--;
        }
      else if (items[I_SPEAR] && skills[SK_SPEAR])
        {
          /* Speer */
          t->weapon = I_SPEAR;
          t->skill = skills[SK_SPEAR];
          items[I_SPEAR]--;
	  /* ... mit Pferd */
          if (items[I_HORSE] && skills[SK_RIDING] >= 2
              && mainterrain == T_PLAIN
              && !t->inside)
            {
              t->riding = 1;
              items[I_HORSE]--;
            }
        }
      else if (items[I_SWORD] && skills[SK_SWORD])
        {
          /* normales Schwert */
          t->weapon = I_SWORD;
          t->skill = skills[SK_SWORD];
          items[I_SWORD]--;
	  /* ... mit Pferd */
          if (items[I_HORSE] && skills[SK_RIDING] >= 2
              && mainterrain == T_PLAIN
              && !t->inside)
            {
              t->riding = 1;
              items[I_HORSE]--;
            }
        }

      /* RUESTUNG: */
      if (items[I_CLOAK_OF_INVULNERABILITY])
        {
          t->armor = A_INVULNERABLE;
          items[I_CLOAK_OF_INVULNERABILITY]--;
        }
      else if (u->type == U_FIREDRAGON)
        {
          t->armor = A_FIREDRAGON;
        }
      else if (u->type == U_DRAGON)
        {
          t->armor = A_DRAGON;
        }
      else if (u->type == U_WYRM)
        {
          t->armor = A_WYRM;
        }
      else if (items[I_DRAGON_PLATE])
        {
          t->armor = A_DRAGON_PLATE;
          items[I_DRAGON_PLATE]--;
        }
      else if (items[I_PLATE_ARMOR])
        {
          t->armor = A_PLATE;
          items[I_PLATE_ARMOR]--;
        }
      else if (items[I_CHAIN_MAIL])
        {
          t->armor = A_CHAIN;
          items[I_CHAIN_MAIL]--;
        }

      /* MAGISCHE GEGENSTAENDE */
      if (u->spells[SP_HEAL] || items[I_AMULET_OF_HEALING] > 0)
        {
          t->canheal = 1;
          items[I_AMULET_OF_HEALING]--;
        }

      if (items[I_RING_OF_POWER])
        {
          t->power = 1;
          items[I_RING_OF_POWER]--;
        }

      if (items[I_SHIELDSTONE])
        {
          t->shieldstone = 1;
          items[I_SHIELDSTONE]--;
        }
    }

  return tp;
}

int
contest (int attacker, int defender)
{
  int attacker_advantage;

  /* Der kleinste Wert ist -2 ("um mehr als 1 schlechter"). Um den array
     attack_table[] verwenden zu koennen, addieren wir also +2, und
     beschraenken die moeglichen Loesungen.  */

  attacker_advantage = attacker - defender + 2;

  attacker_advantage = max (attacker_advantage, 0);
  attacker_advantage = min (attacker_advantage, MAX_ADVANTAGE - 1);

  return rand () % 100 < attack_table[attacker_advantage];
}

int
hits (void)
{
  int k=-1;

  /* Truppen mit Schusswaffen, die sich ohne Waffen verteidigen muessen.  */
  if (defender.weapon == I_CROSSBOW
      || defender.weapon == I_LONGBOW
      || defender.weapon == I_CATAPULT)
    defender.skill = attack_default[U_MAN];

  /* Ist der Angegriffene nicht in einer Burg und wird von einem Berittenen angegriffen, so erhaelt der
     berittene Angreifer den Pferdebonus.  Verwendet er zudem noch einen Speer, erhaelt er noch zusaetzlich
     den Lanzenbonus.  Verwender der Verteidiger allerdings einen Speer, erhaelt dieser den Pikenbonus.  Dies
     alles ist fuer Schuetzen unwesentlich, so dass defender.skill nachher einfach nicht gebraucht wird.  */
  if (!defender.inside && attacker.riding)
    {
      attacker.skill += 2;      /* Pferdebonus fuer Angreifer */
      if (attacker.weapon == I_SPEAR)
        attacker.skill += 1;    /* Lanzenbonus fuer berittene Angreifer */
      if (defender.weapon == I_SPEAR)
        defender.skill += 1;    /* Pikenbonus gegen berittene Angreifer */
    }

  /* Berittene Verteidiger erhalten auch den Pferdebonus, es sei denn der Angreifer verteidigt eine Burg und
   greift von dort her an. Das bedeuted, dass Pferde beim Sturm auf eine Burg unnuetz sind.  Dies hilft nichts
   gegen Schusswaffen, deswegen kann defender.skill bei Schusswaffen weggelassen werden.  */
  if (!attacker.inside && defender.riding)
    defender.skill += 2;

  /* Effekte von Magie - Verwirrte sind nicht einfacher mit Schusswaffen zu treffen, deswegen wird
     defender.skill hier erhoeht, und bei Schusswaffen nicht verwendet.  */
  attacker.skill -= (attacker.demoralized + attacker.dazzled);
  defender.skill -= (defender.demoralized + defender.dazzled);

  /* Effekte der Waffen: k zeigt an, ob getroffen wurde oder nicht.  */
  switch (attacker.weapon)
    {
      /* Der Angegriffene erhaelt den Burgenbonus, falls er in einer Burg ist.  Burginsassen erhalten bei
	 einem Angriff keinen Burgenbonus, da defender.inside nicht gesetzt ist!  */
    case 0:
    case I_SWORD:
    case I_SPEAR:
      k = contest (attacker.skill, defender.skill + defender.inside ? 2 : 0);
      break;

      /* Die Opfer von Schusswaffen koennen defender.skill nicht verwenden!  Burgen schuetzen vor Schuetzen.
         :) */
    case I_CROSSBOW:
      k = contest (attacker.skill, 0 + defender.inside ? 2 : 0);
      break;

    case I_CATAPULT:
      k = contest (attacker.skill, 1 + defender.inside ? 2 : 0);
      break;

    case I_LONGBOW:
      k = contest (attacker.skill, 2 + defender.inside ? 2 : 0);
      break;

    default:
      assert (0);
      break;
    }

  /* Ein Treffer (k==1) kann durch eine Ruestung annuliert werden; die Ruestung schuetzt vor einem Treffer mit
     einer gewissen Chance.  Ein Mantel der Unverletzlichkeit schuetzt allerdings nicht vor einem
     Runenschwert.  */
  if (k && !(attacker.runesword && defender.armor == A_INVULNERABLE)
      && rand () % MAX_ARMOR_VALUE < armor_value[(int) defender.armor])
    k = 0;
  
  /* Eine getroffene Illusion loest sich immer auf.  */
  if (!k && defender.unit && defender.unit->type == U_ILLUSION)
    k = 1;
  /* Eine Illusion trifft nie.  */
  if (k && attacker.unit && attacker.unit->type == U_ILLUSION)
    k = 0;

  assert (k >= 0);

  /* debug */
  sprintf (buf, "A: %d (%s %+d) -> V: %d (%s %+d, R: %d) und %s.",
           attacker.unit ? attacker.unit->no : 0,
           attacker.weapon ? strings[itemnames[0][attacker.weapon]][0] : "unbewaffnet",
           attacker.skill,
           defender.unit ? defender.unit->no : 0,
           defender.weapon ? strings[itemnames[0][defender.weapon]][0] : "unbewaffnet",
           defender.skill,
           defender.armor,
           k ? "trifft" : "trifft nicht");
  battledebug (buf);
  return k;
}

int
validtarget (int i)
{
  return !ta[i].status &&
    ta[i].side == defender.side &&
    (!ta[i].behind || !infront[defender.side]);
}

int
canbedemoralized (int i)
{
  return validtarget (i) && !ta[i].demoralized;
}

int
canbedazzled (int i)
{
  return validtarget (i) && !ta[i].dazzled;
}

int
canberemoralized (int i)
{
  return !ta[i].status &&
    ta[i].side == attacker.side &&
    ta[i].demoralized;
}

int
selecttarget (void)
{
  int i;

  assert (ntroops);
  do
    i = rand () % ntroops;
  while (!validtarget (i));

  return i;
}

void
terminate (int i)
{
  if (!ta[i].attacked)
    {
      ta[i].attacked = 1;
      toattack[defender.side]--;
    }

  ta[i].status = 1;
  left[defender.side]--;
  if (infront[defender.side])
    infront[defender.side]--;
  if (ta[i].runesword)
    runeswords[defender.side]--;
}

void
dozap (int n)
{
  int di, d=0;

  n = lovar (n * (1 + attacker.power));
  n = min (n, left[defender.side]);

  while (--n >= 0)
    {
      di = selecttarget ();
      /* Entweder sind sie draussen, oder sie sind drinnen und haben eine Chance, den Spruch zu ueberleben. */
      if (!ta[di].inside || rand () % 100 > ZAP_SURVIVAL)
	{
	  d++;
	  terminate (di);
	}
    }
  
  sprintf (buf2, "; %d Gegner %s", d, (d != 1) ? "starben" : "starb");
  scat (buf2);
}

void
docombatspell (int i)
{
  int j;
  int z;
  int n, m;

  z = ta[i].unit->combatspell;

  strcpy (buf, 
	  translate (ST_CASTS_IN_COMBAT, ta[i].unit->faction->language, 
		     unitid (ta[i].unit), 
		     strings[spellnames[z]][ta[i].unit->faction->language]));

  if (shields[defender.side])
    if (rand () & 1)
      {
        scat (strings[ST_BREAKS_THROUGH_MAGIC_SHIELD][ta[i].unit->faction->language]);
        shields[defender.side] -= 1 + attacker.power;
      }
    else
      {
        scat (strings[ST_BUT_CANCELLED_BY_MAGIC_SHIELD][ta[i].unit->faction->language]);
        battlerecord (buf);
        return;
      }

  switch (z)
    {
    case SP_BLACK_WIND:
      dozap (1250);
      break;

    case SP_CAUSE_FEAR:
      if (runeswords[defender.side] && (rand () & 1))
        break;

      /* anzahl pot. Gegner dank der magie */
      n = lovar (100 * (1 + attacker.power));

      /* anzahl undemoralisierter gegner */
      m = 0;
      for (j = 0; j != ntroops; j++)
        if (canbedemoralized (j))
          m++;

      /* wenn es keine Gegner mehr gibt, break */
      if (!m)
        return;

      /* effektive Gegner */
      n = min (n, m);

      sprintf (buf2, "; %d Gegner %s eingeschuechtert", n, 
	       (n != 1) ? "wurden" : "wurde"); 
      scat (buf2);

      while (--n >= 0)
        {
          do
            j = rand () % ntroops;
          while (!canbedemoralized (j));

          ta[j].demoralized = 1;
        }

      break;

    case SP_DAZZLING_LIGHT:
      n = lovar (50 * (1 + attacker.power));

      m = 0;
      for (j = 0; j != ntroops; j++)
        if (canbedazzled (j))
          m++;

      /* wenn es keine Gegner mehr gibt, return */

      if (!m)
        return;

      /* effektive Gegner */

      n = min (n, m);

      sprintf (buf2, "; %d Gegner %s geblendet", n, 
	       (n != 1) ? "wurden" : "wurde");
      scat (buf2);

      while (--n >= 0)
        {
          do
            j = rand () % ntroops;
          while (!canbedazzled (j));

          ta[j].dazzled = 1;
        }

      break;

    case SP_FIREBALL:
      dozap (50);
      break;

    case SP_HAND_OF_DEATH:
      dozap (250);
      break;

    case SP_INSPIRE_COURAGE:
      n = lovar (100 * (1 + attacker.power));

      m = 0;
      for (j = 0; j != ntroops; j++)
        if (canberemoralized (j))
          m++;

      n = min (n, m);

      sprintf (buf2, "; %d Verzweifelte %s ermutigt", n, 
	       (n != 1) ? "wurden" : "wurde");
      scat (buf2);

      while (--n >= 0)
        {
          do
            j = rand () % ntroops;
          while (!canberemoralized (j));

          ta[j].demoralized = 0;
        }

      break;

    case SP_LIGHTNING_BOLT:
      dozap (10);
      break;

    case SP_SHIELD:
      shields[attacker.side] += 1 + attacker.power;
      break;

    case SP_SUNFIRE:
      dozap (6250);
      break;

    default:
      assert (0);
    }

  /* achtung, bei spruechen, die abgebrochen werden, weil es keine Gegner
     gibt, erreichen das ende der funktion nicht, sondern machen ein return */

  scat (".");
  battlerecord (buf);
}

void
domonsters (int i)
{
  /* alles ia. progressiv - keine breaks! - also wyrme machen alle sprueche!
     nachher spruch wieder loeschen, sonst kaempft das monster weiter wie ein
     magier...  */

  switch (ta[i].unit->type)
    {
    case U_WYRM:

      ta[i].unit->combatspell = SP_SHIELD;
      docombatspell (i);

    case U_DRAGON:

      ta[i].unit->combatspell = SP_FIREBALL;
      docombatspell (i);

    case U_UNDEAD:

      ta[i].unit->combatspell = SP_CAUSE_FEAR;
      docombatspell (i);

      ta[i].unit->combatspell = -1;
    }
}

void
docatapult (int i)
{
  int n, d, di;

  /* n Leute werden beschossen (normalerweise 20), d Leute davon sterben.  */
  n = min (CATAPULT_HITS, left[defender.side]);
  d = 0;

  while (--n >= 0)
    {
      /* Select defender */
      di = selecttarget ();
      defender = ta[di];
      assert (defender.side == 1 - attacker.side);

      /* If attack succeeds */
      if (hits ())
        {
          d++;
          terminate (di);
        }
    }

  /* buf darf erst nach hits () verwendet werden, weil dort buf
     verwendet wird um battledebug () messages zu erzeugen.  */
  sprintf (buf, "%s feuert ein Katapult ab", unitid (ta[i].unit));
  if (d)
    {
      sprintf (buf2, "; %d Gegner %s", d, (d != 1) ? "starben" : "starb");
      scat (buf2);
    }
  scat (".");
  battlerecord (buf);
}

void
doshot (void)
{
  int ai, di;

  /* Select attacker.  */
  do
    ai = rand () % ntroops;
  while (ta[ai].attacked);

  attacker = ta[ai];
  ta[ai].attacked = 1;
  toattack[attacker.side]--;
  defender.side = 1 - attacker.side;

  ta[ai].dazzled = 0;

  if (attacker.unit)
    {
      /* Nur Bogenschuetzen koennen aus den hinteren Reihen angreifen.  */
      if (attacker.behind &&
          infront[attacker.side] &&
          !attacker.missile)
        return;

      if (attacker.shieldstone)
        shields[attacker.side] += 1 + attacker.power;

      if (attacker.unit->combatspell >= 0)
        {
          docombatspell (ai);
          return;
        }

      /* Bei Monstern kann es geschehen, dass sie durch ihre
         Faehigkeiten (Sprueche) alle Feinde toeten, so dass man hier
         nicht weiter gehen darf - sonst bleibt selecttarget ()
         haengen!  Der weitere code zielt nur noch darauf ab, ein
         weiteres target mit konventionellen Waffen zu toeten.  */
      if (attacker.unit->type)
        {
          domonsters (ai);
          if (!left[defender.side])
            return;
        }

      /* Wenn noch nachgeladen werden muss...  */
      if (attacker.reload)
        {
          ta[ai].reload--;
          return;
        }

      /* Falls Schusswaffen geladen sind, werden sie abgeschossen, und
         eine neue Nachladezeit bestimmt.  */
      if (attacker.weapon == I_CATAPULT)
        {
          docatapult (ai);
          ta[ai].reload = 5;
          return;
          /* Schuss mit docatapult () schon gemacht.  */
        }

      /* Langbogen werden nicht geladen.  */
      if (attacker.weapon == I_CROSSBOW)
        ta[ai].reload = 1;
      /* Aber dieser Schuss wird noch geschossen!  */
    }

  /* Select defender.  */
  di = selecttarget ();
  defender = ta[di];
  assert (defender.side == 1 - attacker.side);

  /* If attack succeeds.  */
  if (hits ())
    terminate (di);
}

int
ispresent (faction *f, region *r)
{
  unit *u;

  for (u = r->units; u; u = u->next)
    if (u->faction == f)
      return 1;

  return 0;
}

int
is_defender (unit *u, region *r, unit *attacker, unit *defender) 
{
  /* Einheit defender ist immer angegriffen, auch wenn sie nicht
     identifiziert ist.  */
  if (u == defender)
    return 1;

  /* Wenn es Wachen gibt, werden sie immer auf der Seite der Verteidiger
     kaempfen.  */
  if (u->type == U_GUARDS)
    return 1;

  /* Verteidiger im Kampf sind alle Einheiten welche der Partei des Opfers (Einheit defender) helfen, es sei
     denn, sie helfen gleichzeitig den Angreifern.  In dem Fall sind sie neutral.  Ob sie einer Partei helfen,
     sieht man in isallied ().  */
  if (isallied (u, defender) && !isallied (u, attacker))
    {
      /* Wenn der Status !ST_AVOID ist, kaempfen sie immer mit (es verteidigen so viele wie moeglich).  */
      if (u->status != ST_AVOID)
	return 1;
      else
	{
	  /* Wenn die Einheit (u) und das Opfer (defender) identifiziert und von der selben Partei sind, wird
             u mit angegriffen.  */
	  if (cansee (attacker->faction, r, defender) == 2 
	      && cansee (attacker->faction, r, u) == 2 
	      && defender->faction == u->faction)
	    return 1;

	  /* Allierte kaempfe-nicht Einheiten werden nicht in den Kampf gezogen, auch wenn sie identifiziert
             wurden, da der Angreifer wahrscheinlich nicht weiss, welche Parteien mit seinem Opfer alliert
             sind.  Genauso geht es, wenn defender nicht identifiziert oder andere Einheiten nicht
             identifiziert werden - sie werden nicht in den Kampf verwickelt.  */
	}
    }
  return 0;
}     

int reportcasualtiesdh;
void
reportcasualties (unit * u)
{
  if (!u->dead)
    return;

  if (!reportcasualtiesdh)
    {
      battlerecord ("");
      reportcasualtiesdh = 1;
    }

  if (u->dead == u->number)
    sprintf (buf, "%s wurde ausgeloescht.", unitid (u));
  else
    sprintf (buf, "%s verlor %d.", unitid (u), u->dead);
  battlerecord (buf);
}

void
combat (void)
{
  int i, j, k, n;
  int nfactions;
  int fno;
  int deadpeasants;
  int maxtactics[2];
  int leader[2];
  int lmoney;
  int dh;
  static litems[MAXITEMS];
  faction *f, *f2, **fa;
  region *r;
  building *b;
  unit *u, *u2, *u3, *u4;
  troop *t, *troops, **tp;
  strlist *S, *S2;

  puts ("- attackieren... ");

  nfactions = listlen (factions);
  fa = cmalloc (nfactions * sizeof (faction *));

  for (r = regions; r; r = r->next)
    {

      /* Create randomly sorted list of factions */

      for (f = factions, i = 0; f; f = f->next, i++)
        fa[i] = f;
      scramble (fa, nfactions, sizeof (faction *));

      /* ATTACKIERE: u - Angreifer, u2 - Opfer, u3 - Mitangreifer, u4 - die
         Einheiten, welche die Mitangreifer angreifen. 0 - defending, 1 -
         attacking */
      for (fno = 0; fno != nfactions; fno++)
        {
          f = fa[fno];
          for (u = r->units; u; u = u->next)
            if (u->faction == f)
              for (S = u->orders; S; S = S->next)
                if (igetkeyword (S->s) == K_ATTACK)
                  {
		    indicator_tick ();

                    /* Einheit u greift Einheit u2 an.  */
		    u2 = getunit (r, u);

                    if (!u2 && !getunitpeasants)
                      {
                        mistake2 (u, S, "Die Einheit wurde nicht gefunden");
                        continue;
                      }

                    if (u2 && u2->faction == f)
                      {
                        mistake2 (u, S, "Die Einheit ist eine der Unsrigen");
                        continue;
                      }

                    if (isallied (u, u2))
                      {
                        mistake2 (u, S, "Die Einheit ist mit uns alliert");
                        continue;
                      }

                    /* Draw up troops for the battle */
                    for (b = r->buildings; b; b = b->next)
                      b->sizeleft = b->size;
                    troops = 0;
                    tp = &troops;
                    left[0] = left[1] = 0;
                    infront[0] = infront[1] = 0;

                    /* If peasants are defenders */
                    if (!u2)
                      {
                        for (i = r->peasants; i; i--)
                          {
                            t = cmalloc (sizeof (troop));
                            memset (t, 0, sizeof (troop));
                            addlist2 (tp, t);
                          }
                        left[0] = r->peasants;
                        infront[0] = r->peasants;
                      }

                    /* ANGREIFENDE PARTEIEN WERDEN MARKIERT

                       u3 sind alle Einheiten, die den ATTACKIERE Befehl
                       gegeben haben.  Fuer sie ist der u->status
                       unwesentlich.

                       u3 ist ein Mitangreifer, falls u2 eine 0, also Bauern
                       sind, und u3 auch Bauern angreift, also u4 eine 0 ist,
                       oder falls u4 zur selben Partei wie das Opfer u2
                       gehoert, u3 also dieselbe Partei angreift, wie u.

                       u3 ist kein Mitangreifer, falls u3 mit u4 verbuendet
                       ist (der ATTACKIERE Befehl von u3 also ungueltig ist.)

                       Alle Parteien, die nun eine mitangreifende Einheit
                       haben, zaehlen unten als angreifer! */

		    /* Falls die Einheiten oder Parteien diese Runde und in dieser Region schon in Kaempfe
                       verwickelt worden sind, wurde u->attacking / f->attacking nicht geloescht...  */
                    for (f2 = factions; f2; f2 = f2->next)
                      f2->attacking = 0;
                    for (u3 = r->units; u3; u3 = u3->next)
                      u3->attacking = 0;

                    for (u3 = r->units; u3; u3 = u3->next)
                      for (S2 = u3->orders; S2; S2 = S2->next)
                        if (igetkeyword (S2->s) == K_ATTACK)
                          {
                            u4 = getunit (r, u3);

                            if ((getunitpeasants && !u2) ||
                                (u4 && u2 && u4->faction == u2->faction &&
                                 !isallied (u3, u4)))
                              {
                                u3->faction->attacking = 1;
                                u3->attacking = 1;
                                S2->s[0] = 0;
                                break;
                              }
                          }

                    /* ALLE EINHEITEN WERDEN ZU ANGREIFERN ODER VERTEIDIGERN
                       EINGETEILT

		       Wenn es Wachen gibt, werden sie immer auf der Seite der
		       Verteidiger kaempfen.

                       Zu den Angreifern gehoeren alle Einheiten u3 der oben
                       markierten Parteien. Einheiten, die nicht kaempfen,
                       kaempfen nur, wenn sie selber den ATTACKIERE Befehl
                       gegeben haben.

                       Zu den Verteidigern gehoeren alle Einheiten u3,
                       fuer die is_defender () den Wert 1 hat.  */
                    for (u3 = r->units; u3; u3 = u3->next)
                      {
                        u3->side = -1;

                        if (!u3->number)
                          continue;

                        if (u3->type != U_GUARDS 
			    && (u3->attacking 
				|| (u3->faction->attacking
				    && u3->status != ST_AVOID)))
                          {
                            u3->side = SI_ATTACKER;
			    assert (u3->type != U_ILLUSION);
                            tp = maketroops (tp, u3, mainterrain (r));
                          }
                        else if (is_defender (u3, r, u, u2))
                          {
			    u3->side = SI_DEFENDER;
			    tp = maketroops (tp, u3, mainterrain (r));
                          }
                      }

                    /* INITIALISIERUNG */
                    *tp = 0;

                    /* If only one side shows up, cancel */
                    if (!left[SI_DEFENDER] || !left[SI_ATTACKER])
                      {
                        freelist (troops);
                        continue;
                      }

                    /* Set up array of troops */
                    ntroops = listlen (troops);
                    ta = cmalloc (ntroops * sizeof (troop));
                    for (t = troops, i = 0; t; t = t->next, i++)
                      ta[i] = *t;
                    freelist (troops);
                    scramble (ta, ntroops, sizeof (troop));

                    initial[SI_DEFENDER] = left[SI_DEFENDER];
                    initial[SI_ATTACKER] = left[SI_ATTACKER];
                    shields[SI_DEFENDER] = 0;
                    shields[SI_ATTACKER] = 0;
                    runeswords[SI_DEFENDER] = 0;
                    runeswords[SI_ATTACKER] = 0;

                    /* set up loot array */
                    lmoney = 0;
                    memset (litems, 0, sizeof litems);

                    /* Initial attack message */
                    for (f2 = factions; f2; f2 = f2->next)
                      {
                        f2->seesbattle = ispresent (f2, r);
                        if (f2->seesbattle && f2->battles)
			  {
			    addstrlist (&f2->battles, "");
			    addstrlist (&f2->battles, "");
			    addstrlist (&f2->battles, "");
			  }
                      }
                    if (u2) 
                      strcpy (buf2, unitid (u2));
                    else /* if peasants */
                      strcpy (buf2, "die Bauern");
                    sprintf (buf, "%s attackiert %s in %s:", 
			     unitid (u), buf2, regionid (r));
                    battlerecord (buf);
                    battledebug (buf);

                    /* List sides */
                    battlerecord ("");
                    battlepunit (r, u);
                    for (u3 = r->units; u3; u3 = u3->next)
                      if (u3->side == SI_ATTACKER && u3 != u)
                        battlepunit (r, u3);
                    battlerecord ("");
                    if (u2)
                      battlepunit (r, u2);
                    else
                      {
                        sprintf (buf, "%d %s.", r->peasants, 
				 (r->peasants != 1) ? "Bauern" : "Bauer");
                        for (f2 = factions; f2; f2 = f2->next)
                          if (f2->seesbattle)
                            addstrlist (&f2->battles, buf);
                            /*sparagraph (&f2->battles, buf, 4, '-');*/
                      }
                    for (u3 = r->units; u3; u3 = u3->next)
                      if (u3->side == SI_DEFENDER && u3 != u2)
                        battlepunit (r, u3);
                    battlerecord ("");

                    /* Does one side have an advantage in tactics? */
                    maxtactics[SI_DEFENDER] = 0;
                    maxtactics[SI_ATTACKER] = 0;
                    for (i = 0; i != ntroops; i++)
                      if (ta[i].unit)
                        {
                          j = effskill (ta[i].unit, SK_TACTICS);

                          if (maxtactics[ta[i].side] < j)
                            {
                              leader[ta[i].side] = i;
                              maxtactics[ta[i].side] = j;
                            }
                        }
                    attacker.side = -1;
                    if (maxtactics[SI_DEFENDER] > maxtactics[SI_ATTACKER])
                      attacker.side = 0;
                    if (maxtactics[SI_ATTACKER] > maxtactics[SI_DEFENDER])
                      attacker.side = 1;

                    /* Better leader gets free round of attacks */
                    if (attacker.side >= 0)
                      {
                        /* Note the fact in the battle report */
                        if (attacker.side)
                          sprintf (buf, "%s hat eine freie Attacke!", 
				   unitid (u));
                        else if (u2)
                          sprintf (buf, "%s hat eine freie Attacke!", 
				   unitid (u2));
                        else
                          sprintf (buf, "Die Bauern haben eine "
				   "freie Attacke!");
                        battlerecord (buf);

                        /* Number of troops to attack */
                        toattack[attacker.side] = 0;
                        for (i = 0; i != ntroops; i++)
                          {
			    /* Mark troops as having attacked, unless
                               they are of the attackers side, in
                               which case they are allowed to attack.  */
                            ta[i].attacked = 1;
                            if (ta[i].side == attacker.side)
                              {
                                ta[i].attacked = 0;
                                toattack[attacker.side]++;
                              }
                          }

                        /* Do round of attacks.  */
                        do
                          doshot ();
                        while (toattack[attacker.side] && left[defender.side]);
                      }

                    /* Handle main body of battle.  Reset the counters
                       on the attacks yet to happen.  */
                    toattack[SI_DEFENDER] = 0;
                    toattack[SI_ATTACKER] = 0;
                    while (left[defender.side])
                      {

                        /* Ende einer Kampfrunde? */
                        if (toattack[SI_DEFENDER] == 0
                            && toattack[SI_ATTACKER] == 0)
                          {
                            /* debug */
                            sprintf (buf, "Angreifer: %d - Verteidiger: %d.",
                                     left[SI_ATTACKER],
                                     left[SI_DEFENDER]);
                            battledebug (buf);

                            /* markiere Angreifer */
                            for (i = 0; i != ntroops; i++)
                              {
				/* status == 0 verhindert Attacken!
                                   Alle Einheiten haben angegriffen,
                                   ausser die Einheiten, welche
                                   status!=0 haben.  */
                                ta[i].attacked = 1;
                                if (!ta[i].status)
                                  {
                                    ta[i].attacked = 0;
                                    toattack[ta[i].side]++;
                                  }
                              }
                          }

                        /* Naechster Kaempfer attackiert, solange bis
                           alle Einheiten angegriffen haben.  */
                        doshot ();
                      }

                    /* Report on winner */
                    if (attacker.side)
                      sprintf (buf, "%s gewinnt die Schlacht!", unitid (u));
                    else if (u2)
                      sprintf (buf, "%s gewinnt die Schlacht!", unitid (u2));
                    else
                      strcpy (buf, "Die Bauern gewinnen die Schlacht!");
                    battlerecord (buf);
                    battledebug ("");
                    battledebug (buf);
                    battledebug ("");

                    /* Can wounded be healed? */
                    n = 0;
                    for (i = 0; i != ntroops &&
                         n != initial[attacker.side] -
                         left[attacker.side]; i++)
                      if (!ta[i].status && ta[i].canheal)
                        {
                          k = lovar (50 * (1 + ta[i].power));
                          k = min (k, initial[attacker.side] -
                                   left[attacker.side] - n);

                          sprintf (buf, "%s heilt %d Verwundete.",
                                   unitid (ta[i].unit), k);
                          battlerecord (buf);

                          n += k;
                        }

                    while (--n >= 0)
                      {
                        do
                          i = rand () % ntroops;
                        while (!ta[i].status || ta[i].side != attacker.side);

                        ta[i].status = 0;
                      }

                    /* Count the casualties */
                    deadpeasants = 0;
                    for (u3 = r->units; u3; u3 = u3->next)
                      u3->dead = 0;

                    for (i = 0; i != ntroops; i++)
                      if (ta[i].unit)
                        ta[i].unit->dead += ta[i].status;
                      else
                        deadpeasants += ta[i].status;

                    /* Report the casualties */
                    reportcasualtiesdh = 0;
		    if (attacker.side)
                      {
                        reportcasualties (u);

                        for (u3 = r->units; u3; u3 = u3->next)
                          if (u3->side == 1 && u3 != u)
                            reportcasualties (u3);
                      }
                    else
                      {
                        if (u2)
                          reportcasualties (u2);
                        else if (deadpeasants)
                          {
                            battlerecord ("");
                            reportcasualtiesdh = 1;
                            sprintf (buf, "Es sterben %d Bauern.", deadpeasants);
                            battlerecord (buf);
                          }

                        for (u3 = r->units; u3; u3 = u3->next)
                          if (u3->side == 0 && u3 != u2)
                            reportcasualties (u3);
                      }

                    /* Dead peasants */
                    k = r->peasants - deadpeasants;
                    j = distribute (r->peasants, k, r->money);
                    lmoney += r->money - j;
                    r->money = j;
                    r->peasants = k;

                    /* Adjust units */
                    for (u3 = r->units; u3; u3 = u3->next)
                      {
                        k = u3->number - u3->dead;
                        /* Redistribute items and skills */
                        if (u3->side == defender.side)
                          {
                            j = distribute (u3->number, k, u3->money);
                            lmoney += u3->money - j;
                            u3->money = j;
			    for (i = 0; i != MAXITEMS; i++)
                              {
                                j = distribute (u3->number, k, u3->items[i]);
                                litems[i] += u3->items[i] - j;
                                u3->items[i] = j;
                              }
                          }
                        for (i = 0; i != MAXSKILLS; i++)
                          u3->skills[i] = distribute (u3->number, 
						      k, u3->skills[i]);
                        /* Adjust unit numbers */
                        u3->number = k;
                        /* Need this flag cleared for reporting of loot */
                        u3->n = 0;
                      }

                    /* Beute (loot) verteilen.  u3 ist eine temporaere Einheit, welche entweder die Einheit
                       einer zufaellig ausgewaehlten Person der Siegerseite ist, oder die Einheit, die von
                       dieser Partei in dieser Region zum Empfaenger der Beute bestimmt wurde (mittels SAMMEL
                       BEUTE wird u->collector bestimmt).  */
                    for (n = lmoney; n; n--)
                      {
                        do
                          j = rand () % ntroops;
                        while (ta[j].status || ta[j].side != attacker.side);
			
                        if (ta[j].unit)
                          {
			    u3 = ta[j].unit->collector ? ta[j].unit->collector : ta[j].unit;
			    u3->money++;
			    u3->n++;
                          }
                        else
                          r->money++;
                      }
                    for (i = 0; i != MAXITEMS; i++)
                      for (n = litems[i]; n; n--)
                        if (i <= I_STONE || rand () & 1)
                          {
                            do
                              j = rand () % ntroops;
                            while (ta[j].status 
				   || ta[j].side != attacker.side);
                            if (ta[j].unit)
                              {
				u3 = ta[j].unit->collector ? ta[j].unit->collector : ta[j].unit;
                                if (!u3->litems)
                                  {
                                    u3->litems = cmalloc (MAXITEMS * sizeof (int));
                                    memset (u3->litems, 0, MAXITEMS * sizeof (int));
                                  }
                                u3->items[i]++;
                                u3->litems[i]++;
                              }
                          }

                    /* Report loot */
                    for (f2 = factions; f2; f2 = f2->next)
                      f2->dh = 0;

                    for (u3 = r->units; u3; u3 = u3->next)
                      if (u3->n || u3->litems)
                        {
                          sprintf (buf, translate (ST_RECEIVES_LOOT, u3->faction->language, unitid (u3)));
                          dh = 0;

                          if (u3->n)
                            {
                              scat ("$");
                              icat (u3->n);
                              dh = 1;
                            }

                          if (u3->litems)
                            {
                              for (i = 0; i != MAXITEMS; i++)
                                if (u3->litems[i])
                                  {
                                    if (dh)
                                      scat (", ");
                                    dh = 1;

				    scat (translate (ST_QUANTITIY, u3->faction->language, u3->litems[i],
						     strings[itemnames[u3->litems[i] != 1][i]][u3->faction->language]));
                                  }

                              free (u3->litems);
                              u3->litems = 0;
                            }

                          if (!u3->faction->dh)
                            {
                              addbattle (u3->faction, "");
                              u3->faction->dh = 1;
                            }

                          scat (".");
                          addbattle (u3->faction, buf);
                        }
                    free (ta);
                  }
        }
    }
  free (fa);
  putchar ('\n'); /* nach dem Indikator */
}
