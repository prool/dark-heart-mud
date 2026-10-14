/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "hell.h"
#include "magic.h"
#include "interp.h"


const char * lang_name[NUM_LANGUAGES] =
{
	"common", "elvish", "gnomish", "camp common"
};

const struct mage_abil_type magabil_table [11][6] =
{
	{
	    { "Unbeliever" , "Unbeliever" },
	    { "Unbeliever" , "Unbeliever" },
	    { "Unbeliever" , "Unbeliever" },
	    { "Unbeliever" , "Unbeliever" },
	    { "Pure of Heart" , "Pure of Heart" },
	    { "Tainted Soul", "Tainted Soul" }
	},
	{
	    { "Adept of the Flame" , "Adept of the Flame" },
	    { "Watcher of the Rains" , "Singer of the Rains" },
	    { "Student of the Skies" , "Student of the Clouds" },
	    { "Student of the Sands" , "Student of the Earth" },
	    { "Malcontent" , "Reprobate" },
	    { "Cleansed Soul", "Cleansed Soul" }
	},
	{
	    { "Brother of the Flame" , "Sister of the Flame" },
	    { "Follower of Rivers" , "Lady of the Stream" },
	    { "Bringer of the Frost" , "Dancer in the Snow" },
	    { "Counter of Pebbles" , "Traveller Beneath" },
	    { "Squire of Corruption" , "Student of the Black Arts" },
	    { "Follower of the Way", "Walker of the Paths" }
	},
	{
	    { "Evoker of Flames" , "Evoker of Flames" },
	    { "Brother of Nature" , "Sister of Harmony" },
	    { "Summoner of Weather" , "Daughter of the Clouds" },
	    { "Delver of the Deep" , "Explorer of the Deep" },
	    { "Watcher of the Apocalypse" , "Swordmaiden of the Apocalypse" },
	    { "Spreader of the Truth", "Sister of Truth" }
	},
	{
	    { "Son of Light" , "Daughter of Light" },
	    { "Master of Trees" , "Abjurer of Beauty" },
	    { "Mage of Illusions" , "Daughter of the Rains" },
	    { "Traveller of the Depths" , "Mistress of Stones" },
	    { "Necromancer" , "Necromancer" },
	    { "Brother Devout", "Daughter of Purity" }
	},
	{
	    { "Baron of the Sun" , "Mistress of the Sun" },
	    { "Protector of the Forest" , "Mistress of the Forest" },
	    { "Evoker of Lightning" , "Daughter of the Skies" },
	    { "Adventurer" , "Mistress of Pearls" },
	    { "Crusader for Evil" , "Witch of Darkness" },
	    { "Priest of Life", "Mistress of Blessings" }
	},
	{
	    { "Prince of Flame" , "Princess of Flames" },
	    { "Prince of Nature" , "Mistress of Flowers" },
	    { "Prince of Clouds" , "Mistress of Birds" },
	    { "Lord of the Sands" , "Mistress of Rubies" },
	    { "Son of Demons" , "Daughter of Voodoo" },
	    { "Prince of Virtue", "Princess of Hearts" }
	},
	{
	    { "Master of the Inferno" , "Mistress of the Inferno" },
	    { "Lord of the Sea" , "Lady of the Sea" },
	    { "Master of Phantasms" , "Mistress of Illusions" },
	    { "Prince of the Underworld" , "Valkyrie of the Depths" },
	    { "Prince of Darkness" , "Princess of Darkness" },
	    { "Prince of Light", "Princess of Light" }
	},
	{
	    { "Lord of the Volcano" , "Lady of the Volcano" },
	    { "King of the Oceans" , "Queen of the Oceans" },
	    { "Baron of the Skies" , "Lady of the Skies" },
	    { "King of Diamonds" , "Queen of the Depths" },
	    { "King of Pain" , "Enchantress of the Night" },
	    { "Master of Blessings", "Lady of the Light" }
	},
	{
	    { "Master of Hellfire" , "Mistress of Hellfire" },
	    { "Druid of Serenity" , "Sorceress of Tranquility" },
	    { "Magus of the Unseen" , "Archmage of the Air" },
	    { "Highmage of the Depths" , "Archmage of Earth" },
	    { "Archmage of Evil" , "Queen of Darkness" },
	    { "Archmage of Goodness", "Queen of Life" }
	},
	{
	    { "Eye of Fear and Flame" , "Mistress of Eternal Torment" },
	    { "Archdruid of Tranquility" , "Goddess of Beauty" },
	    { "Archmage of the Skies" , "Dancer upon the Clouds" },
	    { "Lord of the Mountain" , "Goddess of the Underworld" },
	    { "Avatar of Blackness" , "Sorceress of Destruction" },
	    { "Avatar of Purity", "Sorceress of White Magic" }
	}
};



/* stances */



const struct stance_type stances[12] = 
{
    {   "None",         "You relax your fighting stance.",      "",
	{ 0,   1,   -2,   -4,   -3,   -6,   -8,   -12,   -8,   -16,   -20,   -20 }
    },

    {   "rabbit",       "hop",          "hops",
	{ -1,   0,   2,   0,   10,   4,   -10,   -20,   -4,   -12,   -15,   -20 }
    },

    {   "bat",          "flap",         "flaps",
	{ 2,   2,   0,   -6,   4,   2,   0,   -18,   6,   -10,   -8,   -20 }
    },

    {   "imp",          "fly",          "flies",
	{ 4,   0,   6,   0,   2,   -8,   -12,   -15,   -8,   -8,   -2,   -20 }
    },

    {   "crab",         "scuttle",      "scuttles",
	{ 3,   -10,   -4,   -2,   0,   0,   -6,   4,   -2,   -16,   -12,   -20 }
    },

    {   "scorpion",     "crawl",        "crawls",
	{ 6,   -4,   -2,   8,   0,   0,   -10,   -10,   -18,   -8,   -12,   -20 }
    },

    {   "jackal",       "pounce",       "pounces",
	{ 8,   10,   0,   12,   6,   10,   0,   3,   -6,   -2,   -20,   -20 }
    },

    {   "eagle",        "launch",       "launches",
	{ 12,   20,   18,   15,   -4,   10,   -3,   0,   -20,   -10,   -4,   -20 }
    },

    {   "bear",         "lunge",        "lunges",
	{ 8,   4,   -6,   8,   2,   18,   6,   20,   0,   12,   -8,   -20 }
    },

    {   "wolf",         "leap",         "leaps",
	{ 16,   12,   10,   8,   16,   8,   2,   10,   -12,   0,   0,   -20 }
    },

    {   "tiger",        "dive",         "dives",
	{ 20,   12,   8,   2,   12,   12,   20,   -4,   8,   2,   0,   -20 }
    },

    {   "octopus",      "squelch",      "squelches",
	{ 20,   20,   20,   20,   20,   20,   20,   20,   20,   20,   20,   20 }
    },

};

/* item type list */
const struct item_type          item_table      []      =
{
    {   ITEM_CONT_LIGHT,     "contlight"         },
    {   ITEM_LIGHT,     "light"         },
    {   ITEM_SCROLL,    "scroll"        },
    {   ITEM_WAND,      "wand"          },
    {   ITEM_STAFF,     "staff"         },
    {   ITEM_WEAPON,    "weapon"        },
    {   ITEM_TREASURE,  "treasure"      },
    {   ITEM_ARMOR,     "armor"         },
    {   ITEM_POTION,    "potion"        },
    {   ITEM_CLOTHING,  "clothing"      },
    {   ITEM_FURNITURE, "furniture"     },
    {   ITEM_TRASH,     "trash"         },
    {   ITEM_CONTAINER, "container"     },
    {   ITEM_DRINK_CON, "drink"         },
    {   ITEM_KEY,       "key"           },
    {   ITEM_FOOD,      "food"          },
    {   ITEM_MONEY,     "money"         },
    {   ITEM_BOAT,      "boat"          },
    {   ITEM_CORPSE_NPC,"npc_corpse"    },
    {   ITEM_CORPSE_PC, "pc_corpse"     },
    {   ITEM_FOUNTAIN,  "fountain"      },
    {   ITEM_PILL,      "pill"          },
    {   ITEM_PROTECT,   "protect"       },
    {   ITEM_MAP,       "map"           },
    {   ITEM_PORTAL,    "portal"        },
    {   ITEM_WARP_STONE,"warp_stone"    },
    {   ITEM_ROOM_KEY,  "room_key"      },
    {   ITEM_GEM,       "gem"           },
    {   ITEM_JEWELRY,   "jewelry"       },
    {   ITEM_JUKEBOX,   "jukebox"       },
    {   ITEM_BODY_PART,     "body_part"         },
    {   ITEM_BODY_PART,     "body_part"         },
    {   ITEM_BODY_PART,     "body_part"         },
    {   ITEM_BODY_PART,     "body_part"         },
    {   ITEM_BODY_PART,     "body_part"         },
    {   ITEM_BODY_PART,     "body_part"         },
    {   ITEM_BODY_PART,     "body_part"         },
    {   ITEM_QUEST_CARD,"quest_card"    },
    {   ITEM_SPELL_BOOK,"spell_book"    },
    {   ITEM_BOW       ,"bow"           },
    {   ITEM_ARROW     ,"arrow"         },
    {   ITEM_TOME      ,"tome"          },
    {   ITEM_CAULDRON  ,"cauldron"      },
    {   ITEM_SPEC_POTION  ,"spec_potion"      },
    {   ITEM_ORBUS,     "orbus"         },
    {   ITEM_MAGIC_MIRROR,     "magic_mirror"         },
    {   ITEM_BODY_PART,     "body_part"         },
    {   ITEM_VOODOO_DOLL,     "voodoo_doll"         },
    {   ITEM_FEATHER,     "feather"         },
    {   ITEM_DRAGON_BLOOD,     "dragon_blood"         },
    {   ITEM_RAINBOW_POOL,     "rainbow_pool"         },
    {   ITEM_CASH,     "cash"         },
    {   ITEM_PEN_TOME,     "pen_tome"         },
    {   ITEM_BAUBLE,      "bauble"      },
    {   ITEM_EMPTY_VIAL,      "empty_vial"      },
    {   0,              NULL            }


};


/* weapon selection table */
const   struct  weapon_type     weapon_table    []      =
{
   { "sword",   OBJ_VNUM_SCHOOL_SWORD,  WEAPON_SWORD,   &gsn_sword      },
   { "mace",    OBJ_VNUM_SCHOOL_MACE,   WEAPON_MACE,    &gsn_mace       },
   { "dagger",  OBJ_VNUM_SCHOOL_DAGGER, WEAPON_DAGGER,  &gsn_dagger     },
   { "axe",     OBJ_VNUM_SCHOOL_AXE,    WEAPON_AXE,     &gsn_axe        },
   { "staff",   OBJ_VNUM_SCHOOL_STAFF,  WEAPON_SPEAR,   &gsn_spear      },
   { "flail",   OBJ_VNUM_SCHOOL_FLAIL,  WEAPON_FLAIL,   &gsn_flail      },
   { "whip",    OBJ_VNUM_SCHOOL_WHIP,   WEAPON_WHIP,    &gsn_whip       },
   { "polearm", OBJ_VNUM_SCHOOL_POLEARM,WEAPON_POLEARM, &gsn_polearm    },
   { NULL,      0,                              0,      NULL            }
};


 
/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {    "on",           WIZ_ON,         IM },
   {    "prefix",       WIZ_PREFIX,     IM },
   {    "ticks",        WIZ_TICKS,      IM },
   {    "logins",       WIZ_LOGINS,     IM },
   {    "sites",        WIZ_SITES,      L4 },
   {    "links",        WIZ_LINKS,      L7 },
   {    "newbies",      WIZ_NEWBIE,     IM },
   {    "spam",         WIZ_SPAM,       L5 },
   {    "deaths",       WIZ_DEATHS,     IM },
   {    "resets",       WIZ_RESETS,     L4 },
   {    "mobdeaths",    WIZ_MOBDEATHS,  L4 },
   {    "flags",        WIZ_FLAGS,      L5 },
   {    "penalties",    WIZ_PENALTIES,  L5 },
   {    "saccing",      WIZ_SACCING,    L5 },
   {    "levels",       WIZ_LEVELS,     IM },
   {    "load",         WIZ_LOAD,       L2 },
   {    "restore",      WIZ_RESTORE,    L2 },
   {    "snoops",       WIZ_SNOOPS,     L2 },
   {    "switches",     WIZ_SWITCHES,   L2 },
   {    "secure",       WIZ_SECURE,     L1 },
   {    NULL,           0,              0  }
};

/* attack table  -- not very organized :( */
const   struct attack_type      attack_table    [MAX_DAMAGE_MESSAGE]    =
{
    {   "none",         "hit",  "hits",        -1              },  /*  0*/
    {   "slice",        "slice","slices",        DAM_SLASH       },      
    {   "stab",         "stab","stabs",         DAM_PIERCE      },
    {   "slash",        "slash","slashes",        DAM_SLASH       },
    {   "whip",         "whip","whips",         DAM_SLASH       },
    {   "claw",         "claw","claws",       DAM_SLASH       },  /*  5*/
    {   "blast",        "blast","blasts",        DAM_BASH        },
    {   "pound",        "pound","pounds",        DAM_BASH        },
    {   "crush",        "crush","crushes",        DAM_BASH        },
    {   "grep",         "grep","greps",         DAM_SLASH       },
    {   "bite",         "bite","bites",    DAM_PIERCE      },  /* 10 */
    {   "pierce",       "pierce","pierces",   DAM_PIERCE      },
    {   "suction",      "suction","suction",      DAM_BASH        },
    {   "beating",      "beating","beats",      DAM_BASH        },
    {   "digestion",    "digestion","digests",    DAM_ACID        },
    {   "charge",       "charge","charges",     DAM_BASH        },  /*15*/
    {   "slap",         "slap","slaps",         DAM_BASH        },
    {   "punch",        "punch","punches",        DAM_BASH        },
    {   "wrath",        "wrath","wrath",        DAM_ENERGY      },
    {   "magic",        "magic","magic",        DAM_ENERGY      },
    {   "divine",       "divine power","divine power", DAM_HOLY        },/* 20 */
    {   "cleave",       "cleave","cleaves",       DAM_SLASH       },
    {   "scratch",      "scratch","scratches",      DAM_PIERCE      },
    {   "peck",         "peck","pecks",         DAM_PIERCE      },
    {   "peckb",        "peck","pecks",         DAM_BASH        },
    {   "chop",         "chop","chops",         DAM_SLASH       },  /* 25*/
    {   "sting",        "sting","stings",        DAM_PIERCE      },
    {   "smash",         "smash","smashes",       DAM_BASH        },
    {   "shbite",       "shocking bite","shocking bite",DAM_LIGHTNING   },
    {   "flbite",       "flaming bite","flaming bite", DAM_FIRE        },
    {   "frbite",       "freezing bite","freezing bite", DAM_COLD},/* 30 */
    {   "acbite",       "acidic bite","acidic bite",  DAM_ACID        },
    {   "chomp",        "chomp","chomps",        DAM_PIERCE      },
    {   "drain",        "life drain","life drains",   DAM_NEGATIVE    },
    {   "thrust",       "thrust","thrusts",       DAM_PIERCE      },
    {   "slime",        "slime","slimes",        DAM_ACID        },
    {   "shock",        "shock","shocks",        DAM_LIGHTNING   },
    {   "thwack",       "thwack","thwacks",       DAM_BASH        },
    {   "flame",        "flame","flames",        DAM_FIRE        },
    {   "chill",        "chill","chills",        DAM_COLD        },
    {   NULL,           NULL,  NULL,         0               }
};

/* race table */
const   struct  race_type       race_table      []              =
{
/*
    {
	name,           ishname,      pc_race?,
	act bits,       aff_by bits,    off bits,
	imm,            res,            vuln,
	form,           parts 
    },
*/
    { "unique",   "unique",      FALSE, 0, 0, 0, 0, 0, 0, 0, 0 },

    { 
	"human",  "human",              TRUE, 
	0,              0,              0,
	0,              0,              0,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"elf",    "elvish",              TRUE,
	0,              AFF_INFRARED,   0,
	0,              RES_CHARM,      VULN_IRON,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"half-elf",   "half-elvish",          TRUE,
	0,              AFF_INFRARED,   0,
	0,              RES_CHARM,      0,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"gnome",     "gnomish",           TRUE,
	0,              AFF_INFRARED,           0,
	0,              RES_POISON,             0,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"bat",    "bat",              FALSE,
	0,              AFF_FLYING|AFF_DARK_VISION,     OFF_DODGE|OFF_FAST,
	0,              0,              VULN_LIGHT,
	A|G|V,          A|C|D|E|F|H|J|K|P
    },

    {
	"bear",      "bear",           FALSE,
	0,              0,              OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
	0,              RES_BASH|RES_COLD,      0,
	A|G|V,          A|B|C|D|E|F|H|J|K|U|V
    },

    {
	"cat",         "feline",         FALSE,
	0,              AFF_DARK_VISION,        OFF_FAST|OFF_DODGE,
	0,              0,              0,
	A|G|V,          A|C|D|E|F|H|J|K|Q|U|V
    },

    {
	"centipede",      "centipede",      FALSE,
	0,              AFF_DARK_VISION,        0,
	0,              RES_PIERCE|RES_COLD,    VULN_BASH,
	A|B|G|O,                A|C|K   
    },

    {
	"dog",        "canine",          FALSE,
	0,              0,              OFF_FAST,
	0,              0,              0,
	A|G|V,          A|C|D|E|F|H|J|K|U|V
    },

    {
	"doll",             "doll",    FALSE,
	0,              0,              0,
	IMM_COLD|IMM_POISON|IMM_HOLY|IMM_NEGATIVE|IMM_MENTAL|IMM_DISEASE
	|IMM_DROWNING,  RES_BASH|RES_LIGHT,
	VULN_SLASH|VULN_FIRE|VULN_ACID|VULN_LIGHTNING|VULN_ENERGY,
	E|J|M|cc,       A|B|C|G|H|K
    },

    {   "dragon",       "dragon",        FALSE, 
	0,                      AFF_INFRARED|AFF_FLYING,        0,
	0,                      RES_FIRE|RES_BASH|RES_CHARM, 
	VULN_PIERCE|VULN_COLD,
	A|H|Z,          A|C|D|E|F|G|H|I|J|K|P|Q|U|V|X
    },

    {
	"fido",     "fido",            FALSE,
	0,              0,              OFF_DODGE|ASSIST_RACE,
	0,              0,                      VULN_MAGIC,
	A|B|G|V,        A|C|D|E|F|H|J|K|Q|V
    },          
   
    {
	"fox",          "fox",        FALSE,
	0,              AFF_DARK_VISION,        OFF_FAST|OFF_DODGE,
	0,              0,              0,
	A|G|V,          A|C|D|E|F|H|J|K|Q|V
    },

    {
	"goblin",       "goblin",        FALSE,
	0,              AFF_INFRARED,   0,
	0,              RES_DISEASE,    VULN_MAGIC,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"hobgoblin",    "hobgoblin",        FALSE,
	0,              AFF_INFRARED,   0,
	0,              RES_DISEASE|RES_POISON, 0,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|Y
    },

    {
	"kobold",       "kobold",        FALSE,
	0,              AFF_INFRARED,   0,
	0,              RES_POISON,     VULN_MAGIC,
	A|B|H|M|V,      A|B|C|D|E|F|G|H|I|J|K|Q
    },

    {
	"lizard",            "lizard",   FALSE,
	0,              0,              0,
	0,              RES_POISON,     VULN_COLD,
	A|G|X|cc,       A|C|D|E|F|H|K|Q|V
    },

    {
	"modron",       "modron",        FALSE,
	0,              AFF_INFRARED,           ASSIST_RACE|ASSIST_ALIGN,
	IMM_CHARM|IMM_DISEASE|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
			RES_FIRE|RES_COLD|RES_ACID,     0,
	H,              A|B|C|G|H|J|K
    },

    {
	"orc",         "orcish",         FALSE,
	0,              AFF_INFRARED,   0,
	0,              RES_DISEASE,    VULN_LIGHT,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"pig",          "pig",        FALSE,
	0,              0,              0,
	0,              0,              0,
	A|G|V,          A|C|D|E|F|H|J|K
    },  

    {
	"rabbit",       "rabbit",        FALSE,
	0,              0,              OFF_DODGE|OFF_FAST,
	0,              0,              0,
	A|G|V,          A|C|D|E|F|H|J|K
    },
    
    {
	"school monster",     "monster",  FALSE,
	ACT_NOALIGN,            0,              0,
	IMM_CHARM|IMM_SUMMON,   0,              VULN_MAGIC,
	A|M|V,          A|B|C|D|E|F|H|J|K|Q|U
    },  

    {
	"snake",      "reptilian",          FALSE,
	0,              0,              0,
	0,              RES_POISON,     VULN_COLD,
	A|G|X|Y|cc,     A|D|E|F|K|L|Q|V|X
    },
 
    {
	"song bird",       "avian",     FALSE,
	0,              AFF_FLYING,             OFF_FAST|OFF_DODGE,
	0,              0,              0,
	A|G|W,          A|C|D|E|F|H|K|P
    },

    {
	"troll",         "trollish",       FALSE,
	0,              AFF_REGENERATION|AFF_INFRARED|AFF_DETECT_HIDDEN,
	OFF_BERSERK,
	0,      RES_CHARM|RES_BASH,     VULN_FIRE|VULN_ACID,
	A|B|H|M|V,              A|B|C|D|E|F|G|H|I|J|K|U|V
    },

    {
	"water fowl",      "avian",     FALSE,
	0,              AFF_SWIM|AFF_FLYING,    0,
	0,              RES_DROWNING,           0,
	A|G|W,          A|C|D|E|F|H|K|P
    },          
  
    {
	"wolf",           "lupine",      FALSE,
	0,              AFF_DARK_VISION,        OFF_FAST|OFF_DODGE,
	0,              0,              0,      
	A|G|V,          A|C|D|E|F|J|K|Q|V
    },

    {
	"wyvern",          "wyvern",     FALSE,
	0,              AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,
	OFF_BASH|OFF_FAST|OFF_DODGE,
	IMM_POISON,     0,      VULN_LIGHT,
	A|B|G|Z,                A|C|D|E|F|H|J|K|Q|V|X
    },

    {
	"unique",          "unique",     FALSE,
	0,              0,              0,
	0,              0,              0,              
	0,              0
    },


    {
	NULL, NULL, 0, 0, 0, 0, 0, 0
    }
};

const   struct  pc_race_type    pc_race_table   []      =
{
    { "null race", "", 0, 
      { "" }, { 13, 13, 13, 13, 13, 13 }, { 18, 18, 18, 18, 18, 18 }, 0 },
 
/*
    {
	"race name",    short name,     points,
	{ bonus skills },
	{ base stats },         { max stats },          size 
    },
*/
    {
	"human",        "  Human  ",    0,  
	{ "common" },
	{ 13, 13, 13, 13, 13, 13 },     { 30, 30, 30, 30, 30, 30 },     SIZE_MEDIUM
    },

    {   
	"elf",          "   Elf   ",    0, 
	{ "sneak", "hide", "elvish" },
	{ 12, 14, 13, 15, 11, 14 },     { 30, 30, 30, 30, 30, 30 }, SIZE_MEDIUM
    },

    {
	"half-elf",     " HalfElf ",    0, 
	{ "sneak", "common", "elvish" },
	{ 13, 13, 13, 14, 12, 14 },     { 30,30,30,30,30,30 }, SIZE_MEDIUM
    },

    {
	"gnome",        "  Gnome  ",    0, 
	{ "meditation", "gnome" },
	{ 9, 16, 15, 14, 11,9 },        { 30,30,30,30,30,30 }, SIZE_SMALL
    }
};

	
	
/*
 * Attribute bonus tables.
 */
const   struct  str_app_type    str_app         [31]            =
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 130, 12 },
    {  0,  0, 130, 13 }, /* 13  */
    {  0,  1, 140, 14 },
    {  1,  1, 150, 15 }, /* 15  */
    {  1,  2, 165, 16 },
    {  2,  3, 180, 22 },
    {  2,  3, 200, 25 }, /* 18  */
    {  3,  4, 225, 30 },
    {  3,  5, 250, 35 }, /* 20  */
    {  4,  6, 300, 40 },
    {  4,  6, 350, 45 },
    {  5,  7, 400, 50 },
    {  5,  8, 450, 55 },
    {  6,  9, 500, 60 },  /* 25   */
    {  8,  10, 600, 65 },
    {  10,  12, 700, 70 },
    {  14,  16, 900, 75 },
    {  20,  22, 1200, 80 },
    {  30,  30, 1700, 90 }
};



const   struct  int_app_type    int_app         [31]            =
{
    {  3 },     /*  0 */
    {  5 },     /*  1 */
    {  7 },
    {  8 },     /*  3 */
    {  9 },
    { 10 },     /*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },     /* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },     /* 15 */
    { 34 },
    { 37 },
    { 40 },     /* 18 */
    { 44 },
    { 49 },     /* 20 */
    { 55 },
    { 60 },
    { 65 },
    { 70 },
    { 75 },     /* 25 */
    { 78 },
    { 81 },
    { 83 },
    { 85 },
    { 86 }
};



const   struct  wis_app_type    wis_app         [31]            =
{
    { 0 },      /*  0 */
    { 0 },      /*  1 */
    { 0 },
    { 0 },      /*  3 */
    { 0 },
    { 1 },      /*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 1 },      /* 10 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 2 },      /* 15 */
    { 2 },
    { 2 },
    { 3 },      /* 18 */
    { 3 },
    { 3 },      /* 20 */
    { 3 },
    { 4 },
    { 4 },
    { 4 },
    { 5 },      /* 25 */
    { 6 },
    { 7 },
    { 8 },
    { 9 },
    { 10 }
};


const   struct  cha_app_type    cha_app         [31]            =
{
    { 0 },      /*  0 */
    { 0 },      /*  1 */
    { 0 },
    { 0 },      /*  3 */
    { 0 },
    { 1 },      /*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 2 },      /* 10 */
    { 2 },
    { 2 },
    { 2 },
    { 3 },
    { 3 },      /* 15 */
    { 3 },
    { 4 },
    { 4 },      /* 18 */
    { 5 },
    { 6 },      /* 20 */
    { 7 },
    { 8 },
    { 10 },
    { 12 },
    { 14 },     /* 25 */
    { 16 },
    { 19 },
    { 22 },
    { 26 },
    { 30 }
};



const   struct  dex_app_type    dex_app         [31]            =
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 },    /* 25 */
    { -135 },
    { -150 },
    { -165 },
    { -190 },
    { -210 }
};


const   struct  con_app_type    con_app         [31]            =
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },   /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  1, 90 },   /* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },   /* 18 */
    {  3, 99 },
    {  4, 99 },   /* 20 */
    {  4, 99 },
    {  5, 99 },
    {  7, 99 },
    {  8, 99 },    /* 25 */
    {  9, 99 },
    {  10, 99 },
    {  11, 99 },
    {  12, 99 },
    {  14, 99 }
};



/*
 * Liquid properties.
 * Used in world.obj.
 */
const   struct  liq_type        liq_table       []      =
{
/*    name                      color   proof, full, thirst, food, ssize */
    { "water",                  "clear",        {   0, 1, 10, 0, 16 }   },
    { "beer",                   "amber",        {  12, 1,  8, 1, 12 }   },
    { "red wine",               "burgundy",     {  30, 1,  8, 1,  5 }   },
    { "ale",                    "brown",        {  15, 1,  8, 1, 12 }   },
    { "dark ale",               "dark",         {  16, 1,  8, 1, 12 }   },

    { "whisky",                 "golden",       { 120, 1,  5, 0,  2 }   },
    { "lemonade",               "pink",         {   0, 1,  9, 2, 12 }   },
    { "firebreather",           "boiling",      { 190, 0,  4, 0,  2 }   },
    { "local specialty",        "clear",        { 151, 1,  3, 0,  2 }   },
    { "slime mold juice",       "green",        {   0, 2, -8, 1,  2 }   },

    { "milk",                   "white",        {   0, 2,  9, 3, 12 }   },
    { "tea",                    "tan",          {   0, 1,  8, 0,  6 }   },
    { "coffee",                 "black",        {   0, 1,  8, 0,  6 }   },
    { "blood",                  "red",          {   0, 2, -1, 2,  6 }   },
    { "salt water",             "clear",        {   0, 1, -2, 0,  1 }   },

    { "coke",                   "brown",        {   0, 2,  9, 2, 12 }   }, 
    { "root beer",              "brown",        {   0, 2,  9, 2, 12 }   },
    { "elvish wine",            "green",        {  35, 2,  8, 1,  5 }   },
    { "white wine",             "golden",       {  28, 1,  8, 1,  5 }   },
    { "champagne",              "golden",       {  32, 1,  8, 1,  5 }   },

    { "mead",                   "honey-colored",{  34, 2,  8, 2, 12 }   },
    { "rose wine",              "pink",         {  26, 1,  8, 1,  5 }   },
    { "benedictine wine",       "burgundy",     {  40, 1,  8, 1,  5 }   },
    { "vodka",                  "clear",        { 130, 1,  5, 0,  2 }   },
    { "cranberry juice",        "red",          {   0, 1,  9, 2, 12 }   },

    { "orange juice",           "orange",       {   0, 2,  9, 3, 12 }   }, 
    { "absinthe",               "green",        { 200, 1,  4, 0,  2 }   },
    { "brandy",                 "golden",       {  80, 1,  5, 0,  4 }   },
    { "aquavit",                "clear",        { 140, 1,  5, 0,  2 }   },
    { "schnapps",               "clear",        {  90, 1,  5, 0,  2 }   },

    { "icewine",                "purple",       {  50, 2,  6, 1,  5 }   },
    { "amontillado",            "burgundy",     {  35, 2,  8, 1,  5 }   },
    { "sherry",                 "red",          {  38, 2,  7, 1,  5 }   },      
    { "framboise",              "red",          {  50, 1,  7, 1,  5 }   },
    { "rum",                    "amber",        { 151, 1,  4, 0,  2 }   },

    { "cordial",                "clear",        { 100, 1,  5, 0,  2 }   },
    { NULL,                     NULL,           {   0, 0,  0, 0,  0 }   }
};



/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n) n

const struct  skilltree_type  skill_tree   [MAX_NON_SPELL_SKILLS] =
{
    { &gsn_axe, SKILL_WEAPON, SKILL_WEAP_SLASH },
    { &gsn_sword, SKILL_WEAPON, SKILL_WEAP_SLASH },
    { &gsn_polearm, SKILL_WEAPON, SKILL_WEAP_SLASH },
    { &gsn_whip, SKILL_WEAPON, SKILL_WEAP_BASH },
    { &gsn_mace, SKILL_WEAPON, SKILL_WEAP_BASH },
    { &gsn_spear, SKILL_WEAPON, SKILL_WEAP_STAB },
    { &gsn_dagger, SKILL_WEAPON, SKILL_WEAP_STAB },
    { &gsn_flail, SKILL_WEAPON, SKILL_WEAP_BASH },
    { &gsn_hand_to_hand, SKILL_WEAPON, SKILL_WEAP_BASH },

    { &gsn_shield_block, SKILL_COMBAT, SKILL_COMB_DEFENSE},
    { &gsn_bash, SKILL_COMBAT, SKILL_COMB_ATTACK },
    { &gsn_dirt, SKILL_COMBAT, SKILL_COMB_ATTACK },
    { &gsn_disarm, SKILL_COMBAT, SKILL_COMB_AGILITY },
    { &gsn_dodge, SKILL_COMBAT, SKILL_COMB_AGILITY },
    { &gsn_kick, SKILL_COMBAT, SKILL_COMB_ATTACK },
    { &gsn_parry, SKILL_COMBAT, SKILL_COMB_DEFENSE },
    { &gsn_rescue, SKILL_COMBAT, SKILL_COMB_AGILITY },
    { &gsn_trip, SKILL_COMBAT, SKILL_COMB_ATTACK },
    { &gsn_off_hand, SKILL_COMBAT, SKILL_COMB_AGILITY },

    { &gsn_envenom, SKILL_THIEF, SKILL_THIE_ASSASSIN },
    { &gsn_haggle, SKILL_THIEF, SKILL_THIE_ROGUE },
    { &gsn_hide, SKILL_THIEF, SKILL_THIE_ROGUE },
    { &gsn_backstab, SKILL_THIEF, SKILL_THIE_ASSASSIN },
    { &gsn_peek, SKILL_THIEF, SKILL_THIE_ROGUE },
    { &gsn_pick_lock, SKILL_THIEF, SKILL_THIE_ROGUE },
    { &gsn_sneak, SKILL_THIEF, SKILL_THIE_ROGUE },
    { &gsn_steal, SKILL_THIEF, SKILL_THIE_ROGUE },
    { &gsn_tumbling, SKILL_THIEF, SKILL_THIE_ROGUE },
    { &gsn_dagger_throwing, SKILL_THIEF, SKILL_THIE_ASSASSIN },

    { &gsn_fast_healing, SKILL_PRIEST, SKILL_PRIE_HEALING },
    { &gsn_meditation, SKILL_PRIEST, SKILL_PRIE_HEALING },
    { &gsn_foraging, SKILL_PRIEST, SKILL_PRIE_CRAFT },

    { &gsn_scrolls, SKILL_WIZARD, SKILL_WIZA_ITEMS },
    { &gsn_staves, SKILL_WIZARD, SKILL_WIZA_ITEMS },
    { &gsn_wands, SKILL_WIZARD, SKILL_WIZA_ITEMS },
    { &gsn_lore, SKILL_WIZARD, SKILL_WIZA_ITEMS },

    { &gsn_blacksmith, SKILL_KNIGHT, SKILL_KNIG_FORGING },
    { &gsn_blindfighting, SKILL_KNIGHT, SKILL_KNIG_COMBAT },
    { &gsn_enhanced_damage, SKILL_KNIGHT, SKILL_KNIG_COMBAT },
    { &gsn_berserk, SKILL_KNIGHT, SKILL_KNIG_COMBAT }
};


const   struct  skill_type      skill_table     [MAX_SKILL]     =
{

/*
 * Magic spells.
 */

    {
	"reserved",             { 99, 99, 99, 99 },     { 99, 99, 99, 99},
	0,                      TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT( 0),        0,      0,
	"",                     "",             "","",0,0,0,""
    },

    {
	"acid blast",           { 1, 1, 1, 1 },         { 1,  1,  2,  2},
	spell_acid_blast,       TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(70),       50,     12,
	"acid blast",           "!Acid Blast!","","",4,0,5,""
    },

    {
	"burning hands",        { 1, 1, 1, 1 },         { 1,  1,  2,  2},
	spell_burning_hands,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(631),      25,     12,
	"flame",                "!Burning Hands!","","",1,0,1,"chill touch"
    },

    {
	"armour",                { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_armor,            TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT( 1),        25,    12,
	"",                     "You feel less armoured.","","",4,0,1,"glitterdust"
    },

    {
	"grounding",            { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_grounding,        TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(655),       60,    12,
	"",                     "You feel less protected.","","",4,0,8,"storm herald"
    },

    {
	"resist cold",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_resist_cold,      TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(656),       15,    12,
	"",                     "Your skin feels warmer.","","",1,0,4,""
    },

    {
	"storm herald",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_storm_herald,     TAR_CHAR_OFFENSIVE,     POS_STANDING,
	NULL,                   SLOT(657),       15,    12,
	"",                     "You feel safer.","","",1,0,5,"grounding"
    },

    {
	"water breathing",      { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_water_breathing,  TAR_IGNORE,     POS_STANDING,
	NULL,                   SLOT( 628 ),     75,    12,
	"","Your gills slide back into your skin.","","",3,0,4,""
    },

    {
	"soul blade",           { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_soul_blade,       TAR_IGNORE,     POS_STANDING,
	NULL,                   SLOT( 626 ),     250,   12,
	"",                     "You create a soul blade.","","",4,0,4,""
    },

    {
	"animate dead",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_animate_dead,     TAR_OBJ_ROOM,        POS_STANDING,
	NULL,                   SLOT(672),       60,     24,
	"",                     "!Identify!",           "","",5,0,7,""
    },

    {
	"flame blade",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_flame_blade,      TAR_IGNORE,     POS_STANDING,
	NULL,                   SLOT( 625 ),     350,   12,
	"",                     "You create a flame blade.","","a precious fire opal",1,50,8,""
    },

    {
	"shadow blade",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_shadow_blade,     TAR_IGNORE,     POS_STANDING,
	NULL,                   SLOT( 624 ),     500,   12,
	"",                     "You create a shadow blade.","","the heart of the spectre",5,80,9,""
    },

    {
	"windtomb",             { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_windtomb,         TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(607),       120,   18,
	"",             "The winds around you die down.","","",3,0,9,""
    },

    {
	"reset",                { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_reset,            TAR_OBJ_CHAR_DEF,       POS_STANDING,
	NULL,                   SLOT(617),       80,    12,
	"",                     "You feel like your usual self again.", 
	"","",2,0,7,""
    },

    {
	"bless",                { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_bless,            TAR_OBJ_CHAR_DEF,       POS_STANDING,
	NULL,                   SLOT( 3),        10,    12,
	"",                     "You feel less righteous.", 
	"$p's holy aura fades.","",2,0,1,"curse"
    },

    {
	"cry of anger",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cry_of_anger,     TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT( 610),      150,   12,
	"",                     "The anger within you subsides.", 
	"$p's anger subsides.","",1,0,7,""
    },
    
    {
	"accuracy",             { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_accuracy,         TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT( 662),      20,   12,
	"",                     "You feel less accurate in combat.", 
	"","",1,0,5,""
    },


    {
	"blindness",            { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_blindness,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	&gsn_blindness,         SLOT( 4),        25,    12,
	"",                     "You can see again.",   "","",5,0,3,"cure blindness"
    },

    {
	"ballistic attack",     { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_ballistic_attack, TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT( 611),     5,      12,
	"stone",                "!Ballistic Attack!",   "","",4,0,1,""
    },

    {
	"call lightning",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_call_lightning,   TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT( 6),       50,     12,
	"lightning bolt",       "!Call Lightning!",     "","",3,0,6,""
    },

    {   "calm",                 { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_calm,             TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(509),      40,     12,
	"",                     "You have lost your peace of mind.",    "","",2,0,8,""
    },

    {
	"cancellation",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cancellation,     TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(507),      20,     12,
	"",                     "!Cancellation!",       "","",4,0,5,""
    },

    {
	"cause critical",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cause_critical,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(63),       20,     12,
	"spell",                "!Cause Critical!",     "","",5,0,4,"cure critical"
    },

    {
	"cause light",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cause_light,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(62),       15,     12,
	"spell",                "!Cause Light!",        "","",5,0,1,"cure light"
    },

    {
	"dark pact",            { 1, 1, 1, 1 },     	{ 1,  1,  2,  2},
	spell_dark_pact,      TAR_CHAR_SELF,     	POS_STANDING,
	NULL,                   SLOT(668),       15,     12,
	"spell",                "Bum bum bum",        "","",5,0,1,""
    },

    {
	"cause serious",        { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cause_serious,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(64),       17,     12,
	"spell",                "!Cause Serious!",      "","",5,0,2,"cure serious"
    },

    {   
	"chain lightning",      { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_chain_lightning,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(500),      80,     12,
	"lightning",            "!Chain Lightning!",    "","an amber rod",3,75,7,""
    }, 

    {
	"change sex",           { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_change_sex,       TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(82),       15,     12,
	"",                     "Your body feels familiar again.",      "","",2,0,4,""
    },

    {
	"cockroach",            { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cockroach,        TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(616),      200,    12,
	"transformation",               "Your body feels familiar again.",      "","a live cockroach",5, 30,8,""
    },

    {
	"frog",                 { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_frog,             TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(620),      200,    12,
	"transformation",               "Your body feels familiar again.",      "","a tadpole",4,15,9,""
    },

    {
	"glitterdust",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_glitterdust,      TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(614),      15,     12,
	"",                     "The swirling lights blink rapidly and then vanish.",   "","",4,0,2,"armour"
    },


    {
	"charm person",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_charm_person,     TAR_CHAR_DEFENSIVE,     POS_STANDING,
	&gsn_charm_person,      SLOT( 7),        15,    12,
	"",                     "You feel more self-confident.","","",1,0,5,""
    },

    {
	"chill touch",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_chill_touch,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT( 8),       15,     12,
	"chilling touch",       "You feel less cold.",  "","",5,0,1,"burning hands"
    },

    {
	"colour spray",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_colour_spray,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(10),       35,     12,
	"colour spray",         "!Colour Spray!",       "","",3,0,4,""
    },

    {
	"continual light",      {  1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_continual_light,  TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(57),        7,     12,
	"",                     "!Continual Light!",    "","",1,0,2,""
    },

    {
	"conflaguration",       {  1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_conflaguration,   TAR_CHAR_OFFENSIVE,     POS_STANDING,
	&gsn_conflaguration,    SLOT(603),       200,   12,
	"hellfire",             "The fires surrounding you vanish.",    "","oil of burning",1,75,9,""
    },

    {
	"control weather",      { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_control_weather,  TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(11),       20,     12,
	"",                     "!Control Weather!",    "","",3,0,5,""
    },

    {
	"heroes feast",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_heroes_feast,     TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(12),        200,   12,
	"",                     "!Heroes Feast!",       "","",2,0,9,""
    },

    {
	"create food",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_create_food,      TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(12),        5,     12,
	"",                     "!Create Food!",        "","",4,0,1,""
    },

    {
	"create rose",          { 1, 1, 1, 1 },         { 1, 1, 2, 2 },
	spell_create_rose,      TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(511),      10,     12,
	"",                     "!Create Rose!",        "","",4,0,2,""
    },  

    {
	"create spring",        { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_create_spring,    TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(80),       7,      12,
	"",                     "!Create Spring!",      "","",2,0,3,""
    },

    {
	"create water",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_create_water,     TAR_OBJ_INV,            POS_STANDING,
	NULL,                   SLOT(13),        5,     12,
	"",                     "!Create Water!",       "","",2,0,1,""
    },

    {
	"cure blindness",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cure_blindness,   TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(14),        12,    12,
	"",                     "!Cure Blindness!",     "","",2,0,2,""
    },

    {
	"cure critical",        { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cure_critical,    TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(15),       20,     12,
	"",                     "!Cure Critical!",      "","",2,0,5,"cause critical"
    },

    {
	"cure disease",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cure_disease,     TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(501),      18,     12,
	"",                     "!Cure Disease!",       "","",2,0,4,"plague"
    },

    {
	"remove fear",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_remove_fear,      TAR_CHAR_DEFENSIVE,     POS_STANDING,
	&gsn_fear,              SLOT(646),      20,     12,
	"",                     "!Remove Fear!",        "","",2,0,4,"fear"
    },

    {
	"cure light",           { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cure_light,       TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(16),       10,     12,
	"",                     "!Cure Light!",         "","",2,0,1,"cause light"
    },

    {
	"cure poison",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cure_poison,      TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(43),        12,    12,
	"",                     "!Cure Poison!",        "","",2,0,2,"poison"
    },

    {
	"cure serious",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cure_serious,     TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(61),       15,     12,
	"",                     "!Cure Serious!",       "","",2,0,3,"cause serious"
    },

    {
	"curse",                { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_curse,            TAR_OBJ_CHAR_OFF,       POS_FIGHTING,
	&gsn_curse,             SLOT(17),       15,     12,
	"curse",                "The curse wears off.", 
	"$p is no longer impure.","",5,0,3,"remove curse"
    },

    {
	"demonfire",            { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_demonfire,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(505),      40,     12,
	"torments",             "!Demonfire!",          "","",5,0,6,""
    },  

    {
	"horrid wilting",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_horrid_wilting,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(612),      125,    12,
	"dehydration",          "!Horrid Wilting!",             "","",3,0,9,""
    },  

    {
	"quench",               { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_quench,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(613),      125,    12,
	"quench",               "!Quench!",             "","a glass of water",2,50,8,""
    },  

    {
	"detect evil",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_detect_evil,      TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(18),        5,     12,
	"",                     "The red in your vision disappears.",   "","",3,0,2,"detect good"
    },

    {
	"detect good",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_detect_good,      TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(513),        5,     12,
	"",                     "The gold in your vision disappears.",  "","",3,0,2,"detect evil"
    },

    {
	"detect hidden",        { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_detect_hidden,    TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(44),        5,     12,
	"",                     "You feel less aware of your surroundings.",    
	"","",3,0,3,""
    },

    {
	"detect invis",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_detect_invis,     TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(19),        5,     12,
	"",                     "You no longer see invisible objects.",
	"","",3,0,3,""
    },

    {
	"detect magic",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_detect_magic,     TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(20),        5,     12,
	"",                     "The detect magic wears off.",  "","",3,0,1,""
    },

    {
	"detect poison",        { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_detect_poison,    TAR_OBJ_INV,            POS_STANDING,
	NULL,                   SLOT(21),        5,     12,
	"",                     "!Detect Poison!",      "","",3,0,2,""
    },

    {
	"dispel evil",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_dispel_evil,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(22),       35,     12,
	"dispel evil",          "!Dispel Evil!",        "","",6,0,4,"dispel good"
    },

    {
	"dispel good",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_dispel_good,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(512),      20,     12,
	"dispel good",          "!Dispel Good!",        "","",2,0,4,"dispel evil"
    },

    {
	"dispel magic",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_dispel_magic,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(59),       22,     12,
	"",                     "!Dispel Magic!",       "","",4,0,3,""
    },

    {
	"transform dragon",     { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_dragon_trans,     TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(600),      500,    40,
	"transformation",       "The transformation is over.",  "","an ebony orb",1,100,9,""
    },

    {
	"earthquake",           { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_earthquake,       TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(23),       15,     12,
	"earthquake",           "!Earthquake!",         "","",4,0,6,""
    },

    {
	"star storm",           { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_star_storm,       TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(627),      500,    12,
	"storm",                "!Star Storm!",         "","a firefly",1,25,10,""
    },

    {
	"sunbeam",              { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_sunbeam,          TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(651),      100,    12,
	"sunlight",             "!Sunbeam!",            "","",1,0,8,""
    },

    {
	"rock to mud",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_rock_to_mud,      TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(648),      50,     20,
	"transmutation",        "!transmutation!",      "","",4,0,6,""
    },

    {
	"enchant armor",        { 1, 1, 1, 1 }, { 2,  2,  4,  4 },
	spell_enchant_armor,    TAR_OBJ_INV,            POS_STANDING,
	NULL,                   SLOT(510),      100,    24,
	"",                     "!Enchant Armor!",      "","",1,0,6,""
    },

    {
	"enchant weapon",       { 1, 1, 1, 1 },     { 2,  2, 4,  4},
	spell_enchant_weapon,   TAR_OBJ_INV,            POS_STANDING,
	NULL,                   SLOT(24),       100,    24,
	"",                     "!Enchant Weapon!",     "","",1,0,6,""
    },

    {
	"devour soul",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_devour_soul,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(615),      75,     12,
	"vileness",             "!Devour Soul!",        "","",5,0,7,""
    },


    {
	"energy drain",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_energy_drain,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(25),       35,     12,
	"energy drain",         "!Energy Drain!",       "","",5,0,5,""
    },

    {
	"faerie fire",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_faerie_fire,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(72),        8,     12,
	"faerie fire",          "The pink aura around you fades away.",
	"","",1,0,3,""
    },

    {
	"fear",                 { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_fear,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(644),       180,   12,
	"fear",                 "Your fear subsides.",
	"","an ebony mask",5,10,8,"remove fear"
    },

    {
	"all hallows eve",      { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_allhallowseve,    TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(645),       750,   12,
	"fear",                 "Your fear subsides.",
	"","a pumpkin",5,95,10,""
    },

    {
	"bloodlust",            { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_bloodlust,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(72),        30,    12,
	"lust for blood",       "Your bloodlust subsides.",
	"","",2,0,6,""
    },

    {
	"faerie fog",           { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_faerie_fog,       TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(73),       12,     12,
	"faerie fog",           "!Faerie Fog!",         "","",3,0,3,""
    },

    {
	"farsight",             { 1, 1, 1, 1 }, { 1, 1, 2, 2},
	spell_farsight,         TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(521),      36,     20,
	"farsight",             "!Farsight!",           "","",3,0,2,""
    },  

    {
	"fireball",             { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_fireball,         TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(26),       15,     12,
	"fireball",             "!Fireball!",           "","",1,0,3,""
    },
  
    {
	"fireproof",            { 1, 1, 1, 1 }, { 1, 1, 2, 2},
	spell_fireproof,        TAR_OBJ_INV,            POS_STANDING,
	NULL,                   SLOT(523),      10,     12,
	""      ,       "$p's protective aura fades.","","",1,0,1,""
    },

    {
	"flamestrike",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_flamestrike,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(65),       20,     12,
	"flamestrike",          "!Flamestrike!",        "","",1,0,5,""
    },

    {
	"fly",                  { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_fly,              TAR_CHAR_DEFENSIVE,     POS_STANDING,
	&gsn_fly,                   SLOT(56),       30,     18,
	"",                     "You slowly float to the ground.",      "","",3,0,5,""
    },
    
    {
	"jump",                  { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_jump,              TAR_CHAR_DEFENSIVE,     POS_STANDING,
	&gsn_jump,                   SLOT(660),       5,     18,
	"",                     "You land feet first on the ground.","$n lands back on the ground.","",3,0,3,""
    },


    {
	"floating disc",        { 1, 1, 1, 1 }, { 1, 1, 2, 2},
	spell_floating_disc,    TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(522),      40,     24,
	"",                     "!Floating disc!",      "","",3,0,1,""
    },

    {
	"frenzy",               { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_frenzy,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(504),      30,     24,
	"",                     "Your rage ebbs.",      "","",1,0,6,""
    },

    {
	"gate",                 { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_gate,             TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(83),       80,     12,
	"",                     "!Gate!",               "","",4,0,7,""
    },

    {
	"giant strength",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_giant_strength,   TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(39),       20,     12,
	"",                     "You feel weaker.",     "","",4,0,2,"weaken"
    },

    {
	"harm",                 { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_harm,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(27),       35,     12,
	"harm spell",           "!Harm!",               "","",5,0,5,"heal"
    },
  
    {
	"haste",                { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_haste,            TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(502),      30,     12,
	"",                     "You feel yourself slow down.", "","",3,0,5,"slow"
    },

    {
	"spring of life",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_spring_of_life,   TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(659),      90,     12,
	"",                     "!Spring of Life!",             "","",2,0,9,""
    },
  
    {
	"heal",                 { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_heal,             TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(28),       50,     12,
	"",                     "!Heal!",               "","",2,0,6,"harm"
    },
  
    {
	"heat metal",           { 1, 1, 1, 1 }, { 1, 1, 2, 2 },
	spell_heat_metal,       TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(516),      25,     18,
	"spell",                "!Heat Metal!",         "","",1,0,3,""
    },

    {
	"hold person",          { 1, 1, 1, 1 }, { 1,  1,  2,  2},
	spell_hold_person,      TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	&gsn_holdperson,        SLOT(609),      120,    24,
	"",                     "The stiffness in your joints relaxes.","","",4,0,5,""
    },

    {
	"holy word",            { 1, 1, 1, 1 }, { 2,  2,  4,  4},
	spell_holy_word,        TAR_IGNORE,     POS_FIGHTING,
	NULL,                   SLOT(506),      200,    24,
	"divine wrath",         "!Holy Word!",          "","",2,0,8,""
    },

    {
	"hunger",               { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_hunger,           TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(605),      35,     12,
	"hunger",               "!Hunger!",             "","",5,0,2,""
    },

    {
	"lure of darkness",     { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_lure_of_darkness, TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(606),      35,     12,
	"",                     "!Lure Of Darkness!",           "","a wolf's paw",5,5,6,""
    },


    {
	"identify",             { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_identify,         TAR_OBJ_INV,            POS_STANDING,
	NULL,                   SLOT(53),       30,     24,
	"",                     "!Identify!",           "","bat ichor",4,10,7,""
    },

    {
	"infravision",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_infravision,      TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(77),        5,     18,
	"",                     "You no longer see in the dark.",       "","",4,0,3,""
    },

    {
	"stone grip",           { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_stone_grip,       TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(647),       25,    12,
	"",                     "Your grip on your weapon releases.",           
	"","",4,0,6,""
    },

    {
	"invisibility",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_invis,            TAR_OBJ_CHAR_DEF,       POS_STANDING,
	&gsn_invis,             SLOT(29),        5,     12,
	"",                     "You are no longer invisible.",         
	"$p fades into view.","",3,0,3,""
    },
    
    {
	"regeneration",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_regeneration,            TAR_CHAR_DEFENSIVE,       POS_STANDING,
	NULL,             SLOT(661),        75,     12,
	"",                     "Your accelerated healing stops.",         
	"","",4,0,9,""
    },


    {
	"improved invis",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_improved_invis,   TAR_CHAR_DEFENSIVE,     POS_STANDING,
	&gsn_improved_invis,    SLOT(630),       50,    12,
	"",                     "You are no longer invisible.",         
	"$p fades into view.","",3,0,6,""
    },

    {
	"iron skin",            { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_iron_skin,        TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(601),      75,     18,
	"",                     "Your skin feels softer.",      "","",4,0,7,""
    },

    {
	"know alignment",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_know_alignment,   TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(58),        19,    12,
	"",                     "!Know Alignment!",     "","",4,0,3,""
    },
    
    {
	"magma burst",          { 1, 1, 1, 1 },         { 1,  1,  2,  2},
	spell_magma_burst,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(658),      60,     12,
	"burst of magma",       "!Magma Burst!","","",1,0,6,""
    },

    {
	"lava burst",           { 1, 1, 1, 1 },         { 1,  1,  2,  2},
	spell_lava_burst,       TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(602),      60,     12,
	"burst of lava",        "!Lava Burst!","","",1,0,6,""
    },

    {
	"lightning bolt",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_lightning_bolt,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(30),       20,     12,
	"lightning bolt",       "!Lightning Bolt!",     "","a piece of fur",3,5,3,""
    },

    {
	"locate object",        { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_locate_object,    TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(31),       20,     18,
	"",                     "!Locate Object!",      "","",1,0,2,""
    },

    {
	"magic missile",        { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_magic_missile,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(32),       6,      12,
	"magic missile",        "!Magic Missile!",      "","",3,0,1,""
    },

    {
	"trip",                 { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_trip,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(621),      20,     0,
	"trip",                 "!Trip!",               "","",4,0,4,""
    },

    {
	"mass healing",         { 1, 1, 1, 1 }, { 2,  2,  4,  4},
	spell_mass_healing,     TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(508),      100,    36,
	"",                     "!Mass Healing!",       "","",2,0,7,""
    },

    {
	"mass invis",           { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_mass_invis,       TAR_IGNORE,             POS_STANDING,
	&gsn_mass_invis,        SLOT(69),       20,     24,
	"",                     "You are no longer invisible.",         "","",3,0,8,""
    },


    {
	"pass door",            { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_pass_door,        TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(74),       35,     12,
	"",                     "You feel solid again.",        "","",4,0,5,""
    },

    {
	"plague",               { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_plague,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	&gsn_plague,            SLOT(503),      20,     12,
	"sickness",             "Your sores vanish.",   "","",5,0,4,"cure disease"
    },
    
    {
	"pestilence",           { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_pestilence,       TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	&gsn_pestilence,        SLOT(663),      60,     12,
	"disease",              "Your disease vanishes.",   "","",5,0,7,"cure disease"
    },

    {
	"hideous laughter",     { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_hideous_laughter, TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(650),      20,     12,
	"laughter",             "You dont find it funny any more.","","",3,0,1,""
    },

    {
	"poison",               { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_poison,           TAR_OBJ_CHAR_OFF,       POS_FIGHTING,
	&gsn_poison,            SLOT(33),       10,     12,
	"poison",               "You feel less sick.",  
	"The poison on $p dries up.","",5,0,2,"cure poison"
    },

    {
	"portal",               { 1, 1, 1, 1 },     { 2,  2,  4,  4}, 
	spell_portal,           TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(519),       100,     24,
	"",                     "!Portal!",             "","",4,0,8,""
    },

    {
	"protection evil",      { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_protection_evil,  TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(34),       5,      12,
	"",                     "You feel less protected.",     "","",2,0,3,"protection good"
    },

    {
	"protection good",      { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_protection_good,  TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(514),       5,     12,
	"",                     "You feel less protected.",     "","",5,0,3,"protection evil"
    },

    {
	"power word kill",      { 1, 1, 1, 1 },     { 1, 1, 2, 2},
	spell_power_word_kill,  TAR_CHAR_OFFENSIVE,     POS_STANDING,
	NULL,                   SLOT(608),      500,    4,
	"word",         "You oblige.",  "","a rose",5,95, 10,"raise dead"
    },

    {
	"ray of truth",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_ray_of_truth,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(518),      40,     12,
	"ray of truth",         "!Ray of Truth!",       "","",1,0,7,""
    },

    {
	"recharge",             { 1, 1, 1, 1 },         { 1, 1, 2, 2 },
	spell_recharge,         TAR_OBJ_INV,            POS_STANDING,
	NULL,                   SLOT(517),      60,     24,
	"",                     "!Recharge!",           "","",1,0,4,""
    },

    {
	"refresh",              { 1, 1, 1, 1 },      { 1,  1,  2,  2},
	spell_refresh,          TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(81),       12,     18,
	"refresh",              "!Refresh!",            "","",2,0,2,""
    },

    {
	"remove curse",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_remove_curse,     TAR_OBJ_CHAR_DEF,       POS_STANDING,
	NULL,                   SLOT(35),        15,    12,
	"",                     "!Remove Curse!",       "","poppy seeds",2,40,5,"curse"
    },

    {
	"sanctuary",            { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_sanctuary,        TAR_CHAR_DEFENSIVE,     POS_STANDING,
	&gsn_sanctuary,         SLOT(36),       75,     12,
	"",                     "The white aura around your body dissipates.",
	"","deadly nightshade",6,40,5,""
    },

    {
	"shield",               { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_shield,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(67),       12,     18,
	"",                     "Your force shield glows suddenly then is gone.",
	"","",3,0,4,""
    },

    {
	"shocking grasp",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_shocking_grasp,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(53),       15,     12,
	"shocking grasp",       "!Shocking Grasp!",     "","",3,0,2,""
    },

    {
	"sleep",                { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_sleep,            TAR_CHAR_OFFENSIVE,     POS_STANDING,
	&gsn_sleep,             SLOT(38),       15,     12,
	"",                     "You feel less drowsy.",        "","some sand",2,45,3,""
    },

    {
	"slow",                 { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_slow,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(515),      30,     12,
	"",                     "You feel yourself speed up.",  "","",4,0,5,"haste"
    },

    {
	"stone skin",           { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_stone_skin,       TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(66),       12,     18,
	"",                     "Your skin regains its normal texture.",        "","",4,0,4,""
    },

    {
	"summon",               { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_summon,           TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(40),       50,     12,
	"",                     "!Summon!",             "","",4,0, 6,""
    },

    {
	"black tentacles",      { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_evards_tenticles, TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(643),      200,    12,
	"",                     "!Summon!",             "","a black tentacle",5,30,7,""
    },
/*
    {
	"find familiar",        { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_find_familiar,    TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(654),      100,    12,
	"",                     "!Find Familiar!",      "","a silver whistle",4,0,1,""
    },
*/
    {
	"wyvern watch",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_wyvern_watch,     TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(623),      150,    12,
	"",                     "!Summon!",             "","",1,0,3,""
    },

    {
	"summon flame",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_clfe,             TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(635),      175,    12,
	"",                     "!Summon!",             "","a necklace of flames",1,90,7,""
    },

    {
	"summon wave",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_clwe,             TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(636),      175,    12,
	"",                     "!Summon!",             "","a necklace of waves",2,90,7,""
    },

    {
	"summon whirlwind",     { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_clae,             TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(637),      175,    12,
	"",                     "!Summon!",             "","a necklace of wind",3,90,7,""
    },

    {
	"summon giant",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_clee,             TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(638),      175,    12,
	"",                     "!Summon!",             "","a necklace of stones",4,90,7,""
    },

    {
	"summon inferno",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cgfe,             TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(639),      900,    30,
	"",                     "!Summon!",             "","a candle",1,90,10,""
    },

    {
	"summon tsunami",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cgwe,             TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(640),      900,    30,
	"",                     "!Summon!",             "","a bowl",2,90,10,""
    },

    {
	"summon tornado",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cgae,             TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(641),      900,    30,
	"",                     "!Summon!",             "","a censer",3,90,10,""
    },

    {
	"summon colossus",      { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_cgee,             TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(642),      900,    30,
	"",                     "!Summon!",             "","black powder",4,90,10,""
    },

    {
	"bind daemon",          { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_bind_daemon,      TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(632),      650,    12,
	"",                     "!Bind Daemon!",        "","a gilded iron cage",5,75,8,""
    },

    {
	"pentagram",            { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_pentagram,        TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(633),      350,    12,
	"",                     "The chalk circle fades away.", "","a stick of chalk",5,100,7,""
    },

    {
	"teleport",             { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_teleport,         TAR_CHAR_SELF,          POS_FIGHTING,
	NULL,                   SLOT( 2),       35,     12,
	"",                     "!Teleport!",           "","",4,0,8,""
    },

    {
	"thirst",               { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_thirst,           TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(604),      35,     12,
	"",                     "!Thirst!",             "","",5,0,2,""
    },

    {
	"ventriloquate",        {  1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_ventriloquate,    TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(41),        5,     12,
	"",                     "!Ventriloquate!",      "","",3,0,1,""
    },

    {
	"phantasmal force",     {  1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_phant_force,      TAR_CHAR_OFFENSIVE,     POS_STANDING,
	NULL,                   SLOT(652),       10,    12,
	"",                     "!Phant force!",        "","",3,0,2,""
    },

    {
	"improved phantasm",    {  1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_imp_phant_force,  TAR_CHAR_OFFENSIVE,     POS_STANDING,
	NULL,                   SLOT(653),       20,    12,
	"",                     "!Imp phant force!",    "","",3,0,4,""
    },

    {
	"weaken",               { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_weaken,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(68),       20,     12,
	"spell",                "You feel stronger.",   "","",5,0,2,"giant strength"
    },

    {
	"wish",                 { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_wish,             TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(618),      950,    12,
	"wish",                 "You wish.",            "","",2,0,10,""
    },

    {
	"consultation",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_consultation,     TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(634),      500,    12,
	"consultation",                 "You consult.",         "","",5,0,9,""
    },

    {
	"winds of caina",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_winds_caina,      TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(622),      450,    12,
	"winds",                "!Winds of Caina!.",    "","an unmelting snowflake",3, 95, 10,""
    },

    {
	"collapse",             { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_collapse,         TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(649),      600,    12,
	"collapse",             "!Collapse!.",  "","a shadow",5, 95, 9,""
    },

    {
	"time twister",         { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_time_twister,     TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(622),      450,    12,
	"twist",                "Time flows forward once more.","","an hour hand",3, 75, 9,""
    },

    {
	"harmony",              { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_harmony,          TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(629),      600,    12,
	"harmony",              "!Harmony!.",           "","a yellow rose",2, 80, 10,""
    },

    {
	"arachnophobia",        { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_arachnophobia,    TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(619),      600,    12,
	"glorious transformation",      "MUHAHAHAHAHAHA",       "","a live spider",5, 99, 10,""
    },

    {
	"word of recall",       { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_word_of_recall,   TAR_CHAR_SELF,          POS_RESTING,
	NULL,                   SLOT(42),        5,     12,
	"",                     "!Word of Recall!",     "","",3,0, 7,""
    },
    
    {
	"power word stun",      { 1, 1, 1, 1 },     { 1, 1, 2, 2},
	spell_power_word_stun,  TAR_CHAR_OFFENSIVE,     POS_STANDING,
	NULL,                   SLOT(664),      200,    4,
	"word",         "You are not stunned any more.",  "","a blue rose",1,95, 9,""
    },
    
    {
	"deafness",                 { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_deafness,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                       SLOT(665),       40,   12,
	"deafness",                 "You can hear again.",
	"","",5,0,6,""
    },

    {
	"wither",                  { 1, 1, 1, 1 },     { 1,  1,  2,  2},
	spell_wither,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                       SLOT(666),       120,   12,
	"withering",                 "!Wither!",
	"","",5,0,6,""
    },

    {
	"reflection",                  { 1, 1, 1, 1 },     { 1,  1,  2,2},
	spell_reflection,             TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                       SLOT(667),       60,   12,
	"withering",                 "Your reflective shield fades.",
	"$n's reflective shield fades away.","",1,0,6,""
    },

    {
	"mummy rot",                  { 1, 1, 1, 1 },     { 1,  1,  2,2},
	spell_mummy_rot,             TAR_CHAR_OFFENSIVE,POS_FIGHTING,
	&gsn_mummy_rot,                       SLOT(668),       75,   12,
	"withering",                 "The rotting disease fades away.",
	"$n rotting disease stops suddenly.","",5,0,8,""
    },

    {
	"disintegrate",                  { 1, 1, 1, 1 },     { 1,  1,  2,2},
	spell_disintegrate,             TAR_CHAR_OFFENSIVE,POS_FIGHTING,
	NULL,                       SLOT(669),       750,   12,
	"disintegrate",                 "",
	"$n gets better.","a lodestone",1,95,10,""
    },

    {
	"blood spew",                  { 1, 1, 1, 1 },     { 1,  1,  2,2},
	spell_blood_spew,             TAR_CHAR_OFFENSIVE,POS_FIGHTING,
	&gsn_blood_spew,                       SLOT(670),       120,   12,
	"spewing",                 "You stop spewing blood.",
	"$n stops spewing blood.","",5,0,9,""
    },

    {
	"illumination",                  { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_illumination,      TAR_IGNORE,POS_STANDING,
	NULL,                       SLOT(671),       20,   12,
	"light",                 "The light vanishes.",
	"","",1,0,3,""
    },

    {
	"unravel",                  { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_unravel,      TAR_OBJ_INV,POS_STANDING,
	NULL,                       SLOT(673),       8,   12,
	"unravel",                 "no more unravel.",
	"","",3,0,1,""
    },

    {
	"retribution",                  { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_retribution,      TAR_CHAR_DEFENSIVE ,POS_STANDING,
	NULL,                       SLOT(674),       60,   12,
	"retribution",                 "The white aura fades.",
	"","",1,0,7,""
    },

    {
	"wall of fire",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_wall_of_fire,    TAR_EXIT ,POS_STANDING,
	NULL,                       SLOT(675),       120,   12,
	"wall of fire",                 "The wall disappears.",
	"","",1,0,8,""
    },

    {
	"wall of iron",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_wall_of_iron,    TAR_EXIT ,POS_STANDING,
	NULL,                       SLOT(676),       120,   12,
	"wall of iron",                 "The wall of iron disappears.",
	"","an iron seed",4,20,8,""
    },

    {
	"stinking cloud",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_stinking_cloud,    TAR_IGNORE ,POS_STANDING,
	NULL,                       SLOT(677),       120,   12,
	"stinking cloud",                 "BUG.",
	"","",4,0,6,""
    },

    {
	"life vortex",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_life_vortex,      TAR_IGNORE ,POS_STANDING,
	NULL,                       SLOT(678),       120,   12,
	"life vortex",                 "BUG.",
	"","",3,0,9,"death vortex"
    },

    {
	"death vortex",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_death_vortex,      TAR_IGNORE ,POS_STANDING,
	NULL,                       SLOT(679),       120,   12,
	"death vortex",                 "BUG.",
	"","",5,0,6,"life vortex"
    },

    {
	"desecrate",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_desecrate_exit,      TAR_EXIT ,POS_STANDING,
	NULL,                       SLOT(680),       120,   12,
	"desecrate",                 "BUG.",
	"","",5,0,2,""
    },

    {
	"consecrate",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_consecrate,      TAR_IGNORE ,POS_STANDING,
	NULL,                       SLOT(681),       120,   12,
	"consecrate",                 "BUG.",
	"","",6,0,5,""
    },

    {
	"raise dead",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_raise_dead,     TAR_OBJ_ROOM ,POS_STANDING,
	NULL,                       SLOT(682),       120,   12,
	"raise dead",                 "BUG.",
	"","",6,0,8,"power word kill"
    },

    {
	"ghost light",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_ghost_light,     TAR_IGNORE ,POS_STANDING,
	NULL,                       SLOT(683),       120,   12,
	"light",                 "BUG.",
	"","",6,0,9,""
    },

    {
	"tree",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_tree,     TAR_CHAR_DEFENSIVE ,POS_STANDING,
	NULL,                       SLOT(684),       120,   12,
	"tree",                 "BUG.",
	"","",4,0,7,""
    },

    {
	"haven",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_haven,     TAR_IGNORE, POS_STANDING,
	NULL,                       SLOT(685),       120,   12,
	"haven",                 "BUG.",
	"","",4,0,1,""
    },

    {
	"golum",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_golum,     TAR_CHAR_DEFENSIVE ,POS_STANDING,
	NULL,                       SLOT(686),       120,   12,
	"golem",                 "BUG.",
	"","",4,0,9,""
    },

    {
	"imbue spells",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_imbue,     TAR_CHAR_DEFENSIVE ,POS_STANDING,
	NULL,                       SLOT(687),       120,   12,
	"imbue",                 "BUG.",
	"","ambergris",2,100,10,""
    },

    {
	"vivify",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_vivify,     TAR_CHAR_DEFENSIVE ,POS_STANDING,
	NULL,                       SLOT(688),       120,   12,
	"vivify",                 "BUG.",
	"","",6,0,3,""
    },

    {
	"whispers",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_whispers,     TAR_IGNORE ,POS_RESTING,
	NULL,                       SLOT(689),       120,   12,
	"whispers",                 "BUG.",
	"","",3,0,3,""
    },

    {
	"chelchemy",             { 1, 1, 1, 1 },     { 1,  1,2,2},
	spell_chelchemy,     TAR_IGNORE ,POS_RESTING,
	NULL,                       SLOT(690),       120,   12,
	"chelchemy",                 "BUG.",
	"","",1,0,5,""
    },

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
    {
	"general purpose",      { 1, 1, 1, 1 }, { 0, 0, 0, 0 },
	spell_general_purpose,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(401),      0,      12,
	"general purpose ammo", "!General Purpose Ammo!",       "","",0,0,0,""
    },
 
    {
	"high explosive",       { 1, 1, 1, 1 }, { 0, 0, 0, 0 },
	spell_high_explosive,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(402),      0,      12,
	"high explosive ammo",  "!High Explosive Ammo!",        "","",0,0,0,""
    },


/* Combat and weapons skills */


    {
	"axe",                  { 1, 1, 1,  1 },        { 6, 6, 5, 4},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_axe,               SLOT( 0),       0,      0,
	"",                     "!Axe!",                "","",0,0,0,""
    },

    {
	"dagger",               {  1,  1,  1,  1 },     { 2, 3, 2, 2},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_dagger,            SLOT( 0),       0,      0,
	"",                     "!Dagger!",             "","",0,0,0,""
    },
 
    {
	"flail",                { 1,  1, 1,  1 },       { 6, 3, 6, 4},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_flail,             SLOT( 0),       0,      0,
	"",                     "!Flail!",              "","",0,0,0,""
    },

    {
	"mace",                 { 1,  1,  1,  1 },      { 5, 2, 3, 3},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_mace,              SLOT( 0),       0,      0,
	"",                     "!Mace!",               "","",0,0,0,""
    },

    {
	"polearm",              { 1, 1, 1,  1 },        { 6, 6, 6, 4},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_polearm,           SLOT( 0),       0,      0,
	"",                     "!Polearm!",            "","",0,0,0,""
    },
    
    {
	"shield block",         { 1,  1, 1,  1 },       { 6, 4, 6, 2},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_shield_block,      SLOT(0),        0,      0,
	"",                     "!Shield!",             "","",0,0,0,""
    },
 
    {
	"spear",                {  1,  1,  1,  1 },     { 4, 4, 4, 3},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_spear,             SLOT( 0),       0,      0,
	"",                     "!Spear!",              "","",0,0,0,""
    },

    {
	"sword",                { 1, 1,  1,  1},        { 5, 6, 3, 2},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_sword,             SLOT( 0),       0,      0,
	"",                     "!sword!",              "","",0,0,0,""
    },

    {
	"whip",                 { 1, 1,  1,  1},        { 6, 5, 5, 4},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_whip,              SLOT( 0),       0,      0,
	"",                     "!Whip!",       "","",0,0,0,""
    },

    {
	"backstab",             { 53, 53,  1, 53 },     { 0, 0, 5, 0},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_backstab,          SLOT( 0),        0,     24,
	"backstab",             "!Backstab!",           "","",0,0,0,""
    },

    {
	"bash",                 { 53, 53, 53,  1 },     { 0, 0, 0, 4},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_bash,              SLOT( 0),       0,      24,
	"bash",                 "!Bash!",               "","",0,0,0,""
    },

    {
	"berserk",              { 53, 53, 53, 18 },     { 0, 0, 0, 5},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_berserk,           SLOT( 0),       0,      24,
	"",                     "You feel your pulse slow down.",       "","",0,0,0,""
    },

    {
	"dirt kicking",         { 53, 53,  3,  3 },     { 0, 0, 4, 4}, 
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_dirt,              SLOT( 0),       0,      24,
	"kicked dirt",          "You rub the dirt out of your eyes.",   "","",0,0,0,""
    },

    {
	"disarm",               { 53, 53, 12, 11 },     { 0, 0, 6, 4},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_disarm,            SLOT( 0),        0,     24,
	"",                     "!Disarm!",             "","",0,0,0,""
    },
 
    {
	"dodge",                { 20, 22,  1, 13 },     { 8, 8, 4, 6},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_dodge,             SLOT( 0),        0,     0,
	"",                     "!Dodge!",              "","",0,0,0,""
    },
 
    {
	"battle prowess",      { 45, 30, 25,  1 },     { 10, 9, 5, 3},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_enhanced_damage,   SLOT( 0),        0,     0,
	"",                     "!Enhanced Damage!",    "","",0,0,0,""
    },

    {
	"envenom",              { 53, 53, 10, 53 },     { 0, 0, 4, 0 },
	spell_null,             TAR_IGNORE,             POS_RESTING,
	&gsn_envenom,           SLOT(0),        0,      36,
	"",                     "!Envenom!",            "","",0,0,0,""
    },

    {
	"hand to hand",         { 25,  10, 15, 6 },     { 8, 5, 6, 4},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_hand_to_hand,      SLOT( 0),       0,      0,
	"",                     "!Hand to Hand!",       "","",0,0,0,""
    },

    {
	"kick",                 { 53, 12, 14,  8 },     { 0, 4, 6, 3},
	spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	&gsn_kick,              SLOT( 0),        0,     12,
	"kick",                 "!Kick!",               "","",0,0,0,""
    },

    {
	"parry",                { 22, 20, 13,  1 },     { 8, 8, 6, 4},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_parry,             SLOT( 0),        0,     0,
	"",                     "!Parry!",              "","",0,0,0,""
    },

    {
	"rescue",               { 53, 53, 53,  1 },     { 0, 0, 0, 4},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_rescue,            SLOT( 0),        0,     12,
	"",                     "!Rescue!",             "","",0,0,0,""
    },

    {
	"trip",                 { 53, 53,  1, 15 },     { 0, 0, 4, 8},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_trip,              SLOT( 0),       0,      24,
	"trip",                 "!Trip!",               "","",0,0,0,""
    },

/* non-combat skills */

    { 
	"fast healing",         { 15, 9, 16,  6 },      { 8, 5, 6, 4},
	spell_null,             TAR_IGNORE,             POS_SLEEPING,
	&gsn_fast_healing,      SLOT( 0),       0,      0,
	"",                     "!Fast Healing!",       "","",0,0,0,""
    },

    {
	"haggle",               { 7, 18,  1, 14 },      { 5, 8, 3, 6},
	spell_null,             TAR_IGNORE,             POS_RESTING,
	&gsn_haggle,            SLOT( 0),       0,      0,
	"",                     "!Haggle!",             "","",0,0,0,""
    },

    {
	"elvish",               { 7, 18,  1, 1 },      { 5, 8, 3, 6},
	spell_null,             TAR_IGNORE,             POS_SLEEPING,
	&gsn_elvish,            SLOT( 0),       0,      0,
	"",                     "!Elvish!",             "","",0,0,0,""
    },

    {
	"common",               { 7, 18,  1, 1 },      { 5, 8, 3, 6},
	spell_null,             TAR_IGNORE,             POS_SLEEPING,
	&gsn_common,            SLOT( 0),       0,      0,
	"",                     "!Common!",             "","",0,0,0,""
    },

    {
	"gnome",               { 7, 18,  1, 1 },      { 5, 8, 3, 6},
	spell_null,             TAR_IGNORE,             POS_SLEEPING,
	&gsn_gnome,            SLOT( 0),       0,      0,
	"",                     "!Gnome!",             "","",0,0,0,""
    },

    {
	"hide",                 { 53, 53,  1,  12 },    { 0, 0, 4, 6},
	spell_null,             TAR_IGNORE,             POS_RESTING,
	&gsn_hide,              SLOT( 0),        0,     12,
	"",                     "!Hide!",               "","",0,0,0,""
    },

    {
	"lore",                 { 10, 10,  6, 20 },     { 6, 6, 4, 8},
	spell_null,             TAR_IGNORE,             POS_RESTING,
	&gsn_lore,              SLOT( 0),       0,      36,
	"",                     "!Lore!",               "","",0,0,0,""
    },

    {
	"meditation",           {  6,  6, 15, 15 },     { 5, 5, 8, 8},
	spell_null,             TAR_IGNORE,             POS_SLEEPING,
	&gsn_meditation,        SLOT( 0),       0,      0,
	"",                     "Meditation",           "","",0,0,0,""
    },

    {
	"peek",                 {  8, 21,  1, 14 },     { 5, 7, 3, 6},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_peek,              SLOT( 0),        0,      0,
	"",                     "!Peek!",               "","",0,0,0,""
    },

    {
	"pick lock",            { 25, 25,  7, 25 },     { 8, 8, 4, 8},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_pick_lock,         SLOT( 0),        0,     12,
	"",                     "!Pick!",               "","",0,0,0,""
    },

    {
	"sneak",                { 53, 53,  4,  10 },    { 0, 0, 4, 6},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_sneak,             SLOT( 0),        0,     12,
	"",                     "You no longer feel stealthy.", "","",0,0,0,""
    },

    {
	"steal",                { 53, 53,  5, 53 },     { 0, 0, 4, 0},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_steal,             SLOT( 0),        0,     24,
	"",                     "!Steal!",              "","",0,0,0,""
    },

    {
	"scrolls",              {  1,  1,  1,  1 },     { 2, 3, 5, 8},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_scrolls,           SLOT( 0),       0,      24,
	"",                     "!Scrolls!",            "","",0,0,0,""
    },

    {
	"staves",               {  1,  1,  1,  1 },     { 2, 3, 5, 8},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_staves,            SLOT( 0),       0,      12,
	"",                     "!Staves!",             "","",0,0,0,""
    },
    
    {
	"wands",                {  1,  1,  1,  1 },     { 2, 3, 5, 8},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_wands,             SLOT( 0),       0,      12,
	"",                     "!Wands!",              "","",0,0,0,""
    },

    {
	"off hand",                {  1,  1,  1,  1 },     { 2, 3, 5, 8},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_off_hand,          SLOT( 0),       0,      12,
	"",                     "!Off Hand!",              "","",0,0,0,""
    },

    {
	"tumbling",                {  1,  1,  1,  1 },     { 2, 3, 5, 8},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_tumbling,          SLOT( 0),       0,      12,
	"",                     "!Tumbling!",              "","",0,0,0,""
    },

    {
	"foraging",                {  1,  1,  1,  1 },     { 2, 3, 5, 8},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_foraging,          SLOT( 0),       0,      12,
	"",                     "!Foraging!",              "","",0,0,0,""
    },

    {
	"blacksmith",                {  1,  1,  1,  1 },     { 2, 3, 5,8},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_blacksmith,          SLOT( 0),       0,      12,
	"",                     "!Blacksmith!","","",0,0,0,""
    },

    {
	"dagger throwing",         {  1,  1,  1,  1 },     { 2, 3, 5,8},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_dagger_throwing,      SLOT( 0),       0,      12,
	"",                     "!Dagger!","","",0,0,0,""
    },

    {
	"blind fighting",         { 53, 53, 24, 12 },     { 0, 0, 10, 4},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_blindfighting,      SLOT( 0),        0,     0,
	"",                     "!Blind Fighting!",       "","",0,0,0,""
    },
    {
	"demon",               { 7, 18,  1, 1 },      { 5, 8, 3, 6},
	spell_null,             TAR_IGNORE,             POS_SLEEPING,
	&gsn_demon,            SLOT( 0),       0,      0,
	"",                     "!Demon!",             "","",0,0,0,""
    }



};

const   struct  group_type      group_table     [MAX_GROUP]     =
{

    {
	"rom basics",           { 0, 0, 0, 0 },
	{ "scrolls", "staves", "wands", "recall" }
    },

    {
	"mage basics",          { 0, -1, -1, -1 },
	{ "dagger" }
    },

    {
	"cleric basics",        { -1, 0, -1, -1 },
	{ "mace" }
    },
   
    {
	"thief basics",         { -1, -1, 0, -1 },
	{ "dagger", "steal" }
    },

    {
	"warrior basics",       { -1, -1, -1, 0 },
	{ "weaponsmaster", "second attack" }
    },

    {
	"mage default",         { 40, -1, -1, -1 },
	{ "lore", "beguiling", "combat", "detection", "enhancement", "illusion",
	  "maladictions", "protective", "transportation", "weather" }
    },

    {
	"cleric default",       { -1, 40, -1, -1 },
	{ "flail", "attack", "creation", "curative",  "benedictions", 
	  "detection", "healing", "maladictions", "protective", "shield block", 
	  "transportation", "weather" }
    },
 
    {
	"thief default",        { -1, -1, 40, -1 },
	{ "mace", "sword", "backstab", "disarm", "dodge", "second attack",
	  "trip", "hide", "peek", "pick lock", "sneak" }
    },

    {
	"warrior default",      { -1, -1, -1, 40 },
	{ "shield block", "parry" }
    },

    {
	"weaponsmaster",        { 40, 40, 40, 20 },
	{ "axe", "dagger", "flail", "mace", "polearm", "spear", "sword","whip" }
    },
 
	/* Start count here from 10 */

    {
	"fire1",                {1, 1, 1, 1},
	{ "burning hands", "fireproof" }
    },

    {
	"fire2",                {1, 1, 1, 1},
	{ "continual light", "locate object" }
    },

    {
	"fire3",                {1, 1, 1, 1},
	{ "fireball", "faerie fire", "heat metal" }
    },

    {
	"fire4",                {1, 1, 1, 1},
	{ "frenzy", "recharge", "illumination", "resist cold" }
    },

    {
	"fire5",                {1, 1, 1, 1},
	{ "flame strike", "charm person", "chelchemy" }
    },

    {
	"fire6",                {1, 1, 1, 1},
	{ "enchant weapon", "enchant armor", "lava burst", "magma burst" }
    },

    {
	"fire7",                {1, 1, 1, 1},
	{ "ray of truth", "cry of anger", "summon flame", "retribution" }
    },

    {
	"fire8",                {1, 1, 1, 1},
	{ "flame blade", "wall of fire" }
    },

    {
	"fire9",                {1, 1, 1, 1},
	{ "conflaguration" ,"power word stun" }
    },

    {
	"fire10",               {1, 1, 1, 1},
	{ "star storm", "summon inferno", "disintegrate" }
    },


	/* start count here from 19 */

    {
	"water1",               {1, 1, 1, 1},
	{ "cure light", "bless", "create water" }
    },

    {
	"water2",               {1, 1, 1, 1},
	{ "cure poison", "refresh", "cure blindness" }
    },

    {
	"water3",               {1, 1, 1, 1},
	{ "cure serious", "protection evil", "sleep"  }
    },

    {
	"water4",               {1, 1, 1, 1},
	{ "cure disease", "change sex", "remove fear" }
    },

    {
	"water5",               {1, 1, 1, 1},
	{ "cure critical", "remove curse" }
    },

    {
	"water6",               {1, 1, 1, 1},
	{ "heal", "bloodlust" }
    },

    {
	"water7",               {1, 1, 1, 1},
	{ "mass healing", "reset", "summon wave" }
    },

    {
	"water8",               {1, 1, 1, 1},
	{ "calm", "holy word", "quench" }
    },

    {
	"water9",               {1, 1, 1, 1},
	{ "spring of life", "heroes feast", "life vortex" }
    },

    {
	"water10",              {1, 1, 1, 1},
	{ "wish", "harmony", "summon tsunami", "imbue spells" }
    },

	/* start count here from 28 */

    {
	"air1",                 {1, 1, 1, 1},
	{ "magic missile", "ventriloquate", "floating disc", "detect magic"
		,"unravel" }
    },

    {
	"air2",                 {1, 1, 1, 1},
	{ "shocking grasp", "detect evil", "detect good", "farsight" }
    },

    {
	"air3",                 {1, 1, 1, 1},
	{ "jump", "detect hidden", "detect invis", "faerie fog", "invisibility", "whispers" }
    },

    {
	"air4",                 {1, 1, 1, 1},
	{ "colour spray", "shield", "water breathing" }
    },

    {
	"air5",                 {1, 1, 1, 1},
	{ "control weather", "haste", "fly" }
    },

    {
	"air6",                 {1, 1, 1, 1},
	{ "call lightning", "improved invis" }
    },

    {
	"air7",                 {1, 1, 1, 1},
	{  "chain lightning", "word of recall",
	  "summon whirlwind" }
    },

    {
	"air8",                 {1, 1, 1, 1},
	{ "lightning breath", "gas breath" }
    },

    {
	"air9",                 {1, 1, 1, 1},
	{ "windtomb", "horrid wilting", "time twister" }
    },

    {
	"air10",                {1, 1, 1, 1},
	{ "winds of caina", "summon tornado" }
    },

	/* start count here from 37 */

    {
	"earth1",                       {1, 1, 1, 1},
	{ "haven","armor", "create food", "ballistic attack" }
    },

    {
	"earth2",                       {1, 1, 1, 1},
	{ "giant strength", "create rose", "detect poison", "glitterdust" }
    },

    {
	"earth3",                       {1, 1, 1, 1},
	{ "dispel magic", "know alignment", "infravision" }
    },

    {
	"earth4",                       {1, 1, 1, 1},
	{ "stone skin", "slow", "trip", "soul blade" }
    },

    {
	"earth5",                       {1, 1, 1, 1},
	{ "pass door", "acid blast", "cancellation", "hold person" }
    },

    {
	"earth6",                       {1, 1, 1, 1},
	{ "summon", "earthquake", "rock to mud", "stinking cloud"  }
    },

    {
	"earth7",                       {1, 1, 1, 1},
	{ "gate", "iron skin", "identify", "summon giant", "tree" }
    },

    {
	"earth8",                       {1, 1, 1, 1},
	{ "portal", "acid breath", "grounding", "wall of iron" }
    },

    {
	"earth9",                       {1, 1, 1, 1},
	{ "frog", "regeneration", "golum" }
    },

    {
	"earth10",                      {1, 1, 1, 1},
	{ "summon colossus" }
    },

	/* start count here from 46 */

    {
	"entropy1",                     {1, 1, 1, 1},
	{ "chill touch", "cause light", "dark pact" }
    },

    {
	"entropy2",                     {1, 1, 1, 1},
	{ "weaken", "poison", "cause serious", "hunger", "thirst"
		, "desecrate" }
    },

    {
	"entropy3",                     {1, 1, 1, 1},
	{ "curse", "protect good", "blindness"}
    },

    {
	"entropy4",                     {1, 1, 1, 1},
	{ "plague", "cause critical", "dispel good" }
    },

    {
	"entropy5",                     {1, 1, 1, 1},
	{ "energy drain", "harm", "bloodlust" }
    },

    {
	"entropy6",                     {1, 1, 1, 1},
	{ "demon fire", "lure of darkness", "deafness", "death vortex" }
    },

    {
	"entropy7",                     {1, 1, 1, 1},
	{ "devour soul", "pentagram", "black tentacles", "pestilence" }
    },

    {
	"entropy8",                     {1, 1, 1, 1},
	{ "cockroach", "bind daemon", "fear", "wither", "mummy rot" }
    },

    {
	"entropy9",                     {1, 1, 1, 1},
	{ "shadow blade", "consultation", "collapse", "blood spew" }
    },

    {
	"entropy10",                    {1, 1, 1, 1},
	{ "power word kill", "arachnophobia", "all hallows eve" }
    },

    {
	"life1",                    {1, 1, 1, 1},
	{ "" }
    },
    {
	"life2",                    {1, 1, 1, 1},
	{ "" }
    },
    {
	"life3",                    {1, 1, 1, 1},
	{ "vivify" }
    },
    {
	"life4",                    {1, 1, 1, 1},
	{ "dispel evil" }
    },
    {
	"life5",                    {1, 1, 1, 1},
	{ "consecrate","sanctuary" }
    },
    {
	"life6",                    {1, 1, 1, 1},
	{ "" }
    },
    {
	"life7",                    {1, 1, 1, 1},
	{ "" }
    },
    {
	"life8",                    {1, 1, 1, 1},
	{ "raise dead" }
    },
    {
	"life9",                    {1, 1, 1, 1},
	{ "ghost light" }
    },
    {
	"life10",                    {1, 1, 1, 1},
	{ "" }
    }
};
