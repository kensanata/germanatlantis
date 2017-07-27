/* German Atlantis PB(E)M host Copyright (C) 1995-1998   Alexander Schroeder

   based on:

   Atlantis v1.0  13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

/* Definitions for all strings that players have contact with for the
   atlantis host. Since a lot of data types are thus defined here,
   associated information is also defined here. */

#include "atlantis.h"
 
char *keywords[MAXKEYWORDS] =
{
  "ADRESSE",
  "ARBEITEN",
  "ATTACKIERE",
  "BENENNE",
  "BEKLAUE",
  "BELAGERE",
  "BESCHREIBE",
  "BETRETE",
  "BEWACHE",
  "BOTSCHAFT",
  "ENDE",
  "FINDE",
  "FOLGE",
  "FORSCHEN",
  "GIB",
  "HELFE",
  "KAEMPFE",
  "KAMPFZAUBER",
  "KAUFE",
  "KONTAKTIERE",
  "LEHRE",
  "LERNE",
  "LIEFERE",
  "MACHE",
  "NACH",
  "PASSWORT",
  "REKRUTIERE",
  "SAMMEL",
  "SENDE",
  "STIRB",
  "TREIBE",
  "UNTERHALTEN",
  "VERKAUFE",
  "VERLASSE",
  "ZAUBERE",
  "ZEIGE",
  "ZERSTOEREN",
};

char *parameters[MAXPARAMS] =
{
  "ALLES",
  "BAUERN",
  "BEUTE",
  "BURG",
  "EINHEIT",
  "HINTEN",
  "KOMMANDO",
  "MANN",
  "NICHT",
  "NAECHSTER",
  "PARTEI",
  "PERSONEN",
  "REGION",
  "SCHIFF",
  "SILBER",
  "STRASSEN",
  "TEMP",
  "UND",
  "ZAUBERBUCH",
};

char *options[MAXOPTIONS] =
{
  "AUSWERTUNG",
  "COMPUTER",
  "ZEITUNG",
  "KOMMENTAR",
  "STATISTIK",
  "DEBUG",
  "ZIPPED",
};

char *skillnames[MAXSKILLS] =
{
  "Armbrustschiessen",
  "Bogenschiessen",
  "Katapultbedienung",
  "Schwertkampf",
  "Speerkampf",
  "Reiten",
  "Taktik",
  "Bergbau",
  "Burgenbau",
  "Handeln",
  "Holzfaellen",
  "Magie",
  "Pferdedressur",
  "Ruestungsbau",
  "Schiffsbau",
  "Segeln",
  "Steinbau",
  "Strassenbau",
  "Tarnung",
  "Unterhaltung",
  "Waffenbau",
  "Wagenbau",
  "Wahrnehmung",
};

int typenames[2][MAXTYPES] =
{
  {
    ST_PERSON,
    ST_UNDEAD,
    ST_ILLUSION,
    ST_FIREDRAKE,
    ST_DRAGON,
    ST_WYRM,
    ST_PERSON,  /* Wachen sehen immer aus wie Personen.  */
  },
  {
    ST_PERSONS,
    ST_UNDEADS,
    ST_ILLUSIONS,
    ST_FIREDRAKES,
    ST_DRAGONS,
    ST_WYRMS,
    ST_PERSONS,  /* Wachen sehen immer aus wie Personen.  */
  }
};

int income[MAXTYPES] =
{
  50,
  20,
  0,
  150,
  1000,
  5000,
  15,
};

char *directions[MAXDIRECTIONS] =
{
  "Norden",
  "Sueden",
  "Osten",
  "Westen",
};

char back[MAXDIRECTIONS] =
{
  D_SOUTH,
  D_NORTH,
  D_WEST,
  D_EAST,
};

char delta_x[MAXDIRECTIONS] =
{
  0,
  0,
  1,
  -1,
};

char delta_y[MAXDIRECTIONS] =
{
  -1,
  1,
  0,
  0,
};

int terrainnames[MAXTERRAINS] =
{
  ST_OCEAN,
  ST_PLAIN,
  ST_FOREST,
  ST_SWAMP,
  ST_DESERT,
  ST_HIGHLAND,
  ST_MOUNTAIN,
  ST_GLACIER,
};

char terrainsymbols[MAXTERRAINS] = ".+WSTHBG";

char *roadinto[MAXTERRAINS] =
{
  "in den",
  "in die Ebene von",
  "in den Wald von",
  "in den Sumpf von",
  "durch die Wueste von",
  "auf das Hochland von",
  "in die Berge von",
  "auf den Gletscher von",
};

char *trailinto[MAXTERRAINS] =
{
  "Ozean",
  "die Ebene von",
  "der Wald von",
  "der Sumpf von",
  "die Wueste von",
  "das Hochland von",
  "die Berge von",
  "der Gletscher von",
};

int production[MAXTERRAINS] =
{
  0,
  1000,
  1000,
  200,
  50,
  400,
  100,
  10,
};

char mines[MAXTERRAINS] =
{
  0,
  0,
  0,
  0,
  0,
  0,
  100,
  0,
};

int roadreq[MAXTERRAINS] =
{
  0,
  100,
  150,
  0,
  0,
  200,
  500,
  0,
};

char *shiptypes[2][MAXSHIPS] =
{
  {
    "Boot",
    "Langboot",
    "Drachenschiff",
    "Karavelle",
    "Trireme",
  },
  {"ein Boot",
   "ein Langboot",
   "ein Drachenschiff",
   "eine Karavelle",
   "eine Trireme",
  }
};

int sailors[MAXSHIPS] =
{
  2,
  10,
  50,
  30,
  120,
};

int shipcapacity[MAXSHIPS] =
{
  50,
  500,
  1000,
  3000,
  2000,
};

int shipcost[MAXSHIPS] =
{
  5,
  50,
  100,
  250,
  200,
};

char shipspeed[MAXSHIPS] =
{
  3,
  4,
  6,
  6,
  8,
};

char *buildingnames[MAXBUILDINGS] =
{
  "Baustelle",
  "Befestigung",
  "Turm",
  "Schloss",
  "Festung",
  "Zitadelle",
};

/* Die Groesse gibt auch an, welches Talent man mindestens haben muss,
   um an der Burg weiterzuarbeiten (bei einer Burg zB. 3), sowie den
   Bonus-multiplikator fuer K_WORK! */

int buildingcapacity[MAXBUILDINGS] =
{
  0,
  2,
  10,
  50,
  250,
  1250,
};

int itemnames[2][MAXITEMS] =
{
  {
    ST_IRON,
    ST_WOOD,
    ST_STONE,
    ST_HORSE,
    ST_WAGON,
    ST_CATAPULT,
    ST_SWORD,
    ST_SPEAR,
    ST_CROSSBOW,
    ST_LONGBOW,
    ST_CHAIN_MAIL,
    ST_PLATE_ARMOR,
    ST_BALM,
    ST_SPICE,
    ST_JEWELRY,
    ST_MYRRH,
    ST_OIL,
    ST_SILK,
    ST_INCENSE,
    ST_I1,
    ST_I2,
    ST_AMULET_OF_HEALING,
    ST_AMULET_OF_TRUE_SEEING,
    ST_CLOAK_OF_INVULNERABILITY,
    ST_RING_OF_INVISIBILITY,
    ST_RING_OF_POWER,
    ST_RUNESWORD,
    ST_SHIELDSTONE,
    ST_WINGED_HELMET,
    ST_DRAGON_PLATE,
    ST_I4,
  },
  {
    ST_IRONS,
    ST_WOODS,
    ST_STONES,
    ST_HORSES,
    ST_WAGONS,
    ST_CATAPULTS,
    ST_SWORDS,
    ST_SPEARS,
    ST_CROSSBOWS,
    ST_LONGBOWS,
    ST_CHAIN_MAILS,
    ST_PLATE_ARMORS,
    ST_BALMS,
    ST_SPICES,
    ST_JEWELRIES,
    ST_MYRRHS,
    ST_OILS,
    ST_SILKS,
    ST_INCENSES,
    ST_I1S,
    ST_I2S,
    ST_AMULETS_OF_HEALING,
    ST_AMULETS_OF_TRUE_SEEING,
    ST_CLOAKS_OF_INVULNERABILITY,
    ST_RINGS_OF_INVISIBILITY,
    ST_RINGS_OF_POWER,
    ST_RUNESWORDS,
    ST_SHIELDSTONES,
    ST_WINGED_HELMETS,
    ST_DRAGON_PLATES,
    ST_I4S,
  }
};

int itemskill[LASTPRODUCT] =
{
  SK_MINING,
  SK_LUMBERJACK,
  SK_QUARRYING,
  SK_HORSE_TRAINING,
  SK_CARTMAKER,
  SK_CARTMAKER,
  SK_WEAPONSMITH,
  SK_WEAPONSMITH,
  SK_WEAPONSMITH,
  SK_WEAPONSMITH,
  SK_ARMORER,
  SK_ARMORER,
};

char itemweight[LASTLUXURY] =
{
  5,
  5,
  60,
  50,
  40,
  60,
  1,
  1,
  1,
  1,
  2,
  4,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
};

char itemquality[LASTPRODUCT] =
{
  1,
  1,
  1,
  1,
  1,
  2,
  1,
  2,
  2,
  3,
  1,
  3,
};

int rawmaterial[LASTPRODUCT] =
{
  0,
  0,
  0,
  0,
  I_WOOD,
  I_WOOD,
  I_IRON,
  I_WOOD,
  I_WOOD,
  I_WOOD,
  I_IRON,
  I_IRON,
};

char rawquantity[LASTPRODUCT] =
{
  0,
  0,
  0,
  0,
  5,
  10,
  1,
  1,
  1,
  1,
  3,
  5,
};

int itemprice[MAXLUXURIES] =
{
  4,
  5,
  7,
  5,
  3,
  6,
  4,
};

int spellnames[MAXSPELLS] =
{
    ST_BLACK_WIND,
    ST_CAUSE_FEAR,
    ST_RUST,
    ST_DAZZLING_LIGHT,
    ST_FIREBALL,
    ST_HAND_OF_DEATH,
    ST_HEAL,
    ST_INSPIRE_COURAGE,
    ST_LIGHTNING_BOLT,
    ST_GOLEM_SERVICE,
    ST_CLAWS_OF_THE_DEEP,
    ST_MAKE_AMULET_OF_HEALING,
    ST_MAKE_AMULET_OF_TRUE_SEEING,
    ST_MAKE_CLOAK_OF_INVULNERABILITY,
    ST_MAKE_RING_OF_INVISIBILITY,
    ST_MAKE_RING_OF_POWER,
    ST_MAKE_RUNESWORD,
    ST_MAKE_SHIELDSTONE,
    ST_REMOTE_MAP,
    ST_PLAGUE,
    ST_DREAM_MAP,
    ST_SHIELD,
    ST_SUNFIRE,
    ST_TELEPORT,
    ST_MAP,
    ST_INFERNO,
    ST_HOLY_WOOD,
    ST_TREMMOR,
    ST_SUMMON_UNDEAD,
    ST_CONJURE_KNIGHTS,
    ST_STORM_WINDS,
    ST_FOG_WEBS,
    ST_NIGHT_EYES,
    ST_WATER_WALKING,
    ST_MAKE_WINGED_HELMET,
    ST_HAMMER,
    ST_PROVOCATION,
    ST_BLESSING,
    ST_11,
    ST_12,
    ST_13,
    ST_14,
    ST_15,
    ST_16,
    ST_17,
};

int spellitem[MAXSPELLS] = 
{
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  I_AMULET_OF_HEALING,
  I_AMULET_OF_TRUE_SEEING,
  I_CLOAK_OF_INVULNERABILITY,
  I_RING_OF_INVISIBILITY,
  I_RING_OF_POWER,
  I_RUNESWORD,
  I_SHIELDSTONE,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  I_WINGED_HELMET,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
};

char spelllevel[MAXSPELLS] =
{
  4,
  2,
  1,
  1,
  2,
  3,
  2,
  2,
  1,
  1,
  1,
  3,
  3,
  3,
  3,
  4,
  3,
  4,
  3,
  2,
  2,
  3,
  5,
  3,
  1,
  5,
  1,
  2,
  2,
  1,
  1,
  2,
  1,
  2,
  2,
  4,
  4,
  5,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};

char iscombatspell[MAXSPELLS] =
{
  1,
  1,
  0,
  1,
  1,
  1,
  0,
  1,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};

/* Ich habe Soeren Schwartz garantiert, dass keine Beschreibung laenger als 1500 chars ist.  */

int spelldata[MAXSPELLS] =
{
    ST_BLACK_WIND_DATA,
    ST_CAUSE_FEAR_DATA,
    ST_RUST_DATA,
    ST_DAZZLING_LIGHT_DATA,
    ST_FIREBALL_DATA,
    ST_HAND_OF_DEATH_DATA,
    ST_HEAL_DATA,
    ST_INSPIRE_COURAGE_DATA,
    ST_LIGHTNING_BOLT_DATA,
    ST_GOLEM_SERVICE_DATA,
    ST_CLAWS_OF_THE_DEEP_DATA,
    ST_MAKE_AMULET_OF_HEALING_DATA,
    ST_MAKE_AMULET_OF_TRUE_SEEING_DATA,
    ST_MAKE_CLOAK_OF_INVULNERABILITY_DATA,
    ST_MAKE_RING_OF_INVISIBILITY_DATA,
    ST_MAKE_RING_OF_POWER_DATA,
    ST_MAKE_RUNESWORD_DATA,
    ST_MAKE_SHIELDSTONE_DATA,
    ST_REMOTE_MAP_DATA,
    ST_PLAGUE_DATA,
    ST_DREAM_MAP_DATA,
    ST_SHIELD_DATA,
    ST_SUNFIRE_DATA,
    ST_TELEPORT_DATA,
    ST_MAP_DATA,
    ST_INFERNO_DATA,
    ST_HOLY_WOOD_DATA,
    ST_TREMMOR_DATA,
    ST_SUMMON_UNDEAD_DATA,
    ST_CONJURE_KNIGHTS_DATA,
    ST_STORM_WINDS_DATA,
    ST_FOG_WEBS_DATA,
    ST_NIGHT_EYES_DATA,
    ST_WATER_WALKING_DATA,
    ST_MAKE_WINGED_HELMET_DATA,
    ST_HAMMER_DATA,
    ST_PROVOCATION_DATA,
    ST_BLESSING_DATA,
    ST_11_DATA,
    ST_12_DATA,
    ST_13_DATA,
    ST_14_DATA,
    ST_15_DATA,
    ST_16_DATA,
    ST_17_DATA,
};

char woodsize[MAXWOODS] =
{
  0,
  10,
  20,
  60,
  80,
};

int carryingcapacity[MAXTYPES] =
{
  15,
  20,
  50,
  150,
  450,
  15000,
  15,
};
