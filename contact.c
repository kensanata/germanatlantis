/* German Atlantis PB(E)M host Copyright (C) 1995-1998   Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include <stdio.h>

#include "atlantis.h"

int
besieged (unit * u)
{

  /* belagert kann man in schiffen und burgen werden */

  return (u && u->building && u->building->besieged);

}

int
slipthru (region * r, unit * u, building * b)
{
  unit *u2;
  int n, o;

  /* b ist die Burg, in die man hinein oder aus der man heraus will.  slipthru liefert 1, wenn die unit u die
  Burg b verlassen kann. */

  if (!b || (b && !b->besieged))
    return 1;

  /* u wird am hinein- oder herausschluepfen gehindert, wenn OBSERVATION +2 der belagernden unit u2 >= STEALTH
     der unit u ist.  Wenn u2 u kontaktiert hat (KONTAKTIERE oder HELFE, dh. die Funktion contacts), dann darf
     u die Burg auch betreten oder verlassen.  */

  n = effskill (u, SK_STEALTH);

  for (u2 = r->units; u2; u2 = u2->next)
    if (u2->besieging == b)
      {
        if (u->items[I_RING_OF_INVISIBILITY] >= u->number &&
            !u2->items[I_AMULET_OF_TRUE_SEEING])
          continue;

        o = effskill (u2, SK_OBSERVATION);

        if (o + 2 >= n)
          return 0;             /* entdeckt! */
      }

  return 1;
}

int
can_contact (region * r, unit * u, unit * u2)
{

  /* can_contact () liefert 1, falls die beiden units u und u2 in derselben Burg sind, oder ihre
     jeweilige Burg auch verlassen koennen.

     Es wird hier kein Test darauf gemacht, ob u und u2 ueberhaupt in derselben Region sind.  So
     kann man zB. contacts () auch beim Teleportieren in teleport () gebrauchen, wo die Zieleinheit
     in einer fernen Region mit dem Magier Kontakt aufnehmen muss.  Wo die Einheit u2 sein kann,
     wird in der jeweiligen Funktion bestimmt -- wird u2 mit getunit () erhalten, so ist u2
     zwangslaeufig in derselben Region wie u, wird getunitglobal () verwendet, kann u2 auch in einer
     anderen Region sein.  Selbst wenn kein expliziter KONTAKTIERE Befehl noetig ist, wird manchmal
     can_contact () verwendet, um zu sehen, ob u oder u2 nicht durch eine Belagerung getrennt
     werden.  Dies geschieht zB. beim LEHRE Befehl in teach ().  */

  if (u->building == u2->building)
    return 1;

  /* unit u will unit u2 kontaktieren.  u2 hat sozusagen nicht danach gefragt.  Wenn u oder u2 je entweder
     nicht in einer Burg sind, oder ihre Burg nicht belagert ist, oder sie ihre belagerte Burg verlassen
     können, ist slipthru == 1.  */

  if (slipthru (r, u, u->building) && slipthru (r, u2, u2->building))
    return 1;

  return 0;
}

int
contacts (region * r, unit * u, unit * u2)
{
  runit *ru;

  /* returns 1 if unit u has issued the command to contact unit u2 - a asked for contact. */

  if (!can_contact (r, u, u2))
    return 0;

  if (isallied (u, u2))
    return 1;

  for (ru = u->contacts; ru; ru = ru->next)
    if (ru->unit == u2)
      return 1;

  return 0;
}

void
set_contact (region * r, unit * u, strlist * S)
{

  /* unit u kontaktiert unit u2. Dies setzt den contact einfach auf 1 - ein richtiger toggle ist
     nicht noetig, da contact nur fuer den aktuellen Monat gilt. Die Region als Parameter ist
     wichtig, weil er an getunitglobal () weitergegeben wird. */

  unit *u2;
  runit *ru;

  u2 = getunitglobal (r, u);

  if (u2)
    {

      if (!can_contact (r, u, u2))
        {
          mistakeu (u, "Der Belagerungszustand verunmoeglicht die "
                    "Kontaktaufnahme");
          return;
        }

      if (!isallied (u, u2))
        {
          for (ru = u->contacts; ru; ru = ru->next)
            if (ru->unit == u2)
              break;

          /* haenge u2 nur an die contacts, wenn u2 nicht alliiert ist */

          if (!ru)
            {
              ru = cmalloc (sizeof (runit));
              ru->unit = u2;
              addlist (&u->contacts, ru);
            }
        }
    }
  else
    mistake2 (u, S, "Diese Einheit wurde nicht gefunden");
}

void
docontact (void)
{
  faction *f;
  region *r;
  unit *u;
  strlist *S;
  char *s;
  int i;

  /* FINDE - darf erst ablaufen, nachdem alle ihre Adresse gesetzt
     haben! GIB KOMMANDO - sollt ablaufen, bevor BETRETE beginnt. */

  puts ("- Adressen finden, Kontakt aufnehmen...");

  for (r = regions; r; r = r->next)
    for (u = r->units; u; u = u->next)
      for (S = u->orders; S; S = S->next)
        switch (igetkeyword (S->s))
          {
            case K_FIND:
              s = getstr ();

              if (findparam (s) == P_ALL)
                {
                  i = (getparam () == P_FACTION);

                  addmessage (u->faction, "Liste aller Adressen:");
                  for (f = factions; f; f = f->next)
                    {
                      if (i)
                        sprintf (buf, " %s, %s", factionid (f), 
				 f->addr ? f->addr : "keine Adresse");
                      else
                        sprintf (buf, "%3d, %s", f->no, 
				 f->addr ? f->addr : "keine Adresse");
                      addmessage (u->faction, buf);
                    }
                }
              else
                {
                  f = findfaction (atoi (s));

                  if (f == 0)
                    mistake2 (u, S, "Die Partei wurde nicht gefunden");
                  else
                    {
		      if (f->addr)
			sprintf (buf, "Die Adresse von %s ist '%s'.",
				 factionid (f), f->addr);
		      else
			sprintf (buf, "%s hat keine Adresse.", factionid (f));
                      addmessage (u->faction, buf);
                    }
                }
              break;

            case K_CONTACT:
              set_contact (r, u, S);
              break;

          }
}
