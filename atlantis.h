/* German Atlantis PB(E)M host Copyright (C) 1995-1999  Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed. It may
   not be sold or used commercially without prior written permission
   from the author.  */

#ifndef ATLANTIS_H
#define ATLANTIS_H

/* Fuer FILE braucht man hier eigentlich nur stdio.h; da aber andere
   Funktionen auch in fast allen .o files gebraucht werden, werden
   hier die meisten libraries schon eingebunden.  assert.h fuer assert
   (), stdlib.h fuer rand (), string.h fuer strcat (), etc.  */

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/* Indicator soll einigermassen unabhaengig sein. */
#include "indicato.h"

/* Translation soll unabhaengig sein.  */
#include "translate.h"

/* Zuerst defines, welche gebraucht werden. */

/* Folgende Konstanten werden fuer alte Versionen gebraucht (Vxx).
   Sie werden nur readgame () verwendet werden, um Datenfiles alter
   Versionen richtig lesen zu koennen.  Aendert sich der source code,
   aber nicht der Datenfile (wie so oft), muss nur RELEASE_VERSION
   erhoeht werden.  Alte Versionen koennen periodisch geloescht werden
   (zusammen mit dem code, der diese Konstanten verwendet).  */

#define V20                     20
#define V24                     24
#define V32                     32
#define V40                     40
#define V50                     50
#define V60                     60
#define RELEASE_VERSION         62

/* Default fuer die Auswertung, falls kein ZAT angegeben wird. */

#define NOZAT  "???"

/* Fuer das Lesen der Befehle.  */

#define MAXSTRING               1500
#define MAXLINE                 5000
#define SPACE_REPLACEMENT       ("\240"[0])
#define SPACE                   ' '
#define ESCAPE_CHAR             '\\'

/* Laengenbeschraenkungen fuer einzelne Datenfelder.  Oft ist diese
   Beschraenkung nicht wirklich noetig, weil die Text dynamisch
   cmalloced ("checked" malloc) werden.  Es dient sozusagen zur
   Reduktion des Datenfiles...  */

#define DISPLAYSIZE             400
#define NAMESIZE                80
#define STARTMONEY              5000

/* Beim wievielten NMR wird die Partei geloescht. */

#define ORDERGAP                4

/* Sprache von Debug-Meldungen ist Deutsch.  Falls dort mehrsprachige
   Dinge wie zB. itemnames vorkommen, benoetigt man den Index der
   Default-Sprache.  Dies ist die erste Sprache im [LANGUAGES]
   Abschnitt in language.def -- Deutsch sollte 0 sein.  Da alle Debug
   Texte sowieso sprachunabhaengig sind, wurde dort auch 0
   hineingeschrieben (aus Faulheit).  */

/* Wieviele Tage lernt man in einem Monat bei der Anwendung eines
   Talentes.  Wieviel Silber braucht ein Wesen pro Monat, um sich zu
   ernaehren.  Wieviele Bauern leben in einer "Parzelle" -- eine
   Region hat 1000 Parzellen, in denen je ein Baum oder ein Pferd oder
   (i.A.) 10 Bauern leben koennen.  Wieviel Grundlohn hat man beim
   ARBEITE Befehl.  Wieviel Bonus gibt es bei hoeheren Burgengroessen.  */

#define PRODUCEEXP              10
#define MAINTENANCE             10
#define MAXPEASANTS_PER_AREA    10
#define WAGE                    11
#define BONUS                   1

/* Maximale Geschwindigkeiten -- die doppelte Geschwindigkeit des
   schnellsten Schiffes, wegen dem Sturmwind-Zauber.  Wird benoetigt
   fuer einen array von Regionen, durch den Schiffe segeln.  Im Moment
   ist das die Bootsgeschwindigkeit mit Sturmwind und Helm der Sieben
   Winde: 3 * 2 * 3 = 18; auch sehr hoch: Trireme mit Sturmwind: 8 * 2
   = 16.  */

#define MAXSPEED                19

enum
  {
    M_TERRAIN,
    M_FACTIONS,
    M_UNARMED,
    MAXMODES,
  };

enum
  {
    ST_FIGHT,
    ST_BEHIND,
    ST_AVOID,
  };

enum
  {
    K_ADDRESS,
    K_WORK,
    K_ATTACK,
    K_NAME,
    K_STEAL,
    K_BESIEGE,
    K_DISPLAY,
    K_ENTER,
    K_GUARD,
    K_MAIL,
    K_END,
    K_FIND,
    K_FOLLOW,
    K_RESEARCH,
    K_GIVE,
    K_ALLY,
    K_STATUS,
    K_COMBAT,
    K_BUY,
    K_CONTACT,
    K_TEACH,
    K_STUDY,
    K_DELIVER,
    K_MAKE,
    K_MOVE,
    K_PASSWORD,
    K_RECRUIT,
    K_COLLECT,
    K_SEND,
    K_QUIT,
    K_TAX,
    K_ENTERTAIN,
    K_SELL,
    K_LEAVE,
    K_CAST,
    K_RESHOW,
    K_DESTROY,
    MAXKEYWORDS,
  };

extern char *keywords[MAXKEYWORDS];

enum
  {
    P_ALL,
    P_PEASANT,
    P_LOOT,
    P_BUILDING,
    P_UNIT,
    P_BEHIND,
    P_CONTROL,
    P_MAN,
    P_NOT,
    P_NEXT,
    P_FACTION,
    P_PERSON,
    P_REGION,
    P_SHIP,
    P_SILVER,
    P_ROAD,
    P_TEMP,
    P_AND,
    P_SPELLBOOK,
    MAXPARAMS,
  };

extern char *parameters[MAXPARAMS];

enum
  {
    O_REPORT,
    O_COMPUTER,
    O_ZINE,
    O_COMMENTS,
    O_STATISTICS,
    O_DEBUG,
    O_COMPRESS,
    MAXOPTIONS,
  };

extern char *options[MAXOPTIONS];

enum
  {
    SK_CROSSBOW,
    SK_LONGBOW,
    SK_CATAPULT,
    SK_SWORD,
    SK_SPEAR,
    SK_RIDING,
    SK_TACTICS,
    SK_MINING,
    SK_BUILDING,
    SK_TRADE,
    SK_LUMBERJACK,
    SK_MAGIC,
    SK_HORSE_TRAINING,
    SK_ARMORER,
    SK_SHIPBUILDING,
    SK_SAILING,
    SK_QUARRYING,
    SK_ROAD_BUILDING,
    SK_STEALTH,
    SK_ENTERTAINMENT,
    SK_WEAPONSMITH,
    SK_CARTMAKER,
    SK_OBSERVATION,
    MAXSKILLS
    /* Changes here will affect the syntax checker acheck.c! */
  };

extern char *skillnames[MAXSKILLS];

enum
  {
    U_MAN,
    U_UNDEAD,
    U_ILLUSION,
    U_FIREDRAGON,
    U_DRAGON,
    U_WYRM,
    U_GUARDS,
    MAXTYPES,
  };

extern int typenames[2][MAXTYPES];
extern int income[MAXTYPES];

enum
  {
    D_NORTH,
    D_SOUTH,
    D_EAST,
    D_WEST,
    MAXDIRECTIONS,
  };

extern char back[MAXDIRECTIONS];
extern char delta_x[MAXDIRECTIONS];
extern char delta_y[MAXDIRECTIONS];
extern char *directions[MAXDIRECTIONS];

/* Sollten sich diese Terrains aendern, muessen gewisse Zaubersprueche auch angepasst werden!  */

enum
  {
    T_OCEAN,
    T_PLAIN,
    T_FOREST,                   /* wird zu T_PLAIN konvertiert */
    T_SWAMP,
    T_DESERT,                   /* kann aus T_PLAIN entstehen */
    T_HIGHLAND,
    T_MOUNTAIN,
    T_GLACIER,                  /* kann aus T_MOUNTAIN entstehen */
    MAXTERRAINS,
  };

extern int terrainnames[MAXTERRAINS];
extern char terrainsymbols[MAXTERRAINS];
extern int production[MAXTERRAINS];
extern char mines[MAXTERRAINS];
extern int roadreq[MAXTERRAINS];
extern char *roadinto[MAXTERRAINS];
extern char *trailinto[MAXTERRAINS];

enum
  {
    SH_BOAT,
    SH_LONGBOAT,
    SH_DRAGONSHIP,
    SH_CARAVELL,
    SH_TRIREME,
    MAXSHIPS,
  };

extern char *shiptypes[2][MAXSHIPS];
extern int sailors[MAXSHIPS];
extern int shipcapacity[MAXSHIPS];
extern int shipcost[MAXSHIPS];
extern char shipspeed[MAXSHIPS];

enum
  {
    B_SITE,
    B_FORTIFICATION,
    B_TOWER,
    B_CASTLE,
    B_FORTRESS,
    B_CITADEL,
    MAXBUILDINGS,
  };

#define STONERECYCLE                    50

extern int buildingcapacity[MAXBUILDINGS];
extern char *buildingnames[MAXBUILDINGS];

enum
  {
    I_IRON,		/* start of ressources and products */
    I_WOOD,
    I_STONE,
    I_HORSE,		/* end of ressources */
    I_WAGON,
    I_CATAPULT,
    I_SWORD,
    I_SPEAR,
    I_CROSSBOW,
    I_LONGBOW,
    I_CHAIN_MAIL,
    I_PLATE_ARMOR,	/* end of products */
    I_BALM,		/* start of luxuries */
    I_SPICE,
    I_JEWELRY,
    I_MYRRH,
    I_OIL,
    I_SILK,
    I_INCENSE,		/* end of luxuries */
    I_1,		/* start of magic items */
    I_2,
    I_AMULET_OF_HEALING,
    I_AMULET_OF_TRUE_SEEING,
    I_CLOAK_OF_INVULNERABILITY,
    I_RING_OF_INVISIBILITY,
    I_RING_OF_POWER,
    I_RUNESWORD,
    I_SHIELDSTONE,
    I_WINGED_HELMET,
    I_DRAGON_PLATE,
    I_4,		/* end of magic items */
    MAXITEMS
    /* Changes here will affect the syntax checker acheck.c! */
  };

extern int itemnames[2][MAXITEMS];

#define LASTRESSOURCE   (I_HORSE +1)
#define isressource(i)  (0 <= i && i < LASTRESSOURCE)

#define LASTPRODUCT     (I_PLATE_ARMOR +1)
#define isproduct(i)    (0 <= i && i < LASTPRODUCT)

#define FIRSTLUXURY     (I_BALM)
#define LASTLUXURY      (I_INCENSE +1)
#define MAXLUXURIES     (LASTLUXURY - FIRSTLUXURY)
#define isluxury(i)     (FIRSTLUXURY <= i && i < LASTLUXURY)

#define FIRST_MAGIC_ITEM (I_1)
#define LAST_MAGIC_ITEM (I_4)

extern int itemskill[LASTPRODUCT];
extern char itemweight[LASTLUXURY];
extern char itemquality[LASTPRODUCT];
extern int rawmaterial[LASTPRODUCT];
extern char rawquantity[LASTPRODUCT];
extern int itemprice[MAXLUXURIES];

#define TEACHNUMBER              10
#define STUDYCOST               200

#define MAXMAGICIANS            3
#define ITEMCOST                200

/* Werden die folgenden defines geaendert, muessen die
   Spruchbeschreibungen auch geaendert werden. */

#define ZAP_SURVIVAL           80

#define NIGHT_EYE_TALENT        5
#define NIGHT_EYES_MAX          3

#define PLAGUE_SURVIVAL        40

#define MAP_RANGE               3
#define DIRECTED_MAP_DISTANCE  40

#define WATERWALK_MAX           4

#define TELEPORT_MAX_WEIGHT    1000

enum
  {
    SP_BLACK_WIND,
    SP_CAUSE_FEAR,
    SP_RUST,
    SP_DAZZLING_LIGHT,
    SP_FIREBALL,
    SP_HAND_OF_DEATH,
    SP_HEAL,
    SP_INSPIRE_COURAGE,
    SP_LIGHTNING_BOLT,
    SP_GOLEM_SERVICE,
    SP_CLAWS_OF_THE_DEEP,
    SP_MAKE_AMULET_OF_HEALING,
    SP_MAKE_AMULET_OF_TRUE_SEEING,
    SP_MAKE_CLOAK_OF_INVULNERABILITY,
    SP_MAKE_RING_OF_INVISIBILITY,
    SP_MAKE_RING_OF_POWER,
    SP_MAKE_RUNESWORD,
    SP_MAKE_SHIELDSTONE,
    SP_REMOTE_MAP,
    SP_PLAGUE,
    SP_DREAM_MAP,
    SP_SHIELD,
    SP_SUNFIRE,
    SP_TELEPORT,
    SP_MAP,
    SP_INFERNO,
    SP_HOLY_WOOD,
    SP_TREMMOR,
    SP_SUMMON_UNDEAD,
    SP_CONJURE_KNIGHTS,
    SP_STORM_WINDS,
    SP_FOG_WEBS,
    SP_NIGHT_EYES,
    SP_WATER_WALKING,
    SP_MAKE_WINGED_HELMET,
    SP_HAMMER,
    SP_PROVOCATION,
    SP_BLESSING,
    SP_11, /* Locate Plane */
    SP_12, /* Map other Plane */
    SP_13, /* Plane-shift */
    SP_14,
    SP_15,
    SP_16,
    SP_17,
    MAXSPELLS,
    /* Changes here will affect the syntax checker acheck.c! */
  };

extern int spellitem[MAXSPELLS];
extern int spellnames[MAXSPELLS];
extern char spelllevel[MAXSPELLS];
extern char iscombatspell[MAXSPELLS];
extern int spelldata[MAXSPELLS];

enum
  {
    W_NONE,
    W_BARELY,
    W_COPSE,
    W_FOREST,
    W_JUNGLE,
    MAXWOODS,
  };

extern char woodsize[MAXWOODS];

#define DEMANDRISE          (25)
#define DEMANDFALL          (150)
#define DEMANDFACTOR        (2500)
#define MAXDEMAND           (10000)
#define MINDEMAND           (100)

#define PEASANTGROWTH       (5)
#define PEASANTMOVE         (5)   /* Normales Wandern in Nachbarregionen */
#define PEASANTGREED        (5)   /* Zusaetzl. zur reichsten Nachbarregion. */

#define STARVATION_SURVIVAL       (3)
#define OVERPOPULATION_FRACTION   (10)

#define HORSEGROWTH         (5)
#define HORSEMOVE           (5)

#define FORESTGROWTH        (2)
#define FORESTSPREAD        (1)

/* Defines for items and movement.  */

/* Die Tragekapaz. ist hardcodiert mit defines, da es bis jetzt
   sowieso nur 2 Objecte gibt, die etwas tragen. Ein Mensch wiegt 10,
   traegt also 5, ein Pferd wiegt 50, traegt also 20.  ein Wagen wird
   von zwei Pferden gezogen und traegt total 140, davon 40 die Pferde,
   macht nur noch 100, aber samt eigenem Gewicht (40) macht also
   140. */

#define PERSONWEIGHT    10

#define HORSECAPACITY   70
#define WAGONCAPACITY  140

#define HORSESNEEDED    2

extern int carryingcapacity[MAXTYPES];

/* Verschiedene kleine Funktionen. */

#define min(a,b)                ((a) < (b) ? (a) : (b))
#define max(a,b)                ((a) > (b) ? (a) : (b))

#define addptr(p,i)             ((void *)(((char *)p) + i))

#define addlist2(l,p)           (*l = p, l = &p->next)

/* Structs, welche fuer die Datenstruktur wichtig sind. */

typedef struct list
  {
    struct list *next;
  }
list;

typedef struct strlist
  {
    struct strlist *next;
    char s[1];
  }
strlist;

struct unit;
typedef struct unit unit;

typedef struct building
  {
    struct building *next;
    int no;
    char *name;
    char *display;
    char besieged;
    int size;
    int sizeleft;
  }
building;

typedef struct ship
  {
    struct ship *next;
    int no;
    char *name;
    char *display;
    char enchanted;
    int type;
    int left;
  }
ship;

typedef struct region
  {
    struct region *next;
    int x, y, z;
    char *name;
    char *display;
    struct region *connect[MAXDIRECTIONS];
    int terrain;
    int trees;
    int horses;
    int peasants;
    int money;
    int road;
    int demand[MAXLUXURIES];
    int produced_good;
    char blocked;
    strlist *comments;
    strlist *debug;
    building *buildings;
    ship *ships;
    unit *units;
    int newpeasants;
    int newhorses;
    int newtrees;
  }
region;

struct faction;

typedef struct rfaction
  {
    struct rfaction *next;
    struct faction *faction;
    int factionno;
  }
rfaction;

typedef struct runit
  {
    struct runit *next;
    struct unit *unit;
  }
runit;

typedef struct faction
  {
    struct faction *next;
    int no;
    char *name;
    char *addr;
    char *passw;
    int lastorders;
    char newbie;
    int options;
    char seendata[MAXSPELLS];
    char showdata[MAXSPELLS];
    rfaction *allies;
    strlist *mistakes;
    strlist *warnings;
    strlist *messages;
    strlist *battles;
    strlist *events;
    strlist *income;
    strlist *commerce;
    strlist *production;
    strlist *movement;
    strlist *debug;
    char alive;
    char attacking;
    char seesbattle;
    char dh;
    int nregions;
    int nunits;
    int number;
    int money;
    int old_value;
    int language;
  }
faction;

struct unit
  {
    struct unit *next;
    int no;
    char *name;
    char *display;
    int number;
    int type;
    int money;
    int effect;
    int enchanted;
    faction *faction;
    runit *contacts;
    unit *target;
    building *building;
    ship *ship; 
    char owner;
    char status;
    char guard;
    char attacking;
    building *besieging; 
    /* Unterschied von thisorder und lastoder macht es moeglich, beim NACH Befehl den alten Default Befehl zu
       verwenden.  */
    char *thisorder;
    char *lastorder;
    char *thisorder2;        /* Fuer den LIEFERE Befehl.  */
    int combatspell;
    int skills[MAXSKILLS];
    int items[MAXITEMS];
    char spells[MAXSPELLS];
    strlist *orders;
    int alias;
    int dead;
    int learning;
    int n;
    int wants;               /* Bei der Produktion: Anzahl der gewuenschten Produkte.  */
    int wants2;              /* Beim Handel:  Anzahl Produkte, die zu kaufen sind.  */
    int wants_recruits;      /* Beim Rekrutieren: Anzahl gewuenschter Rekruten -- da REKRUTIERE ein kurzer Befehl ist.  */
    int *litems;
    int side;
    struct unit *collector;  /* Falls gesetzt erhaelt diese Einheit alle Beute nach dem Kampf.  */
  };

typedef struct order
  {
    struct order *next;
    unit *unit;
    int qty;
    int type;
  }
order;

/* globale Variablen */

extern int turn;
extern int highest_unit_no;
extern char buf[MAXLINE];
extern char zat[NAMESIZE];
extern FILE *F;

extern region *regions;
extern faction *factions;

/* Funktionen fuer Speicher und Listen-Management */

void *cmalloc (int n);

void addlist (void *l1, void *p1);
void translist (void *l1, void *l2, void *p);
void removelist (void *l, void *p);
void promotelist (void *l, void *p);
void freelist (void *p1);
int listlen (void *l);

strlist *makestrlist (char *s);
void addstrlist (strlist **SP, char *s);

/* Fehler, Warnungen, Meldungen */

void mistake2 (unit *u, strlist *S, char *comment);
void mistakeu (unit *u, char *comment);

void addwarning (faction *f, char *s);
void addevent (faction *f, char *s);
void addmessage (faction *f, char *s);
void addcomment (region *r, char *s);
void addbattle (faction *f, char *s);
void adddebug (region *r, char *s);
void addincome (faction *f, char *s);
void addcommerce (faction *f, char *s);
void addproduction (faction *f, char *s);
void addmovement (faction *f, char *s);

/* Manipulation von buf */

void scat (char *s);
void icat (int n);

/* String Manipulation allgemein */

int findstr (char **v, char *s, int n);
int _findstr (int v[], char *s, int n); /* Testversion als Ersatz */

void nstrcpy (char *to, char *from, int n);
void mnstrcpy (char **to, char *from, int n);
void mstrcpy (char **to, char *from);

int geti (void);
int atoip (char *s);

/* Verteilung von vielen Zufallszahlen, meist die Anzahl Opfer von Zauberspruechen (zB. werden 2-50 getoetet). */

int lovar (int n);

/* Funktionen fuer Talente */

int distribute (int old, int new, int n);

int cansee (faction *f, region *r, unit *u);
int effskill (unit *u, int i);

/* Befehle lesen */

char *getstr (void);
char *agetstr (void);
char *igetstr (char *init);
char *aigetstr (char *init);

int finditem (char *s);
int getitem (void);
int findparam (char *s);
int igetparam (char *s);
int getparam (void);
int getspell (void);
int findkeyword (char *s);
int igetkeyword (char *s);

region *findregion (int x, int y, int z);

extern int getunit0;
extern int getunitpeasants;
extern char getunit_text[20];
unit *getunit (region *r, unit *u);
unit *getunitglobal (region *r, unit *u);
unit *findunitglobal (int n);
faction *findfaction (int n);
faction *getfaction (void);

region *findregion (int x, int y, int z);

int isallied (unit *u, unit *u2);

/* Namen von Objekten */

char *factionid (faction *f);
char *regionid (region *r);
char *unitid (unit *u);
char *unitid2 (unit *u);
char *buildingid (building *b);
char *buildingtype (building *b);
char *shipid (ship *sh);

building *largestbuilding (region *r);

/* Burgen und Schiffe */

int armedmen (unit * u);

void siege (void);

int buildingeffsize (building * b);
void build_road (region * r, unit * u);

void build_building (region * r, unit * u);
building *create_building (region *r, unit *u);

void create_ship (region * r, unit * u, int newtype);
void continue_ship (region * r, unit * u);

building *getbuilding (region * r);
ship *getship (region * r);

building *findbuilding (int n);
ship *findship (int n);

unit *buildingowner (region * r, building * b);
unit *shipowner (region * r, ship * sh);

void enter (void);
void givecommand (void);
void leaving (void);
void destroy (void);

/* Kampf */

void combat (void);

/* Kontakte */

int besieged (unit * u);
int slipthru (region * r, unit * u, building * b);
int can_contact (region * r, unit * u, unit * u2);
int contacts (region * r, unit * u, unit * u2);
void docontact (void);

/* Erschaffen von Monstern und Inseln */

void createmonsters (void);
void addunit (int type);
void addplayers (void);
void connectregions (void);
void makeblock (int x1, int y1, int z);
void seed_monsters (int x1, int y1, int z);
void createcontinent (void);
void listnames (void);
void changeterrain (void);
void addbuilding (void);
void regioninfo (void);
void showmap (int mode);
void writemap (int mode);
int sphericalx (int coord);
int sphericaly (int coord);

/* Wirtschaft */

/* Wieviel ein Rekrut kostet, und welcher Teil (1/4) der Bauern rekrutiert
   werden kann. */

#define RECRUITCOST             50
#define RECRUITFRACTION         4

/* Welchen Teil des Silbers die Bauern fuer Unterhaltung ausgeben (1/20), und
   wieviel Silber ein Unterhalter pro Talentpunkt bekommt. */

#define ENTERTAININCOME         20
#define ENTERTAINFRACTION       20

/* Wieviele Silbermuenzen jeweils auf einmal "getaxed" werden. */

#define TAXFRACTION             10

/* Wieviel Silber pro Talentpunkt geklaut wird. */

#define STEALINCOME             50

extern int income[MAXTYPES];
extern int itemprice[MAXLUXURIES];

int findshiptype (char *s);

void scramble (void *v1, int n, int width);
void recruiting (void);
void giving (void);
void produce (void);
void stealing (void);
int collectmoney (region * r, unit * collector, int n);

/* Diverse */

void quit (void);
int getoption (void);
int wanderoff (region * r, int p);
void demographics (void);
void instant_orders (void);
void last_orders (void);
void set_passw (void);
void mail (void);

void setdefaults (void);

/* Magie */

int magicians (faction *f);
int findspell (char *s);
int cancast (unit *u, int i);

void magic (void);
void research (unit *u);

/* Monster */

unit *make_undead_unit (region * r, faction * f, int n);
unit *make_illsionary_unit (region * r, faction * f, int n);
unit *make_guards_unit (region * r, faction * f, int n);
unit *make_guarded_tower_unit (region *r, faction *f, int n, int m);
unit *make_firedragon_unit (region * r, faction * f, int n);
unit *make_dragon_unit (region * r, faction * f, int n);
unit *make_wyrm_unit (region * r, faction * f, int n);

unit *createunit (region *r1);
void new_units (void);

int richest_neighbour (region * r);
void plan_monsters (void);
void age_unit (region * r, unit * u);

/* Bewegung */

int getdirection (void);

int weight (unit *u);
int capacity (region *r, ship *sh);
void leave (region *r, unit *u);
void movement (void);
void drowning_men (region *r);

/* Reports */

extern char *directions[MAXDIRECTIONS];
extern char *roadinto[MAXTERRAINS];
extern char *trailinto[MAXTERRAINS];

void rparagraph (char *s, int indent, int mark);
char *spunit (faction *f, region *r, unit *u, int battle);
void rpunit (faction *f, region *r, unit *u, int indent);

void describe (region *r, faction *f);
int roadto (region * r, region * r2);
char *gamedate (faction *f);

void reports (void);
void report (faction * f);
void report_computer (faction * f);

/* Speichern und Laden */

int cfopen (char *filename, char *mode);
void getbuf (void);
int readorders (void);
void initgame (int lean);
void showaddresses (void);
void writeaddresses (void);
void writegame (void);
void writesummary (void);

/* Lernen und Lehren */

void learn (void);
void teach (region * r, unit * u);

/* Terrain */

void makeblock (int x1, int y1, int z);
int mainterrain (region * r);

#endif
