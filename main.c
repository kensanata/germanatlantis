/* German Atlantis PB(E)M host Copyright (C) 1995-1998  Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

#include "atlantis.h"

void
getgarbage (void)
{
  faction *f;
  region *r;

  /* Get rid of stuff that was only relevant last turn */

  puts ("- entferne Texte der letzten Runde und begruesse newbies...");

  for (f = factions; f; f = f->next)
    {
      memset (f->showdata, 0, sizeof f->showdata);
      /* f->allies natuerlich nicht loeschen, dort sind die Alliierten
         der letzten Runde gespeichert, die noch gebraucht werden.  */
      freelist (f->mistakes);
      freelist (f->warnings);
      freelist (f->messages);
      freelist (f->battles);
      freelist (f->events);
      freelist (f->income);
      freelist (f->commerce);
      freelist (f->production);
      freelist (f->movement);
      freelist (f->debug);

      f->mistakes = 0;
      f->warnings = 0;
      f->messages = 0;
      f->battles = 0;
      f->events = 0;
      f->income = 0;
      f->commerce = 0;
      f->production = 0;
      f->movement = 0;
      f->debug = 0;

      if (f->newbie)
	addmessage (f, "Automatische Begruessung durch den Computer: Willkommen auf Atlantis! "
		    "Bitte vergesst euer Passwort nicht. Es ist in der Vorlage der Befehle am Ende der "
		    "Auswertung noch einmal aufgefuehrt, zusammen mit euer Partei-Nr. und eurer Einheit-Nr. "
		    "Wenn ihr Fragen habt, koennt ihr euch gerne an den Spielleiter wenden. "
		    "Beispiele fuer eure Befehle findet ihr in der Doku.");
    }
  
  for (r = regions; r; r = r->next)
    {
      freelist (r->comments);
      freelist (r->debug);

      r->comments = 0;
      r->debug = 0;
    }
}

void
clear_newbie_flags (void)
{
  faction *f;

  puts ("- loesche newbies flag...");

  for (f = factions; f; f = f->next)
    f->newbie=0;
}

void
show_all_reports (void)
{
  printf ("Schreibe die Reports der %d. Runde...\n", turn);
  reports ();
  writeaddresses ();
}

/* Hauptschlaufe zur Abwicklung des Spieles! */
void
processorders (void)
{
  new_units ();
  plan_monsters ();
  set_passw (); /* und pruefe auf illegale Befehle */
  setdefaults ();
  instant_orders ();
  mail ();
  docontact ();
  combat (); /* leere Einheiten tauchen auf */
  siege ();
  giving ();
  recruiting ();
  quit ();
  enter ();
  givecommand ();
  leaving ();
  destroy ();
  produce ();
  learn ();
  magic ();
  stealing ();
  movement ();
  last_orders ();
  demographics ();
}

void
processturn (void)
{
  /* Der Name der Befehlsdatei liegt in buf.  */
  if (!strcmp (zat, NOZAT))
    puts ("Kein ZAT angegeben!");
  turn++;
  getgarbage ();
  if (!readorders ())
    return;
  processorders ();
  writesummary ();
  clear_newbie_flags ();
  writegame ();
  show_all_reports ();
}

void
show_report (faction *f)
{
  sprintf (buf, "%d.r", f->no);
  if (!cfopen (buf, "wt"))
    return;

  report (f);

  fclose (F);
}

void
show_computer_report (faction *f)
{
  sprintf (buf, "%d.rc", f->no);
  if (!cfopen (buf, "wt"))
    return;

  report_computer (f);

  fclose (F);
}

int
main (int argc, char *argv[])
{
  faction *f;
  int i, n, errorlevel, lean=0;

  printf (
           "\n"
           "German Atlantis %d.%d PB(E)M host\n"
           "Copyright (C) 1995-1998 by Alexander Schroeder.\n\n"

           "based on Atlantis v1.0\n"
           "Copyright (C) 1993 by Russell Wallace.\n\n"

           "German Atlantis is distributed in the hope that it will be useful,\n"
           "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n"

           "This program may be freely used, modified and distributed. It may\n"
           "not be sold or used commercially without prior written permission\n"
           "from the author.\n\n",

           RELEASE_VERSION / 10, RELEASE_VERSION % 10);

  /* Es soll auch moeglich sein, eine turn Nummer auf der
     Kommandozeile anzugeben ("-t n"). Dieses turn wird dann in
     initgame () gelesen.  Ein Resultat von 0 muss allerdings
     ueberprueft werden. */
  turn = -1;
  for (i = 1; i+1 < argc; i++)
    if (argv[i][0] == '-'
        || argv[i][0] == '/')
      switch (argv[i][1])
	{

	case 't':
	  turn = atoip (argv[i+1]);
	  if (!turn && strcmp (argv[i+1], "0"))
	    turn = -1;
	  break;

	case 'v':
	  lean = 1;
	  break;
	}
  /* Falls die automatische Verarbeitung mit -v angegeben ist, dann
     kann das Spiel "lean" geladen werden, das heisst alle Texte der
     letzten Runde werden nicht mehr geladen.  */
  initgame (lean);

  /* Bei errorlevel -1 wird das Menu angezeigt. Bei 0 wird etwas
     automatisch prozessiert, und kein Fehler ist passiert. Bei einem
     errorlevel ueber 0 ist ein Fehler passiert. */
  errorlevel = -1;
  for (i = 1; i != argc; i++)
    if (argv[i][0] == '-'
        || argv[i][0] == '/')
      switch (argv[i][1])
        {

        case 't':
          break;   /* muss vor initgames () ausgewertet werden! */

        case 'r':
          errorlevel = 0;
          f = 0;

          /* Schreibe report fuer eine Partei, falls "-r n" und n eine
             Parteinummer ist. n darf nur 0 sein, falls wirklich "0"
             dort steht. atoip liefert naemlich auch 0, wenn ein Fehler
             passiert ist, und an der Stelle von n keine Zahl steht. */

          if (i+1 < argc
	      && argv[i+1][0]
              && argv[i+1][0] != '-'
              && argv[i+1][0] != '/')
            {
              n = atoip (argv[i+1]);
              if (n || !strcmp (argv[i+1], "0"))
		{
		  f = findfaction (n);
		  if (f)
		    show_report (f);
		  else
		    printf ("Partei %d nicht gefunden.\n", n);
		}
	      break;
            }
          else
	    show_all_reports ();
          break;

        case 'c':
          errorlevel = 0;
          f = 0;
          if (i+1 < argc
	      && argv[i+1][0]
              && argv[i+1][0] != '-'
              && argv[i+1][0] != '/')
            {
              n = atoip (argv[i+1]);
              if (n || !strcmp (argv[i+1], "0"))
                f = findfaction (n);
              if (f)
                {
                  show_computer_report (f);
                  break;
                }
            }
          /* falls nicht gelungen: Fehler */
          puts ("Keine gueltige Partei-Nr. angegeben.");
          errorlevel = 1;
          break;

        case 's':
          errorlevel = 0;
          writesummary ();
          break;

        case 'v':
          if (i+1 < argc
	      && argv[i+1][0]
              && argv[i+1][0] != '-'
              && argv[i+1][0] != '/')
	    {
	      assert (lean);
	      strcpy (buf, argv[i+1]);
	      printf ("Verwende Befehlsdatei: %s\n", buf);
	      processturn ();
	      errorlevel = 0;
	    }
	  else
            {
              puts ("Keine Befehlsdatei angegeben.\n\n");
              errorlevel = 1;
            }
	  break;

	case 'z':
          if (i+1 < argc
	      && argv[i+1][0]
              && argv[i+1][0] != '-'
              && argv[i+1][0] != '/')
	    {
	      strcpy (zat, argv[i+1]);
            }
	  else
	    {
              puts ("Kein ZAT angegeben.");
              errorlevel = 1;
	    }
	  break;

        default:
          errorlevel = 1;
          fprintf (stderr, "Usage: %s [options]\n"
                   "-r [partei-nr]   : schreibt Reports fuer alle oder nur fuer die\n"
                   "                   angegebene Partei\n"
                   "-c partei-nr     : schreibt Computer Report fuer die angegebene Partei\n"
                   "-t runde         : liest angegeben Runden Daten\n"
                   "-s               : schreibt Zusammenfassung neu\n"
		   "-z zat           : setzt den ZAT fuer die Reports\n"
                   "-v datei         : verarbeitet die angegebene Befehlsdatei\n",
                   argv[0]);
          break;
        }

  if (errorlevel >= 0)
    return errorlevel;

  puts ("? zeigt das Menue an.");

  for (;;)
    {
      printf ("> ");
      gets (buf);

      switch (buf[0])
        {
        case 'a':
          showaddresses ();
          break;

        case 'A':
          writeaddresses ();
          break;

        case 'e':
        case 'E':
          createcontinent ();
          break;

        case 'i':
        case 'I':
          regioninfo ();
          break;

        case 'k':
          showmap (M_TERRAIN);
          break;

        case 'K':
          writemap (M_TERRAIN);
          break;

        case 'p':
          showmap (M_FACTIONS);
          break;

        case 'P':
          writemap (M_FACTIONS);
          break;

        case 'u':
          showmap (M_UNARMED);
          break;

        case 'U':
          writemap (M_UNARMED);
          break;

        case 'n':
        case 'N':
          addplayers ();
          break;

        case 'v':
        case 'V':
          printf ("Datei mit den Befehlen? ");
          gets (buf);
          if (buf[0])
            processturn ();
          break;

        case 'r':
        case 'R':
          printf ("Partei Nr.? (default: alle) ");
          gets (buf);
          i = atoip (buf);
          if (i || !strcmp (buf, "0"))
            {
              f = findfaction (i);
              if (f)
                show_report (f);
            }
          else
            show_all_reports ();
          break;

        case 's':
        case 'S':
	  /* Die Zusammenfassung rechnet fuer jede Partei den Wert
             aus, der nachher auch in den Datenfile geschrieben werden
             muss. */
	  writesummary ();
          writegame ();
          break;

        case 't':
        case 'T':
          changeterrain ();
          break;

        case 'q':
        case 'Q':
          return 0;

        case 'l':
        case 'L':
          listnames ();
          break;

        case 'm':
        case 'M':
          addunit (-1);
          break;

        case 'b':
        case 'B':
          addbuilding ();
          break;

        case 'y':
        case 'Y':
          writesummary ();
          break;

	case 'z':
	case 'Z':
	  fputs ("Neuer ZAT: ", stdout);
	  if (!gets (zat))
	    {
	      puts ("Kein ZAT angegeben.");
	      strcpy (zat, NOZAT);
	    }
	  break;

        default:
          printf ("z - ZAT setzen.  ZAT jetzt: %s\n"
                "v - Befehle verarbeiten -- besser mit der -v Option direkt!\n"
		"\n"
                "e - Erzeuge Regionen.\n"
                "t - Terraform Regionen.\n"
                "m - Erschaffe Einheiten und Monster.\n"
                "b - Erbaue eine Burg.\n"
                "n - Neue Spieler hinzufuegen.\n"
                "\n"
                "a - Adressen anzeigen.\n"
                "i - Info ueber eine Region.\n"
                "k - Karte anzeigen (K - in die Datei 'karte' schreiben).\n"
                "p - Politische Karte (P - in die Datei 'karte' schreiben).\n"
                "u - Unbewaffneter Regionen (U - in die Datei 'karte' schreiben)\n"
                "l - Liste aller Laendernamen zeigen.\n"
                "r - Reports schreiben.\n"
                "y - Zusammenfassung schreiben.\n"
                "\n"
                "s - Spielstand speichern.\n"
                "\n"
                "q - Beenden.\n", zat);
        }
    }
}


