/* German Atlantis PB(E)M host Copyright (C) 1995-1998   Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "atlantis.h"
 
#define  C_REPORT_VERSION        6

#define  REPORTWIDTH            72
#define  MAILDELAY             120
#define  MAILCHECK_INTERVAL     10

/* set to "" if unwanted */

#define MAILCHECK "\nif waithost; then echo Mailing more...; else exit 1; fi\n\n"

/* ------------------------------------------------------------- */

char *
buildingtype (building * b)
{
  return buildingnames[buildingeffsize (b)];
}

/* ------------------------------------------------------------- */

char *
gamedate (faction *f)
{
  static char buf[40];
  static int monthnames[] =
  {
    ST_JANUARY,
    ST_FEBRUARY,
    ST_MARCH,
    ST_APRIL,
    ST_MAY,
    ST_JUNE,
    ST_JULY,
    ST_AUGUST,
    ST_SEPTEMBER,
    ST_OCTOBER,
    ST_NOVEMBER,
    ST_DECEMBER,
  };

  if (turn)
    strcpy (buf, translate (ST_DATE, f->language,
			    strings[monthnames[(turn - 1) % 12]][f->language],
			    ((turn - 1) / 12) + 1));
  else
    strcpy (buf, strings[ST_NO_TIME][f->language]);

  return buf;
}

/* ------------------------------------------------------------- */

void
sparagraph (strlist ** SP, char *s, int indent, int mark)
{

  /* Die Liste SP wird mit dem String s aufgefuellt, mit indent und
     einer mark, falls angegeben. SP wurde also auf 0 gesetzt vor dem
     Aufruf. */

  int i, j, width;
  int firstline;
  static char buf[128];

  width = REPORTWIDTH - indent;
  firstline = 1;

  for (;;)
    {
      i = 0;
      
      /* i zeigt auf das Ende der aktuellen Zeile.  j wird auf i gesetzt und um ein Wort verlaengert.  Falls das
	 gelingt, wird i auf j gesetzt.  Ist j breiter als der Report, wird abgebrochen.  Wird abgebrochen, obwohl
	 i immer noch 0 ist (dh. das erste Wort der Zeile ist laenger als die Zeile selber), wird i einfach auf die
	 maximale Breite gesetzt (und dieses erste Wort wird zwangs-getrennt).  */
      do
        {
          j = i;
          while (s[j] && s[j] != ' ')
            j++;
          if (j > width)
            {
              if (i == 0)
                i = width - 1;
              break;
            }
          i = j + 1;
        }
      while (s[j]);

      /* Einrueckung, markierung innerhalb der Einrueckung.  */
      for (j = 0; j != indent; j++)
        buf[j] = ' ';
      if (firstline && mark)
        buf[indent - 2] = mark;

      for (j = 0; j != i - 1; j++)
        buf[indent + j] = s[j];
      buf[indent + j] = 0;

      addstrlist (SP, buf);

      if (s[i - 1] == 0)
        break;

      s += i;
      firstline = 0;
    }
}

void
spskill (unit * u, int i, int *dh, int days)
{
  if (!u->skills[i])
    return;

  scat (", ");

  if (!*dh)
    {
      scat ("Talente: ");
      *dh = 1;
    }

  scat (skillnames[i]);
  scat (" ");
  icat (effskill (u, i));

  if (days)
    {
      assert (u->number);
      scat (" [");
      icat (u->skills[i] / u->number);
      scat ("]");
    }
}

char *
spunit (faction * f, region * r, unit * u, int battle)
{
  int i;
  int dh;

  strcpy (buf, u->faction == f ? "*" : "-");  /* Wird nachher gesondert verwendet als buf[0]!  */
  scat (unitid (u));                          /* Die wirkliche Beschreibung beginnt bei buf[1].  */

  /* Parteizugehoerigkeit nur bei cansee () */
  if (cansee (f, r, u) == 2)
    {
      scat (", ");
      scat (factionid (u->faction));
    }

  /* Fuer fremde Parteien erscheinen Illusionen als Menschen.  In Kaempfen werden die Illusionen allerdings
     aufgedeckt.  Aenderungen hier im Computer Report nicht vergessen!  */
  scat (translate (ST_QUANTITY_IN_LIST, f->language,
		   u->number,
		   strings[typenames[u->number != 1][u->type != U_ILLUSION ? u->type : 
						    ((!battle && u->faction != f) ? 
						     U_MAN : U_ILLUSION)]][f->language]));

  /* status */
  if (u->faction == f || battle)
    switch (u->status)
      {
      case ST_FIGHT:
        scat (", kampfbereit");
        break;

      case ST_BEHIND:
        scat (", kaempft hinten");
        break;

      case ST_AVOID:
        scat (", kaempft nicht");
        break;
      }

  if (u->guard)
    scat (", bewacht die Region");
  if (u->besieging)
    {
      scat (", belagert ");
      scat (buildingid (u->besieging));
    }
  if (u->faction == f && !battle && u->money)
    {
      scat (", $");
      icat (u->money);
    }

  dh = 0;
  if (u->faction == f)
    {
      for (i = 0; i != MAXSKILLS; i++)
	spskill (u, i, &dh, 1);
      switch (u->enchanted)
	{
	case SP_NIGHT_EYES:
	  scat (", ");
	  if (!dh)
	    scat ("hat ");
	  scat ("leuchtende Augen");
	  break;
	case SP_WATER_WALKING:
	  scat (", ");
	  if (!dh)
	    scat ("hat ");
	  scat ("einen besonders leichtfuessigen Gang");
	  break;
	}
    }

  dh = 0;
  for (i = 0; i != MAXITEMS; i++)
    if (u->items[i])
      {
        if (!dh)
          {
	    scat (strings[ST_HAS][f->language]);
	    scat (translate (ST_FIRST_QUANTITY_IN_LIST, f->language, u->items[i],
			     strings[itemnames[u->items[i] != 1][i]][f->language]));
            dh = 1;
          }
	else
	  scat (translate (ST_QUANTITY_IN_LIST, f->language, u->items[i],
			   strings[itemnames[u->items[i] != 1][i]][f->language]));
      }

  if (u->faction == f)
    {
      dh = 0;
      for (i = 0; i != MAXSPELLS; i++)
        if (!iscombatspell[i] && u->spells[i])
          {
            scat (", ");
            if (!dh)
              {
                scat (strings[ST_SPELLS][f->language]);
                dh = 1;
              }
            scat (strings[spellnames[i]][f->language]);
          }

      dh = 0;
      for (i = 0; i != MAXSPELLS; i++)
        if (iscombatspell[i] && u->spells[i])
          {
            scat (", ");
            if (!dh)
              {
                scat (strings[ST_COMBAT_SPELLS][f->language]);
                dh = 1;
              }
            scat (strings[spellnames[i]][f->language]);
          }

      if (u->combatspell >= 0)
        {
	  assert (dh);
          scat (", ");
	  scat (strings[ST_SET][f->language]);
          scat (strings[spellnames[u->combatspell]][f->language]);
        }

      if (!battle)
	{
	  if (u->lastorder || u->thisorder2)
	    scat (", Default: ");
	  if (u->lastorder)
	    {
	      scat ("\"");
	      scat (u->lastorder);
	      scat ("\"");
	    }
	  if (u->lastorder && u->thisorder2)
	    scat (" und ");
	  if (u->thisorder2)
	    {
	      scat ("\"");
	      scat (u->thisorder2);
	      scat ("\"");
	    }
	}
    }
  
  i = 0;

  if (u->display)
    {
      scat ("; ");
      scat (u->display);

      i = u->display[strlen (u->display) - 1];
    }

  if (i != '!' && i != '?' && i != '.')
    scat (".");

  return buf;
}

/* ------------------------------------------------------------- */

int outi;
char outbuf[1500];

void
rpc (int c)
{
  outbuf[outi++] = c;
  assert (outi < (int) sizeof outbuf);
}

void
rnl (void)
{
  int i;
  int rc, vc;

  i = outi;
  while (i && isspace (outbuf[i - 1]))
    i--;
  outbuf[i] = 0;

  i = 0;
  rc = 0;
  vc = 0;

  while (outbuf[i])
    {
      switch (outbuf[i])
        {
        case ' ':
          vc++;
          break;

        case '\t':
          vc = (vc & ~7) + 8;
          break;

        default:
          while (rc / 8 != vc / 8)
            {
              if ((rc & 7) == 7)
                fputc (' ', F);
              else
                fputc ('\t', F);
              rc = (rc & ~7) + 8;
            }

          while (rc != vc)
            {
              fputc (' ', F);
              rc++;
            }

          fputc (outbuf[i], F);
          rc++;
          vc++;
        }

      i++;
    }

  fputc ('\n', F);
  outi = 0;
}

void
rps (char *s)
{
  rpc (' ');                    /* neuer rand */
  while (*s)
    rpc (*s++);
}

void
rpstrlist (strlist * S)
{
  while (S)
    {
      rps (S->s);
      rnl ();
      S = S->next;
    }
}

void
crpstrlist (char *title, strlist * S)
{
  fprintf (F, "%s\n", title);
  while (S)
    {
      fprintf (F, "\"%s\"\n", S->s);
      S = S->next;
    }
}

void
rparagraph (char *s, int indent, int mark)
{
  strlist *S;

  S = 0;
  sparagraph (&S, s, indent, mark);
  rpstrlist (S);
  freelist (S);
}

void
rpunit (faction * f, region * r, unit * u, int indent)
{
  strlist *S;

  rnl ();                       /* neue leerzeile */

  S = 0;
  spunit (f, r, u, 0);
  sparagraph (&S, buf + 1, indent, buf[0]);
  rpstrlist (S);
  freelist (S);
}

void
center (char *s)
{
  int i;

  /* Bei Namen die genau 80 Zeichen lang sind, kann es hier Probleme geben.
     Seltsamerweise wird i dann auf MAXINT oder aehnlich initialisiert.
     Deswegen keine Strings die laenger als REPORTWIDTH sind! */

  assert (REPORTWIDTH >= strlen (s));

  for (i = (REPORTWIDTH - strlen (s)) / 2; i; i--)
    rpc (' ');
  rps (s);
  rnl ();
}

void
untitled_strlist (strlist * S)
{
  strlist *T;

  while (S)
    {
      T = 0;
      sparagraph (&T, S->s, 0, 0);
      rpstrlist (T);
      freelist (T);
      S = S->next;
    }
}

void
titled_strlist (char *s, strlist * S)
{
  if (S)
    {
      rnl ();
      rps (s);
      rnl ();
      rnl ();
      untitled_strlist (S);
    }
}

void
centred_title_strlist (char *s, strlist * S)
{
  strlist *T;

  if (S)
    {
      rnl ();
      center (s);
      rnl ();

      while (S)
        {
          T = 0;
          if (S->s[0] == '*' || S->s[0] == '-')
            /* Eine Einheit in f->battles, muss noch formatiert werden. */
            sparagraph (&T, S->s + 1, 4, S->s[0]);
          else
            sparagraph (&T, S->s, 0, 0);
          rpstrlist (T);
          freelist (T);
          S = S->next;
        }
    }
}

void
centred_paragraph (char *s)
{
  strlist *S;

  S = 0;
  sparagraph (&S, s, 0, 0);
  while (S)
    {
      center (S->s);
      S = S->next;
    }
  freelist (S);
}

/* ------------------------------------------------------------- */

void
prices (region *r, faction *f)
{
  int i;

  /* Beginne Paragraphen */
  if (r->buildings)
    strcpy (buf, strings[ST_CURRENT_PRICES][f->language]);
  else
    strcpy (buf, strings[ST_PRICES_WITHOUT_BUILDINGS][f->language]);
  for (i = 0; i != MAXLUXURIES; i++)
    {
      if (i==0)
	scat (translate (ST_FIRST_PRICE, f->language, strings[itemnames[1][FIRSTLUXURY + i]][f->language],
			 itemprice[i] * r->demand[i] / 100));
      else if (i == MAXLUXURIES - 1)
	scat (translate (ST_LAST_PRICE, f->language, strings[itemnames[1][FIRSTLUXURY + i]][f->language],
			 itemprice[i] * r->demand[i] / 100));
      else 
	scat (translate (ST_PRICE, f->language, strings[itemnames[1][FIRSTLUXURY + i]][f->language],
			 itemprice[i] * r->demand[i] / 100));
    }
  scat (translate (ST_PEASANTS_MAKE, f->language,
		   strings[itemnames[1][FIRSTLUXURY + r->produced_good]][f->language]));

  /* Schreibe Paragraphen */
  rparagraph (buf, 0, 0);
}

/* ------------------------------------------------------------- */

int
roads (region * r)
{
  return (r->road && r->road >= roadreq[r->terrain]);
}

int
roadto (region * r, region * r2)
{
  /* wenn es hier genug strassen gibt, und verbunden ist, und es dort genug
     strassen gibt, dann existiert eine strasse in diese richtung */

  int i;

  if (!r || !r2 || !roads (r) || !roads (r2))
    return 0;

  for (i = 0; i != MAXDIRECTIONS; i++)
    if (r->connect[i] == r2)
      return 1;

  return 0;
}

/* ------------------------------------------------------------- */

void
describe (region *r, faction *f)
{
  int dh, n, d, i;

  /* Name */

  strcpy (buf, regionid (r));

  /* Terrain */

  scat (", ");
  scat (strings[terrainnames[mainterrain (r)]][f->language]);

  /* Baeume */

  if (r->trees)
    {
      scat (", ");
      scat (translate (r->trees != 1 ? ST_TREES : ST_TREE, f->language, r->trees));
    }

  /* Bauern & Geld */

  if (r->peasants)
    {
      scat (", ");
      scat (translate (r->peasants != 1 ? ST_PEASANTS : ST_PEASANT, f->language, r->peasants));

      if (r->money)
        {
          scat (", $");
          icat (r->money);
        }
    }

  /* Pferde */
  if (r->horses)
      scat (translate (ST_QUANTITY_IN_LIST, f->language, r->horses,
		       strings[itemnames[r->horses > 1][I_HORSE]][f->language]));

  scat (".");

  /* Strassen */

  n = 0;

  if (!roads (r))
    {
      /* Strassen nicht vollstaendig */

      if (r->road)
        {
          scat (" Das Strassennetz ist zu ");
          icat (100 * r->road / roadreq[r->terrain]);
          scat ("% vollendet.");
        }

      /* ohne Strassen, keine besondere Meldung */
    }
  else
    {
      /* Strassenverbindungen zaehlen */

      for (d = 0; d != MAXDIRECTIONS; d++)
        if (roadto (r, r->connect[d]))
          n++;

      /* Der Satz beginnt */

      if (!n)
        {
          /* Strassen ohne weitere Verbindungen */

          scat (" Ein Strassennetz ueberzieht die Region.");
        }
      else
        {
          /* Strassen mit weiteren Verbindungen */

          if (n == 1)
            scat (" Eine Strasse fuehrt");
          else
            scat (" Strassen fuehren");

          dh = 0;
          i = 0;
          for (d = 0; d != MAXDIRECTIONS; d++)
            if (roadto (r, r->connect[d]))
              {
                /* "und" vor dem letzten, aber nicht beim ersten */

                i++;
                if (dh)
                  {
                    if (i == n)
                      scat (" und");
                    else
                      scat (",");
                  }
                dh = 1;

                scat (" nach ");
                scat (directions[d]);
                scat (" ");
                scat (roadinto[mainterrain (r->connect[d])]);
                scat (" ");
                scat (regionid (r->connect[d]));
              }
          scat (".");
        }
    }

  /* Richtungen ohne Strassen zaehlen */

  n = 0;
  for (d = 0; d != MAXDIRECTIONS; d++)
    if (!roadto (r, r->connect[d]))
      n++;

  if (n)
    {
      scat (" Im");

      dh = 0;
      i = 0;
      for (d = 0; d != MAXDIRECTIONS; d++)
        if (!roadto (r, r->connect[d]))
          {
            /* "und" vor dem letzten, aber nicht beim ersten */

            i++;
            if (dh)
              {
                if (i == n)
                  scat (" und im");
                else
                  scat (", im");
              }

            scat (" ");
            scat (directions[d]);
            scat (" ");
            if (!dh)
	      {
		/* Fallunterscheidung bei "Berge", falls die erste Region "Berge" heisst. */
		if (mainterrain (r->connect[d]) == T_MOUNTAIN)
		  scat ("liegen ");
		else
		  scat ("liegt ");
	      }
            scat (trailinto[mainterrain (r->connect[d])]);
            scat (" ");
            scat (regionid (r->connect[d]));

            dh = 1;
          }
      scat (".");
    }

  /* Beschreibung */

  if (r->display)
    {
      scat (" ");
      scat (r->display);

      n = r->display[strlen (r->display) - 1];
      if (n != '!' && n != '?' && n != '.')
        scat (".");
    }

  /* Schreibe Paragraphen */

  rparagraph (buf, 0, 0);

  /* Kommentare, zB. Effekte von Zauberspruechen wie "Nebelnetze" */

  if (r->comments)
    untitled_strlist (r->comments);

}

void
guards (region *r)
{
  faction *f;
  unit *u;
  int i, n;

  /* Bewachung */

  for (u = r->units; u; u = u->next)
    if (u->guard)
      break;
  if (!u)
    return;

  for (f = factions; f; f = f->next)
    f->dh = 0;

  n = 0;
  for (u = r->units; u; u = u->next)
    if (u->guard)
      {
        u->faction->dh = 1;
        n++;
      }

  strcpy (buf, "Die Region wird von ");

  i = 0;
  n = 0;
  for (f = factions; f; f = f->next)
    if (f->dh)
    {
      i++;
      if (n)
        {
          if (i == n)
            scat (" und ");
          else
            scat (", ");
        }
      n = 1;
      scat (factionid (f));
    }
  scat (" bewacht.");
  rnl ();
  rparagraph (buf, 0, 0);
}

void
statistics (region * r, faction * f)
{
  unit *u;
  building *b;
  strlist *S;
  int i, number, money, maxwork, rmoney, items[MAXITEMS], wage;

  S = 0;

  /* Arbeiten.  */
  if (production[r->terrain])
    {
      wage = WAGE;
      b = largestbuilding (r);
      if (b)
        wage += buildingeffsize (b) * BONUS;
      sprintf (buf, "Lohn fuer einen Monat Arbeiten: $%d", wage);
      addstrlist (&S, buf);

      maxwork = (production[r->terrain] - r->trees) * MAXPEASANTS_PER_AREA;
      sprintf (buf, "Arbeitsplaetze in der Region diesen Monat: max. %d", maxwork);
      addstrlist (&S, buf);
      
      rmoney = min (r->peasants, (production[r->terrain] - r->trees) * MAXPEASANTS_PER_AREA) * wage;
      sprintf (buf, "Erwarteter Lohn der arbeitenden Bauern: ca. $%d", rmoney);
      addstrlist (&S, buf);

      rmoney += r->money;
      sprintf (buf, "Davon Geld fuer Unterhaltung: max. $%d", rmoney / ENTERTAINFRACTION);
      addstrlist (&S, buf);

      rmoney -= rmoney / ENTERTAINFRACTION + r->peasants * MAINTENANCE;
      rmoney = max (rmoney, 0);

      sprintf (buf, "Steuern fuer eine stabile Population bei max. Unterhaltung: $%d", rmoney);
      addstrlist (&S, buf);

      sprintf (buf, "Kaufkraft der Bauern bei max. Unterhaltung: $%d", rmoney);
      addstrlist (&S, buf);

      sprintf (buf, "Anzahl Luxusgueter zu kaufen bei stabilen Preisen: %d",
		  DEMANDFALL * r->peasants / DEMANDFACTOR);
      /* 6x mehr als beim Verkauf */
      addstrlist (&S, buf);

      sprintf (buf, "Anzahl Luxusgueter zu verkaufen bei stabilen Preisen: %d",
		  DEMANDRISE * r->peasants / DEMANDFACTOR);
      /* Bei 10000 Bauern ist das 25 * 10000 / 2500 = 100.  */
      addstrlist (&S, buf);

      sprintf (buf, "Anzahl rekrutierbarer Bauern: max. %d",
               r->peasants / RECRUITFRACTION);
      addstrlist (&S, buf);
    }

  number = 0;
  money = 0;
  memset (items, 0, sizeof items);

  for (u = r->units; u; u = u->next)
    if (u->faction == f)
      {
        number += u->number;
        money += u->money;
        for (i = 0; i != MAXITEMS; i++)
          items[i] += u->items[i];
      }
  sprintf (buf, "Personen: %d", number);
  addstrlist (&S, buf);
  sprintf (buf, "Silber: %d", money);
  addstrlist (&S, buf);

  for (i = 0; i != MAXITEMS; i++)
    if (items[i])
      {
        sprintf (buf, "%s: %d", strings[itemnames[1][i]][f->language], items[i]);
        addstrlist (&S, buf);
      }

  /* Ausgabe */
  titled_strlist ("Statistik", S);
  freelist (S);
}

void 
template_unit (strlist *S, unit *u)
{
  /* In order_template () verwendet man *S, und damit das dortige *S
     manipuliert werden kann, verwenden wir hier also **S.  */

  /* Einheit mit Name.  */
  sprintf (buf, "%s %d;\t\t%s [%d, $%d]", parameters[P_UNIT],
	   u->no, u->name, u->number, u->money);
  addstrlist (&S, buf);
  
  /* Default Befehl.  */
  if (u->lastorder)
    {
      sprintf (buf, "   %s", u->lastorder);
      addstrlist (&S, buf);
    }
  if (u->thisorder2)
    {
      sprintf (buf, "   %s", u->thisorder2);
      addstrlist (&S, buf);
    }
}

void
order_template (faction *f)
{
  strlist *S;
  region *r;
  building *b;
  ship *sh;
  unit *u;

  S = 0;
  addstrlist (&S, "");
  addstrlist (&S, "");
  addstrlist (&S, "Vorlage fuer den naechsten Zug:");
  addstrlist (&S, "");

  sprintf (buf, "%s %d \"%s\"", parameters[P_FACTION], f->no, 
	   f->passw ? f->passw : "");
  addstrlist (&S, buf);

  for (r = regions; r; r = r->next)
    {
      /* Nur falls es units gibt.  */

      for (u = r->units; u; u = u->next)
        if (u->faction == f)
          break;
      if (!u)
        continue;

      /* Region */
      addstrlist (&S, "");
      addstrlist (&S, "");
      sprintf (buf, "; %s", regionid (r));
      addstrlist (&S, buf);

      /* Einheiten in Burgen */
      for (b = r->buildings; b; b = b->next)
        {
	  addstrlist (&S, "");
	  sprintf (buf, "; BURG %d\t\t%s", b->no, b->name);
	  addstrlist (&S, buf);
          for (u = r->units; u; u = u->next)
            if (u->faction == f && u->building == b && u->owner)
	      template_unit (S, u);

          for (u = r->units; u; u = u->next)
            if (u->faction == f && u->building == b && !u->owner)
              template_unit (S, u);
        }

      /* Einheiten in Schiffen */
      for (sh = r->ships; sh; sh = sh->next)
        {
	  addstrlist (&S, "");
	  sprintf (buf, "; SCHIFF %d\t\t%s", sh->no, sh->name);
	  addstrlist (&S, buf);
          for (u = r->units; u; u = u->next)
            if (u->faction == f && u->ship == sh && u->owner)
              template_unit (S, u);

          for (u = r->units; u; u = u->next)
            if (u->faction == f && u->ship == sh && !u->owner)
              template_unit (S, u);
        }

      /* Restliche Einheiten */
      addstrlist (&S, "");
      for (u = r->units; u; u = u->next)
        if (u->faction == f && !u->building && !u->ship && cansee (f, r, u))
	  template_unit (S, u);
    }

  /* Zum Schluss: NAECHSTER.  */
  addstrlist (&S, "");
  sprintf (buf, parameters[P_NEXT]);
  addstrlist (&S, buf);
  
  rpstrlist (S);
  freelist (S);
}

void
allies (faction * f)
{
  int dh, m, i;
  rfaction *rf;

  if (f->allies)
    {
      m = 0;
      for (rf = f->allies; rf; rf = rf->next)
        m++;

      dh = 0;
      strcpy (buf, "Wir helfen folgenden Parteien: ");

      i = 0;
      for (rf = f->allies; rf; rf = rf->next)
        {
          i++;
          if (dh)
            {
              if (i == m)
                scat (" und ");
              else
                scat (", ");
            }
          dh = 1;
          scat (factionid (rf->faction));
        }

      scat (".");
      rnl ();
      rparagraph (buf, 0, 0);
    }
}

void
report_computer (faction * f)
{
  int i, visible, d, wage, rmoney;
  region *r;
  rfaction *rf;
  building *b;
  ship *sh;
  unit *u;
  /* Wurde fuer diese Variable ein Kommentar gezeigt? */
  int cr=0, cu=0, cuu=0, cb=0, cs=0, csp=0;

  /* Computer Report Kommentare sind nicht sprachunabhaengig! */

  printf ("- Computer Report fuer %s...\n", factionid (f));
  fprintf (F, "VERSION %d; Version des Computer Reports\n", C_REPORT_VERSION);
  fprintf (F, "PARTEI %d\n", f->no);
  fprintf (F, "%d; Runde\n", turn);
  fprintf (F, "\"%s\"; ZAT\n", zat);
  fprintf (F, "\"%s\"; Passwort\n", f->passw ? f->passw : "");

  crpstrlist ("FEHLER", f->mistakes);
  crpstrlist ("WARNUNGEN", f->warnings);
  crpstrlist ("MELDUNGEN", f->messages);
  crpstrlist ("KAEMPFE", f->battles);
  crpstrlist ("EREIGNISSE", f->events);
  crpstrlist ("EINKOMMEN", f->income);
  crpstrlist ("HANDEL", f->commerce);
  crpstrlist ("PRODUKTION", f->production);
  crpstrlist ("BEWEGUNGEN", f->movement);

  fputs ("ZAUBER\n", F);
  for (i = 0; i != MAXSPELLS; i++)
    if (f->showdata[i])
      {
	fprintf (F, "\"%s\"%s\n", strings[spellnames[i]][f->language], !csp ? "; Spruch" : "");
	fprintf (F, "%d%s\n", spelllevel[i], !csp ? "; Stufe" : "");
	fprintf (F, "\"%s\"%s\n", strings[spelldata[i]][f->language], !csp ? "; Beschreibung" : "");
	csp = 1;
      }

  fputs ("ALLIIERTE\n", F);
  for (i = 0, rf = f->allies; rf; rf = rf->next, i = 1)
    {
      fprintf (F, "%d%s\n", rf->faction->no, !i ? "; Partei" : "");
      fprintf (F, "\"%s\"%s\n", rf->faction->name ? rf->faction->name : "", !i ? "; Parteiname" : "");
    }

  for (r = regions; r; r = r->next)
    {
      for (u = r->units; u; u = u->next)
        if (u->faction == f)
          break;
      if (!u)
        continue;

      fprintf (F, "REGION %d %d\n", r->x, r->y);
      fprintf (F, "\"%s\"%s\n", r->name ? r->name : "", !cr ? "; Name" : "");
      fprintf (F, "\"%s\"%s\n", r->display ? r->display : "", !cr ? "; Beschreibung" : "");
      fprintf (F, "\"%s\"%s\n", strings[terrainnames[mainterrain (r)]][f->language], !cr ? "; Terrain" : "");
      fprintf (F, "%d%s\n", r->trees, !cr ? "; Baeume" : "");
      fprintf (F, "%d%s\n", r->peasants, !cr ? "; Bauern" : "");
      fprintf (F, "%d%s\n", r->money, !cr ? "; Silber" : "");
      fprintf (F, "%d%s\n", r->horses, !cr ? "; Pferde" : "");
      fprintf (F, "%d%s\n", 
	       roadreq[r->terrain] 
	       ? 100 * r->road / roadreq[r->terrain] : 0,
               !cr ? "; Prozent der Strasse" : "");

      /* Statistik */

      wage = 0;
      if (production[r->terrain])
        {
          wage = WAGE;
          b = largestbuilding (r);
          if (b)
            wage += buildingeffsize (b) * BONUS;
        }
      fprintf (F, "%d%s\n", wage, 
	       !cr ? "; Lohn fuer einen Monat Arbeiten" : "");
      rmoney = min (r->peasants, (production[r->terrain] - r->trees) * MAXPEASANTS_PER_AREA) * wage;
      fprintf (F, "%d%s\n", rmoney, 
	       !cr ? "; erwarteter Lohn der arbeitenden Bauern" : "");
      rmoney += r->money;
      fprintf (F, "%d%s\n", rmoney / ENTERTAINFRACTION, 
	       !cr ? "; davon max. fuer Unterhaltung" : "");
      rmoney -= rmoney / ENTERTAINFRACTION + r->peasants * MAINTENANCE;
      rmoney = max (rmoney, 0);
      fprintf (F, "%d%s\n", rmoney, 
	       !cr ? "; Steuern fuer eine stabile Population bei max. Unterhaltung" : "");
      fprintf (F, "%d%s\n", rmoney, 
	       !cr ? "; Kaufkraft der Bauern bei max. Unterhaltung" : "");
      /* DEMANDFALL / DEMANDRISE = 6! */
      if (!cr)
	fprintf (F, "%d; Anzahl Luxusgueter zu verkaufen bei stabilen Preisen (kaufen: %dx mehr)\n",
		 DEMANDRISE * r->peasants / DEMANDFACTOR,
		 DEMANDFALL / DEMANDRISE);
      else
	fprintf (F, "%d\n", DEMANDRISE * r->peasants / DEMANDFACTOR);

      fprintf (F, "%d%s\n", r->peasants / RECRUITFRACTION, !cr ? "; Rekrutieren" : "");
      
      for (d = 0; d != MAXDIRECTIONS; d++)
        {
          fprintf (F, "\"%s\"%s%s\n",
                   strings[terrainnames[mainterrain (r->connect[d])]][f->language],
                   !cr ? "; " : "",
                   !cr ? directions[d] : "");
          fprintf (F, "\"%s\"%s\n",
                   regionid (r->connect[d]),
                   !cr ? "; Name der Region" : "");
        }

      fputs ("PREISE\n", F);
      for (i = 0; i != MAXLUXURIES; i++)
        fprintf (F, "%d%s%s\n",
                 r->terrain != T_OCEAN
                 ? (itemprice[i] * r->demand[i] / 100
		    * (i == r->produced_good ? -1 : 1))
                 : 0,
                 (!cr 
		  ? (i == r->produced_good
		     ? "; Preis (wird produziert, daher negativ) "
		     : "; Preis")
		  : ""),
                 !cr ? strings[itemnames[1][FIRSTLUXURY + i]][0] : "");

      for (b = r->buildings; b; b = b->next)
        {
          fprintf (F, "BURG %d\n", b->no);
          fprintf (F, "\"%s\"%s\n", b->name ? b->name : "",
                   !cb ? "; Name" : "");
          fprintf (F, "\"%s\"%s\n", b->display ? b->display : "",
                   !cb ? "; Beschreibung" : "");
          fprintf (F, "%d%s\n", b->size,
                   !cb ? "; Groesse" : "");
          u = buildingowner (r, b);
          fprintf (F, "%d%s\n", u ? u->no : -1,
                   !cb ? "; Burgherr (-1 = niemand)" : "");
          fprintf (F, "%d%s\n", u && cansee (f, r, u) == 2
                   ? u->faction->no
                   : -1,
                   !cb ? "; Partei des Burgherren (-1 = unbekannt)" : "");
          cb = 1;
        }

      for (sh = r->ships; sh; sh = sh->next)
        {
          fprintf (F, "SCHIFF %d\n", sh->no);
          fprintf (F, "\"%s\"%s\n", sh->name ? sh->name : "",
                   !cs ? "; Name" : "");
          fprintf (F, "\"%s\"%s\n", sh->display ? sh->display : "",
                   !cs ? "; Beschreibung" : "");
          fprintf (F, "\"%s\"%s\n", shiptypes[0][sh->type],
                   !cs ? "; Typ" : "");
          fprintf (F, "%d%s\n",
                   100 * (shipcost[sh->type] - sh->left) / shipcost[sh->type],
                   !cs ? "; Prozent Vollendet" : "");
          u = shipowner (r, sh);
          fprintf (F, "%d%s\n", u ? u->no : -1,
                   !cs ? "; Kapitaen (-1 = niemand)" : "");
          fprintf (F, "%d%s\n", u && cansee (f, r, u) == 2
                   ? u->faction->no
                   : -1,
                   !cs ? "; Partei des Kapitaens (-1 = unbekannt)" : "");
          cs = 1;
        }

      for (u = r->units; u; u = u->next)
        if ((visible = cansee (f, r, u)))
          {
            fprintf (F, "EINHEIT %d\n", u->no);

            fprintf (F, "\"%s\"%s\n", u->name ? u->name : "",
                     !cu ? "; Name" : "");
            fprintf (F, "\"%s\"%s\n", u->display ? u->display : "",
                     !cu ? "; Beschreibung" : "");
            fprintf (F, "%d%s\n", visible == 2 ? u->faction->no : -1,
                     !cu ? "; Partei (-1 = unbekannt)" : "");
            fprintf (F, "\"%s\"%s\n",
                     visible == 2 
		     ? (u->faction->name ? u->faction->name : "")
		     : "",
                     !cu ? "; Parteiname" : "");
            fprintf (F, "%d%s\n", u->number,
                     !cu ? "; Anzahl" : "");
            fprintf (F, "\"%s\"%s\n", u->type == U_ILLUSION && u->faction != f 
		     ? strings[typenames[1][U_MAN]][f->language] 
		     : strings[typenames[1][u->type]][f->language],
                     !cu ? "; Typ" : "");
            fprintf (F, "%d%s\n",
                     u->building ? u->building->no : 0,
                     !cu ? "; Burg Nr." : "");
            fprintf (F, "%d%s\n",
                     u->ship ? u->ship->no : 0,
                     !cu ? "; Schiff Nr." : "");
            fprintf (F, "%d%s\n", u->guard,
                     !cu ? "; bewacht Region" : "");

            if (u->faction == f)
              {
                fprintf (F, "%d%s\n", u->money,
                         !cuu ? "; Silber" : "");
                fprintf (F, "%d%s\n", u->status,
                         !cuu ? "; kaempft (0-ja, 1-hinten, 2-nein)" : "");
                fprintf (F, "\"%s\"%s\n", u->lastorder ? u->lastorder : "",
                         !cuu ? "; Default" : "");
                fputs ("TALENTE\n", F);
                for (d = 0; d != MAXSKILLS; d++)
                  fprintf (F, "%d %d%s%s\n", u->skills[d], effskill (u, d),
                           !cuu ? "; " : "",
                           !cuu ? skillnames[d] : "");
                fputs ("SPRUECHE\n", F);
		csp = 0;
                for (d = 0; d != MAXSPELLS; d++)
                  if (u->spells[d])
		    {
		      fprintf (F, "\"%s\"%s\n", strings[spellnames[d]][f->language],
			       !csp ? "; Spruch" : "");
		      csp = 1;
		    }
                cuu = 1;
              }

            fputs ("GEGENSTAENDE\n", F);
            for (d = 0; d != LASTLUXURY; d++)
              fprintf (F, "%d%s%s\n", u->items[d],
                       !cu ? "; " : "",
                       !cu ? strings[itemnames[0][d]][0] : "");

	    /*  Magische Gegenstaende werden nur angezeigt, falls man diese auch wirklich besitzt.  */
            fputs ("MAGISCHES\n", F);
            for (d = FIRST_MAGIC_ITEM; d != LAST_MAGIC_ITEM; d++)
	      if (u->items[d])
		fprintf (F, "%d \"%s\"%s\n", u->items[d], strings[itemnames[0][d]][f->language],
			 !cu ? "; magischer Gegenstand" : "");
            cu = 1;
          }
      cr = 1;
    }
}

void
report (faction * f)
{
  int i;
  int anyunits;
  region *r;
  building *b;
  ship *sh;
  unit *u;
  char wants_stats;
  char wants_debug;

  i = pow (2, O_STATISTICS);
  wants_stats = (f->options & i);
  i = pow (2, O_DEBUG);
  wants_debug = (f->options & i);

  printf ("- Report fuer %s...\n", factionid (f));

  center (strings[ST_ATLANTIS_REPORT][f->language]);
  centred_paragraph (factionid (f));
  center (gamedate (f));
  strcpy (buf, translate (ST_ZAT, f->language, zat));
  center (buf);

  centred_title_strlist (strings[ST_MISTAKES][f->language], f->mistakes);
  centred_title_strlist (strings[ST_WARNINGS][f->language], f->warnings);
  centred_title_strlist (strings[ST_MESSAGES][f->language], f->messages);
  centred_title_strlist (strings[ST_BATTLES][f->language], f->battles);

  if (wants_debug) /* Debug ist nicht sprachunabhaengig! */
    {
      centred_title_strlist ("Debug", f->debug);
      printf ("  %s hat DEBUG gesetzt!\n", factionid (f));
    }

  centred_title_strlist (strings[ST_MISCELLANEOUS][f->language], f->events);
  centred_title_strlist (strings[ST_INCOME][f->language], f->income);
  centred_title_strlist (strings[ST_COMMERCE][f->language], f->commerce);
  centred_title_strlist (strings[ST_PRODUCTION][f->language], f->production);
  centred_title_strlist (strings[ST_MOVEMENTS][f->language], f->movement);

  for (i = 0; i != MAXSPELLS; i++)
    if (f->showdata[i])
      break;

  if (i != MAXSPELLS)
    {
      rnl ();
      center (strings[ST_NEW_SPELLS][f->language]);

      for (i = 0; i != MAXSPELLS; i++)
        if (f->showdata[i])
          {
            rnl ();
            center (strings[spellnames[i]][f->language]);
            strcpy (buf, translate (ST_LEVEL, f->language, spelllevel[i]));
            center (buf);
            rnl ();

            rparagraph (strings[spelldata[i]][f->language], 0, 0);
          }
    }

  rnl ();
  center (strings[ST_STATE_OF_AFFAIRS][f->language]);

  allies (f);

  anyunits = 0;

  for (r = regions; r; r = r->next)
    {
      /* nur report, falls es units gibt */

      for (u = r->units; u; u = u->next)
        if (u->faction == f)
          break;
      if (!u)
        continue;

      anyunits = 1;

      /* Debug */

      if (wants_debug && r->debug)
	{
	  rnl ();                   /* leerzeile */
	  center ("------------------------------------------------------------------------");
	  titled_strlist ("Debug", r->debug);
	}

      /* Beschreibung */

      rnl ();                   /* leerzeile */
      center ("------------------------------------------------------------------------");
      rnl ();                   /* leerzeile */
      describe (r, f);

      /* Preise nur auf dem Festland */

      if (r->terrain != T_OCEAN)
        {
          rnl ();               /* leerzeile */
          prices (r, f);
        }

      /* Bewachen */

      guards (r);

      /* Statistik */

      if (wants_stats)
        statistics (r, f);

      /* Burgen und ihre Einheiten */

      for (b = r->buildings; b; b = b->next)
        {
          rnl ();               /* neue leerzeile */

          sprintf (buf, "%s, %s, %s", 
		   buildingid (b),
		   translate (ST_SIZE, f->language, b->size), 
		   buildingtype (b));

          if (b->besieged)
            scat (strings[ST_BESIEGED][f->language]);

          i = 0;
          if (b->display)
            {
              scat ("; ");
              scat (b->display);

              i = b->display[strlen (b->display) - 1];
            }
          if (i != '!' && i != '?' && i != '.')
            scat (".");

          rparagraph (buf, 4, 0);

          /* Ich vertraue nicht darauf, dass die jeweiligen Kommando
             Inhaber wirklich weiter vorne in der Liste stehen.
             Deswegen werden sie hier explizit wieder gesucht und an
             die erste Stelle gesetzt.  Es waere interessant, woher
             diese Fehler herruehren.  */

          for (u = r->units; u; u = u->next)
            if (u->building == b && u->owner)
              {
                rpunit (f, r, u, 8);
                break;
              }

          for (u = r->units; u; u = u->next)
            if (u->building == b && !u->owner)
              rpunit (f, r, u, 8);
        }

      /* Schiffe und ihre Einheiten */

      for (sh = r->ships; sh; sh = sh->next)
        {
          rnl ();               /* neue leerzeile */

          sprintf (buf, "%s, %s", shipid (sh), shiptypes[0][sh->type]);
          if (sh->left)
            {
              if (r->terrain == T_OCEAN)
                {
                  scat (", ");
                  scat (translate (ST_DAMAGED, f->language, 
				   100 * sh->left / shipcost[sh->type]));
                }
              else
                {
                  scat (", ");
                  scat (translate (ST_INCOMPLETE, f->language, 
				   100 * (shipcost[sh->type] - sh->left) / shipcost[sh->type]));
                }
            }
	  else
	    {
	      scat (", ");
	      scat (translate (ST_CAPACITY, f->language, capacity (r, sh), shipcapacity[sh->type])); 
	    }

          i = 0;
          if (sh->display)
            {
              scat ("; ");
              scat (sh->display);

              i = sh->display[strlen (sh->display) - 1];
            }
          if (i != '!' && i != '?' && i != '.')
            scat (".");

          rparagraph (buf, 4, 0);

          /* Hier gilt der gleiche Kommentar wie oben bei den Burgen.  */

          for (u = r->units; u; u = u->next)
            if (u->ship == sh && u->owner)
              {
                rpunit (f, r, u, 8);
                break;
              }

          for (u = r->units; u; u = u->next)
            if (u->ship == sh && !u->owner)
              rpunit (f, r, u, 8);
        }

      /* Restliche Einheiten */
      for (u = r->units; u; u = u->next)
        if (!u->building && !u->ship && cansee (f, r, u))
            rpunit (f, r, u, 4);
    }

  rnl ();                   /* leerzeile */
  center ("------------------------------------------------------------------------");

  if (f->no)
    {
      if (!anyunits)
        {
          rnl ();
          rparagraph (strings[ST_NO_MORE_UNITS][f->language], 0, 0);
        }
      else
        order_template (f);
    }
}

/* ------------------------------------------------------------- */

FILE *BAT;

/* ------------------------------------------------------------- */

int
netaddress (char *s)
{
  /* Die erste email Adresse wird verwendet.  Simulierte regexp:
     [-._@0-9a-zA-Z]*@[-._@0-9a-zA-Z]* -- es ist moeglich nach der
     adresse noch info - zB. den vollen Namen oder die Tel Nr
     anzufuegen: "alex@zool.unizh.ch - Alexander Schroeder, Tel 01 /
     313 13 72" ist OK.  */
  int i, j;
  char *c;

  if (!s)
    return 0;

  c = strchr (s, '@');
  if (!c)
    return 0;

  /* Setzte buf auf die Netadresse!  */
  i = c - s;
  j = 0;

  /* Finde Start der Adresse.  Am Ende zeigt i auf den ersten Char,
     der *nicht* mehr zur Adresse gehört.  */
  while (i >= 0 && s[i] &&
         (isalnum (s[i]) ||
          s[i] == '@' ||
          s[i] == '-' ||
          s[i] == '.' ||
          s[i] == '_'))
    i--;

  /* Kopiere Adresse -- zuerst i auf Start der Adresse zeigen lassen.
     Falls s mit der email Adresse began, war i -1, nun wird i++
     gemacht, so dass ab s[0] kopiert wird -- was richtig ist.  */
  i++;
  while (s[i] &&
         (isalnum (s[i]) ||
          s[i] == '@' ||
          s[i] == '-' ||
          s[i] == '.' ||
          s[i] == '_'))
    buf[j++] = s[i++];

  /* beende adresse */
  buf[j] = 0;

  /* test der adresse, weitere koennen folgen: */
  if (!strchr (buf, '@'))
    return 0;

  return 1;
}

/* ------------------------------------------------------------- */

int net_report_count;

/* ------------------------------------------------------------- */

void
openbatch (char *dir)
{
  faction *f;

  /* Falls mind. ein Spieler mit email Adresse gefunden wird, schreibe
     den header des batch files. "if (BAT)" kann verwendet werden, um
     zu pruefen ob netspieler vorhanden sind und ins mailit batch
     geschrieben werden darf. Mit dem batch werden auch alle Zeitungen
     und Kommentare verschickt. */
  for (f = factions; f; f = f->next)
    if (netaddress (f->addr))
      break;

  if (f)
    {
      sprintf (buf, "%s/mailit", dir);
      if (!(BAT = fopen (buf, "w")))
        puts ("Die Datei mailit konnte nicht erzeugt werden!");
      else
        fputs (
                "#\n"
                "# MAILIT shell file, vom Atlantis Host generiert\n"
                "#\n"
                "# MAILIT versendet Atlantis Reports fuer alle Spieler, deren Adresse\n"
                "# die Form \"name@machine\" hat.\n"
                "#\n"
                "# Verwendung: nohup sh mailit &\n"
                "#\n"
                "# MAILIT erwartet das script 'waithost' im Pfad. 'waithost' liefert\n"
                "# exit 0, wenn der mailhost laeuft und mail akzeptiert. MAILIT endet\n"
                "# mit exit 1, wenn der mailhost down ist.\n"
                "#\n"
                "# Desweiteren muss MAILIT in einem directory zusammen mit der Zeitung\n"
                "# (zeitung), dem Kommentar (komment) und den reports *.nr und *.cr \n"
                "# ausgefuehrt werden.\n"
                "#\n", BAT);
      net_report_count = 0;

    }
}

void
closebatch (void)
{
  if (BAT)
    {
      fputs (   "\n"
                "echo Done!\n", BAT);

      fclose (BAT);
    }
}

/* ------------------------------------------------------------- */

void
repdir (char *dir)
{

  /* macht REPORT subdirectory (falls nicht vorhanden) und loescht alle
     files darin (falls vorhanden) */

  DIR *dp;
  struct dirent *ep;

  dp = opendir (dir);
  if (dp != NULL)
    {
      while ((ep = readdir (dp)))
	unlink (ep->d_name);
      closedir (dp);
    }
  else
    mkdir (dir, S_IRWXU);
}

/* ------------------------------------------------------------- */

void
reports (void)
{
  faction *f;
  int gotit, wants_report, wants_computer_report, wants_zine,
      wants_comment, wants_compressed;

  /* macht REPORT subdirectory (falls nicht vorhanden) und loescht alle
     files darin (falls vorhanden) */

  repdir ("reports");

  /* oeffnet file BAT (mailit batch file) */

  openbatch ("reports");

  wants_report = pow (2, O_REPORT);
  wants_computer_report = pow (2, O_COMPUTER);
  wants_zine = pow (2, O_ZINE);
  wants_comment = pow (2, O_COMMENTS);
  wants_compressed = pow (2, O_COMPRESS);

  for (f = factions; f; f = f->next)
    {
      gotit = 0;

      /* Schreiben der Reports ind /REPORTS: *.nr sind normale Reports, *.cr sind Computer Reports.
         Netreports werden im mailit file verschickt. Der mailit file
         wird mit BAT gebraucht. */

      if ((f->options & wants_report))
        {
          sprintf (buf, "reports/%d.nr", f->no);
          if (cfopen (buf, "wt"))
            {
              report (f);
              fclose (F);
              gotit = 1;
            }
        }

      if ((f->options & wants_computer_report))
        {
          sprintf (buf, "reports/%d.cr", f->no);
          if (cfopen (buf, "wt"))
            {
              report_computer (f);
              fclose (F);
              gotit = 1;
            }
        }

      /* Auch Kommentar und Zeitung muss per mail verschickt werden. buf enthaelt die adresse nach Aufruf von
         netaddress.  BAT sollte gesetzt sein, sonst gab es schon eine Fehlermeldung. */
      if (netaddress (f->addr) && BAT)
	{
          /* mailhost schonen... */

          if (!(net_report_count++ % MAILCHECK_INTERVAL))
            fputs (MAILCHECK, BAT);
          fprintf (BAT, "sleep %d;date;echo %s\n", MAILDELAY, buf);

          if (f->options & wants_compressed)
            {
              /* Alle Reports auf alle Faelle in die Liste. Hiermit wird auch automatisch das file list wieder
                 initialisiert.  Selbst wenn es keine Reports gibt, ist list initialisiert. */

              fprintf (BAT, "echo %d.* > list\n", f->no);

              if (f->options & wants_zine)
                fputs ("echo zeitung >> list\n", BAT);

              if (f->options & wants_comment)
                fputs ("echo komment >> list\n", BAT);

              fprintf (BAT, "zip -@ %d.zip < list\n", f->no);

              fprintf (BAT, "uuencode %d.zip %d.zip | "
                       "mail -s \"Atlantis %s\" %s\n",
                       f->no, f->no, gamedate (f), buf);
            }
          else /* Normale Reports, Zeitung und Kommentar verschicken */
            {
              if (f->options & wants_report)
                fprintf (BAT, "mail -s \"%s %s\" %s < %d.nr\n",
                         strings[ST_MAIL_REPORT][f->language], gamedate (f), buf, f->no);
              if (f->options & wants_computer_report)
                fprintf (BAT, "mail -s \"%s %s\" %s < %d.cr\n",
                         strings[ST_MAIL_COMPUTER_REPORT][f->language], gamedate (f), buf, f->no);
              if (f->options & wants_zine)
                fprintf (BAT, "mail -s \"%s %s\" %s  < zeitung\n", 
			 strings[ST_MAIL_NEWS][f->language], gamedate (f), buf);
              if (f->options & wants_comment)
                fprintf (BAT, "mail -s \"%s %s\" %s < komment\n", 
			 strings[ST_MAIL_COMMENT][f->language], gamedate (f), buf);
            }
        }

      if (!gotit)
        printf ("Kein Report fuer Partei Nr. %d!\n",
                f->no);
    }

  /* schliesst BAT und verschickt Zeitungen und Kommentare */

  closebatch ();

}
