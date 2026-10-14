/***************************************************************************

    Yes, welcome to HELL, this is our header file..  Yay!
    (C) 1997, Chris Mellish and Adam Powell of Mellwell Corp Industries Ltd.

***************************************************************************/

#define NAME_OF_MUD "Vault of Darkness"
#define SHORT_MUD_NAME "VOD"
#define MUD_VERSION "Vault of Darkness (VOD) v1.0a/2.95\n\r"\
                    "Mostly from BTL 3.0/Hell 6.66\n\r"\
                    "Thanks for shopping.\n\r"
#define CODE_VER    "VOD 1.0a/2.95; BTL 3.0"

/* want the game to dump core instead of showing the pretty screen? */
#define USE_CORE_FILE

/* adds extra security I suppose, modify and recompile if you need to add people */
#define IS_REALIMM(ch)		(str_cmp(GET_PROPER_NAME((ch)), "Mitzrah") \
	                        && str_cmp(GET_PROPER_NAME((ch)), "Soder") \
	                        && str_cmp(GET_PROPER_NAME((ch)), "Jobe") \
                                && str_cmp(GET_PROPER_NAME((ch)), "Whois") \
                                && str_cmp(GET_PROPER_NAME((ch)), "Gurgle") \
	                        && str_cmp(GET_PROPER_NAME((ch)), "Wayne")) 


/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )                    ( )
#define DECLARE_DO_FUN( fun )           void fun( )
#define DECLARE_SPELL_FUN( fun )        void fun( )
#define DECLARE_PSI_FUN( fun )          void fun( )
#else
#define args( list )                    list
#define DECLARE_DO_FUN( fun )           DO_FUN    fun
#define DECLARE_SPELL_FUN( fun )        SPELL_FUN fun
#define DECLARE_PSI_FUN( fun )          void fun( )
#endif

/* system calls */
int unlink();
int system();



/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if     !defined(FALSE)
#define FALSE    0
#endif

#if     !defined(TRUE)
#define TRUE     1
#endif

#if     defined(_AIX)
#if     !defined(const)
#define const
#endif
typedef int                             sh_int;
typedef int                             bool;
#define unix
#else
typedef short   int                     sh_int;
typedef unsigned char                   bool;
#endif




/*
 * Structure types.
 */
typedef struct	clan_data		CLAN_DATA;
typedef struct  affect_data             AFFECT_DATA;
typedef struct  area_data               AREA_DATA;
typedef struct  realm_data              REALM_DATA;
typedef struct  ban_data                BAN_DATA;
typedef struct  buf_type                BUFFER;
typedef struct  char_data               CHAR_DATA;
typedef struct  prog_data               PROG_DATA;
typedef struct  mob_prog_data           MOB_PROG_DATA;
typedef struct  balance_data		BALANCE_DATA;
typedef struct  config_data           	CONFIG_DATA;
typedef struct  site_data           	SITE_DATA;
typedef struct  email_data		EMAIL_DATA;
typedef struct  descriptor_data         DESCRIPTOR_DATA;
typedef struct  exit_data               EXIT_DATA;
typedef struct  extra_descr_data        EXTRA_DESCR_DATA;
typedef struct  help_data               HELP_DATA;
typedef struct  manual_data             MANUAL_DATA;
typedef struct  kill_data               KILL_DATA;
typedef struct  mem_data                MEM_DATA;
typedef struct  mob_index_data          MOB_INDEX_DATA;
typedef struct  note_data               NOTE_DATA;
typedef struct  obj_data                OBJ_DATA;
typedef struct  obj_index_data          OBJ_INDEX_DATA;
typedef struct  pc_data                 PC_DATA;
typedef struct  gen_data                GEN_DATA;
typedef struct  reset_data              RESET_DATA;
typedef struct  room_index_data         ROOM_INDEX_DATA;
typedef struct  shop_data               SHOP_DATA;
typedef struct  supplier_data           SUPPLIER_DATA;
typedef struct  time_info_data          TIME_INFO_DATA;
typedef struct  weather_data            WEATHER_DATA;
typedef struct  trade_data		TRADE_DATA;
typedef struct	auto_look_data		AUTO_LOOK_DATA;
typedef struct	timer_data		TIMER;
typedef struct	variable_data		VARIABLE;

struct  psiupkeep_type
{
    CHAR_DATA * victim;
    int         dur;
};

typedef struct  psiupkeep_type          PSI_UPKEEP;


struct trade_data
{
    char * 		name;
    int 		cost;
    int			time;
    int			exp;
    int			max_level;
};


/*
 * Clan data
 */


struct clan_data
{
    char *		name;
    char *		leader;
    int			members;
    char *		description;
    int			fighting;
    int			ally;
};

/*
 * Function types.
 */
typedef void DO_FUN     args( ( CHAR_DATA *ch, char *argument ) );
typedef void SPELL_FUN  args( ( int sn, int level, CHAR_DATA *ch, void *vo,
				int target ) );
typedef void PSI_FUN    args( ( CHAR_DATA *ch, void *vo, int sn, int target,
				int round) );


struct league_table_type
{
    char name[40];
    int  pkills, pkilled, bmk, score;
};

#define MAX_LEAGUE		20


/* Utility macros */

#define MSL	MAX_STRING_LENGTH
#define MIL	MAX_INPUT_LENGTH

#define CH(descriptor) ((descriptor)->original ? (descriptor)->original : (descriptor)->character)



/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH             1024
#define MAX_REALMS		 30
#define MAX_STRING_LENGTH        4608
#define MAX_INPUT_LENGTH          500
#define PAGELEN                    22
#define MAX_STDOBJ		   30


/*
 * Languages
 */
#define LANG_COMMON                 1
#define LANG_ELVISH                 2
#define LANG_CAMP_COMMON	    3 // we dont use this one at all
#define LANG_GNOMISH                4
#define LANG_DEMONIC                5
#define LANG_DRUNK                  6
#define LANG_BABBLE                 6


/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_SOCIALS               256
#define MAX_SKILL                 280    /* 180 spells + 60 skills ? */
#define MAX_SKILLS                 (MAX_SKILL)
#define MAX_NON_SPELL_SKILLS	  100
#define MAX_GROUP                  75
#define NUM_PSI_SLOTS              50
#define NUM_LANGUAGES               7
#define MAX_IN_GROUP               15
#define MAX_ALIAS                  10
#define MAX_PC_RACE                 5
#define MAX_CLAN                   40
#define MAX_DAMAGE_MESSAGE         41
#define MAX_LEVEL                  120
#define LEVEL_HERO                 (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL             (MAX_LEVEL - 8)
#define NUMBER_DISCIPLINES	    10
#define MAX_DISCIPLINES		    91
#define MAX_TRADES		    10


#define CLAN_NONE			0
#define CLAN_VAMP_GANGREL		1
#define CLAN_VAMP_ANARCH		2
#define CLAN_VAMP_TREMERE		3
#define CLAN_VAMP_BRUJAH		4
#define CLAN_VAMP_NOSFERATU		5
#define CLAN_VAMP_TOREADOR		6
#define CLAN_VAMP_VENTRUE		7
#define CLAN_VAMP_MALKAVIAN		8
#define CLAN_VAMP_TZIMISCE		9
#define CLAN_DAEM_CAINA			10
#define CLAN_DAEM_BANSHE		11
#define CLAN_DAEM_OUTCAST		12
#define CLAN_DAEM_SHAITAN		13
#define CLAN_ELAD_MORWEL		14
#define CLAN_ELAD_LORIEN		15
#define CLAN_ELAD_EVLOCK		16
#define CLAN_ELAD_SILARIN		17
#define CLAN_LUPI_ROGUE			18
#define CLAN_LUPI_LUPE			19
#define CLAN_LUPI_LUNE			20
#define CLAN_LUPI_AETHIR		21
#define CLAN_AETHIR_GAROU		(CLAN_LUPI_AETHIR)
#define CLAN_WARL_CRIMSON		22
#define CLAN_WARL_DELVERS		23
#define CLAN_WARL_LOTHARIEL		24
#define CLAN_HERO_SUN			25
#define CLAN_HERO_DRUID			26
#define CLAN_HERO_KNIGHTS		27
#define CLAN_DROW_OUTCAST		28
#define CLAN_DROW_FIRST			29
#define CLAN_DROW_SECOND		30
#define CLAN_DROW_THIRD 		31
#define CLAN_DROW_FOURTH		32
#define CLAN_DROW_FIFTH			33
#define CLAN_VAMP_MANTICORE		34
#define CLAN_DAEM_SHADA			35
#define CLAN_DAEM_MALBORDIA		36


// for the colour codes:

#define CHANNEL_CHAT			0
#define CHANNEL_MUSIC			1
#define CHANNEL_PLEAD			2
#define CHANNEL_QUOTE			3
#define CHANNEL_GRATS			4
#define CHANNEL_GOD			5
#define CHANNEL_SHOUTS			6
#define CHANNEL_TELLS			7


/* Right... adams skills sections, done my way to induce crashing */
#define SKILL_FUCKED	0
#define SKILL_WEAPON	1
#define SKILL_COMBAT	2
#define SKILL_THIEF	3
#define SKILL_PRIEST	4
#define SKILL_WIZARD	5
#define SKILL_KNIGHT	6

/* These are all the semi skills, level 2 of the tree
    Do not modify them, cos the numbers are important -
    I hope I left enough room */

#define SKILL_WEAP_SLASH	 10
#define SKILL_WEAP_BASH 	 11
#define SKILL_WEAP_STAB 	 12

#define SKILL_COMB_DEFENSE 	 14
#define SKILL_COMB_ATTACK	 15
#define SKILL_COMB_AGILITY 	 16

#define SKILL_THIE_ASSASSIN 	 18
#define SKILL_THIE_ROGUE 	 19

#define SKILL_PRIE_HEALING 	 21
#define SKILL_PRIE_CRAFT 	 22

#define SKILL_PRIE_SPELLS 	 25
#define SKILL_WIZA_ITEMS 	 26
#define SKILL_WIZA_SPELLS 	 27

#define SKILL_KNIG_FORGING 	 28
#define SKILL_KNIG_COMBAT	 29
#define MAX_SKILL_TREE		 43

/* These are the weird weather effects - wwweirrriidddd */
#define WEIRD_WEATHER_BALL_LIG	1
#define WEIRD_WEATHER_FISH	2
#define WEIRD_WEATHER_FROGS	3
#define WEIRD_WEATHER_SUNRISE	4
#define WEIRD_WEATHER_FOG	5
#define WEIRD_WEATHER_HURRICANE	6
#define WEIRD_WEATHER_TORNADO	7
#define WEIRD_WEATHER_GOOD_RAIN	8
#define WEIRD_WEATHER_BAD_LIG	9
#define WEIRD_WEATHER_LIGHT	10
#define WEIRD_WEATHER_AURORA	11

#define MAX_WEIRD_WEATHER	12

/* trades - by Chris */

#define TRADE_BLACKSMITH		1
#define TRADE_RIDING			2
#define TRADE_SURVIVAL			3
#define TRADE_FLETCHING			4


/* Code added so that you can have 70 disciplines!!!! By Adam */
#define DISC_DRAG_FIRE			1
#define DISC_DRAG_PROW			2
#define DISC_DRAG_MIGT			3
#define DISC_DRAG_AWE			4
#define DISC_DRAG_RESI			5
#define DISC_DRAG_POLY			6
#define DISC_DRAG_META			7
#define DISC_DRAG_CONJ			8

#define DISC_DAEM_HELL			11
#define DISC_DAEM_ATTA			12
#define DISC_DAEM_TEMP			13
#define DISC_DAEM_MORP			14
#define DISC_DAEM_CORR			15
#define DISC_DAEM_GELU			16
#define DISC_DAEM_DISC			17
#define DISC_DAEM_NETH			18
#define DISC_DAEM_IMUN			19

#define DISC_VAMP_CELE		 	21
#define DISC_VAMP_POTE 		 	22
#define DISC_VAMP_OBFU		 	23
#define DISC_VAMP_AUSP		 	24
#define DISC_VAMP_FORT		 	25
#define DISC_VAMP_THAU		 	26
#define DISC_VAMP_PROT		 	27
#define DISC_VAMP_DOMI		 	28
#define DISC_VAMP_ANIM		 	29
#define DISC_VAMP_PRES		 	30

#define DISC_LUPI_LEON			31
#define DISC_LUPI_BOAR			32
#define DISC_LUPI_LUPI			33
#define DISC_LUPI_URSI			34
#define DISC_LUPI_HAWK			35
#define DISC_LUPI_COBR			36
#define DISC_LUPI_LUNA			37
#define DISC_LUPI_FOX			38

#define DISC_ELAD_AURA			41
#define DISC_ELAD_MORP			42
#define DISC_ELAD_AISE			43
#define DISC_ELAD_MELO			44
#define DISC_ELAD_SOMA			45
#define DISC_ELAD_ANSO			46
#define DISC_ELAD_ARTI			47
#define DISC_ELAD_PHYS			48
#define DISC_ELAD_ARBO			49

#define DISC_WARL_FARR			51
#define DISC_WARL_LUCU			52
#define DISC_WARL_SCRI			53
#define DISC_WARL_BREW			54
#define DISC_WARL_ENCH			55
#define DISC_WARL_CONT			56
#define DISC_WARL_THEA			57
#define DISC_WARL_WILL			58
#define DISC_WARL_BIND			58
#define DISC_WARL_RUNE			59
#define DISC_WARL_FAKI			60

#define DISC_HERO_SIGI			61
#define DISC_HERO_RELI			62
#define DISC_HERO_EXOR			63
#define DISC_HERO_CRUS			64
#define DISC_HERO_VIVI			65
#define DISC_HERO_TRUE			66
#define DISC_HERO_ELEM			67
#define DISC_HERO_BATT			68

/* These are the extra ones to go in */
#define DISC_VAMP_ANIME			71
#define DISC_VAMP_SATY			72


/* Runic magic.   Yum. */
#define RUNE_DYN		1
#define RUNE_TOI		2
#define RUNE_YMA		3
#define RUNE_MELC		4
#define RUNE_VICT		5
#define RUNE_ELB		6
#define RUNE_GER		7
#define RUNE_BIA		8
#define RUNE_LOG		9
#define RUNE_ZOE		10


#define REALM_FIRE		1
#define REALM_FLAME		1
#define REALM_WATER		2
#define REALM_AIR		3
#define REALM_EARTH		4
#define REALM_ENTROPY		5
#define REALM_LIFE		6


/* These are the good old dragon/werewolf types - same variable */
#define WERE_WOLF		    1
#define WERE_RAT		    2
#define WERE_TIGER		    3
#define WERE_BEAR		    4
#define WERE_SHARK		    5

#define DRAGON_RED		    1
#define DRAGON_WHITE		    2
#define DRAGON_BLACK		    3
#define DRAGON_GREEN		    4
#define DRAGON_BLUE		    5
#define DRAGON_SILVER		    6
#define DRAGON_GOLD		    7
#define DRAGON_SHADOW		    8

#define RACE_DRAGON                 1
#define RACE_DAEMON                 2
#define RACE_VAMPIRE                3
#define RACE_LUPINE                 4
#define RACE_ELADRIN                5
#define RACE_WARLOCK                6
#define RACE_HERO                   7
#define RACE_DEATH_KNIGHT           8
#define RACE_PSIONICIST             9

#define RACE_HUMAN		    1
#define RACE_ELF		    2
#define RACE_HALF_ELF		    3
#define RACE_GNOME		    4

#define TERTIARY_CRIMSON            1
#define TERTIARY_WITCH              2
#define TERTIARY_LICH               3
#define TERTIARY_AASIMON            4
#define TERTIARY_DROW		    5

#define ADVANCED_FLIGHT             1
#define ADVANCED_TREE               2
#define ADVANCED_IMMUNITY           3
#define ADVANCED_UNDEATH            4
#define ADVANCED_PLANAR             5
#define ADVANCED_PSIONICS           6
#define ADVANCED_CANTRUM            7
#define ADVANCED_SUPREME            8
#define ADVANCED_STARBURST          9
#define ADVANCED_CLEANSING          10
#define ADVANCED_POSSESSION         11
#define ADVANCED_WILDMAGE           12
#define ADVANCED_ARCHMAGE           13

#define ELDER_INVULNERABILITY       1
#define ELDER_DAMCAP                2

#define PULSE_PER_SECOND            4
#define PULSE_VIOLENCE            ( 3 * PULSE_PER_SECOND)
#define PULSE_MOBILE              ( 4 * PULSE_PER_SECOND)
#define PULSE_TICK                (30 * PULSE_PER_SECOND)
#define PULSE_AREA                (120 * PULSE_PER_SECOND)

#define IMPLEMENTOR             MAX_LEVEL
#define CREATOR                 (MAX_LEVEL - 1)
#define SUPREME                 (MAX_LEVEL - 2)
#define DEITY                   (MAX_LEVEL - 3)
#define GOD                     (MAX_LEVEL - 4)
#define IMMORTAL                (MAX_LEVEL - 5)
#define DEMI                    (MAX_LEVEL - 6)
#define ANGEL                   (MAX_LEVEL - 7)
#define AVATAR                  (MAX_LEVEL - 8)
#define HERO                    LEVEL_HERO

#define FRM_WOLF               1
#define FRM_BAT                2
#define FRM_MIST               3
#define FRM_DOLPHIN            4
#define FRM_FLAME              5
#define FRM_SPIDER             6
#define FRM_LIGHT              7
#define FRM_COCKROACH          8
#define FRM_RABBIT             9
#define FRM_BLOB               10
#define FRM_FROG               11
#define FRM_IDIOT              12
#define FRM_WHIRLWIND          13
#define FRM_CRIMSON            14
#define FRM_HAWK               15
#define FRM_WEREWOLF           16
#define FRM_FOX	               17
#define FRM_WYVERN             18
#define FRM_GARGANTUA          19
#define FRM_PEASANT            20
#define FRM_WARRIOR            21
#define FRM_BARBARIAN          22
#define FRM_GIRL               23

/* The demonic forms */
#define FRM_LEMURE             24
#define FRM_NUPPERIBO          25
#define FRM_SPINAGON           26
#define FRM_BARBAZU            27
#define FRM_ABISHAI            28
#define FRM_OSYLUTH            29
#define FRM_HAMATULA           30
#define FRM_ERINYES            31
#define FRM_AMNIZU             32
#define FRM_CORNUGON           33
#define FRM_GELUGON            34
#define FRM_PIT_FIEND          35
#define FRM_PIT_LORD           36

#define FRM_TREE               37

#define FRM_DRAGON_1           38
#define FRM_DRAGON_13          50

#define FRM_WERE_RAT	       51
#define FRM_WERE_BEAR	       52
#define FRM_WERE_TIGER	       53
#define FRM_WERE_SHARK	       54
#define FRM_GOLEM	       55
#define FRM_PIXIE	       56
#define FRM_MANTICORE	       57

#define MAX_FORMS	       60


#define CASH_COPPER		1
#define CASH_SILVER		2
#define CASH_GOLD		3
#define CASH_EMERALDS		4
#define CASH_SAPPHIRES		5
#define CASH_DIAMONDS		6

#define CNV_DIA			100000
#define CNV_SAP			10000
#define CNV_EME			1000
#define CNV_GOL			100
#define CNV_SIL			10
#define CNV_COP 		1

/*
 * Site ban structure.
 */

#define BAN_SUFFIX              A
#define BAN_PREFIX              B
#define BAN_NEWBIES             C
#define BAN_ALL                 D       
#define BAN_PERMIT              E
#define BAN_PERMANENT           F

struct  ban_data
{
    BAN_DATA *  next;
    bool        valid;
    sh_int      ban_flags;
    sh_int      level;
    char *      name;
};

struct buf_type
{
    BUFFER *    next;
    bool        valid;
    sh_int      state;  /* error state of the buffer */
    sh_int      size;   /* size in k */
    char *      string; /* buffer's string */
};



/*
 * Time and weather stuff.
 */
#define SUN_DARK                    0
#define SUN_RISE                    1
#define SUN_LIGHT                   2
#define SUN_SET                     3

#define SKY_CLOUDLESS               0
#define SKY_CLOUDY                  1
#define SKY_RAINING                 2
#define SKY_LIGHTNING               3

struct  time_info_data
{
    int		min;
    int         hour;
    int         day;
    int         month;
    int         year;
};

struct  weather_data
{
    int         mmhg;
    int         change;
    int         sky;
    int         sunlight;
};


/*
 * Connected state for a channel.
 */
#define CON_PLAYING                      0
#define CON_GET_NAME                     1
#define CON_GET_OLD_PASSWORD             2
#define CON_CONFIRM_NEW_NAME             3
#define CON_GET_NEW_PASSWORD             4
#define CON_CONFIRM_NEW_PASSWORD         5
#define CON_GET_NEW_RACE                 6
#define CON_GET_NEW_SEX                  7
#define CON_GET_ALIGNMENT                9
#define CON_DEFAULT_CHOICE              10 
#define CON_GEN_GROUPS                  11 
#define CON_PICK_WEAPON                 12
#define CON_READ_IMOTD                  13
#define CON_READ_MOTD                   14
#define CON_BREAK_CONNECT               15
#define CON_MANUAL                      16
#define CON_CONSOLE			17
#define CON_WANT_ANSI			18
#define CON_EDITOR			19
#define CON_NAME_HELP			20
#define CON_TALKING			22

/*
 * Timer macros.
 */

#define TIMER(ch, tmr)		((ch)->tick_timer[(tmr)])
#define SET_TIMER(ch, tmr, tm)	((ch)->tick_timer[(tmr)] = (tm))
#define ADD_TIMER(ch, tmr, tm)  ((ch)->tick_timer[(tmr)] += (tm))
#define SUB_TIMER(ch, tmr, tm)  ((ch)->tick_timer[(tmr)] -= (tm))
#define TIME_UP(ch, tmr)	((ch)->tick_timer[(tmr)] == 0 ? TRUE : FALSE)

#define RTIMER(room, rtmr)	 ((room)->tick_timer[(rtmr)])
#define SET_RTIMER(room, rtmr, rtm) ((room)->tick_timer[(rtmr)] = (rtm))
#define ADD_RTIMER(room, rtmr, rtm)  ((room)->tick_timer[(rtmr)] += (rtm))
#define SUB_RTIMER(room, rtmr, rtm)  ((room)->tick_timer[(rtmr)] -= (rtm))
#define RTIME_UP(room, rtmr)	 ((room)->tick_timer[(rtmr)] == 0 ? TRUE : FALSE)

#define TIMER_LAYONHANDS	0
#define TIMER_WRENCH		1
#define TIMER_WRENCHED 		2
#define TIMER_VAMPCALL 		3
#define TIMER_UNCONCIOUS 	4
#define TIMER_VAMP_INHABIT 	5
#define TIMER_DAEMONIC_TRANSFORM 6
#define TIMER_MESMERISE		7
#define TIMER_MESMERISED	8
#define TIMER_FORAGE		9
#define TIMER_NO_ARBOREA	11
#define TIMER_TREE_WALK		12
#define TIMER_CAN_PRAC		13
#define TIMER_CAN_EAT_ARMS	14
#define TIMER_THIRD_ARM_GROWING	15
#define TIMER_FOURTH_ARM_GROWING 16
#define TIMER_THIRD_ARM_GOING	17
#define TIMER_FOURTH_ARM_GOING  18
#define TIMER_SCALPED		19
#define TIMER_CAN_CALL_ROCKS    20
#define TIMER_CANT_BE_TURNED    21
#define TIMER_CANT_TURN    	22
#define TIMER_FIGHT_LAG    	23
#define TIMER_CAN_CHANGE_HAWK   24
#define TIMER_CAN_CREATE_SHARD  25
#define TIMER_CAN_GUST	        26
#define TIMER_CAN_ENTER_STASIS  27
#define TIMER_MAKE_SNOWMAN      28
#define TIMER_ENTOMB            29
#define TIMER_CAN_BREATHE_FROST 30
#define TIMER_HELLFIRE_SUMMON   31
#define TIMER_ON_SPEED	   	32
#define TIMER_ON_LSD		33
#define TIMER_ON_HASH		34
#define TIMER_CAN_CALL_WAR_HORSE 35
#define TIMER_WAR_HORSE_GO      36
#define TIMER_CAN_SPIT_VENOM    37
#define TIMER_CAN_GAIN_VOODOO   38
#define TIMER_CAN_FEATHER       39
#define TIMER_CAN_SHRIEK        40
#define TIMER_CAN_POLYMORPH     41
#define TIMER_DRAGON_GROW       42
#define TIMER_VAMPIRE_GROW      43
#define TIMER_SKILL_LEV1        44
#define TIMER_SKILL_LEV2        45
#define TIMER_CANT_SWARM        46
#define TIMER_CANT_BORROWLIFE   47
#define TIMER_TREE		48
#define TIMER_NEXUS_STUNNED	49
#define TIMER_GOLEM		50
#define TIMER_TAINT		51
#define TIMER_NEWBIE_IMM	52
#define TIMER_CAN_DO_NEXUS	53
#define TIMER_CAN_USE_HEALER	54
#define TIMER_DISCORD		55
#define TIMER_SPHINX_ROAR	56
#define TIMER_YOUTH		57
#define TIMER_HEAL		58
#define TIMER_ELADRIN_PATH	59

#define MAX_TIMER		60


#define RTIMER_STINKING_CLOUD	0
#define RTIMER_LIFE_VORTEX	1
#define RTIMER_DEATH_VORTEX	2
#define RTIMER_GLYPH_PROTECTION	3
#define RTIMER_HIDE_ROOM	4
#define RTIMER_SWARM_BEES	5
#define RTIMER_SWARM_RATS	6
#define RTIMER_SWARM_BATS	7
#define RTIMER_GHOST_LIGHT	8
#define RTIMER_NEXUS_FLAME	9
#define RTIMER_NEXUS_WATER	10
#define RTIMER_NEXUS_AIR	11
#define RTIMER_NEXUS_EARTH	12
#define RTIMER_NEXUS_ENTROPY	13
#define RTIMER_WALL_NORTH	14
#define RTIMER_WALL_EAST	15
#define RTIMER_WALL_SOUTH	16
#define RTIMER_WALL_WEST	17
#define RTIMER_WALL_UP		18
#define RTIMER_WALL_DOWN	19
#define RTIMER_DISCORD		20
#define RTIMER_DARK_ROOM	21

#define MAX_RTIMER		30


/*
 * New Advanced Timer System - NATS
 */

struct timer_data
{
    TIMER * 		next;
    char * 		name;
    int			id;
    int			ticks;
    int			rounds;
    int			secs;
    void *		func;
    void *		term_func;
    char * 		term_mesg;
    char * 		term_act;
    VARIABLE *		vars;
    char * 		affect_mesg;
};


/*
 * Variable Information for Collective Keeping of Yaks - VICKY
 */

struct variable_data
{
    VARIABLE *		next;
    int			type;
    union
    {
	char *		string;
	int		integer;
	bool		boolean;
    } data;
    char *		name;
};


#define VARTYPE_STRING 		0
#define VARTYPE_INTEGER		1
#define VARTYPE_BOOLEAN 	2

#define READ_VAR(base, name, store) { VARIABLE *var; 			\
		for (var = (base)->vars; var != NULL; var = var->next)  \
		    if (!str_cmp(var->name, (name))) break;		\
		if (var == NULL) {(store) = NULL;			\
		bug("Variable not found.", 0);}				\
		else if (var->type == VARTYPE_STRING)			\
		{ free_string((store));					\
		(store) = str_dup(var->data.string);}			\
		else if (var->type == VARTYPE_INTEGER)			\
			(store) = var->data.integer;			\
		else (store) = var->data.boolean;}			


/*
 * Descriptor (channel) structure.
 */
struct  descriptor_data
{
    DESCRIPTOR_DATA *   next;
    DESCRIPTOR_DATA *   snoop_by;
    DESCRIPTOR_DATA *   as;
    CHAR_DATA *         character;
    CHAR_DATA *         original;
    bool                valid;
    char *              host;
    sh_int              descriptor;
    sh_int              connected;
    char *		add;
    bool                fcommand;
    char                inbuf           [4 * MAX_INPUT_LENGTH];
    char                incomm          [MAX_INPUT_LENGTH];
    char                inlast          [10][MAX_INPUT_LENGTH];
    int			inlasti;
    int                 repeat;
    char *              outbuf;
    int                 outsize;
    int                 outtop;
    char *              showstr_head;
    char *              showstr_point;
    HELP_DATA *		ani_screen;
    int			value;
    int			ani_data;
    int			prevroom;
    int			sent_bytes[3];
    int			sent_secs[2];
    int			sent_pulse;
};

/*
 * Attribute bonus structures.
 */
struct  str_app_type
{
    sh_int      tohit;
    sh_int      todam;
    sh_int      carry;
    sh_int      wield;
};

struct  int_app_type
{
    sh_int      learn;
};

struct  wis_app_type
{
    sh_int      practice;
};

struct  dex_app_type
{
    sh_int      defensive;
};

struct  con_app_type
{
    sh_int      hitp;
    sh_int      shock;
};

struct cha_app_type
{
    sh_int      react;
};


/*
 * TO types for act.
 */
#define TO_ROOM             0
#define TO_NOTVICT          1
#define TO_VICT             2
#define TO_CHAR             3
#define TO_ALL              4



/*
 * Help table types.
 */
struct  help_data
{
    HELP_DATA * next;
    sh_int      level;
    char *      keyword;
    char *      text;
};

struct manual_data
{
    MANUAL_DATA *next;
    sh_int      level;
    char *      keyword;
    char *      text;
    char *      link[10];
    char *      linkname[10];
};



/*
 * Shop types.
 */
#define MAX_TRADE        5

struct	supplier_data
{
    SUPPLIER_DATA * next;		/* Next in shops supply details */
    char *	name;			/* Suppliers name */
    int		vnum_supplied;		/* Vnum of supplied item */
    int		price;			/* Price per item */
    int		varience;		/* varience in price: 100 = +-10%*/
    int		number;			/* Number in each batch */
    int		frequency;		/* Num ticks between supplies */

/* not to save */

    int		timer;			/* time till next delivery */
};

struct  shop_data
{
    SHOP_DATA * next;
    SUPPLIER_DATA * supply_list;	/* list of items supplied */
    int		realm;			/* start realm */
    int		vnum;			/* vnum of keeper */
    int		shop_room;		/* shops rnum within realm */
    int		restart_cash;		/* cash given by ruler each ressurection */
    sh_int	time_open;
    sh_int	time_close;
    long	flags;
    char *	route_home;
    char *	route_shop;
    char *	route_to_stores;
    char *	route_from_stores;
    int		profit_buying;		/* 110 = 10% profit */
    int		profit_selling;		/* 110 = 10% profit */
    char *	offender;		/* last person to steal or kill */

/* not to save */

    char *	getting;		/* info for buying */
    CHAR_DATA * char_for;
};


#define	REFUSE_HUMAN			(A)
#define	REFUSE_GNOME			(B)
#define	REFUSE_ELF			(C)
#define	REFUSE_HALF_ELF			(D)
#define	REFUSE_DRAGON			(E)
#define	REFUSE_VAMPIRE			(F)
#define	REFUSE_LUPINE			(G)
#define	REFUSE_WARLOCK			(H)
#define	REFUSE_DEMON			(I)
#define	REFUSE_ELADRIN			(J)
#define	REFUSE_HERO			(K)
#define	REFUSE_TERTIARY			(L)
#define	REFUSE_MALE			(M)
#define	REFUSE_FEMALE			(N)

#define SHOP_KILLED			(O)
#define SHOP_BUY			(P)
#define SHOP_LIST			(Q)
#define SHOP_VALID			(R)

/*
 * Per-class stuff.
 */

#define MAX_GUILD       2
#define MAX_STATS       6
#define STAT_STR        0
#define STAT_INT        1
#define STAT_WIS        2
#define STAT_DEX        3
#define STAT_CON        4
#define STAT_CHA        5

struct stdobj_type
{
    char * 	name;
    sh_int	item_type;
    int		wear_flags;
    sh_int	size;
    char * 	material;
    sh_int	weight;
    int		cost;      /* copper coins */		  
    int		value1;
    int		value2;
    int		value3;
    int		value4;
    int		value5;
};

struct form_type
{
    char * 	lookname;
    char *      name;
    int         hit;
    int         dam;
    int         ac;
    int         hit_mod;
    bool        squishable;
    sh_int	size;
    bool	can_cast;
    bool	can_fly;
    bool	can_wear;
    int		carry_max;
    int		max_items;
    char *	say_word;
    char *	move_word;
    int		xp_bonus;
    int		polymorph_level;
    char *	in_room_desc;
    char *	short_desc;
    bool	can_use_claws;
};

struct item_type
{
    int         type;
    char *      name;
};

struct weapon_type
{
    char *      name;
    sh_int      vnum;
    sh_int      type;
    sh_int      *gsn;
};

struct wiznet_type
{
    char *      name;
    long        flag;
    int         level;
};

struct attack_type
{
    char *      name;                   /* name */
    char *      noun;                   /* message */
    char *	noun_p;			/* plural message */
    int         damage;                 /* damage class */
};

struct secondaryrace_type
{
    char *      name;                   /* name of super race */
    int         min_level;              /* minimum level to become race */
    char *      propname;		/* proper name for racestats */
};

struct tertiaryrace_type
{
    char *      name;                   /* name of super super race */
    char *      whoname;                /* 7 digits on who list     */
    int         disc1;                  /* the three disciplines they get */
    int         disc2;
    int         disc3;
};

struct mage_abil_type
{
    char *      him;                    /* the him     */
    char *      her;                    /* and the her */
};

struct clanrank_type
{
    char *      rank;                   /* rank name */
    int         min_level;              /* minimum level to gain that rank */
    char *	male_name;		/* rank name for males */
    char * 	female_name;		/* use some imagination */
};

struct stance_type
{
    char *      name;                   /* stance name */
    char *      switchto;               /* switch into text */
    char *      plural;                 /* switch into text - plural */
    int         score[12];              /* scores against the other stances */
};

struct race_type
{
    char *      name;                   /* call name of the race */
    char *      ishname;                /* elvish, gnomish etc... */
    bool        pc_race;                /* can be chosen by pcs */
    long        act;                    /* act bits for the race */
    long        aff;                    /* aff bits for the race */
    long        off;                    /* off bits for the race */
    long        imm;                    /* imm bits for the race */
    long        res;                    /* res bits for the race */
    long        vuln;                   /* vuln bits for the race */
    long        form;                   /* default form flag for the race */
    long        parts;                  /* default parts for the race */
};


struct pc_race_type  /* additional data for pc races */
{
    char *      name;                   /* MUST be in race_type */
    char        who_name[11];
    sh_int      points;                 /* cost in points of the race */
    char *      skills[5];              /* bonus skills for the race */
    sh_int      stats[MAX_STATS];       /* starting stats */
    sh_int      max_stats[MAX_STATS];   /* maximum stats */
    sh_int      size;                   /* aff bits for the race */
};



/*
 * Data structure for notes.
 */

#define NOTE_NOTE       0
#define NOTE_IDEA       1
#define NOTE_PENALTY    2
#define NOTE_NEWS       3
#define NOTE_CHANGES    4
struct  note_data
{
    NOTE_DATA * next;
    bool        valid;
    sh_int      type;
    char *      sender;
    char *      date;
    char *      to_list;
    char *      subject;
    char *      text;
    time_t      date_stamp;
};



/*
 * An affect.
 */
struct  affect_data
{
    AFFECT_DATA *       next;
    bool                valid;
    sh_int              where;
    sh_int              type;
    sh_int              level;
    sh_int              duration;
    sh_int              location;
    sh_int              modifier;
    int                 bitvector;
    char 	*	name;
    char 	*	end_message; /* Used only with name */
};

/* where definitions */
#define TO_AFFECTS      0
#define TO_OBJECT       1
#define TO_IMMUNE       2
#define TO_RESIST       3
#define TO_VULN         4
#define TO_WEAPON       5
#define TO_NEWFLAGS     6
#define TO_FLAG3        7
#define TO_FLAG4        8
#define TO_FLAG5        9
#define TO_FLAG6        10

/*
 * A kill structure (indexed by level).
 */
struct  kill_data
{
    sh_int              number;
    sh_int              killed;
};


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_FIDO              3090
#define VNUM_HELL_DEMON              27
#define MOB_VNUM_CITYGUARD         3060
#define MOB_VNUM_VAMPIRE           3404
#define MOB_VNUM_MUSHROOM_KING	     30


/* ASCII conversions -- used so we can have letters in this file */

#define A                       1
#define B                       2
#define C                       4
#define D                       8
#define E                       16
#define F                       32
#define G                       64
#define H                       128

#define I                       256
#define J                       512
#define K                       1024
#define L                       2048
#define M                       4096
#define N                       8192
#define O                       16384
#define P                       32768

#define Q                       65536
#define R                       131072
#define S                       262144
#define T                       524288
#define U                       1048576
#define V                       2097152
#define W                       4194304
#define X                       8388608

#define Y                       16777216
#define Z                       33554432
#define aa                      67108864        /* doubled due to conflicts */
#define bb                      134217728
#define cc                      268435456    
#define dd                      536870912
#define ee                      1073741824
#define ff                      2147483648
#define gg                      4294967256

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC              (A)             /* Auto set for mobs    */
#define ACT_SENTINEL            (B)             /* Stays in one room    */
#define ACT_SCAVENGER           (C)             /* Picks up objects     */
#define ACT_AGGRESSIVE          (F)             /* Attacks PC's         */
#define ACT_STAY_AREA           (G)             /* Won't leave area     */
#define ACT_WIMPY               (H)
#define ACT_PET                 (I)             /* Auto set for pets    */
#define ACT_TRAIN               (J)             /* Can train PC's       */
#define ACT_PRACTICE            (K)             /* Can practice PC's    */
#define ACT_UNDEAD              (O)     
#define ACT_CLERIC              (Q)
#define ACT_MAGE                (R)
#define ACT_THIEF               (S)
#define ACT_WARRIOR             (T)
#define ACT_NOALIGN             (U)
#define ACT_NOPURGE             (V)
#define ACT_OUTDOORS            (W)
#define ACT_INDOORS             (Y)
#define ACT_IS_HEALER           (aa)
#define ACT_GAIN                (bb)
#define ACT_UPDATE_ALWAYS       (cc)
#define ACT_IS_CHANGER          (dd)
#define ACT_RIDEABLE		(ee)

/* JANDERS BITS FOR MAKING MOBOLC THE BEST IN THE WORLD - ch->mobolc_flags */
/* These dont actually set in CHAR_DATA, just activate their own little
    things, i just need 32 bits to take advantage of the editor! */

#define MOBOLC_SENTINEL		(A) /* these set bits in ch->act */
#define MOBOLC_SCAVENGER	(B)
#define MOBOLC_AGGRESSIVE	(C)
#define MOBOLC_STAY_AREA	(D)
#define MOBOLC_WIMPY		(E)
#define MOBOLC_UNDEAD		(F)
#define MOBOLC_OUTDOORS		(G)
#define MOBOLC_INDOORS		(H)
#define MOBOLC_RIDEABLE		(I)
#define MOBOLC_INVISIBLE	(J) /* ch->affected_by */
#define MOBOLC_SANCTUARY	(K)
#define MOBOLC_FAERIE_FIRE	(J)
#define MOBOLC_HIDE		(L)
#define MOBOLC_FLYING		(M)
#define MOBOLC_HASTE		(N)
#define MOBOLC_REGENERATION	(O)
#define MOBOLC_PASS_DOOR	(P)
#define MOBOLC_PROTECT_EVIL	(Q)
#define MOBOLC_PROTECT_GOOD	(R)
#define MOBOLC_REFLECTION	(S) /* ch->flag3 */
#define MOBOLC_BEAST		(T) /* AFF5_BEAST in flag5 */
#define MOBOLC_ELAD_AURA	(U) /* sets the discipline dice(1, 10) */
#define MOBOLC_WARL_CONTROL	(V) /* sets the discipline dice(1, 10) */
#define MOBOLC_INSUBSTANTIAL	(W) /* flag5, Aff5_insubstantial */
/* #define MOBOLC_			(X)
#define MOBOLC_			(Y)
#define MOBOLC_			(Z)
#define MOBOLC_			(aa)
#define MOBOLC_			(bb)
#define MOBOLC_			(cc)
#define MOBOLC_			(dd)
#define MOBOLC_			(ee) */

/* ADD THE NEXT FLAG FIELD INTO THE EDITOR AS WELL!!!! ch->mobolc_flags2
     all combat orientated ones go here ! */

/* New flag for mob special procedures - used in editor, need 32! */
#define SPEC_POISON_BITE	(A) /* All these in ch->mob_spec */
#define SPEC_PETRIFICATION	(B)
#define SPEC_FLAME_CLAW		(C) /* Doesnt work yet */
#define SPEC_WEB		(D)
#define SPEC_FIRE_SPIKES	(E)
#define SPEC_FREEZE		(F)
#define SPEC_EVERLIVING		(G)
#define SPEC_SPLIT		(H)
#define SPEC_COMBAT_CLAWS	(I) /* flag2, AFF_CLAW */
#define SPEC_COMBAT_HORNS	(J) /* flag4, AFF_HORN */
#define SPEC_COMBAT_FANGS	(K) /* flag2, AFF_BITE */
#define SPEC_COMBAT_WINGS	(L) /* flag2, AFF_WING */
#define SPEC_COMBAT_TAIL	(M) /* flag2, AFF_TAIL */
#define SPEC_STEEL_CLAWS	(N) /* flag4, AFF_STEEL_CLAWS */
#define SPEC_FIRE_BREATH	(O) /* hellfire discipline lvl 10 */
#define SPEC_AGE_ATTACK		(P) /* ghosts etc.... */


/* damage classes */
#define DAM_NONE                0
#define DAM_BASH                1
#define DAM_PIERCE              2
#define DAM_SLASH               3
#define DAM_FIRE                4
#define DAM_COLD                5
#define DAM_LIGHTNING           6
#define DAM_ACID                7
#define DAM_POISON              8
#define DAM_NEGATIVE            9
#define DAM_HOLY                10
#define DAM_ENERGY              11
#define DAM_MENTAL              12
#define DAM_DISEASE             13
#define DAM_DROWNING            14
#define DAM_LIGHT               15
#define DAM_OTHER               16
#define DAM_HARM                17
#define DAM_CHARM               18
#define DAM_SOUND               19

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK         (A)
#define OFF_BACKSTAB            (B)
#define OFF_BASH                (C)
#define OFF_BERSERK             (D)
#define OFF_DISARM              (E)
#define OFF_DODGE               (F)
#define OFF_FADE                (G)
#define OFF_FAST                (H)
#define OFF_KICK                (I)
#define OFF_KICK_DIRT           (J)
#define OFF_PARRY               (K)
#define OFF_RESCUE              (L)
#define OFF_TAIL                (M)
#define OFF_TRIP                (N)
#define OFF_CRUSH               (O)
#define ASSIST_ALL              (P)
#define ASSIST_ALIGN            (Q)
#define ASSIST_RACE             (R)
#define ASSIST_PLAYERS          (S)
#define ASSIST_GUARD            (T)
#define ASSIST_VNUM             (U)
#define OFF_DAGGER              (V)

/* return values for check_imm */
#define IS_NORMAL               0
#define IS_IMMUNE               1
#define IS_RESISTANT            2
#define IS_VULNERABLE           3

/* IMM bits for mobs */
#define IMM_SUMMON              (A)
#define IMM_CHARM               (B)
#define IMM_MAGIC               (C)
#define IMM_WEAPON              (D)
#define IMM_BASH                (E)
#define IMM_PIERCE              (F)
#define IMM_SLASH               (G)
#define IMM_FIRE                (H)
#define IMM_COLD                (I)
#define IMM_LIGHTNING           (J)
#define IMM_ACID                (K)
#define IMM_POISON              (L)
#define IMM_NEGATIVE            (M)
#define IMM_HOLY                (N)
#define IMM_ENERGY              (O)
#define IMM_MENTAL              (P)
#define IMM_DISEASE             (Q)
#define IMM_DROWNING            (R)
#define IMM_LIGHT               (S)
#define IMM_SOUND               (T)
#define IMM_WOOD                (X)
#define IMM_SILVER              (Y)
#define IMM_IRON                (Z)
 
/* RES bits for mobs */
#define RES_SUMMON              (A)
#define RES_CHARM               (B)
#define RES_MAGIC               (C)
#define RES_WEAPON              (D)
#define RES_BASH                (E)
#define RES_PIERCE              (F)
#define RES_SLASH               (G)
#define RES_FIRE                (H)
#define RES_COLD                (I)
#define RES_LIGHTNING           (J)
#define RES_ACID                (K)
#define RES_POISON              (L)
#define RES_NEGATIVE            (M)
#define RES_HOLY                (N)
#define RES_ENERGY              (O)
#define RES_MENTAL              (P)
#define RES_DISEASE             (Q)
#define RES_DROWNING            (R)
#define RES_LIGHT               (S)
#define RES_SOUND               (T)
#define RES_WOOD                (X)
#define RES_SILVER              (Y)
#define RES_IRON                (Z)
 
/* VULN bits for mobs */
#define VULN_SUMMON             (A)
#define VULN_CHARM              (B)
#define VULN_MAGIC              (C)
#define VULN_WEAPON             (D)
#define VULN_BASH               (E)
#define VULN_PIERCE             (F)
#define VULN_SLASH              (G)
#define VULN_FIRE               (H)
#define VULN_COLD               (I)
#define VULN_LIGHTNING          (J)
#define VULN_ACID               (K)
#define VULN_POISON             (L)
#define VULN_NEGATIVE           (M)
#define VULN_HOLY               (N)
#define VULN_ENERGY             (O)
#define VULN_MENTAL             (P)
#define VULN_DISEASE            (Q)
#define VULN_DROWNING           (R)
#define VULN_LIGHT              (S)
#define VULN_SOUND              (T)
#define VULN_WOOD               (X)
#define VULN_SILVER             (Y)
#define VULN_IRON               (Z)
 
/* body form */
#define FORM_EDIBLE             (A)
#define FORM_POISON             (B)
#define FORM_MAGICAL            (C)
#define FORM_INSTANT_DECAY      (D)
#define FORM_OTHER              (E)  /* defined by material bit */
 
/* material bits */
#define MATERIAL_GLASS          1
#define MATERIAL_STEEL          2
#define MATERIAL_IRON           3
#define MATERIAL_SILVER         4
#define MATERIAL_BREAD          5
#define MATERIAL_LEATHER        6
#define MATERIAL_POTTERY        7
#define MATERIAL_ADAMANTITE     8
#define MATERIAL_MITHRIL        9
#define MATERIAL_PAPER          10
#define MATERIAL_CLOTH		11
#define MATERIAL_WOOD		12

/* actual form */
#define FORM_ANIMAL             (G)
#define FORM_SENTIENT           (H)
#define FORM_UNDEAD             (I)
#define FORM_CONSTRUCT          (J)
#define FORM_MIST               (K)
#define FORM_INTANGIBLE         (L)
 
#define FORM_BIPED              (M)
#define FORM_CENTAUR            (N)
#define FORM_INSECT             (O)
#define FORM_SPIDER             (P)
#define FORM_CRUSTACEAN         (Q)
#define FORM_WORM               (R)
#define FORM_BLOB               (S)
 
#define FORM_MAMMAL             (V)
#define FORM_BIRD               (W)
#define FORM_REPTILE            (X)
#define FORM_SNAKE              (Y)
#define FORM_DRAGON             (Z)
#define FORM_AMPHIBIAN          (aa)
#define FORM_FISH               (bb)
#define FORM_COLD_BLOOD         (cc)    
 
/* body parts */
#define PART_HEAD               (A)
#define PART_ARMS               (B)
#define PART_LEGS               (C)
#define PART_HEART              (D)
#define PART_BRAINS             (E)
#define PART_GUTS               (F)
#define PART_HANDS              (G)
#define PART_FEET               (H)
#define PART_FINGERS            (I)
#define PART_EAR                (J)
#define PART_EYE                (K)
#define PART_LONG_TONGUE        (L)
#define PART_EYESTALKS          (M)
#define PART_TENTACLES          (N)
#define PART_FINS               (O)
#define PART_WINGS              (P)
#define PART_TAIL               (Q)
/* for combat */
#define PART_CLAWS              (U)
#define PART_FANGS              (V)
#define PART_HORNS              (W)
#define PART_SCALES             (X)
#define PART_TUSKS              (Y)


/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND               (A)
#define AFF_INVISIBLE           (B)
#define AFF_DETECT_EVIL         (C)
#define AFF_DETECT_INVIS        (D)
#define AFF_DETECT_MAGIC        (E)
#define AFF_DETECT_HIDDEN       (F)
#define AFF_DETECT_GOOD         (G)
#define AFF_SANCTUARY           (H)
#define AFF_FAERIE_FIRE         (I)
#define AFF_INFRARED            (J)
#define AFF_CURSE               (K)
#define AFF_BLOODLUST           (L)
#define AFF_POISON              (M)
#define AFF_PROTECT_EVIL        (N)
#define AFF_PROTECT_GOOD        (O)
#define AFF_SNEAK               (P)
#define AFF_HIDE                (Q)
#define AFF_SLEEP               (R)
#define AFF_CHARM               (S)
#define AFF_FLYING              (T)
#define AFF_PASS_DOOR           (U)
#define AFF_HASTE               (V)
#define AFF_CALM                (W)
#define AFF_PLAGUE              (X)
#define AFF_WEAKEN              (Y)
#define AFF_DARK_VISION         (Z)
#define AFF_BERSERK             (aa)
#define AFF_SWIM                (bb)
#define AFF_REGENERATION        (cc)
#define AFF_SLOW                (dd)
#define AFF_CONFLAGURATION      (ee)

/*
 * New flags
 * in ch->flag2 variable
 */

#define AFF_NOREGEN             (A)
#define AFF_HOLD                (B)
#define AFF_ALLOW_VAMP          (C)
#define AFF_ALLOW_WERE          (D)
#define AFF_ALLOW_ELAD          (E)
#define AFF_ALLOW_DEMON         (F)
#define AFF_ALLOW_WARLOCK       (G)
#define AFF_BLOOD_POTENCE       (H)
#define AFF_WATERBREATHING      (I)
#define AFF_WATER_BREATHING      (I)
#define AFF_SHIELD_AURA         (J)
#define AFF_IMPROVED_INVIS      (K)
#define AFF_CLAW                (L)
#define AFF_BITE                (M)
#define AFF_TAIL                (N)
#define AFF_WING                (O)
#define AFF_PENTAGRAM           (P)
#define AFF_FEAR                (Q)
#define AFF_EARTH_MELD          (R)
#define AFF_SHIVER              (S)
#define AFF_NO_DISARM           (T)
#define AFF_MAGIC_DEAF          (U)
#define AFF_SHIELD              (V)
#define AFF_SHADOW_PLANE        (W)
#define AFF_WRENCHED            (X)
#define AFF_SHADOW_SIGHT        (Y)
#define AFF_LAUGHTER            (Z)
#define AFF_WATER_SHIELD        (aa)
#define AFF_MASKED              (bb)
#define AFF_SHAPE_SHIFTED       (cc)
#define AFF_FAMILIAR            (dd)
#define AFF_TIED                (ee)

/* affects for flag3 */

#define AFF_MOB_PROG_GRANT      (A)
#define AFF_MOB_PROG_NOSHOW     (B)
#define AFF_BORN_DONE           (C)
#define AFF_NO_INTERRUPT        (D)
#define AFF_PESTILENCE          (E)
#define AFF_UNUSED              (F)
#define AFF_STUN                (G)
#define AFF_INVITED             (H)
#define AFF_DUNNO		(I)
#define AFF_STR_BOOST           (J)
#define AFF_DEX_BOOST           (K)
#define AFF_CON_BOOST           (L)
#define AFF_KARMA               (M)
#define AFF_ENRAPTURED          (N)
#define AFF_ENTRAPTURING        (O)
#define AFF_RECEPTACLE          (P)
#define AFF_INSPIRATION         (Q)
#define AFF_ILLUSION            (T)
#define AFF_CHAR_IS_OBJ         (U)
#define AFF_SAY_LOCK            (V)
#define AFF_REFLECTION          (W)
#define AFF_OBJECT_READING      (Y)
#define AFF_SPAM_VIOLATION      (Z)
#define AFF_SPAM_WARNING        (aa)
#define AFF_MUMMY_ROT           (bb)
#define AFF_BABBLE              (cc)
#define AFF_BLOOD_SPEW          (dd)
#define AFF_LYCANTHROPY         (ee)

/* Affects for Flag4, boy there are a lot of these buggers now! */
#define AFF_COMMAND_ENTERED	(A)
#define AFF_BEING_SUCKED	(B)
#define AFF_BALANCED		(C)
#define AFF_AUTOWALKING		(D)
#define AFF_TREE_MELD		(E)
#define AFF_SMOTHERED		(F)
#define AFF_ENTWINED		(G)
#define AFF_WALK_ON_WATER 	(H)
#define AFF_SENESCHIA	 	(I)
#define AFF_PHASING	 	(J)
#define AFF_CAN_FLY	 	(K)
#define AFF_SHADOW_HIDE	 	(L)
#define AFF_HORN	 	(M)
#define AFF_STEEL_CLAWS	 	(N)
#define AFF_TAIL_STINGER 	(O)
#define AFF_FLICKER	 	(P)
#define AFF_RETRIBUTION	 	(Q)
#define AFF_STASIS	 	(R)
#define AFF_ON_A_QUEST	 	(S)
#define AFF_INVULNERABLE 	(T)
#define AFF_NO_TUTOR	 	(U)
#define AFF_ELEMENTAL	 	(V)     /* For werewolf entrapping */
#define AFF_PSI_RECALL	 	(W) 
#define AFF_LANDED	 	(X) 
#define AFF_QUITTING	 	(Y) 
#define AFF_WINGS	 	(Z) 
#define AFF_NO_EXP	 	(aa) 
#define AFF_BECKONED	 	(bb) 
#define AFF_GEN_RESIST  	(cc) 
#define AFF_SCREEN_SPLIT	(dd)
#define AFF_SET_A_RECALL	(ee)

/* Affects for flag 5 I think my sir */
#define AFF5_SUCCULENCE		(A)
#define AFF5_BEAST		(B) /* This one is very important i reckon */
#define AFF5_IMMTRUST		(C) /* Giving mortals the power to stop spam */
#define AFF5_BLINK_1ST_RD	(D)
#define AFF5_BLINK_2ND_RD	(E)
#define AFF5_AIR_BLOOD		(F)
#define AFF5_MAW		(G)
#define AFF5_CLOAKING		(H)
#define AFF5_BANE		(I)
#define AFF5_AIR_BARRIER	(J)
#define AFF5_GED_TRANSFORM	(K)
#define AFF5_BACKFIRE		(L)
#define AFF5_NO_PROMPT_FOR_ANSI	(M)
#define AFF5_WEB_WEAK		(N)
#define AFF5_WEB_STRONG		(O)
#define AFF5_IMPROVED_HASTE	(P)
#define AFF5_IMPROVED_SLOW	(Q)
#define AFF5_SUCCESS		(R)
#define AFF5_MISFORTUNE		(S)
#define AFF5_WHO_LIST		(T)
#define AFF5_BUILDING		(U)
#define AFF5_FAMILIAR		(V)
#define AFF5_INSUBSTANTIAL	(W)
#define AFF5_AGE_ATTACK		(X)
#define AFF5_FAMILIAR_CAT	(Y)
#define AFF5_FAMILIAR_QUASIT	(Z)
#define AFF5_FAMILIAR_HAWK	(aa)
#define AFF5_FAMILIAR_MOUSE	(bb)
#define AFF5_FAMILIAR_WEASEL	(cc)
#define AFF5_WITHERING		(dd)
#define AFF5_SWITCHED		(ee)

/* Holy moley batman, its Fffffffffffflag 6! */
#define AFF6_OBJ_SWITCHED  	(A) /* ONLY for determining chars logging on */
#define AFF6_SHRINK		(B)
#define AFF6_GROWTH		(C)
#define AFF6_REIMBURSED		(D)
#define AFF6_REIMBURSEDTWICE	(E)
#define AFF6_CAN_SPEAK_ELVISH	(F)
#define AFF6_CAN_SPEAK_GNOMISH	(G)
#define AFF6_CAN_SPEAK_DEMONIC	(H)
#define AFF6_CAN_BUILD		(I)
#define AFF6_REGROWTH		(J) /* Limb regenation */
#define AFF6_AUTO_ATTACK	(K)

/* Mushroom Types */
#define MUSHROOM_SPEED		1
#define MUSHROOM_LSD		2
#define MUSHROOM_HASH		3
#define MUSHROOM_KING		4
#define MUSHROOM_WIPE_SPELLS	5
#define MUSHROOM_DEATH		6
#define MUSHROOM_PASSOUT	7
#define MUSHROOM_WALL		8
#define MUSHROOM_ARM_GROW	9
#define MUSHROOM_GET_ALL_SPELLS	10

/* Update this, and put all non-mushroom effects after the number */
#define MAX_MUSHROOM_EFFECT	10

/* Feather types  -  like mushrooms but for zapping */
#define FEATHER_COPPER		1
#define FEATHER_SILVER		2
#define FEATHER_GOLDEN		3
#define FEATHER_PLATINUM	4

/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL                   0
#define SEX_MALE                      1
#define SEX_FEMALE                    2

/* AC types */
#define AC_PIERCE                       0
#define AC_BASH                         1
#define AC_SLASH                        2
#define AC_EXOTIC                       3

/* dice */
#define DICE_NUMBER                     0
#define DICE_TYPE                       1
#define DICE_BONUS                      2

/* size */
#define SIZE_TINY                       0
#define SIZE_SMALL                      1
#define SIZE_MEDIUM                     2
#define SIZE_LARGE                      3
#define SIZE_HUGE                       4
#define SIZE_GIANT                      5



/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_SILVER_ONE           1
#define OBJ_VNUM_GOLD_ONE             2
#define OBJ_VNUM_GOLD_SOME            3
#define OBJ_VNUM_SILVER_SOME          4
#define OBJ_VNUM_COINS                5

#define OBJ_VNUM_CORPSE_NPC          10
#define OBJ_VNUM_CORPSE_PC           11
#define OBJ_VNUM_SEVERED_HEAD        12
#define OBJ_VNUM_TORN_HEART          13
#define OBJ_VNUM_SLICED_ARM          14
#define OBJ_VNUM_SLICED_LEG          15
#define OBJ_VNUM_GUTS                16
#define OBJ_VNUM_BRAINS              17

#define OBJ_VNUM_MUSHROOM            20
#define OBJ_VNUM_LIGHT_BALL          21
#define OBJ_VNUM_SPRING              22
#define OBJ_VNUM_DISC                23
#define OBJ_VNUM_PORTAL              25

#define OBJ_VNUM_BLOOD               26

#define OBJ_VNUM_ROSE                77
#define OBJ_VNUM_BAUBLE		    213

#define OBJ_VNUM_PIT               3010

#define OBJ_VNUM_SCHOOL_MACE       3700
#define OBJ_VNUM_SCHOOL_DAGGER     3701
#define OBJ_VNUM_SCHOOL_SWORD      3702
#define OBJ_VNUM_SCHOOL_SPEAR      3717
#define OBJ_VNUM_SCHOOL_STAFF      3718
#define OBJ_VNUM_SCHOOL_AXE        3719
#define OBJ_VNUM_SCHOOL_FLAIL      3720
#define OBJ_VNUM_SCHOOL_WHIP       3721
#define OBJ_VNUM_SCHOOL_POLEARM    3722

#define OBJ_VNUM_SCHOOL_VEST       3703
#define OBJ_VNUM_SCHOOL_SHIELD     3704
#define OBJ_VNUM_SCHOOL_BANNER     3716
#define OBJ_VNUM_MAP               3162

#define OBJ_VNUM_WHISTLE           2116

#define OBJ_VNUM_STATUE            93


#define POTION_GAIN_BREW	   30
#define POTION_BEER		   31


/*
 * Perm item affects - cannot use sn, but must use different flags
 *  Only one per item please.... :)
 */
#define ITEMA_SANCTUARY		0
#define ITEMA_HASTE		1
#define ITEMA_WINDTOMB		2
#define ITEMA_FLY		3
#define ITEMA_ACCURACY		4
#define ITEMA_DETECT_EVIL	5
#define ITEMA_INVISIBILITY	6
#define ITEMA_DETECT_MAGIC	7
#define ITEMA_ARMOR		8
#define ITEMA_STONE_SKIN	9
#define ITEMA_SHIELD		10
#define ITEMA_DETECT_GOOD	11
#define ITEMA_BLESS		12
#define ITEMA_WATER_BREATHING	13
#define ITEMA_IRON_SKIN		14
#define ITEMA_PLAGUE		15
#define ITEMA_PROTECTION_EVIL	16


/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT                    1
#define ITEM_SCROLL                   2
#define ITEM_WAND                     3
#define ITEM_STAFF                    4
#define ITEM_WEAPON                   5
#define ITEM_TREASURE                 8
#define ITEM_ARMOR                    9
#define ITEM_POTION                  10
#define ITEM_CLOTHING                11
#define ITEM_FURNITURE               12
#define ITEM_TRASH                   13
#define ITEM_CONTAINER               15
#define ITEM_DRINK_CON               17
#define ITEM_KEY                     18
#define ITEM_FOOD                    19
#define ITEM_MONEY                   20
#define ITEM_BOAT                    22
#define ITEM_CORPSE_NPC              23
#define ITEM_CORPSE_PC               24
#define ITEM_FOUNTAIN                25
#define ITEM_PILL                    26
#define ITEM_PROTECT                 27
#define ITEM_MAP                     28
#define ITEM_PORTAL                  29
#define ITEM_WARP_STONE              30
#define ITEM_ROOM_KEY                31
#define ITEM_GEM                     32
#define ITEM_JEWELRY                 33
#define ITEM_JUKEBOX                 34
#define ITEM_QUEST_CARD              35
#define ITEM_SPELL_BOOK              36
#define ITEM_BOW                     37
#define ITEM_ARROW                   38
#define ITEM_TOME                    39
#define ITEM_CAULDRON                40
#define ITEM_SPEC_POTION	     41
#define ITEM_ORBUS		     42
#define ITEM_PSI_STONE		     43
#define ITEM_MAGIC_MIRROR	     44
#define ITEM_CONT_LIGHT		     45
#define ITEM_BODY_PART		     46
#define ITEM_VOODOO_DOLL	     47
#define ITEM_FEATHER		     48
#define ITEM_DRAGON_BLOOD	     49
#define ITEM_RAINBOW_POOL	     50
#define ITEM_CASH		     51
#define ITEM_PEN_TOME		     52
#define ITEM_BAUBLE		     53
#define ITEM_EMPTY_VIAL		     54
#define ITEM_QUIVER		     55


/* Parts of a voodoo doll */
#define DOLL_RIGHT_LEG		(A)
#define DOLL_LEFT_LEG		(B)
#define DOLL_RIGHT_ARM		(C)
#define DOLL_LEFT_ARM		(D)

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW               (A)
#define ITEM_HUM                (B)
#define ITEM_DARK               (C)
#define ITEM_LOCK               (D)
#define ITEM_EVIL               (E)
#define ITEM_INVIS              (F)
#define ITEM_MAGIC              (G)
#define ITEM_NODROP             (H)
#define ITEM_BLESS              (I)
#define ITEM_ANTI_GOOD          (J)
#define ITEM_ANTI_EVIL          (K)
#define ITEM_ANTI_NEUTRAL       (L)
#define ITEM_NOREMOVE           (M)
#define ITEM_INVENTORY          (N)
#define ITEM_NOPURGE            (O)
#define ITEM_ROT_DEATH          (P)
#define ITEM_VIS_DEATH          (Q)
#define ITEM_NONMETAL           (S)
#define ITEM_NOLOCATE           (T)
#define ITEM_MELT_DROP          (U)
#define ITEM_HAD_TIMER          (V)
#define ITEM_SELL_EXTRACT       (W)
#define ITEM_BURN_PROOF         (Y)
#define ITEM_NOUNCURSE          (Z)
#define ITEM_ARTIFACT           (aa)
#define ITEM_RELIC              (bb)
#define ITEM_FLAMING            (cc)
#define ITEM_HIT_EMPOWERED      (dd)
#define ITEM_DAM_EMPOWERED      (ee)

/* Item extra flags II, the return of the item flags!  */

#define ITEM_WEAP_EFFECT_ADDED  (A)
#define ITEM_NYSTUL             (B)
#define ITEM_NO_INTERRUPT       (C)
#define ITEM_DAEMONSEED		(D)
#define ITEM_STAKE		(E)
#define ITEM_JUJU_BAG		(F)
#define ITEM_VOO_HEAD		(G)
#define ITEM_VOO_DEAD		(H)
#define ITEM_VOO_BODY		(I)
#define ITEM_VOO_THREAD		(J)
#define ITEM_INGRED_GLOIN	(K)
#define ITEM_INGRED_FROGBREATH	(L)
#define ITEM_INGRED_PAPACOCO	(M)
#define ITEM_INGRED_MULDALEAF	(N)
#define ITEM_INGRED_SCULLYWEED	(O)
#define ITEM_INGRED_WORMWART	(P)
#define ITEM_INGRED_TILLIFREEN	(Q)
#define ITEM_INGRED_BAJUJU	(R)
#define ITEM_ATTACK_NON_ELAD    (S)
#define ITEM_ITEMHIDE	        (T)
#define ITEM_ICE	        (U)

/* artifact and relic flags */
#define ITEM_LOYAL		(V)  /* transferred to owner upon owner death */
#define ITEM_TELEPORTS		(W)  /* teleports upon owner death */
#define ITEM_DESIRED		(Z)  /* you can't let go of it ever */
#define ITEM_INDESTRUCTABLE	(aa) /* guess */
#define ITEM_TELEPORT_PROTECTION (bb) /* teleports when attempts to destroy it */
#define ITEM_KNOW_OWNER 	 (cc) /* owner displayed on artifact command */

#define ITEM_FLYING	        (dd)
#define ITEM_FORM_MELDED	(ee)

/* Item extra flags III, the return of the return of the item flags!  */
#define ITEM_BODY_HAND		(A)
#define ITEM_BODY_LEG		(B) /* for grafting purposes only */
#define ITEM_BODY_ARM		(C)
#define ITEM_EMPOWER_IMPROVED   (D)
#define ITEM_DESTROY_CHAR_RETURN  (E)
#define ITEM_CANNOT_RETURN      (F)
#define ITEM_INHABITED	        (G)
#define ITEM_CANNOT_MOVE        (H)
#define ITEM_DESTROY_CHAR_IF_SACCED  (I)
#define ITEM_HIDE_MAGIC         (J)
#define ITEM_IRON               (K)
#define ITEM_STRUCTURE          (L)
#define ITEM_STRETCHY           (M)

/* Magic bows, arrows and quivers */
#define ITEM_ARROW_RETURNING	(O)
#define ITEM_BOW_FLAMING	(N)
#define ITEM_SPELL_BOW		(O) /* V[5] contains the spell! */
#define ITEM_ARROW_FLAMING	(P)
#define ITEM_DESTROYED		(Q)



/*
 *  Magical artifacts, coded into the game!!!! Hheeeeeeeheheheheh
 */

#define ARTIFACT_LODERING	1    /* Immune to hurl and gust */
#define ARTIFACT_ARMET		2    /* Double damage fireballs */
#define ARTIFACT_KHAROS		3    /* Gain extra spells */
#define ARTIFACT_SYMAL		4    /* Cant pick up any items */
#define ARTIFACT_JANDOR		5    /* Immune to everything */
#define ARTIFACT_STAFF_WIZARDRY 6    /* Get spells back quicker */
#define ARTIFACT_CROWN_OF_AGES  7    /* May use psionics!!! + better rate */
#define ARTIFACT_FIRE_OPAL	8    /* Can cast flame spells !!! */
#define ARTIFACT_PTERMIAN_SCARAB 9   /* Can cast earth spells !!! */
#define ARTIFACT_NECKLACE_BONES 10    /* Can cast entropy spells */
#define ARTIFACT_EYE_SENSUS     11    /* Increased wisdom and see arti-owners */
#define ARTIFACT_TELECASTER     12    /* Play and blow people up */


#define RELIC_WIND_BOOTS	100  /* Perm fly */

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE               (A)
#define ITEM_WEAR_FINGER        (B)
#define ITEM_WEAR_NECK          (C)
#define ITEM_WEAR_BODY          (D)
#define ITEM_WEAR_HEAD          (E)
#define ITEM_WEAR_LEGS          (F)
#define ITEM_WEAR_FEET          (G)
#define ITEM_WEAR_HANDS         (H)
#define ITEM_WEAR_ARMS          (I)
#define ITEM_WEAR_SHIELD        (J)
#define ITEM_WEAR_ABOUT         (K)
#define ITEM_WEAR_WAIST         (L)
#define ITEM_WEAR_WRIST         (M)
#define ITEM_WIELD              (N)
#define ITEM_HOLD               (O)
#define ITEM_NO_SAC             (P)
#define ITEM_WEAR_FLOAT         (Q)

/* weapon class */
#define WEAPON_EXOTIC           0
#define WEAPON_SWORD            1
#define WEAPON_DAGGER           2
#define WEAPON_SPEAR            3
#define WEAPON_MACE             4
#define WEAPON_AXE              5
#define WEAPON_FLAIL            6
#define WEAPON_WHIP             7       
#define WEAPON_POLEARM          8

/* weapon types */
#define WEAPON_FLAMING          (A)
#define WEAPON_FROST            (B)
#define WEAPON_VAMPIRIC         (C)
#define WEAPON_SHARP            (D)
#define WEAPON_VORPAL           (E)
#define WEAPON_TWO_HANDS        (F)
#define WEAPON_SHOCKING         (G)
#define WEAPON_POISON           (H)
#define WEAPON_SUNBLADE         (I)
#define WEAPON_DRAGONLANCE      (J)
#define WEAPON_SILVER           (K)
#define WEAPON_RUNE_FORCE_BOLT  (L)
#define WEAPON_RUNE_SMITE_EVIL  (M)
#define WEAPON_RUNE_BLAZE       (N)
#define WEAPON_RUNE_LIGHTNING   (O)
#define WEAPON_RUNE_DANCING     (P)
#define WEAPON_ELE_FLAME        (Q)
#define WEAPON_ELE_WATER        (R)
#define WEAPON_ELE_EARTH        (S)
#define WEAPON_ELE_AIR          (T)
#define WEAPON_SLAY_UNDEAD      (U)
#define WEAPON_CAUSTIC          (V)
#define WEAPON_SMITE_ELADRIN 	(W)
#define WEAPON_SMITE_DRAGONS 	(X)
#define WEAPON_SMITE_WARLOCKS 	(Y)
#define WEAPON_SMITE_DEMONS 	(Z)

/* gate flags */
#define GATE_NORMAL_EXIT        (A)
#define GATE_NOCURSE            (B)
#define GATE_GOWITH             (C)
#define GATE_BUGGY              (D)
#define GATE_RANDOM             (E)

/* furniture flags */
#define STAND_AT                (A)
#define STAND_ON                (B)
#define STAND_IN                (C)
#define SIT_AT                  (D)
#define SIT_ON                  (E)
#define SIT_IN                  (F)
#define REST_AT                 (G)
#define REST_ON                 (H)
#define REST_IN                 (I)
#define SLEEP_AT                (J)
#define SLEEP_ON                (K)
#define SLEEP_IN                (L)
#define PUT_AT                  (M)
#define PUT_ON                  (N)
#define PUT_IN                  (O)
#define PUT_INSIDE              (P)




/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE                    0
#define APPLY_STR                     1
#define APPLY_DEX                     2
#define APPLY_INT                     3
#define APPLY_WIS                     4
#define APPLY_CHA                     26
#define APPLY_CON                     5
#define APPLY_SEX                     6
#define APPLY_CLASS                   7
#define APPLY_LEVEL                   8
#define APPLY_AGE                     9
#define APPLY_HEIGHT                 10
#define APPLY_WEIGHT                 11
#define APPLY_HIT                    13
#define APPLY_MOVE                   14
#define APPLY_GOLD                   15
#define APPLY_EXP                    16
#define APPLY_AC                     17
#define APPLY_HITROLL                18
#define APPLY_DAMROLL                19
#define APPLY_SAVES                  20
#define APPLY_SAVING_PARA            20
#define APPLY_SAVING_ROD             21
#define APPLY_SAVING_PETRI           22
#define APPLY_SAVING_BREATH          23
#define APPLY_SAVING_SPELL           24
#define APPLY_SPELL_AFFECT           25

/* Offensive and defensive flags for TOMES, A Warlocks Personal Arsenal */
#define TOME_SPEED                   (A)
#define TOME_HEAL                    (B)
#define TOME_SELFHEAL                (C)
#define TOME_ARMOR                   (D)
#define TOME_SHROUD                  (E)
#define TOME_PLAGUE                  (F)
#define TOME_FLAMEBALL               (G)
#define TOME_BLAST                   (H)
#define TOME_SUNBEAM                 (I)
#define TOME_VENOM                   (J)
#define TOME_SHATTER                 (K)




/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE                1
#define CONT_PICKPROOF                2
#define CONT_CLOSED                   4
#define CONT_LOCKED                   8
#define CONT_PUT_ON                  16



/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO               2
#define ROOM_VNUM_CHAT             1200
#define ROOM_VNUM_TEMPLE           3001
#define ROOM_VNUM_ALTAR            3054
#define ROOM_VNUM_SCHOOL           3700
#define ROOM_VNUM_BALANCE          4500
#define ROOM_VNUM_CIRCLE           4400
#define ROOM_VNUM_DEMISE           4201
#define ROOM_VNUM_HONOR            4300
#define ROOM_VNUM_START		   7900


/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK               (A)
#define ROOM_NO_MOB             (C)
#define ROOM_INDOORS            (D)

#define ROOM_PRIVATE            (J)
#define ROOM_SAFE               (K)
#define ROOM_SOLITARY           (L)
#define ROOM_PET_SHOP           (M)
#define ROOM_NO_RECALL          (N)
#define ROOM_CROSS_WATER        (O)
#define ROOM_HOLY               (P)
#define ROOM_MAGICAL            (Q)
#define ROOM_UNUSED		(R)
#define ROOM_SMALL              (S)
#define ROOM_NOWHERE            (T)
#define ROOM_NO_EXIT            (U)
#define ROOM_TORNADO            (V)
#define ROOM_VOLCANO            (W)
#define ROOM_WHIRLWIND          (X)
#define ROOM_PIT_TRAP           (Y)
#define ROOM_NO_MAGIC           (Z)
#define ROOM_NO_MELD            (aa)
#define ROOM_NO_SHADOWS         (bb)
#define ROOM_RESTRICTED         (cc)
#define ROOM_ETHERIAL           (dd)
#define ROOM_CONT_LIGHT		(ee)


/* ROOM 2 flags, just cos we run out of room... */
#define ROOM_CLOUD_PUTREFACTION (A)
#define ROOM_CLOUD_PAIN		(B)
#define ROOM_QUEST_CENTRE	(C)
#define ROOM_TAVERN		(D)


#define GAME_RUNESTONE 		(A)
#define GAME_DICE		(B)
#define GAME_SKULL_DICE		(C)
#define GAME_ROULETTE		(D)

/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH                     0
#define DIR_EAST                      1
#define DIR_SOUTH                     2
#define DIR_WEST                      3
#define DIR_UP                        4
#define DIR_DOWN                      5


/* TESTS for the new test function */
#define IS_ELADRIN			1


/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR                     (A)
#define EX_CLOSED                     (B)
#define EX_LOCKED                     (C)
#define EX_PICKPROOF                  (F)
#define EX_NOPASS                     (G)
#define EX_EASY                       (H)
#define EX_HARD                       (I)
#define EX_INFURIATING                (J)
#define EX_NOCLOSE                    (K)
#define EX_NOLOCK                     (L)

#define EX_ICE_WALL                   (M)
#define EX_FIRE_WALL                  (N)
#define EX_SWORD_WALL                 (O)
#define EX_PRISMATIC_WALL             (P)
#define EX_IRON_WALL                  (Q)
#define EX_MUSHROOM_WALL              (R)
#define EX_CALTROP_WALL               (S)
#define EX_WATER_WALL                 (T)
#define EX_ASH_WALL                   (U)

#define EX_WARDING                    (V)

#define MAX_WALL		       9

/* Change the new ones in is_wall in update.c, and update MAX_WALL */


/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE                   0
#define SECT_CITY                     1
#define SECT_FIELD                    2
#define SECT_FOREST                   3
#define SECT_HILLS                    4
#define SECT_MOUNTAIN                 5
#define SECT_WATER_SWIM               6
#define SECT_WATER_NOSWIM             7
#define SECT_UNDERWATER               8
#define SECT_AIR                      9
#define SECT_DESERT                  10
#define SECT_NARROW_TUNNEL           11
#define SECT_HOME		     12
#define SECT_MAX                     13

/*
 * Body parts used for wearing etc.
 */

#define	BODY_RIGHT_LEG		(A)
#define	BODY_LEFT_LEG		(B)
#define BODY_RIGHT_HAND		(C)
#define BODY_LEFT_HAND		(D)
#define	BODY_RIGHT_ARM		(E)
#define BODY_LEFT_ARM		(F)
#define BODY_THIRD_HAND		(G)
#define BODY_FOURTH_HAND	(H)
#define BODY_THIRD_ARM		(I)
#define BODY_FOURTH_ARM		(J)
#define BODY_LEFT_EAR		(K)
#define BODY_RIGHT_EAR		(L)
#define BODY_LEFT_EYE		(M)
#define BODY_RIGHT_EYE		(N)
#define BODY_ENABLE_THIRD_ARM	(O)
#define BODY_ENABLE_FOURTH_ARM	(P)

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE                    -1
#define WEAR_FINGER_L                 0
#define WEAR_FINGER_R                 1
#define WEAR_NECK_1                   2
#define WEAR_NECK_2                   3
#define WEAR_BODY                     4
#define WEAR_HEAD                     5
#define WEAR_LEGS                     6
#define WEAR_FEET                     7
#define WEAR_HANDS                    8
#define WEAR_ARMS                     9
#define WEAR_ABOUT                   10
#define WEAR_WAIST                   11
#define WEAR_WRIST_L                 12
#define WEAR_WRIST_R                 13
#define WEAR_FLOAT                   14
#define WEAR_RIGHT_HAND		     15
#define WEAR_LEFT_HAND		     16
#define WEAR_THIRD_HAND		     17
#define WEAR_FOURTH_HAND	     18
#define MAX_WEAR                     19



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK                    0
#define COND_FULL                     1
#define COND_THIRST                   2
#define COND_HUNGER                   3



/*
 * Positions.
 */
#define POS_DEAD                      0
#define POS_MORTAL                    1
#define POS_INCAP                     2
#define POS_STUNNED                   3
#define POS_SLEEPING                  4
#define POS_RESTING                   5
#define POS_SITTING                   6
#define POS_FIGHTING                  7
#define POS_STANDING                  8


/*
 * ACT bits for players.
 */
#define PLR_IS_NPC              (A)             /* Don't EVER set.      */

/* Auto flags  - Bagashite really, so we made them better */
#define PLR_AUTOASSIST          (C)
#define PLR_AUTOEXIT            (D)
#define PLR_AUTOLOOT            (E)
#define PLR_AUTOSAC             (F)
#define PLR_AUTOGOLD            (G)
#define PLR_AUTOSPLIT           (H)
#define PLR_AUTOSCAN            (I)
#define PLR_HOLYLIGHT           (N)
#define PLR_CANLOOT             (P)
#define PLR_NOSUMMON            (Q)
#define PLR_NOFOLLOW            (R)
#define PLR_PERMIT              (U)
#define PLR_LOG                 (W)
#define PLR_DENY                (X)
#define PLR_FREEZE              (Y)
#define PLR_THIEF               (Z)
#define PLR_KILLER              (aa)


#define COMM_QUIET              (A)
#define COMM_DEAF               (B)
#define COMM_NOWIZ              (C)
#define COMM_NOAUCTION          (D)
#define COMM_NOGOSSIP           (E)
#define COMM_NOQUESTION         (F)
#define COMM_NOMUSIC            (G)
#define COMM_NOCLAN             (H)
#define COMM_NOQUOTE            (I)
#define COMM_SHOUTSOFF          (J)

/* display flags */
#define COMM_COMPACT            (L)
#define COMM_BRIEF              (M)
#define COMM_PROMPT             (N)
#define COMM_COMBINE            (O)
#define COMM_TELNET_GA          (P)
#define COMM_SHOW_AFFECTS       (Q)
#define COMM_NOGRATS            (R)

/* penalties */
#define COMM_NOEMOTE            (T)
#define COMM_NOSHOUT            (U)
#define COMM_NOTELL             (V)
#define COMM_NOCHANNELS         (W) 
#define COMM_SNOOP_PROOF        (Y)
#define COMM_AFK                (Z)


#define COMM_FIGHT_PROMPT       (bb)

/* colour */

#define COMM_ANSI		(aa)

/* Wiz command granting flags */
#define CMD_CONNECTIONS         (A)
#define CMD_ECHO                (B)
#define CMD_CONTROL             (C)
#define CMD_CHANNELS            (E)
#define CMD_LOCK                (F)
#define CMD_REBOOT              (G)
#define CMD_SWITCH              (H)
#define CMD_SITES               (I)
#define CMD_FUN                 (J)
#define CMD_LOCATION            (K)
#define CMD_GLOBAL              (L)
#define CMD_LOAD                (M)




/* WIZnet flags */
#define WIZ_ON                  (A)
#define WIZ_TICKS               (B)
#define WIZ_LOGINS              (C)
#define WIZ_SITES               (D)
#define WIZ_LINKS               (E)
#define WIZ_DEATHS              (F)
#define WIZ_RESETS              (G)
#define WIZ_MOBDEATHS           (H)
#define WIZ_FLAGS               (I)
#define WIZ_PENALTIES           (J)
#define WIZ_SACCING             (K)
#define WIZ_LEVELS              (L)
#define WIZ_SECURE              (M)
#define WIZ_SWITCHES            (N)
#define WIZ_SNOOPS              (O)
#define WIZ_RESTORE             (P)
#define WIZ_LOAD                (Q)
#define WIZ_NEWBIE              (R)
#define WIZ_PREFIX              (S)
#define WIZ_SPAM                (T)

#define QUEST_ITEM_1		(A)
#define QUEST_ITEM_2		(B)
#define QUEST_ITEM_3		(C)
#define QUEST_MOB_1		(D)
#define QUEST_MOB_2		(E)
#define QUEST_MOB_3		(F)
#define QUEST_ROOM_1		(G)
#define QUEST_ROOM_2		(H)
#define QUEST_ROOM_3		(I)

#define NUM_ITEM_TRIG           1
#define IP_MAX_LINES            30
#define ITEM_TRIG_WEAR          0
#define ITEM_TRIG_DROP          1
#define ITEM_TRIG_GET           2
#define ITEM_TRIG_TURN          3
#define ITEM_TRIG_TWIST         4
#define ITEM_TRIG_GIVE          5

#define NUM_TRIGS		20
#define TRIG_SAY		0
#define TRIG_ENTER_ROOM		1
#define TRIG_KILLED		2
#define TRIG_BORN		3
#define TRIG_KILL		4
#define TRIG_INIT_COMBAT	5
#define TRIG_FIGHTING		6
#define TRIG_SOCIAL		7
#define TRIG_THREAT		8
#define TRIG_PROG		9
#define TRIG_LEAVE_ROOM		10
#define TRIG_GIVE		11
#define TRIG_TIMED_OUT		12
#define TRIG_DROP		13
#define TRIG_ENEMY_FLEE		14
#define TRIG_MOB_FLEE		15
#define TRIG_WIMPY		16
#define TRIG_RANDOM		17
#define TRIG_ENEMY_CAST		18
#define TRIG_KILL_ENEMY		19


struct delay_item_trig_data
{
    OBJ_DATA *obj;
    CHAR_DATA *ch;
    CHAR_DATA *victim;
    char *data;
    char *info;
    int ln;
    int iflevel;
};


/* colour values */

#define ANSI_NORMAL	"ESC [ 0 m"
#define ANSI_BOLD	"ESC [ 5 m"
#define ANSI_REVERSE    "ESC [ 7 m"
#define ANSI_COLOUR     "ESC [ 3 * m"
#define ANSI_BACKGROUND "ESC [ 4 * m"
 
/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct  mob_index_data
{
    MOB_INDEX_DATA *    next;
    AREA_DATA	   *	area;
    sh_int              vnum;
    sh_int              group;
    bool                new_format;
    sh_int              count;
    sh_int              killed;
    char *              player_name;
    char *              short_descr;
    char *              long_descr;
    char *              description;
    long                mobolc_flags;  /* Movement and affected by */
    long                mobolc_flags2; /* Combat stuff */
    long                act;
    long                affected_by;
    sh_int              alignment;
    sh_int              level;
    sh_int              hitroll;
    sh_int              hit[3];
    sh_int              damage[3];
    sh_int              ac[4];
    sh_int              dam_type;
    long                off_flags;
    long                imm_flags;
    long                res_flags;
    long                vuln_flags;
    sh_int              start_pos;
    sh_int              default_pos;
    sh_int              sex;
    sh_int              race;
    long                wealth;
    long                form;
    long                parts;
    long                mob_spec;
    sh_int              size;
    char *              material;
    int              	body_form;
    sh_int              secondaryrace;
    MOB_PROG_DATA *	trig[NUM_TRIGS];
    sh_int              tertiaryrace;
};


/* marriage status */
#define MARRY_PROPOSING  1
#define MARRY_ENGAGED    2
#define MARRY_MARRIED    3

/* memory settings */
#define MEM_CUSTOMER    A       
#define MEM_SELLER      B
#define MEM_HOSTILE     C
#define MEM_AFRAID      D

/* memory for mobs */
struct mem_data
{
    MEM_DATA    *next;
    bool        valid;
    int         id;     
    int         reaction;
    time_t      when;
};

#define NUM_VALS_MOBS           10

#define MAX_MOB_PROG_LINES	120
#define MAX_EDITOR_LINES	(MAX_MOB_PROG_LINES)

struct email_data
{
    char *		address;
    EMAIL_DATA *	next;
};

struct site_data
{
    char *		number;
    char *		name;
    SITE_DATA *		next;
};

struct config_data
{
    int			mob_hp_mult;
    int			mob_hp_div;
    int			mob_level_mult;
    int			mob_level_div;
    int			mob_dam_mult;
    int			mob_dam_div;
    int			mob_ac_mult;
    int			mob_ac_div;
    int			prac_exp_mult;
    int			prac_exp_div;
    int			research_exp_mult;
    int			research_exp_div;
    int			research_time_mult;
    int			research_time_div;
    int			train_exp_mult;
    int			train_exp_div;
    int			max_players;
    char *		autohelp;
};

struct mob_prog_data
{
    MOB_PROG_DATA *	next;
    char * 		line[MAX_MOB_PROG_LINES];
    char * 		proc_name;
    bool		valid;
};

struct balance_data
{
    int			pkills;
    int			pkilled;
    int			members;
    int			total_score;
    int			position;
    int			total_gen;
    int			num_tertiary;
    int			total_alignment;
};


struct prog_data
{
    PROG_DATA *		next;
    CHAR_DATA *		ch;
    CHAR_DATA *		trig;
    MOB_PROG_DATA *	running;
    CHAR_DATA *		trg;
    char		r[8][MAX_INPUT_LENGTH];
    int			iflevel, whilelevel;
    int			whilestack[20];
    int			pause;
    int			frozen;
    int			cur_line;
    int			trigger;
};


/* Fighting Style flags */

#define	FS_BLOCK		(A)
#define FS_ATTACK		(B)
#define FS_CONSERVATIVE		(C)
#define FS_AGGRESSIVE		(D)
#define FS_CONCENTRATE		(E)
#define FS_RETREAT		(F)
#define FS_HIGHEST_BLOCK	(G)
#define FS_LOWEST_BLOCK		(H)
#define FS_ANY_BLOCK		(I)
#define FS_SELECTIVE_BLOCK	(J)

/*
 * One character (PC or NPC).
 */
struct  char_data
{
    CHAR_DATA *         next;
    CHAR_DATA *         next_in_room;
    CHAR_DATA *         master;
    CHAR_DATA *         leader;
    CHAR_DATA *         fighting;
    CHAR_DATA *         reply;
    CHAR_DATA *         pet;
    CHAR_DATA *		riding;
    CHAR_DATA *		blinkykill;
    CHAR_DATA *		riden_by;
    CHAR_DATA *		shadow_victim;
    CHAR_DATA *		shadow_master;
    CHAR_DATA * 	sucking;
    SHOP_DATA *         pShop;
    OBJ_DATA *		sw_obj;
    OBJ_DATA *		made_from;
    MEM_DATA *          memory;
    MOB_INDEX_DATA *    pIndexData;
    DESCRIPTOR_DATA *   desc;
    REALM_DATA *	realm;
    AFFECT_DATA *       affected;
    NOTE_DATA *         pnote;
    OBJ_DATA *          carrying;
    OBJ_DATA *          on;
    ROOM_INDEX_DATA *   in_room;
    ROOM_INDEX_DATA *   was_in_room;
    AREA_DATA *         zone;
    PC_DATA *           pcdata;
    GEN_DATA *          gen_data;
    sh_int		dragtype;
    sh_int		aura;
    bool                valid;
    bool                autoroute;
    char *              route;
    int                 routepos;
    char *              name;
    long                id;
    char *		married_to;
    int			marriage_status;
    sh_int              version;
    long                currentexp;
    int			build_realm;
    int			body_form;
    int                 secondaryrace;
    sh_int		auto_look_line;
    int			beast;  /* Adams new blood thing erplacing value[4] */
    int			blood;  /* Adams new blood thing erplacing value[0] */
    int			initiative[5]; /* fight move points, 4 hands  + body */
    int			stunned; /* length of time before initiative moves again */
    int			block_arm;
    int			distance;
    int			des_dist;
    long		fight_style;
    int			registration_key;
    int			registration_logins;
    int 		email_set;
    int			registerred;
    int			breaths;
    int			langskill[NUM_LANGUAGES];
    int			curlang;
    int			altitude;
    sh_int		tick_timer[MAX_TIMER];
    sh_int              tertiaryrace;
    int                 illus_dam;
    sh_int              drowning;
    int                 upkeep;
    int                 stance;
    int                 stance_skill[12];
    sh_int              cur_form;
    long                grant_cmds;
    long                flag2;
    long                flag3;
    long                flag4;
    long                flag5;
    long                flag6;
    long                mob_spec;
    int                 pkills;
    int                 pkilled;
    int                 value[NUM_VALS_MOBS];
    long                quest_progress;
    long                quest2_progress;
    int                 quest_status[100][5];
    sh_int              quest_claimed[100];
    int                 generation;
    int                 power[MAX_DISCIPLINES];
    int			trades[MAX_TRADES];
    int			trade_researching;
    int			trade_time;
    int                 immpower;
    int                 qp;
    int                 psp;
    int                 max_psp;
    int                 psi_level;
    int                 psi_potential;
    sh_int              psi_learnt[NUM_PSI_SLOTS];
    PSI_UPKEEP          psi_upkeep[NUM_PSI_SLOTS];
    int                 fire;
    int                 water;
    int                 air;
    int                 earth;
    int                 entropy;
    int                 lifemagic;
    int                 primary;
    int                 faith;
    int                 true_faith;
    char *              short_descr;
    char *              long_descr;
    char *              description;
    char *              prompt;
    char *              fightprompt;
    char *		tempprompt;
    char *              prefix;
    int			vision;
    CHAR_DATA *		visionary;
    sh_int              group;
    sh_int              clan;
    sh_int              sex;
    sh_int              race;
    sh_int              level;
    sh_int              trust;
    int                 played;
    int                 lines;  /* for the pager */
    time_t              logon;
    sh_int              timer;
    sh_int              wait;
    sh_int              daze;
    int                 hit;
    int                 max_hit;
    sh_int              move;
    sh_int              max_move;
    long                gold;
    long                silver;
    long                copper;
    long                emeralds;
    long                sapphires;
    long                diamonds;
    int                 exp;
    long                act;
    long                comm; 
    long                wiznet; /* wiz stuff */
    long                imm_flags;
    long                res_flags;
    long                vuln_flags;
    sh_int              invis_level;
    sh_int              incog_level;
    int                 advpower[14];
    long                affected_by;
    sh_int              position;
    sh_int              carry_weight;
    int                 recall_room;
    sh_int              spikes;
    sh_int              carry_number;
    sh_int              saving_throw;
    sh_int              alignment;
    sh_int              hitroll;
    sh_int              damroll;
    sh_int              xhitroll;
    sh_int              xdamroll;
    sh_int              armor[4];
    sh_int              wimpy;
    /* stats */
    sh_int              perm_stat[MAX_STATS];
    sh_int              mod_stat[MAX_STATS];
    /* parts stuff */
    long                form;
    long                parts;
    sh_int              size;
    char*               material;
    int                 style_skill[8];
    int                 elder[9];
    /* mobile stuff */
    long                off_flags;
    sh_int              damage[3];
    sh_int              dam_type;
    sh_int              start_pos;
    sh_int              default_pos;
    int                 language;
    bool                know_lang[NUM_LANGUAGES];
    int			best_mob_killed;
    int			rage;
    bool		run_trig[NUM_TRIGS];
    MOB_PROG_DATA *	attached_trig[NUM_TRIGS];
    PROG_DATA *		halted_trig[NUM_TRIGS];
    bool		latch_halted[NUM_TRIGS];
    bool		all_trig_claim;
};



/*
 * Data which only PC's have.
 */
struct  pc_data
{
    PC_DATA *           next;
    char *              roomdesc;
    BUFFER *            buffer;
    bool                valid;
    int			voodoo_points;
    int			disc_points;
    int			disc_research;
    int 		faeriepath[4];
    int			sanity;
    char *              pwd;
    char *              bamfin;
    char *              bamfout;
    char *              title;
    char *              custom_enter_room;
    char *              enter_room;
    char *              lineage[13];
    char *              title_prefix;
    char *              conditioned_by;
    char *		email;
    char * 		real_name; 
    char * 		homepage; 
    char *		plan;
    char * 		quest_item_1;
    char * 		quest_item_2;
    char *		quest_item_3;
    char *		quest_room_1;
    char *		quest_room_2;
    char *		quest_room_3;
    char *		quest_mob_1;
    char *		quest_mob_2;
    char *		quest_mob_3;
    char *		evileye;
    char * 		racial_enemy;
    int			education;
    sh_int		colours[8];
    int			extra_age;
    int			quests_completed;
    long		quest_status;
    sh_int		max_memorize[11];
    sh_int		memorize[11][11];
    sh_int		last_memorized[11][11];
    MANUAL_DATA *       curman;
    char *              switchname;
    time_t              last_note;
    time_t              last_idea;
    time_t              last_penalty;
    time_t              last_news;
    time_t              last_changes;
    sh_int              things_researched;
    int			souls;
    sh_int              perm_hit;
    sh_int              perm_move;
    sh_int              true_sex;
    sh_int              damcap;
    int                 last_level;
    int			curr_vnum_medit;
    int			curr_vnum_oedit;
    sh_int              condition       [4];
    sh_int              learned         [MAX_SKILL];
    sh_int              innate          [MAX_SKILL];
    sh_int              innate_max      [MAX_SKILL];
    sh_int              innate_regain   [MAX_SKILL];
    sh_int              innate_timer    [MAX_SKILL];
    sh_int		treelearn	[MAX_NON_SPELL_SKILLS];
    bool                group_known     [MAX_GROUP];
    sh_int              points;
    bool                confirm_delete;
    char *              alias[MAX_ALIAS];
    char *              alias_sub[MAX_ALIAS];
    int			home;
    int			mob_kills;
    int			mob_killed;
    char *		create_site;
    char *		last_login_from;
    char *		last_login_time;
    char		edit[MAX_EDITOR_LINES][80]; 		/* editor stufF */
    char ** 		editing;
    long *		editing_flag;
    char *		editor_title;
    int			edit_con;
    int			csrx;
    int			csry;
    int			csrrx;
    int			scry;
    long		menu_ecf[MAX_EDITOR_LINES];
    long		menu_esg[MAX_EDITOR_LINES];
    int			menu_range[MAX_EDITOR_LINES][2];
    void **		menu_variable[MAX_EDITOR_LINES];
    int			menu_flag[MAX_EDITOR_LINES];
    int			edit_flags;	/* editor control flags */
    char *		tempbuf;
    char *		edit_excom;	/* command to execute when done editing */
    int		psi_disc[8];		/* order of discs bought */
    int		psi_disc_points;	/* Number of discs you can gain */
    int		psi_sci_points;		/* number of sciences you can learn */
    int		psi_dev_points;		/* number of devotions you can learn */
    int		psi_def_points;		/* number of defences you can learn */
};


#define EDIT_EDITING		0
#define EDIT_CONFIRM_CANCEL	1
#define EDIT_CONFIRM_SAVE	2
#define EDIT_INITIALISING	3
#define EDIT_TEST		4

#define EF_OVERWRITE		(A)
#define EF_FIX			(B)
#define EF_MENU			(C)
#define EF_NO_OUT		(D)
#define EF_OPTION_MENU		(E)

#define MENU_TYPE_END_MARKER	-1
#define MENU_TYPE_BOOL		0
#define MENU_TYPE_LETTER	1
#define MENU_TYPE_STRING	2
#define MENU_TYPE_INT		3
#define MENU_TYPE_DIVIDE	4
#define MENU_TYPE_FLAG		5
#define MENU_TYPE_SH_INT	6
#define MENU_TYPE_LONG		7

#define MENU_BOOL		"B%d"
#define MENU_CHAR		"C%c"
#define MENU_STRING		"S%s"
#define MENU_NUMBER		"I%d%d%d"
#define MENU_INT		"I%d%d%d"
#define MENU_SH_INT		"H%d%d%d"
#define MENU_LONG		"L%ld"
#define MENU_DIVIDE		"D%s"
#define MENU_FLAG		"F%ld%d"

/* Data for generating characters -- only used during generation */
struct gen_data
{
    GEN_DATA    *next;
    bool        valid;
    bool        skill_chosen[MAX_SKILL];
    bool        group_chosen[MAX_GROUP];
    int         points_chosen;
};



/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_BEER	 1

struct  liq_type
{
    char *      liq_name;
    char *      liq_color;
    sh_int      liq_affect[5];
};



/*
 * Extra description data for a room or object.
 */
struct  extra_descr_data
{
    EXTRA_DESCR_DATA *next;     /* Next in list                     */
    bool valid;
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct  obj_index_data
{
    OBJ_INDEX_DATA *    next;
    EXTRA_DESCR_DATA *  extra_descr;
    AFFECT_DATA *       affected;
    bool                new_format;
    char *              name;
    char *              short_descr;
    char *              description;
    sh_int              vnum;
    sh_int              reset_num;
    char *              material;
    sh_int              item_type;
    sh_int		size;
    int			perm_spell;
    int                 extra_flags;
    int                 extra_flags2;
    int                 extra_flags3;
    int                 wear_flags;
    sh_int              level;
    sh_int              condition;
    sh_int              count;
    sh_int              weight;
    int                 cost;
    int                 value[10];
    bool                trig[NUM_ITEM_TRIG];
    char *              prog[NUM_ITEM_TRIG][IP_MAX_LINES];
    int                 spec_proc;
    int                 artifact;
};



/*
 * One object.
 */
struct  obj_data
{
    OBJ_DATA *          next;
    OBJ_DATA *          next_content;
    OBJ_DATA *          contains;
    OBJ_DATA *          in_obj;
    OBJ_DATA *          on;
    CHAR_DATA *		animated;
    CHAR_DATA *         carried_by;
    CHAR_DATA *		sw_char;
    EXTRA_DESCR_DATA *  extra_descr;
    AFFECT_DATA *       affected;
    OBJ_INDEX_DATA *    pIndexData;
    ROOM_INDEX_DATA *   in_room;
    MOB_INDEX_DATA *	corpse;
    int			altitude;
    bool                valid;
    bool                enchanted;
    char *              owner;
    char *              name;
    char *              short_descr;
    char *              description;
    sh_int              item_type;
    char *              last_owner; /* items + corpses!!! */
    char *              voodoo_name; /* heh heh heh */
    sh_int		size;
    int                 extra_flags;
    int                 extra_flags2;
    int                 extra_flags3;
    int                 wear_flags;
    int                 artifact;
    int 		perm_spell;
    sh_int              wear_loc;
    sh_int              weight;
    int                 cost;
    sh_int              level;
    sh_int              condition;
    char *              material;
    sh_int              timer;
    int                 value   [10];
    bool                paused_trig[NUM_ITEM_TRIG];
    struct delay_item_trig_data pause_trig_data[NUM_ITEM_TRIG];
    int                 spec_proc;
};



/*
 * Exit data.
 */
struct  exit_data
{
    union
    {
	ROOM_INDEX_DATA *       to_room;
	sh_int                  vnum;
    } u1;
    int                 exit_info;
    int                 key;
    char *              keyword;
    char *              description;
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct  reset_data
{
    RESET_DATA *        next;
    char                command;
    sh_int              arg1;
    sh_int              arg2;
    sh_int              arg3;
    sh_int              arg4;
};



/*
 * Area definition.
 */
struct  area_data
{
    AREA_DATA *         next;
    RESET_DATA *        reset_first;
    RESET_DATA *        reset_last;
    char *              file_name;
    char *              name;
    char *              credits;
    sh_int              age;
    sh_int              nplayer;
    sh_int              low_range;
    sh_int              high_range;
    sh_int              min_vnum;
    sh_int              max_vnum;
    REALM_DATA *	realm;
    MOB_PROG_DATA *	trig[NUM_TRIGS];
    AUTO_LOOK_DATA *	auto_mesg_indoors;
    AUTO_LOOK_DATA *	auto_mesg_outside;
    AUTO_LOOK_DATA *	auto_mesg_outside_night;
    AUTO_LOOK_DATA *	auto_mesg_indoors_night;
    bool                empty;
    int			repop_status;
    int			clan;
    SHOP_DATA *		shop_list;
};

#define			REPOP_NEVER		0
#define			REPOP_DONE		1
#define			REPOP_NEEDED		2
#define			REPOP_HOUSING_NEEDED	3
#define			REPOP_HOUSING_DONE	4



/************************************************************************
* REALMS DATA TABLES AND FLAGS                                          *
************************************************************************/

struct realm_data
{
    REALM_DATA *	next;
    char * 		name;
    char *		description;
    long		flags;
    int			start_room;
    int			vnum;
};


/*
 *
 * Realm flags
 *
 */

#define			REALM_ESCAPABLE		(A)
#define			REALM_RESET_ON_DEATH	(B)




/*
 *
 * Auto look for areas
 *
 */

struct	auto_look_data
{
    char *		text;
    AUTO_LOOK_DATA *	next;
};


/*
 * Room type.
 */
struct  room_index_data
{
    ROOM_INDEX_DATA *   next;
    ROOM_INDEX_DATA *   next_room;
    CHAR_DATA *         people;
    OBJ_DATA *          contents;
    EXTRA_DESCR_DATA *  extra_descr;
    AUTO_LOOK_DATA *	auto_look;
    AREA_DATA *         area;
    EXIT_DATA *         exit    [6];
    EXIT_DATA *         old_exit[6];
    char *              name;
    char *		false_sect_type;
    char *              description;
    char *              night_description;
    char *              owner;
    char *		alarm;
    sh_int              vnum;
    int                 room_flags;
    int                 room_flags2;
    sh_int              light;
    sh_int              sector_type;
    sh_int              heal_rate;
    sh_int              clan;
    char *              track[3];
    char *              trackdir[3];
    long		copper;
    long		silver;
    long		gold;
    long		emeralds;
    long		sapphires;
    long		diamonds;
    sh_int		gore;
    sh_int		tick_timer[MAX_RTIMER];
    long		games;
    ROOM_INDEX_DATA *	far_link;
};




/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_2ND_WEAPON		     -2
#define TYPE_3RD_WEAPON		     -3
#define TYPE_4TH_WEAPON		     -4
#define TYPE_HIT                     1000



/*
 *  Target types.
 */
#define TAR_IGNORE                  0
#define TAR_CHAR_OFFENSIVE          1
#define TAR_CHAR_DEFENSIVE          2
#define TAR_CHAR_SELF               3
#define TAR_OBJ_INV                 4
#define TAR_OBJ_CHAR_DEF            5
#define TAR_OBJ_CHAR_OFF            6
#define TAR_OBJ_ROOM		    7
#define TAR_EXIT		    8
#define TAR_CHAR_WORLD		    9

#define TARGET_CHAR                 0
#define TARGET_OBJ                  1
#define TARGET_ROOM                 2
#define TARGET_NONE                 3



/*
 * Skills include spells as a particular case.
 */
struct  skill_type
{
    char *      name;                   /* Name of skill                */
    sh_int      skill_level[4];         /* Level needed by class        */
    sh_int      rating[4];              /* How hard it is to learn      */      
    SPELL_FUN * spell_fun;              /* Spell pointer (for spells)   */
    sh_int      target;                 /* Legal targets                */
    sh_int      minimum_position;       /* Position for caster / user   */
    sh_int *    pgsn;                   /* Pointer to associated gsn    */
    sh_int      slot;                   /* Slot for #OBJECT loading     */
    sh_int      min_mana;               /* Minimum mana used            */
    sh_int      beats;                  /* Waiting time after use       */
    char *      noun_damage;            /* Damage message               */
    char *      msg_off;                /* Wear off message             */
    char *      msg_obj;                /* Wear off message for obects  */
    char *      material_comp;          /* Adam is great!!!!!           */
    sh_int      realm;                  /* -"--------------"-           */
    sh_int      per_used;               /* -"--------------"-           */
    sh_int      level;                  /* -"--------------"-           */
    char *      rev_name;               /* The reversed spell function  */
};

struct  skilltree_type
{
    sh_int * 	pgsn;			/* Pointer to associated gsn */
    int		level1;			/* top level of skill */
    int 	level2;			/* semi level of skill */
};

struct psi_type
{
    char *      name;                   /* ability name                 */
    PSI_FUN *   func;                   /* handling procedure           */
    sh_int      target;                 /* legal targets                */
    sh_int      init;                   /* initialisation psi cost      */
    sh_int      type;                   /* Upkeep type                  */
    sh_int      upkeep;                 /* Upkeep psi cost              */
    sh_int      beats;                  /* Delay for using ability      */
    sh_int      discipline;             /* Discipline of Use            */
    sh_int      level;                  /* Level of ability             */
    sh_int      minimum_position;       /* Position for caster / user   */
    char *	pre_req;		/* prerequsit power name	*/
    bool	science;		/* true = science, false = devotion */
};


/* disciplines */
#define PSI_PSYCHOMETABOLISM    1       /* altering self                */
#define PSI_PSYCHOKINESIS       2       /* altering around self         */
#define PSI_TELEPATHY           3       /* mind control                 */
#define PSI_METAPSIONICS        4       /* misc                         */
#define PSI_PSYCHOPORTATION     5       /* guess you miserable git      */
#define PSI_CLAIRSENTIENCE      6       /* guess you miserable git      */
#define PSI_ANCIENTS		7	/* Ancient powers held by only the top */
/* new psionic flags */
#define PSI_IMMUNE              (A)

/* psionic upkeep type */
#define PSI_ONCE                1
#define PSI_FAST                2
#define PSI_SLOW                3
#define PSI_PROGRESSIVE_SLOW    4
#define PSI_PROGRESSIVE_FAST    5

struct  group_type
{
    char *      name;
    sh_int      rating[4];
    char *      spells[MAX_IN_GROUP];
};



/*
 * These are skill_lookup return values for common skills and spells.
 */
extern  sh_int  gsn_retribution;
extern  sh_int  gsn_blindfighting;
extern  sh_int  gsn_backstab;
extern  sh_int  gsn_blacksmith;
extern  sh_int  gsn_dagger_throwing;
extern  sh_int  gsn_foraging;
extern  sh_int  gsn_elvish;
extern  sh_int  gsn_gnome;
extern  sh_int  gsn_demon;
extern  sh_int  gsn_common;
extern  sh_int  gsn_dodge;
extern  sh_int  gsn_envenom;
extern  sh_int  gsn_hide;
extern  sh_int  gsn_peek;
extern  sh_int  gsn_pick_lock;
extern  sh_int  gsn_sneak;
extern  sh_int  gsn_off_hand;
extern  sh_int  gsn_tumbling;
extern  sh_int  gsn_steal;

extern  sh_int  gsn_disarm;
extern  sh_int  gsn_enhanced_damage;
extern  sh_int  gsn_kick;
extern  sh_int  gsn_parry;
extern  sh_int  gsn_rescue;
extern  sh_int  gsn_second_attack;
extern  sh_int  gsn_third_attack;

extern  sh_int  gsn_blindness;
extern  sh_int  gsn_charm_person;
extern  sh_int  gsn_curse;
extern  sh_int  gsn_invis;
extern  sh_int  gsn_improved_invis;
extern  sh_int  gsn_mass_invis;
extern  sh_int  gsn_plague;
extern  sh_int  gsn_poison;
extern  sh_int  gsn_jump;
extern  sh_int  gsn_sleep;
extern  sh_int  gsn_fly;
extern  sh_int  gsn_sanctuary;
extern  sh_int  gsn_conflaguration;
extern  sh_int  gsn_fear;
extern  sh_int  gsn_mummy_rot;
extern  sh_int  gsn_blood_spew;
extern  sh_int  gsn_shiver;
extern  sh_int  gsn_holdperson;
extern  sh_int  gsn_pestilence;

/* new gsns */
extern sh_int  gsn_axe;
extern sh_int  gsn_dagger;
extern sh_int  gsn_flail;
extern sh_int  gsn_mace;
extern sh_int  gsn_polearm;
extern sh_int  gsn_shield_block;
extern sh_int  gsn_spear;
extern sh_int  gsn_sword;
extern sh_int  gsn_whip;
 
extern sh_int  gsn_bash;
extern sh_int  gsn_berserk;
extern sh_int  gsn_dirt;
extern sh_int  gsn_hand_to_hand;
extern sh_int  gsn_trip;
 
extern sh_int  gsn_fast_healing;
extern sh_int  gsn_haggle;
extern sh_int  gsn_lore;
extern sh_int  gsn_meditation;
 
extern sh_int  gsn_scrolls;
extern sh_int  gsn_staves;
extern sh_int  gsn_wands;
/* gsns that may not exist */
extern sh_int  gsn_air_barrier;
extern sh_int  gsn_appraisal;
extern sh_int  gsn_repair;
extern sh_int  gsn_archery;
extern sh_int  gsn_throwing;
extern sh_int  gsn_time_sight;



/*
 * Utility macros.
 */
#define IS_DAY()	    ((weather_info.sunlight == SUN_RISE \
			      || weather_info.sunlight == SUN_LIGHT))

#define IS_NIGHT()	    ((!IS_DAY()))

#define CAN_BUILD(ch)	    (IS_SET( (ch)->flag6, AFF6_CAN_BUILD ) || ch->level == 120 )

#define CAN_BUILD_ROOM(ch)  (CAN_BUILD((ch)) || is_clan_leader((ch)))

#define IS_IN_WATER(ch)     ((ch)->in_room->sector_type == SECT_UNDERWATER || \
			     (ch)->in_room->sector_type == SECT_WATER_SWIM || \
			     (ch)->in_room->sector_type == SECT_WATER_NOSWIM)
		
#define SS(stage)		debug_stage = (stage)


#define NO_NPC(ch)		if (IS_NPC((ch))) {stc("Player Only.\n\r", (ch));return;}
#define CHECK_MENU(ch)		if (IS_NPC((ch)) || (!IS_ROOM_STAT((ch)->in_room, ROOM_SAFE) && (ch)->level < 112)) \
					 {stc("You may only use menus in a safe room.\n\r", (ch));return;}

#define IS_INSIDE(ch)		(((ch)->in_room->sector_type == SECT_INSIDE) \
			      || ((ch)->in_room->sector_type == SECT_NARROW_TUNNEL) \
			      || (IS_SET((ch)->in_room->room_flags, ROOM_INDOORS)))

#define ROOM_INSIDE(room)	(((room)->sector_type == SECT_INSIDE) \
			      || ((room)->sector_type == SECT_NARROW_TUNNEL) \
			      || (IS_SET((room)->room_flags, ROOM_INDOORS)))

#define IS_VALID(data)          ((data) != NULL && (data)->valid)
#define VALIDATE(data)          ((data)->valid = TRUE)
#define INVALIDATE(data)        ((data)->valid = FALSE)
#define UMIN(a, b)              ((a) < (b) ? (a) : (b))
#define UMAX(a, b)              ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)         ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)                ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)                ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)       ((flag) & (bit))
#define SET_BIT(var, bit)       ((var) |= (bit))
#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))

#define IS_ANSI(ch)		(!IS_NPC( (ch) ) && IS_SET((ch)->comm, COMM_ANSI))

#define IS_SAME_ZONE(ch, vch)   ((ch)->in_room->area == (vch)->in_room->area)

#define WRITE_STR(field, value); free_string(field); field = str_dup( value );

#define IS_INSUBSTANTIAL(ch)    ((ch)->cur_form == FRM_MIST	\
				  || (ch)->cur_form == FRM_CRIMSON \
				  || (ch)->cur_form == FRM_WHIRLWIND \
				  || IS_FLAG5( (ch), AFF5_INSUBSTANTIAL))

#define IS_RESISTING(ch)	( IS_SET((ch)->flag4, AFF_GEN_RESIST))
/*
 * Character macros.
 */

#define IS_BASE_FORM(ch)	(((ch)->cur_form >= 38 && (ch)->cur_form <= 54) || ((ch)->cur_form >=24 && (ch)->cur_form <= 36) || (ch)->cur_form == 16 || (ch)->cur_form == FRM_PIXIE || (ch)->cur_form == (FRM_PIXIE + 1) )
#define GOT_INNATE(ch)		((ch) != NULL && !IS_NPC((ch)) && (ch)->pcdata != NULL)
#define CAN_INNATE(ch, sl)      ((ch)->pcdata->innate[(sl)] > 0)
#define TOTAL_WORTH(ch)		((ch)->diamonds * CNV_DIA + (ch)->sapphires * CNV_SAP + (ch)->emeralds * CNV_EME + (ch)->gold * CNV_GOL + (ch)->silver * CNV_SIL + (ch)->copper * CNV_COP)
#define ADD_INNATE(ch, sl, max, regain) \
		{ (ch)->pcdata->innate_max[(sl)] = (max); \
		(ch)->pcdata->innate_regain[(sl)] = (regain);\
		(ch)->pcdata->innate_timer[(sl)] = ((ch)->pcdata->innate_timer[(sl)] == 0) ? (ch)->pcdata->innate_regain[(sl)] : (ch)->pcdata->innate_timer[(sl)];}
#define USE_INNATE(ch, sl)	(ch)->pcdata->innate[(sl)] -= 1; \
		(ch)->pcdata->innate_timer[(sl)] = ((ch)->pcdata->innate_timer[(sl)] == 0) ? (ch)->pcdata->innate_regain[(sl)] : (ch)->pcdata->innate_timer[(sl)]
#define BODY_PART(ch, prt)	(IS_SET((ch)->body_form, (prt)))
//#define IS_NPC(ch)              (IS_SET((ch)->act, ACT_IS_NPC))
#define IS_NPC(ch)              ((ch)->pcdata == NULL)
#define IS_IMMORTAL(ch)         (get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)             (get_trust(ch) >= LEVEL_HERO)
#define IS_IMPLEMENTOR(ch)	(get_trust(ch) == MAX_LEVEL)
#define IS_CREATOR(ch)		IS_IMPLEMENTOR((ch))
#define IS_TRUSTED(ch,level)    (get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)     (IS_SET((ch)->affected_by, (sn)))
#define IS_NEWFLAG(ch, sn)      (IS_SET((ch)->flag2, (sn)))
#define IS_FLAG2(ch, sn)        (IS_SET((ch)->flag2, (sn)))
#define IS_FLAG3(ch, sn)        (IS_SET((ch)->flag3, (sn)))
#define IS_FLAG4(ch, sn)        (IS_SET((ch)->flag4, (sn)))
#define IS_FLAG5(ch, sn)        (IS_SET((ch)->flag5, (sn)))
#define IS_FLAG6(ch, sn)        (IS_SET((ch)->flag6, (sn)))
#define IS_SPEC(ch, sn)         (IS_SET((ch)->mob_spec, (sn)))
#define IS_QUEST(ch, sn)        (IS_SET((ch)->pcdata->quest_status, (sn)))
#define GET_NAME(ch)            (IS_NPC((ch)) ? (ch)->short_descr : (ch)->name)
#define GET_PROPER_NAME(ch)     (IS_NPC((ch)) ? (ch)->short_descr : (ch)->pcdata->switchname)
#define GET_PC_NAME(ch)     	(IS_NPC((ch)) ? "<npc>" : (ch)->pcdata->switchname)

#define IS_GRANTED(ch, sn)      (IS_SET((ch)->grant_cmds, (sn)) \
				    || IS_FLAG3((ch), AFF_MOB_PROG_GRANT) \
				    || IS_FLAG3((ch), AFF_MOB_PROG_NOSHOW))

#define IS_COOL(ch)		((ch)->level == 120)
#define SET_GRANT(ch, sn)       (SET_BIT((ch)->grant_cmds, (sn)))
#define REMOVE_GRANT(ch,sn)     (REMOVE_BIT((ch)->grant_cmds, (sn)))
#define TOGGLE_GRANT(ch,sn)     (IS_GRANTED((ch),(sn)) ? REMOVE_GRANT((ch),(sn)) : SET_GRANT((ch),(sn)))

#define GET_FORM(ch)            				\
	((form_data[(ch)->cur_form].short_desc == NULL || 	\
	form_data[(ch)->cur_form].short_desc[0] == '\0') ? 	\
		form_data[(ch)->cur_form].name : "")

#define GET_AGE(ch)             ((int) (16 + ((ch)->played \
				    + current_time - (ch)->logon )/72000))

#define IS_GOOD(ch)             (ch->alignment >= 350)
#define IS_EVIL(ch)             (ch->alignment <= -350)
#define IS_NEUTRAL(ch)          (!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)            (ch->position > POS_SLEEPING)

#define IS_OUTSIDE(ch)          (!IS_INSIDE((ch)))

#define IS_TAINTED(ch)		(!TIME_UP((ch), TIMER_TAINT))

#define WAIT_STATE(ch, npulse)  ((ch)->wait =  \
		((ch)->level == 120) ? 0 : UMAX((ch)->wait, (npulse)) \
		/ ((!TIME_UP((ch), TIMER_ON_SPEED)) ? 2 : 1))
#define DAZE_STATE(ch, npulse)  ((ch)->daze = UMAX((ch)->daze, (npulse)))
#define get_carry_weight(ch)    ((ch)->carry_weight + (ch)->silver/10 +  \
						      (ch)->gold/10  +  \
						      (ch)->copper/10  +  \
						      (ch)->diamonds  +  \
						      (ch)->emeralds  +  \
						      (ch)->sapphires)


/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)     (IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)  (IS_SET((obj)->extra_flags, (stat)))
#define IS_OBJ_STAT2(obj, stat) (IS_SET((obj)->extra_flags2,(stat)))
#define IS_OBJ_STAT3(obj, stat) (IS_SET((obj)->extra_flags3,(stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj)        ((obj)->item_type == ITEM_CONTAINER ? \
	(obj)->value[4] : 100)


/*
 * Room macros.
 */
#define IS_ROOM_STAT(room, stat)  (IS_SET((room)->room_flags, (stat)))
#define IS_ROOM_STAT2(room, stat) (IS_SET((room)->room_flags2,(stat)))


/*
 * Description macros.
 */
#define PERS(ch, looker)        (can_see(looker,(ch) ) ? ( (IS_NPC(ch) ? (ch)->short_descr :(ch)->name) ) : "something" )


/*
 * Structure for a social in the socials table.
 */
struct  social_type
{
    char      name[20];
    char *    char_no_arg;
    char *    others_no_arg;
    char *    char_found;
    char *    others_found;
    char *    vict_found;
    char *    char_not_found;
    char *      char_auto;
    char *      others_auto;
};



/*
 * Global constants.
 */
extern  const   struct  str_app_type    str_app         [31];
extern  const   struct  int_app_type    int_app         [31];
extern  const   struct  wis_app_type    wis_app         [31];
extern  const   struct  dex_app_type    dex_app         [31];
extern  const   struct  con_app_type    con_app         [31];
extern  const   struct  cha_app_type    cha_app         [31];

extern  char    * const where_name                              [][3];
extern  const   struct  weapon_type     weapon_table    [];
extern  const   struct  item_type       item_table      [];
extern  const   struct  wiznet_type     wiznet_table    [];
extern  const   struct  attack_type     attack_table    [];
extern  const   struct  race_type       race_table      [];
extern  const   struct  form_type       form_data       [MAX_FORMS];
extern  const   struct  stance_type     stances         [12];
extern  const   struct  mage_abil_type  magabil_table   [11][6];
extern  const   char    *               lang_name       [NUM_LANGUAGES];
extern  const   struct  worship_type    worship_table   [];
extern  const   struct  pc_race_type    pc_race_table   [];
extern  const   struct  spec_type       spec_table      [];
extern  const   struct  liq_type        liq_table       [];
extern  const	char	*		trigger		[NUM_TRIGS];
extern  const   struct  skill_type      skill_table     [MAX_SKILL];
extern  const	struct	skilltree_type	skill_tree	[MAX_NON_SPELL_SKILLS];
extern  const   struct  psi_type        psi_table       [NUM_PSI_SLOTS];
extern  const   struct  group_type      group_table     [MAX_GROUP];
extern          struct  social_type      social_table    [MAX_SOCIALS];
extern  const   char    *		discipline	[MAX_DISCIPLINES];
extern	const	char	*		dir_name	[6];
extern	const	char	*		colour_name	[8];
extern	const	char	*		score_word	[30];
extern	const	struct	form_type	form_data	[MAX_FORMS];

/*
 * Global variables.
 */
extern		CLAN_DATA		clan_data[MAX_CLAN];
extern          HELP_DATA         *     help_first;
extern          MANUAL_DATA       *     manual_first;
extern          SHOP_DATA         *     shop_first;

extern		int			count_down;
extern		char		  *	count_down_to;

extern		int			rb_timer;
extern		ROOM_INDEX_DATA	  *	rb_room;
extern		char		  *	rb_desc;
extern		CHAR_DATA	  *	rb_mob;
extern		CHAR_DATA	  *	rb_buildfor;

extern		int			debug_counter;
extern		char		  *	debug_procedure[20];
extern		char		  *	debug_last_character;
extern		int			debug_last_room;
extern		int			debug_stage;
extern		char		  *	debug_last_command;
extern		bool			debug_dont_run;
extern		int			hits;
extern		int			highhits;

extern		REALM_DATA	  *	realm_list;
extern		REALM_DATA	  *	prime_realm;
extern          ROOM_INDEX_DATA   *     room_list;
extern          CHAR_DATA         *     char_list;
extern          PROG_DATA         *     prog_list;
extern          MOB_PROG_DATA     *     mob_prog_list;
extern		BALANCE_DATA	  	balance_info[7];
extern		CONFIG_DATA	  *	config_info;
extern		SITE_DATA	  *	site_list;
extern		EMAIL_DATA	  *	email_list;
extern		EMAIL_DATA	  *	emailing_to;
extern          DESCRIPTOR_DATA   *     descriptor_list;
extern          OBJ_DATA          *     object_list;

extern          char                    bug_buf         [];
extern          time_t                  current_time;
extern          bool                    fLogAll;
extern		int			weird_weather_type;
extern		int			weird_weather_timer;

extern          FILE *                  fpReserve;
extern          KILL_DATA               kill_table      [];
extern          char                    log_buf         [];
extern          TIME_INFO_DATA          time_info;
extern          WEATHER_DATA            weather_info;
extern		struct	league_table_type league_table	[MAX_LEAGUE+1];

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if     defined(_AIX)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(apollo)
int     atoi            args( ( const char *string ) );
void *  calloc          args( ( unsigned nelem, size_t size ) );
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(hpux)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(linux)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(riscbsd)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(macintosh)
#define NOCRYPT
#if     defined(unix)
#undef  unix
#endif
#endif

#if     defined(MIPS_OS)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(MSDOS)
#define NOCRYPT
#if     defined(unix)
#undef  unix
#endif
#endif

#if     defined(NeXT)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(sequent)
char *  crypt           args( ( const char *key, const char *salt ) );
int     fclose          args( ( FILE *stream ) );
int     fprintf         args( ( FILE *stream, const char *format, ... ) );
/*int   fread           args( ( void *ptr, int size, int n, FILE *stream ) );*/
int     fseek           args( ( FILE *stream, long offset, int ptrname ) );
void    perror          args( ( const char *s ) );
int     ungetc          args( ( int c, FILE *stream ) );
#endif

#if     defined(sun)
char *  crypt           args( ( const char *key, const char *salt ) );
int     fclose          args( ( FILE *stream ) );
int     fprintf         args( ( FILE *stream, const char *format, ... ) );
#if     defined(SYSV)
/*siz_t fread           args( ( void *ptr, size_t size, size_t n, */
			    FILE *stream) );
#else
/*int   fread           args( ( void *ptr, int size, int n, FILE *stream ) );*/
#endif
int     fseek           args( ( FILE *stream, long offset, int ptrname ) );
void    perror          args( ( const char *s ) );
int     ungetc          args( ( int c, FILE *stream ) );
#endif

#if     defined(ultrix)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#undef crypt
#define NOCRYPT

/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if     defined(NOCRYPT)
#define crypt(s1, s2)   (s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(macintosh)
#define PLAYER_DIR      ""                      /* Player files */
#define TEMP_FILE       "romtmp"
#define NULL_FILE       "proto.are"             /* To reserve one stream */
#endif

#if defined(MSDOS)
#define PLAYER_DIR      ""                      /* Player files */
#define TEMP_FILE       "romtmp"
#define NULL_FILE       "nul"                   /* To reserve one stream */
#endif

#if defined(unix)
#define PLAYER_DIR      "../player/"            /* Player files */
#define GOD_DIR         "../gods/"              /* list of gods */
#define TEMP_FILE       "../player/romtmp"
#define NULL_FILE       "/dev/null"             /* To reserve one stream */
#endif

#define COPYOVER_FILE	"desc.data"
#define EXEC_FILE	"btl"
#define AREA_LIST       "area.lst"  /* List of areas*/
#define BUG_FILE        "bugs.txt" /* For 'bug' and bug()*/
#define TYPO_FILE       "typos.txt" /* For 'typo'*/
#define NOTE_FILE       "notes.not"/* For 'notes'*/
#define IDEA_FILE       "ideas.not"
#define PENALTY_FILE    "penal.not"
#define NEWS_FILE       "news.not"
#define CHANGES_FILE    "chang.not"
#define SHUTDOWN_FILE   "shutdown.txt"/* For 'shutdown'*/
#define BAN_FILE        "ban.txt"
#define MUSIC_FILE      "music.txt"



/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD      CHAR_DATA
#define MID     MOB_INDEX_DATA
#define OD      OBJ_DATA
#define OID     OBJ_INDEX_DATA
#define RID     ROOM_INDEX_DATA
#define AD      AFFECT_DATA

/* act_comm.c */
void    check_sex       args( ( CHAR_DATA *ch) );
void    add_follower    args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void    stop_follower   args( ( CHAR_DATA *ch ) );
void    nuke_pets       args( ( CHAR_DATA *ch ) );
void    die_follower    args( ( CHAR_DATA *ch ) );
bool    is_same_group   args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
void    informational   args( ( char *argument, int race, int level ) );
int	col_str_len	args( ( char *txt ) );

/* act_enter.c */
RID  *get_random_room   args ( (CHAR_DATA *ch) );
RID  *get_rand_room   args ( ( ) );
RID  *get_treemeld_room   args ( ( ) );
RID  *get_rand_room_by_sect   args ( (int sect ) );
void make_wall          args( (ROOM_INDEX_DATA *room, int dir, int wall) );
void put_wall           args( (ROOM_INDEX_DATA *room, int dir, int wall) );

/* voodoo.c */
bool    check_social    args( ( CHAR_DATA *ch, char *command,
                            char *argument ) );
int voodoo_level	args( (CHAR_DATA *ch) );

/* act_info.c */
bool	can_see_name	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	name_prefix_to_char args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    set_pc_name     args( ( CHAR_DATA *ch, char *title ) );
void    set_switchname  args( ( CHAR_DATA *ch, char *title ) );
void    set_title       args( ( CHAR_DATA *ch, char *title ) );
void    set_title_prefix     args( ( CHAR_DATA *ch, char *title ) );
void    look_name	args( ( CHAR_DATA *ch ) );
void    set_enter_room_text args( ( CHAR_DATA *ch, char *argument ) );
void    set_form        args( ( CHAR_DATA *ch , sh_int form ) );
bool    set_stance      args( ( CHAR_DATA *ch , int stance ) );
bool    able_stance     args( ( CHAR_DATA *ch , int stance ) );
void    improve_stance  args( ( CHAR_DATA *ch , int stance ) );
void    banner_to_char  args( ( char *txt, CHAR_DATA *ch ) );
void    banner2_to_char args( ( char *txt, CHAR_DATA *ch ) );
void    divide_to_char  args( ( CHAR_DATA *ch ) );
void    divide2_to_char args( ( CHAR_DATA *ch ) );
void    divide3_to_char args( ( CHAR_DATA *ch ) );
void    divide4_to_char args( ( CHAR_DATA *ch ) );
void    divide5_to_char args( ( CHAR_DATA *ch ) );
void    divide6_to_char args( ( CHAR_DATA *ch ) );
void    banner_to_buf   args( ( char *txt, BUFFER *out ) );
void    banner2_to_buf  args( ( char *txt, BUFFER *out ) );
void    divide_to_buf   args( ( BUFFER *out ) );
void    divide2_to_buf  args( ( BUFFER *out ) );
void    divide3_to_buf  args( ( BUFFER *out ) );
void    divide4_to_buf  args( ( BUFFER *out ) );
void    divide5_to_buf  args( ( BUFFER *out ) );
void    divide6_to_buf  args( ( BUFFER *out ) );
bool    change_upkeep   args( ( CHAR_DATA *ch , int change ) );
void    rand_to_char    args( ( CHAR_DATA *ch , char *str1,char *str2,char *str3,char *str4,char *str5 ) );
bool    is_next_room    args( ( CHAR_DATA *ch, CHAR_DATA *vch) );
int  	get_size	args( (CHAR_DATA *ch) );
bool    is_vowel	args( (char arg) );
int	souls_needed	args( (CHAR_DATA *ch) );

/* act_move.c */
char *  get_move_message args( (CHAR_DATA *ch ) );
void    move_char       args( ( CHAR_DATA *ch, int door, bool follow ) );
void    check_fall      args( ( CHAR_DATA *ch ) );
void	make_unconcious    args( ( CHAR_DATA *ch, int tim) );
bool    can_pass_wall	args ( (CHAR_DATA *ch, EXIT_DATA *pexit, int dir) );
bool	is_flying	args ( (CHAR_DATA *ch) );
bool	is_default_flying args ( (CHAR_DATA *ch) );
bool    can_go_up	args( (CHAR_DATA *ch) );
bool    can_fly		args( (CHAR_DATA *ch) );
int	max_vamp_disc_level args( (CHAR_DATA *ch) );

/* act_obj.c */
void    recalc_wear     args( (CHAR_DATA *ch) );
int	calc_hands_needed args( (CHAR_DATA *ch, OBJ_DATA *obj) );
OBJ_DATA * wither_arm	args( (CHAR_DATA *ch) );
void	display_clatter args( (CHAR_DATA *ch, OBJ_DATA *obj) );
bool 	can_loot        args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void    get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj,
				OBJ_DATA *container ) );
void     make_item      args( (CHAR_DATA *ch, int vnum, int equip) );
void     outfit         args( (CHAR_DATA *ch) );
bool    remove_obj      args( (CHAR_DATA *ch, int iWear, bool fReplace ) );
bool	has_got_money	args( (CHAR_DATA *ch, char *arg, int amount) );


/* act_wiz.c */
bool is_mob_switched	args( (CHAR_DATA *ch) );
void wiznet             args( (char *string, CHAR_DATA *ch, OBJ_DATA *obj,
			       long flag, long flag_skip, int min_level ) );
void obj_return 	args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void obj_switch 	args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void restore_char 	args( (CHAR_DATA *ch) );
void gecho		args( (char *message) );
void gecho_outside	args( (char *message) );

/* misc.c */
void    substitute_alias args( (DESCRIPTOR_DATA *d, char *input) );

/* system.c */
void copyover_recover   args( ( ) );
void recover_descriptor args( ( DESCRIPTOR_DATA *dnew, int desc ) );
void menu_init		args( ( CHAR_DATA *ch ) );
void menu_init_flags	args( ( CHAR_DATA *ch, long *flg ) );
void menu_option	args( ( CHAR_DATA *ch, int line, char *label, void **var, const char *type, ...) );
void menu_divider	args( ( CHAR_DATA *ch, int line) );
void do_menu		args( ( CHAR_DATA *ch, char *excom, char *title ) );
void set_alarm		args( ( int secs ) );
void stcf		args( (CHAR_DATA *ch, const char *fmt, ...));
void console		args( (const char *fmt, ...));
void redraw_cursor	args( ( CHAR_DATA *ch ) );
void redraw_character	args( ( CHAR_DATA *ch ) );
void char_to_game	args( ( CHAR_DATA *ch ) );
void char_from_game	args( ( CHAR_DATA *ch, int con ) );
void init_signals	args( ( void ) );
void editor		args( ( DESCRIPTOR_DATA *d ) );
void do_edit		args( ( CHAR_DATA *ch, char **editing, char *edit_excom, char *title ) );
void manual_edit	args( ( CHAR_DATA *ch, char *edit_excom, char *title ) );
void redraw_editor	args( ( DESCRIPTOR_DATA *d ) );
void editor_title	args( ( char *txt, CHAR_DATA *ch ) );
void editor_mesg	args( ( char *txt, CHAR_DATA *ch ) );
void email_next		args( ( ) );

/* comm.c */
void	boot_screen	args( ( int control, int pos, int size ) );
void    edit_mode_to_char args( ( CHAR_DATA *ch ) );
void    norm_mode_to_char args( ( CHAR_DATA *ch ) );
void    ctrl_mode_to_char args( ( CHAR_DATA *ch ) );
void    echo_off_to_char args( ( CHAR_DATA *ch ) );
void    echo_on_to_char args( ( CHAR_DATA *ch ) );
void    show_string     args( ( struct descriptor_data *d, char *input) );
void    close_socket    args( ( DESCRIPTOR_DATA *dclose ) );
void    write_to_buffer args( ( DESCRIPTOR_DATA *d, const char *txt,
			    int length ) );
bool    write_to_descriptor args( ( int desc, char *txt,
			    int length ) );
void    send_to_char    args( ( const char *txt, CHAR_DATA *ch ) );
void    stc	        args( ( const char *txt, CHAR_DATA *ch ) );
void    cent_to_char    args( ( char *txt, CHAR_DATA *ch ) );
void    cent_to_buf     args( ( char *txt, BUFFER *out ) );
void    ctc 		args( ( char *txt, CHAR_DATA *ch ) );
void    page_to_char    args( ( const char *txt, CHAR_DATA *ch ) );
void    act             args( ( const char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type ) );
void    act_new         args( ( const char *format, CHAR_DATA *ch, 
			    const void *arg1, const void *arg2, int type,
			    int min_pos) );
bool    check_ban       args( ( char *site, int type) );

/* core.c */
void    smash_bauble	args( ( CHAR_DATA *ch, OBJ_DATA *obj) );
DECLARE_DO_FUN( do_manual       );
char * banner_name      args( (char * bname) );
void   banner_buffer    args( (char * bname, char *buf) );
void do_autowalk	args( ( DESCRIPTOR_DATA *d ) );
void	conv_language   args( (char *argument, char *buf, int lang, CHAR_DATA *ch) );
void    show_vision	args( ( CHAR_DATA *ch ) );
void	wrap_to_char	args( ( char *argument, CHAR_DATA *ch ) );


/* db.c */
void    reset_area      args( ( AREA_DATA * pArea ) );
void    add_room	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
void    add_mob		args( ( MOB_INDEX_DATA *mob ) );
void    add_obj		args( ( OBJ_INDEX_DATA *obj ) );
char *  print_flags     args( ( int flag ));
void    boot_db         args( ( bool fCopyOver ) );
void    area_update     args( ( void ) );
CD *    create_mobile   args( ( MOB_INDEX_DATA *pMobIndex ) );
void    clone_mobile    args( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OD *    create_object   args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void    clone_object    args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
void    clear_char      args( ( CHAR_DATA *ch ) );
char *  get_extra_descr args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *   get_mob_index   args( ( int vnum ) );
OID *   get_obj_index   args( ( int vnum ) );
RID *   get_room_index  args( ( REALM_DATA *realm, int vnum ) );
char    fread_letter    args( ( FILE *fp ) );
int     fread_number    args( ( FILE *fp ) );
long    fread_flag      args( ( FILE *fp ) );
char *  fread_string    args( ( FILE *fp ) );
char *  fread_string_eol args(( FILE *fp ) );
void    fread_to_eol    args( ( FILE *fp ) );
char *  fread_word      args( ( FILE *fp ) );
long    flag_convert    args( ( char letter) );
void *  alloc_mem       args( ( int sMem ) );
void *  alloc_perm      args( ( int sMem ) );
void    free_mem        args( ( void *pMem, int sMem ) );
char *  str_dup         args( ( const char *str ) );
void    free_string     args( ( char *pstr ) );
int     number_fuzzy    args( ( int number ) );
int     number_range    args( ( int from, int to ) );
int     number_percent  args( ( void ) );
int     number_door     args( ( void ) );
int     number_bits     args( ( int width ) );
long     number_mm       args( ( void ) );
int     dice            args( ( int number, int size ) );
int     interpolate     args( ( int level, int value_00, int value_32 ) );
void    smash_tilde     args( ( char *str ) );
void    smash_misc      args( ( char *str, char cFind, char cRepl ) );
void    smash_space     args( ( char *str ) );
bool    str_cmp         args( ( const char *astr, const char *bstr ) );
bool    str_prefix      args( ( const char *astr, const char *bstr ) );
bool    str_infix       args( ( const char *astr, const char *bstr ) );
bool    str_suffix      args( ( const char *astr, const char *bstr ) );
char *  capitalize      args( ( const char *str ) );
void    append_file     args( ( CHAR_DATA *ch, char *file, char *str ) );
void    bug             args( ( const char *str, int param ) );
void    log_string      args( ( const char *str ) );
void logf (char * fmt, ...);
void    tail_chain      args( ( void ) );
char *  person          args( ( CHAR_DATA *ch, CHAR_DATA *looker ) );
MOB_PROG_DATA * get_prog_index args( ( char *argument ) );

/* effect.c */
void    acid_effect     args( (void *vo, int level, int dam, int target) );
void    cold_effect     args( (void *vo, int level, int dam, int target) );
void    fire_effect     args( (void *vo, int level, int dam, int target) );
void    poison_effect   args( (void *vo, int level, int dam, int target) );
void    shock_effect    args( (void *vo, int level, int dam, int target) );


/* fight.c */
void	teleport_artifacts args( ( CHAR_DATA *victim, OBJ_DATA *list ) );
bool    is_safe         args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    player_kill         args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
void    player_kill_update args( (CHAR_DATA *ch, CHAR_DATA *victim, bool combat ) );
bool    is_safe_spell   args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
void    violence_update args( ( void ) );
void    multi_hit       args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
bool    damage          args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				int dt, int class, bool show ) );
bool    damage_old      args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				int dt, int class, bool show ) );
void    update_pos      args( ( CHAR_DATA *victim ) );
void    stop_fighting   args( ( CHAR_DATA *ch, bool fBoth ) );
void    check_killer    args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
void    set_fighting    args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
void    destroy_char    args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool crp) );
void    death_move      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    group_gain      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    raw_kill        args( ( CHAR_DATA *victim ) );
bool 	can_kill	args( ( CHAR_DATA *ch, CHAR_DATA *victim) );

/* handler.c */
OD   *	get_random_armour args( ( CHAR_DATA *ch ) );
REALM_DATA * get_realm  args( ( char *argument ) );
REALM_DATA * get_by_number args( ( int rnum ) );
REALM_DATA * char_realm  args( ( CHAR_DATA *ch ) );
bool 	enough_money	args( ( CHAR_DATA *ch, long amount ) );
void	give_cash	args( ( CHAR_DATA *ch, CHAR_DATA *victim, long amount ) );
bool	arg_check	args( (char *arg1, char *arg2) );
bool	multi_check	args( (char *arg1, char *arg2) );
void    lose_obj	args( (CHAR_DATA *ch, OBJ_DATA *obj) );
OD   *  create_body_part args((CHAR_DATA *ch, char *argument) );
bool	lose_body_part	args( (CHAR_DATA *ch, int part) );
char *  color_value_string args( ( int color, bool bold, bool flash ) );
int     strlen_color    args( ( char *argument ) );
void	scroll_to_char 	args( ( char *text, CHAR_DATA *ch) );
char *  split_number_misc args( ( char *argument, char *arg_first, char cEnd, int num) );
char *  split_misc    	args( ( char *argument, char *arg_first, char cEnd) );
char *  split_misc2    	args( ( char *argument, char *arg_first, char cEnd) );
char *  split_hash    	args( ( char *argument, char *arg_first ) );
char *  split_semicolon	args( ( char *argument, char *arg_first ) );
AD      *affect_find 	args( (AFFECT_DATA *paf, int sn));
void    affect_check    args( (CHAR_DATA *ch, int where, int vector) );
int     count_users     args( (OBJ_DATA *obj) );
void    deduct_cost     args( (CHAR_DATA *ch, int cost) );
void    affect_enchant  args( (OBJ_DATA *obj) );
int     check_immune    args( (CHAR_DATA *ch, int dam_type) );
int     liq_lookup      args( ( const char *name) );
int     material_lookup args( ( const char *name) );
int     weapon_lookup   args( ( const char *name) );
int     weapon_type     args( ( const char *name) );
char    *weapon_name    args( ( int weapon_Type) );
int     item_lookup     args( ( const char *name) );
char    *item_name      args( ( int item_type) ); 
int     attack_lookup   args( ( const char *name) );
int     race_lookup     args( ( const char *name) );
long    wiznet_lookup   args( ( const char *name) );
int     class_lookup    args( ( const char *name) );
bool	is_obj_held	args( (	CHAR_DATA *ch, sh_int iType) );
bool    is_clan         args( (CHAR_DATA *ch) );
bool    is_same_clan    args( (CHAR_DATA *ch, CHAR_DATA *victim));
bool    is_same_secondaryrace args( (CHAR_DATA *ch, CHAR_DATA *victim));
bool    is_old_mob      args ( (CHAR_DATA *ch) );
int     get_skill       args( ( CHAR_DATA *ch, int sn ) );
int     get_weapon_sn_by_type args( ( OBJ_DATA *wield ) );
int     get_weapon_sn   args( ( CHAR_DATA *ch ) );
int     get_weapon_snsl args( ( CHAR_DATA *ch, int sl ) );
int     get_weapon_skill args(( CHAR_DATA *ch, int sn ) );
int     get_age         args( ( CHAR_DATA *ch ) );
void    reset_char      args( ( CHAR_DATA *ch )  );
int     get_trust       args( ( CHAR_DATA *ch ) );
int     get_curr_stat   args( ( CHAR_DATA *ch, int stat ) );
int     get_max_train   args( ( CHAR_DATA *ch, int stat ) );
int     can_carry_n     args( ( CHAR_DATA *ch ) );
int     can_carry_w     args( ( CHAR_DATA *ch ) );
bool    is_name         args( ( char *str, char *namelist ) );
bool    is_exact_name   args( ( char *str, char *namelist ) );
void    affect_name_to_char  args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_to_char  args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_to_obj   args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void    affect_remove   args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_remove_obj args( (OBJ_DATA *obj, AFFECT_DATA *paf ) );
void    affect_strip    args( ( CHAR_DATA *ch, int sn ) );
bool    is_affected     args( ( CHAR_DATA *ch, int sn ) );
void    affect_join     args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_name_join  args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    char_from_room  args( ( CHAR_DATA *ch ) );
void    char_to_room    args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex, int alt ) );
void    obj_to_char     args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    obj_from_char   args( ( OBJ_DATA *obj ) );
OD *	get_obj_by_name args( ( CHAR_DATA *ch, char *name) );
OD *	get_obj_by_location args( ( CHAR_DATA *ch, int wf) );
OD *	get_obj_by_type args( ( CHAR_DATA *ch, sh_int iType) );
OD *	get_obj_by_stat args( ( CHAR_DATA *ch, sh_int iType) );
OD *	get_obj_by_stat2 args( ( CHAR_DATA *ch, sh_int iType) );
OD *	get_obj_by_ntype args( ( CHAR_DATA *ch, sh_int iType, int cnt) );
int	count_obj_by_type args( ( CHAR_DATA *ch, sh_int iType) );
int     apply_ac        args( ( OBJ_DATA *obj, int iWear, int type ) );
OD *    get_eq_char     args( ( CHAR_DATA *ch, int iWear ) );
void    equip_char      args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void    unequip_char    args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int     count_obj_list  args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void    obj_from_room   args( ( OBJ_DATA *obj ) );
void    obj_to_room     args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex , int alt) );
void    obj_to_obj      args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void    obj_from_obj    args( ( OBJ_DATA *obj ) );
void    extract_obj     args( ( OBJ_DATA *obj ) );
void    extract_char    args( ( CHAR_DATA *ch, bool fPull ) );
CD *    get_char_room   args( ( CHAR_DATA *ch, char *argument ) );
CD *    get_char_exit   args( ( CHAR_DATA *ch,ROOM_INDEX_DATA *room, char *argument ) );
CD *    get_char_far    args( ( CHAR_DATA *ch, char *argument, bool anyone ) ); 
CD *    get_char_world  args( ( CHAR_DATA *ch, char *argument ) );
OD *    get_obj_type    args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *    get_obj_list    args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *    get_obj_carry   args( ( CHAR_DATA *ch, char *argument, 
			    CHAR_DATA *viewer ) );
OD *    get_obj_eq   args( ( CHAR_DATA *ch, char *argument, 
			    CHAR_DATA *viewer ) );
OD *    get_obj_wear    args( ( CHAR_DATA *ch, char *argument ) );
OD *    get_obj_here    args( ( CHAR_DATA *ch, char *argument ) );
OD *    get_obj_world   args( ( CHAR_DATA *ch, char *argument ) );
OD *    create_money    args( ( int gold, int silver ) );
int     get_obj_number  args( ( OBJ_DATA *obj ) );
int     get_obj_weight  args( ( OBJ_DATA *obj ) );
int     get_true_weight args( ( OBJ_DATA *obj ) );
bool    room_is_dark    args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool    is_room_owner   args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room) );
bool    room_is_private args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool    can_see_shadows args( ( CHAR_DATA *ch ) );
bool    can_see_invis   args( ( CHAR_DATA *ch ) );
bool    can_see_hidden  args( ( CHAR_DATA *ch ) );
bool    can_see_in_dark args( ( CHAR_DATA *ch ) );
bool    can_see         args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    can_see_obj     args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool    can_see_obj_no_alt args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool    can_see_room    args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool    can_drop_obj    args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *  affect_loc_name args( ( int location ) );
char *  affect_bit_name args( ( int vector ) );
char *  extra_bit_name  args( ( int extra_flags ) );
char *  wear_bit_name   args( ( int wear_flags ) );
char *  act_bit_name    args( ( int act_flags ) );
char *  off_bit_name    args( ( int off_flags ) );
char *  imm_bit_name    args( ( int imm_flags ) );
char *  form_bit_name   args( ( int form_flags ) );
char *  part_bit_name   args( ( int part_flags ) );
char *  weapon_bit_name args( ( int weapon_flags ) );
char *  comm_bit_name   args( ( int comm_flags ) );
char *  cont_bit_name   args( ( int cont_flags) );
void    set_learnable_disciplines  args ( (CHAR_DATA *ch, int secrace) );
void    reset_learnable_disciplines  args ( (CHAR_DATA *ch, int secrace) );
void    set_clan_disciplines  args ( (CHAR_DATA *ch) );
int	get_treelev	args( (int sn, sh_int level) );

/* interp.c */
void    interpret       args( ( CHAR_DATA *ch, char *argument ) );
bool    is_number       args( ( char *arg ) );
int     number_argument args( ( char *argument, char *arg ) );
int     mult_argument   args( ( char *argument, char *arg) );
char *  one_argument    args( ( char *argument, char *arg_first ) );

/* magic.c */
int     find_spell      args( ( CHAR_DATA *ch, const char *name) );
void    exec_fear       args( (CHAR_DATA *ch) );
int     skill_lookup    args( ( const char *name ) );
int     slot_lookup     args( ( int slot ) );
bool    saves_spell     args( ( int level, CHAR_DATA *victim, int dam_type ) );
void    obj_cast_spell  args( ( int sn, int level, CHAR_DATA *ch,
				    CHAR_DATA *victim, OBJ_DATA *obj ) );
void    fire_breath   	  args( (CHAR_DATA *ch, CHAR_DATA *victim, bool all) );
void    gas_breath    	  args( (CHAR_DATA *ch, CHAR_DATA *victim,bool all) );
void    lightning_breath  args( (CHAR_DATA *ch, CHAR_DATA *victim,bool all) );
void    acid_breath       args( (CHAR_DATA *ch, CHAR_DATA *victim,bool all) );
void    frost_breath       args( (CHAR_DATA *ch, CHAR_DATA *victim,bool all) );
void    shadow_breath     args( (CHAR_DATA *ch, CHAR_DATA *victim,bool all) );

/* save.c */
void    save_char_obj   args( ( CHAR_DATA *ch ) );
bool    load_char_obj   args( ( DESCRIPTOR_DATA *d, char *name ) );
bool    do_load_char_obj args( ( CHAR_DATA *ch, char *name ) );

/* skills.c */
void    check_improve   args( ( CHAR_DATA *ch, int sn, bool success, 
				    int multiplier ) );
int     group_lookup    args( (const char *name) );
void    gn_add          args( ( CHAR_DATA *ch, int gn) );
void    gn_remove       args( ( CHAR_DATA *ch, int gn) );
void    group_add       args( ( CHAR_DATA *ch, const char *name, bool deduct) );
void    group_remove    args( ( CHAR_DATA *ch, const char *name) );

/* update.c */
void    advance_level   args( ( CHAR_DATA *ch, bool hide ) );
void    gain_exp        args( ( CHAR_DATA *ch, int gain ) );
void    gain_condition  args( ( CHAR_DATA *ch, int iCond, int value ) );
void    update_handler  args( ( void ) );
void    second_update   args( ( void ) );

/* psionics.c */
bool has_psi		args( ( CHAR_DATA *ch, char *argument ) );
void psi_slowticker     args( ( CHAR_DATA *ch ) );
void psi_fastticker     args( ( CHAR_DATA *ch ) );
bool charge_psp         args( ( CHAR_DATA *ch, int c ) );
bool can_use_psionics	args( (CHAR_DATA *ch) );
void psi_gain_gen	args( ( CHAR_DATA *ch ) );

/* act_evil.c */
void initialise_quest   args( (CHAR_DATA *ch) );
MID   * get_random_mob	args( (CHAR_DATA *ch) );
OID   * get_random_item	args( ( ) );
bool  can_cast_life     args( (CHAR_DATA *ch) );
bool  can_cast_flame    args( (CHAR_DATA *ch) );
bool  can_cast_water    args( (CHAR_DATA *ch) );
bool  can_cast_earth    args( (CHAR_DATA *ch) );
bool  can_cast_air      args( (CHAR_DATA *ch) );
bool  can_cast_entropy  args( (CHAR_DATA *ch) );
OD    * is_wearing_artifact args( (CHAR_DATA *ch, int artifact) );
OD    * is_carrying_artifact args( (CHAR_DATA *ch, int artifact) );
char  * dragon_colour   args( (sh_int dragtype) );
char  * dragon_colour2   args( (sh_int dragtype) );
bool  IS_AIR		args( (ROOM_INDEX_DATA *room) );
int   is_wall		args( (EXIT_DATA *ex) );

/* vampire.c */
int   max_blood_pool	args( (CHAR_DATA *ch) );
void  taint_transform   args( (CHAR_DATA *ch, int race) );
void  taint_untransform args( (CHAR_DATA *ch) );
int   get_drow_magic_resistance args( (CHAR_DATA *ch ) );
bool  clear_some_spells args( (CHAR_DATA *ch, int percent) );

/* realms.c */
bool  is_same_realm	args( (CHAR_DATA *ch, CHAR_DATA *victim) );

/* runes.c */
void  rune_cast_effect  args( (CHAR_DATA *ch, char *argument, sh_int rune1, sh_int rune2, sh_int rune3) );

/* treasure.c */
void  set_armor		args( (OBJ_DATA *obj, int armour, bool magic) );
OD *  create_blob	args( () );
OD *  create_random_food   	args( (int something) );
OD *  create_random_ring   	args( (int something) );
OD *  create_random_weapon   	args( (int something) );
OD *  create_random_bow   	args( (int something) );
OD *  create_random_quiver   	args( (int something) );
OD *  create_random_arrow   	args( (int something) );
OD *  create_random_treasure   	args( (int something) );

/* beastiary.c */

CHAR_DATA * create_blobling args( ( ) );

/* hero.c */
int   revdir		args( (int direction) );
bool  is_clan_leader	args( (CHAR_DATA *ch) );
CD *  get_rand_person_in_room args( (RID *room) );

/* mage.c */
void  obj_exorcise	args( (CHAR_DATA *ch) );
void  regain_sanity	args( (CHAR_DATA *ch) );
bool  sanity_resist 	args( (CHAR_DATA *ch, CHAR_DATA *victim, int damage) );
int   sanity_regain_rate args( (CHAR_DATA *ch) );
int   get_max_sanity 	args( (CHAR_DATA *ch) );
int   get_sanity 	args( (CHAR_DATA *ch) );
void  move_char_to_obj  args( (CHAR_DATA *ch) );
void  move_obj_to_char  args( (CHAR_DATA *ch) );
bool  char_to_obj	args( (CHAR_DATA *ch, OBJ_DATA *obj) );
bool  char_from_obj	args( (CHAR_DATA *ch) );
bool  is_item		args( (CHAR_DATA *ch) );
void  check_char_sw_obj args( (CHAR_DATA *ch) );

/* heart.c */
bool  can_speak		args( (CHAR_DATA *ch, int lang) );
int   hands_taken_up	args( (CHAR_DATA *ch) );
int   hand_slots	args( (CHAR_DATA *ch) );
bool  damage_obj	args( (OBJ_DATA *obj, int damage) );
void  gain_elad_path    args( (CHAR_DATA *ch, int path) );
bool  test		args( (CHAR_DATA *ch, int test) );
bool  IS_NOVIERE	args( (CHAR_DATA *ch) );
bool  in_same_room	args( (CHAR_DATA *ch, CHAR_DATA *victim) );
bool  can_age		args( (CHAR_DATA *ch) );
int   get_extra_attacks args( (CHAR_DATA *ch) );
bool  can_breathe_underwater args( (CHAR_DATA *ch) );
bool  can_be_made	args( (CHAR_DATA *victim) );
bool  is_snake		args( (CHAR_DATA *ch) );
int   max_disciplines   args( (CHAR_DATA *ch) );
int   disciplines_known args( (CHAR_DATA *ch) );
void  werewolf		args( (CHAR_DATA *ch) );
void  unwerewolf	args( (CHAR_DATA *ch) );
bool  lost_one_leg	args( (CHAR_DATA *ch) );
bool  lost_both_legs	args( (CHAR_DATA *ch) );
bool  has_no_arms	args( (CHAR_DATA *ch) );
char * GET_EXTENSION	args( (CHAR_DATA *ch) );
CD *   obj_owner        args( (OBJ_DATA *obj) );
bool  gain_hp		args( (CHAR_DATA *ch, int hit) );
bool  gain_move 	args( (CHAR_DATA *ch, int move) );
bool  gain_psp		args( (CHAR_DATA *ch, int psp) );
int   GET_HITROLL	args( (CHAR_DATA *ch) );
int   GET_DAMROLL	args( (CHAR_DATA *ch) );
int   GET_AC		args( (CHAR_DATA *ch, int type) );
int   GET_DAMCAP	args( (CHAR_DATA *ch) );
int   get_normal_form   args( (CHAR_DATA *ch) );
bool  is_demonic_form   args( (int form) );
int   disc_points_needed args( (CHAR_DATA *ch) );
void  gain_disc_points  args( (CHAR_DATA *ch, int points ) );
void  crash		args( (char *arg) );
bool  in_were_form	args( (CHAR_DATA *ch) );
void  defang		args( (CHAR_DATA *ch) );
void  declaw		args( (CHAR_DATA *ch) );
void  dehorn		args( (CHAR_DATA *ch) );
void  horn		args( (CHAR_DATA *ch) );
void  detail		args( (CHAR_DATA *ch) );
void  tail		args( (CHAR_DATA *ch) );
void  construct_aura	args( (CHAR_DATA *ch) );
void  deconstruct_aura	args( (CHAR_DATA *ch) );
bool  is_null_room	args( (CHAR_DATA *ch) );
bool  bad_to_interpret	args( (char *argument) );
void  strip_perm_spell  args( (CHAR_DATA *ch, OBJ_DATA *obj) );

/* act_dark.c */
void do_anything        args ( ( CHAR_DATA *ch, int level) );
RID * locate_obj	args ( ( OBJ_DATA *obj ) );
bool is_undead		args ( ( CHAR_DATA *ch) );
void room_message	args ( (ROOM_INDEX_DATA *room, char *message) );


/* stat_save.c */ 
int  league_score	args( ( CHAR_DATA *ch ) );
void save_hits		args( ( ) );
void load_hits		args( ( ) );
void initialise_league_table args( ( ) );  
void check_player_table args( (CHAR_DATA *ch) );  
void add_pkill args((CHAR_DATA *ch, CHAR_DATA *victim, bool combat) );  
void check_pkill_stat_file args( ( ) ); 
void load_config	args( ( ) );
void save_config	args( ( ) );
void initialise_config args( ( ) );
void initialise_balance_table args( ( ) );
void initialise_site_list args( ( ) );
void initialise_email_list args( ( ) );
void add_email_to_list	args( ( char *address ) );
bool check_email	args( ( char *address ) );
void add_site_to_list   args( ( char *num, char *name ) );
char * get_site_old	args( ( char *num ) );


/* prog.c */
void validate_prog	args( ( CHAR_DATA *ch, MOB_PROG_DATA *prog ) );
void check_trigger_room	args( ( CHAR_DATA *ch, int trigger, char *argument) );
void check_trigger	args( ( CHAR_DATA *ch, CHAR_DATA *vch, int trigger, char *argument) );
void init_prog		args( ( CHAR_DATA *mob, CHAR_DATA *trg, char *data, MOB_PROG_DATA *code, int trigger) ); 
void resume_prog	args( ( CHAR_DATA *mob, CHAR_DATA *trg, char *data, int trigger, PROG_DATA *prog) );
PROG_DATA * new_prog    args( ( ) );
void run_prog		args( ( PROG_DATA *prog ) );
void terminate_prog	args( ( PROG_DATA *prog, bool mobalive ) );
void close_progs	args( ( CHAR_DATA *ch) );

/* olc.c */
int  find_free_vnum	args( ( ) );
AREA_DATA *create_area	args( ( ) );
ROOM_INDEX_DATA *olc_create_room_in_area args( ( AREA_DATA *area ) );
void olc_save_area_header args( ( FILE *fp, AREA_DATA *pArea ) );
void olc_save_zone	args( ( AREA_DATA *zone, CHAR_DATA *ch ) );
void olc_save_room	args( ( FILE *fp, ROOM_INDEX_DATA *r ) );
void olc_save_mob	args( ( FILE *fp, MOB_INDEX_DATA *mob ) );
void olc_save_object	args( ( FILE *fp, OBJ_INDEX_DATA *obj ) );
void olc_save_reset	args( ( FILE *fp, RESET_DATA *reset ) );
void copy_description	args( ( char *to, char *from ) );
OID * olc_create_obj    args( ( CHAR_DATA *ch, char *argument ) );
MID * olc_create_mob    args( ( CHAR_DATA *ch, char *argument ) );
RID * olc_create_room   args( ( ROOM_INDEX_DATA *room, int dir ) );
int  get_direction      args( ( char *argument) );
int  reverse_direction  args( ( int dir ) );
int  find_free_room_vnum args( ( AREA_DATA *area ) );
int  find_free_mob_vnum args( ( AREA_DATA *area ) );
int  find_free_obj_vnum args( ( AREA_DATA *area ) );

/* ansi.c */
void ansi_red_desc 	args( ( DESCRIPTOR_DATA *d ) );
void ansi_save_cursor   args( ( CHAR_DATA *ch ) );
void ansi_restore_cursor args(( CHAR_DATA *ch ) );
void send_title		args( ( DESCRIPTOR_DATA *d ) );
void ansi_goto		args( ( CHAR_DATA *ch, int x, int y ) );
void ansi_cls		args( ( CHAR_DATA *ch ) );
void ansi_goto_desc	args( ( DESCRIPTOR_DATA *d, int x, int y ) );
void ansi_cls_desc	args( ( DESCRIPTOR_DATA *d ) );
void ansi_up		args( ( CHAR_DATA *ch, int n ) );
void ansi_down		args( ( CHAR_DATA *ch, int n ) );
void ansi_left		args( ( CHAR_DATA *ch, int n ) );
void ansi_right		args( ( CHAR_DATA *ch, int n ) );
void ansi_split_screen  args( ( CHAR_DATA *ch ) );
void ansi_reset  	args( ( CHAR_DATA *ch ) );

/* General stuff needed for combat.c */
int 	initiative_cost args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	get_punch_dam	args( (CHAR_DATA *ch) );
void	check_quest_kill args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
void    exec_fear       args( ( CHAR_DATA *ch ) );
void    do_bite_atk     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    do_claw_atk     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    do_wing_atk     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    do_horn_atk     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    do_tail_atk     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    check_assist    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_dodge     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    check_killer    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_parry     args( ( CHAR_DATA *ch, CHAR_DATA *victim, int penalty ) );
bool    check_shield_block     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    dam_message     args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, bool immune ) );
void    death_cry       args( ( CHAR_DATA *ch, bool corpse ) );
int     xp_compute      args( ( CHAR_DATA *gch, CHAR_DATA *victim, 
			    int total_levels ) );
bool    is_safe         args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    make_corpse     args( ( CHAR_DATA *ch ) );
 void   one_hit         args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void    mob_hit         args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void    consume_kill    args( ( CHAR_DATA *victim, bool blood ) );
void    disarm          args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_if_crimson      args( ( CHAR_DATA *ch ) );
void	petrify		args( (CHAR_DATA *ch, CHAR_DATA *victim) );
void	turn_to_stone	args( (CHAR_DATA *ch) );
void    wipe_spells	args( (CHAR_DATA *victim) );
int	calc_thaco	args( (CHAR_DATA *ch) );


/* Combat.c */
char * damtext_v	args( ( int dam ) );
char * damtext_vp	args( ( int dam ) );
char * damtext_vs	args( ( int dam ) );
int  des_dist		args( ( CHAR_DATA *ch ) );
int  get_block_arm	args( ( CHAR_DATA *ch, int itemtype ) );
int  body_arm		args( ( int arm ) );
int  advance_rate	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int  attack_reach	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool will_attack	args( ( CHAR_DATA *ch, int cur_init, int init_cost ) );
int  randomize_damage	args( ( CHAR_DATA *ch, int dam, int am ) );
void char_attack	args( ( CHAR_DATA *ch ) );
void check_group_combat args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int  did_hit		args( ( CD *ch, CD *victim, int sn, int dam_type));
int  cap_dam		args( ( CD *ch, CD *victim, int dam, int dam_type, bool fitness));
void display_weapon_damage args((CD *ch,CD *victim, OD *wield,int dam, int atm));
bool deal_damage	args( ( CD *ch, CD *victim, int dam ) );
void display_weapon_miss args(( CD *ch, CD *victim, OD *wield ) );
void display_punch_miss args(( CD *ch, CD *victim ) );
void display_punch_damage args(( CD *ch, CD *victim, int dam ) );
void display_kick_miss args(( CD *ch, CD *victim ) );
void display_kick_damage args(( CD *ch, CD *victim, int dam ) );
void display_bash_miss args(( CD *ch, CD *victim ) );
void display_bash_damage args(( CD *ch, CD *victim, int dam ) );
void display_wing_miss args(( CD *ch, CD *victim ) );
void display_wing_damage args(( CD *ch, CD *victim, int dam ) );
void display_bite_miss args(( CD *ch, CD *victim ) );
void display_bite_damage args(( CD *ch, CD *victim, int dam ) );
void display_tail_miss args(( CD *ch, CD *victim ) );
void display_tail_damage args(( CD *ch, CD *victim, int dam ) );
void check_funky_weapon args( (CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield) );

/* trade.c */

void gain_trade		args( ( CHAR_DATA *ch ) );
void improve_trade	args( ( CHAR_DATA *ch, int tr ) );

/* site.c */

char * get_site		args( ( char *add ) );
void add_site		args( ( char *add, char *name ) );
bool site_found		args( ( char *add ) );

/* whore.c */

CD * 	animate_object		args( ( OBJ_DATA *obj ) );
char * 	clan_fighting		args( ( int clan ) );
char * 	clan_ally		args( ( int clan ) );
void	save_clan_leaders 	args( ( ) );
void	load_clan_leaders 	args( ( ) );


#undef  CD
#undef  MID
#undef  OD
#undef  OID
#undef  RID
#undef  SF
#undef AD
