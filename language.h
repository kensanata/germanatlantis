/* Automatically generated header file by mklang.pl */

#ifndef LANGUAGE_H
#define LANGUAGE_H

/* The first language defined ist the default.  */
#define DEFAULT_LANGUAGE	0

enum
  {
    L_DEUTSCH,
    L_ENGLISH,
    MAXLANGUAGES,
  };

enum
  {
    ST_FEBRUARY,
    ST_CANNOT_DETERMINE_RECIPIENT,
    ST_HORSE,
    ST_CLAWS_OF_THE_DEEP_DATA,
    ST_CURRENT_PRICES,
    ST_STONE,
    ST_GIVE_PEASANTS_ITEMS,
    ST_MOUNTAIN,
    ST_HOLY_WOOD_DATA,
    ST_SPEARS,
    ST_CAPACITY,
    ST_SPELLS,
    ST_SILK,
    ST_MESSAGE_FROM_UNIT,
    ST_BUILDING_REQUIRES_STONES,
    ST_NOVEMBER,
    ST_16_DATA,
    ST_MISTAKES,
    ST_TREES,
    ST_CAUSE_FEAR_DATA,
    ST_GOAL_UNIT_HAS_NOT_CONTACTED_US,
    ST_WATER_WALKING_DATA,
    ST_MAKE_AMULET_OF_HEALING_DATA,
    ST_MAIL_COMPUTER_REPORT,
    ST_CONJURE_KNIGHTS,
    ST_CAUSE_FEAR,
    ST_MYRRH,
    ST_MAP_DATA,
    ST_13_DATA,
    ST_MAP,
    ST_CLOAK_OF_INVULNERABILITY,
    ST_MAKE_RUNESWORD_DATA,
    ST_DRAGON_PLATES,
    ST_DREAM_MAP,
    ST_MAY,
    ST_SUMMON_UNDEAD_DATA,
    ST_TELEPORT_DATA,
    ST_IRONS,
    ST_SET,
    ST_MAKE_AMULET_OF_TRUE_SEEING,
    ST_IS_BURNED_DOWN,
    ST_HOLY_WOOD,
    ST_MAKE_RING_OF_INVISIBILITY_DATA,
    ST_BESIEGED,
    ST_FIREBALL_DATA,
    ST_SPEAR,
    ST_RUNESWORDS,
    ST_ANONYMOUS_MESSAGE_FROM_UNIT,
    ST_BUYS,
    ST_CHAIN_MAIL,
    ST_SPICES,
    ST_WYRM,
    ST_BLESSING_DATA,
    ST_NO_ENTERTAINERS,
    ST_PEASANTS_DONT_TRADE_IN_IT,
    ST_HORSES,
    ST_AMULET_OF_TRUE_SEEING,
    ST_PLAGUE_DATA,
    ST_LONGBOW,
    ST_STORM_WINDS,
    ST_UNIT_MUST_GUARD,
    ST_IS_SUNK,
    ST_PLAGUE,
    ST_FIREBALL,
    ST_GIVE_MEN_TO_NOTHING,
    ST_11,
    ST_12,
    ST_RINGS_OF_POWER,
    ST_13,
    ST_PRODUCES,
    ST_14,
    ST_15,
    ST_16,
    ST_17,
    ST_GIVE_SPELL,
    ST_INFERNO_DATA,
    ST_ILLUSION,
    ST_SHIELD_DATA,
    ST_OILS,
    ST_MAKE_WINGED_HELMET,
    ST_CASTS,
    ST_PEASANTS,
    ST_WYRMS,
    ST_CANNOT_FIND_FACTION_FOR_MAIL,
    ST_NIGHT_EYES,
    ST_INFERNO,
    ST_UNDEAD,
    ST_RECEIVES_LOOT,
    ST_GOLEM_SERVICE_DATA,
    ST_17_DATA,
    ST_ILLUSIONS,
    ST_DAMAGED,
    ST_LANGUAGE,
    ST_MYRRHS,
    ST_INCENSE,
    ST_SILKS,
    ST_NO_MEN_TO_GIVE,
    ST_MAKE_AMULET_OF_TRUE_SEEING_DATA,
    ST_PEASANT,
    ST_GOAL_UNIT_NOT_FOUND,
    ST_PROVOCATION_DATA,
    ST_WAGONS,
    ST_RING_OF_POWER,
    ST_STARTS_TO_BUILD,
    ST_PRICE,
    ST_MONSTERS_DONT_TAKE,
    ST_PLATE_ARMOR,
    ST_BLESSING,
    ST_PRODUCES_LESS,
    ST_INCOME,
    ST_ATLANTIS_REPORT,
    ST_14_DATA,
    ST_MOVEMENTS,
    ST_HIGHLAND,
    ST_TREMMOR,
    ST_REMOTE_MAP,
    ST_RUNESWORD,
    ST_CANNOT_FIND_UNIT_FOR_MAIL,
    ST_PERSONS,
    ST_GIVE_UNIT_ITEMS,
    ST_UNIT_UNARMED,
    ST_NO_TIME,
    ST_FOG_WEBS,
    ST_CANNOT_DESTROY,
    ST_WOODS,
    ST_DRAGONS,
    ST_MAKE_AMULET_OF_HEALING,
    ST_CLOAKS_OF_INVULNERABILITY,
    ST_CROSSBOW,
    ST_HEAL_DATA,
    ST_IS_BESIEGED,
    ST_GIVE_NOBODY_ITEMS,
    ST_IRON,
    ST_PRICES_WITHOUT_BUILDINGS,
    ST_11_DATA,
    ST_MAKE_CLOAK_OF_INVULNERABILITY,
    ST_NO_PEASANTS,
    ST_HAS,
    ST_WAGON,
    ST_MAKE_SHIELDSTONE,
    ST_LAST_QUANTITY_IN_LIST,
    ST_GOLEM_SERVICE,
    ST_CONTINUING_NEEDS_MORE_TALENT,
    ST_AMULETS_OF_HEALING,
    ST_MAKE_RING_OF_POWER_DATA,
    ST_SEPTEMBER,
    ST_CONTINUES_TO_BUILD,
    ST_GIVE_MEN,
    ST_JEWELRIES,
    ST_NOBODY_CAN_BUILD,
    ST_IS_BESIEGED_BY,
    ST_STATE_OF_AFFAIRS,
    ST_BREAKS_THROUGH_MAGIC_SHIELD,
    ST_MAKE_CLOAK_OF_INVULNERABILITY_DATA,
    ST_PROVOCATION,
    ST_INSPIRE_COURAGE,
    ST_UNDEADS,
    ST_THIS_REQUIRES_ITEMS,
    ST_QUANTITIY,
    ST_MARCH,
    ST_I1S,
    ST_MAX_MAGICIANS,
    ST_MISCELLANEOUS,
    ST_MONSTERS_DONT_CHANGE_STATUS,
    ST_GLACIER,
    ST_AUGUST,
    ST_MESSAGE_FROM_FACTION,
    ST_DRAGON,
    ST_SUNFIRE,
    ST_MAIL_REPORT,
    ST_DREAM_MAP_DATA,
    ST_UNIT_HASNT_ANY,
    ST_MAKE_RUNESWORD,
    ST_BLACK_WIND_DATA,
    ST_NIGHT_EYES_DATA,
    ST_WARNINGS,
    ST_LIGHTNING_BOLT_DATA,
    ST_I2S,
    ST_BLACK_WIND,
    ST_BATTLES,
    ST_INCOMPLETE,
    ST_MONSTERS_CANT_LAY_SIEGE,
    ST_STORM_WINDS_DATA,
    ST_PLAIN,
    ST_CHAIN_MAILS,
    ST_MONSTERS_DONT_GIVE,
    ST_FIRST_PRICE,
    ST_GOAL_UNIT_ON_ANOTHER_PLANE,
    ST_DATE,
    ST_FIREDRAKES,
    ST_JEWELRY,
    ST_SHIELDSTONE,
    ST_SELLS,
    ST_OCEAN,
    ST_PRODUCTION,
    ST_DESERT,
    ST_AMULETS_OF_TRUE_SEEING,
    ST_CASTS_IN_COMBAT,
    ST_ILLUSION_DISSOLVES,
    ST_FIREDRAKE,
    ST_CANNOT_CAST,
    ST_BALMS,
    ST_MAKE_WINGED_HELMET_DATA,
    ST_FOREST,
    ST_DAZZLING_LIGHT_DATA,
    ST_HAMMER_DATA,
    ST_SKILL_NEEDED_FOR_ITEM,
    ST_LEVEL,
    ST_WOOD,
    ST_HEAL,
    ST_PEASANTS_MAKE,
    ST_STONES,
    ST_NEW_SPELLS,
    ST_CATAPULT,
    ST_15_DATA,
    ST_NO_MORE_UNITS,
    ST_RING_OF_INVISIBILITY,
    ST_MONSTERS_REFUSE_PEASANTS,
    ST_CROSSBOWS,
    ST_CONJURE_KNIGHTS_DATA,
    ST_SWAMP,
    ST_COMBAT_SPELLS,
    ST_GIVE_MEN_TO_PEASANTS,
    ST_COMMERCE,
    ST_HAND_OF_DEATH_DATA,
    ST_ZAT,
    ST_WINGED_HELMETS,
    ST_WATER_WALKING,
    ST_I4S,
    ST_CATAPULTS,
    ST_MESSAGE_EMPTY,
    ST_AMULET_OF_HEALING,
    ST_SPICE,
    ST_LIGHTNING_BOLT,
    ST_MESSAGES,
    ST_APRIL,
    ST_MAKE_SHIELDSTONE_DATA,
    ST_BUILDING_NOT_FOUND,
    ST_GIVE_ALL_MEN_TO_NOTHING,
    ST_I1,
    ST_JANUARY,
    ST_I2,
    ST_BALM,
    ST_12_DATA,
    ST_I4,
    ST_SHIP_NOT_ON_SHORE,
    ST_GIVE_ALL_MEN_TO_PEASANTS,
    ST_NO_MONEY,
    ST_FOG_WEBS_DATA,
    ST_RUST_DATA,
    ST_MAKE_RING_OF_POWER,
    ST_RUST,
    ST_SWORD,
    ST_TELEPORT,
    ST_SUMMON_UNDEAD,
    ST_PERSON,
    ST_INCENSES,
    ST_INSPIRE_COURAGE_DATA,
    ST_SHIELD,
    ST_WINGED_HELMET,
    ST_MAIL_NEWS,
    ST_MAKE_RING_OF_INVISIBILITY,
    ST_REMOTE_MAP_DATA,
    ST_OIL,
    ST_JULY,
    ST_LONGBOWS,
    ST_QUANTITY_IN_LIST,
    ST_TREE,
    ST_HAND_OF_DEATH,
    ST_RINGS_OF_INVISIBILITY,
    ST_DECEMBER,
    ST_BUT_CANCELLED_BY_MAGIC_SHIELD,
    ST_IS_BESIEGED_WITH_CATAPULTS_BY,
    ST_SIZE,
    ST_TREMMOR_DATA,
    ST_SWORDS,
    ST_RESEARCHES,
    ST_LAST_PRICE,
    ST_BUILDS_ROAD,
    ST_PLATE_ARMORS,
    ST_TALENT_IS_INSUFFICIENT,
    ST_CLAWS_OF_THE_DEEP,
    ST_OCTOBER,
    ST_DRAGON_PLATE,
    ST_JUNE,
    ST_MAIL_COMMENT,
    ST_SHIELDSTONES,
    ST_SUNFIRE_DATA,
    ST_FIRST_QUANTITY_IN_LIST,
    ST_DAZZLING_LIGHT,
    ST_HAMMER,
    MAXSTRINGS,
  };

extern char *strings[MAXSTRINGS][MAXLANGUAGES];

#endif /* LANGUAGE_H */

