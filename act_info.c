/*
 * HELL, formerly known as BTL/Darkheart, by Adam Powell and Chris Mellish,
 *  1996 to 1998.  If you reproduce this code in any way please give credit
 *  to the writers.  Thanks.
 */


#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "hell.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"

/* command procedures needed */
DECLARE_DO_FUN( do_exits        );
DECLARE_DO_FUN( do_visible      );
DECLARE_DO_FUN( do_look         );
DECLARE_DO_FUN( do_help         );
DECLARE_DO_FUN( do_affects      );
DECLARE_DO_FUN( do_play         );
DECLARE_DO_FUN( do_sremove      );
DECLARE_DO_FUN( do_examine      );

ROOM_INDEX_DATA * find_location args( ( CHAR_DATA *ch, char *arg ) );
void show_gore_to_char args( (CHAR_DATA *ch) );
void form_in_room_desc args( ( CHAR_DATA *ch, char *buf ) );

/* Fun fun fun fun fun fun forms */
/* name, hitprob, damprob, acmod, hitpointmod, squishable? , size!!!!*/
/* can_cast, can_fly, can_wear, carry_max, max_items, say_word, 
move_word, xp_bonus, polymorph_level, in_room_desc, can use claws
*/

const struct form_type form_data [MAX_FORMS] =
{
    {"","",                0,      0,      0,      0,      FALSE, 0,
	FALSE, FALSE, FALSE, 0, 0, "","", 0, 0, "","", TRUE},
    {"wolf","",  40,     40,     -50,    500,    FALSE, 
	SIZE_MEDIUM, FALSE, FALSE, FALSE, 10, 1, "", "bounds", 120, 5, "","the dire wolf",TRUE},
    {"bat","",                -10,    -10,    -80,    
	150,    FALSE, SIZE_TINY, FALSE,TRUE,FALSE,1,1,"","flaps",70,-1,"","the vampire bat",FALSE},
    {"mist"," the vampiric mist",      -50,    -50,    -200,   0,      
	FALSE, SIZE_MEDIUM, FALSE, TRUE, FALSE, 0,0,"","flows",110,-1,"","",FALSE},
    {"dolphin","",    -5,     -5,     -10,    200,    FALSE, 
	SIZE_SMALL,FALSE,FALSE,FALSE,10,1,"","swims",80,3,"","the dolphin",FALSE},
    {"flame"," the pillar of flame",    10,     10,     -60,    300,    
	FALSE, SIZE_LARGE,FALSE,FALSE,FALSE,0,0,"","roars",100,-1,"","",FALSE},
    {"spider","",-20,   0,      0,      0,      TRUE, 
	SIZE_TINY,FALSE,FALSE,FALSE,0,0,"","crawls",50,1,"","the hairy spider",FALSE},
    {"light"," the ball of light",      20,     0,      -60,    150,    
	FALSE, SIZE_SMALL,FALSE,FALSE,FALSE,0,0,"","floats",160,-1,"","",FALSE},
    {"cockroach","",  -20,    -20,    60,     0,      TRUE, 
	SIZE_TINY,FALSE,FALSE,FALSE,0,0,"","creeps",50,1,"","the cockroach",FALSE},
    {"rabbit","",       -20,    -20,    20,     -10,    
	FALSE, SIZE_TINY,FALSE,FALSE,FALSE,1,1,"","hops",70,2,"","the bunny rabbit",FALSE},
    {"blob"," the amorphous blob",     -20,    -20,    -20,    -10,    
	FALSE, SIZE_SMALL,FALSE,FALSE,FALSE,0,0,"","oozes",50,-1,"","the amorphous blob",FALSE},
/* blob = 10 */
    {"frog","",       -20,    -20,    20,     -10,    TRUE, 
	SIZE_TINY,FALSE,FALSE,FALSE,0,0,"","hops",50,2,"","the frog",FALSE},
    {"idiot"," the idiot",      -100,   -100,   100,    -20,    TRUE, 
	SIZE_MEDIUM,FALSE,FALSE,FALSE,0,0,"","rolls",0,-1,"","",FALSE},
    {"whirlwind"," the whirlwind",  40,     20,     -60,    350,    
	FALSE,SIZE_LARGE,FALSE,TRUE,FALSE,1000,100,"","blows",100,-1,"","",FALSE},
    {"crimson death"," the crimson death",  150,     150,     -200,    
	1000,FALSE, SIZE_MEDIUM,TRUE,TRUE,FALSE,0,0,"","flows",170,-1,"","",FALSE},
    {"hawk","",  20,     20,     -80,    0,    FALSE, SIZE_SMALL,
	FALSE,TRUE,FALSE,2,1,"screetches","flies", 100, 4,
	"A large hawk, red in colour, flies gracefully overhead.", "a hawk", FALSE},
    {"werewolf","",  60,     60,     -40,    0,    FALSE, SIZE_MEDIUM,
	FALSE,FALSE,TRUE,500,60,"growl","bounds",140,-1,
	"A ravenous half-man, half-wolf is standing here.", "a savage werewolf",TRUE},
    {"fox","",  -20,     -20,     -40,    0,    FALSE, SIZE_SMALL,
	FALSE,FALSE,FALSE,20,1,"growl","bounds",80,3,
	"A small brown fox is here.", "a small fox",FALSE},
    {"wyvern","",  50,     70,     -40,    0,    FALSE, SIZE_LARGE,
	TRUE,TRUE,FALSE,200,1,"growl","flies",120,8,
	"A black wyvern is flying overhead here.", "the monsterous wyvern",TRUE},
    {"gargantua", " the gargantua", 150, 150, -200, 500, FALSE,SIZE_GIANT,
	FALSE, FALSE, FALSE, 10000, 10, "rumble", "crashes", 190, 10,
	"A huge reptillian gargantua is towering over you.", "the reptillian gargantua",TRUE},

/* Fun fun fun fun fun fun forms */
/* name, hitprob, damprob, acmod, hitpointmod, squishable? , size!!!!*/
/* can_cast, can_fly, can_wear, carry_max, max_items, say_word, 
move_word, xp_bonus, polymorph_level, in_room_desc, form_name, can_use_claws
*/

    {"peasant","", 0,      0,      0,      0, FALSE, SIZE_MEDIUM,
	TRUE, FALSE, TRUE, 300, 30, "say","walks", 0, 5,
	 "A peasant in shabby clothes is standing here.", "the peasant",FALSE},

/* peasant = 20 */

    {"warrior","", 10,      10,      -20,      50,FALSE,SIZE_MEDIUM,
	TRUE, FALSE, TRUE, 300, 30, "say","strides", 0, 7,
	 "A strong warrior is here.", "the warrior",FALSE},

    {"barbarian","", 5,      5,      -10,      20, FALSE,SIZE_MEDIUM,
	TRUE, FALSE, TRUE, 300, 30, "say","walks", 0, 6,
	 "A rough barbarian is here, covered in war paint.", "the barbarian",FALSE},

    {"girl","", 5,      5,      -10,      20, FALSE,SIZE_MEDIUM,
	TRUE, FALSE, TRUE, 300, 30, "giggle","skips", 0, 6,
	 "A small girl is here, skipping.", "the small girl",FALSE},


    /* The demonic forms - 13 of them, oh goody */

/* lemure = 24 */

    {"lemure","", 0,      0,      0,      20, FALSE,SIZE_MEDIUM,
	TRUE, FALSE, TRUE, -1, -1, "gurgle","slimes", 0, -1,
	 "A slimy misshapen lump of flesh lies here in a pile.", "the lemure",TRUE},
    {"nupperibo","", 0,      0,      0,      20, FALSE,SIZE_MEDIUM,
	TRUE, FALSE, TRUE, -1, -1, "gurgle","slimes", 0, -1,
	 "A foul bloated humanoid is here, infested with maggots.", "the nupperibo",TRUE},
    {"spinagon","", 0,      0,      0,      20, FALSE,SIZE_MEDIUM,
	TRUE, FALSE, TRUE, -1, -1, "growl","flies", 0, -1,
	 "A small imp like creature covered with spines hovers here.", "the spinagon",TRUE},
    {"barbazu","", 0,      0,      0,      20, FALSE,SIZE_MEDIUM,
	TRUE, FALSE, TRUE, -1, -1, "roar","stalks", 0, -1,
	 "A foul, bearded and tall demonic warrior patrols here.", "the barbazu",TRUE},
    {"abishai","", 0,      0,      0,      20, FALSE,SIZE_MEDIUM,
	TRUE, FALSE, TRUE, -1, -1, "growl","flies", 0, -1,
	 "A red-skinned, gargoyle-like demon flaps its wings here. ", "the abishai",TRUE},
    {"osyluth","", 0,      0,      0,      20, FALSE,SIZE_MEDIUM,
	TRUE, FALSE, TRUE, -1, -1, "growl","clatters", 0, -1,
	 "A sickly stretched skeletal demon with a poisonous tail stands here.", "the osyluth",TRUE},
    {"hamatula","", 0,      0,      0,      20, FALSE,SIZE_MEDIUM,
	TRUE, FALSE, TRUE, -1, -1, "growl","stalks", 0, -1,
	 "A tall spiny demonic knight stands here proudly.", "the hamatula",TRUE},
    {"erinyes","", 0,      0,      0,      20, FALSE,SIZE_MEDIUM,
	TRUE, FALSE, TRUE, -1, -1, "say","walks", 0, -1,
	 "A beautiful woman with white feathery wings stands here.", "the erinyes",TRUE},
    {"amnizu","", 0,      0,      0,      20, FALSE,SIZE_SMALL,
	TRUE, FALSE, TRUE, -1, -1, "cough","floats", 0, -1,
	 "A foul little winged, pug-nosed dwarf hovers here, sneering.", "the amnizu",TRUE},
    {"cornugon","", 0,      0,      0,      20, FALSE,SIZE_LARGE,
	TRUE, FALSE, TRUE, -1, -1, "growl","crashes", 0, -1,
	 "A huge red-skinned demon with a spiny tail stands here.", "the cornugon",TRUE},
    {"gelugon","", 0,      0,      0,      20, FALSE,SIZE_LARGE,
	TRUE, FALSE, TRUE, -1, -1, "howl","slimes", 0, -1,
	 "An insect-like frost demon with a long tail stands here.", "the gelugon",TRUE},
    {"pit fiend","", 0,      0,      0,      20, FALSE,SIZE_HUGE,
	TRUE, FALSE, TRUE, -1, -1, "bellow","crashes", 0, -1,
	 "A hulking twelve-foot demon is here, dripping with molten lava.", "the pit fiend",TRUE},
    {"pit lord","", 0,      0,      0,      20, FALSE,SIZE_HUGE,
	TRUE, FALSE, TRUE, -1, -1, "thunder","crashes", 0, -1,
	 "A huge black-skinned demon lord stands here.", "the pit lord",TRUE},

    /* tree form */
/* tree = 37 */
    {"tree","", -20,      -20,     -200,      0, FALSE,SIZE_LARGE,
	FALSE, FALSE, FALSE, -1, -1, "creak","uproots", 0, -1,
	 "A huge gnarled tree stands here.", "the gnarled tree",FALSE},


    /* dragon forms */

    {"hatchling dragon", "", 0, 0, 0, 0, FALSE, SIZE_MEDIUM,
	TRUE, TRUE, TRUE, -1, -1, "slurp", "crawls", 0, -1,
	"A small %s dragon hatchling is here.", "the %s hatchling",TRUE},

    {"young dragon", "", 0, 0, 0, 0, FALSE, SIZE_MEDIUM,
	TRUE, TRUE, TRUE, -1, -1, "hisses", "crawls", 0, -1,
	"A young %s dragon is here.", "the %s young dragon",TRUE},

    {"juvenile dragon", "", 0, 0, 0, 0, FALSE, SIZE_MEDIUM,
	TRUE, TRUE, TRUE, -1, -1, "hisses", "crawls", 0, -1,
	"A juvenile %s dragon stands here.", "the %s dragon",TRUE},

    {"young adult dragon", "", 0, 0, 0, 0, FALSE, SIZE_LARGE,
	TRUE, TRUE, TRUE, -1, -1, "rumble", "walks", 0, -1,
	"A medium sized %s dragon stands here.", "the %s dragon",TRUE},

    {"mature dragon", "", 0, 0, 0, 0, FALSE, SIZE_LARGE,
	TRUE, TRUE, TRUE, -1, -1, "rumble", "walks", 0, -1,
	"A mature %s dragon stands here.", "the mature %s dragon",TRUE},

    {"adult dragon", "", 0, 0, 0, 0, FALSE, SIZE_LARGE,
	TRUE, TRUE, TRUE, -1, -1, "growl", "strides", 0, -1,
	"A large, adult %s dragon stands here.", "the adult %s dragon",TRUE},

    {"elder dragon", "", 0, 0, 0, 0, FALSE, SIZE_HUGE,
	TRUE, TRUE, TRUE, -1, -1, "growl", "crashes", 0, -1,
	"A huge elder %s dragon is here.", "the elder %s dragon",TRUE},

    {"venerable dragon", "", 0, 0, 0, 0, FALSE, SIZE_HUGE,
	TRUE, TRUE, TRUE, -1, -1, "bellow", "crashes", 0, -1,
	"A venerable %s dragon is here.", "the venerable %s dragon",TRUE},

    {"ancient dragon", "", 0, 0, 0, 0, FALSE, SIZE_GIANT,
	TRUE, TRUE, TRUE, -1, -1, "roar", "crashes", 0, -1,
	"A giant ancient %s dragon is standing here.", "the ancient %s dragon",TRUE},

    {"lesser wyrm", "", 0, 0, 0, 0, FALSE, SIZE_GIANT,
	TRUE, TRUE, TRUE, -1, -1, "roar", "crashes", 0, -1,
	"A giant lesser %s wyrm is standing here.", "the lesser %s wyrm",TRUE},

    {"wyrm", "", 0, 0, 0, 0, FALSE, SIZE_GIANT,
	TRUE, TRUE, TRUE, -1, -1, "roar", "crashes", 0, -1,
	"A giant %s wyrm is here, towering over you.", "the %s wyrm",TRUE},

    {"great wyrm", "", 0, 0, 0, 0, FALSE, SIZE_GIANT,
	TRUE, TRUE, TRUE, -1, -1, "thunder", "crashes", 0, -1,
	"A Great %s Wyrm is here, towering over you.", "the great %s wyrm",TRUE},

    {"supreme dragon", "", 0, 0, 0, 0, FALSE, SIZE_GIANT,
	TRUE, TRUE, TRUE, -1, -1, "thunder", "crashes", 0, -1,
	"The colossal Supreme %s Dragon towers over you.", "the supreme %s dragon",TRUE},

    /* The alternative were beasties */

/* wererat = 51 */

    {"wererat","",  60,     60,     -40,    0,    FALSE, SIZE_MEDIUM,
	FALSE,FALSE,TRUE,500,60,"growl","bounds",140,-1,
	"A filthy half-man, half-rat is standing here.", "a filthy wererat",TRUE},
    {"werebear","",  60,     60,     -40,    0,    FALSE, SIZE_LARGE,
	FALSE,FALSE,TRUE,500,60,"boom","lumbers",140,-1,
	"A hulking great half-man, half-bear is standing here.", "a hulking werebear",TRUE},
    {"weretiger","",  60,     60,     -40,    0,    FALSE, SIZE_MEDIUM,
	FALSE,FALSE,TRUE,500,60,"roar","pounces",140,-1,
	"A ferocious half-man, half-tiger is standing here.", "a ferocious weretiger",TRUE},
    {"wereshark","",  60,     60,     -40,    0,    FALSE, SIZE_MEDIUM,
	FALSE,FALSE,TRUE,500,60,"growl","stalks",140,-1,
	"A ravenous half-man, half-shark is standing here.", "a hungry wereshark",TRUE},
    {"stone golem","",  0,     100,     -300,    0,    FALSE, SIZE_MEDIUM,
	FALSE,FALSE,FALSE,500,60,"scrape","lumbers",140,-1,
	"A roughly carved but lifelike humanoid statue stands here.", "a stone golem",TRUE},
    {"pixie","",  20,     1,     -10,    0,    FALSE, SIZE_SMALL,
	TRUE,TRUE,TRUE,500,60,"squeak","flies",140,-1,
	"A little pixie hovers around the room here, flapping its tiny wings.", "a pixie",FALSE},
    {"manticore","",  50,     50,     -90,    1000,    FALSE, SIZE_LARGE,
	TRUE,TRUE,FALSE,500,60,"roar","flies",140,-1,
	"A manticore patrolls the area proudly, growling at you.", "a manticore",TRUE}
};

/* Fun fun fun fun fun fun forms */
/* name, hitprob, damprob, acmod, hitpointmod, squishable? , size!!!!*/
/* can_cast, can_fly, can_wear, carry_max, max_items, say_word, 
move_word, xp_bonus, polymorph_level, in_room_desc, short_name, can use claws
*/


const char * daytime[24] = 
{"midnight", "just past midnight", "the middle of the night",
 "the middle of the night", "nearly sunrise",
 "just before sunrise", "daybreak", "very early in the morning",
 "early in the morning", "morning", "late morning",
 "approaching noon", "noon", "early afternoon",
 "mid afternoon", "mid afternoon", "late afternoon",
 "early evening", "evening time", "late evening",
 "starting to get dark", "getting very dark",
 "late at night", "nearly midnight" };


/* secondary races */
/* Name, min level, propname */
const struct secondaryrace_type secondaryrace [10] =
{
    {   "  None   ",                0 , "Mortal"     },
    {   " Dragon  ",          0 ,"Dragon"     },
    {   "  Demon  ",          0 ,"Demon"     },
    {   " Vampire ",          0 ,"Vampire"     },
    {   " Lupine  ",          0 ,"Lupine"     },
    {   " Eladrin ",          0 ,"Eladrin"     },
    {   " Warlock ",          0 ,"Warlock"     },
    {   "  *Hero* ",          0 ,"Hero"     },
    {   "  Death  ",          0 ,"Death Knight"   },
    {   " Psionic ",          0 ,"Psionicist"   }
};

/* tertiary races */
const struct tertiaryrace_type tertiaryrace [6] =
{
    {  "",         "",          0, 0, 0 },
    {  "Crimson Death",  " Crimson ", 4, 8, 13},
    {  "Witch",    "  Witch  ", 5, 6, 12},
    {  "Lich",    "   Lich  ", 4, 11, 13},
    {  "Aasimon",  " Aasimon ", 6, 9, 10},
    {  "Drow",	   "  Drow   ", 1 ,2 ,3 }
};

/* %age scores */

/* use formula score_word[UMAX(100,%age/5)] */
const char * score_word[30] =
{
	"untrained",     /*  1% -  4% */
	"abysmal",       /*  5% -  9% */
	"pathetic",      /* 10% - 14% */
	"bad",           /* 15% - 19% */
	"very poor",     /* 20% - 24% */
	"poor",          /* 25% - 29% */
	"adequate",      /* 30% - 34% */
	"average",       /* 35% - 39% */
	"well-trained",  /* 40% - 44% */
	"good",          /* 45% - 49% */
	"very good",     /* 50% - 54% */
	"adept",         /* 55% - 59% */
	"artisan",       /* 60% - 64% */
	"great",         /* 65% - 69% */
	"superb",        /* 70% - 74% */
	"excellent",     /* 75% - 79% */
	"exceptional",   /* 80% - 84% */
	"amazing",       /* 85% - 89% */
	"master",        /* 90% - 94% */
	"grand master",  /* 95% - 99% */
	"perfect",        /* 100%  */
	"! perfect !",        /* 100%  */
	"= perfect =",        /* 100%  */
	"# perfect #",        /* 100%  */
	"^ perfect ^",       /* 100%  */
	"~ perfect ~",        /* 100%  */
	"perfect *",        /* 100%  */
	"perfect **",        /* 100%  */
	"perfect ***",       /* 100%  */
	"perfect ****"        /* 100%  */
};

const char * exitname [6] =
{
	"north",
	"east",
	"south",
	"west",
	"up",
	"down"
};

const char * exitname2 [6] =
{
	"to the north",
	"to the east",
	"to the south",
	"to the west",
	"above you",
	"below you"
};

const char * circlesymbol [7] =
{
	" ",
	"#1*#n",
	"#4~#n",
	"#6=#n",
	"#2###n",
	"#5+#n",
	"#3-#n"
};

/* ability score table */
const char * abilityword [31] =
{
	"none       ",
	"pitiful    ",
	"abysmal    ",
	"pathetic   ",
	"minimal    ",     
	"bad        ",          /* 5 */
	"very poor  ",
	"poor       ",
	"low        ",
	"adequate   ",
	"average    ",  /* 10 */
	"average    ",
	"average    ",
	"quite good ",
	"good       ",
	"very good  ",  /* 15 */
	"great      ",
	"superb     ",
	"excellent  ",
	"exceptional",
	"amazing    ",  /* 20 */
	"superhuman ",
	"supreme    ",
	"divine     ",
	"godly      ",
	"ultimate   ",  /* 25 */
	"ultimate   ",
	"ultimate   ",
	"ultimate   ",
	"ultimate   ",
	"ultimate   "   /* 30 */
};

/* promotion messages */

const char * transformation_message[2][12] =

{
	{
		"Your body shudders and convulses as your form transforms.  Arms and legs grow out of
your slimy torso, your skin turns a sickly veined green.\n\r" ,
		"Your demonic form is reduced to a pool of pulsating jelly, the fires of hell
heat and char your semi-liquid form until you stiffen.  Limbs grow and spiny
talons extend from your arms and back.\n\r" ,
		"You start the transformation.  Your gargoylish apperance is changed as you grow
another two feet in height.  Scales, dripping with acid, begin to cover your
skin.\n\r" ,
		"The fires of hell surround you once more.  Sharp claws start to grow from
your hands, and you grow a viscious barbed tail.  Small flames play over your
burnt and blackened skin.\n\r" ,
		"Your next form is that of a sickly, gaunt skeleton.  Your leathery flesh
held tight over a demonic frame.  A huge tail, with a poisonous barb on the end
protrudes out of the base of your spine.\n\r" ,
		"As you transform, you grow another four feet in height.  This time, wicked
barbs cover your entire body.  You find you have the ability to wield weapons
weapons, and use armor.\n\r" ,
		"What?  What is happening, with the transformation into an Erinyes, your
form is no longer repulsive and demonic, but you are now a beautiful
mortal, with long auburn hair and huge white wings growing out of your
back.  You can see the world again with your mortal eyes, but the 
passions of a demon will always stay.\n\r",
		"Your skin melts and you transform once more, but this time you become a gruesome
pug nosed, vile dwarf-like creature with large leathery wings and a mouth
filled of razor sharp teeth, an Amnizu.\n\r" ,
		"The intense heat of the fires of Baator surround you.  Your skin is
wrenched apart and you transform into a grotesque 9-foot tall monstrosity
with huge wings, a snaking, prehensile tail, and a long barbed
whip.  A true demon, a Cornugon.\n\r",
		"The power of Baator now fills your very soul.  You grow again
as the chill winds of Caina tear the flesh from your bones.  You look to see what
you have become, you look alien, with a twelve foot long icy insect-like
body, your head bulging with multi-faceted eyes and you have
a long tail covered with razor-sharp spikes.\n\r",
		"Aaargh!  You are cast into the pit of flame. The fires, more intense
than ever before sear and scar your flesh as it bubbles and boils.
You have become the most terrible demon, the Pit Fiend, a huge bat winged 
humanoid, twelve feet tall, with large fangs that drip with a vile green liquid
and a hulking red scaly body that bursts into flame when you are angered or
excited.\n\r",
		"Your fiery skin blackens as you are infused with the embodiment of
ultimate evil.  You are the most powerful demon, the lord of all.  You
are everything, nothing can oppose you.  Twenty foot tall, your torso
drips with the vile acid of the Abyss.  A ravenous hunger is gnawing at your
bones.  Welcome to Immortality..."
	},
	{
"Your first transformation is quite remarkable, you grow to over seven feet tall,
and your skin starts to dull and lose all signs of vitality.  Your teeth begin to
extend.  The world starts to look different, you have left mortality behind.\n\r",
"You start to grow, the base of your spine slowly extends into a short stump of
a tail.  The metamorphosis is tearing you apart.  Your skin begin to harden, and
your hands twist into claws.\n\r",
"Once again the painful metamorphosis strikes.  Your bones rend through your
scaled flesh as jagged spines shoot through your back.  You feel as if you are
being wrenched apart.  You are now over ten feet tall, and your skin is covered
with thick green scales.\n\r",
"You transform once again.  This time a small pair of wings burst through your
back.  Your hands and feet have become viscious claws, and your nose has extended
into a snout.  Long jagged teeth begin to grow in your mouth, and your tail now
reaches the floor.\n\r",
"You fall to the floor in agony.  Clutching your chest as the transformation
strikes.  You grow to over fifteen feet in length, towering above all the
mortals that you have left behind.  Impenetrable scales now cover your body, 
and smoke issues from your nostrils when you breathe.\n\r",
"You grow to over twenty feet in length, your tail and claws grow at an alarming
rate.  Your wings are now so powerful that you can fly.  Nobody can oppose you
now.  Suddenly it hits you, a hunger so intense that you cannot ignore it.
You must feed.\n\r",
"",
"",
"",
"",
"You have reached the pinnacle of power for a dragon.  As your body undergoes
the by now familiar transformation you grow to over fifty feet tall, with
incredibly sharp claws and fangs, a powerful tail and a fiery breath weapon.
Nobody can oppose you now.  You are the lord of the realm.\n\r",
""
	}
};

/* Clan Rank table */
const struct clanrank_type clanrank [10][14] =
{
    { /* None */
	{       "",              0   ,"" ,""   },
	{       "",              0   ,"" ,""   },
	{       "",              0   ,"" ,""   },
	{       "",              0   ,"" ,""   },
	{       "",              0   ,"" ,""   },
	{       "",              0   ,"" ,""   },
	{       "",              0   ,"" ,""   },
	{       "",              0   ,"" ,""   },
	{       "",              0   ,"" ,""   },
	{       "",              0   ,"" ,""   },
	{       "",              0   ,"" ,""   },
	{       "",              0   ,"" ,""   },
	{       "",              0   ,"" ,""   },
	{       "",              0   ,"" ,""   }
    },
    { /* Dragon */
	{       " bug - chris you git",  0 ,"",""     },
	{       "Hatchling",       30 ,"Dragon Hatchling","Dragon Hatchling"     },
	{       "Young",           34 ,"Young Dragon","Young Dragon"     },
	{       "Juvenile",        40 ,"Juvenile Dragon","Juvenile Dragon"     },
	{       "Young Adult",     46 ,"Dragon","Dragon"     },
	{       "Mature",          52 ,"Mature Dragon","Mature Dragon"     },
	{       "Adult",           58 ,"Adult Dragon","Adult Dragon"     },
	{       "Elder",           65 ,"Elder Dragon","Elder Dragon"     },
	{       "Venerable",       72 ,"Venerable Dragon","Venerable Dragon"     },
	{       "Ancient",         79 ,"Ancient Dragon","Ancient Dragon"     },
	{       "Lesser Wyrm",     86  ,"Lesser Wyrm","Lesser Wyrm"    },
	{       "Wyrm",            93  ,"Wyrm","Wyrm"    },
	{       "Great Wyrm",      100 ,"Great Wyrm","Great Wyrm"    },
	{       "Supreme Dragon",  112 ,"Supreme Dragon","Supreme Dragon"    }
    },
    { /* Demon */
	{       "  *** BUG ***",         0  ,"bug",""    },
	{       " Lemure>",             20  ," the lemure","lemure"    },
	{       " Nupperibo>",          25  ," the nupperibo","nupperibo"    },
	{       " Spinagon>",           31  ," the spinagon","spinagon"    },
	{       " Barbazu>",            38  ," the barbazu","barbazu"    },
	{       " Abishai>",            45  ," the abishai","abishai"    },
	{       " Osyluth>",            52  ," the osyluth","osyluth"    },
	{       " Hamatula>",           59  ," the hamatula","hamatula"    },
	{       " Erinyes>",            67  ," the erinyes","erinyes"    },
	{       " Amnizu>",             75  ," the amnizu","amnizu"    },
	{       " Cornugon>",           84  ," the cornugon","cornugon"    },
	{       " Gelugon>",            93  ," the gelugon","gelugon"    },
	{       " Pit Fiend>",          102  ," the pit fiend","pit fiend"   },
	{       " Pit Lord>",           112  ," the pit lord","pit lady"   }
    },
    { /* Vampires */
	{       " AARGH bug bzzz",       0   ,"",""   },
	{       " Childe}",             20   ,"Childe Vampire","Childe Vampiress"   },
	{       " Childe}",             25   ,"Childe Vampire","Childe Vampiress"   },
	{       " Neonate}",            31   ,"Neonate Vampire","Neonate Vampiress"   },
	{       " Primogen}",           38   ,"Vampire","Vampiress"   },
	{       " Elder}",              45   ,"Vampire","Vampiress"   },
	{       " Archon}",             52   ,"Vampire","Vampiress"   },
	{       " Justicar}",           59   ,"Vampire","Vampiress"   },
	{       " Prince}",             67   ,"Vampire Prince","Vampire Princess"   },
	{       " of the Inner Circle}",75   ,"Vampire Prince","Vampire Princess"   },
	{       " Methuselah}",         84   ,"Methuselah","Methuselah"   },
	{       " Antediluvian}",       93   ,"Antediluvian","Antediluvian"   },
	{       " Inconnu}",       102  ,"Vampire Lord","Vampire Lady"},
	{       " Supreme Vampire}",       112  ,"Son of Caine","Daughter of Caine"   }
    },
    { /* Lupine */
	{       "None ",                 0    ,"",""   },
	{       " Pup)",                 20   ,"Were-pup","Were-pup"   },
	{       " Kid)",                 25   ,"Werewolf","Werewolf"   },
	{       " Adult)",               31   ,"Werewolf","Werewolf"   },
	{       " Mature Adult)",        38   ,"Werewolf","Werewolf"   },
	{       " Tracker)",             45   ,"Werewolf","Werewolf"   }, 
	{       " Hunter)",              52   ,"Werewolf","Werewolf"   },
	{       " Warrior)",             59   ,"Werewolf","Werewolf"   },
	{       " Warlord)",             67   ,"Werewolf Lord","Werewolf Lady"   },
	{       " Shaman)",              75   ,"Werewolf Shaman","Werewolf Shaman"   },
	{       " Tribal Chief)",        84   ,"Werewolf Chief","Werewolf Chief"   },
	{       " War Chief)",           93   ,"Werewolf Prince","Werewolf Princess"   },
	{       " Overlord)",            102  ,"Werewolf Overlord","Werewolf Overlord"   },	{       " Deity)",               112  ,"Werewolf Diety","Werewolf Diety"   
}
    },
    { /* Eladrin */
	{       "None ",                 0      ,"",""},
	{       " Coure]",              20      ,"Coure","Coure"},
	{       " Coure]",              25      ,"Coure","Coure"},
	{       " Noviere]",            31      ,"Noviere","Noviere"},
	{       " Noviere]",            38      ,"Noviere","Noviere"},
	{       " Bralani]",            45      ,"Bralani","Bralani"},
	{       " Bralani]",            52      ,"Bralani","Bralani"},
	{       " Shiere]",             59      ,"Shiere","Shiere"},
	{       " Firre]",              67      ,"Firre","Firre"},
	{       " Firre]",              75      ,"Firre","Firre"},
	{       " Ghaele]",             84      ,"Ghaele","Ghaele"},
	{       " Ghaele]",             93      ,"Ghaele","Ghaele"},
	{       " Tulani]",             102     ,"Tulani","Tulani" },
	{       " Faerie Lord]",        112     ,"Faerie Prince","Faerie Princess" }
    },
    { /* Warlocks */
	{       "None ",                  0   ,"",""   },
	{       " Student>-",             15  ,"Student Wizard","Student Wizard"    },
	{       " Adept>-",               19  ,"Adept Wizard","Adept Wizard"    },
	{       " Artisan>-",           24   ,"Artisan Wizard","Artisan Wizard"   },
	{       " Master>-",            30   ,"Master Wizard","Lady Wizard"   },
	{       " Conjurer>-",          36   ,"Conjurer","Conjuress"   },
	{       " Enchanter>-",         42   ,"Enchanter","Enchantress"   },
	{       " Abjurer>-",           49   ,"Abjurer","Abjuress"   },
	{       " Sorcerer>-",          57   ,"Sorcerer","Sorceress"   },
	{       " Mage>-",              66   ,"Mage","Magi"   },
	{       " Wizard>-",            78   ,"High Wizard","High Wizard"   },
	{       " Grand Wizard>-",      93   ,"Grand Wizard","Grand Magi"   },
	{       " Archmage>-",          99   ,"Archmage","Archmagi"   },
	{       " Deity>-",    112  ,"God","Goddess"   }
    },
    { /* Hero */
	{       "None ",                 0   ,"",""   },
	{       " Traveller<>",          15  ,"Adventurer","Adventurer"    },
	{       " Journeyman<>",         19  ,"Adventurer","Adventurer"    },
	{       " Artisan<>",            24  ,"Adventurer","Adventurer"    },
	{       " Master<>",             30  ,"Adventurer","Adventurer"    },
	{       " Defender<>",           37  ,"Adventurer","Adventurer"    },
	{       " Protector<>",          43  ,"Adventurer","Adventurer"    },
	{       " Crusader<>",           50  ,"Crusader","Crusader"    },
	{       " Hero<>",               58  ,"Hero","Heroin"    },
	{       " Legend<>",             68  ,"Hero","Heroin"    },
	{       " Legend<>",             80  ,"Hero","Heroin"    },
	{       " Avatar<>",             93  ,"Avatar","Avatar"    },
	{       " Saviour<>",            102   ,"Avatar","Avatar"  },
	{       " Immortal<>",           112   ,"Immortal","Immortal"  }
    },
    { /* Death Knight */
	{       "None ",                 0   ,"",""   },
	{       " Squire>",             15  ,"death knight","death knight"    },
	{       " Squire>",             19  ,"Adventurer","Adventurer"    },
	{       " Trainee>",            24  ,"Adventurer","Adventurer"    },
	{       " Master>",             30  ,"Adventurer","Adventurer"    },
	{       " Lord>",               37  ,"Adventurer","Adventurer"    },
	{       " Anti-Hero>",          43  ,"Adventurer","Adventurer"    },
	{       " Anti-Hero>",          50  ,"Crusader","Crusader"    },
	{       " Knight>",             58  ,"Hero","Heroin"    },
	{       " Grey Knight>",        68  ,"Hero","Heroin"    },
	{       " Dark Knight>",        80  ,"Dark Knight","Dark Knight"    },
	{       " Black Knight>",       93  ,"Black Knight","Black Knight"    },
	{       " Death Knight>",       102   ,"Death Knight","Death Knight"  },
	{       " Death Knight>",       112   ,"Death Knight","Death Knight"  }
    },
    { /* Psionicist */
	{       "None ",                0   ,"",""   },
	{       " ",         15  ,"death knight","death knight"    },
	{       " ",          19  ,"Adventurer","Adventurer"    },
	{       " ",         24  ,"Adventurer","Adventurer"    },
	{       " ",           30  ,"Adventurer","Adventurer"    },
	{       " ",       37  ,"Adventurer","Adventurer"    },
	{       " ",          	43  ,"Adventurer","Adventurer"    },
	{       " ",       50  ,"Crusader","Crusader"    },
	{       " ",             58  ,"Hero","Heroin"    },
	{       " ",        68  ,"Hero","Heroin"    },
	{       " ",        80  ,"Dark Knight","Dark Knight"    },
	{       " ",       93  ,"Black Knight","Black Knight"    },
	{       " ",    102  ,"Death Knight","Death Knight"  },
	{       " ",        112  ,"Death Knight","Death Knight"  }
    }
};


char *  const   where_name      [][3] =
{
    { "<worn on finger>    ", "You place $p on your finger.", "$n places $p on $s finger." },
    { "<worn on finger>    ", "You place $p on your finger.", "$n places $p on $s finger." },
    { "<worn around neck>  ", "You slip $p around your neck.", "$n slips $p around $s neck." },
    { "<worn around neck>  ", "You slip $p around your neck.", "$n slips $p around $s neck." },
    { "<worn on torso>     ", "You wear $p on your body.", "$n wears $p on $s body." },
    { "<worn on head>      ", "You place $p on your head.", "$n places $p on $s head." },
    { "<worn on legs>      ", "You pull your legs into $p.", "$n pulls $s legs into $p." },
    { "<worn on feet>      ", "You pull your feet into $p.", "$n pulls $s feet into $p." },
    { "<worn on hands>     ", "You pull your hands into $p.", "$n pulls $s hands into $p." },
    { "<worn on arms>      ", "You wear $p on your arms.", "$n wears $p on $s arms." },
    { "<worn about body>   ", "You wear $p about your body.", "$n wears $p about $s body." },
    { "<worn about waist>  ", "You tie $p around your waist.", "$n ties $p around $s waist." },
    { "<worn around wrist> ", "You wear $p on your left wrist.", "$n wears $p on $s left wrist." },
    { "<worn around wrist> ", "You wear $p on your right wrist.", "$n wears $p on $s right wrist." },
    { "<floating nearby>   ", "You release $p and it floats nearby.", "$n releases $p and it floats near $m." },
    { "<right hand>        ", "You wield $p.", "$n wields $p." },
    { "<left hand>         ", "You wield $p.", "$n wields $p." },
    { "<third hand>	    ", "You grasp $p in your third hand.", "$n grasps $p in $s third hand."},
    { "<fourth hand>	    ", "You grasp $p in your fourth hand.", "$n grasps $p in $s fourth hand."} };

/* for do_count */
int max_on = 0;



/*
 * Local functions.
 */
char *  format_obj_to_char      args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void    show_list_to_char       args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
void    show_char_to_char_0     args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void    show_char_to_char_1     args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void    show_char_to_char       args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool    check_blind             args( ( CHAR_DATA *ch ) );
char *  get_who_race		args( ( CHAR_DATA *ch, CHAR_DATA *wch) );


void look_name(CHAR_DATA *ch)
{
	char const * class;
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
	return;
	
	class = race_table[ch->race].name;
	
	if (ch->secondaryrace>0)
	{
		if (ch->sex != SEX_FEMALE)
		  class = clanrank[ch->secondaryrace][ch->generation].male_name;
		else
		  class = clanrank[ch->secondaryrace][ch->generation].female_name;
	}

	    sprintf(buf, " the %s", class);

/* I think this is tacky--whois
	if (!str_cmp(ch->name, "Aurora"))
		sprintf(buf, " the Whore");
	else if (!str_cmp(ch->name, "Cordeau"))
		sprintf(buf, " the Wanker");
	else if (!str_cmp(ch->name, "Kas"))
		sprintf(buf, " the Tosser");
	else if (!str_cmp(ch->name, "Kat"))
		sprintf(buf, " the Bitch");
	else if (!str_cmp(ch->name, "Justicar"))
		sprintf(buf, " the Oaf");
*/
	if (strlen(ch->pcdata->custom_enter_room) > 3)
	    strcpy(buf, ch->pcdata->custom_enter_room);

	set_enter_room_text(ch, buf);
}

void do_balance(CHAR_DATA *ch, char *argument)
{
    int i, j;
    char buf[MAX_STRING_LENGTH];
    
    if (argument[0]=='\0')
    {
        banner_to_char("T H E   B A L A N C E", ch);
        for (j = 1; j < 8; j++)
        {
	    for (i = 0; i < 7; i++)
	    {
		if (balance_info[i].position != j)
		    continue;
		sprintf(buf, "The %s%s - with 0 points",
			secondaryrace[i+1].propname, (i==4) ? "" : "s");
		cent_to_char(buf, ch);		
		sprintf(buf,"Pkills %d, Pkilled %d, Members %d, Av. Generation %d, Av. Score %d",
			balance_info[i].pkills,
			balance_info[i].pkilled,
			balance_info[i].members,
			(balance_info[i].members == 0) ? 0 : balance_info[i].total_gen / balance_info[i].members,
			(balance_info[i].members == 0) ? 0 : balance_info[i].total_score / balance_info[i].members);
		cent_to_char(buf, ch);
		if (j < 7) divide6_to_char(ch);
	    }
	}
	banner_to_char("T H E   B A L A N C E", ch);
    }
}
 
bool obj_is_magical(OBJ_DATA *obj)
{  
    if (IS_OBJ_STAT(obj, ITEM_ARTIFACT)) return FALSE; 
    if (IS_SET(obj->extra_flags3, ITEM_HIDE_MAGIC)) return FALSE;
    if (IS_OBJ_STAT(obj, ITEM_MAGIC)) return TRUE;
    if (IS_OBJ_STAT2(obj, ITEM_NYSTUL)) return TRUE;
    return FALSE;
}		

char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
    ||  (obj->description == NULL || obj->description[0] == '\0'))
	return buf;
    if ( IS_OBJ_STAT2(obj, ITEM_ITEMHIDE)
	&& ch->power[DISC_VAMP_AUSP] >= 5 )   strcat( buf, "(Vamphide) "     );
    if ( IS_OBJ_STAT(obj, ITEM_ARTIFACT)     )   strcat( buf, "(Artifact) "     );
    if ( IS_OBJ_STAT(obj, ITEM_RELIC)     )   strcat( buf, "(Relic) "     );
    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "(Invis) "     );
    if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
	 && IS_OBJ_STAT(obj, ITEM_EVIL)   )   strcat( buf, "(Red Aura) "  );
    if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
    &&  IS_OBJ_STAT(obj,ITEM_BLESS))          strcat(buf,"(Blue Aura) " );
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
	 && obj_is_magical(obj) ) strcat( buf, "(Magical) "   );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "(Glowing) "   );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "(Humming) "   );
    if ( IS_OBJ_STAT(obj, ITEM_FLAMING)   )   strcat( buf, "(Flaming) "   );
    if ( obj->condition < 10 ) strcat(buf, "(V.Damaged) " );
    else if ( obj->condition < 30 ) strcat(buf, "(Damaged) " );

    if (IS_OBJ_STAT2(obj, ITEM_DAEMONSEED)
	&& (IS_SET(ch->flag3, AFF_OBJECT_READING) || ch->level == 120) )
 	strcat(buf, "(Bomb!) " );

    if ( fShort )
    {
	if ( obj->short_descr != NULL )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description != NULL)
	    strcat( buf, obj->description );
    }

    return buf;
}


bool is_vowel(char arg)
{
    if (arg == 'a' || arg == 'e' || arg == 'i'
	|| arg == 'o' || arg == 'u') return TRUE;

    return FALSE;
}

void form_in_room_desc( CHAR_DATA *ch, char *buf )
{
    if (ch->cur_form >= 38 && ch->cur_form <= 50)
    {
	sprintf(buf, form_data[ch->cur_form].in_room_desc, dragon_colour2(ch->dragtype));
	bug(buf, 0);
	bug(dragon_colour2(ch->dragtype), 0);
	return;
    }
    sprintf(buf,form_data[ch->cur_form].in_room_desc);
}

char * GET_EXTENSION(CHAR_DATA *ch)
{
    return "";
    if (IS_NPC(ch)) return "";

    if (ch->secondaryrace == RACE_DAEMON)
        return clanrank[RACE_DAEMON][ch->generation].male_name;
    return "";

}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    output = new_buf();

    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    prgpstrShow = alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    nShow       = 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj )) 
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );

	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if (prgpstrShow[iShow][0] == '\0')
	{
	    free_string(prgpstrShow[iShow]);
	    continue;
	}

	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		add_buf(output,buf);
	    }
	    else
	    {
		add_buf(output,"     ");
	    }
	}
	add_buf(output,prgpstrShow[iShow]);
	add_buf(output,"\n\r");
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }
    page_to_char(buf_string(output),ch);

    /*
     * Clean up.
     */
    free_buf(output);
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return;
}

bool can_see_name( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if (victim->cur_form == 0 || IS_NEWFLAG(victim, AFF_MASKED)) return FALSE;
    if (!can_see(ch, victim) || victim->secondaryrace == 0 || ch == victim || (victim->level > 112 && ch->level < victim->level) )
	return FALSE;
    if (ch->power[DISC_VAMP_AUSP] >= 10 || ch->power[DISC_DRAG_CONJ] >= 5)
	return TRUE;
    if (IS_BASE_FORM(victim) && victim->secondaryrace == ch->secondaryrace)
	return TRUE;
    return FALSE;
}

void name_prefix_to_char( CHAR_DATA *ch, CHAR_DATA *victim)
{
    char buf[MAX_STRING_LENGTH];

    if (!can_see_name(ch, victim)) return;

    sprintf(buf, "(%s) ", GET_PROPER_NAME(victim) );
    stc(buf, ch);
}

void do_vanish(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->affected_by, AFF_INVISIBLE))
    {
	WAIT_STATE(ch, 6);
   	do_visible(ch, "");
	return;
    }

    WAIT_STATE(ch, 6);
    stc("You vanish from sight.\n\r", ch);
    act("$n slowly fades from view.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->affected_by, AFF_INVISIBLE);
}

void do_realm( CHAR_DATA *ch, char *argument )
{
    REALM_DATA *realm;

    if (ch == NULL || ch->in_room == NULL)
	return;

    realm = char_realm( ch );

    stcf(ch, "You are in %s.\n\r%s", realm->name, realm->description);
}

void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH],message[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if (victim->riden_by != NULL) return;

    if (victim->cur_form != 0 && victim->cur_form != FRM_IDIOT)
    {
    buf[0] = '\0';
    if ( is_flying(victim)
	&& victim->position == POS_STANDING) strcat( buf, "(Flying) ");
    if ( is_flying(victim)
	&& victim->position != POS_STANDING) strcat( buf, "(Hovering) ");

    if (can_see_name(ch, victim))
    {
	sprintf(buf2, "(%s) ", GET_PROPER_NAME(victim) );
	strcat(buf, buf2);
    }

    if (victim->cur_form == FRM_MIST && victim->position == POS_STANDING)
    	strcat(buf, "A flowing white mist is here, swirling around the room.");
    else if (victim->cur_form == FRM_MIST)
	strcat(buf, "A flowing white mist");
    else if (victim->cur_form == FRM_WOLF && victim->position == POS_STANDING)
    	strcat(buf, "A ferocious dire wolf is here snarling at you.");
    else if (victim->cur_form == FRM_WOLF)
	strcat(buf, "A ferocious dire wolf");
    else if (victim->cur_form == FRM_BAT && victim->position == POS_STANDING)
    	strcat(buf, "A vampire bat is flying around here.");
    else if (victim->cur_form == FRM_BAT)
	strcat(buf, "A vampire bat");
    else if (victim->cur_form == FRM_DOLPHIN && victim->position == POS_STANDING)
    	strcat(buf, "A beautiful dolphin is swimming around here.");
    else if (victim->cur_form == FRM_DOLPHIN)
	strcat(buf, "A beautiful dolphin");
    else if (victim->cur_form == FRM_FLAME && victim->position == POS_STANDING)
    	strcat(buf, "A roaring pillar of flame stands in the centre of the room.");
    else if (victim->cur_form == FRM_FLAME)
	strcat(buf, "A roaring pillar of flame");
    else if (victim->cur_form == FRM_SPIDER && victim->position == POS_STANDING)
    	strcat(buf, "A hairy little spider is creeping along here.");
    else if (victim->cur_form == FRM_SPIDER)
	strcat(buf, "A hairy little spider");
    else if (victim->cur_form == FRM_LIGHT && victim->position == POS_STANDING)
    	strcat(buf, "A ball of glowing light hangs in mid-air here.");
    else if (victim->cur_form == FRM_LIGHT)
	strcat(buf, "A ball of glowing light");
    else if (victim->cur_form == FRM_COCKROACH && victim->position == POS_STANDING)
    	strcat(buf, "A nasty little cockroach is here, waiting to be squished.");
    else if (victim->cur_form == FRM_COCKROACH)
	strcat(buf, "A nasty little cockroach");
    else if (victim->cur_form == FRM_RABBIT && victim->position == POS_STANDING)
    	strcat(buf, "A cute little bunny rabbit is sitting here.");
    else if (victim->cur_form == FRM_RABBIT)
	strcat(buf, "A cute little bunny rabbit");
    else if (victim->cur_form == FRM_BLOB && victim->position == POS_STANDING)
    	strcat(buf, "A slimy green blob is oozing in a corner.");
    else if (victim->cur_form == FRM_BLOB)
	strcat(buf, "A slimy green blob");
    else if (victim->cur_form == FRM_FROG && victim->position == POS_STANDING)
    	strcat(buf, "A slimy little frog is sitting on the floor.");
    else if (victim->cur_form == FRM_FROG)
	strcat(buf, "A slimy little frog");
    else if (victim->cur_form == FRM_WHIRLWIND && victim->position == POS_STANDING)
    	strcat(buf, "A roaring whirlwind is sweeping you around the room.");
    else if (victim->cur_form == FRM_WHIRLWIND)
	strcat(buf, "A roaring whirlwind");
    else if (victim->cur_form == FRM_CRIMSON && victim->position == POS_STANDING)
    	strcat(buf, "An eerie pinkish mist hangs suspended in the air.");
    else if (victim->cur_form == FRM_CRIMSON)
	strcat(buf, "An eerie pinkish mist");
    else if (victim->cur_form == FRM_HAWK && victim->position == POS_STANDING)
    	strcat(buf, "A hawk flies gracefully overhead.");
    else if (victim->cur_form == FRM_HAWK)
	strcat(buf, "A hawk");
    else if (victim->cur_form != 0)
    {
	if (victim->position == POS_STANDING)
	{
	    char buf2[MAX_STRING_LENGTH];
	    form_in_room_desc(victim, buf2);
	    strcat(buf, buf2);
	}
	else
	{
	    if (is_vowel(form_data[victim->cur_form].lookname[0]))
	        strcat(buf, "An ");
	    else strcat(buf, "A ");
	    strcat(buf, form_data[victim->cur_form].lookname);
	}
    }

    switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, " is lying dead on the floor." );  break;
    case POS_MORTAL:   strcat( buf, " is lying here, mortally wounded." );   break;
    case POS_INCAP:    strcat( buf, " is lying here incapacitated." ); break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." ); break;
    case POS_SLEEPING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],SLEEP_AT))
	    {
		sprintf(message," is sleeping at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],SLEEP_ON))
	    {
		sprintf(message," is sleeping on %s.",
		    victim->on->short_descr); 
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message, " is sleeping in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else 
	    strcat(buf," is sleeping here.");
	break;
    case POS_RESTING:  
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],REST_AT))
	    {
		sprintf(message," is resting at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],REST_ON))
	    {
		sprintf(message," is resting on %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else 
	    {
		sprintf(message, " is resting in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else
	    strcat( buf, " is resting here." );       
	break;
    case POS_SITTING:  
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],SIT_AT))
	    {
		sprintf(message," is sitting at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],SIT_ON))
	    {
		sprintf(message," is sitting on %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message, " is sitting in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else
	    strcat(buf, " is sitting here.");
	break;
    case POS_FIGHTING:
	strcat( buf, " is here"); 
	if (victim->stance > 0)
	{
	    strcat( buf, ", in the " );
	    strcat( buf, stances[victim->stance].name);
	    strcat( buf, " fighting stance");
	}
	strcat( buf, ", fighting " );
	if ( victim->fighting == NULL )
	    strcat( buf, "thin air??" );
	else if ( victim->fighting == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "someone who left??" );
	break;
    }

    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
    }	

    buf[0] = '\0';
    if (IS_SET(victim->comm, COMM_AFK))
	strcat(buf,  "#3(#6>#7AFK#6<#3) #n");
    if ( is_flying(victim)
	&& victim->position == POS_STANDING) strcat( buf, "(Flying) ");
    else if ( is_flying(victim)
	&& victim->position != POS_STANDING) strcat( buf, "(Hovering) ");
    if ( IS_FLAG6(victim, AFF6_SHRINK) ) strcat( buf, "(Shrunk) ");
    if ( IS_FLAG6(victim, AFF6_GROWTH) ) strcat( buf, "(Grown) ");
    if ( IS_FLAG4(victim, AFF_SENESCHIA) ) strcat( buf, "(Leaves) ");
    if ( IS_NEWFLAG(victim, AFF_SHADOW_PLANE) ) strcat( buf, "(Shadow) ");
    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "(Invis) "      );
    if ( IS_NEWFLAG(victim, AFF_IMPROVED_INVIS)) strcat( buf, "(Imp Invis) "      );
    if ( victim->invis_level >= LEVEL_HERO    ) strcat( buf, "(Wizi) "       );
    if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "(Hide) "       );
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "(Translucent) ");
    if ( IS_SET(victim->flag3, AFF_RECEPTACLE) ) strcat( buf, "(Glowing) "  );
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "(Pink Aura) "  );
    if ( IS_AFFECTED(victim, AFF_CONFLAGURATION) ) strcat( buf, "(Demonic Flames) " );
    if ( IS_EVIL(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) strcat( buf, "(Red Aura) "   );
    if ( IS_GOOD(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_GOOD)     ) strcat( buf, "(Golden Aura) ");
    if ( IS_AFFECTED(victim, AFF_SANCTUARY) ||
	  IS_FLAG4(victim, AFF_RETRIBUTION)   ) strcat( buf, "(White Aura) " );


    if ( victim->position == victim->start_pos && victim->long_descr[0] !='\0')
    {
	strcat( buf, victim->long_descr );
	send_to_char( buf, ch );
	return;
    }

    strcat( buf, PERS( victim, ch ) );

    if (victim->riding != NULL)
    {
	strcat( buf, GET_EXTENSION(victim) );
        strcat( buf, " is here, riding on ");
	if (ch == victim->riding)
		strcat(buf,"you");
	else strcat( buf, PERS(victim->riding, ch));
	strcat( buf, ".\n\r");
	send_to_char(buf, ch);
	return;
    }
	
  /*  if ( !IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF) 
         && ch->on == NULL )
	strcat( buf, victim->pcdata->enter_room ); */


    switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, " is lying dead on the floor." );  break;
    case POS_MORTAL:   strcat( buf, " is lying here, mortally wounded." );   break;
    case POS_INCAP:    strcat( buf, " is lying here incapacitated." );      break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." ); break;
    case POS_SLEEPING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],SLEEP_AT))
	    {
		sprintf(message," is sleeping at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],SLEEP_ON))
	    {
		sprintf(message," is sleeping on %s.",
		    victim->on->short_descr); 
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message, " is sleeping in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else 
	    strcat(buf," is sleeping here.");
	break;
    case POS_RESTING:  
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],REST_AT))
	    {
		sprintf(message," is resting at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],REST_ON))
	    {
		sprintf(message," is resting on %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else 
	    {
		sprintf(message, " is resting in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else
	    strcat( buf, " is resting here." );       
	break;
    case POS_SITTING:  
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],SIT_AT))
	    {
		sprintf(message," is sitting at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],SIT_ON))
	    {
		sprintf(message," is sitting on %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message, " is sitting in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else
	    strcat(buf, " is sitting here.");
	break;
    case POS_STANDING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],STAND_AT))
	    {
		sprintf(message," is standing at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],STAND_ON))
	    {
		sprintf(message," is standing on %s.",
		   victim->on->short_descr);
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message," is standing in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else
	    strcat( buf, " is here." );               
	break;
    case POS_FIGHTING:
	strcat( buf, " is here"); 
	if (victim->stance > 0)
	{
	    strcat( buf, ", in the " );
	    strcat( buf, stances[victim->stance].name);
	    strcat( buf, " fighting stance");
	}
	strcat( buf, ", fighting " );
	if ( victim->fighting == NULL )
	    strcat( buf, "thin air??" );
	else if ( victim->fighting == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "someone who left??" );
	break;
    }

    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    /* The extra line descriptions */

    if (IS_NEWFLAG(victim, AFF_WATER_SHIELD))
    {
	send_to_char("   .... surrounded by a flowing shield of water.\n\r",ch);
    }

    if (IS_FLAG5(victim, AFF5_WEB_WEAK) ||
	IS_FLAG5(victim, AFF5_WEB_STRONG) )
    {
	send_to_char("   .... surrounded by sticky webbing.\n\r",ch);
    }

    switch (victim->aura)
    {
	case 1 : send_to_char("   .... surrounded by a shimmering red aura.\n\r",ch);
		 break;
	case 2 : send_to_char("   .... surrounded by a shimmering orange aura.\n\r",ch);
		 break;
	case 3 : send_to_char("   .... surrounded by a shimmering yellow aura.\n\r",ch);
		 break;
	case 4 : send_to_char("   .... surrounded by a shimmering green aura.\n\r",ch);
		 break;
	case 5 : send_to_char("   .... surrounded by a shimmering blue aura.\n\r",ch);
		 break;
	case 6 : send_to_char("   .... surrounded by a shimmering indigo aura.\n\r",ch);
		 break;
	case 7 : send_to_char("   .... surrounded by a shimmering violet aura.\n\r",ch);
		 break;
	case 8 : send_to_char("   .... surrounded by a shimmering brown aura.\n\r",ch);
		 break;
	case 9 : send_to_char("   .... surrounded by a shimmering silvery aura.\n\r",ch);
		 break;
	case 10 : send_to_char("   .... surrounded by a shimmering golden aura.\n\r",ch);
		 break;
    }

    return;
}




void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;
    bool special = FALSE;

    if ( can_see( victim, ch ) )
    {
	if (ch == victim)
	    act( "$n looks at $mself.",ch,NULL,NULL,TO_ROOM);
	else
	{
	    act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	    act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
	}
    }

    if ( victim->description[0] != '\0' )
    {
	send_to_char( victim->description, ch );
	special = TRUE;
    }
    if (IS_SET(victim->flag2, AFF_BITE))
    {
	act("$N has a pair of pointed fangs extending from $S gums.", ch, NULL, victim, TO_CHAR);
	special = TRUE;
    }
    if (IS_SET(victim->flag2, AFF_CLAW))
    {
	act("$N has sharp claws extending from $S fingers.", ch, NULL, victim, TO_CHAR);
	special = TRUE;
    }
    if (IS_SET(victim->flag4, AFF_WINGS) || IS_SET(victim->flag2, AFF_WING))
    {
        if (victim->secondaryrace == RACE_DAEMON)
	    act("$N has a pair of leathery wings extending from $S back.", ch, NULL, victim, TO_CHAR);
	else
	    act("$N has a beautiful pair of wings extending from $S back.", ch, NULL, victim, TO_CHAR);
	special = TRUE;
    }
    if (IS_SET(victim->flag5, AFF5_MAW))
    {
	act("$N has a gaping maw full of sharp teeth.", ch, NULL, victim, TO_CHAR);
	special = TRUE;
    }
    if (!special)
    {
	act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS(victim, ch) );

    if (percent >= 100) 
	strcat( buf, " is in excellent condition.\n\r");
    else if (percent >= 90) 
	strcat( buf, " has a few scratches.\n\r");
    else if (percent >= 75) 
	strcat( buf," has some small wounds and bruises.\n\r");
    else if (percent >=  50) 
	strcat( buf, " has quite a few wounds.\n\r");
    else if (percent >= 30)
	strcat( buf, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
	strcat ( buf, " looks pretty hurt.\n\r");
    else if (percent >= 0 )
	strcat (buf, " is in awful condition.\n\r");
    else
	strcat(buf, " is bleeding to death.\n\r");

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear][0], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
    }

    if ( victim != ch
    &&   !IS_NPC(ch)
    &&   number_percent( ) < get_skill(ch,gsn_peek))
    {
	send_to_char( "\n\rYou peek at their inventory:\n\r", ch );
	check_improve(ch,gsn_peek,TRUE,4);
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

    /* The evil eye.   Oooooh nasty. */
    if (!IS_NPC(victim) && victim->pcdata->evileye != NULL)
    {
	if (ch->power[DISC_VAMP_PRES] >= 7 || ch->level >= 112)
	    stc("You are unaffected by the curse of the evil eye.\n\r", ch);
	else    
	{
	    stc("You feel the curse of the evil eye upon you.\n\r", ch);
	    interpret(ch, victim->pcdata->evileye);
	}
    }

    return;
}

void do_peek( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if (argument == '\0')
    {
	send_to_char("Who do you wish to peek at?\n\r", ch);
	return;
    }

    if ((victim = get_char_room( ch, argument )) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim == ch)
    {
	stc("You can't peek at yourself.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 8);

    if (number_percent( ) < get_skill(ch,gsn_peek))
    {
	send_to_char( "You peek at their inventory:\n\r", ch );
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }
    else
    {
	check_improve(ch,gsn_peek,FALSE,4);
	stc("You failed to get a good look.\n\r", ch);
    }
}

void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch || rch->altitude != ch->altitude)
	    continue;

	if ( get_trust(ch) < rch->invis_level)
	    continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
	else if ( ( room_is_dark( ch->in_room )
		   && !can_see_in_dark(ch) )
	   	|| ( IS_NEWFLAG(rch, AFF_SHADOW_PLANE)
		   && !can_see_shadows(ch) ) )
	{
	    send_to_char( "You see glowing red eyes watching YOU!\n\r", ch );
	}
    }

    return;
} 



bool check_blind( CHAR_DATA *ch )
{

    if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    { 
	send_to_char( "You can't see a thing!\n\r", ch ); 
	return FALSE; 
    }

    return TRUE;
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (ch->lines == 0)
	    send_to_char("You do not page long messages.\n\r",ch);
	else
	{
	    sprintf(buf,"You currently display %d lines per page.\n\r",
		    ch->lines + 2);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("You must provide a number.\n\r",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
	send_to_char("Paging disabled.\n\r",ch);
	ch->lines = 0;
	return;
    }

    if (lines < 10 || lines > 50)
    {
	send_to_char("You must provide a reasonable number.\n\r",ch);
	return;
    }

    sprintf(buf,"Scroll set to %d lines.\n\r",lines);
    send_to_char(buf,ch);
    ch->lines = lines - 2;
}

void set_form(CHAR_DATA *ch, sh_int form)
{
    char shortdesc[MAX_STRING_LENGTH];
    char name[MAX_STRING_LENGTH];
    int oldform;

    if (is_item(ch)) return;

    if (form == 0 && ch->secondaryrace == RACE_DRAGON)
    {
        form = 37 + ch->generation;
	if (ch->generation < 1 || ch->generation > 13) form = 0;
    }

    oldform = form;
    ch->max_hit -= form_data[ch->cur_form].hit_mod;
    ch->xhitroll -= form_data[ch->cur_form].hit;
    ch->xdamroll -= form_data[ch->cur_form].dam;
    ch->armor[0] -= form_data[ch->cur_form].ac;
    ch->armor[1] -= form_data[ch->cur_form].ac;
    ch->armor[2] -= form_data[ch->cur_form].ac;
    ch->armor[3] -= form_data[ch->cur_form].ac;
    ch->cur_form = form;
    ch->max_hit += form_data[ch->cur_form].hit_mod;
    ch->xhitroll += form_data[ch->cur_form].hit;
    ch->xdamroll += form_data[ch->cur_form].dam;
    ch->armor[0] += form_data[ch->cur_form].ac;
    ch->armor[1] += form_data[ch->cur_form].ac;
    ch->armor[2] += form_data[ch->cur_form].ac;
    ch->armor[3] += form_data[ch->cur_form].ac;
    if (ch->secondaryrace == RACE_ELADRIN) ch->body_form = 0;
    if (form >= 38 && form <=50)
    {
	if (form_data[form].short_desc != NULL
		&& form_data[form].short_desc[0] != '\0')
	sprintf(shortdesc, form_data[ch->cur_form].short_desc, dragon_colour2(ch->dragtype));
	else shortdesc[0] = 0;
    }
    else
    {
	if (form_data[form].short_desc != NULL
		&& form_data[form].short_desc[0] != '\0')
	sprintf(shortdesc, form_data[ch->cur_form].short_desc);
	else shortdesc[0] = 0;
    }
    sprintf(name, form_data[ch->cur_form].name);
    set_enter_room_text(ch, name);
    if (ch->cur_form == 0)
    {
	look_name(ch);
	set_pc_name(ch, GET_PROPER_NAME(ch));
    }
    else
	if (shortdesc != NULL
		&& shortdesc[0] != '\0')
	set_pc_name(ch, shortdesc);

    if (ch->cur_form != 0 && !form_data[ch->cur_form].can_wear)
    {
	OBJ_DATA *obj;

	do_sremove(ch, "all");
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{
	    SET_BIT(obj->extra_flags2, ITEM_FORM_MELDED);
	}
    }
    else
    {
	OBJ_DATA *obj;

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{
	    REMOVE_BIT(obj->extra_flags2, ITEM_FORM_MELDED);
	}
 	recalc_wear(ch);
    }


    if (ch->cur_form == FRM_WOLF)
    {
	SET_BIT(ch->flag2, AFF_BITE);
	SET_BIT(ch->flag2, AFF_CLAW);
    }
    else if (!( oldform == 0
	  && ch->cur_form == 0))
    {
	REMOVE_BIT(ch->flag2, AFF_BITE);
	REMOVE_BIT(ch->flag2, AFF_CLAW);
    }

    save_char_obj(ch);
}

int get_size(CHAR_DATA *ch)
{
    int size = -1;

    if (ch->cur_form != 0) size = form_data[ch->cur_form].size;

    else if (ch->secondaryrace == 0 && !IS_NPC(ch))
    {
	if (ch->race == RACE_GNOME) size = SIZE_SMALL;
	else size = SIZE_MEDIUM;
    }
    else if (ch->secondaryrace == RACE_ELADRIN)
    {
	if (ch->generation < 3) size = SIZE_SMALL;
	else size = SIZE_MEDIUM;
    }
    else if (ch->secondaryrace == RACE_DRAGON)
    {
	if (ch->generation > 9) size = SIZE_GIANT;
	else if (ch->generation > 7) size =SIZE_HUGE;
	else if (ch->generation > 3) size = SIZE_LARGE;
    }
    else if (ch->secondaryrace == RACE_DAEMON)
    {
	if (ch->generation == 10) size = SIZE_LARGE;
	else if (ch->generation > 10) size = SIZE_HUGE;
    }
    else if (ch->secondaryrace == RACE_VAMPIRE
	    || ch->secondaryrace == RACE_HERO
	    || ch->secondaryrace == RACE_WARLOCK
	    || ch->secondaryrace == RACE_LUPINE)
    {
	size = SIZE_MEDIUM;
    }
    else size = ch->size;

    /* Magical spells to alter size */
    if (IS_FLAG6(ch, AFF6_SHRINK)) size--;
    else if (IS_FLAG6(ch, AFF6_GROWTH)) size++;

    if (size < SIZE_TINY) size = SIZE_TINY;
    else if (size > SIZE_GIANT) size = SIZE_GIANT;

    return size;
}

void do_socials(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;
     
    col = 0;
   
    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
	sprintf(buf,"%-12s",social_table[iSocial].name);
	send_to_char(buf,ch);
	if (++col % 6 == 0)
	    send_to_char("\n\r",ch);
    }

    if ( col % 6 != 0)
	send_to_char("\n\r",ch);
    return;
}


 
void do_motd(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"motd");
}

void do_message(CHAR_DATA *ch, char *argument)
{
    free_string(ch->tempprompt);
    ch->tempprompt = NULL;
    do_help(ch,"message");
}

void do_imotd(CHAR_DATA *ch, char *argument)
{  
    do_help(ch,"imotd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"rules");
}

void do_story(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"story");
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"wizlist");
}

void do_autolist(CHAR_DATA *ch, char *argument)
{
    /* lists most player flags */
    if (IS_NPC(ch))
      return;

    stc("Remember if you are in a safe room you can now use automenu instead.\n\n\r", ch);
    send_to_char("   action     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);
 
    send_to_char("autoassist     ",ch);
    if (IS_SET(ch->act,PLR_AUTOASSIST))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch); 

    send_to_char("autoexit       ",ch);
    if (IS_SET(ch->act,PLR_AUTOEXIT))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("autogold       ",ch);
    if (IS_SET(ch->act,PLR_AUTOGOLD))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("autoloot       ",ch);
    if (IS_SET(ch->act,PLR_AUTOLOOT))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("autosac        ",ch);
    if (IS_SET(ch->act,PLR_AUTOSAC))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("autosplit      ",ch);
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("compact mode   ",ch);
    if (IS_SET(ch->comm,COMM_COMPACT))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("prompt         ",ch);
    if (IS_SET(ch->comm,COMM_PROMPT))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("combine items  ",ch);
    if (IS_SET(ch->comm,COMM_COMBINE))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    if (!IS_SET(ch->act,PLR_CANLOOT))
	send_to_char("Your corpse is safe from thieves.\n\r",ch);
    else 
	send_to_char("Your corpse may be looted.\n\r",ch);

    if (IS_SET(ch->act,PLR_NOSUMMON))
	send_to_char("You cannot be summoned.\n\r",ch);
    else
	send_to_char("You can be summoned.\n\r",ch);
   
    if (IS_SET(ch->act,PLR_NOFOLLOW))
	send_to_char("You do not welcome followers.\n\r",ch);
    else
	send_to_char("You accept followers.\n\r",ch);
}

void do_autoassist(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    
    if (IS_SET(ch->act,PLR_AUTOASSIST))
    {
      send_to_char("Autoassist removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOASSIST);
    }
    else
    {
      send_to_char("You will now assist when needed.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOASSIST);
    }
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->act,PLR_AUTOEXIT))
    {
      send_to_char("Exits will no longer be displayed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOEXIT);
    }
    else
    {
      send_to_char("Exits will now be displayed.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOEXIT);
    }
}

void do_autoscan(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->act,PLR_AUTOSCAN))
    {
      send_to_char("Autoscan will no longer be displayed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSCAN);
    }
    else
    {
      send_to_char("Autoscan will now be displayed.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSCAN);
    }
}

void do_autogold(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOGOLD))
    {
      send_to_char("Autogold removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOGOLD);
    }
    else
    {
      send_to_char("Automatic gold looting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOGOLD);
    }
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOLOOT))
    {
      send_to_char("Autolooting removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOLOOT);
    }
    else
    {
      send_to_char("Automatic corpse looting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOLOOT);
    }
}

void do_fullauto(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    send_to_char("Automatic corpse looting set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOLOOT);
    send_to_char("Automatic gold looting set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOGOLD);
    send_to_char("Exits will now be displayed.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOEXIT);
    send_to_char("You will now assist when needed.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOASSIST);
    send_to_char("Automatic corpse sacrificing set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOSAC);
    send_to_char("Automatic gold splitting set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOSPLIT);
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSAC))
    {
      send_to_char("Autosacrificing removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSAC);
    }
    else
    {
      send_to_char("Automatic corpse sacrificing set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSAC);
    }
}

void do_autosplit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
    {
      send_to_char("Autosplitting removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
    }
    else
    {
      send_to_char("Automatic gold splitting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSPLIT);
    }
}

void do_brief(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_BRIEF))
    {
      send_to_char("Full descriptions activated.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_BRIEF);
    }
    else
    {
      send_to_char("Short descriptions activated.\n\r",ch);
      SET_BIT(ch->comm,COMM_BRIEF);
    }
}

void do_compact(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMPACT))
    {
      send_to_char("Compact mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMPACT);
    }
    else
    {
      send_to_char("Compact mode set.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMPACT);
    }
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];

   if ( argument[0] == '\0' )
   {
	if (IS_SET(ch->comm,COMM_PROMPT))
	{
	    send_to_char("You will no longer see prompts.\n\r",ch);
	    REMOVE_BIT(ch->comm,COMM_PROMPT);
	}
	else
	{
	    send_to_char("You will now see prompts.\n\r",ch);
	    SET_BIT(ch->comm,COMM_PROMPT);
	}
       return;
   }
 
   if( !strcmp( argument, "all" ) )
      strcpy( buf, "<%hhp %vmv %ppsp> ");
   else if (!strcmp(argument, "god" ) && ch->level > 111)
      strcpy(buf, "Vnum:%R, Area:%z, Exits: %e%c<%h/%Hhp %v/%Vmv>%c> ");
   else
   {
      if ( strlen(argument) > 70 )
	 argument[70] = '\0';
      strcpy( buf, argument );
      smash_tilde( buf );
      if (str_suffix("%c",buf))
	strcat(buf," ");
   }
 
   free_string( ch->prompt );
   ch->prompt = str_dup( buf );
   sprintf(buf,"Prompt set to %s\n\r",ch->prompt );
   send_to_char(buf,ch);
   return;
}

void do_combine(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMBINE))
    {
      send_to_char("Long inventory selected.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMBINE);
    }
    else
    {
      send_to_char("Combined inventory selected.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMBINE);
    }
}


void do_nofollow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_NOFOLLOW))
    {
      send_to_char("You now accept followers.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    }
    else
    {
      send_to_char("You no longer accept followers.\n\r",ch);
      SET_BIT(ch->act,PLR_NOFOLLOW);
      die_follower( ch );
    }
}

void do_nosummon(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
      if (IS_SET(ch->imm_flags,IMM_SUMMON))
      {
	send_to_char("You are no longer immune to summon.\n\r",ch);
	REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
      }
      else
      {
	send_to_char("You are now immune to summoning.\n\r",ch);
	SET_BIT(ch->imm_flags,IMM_SUMMON);
      }
    }
    else
    {
      if (IS_SET(ch->act,PLR_NOSUMMON))
      {
	send_to_char("You are no longer immune to summon.\n\r",ch);
	REMOVE_BIT(ch->act,PLR_NOSUMMON);
      }
      else
      {
	send_to_char("You are now immune to summoning.\n\r",ch);
	SET_BIT(ch->act,PLR_NOSUMMON);
      }
    }
}

void show_gore_to_char (CHAR_DATA *ch)
{
    int gore;
    char buffer[MAX_INPUT_LENGTH];
    char *buf;

    buffer[0] = '#';
    buffer[1] = '1';
    buf = &buffer[2];
    buf[0] = '\0';

    gore = ch->in_room->gore + 3;

    if (ch->in_room->sector_type == SECT_FOREST)
    {
	if (gore > 60)
	sprintf(buf, "You are wading through scenes of absolute carnage");
	else if (gore > 40)
	sprintf(buf, "The forest floor is littered with the remains of body parts");
	else if (gore > 20)
	sprintf(buf, "Blood and entrails coat the floor of the forest");
	else if (gore > 15)
	sprintf(buf, "Blood is splattered all over the floor here");
	else if (gore > 10)
	sprintf(buf, "You see a few pools of blood lying on the forest floor");
	else if (gore > 8)
	sprintf(buf, "In places, blood has soaked into the forest floor");
	else if (gore > 5)
	sprintf(buf, "You see a few drops of blood on the floor here");
    }
    else if (ch->in_room->sector_type == SECT_DESERT
		|| ch->in_room->sector_type == SECT_CITY
		|| ch->in_room->sector_type == SECT_NARROW_TUNNEL
		|| ch->in_room->sector_type == SECT_INSIDE
		|| ch->in_room->sector_type == SECT_HOME)
    {
	if (gore > 60)
	sprintf(buf, "You are wading through scenes of absolute carnage");
	else if (gore > 40)
	sprintf(buf, "You are surrounded by twitching corpses and severed limbs");
	else if (gore > 20)
	sprintf(buf, "Mangled body parts are strewn around here");
	else if (gore > 15)
	sprintf(buf, "Blood is splattered all over the floor here");
	else if (gore > 10)
	sprintf(buf, "You see a few pools of blood lying on the floor");
	else if (gore > 8)
	sprintf(buf, "In places, blood gathers in pools on the floor");
	else if (gore > 5)
	sprintf(buf, "You see a few drops of blood on the floor here");
    }
    else if (ch->in_room->sector_type == SECT_FIELD
		|| ch->in_room->sector_type == SECT_HILLS)
    {
	if (gore > 60)
	sprintf(buf, "The floor is two foot deep in the dripping and twitching corpses of enemies");
	else if (gore > 40)
	sprintf(buf, "You are surrounded by twitching corpses and severed limbs");
	else if (gore > 20)
	sprintf(buf, "Mangled body parts are strewn around the fields");
	else if (gore > 15)
	sprintf(buf, "The grass here is stained red from battle");
	else if (gore > 10)
	sprintf(buf, "You see a many pools of blood gathering on the grass");
	else if (gore > 8)
	sprintf(buf, "You see a few signs of battle here, corpses and limbs lie strewn around");
	else if (gore > 5)
	sprintf(buf, "You see a few drops of blood on the grass here");
    }
    else if (ch->in_room->sector_type == SECT_UNDERWATER
		|| ch->in_room->sector_type == SECT_WATER_SWIM
		|| ch->in_room->sector_type == SECT_WATER_NOSWIM)
    {
	if (gore > 60)
	sprintf(buf, "The water is clustered by the dripping and twitching corpses of enemies");
	else if (gore > 40)
	sprintf(buf, "You are surrounded by twitching corpses and severed limbs");
	else if (gore > 20)
	sprintf(buf, "Mangled body parts are floating around here");
	else if (gore > 15)
	sprintf(buf, "The water is red, and body parts float around you");
	else if (gore > 10)
	sprintf(buf, "You see a hazy red mist forming around you, blood staining the water red.");
	else if (gore > 8)
	sprintf(buf, "The water is stained red in places from spilt blood");
	else if (gore > 5)
	sprintf(buf, "A few drops of blood are floating in the water");
    }
    if (buf[0] == '\0') return;

    strcat(buf, ".\n\r");
    send_to_char(buffer, ch);
}

void do_look( CHAR_DATA *ch, char *argument )
{
    char liqu [MAX_STRING_LENGTH];
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door;
    int number,count;
    int totworth, numcoins, aveworth;

    if (ch->sw_obj != NULL && ch->sw_obj->in_obj != NULL)
    {
	stcf(ch, "#bInside %s.\n\r", ch->sw_obj->in_obj->short_descr);
	stc("  Its very claustrophobic in here.\n\n\r", ch);
	
	for (obj = ch->sw_obj->in_obj->contains ; obj != NULL ; obj = obj->next_content)
 	{
	    if (can_see_obj(ch, obj))
	    {
	  	stcf(ch, "    You see %s in here with you.\n\r", 
		    obj->short_descr);
	    }
	}
	return;
    }


    if (argument[0] != '%' && ch->auto_look_line != -1)
    {
	stc("Not yet.\n\r", ch);
	return;
    }

    if (ch->in_room != NULL
	&& ch->in_room->auto_look != NULL
	&& !IS_FLAG5(ch, AFF5_BUILDING))
    {
	AUTO_LOOK_DATA *ald;
	int cnt;

	ald = ch->in_room->auto_look;
	if (ch->auto_look_line == -1)
	    ch->auto_look_line = 0;
	else for (cnt = 0; cnt < ch->auto_look_line; cnt++)
	{
	    if (ald->next == NULL)
	    {
		ch->auto_look_line = -1;
		return;
	    }
	    ald = ald->next;
	}
	ch->auto_look_line += 1;
	if (ald->text[0] == '$')
	{
	    char_from_room( ch );
	    char_to_room( ch, get_room_index( char_realm(ch), atoi(&(ald->text[1])) ), 0 );
	    ch->auto_look_line = -1;
	    return;
	}
	if (ald->text[0] == '%')
	{
	    char_from_room( ch );
	    char_to_room( ch, get_room_index( char_realm(ch), atoi(&(ald->text[1])) ), 0 );
	    ch->auto_look_line = -1;
	    do_look(ch, "auto");
	    return;
	}
	stc(ald->text, ch);
	return;
    }

    if ( ch->desc == NULL )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !RTIME_UP(ch->in_room, RTIMER_STINKING_CLOUD) && ch->altitude == 0)
    {
	send_to_char("You can't see a thing in this green cloud.\n\r", ch);
	return;
    }

    if ( !RTIME_UP(ch->in_room, RTIMER_SWARM_BEES) && ch->altitude == 0)
    {
	send_to_char("There are too many bees buzzing everywhere.\n\r", ch);
	return;
    }

    if ( !RTIME_UP(ch->in_room, RTIMER_SWARM_BATS) && ch->altitude == 0)
    {
	send_to_char("There are too many bats flapping everywhere.\n\r", ch);
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   IS_NEWFLAG(ch, AFF_SHADOW_PLANE) )
    {
	send_to_char("The Shadow Plane\n\r",ch);
	send_to_char("You are in a vast expanse of shifting and moving shadows.  Everything is\n\rformless, and has no meaning, yet is strangely familar to your eyes.\n\r", ch);
	send_to_char("You recognise none, yet recognise all. Understand none, yet\n\runderstand all.  The plane of the shadows is unfamiliar yet your home.\n\r",ch);
	show_char_to_char(ch->in_room->people, ch);
	return;
    }

    if ( IS_OUTSIDE(ch) && IS_DAY() && ch->tertiaryrace == TERTIARY_DROW )
    {
	stc("Your eyes are burning from the blinding white light.\n\r", ch);
	return;
    }
    else if ( room_is_dark(ch->in_room)
	 && weird_weather_type == WEIRD_WEATHER_AURORA)
	stc("Weird glowing lights illuminate the room.\n\r", ch);

    else if ( !can_see_in_dark(ch) && room_is_dark(ch->in_room) )
    {
	send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->people, ch );
	return;

	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
	{
	    send_to_char("\n\r",ch);
	    do_exits( ch, "auto" );
	}
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    count = 0;

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	/* 'look' or 'look auto' */
	if (ch->altitude > 5)
	{
	    sprintf(buf, "#bYou are %d feet in the air.\n\r",ch->altitude * 50);
	    send_to_char(buf, ch);
	}
	else
	{
	    if (ch->altitude > 0)
	    {
	    	sprintf(buf, "#bYou are %d feet above ", ch->altitude * 50);
		send_to_char(buf, ch);
	    }
	    sprintf(buf, "#b%c%s#n",
		(ch->altitude > 0) ? LOWER(ch->in_room->name[0]) : ch->in_room->name[0],
		&ch->in_room->name[1]);
	    send_to_char( buf, ch );
	}

	if ((CAN_BUILD(ch) && (IS_NPC(ch) || IS_SET(ch->act,PLR_HOLYLIGHT)))
		|| (ch->pcdata != NULL && ch->pcdata->home == ch->in_room->vnum)
		|| (ch->clan == ch->in_room->area->clan && is_clan_leader(ch)))
	{
	    sprintf(buf," [Room %d]",ch->in_room->vnum);
	    send_to_char(buf,ch);
	}

	if (IS_OUTSIDE(ch) && ch->altitude < 6)
	{
		sprintf(buf, "  :   It is %s", daytime[time_info.hour]);
		send_to_char(buf, ch);
        }

	else if (IS_OUTSIDE(ch))
	{
		sprintf(buf, "    It is %s", daytime[time_info.hour]);
		send_to_char(buf, ch);
        }

	send_to_char( "\n\r", ch );

	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) && !IS_SET(ch->flag4, AFF_AUTOWALKING) ) )
	{
//	    removed for looking shit.
//	    send_to_char( "  ",ch);
	    if (ch->altitude == 0
		&& (!IS_NIGHT() || ch->in_room->night_description == NULL || ch->in_room->night_description[0] == 0) )
	    {	
		send_to_char( ch->in_room->description, ch );
	    }
	    else if (ch->altitude == 0 && IS_NIGHT() )
	    {	
		send_to_char( ch->in_room->night_description, ch );
	    }
	    else
	    {
		if (ch->altitude < 6)
		{
		    send_to_char("You are flying ", ch);
		    switch (ch->in_room->sector_type)
		    {
			case SECT_CITY          : send_to_char("above a vast city",ch); break;
			case SECT_FIELD         : send_to_char("above a field",ch); break;
			case SECT_FOREST        : send_to_char("above a forest", ch); break;
			case SECT_HILLS         : send_to_char("above hills",ch);break;
			case SECT_MOUNTAIN      : send_to_char("above a mountain",ch); break;
			case SECT_WATER_SWIM    : send_to_char("above a stream",ch); break;
			case SECT_WATER_NOSWIM  : send_to_char("above water",ch); break;
			case SECT_UNDERWATER    : send_to_char("above water",ch);break;
			case SECT_AIR           : send_to_char("high in the air",ch);break;
			case SECT_DESERT        : send_to_char("above a desert",ch);break;
			case SECT_NARROW_TUNNEL : send_to_char("high above the ground", ch);break;
			case SECT_INSIDE        : send_to_char("above a building", ch);break;
			case SECT_HOME          : send_to_char("above a building", ch);break;
			default  : send_to_char("in the air", ch);break;
		    }
		}
		else send_to_char("You are flying very high in the air",ch);
		send_to_char(".\n\r",ch);
	    }
	}

	show_gore_to_char(ch);

	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
	{
	    send_to_char("\n\r",ch);
	    do_exits( ch, "auto" );
	}

	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOSCAN) )
	{
	    CHAR_DATA *wch = NULL;
	    int door, count = 0;
	    bool found = FALSE;


	    for (door = 0; door < 6; door++)
	    {
		if (ch->in_room == NULL
		    || ch->in_room->exit[door] == NULL
		    || ch->in_room->exit[door]->u1.to_room == NULL
		    || ch->in_room->exit[door]->u1.to_room->people == NULL)
			continue;

		count = 0;
		for (wch = ch->in_room->exit[door]->u1.to_room->people; wch != NULL; wch = wch->next_in_room)
		{
		    if (wch->altitude == ch->altitude)
		    	count++;
		}
		if (!found)
	            send_to_char("#b[You see life to the: ",ch);
		sprintf(buf, "%s%s%s%s",
		    (found) ? " " : "",
		    (count > 10) ? "#b>>" : (count > 5) ? "#b>" : (count > 1) ? "#b-" : "#b",
		    exitname[door],
		    (count > 10) ? "<<" : (count > 5) ? "<" : (count > 1) ? "-" : "" );
		send_to_char(buf, ch);
		found = TRUE;
	    }

	    if (found)
		send_to_char("]\n\r",ch);
	    else
		send_to_char("#b[ You see no life near by ]\n\r",ch);
	}


	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );

	/* This is the nexus bit */
	if (!RTIME_UP(ch->in_room, RTIMER_NEXUS_FLAME) && ch->altitude == 0)
	    send_to_char("     A ring of fire hangs in the air.\n\r", ch);
	if (!RTIME_UP(ch->in_room, RTIMER_NEXUS_WATER) && ch->altitude == 0)
	    send_to_char("     A ring of water hangs in the air.\n\r", ch);
	if (!RTIME_UP(ch->in_room, RTIMER_NEXUS_AIR) && ch->altitude == 0)
	    send_to_char("     A spinning whirwind hangs in the air.\n\r", ch);
	if (!RTIME_UP(ch->in_room, RTIMER_NEXUS_EARTH) && ch->altitude == 0)
	    send_to_char("     A ring of leaves floats in the air.\n\r", ch);
	if (!RTIME_UP(ch->in_room, RTIMER_NEXUS_ENTROPY) && ch->altitude == 0)
	    send_to_char("     A circle of blackness hangs in the air.\n\r", ch);


	/* show dem stinky clouds and uther thingz */

	if (!RTIME_UP(ch->in_room, RTIMER_SWARM_RATS) && ch->altitude == 0)
	    send_to_char("     You see hundreds of rats swarming all over you.\n\r", ch);

	if (!RTIME_UP(ch->in_room, RTIMER_GHOST_LIGHT))
	    send_to_char("     Shrieking white spirits howl around the room.\n\r", ch);

	if (!RTIME_UP(ch->in_room, RTIMER_LIFE_VORTEX))
	    send_to_char("     You see a spinning vortex of magical light here.\n\r", ch);

	if (!RTIME_UP(ch->in_room, RTIMER_DEATH_VORTEX))
	    send_to_char("     You see a spinning vortex of magical blackness here.\n\r", ch);


	for (door=0 ; door < 6 ; door++)
	{
	    if (ch->in_room->exit[door] != NULL
		&& !RTIME_UP(ch->in_room->exit[door]->u1.to_room, 
			RTIMER_STINKING_CLOUD))
	    {
		sprintf(buf, "     You see poisonous green vapours %s.\n\r", exitname2[door]);
		send_to_char(buf, ch);
	    }
	}


	/* show dem magic walls */
	for (door=0 ; door < 6 ; door++)
	{	
	    if (ch->altitude != 0) continue;
	    if (ch->in_room == NULL) continue;
	    if (ch->in_room->exit[door] == NULL) continue;

	    if (IS_SET(ch->in_room->exit[door]->exit_info, EX_ICE_WALL)
		&& can_see_room(ch, ch->in_room->exit[door]->u1.to_room) )
	    {
		sprintf(buf, "     You see a glacier of ice %s.\n\r", exitname2[door]);
	        send_to_char(buf, ch);
	    }
	    else if (IS_SET(ch->in_room->exit[door]->exit_info,EX_CALTROP_WALL)
		&& can_see_room(ch, ch->in_room->exit[door]->u1.to_room) )
	    {
		sprintf(buf, "     You see a wall of caltrops %s.\n\r", exitname2[door]);
	        send_to_char(buf, ch);
	    }
	    else if (IS_SET(ch->in_room->exit[door]->exit_info,EX_FIRE_WALL)
		&& can_see_room(ch, ch->in_room->exit[door]->u1.to_room) )
	    {
		sprintf(buf, "     You see a blazing wall of fire %s.\n\r", exitname2[door]);
	        send_to_char(buf, ch);
	    }
	    else if (IS_SET(ch->in_room->exit[door]->exit_info,EX_SWORD_WALL)
		&& can_see_room(ch, ch->in_room->exit[door]->u1.to_room) )
	    {
		sprintf(buf, "     You see a spinning wall of swords %s.\n\r", exitname2[door]);
	        send_to_char(buf, ch);
	    }
	    else if (IS_SET(ch->in_room->exit[door]->exit_info,EX_MUSHROOM_WALL)
		&& can_see_room(ch, ch->in_room->exit[door]->u1.to_room) )
	    {
		sprintf(buf, "     You see a vibrating mound of mushrooms %s.\n\r", exitname2[door]);
	        send_to_char(buf, ch);
	    }
	    else if (IS_SET(ch->in_room->exit[door]->exit_info,EX_PRISMATIC_WALL)
		&& can_see_room(ch, ch->in_room->exit[door]->u1.to_room) )
	    {
		sprintf(buf, "     You see a shimmering wall of many colours %s.\n\r", exitname2[door]);
	        send_to_char(buf, ch);
	    }
	    else if (IS_SET(ch->in_room->exit[door]->exit_info, EX_IRON_WALL)
		&& can_see_room(ch, ch->in_room->exit[door]->u1.to_room) )
	    {
		sprintf(buf, "    You see a solid wall of iron %s.\n\r", exitname2[door]);
	        send_to_char(buf, ch);
	    }
	    else if (IS_SET(ch->in_room->exit[door]->exit_info, EX_ASH_WALL)
		&& can_see_room(ch, ch->in_room->exit[door]->u1.to_room) )
	    {
		sprintf(buf, "    You see a deadly wall of ash %s.\n\r", exitname2[door]);
	        send_to_char(buf, ch);
	    }
	}

        /* show all the treasure */
	totworth = (ch->in_room->diamonds  * 100000
		+   ch->in_room->sapphires * 10000
		+   ch->in_room->emeralds  * 1000
		+   ch->in_room->gold      * 100
		+   ch->in_room->silver    * 10
		+   ch->in_room->copper );

	if (totworth > 0)
	{
	    numcoins = (ch->in_room->diamonds
		+   ch->in_room->sapphires
		+   ch->in_room->emeralds
		+   ch->in_room->gold
		+   ch->in_room->silver
		+   ch->in_room->copper);
	      
            sprintf( buf, "You see ");

	    if ( numcoins > 10000) 
	    {
	      strcat(buf, "a gigantic pile of ");
	    }
	    else if ( numcoins > 5000) 
	    {
	      strcat(buf, "an enormous pile of ");
	    }
	    else if ( numcoins > 1000) 
	    {
	      strcat(buf, "a massive pile of ");
	    }
	    else if ( numcoins > 500) 
	    {
	      strcat(buf, "a large pile of ");
	    }
	    else if ( numcoins > 100) 
	    {
	      strcat(buf, "a pile of ");
	    }
	    else if ( numcoins > 50) 
	    {
	      strcat(buf, "a scattering of ");
	    }
	    else if ( numcoins > 20) 
	    {
	      strcat(buf, "a small amount of ");
	    }
	    else if ( numcoins > 10) 
	    {
	      strcat(buf, "a very few ");
	    }
	    else if ( numcoins == 1) 
	    {
	      strcat(buf, "a single ");
	    }

	    if (numcoins == 1)
	    {
	      if (ch->in_room->copper)
	      {
		strcat(buf, "copper piece ");
	      }
	      else if (ch->in_room->silver)
	      {
		strcat(buf, "silver coin ");
	      }
	      else if (ch->in_room->gold)
	      {
		strcat(buf, "gold coin ");
	      }
	      else if (ch->in_room->emeralds)
	      {
		strcat(buf, "emerald ");
	      }
	      else if (ch->in_room->sapphires)
	      {
		strcat(buf, "sapphire ");
	      }
	      else if (ch->in_room->diamonds)
	      {
		strcat(buf, "diamond ");
	      }

	      strcat(buf, "lying here.");
	    }
	    else
	    {
	      aveworth = totworth/numcoins;

	      if ( aveworth > 100000 )
	      {
		strcat(buf, "extremely valuable ");
	      }
	      else if ( aveworth > 50000 )
	      {
		strcat(buf, "very valuable ");
	      }
	      else if ( aveworth > 10000 )
	      {
		strcat(buf, "valuable ");
	      }
	      else if ( aveworth > 5000 )
	      {
		strcat(buf, "");
	      }
	      else if ( aveworth > 1000 )
	      {
		strcat(buf, "");
	      }
	      else if ( aveworth > 500 )
	      {
		strcat(buf, "");
	      }
	      else
	      {
		strcat(buf, "worthless ");
	      }

	      strcat(buf, "treasure here.");
	    }

	    strcat(buf, "\n\r");
	    send_to_char(buf, ch);

	}

	show_char_to_char( ch->in_room->people,   ch );
	if (ch->visionary != NULL && ch->vision != -2)
	    send_to_char("The shadows all seem to connect to outline the form of a being standing here.\n\r",ch);
	return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp(arg1, "in")  || !str_cmp(arg1,"on"))
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	/* This deals with the nexus */
	if ((!str_prefix(arg2, "nexus") 
	    || !str_prefix(arg2, "disc")
	    || !str_prefix(arg2, "whirlwind")
	    || !str_prefix(arg2, "ring"))
	    && ch->in_room->far_link != NULL
	    && ch->altitude == 0)
  	{
	    ROOM_INDEX_DATA *room;

	    stc("You look inside the nexus.\n\r", ch);
	    room = ch->in_room;
	    char_from_room(ch);
	    char_to_room(ch, room->far_link, 0);
	    do_look(ch, "auto");
	    room = ch->in_room;
	    char_from_room(ch);
	    char_to_room(ch, room->far_link, 0);
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }

	    sprintf( buf, "It's %sfilled with a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than half-" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about half-"     : "more than half-",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

	case ITEM_CAULDRON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }
	    
	    if (skill_table[obj->value[2]].realm == 1)
		sprintf(liqu,"reddish ooze");
	    else if (skill_table[obj->value[2]].realm == 2)
		sprintf(liqu,"blue slime");
	    else if (skill_table[obj->value[2]].realm == 3)
		sprintf(liqu,"yellow concoction");
	    else if (skill_table[obj->value[2]].realm == 4)
		sprintf(liqu,"greenish mixture");
	    else if (skill_table[obj->value[2]].realm == 5)
		sprintf(liqu,"thick black sludge");
	    else sprintf(liqu,"brownish sludge");

	    sprintf( buf, "It's %sfilled with a %s.\n\r",
		obj->value[1] < 6
		    ? "less than half-" :
		obj->value[1] < 9
		    ? "about half-"     : "more than half-",
		liqu);
	    	    
	    send_to_char( buf, ch );
	    break;


	case ITEM_QUIVER:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    act( "$p holds:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{  /* player can see object */
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
		if (++count == number)
		{
		    send_to_char( pdesc, ch );
		    return;
		}
		else continue;

	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
		if (++count == number)
		{       
		    send_to_char( pdesc, ch );
		    return;
		}
		else continue;

	    if ( is_name( arg3, obj->name ) )
		if (++count == number)
		{
		    send_to_char( obj->description, ch );
		    send_to_char( "\n\r",ch);
		    return;
		  }
	  }
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
		if (++count == number)
		{
		    send_to_char( pdesc, ch );
		    return;
		}

	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
		if (++count == number)
		{
		    send_to_char( pdesc, ch );
		    return;
		}

	    if ( is_name( arg3, obj->name ) )
		if (++count == number)
		{
		    send_to_char( obj->description, ch );
		    send_to_char("\n\r",ch);
		    return;
		}
	}
    }

    pdesc = get_extra_descr(arg3,ch->in_room->extra_descr);
    if (pdesc != NULL)
    {
	if (++count == number)
	{
	    send_to_char(pdesc,ch);
	    return;
	}
    }
    
    if (count > 0 && count != number)
    {
	if (count == 1)
	    sprintf(buf,"You only see one %s here.\n\r",arg3);
	else
	    sprintf(buf,"You only see %d of those here.\n\r",count);
	
	send_to_char(buf,ch);
	return;
    }

	 if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = 0;
    else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = 1;
    else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = 2;
    else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = 3;
    else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = 4;
    else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = 5;
    else
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    /* 'look direction' */
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	send_to_char( "Nothing special there.\n\r", ch );
	return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Nothing special there.\n\r", ch );

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&   pexit->keyword[0] != ' ' )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	}
	else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
	{
	    act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
	}
    }

    return;
}

void do_read (CHAR_DATA *ch, char *argument )
{
    do_examine(ch,argument);
}

void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

    do_look( ch, arg );

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;
	
	case ITEM_JUKEBOX:
	    do_play(ch,"list");
	    break;

	case ITEM_MONEY:
	    break;

	case ITEM_PEN_TOME:
	    if (ch->secondaryrace != RACE_WARLOCK) break;
	    if (ch->power[DISC_WARL_SCRI] == 10) break;
	    ch->power[DISC_WARL_SCRI]++;
	    act("As you read $p it vanishes in a puff of smoke.",
		ch, obj, NULL, TO_CHAR);
	    stc("You feel more experienced.\n\r", ch);
	    extract_obj(obj);
	    return;
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
 	case ITEM_CAULDRON:
	case ITEM_CORPSE_PC:
	    sprintf(buf,"in %s",argument);
	    do_look( ch, buf );
        }

    if (obj->item_type == ITEM_WEAPON)
    {
	stcf(ch, "You estimate the reach of this weapon to be %d.\n\r", attack_reach(ch, obj));

        if (obj->size <= get_size(ch) - 2)
	    stc("This weapon is much too tiny to wield.\n\r", ch);
        else if (calc_hands_needed(ch, obj) == 1)
	    stc("You can wield this weapon easily with one hand.\n\r", ch);
        else if (calc_hands_needed(ch, obj) == 2)
	    stc("You will need two hands to wield this weapon.\n\r", ch);
        else if (calc_hands_needed(ch, obj) == 4)
	    stc("You will need four hands to wield this weapon.\n\r", ch);
        else if (calc_hands_needed(ch, obj) > 4)
	    stc("This weapon is WAY to big to hold.\n\r", ch);
    }

    if (obj->size == SIZE_TINY)
	stc("It is tiny.\n\r", ch);
    else if (obj->size == SIZE_SMALL)
	stc("It is quite small.\n\r", ch);
    else if (obj->size == SIZE_LARGE)
	stc("It is quite large, over five feet long.\n\r", ch);
    else if (obj->size == SIZE_HUGE)
	stc("It is absolutely huge.\n\r", ch);
    else if (obj->size == SIZE_GIANT)
	stc("This object was made for a giant.\n\r", ch);
 
    if (obj->weight > 9)
        stcf(ch, "You estimate the weight to be around %d pounds.\n\r",
		obj->weight / (dice(1, 3) + 8));
    else stc("You estimate the weight to be under a pound.\n\r", ch); 

    if (obj->condition < 12)
	stc("It looks like it is about to break.\n\r", ch);
    else if (obj->condition < 25)
	stc("It looks heavily damaged.\n\r", ch);
    else if (obj->condition < 40)
	stc("It looks quite damaged.\n\r", ch);
    else if (obj->condition < 60)
	stc("It it slightly damaged, but will hold together.\n\r", ch);

    }
    else if (!str_prefix( arg, "treasure"))
    {
	sprintf(buf, "The treasure contains %ld diamonds, %ld sapphires,\n\r", ch->in_room->diamonds, ch->in_room->sapphires);	
	send_to_char(buf, ch);
	sprintf(buf, " %ld emeralds, %ld gold coins, %ld silver coins and %ld copper pieces.\n\r", ch->in_room->emeralds, ch->in_room->gold, ch->in_room->silver, ch->in_room->copper);
	send_to_char(buf, ch);
    }	

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;

    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) && IS_NEWFLAG(ch, AFF_SHADOW_PLANE))
	return;

    if (weird_weather_type == WEIRD_WEATHER_FOG)
    {
	stc("You cannot see a thing in this fog.\n\r", ch);
	return;
    }
    if (fAuto)
    {
	if (weird_weather_type == WEIRD_WEATHER_FOG) return;
	sprintf(buf,"#b[Exits:");
    }
    else if (IS_IMMORTAL(ch)) 
	sprintf(buf,"Obvious exits from room %d:\n\r",ch->in_room->vnum);
    else
	sprintf(buf,"Obvious exits:\n\r");

    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   can_see_room(ch,pexit->u1.to_room) 
	&&   (!IS_SET(pexit->exit_info, EX_CLOSED) || fAuto) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		if (IS_SET(pexit->exit_info,EX_CLOSED))
		    strcat( buf, " [");
		else if (pexit->exit_info >= EX_ICE_WALL
			&& pexit->exit_info <= EX_ASH_WALL)
		    strcat( buf, " <");
		else strcat( buf, " " );
		strcat( buf, dir_name[door] );
		if (IS_SET(pexit->exit_info,EX_CLOSED))
		    strcat( buf, "]");
		else if (pexit->exit_info >= EX_ICE_WALL
			&& pexit->exit_info <= EX_ASH_WALL)
		    strcat( buf, ">");
	    }
	    else
	    {
		sprintf( buf + strlen(buf), "%-5s - %s",
		    capitalize( dir_name[door] ),
		    room_is_dark( pexit->u1.to_room )
			?  "Too dark to tell"
			: pexit->u1.to_room->name
		    );
		if (IS_IMMORTAL(ch))
		    sprintf(buf + strlen(buf), 
			" (room %d)\n\r",pexit->u1.to_room->vnum);
		else
		    sprintf(buf + strlen(buf), "\n\r");
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\n\r" );

	if (fAuto)
	{
	    if (ch->in_room != NULL)
	    {
		strcat( buf, "]     --> ");
		if (ch->altitude == 0)
		{
	 	    if (ch->in_room->false_sect_type == NULL
			|| !str_cmp(ch->in_room->false_sect_type, "None") )
		    {
		        switch (ch->in_room->sector_type)
		        {
			    case SECT_INSIDE		: strcat(buf, "Inside"); break;
			    case SECT_HOME  		: strcat(buf, "Inside"); break;
			    case SECT_CITY		: strcat(buf, "City"); break;
			    case SECT_FIELD		: strcat(buf, "Field"); break;
			    case SECT_FOREST		: strcat(buf, "Forest"); break;
			    case SECT_HILLS 		: strcat(buf, "Hill"); break;
			    case SECT_MOUNTAIN 		: strcat(buf, "Mountain");break;
			    case SECT_WATER_SWIM 	: strcat(buf, "Deep Water");break;
			    case SECT_WATER_NOSWIM 	: strcat(buf, "Shallow Water"); break;
			    case SECT_UNDERWATER	: strcat(buf, "Underwater");break;
			    case SECT_AIR 		: strcat(buf, "Air"); break;
			    case SECT_DESERT 		: strcat(buf, "Desert"); break;
			    case SECT_NARROW_TUNNEL	: strcat(buf, "Tunnel");break;
			    default : strcat(buf, "BUG - report room to imms"); break;
		       }
		   }
		   else strcat(buf, ch->in_room->false_sect_type);
		}
	        else strcat(buf, "Air");
	        strcat(buf, " <--\n\r");
	    }
	}
    send_to_char( buf, ch );
    return;
}

void do_worth( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    divide6_to_char(ch);
    sprintf(buf, "You have %ld diamonds,", ch->diamonds);
    cent_to_char(buf, ch);
    sprintf(buf, "%ld sapphires,", ch->sapphires);
    cent_to_char(buf, ch);
    sprintf(buf, "%ld emeralds,", ch->emeralds);
    cent_to_char(buf, ch);
    sprintf(buf, "%ld gold coins,", ch->gold);
    cent_to_char(buf, ch);
    sprintf(buf, "%ld silver coins,", ch->silver);
    cent_to_char(buf, ch);
    sprintf(buf, "and %ld copper pieces.", ch->copper);
    cent_to_char(buf, ch);
    send_to_char("\n\r", ch);

    sprintf(buf, "Total worth = %ld gold coins", TOTAL_WORTH(ch) / 100);
    cent_to_char(buf, ch);

    divide6_to_char(ch);
    return;
}

void divide_to_char(CHAR_DATA *ch)
{
send_to_char("-------------------------------------------------------------------------------\r\n", ch); }

void divide2_to_char(CHAR_DATA *ch)
{
send_to_char("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\r\n", ch); }

void divide3_to_char(CHAR_DATA *ch)
{
send_to_char("===============================================================================\r\n",ch);
}

void divide4_to_char(CHAR_DATA *ch)
{
send_to_char("#4-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6***#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-#n\r\n",ch);
}

void divide5_to_char(CHAR_DATA *ch)
{
cent_to_char("#4-=[#6***********#4]=-------------=[#6***********#4]=-#n",ch); }

void divide6_to_char(CHAR_DATA *ch)
{
cent_to_char("#4-    -   -  - - -#6- ---====#7*#6====--- -#4- - -  -   -    -",ch); }

void banner_to_char(char *txt, CHAR_DATA *ch)
{
char buf[MAX_STRING_LENGTH];
int loop,wdth,ln;
ln = strlen(txt);
if (ln > 16)
{
	sprintf(buf, "#b#4-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#n                               #4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-#n\r\n"); 
	wdth = (31 - ln) / 2 + 20;
}
else
{
	sprintf(buf, "#b#4-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#n                 #4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-#n\r\n");
	wdth = (16 - ln) / 2 + 32;
}
for (loop = 0; loop < ln; loop++)
    buf[loop + wdth + 22] = txt[loop];
send_to_char(buf,ch);
}

void banner2_to_char(char *txt, CHAR_DATA *ch)
{
char buf[MAX_STRING_LENGTH];
int loop,wdth,ln;
ln = strlen(txt);
if (ln > 16)
{
	sprintf(buf, "#4    -   -  - - -#6- ---===#7                               #6===--- -#4- - -  -   -\r\n"); 
	wdth = (31 - ln) / 2 + 24;
}
else
{
	sprintf(buf, "#4     -    -   -  - - -#6- ---====#7                 #6====--- -#4- - -  -   -    -\r\n");
	wdth = (16 - ln) / 2 + 32;
}
for (loop = 0; loop < ln; loop++)
    buf[loop + wdth + 6] = txt[loop];
send_to_char(buf,ch);
}


void divide_to_buf(BUFFER *out)
{
add_buf(out, "-------------------------------------------------------------------------------\r\n"); }

void divide2_to_buf(BUFFER *out)
{
add_buf(out, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\r\n"); }

void divide3_to_buf(BUFFER *out)
{
add_buf(out,"===============================================================================\r\n");
}

void divide4_to_buf(BUFFER *out)
{
add_buf(out, "#4-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6***#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-#n\r\n");
}

void divide5_to_buf(BUFFER *out)
{
cent_to_buf("#4-=[#6***********#4]=-------------=[#6***********#4]=-#n",out); }

void divide6_to_buf(BUFFER *out)
{
cent_to_buf("#4-    -   -  - - -#6- ---====#7*#6====--- -#4- - -  -   -    -",out); }

void banner_to_buf(char *txt, BUFFER *out)
{
char buf[MAX_STRING_LENGTH];
int loop,wdth,ln;
ln = strlen(txt);
if (ln > 16)
{
	sprintf(buf, "#b#4-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#n                               #4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-#n\r\n"); 
	wdth = (31 - ln) / 2 + 20;
}
else
{
	sprintf(buf, "#b#4-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#n                 #4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-=[#6**#4]=-#n\r\n");
	wdth = (16 - ln) / 2 + 32;
}
for (loop = 0; loop < ln; loop++)
    buf[loop + wdth + 22] = txt[loop];
add_buf(out, buf);
}

void banner2_to_buf(char *txt, BUFFER *out)
{
char buf[MAX_STRING_LENGTH];
int loop,wdth,ln;
ln = strlen(txt);
if (ln > 16)
{
	sprintf(buf, "#4    -   -  - - -#6- ---===#7                               #6===--- -#4- - -  -   -\r\n"); 
	wdth = (31 - ln) / 2 + 24;
}
else
{
	sprintf(buf, "#4     -    -   -  - - -#6- ---====#7                 #6====--- -#4- - -  -   -    -\r\n");
	wdth = (16 - ln) / 2 + 32;
}
for (loop = 0; loop < ln; loop++)
    buf[loop + wdth + 6] = txt[loop];
add_buf(out, buf);
}

void do_upkeep( CHAR_DATA *ch, char *argument )
{
    int totupk; 
    char buf[MAX_STRING_LENGTH];
    totupk =1;
    divide3_to_char(ch);
    cent_to_char("*** Blood Upkeep ***", ch);
    divide2_to_char(ch);
    send_to_char("\n\r", ch); 
	send_to_char("  Being a vampire  :    one point\n\r", ch);
    if (IS_NEWFLAG(ch, AFF_BITE) )
    {
	send_to_char("  Fangs extended   :    no points\n\r", ch);
    }
    if (IS_NEWFLAG(ch, AFF_CLAW) )
    {
	send_to_char("  Claws extended   :    no points\n\r", ch);
    }
    
    if (ch->value[7] != 0 && IS_FLAG3(ch, AFF_STR_BOOST) )
    {
	send_to_char("  Strength boost   :    twelve points\n\r", ch);
	totupk += 12;
    }
    else if (ch->value[7] != 0)
    {    
	send_to_char("  Strength boost   :    four points\n\r", ch);
	totupk += 4;
    }    
    
    if (ch->value[8] != 0 && IS_FLAG3(ch, AFF_DEX_BOOST) )
    {
	send_to_char("  Agility boost    :    twelve points\n\r", ch);
	totupk += 12;
    }
    else if (ch->value[8] != 0)
    {    
	send_to_char("  Agility boost    :    four points\n\r", ch);
	totupk += 4;
    }    
    
    if (ch->value[9] != 0 && IS_FLAG3(ch, AFF_CON_BOOST) )
    {
	send_to_char("  Resilience boost :    twelve points\n\r", ch);
	totupk += 12;
    }
    else if (ch->value[9] != 0)
    {    
	send_to_char("  Resilience boost :    four points\n\r", ch);
	totupk += 4;
    }    
    
    if (IS_FLAG5(ch, AFF5_AIR_BLOOD) )
    {
	send_to_char("  Airy blood       :    seven points\n\r", ch);
	totupk += 7;
    }
    if (IS_AFFECTED(ch, AFF_BLOODLUST) )
    {
	send_to_char("  Bloodlust        :    three points\n\r", ch);
	totupk +=3;
    }
    if (IS_AFFECTED(ch, AFF_DARK_VISION) )
    {
	send_to_char("  Dark sight       :    one point\n\r", ch);
	totupk +=1;
    }
    if (IS_NEWFLAG(ch, AFF_SHADOW_PLANE) )
    {
	send_to_char("  Shadow walk      :    five points\n\r", ch);
	totupk +=5;
    }
    if (IS_NEWFLAG(ch, AFF_SHADOW_SIGHT) )
    {
	send_to_char("  Shadow sight     :    five points\n\r", ch);
	totupk +=5;
    }
    if (IS_NEWFLAG(ch, AFF_MASKED) )
    {
	send_to_char("  Masking          :    four points\n\r", ch);
	totupk +=4;
    }
    if (ch->cur_form == 1)
    {
	send_to_char("  Wolf form        :    three points\n\r", ch);
	totupk +=3;
    }
    if (ch->cur_form == FRM_CRIMSON)
    {
	send_to_char("  Crimson Death    :    eight points\n\r", ch);
	totupk +=8;
    }
    if (ch->cur_form == 2)
    { 
	send_to_char("  Bat form         :    three points\n\r", ch);
	totupk +=3;
    }
    if (ch->cur_form == 3)
    { 
	send_to_char("  Mist form        :    three points\n\r", ch);
	totupk +=3;
    }
    if (IS_SET(ch->flag4, AFF_FLICKER) )
    { 
	send_to_char("  Flickering       :    six points\n\r", ch);
	totupk +=6;
    }
    if (IS_SET(ch->act, PLR_HOLYLIGHT) )
    { 
	send_to_char("  True sight       :    four points\n\r", ch);
	totupk +=4;
    }
    if ( IS_NEWFLAG(ch, AFF_SHIELD ) )
    { 
	send_to_char("  Shielding        :    two points\n\r", ch);
	totupk +=2;
    }
    if ( IS_FLAG4(ch, AFF_PHASING ) )
    { 
	send_to_char("  Phasing          :    four points\n\r", ch);
	totupk +=4;
    }
    send_to_char("\n\r", ch);
    divide2_to_char(ch);
    sprintf(buf, "Upkeep : %d    Current Blood Pool : %d/%d    Hours till next feed : %d", totupk, ch->blood,max_blood_pool(ch), ch->blood/totupk); 
    cent_to_char(buf, ch);
    divide3_to_char(ch);
    if (totupk != ch->upkeep)
    {
	bug("Upkeep:  differing upkeep values.",0);
	bug("Upkeep:  using freshly calculated value.",0);
	ch->upkeep = totupk;
    }
}

void do_finger(CHAR_DATA *ch, char *argument)
{       
    bool found=TRUE, loaded = FALSE;
    CHAR_DATA *vch;
    char buf[MAX_STRING_LENGTH];
    char sexbuf[10];
    char sexbuf2[10];

    return;

    if (argument[0] == '\0')
    {
        send_to_char("Finger who?\n\r",ch);
        return;
    }

    if ((vch = get_char_world(ch, argument)) == NULL || IS_NPC(vch))
    {
	loaded = TRUE;
	found = FALSE;
    	vch = alloc_mem(sizeof(*vch));
    
    	vch = new_char();
    	vch->pcdata = new_pcdata();

    	found = do_load_char_obj(vch, argument);
    	if (found) char_to_room(vch, get_room_index(prime_realm, 47), 0);
    }
    crash("do_finger zormic 1");

    if (!found || vch == NULL) 
    {
        send_to_char("Nobody of that name exists.\n\r", ch);
        return;
    }

    WAIT_STATE(ch, 12);

    if (vch->level < 2)
    {
	stc("They haven't been playing long enough to have information stored.\n\r", ch);
	return;
    }
     
    sprintf(buf, GET_PROPER_NAME(vch) );    
    banner_to_char(buf, ch);

//    free_string(buf);
    if (vch->registerred != 1) cent_to_char("#1--== UNREGISTERED ==--",ch);

    if (vch->pcdata->true_sex == SEX_MALE) sprintf(sexbuf2, "His");
    else if (vch->pcdata->true_sex == SEX_FEMALE) sprintf(sexbuf2, "Her");
    else sprintf(sexbuf2, "Its");

    if (vch->pcdata->true_sex == SEX_MALE) sprintf(sexbuf, "He");
    else if (vch->pcdata->true_sex == SEX_FEMALE) sprintf(sexbuf, "She");
    else sprintf(sexbuf, "Its");

    if (vch->pcdata->real_name != NULL)
    {
	sprintf(buf, "%s real name is %s.", sexbuf2, vch->pcdata->real_name);
	cent_to_char(buf, ch);
    }

    crash("do_finger zormic 2");

    sprintf(buf, "Player kills : %d    Player deaths : %d"
		, vch->pkills, vch->pkilled);
    cent_to_char(buf, ch);

    sprintf(buf, "They have been playing %d hours.", vch->played / 3600);
    cent_to_char(buf, ch);

//    if (vch->level < 112 || ch->level == 120)
//    {
//        sprintf(buf, "Last Login from: %s on %s",
//	    (vch->pcdata->last_login_from == NULL) ? "(Unknown)" :vch->pcdata->last_login_from,
//	(vch->pcdata->last_login_time == NULL) ? "(Unknown)" :
// vch->pcdata->last_login_time);
//        cent_to_char(buf, ch);
//    }

    crash("do_finger zormic 6");

    if (vch->married_to != NULL
	&& vch->married_to[0] != 0)
    {
	sprintf(buf, "%s is %s %s.", sexbuf, (vch->marriage_status == 2) ? "engaged to" : 
		(vch->marriage_status == 1) ? "in love with" : "married to", vch->married_to);
	cent_to_char(buf, ch);
    }

    crash("do_finger zormic 7");

    if (vch->pcdata->email != NULL)
    {
	sprintf(buf, "E-mail address : %s", vch->pcdata->email);
	cent_to_char(buf, ch);
    }

    if (vch->pcdata->homepage != NULL)
    {
	sprintf(buf, "Homepage address : %s", vch->pcdata->homepage);
	cent_to_char(buf, ch);
    }

    if (vch->pcdata->plan != NULL &&
	(vch->pcdata->plan[0] != 'D'
	|| vch->pcdata->plan[1] != 'I'
	|| vch->pcdata->plan[2] != 'S'
	|| vch->pcdata->plan[3] != 'A'
	|| vch->pcdata->plan[4] != 'B'
	|| vch->pcdata->plan[5] != 'L'
	|| vch->pcdata->plan[6] != 'E'
	|| vch->pcdata->plan[7] != 'D'
	|| vch->pcdata->plan[8] != ':'))
    {
	sprintf(buf, "Plan : %s.", vch->pcdata->plan);
	cent_to_char(buf, ch);
    }

    crash("do_finger zormic 8");

    sprintf(buf, "League Score : %d", league_score(vch));
    cent_to_char(buf, ch);

    crash("do_finger zormic 8b");

    if (vch->level > 111)
    {
	char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';
	banner_buffer(get_who_race(ch, vch), buf);
	banner_to_char(buf, ch);
    }
    else if ( vch->alignment >  900 ) banner_to_char( "A N G E L I C",ch);
    else if ( vch->alignment >  700 ) banner_to_char( "S A I N T L Y", ch);
    else if ( vch->alignment >  350 ) banner_to_char( "G O O D",ch );
    else if ( vch->alignment >  100 ) banner_to_char( "K I N D",    ch );
    else if ( vch->alignment > -100 ) banner_to_char( "N E U T R A L", ch);
    else if ( vch->alignment > -350 ) banner_to_char( "M E A N",    ch );
    else if ( vch->alignment > -700 ) banner_to_char( "E V I L",    ch );
    else if ( vch->alignment > -900 ) banner_to_char( "D E M O N I C", ch);
    else banner_to_char( "S A T A N I C", ch );

    if (loaded) extract_char(vch, TRUE);
}
    

void do_racestats( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    
    banner_to_char("S T A T I S T I C S",ch);
    
    if (ch->secondaryrace == 0)
       sprintf(buf, "You are a %s, and are %d years old.", 
	 race_table[ch->race].name, get_age(ch));
    else if (ch->secondaryrace == RACE_WARLOCK)
       sprintf(buf, "You are a %s warlock, and are %d years old.",
	 race_table[ch->race].ishname, get_age(ch) );
    else sprintf(buf, "You have been transformed from a %s into a %s, and are %d years old.",
	race_table[ch->race].name,secondaryrace[ch->secondaryrace].propname,get_age(ch) );

    cent_to_char( buf, ch );

    if (ch->secondaryrace == 0)
    {
	sprintf(buf, "You are %d%% of the way to being powerful enough for transformation,",
		ch->level * 100 / 20);
	cent_to_char(buf, ch);
	if (ch->max_hit < 500 && ch->max_move >= 500)
	    sprintf(buf, "and you need %d more hit points.", 500 - ch->max_hit);
	else if (ch->max_hit < 500 && ch->max_move < 500)
	    sprintf(buf, "and you need %d more hit points and %d more vitality.",
		500 - ch->max_hit, 500 - ch->max_move);
	else if (ch->max_move < 500)
	    sprintf(buf, "and you need %d more vitality.", 500 - ch->max_move);
	else sprintf(buf, "and you have the required hit points and vitality.");
	cent_to_char(buf, ch);
    }


    if (ch->tertiaryrace != 0)
    {
	sprintf(buf, "You have been transformed from a %s into a %s.",
	    secondaryrace[ch->secondaryrace].propname,
	    tertiaryrace[ch->tertiaryrace].name);
	cent_to_char(buf, ch);
    }

    if (ch->secondaryrace == RACE_VAMPIRE)
    {
	sprintf(buf, "You are of generation %d.", 14 - ch->generation);
	cent_to_char(buf, ch);
    }

    if (ch->cur_form != 0)
    {
	sprintf(buf, "You are currently in the form of a %s.",
	    form_data[ch->cur_form].lookname);
	cent_to_char(buf, ch);
    }

    if (!IS_NPC(ch) && ch->pcdata->conditioned_by != NULL)
    {
	sprintf(buf, "You have been conditioned by %s.",
	    ch->pcdata->conditioned_by);
	cent_to_char(buf, ch);
    }	

    if (ch->married_to != NULL && ch->marriage_status > 0)
    {
	sprintf(buf, "You are %s to %s.",
	    (ch->marriage_status == MARRY_PROPOSING) ? "in love with" :
	    (ch->marriage_status == MARRY_ENGAGED) ? "engaged" : "married",
	    ch->married_to);
	cent_to_char(buf, ch);
    }

    if (get_size(ch) == SIZE_TINY)
	cent_to_char("You are of tiny size.", ch);
    else if (get_size(ch) == SIZE_SMALL)
	cent_to_char("You are of small size.", ch);
    else if (get_size(ch) == SIZE_MEDIUM)
	cent_to_char("You are of medium size.", ch);
    else if (get_size(ch) == SIZE_LARGE)
	cent_to_char("You are of large size.", ch);
    else if (get_size(ch) == SIZE_HUGE)
	cent_to_char("You are of huge size.", ch);
    else if (get_size(ch) == SIZE_GIANT)
	cent_to_char("You are absolutely gigantic.", ch);
    sprintf(buf, "Your attacking reach with all body attacks is %d.", attack_reach(ch, NULL));
    cent_to_char(buf, ch);

    if (ch->invis_level && IS_IMMORTAL(ch))
    {
	sprintf( buf, "You are invisible at level %d",ch->invis_level);
	cent_to_char(buf,ch);
    }

    if (ch->incog_level && IS_IMMORTAL(ch))
    {
	sprintf(buf,"You are incognito at level %d",ch->incog_level);
	cent_to_char(buf,ch);
    }

    if (ch->tertiaryrace == TERTIARY_DROW
	&& IS_FLAG4(ch, AFF_GEN_RESIST) )
    {
	sprintf(buf,"Your drow magic resistance is at %d%%.",
	    get_drow_magic_resistance(ch) );
	ctc(buf, ch);
    }

    divide5_to_char(ch);

    sprintf(buf, "Your total magic power is %d.", ch->earth+ch->fire+ch->entropy+ch->water+ch->air+ch->lifemagic);
    cent_to_char(buf, ch);

    if ((BODY_PART(ch, BODY_THIRD_ARM) && !BODY_PART(ch,BODY_FOURTH_ARM) )
        || (!BODY_PART(ch, BODY_THIRD_ARM) && BODY_PART(ch,BODY_FOURTH_ARM) ) )
	cent_to_char("You have grown an extra arm.", ch);
	
    else if (BODY_PART(ch, BODY_THIRD_ARM) && BODY_PART(ch, BODY_FOURTH_ARM) )
	cent_to_char("You have grown two extra arms.", ch);

    if (ch->aura > 0)
    {
	sprintf(buf, "You have a level %d aura in operation.", ch->aura);
	cent_to_char(buf, ch);
    }

    if (IS_NEWFLAG(ch, AFF_WATER_SHIELD))
	cent_to_char("You are surrounded by a shield of water.", ch);

    if (IS_SET(ch->flag3, AFF_RECEPTACLE))
	cent_to_char("You are currently a receptacle of magical energy.",ch);

    if (IS_SET(ch->flag3, AFF_KARMA))
	cent_to_char("You are currently using karma to heal yourself.",ch);

    if (IS_SET(ch->flag4, AFF_PHASING))
	cent_to_char("You are currently phasing out of the sun.",ch);

    if (IS_SET(ch->flag4, AFF_STASIS))
	cent_to_char("You are currently in stasis.",ch);

    if (IS_SET(ch->flag2, AFF_EARTH_MELD))
	cent_to_char("You are currently melded with the earth.",ch);

    if (IS_SET(ch->flag4, AFF_SENESCHIA))
	cent_to_char("You are currently in seneschia form.",ch);

    if (IS_SET(ch->flag4, AFF_TREE_MELD))
	cent_to_char("You are currently melded with a tree.",ch);

    if (ch->secondaryrace == RACE_VAMPIRE
	&& ch->blood < 20)
    	cent_to_char("You are dangerously low on blood.", ch);

    if (!IS_NPC(ch) && str_cmp(ch->name, ch->pcdata->switchname)
	&& !IS_BASE_FORM(ch))
    {
	sprintf(buf,"You are currently masked as %s.", ch->name);
	cent_to_char(buf, ch);
    }

    if (ch->riding != NULL)
    {
	sprintf(buf, "You are currently riding %s.", ch->riding->name);
	cent_to_char(buf, ch);
    }


    if (ch->secondaryrace == RACE_DAEMON
	&& !IS_NPC(ch)
	&& ch->generation != 13)
    {
	sprintf(buf,"You have collected %d souls, and need %d more for promotion."
		, ch->pcdata->souls, souls_needed(ch) - ch->pcdata->souls);
	cent_to_char(buf, ch);
    }
   
    if (ch->secondaryrace == 0)
    {
	if (IS_NEWFLAG(ch,AFF_ALLOW_VAMP))
		cent_to_char("You may be bitten by vampires.",ch);
	if (IS_NEWFLAG(ch,AFF_ALLOW_WERE))
		cent_to_char("You may be cursed with lycanthropy.",ch);
	if (IS_NEWFLAG(ch,AFF_ALLOW_ELAD))
		cent_to_char("You may join the eladrin.",ch);
	if (IS_NEWFLAG(ch,AFF_ALLOW_DEMON))
		cent_to_char("You are willing to sell your soul to Baator.",ch);
	if (IS_NEWFLAG(ch,AFF_ALLOW_WARLOCK))
		cent_to_char("You are ready to study to become a Warlock.",ch);
    }

    if (ch->secondaryrace == RACE_DRAGON)
    {
       SET_BIT(ch->flag2, AFF_BITE);
       SET_BIT(ch->flag2, AFF_CLAW);
       SET_BIT(ch->flag2, AFF_WING);
       SET_BIT(ch->flag2, AFF_TAIL);
    }

    divide5_to_char(ch);

    if (!IS_NPC(ch) && FALSE)
    {
	int dc = GET_DAMCAP(ch);
	if (dc > 80) dc += (dc - 80) * 2;
	if (dc > 35) dc += (dc - 35) * 2;
        sprintf(buf,"Your maximum damage is currently %d.",dc);
        cent_to_char(buf, ch);
	sprintf(buf, "You have killed %d enemy mobs, and been killed by %d.", ch->pcdata->mob_kills, ch->pcdata->mob_killed);
	cent_to_char(buf, ch);
    }

    if (ch->in_room->sector_type == SECT_FOREST
	&& ch->power[DISC_ELAD_ARBO] > 0
	&& ch->alignment > 300)
    {
	sprintf(buf, "You are gaining strength from the earth.");
	cent_to_char(buf, ch);
    }

    if (ch->power[DISC_ELAD_PHYS] > 4 && ch->played/36000 > 0)
    {
	sprintf(buf,"Your toughened skin has improved your armor class by %d.", ch->played/36000);
	cent_to_char(buf, ch);
    }

    if (ch->power[DISC_DAEM_GELU] > 1 && ch->played/18000 > 0)
    {
	sprintf(buf,"Your icy strength is improving your damage by %d.", ch->played/18000);
	cent_to_char(buf, ch);
    }

    if (ch->power[DISC_HERO_BATT] > 1 && ch->played/10000 > 0)
    {
        sprintf(buf,"Your mastery of weapons is improving your damage by %d.", ch->played/10000);
        cent_to_char(buf, ch);
    }

    if (IS_SET(ch->flag2, AFF_BITE)
	|| IS_SET(ch->flag2, AFF_CLAW)
	|| IS_SET(ch->flag2, AFF_WING)
	|| IS_SET(ch->flag4, AFF_HORN)
	|| IS_SET(ch->flag2, AFF_TAIL))
    {
	bool comma = FALSE;
	strcpy(buf, "You have extra ");
	if (IS_SET(ch->flag2, AFF_BITE))
	{
	    comma = TRUE;
	    strcat(buf, "bite");
        }
	if (IS_SET(ch->flag2, AFF_CLAW))
	{
	    if (comma)
	    {
		if (!IS_SET(ch->flag2, AFF_WING) && !IS_SET(ch->flag2,AFF_TAIL))
		    strcat(buf, " and ");
		else strcat(buf, ", ");
	    }
	    comma = TRUE;
	    strcat(buf, "claw");
        }
	if (IS_SET(ch->flag2, AFF_WING))
	{
	    if (comma)
	    {
		if (!IS_SET(ch->flag2, AFF_TAIL))
		    strcat(buf, " and ");
		else strcat(buf,", ");
	    }
	    comma = TRUE;
	    strcat(buf,"wing");
        }
	if (IS_SET(ch->flag4, AFF_HORN))
	{
	    if (comma)
	    {
		if (!IS_SET(ch->flag2, AFF_TAIL))
		    strcat(buf, " and ");
		else strcat(buf,", ");
	    }
	    comma = TRUE;
	    strcat(buf,"horn");
        }
	if (IS_SET(ch->flag2, AFF_TAIL))
	{
	    if (comma) strcat(buf," and ");
	    strcat(buf,"tail");
        }

	strcat(buf," attacks.");
	cent_to_char(buf,ch);
    }

    divide4_to_char(ch);
}

void do_score( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int i, hit, dam;
    bool done;

    if (is_item(ch))
    {
	stcf(ch, "You are %s.\n\r", ch->sw_obj->short_descr);
	stcf(ch, "You weigh %d pounds.\n\r", ch->sw_obj->weight/10);
	stcf(ch, "You have %d hit points remaining out of 100.\n\r",
	    ch->sw_obj->condition);
	return;
    }

    banner_to_char("S C O R E",ch);
    sprintf( buf,
	"You are %s, %d years old (%d hours).",
	ch->name,
	get_age(ch),
	( ch->played + (int) (current_time - ch->logon) ) / 3600);
    cent_to_char( buf, ch );

    if ( get_trust( ch ) != ch->level && get_trust(ch) != 0)
    {
	sprintf( buf, "You are trusted at level %d.",
	    get_trust( ch ) );
	cent_to_char( buf, ch );
    }

    sprintf(buf, "Race: %s  Sex: %s",
	race_table[ch->race].name,
	ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female");
    cent_to_char(buf,ch);

    divide5_to_char(ch);
    crash("score place 1");

    if (ch->true_faith > 0)
        sprintf(buf, "Current XP : %ld     Faith : %d(%d)     Kai : %d",
	  ch->currentexp, ch->faith,ch->true_faith, ch->qp);
    else sprintf(buf, "Current XP : %ld      Faith : %d      Kai : %d", ch->currentexp, ch->faith, ch->qp);

    if (ch->secondaryrace==RACE_VAMPIRE)
    {
	sprintf( buf2, "      Blood pool: %d", ch->blood);
	strcat(buf, buf2);
    }
    else if (ch->secondaryrace == RACE_DRAGON)
    {
	sprintf( buf2, "      Breaths: %d",ch->breaths);
	strcat(buf, buf2);
    }

    cent_to_char(buf,ch);
    crash("score place 2");

    if (IS_NPC(ch))
    sprintf( buf,
	"You have %d / %d hit, %d / %d vitality.",
	ch->hit,  ch->max_hit,
	ch->move, ch->max_move);
    else sprintf( buf,
	"You have %d / %d hit, %d / %d vitality, and %dx edu.",
	ch->hit,  ch->max_hit,
	ch->move, ch->max_move, ch->pcdata->education);
    cent_to_char( buf, ch );

    crash("score place 3");

    sprintf( buf, "%d / %d psp, and are a level %d psionicist.",
	ch->psp, ch->max_psp, ch->psi_level);
    cent_to_char( buf, ch );

    sprintf( buf,
	"You are carrying %d / %d items with weight %ld / %d pounds.",
	ch->carry_number, can_carry_n(ch),
	get_carry_weight(ch) / 10, can_carry_w(ch) /10 );
    cent_to_char( buf, ch );

    crash("score place 4");

    done = FALSE;
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
    {
	sprintf( buf, "You are drunk");
	done = TRUE;
    }
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER]   ==  0 )
    {
	if (done)
	    strcat(buf, ", hungry");
	else
	{       
	sprintf(buf, "You are hungry");
	done = TRUE;
	}
    }
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
    {
	if (done)
	    strcat(buf, ", thirsty");
	else
	{       
	sprintf(buf, "You are thirsty");
	done = TRUE;
	}
    }
    if ( !IS_NPC(ch) && ch->blood < 10 && ch->secondaryrace == RACE_VAMPIRE  )
    {
	if (done)
	    strcat(buf, ", and you thirst for blood");
	else
	{       
	sprintf(buf, "You thirst for blood");
	done = TRUE;
	}
    }

    crash("score place 5");

    if (done)
	cent_to_char(buf, ch);

    switch ( ch->position )
    {
    case POS_DEAD:     
	cent_to_char("You are DEAD!!",ch);
	break;
    case POS_MORTAL:
	cent_to_char( "You are mortally wounded.",ch );
	break;
    case POS_INCAP:
	cent_to_char( "You are incapacitated.",ch);
	break;
    case POS_STUNNED:
	cent_to_char( "You are stunned.",ch);
	break;
    case POS_SLEEPING:
	cent_to_char( "You are sleeping.",ch);
	break;
    case POS_RESTING:
	cent_to_char( "You are resting.",ch);
	break;
    case POS_SITTING:
	cent_to_char( "You are sitting.",ch);
	break;
    case POS_STANDING:
	sprintf( buf, "You are standing");
	break;
    case POS_FIGHTING:
	sprintf( buf, "You are fighting");
	break;
    }

    crash("score place 6");
 
    if (ch->position >= POS_STANDING)
    {
    sprintf( buf2, ", and will flee on %d hit points.", ch->wimpy );
    strcat(buf, buf2);
    cent_to_char( buf, ch );
    }

    divide5_to_char(ch);

    crash("score place 7");

    sprintf( buf,
	"Str: %s  Int: %s  Wis: %s",
	abilityword[get_curr_stat(ch,STAT_STR)],
	abilityword[get_curr_stat(ch,STAT_INT)],
	abilityword[get_curr_stat(ch,STAT_WIS)]);
    cent_to_char(buf,ch);
    sprintf( buf,
	"Agi: %s  Res: %s  Cha: %s",
	abilityword[get_curr_stat(ch,STAT_DEX)],
	abilityword[get_curr_stat(ch,STAT_CON)],
	abilityword[get_curr_stat(ch,STAT_CHA)] );
    cent_to_char( buf, ch );

    divide5_to_char(ch);

    crash("score place 8");

    for (i = 0; i < 4; i++)
    {
	char * temp;

	switch(i)
	{
	    case(AC_PIERCE):    temp = "piercing";      break;
	    case(AC_BASH):      temp = "bashing";       break;
	    case(AC_SLASH):     temp = "slashing";      break;
	    case(AC_EXOTIC):    temp = "magic";         break;
	    default:            temp = "error";         break;
	}
	
	if      (GET_AC(ch,i) >= 101)
	    sprintf(buf,"You are hopelessly vulnerable to %s.",temp);
	else if (GET_AC(ch,i) >= 80) 
	    sprintf(buf,"You are defenseless against %s.",temp);
	else if (GET_AC(ch,i) >= 60)
	    sprintf(buf,"You are barely protected from %s.",temp);
	else if (GET_AC(ch,i) >= 40)
	    sprintf(buf,"You are slightly armored against %s.",temp);
	else if (GET_AC(ch,i) >= 20)
	    sprintf(buf,"You are somewhat armored against %s.",temp);
	else if (GET_AC(ch,i) >= 0)
	    sprintf(buf,"You are armored against %s.",temp);
	else if (GET_AC(ch,i) >= -20)
	    sprintf(buf,"You are well-armored against %s.",temp);
	else if (GET_AC(ch,i) >= -40)
	    sprintf(buf,"You are very well-armored against %s.",temp);
	else if (GET_AC(ch,i) >= -60)
	    sprintf(buf,"You are heavily armored against %s.",temp);
	else if (GET_AC(ch,i) >= -80)
	    sprintf(buf,"You are superbly armored against %s.",temp);
	else if (GET_AC(ch,i) >= -100)
	    sprintf(buf,"You are very heavily armored against %s.",temp);
	else if (GET_AC(ch,i) >= -130)
	    sprintf(buf,"You are almost invulnerable to %s.",temp);
	else if (GET_AC(ch,i) >= -160)
	    sprintf(buf,"You are supremely armored against %s.",temp);
	else if (GET_AC(ch,i) >= -190)
	    sprintf(buf,"You are divinely armored against %s.",temp);
	else
	    sprintf(buf,"You are ultimately armored against %s.",temp);

	cent_to_char(buf,ch);
    }

    divide5_to_char(ch);
    crash("score place 9");
 
    if (!IS_NPC(ch))
    {
        sprintf(buf, "You have killed %d players and been killed %d times.", ch->pkills, ch->pkilled);
        cent_to_char(buf, ch);
    }

    hit = GET_HITROLL(ch);
    dam = GET_DAMROLL(ch);

    if (ch->rage <= 0)
        sprintf( buf, "Beast: %d  Hitroll: %d  Damroll: %d.",ch->beast,hit, dam );
    else
	sprintf( buf, "Beast: %d  Hitroll: %d  Damroll: %d.  Raging: +%d"
	    ,ch->beast, hit, dam, ch->rage);

    cent_to_char( buf, ch );
    

/*    if ( IS_IMMORTAL(ch))
    {
      send_to_char("Holy Light: ",ch);
      if (IS_SET(ch->act,PLR_HOLYLIGHT))
	send_to_char("on",ch);
      else
	send_to_char("off",ch);
 
      if (ch->invis_level)
      {
	sprintf( buf, "  Invisible: level %d",ch->invis_level);
	send_to_char(buf,ch);
      }

      if (ch->incog_level)
      {
	sprintf(buf,"  Incognito: level %d",ch->incog_level);
	send_to_char(buf,ch);
      }
      send_to_char("\n\r",ch);
    }
*/
	 if ( ch->alignment >  900 ) banner_to_char( "A N G E L I C",ch );
    else if ( ch->alignment >  700 ) banner_to_char( "S A I N T L Y", ch );
    else if ( ch->alignment >  350 ) banner_to_char( "G O O D",ch );
    else if ( ch->alignment >  100 ) banner_to_char( "K I N D",    ch );
    else if ( ch->alignment > -100 ) banner_to_char( "N E U T R A L", ch );
    else if ( ch->alignment > -350 ) banner_to_char( "M E A N",    ch );
    else if ( ch->alignment > -700 ) banner_to_char( "E V I L",    ch );
    else if ( ch->alignment > -900 ) banner_to_char( "D E M O N I C", ch );
    else                             banner_to_char( "S A T A N I C", ch );
}

void do_affects(CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf, *paf_last = NULL;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int last_len = 0, lp;

    if ( ch->affected != NULL 
	&& (!IS_FLAG3(ch, AFF_LYCANTHROPY)
	|| TIME_UP(ch, TIMER_MESMERISED) ) )
    {
	banner_to_char("A F F E C T S", ch);
	if (IS_FLAG3(ch, AFF_LYCANTHROPY) && ch->secondaryrace == 0)
	    cent_to_char("you have contracted lycanthropy", ch);
	if (!TIME_UP(ch, TIMER_MESMERISED) )
	    cent_to_char("you are mesmerised and cannot move", ch);

	for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
	    if (paf->type == -1) continue;
	    if (paf->name == NULL)
	        sprintf( buf, "%s ", skill_table[paf->type].name);
 	    else sprintf ( buf, "%s ", paf->name);
	    if (paf_last != NULL && paf->type == paf_last->type
		&& (paf_last->name == NULL || paf->name == NULL
		|| !str_cmp(paf_last->name, paf->name)))
 	    {
		if (ch->level < 112) continue;
		for (lp = 0; lp < last_len; lp++)
			buf2[lp] = ' ';
		buf2[last_len] = '\0';
	    }
	    else
	    {
	    	strcpy(buf2, buf);
		if ( paf->duration == -1 )
		    sprintf( buf, "affecting you permanently" );
		else if (paf->duration < 1)
		    sprintf( buf, "is about to run out");
		else if (paf->duration < 5)
		    sprintf( buf, "lasting for a short while");
		else if (paf->duration < 10)
		    sprintf( buf, "lasting for a while");
		else if (paf->duration < 20)
		    sprintf( buf, "lasting for quite some time");
		else if (paf->duration < 40)
		    sprintf( buf, "lasting for ages");
		else
		    sprintf( buf, "lasting for days");
		strcat(buf2, buf);
		last_len = strlen(buf2);
	    }

	    if ( ch->level >= 112 )
	    {
		sprintf( buf,
		    " : modifies %s by %d ",
		    affect_loc_name( paf->location ),
		    paf->modifier);
		strcat(buf2, buf);
	    }

	    cent_to_char( buf2, ch );
	    paf_last = paf;
	}
	divide4_to_char(ch);
    }
    else 
	send_to_char("You are not affected by any spells.\n\r",ch);

    return;
}



char *  const   day_name        [] =
{
    "the Mist", "the Dragon", "the Spirit", "Thunder", "Freedom",
    "the Knight", "the Demon"
};

char *  const   month_name      [] =
{
    "Winter", "the Winter Wolf", "Eternal Slumber", "the Forest",
    "Elven Beauty", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "Dark Hearts", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char *suf;
    int day;

    day     = time_info.day + 1;

	 if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf( buf,
	"It is %d minutes past %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r",
	time_info.min,	
	(time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name[day % 7],
	day, suf,
	month_name[time_info.month]);
    send_to_char(buf,ch);
    sprintf(buf,"%s started up at %s\n\rThe system time is %s",
	NAME_OF_MUD,
	str_boot_time,
	(char *) ctime( &current_time )
	);

    send_to_char( buf, ch );
    return;
}

void do_scry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg);

    if (arg[0] == '\0')
    {
	send_to_char("Who do you wish to scry upon?\n\r",ch);
	return;
    }
	
    if (ch->blood < 10 && ch->secondaryrace == RACE_VAMPIRE)
    {
	send_to_char("You have insufficient blood in your blood pool.\n\r", ch);
	return;
    }

    if (ch->secondaryrace == RACE_VAMPIRE) ch->blood -= 10;

    if ((victim = get_char_world( ch, arg)) == NULL)
    {
	send_to_char( "You fail to locate anybody of that name.\n\r",ch);
	return;
    }

    if (IS_NEWFLAG(victim, AFF_SHIELD) 
	|| victim->in_room == NULL)
    {
	send_to_char( "You fail to scry your victim.\n\r",ch);
	return;
    }

    send_to_char( "Your vision distorts as you scry throughout the land.\n\r",ch);
    send_to_char( "   ", ch);
    send_to_char( victim->in_room->name, ch );
    send_to_char( "\n\r",ch);
    send_to_char( victim->in_room->description, ch );       

    if   (!can_see_in_dark(ch) && room_is_dark(ch->in_room) )
    {               
	send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( victim->in_room->people, ch );  
	return;
    }
    show_list_to_char( victim->in_room->contents, ch, FALSE, FALSE );
    show_char_to_char( victim->in_room->people,   ch );
    if ((victim->level > ch-> level) && !IS_NPC(victim))
    {
	send_to_char( "You get the feeling someone is watching you...\n\r",victim);
	send_to_char( "Maybe that wasn't such a good idea...\n\r",ch);
	return;
    }
}

void do_weather( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    static char * const sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the weather indoors.\n\r", ch );
	return;
    }

    sprintf( buf, "The sky is %s and %s.\n\r",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "a warm southerly breeze blows"
	: "a cold northern gust blows"
	);
    send_to_char( buf, ch );
    return;
}

void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp, *lHelp=NULL;
    BUFFER *output;
    char *savearg;
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char title[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    char *buf;
    bool found=FALSE;
    int level, cnt=0;

    output = new_buf();
    savearg = argument;
    title[0] = '\0';

    if ( argument[0] == '\0' )
	argument = "summary";

    crash("doing help... place 1");

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0')
	    strcat(argall," ");
	strcat(argall,argone);
    }

    crash("doing help... place 2");

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	level = (pHelp->level < 0) ? -1 * pHelp->level -1 : pHelp->level;

	if (level > get_trust(ch))
	    continue;

	buf = pHelp->keyword;

	while (strlen(buf) > 0)
	{
	    buf = split_hash(buf, arg);

	    if (is_name(argall, arg))
	    {
		cnt += 1;
		if (cnt == 1)
		{
		    lHelp = pHelp;
		    strcpy(title, arg);
		}
	    }
	    if (!str_cmp(argall, arg))
	    {
		lHelp = pHelp;
		strcpy(title, arg);
		found = TRUE;
	    }
	}
    }

    crash("doing help... place 3");

    if (cnt == 0)
    {
	send_to_char( "No help on that word.\n\r", ch );
	bug("Need help on: ",0);
	bug(savearg,0);
	page_to_char(buf_string(output),ch);
	free_buf(output);
	return;
    }   

    crash("doing help... place 4");

    if (cnt > 1 && !found)
    {
	cnt = 0;
	add_buf(output, "There is help on:\n\r");

	for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
	{
	    level = (pHelp->level < 0) ? -1 * pHelp->level -1 : pHelp->level;

	    if (level > get_trust(ch))
		continue;

	    buf = pHelp->keyword;

	    while (strlen(buf) > 0)
	    {
	        buf = split_hash(buf, arg);

		if (is_name(argall, arg) && strlen(arg) < 19)
	 	{
		    sprintf(buf2, "%-18s ", arg);
		    add_buf(output, buf2);
		    if (++cnt % 4 == 0)
			add_buf(output, "\n\r");
		}
	    }
	}

        crash("doing help... place 5");

	if (cnt %4 != 0)
	    add_buf(output, "\n\r");

	for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
	{
	    level = (pHelp->level < 0) ? -1 * pHelp->level -1 : pHelp->level;

	    if (level > get_trust(ch))
		continue;

	    buf = pHelp->keyword;

	    while (strlen(buf) > 0)
	    {
	        buf = split_hash(buf, arg);

		if (is_name(argall, arg) && strlen(arg) > 18)
	 	{
		    add_buf(output, arg);
		    add_buf(output, "\n\r");
		}
	    }
	}
	
	page_to_char(buf_string(output),ch);
	free_buf(output);
	return;
    }

        crash("doing help... place 6");

	pHelp = lHelp;
	    if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
	    {
		add_buf(output,title);
		add_buf(output,"\n\r");
	    }

	if (config_info->autohelp != NULL && !str_cmp(pHelp->keyword, config_info->autohelp))
	{
	    if (ch->tempprompt != NULL) free_string(ch->tempprompt);
	    ch->tempprompt = str_dup(config_info->autohelp);
	}

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		add_buf(output,pHelp->text+1);
	    else
		add_buf(output,pHelp->text);

    crash("doing help... place 7");

    page_to_char(buf_string(output),ch);
    free_buf(output);
}


/*
 *  Our who command, and its very good ok!
 */
void do_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char sc[MAX_STRING_LENGTH];
    char c[10];
    BUFFER *output;
    int iRace;
    int iClan;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    bool rgfRace[MAX_PC_RACE];
    bool rgfClan[MAX_CLAN];
    bool fClanRestrict = FALSE;
    bool fClan = FALSE;
    bool fRaceRestrict = FALSE;
    bool fImmortalOnly = FALSE;
    bool imminfo = FALSE;
    int best[6], score;
    bool rank = FALSE;
    CHAR_DATA *wch;

    best[1] = -10000;
    best[2] = -10000;
    best[3] = -10000;
    best[4] = -10000;
    best[5] = -10000;

    if (argument[0] != 0 && ch->level > 115)
	imminfo = TRUE;	
 
    if (!str_prefix(argument, "rank")) rank = TRUE;
    if (argument[0] == '\0') rank = FALSE;

    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
	rgfRace[iRace] = FALSE;
    for (iClan = 0; iClan < MAX_CLAN; iClan++)
	rgfClan[iClan] = FALSE;
 
    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
	char arg[MAX_STRING_LENGTH];
 
	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;
 
	if ( is_number( arg ) )
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atoi( arg ); break;
	    case 2: iLevelUpper = atoi( arg ); break;
	    default:
		send_to_char( "Only two level numbers allowed.\n\r", ch );
		return;
	    }
	}
    }

    for ( wch = char_list ; wch != NULL ; wch = wch->next )
    {
	if (IS_NPC(wch) && !IS_FLAG5(wch, AFF5_WHO_LIST) ) continue;
	if (!can_see(ch, wch) || wch->level > 99) continue;

	score = (wch->pkills - wch->pkilled);
	if (score > best[1])
	{
	    best[5] = best[4];
	    best[4] = best[3];
	    best[3] = best[2];
	    best[2] = best[1];
	    best[1] = score;
	    continue;
	}
	else if (score > best[2])
	{
	    best[5] = best[4];
	    best[4] = best[3];
	    best[3] = best[2];
	    best[2] = score;
	    continue;
	}
	else if (score > best[3])
	{
	    best[5] = best[4];
	    best[4] = best[3];
	    best[3] = score;
	    continue;
	}
	else if (score > best[4])
	{
	    best[5] = best[4];
	    best[4] = score;
	    continue;
	}
	else if (score > best[5])
	{
	    best[5] = score;
	}
    }


 
    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output = new_buf();
        sprintf( buf2, "Most Players Ever: %d ", config_info->max_players );
    banner2_to_char(buf2, ch);

    for ( wch = char_list ; wch != NULL ; wch = wch->next )
    {
 	char const * clan; 
	char clanbuf[100];
  	bool show = FALSE;

	if (IS_NPC(wch) && !IS_FLAG5(wch, AFF5_WHO_LIST)) continue;
	if (!can_see(ch, wch)) continue;

	if ( ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
	|| ( fRaceRestrict && !rgfRace[wch->race])
	|| ( fClan && !is_clan(wch))
	|| ( fClanRestrict && !rgfClan[wch->clan]))
	    continue;
 
	nMatch++;
 
	score = (wch->pkills * 2 - wch->pkilled);

	if ( best[1] == score ) strcpy(sc,     " <I>  ");
	else if ( best[2] == score ) strcpy(sc," <II> ");
	else if ( best[3] == score ) strcpy(sc," <III>");
	else if ( best[4] == score ) strcpy(sc," <IV> ");
	else if ( best[5] == score ) strcpy(sc," <V>  ");
	else strcpy(sc, "      ");

	if (ch->level > 115 && imminfo)
	    sprintf(buf3,"%s(%-3d) ", 
			(wch->alignment < -300) ? "#5-#n" : 
			(wch->alignment > 300)  ? "#7+#n" : "#2*#n",
			wch->level);
	else
	{	
	    buf3[0]=' ';
	    buf3[1]=0;
	}


	if (wch->level > 99)
		strcpy(sc, "      ");

	/* Work out the clan - hard bit */

    clan = "";

    /* Dragon colour matching */
    if (wch->secondaryrace == RACE_DRAGON)
    {
  	if (wch->dragtype < DRAGON_RED)
            sprintf(clanbuf, " -=%s=-",clanrank[wch->secondaryrace][wch->generation].rank );
        else sprintf(clanbuf, " -=%s %s=-",dragon_colour(wch->dragtype), clanrank[wch->secondaryrace][wch->generation].rank );
	show = TRUE;
    }

    else if (wch->secondaryrace != RACE_DRAGON
	&& wch->clan == 0
	&& wch->tertiaryrace != TERTIARY_DROW)
 	sprintf(clanbuf,  " <Rogue>");

    else if (wch->tertiaryrace == TERTIARY_DROW)
    {
	if (wch->clan < CLAN_DROW_OUTCAST
	    || wch->clan > CLAN_DROW_FIFTH)
	    sprintf(clanbuf, " {Outcast}");
	else sprintf(clanbuf, "%s", clan_table[wch->clan].who_name);
    }
    else sprintf(clanbuf, "%s%s", clan_table[wch->clan].who_name,clanrank[wch->secondaryrace][wch->generation].rank);

    clan = (char *) clanbuf;

    /* Secondaryrace at start */

    if (wch->secondaryrace == RACE_VAMPIRE
	&& ch->secondaryrace != RACE_VAMPIRE) show = FALSE;
    if (wch->secondaryrace == RACE_VAMPIRE
	&& ch->secondaryrace == RACE_VAMPIRE) show = TRUE;

    else if (wch->secondaryrace == RACE_LUPINE
	&& ch->secondaryrace != RACE_LUPINE) show = FALSE;
    else if (wch->secondaryrace == RACE_LUPINE
	&& ch->secondaryrace == RACE_LUPINE) show = TRUE;

    else if (wch->secondaryrace == RACE_PSIONICIST
	&& ch->secondaryrace != RACE_PSIONICIST) show = FALSE;
    else if (wch->secondaryrace == RACE_PSIONICIST
	&& ch->secondaryrace == RACE_PSIONICIST) show = TRUE;

    else if (wch->secondaryrace == RACE_DEATH_KNIGHT) show = TRUE;
    else if (wch->secondaryrace == RACE_HERO) show = TRUE;
    else if (wch->secondaryrace == RACE_WARLOCK) show = TRUE;
    else if (wch->secondaryrace == RACE_ELADRIN) show = TRUE;
    else if (wch->secondaryrace == RACE_DAEMON) show = TRUE;
    else if (wch->secondaryrace == RACE_DRAGON) show = TRUE;

    /* Tertiary override secondary */

    if (wch->tertiaryrace == TERTIARY_DROW) show = TRUE;

    /* These 5 checks override the others */
    if (wch->level == 120
	|| ch->level == 120) show = TRUE;
    if (wch->clan == 0) show = TRUE;
    if (IS_TAINTED(wch)) show = FALSE;
    if (wch->secondaryrace == 0) show = FALSE;
    if (IS_NPC(wch)) show = TRUE;


    if (IS_NPC(wch))
    {
        strcpy(c,circlesymbol[wch->primary]);

	sprintf( buf, "%s%s %s %s%s %s#n\n\r",
	 (wch->level < 20) ? "#4(#n" : (IS_FLAG5(wch, AFF5_IMMTRUST)) ? "#7<#n" : (wch->secondaryrace == 0) ? "#5{#n" : "#3[#n",
	 (wch->primary == 6) ? "#3>#n" : c,
	 get_who_race(ch, wch),
	 (wch->primary == 6) ? "#3<#n" : c,
	 (wch->level < 20) ? "#4)#n" : (IS_FLAG5(wch, AFF5_IMMTRUST)) ? "#7>#n" : (wch->secondaryrace == 0) ? "#5}#n" : "#3]#n",
	 GET_NAME(wch));

	add_buf(output,buf);
    }
    else
    {
	char buf_beg[MAX_STRING_LENGTH];
	char buf_end[MAX_STRING_LENGTH];
	int l, i, lc;
	char title[1000], title2[1000];

        strcpy(c,circlesymbol[wch->primary]);

	sprintf(buf_beg, "%s%s %s %s%s%s%s%s%s%s",
	 (wch->level < 20) ? "#4(#n" : (IS_FLAG5(wch, AFF5_IMMTRUST)) ? "#7<#n" : (wch->secondaryrace == 0) ? "#5{#n" : "#3[#n",
	 (wch->primary == 6) ? "#3>#n" : c,
	 get_who_race(ch, wch),
	 (wch->primary == 6) ? "#3<#n" : c,
	 (wch->level < 20) ? "#4)#n" : (IS_FLAG5(wch, AFF5_IMMTRUST)) ? "#7>#n" : (wch->secondaryrace == 0) ? "#5}#n" : "#3]#n",
	 buf3,
	 wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
	 wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
	 (wch->pcdata->title_prefix == NULL) ? "" : wch->pcdata->title_prefix,
	 wch->pcdata->switchname);
	sprintf(buf_end, "%s%s#n\n\r",
	 (show) ? clan : "",
	 sc);

	lc = col_str_len(buf_beg) + col_str_len(buf_end);

	l = 70 - lc;

	for (i = 0; buf_beg[i] != 0; i++)
	    if (buf_beg[i] == '#' && buf_beg[i+1] != '#') l += 1;
	for (i = 0; buf_end[i] != 0; i++)
	    if (buf_end[i] == '#' && buf_end[i+1] != '#') l += 1;

	if (l > 900) l = 900;

	for (i = 0; i < l; i++)
	    title[i] = ' ';
	title[l] = 0;

	strncpy(title2, (IS_NPC(ch) || wch->pcdata == NULL || wch->pcdata->title == NULL) ? "" : wch->pcdata->title, 80);
	title2[80] = 0;

	for (i = 0; title2[i] != 0; i++)
	{
	    if (i >= l) break;
	    title[i] = title2[i];
	}

	sprintf(buf, "%s%s %s", buf_beg, title, buf_end);

	add_buf(output,buf);
    }
    }
    sprintf( buf2, "Players found: %d ", nMatch );
    banner2_to_buf(buf2,output);
    page_to_char( buf_string(output), ch );
    free_buf(output);
    return;
}

void do_tribes( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char sc[MAX_STRING_LENGTH];
    char c[10];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int iRace;
    int iClan;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    bool rgfRace[MAX_PC_RACE];
    bool rgfClan[MAX_CLAN];
    bool fClanRestrict = FALSE;
    bool fClan = FALSE;
    bool fRaceRestrict = FALSE;
    bool fImmortalOnly = FALSE;
    bool imminfo = FALSE;
    int best[6], score;
    bool rank = FALSE;

    best[1] = -10000;
    best[2] = -10000;
    best[3] = -10000;
    best[4] = -10000;
    best[5] = -10000;

    if (argument[0] == '\0')
    {
	stc("Syntax : tribes <tribe>\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "eladrin")) iRace = RACE_ELADRIN;
    else if (!str_prefix(argument, "lupine")
	&& ch->secondaryrace == RACE_LUPINE) iRace = RACE_LUPINE;
    else if (!str_prefix(argument, "wereshark")
	&& ch->secondaryrace == RACE_LUPINE) iRace = RACE_LUPINE;
    else if (!str_prefix(argument, "dragon")) iRace = RACE_DRAGON;
    else if (!str_prefix(argument, "demon")) iRace = RACE_DAEMON;
    else if (!str_prefix(argument, "warlock")) iRace = RACE_WARLOCK;
    else if (!str_prefix(argument, "vampire")
	&& ch->secondaryrace == RACE_VAMPIRE) iRace = RACE_VAMPIRE;
    else if (!str_prefix(argument, "hero")) iRace = RACE_HERO;
    else if (!str_prefix(argument, "death knight")) iRace = RACE_DEATH_KNIGHT;
    else if (!str_prefix(argument, "psionicist")
	&& ch->secondaryrace == RACE_PSIONICIST) iRace = RACE_PSIONICIST;
    else
    {
	do_tribes(ch, "");
	return;
    }

    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    for (iClan = 0; iClan < MAX_CLAN; iClan++)
	rgfClan[iClan] = FALSE;
 
    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
	char arg[MAX_STRING_LENGTH];
 
	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;
 
	if ( is_number( arg ) )
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atoi( arg ); break;
	    case 2: iLevelUpper = atoi( arg ); break;
	    default:
		send_to_char( "Only two level numbers allowed.\n\r", ch );
		return;
	    }
	}
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if (d->connected != CON_PLAYING || !can_see(ch,d->character)
		|| d->character->level > 111)
		continue;

	score = (d->character->pkills - d->character->pkilled);
	if (score > best[1])
	{
	    best[5] = best[4];
	    best[4] = best[3];
	    best[3] = best[2];
	    best[2] = best[1];
	    best[1] = score;
	    continue;
	}
	else if (score > best[2])
	{
	    best[5] = best[4];
	    best[4] = best[3];
	    best[3] = best[2];
	    best[2] = score;
	    continue;
	}
	else if (score > best[3])
	{
	    best[5] = best[4];
	    best[4] = best[3];
	    best[3] = score;
	    continue;
	}
	else if (score > best[4])
	{
	    best[5] = best[4];
	    best[4] = score;
	    continue;
	}
	else if (score > best[5])
	{
	    best[5] = score;
	}
    }


 
    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output = new_buf();
        sprintf( buf2, "Most Players Ever: %d ", config_info->max_players );
    banner2_to_char(buf2, ch);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *wch;
 	char const * clan; 
	char clanbuf[100];
  	bool show = FALSE;

	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	 */
	if ( d->connected != CON_PLAYING || !can_see( ch, d->character))
	    continue;

	wch   = ( d->original != NULL ) ? d->original : d->character;

	if (wch->secondaryrace != iRace) continue;
 
	if (!can_see(ch,wch))
	    continue;

	if ( ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
	|| ( fRaceRestrict && !rgfRace[wch->race])
	|| ( fClan && !is_clan(wch))
	|| ( fClanRestrict && !rgfClan[wch->clan]))
	    continue;
 
	nMatch++;
 
	score = (d->character->pkills - d->character->pkilled);
	if ( best[1] == score ) strcpy(sc,"<I>");
	else if ( best[2] == score ) strcpy(sc,"<II>");
	else if ( best[3] == score ) strcpy(sc,"<III>");
	else if ( best[4] == score ) strcpy(sc,"<IV>");
	else if ( best[5] == score ) strcpy(sc,"<V>");
	else strcpy(sc, "");

	if (ch->level > 115 && imminfo)
	    sprintf(buf3,"%s(%-3d) ", 
			(wch->alignment < -300) ? "#5-#n" : 
			(wch->alignment > 300)  ? "#7+#n" : "#2*#n",
			wch->level);
	else
	{
	    buf3[0]=' ';
	    buf3[1]=0;
	}


	if (d->character->level > 111)
		strcpy(sc, "");

	/* Work out the clan - hard bit */

    clan = "";


    /* Dragon colour matching */
    if (wch->secondaryrace == RACE_DRAGON)
    {
  	if (wch->dragtype < DRAGON_RED)
            sprintf(clanbuf, " -=%s=-",clanrank[wch->secondaryrace][wch->generation].rank );
        else sprintf(clanbuf, " -=%s %s=-",dragon_colour(wch->dragtype), clanrank[wch->secondaryrace][wch->generation].rank );
	show = TRUE;
    }

    else if (wch->secondaryrace != RACE_DRAGON
	&& wch->clan == 0
	&& wch->tertiaryrace != TERTIARY_DROW)
 	sprintf(clanbuf,  " <Rogue>");

    else if (wch->tertiaryrace == TERTIARY_DROW)
    {
	if (wch->clan < CLAN_DROW_OUTCAST
	    || wch->clan > CLAN_DROW_FIFTH)
	    sprintf(clanbuf, " {Outcast}");
	else sprintf(clanbuf, "%s", clan_table[wch->clan].who_name);
    }
    else sprintf(clanbuf, "%s%s", clan_table[wch->clan].who_name,clanrank[wch->secondaryrace][wch->generation].rank);

    clan = (char *) clanbuf;

    /* Secondaryrace at start */

    if (wch->secondaryrace == RACE_VAMPIRE
	&& ch->secondaryrace != RACE_VAMPIRE) show = FALSE;
    if (wch->secondaryrace == RACE_VAMPIRE
	&& ch->secondaryrace == RACE_VAMPIRE) show = TRUE;

    else if (wch->secondaryrace == RACE_LUPINE
	&& ch->secondaryrace != RACE_LUPINE) show = FALSE;
    else if (wch->secondaryrace == RACE_LUPINE
	&& ch->secondaryrace == RACE_LUPINE) show = TRUE;

    else if (wch->secondaryrace == RACE_PSIONICIST
	&& ch->secondaryrace != RACE_PSIONICIST) show = FALSE;
    else if (wch->secondaryrace == RACE_PSIONICIST
	&& ch->secondaryrace == RACE_PSIONICIST) show = TRUE;

    else if (wch->secondaryrace == RACE_DEATH_KNIGHT) show = TRUE;
    else if (wch->secondaryrace == RACE_HERO) show = TRUE;
    else if (wch->secondaryrace == RACE_WARLOCK) show = TRUE;
    else if (wch->secondaryrace == RACE_ELADRIN) show = TRUE;
    else if (wch->secondaryrace == RACE_DAEMON) show = TRUE;
    else if (wch->secondaryrace == RACE_DRAGON) show = TRUE;

    /* Tertiary override secondary */

    if (wch->tertiaryrace == TERTIARY_DROW) show = TRUE;


    /* These 4 checks override the others */
    if (wch->level == 120
	|| ch->level == 120) show = TRUE;

    if (wch->clan == 0) show = TRUE;

    if (IS_TAINTED(wch)) show = FALSE;

    if (wch->secondaryrace == 0) show = FALSE;


	/*
	 * Format it up.
	 */
	    strcpy(c,circlesymbol[wch->primary]);

	    sprintf( buf, "%s%s %s %s%s%s%s%s%s%s%s%s %s#n\n\r",
	    (wch->level < 20) ? "#4(#n" : (IS_FLAG5(wch, AFF5_IMMTRUST)) ? "#7<#n" : (wch->secondaryrace == 0) ? "#5{#n" : "#3[#n",
	    (wch->primary == 6) ? "#3>#n" : c,
	    get_who_race(ch, wch),
	    (wch->primary == 6) ? "#3<#n" : c,
	    (wch->level < 20) ? "#4)#n" : (IS_FLAG5(wch, AFF5_IMMTRUST)) ? "#7>#n" : (wch->secondaryrace == 0) ? "#5}#n" : "#3]#n",
	    buf3,
	    wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
	    wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
	    (wch->pcdata->title_prefix == NULL) ? "" : wch->pcdata->title_prefix,
	    wch->pcdata->switchname,
	    IS_NPC(wch) ? "" : wch->pcdata->title,
	    (show) ? clan : "",
	    (rank) ? "" : "");
	add_buf(output,buf);
    }

    sprintf( buf2, "Players found: %d ", nMatch );
    page_to_char( buf_string(output), ch );
    free_buf(output);
    banner2_to_char(buf2,ch);
    return;
}

/* The seven letters at the start of the who name */

char * get_who_race (CHAR_DATA *ch, CHAR_DATA *wch)
{
    crash("get who race!");

    if (!IS_NPC(wch))
    {
        if (IS_SET(wch->comm, COMM_AFK))
	    return "#3-#6>#7AFK#6<#3-#n";

        if (!str_cmp(wch->pcdata->switchname, "Mitzrah"))
	    return  "#5Friend#n";

        if (wch->registerred != 1 && wch->email_set != 0) return " <U-R> ";
            else if (wch->registerred != 1) return " <U R> ";

        if (wch->level > 99 && wch->level < 112) return " Lord ";
        if (wch->level > 111 && wch->level < 113) return " Ruler ";
	if (wch->level > 112 && wch->level < 115) return " Diety ";
	if (wch->level > 114 && wch->level < 117) return "Godling";
	if (wch->level > 116 && wch->level < 119) return "Demigod";
	if (wch->level > 118 && wch->level < 120) return "Wierdo";
    } 

    /* Taint must come first */
    if (!TIME_UP(wch, TIMER_TAINT))
    {
        if (wch->race == RACE_ELF) return "  Elf  ";
        if (wch->race == RACE_GNOME) return " Gnome ";
        if (wch->race == RACE_HUMAN) return " Human ";
        if (wch->race == RACE_HALF_ELF) return "HalfElf";
	bug("wrong tainted race", 0);
    }

    if (wch->tertiaryrace == TERTIARY_DROW) return "  Drow ";
    else if (wch->tertiaryrace == TERTIARY_LICH) return "  Lich ";
    else if (wch->tertiaryrace == TERTIARY_AASIMON) return "Aasimon";
    else if (wch->tertiaryrace == TERTIARY_WITCH) return " Witch ";

    if (wch->secondaryrace == RACE_DRAGON) return " Dragon";

    if (wch->secondaryrace == RACE_DEATH_KNIGHT) return " Death ";

    if (wch->secondaryrace == RACE_PSIONICIST) return "Psionic";

    if (wch->secondaryrace == RACE_DAEMON)
	return " Demon ";

    if (wch->secondaryrace == RACE_ELADRIN)
	return "Eladrin";

    if ((ch->secondaryrace == RACE_VAMPIRE || ch->level == 120)
	&& wch->secondaryrace == RACE_VAMPIRE)
	return "Vampire";

    if ((ch->secondaryrace == RACE_LUPINE || ch->level == 120)
	&& wch->secondaryrace == RACE_LUPINE)
	return " Lupine";

    if ((ch->secondaryrace == RACE_WARLOCK || ch->level == 120)
	&& wch->secondaryrace == RACE_WARLOCK)
	return "Warlock";

    if (IS_NPC(wch)) return "  Mob  ";

    if (wch->race == RACE_ELF) return "  Elf  ";
    if (wch->race == RACE_GNOME) return " Gnome ";
    if (wch->race == RACE_HUMAN) return " Human ";
    if (wch->race == RACE_HALF_ELF) return "HalfElf";
    return " Human ";
}



void do_maskwho( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int nMatch=0;

    buf[0] = '\0';
    output = new_buf();
    divide6_to_char(ch);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *wch;
	char const *class;
 
	if ( d->connected != CON_PLAYING )
	    continue;
 
	wch   = ( d->original != NULL ) ? d->original : d->character;

	if ( !str_cmp(wch->name, wch->pcdata->switchname) )
	    continue;
 
	nMatch++;
 
	/*
	 * Figure out what to print for class.
	 */
	class = pc_race_table[wch->race].who_name;
	if (wch->secondaryrace>0) 
		class = secondaryrace[wch->secondaryrace].name;
	if (wch->tertiaryrace>0)
		class = tertiaryrace[wch->tertiaryrace].whoname;
	if (wch->level >99) class = "  Ruler  ";
	switch ( wch->level )
	{
	default: break;
	    {
		/* Justify to 13 characters */
		case 120 : class = " Creator ";     break;
		case 119 : class = "  Power  ";     break;
		case 118 : class = "  Power  ";     break;
		case 117 : class = "  Power  ";     break;
		case 116 : class = "  Power  ";     break;
		case 115 : class = "  Deity  ";     break;
		case 114 : class = "  Deity  ";     break;
		case 113 : class = "  Deity  ";     break;
		case 112 : class = "  Deity  ";     break;
	    }
	}

	/*
	 * Format it up.
	 */

	    sprintf( buf, "[%s%s%s] %s%s%s%s masked as %s%s%s\n\r",
	    
	    circlesymbol[wch->primary],
	    class,
	    circlesymbol[wch->primary],
	    wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
	    wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
	    IS_SET(wch->comm, COMM_AFK) ? "[AFK] " : "",
	    wch->pcdata->switchname,
	    wch->name,
	    clan_table[wch->clan].who_name,
	    clanrank[wch->secondaryrace][wch->generation].rank);
	add_buf(output,buf);
    }

    sprintf( buf2, "Masked players found: %d ", nMatch );
    page_to_char( buf_string(output), ch );
    free_buf(output);
    banner2_to_char(buf2,ch);
    return;
}

void do_clanwho( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int nMatch=0;

    if (ch->clan == 0)
    {
	stc("You aren't in a clan.\n\r", ch);
	return;
    }

    buf[0] = '\0';
    output = new_buf();
    divide6_to_char(ch);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *wch;
	char const *class;
 
	if ( d->connected != CON_PLAYING )
	    continue;
 
	wch   = ( d->original != NULL ) ? d->original : d->character;

	if ( wch->clan != ch->clan || ch->clan == 0)
	    continue;
 
	nMatch++;
 
	/*
	 * Figure out what to print for class.
	 */
	class = pc_race_table[wch->race].who_name;
	if (wch->secondaryrace>0) 
		class = secondaryrace[wch->secondaryrace].name;
	if (wch->tertiaryrace>0)
		class = tertiaryrace[wch->tertiaryrace].whoname;
	if (wch->level >99) class = "  Ruler  ";
	switch ( wch->level )
	{
	default: break;
	    {
		/* Justify to 13 characters */
		case 120 : class = " Creator ";     break;
		case 119 : class = "  Power  ";     break;
		case 118 : class = "  Power  ";     break;
		case 117 : class = "  Power  ";     break;
		case 116 : class = "  Power  ";     break;
		case 115 : class = "  Deity  ";     break;
		case 114 : class = "  Deity  ";     break;
		case 113 : class = "  Deity  ";     break;
		case 112 : class = "  Deity  ";     break;
	    }
	}

	/*
	 * Format it up.
	 */

	    sprintf( buf, "[%s%s%s] %s%s%s%s%s%s%s\n\r",
	    
	    circlesymbol[wch->primary],
	    class,
	    circlesymbol[wch->primary],
	    wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
	    wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
	    IS_SET(wch->comm, COMM_AFK) ? "[AFK] " : "",
	    wch->pcdata->switchname,
	    IS_NPC(wch) ? "" : wch->pcdata->title,
	    clan_table[wch->clan].who_name,
	    clanrank[wch->secondaryrace][wch->generation].rank);
	add_buf(output,buf);
    }

    sprintf( buf2, "Players in your clan: %d ", nMatch );
    page_to_char( buf_string(output), ch );
    free_buf(output);
    stc("\n\r",ch);

    if (!is_clan_leader(ch))
        sprintf(buf3, "The leader of clan %s is %s.\n\r",
	    clan_table[ch->clan].name, clan_data[ch->clan].leader);
    else sprintf(buf3, "You are the leader of this sorry clan.\n\r");

    ctc(buf3, ch);

    banner2_to_char(buf2,ch);

    return;
}

void do_count ( CHAR_DATA *ch, char *argument )
{
    int count;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    count = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
	if ( d->connected == CON_PLAYING && can_see( ch,d->character) )
	    count++;

    max_on = UMAX(count,max_on);

    if (max_on == count)
	sprintf(buf,"There are %d characters on, the most so far today.\n\r",
	    count);
    else
	sprintf(buf,"There are %d characters on, the most on today was %d.\n\r",
	    count,max_on);

    send_to_char(buf,ch);
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    if (is_item(ch))
    {
	bool contained = FALSE;
	OBJ_DATA *obj;
	
	stcf(ch, "You are %s.\n\r", ch->sw_obj->short_descr);

	if (ch->sw_obj->contains == NULL)
	{
	    stc("You are not a container, however.\n\r", ch);
	    return;
	}

	for (obj = ch->sw_obj->contains ; obj != NULL ; obj = obj->next_content)
	{
	    stcf(ch, "You contain %s.\n\r", obj->short_descr);
	    contained = TRUE;
	}

	if (!contained)
	    stc("There is nothing inside you at the moment.\n\r", ch);
	return;
    }

    banner2_to_char("YOU ARE CARRYING",ch);
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    divide6_to_char(ch);
    return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;
    int hands;

    if (is_item(ch))
    {
	stc("You cannot wear anything, you are an object.\n\r", ch);
	return;
    }
  
    banner2_to_char("YOU ARE USING",ch);
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	    continue;

	send_to_char( where_name[iWear][0], ch );
	if ( can_see_obj( ch, obj ) )
	{
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );

	    /* 2 handed???? */
	    if (obj->item_type != ITEM_ARMOR
		&& (hands = calc_hands_needed(ch, obj)) == 2)
	        stc("    (2 handed)", ch);
	    else if (obj->item_type != ITEM_ARMOR
		&& hands == 4)
	        stc("    (4 handed)", ch);

	    send_to_char( "\n\r", ch );
	}
	else
	{
	    send_to_char( "something.\n\r", ch );
	}
	found = TRUE;
    }

    if ( !found )
	banner2_to_char( "NOTHING", ch );
    else
	divide6_to_char(ch);
    return;
}



void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (arg2[0] == '\0')
    {
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
	    if (obj2->wear_loc != WEAR_NONE
	    &&  can_see_obj(ch,obj2)
	    &&  obj1->item_type == obj2->item_type
	    &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if (obj2 == NULL)
	{
	    send_to_char("You aren't wearing anything comparable.\n\r",ch);
	    return;
	}
    } 

    else if ( (obj2 = get_obj_carry(ch,arg2,ch) ) == NULL )
    {
	send_to_char("You do not have that item.\n\r",ch);
	return;
    }

    msg         = NULL;
    value1      = 0;
    value2      = 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;

	case ITEM_WEAPON:
	    if (obj1->pIndexData->new_format)
		value1 = (1 + obj1->value[2]) * obj1->value[1];
	    else
		value1 = obj1->value[1] + obj1->value[2];

	    if (obj2->pIndexData->new_format)
		value2 = (1 + obj2->value[2]) * obj2->value[1];
	    else
		value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}




void do_eaglesight( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vict;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument( argument, arg );

    if (argument[0] == '\0')
    {
	stc("Who do you wish to use eagle sight on?\n\r", ch);
	return;
    }

    if ((vict = get_char_world(ch, argument)) == NULL
	|| IS_SET(vict->flag2, AFF_SHIELD_AURA))
    {
	stc("You cannot locate that person.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 6);

	send_to_char( "You scry throughout the land.\n\r", ch );
	found = FALSE;
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   IS_OUTSIDE(victim)
	    &&   !IS_SET(victim->flag2, AFF_SHIELD_AURA)
	    &&   victim->in_room != NULL
	    &&   !IS_SET(victim->in_room->room_flags,ROOM_NOWHERE)
	    &&   victim->in_room->area == vict->in_room->area
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    victim->name, victim->in_room->name );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "You did not locate anybody.\n\r", ch );
}


void do_where( CHAR_DATA *ch, char *argument ) 
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "You close your eyes and concentrate...\n\r", ch );
	found = FALSE;
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   victim->in_room != NULL
	    &&   !IS_SET(victim->in_room->room_flags,ROOM_NOWHERE)
	    &&   (is_room_owner(ch,victim->in_room) 
	    ||    !room_is_private(victim->in_room))
	    &&   victim->in_room->area == ch->in_room->area
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    victim->name, victim->in_room->name );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "None\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   !IS_AFFECTED(victim, AFF_HIDE)
	    &&   !IS_AFFECTED(victim, AFF_SNEAK)
	    &&   can_see( ch, victim )
	    &&   is_name( arg, victim->name ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    PERS(victim, ch), victim->in_room->name );
		send_to_char( buf, ch );
		break;
	    }
	}
	if ( !found )
	    act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}


void do_roots( CHAR_DATA *ch, char *argument ) 
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    if (ch->in_room->sector_type != SECT_FOREST)
    {
	stc("You may only use this power in the forest.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 8);
    stc("You ask the forest for information.\n\r", ch );
    for ( d = descriptor_list; d; d = d->next )
    {
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   victim->in_room != NULL
	    &&   !IS_SET(victim->in_room->room_flags,ROOM_NOWHERE)
	    &&   victim->in_room->area == ch->in_room->area
	    &&   victim->in_room->sector_type == SECT_FOREST
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    victim->name, victim->in_room->name );
		send_to_char( buf, ch );
	    }
    }
	if ( !found )
	    stc( "There is nobody in the nearby forest.\n\r", ch );
}

void do_readaura( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int diff;
    char buf[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Whose aura do you wish to read?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    if ( IS_NEWFLAG(victim, AFF_SHIELD_AURA) )
    {
	send_to_char( "Their aura is too powerful to read.\n\r", ch );
	return;
    }

    diff = ch->level - victim->level;
    act("$n stares at $N intently, sending shivers up $S spine",ch,NULL,victim,TO_NOTVICT);
    act("$n is staring at you, creepy eh?",ch,NULL,victim,TO_VICT);
    
    if (diff <= -30)
    {
	act("You stare at $N intently but fail to gain any information." ,ch,NULL,victim,TO_CHAR);
    }
    if (diff > -30)
    {
	act("You read $N's aura and gain the following information :\n\r",ch,NULL,victim,TO_CHAR);
	sprintf( buf, " Hit Points   : %d	\n\r", victim->hit);
	send_to_char(buf, ch);
    }
    if (diff > -10)
    {
	sprintf( buf, " Armour Class : %d   %d   %d   %d \n\r", 
		victim->armor[0]+(diff/2), 
		victim->armor[1]+(diff/2),
		victim->armor[2]+(diff/2),
		victim->armor[3]+(diff/2));
	send_to_char(buf, ch);
    }
    if (diff > 5)
    {
	sprintf( buf, " Magic Power  : %d \n\r", victim->fire + victim->earth
			+ victim->air + victim->water + victim->entropy);
	send_to_char(buf, ch);
    }
    if (diff > 20)
    {
	sprintf( buf, " Alignment    : %d \n\r", victim->alignment);
	send_to_char(buf, ch);
    }
    if (ch->power[DISC_VAMP_AUSP] > 7 && !IS_NPC(victim) )
    {
	sprintf(buf, " Skills        : %d %d %d %d %d %d \n\r", 
	    victim->pcdata->treelearn[SKILL_WEAPON],
	    victim->pcdata->treelearn[SKILL_COMBAT],
	    victim->pcdata->treelearn[SKILL_THIEF],
	    victim->pcdata->treelearn[SKILL_WIZARD],
	    victim->pcdata->treelearn[SKILL_PRIEST],
	    victim->pcdata->treelearn[SKILL_KNIGHT]);
   	send_to_char(buf, ch);
    }
    return;
}

void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
    {
	send_to_char("Don't even think about it.\n\r",ch);
	return;
    }

    diff = ((victim->hit) * 100) / ((ch->hit < 1) ? 1 : ch->hit);

	 if ( diff <= 40 ) msg = "You are much healthier than $M.";
    else if ( diff <= 60 ) msg = "You are healthier than $M.";
    else if ( diff <= 80 ) msg = "You are slightly healthier than $M.";
    else if ( diff <= 100 ) msg = "You are of similar health.";
    else if ( diff <= 120 ) msg = "$N is slightly healthier than you.";
    else if ( diff <= 140 ) msg = "$N is healthier than you.";
    else                    msg = "$N is much healthier than you.";

    act( msg, ch, NULL, victim, TO_CHAR );

    diff = victim->level - ch->level;


	 if ( diff <= -10 ) msg = "Why waste your time with such weaklings.";
    else if ( diff <=  -5 ) msg = "$N is no match for you.";
    else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
    else if ( diff <=   1 ) msg = "You seem of roughly equal ability.";
    else if ( diff <=   4 ) msg = "$N says 'Make my day'.";
    else if ( diff <=   9 ) msg = "$N laughs at you mercilessly.";
    else                    msg = "I think you better make a will first.";

    act( msg, ch, NULL, victim, TO_CHAR );

    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH]; 

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}

void set_title_prefix( CHAR_DATA *ch, char *title )
{
    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if (title[0] == '.')
    {
    	free_string( ch->pcdata->title_prefix );
	ch->pcdata->title_prefix = NULL;
	return;
    }

    free_string( ch->pcdata->title_prefix );
    ch->pcdata->title_prefix = str_dup( title );
    return;
}


void set_switchname( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_switchname: NPC.", 0 );
	return;
    }
    strcpy( buf, title );
    free_string( ch->pcdata->switchname );
    ch->pcdata->switchname = str_dup( buf );
    return;
}

void set_pc_name( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    crash("set pc name has been called");

    if ( IS_NPC(ch) )
    {
	bug( "Set_pc_name: NPC.", 0 );
	return;
    }
    strcpy( buf, title );
    free_string( ch->name);
    ch->name = str_dup( buf );
    return;
}




void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }

    if ( strlen(argument) > 45 )
	argument[45] = '\0';

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );
}

void set_enter_room_text( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH]; 

    if ( IS_NPC(ch) )
	return;

    if ( strlen(argument) > 45 )
	argument[45] = '\0';

    smash_tilde( argument );

    strcpy( buf, argument );

    free_string( ch->pcdata->enter_room );
    ch->pcdata->enter_room = str_dup( buf );
    return;

}



void do_description( CHAR_DATA *ch, char *argument )
{
    if (!IS_SET(ch->in_room->room_flags, ROOM_SAFE) && ch->level < 112)
    {
	stc("You can only set your description in a safe room.\n\r", ch);
	return;
    }

    do_edit(ch, &ch->description, NULL, NULL);
    return;
}



void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf,
	"You say 'I have %d/%d hp and %d/%d mv.'\n\r",
	ch->hit,  ch->max_hit,
	ch->move, ch->max_move);

    send_to_char( buf, ch );

    sprintf( buf, "$n says 'I have %d/%d hp and %d/%d mv.'",
	ch->hit,  ch->max_hit,
	ch->move, ch->max_move);

    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}

/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
	return;
    }

    ch->wimpy   = wimpy;
    sprintf( buf, "You will now flee on %d hit points.\n\r", wimpy );
    send_to_char( buf, ch );
    return;
}


void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}


/**************  All new clan commands added here *************/
/**************         Don't ask why             *************/
/**************            Please                 *************/

int souls_needed(CHAR_DATA *ch)
{
    if (ch->generation < 6) return ch->generation * 5;
    return ((ch->generation - 5) * 7) + 25;
}

void do_promote (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    bool override = FALSE;

    if (IS_NPC(ch)) return;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Who do you wish to promote?\n\r", ch );
	return;
    }

    if (argument[0] != 0 && !str_prefix(argument, "override") && IS_CREATOR(ch))
    {
	stc("Manual override granted.\n\r", ch);
	override = TRUE;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("You cannot promote a NPC.\n\r", ch);
	return;
    }

    if (victim->secondaryrace != RACE_DAEMON)
    {
        send_to_char("Only demons can be promoted in this fashion.\n\r", ch);
        return;
    }

    if ( victim == ch )
    {
        send_to_char("You must be promoted by another demon.\n\r",ch);
        return;
    }

    if ( victim->secondaryrace != ch->secondaryrace && ch->level!=120)
    {
        send_to_char("You can't promote someone of a different race.\n\r",ch);
        return;
    }
     
    if ( victim->generation >= ch->generation)
    {
        send_to_char("You can't promote someone of an equal or higher rank.\n\r", ch);
        return;
    }

    if (ch->clan == CLAN_DAEM_OUTCAST)
    {
        send_to_char("You must be in a clan to promote other demons.\n\r", ch);
        return;
    }

    if (ch->clan != victim->clan)
    {
        send_to_char("You can only promote demons within your own clan.\n\r", ch);
        return;
    }
 
    if (ch->secondaryrace == RACE_DAEMON)
    {
        if (ch->generation < 10)
        {
            send_to_char("You must be off Cornugon rank or higher to promote other demons.\r\n", ch);
            return;
        }
        else if (ch->generation < 11
                 && victim->generation > 4)
        {
            send_to_char("Cornugons may only promote demons of Barbazu rank and lower.\n\r", ch);
            return;
        }
        else if (ch->generation < 12
                 && victim->generation > 7)
        {
            send_to_char("Gelugons may only promote demons of Hamatula rank and lower.\r\n", ch);
            return;
        }
    }

    if (victim->generation == 10
	&& victim->power[DISC_DAEM_GELU] < 10)
    {
	send_to_char("Only a demon with level 10 Geluge may become a Gelugon.\n\r", ch);
	return;
    }

    if (victim->pcdata->souls < souls_needed(victim) && !override)
    {
	send_to_char("They have not collected enough souls to be promoted.\n\r", ch);
	return;
    }

    /* WHOO WHOO ... THEY CAN NOW BE PROMOTED :) */

    send_to_char("You have been chosen for promotion.\n\r",victim);

    if ( victim->generation == 13)
    {
	send_to_char("But you cannot transform any further.\n\r",victim);
	send_to_char("They are already maximum generation.\n\r",ch);
	return;
    }

    sprintf(buf, "--> %s has been promoted by %s.", victim->name, ch->name);
    informational(buf, ch->secondaryrace, 0);

    set_form(victim, get_normal_form(victim) + 1);
    victim->generation++;
    look_name(victim);

    /* DEMON GENERATIONS....  HAVE FUN */
    
    if (victim->secondaryrace == 2 && victim->generation == 3)
    {
	SET_BIT(victim->flag2,AFF_BITE); 
	SET_BIT(victim->flag2,AFF_CLAW); 
    }

    if (victim->secondaryrace == 2 && victim->generation == 8)
    {
	REMOVE_BIT(victim->flag2,AFF_BITE); 
	REMOVE_BIT(victim->flag2,AFF_CLAW); 
    }

    if (victim->secondaryrace == 2 && victim->generation > 8)
    {
	victim->perm_stat[STAT_STR]+=1;
    }

    if (victim->secondaryrace == 2 && victim->generation == 9)
    {
	SET_BIT(victim->flag2,AFF_BITE); 
	SET_BIT(victim->flag2,AFF_CLAW); 
    }

    if (victim->secondaryrace == 2 && victim->generation == 8)
    {
	SET_BIT(victim->affected_by, AFF_FLYING); 
    }

    victim->hit += 100;
    victim->max_hit += 100;
    victim->pcdata->perm_hit += 100;
    victim->max_psp += dice (1,8);
    victim->pcdata->damcap += ch->generation * 10;
    victim->xhitroll++;
    victim->xdamroll++;

    send_to_char(transformation_message[0][victim->generation-2],victim);
    act("$N has been successfully promoted to the next rank.", ch, NULL, victim, TO_CHAR);
}

void do_bite(CHAR_DATA *ch, char *argument)
{           
    char arg[MAX_INPUT_LENGTH];   
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    int i;
    argument = one_argument( argument, arg );
	
    if ( arg[0] == '\0' )
    {
	send_to_char( "Who do you wish to bite?\n\r", ch );
	return;
    }

    if (ch->generation <= 1 || IS_TAINTED(ch))
    {
	send_to_char("Your blood is too weak to pass on the curse.\n\r",ch);
	return;
    }
	
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    
    if (!can_be_made(victim))
    {
	send_to_char("Please do not pollute your vampiric lineage.\n\r", ch);
	return;
    }

    if ( victim->secondaryrace != 0)
    {
	send_to_char("This glorious transformation is reserved for mortals alone.\n\r",ch);
	return;
    }

    if ( victim->race == RACE_GNOME)
    {
	send_to_char("Eugh! You dont want to bite one of those.\n\r",ch);
	return;
    }

    if ( victim->level < secondaryrace[3].min_level )
    {
	send_to_char("They are not powerful enough to survive the transformation.\n\r",ch);
	return;
    }

    if ( !IS_NEWFLAG(victim, AFF_ALLOW_VAMP) )
    {
	send_to_char("They are not a willing subject.\n\r",ch);
	return;
    }

    if (!IS_NEWFLAG(ch, AFF_BITE) )
    {
	send_to_char("You have your fangs retracted.\n\r", ch);
	return;
    }

    if (ch == victim)
    {
	send_to_char("You can't bite yourself.\n\r", ch);
	return;
    }

    victim->secondaryrace = 3;
    victim->generation = ch->generation - 1;
 
    set_learnable_disciplines(victim, RACE_VAMPIRE);
    SET_BIT(victim->act, ACT_UNDEAD);
    SET_TIMER(victim, TIMER_VAMPIRE_GROW, 3000);
    victim->hit = 1;
    victim->perm_stat[STAT_STR] += 1;
    victim->perm_stat[STAT_DEX] += 1;
    victim->move = 0;
    victim->clan = CLAN_VAMP_ANARCH;
    victim->cur_form = 0;
    victim->pcdata->condition[COND_FULL] = 0;
    victim->pcdata->condition[COND_HUNGER] = 40;
    victim->pcdata->condition[COND_THIRST] = 40;
    victim->pcdata->condition[COND_DRUNK] = 0;
    victim->blood = ch->blood/2;
    victim->pcdata->damcap = 100;
    ADD_INNATE(victim, skill_lookup("cause light"), 2, 8);
    look_name(victim);
    if (victim->blood > max_blood_pool(victim) )
	victim->blood = max_blood_pool(victim);
    victim->alignment = -1000;
    for (i = 0; i < 13; i++)
	if (victim->pcdata->lineage[i] != NULL) 
	    free_string(victim->pcdata->lineage[i]);

    for (i = 0; i < 13; i++)
    {
	if (victim->pcdata->lineage[i] != NULL)
	    free_string(victim->pcdata->lineage[i]);
	if (ch->pcdata->lineage[i] != NULL)
	{
	    victim->pcdata->lineage[i] = str_dup(ch->pcdata->lineage[i]);
	}
    }
    
    victim->pcdata->lineage[13 - ch->generation] =str_dup(ch->pcdata->switchname);
    
    sprintf(buf, "--> %s has been bitten by %s.", victim->name, ch->name);
    informational(buf, 3 , 0);

    if (victim->entropy < 2)
    {
	if (victim->entropy < 1)
		gn_add(victim, 50);
	gn_add(victim, 51);
	victim->entropy = 2;
	victim->pcdata->max_memorize[1]++;
	victim->pcdata->max_memorize[2]++;
    }

    ch->hit = ch->hit / 5;

    WAIT_STATE(ch, 10);
    WAIT_STATE(ch, 60);

    send_to_char("You plunge your fangs deep into their neck, draining their life force.\n\r",ch);
    send_to_char("As they are on the point of dying, you feed them some of your lifeblood.\n\r",ch);
    send_to_char("You feel a sharp pain stab down your spine from your kneck.  Your vision\n\r",victim);
    send_to_char("blurs and starts spinning, but you manage to focus on a trickle of blood\n\r",victim);
    send_to_char("flowing from your neck.  Everthing slowly fades to black as your life\n\r",victim);
    send_to_char("force escapes your grasp.\n\n\r",victim);
    send_to_char("You open your eyes, and wretch.  Welcome to Undeath.\n\r",victim);
    act("$n sinks $s fangs deep into $N.  Blood pouring from $S wound,
$N collapses to the ground.  $n then feeds $N blood from $s wrist, 
giving life back to the body.",ch,NULL,victim,TO_NOTVICT); 
    save_char_obj(victim);
    save_char_obj(ch);

}



void do_learn(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int n, i;    
    int nine;
    int six;
    nine = 9;
    six = 7;

    if (!can_cast_flame(ch)
	&& !can_cast_earth(ch)
	&& !can_cast_air(ch)
	&& !can_cast_water(ch)
	&& !can_cast_entropy(ch)
	&& !can_cast_life(ch))
    {
	stc("You cannot learn any spells.\n\r", ch);
	return;
    }

    if (IS_NPC(ch))
    {
	send_to_char("You have no spells.\n\r", ch);
	return;
    }

    for (i = 1 ; i < 11 ; i++)
    {
        if (ch->pcdata->max_memorize[i] > 9)
            ch->pcdata->max_memorize[i] = 9;
    }

    if (ch->secondaryrace == RACE_WARLOCK)
    {
	nine = 9;
	six = 8;
    }
    
    if (ch->level == 120)
    {
	nine = 10;
	six = 10;
    }

    /* This one must come last... its important */
    if (IS_TAINTED(ch))
    {
	nine = 4;
	six = 4;
    }

    if ( argument[0] == '\0' )
    {
	cent_to_char("- ---===> Realms of Magic <===--- -", ch);
	send_to_char( "\n\r", ch);
	sprintf(buf, "     Flame       %d   %s    %s\n\r",
		ch->fire,
		(ch->fire < 10) ? " " : "",
		(ch->pcdata->true_sex == SEX_FEMALE) ? magabil_table[ch->fire][0].her : magabil_table[ch->fire][0].him);
	if (can_cast_flame(ch))
	send_to_char(buf,ch);
	sprintf(buf, "     Water       %d    %s   %s\n\r",
		ch->water,
		(ch->water < 10) ? " " : "",
		(ch->pcdata->true_sex == SEX_FEMALE) ? magabil_table[ch->water][1].her : magabil_table[ch->water][1].him);
	if (can_cast_water(ch))
	send_to_char(buf,ch);

	sprintf(buf, "     Air         %d  %s     %s\n\r",
		ch->air,
		(ch->air < 10) ? " " : "",
		(ch->pcdata->true_sex == SEX_FEMALE) ? magabil_table[ch->air][2].her : magabil_table[ch->air][2].him);
	if (can_cast_air(ch))
	send_to_char(buf,ch);

	sprintf(buf, "     Earth       %d    %s   %s\n\r",
		ch->earth,
		(ch->earth < 10) ? " " : "",
		(ch->pcdata->true_sex == SEX_FEMALE) ? magabil_table[ch->earth][3].her : magabil_table[ch->earth][3].him);
	if (can_cast_earth(ch))
	send_to_char(buf,ch);

	sprintf(buf, "     Entropy     %d   %s    %s\n\r", 
		ch->entropy,
		(ch->entropy < 10) ? " " : "",
		(ch->pcdata->true_sex == SEX_FEMALE) ? magabil_table[ch->entropy][4].her : magabil_table[ch->entropy][4].him);
	if (can_cast_entropy(ch))
	send_to_char(buf,ch);

	sprintf(buf, "     Life        %d   %s    %s\n\r", 
		ch->lifemagic,
		(ch->lifemagic < 10) ? " " : "",
		(ch->pcdata->true_sex == SEX_FEMALE) ? magabil_table[ch->lifemagic][5].her : magabil_table[ch->lifemagic][5].him);
	if (can_cast_life(ch))
	send_to_char(buf,ch);


	send_to_char("\n\r",ch);
	sprintf(buf, "       Your faith is %d.", ch->faith);
	send_to_char(buf, ch);
	send_to_char("\n\r",ch);
	sprintf(buf, "\n         Memorization : ");
	if (ch->pcdata->max_memorize[1] > 0)
	{
	    for (i=1 ; i < 11 ; i++)
 	    {
		sprintf(buf2, "%d ", ch->pcdata->max_memorize[i]);
		strcat(buf, buf2);
	    }
	    strcat(buf, "\n\r");
	    send_to_char(buf , ch);
	}
	return;
    }

    if (ch->level < 1)
    {
	send_to_char("You are too inexperienced to learn the arcane arts.\n\r",ch);
	return;
    }

    check_player_table(ch);

    if ( !str_prefix(argument,"life")
	 && can_cast_life(ch))
    {
	if (ch->alignment < 500)
	{
	    send_to_char("You are too evil to learn life magic.\n\r", ch);
	    return;
	}

	n = ch->lifemagic;
	if (((n+2)*(n+1))/2 > ch->faith)
	{
	   sprintf(buf, "You need %d more faith points in life.\n\r",
		((n+2)*(n+1))/2 - ch->faith);
	   send_to_char(buf,ch);
	   return;
	}
	if ((ch->primary == 6 && ch->lifemagic < nine) || (ch->lifemagic < six))
	{
		send_to_char("You feel more pure and holy.\n\r",ch);
	 	ch->alignment = UMIN(1000, ch->alignment + 100);
		ch->faith -= ((n+2)*(n+1))/2;

		ch->lifemagic += 1;
		ch->pcdata->max_memorize[ch->lifemagic] += 1;
		gn_add(ch, ch->lifemagic+59);
		if (ch->lifemagic > 5)
		{
		    if (dice(1, 200) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[1] += 1;		
		    else if (dice(1, 300) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[2] += 1;		
		    else if (dice(1, 400) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[3] += 1;		
		    else if (dice(1, 500) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[4] += 1;		
		}
	}
	else
	{
		send_to_char("You are already a master of life magic.\n\r",ch);
	}
	return;
    }
    if ( !str_prefix(argument,"flame")
	 && can_cast_flame(ch))
    {
	n = ch->fire;

	if (ch->secondaryrace == RACE_VAMPIRE
	    && n >= ch->power[DISC_VAMP_THAU] / 2)
  	{
	    stc("You cannot learn any more spells without a greater mastery of Thaumaturgy.\n\r", ch);
	    return;
	}

	if (ch->primary != 1 && ch->primary != 0)  n = n * 3 / 2;
	if (((n+2)*(n+1))/2 > ch->faith)
	{
	   sprintf(buf, "You need %d more faith points in the flames.\n\r",
		((n+2)*(n+1))/2 - ch->faith);
	   send_to_char(buf,ch);
	   return;
	}
	if ((ch->primary == 1 && ch->fire < nine) || (ch->fire < six))
	{
		send_to_char("The eternal flame burns bright within your heart.\n\r",ch);
		ch->faith -= ((n+2)*(n+1))/2;
		
		ch->fire += 1;
		gn_add(ch, ch->fire+9);
		ch->pcdata->max_memorize[ch->fire] += 1;

	 	/* Warlocks gaining generations */
		if (ch->secondaryrace == RACE_WARLOCK
		    && ch->generation < 12
		    && ch->primary == REALM_FLAME
		    && ch->fire >= 6)
		{
		    stc("You have gained a rank.  Congratulations.\n\r", ch);
		    restore_char(ch);
		    ch->generation++;
		}
		
		if (ch->fire > 5)
		{
		    if (dice(1, 200) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[1] += 1;		
		    else if (dice(1, 300) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[2] += 1;		
		    else if (dice(1, 400) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[3] += 1;		
		    else if (dice(1, 500) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[4] += 1;		
		}

		if (ch->fire == 6 && ch->primary == 0) ch->primary = 1;
	}
	else
	{
		send_to_char("You are already a master of flame.\n\r",ch);
	}
	return;
    }
    if ( !str_prefix(argument,"water")
	 && can_cast_water(ch))
    {
	n = ch->water;
	if (ch->secondaryrace == RACE_VAMPIRE
	    && n >= ch->power[DISC_VAMP_THAU] / 2)
  	{
	    stc("You cannot learn any more spells without a greater mastery of Thaumaturgy.\n\r", ch);
	    return;
	}

	if (ch->primary != 2 && ch->primary != 0)  n = n * 3 / 2;
	if (((n+1)*(n+2))/2 > ch->faith)
	{
	   sprintf(buf, "You need %d more faith points in the waters.\n\r",
		((n+2)*(n+1))/2 - ch->faith);
	   send_to_char(buf,ch);
	   return;
	}
	if ((ch->primary == 2 && ch->water < nine) || (ch->water < six))
	{
		send_to_char("The water of Lothlariel floods your very being.\n\r",ch);
		ch->faith -= ((n+2)*(n+1))/2;
		ch->water += 1;
		gn_add(ch, ch->water+19);
		ch->pcdata->max_memorize[ch->water] += 1;
	 	/* Warlocks gaining generations */
		if (ch->secondaryrace == RACE_WARLOCK
		    && ch->generation < 12
		    && ch->primary == REALM_WATER
		    && ch->water >= 6)
		{
		    stc("You have gained a rank.  Congratulations.\n\r", ch);
		    restore_char(ch);
		    ch->generation++;
		}
		if (ch->water > 5)
		{
		    if (dice(1, 200) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[1] += 1;		
		    else if (dice(1, 300) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[2] += 1;		
		    else if (dice(1, 400) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[3] += 1;		
		    else if (dice(1, 500) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[4] += 1;		
		}
		if (ch->water == 6 && ch->primary == 0) ch->primary = 2;
	}
	else
	{
		send_to_char("You have progressed as far as possible in the realm of water.\n\r",ch);
	}
	return;
    }
    if ( !str_prefix(argument, "air")
	 && can_cast_air(ch))
    {
	n = ch->air;
	if (ch->secondaryrace == RACE_VAMPIRE
	    && n >= ch->power[DISC_VAMP_THAU] / 2)
  	{
	    stc("You cannot learn any more spells without a greater mastery of Thaumaturgy.\n\r", ch);
	    return;
	}

	if (ch->primary != 3 && ch->primary != 0)  n = n * 3 / 2;
	if (((n+2)*(n+1))/2 > ch->faith)
	{
	   sprintf(buf, "You need %d more faith points to fly even higher.\n\r",
		((n+2)*(n+1))/2 - ch->faith);
	   send_to_char(buf,ch);
	   return;
	}
	if ((ch->primary == 3 && ch->air < nine) || (ch->air < six))
	{
		send_to_char("Your soul flies to greater heights than ever before.\n\r",ch);
		ch->faith -= ((n+2)*(n+1))/2;
		ch->air += 1;
		gn_add(ch, ch->air+29);
		ch->pcdata->max_memorize[ch->air] += 1;
	 	/* Warlocks gaining generations */
		if (ch->secondaryrace == RACE_WARLOCK
		    && ch->generation < 12
		    && ch->primary == REALM_AIR
		    && ch->air >= 6)
		{
		    stc("You have gained a rank.  Congratulations.\n\r", ch);
		    restore_char(ch);
		    ch->generation++;
		}
		if (ch->air > 5)
		{
		    if (dice(1, 200) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[1] += 1;		
		    else if (dice(1, 300) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[2] += 1;		
		    else if (dice(1, 400) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[3] += 1;		
		    else if (dice(1, 500) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[4] += 1;		
		}
		if (ch->air == 6 && ch->primary == 0) ch->primary = 3;
	}
	else
	{
		send_to_char("You are on top of the world already.\n\r",ch);
	}
	return;

    }
    if ( !str_prefix(argument,"earth")
	 && can_cast_earth(ch))
    {
	n = ch->earth;

	if (ch->secondaryrace == RACE_VAMPIRE
	    && n >= ch->power[DISC_VAMP_THAU] / 2)
  	{
	    stc("You cannot learn any more spells without a greater mastery of Thaumaturgy.\n\r", ch);
	    return;
	}

	if (ch->primary != 4 && ch->primary != 0)  n = n * 3 / 2;
	if (((n+2)*(n+1))/2 > ch->faith)
	{
	   sprintf(buf, "You need %d more faith points to learn more.\n\r",
		((n+2)*(n+1))/2 - ch->faith);
	   send_to_char(buf,ch);
	   return;
	}
	if ((ch->primary == 4 && ch->earth < nine) || (ch->earth < six))
	{
		send_to_char("You pledge your soul to the Mother Earth.\n\r",ch);
		ch->faith -= ((n+2)*(n+1))/2;
		ch->earth += 1;
		gn_add(ch, ch->earth+39);
		ch->pcdata->max_memorize[ch->earth] += 1;
	 	/* Warlocks gaining generations */
		if (ch->secondaryrace == RACE_WARLOCK
		    && ch->generation < 12
		    && ch->primary == REALM_EARTH
		    && ch->earth >= 6)
		{
		    stc("You have gained a rank.  Congratulations.\n\r", ch);
		    restore_char(ch);
		    ch->generation++;
		}
		if (ch->earth > 5)
		{
		    if (dice(1, 200) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[1] += 1;		
		    else if (dice(1, 300) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[2] += 1;		
		    else if (dice(1, 400) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[3] += 1;		
		    else if (dice(1, 500) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[4] += 1;		
		}
		if (ch->earth == 6 && ch->primary == 0) ch->primary = 4;

	}
	else
	{
		send_to_char("You are at the summit of your learning and cannot progress any further.\n\r",ch);
	}
	return;
    }
    if ( !str_prefix(argument,"entropy")
	 && can_cast_entropy(ch))
    {
	n = ch->entropy;
	if (ch->primary != 5 && ch->primary != 0)  n = n * 3 / 2;
	if (((n+2)*(n+1))/2 > ch->faith)
	{
	   sprintf(buf, "Your faith is %d points too weak mortal.\n\r",
		((n+2)*(n+1))/2 - ch->faith);
	   send_to_char(buf,ch);
	   return;
	}
	
	if ((ch->primary == 5 && ch->entropy < nine) || (ch->entropy < six))
	{
		send_to_char("The grasp of evil tightens its grip on your soul.\n\r",ch);
		ch->alignment -= 50;
		ch->faith -= ((n+2)*(n+1))/2;
		ch->entropy += 1;
		gn_add(ch, ch->entropy+49);
		ch->pcdata->max_memorize[ch->entropy] += 1;
	 	/* Warlocks gaining generations */
		if (ch->secondaryrace == RACE_WARLOCK
		    && ch->generation < 12
		    && ch->primary == REALM_ENTROPY
		    && ch->entropy >= 6)
		{
		    stc("You have gained a rank.  Congratulations.\n\r", ch);
		    restore_char(ch);
		    ch->generation++;
		}
		if (ch->entropy > 5)
		{
		    if (dice(1, 200) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[1] += 1;		
		    else if (dice(1, 300) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[2] += 1;		
		    else if (dice(1, 400) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[3] += 1;		
		    else if (dice(1, 500) < get_curr_stat(ch, STAT_INT) )
		        ch->pcdata->max_memorize[4] += 1;		
		}
		if (ch->entropy == 6 && ch->primary == 0) ch->primary = 5;
	}
	else
	{
		send_to_char("You must hang on to what precious little soul you have left.\n\r",ch);
	}
	return;
    }
    send_to_char("Syntax:  learn [flame water air earth entropy",ch);
    if (can_cast_life(ch)) stc(" life", ch);
    stc("]\n\r", ch);
}


void do_devour(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim; 
    char buf[MAX_STRING_LENGTH];
	
    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Whose soul do you wish to devour?\n\r", ch );
	return;
    }
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    
    if ( victim->secondaryrace != 0 || IS_NPC(victim))
    {
	send_to_char("You cannot wrench their soul from their deity.\n\r",ch);
	send_to_char("You feel a tugging at your soul.\n\r",victim);
	return;
    }
     
    if ( !can_be_made(victim) || victim->level < secondaryrace[2].min_level )
    {
	send_to_char("You do not need such weaklings in your clan.\n\r",ch);
	return;
    }

    if ( !IS_NEWFLAG(victim, AFF_ALLOW_DEMON) )
    {
	send_to_char("They are not a willing subject.\n\r",ch);
	return;
    }

    if (ch->clan == CLAN_DAEM_OUTCAST)
    {
	stc("You are not able to create new demons until you are in a clan.\n\r", ch);
	return;
    }

    victim->secondaryrace = 2;
    victim->generation = 1;

    set_learnable_disciplines(victim, RACE_DAEMON);

    ch->pcdata->souls++;
    if (ch->pcdata->souls > souls_needed(ch) )
	ch->pcdata->souls = souls_needed(ch);
    SET_BIT(victim->act, ACT_UNDEAD);
    set_form(victim,  0);
    victim->perm_stat[STAT_INT] -= 4;
    victim->alignment = -666;
    victim->hit = 1;
    victim->move = 1;
    victim->clan = CLAN_DAEM_OUTCAST;
    save_char_obj(victim);
    save_char_obj(ch);
    sprintf(buf, "--> %s's soul has been devoured by %s.", victim->name, ch->name);
    victim->pcdata->damcap = 200;
    informational(buf, 2, 0);

    /* Now demons actually have forms.  Kooky. */
    set_form(victim, FRM_LEMURE);

    if (victim->fire < 1)
    {
	victim->fire = 1;
	gn_add(victim, 10);
	victim->pcdata->max_memorize[1]++;
    }

    if (victim->entropy < 1)
    {
	victim->entropy = 1;
	gn_add(victim, 50);
	victim->pcdata->max_memorize[1]++;
    }

    look_name(victim);

    SET_BIT(victim->imm_flags, IMM_IRON);

    ch->hit = ch->max_hit;

    WAIT_STATE(ch, 10);
    WAIT_STATE(victim, 30);

    send_to_char("Plunging your hand deep within your victims chest, you rip out their\n\r",ch);
    send_to_char("soul, leaving a lifeless husk motionless at your feet.  You have been\n\r",ch);
    send_to_char("rewarded by your diety by enrolling another follower.\n\r",ch);

    act("$N's vast demonic form towers over you.  With a mighty roar, $E plunges $S\n\rscaly claw into your chest.  Blood sprays everywhere, and you hear the eldritch\n\rmocking cry of conquest as your mortal conciousness fades forever.", 
		victim, NULL, ch, TO_CHAR);
    send_to_char("The demonic transformation *****TOTALLY DESTROYS***** you.\n\r",victim);
    send_to_char("That really did HURT!\n\r",victim);
    send_to_char("You have been KILLED!\n\r",victim);

    act("With a deafening roar, the great demon $N plunges $S claw into $n's chest\n\rand rips out $s soul.  $n drops to the ground, lifeless.", victim, NULL, ch, TO_NOTVICT);
}       

void do_infect(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    
    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' ) 
    {
	send_to_char( "Who do you wish to infect with lycanthropy?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }   

    if ( IS_NPC(victim))
    {
	send_to_char( "You may only infect players with lycanthopy.\n\r",ch );
	return;
    }

 
    if ( victim->secondaryrace != 0)
    {   
	send_to_char("Only mortals can undergo the transformation.\n\r",ch);
	return;
    }

    if ( victim->level < secondaryrace[5].min_level || !can_be_made(victim))
    {
	send_to_char("Not yet! They wouldnt stand a chance.\n\r",ch);
	return;
    }
    
    if ( !IS_NEWFLAG(victim, AFF_ALLOW_WERE) )
    {
	send_to_char("They are not a willing subject.\n\r",ch);
	return;
    }

    victim->value[0] = 0;
    victim->secondaryrace = 4;
    victim->generation = 1;
    set_learnable_disciplines(victim, RACE_LUPINE);
    SET_BIT(victim->affected_by, AFF_DARK_VISION);
    victim->dragtype = ch->dragtype;
    victim->pcdata->damcap = 200;
    victim->cur_form = 0;
    victim->hit = victim->max_hit;
    victim->move = victim->max_move;
    victim->clan = ch->clan;
    set_clan_disciplines(victim);
    save_char_obj(victim);
    save_char_obj(ch);
    look_name(victim);

    sprintf(buf, "--> %s has been infected with lycanthropy by %s.", victim->name, ch->name);
    informational(buf, 4, 0);

    WAIT_STATE(victim, 40);
    WAIT_STATE(ch, 10);

    act("You sink your claws deep into $N's chest, spreading the evil disease.",ch,NULL,victim,TO_CHAR);
    send_to_char("As the light of the moon bathes your victim, coarse hair sprouts
from their body.  You have created another creature of the night.\n\r",ch);
    act("$N plunges his claws deep within your chest.  You feel a searing pain, 
then a horrible ripping sensation as your body twists and writhes under the
transformation.  Coarse hair sprouts from your body and your teeth extend into
fangs.\n\r", victim, NULL, ch , TO_CHAR);
}

void do_sprinkle(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    
    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' ) 
    {
	send_to_char( "On whom do you wish to sprinkle the magic dust?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }   

    if (ch->tertiaryrace == TERTIARY_DROW)
    {
	stc("Drow cannot use this ability.\n\r", ch);
	return;
    }
 
    if ( victim->secondaryrace != 0)
    {   
	send_to_char("You cannot transform them into Eladrin.\n\r",ch);
	return;
    }

    if ( victim->race != 2)
    {
	send_to_char("Only elves are privileged enough to join.\n\r",ch);
	return;
    }
	
    if ( victim->level < secondaryrace[5].min_level || !can_be_made(victim))
    {
	send_to_char("Wait till they are a bit more experienced.\n\r",ch);
	return;
    }
    
    if ( !IS_NEWFLAG(victim, AFF_ALLOW_ELAD) )
    {
	send_to_char("They are not a willing subject.\n\r",ch);
	return;
    }

    victim->secondaryrace = 5;
    victim->generation = 1;

    set_learnable_disciplines(victim, RACE_ELADRIN);

    victim->cur_form = 0;
    victim->hit = victim->max_hit;
    victim->move = victim->max_move;
    victim->clan = ch->clan;
    set_clan_disciplines(victim);
    victim->pcdata->damcap = 200;
    recalc_wear(victim);

    sprintf(buf, "--> %s has been sprinkled with faerie dust by %s.", victim->name, ch->name);
    informational(buf, 5, 0);

    if (victim->earth < 1)
    {
	victim->earth = 1;
	gn_add(victim, 40);
    }

    victim->pcdata->max_memorize[1]++;
    victim->pcdata->max_memorize[1]++;

    if (victim->water < 1)
    {
	victim->water = 1;
	gn_add(victim, 20);
    }

    WAIT_STATE(victim, 40);
    WAIT_STATE(ch, 40);
    look_name(victim);


    act("You reach into a bag of sparkling elven dust, and sprinkle a handful over $N.\n\r",ch,NULL,victim,TO_CHAR);
    send_to_char("They sprout huge gossamer wings as a result of the transformation.\n\r",ch);
    send_to_char("Another elf has joined the ranks of the Eladrin, Congratulations.\n\r",ch);

    act("$N's reaches into a bag of magic dust.  Chanting mystical syllables
you suddenly feel a strange transformation taking place.  A pair of huge 
gossamer wings sprout from your back.  You are now an Eladrin, a Coure to 
be precise, one of the faerie-lords of lore.", victim, NULL, ch , TO_CHAR);
}

void do_teach(CHAR_DATA *ch, char *argument)
    
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    
    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' ) 
    {
	send_to_char( "Who do you wish to teach the mystic arts?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }   
 
    if ( IS_NPC(victim))
    {   
	send_to_char("Only players may become Warlocks.\n\r",ch);
	return;
    }

    if ( !IS_NEWFLAG(victim, AFF_ALLOW_WARLOCK) )
    {
	send_to_char("They are not a willing subject.\n\r",ch);
	return;
    }

    if (ch->clan == CLAN_WARL_CRIMSON
	&& victim->primary != REALM_FLAME)
    {
	stc("Their primary realm is not fire.\n\r", ch);
	return;
    }
    
    if (victim->secondaryrace != 0)
    {
	send_to_char("They are already a secondary race.\n\r", ch);
	return;
    }

    victim->secondaryrace = 6;
    victim->generation = 1;

    set_learnable_disciplines(victim, RACE_WARLOCK);

    save_char_obj(victim);
    save_char_obj(ch);
    victim->cur_form = 0;
    victim->hit = victim->max_hit;
    victim->move = victim->max_move;
    victim->clan = ch->clan;
    set_clan_disciplines(victim);
    look_name(victim);
    victim->pcdata->damcap = 100;

    sprintf(buf, "--> %s's has been taught the ways of the warlocks by %s.", victim->name, ch->name);
    informational(buf, 6, 0);

    if (victim->earth < 2)
    {
	if (victim->earth < 1)
		gn_add(victim, 40);
	victim->earth = 2;
	gn_add(victim, 41);
	victim->pcdata->max_memorize[1]++;
	victim->pcdata->max_memorize[2]++;
    }

    if (victim->fire < 2)
    {
	if (victim->fire < 1)
		gn_add(victim, 10);
	victim->fire = 2;
	gn_add(victim, 11);
	victim->pcdata->max_memorize[1]++;
	victim->pcdata->max_memorize[2]++;
    }

    if (victim->water < 2)
    {
	if (victim->water < 1)
		gn_add(victim, 20);
	victim->water = 2;
	gn_add(victim, 21);
	victim->pcdata->max_memorize[1]++;
	victim->pcdata->max_memorize[2]++;
    }

    if (victim->air < 2)
    {
	if (victim->air < 1)
		gn_add(victim, 30);
	victim->air = 2;
	gn_add(victim, 31);
	victim->pcdata->max_memorize[1]++;
	victim->pcdata->max_memorize[2]++;
    }

    WAIT_STATE(victim, 40);
    WAIT_STATE(ch, 40);

    act("You enroll $N as an apprentice in the mystic arts.\n\r",ch,NULL,victim,TO_CHAR);
    send_to_char("They are a good student, and soon they about to begin their training.\n\r",ch);

    act("$n starts to teach you about the ways of the Warlocks, and you 
take your first step into becoming an Archmage.", ch, NULL, victim, 
TO_VICT); }

void do_gate( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
	
    argument = one_argument( argument, arg1 );
    
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Gate whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	if ( ch->generation < 12 )
	{
	    send_to_char("You do not yet have the power to gate all demons.\n\r",ch);
	}
	else
	{
		for ( d = descriptor_list; d != NULL; d = d->next )
		{
		    if ( d->connected == CON_PLAYING
		    &&   d->character != ch
		    &&   d->character->in_room != NULL
		    &&   can_see( ch, d->character ) 
		    &&   d->character->generation < ch->generation
		    &&   d->character->secondaryrace == 2
		    &&   d->character->clan == ch->clan)
		    {
			char buf[MAX_STRING_LENGTH];
			sprintf( buf, "%s", d->character->name );
			do_gate( ch, buf );
		    }
		}
	}
	return;
    }
 
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->secondaryrace !=2 )
    {
	send_to_char( "You may only gate in demons.\n\r", ch );
	return;
    }

    if ( victim->clan != ch->clan )
    {
	stc("You may only gate demons of the same clan as you.\n\r", ch);
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->generation >= ch->generation )
    {
	send_to_char("You may only gate one of lesser power than yourself.\n\r",ch);
	return;
    }    

    if ( dice(1 , 13) > ch->generation )
    {
	send_to_char("Your powers seem to have failed you.\n\r",ch);
	send_to_char("You hear demonic chanting in the distance.\n\r",victim);
	return;
    }
    
    if ( victim->fighting != NULL ) stop_fighting( victim, TRUE );
    act( "A scaly hand reaches down and drags $n away.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room, ch->altitude );
    act( "You have successfully gated $N.", ch, NULL, victim, TO_CHAR );
    act( "$n has successfully gated $N.", ch, NULL, victim, TO_NOTVICT );
    if ( ch != victim )
	act( "$n has pulled you through the demonic void.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}

void do_call_faerie( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
	
    argument = one_argument( argument, arg1 );
    
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Call whom?\n\r", ch );
	return;
    }

    if ( ch->generation < 7)
    {
	send_to_char( "You must be at least a Shiere to call other eladrin.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	if ( ch->generation < 12 )
	{
	    send_to_char("You do not yet have the power to call faeries.\n\r",ch);
	}
	else
	{
		for ( d = descriptor_list; d != NULL; d = d->next )
		{
		    if ( d->connected == CON_PLAYING
		    &&   d->character != ch
		    &&   d->character->in_room != NULL
		    &&   can_see( ch, d->character ) 
		    &&   d->character->generation < ch->generation
		    &&   d->character->secondaryrace == RACE_ELADRIN
		    &&   d->character->tertiaryrace != TERTIARY_DROW)
		    {
			char buf[MAX_STRING_LENGTH];
			sprintf( buf, "%s", d->character->name );
			do_gate( ch, buf );
		    }
		}
	}
	return;
    }
 
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->secondaryrace != RACE_ELADRIN )
    {
	send_to_char( "You may only call faeries.\n\r", ch );
	return;
    }

    if (victim->tertiaryrace == TERTIARY_DROW)
    {
	stc("You cannot call drow using this power.\n\r", ch);
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->generation >= ch->generation )
    {
	send_to_char("You may only call one of lesser power than yourself.\n\r",ch);
	return;
    }    

    if ( dice(6 , 10) > ch->level )
    {
	send_to_char("Your powers seem to have failed you.\n\r",ch);
	send_to_char("You hear elven chanting in the distance.\n\r",victim);
	return;
    }
    
    if ( victim->fighting != NULL ) stop_fighting( victim, TRUE );
    act( "$n vanishes in a cloud of sparkling dust.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room, ch->altitude );
    act( "You have successfully called $N.", ch, NULL, NULL, TO_CHAR );
    act( "$n has successfully called $N.", ch, NULL, victim, TO_NOTVICT );
    if ( ch != victim )
	act( "$n has called you to $m.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}


void do_faeriegate( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
	
    argument = one_argument( argument, arg1 );
    
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Who do you wish to send a faeriegate to?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->secondaryrace != RACE_ELADRIN )
    {
	send_to_char( "You may only faeriegate to other Eladrin.\n\r", ch );
	return;
    }

    if (victim->tertiaryrace == TERTIARY_DROW)
    {
	stc("You cannot faeriegate to drow.\n\r", ch);
	return;
    }

    if (ch == victim)
    {   
	send_to_char ( "You can't faeriegate to yourself!\n\r", ch);
	return;
    }

    if (ch->move < 200)
    {   
	send_to_char ( "You are too tired to do that.\n\r", ch);
	return;
    }

    if ( dice(10 , 10) > ch->level )
    {
	send_to_char("You failed to create a faeriegate.\n\r",ch);
	send_to_char("A little faeriegate pops into existence and then disappears.\n\r",victim);
	return;
    }
    
    if ( ch->fighting != NULL ) stop_fighting( victim, TRUE );
    act( "$n waves $s hands, and a faeriegate whisks $m away.", ch, NULL, NULL, TO_ROOM );
    WAIT_STATE(ch, 8);
    char_from_room( ch );
    char_to_room( ch, victim->in_room, victim->altitude );
    act( "$n appears through a faeriegate.", ch, NULL, victim, TO_ROOM );
    do_look( ch, "auto" );
    ch->move /=2;
}


/***** Global stance and upkeep commands *****/

bool change_upkeep(CHAR_DATA *ch, int change)
{
    ch->upkeep += change;
    if ( ch->upkeep < 0 )
	ch->upkeep = 0;
    if ( ch->upkeep > ch->generation + 19 )
    {
	ch->upkeep -= change;
	return FALSE;
    }
    return TRUE;
}

bool able_stance(CHAR_DATA *ch, int stance)
{
    bool able = FALSE;

    if (IS_NPC(ch))
	return TRUE;

    if (stance>11 || stance<0)
	return FALSE;

    if (stance < 6)
    {
	able = TRUE;
    }
    else
    {
	able = FALSE;
	switch (stance)
	{
	    case 6  :   /* crab and bat */
		if (ch->stance_skill[4] >= 80 && ch->stance_skill[2] >= 80)
		able = TRUE;
			break;
	    case 7  :   /* bat and rabbit */
		if (ch->stance_skill[2] >= 80 && ch->stance_skill[1] >= 80)
		able = TRUE;
			break;
	    case 8  :   /* imp and crab */
		if (ch->stance_skill[4] >= 80 && ch->stance_skill[3] >= 80)
		able = TRUE;
			break;
	    case 9  :   /* scorpion and imp */
		if (ch->stance_skill[5] >= 80 && ch->stance_skill[3] >= 80)
		able = TRUE;
			break;
	    case 10 :   /* scorpion and rabbit */
		if (ch->stance_skill[5] >= 80 && ch->stance_skill[1] >= 80)
		able = TRUE;
			break;
	}
    }

    if (ch->level == 120) 
	able=TRUE;

    return able;
}

bool set_stance(CHAR_DATA *ch, int stance)
{
    bool able;

    able = able_stance(ch, stance);

    if (IS_FLAG5(ch, AFF5_WEB_WEAK) || IS_FLAG5(ch, AFF5_WEB_STRONG) )
    {
	stc("You cannot enter a stance with all these webs on.\n\r", ch);
	return FALSE;
    }

    if (!able)
    {
	send_to_char("You cannot attempt such a difficult stance without proper training.\n\r",ch);
	return FALSE;
    }

    if (ch->stance!=0)
    {
	send_to_char("You relax from your ",ch);
	send_to_char(stances[ch->stance].name,ch);
	send_to_char(" fighting stance.\n\r",ch);
	act("$n relaxes from $s $t fighting stance",
		ch, stances[ch->stance].name, NULL, TO_ROOM);
    }

    ch->stance = stance;

    if (ch->stance!=0)
    {
	act("You $t into the $T fighting stance.",
		ch,
		stances[ch->stance].switchto,
		stances[ch->stance].name,
		TO_CHAR);
	act("$n $t into the $T fighting stance.",
		ch,
		stances[ch->stance].plural,
		stances[ch->stance].name,
		TO_ROOM);
    }
    return TRUE;
}

void improve_stance(CHAR_DATA *ch, int stance)
{
    char buf[MAX_STRING_LENGTH];
    if (IS_NPC(ch))
	return;

    if (stance>11 || stance<1)
	return;

    if (stance==11 && ch->level<120)
	return;

    if (++ch->stance_skill[stance] > 100)
    {
	ch->stance_skill[stance] = 100;
    }
    else
    {
	sprintf( buf ,"You have become better at the %s stance!\n\r", stances[stance].name);
	send_to_char( buf, ch);
    }
}


void do_stance(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int st, max_st;

    max_st = 11;
    if (ch->level == 120) max_st = 12; 

    if (!str_prefix(argument,"skill") || argument[0]=='\0')
    {
        if (argument[0]=='\0')
	{
	    if (ch->stance != 0)
	    {
		set_stance(ch,0);
		return;
	    }
	}
	divide3_to_char(ch);
	cent_to_char("Skill with stances:",ch);
	divide2_to_char(ch);
	send_to_char("\n\r",ch);
	for (st = 1; st<max_st; st++)
	{
	    if (able_stance(ch, st))
	    {
	    	sprintf(buf,"      %-12s:  %s\n\r",
		    stances[st].name,
		    score_word[UMIN(20,ch->stance_skill[st]/5)]);
		send_to_char(buf,ch);
	    }
	}
	send_to_char("\n\r",ch);
	divide3_to_char(ch);
	return;
    }

    if (ch->riding != NULL)
    {
	stc("You cannot go into a stance while riding.\n\r", ch);
	return;
    }

    if (ch->riden_by != NULL)
    {
	stc("You cannot go into a stance while being ridden.\n\r", ch);
	return;
    }

    if (!str_prefix(argument,"none")
	|| !str_prefix(argument,"relax") )
    {
	set_stance(ch,0);
	return;
    }

    for (st = 1; st < max_st; st++)
    {
	if (!str_prefix(argument,stances[st].name))
	{
	    set_stance(ch,st);
	    return;
	}
    }

    send_to_char("You don't know any stances by that name.\n\r",ch);
}


/**** ALLOW COMMANDS ****/

void do_vampallow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) return;

    if (IS_NEWFLAG(ch,AFF_ALLOW_VAMP))
    {
	send_to_char("You refuse your body to the kindred.\n\r",ch);
	REMOVE_BIT(ch->flag2,AFF_ALLOW_VAMP);
	return;
    }
    else
    {
	send_to_char("You give your body to the kindred.\n\r",ch);
	SET_BIT(ch->flag2,AFF_ALLOW_VAMP);
	return;
    }
}

void do_wereallow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) return;

    if (IS_NEWFLAG(ch,AFF_ALLOW_WERE))
    {
	send_to_char("You are now immune to lycanthropy.\n\r",ch);
	REMOVE_BIT(ch->flag2,AFF_ALLOW_WERE);
	return;
    }
    else
    {
	send_to_char("You gift your body to the lupines.\n\r",ch);
	SET_BIT(ch->flag2,AFF_ALLOW_WERE);
	return;
    }
}

void do_eladallow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) return;

    if (ch->race != RACE_ELF)
    {
	send_to_char("Only elves may join the eladrin.\n\r", ch);
	return;
    }

    if (IS_NEWFLAG(ch,AFF_ALLOW_ELAD))
    {
	send_to_char("You reject the ways of the faerie folk.\n\r",ch);
	REMOVE_BIT(ch->flag2,AFF_ALLOW_ELAD);
	return;
    }
    else
    {
	send_to_char("You pledge your life to nature.\n\r",ch);
	SET_BIT(ch->flag2,AFF_ALLOW_ELAD);
	return;
    }
}

void do_demonallow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) return;

    if (IS_NEWFLAG(ch,AFF_ALLOW_DEMON))
    {
	send_to_char("You snatch your soul back from eternal damnation.\n\r",ch);
	REMOVE_BIT(ch->flag2,AFF_ALLOW_DEMON);
	return;
    }
    else
    {
	send_to_char("You sell your soul to darkness.\n\r",ch);
	SET_BIT(ch->flag2,AFF_ALLOW_DEMON);
	return;
    }
}

void do_warlockallow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) return;

    if (IS_NEWFLAG(ch,AFF_ALLOW_WARLOCK))
    {
	send_to_char("You reject the paths of magic.\n\r",ch);
	REMOVE_BIT(ch->flag2,AFF_ALLOW_WARLOCK);
	return;
    }
    else
    {
	send_to_char("You prepare yourself for a life of studying.\n\r",ch);
	SET_BIT(ch->flag2,AFF_ALLOW_WARLOCK);
	return;
    }
}

void do_aura(CHAR_DATA *ch, char *argument)
{
    if (argument[0] == '\0')
    {
	if (ch->aura != 0)
	{
		WAIT_STATE(ch, 6);
		send_to_char("Your shimmering aura vanishes.\n\r",ch);
		act("$n's shimmering aura vanishes.",ch,NULL,NULL,TO_ROOM);
		deconstruct_aura(ch);
	}
	else
	{
		WAIT_STATE(ch, 6);
		act("$n is surrounded by a shimmering aura.",ch,NULL,NULL, TO_ROOM);
		send_to_char("You surround yourself with a shimmering aura.\n\r",ch);
		construct_aura(ch);
	}
	return;
    }

    if (!str_prefix(argument, "on"))
    {
	if (ch->aura != 0)
	{
	    send_to_char("You are already surrounded by an aura.\n\r",ch);
	    return;
	}
	construct_aura(ch);
	WAIT_STATE(ch, 6);
	act("$n is surrounded by a shimmering aura.",ch,NULL,NULL, TO_ROOM);
	send_to_char("You surround yourself with a shimmering aura.\n\r",ch);
	return;
    }

    if (!str_prefix(argument, "off"))
    {
	if (ch->aura == 0)
	{
	    send_to_char("You aren't currently surrounded with an aura.\n\r",ch);
	    return;
	}
	deconstruct_aura(ch);
	send_to_char("Your shimmering aura vanishes.\n\r",ch);
	WAIT_STATE(ch, 6);
	act("$n's shimmering aura vanishes.",ch,NULL,NULL,TO_ROOM);
	return;
    }

    send_to_char("Syntax:  aura [on off]\n\r",ch);
}

/* Earth Meld */

void do_meld(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_FLAG4(ch, AFF_TREE_MELD))
    {
	send_to_char("Not while melded with a tree.\n\r", ch);
	return;
    }

    if (ch->power[DISC_VAMP_PROT]<3)
    {
	send_to_char("You have not yet gained mastery of this ability.\n\r",ch);
	return;
    }

    if (ch->fighting != NULL)
    {
	send_to_char("You cannot meld while fighting.\n\r", ch);
	return;
    }

    if (ch->blood < 4)
    {
	send_to_char("You do not have enough blood.\n\r", ch);
	return;
    }

    ch->blood -= 4;

    if (IS_NEWFLAG(ch, AFF_EARTH_MELD))
    {
	REMOVE_BIT(ch->flag2, AFF_EARTH_MELD);
	ch->position = POS_STANDING;
	sprintf(buf, "%d", ch->value[1]);
	char_from_room( ch );
	char_to_room( ch, find_location( ch, buf ),0 );
	do_look(ch, "auto");
	send_to_char("You rise out of the earth slowly.\n\r",ch);
	act("$n rises out of the earth slowly.",ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (ch->cur_form == FRM_MIST && !TIME_UP(ch, TIMER_WRENCHED) )
    {
        send_to_char("You have been wrenched from the negative plane and may not do that.\n\r", ch);
        return;
    }

    send_to_char("You slowly sink into the earth.\n\r",ch);
    act("$n sinks into the earth slowly.",ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->flag2, AFF_EARTH_MELD);
    ch->position = POS_SLEEPING;
    ch->value[1] = ch->in_room->vnum;
    char_from_room( ch );
    char_to_room( ch, find_location( ch, "1" ),0);
}

void do_tree_meld(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (ch->fighting != NULL)
    {
	send_to_char("You cannot meld while fighting.\n\r", ch);
	return;
    }

    if (IS_SET(ch->flag2, AFF_EARTH_MELD))
    {
	send_to_char("Not while earth melded.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_NO_ARBOREA) )
    {
	send_to_char("The powers of the forest do not trust you.\n\r",ch);
	return;
    }

    if (IS_FLAG4(ch, AFF_TREE_MELD))
    {
	REMOVE_BIT(ch->flag4, AFF_TREE_MELD);
	ch->position = POS_STANDING;
	sprintf(buf, "%d", ch->value[1]);
	char_from_room( ch );
	char_to_room( ch, find_location( ch, buf ),0 );
	WAIT_STATE(ch, 12);
	do_look(ch, "auto");
	send_to_char("You slowly emerge from the tree.\n\r",ch);
	act("$n emerges from one of the trees.",ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (ch->in_room->sector_type != SECT_FOREST)
    {
	send_to_char("But there are no trees about to meld with.\n\r",ch);
	return;
    }

    send_to_char("You meld with one of the trees around you.\n\r",ch);
    act("$n melds with the trees and is gone.",ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->flag4, AFF_TREE_MELD);
    ch->position = POS_SLEEPING;
    ch->value[1] = ch->in_room->vnum;
    char_from_room( ch );
    WAIT_STATE(ch, 12);
    char_to_room( ch, find_location( ch, "1" ),0);
}

/* Fun and funky display routines! :) */

void rand_to_char(CHAR_DATA *ch, char *str1, char *str2, char *str3, char *str4, char *str5) 
{
    switch(dice(1,5))
    {
	case 1 : send_to_char(str1, ch);
		 break;
	case 2 : send_to_char(str2, ch);
		 break;
	case 3 : send_to_char(str3, ch);
		 break;
	case 4 : send_to_char(str4, ch);
		 break;
	case 5 : send_to_char(str5, ch);
		 break;
	default:bug("BUG: rand_to_char: number out of range.",0);
		 break;
    }
}

void vamp_say( CHAR_DATA *ch, char *argument )
{
/*    sh_int door;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    int chr;

    for (door = 0; door < 6; door++)
    {
	if (ch->in_room->exit[door]->u1.to_room != NULL)
	{
    bug("boo",0);

	    for (victim = ch->in_room->exit[door]->u1.to_room->people; victim!=NULL; victim = victim->next_in_room)
	    {
    bug("boo2",2);
		if (ch->altitude != victim->altitude) continue;
		if (victim->secondaryrace==3 && victim->power[DISC_VAMP_AUSP]>0 && victim!=ch)
		{
		send_to_char("To the ",victim);
		switch( door )
		{
		case 0 : send_to_char("south",victim);
			 break;
		case 1 : send_to_char("west",victim);
			 break;
		case 2 : send_to_char("north",victim);
			 break;
		case 3 : send_to_char("east",victim);
			 break;
		case 4 : send_to_char("down",victim);
			 break;
		case 5 : send_to_char("up",victim);
			 break;
		}
		send_to_char(" you think you hear someone say '",victim);
		for (chr = 0; argument[chr] != '\0'; chr++)
		{
		    if (dice(1,100) < (victim->power[DISC_VAMP_AUSP]*10+20))
			buf[chr] = argument[chr];
		    else
			buf[chr] = '.';
		}
		buf[chr] = '\0';
		send_to_char(buf, victim);                      
		send_to_char("'\n\r",victim);
		}
	    }
	}
    }*/
}

bool is_next_room( CHAR_DATA *ch, CHAR_DATA *wch )
{
    sh_int door;
    CHAR_DATA *victim=NULL;

    for (door = 0; door < 6; door++)
    {
	if (ch->in_room->exit[door]->u1.to_room != NULL)
	{
	    for (victim = ch->in_room->exit[door]->u1.to_room->people; victim!=NULL; victim = victim->next_in_room)
	    {
		if (victim == wch && wch->altitude == ch->altitude)
		    return TRUE;
	    }
	}
    }
    return FALSE;
}
