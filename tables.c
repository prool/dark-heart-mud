/*
 * HELL, formerly known as BTL/Darkheart, by Adam Powell and Chris Mellish,
 *  1996 to 1998.  If you reproduce this code in any way please give credit
 *  to the writers.  Thanks.
 */


#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "hell.h"
#include "tables.h"

/* for clans */
const struct clan_type clan_table[MAX_CLAN] =
{
   /*  name,           who entry,      death-transfer room,    independent
  	AND NOW.... The 3 disciplines gained on entry to the clan
	- called disc1, disc2, and disc3 - all sh_ints !!!!  */

    /* independent should be FALSE if is a real clan */
    {   "",             "",             ROOM_VNUM_ALTAR,        TRUE   
	,0,0,0 },
    {   "gangrel",      " {Gangrel",   ROOM_VNUM_ALTAR, FALSE,
	DISC_VAMP_PROT, DISC_VAMP_FORT, DISC_VAMP_ANIM   },
    {   "anarch",       " {Anarch",    ROOM_VNUM_ALTAR, TRUE,
	0,0,0   },
    {   "tremere",      " {Tremere",   ROOM_VNUM_ALTAR, FALSE,
	DISC_VAMP_THAU, DISC_VAMP_DOMI, DISC_VAMP_AUSP   },
    {   "brujah",       " {Brujah",    ROOM_VNUM_ALTAR, FALSE,
	DISC_VAMP_CELE, DISC_VAMP_POTE, DISC_VAMP_PRES   },
    {   "nosferatu",    " {Nosferatu", ROOM_VNUM_ALTAR, FALSE,
	DISC_VAMP_ANIM, DISC_VAMP_OBFU, DISC_VAMP_POTE   },
    {   "toreador",     " {Toreador",  ROOM_VNUM_ALTAR, FALSE,
	DISC_VAMP_AUSP, DISC_VAMP_CELE, DISC_VAMP_PRES   },
    {   "ventrue",      " {Ventrue",   ROOM_VNUM_ALTAR, FALSE,
	DISC_VAMP_DOMI, DISC_VAMP_FORT, DISC_VAMP_PRES   },
    {   "malkavian",    " {Malkavian", ROOM_VNUM_ALTAR, FALSE,
	DISC_VAMP_DOMI, DISC_VAMP_AUSP, DISC_VAMP_OBFU   },
    {   "tzimisce",       "{Tzimisce", ROOM_VNUM_ALTAR, FALSE,
	DISC_VAMP_PROT, DISC_VAMP_AUSP, DISC_VAMP_DOMI   },

    {   "caina",         " <Caina",       ROOM_VNUM_ALTAR,        FALSE,
	DISC_DAEM_GELU, DISC_DAEM_ATTA, DISC_DAEM_TEMP  },
    {   "banshe",       " <Banshe",     ROOM_VNUM_ALTAR,        FALSE,
	DISC_DAEM_MORP, DISC_DAEM_CORR, DISC_DAEM_HELL   },
    {   "outcast",      " <Outcast",     ROOM_VNUM_ALTAR,        TRUE,
	0,0,0   },
    {   "shaitan",      " <Shaitan",    ROOM_VNUM_ALTAR,        FALSE, 
	DISC_DAEM_HELL, DISC_DAEM_CORR, DISC_DAEM_DISC   },

    {   "morwel",       " [Morwel",     ROOM_VNUM_ALTAR,        FALSE, 
	DISC_ELAD_MELO, DISC_ELAD_ARBO, DISC_ELAD_ARTI  },
    {   "lorien",       " [Lorien",     ROOM_VNUM_ALTAR,        FALSE, 
	DISC_ELAD_MELO, DISC_ELAD_AURA, DISC_ELAD_PHYS  },
    {   "evlock",       " [Evlock",     ROOM_VNUM_ALTAR,        FALSE, 
	DISC_ELAD_ANSO, DISC_ELAD_MORP, DISC_ELAD_ARBO  },
    {   "silarin",      " [Silarin",    ROOM_VNUM_ALTAR,        FALSE,
	DISC_ELAD_SOMA, DISC_ELAD_ARTI, DISC_ELAD_AURA   },

    {   "rogue"," (Rogue",ROOM_VNUM_ALTAR,      TRUE, 0,0,0  },
    {   "lupe garou",   " (Lupe Garou", ROOM_VNUM_ALTAR,        FALSE,
	DISC_LUPI_LUNA, DISC_LUPI_LUPI, DISC_LUPI_BOAR   },
    {   "lune verdi",   " (Lune Verdi", ROOM_VNUM_ALTAR,        FALSE,
   	DISC_LUPI_LUNA, DISC_LUPI_LEON, DISC_LUPI_LUPI  },
    {   "aethir garou", " (Aethir Garou",  ROOM_VNUM_ALTAR,     FALSE,
   	DISC_LUPI_HAWK, DISC_LUPI_LEON, DISC_LUPI_COBR  },

    {   "crimson flame"," -<Crimson Flame", ROOM_VNUM_ALTAR,    FALSE,
   	DISC_WARL_THEA, DISC_WARL_CONT, DISC_WARL_BREW},
    {   "delvers", " -<Delvers", ROOM_VNUM_ALTAR,       FALSE,
   	DISC_WARL_FARR, DISC_WARL_SCRI, DISC_WARL_ENCH},
    {   "lothariel",     " -<Lothariel",  ROOM_VNUM_ALTAR,        FALSE,
   	DISC_WARL_LUCU, DISC_WARL_RUNE, DISC_WARL_BREW},

    {   "sun warrior",  " <>Sun", ROOM_VNUM_ALTAR,      FALSE,
   	DISC_HERO_ELEM,DISC_HERO_TRUE,DISC_HERO_EXOR},
    {   "druid",        " <>Druid",     ROOM_VNUM_ALTAR,        FALSE,
   	DISC_HERO_VIVI,DISC_HERO_SIGI,DISC_HERO_RELI},
    {   "knights",      " <>Knight", ROOM_VNUM_ALTAR,        FALSE,
   	DISC_HERO_CRUS,DISC_HERO_BATT,DISC_HERO_ELEM},

    /* Drow clan */
    {   "outcast",       " {Outcast}", ROOM_VNUM_ALTAR,        FALSE,
   	0,0,0},
    {   "first house",   " {First House}", ROOM_VNUM_ALTAR,       FALSE,
   	0,0,0},
    {   "second house",  " {Second House}", ROOM_VNUM_ALTAR,        FALSE,
   	0,0,0},
    {   "third house",   " {Third House}", ROOM_VNUM_ALTAR,        
FALSE, 0,0,0  },
    {   "fourth house",  " {Fourth House}", ROOM_VNUM_ALTAR,        
FALSE, 0,0,0  },
    {   "fifth house",   " {Fifth House}", ROOM_VNUM_ALTAR,        
FALSE, 0,0,0  },

    /* odd vampire clans */
    {   "manticore",   " {Manticore", ROOM_VNUM_ALTAR,        FALSE,
	DISC_VAMP_CELE, DISC_VAMP_SATY, DISC_VAMP_ANIM   },

    /* a new demon clan with nether! */
    {   "shada",         " <Shada",       ROOM_VNUM_ALTAR,        FALSE,
	DISC_DAEM_NETH, DISC_DAEM_DISC, DISC_DAEM_TEMP  },

    {   "nephandic",         " -<Nephandic",       ROOM_VNUM_ALTAR, FALSE,
	DISC_WARL_FARR, DISC_WARL_FAKI, DISC_WARL_CONT  },
    
    {   "unseen",            " -<Unseen",          ROOM_VNUM_ALTAR, FALSE,
        DISC_WARL_FARR, DISC_WARL_THEA, DISC_WARL_CONT  },
    
    {   "hakken garou", " (Hakken Garou",  ROOM_VNUM_ALTAR,     FALSE,
   	DISC_LUPI_FOX, DISC_LUPI_LEON, DISC_LUPI_URSI  },

    {   NULL,             NULL,             ROOM_VNUM_ALTAR,        TRUE   
	,0,0,0 }
};

/* for position */
const struct position_type position_table[] =
{
    {   "dead",                 "dead"  },
    {   "mortally wounded",     "mort"  },
    {   "incapacitated",        "incap" },
    {   "stunned",              "stun"  },
    {   "sleeping",             "sleep" },
    {   "resting",              "rest"  },
    {   "sitting",              "sit"   },
    {   "fighting",             "fight" },
    {   "standing",             "stand" },
    {   NULL,                   NULL    }
};

/* for sex */
const struct sex_type sex_table[] =
{
   {    "none"          },
   {    "male"          },
   {    "female"        },
   {    "either"        },
   {    NULL            }
};

/* for sizes */
const struct size_type size_table[] =
{ 
    {   "tiny"          },
    {   "small"         },
    {   "medium"        },
    {   "large"         },
    {   "huge",         },
    {   "giant"         },
    {   NULL            }
};

/* various flag tables */
const struct flag_type act_flags[] =
{
    {   "npc",                  A,      FALSE   },
    {   "sentinel",             B,      TRUE    },
    {   "scavenger",            C,      TRUE    },
    {   "aggressive",           F,      TRUE    },
    {   "stay_area",            G,      TRUE    },
    {   "wimpy",                H,      TRUE    },
    {   "pet",                  I,      TRUE    },
    {   "train",                J,      TRUE    },
    {   "practice",             K,      TRUE    },
    {   "undead",               O,      TRUE    },
    {   "cleric",               Q,      TRUE    },
    {   "mage",                 R,      TRUE    },
    {   "thief",                S,      TRUE    },
    {   "warrior",              T,      TRUE    },
    {   "noalign",              U,      TRUE    },
    {   "nopurge",              V,      TRUE    },
    {   "outdoors",             W,      TRUE    },
    {   "indoors",              Y,      TRUE    },
    {   "healer",               aa,     TRUE    },
    {   "gain",                 bb,     TRUE    },
    {   "update_always",        cc,     TRUE    },
    {   "changer",              dd,     TRUE    },
    {   NULL,                   0,      FALSE   }
};

const struct flag_type plr_flags[] =
{
    {   "npc",                  A,      FALSE   },
    {   "autoassist",           C,      FALSE   },
    {   "autoexit",             D,      FALSE   },
    {   "autoloot",             E,      FALSE   },
    {   "autosac",              F,      FALSE   },
    {   "autogold",             G,      FALSE   },
    {   "autosplit",            H,      FALSE   },
    {   "holylight",            N,      FALSE   },
    {   "can_loot",             P,      FALSE   },
    {   "nosummon",             Q,      FALSE   },
    {   "nofollow",             R,      FALSE   },
    {   "permit",               U,      TRUE    },
    {   "log",                  W,      FALSE   },
    {   "deny",                 X,      FALSE   },
    {   "freeze",               Y,      FALSE   },
    {   "thief",                Z,      FALSE   },
    {   "killer",               aa,     FALSE   },
    {   NULL,                   0,      0       }
};

const struct flag_type affect_flags[] =
{
    {   "blind",                A,      TRUE    },
    {   "invisible",            B,      TRUE    },
    {   "detect_evil",          C,      TRUE    },
    {   "detect_invis",         D,      TRUE    },
    {   "detect_magic",         E,      TRUE    },
    {   "detect_hidden",        F,      TRUE    },
    {   "detect_good",          G,      TRUE    },
    {   "sanctuary",            H,      TRUE    },
    {   "faerie_fire",          I,      TRUE    },
    {   "infrared",             J,      TRUE    },
    {   "curse",                K,      TRUE    },
    {   "poison",               M,      TRUE    },
    {   "protect_evil",         N,      TRUE    },
    {   "protect_good",         O,      TRUE    },
    {   "sneak",                P,      TRUE    },
    {   "hide",                 Q,      TRUE    },
    {   "sleep",                R,      TRUE    },
    {   "charm",                S,      TRUE    },
    {   "flying",               T,      TRUE    },
    {   "pass_door",            U,      TRUE    },
    {   "haste",                V,      TRUE    },
    {   "calm",                 W,      TRUE    },
    {   "plague",               X,      TRUE    },
    {   "weaken",               Y,      TRUE    },
    {   "dark_vision",          Z,      TRUE    },
    {   "berserk",              aa,     TRUE    },
    {   "swim",                 bb,     TRUE    },
    {   "regeneration",         cc,     TRUE    },
    {   "slow",                 dd,     TRUE    },
    {   "conflaguration",       ee,     TRUE    },
    {   NULL,                   0,      0       }
};

const struct flag_type new_flags[] =

{
    {   "noregen",              A,      TRUE    },
    {   "hold",                 B,      TRUE    },
    {   "allowvamp",            C,      TRUE    },
    {   "allowwere",            D,      TRUE    },
    {   "allowelad",            E,      TRUE    },
    {   "allowdemon",           F,      TRUE    },
    {   "allowwarlock",         G,      TRUE    },
    {   "bloodpotence",         H,      TRUE    },
    {   "waterbreathing",       I,      TRUE    },
    {   "shieldaura",           J,      TRUE    },
    {   "improvedinvis",        K,      TRUE    },
    {   "claw",                 L,      TRUE    },
    {   "bite",                 M,      TRUE    },
    {   "tail",                 N,      TRUE    },
    {   "wing",                 O,      TRUE    },

    {   NULL,                   0,      0       }
};


const struct flag_type off_flags[] =
{
    {   "area_attack",          A,      TRUE    },
    {   "backstab",             B,      TRUE    },
    {   "bash",                 C,      TRUE    },
    {   "berserk",              D,      TRUE    },
    {   "disarm",               E,      TRUE    },
    {   "dodge",                F,      TRUE    },
    {   "fade",                 G,      TRUE    },
    {   "fast",                 H,      TRUE    },
    {   "kick",                 I,      TRUE    },
    {   "dirt_kick",            J,      TRUE    },
    {   "parry",                K,      TRUE    },
    {   "rescue",               L,      TRUE    },
    {   "tail",                 M,      TRUE    },
    {   "trip",                 N,      TRUE    },
    {   "crush",                O,      TRUE    },
    {   "assist_all",           P,      TRUE    },
    {   "assist_align",         Q,      TRUE    },
    {   "assist_race",          R,      TRUE    },
    {   "assist_players",       S,      TRUE    },
    {   "assist_guard",         T,      TRUE    },
    {   "assist_vnum",          U,      TRUE    },
    {   NULL,                   0,      0       }
};


const struct flag_type imm_flags[] =
{
    {   "summon",               A,      TRUE    },
    {   "charm",                B,      TRUE    },
    {   "magic",                C,      TRUE    },
    {   "weapon",               D,      TRUE    },
    {   "bash",                 E,      TRUE    },
    {   "pierce",               F,      TRUE    },
    {   "slash",                G,      TRUE    },
    {   "fire",                 H,      TRUE    },
    {   "cold",                 I,      TRUE    },
    {   "lightning",            J,      TRUE    },
    {   "acid",                 K,      TRUE    },
    {   "poison",               L,      TRUE    },
    {   "negative",             M,      TRUE    },
    {   "holy",                 N,      TRUE    },
    {   "energy",               O,      TRUE    },
    {   "mental",               P,      TRUE    },
    {   "disease",              Q,      TRUE    },
    {   "drowning",             R,      TRUE    },
    {   "light",                S,      TRUE    },
    {   "sound",                T,      TRUE    },
    {   "wood",                 X,      TRUE    },
    {   "silver",               Y,      TRUE    },
    {   "iron",                 Z,      TRUE    },
    {   NULL,                   0,      0       }
};

const struct flag_type form_flags[] =
{
    {   "edible",               FORM_EDIBLE,            TRUE    },
    {   "poison",               FORM_POISON,            TRUE    },
    {   "magical",              FORM_MAGICAL,           TRUE    },
    {   "instant_decay",        FORM_INSTANT_DECAY,     TRUE    },
    {   "other",                FORM_OTHER,             TRUE    },
    {   "animal",               FORM_ANIMAL,            TRUE    },
    {   "sentient",             FORM_SENTIENT,          TRUE    },
    {   "undead",               FORM_UNDEAD,            TRUE    },
    {   "construct",            FORM_CONSTRUCT,         TRUE    },
    {   "mist",                 FORM_MIST,              TRUE    },
    {   "intangible",           FORM_INTANGIBLE,        TRUE    },
    {   "biped",                FORM_BIPED,             TRUE    },
    {   "centaur",              FORM_CENTAUR,           TRUE    },
    {   "insect",               FORM_INSECT,            TRUE    },
    {   "spider",               FORM_SPIDER,            TRUE    },
    {   "crustacean",           FORM_CRUSTACEAN,        TRUE    },
    {   "worm",                 FORM_WORM,              TRUE    },
    {   "blob",                 FORM_BLOB,              TRUE    },
    {   "mammal",               FORM_MAMMAL,            TRUE    },
    {   "bird",                 FORM_BIRD,              TRUE    },
    {   "reptile",              FORM_REPTILE,           TRUE    },
    {   "snake",                FORM_SNAKE,             TRUE    },
    {   "dragon",               FORM_DRAGON,            TRUE    },
    {   "amphibian",            FORM_AMPHIBIAN,         TRUE    },
    {   "fish",                 FORM_FISH ,             TRUE    },
    {   "cold_blood",           FORM_COLD_BLOOD,        TRUE    },
    {   NULL,                   0,                      0       }
};

const struct flag_type part_flags[] =
{
    {   "head",                 PART_HEAD,              TRUE    },
    {   "arms",                 PART_ARMS,              TRUE    },
    {   "legs",                 PART_LEGS,              TRUE    },
    {   "heart",                PART_HEART,             TRUE    },
    {   "brains",               PART_BRAINS,            TRUE    },
    {   "guts",                 PART_GUTS,              TRUE    },
    {   "hands",                PART_HANDS,             TRUE    },
    {   "feet",                 PART_FEET,              TRUE    },
    {   "fingers",              PART_FINGERS,           TRUE    },
    {   "ear",                  PART_EAR,               TRUE    },
    {   "eye",                  PART_EYE,               TRUE    },
    {   "long_tongue",          PART_LONG_TONGUE,       TRUE    },
    {   "eyestalks",            PART_EYESTALKS,         TRUE    },
    {   "tentacles",            PART_TENTACLES,         TRUE    },
    {   "fins",                 PART_FINS,              TRUE    },
    {   "wings",                PART_WINGS,             TRUE    },
    {   "tail",                 PART_TAIL,              TRUE    },
    {   "claws",                PART_CLAWS,             TRUE    },
    {   "fangs",                PART_FANGS,             TRUE    },
    {   "horns",                PART_HORNS,             TRUE    },
    {   "scales",               PART_SCALES,            TRUE    },
    {   "tusks",                PART_TUSKS,             TRUE    },
    {   NULL,                   0,                      0       }
};

const struct flag_type comm_flags[] =
{
    {   "quiet",                COMM_QUIET,             TRUE    },
    {   "deaf",                 COMM_DEAF,              TRUE    },
    {   "nowiz",                COMM_NOWIZ,             TRUE    },
    {   "noclangossip",         COMM_NOAUCTION,         TRUE    },
    {   "nogossip",             COMM_NOGOSSIP,          TRUE    },
    {   "noquestion",           COMM_NOQUESTION,        TRUE    },
    {   "nomusic",              COMM_NOMUSIC,           TRUE    },
    {   "noclan",               COMM_NOCLAN,            TRUE    },
    {   "noquote",              COMM_NOQUOTE,           TRUE    },
    {   "shoutsoff",            COMM_SHOUTSOFF,         TRUE    },
    {   "compact",              COMM_COMPACT,           TRUE    },
    {   "brief",                COMM_BRIEF,             TRUE    },
    {   "prompt",               COMM_PROMPT,            TRUE    },
    {   "combine",              COMM_COMBINE,           TRUE    },
    {   "telnet_ga",            COMM_TELNET_GA,         TRUE    },
    {   "show_affects",         COMM_SHOW_AFFECTS,      TRUE    },
    {   "nograts",              COMM_NOGRATS,           TRUE    },
    {   "noemote",              COMM_NOEMOTE,           FALSE   },
    {   "noshout",              COMM_NOSHOUT,           FALSE   },
    {   "notell",               COMM_NOTELL,            FALSE   },
    {   "nochannels",           COMM_NOCHANNELS,        FALSE   },
    {   "snoop_proof",          COMM_SNOOP_PROOF,       FALSE   },
    {   "afk",                  COMM_AFK,               TRUE    },
    {   NULL,                   0,                      0       }
};

const	char	*		trigger		[NUM_TRIGS] =
{
   "say",
   "enter_room",
   "killed",
   "born",
   "kill",
   "init_combat",
   "fighting",
   "social",
   "threat",
   "prog",
   "leave_room",
   "give",
   "timed_out",
   "drop",
   "enemy_flee",
   "mob_flee",
   "wimpy",
   "random",
   "enemy_cast",
   "kill_enemy"
};
