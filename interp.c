/*
 * HELL, formerly known as BTL/Darkheart, by Adam Powell and Chris Mellish,
 *  1996 to 1998.  If you reproduce this code in any way please give credit
 *  to the writers.  Thanks.
 */


#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hell.h"
#include "interp.h"
#include "recycle.h"

/*
 * Command logging types.
 */
#define LOG_NORMAL      0
#define LOG_ALWAYS      1
#define LOG_NEVER       2

#define CREATOR_ONLY	120
#define OLC		120
#define ROLC		-1
#define DEMON_LEADER	-2
#define CHRIS_ONLY	120
#define SECRACE		-4

/*
 * Log-all switch.
 */
bool                            fLogAll         = FALSE;

int can_interpret args( (CHAR_DATA *ch, int cmd) );

/*
 * Command table.
 */
/* name, algorithm name, min position, min level, log switch
    show?, race , discipline needed, level in discipline, do while switched?
	imm grant group      SHIT - THATS A LOT OF BLOODY THINGS TO DO */
    

const   struct  cmd_type        cmd_table       [] =
{
    /*
     * Common movement commands.
     */
    { "north",          do_north,       POS_STANDING,    0,  LOG_NEVER,0,0, 0, 0,1,0 },
    { "east",           do_east,        POS_STANDING,    0,  LOG_NEVER,0,0, 0, 0,1,0 },
    { "south",          do_south,       POS_STANDING,    0,  LOG_NEVER,0,0, 0, 0,1,0 },
    { "west",           do_west,        POS_STANDING,    0,  LOG_NEVER,0,0, 0, 0,1,0 },
    { "up",             do_up,          POS_STANDING,    0,  LOG_NEVER,0,0, 0, 0,1,0 },
    { "down",           do_down,        POS_STANDING,    0,  LOG_NEVER,0,0,0, 0,1,0 },
    { "%save_prog",     do_save_prog,   POS_DEAD,    	 0,  LOG_NEVER,0,0,0, 0,1,0 },

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    { "at",             do_at,          POS_DEAD,       112,LOG_NORMAL,1, 0, 0, 0,1,CMD_LOCATION },
    { "activate",       do_activate,    POS_FIGHTING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "cast",           do_cast,        POS_FIGHTING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "!!",             do_history,     POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "history",        do_history,     POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
/*    { "jump",           do_jump,        POS_FIGHTING,    0,LOG_NORMAL,1,0,0, 0,2,0 }, */
    { "change",         do_change,      POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_PROT, 4,0,0 },
    { "reform",         do_reform,      POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_PROT, 8,0,0 },
    { "change",         do_morphia,     POS_STANDING,    0,  LOG_NORMAL,1, 5,DISC_ELAD_MORP, 1,0,0 },
    { "pixie",          do_pixie,       POS_STANDING,    0,  LOG_NORMAL,1, 5,DISC_ELAD_MORP, 6,0,0 },
    { "buy",            do_buy,         POS_RESTING,     0,  LOG_NORMAL,1, 0, 0 ,0,0,0 },
    { "channels",       do_channels,    POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "exits",          do_exits,       POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "get",            do_get,         POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "resist",         do_resist,      POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "goto",           do_goto,        POS_DEAD,      112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_LOCATION },
    { "gotoobject",           do_goto_object,        POS_DEAD,      112,LOG_NORMAL,1, 0, 0, 0,1,CMD_LOCATION },
    { "group",          do_group,       POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "guild",          do_guild,       POS_DEAD,   CHRIS_ONLY,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONTROL },
    { "hit",            do_kill,        POS_FIGHTING,    0,  LOG_NORMAL,0, 0, 0, 0,0,0 },
/* berserk is evil -- whois */
/*   { "berserk",        do_berserk,     POS_FIGHTING,    0,  LOG_NORMAL,0, 0, 0, 0,0,0 }, */
    { "inventory",      do_inventory,   POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "kill",           do_kill,        POS_FIGHTING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "look",           do_look,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "bandage",        do_bandage,     POS_STANDING,    0,LOG_NORMAL,1,0, 0, 0,0,0 },
    { "version",        do_version,     POS_DEAD,        0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "willpower",      do_sanity,      POS_RESTING,     0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "rub",            do_rub,         POS_STANDING,    0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "moon",           do_moon,        POS_RESTING,     0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "kick",           do_kick,        POS_FIGHTING,     0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "fire",           do_fire,        POS_STANDING,    0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "repair",         do_repair,      POS_STANDING,    0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "appraise",       do_appraise,    POS_STANDING,    0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "immune",         do_immune,      POS_DEAD,        0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "empower",        do_empower,     POS_STANDING,    0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "clantalk",       do_clantalk,    POS_DEAD,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "clanleader",     do_clanleader,  POS_RESTING,     120,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "clanhelp",       do_clanhelp,    POS_DEAD,        0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "clanlist",       do_clanlist,    POS_RESTING,     0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "clanwar",        do_clanwar,     POS_RESTING,     0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "clanally",       do_clanally,    POS_RESTING,     0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "clandesc",       do_clandesc,    POS_RESTING,     0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "successor",      do_successor,   POS_RESTING,     0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "nochannels",     do_nochannels,  POS_DEAD,         0,  LOG_ALWAYS,1, 0, 0, 0,1,0 },
    { "research",       do_research,    POS_RESTING,     0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "peek",           do_peek,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "music",          do_music,       POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 }, 
    { "language",       do_language,    POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 }, 
    { "order",          do_order,       POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "rest",           do_rest,        POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "sit",            do_sit,         POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "sockets",        do_sockets,     POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_CONNECTIONS },
    { "stand",          do_stand,       POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "tell",           do_tell,        POS_DEAD,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "tear",           do_tear,       POS_FIGHTING,    0,  LOG_NORMAL,1, 4, 0, 0,0,0 },
    { "unlock",         do_unlock,      POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "wield",          do_wear,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "wizhelp",        do_wizhelp,     POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "tutor",          do_tutor,       POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },    
    { "buydiscipline",  do_buydiscipline,POS_STANDING,SECRACE,  LOG_ALWAYS,1, 0, 0, 0,0,0 },    
    { "voodoo",         do_voodoo,      POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },    
    { "shake",          do_shake,       POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },    
    { "ritual",         do_ritual,      POS_STANDING,   0,  LOG_NORMAL,1, 0, 0, 0,0,0 },    
    { "memorize",       do_memorize,    POS_STANDING,   0,  LOG_NORMAL,1, 0, 0, 0,0,0 },    
    { "unmemorize",     do_unmemorize,  POS_STANDING,   0,  LOG_NORMAL,1, 0, 0, 0,0,0 },    
    { "diagnose",       do_diagnose,    POS_STANDING,   0,  LOG_NORMAL,1, 0, 0, 0,0,0 },    
    { "recall",         do_recall,      POS_STANDING,   0,  LOG_NORMAL,1, 0, 0, 0,0,0 },    
    { "gamelist",       do_gamelist,    POS_STANDING,   0,  LOG_NORMAL,1, 0, 0, 0,0,0 },    

    /*
     * Psionic commands.
     */

    { "psi",            do_psi,         POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "z",              do_psi,         POS_RESTING,     0,  LOG_NORMAL,0, 0, 0, 0,0,0 },
    { "psioff",         do_psioff,      POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "x",              do_psioff,      POS_RESTING,     0,  LOG_NORMAL,0, 0, 0, 0,0,0},
    { "psiskill",       do_psiskill,    POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "psiupkeep",      do_psiupkeep,   POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "kaipsi",         do_kaipsi,      POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "psilearn",       do_psilearn,    POS_RESTING,     0,  LOG_NORMAL,1, 9, 0, 0,1,0 },


    /*
     * Informational commands.
     */
    { "affects",        do_affects,     POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "areas",          do_areas,       POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "realm",          do_realm,       POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "bug",            do_bug,         POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "balance",        do_balance,     POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "changes",        do_changes,     POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "racecommands",   do_racecommands,POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "powers",         do_racecommands,POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "compare",        do_compare,     POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "consider",       do_consider,    POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "count",          do_count,       POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "equipment",      do_equipment,   POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "expand",         do_expand,      POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "examine",        do_examine,     POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "help",           do_help,        POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "idea",           do_idea,        POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "motd",           do_motd,        POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "news",           do_news,        POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "read",           do_read,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "report",         do_report,      POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "rules",          do_rules,       POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "finger",         do_finger,      POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "racestats",      do_racestats,       POS_DEAD,        0,LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "innate",         do_innate,      POS_DEAD,        0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "score",          do_score,       POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "trades",         do_trades,      POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "socials",        do_socials,     POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "spells",         do_spells,      POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "story",          do_story,       POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "style",          do_style,       POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "time",           do_time,        POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "typo",           do_typo,        POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "total",          do_total,       POS_DEAD,        0,  LOG_NORMAL,1, 0, 0,0,1,0 },
    { "weather",        do_weather,     POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "who",            do_who,         POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "tribes",         do_tribes,      POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "clanwho",        do_clanwho,     POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "cwho",           do_clanwho,     POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "claninduct",     do_induct,      POS_RESTING,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "wizlist",        do_wizlist,     POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "worth",          do_worth,       POS_SLEEPING,    0,  LOG_NORMAL,1,0, 0, 0,0,0 },

    /*
     * OLC commands.
     */

    { "build",          do_build,       POS_SLEEPING,    120,  LOG_NORMAL,1,0, 0, 0,0,0 },
    { "dig",            do_dig,         POS_SLEEPING,    ROLC,  LOG_NORMAL,1,0, 0, 0,0,0 },
    { "olc",            do_olc,         POS_SLEEPING,    ROLC,  LOG_NORMAL,1,0, 0, 0,0,0 },
    { "findfreernum",   do_findfreernum,POS_SLEEPING,    OLC,  LOG_NORMAL,1,0, 0, 0,0,0 },
    { "molc",           do_molc,        POS_SLEEPING,    OLC,  LOG_NORMAL,1,0, 0, 0,0,0 },
    { "oolc",           do_oolc,        POS_SLEEPING,    OLC,  LOG_NORMAL,1,0, 0, 0,0,0 },
    { "validateprog",   do_validate_prog,POS_SLEEPING,    OLC,  LOG_NORMAL,1,0, 0, 0,0,0 },


    /*
     * Configuration commands.
     */
    { "alia",           do_alia,        POS_DEAD,        0,  LOG_NORMAL,0, 0, 0, 0,1,0 },
    { "alias",          do_alias,       POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "artifacts",      do_artifacts,   POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "bathe",          do_bathe,       POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "autolist",       do_autolist,    POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "automenu",       do_automenu,    POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "autoassist",     do_autoassist,  POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "autoexit",       do_autoexit,    POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "autoscan",       do_autoscan,    POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "autogold",       do_autogold,    POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "autoloot",       do_autoloot,    POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "autosac",        do_autosac,     POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "autosplit",      do_autosplit,   POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "brief",          do_brief,       POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
/*  { "channels",       do_channels,    POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 }, */
    { "colour",         do_colour,      POS_DEAD,        0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "color",         do_colour,      POS_DEAD,        0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "config",         do_configure,      POS_DEAD,        120,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "ansi",         do_colour,      POS_DEAD,        0,  LOG_NORMAL,1,0,0, 0,1,0 },
    { "compact",        do_compact,     POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "housedescription",do_housedescription, POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "description",    do_description, POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "plan",           do_plan,        POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "delet",          do_delet,       POS_DEAD,        0,  LOG_ALWAYS,0, 0, 0, 0,1,0 },
    { "delete",         do_delete,      POS_DEAD,    0,  LOG_ALWAYS,1, 0, 0, 0,1,0 },
    { "fullauto",       do_fullauto,    POS_DEAD,        0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "nofollow",       do_nofollow,    POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "nosummon",       do_nosummon,    POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "password",       do_password,    POS_DEAD,        0,  LOG_NEVER,1, 0, 0, 0,1,0 },
    { "prompt",         do_prompt,      POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "fightprompt",    do_fightprompt, POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "combattprompt",  do_fightprompt, POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "scroll",         do_scroll,      POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "title",          do_title,       POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "unalias",        do_unalias,     POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "wimpy",          do_wimpy,       POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "primary",        do_primary,     POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "hover",          do_hover,       POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "land",           do_land,        POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "screensplit",    do_screensplit, POS_DEAD,      120,  LOG_NORMAL,1,
0, 0, 0,1,0 },

    /*
     * Communication commands.
     */
    { "afk",            do_afk,         POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "answer",         do_answer,      POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "deaf",           do_deaf,        POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "emote",          do_emote,       POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "email",          do_email,       POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "homepage",       do_homepage,    POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "register",       do_register,    POS_DEAD,        0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "reregister",     do_reregister,  POS_DEAD,        120,LOG_NORMAL,1,0, 0, 0,1,0 },
    { "realname",       do_realname,    POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "pmote",          do_pmote,       POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { ".",              do_gossip,      POS_SLEEPING,    0,  LOG_NORMAL,0, 0, 0, 0,1,0 },
    { "chat",           do_gossip,      POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { ",",              do_emote,       POS_RESTING,     0,  LOG_NORMAL,0, 0, 0, 0,1,0 },
    { "grats",          do_grats,       POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "gtell",          do_gtell,       POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { ";",              do_gtell,       POS_DEAD,        0,  LOG_NORMAL,0, 0, 0, 0,1,0 },
/*  { "music",          do_music,       POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0}, */
    { "note",           do_note,        POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "plead",          do_question,    POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "quote",          do_quote,       POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "quest",          do_quest,       POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "quiet",          do_quiet,       POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "reply",          do_reply,       POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "replay",         do_replay,      POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "say",            do_say,         POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "think",          do_think,       POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "'",              do_say,         POS_RESTING,     0,  LOG_NORMAL,0, 0, 0, 0,1,0 },
    { "shout",          do_shout,       POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "unread",         do_unread,      POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "yell",           do_yell,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "call",           do_call,        POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_CRUS, 3,0,0 },
    { "reveal",         do_reveal,      POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_ELEM, 5,0,0 },
    { "youth",          do_youth,       POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_VIVI, 6,0,0 },
    { "heal",           do_heal,        POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_VIVI, 9,0,0 },

    /*
     * Object manipulation commands.
     */
    { "brandish",       do_brandish,    POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "close",          do_close,       POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "clear",          do_clear,       POS_SLEEPING,    0,  LOG_NORMAL,1,0, 0, 0,1,0 },
    { "cls",            do_clear,       POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "drink",          do_drink,       POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "dream",          do_dream,       POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "drop",           do_drop,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "eat",            do_eat,         POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "envenom",        do_envenom,     POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "fill",           do_fill,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "scribe",         do_scribe,      POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "give",           do_give,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "hold",           do_wear,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "list",           do_list,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "lock",           do_lock,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "open",           do_open,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "pick",           do_pick,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "pour",           do_pour,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "put",            do_put,         POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "quaff",          do_quaff,       POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "recite",         do_recite,      POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "remove",         do_remove,      POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "sell",           do_sell,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "take",           do_get,         POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "findwater",      do_findwater,   POS_STANDING,    0,  LOG_NORMAL,1, 0, -TRADE_SURVIVAL, 4,0,0 },
    { "sacrifice",      do_sacrifice,   POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "junk",           do_sacrifice,   POS_RESTING,     0,  LOG_NORMAL,0, 0, 0, 0,0,0 },
    { "tap",            do_sacrifice,   POS_RESTING,     0,  LOG_NORMAL,0, 0 ,0, 0,0,0},   
/*  { "unlock",         do_unlock,      POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 }, */
    { "value",          do_value,       POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "wear",           do_wear,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "zap",            do_zap,         POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },

    /*
     * Combat commands.
     */
    { "backstab",       do_backstab,    POS_FIGHTING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
//    { "bash",           do_bash,        POS_FIGHTING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "bs",             do_backstab,    POS_FIGHTING,    0,  LOG_NORMAL,0, 0, 0, 0,0,0 },
//    { "berserk",        do_berserk,     POS_FIGHTING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "breathe",        do_breathe,     POS_FIGHTING,    0,  LOG_NORMAL,1, 1, DISC_DRAG_FIRE, 1,0,0 },
    { "expell",         do_expell,      POS_FIGHTING,    0,  LOG_NORMAL,1, 1, DISC_DRAG_FIRE, 4,0,0 },
    { "shadowbreath",   do_shadow_breath, POS_FIGHTING,  0,  LOG_NORMAL,1, 1, DISC_DRAG_FIRE, 1,0,0 },
    { "fightstyle",     do_fightstyle,  POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,0,0},
    { "fs",             do_fightstyle,  POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,0,0},
    { "fsmenu",         do_fsmenu,      POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,0,0},
//    { "dirt",           do_dirt,        POS_FIGHTING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0},
//    { "disarm",         do_disarm,      POS_FIGHTING,    0,  LOG_NORMAL,1, 0, 0, 0 ,0,0},
    { "escape",         do_escape,      POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "forage",         do_forage,      POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "flee",           do_flee,        POS_FIGHTING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
//    { "kick",           do_kick,        POS_FIGHTING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "rescue",         do_rescue,      POS_FIGHTING,    0,  LOG_NORMAL,0, 0, 0, 0,0,0 },
    { "squish",         do_squish,      POS_FIGHTING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "stance",         do_stance,      POS_FIGHTING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
//    { "trip",           do_trip,        POS_FIGHTING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },

    /*
     * Miscellaneous commands.
     */
    { "enter",          do_enter,       POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "follow",         do_follow,      POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "go",             do_enter,       POS_STANDING,    0,  LOG_NORMAL,0, 0, 0, 0,0,0 },
    { "mount",          do_mount,       POS_STANDING,    0,  LOG_NORMAL,0, 0, 0, 0,0,0 },
    { "dismount",       do_dismount,    POS_STANDING,    0,  LOG_NORMAL,0, 0, 0, 0,0,0 },
/*  { "group",          do_group,       POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 }, */
    { "hide",           do_hide,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "learn",          do_learn,       POS_STANDING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "play",           do_play,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "skills",         do_skills,      POS_SLEEPING,    0,LOG_NORMAL,1, 0 , 0, 0,0,0},
    { "plant",          do_plant,       POS_SLEEPING,    0,LOG_NORMAL,1, 0 , 0, 0,0,0},
    { "quit",           do_quit,        POS_DEAD,        0,  LOG_ALWAYS,1, 0, 0, 0,0,0 },
    { "kaiprayer",      do_kaiprayer,   POS_RESTING,     0,  LOG_ALWAYS,1, 0, 0, 0,1,0 },    
    { "save",           do_save,        POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "scan",           do_scan,        POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "scalp",          do_scalp,       POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },    
    { "sleep",          do_sleep,       POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "sneak",          do_sneak,       POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "split",          do_split,       POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "dragonpromote",  do_dragonpromote,POS_RESTING,  120,  LOG_NORMAL,1,
0, 0, 0,0,0 },
    { "steal",          do_steal,       POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "train",          do_train,       POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "visible",        do_visible,     POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "wake",           do_wake,        POS_SLEEPING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "propose",        do_propose,     POS_STANDING,    0,  LOG_NORMAL,1,0, 0, 0,0,0 },
    { "accept",         do_accept,      POS_STANDING,    0,  LOG_NORMAL,1,0, 0, 0,0,0 },
    { "prefix",         do_prefix,      POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "beg",            do_beg,         POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "stake",          do_stake,       POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,0,0 },

    /*
     * Race specific commands.
     */
    
    
    { "elder",          do_elder,       POS_STANDING,    0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "kaielder",       do_kaielder,    POS_STANDING,    0,  LOG_ALWAYS,1, 0, 0, 0,1,0 },
    { "promote",        do_promote,     POS_STANDING,    0,  LOG_ALWAYS,1, 2, 0, 0,0,0 },
    { "commune",        do_commune,     POS_SLEEPING,    0,  LOG_NORMAL,1, 2, 0, 0,1,0 },
    { "whisper",        do_whisper,     POS_SLEEPING,    0,  LOG_NORMAL,1, 3, 0, 0,1,0 },
    { "howl",           do_howl,        POS_SLEEPING,    0,  LOG_NORMAL,1, 4, 0, 0,1,0 },
    { "sing",           do_chant,       POS_SLEEPING,    0,  LOG_NORMAL,1, 5, 0, 0,1,0 },
    { "incant",         do_project,     POS_SLEEPING,    0,  LOG_NORMAL,1, 6, 0, 0,1,0 },
    { "pray",           do_pray,        POS_SLEEPING,    0,  LOG_NORMAL,1, 7, 0, 0,1,0 },
    { ">",              do_commune,     POS_SLEEPING,    0,  LOG_NORMAL,1, 2, 0, 0,1,0 },
    { ">",              do_whisper,     POS_SLEEPING,    0,  LOG_NORMAL,1, 3, 0, 0,1,0 },
    { ">",              do_howl,        POS_SLEEPING,    0,  LOG_NORMAL,1, 4, 0, 0,1,0 },
    { ">",              do_chant,       POS_SLEEPING,    0,  LOG_NORMAL,1, 5, 0, 0,1,0 },
    { ">",              do_project,     POS_SLEEPING,    0,  LOG_NORMAL,1, 6, 0, 0,1,0 },
    { ">",              do_pray,        POS_SLEEPING,    0,  LOG_NORMAL,1, 7, 0, 0,1,0 },
    { "embrace",        do_embrace,     POS_STANDING,    0,  LOG_NORMAL,1, 3, 0, 0,0,0 },
    { "spike",          do_spike,       POS_FIGHTING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_SATY, 5,0,0 },
    { "maw",            do_maw,         POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_SATY, 1,0,0 },
    { "tiger",          do_tiger,       POS_FIGHTING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_SATY, 8,0,0 },
    { "food",           do_food,        POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_SATY, 10,0,0 },
    { "unchain",        do_unchain,     POS_FIGHTING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_SATY, 9,0,0 },
    { "roar",           do_sphinx_roar, POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_SATY, 6,0,0 },
    { "entrance",       do_entrance,    POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_DOMI, 2,0,0 },
    { "bite",           do_bite,        POS_STANDING,    0,  LOG_ALWAYS,1, 3, 0, 0,0,0 },
    { "introduce",      do_introduce,   POS_RESTING,     0,  LOG_NORMAL,1, 3, 0, 0,0,0 },
    { "scry",           do_scry,        POS_RESTING,     0,  LOG_NORMAL,1, 3, DISC_VAMP_AUSP, 7,1,0 },
    { "readaura",       do_readaura,    POS_RESTING,     0,  LOG_NORMAL,1, 3, DISC_VAMP_AUSP, 2,1,0 },
    { "readaura",       do_readaura,    POS_RESTING,     0,  LOG_NORMAL,1, 5, 0, 0,1,0 },
    { "crush",          do_crush,	POS_RESTING,     0,  LOG_NORMAL,1, 1, DISC_DRAG_PROW, 9,0,0 },
    { "plummet",        do_plummet,	POS_STANDING,    0,  LOG_NORMAL,1, 1, DISC_DRAG_PROW, 10,0,0 },
    { "devour",         do_dragon_devour,POS_RESTING,     0,  LOG_NORMAL,1, 1, 0, 0,0,0 },
    { "devoursoul",     do_devour,      POS_STANDING,    0,  LOG_ALWAYS,1, 2, 0, 0,0,0 },
    { "teach",          do_teach,       POS_STANDING,    0,  LOG_ALWAYS,1, 6, 0, 0,0,0 },
    { "sprinkle",       do_sprinkle,    POS_STANDING,    0,  LOG_ALWAYS,1, 5, 0, 0,0,0 },
    { "paths",          do_paths,       POS_STANDING,    0,  LOG_NORMAL,1, 5, 0, 0,1,0 },
    { "callfaerie",     do_call_faerie, POS_STANDING,    0,  LOG_NORMAL,1, 5, 0, 0,1,0 },
    { "faeriegate",     do_faeriegate,  POS_STANDING,    0,  LOG_NORMAL,1, 5, 0, 0,1,0 },
    { "watershield",    do_water_shield,POS_STANDING,    0,  LOG_NORMAL,1, 5, 0, 0,0,0 },
    { "hawkform",       do_hawkform,    POS_STANDING,    0,  LOG_NORMAL,1, 4, DISC_LUPI_HAWK, 9,0,0 },
    { "feather",        do_feather,     POS_STANDING,    0,  LOG_NORMAL,1, 4, DISC_LUPI_HAWK, 2,0,0 },
    { "fly",            do_fly,         POS_STANDING,    0,  LOG_NORMAL,1, 4, DISC_LUPI_HAWK, 10,0,0 },
    { "shriek",         do_shriek,      POS_STANDING,    0,  LOG_NORMAL,1, 4, DISC_LUPI_HAWK, 6,0,0 },
    { "entrap",         do_entrap,      POS_FIGHTING,    0,  LOG_NORMAL,1, 4, DISC_LUPI_HAWK, 5,0,0 },
    { "infect",         do_infect,      POS_STANDING,    0,  LOG_ALWAYS,1, 4, 0, 0,0,0 },
    { "tolerate",       do_tolerate,    POS_STANDING,    0,  LOG_NORMAL,1, 4, 0, 0,0,0 },
    { "gate",           do_gate,        POS_STANDING,    0,  LOG_NORMAL,1, 2, 0, 0,1,0 },
    { "disciplines",    do_disciplines, POS_SLEEPING,    0,  LOG_NORMAL,1, 8, 0, 0,1,0 },
    { "disciplines",    do_disciplines, POS_SLEEPING,    0,  LOG_NORMAL,1, 1, 0, 0,1,0 },
    { "disciplines",    do_disciplines, POS_SLEEPING,    0,  LOG_NORMAL,1, 2, 0, 0,1,0 },
    { "disciplines",    do_disciplines, POS_SLEEPING,    0,  LOG_NORMAL,1, 3, 0, 0,1,0 },
    { "guardians",      do_disciplines, POS_SLEEPING,    0,  LOG_NORMAL,1, 4, 0, 0,1,0 },
    { "disciplines",    do_disciplines, POS_SLEEPING,    0,  LOG_NORMAL,1, 5, 0, 0,1,0 },
    { "disciplines",    do_disciplines, POS_SLEEPING,    0,  LOG_NORMAL,1, 6, 0, 0,1,0 },
    { "disciplines",    do_disciplines, POS_SLEEPING,    0,  LOG_NORMAL,1, 7, 0, 0,1,0 },
    { "eaglesight",     do_eaglesight,  POS_RESTING,     0,  LOG_NORMAL,1, 3, DISC_VAMP_AUSP, 9,1,0 },
    { "bloodsense",     do_where,       POS_RESTING,     0,  LOG_NORMAL,1, 3, 0, 0,1,0 },
    { "lifesense",      do_where,       POS_RESTING,     0,  LOG_NORMAL,1, 4, 0, 0,1,0 },
    { "track",          do_track,       POS_STANDING,    0,  LOG_NORMAL,1, 4, 0, 0,0,0 },
    { "wings",          do_wings,       POS_STANDING,    0,  LOG_NORMAL,1, 4, 0, 0,0,0 },
    { "wings",          do_wings,       POS_STANDING,    0,  LOG_NORMAL,1, 5, 0, 0,0,0 },
    { "pounce",         do_pounce,      POS_STANDING,   0,LOG_NORMAL,1, 4, DISC_LUPI_LEON, 4,0,0 },
    { "wolf",           do_wolf,        POS_STANDING,   0,LOG_NORMAL,1, 4, 0, 0,0,0 },
    { "regenerate",     do_regenerate,  POS_RESTING,     0,  LOG_NORMAL,1, 3, 0, 0,1,0 },
    { "upkeep",         do_upkeep,      POS_SLEEPING,    0,  LOG_NORMAL,1, 3, 0, 0,1,0 },
    { "bane",           do_bane,        POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_CORR, 10,0,0 },
    { "wither",         do_wither,      POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_CORR, 6,0,0 },
    { "web",            do_web,         POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_CORR, 4,0,0 },
    { "wings",          do_wings,       POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_ATTA, 5,0,0 },
    { "caust",          do_caust,       POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_CORR, 5,0,0 },
    { "stench",         do_stench,      POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_CORR, 8,0,0 },
    { "deathpool",      do_deathpool,   POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_CORR, 9,1,1 },
    { "inferno",        do_inferno,     POS_DEAD,        0,  LOG_NORMAL,1, 2, DISC_DAEM_HELL, 6,0,0 },
    { "destroy",        do_destroy,     POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_HELL, 8,0,0 },
    { "immolate",       do_immolate,    POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_HELL, 5,0,0 },
    { "summon",         do_summon,      POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_HELL, 10,0,0 },
    { "seed",           do_seed,        POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_HELL, 7,0,0 },
    { "shard",          do_shard,       POS_RESTING,     0,  LOG_NORMAL,1, 2, DISC_DAEM_GELU, 3,0,0 },
    { "deathsense",     do_deathsense,  POS_RESTING,     0,  LOG_NORMAL,1, 2, DISC_DAEM_NETH, 2,0,0 },
    { "breathe",        do_frostbreath, POS_RESTING,     0,  LOG_NORMAL,1, 2, DISC_DAEM_GELU, 8,0,0 },
    { "gust",           do_gust,        POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_GELU, 7,0,0 },
    { "hurl",           do_hurl,        POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DRAG_PROW, 6,0,0 },
    { "snowman",        do_snowman,     POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_GELU, 6,0,0 },
    { "stasis",         do_stasis,      POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_GELU, 9,0,0 },
    { "entomb",         do_entomb,      POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_GELU, 10,0,0 },
    { "devil",          do_devil,       POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_TEMP, 9,0,0 },
    { "tempt",          do_tempt,       POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_TEMP, 4,0,0 },
    { "evileye",        do_evileye,     POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_DISC, 4,0,0 },
    { "unnerve",        do_unnerve,     POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_DISC, 1,0,0 },
    { "forget",         do_forget,      POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_DISC, 6,0,0 },
    { "discord",        do_discord,     POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_DISC, 3,0,0 },
    { "shadowdemon",    do_shadowdemon, POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_TEMP, 8,0,0 },
    { "kiss",           do_kiss,        POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_TEMP, 2,0,0 },
    { "lure",           do_lure,        POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_TEMP, 6,0,0 },
    { "insight",        do_insight,     POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_TEMP, 5,0,0 },
    { "breakcharm",     do_breakcharm,  POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_TEMP, 3,0,0 },
    { "darkvision",     do_dark_vision, POS_RESTING,     0,  LOG_NORMAL,1, 3, DISC_VAMP_AUSP, 1,0,0 },
    { "touch",          do_touch,	POS_RESTING,     0,  LOG_NORMAL,1, 3, DISC_VAMP_AUSP, 6,0,0 },
    { "truesight",      do_truesight,   POS_RESTING,     0,  LOG_NORMAL,1, 3, DISC_VAMP_AUSP, 4,0,0 },
    { "league",         do_league,      POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "decapitate",     do_decap,       POS_FIGHTING,    0,  LOG_ALWAYS,1, 0, 0, 0,0,0 },
    { "archkai",        do_archkai,     POS_STANDING,    0,  LOG_ALWAYS,1, 0, 0 ,0,0,0 },
    { "vampallow",      do_vampallow,   POS_FIGHTING,    0,  LOG_ALWAYS,1, 0, 0, 0,0,0 },
    { "soulsell",       do_demonallow,  POS_FIGHTING,    0,  LOG_ALWAYS,1, 0, 0, 0,0,0 },
    { "dedicate",       do_warlockallow,POS_FIGHTING,    0,  LOG_ALWAYS,1, 0, 0, 0,0,0 },
    { "submit",         do_wereallow,   POS_FIGHTING,    0,  LOG_ALWAYS,1, 0, 0, 0,0,0 },
    { "pledge",         do_eladallow,   POS_FIGHTING,    0,  LOG_ALWAYS,1, 0, 0, 0,0,0 },
    { "condition",      do_condition,   POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_DOMI, 4,0,0 },
    { "tasteblood",     do_thaumaone,   POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_THAU, 1,0,0 },
    { "bloodrage",      do_thaumatwo,   POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_THAU, 2,0,0 },
    { "potency",        do_thaumathree, POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_THAU, 3,0,0 },
    { "drawblood",      do_thaumafour,  POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_THAU, 4,0,0 },
    { "cauldron",       do_thaumafive,  POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_THAU, 5,0,0 },
    { "airblood",       do_airblood,    POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_THAU, 6,0,0 },
    { "brew",           do_brew,        POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_BREW, 1,0,0 },
    { "mix",            do_mix,         POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_BREW, 3,0,0 },
    { "recharge",       do_recharge,    POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_ENCH, 3,0,0 },
//    { "tome",           do_tome,        POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_BIND, 1,0,0 },
    { "necromancy",     do_necromancy,  POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_THEA, 4,0,0 },
    { "familiar",       do_familiar,    POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_THEA, 1,0,0 },
    { "enhance",        do_enhance,     POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_THEA, 2,0,0 },
    { "callpet",        do_callpet,     POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_THEA, 5,0,0 },
    { "divine",         do_divine,      POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_THEA, 6,0,0 },
    { "backfire",       do_backfire,    POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_THEA, 8,0,0 },
//    { "flip",           do_flip,        POS_FIGHTING,    0,  LOG_NORMAL,1, 6, DISC_WARL_BIND, 1,0,0 },
    { "overload",       do_overload,    POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_ENCH, 5,0,0 },
    { "enchant",        do_enchant,     POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_ENCH, 1,0,0 },
    { "aura",           do_aura,        POS_FIGHTING,    0,  LOG_NORMAL,1, 5, DISC_ELAD_AURA, 1,0,0 },
    { "callrock",       do_callrock,    POS_FIGHTING,    0,  LOG_NORMAL,1, 5, DISC_ELAD_PHYS, 4,0,0 },
    { "treemeld",       do_tree_meld,   POS_SLEEPING,    0,  LOG_NORMAL,1, 5, DISC_ELAD_ARBO, 4,0,0 },
    { "roots",          do_roots,       POS_STANDING,    0,  LOG_NORMAL,1, 5, DISC_ELAD_ARBO, 7,0,0 },
    { "seneschia",      do_seneschia,   POS_STANDING,    0,  LOG_NORMAL,1, 5, DISC_ELAD_ARBO, 6,0,0 },
    { "entwine",        do_entwine,     POS_STANDING,    0,LOG_NORMAL,1, 5, DISC_ELAD_ARBO, 2,0,0 },
    { "smother",        do_smother,     POS_STANDING,    0,LOG_NORMAL,1, 5, DISC_ELAD_ARBO, 3,0,0 },
    { "treewalk",       do_tree_walk,   POS_SLEEPING,    0,LOG_NORMAL,1,5, DISC_ELAD_ARBO, 5,0,0 },
    { "aurashield",     do_shield,      POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_OBFU, 4,0,0 },
    { "melody",         do_melody,      POS_FIGHTING,    0,  LOG_NORMAL,1, 5, DISC_ELAD_MELO, 1,0,0 },
    { "feed",           do_feed,        POS_FIGHTING,    0,  LOG_NORMAL,1, 3, 0, 0,0,0 },
    { "horns",          do_horns,       POS_FIGHTING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_ATTA,4,0,0 },
    { "tail",           do_tail,        POS_FIGHTING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_SATY,4,0,0 },
    { "blink",          do_blink,       POS_FIGHTING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_ATTA,6,0,0 },
    { "claws",          do_claw,        POS_FIGHTING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_ATTA,2,0,0 },
    { "claws",          do_claw,        POS_FIGHTING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_PROT,2,0,0 },
    { "inhabit",        do_inhabit,     POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_PROT,6,0,0 },
    { "fangs",          do_fang,        POS_FIGHTING,    0,  LOG_NORMAL,1, 3, 0, 0,0,0 },
    { "claws",          do_claw,        POS_FIGHTING,    0,  LOG_NORMAL,1, 4, 0, 0,0,0 },
    { "claws",          do_claw,        POS_FIGHTING,    0,  LOG_NORMAL,1, 1, DISC_DRAG_PROW, 1,0,0 },
    { "fangs",          do_fang,        POS_FIGHTING,    0,  LOG_NORMAL,1, 4, 0, 0,0,0 },
    { "fear",           do_fear,        POS_FIGHTING,    0,  LOG_NORMAL,1, 1, DISC_DRAG_AWE, 1,0,0 },
    { "roar",           do_roar,        POS_FIGHTING,    0,  LOG_NORMAL,1, 1, DISC_DRAG_AWE, 1,0,0 },
    { "meld",           do_meld,        POS_SLEEPING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_PROT, 3,0,0 },
    { "song",           do_song,        POS_STANDING,    0,  LOG_NORMAL,1,3, DISC_VAMP_ANIM, 2,0,0 },
    { "soothe",         do_song,        POS_STANDING,    0,  LOG_NORMAL,1,7, DISC_HERO_VIVI, 4,0,0 },
    { "beast",          do_beast,       POS_STANDING,    0,  LOG_NORMAL,1,3, DISC_VAMP_ANIM, 3,0,0 },
    { "beckon",         do_beckon,      POS_STANDING,    0,  LOG_NORMAL,1,3, DISC_VAMP_ANIM, 4,0,0 },
    { "succulence",     do_succulence,  POS_STANDING,    0,  LOG_NORMAL,1,3, DISC_VAMP_ANIM, 6,0,0 },
    { "mass", 	        do_mass,	POS_STANDING,    0,  LOG_NORMAL,1,3, DISC_VAMP_ANIM, 9,0,0 },
    { "borrowlife",     do_borrowlife,  POS_STANDING,    0,  LOG_NORMAL,1,3, DISC_VAMP_ANIM, 5,0,0 },
    { "swarm",          do_swarm,       POS_FIGHTING,    0,  LOG_NORMAL,1,3, DISC_VAMP_ANIM, 8,0,0 },
    { "rage",           do_rage,        POS_FIGHTING,    0,  LOG_NORMAL,1, 1, DISC_DRAG_PROW, 3,0,0 },
    { "rage",           do_rage,        POS_FIGHTING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_ATTA, 2,0,0 },
    { "rage",           do_rage,        POS_FIGHTING,    0,  LOG_NORMAL,1, 4, DISC_LUPI_LEON, 2,0,0 },
    { "calm",           do_calm,        POS_FIGHTING,    0,  LOG_NORMAL,1,1, DISC_LUPI_LEON, 3,0,0 },    
    { "calm",           do_calm,        POS_FIGHTING,    0,  LOG_NORMAL,1,1, DISC_DRAG_PROW, 4,0,0 },    
    { "calm",           do_calm,        POS_FIGHTING,    0,  LOG_NORMAL,1,2, DISC_DAEM_ATTA, 3,0,0 },    
    { "shadowwalk",     do_shadowwalk,  POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_OBFU, 5,0,0 },
    { "rampage",        do_rampage,     POS_FIGHTING,    0,  LOG_NORMAL,1, 4, DISC_LUPI_LEON, 5,0,0 },    
    { "spit",           do_spit,        POS_FIGHTING,    0,  LOG_NORMAL,1, 4, DISC_LUPI_COBR, 2,0,0 },    
    { "shadowsight",    do_shadowsight, POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_AUSP, 5,0,0 },
    { "artificia",      do_artificia,   POS_STANDING,    0,  LOG_NORMAL,1, 5, DISC_ELAD_ARTI, 1,0,0 },
    { "sanctify",       do_sanctify,    POS_STANDING,    0,  LOG_NORMAL,1, 5, DISC_ELAD_ARTI, 4,0,0 },
    { "infuse",         do_infuse,      POS_STANDING,    0,  LOG_NORMAL,1, 5, DISC_ELAD_ARTI, 10,0,0 },
    { "touch",	        do_touch,       POS_STANDING,    0,  LOG_NORMAL,1, 5, DISC_ELAD_ARTI, 6,0,0 },
    { "mask",           do_mask,        POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_OBFU, 7,0,0 },
    { "cloak",          do_cloak,       POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_OBFU, 2,0,0 },
    { "vanish",         do_vanish,      POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_OBFU, 3,0,0 },
    { "vanish",         do_vanish,      POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_LUPI_LUNA, 2,0,0 },
    { "darkness",       do_darkness,    POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_LUPI_LUNA, 3,0,0 },
    { "moonstrike",     do_moonstrike,  POS_FIGHTING,    0,  LOG_NORMAL,1, 3, DISC_LUPI_LUNA, 9,0,0 },
    { "flicker",        do_flicker,     POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_OBFU, 8,0,0 },
    { "shadowhide",     do_shadow_hide, POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_OBFU, 9,0,0 },
    { "itemhide",       do_itemhide,    POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_OBFU, 6,0,0 },
    { "phase",          do_phase,       POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_OBFU, 10,0,0 },
    { "command",        do_command,     POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_DOMI, 1,1,0 },
    { "mesmerise",      do_vamp_mesmerise,POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_DOMI, 2,1,0 },
    { "insight",        do_insight,     POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_DOMI, 7,1,0 },
    { "love",           do_love,	POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_PRES, 8,1,0 },
    { "mesmerise",      do_mesmerise,   POS_STANDING,    0,  LOG_NORMAL,1, 4, DISC_LUPI_COBR, 3,0,0 },    
    { "commands",       do_commands,    POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "boost",          do_boost,       POS_FIGHTING,    0,  LOG_NORMAL,1, 3, 0, 0,0,0 },
    { "sunlight",       do_sunlight,    POS_STANDING,    0,  LOG_NORMAL,1,3, DISC_VAMP_FORT, 4,0,0 },
    { "pcast",          do_pcast,       POS_RESTING,     0,  LOG_NORMAL,1, 6, DISC_WARL_LUCU, 2,0,0 },
    { "arcanum",        do_arcanum,     POS_RESTING,     0,  LOG_NORMAL,1, 6, DISC_WARL_LUCU, 6,0,0 },
    { "aroma",          do_aroma,       POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_THEA, 7,0,0 },
    { "component",      do_component,   POS_RESTING,     0,  LOG_NORMAL,1, 6, DISC_WARL_LUCU, 1,0,0 },
    { "sip",            do_sip,         POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_BREW, 4,0,0 },
    { "transfer",       do_transfer,    POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_LOCATION },
    { "transform",      do_transform,   POS_STANDING,    0,  LOG_NORMAL,1, 2,DISC_DAEM_MORP, 1,1,0 },
    { "polymorph",      do_polymorph,   POS_STANDING,    0,  LOG_NORMAL,1, 2,DISC_DAEM_MORP, 4,0,0 },
    { "polymorph",      do_polymorph,   POS_STANDING,    0,  LOG_NORMAL,1, 1,DISC_DRAG_POLY, 1,0,0 },
    { "graft",          do_graft,       POS_STANDING,    0,  LOG_NORMAL,1, 2,DISC_DAEM_MORP, 2,0,0 },
    { "transport",      do_transport,   POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_FARR, 4,0,0 },
    { "nexus",          do_nexus,       POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_FARR, 1,0,0 },
    { "nexuscall",      do_nexuscall,   POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_FARR, 9,0,0 },
    { "nexusgate",      do_nexusgate,   POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_FARR, 6,0,0 },
    { "subvert",        do_subvert,     POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_CONT, 7,0,0 },
    { "runes",          do_runes,       POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_RUNE, 1,0,0 },
    { "revcast",        do_revcast,     POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_CONT, 1,0,0 },
    { "bless",          do_bless,       POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_CRUS, 1,0,0 },
    { "wall",           do_wall,        POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_CRUS, 6,0,0 },
    { "truefaith",      do_truefaith,   POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_TRUE, 1,0,0 },
    { "sigil",          do_sigil,       POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_SIGI, 1,0,0 },
    { "glyph",          do_glyph,       POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_SIGI, 2,0,0 },
    { "layhands",       do_layhands,    POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_VIVI, 1,0,0 },
    { "relics",         do_relics,      POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_CRUS, 4,0,0 },
    { "wrench",         do_wrench,      POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_EXOR, 4,0,0 },
    { "dispel",         do_dispel,      POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_EXOR, 6,0,0 },
    { "dolls",          do_voohunt,     POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_EXOR, 7,0,0 },
    { "exorcise",       do_exorcise,    POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_EXOR, 8,0,0 },
    { "turn",           do_turn,        POS_STANDING,    0,  LOG_NORMAL,1, 7, DISC_HERO_EXOR, 3,0,0 },
    { "sharpen",        do_sharpen,     POS_STANDING,    0, LOG_NORMAL,1, 7, DISC_HERO_EXOR, 5,0,0 },
    { "preserve",       do_preserve,    POS_STANDING,    0,  LOG_NORMAL,1,7, DISC_HERO_EXOR, 1,0,0 },
    { "preserve",       do_preserve,    POS_STANDING,    0,LOG_NORMAL,1,6, DISC_WARL_ENCH, 1,0,0 },

    /*
     * Immortal commands.
     */
    { "vision",         do_vision,    	POS_DEAD,      120,LOG_NORMAL,1, 0, 0, 0,1,CMD_GLOBAL },
    { "treasure",       do_treasure,    POS_DEAD,      120,  LOG_NORMAL,1, 0, 0, 0,1,CMD_GLOBAL },
    { "proglist",       do_proglist,    POS_DEAD,      120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_GLOBAL },
    { "langtest",       do_langtest,    POS_DEAD,      120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_GLOBAL },
    { "tick",           do_tick,        POS_DEAD,      120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_GLOBAL },
    { "reset",          do_reset,       POS_DEAD,      120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_GLOBAL },
    { "reimburse",      do_reimburse,   POS_DEAD,      120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_GLOBAL },
    { "advance",        do_advance,     POS_DEAD,      120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONTROL },
    { "dump",           do_dump,        POS_DEAD,      120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_GLOBAL },
    { "trust",          do_trust,       POS_DEAD,      120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONTROL },
    { "violate",        do_violate,     POS_DEAD,      120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_LOCATION },
    { "allow",          do_allow,       POS_DEAD,       112,LOG_ALWAYS,1, 0, 0, 0,1,CMD_SITES },
    { "ban",            do_ban,         POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_SITES },
    { "deny",           do_deny,        POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_SITES },
    { "immtrust",       do_immtrust,    POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONTROL },
    { "flag",           do_flag,        POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_LOAD },
    { "freeze",         do_freeze,      POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONTROL },
    { "permban",        do_permban,     POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_SITES },
    { "protect",        do_protect,     POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONTROL },
    { "grant",          do_grant,       POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_GLOBAL },
    { "reboo",          do_reboo,       POS_DEAD,       112,  LOG_NORMAL,0, 0, 0, 0,1,CMD_REBOOT },
    { "reboot",         do_reboot,      POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_REBOOT },
    { "materials",      do_materials,   POS_DEAD,       120,  LOG_NORMAL,1, 0, 0, 0,1,CMD_FUN },
    { "make",      	do_make,	POS_DEAD,       120,  LOG_NORMAL,1, 0,0, 0,1,CMD_FUN },
    { "set",            do_set,         POS_DEAD,   CHRIS_ONLY,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_LOAD },
    { "setup",    	do_setup,	POS_DEAD,        0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "setdisciplines", do_setdisciplines, POS_DEAD,CHRIS_ONLY,LOG_ALWAYS, 1, 0, 0, 0,1,CMD_CONTROL },
    { "dataload",       do_dataload,    POS_DEAD,       120,  LOG_NORMAL,1, 0, 0, 0,1,CMD_CONTROL },
    { "shutdow",        do_shutdow,     POS_DEAD,       112,  LOG_NORMAL,0, 0, 0, 0,1,CMD_REBOOT },
    { "shutdown",       do_shutdown,    POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_REBOOT },
    { "copyover",       do_copyover,    POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_REBOOT },
/*  { "sockets",        do_sockets,     POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_SITES }, */
    { "wizlock",        do_wizlock,     POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONNECTIONS },
    { "force",          do_force,       POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONTROL },
    { "load",           do_load,        POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_LOAD },
    { "mload",          do_mload,       POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_LOAD },
    { "oload",          do_oload,       POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_LOAD },
    { "sprout",         do_sprout,      POS_DEAD,       120,  LOG_NORMAL,1, 0, 0, 0,1,CMD_FUN },
    { "makecool",       do_makecool,    POS_DEAD,       120,LOG_NORMAL, 1,0, 0, 0,1,CMD_FUN },
    { "makebuilder",    do_makebuilder, POS_DEAD,       120,LOG_NORMAL, 1,0, 0, 0,1,CMD_FUN },
    { "newlock",        do_newlock,     POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONNECTIONS },
    { "noemote",        do_noemote,     POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CHANNELS },
    { "noreply",        do_noreply,     POS_DEAD,       120,  LOG_ALWAYS,0, 0, 0, 0,1,0 },
    { "noshout",        do_noshout,     POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CHANNELS },
    { "notell",         do_notell,      POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CHANNELS },
    { "pecho",          do_pecho,       POS_DEAD,       112,  LOG_NEVER,1, 0, 0, 0,1,CMD_ECHO }, 
    { "purge",          do_purge,       POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONNECTIONS },
    { "restore",        do_restore,     POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONTROL },
    { "slay",           do_slay,        POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONTROL },
    { "teleport",       do_transfer,    POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0 ,0,1,CMD_LOCATION },       
    { "poofin",         do_bamfin,      POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_LOCATION },
    { "poofout",        do_bamfout,     POS_DEAD,       112,  LOG_NORMAL,1, 0, 0 ,0,1,CMD_LOCATION },
    { "gecho",          do_echo,        POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0 ,0,1,CMD_ECHO },
    { "holylight",      do_holylight,   POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_CONTROL },
    { "incognito",      do_incognito,   POS_DEAD,       112,LOG_NORMAL,1, 0, 0, 0,1,CMD_CONTROL },
    { "invis",          do_invis,       POS_DEAD,       112,LOG_NORMAL,0, 0, 0, 0,0,CMD_CONTROL },
    { "log",            do_log,         POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_GLOBAL },
    { "memory",         do_memory,      POS_DEAD,       120,  LOG_NORMAL,1, 0, 0, 0,1,CMD_GLOBAL },
    { "mwhere",         do_mwhere,      POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_LOAD },
    { "owhere",         do_owhere,      POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_LOAD },
    { "peace",          do_peace,       POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_CONTROL },
    { "peace",          do_elad_peace,  POS_STANDING,    0,  LOG_NORMAL,1, 5, DISC_ELAD_ARBO, 10,0,0 },
    { "pen",            do_pen,         POS_STANDING,    0,  LOG_NORMAL,1, 6, DISC_WARL_SCRI, 1,0,0 },
    { "penalty",        do_penalty,     POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_GLOBAL },
    { "echo",           do_recho,       POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_ECHO },
    { "return",         do_return,      POS_DEAD,         0,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "oswitch",        do_oswitch,     POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_SWITCH },
    { "ostats",         do_ostats,      POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_SWITCH },
    { "oreturn",        do_oreturn,     POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_SWITCH },
    { "smite",          do_smite,       POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_FUN },        
    { "snoop",          do_snoop,       POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONTROL },
    { "timer",          do_timer,       POS_DEAD,       120,LOG_ALWAYS,1,0, 0, 0,1,CMD_CONTROL },
    { "stat",           do_stat,        POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_LOAD },
    { "stats",      do_racestats,       POS_DEAD,        0,LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "statarea",       do_statarea,    POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "string",         do_string,      POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_LOAD },
    { "disconnect",     do_disconnect,  POS_DEAD,       0,    LOG_ALWAYS,1, 0, 0, 0,1,0 },
    { "switch",         do_switch,      POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_SWITCH },
    { "twit",           do_twit,        POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_FUN },
    { "animate",        do_animate,     POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_FUN },
    { "beep",           do_beep,        POS_DEAD,       120,LOG_NORMAL,1, 0, 0, 0,1,CMD_FUN },
    { "weirdweather",   do_weirdweather,POS_DEAD,       120,LOG_NORMAL,1, 0, 0, 0,1,CMD_FUN },
    { "testlink",       do_testlink,    POS_DEAD,       120,LOG_NORMAL,1, 0, 0, 0,1,CMD_FUN },
    { "resetarea",      do_resetarea,   POS_DEAD,       120,LOG_NORMAL,1, 0, 0, 0,0,CMD_FUN },
/*    { "itemstats",      do_itemstats,        POS_DEAD,       120,LOG_NORMAL,1, 0, 0, 0,1,CMD_FUN }, */
    { "wizinvis",       do_invis,       POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_CONTROL },
    { "savearea",       do_olc_save_area, POS_DEAD,     ROLC,  LOG_NORMAL,1, 0, 0, 0,1,CMD_CONTROL },
    { "form",           do_form,        POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_FUN },
    { "as",             do_as,          POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_FUN },
    { "bauble",         do_bauble,      POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_FUN },
    { "console",        do_console,     POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_FUN },
    { "cert",           do_cert,        POS_DEAD,       120, LOG_ALWAYS,1,0, 0, 0,1,CMD_FUN },
    { "saylock",        do_saylock,     POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_FUN },
    { "formobject",     do_formobject,  POS_DEAD,       120,LOG_ALWAYS,1, 0, 0, 0,1,CMD_FUN },
    { "maskwho",        do_maskwho,     POS_DEAD,       120,  LOG_NORMAL,1, 0, 0, 0,1,CMD_FUN },
    { "smashmask",      do_smashmask,   POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_FUN },
    { "unform",         do_unform,      POS_DEAD,       120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_FUN },
    { "vnum",           do_vnum,        POS_DEAD,       112,  LOG_NORMAL,1, 0, 0 ,0,1,CMD_LOAD },
    { "zecho",          do_zecho,       POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_ECHO },
    { "clone",          do_clone,       POS_DEAD,       112,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_LOAD },
    { "wiznet",         do_wiznet,      POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "bandage",        do_bandage,     POS_STANDING,     0,LOG_NORMAL,1,0, 0, 0,0,0 },
    { "immtalk",        do_immtalk,     POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { "imotd",          do_imotd,       POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,0 },
    { ":",              do_immtalk,     POS_DEAD,       112,  LOG_NORMAL,0, 0, 0, 0,1,0 },
    { "smote",          do_smote,       POS_DEAD,       112,  LOG_NORMAL,1, 0, 0, 0,1,CMD_ECHO },
    { "emaillist",      do_email_list,  POS_DEAD,      120,  LOG_ALWAYS,1,
0, 0, 0,1,0 },
    { "makebloodpool",  do_makebloodpool, POS_DEAD,      120,  LOG_ALWAYS,1, 0, 0, 0,1,CMD_CONTROL },
    { "savemobprog",    do_olc_save_mob_progs,    POS_RESTING,     0,  LOG_NORMAL,1, 0, 0, 0,0,0 },
    { "assail",         do_assail,      POS_STANDING,    0,  LOG_NORMAL,1, 3, DISC_VAMP_DOMI, 5,0,0 },
    { "assail",         do_assail,      POS_STANDING,    0,  LOG_NORMAL,1, 2, DISC_DAEM_DISC, 5,0,0 },

    /*
     * End of list.
     */
    { "",               0,              POS_DEAD,        0,  LOG_NORMAL,0,0, 0, 0,0,0 } };



/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char comand[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int cmd;
    int trust;
    int race;
    bool found, foundstar = FALSE;
    sh_int col = 0;
    int star = 0;

    WRITE_STR(debug_last_character, GET_PROPER_NAME(ch));
    WRITE_STR(debug_last_command,   argument           );
    debug_last_room = ch->in_room->vnum;

    /* My way is better */
    if (is_null_room(ch)) return;

    if (strlen(argument) > 254)
    {
	stc("Line too long.\n\r", ch);
	return;
    }

    /*
     * Strip leading spaces.
     */
    while ( isspace(*argument) )
	argument++;
    if ( argument[0] == '\0' )
	return;

    /*
     * Implement freeze command.
     */
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
    {
	send_to_char( "You're totally frozen!\n\r", ch );
	return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy( logline, argument );
    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;
	while ( isspace(*argument) )
	    argument++;
    }
    else
    {
	argument = one_argument( argument, command );
    }

    buf[0] = '\0';

   /*
    *    Do my weird star thingy
    */ 
    if (command[strlen(command) - 1] == '*')
    {
	BUFFER *output;
	command[strlen(command) - 1] = '\0';
	output = new_buf();

        for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
        {
	    if ((!str_prefix(command, cmd_table[cmd].name) || strlen(command) == 0)
		&& can_interpret(ch, cmd) > 0
		&& cmd_table[cmd].show)
	    {
		foundstar = TRUE;
		star++;
	        sprintf( buf, "%-15s", cmd_table[cmd].name );
	        add_buf( output, buf );
	        if ( ++col % 5 == 0 )
		    add_buf( output, "\n\r");
	    }
	}
	if (col % 5 != 0 && foundstar)
	    add_buf(output, "\n\r");

	if (foundstar && star != 0)
  	{
	    sprintf(buf, "\n%d command%s found.\n\r", star,
		(star > 1) ? "s" : "");
	    add_buf(output, buf);
	}

	if (!foundstar)
	{
	    add_buf(output, "No commands found.\n\r");
  	}
	page_to_char( buf_string(output), ch );
	free_buf(output);
	return;
    }

    /*
     * Look for command in command table.
     */
    found = FALSE;
    trust = get_trust( ch );
    race  = ch->secondaryrace;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( command, cmd_table[cmd].name )
	&&   can_interpret(ch, cmd) != 0)
	{
	    if (ch->level == 120)
	    {
		found = TRUE;
		break;
	    }
	    if (cmd_table[cmd].race == 0
	        && cmd_table[cmd].discipline == 0)
 	    {
		found = TRUE;
		break;
	    }
	    else if (cmd_table[cmd].race > 0
		     && cmd_table[cmd].discipline == 0
		     && race == cmd_table[cmd].race)
	    {
		found = TRUE;
		break;
	    }
	    else if (cmd_table[cmd].discipline > 0
		    && ch->power [ cmd_table[cmd].discipline ] >= cmd_table[cmd].disclevel)
	    {
 	        found = TRUE;
	        break;
	    }
	}
    }

    strcpy(comand, cmd_table[cmd].name);

    if (ch->blinkykill != NULL)
    {
	stc("You don't exist!\n\r", ch);
	WAIT_STATE(ch, 4);
	return;
    }

    /* Immoral hack protection */

    if (IS_REALIMM(ch) && (get_trust(ch) > 111 || ch->level > 111) 
	&& !IS_NPC(ch))
    {
	stc("You have been deemed a cheat by the mud, and have therefore been\n\rlagged for 150 second and had your trust stripped.\n\rMuhahahhahahaa.\n\r", ch);
	ch->level = 1;
	ch->trust = 1;
	ch->faith = 0;
	ch->move = 1;
	WAIT_STATE(ch, 50);
	return;
    }

    if (IS_FLAG4(ch, AFF_BEING_SUCKED)
	&& str_prefix(comand, "upkeep")
	&& str_prefix(comand, "stats")
	&& str_prefix(comand, "who")
	&& str_prefix(comand, "say")
	&& str_prefix(comand, "score")
	&& str_prefix(comand, "help"))
    {
	send_to_char("You can do nothing while your blood is being drained.\n\r", ch);
	return;
    }

    if (IS_FLAG4(ch, AFF_SHADOW_HIDE)
	&& str_prefix(comand, "upkeep")
	&& str_prefix(comand, "stats")
	&& str_prefix(comand, "who")
	&& str_prefix(comand, "score")
	&& str_prefix(comand, "shadowhide")
	&& str_prefix(comand, "help"))
    {
	send_to_char("You can do nothing while shadow-melded.\n\r", ch);
	return;
    }

    if (ch->sucking != NULL
	&& str_prefix(comand, "upkeep")
	&& str_prefix(comand, "stats")
	&& str_prefix(comand, "embrace")
	&& str_prefix(comand, "who")
	&& str_prefix(comand, "say")
	&& str_prefix(comand, "score")
	&& str_prefix(comand, "help"))
    {
	send_to_char("While you are feeding, nothing else matters to you.\n\r", ch);
	return;
    }

    /*
     *Earth meld only allows unearthmelding :)
     */
    if ((IS_NEWFLAG(ch, AFF_EARTH_MELD) || IS_FLAG4(ch, AFF_TREE_MELD) )
	&& str_prefix(comand, "meld")
	&& str_prefix(comand, "who")
	&& str_prefix(comand, "score")
	&& str_prefix(comand, "quit")
	&& str_prefix(comand, "save")
	&& str_prefix(comand, "help")
	&& str_prefix(comand, "chat")
	&& str_prefix(comand, "treewalk")
	&& str_prefix(comand, "treemeld")
	&& ch->level < 112)
    {
	send_to_char("You cannot do that while in stasis.\n\r",ch);
	return;
    }
    if ((IS_FLAG4(ch, AFF_SENESCHIA )
	&& str_prefix(comand, "who")
	&& str_prefix(comand, "score")
	&& str_prefix(comand, "quit")
	&& str_prefix(comand, "save")
	&& str_prefix(comand, "help")
	&& str_prefix(comand, "chat")
	&& str_prefix(comand, "seneschia")
	&& ch->level < 112))
    {
	send_to_char("But you are a pile of leaves!\n\r",ch);
	return;
    }

    if (IS_FLAG3(ch, AFF_STUN)
	&& (str_prefix(comand, "who")
	&& str_prefix(comand, "score")
	&& str_prefix(comand, "help")
	&& ch->level < 112))
    {
	send_to_char("No way! You are too stunned to do that.\n\r",ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_UNCONCIOUS) 
	&& !str_prefix(comand, "wake")
	&& str_cmp(comand, "w")
	&& ch->level < 112)
    {
	send_to_char("You are unconcious and cannot wake up.\n\r",ch);
	return;
    }
 
    if (IS_NEWFLAG(ch, AFF_TIED)
	&& (str_prefix(comand, "who")
	&& str_prefix(comand, "score")
	&& str_prefix(comand, "quit")
	&& str_prefix(comand, "save")
	&& str_prefix(comand, "help")
	&& str_prefix(comand, "chat")
	&& ch->level < 112))
    {
	send_to_char("You cannot do that while tied up.\n\r",ch);
	return;
    }

    if (IS_FLAG4(ch, AFF_STASIS)
	&& (str_prefix(comand, "who")
	&& str_prefix(comand, "score")
	&& str_prefix(comand, "save")
	&& str_prefix(comand, "help")
	&& str_prefix(comand, "stasis")
	&& ch->level < 120))
    {
	send_to_char("You cannot do that while in stasis.\n\r",ch);
	return;
    }

    /*
     * No hiding.
     */

    if (found 
	&& IS_SET(ch->affected_by, AFF_HIDE)
	&& str_cmp(cmd_table[cmd].name, "look") 
	&& str_cmp(cmd_table[cmd].name,"who")
	&& str_cmp(cmd_table[cmd].name,"score")
        && str_cmp(cmd_table[cmd].name,"examine")
        && str_cmp(cmd_table[cmd].name,"scan") 
        && str_cmp(cmd_table[cmd].name,"fangs") 
        && str_cmp(cmd_table[cmd].name,"claws")  
        && str_cmp(cmd_table[cmd].name,"help") 
        && str_cmp(cmd_table[cmd].name,"commands") 
        && str_cmp(cmd_table[cmd].name,"note") 
        && str_cmp(cmd_table[cmd].name,"score") 
	&& str_cmp(cmd_table[cmd].name,"whisper"))
	REMOVE_BIT( ch->affected_by, AFF_HIDE );

    /*
     * Log and snoop.
     */
    if ( cmd_table[cmd].log == LOG_NEVER )
	strcpy( logline, "" );

    if ( ( IS_SET(ch->act, PLR_LOG)
    ||   fLogAll
    ||   cmd_table[cmd].log == LOG_ALWAYS ) 
    &&   !IS_NPC(ch) )
    {
	sprintf( log_buf, "Log %s: %s", ch->pcdata->switchname, logline );
	wiznet(log_buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
	log_string( log_buf );
    }

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    if ( !found )
    {
	/*
	 * Look for command in socials table.
	 */
	if ( !check_social( ch, command, argument ) )
	    send_to_char( "Huh?\n\r",ch );
	return;
    }

    if (ch->stunned > 0 && cmd_table[cmd].position >= POS_SLEEPING)
    {
	stc("You are too stunned to do that.\n\r", ch);
	return;
    }

    /*
     * Character not in position for command?
     */
    if ( ch->position < cmd_table[cmd].position && !is_item(ch))
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "You are deceased, you have shuffled off your mortal coil and may not do that.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "You feel too relaxed to do that.\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "Better stand up first.\n\r",ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;

	}
	return;
    }

    /*
     * Dispatch the command.
     */
    (*cmd_table[cmd].do_fun) ( ch, argument );

    tail_chain( );
    return;
}



bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "Lie still; you are DEAD.\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "You are hurt far too bad for that.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "You are too stunned to do that.\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
	if ( !str_cmp( social_table[cmd].name, "fart" ) )
	    break;
	if ( !str_cmp( social_table[cmd].name, "drool" ) )
	    break;
	send_to_char( "In your dreams, or what?\n\r", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
	act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
	act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim) 
	&&   victim->desc == NULL)
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( social_table[cmd].others_found,
		    victim, NULL, ch, TO_NOTVICT );
		act( social_table[cmd].char_found,
		    victim, NULL, ch, TO_CHAR    );
		act( social_table[cmd].vict_found,
		    victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
		break;
	    }
	}
    }

    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number ( char *arg )
{
 
    if ( *arg == '\0' )
	return FALSE;
 
    if ( *arg == '+' || *arg == '-' )
	arg++;
 
    for ( ; *arg != '\0'; arg++ )
    {
	if ( !isdigit( *arg ) )
	    return FALSE;
    }
 
    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(char *argument, char *arg)
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '*' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '*';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }
 
    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

void do_commands( CHAR_DATA *ch, char *argument )
{
    stc("Instead of typing 'commands', use the wildcard '*'.\n\r", ch);
    stc("Also, 'r*' will display all commands beginning with r, and so on.\n\r", ch);
}


int can_interpret (CHAR_DATA *ch, int cmd)
{
    char *comand = cmd_table[cmd].name;
    bool cando = FALSE;

    if (ch->level == 120)
	cando = TRUE;

    if (cmd_table[cmd].level == SECRACE)
    {
	if (ch->secondaryrace == 0)
	    return 0;
	else cando = TRUE;
    }

    if (cmd_table[cmd].level == DEMON_LEADER)
    {
	if (IS_CREATOR(ch))
	    cando = TRUE;
	else if (ch->secondaryrace == RACE_DAEMON && is_clan_leader(ch))
	    cando = TRUE;
	else return 0;
    }

    /* OLC check goes here cos of its level */
    if (cmd_table[cmd].level == ROLC)
    {
	if (CAN_BUILD_ROOM(ch)) cando = TRUE;
	else return 0;
    }
/*    if (cmd_table[cmd].level == OLC)
    {
	if (CAN_BUILD(ch)) cando = TRUE;
	else return 0;
    }*/

    if (cmd_table[cmd].level > get_trust(ch)) return 0;

    if (cmd_table[cmd].race == 0
        && cmd_table[cmd].discipline == 0)
	cando = TRUE;

    if (cmd_table[cmd].race == 0 && cmd_table[cmd].discipline != 0)
    {
	// check trades
	if (ch->trades[cmd_table[cmd].discipline] < cmd_table[cmd].disclevel)
	    cando = TRUE;
    }

    if (cmd_table[cmd].race > 0
         && cmd_table[cmd].discipline == 0
         && ch->secondaryrace == cmd_table[cmd].race)
	cando = TRUE;

    if (cmd_table[cmd].discipline > 0
         && ch->power [ cmd_table[cmd].discipline ] >= cmd_table[cmd].disclevel)
 	cando = TRUE;

    /* Oswtiching takes priority here */
    if (cmd_table[cmd].switched == 0 && is_item(ch) ) 
	cando = FALSE;

    if (!cando) return 0;

    if (IS_FLAG4(ch, AFF_BEING_SUCKED)                                           
        && str_prefix(comand, "upkeep")                                          
        && str_prefix(comand, "stats")                                           
        && str_prefix(comand, "who")                                             
        && str_prefix(comand, "say")                                             
        && str_prefix(comand, "score")                                           
        && str_prefix(comand, "help"))                                           
        return -1;                                                                  
 
    if (IS_FLAG4(ch, AFF_SHADOW_HIDE)                                            
        && str_prefix(comand, "upkeep")                                          
        && str_prefix(comand, "stats")                                           
        && str_prefix(comand, "who")                                             
        && str_prefix(comand, "score")                                           
        && str_prefix(comand, "shadowhide")                                      
        && str_prefix(comand, "help"))                                           
        return -1;                                                                  
 
    if (ch->sucking != NULL                                                      
        && str_prefix(comand, "upkeep")                                          
        && str_prefix(comand, "stats")                                           
        && str_prefix(comand, "embrace")                                         
        && str_prefix(comand, "who")                                             
        && str_prefix(comand, "say")                                             
        && str_prefix(comand, "score")                                           
        && str_prefix(comand, "help"))                                           
        return -1;                                                                  
 
    if ((IS_NEWFLAG(ch, AFF_EARTH_MELD) || IS_FLAG4(ch, AFF_TREE_MELD) )         
        && str_prefix(comand, "meld")                                           
        && str_prefix(comand, "who")                                            
        && str_prefix(comand, "score")                                          
        && str_prefix(comand, "quit")                                    
        && str_prefix(comand, "save")                                      
        && str_prefix(comand, "help")                                  
        && str_prefix(comand, "chat")                                       
        && str_prefix(comand, "treewalk")                                       
        && str_prefix(comand, "treemeld")                                   
        && ch->level < 112)                                                     
	return -1;

    if ((IS_NEWFLAG(ch, AFF_SENESCHIA )                                          
        && str_prefix(comand, "who")                                             
        && str_prefix(comand, "score")                                           
        && str_prefix(comand, "quit")                                            
        && str_prefix(comand, "save")                                            
        && str_prefix(comand, "help")                                            
        && str_prefix(comand, "chat")                                            
        && str_prefix(comand, "seneschia")                                       
        && ch->level < 112))                                                     
        return -1;                                                                  

    if (IS_FLAG3(ch, AFF_STUN)                                                   
        && (str_prefix(comand, "who")                                            
        && str_prefix(comand, "score")                                           
        && str_prefix(comand, "help")                                            
        && ch->level < 112))                                                     
	return -1;
                                                                                 
    if (!TIME_UP(ch, TIMER_UNCONCIOUS)                                           
        && !str_prefix(comand, "wake")                                           
        && str_cmp(comand, "w")                                                  
        && ch->level < 112)                                                      
	return -1;  

    if (IS_NEWFLAG(ch, AFF_TIED)                                                 
        && (str_prefix(comand, "who")                                            
        && str_prefix(comand, "score")                                           
        && str_prefix(comand, "quit")                                            
        && str_prefix(comand, "save")                                            
        && str_prefix(comand, "help")                                            
        && str_prefix(comand, "chat")                                            
        && ch->level < 112))                                                     
 	return -1;

    if (IS_FLAG4(ch, AFF_STASIS)                                                 
        && (str_prefix(comand, "who")                                    
        && str_prefix(comand, "score")                                           
        && str_prefix(comand, "save")                                            
        && str_prefix(comand, "help")                                            
        && str_prefix(comand, "stasis")                                          
        && ch->level < 120))                                                     
 	return -1;

    if (ch->stunned > 0 && cmd_table[cmd].position >= POS_SLEEPING)
	return -1;

    if ( ch->position < cmd_table[cmd].position )
	return -1; 

    return 1;
}

void do_racecommands( CHAR_DATA *ch, char *argument )
{
    int i, cmd, foundpower = 0;
    char buf[MAX_STRING_LENGTH];
    bool displayed;

    if ( (ch->secondaryrace == 0 && !IS_IMMORTAL(ch)) || IS_NPC(ch) )
    {
	send_to_char("You don't have any special powers.\n\r", ch);
	return;
    }

    banner_to_char("Powers", ch);

    for (i = 1 ; i < MAX_DISCIPLINES ; i++)
    {
	displayed = FALSE;
	if (ch->power[i] > 0 && strlen(discipline[i]) > 0
		&& (ch->level < 111 || argument[0] == '\0' 
		|| !str_prefix(argument, discipline[i])))
	{
	    foundpower = 1;

		for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
		{
		    if (cmd_table[cmd].discipline == i
			&& ch->power[i] >= cmd_table[cmd].disclevel)
		    {
			if (!displayed)
			{
			    displayed = TRUE;
	    		    sprintf(buf, " %15s : ", discipline[i]);
			    send_to_char(buf, ch);
			}			    
			send_to_char(cmd_table[cmd].name, ch);
			send_to_char(" ", ch);
		    }
	        }
		if (displayed) send_to_char("\n\r", ch);
	}
    }
    i = 0;
    displayed = FALSE;
    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
    {
	if (cmd_table[cmd].race == ch->secondaryrace
	    && cmd_table[cmd].discipline ==0)

	{
	    strcpy(buf, "");
	    if (i == 0)
	    {
		switch (ch->secondaryrace)
		{
		    case 1 : sprintf(buf, " %15s : ", "Dragon Kind");break;
                    case 2 : sprintf(buf, " %15s : ", "Demon Kind");break;
                    case 3 : sprintf(buf, " %15s : ", "Vampiric"); break;
                    case 4 : sprintf(buf, " %15s : ", "Innate");break;
                    case 5 : sprintf(buf, " %15s : ", "Faerie Kind");break;
                    case 6 : sprintf(buf, " %15s : ", "Magi");break;
                    case 7 : sprintf(buf, " %15s : ", "Innate");break;
                    case 8 : sprintf(buf, " %15s : ", "Death Knight");break;
                    case 9 : sprintf(buf, " %15s : ", "Psionicist");break;
		}
	    }
	    else if (i % 4 == 0)
	    {
		sprintf(buf, "\n\r                   ");
	    }
	    i++;

	    send_to_char(buf, ch);
	    send_to_char(cmd_table[cmd].name,ch);
	    send_to_char(" ",ch);
	    displayed = TRUE;
	    foundpower++;
	}
    }

    if (displayed) send_to_char("\n\r",ch);
  
    if (foundpower == 0)
    {
        send_to_char(" You do not yet possess any powers.\n\r", ch);
    }
    else divide4_to_char(ch);

}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    send_to_char("	at		poofin		poofout		goto\n\r",ch);
    send_to_char("	holylight	wizinvis	invis		mwhere\n\r",ch);
    send_to_char("	owhere		stat		peace		penalty\n\r",ch);
    send_to_char("	imotd		immtalk		:\n\r",ch);
    if (IS_GRANTED(ch, CMD_CONNECTIONS) || ch->level==120)
    {
    send_to_char("	disconnect	sockets\n\r",ch);
    }
    if (IS_GRANTED(ch, CMD_LOAD) || ch->level==120)
    {
    send_to_char("	set		string		load		vnum\n\r",ch);
    send_to_char("	clone		flag\n\r",ch);
    }
    if (IS_GRANTED(ch, CMD_ECHO) || ch->level==120)
    {
    send_to_char("	echo		gecho		pecho		zecho\n\r",ch);
    }
    if (IS_GRANTED(ch, CMD_CONTROL) || ch->level==120)
    {
    send_to_char("	freeze		pardon		slay		force\n\r",ch);
    send_to_char("	transfer	teleport	purge\n\r",ch);
    }
    if (IS_GRANTED(ch, CMD_CHANNELS) || ch->level==120)
    {
    send_to_char("	nochannel	noemote		noshout		notell\n\r",ch);
    }
    if (IS_GRANTED(ch, CMD_LOCK) || ch->level==120)
    {
    send_to_char("	newlock		wizlock\n\r",ch);
    }
    if (IS_GRANTED(ch, CMD_REBOOT) || ch->level==120)
    {
    send_to_char("	reboot		shutdown\n\r",ch);
    }
    if (IS_GRANTED(ch, CMD_SWITCH) || ch->level==120)
    {
    send_to_char("	switch		return\n\r",ch);
    }
    if (IS_GRANTED(ch, CMD_SITES) || ch->level==120)
    {
    send_to_char("	allow		ban\n\r",ch);
    }
    if (ch->level==120)
    {
    send_to_char("	advance		trust		grant		permban\n\r",ch);
    send_to_char("	violate		deny		protect		memory\n\r",ch);
    send_to_char("	guild		log		twit		snoop\n\r",ch);
    send_to_char("	dump		maskwho		smashmask	form\n\r",ch);
    send_to_char("      materials       makecool        beep            formobject\n\r",ch);
    send_to_char("      saylock	\n\r",ch);
    }
}

void do_expand(CHAR_DATA *ch, char *argument)
{
    bool found = FALSE;
    int trust, race, cmd;
    char buf[MAX_STRING_LENGTH];
 
    if (argument[0] == '\0')
    {
	send_to_char("What word do you wish to expand?\n\r", ch);
	return;
    }

    found = FALSE;
    trust = get_trust( ch );
    race  = ch->secondaryrace;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( argument[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( argument, cmd_table[cmd].name )
	&&   (cmd_table[cmd].level <= trust || IS_FLAG3(ch, AFF_MOB_PROG_GRANT)))
	{
	    if (cmd_table[cmd].race == 0
	        && cmd_table[cmd].discipline == 0)
 	    {
		found = TRUE;
		break;
	    }
	    else if (cmd_table[cmd].race > 0
		     && cmd_table[cmd].discipline == 0
		     && race == cmd_table[cmd].race)
	    {
		found = TRUE;
		break;
	    }
	    else if (cmd_table[cmd].discipline > 0
		    && ch->power [ cmd_table[cmd].discipline ] >= cmd_table[cmd].disclevel)
	    {
 	        found = TRUE;
	        break;
	    }
	}
    }

    if (found)
    {
	sprintf(buf, "'%s' expands to '%s'.\n\r",argument,cmd_table[cmd].name);
	send_to_char(buf,ch);
	return;
    }

    sprintf(buf, "'%s' doesn't expand to any valid command.\n\r",argument);
    send_to_char(buf, ch);
}
