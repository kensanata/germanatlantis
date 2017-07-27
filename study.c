/* German Atlantis PB(E)M host Copyright (C) 1995-1998   Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "atlantis.h"
 
/* ------------------------------------------------------------- */

int
findskill (char *s)
{
  return findstr (skillnames, s, MAXSKILLS);
}

int
getskill (void)
{
  return findskill (getstr ());
}

/* ------------------------------------------------------------- */

void
teach (region * r, unit * u)
{

  /* Parameter r gebraucht, um kontrollieren zu koennen, dass die Ziel-unit auch in der selben region ist
     (getunit).  LEHRE vor LERNE.  In new_order[] speichern wir den neuen LEHRE Befehl ab, in dem TEMP Einheiten
     durch normale Einheitsnummern ersetzt wurden.  Ob dies noetig ist, entscheidet alias_used.  buf[] wird zum
     Schreiben von Fehlermeldungen gebraucht.  */

  int teaching, n, i, alias_used=0;
  unit *u2=0;
  char new_order[NAMESIZE];

  teaching = u->number * 30 * TEACHNUMBER;
  /* Neuer Default, falls TEMP Einheiten verwendet werden (alias_used). */
  strcpy (new_order, keywords[K_TEACH]); 

  for (;;)
    {
      u2 = getunit (r, u);

      /* Falls keine unit gefunden wurde, weil auch keine angegeben wurde, wird abgebrochen.  Falls keine unit
         gefunden wurde, es aber Text gab, dann muss ein Fehler passiert sein.  */
      if (!u2)
        {
          /* Falls es keinen String gibt, ist die Liste der Einheiten zuende */
          if (!getunit_text[0])
            break; /* the for loop.  */

          /* Beginne die Fehlermeldung */
          sprintf (buf, "Die Einheit `%s' wurde nicht gefunden", getunit_text);
          mistakeu (u, buf);
          continue; /* the for loop.  */
        }

      /* Im Moment sind derartig konvertierte Befehle noch auf NAMESIZE beschraenkt!  Das sollte meistens kein
         Problem sein, denn man kann mit 1 Lehrer nur 10 Personen lehren.  Es werden einfach soviele Einheiten wie
         moeglich aufgelistet.  */
      sprintf (buf, " %d", u2->no);
      if (strlen (new_order) + strlen (buf) < NAMESIZE)
	strcat (new_order, buf);

      /* Falls u2 ein alias hat, ist sie neu, und es wurde ein TEMP verwendet, um sie zu beschreiben. */
      if (u2->alias)
	alias_used = 1;

      /* Hier wird "alternate-(i)getstr" verwendet, um *s in igetstr () nicht neu zu initialisieren.  Nur so
         koennen wir am Anfang des for-loops getunit () verwenden, und Einheiten aus u->thisorder lesen!  */
      i = findkeyword (aigetstr (u2->thisorder));
      if (i != K_STUDY || (i = findskill (agetstr())) < 0)
        {
          sprintf (buf, "Einheit %d lernt nicht", u2->no);
          mistakeu (u, buf);
          continue;
        }

      if (effskill (u, i) <= effskill (u2, i))
        {
          sprintf (buf, "Einheit %d ist mindestens gleich gut wie wir",
                   u2->no);
          mistakeu (u, buf);
          continue;
        }

      /* Auch ohne expliziten KONTAKTIERE Befehl kann gelehrt werden.  Die Einheiten duerfen allerdings nicht durch
         eine Belagerung getrennt sein.  */
      if (!can_contact (r, u, u2))
	{
          sprintf (buf, "Der Belagerungszustand verunmoeglicht die Kontaktaufnahme mit Einheit %d",
		   u2->no);
	  mistakeu (u, buf);
	  continue;
	}

      /* learning sind die Tage, die sie schon durch andere Lehrer zugute geschrieben bekommen haben.  Total darf
         dies nicht ueber 30 Tage pro Mann steigen.  n ist die Anzahl zusaetzlich gelernter Tage.  n darf max. die
         Differenz von schon gelernten Tagen zum max (30 Tage pro Mann) betragen.  */
      n = (u2->number * 30) - u2->learning;
      n = min (n, teaching);

      if (n == 0)
        continue;

      u2->learning += n;

      /* Teaching ist die anzahl Leute, denen man noch was beibringen
         kann.  da hier nicht n verwendet wird, werden die leute
         gezaehlt und nicht die effektiv gelernten tage.

         Eine Einheit A von 11 Mann mit Talent 0 profitiert vom ersten
         Lehrer B also 10x30=300 tage, und der zweite Lehrer C lehrt
         fuer nur noch 1x30=30 Tage (damit das Maximum von 11x30=330
         nicht ueberschritten wird).

         Damit es aber in der Ausfuehrung nicht auf die Reihenfolge
         drauf ankommt, darf der zweite Lehrer C keine weiteren
         Einheiten D mehr lehren.  Also wird u2 30 Tage
         gutgeschrieben, aber Teaching sinkt auf 0 (300-11x30 <= 0).

         Sonst traete dies auf:

         A: lernt B: lehrt A C: lehrt A D D: lernt

         Wenn B vor C dran ist, lehrt C nur 30 tage an A (wie oben)
         und 270 tage an D.

         Ist C aber vor B dran, lehrt C 300 tage an A, und 0 tage an
         D, und B lehrt auch 0 tage an A.

         Deswegen darf C D nie lehren duerfen.  */

      teaching = max (0, teaching - u2->number * 30);

      strcpy (buf, unitid (u));
      scat (" lehrt ");
      scat (unitid (u2));
      scat (" ");
      scat (skillnames[i]);
      scat (".");

      addevent (u->faction, buf);
      if (u2->faction != u->faction)
        addevent (u2->faction, buf);
    }
  if (alias_used)
    mstrcpy (&u->lastorder, new_order);
}

/* ------------------------------------------------------------- */

void
learn (void)
{
  region *r;
  unit *u;
  int i;

  puts ("- lernen...");

  /* lernen nach lehren */

  for (r = regions; r; r = r->next)
    if (r->terrain != T_OCEAN)
      for (u = r->units; u; u = u->next)
        if (igetkeyword (u->thisorder) == K_STUDY)
          {
            i = getskill ();
            if (i < 0)
              {
                mistakeu (u, "Dieses Talent wurde nicht erkannt");
                continue;
              }

            if (u->type != U_MAN)
              {
                mistakeu (u, "Nur Menschen koennen lernen");
                continue;
              }

            if (i == SK_TACTICS || i == SK_MAGIC)
              {
                if (u->money < STUDYCOST * u->number)
                  {
                    mistakeu (u, "Die Lehrkosten koennen nicht bezahlt "
                              "werden");
                    continue;
                  }

                if (i == SK_MAGIC && !u->skills[SK_MAGIC] &&
                    magicians (u->faction) + u->number > MAXMAGICIANS)
                  {
                    sprintf (buf, "Es kann maximal %d Magier pro Partei geben",
			     MAXMAGICIANS);
                    mistakeu (u, buf);
                    continue;
                  }

                u->money -= STUDYCOST * u->number;
              }

            sprintf (buf, "%s lernt %s.", unitid (u), skillnames[i]);
            addevent (u->faction, buf);

            u->skills[i] += (u->number * 30) + u->learning;
          }
}
