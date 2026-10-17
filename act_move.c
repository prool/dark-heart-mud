/*
 * HELL, formerly known as BTL/Darkheart, by Adam Powell and Chris Mellish,
 *  1996 to 1998.  If you reproduce this code in any way please give credit
 *  to the writers.  Thanks.
 */


#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hell.h"

/* command procedures needed */
DECLARE_DO_FUN(do_goto          );
DECLARE_DO_FUN(do_say           );
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_stand         );
DECLARE_DO_FUN(do_berserk       );
DECLARE_DO_FUN(do_land          );

const  char * discipline [MAX_DISCIPLINES] =
{
/* Dragon */
        "",
	"breath",               "prowess",
	"might",                "awe",
	"resistance",           "polymorph",
	"metabolism",           "conjunction",
	"",			"",

/* Daemon */

	"hellfire",             "attack",
	"temptation",           "morphosis",
	"corruption",           "geluge",
	"discord",              "nether",
	"imunae",		"",

/* Vampire */
	
	"celerity",             "potence",
	"obfuscate",            "auspex",
	"fortitude",            "thaumaturgy",
	"protean",              "dominate",
	"animalism",		"presence",

/* Lupine */

	"lion",                 "boar",
	"wolf",                 "bear",
	"hawk",                 "cobra",
	"luna",                 "fox",
	"",			"",

/* Eladrin */

	"aura",                 "morphia",    /* aura, changing form */
	"?????",                "melodia",    /* ?????, songs */
	"somatia",              "ansorcia",   /* spell speed, MR */
	"artificia",            "physica",    /* create items, fighting */
	"arborea",		"",           /* nature?!? */

/* Warlock */

	"nexus",  	        "lucubration",
	"scribing",             "brewing",
	"enchanting",           "control",
	"the art",              "willpower", /* Binding is gone for now! */
	"runecraft",		"fakir",

/* Hero */

	"sigil",                "relic",
	"exorcism",             "crusade",
	"vivis",                "true faith",
	"element",              "battle",
	"",			"",

/* Any blatent other ones that we wanna stick in....
	starting at 71  - only thing that needs updating is
	merc.h and set_learnable disciplines */

  	"anime", 		"satyralis",
  	"", 			"",
  	"", 			"",
  	"", 			"",
  	"", 			"",
  	"", 			"",
  	"", 			"",
  	"", 			"",
  	"", 			"",
  	"",			""
};



const sh_int  rev_dir [] =
{
    2, 3, 0, 1, 5, 4
};

const   sh_int  movement_loss   [SECT_MAX]      =
{
    1, 2, 2, 3, 4, 6, 4, 10, 6, 10, 6, 2
};



/*
 * Local functions.
 */
int     find_door       args( ( CHAR_DATA *ch, char *arg ) );
bool    has_key         args( ( CHAR_DATA *ch, int key ) );


/*void do_run(CHAR_DATA *ch, char *argument)
{
    char buf[50];
    int i, bufplac = 0;

    buf[0] = '\0';

    if (argument[0] == '\0')
    {
	send_to_char("Syntax : run <neswud>\n\r", ch);
	return;
    }

    for (i=0 ; i<strlen(argument) || i < 20 ; i++)
    {
	if (argument[i] == 'n'
	    || argument[i] == 's'
	    || argument[i] == 'e'
	    || argument[i] == 'w'
	    || argument[i] == 'u'
	    || argument[i] == 'd')
	{
	    buf[bufplac] = argument[i];
	    buf[bufplac+1] = ';';
	    bufplac += 2;
	}
    }

    sprintf(ch->desc->incomm, buf);
}*/

bool is_flying(CHAR_DATA *ch)
{
    if (ch->riding != NULL
	&& is_flying(ch->riding) )
	return TRUE;

    if (IS_FLAG4(ch, AFF_LANDED)) return FALSE;

    if (IS_SET(ch->flag4, AFF_WINGS)) return TRUE;

    if (IS_SET(ch->affected_by, AFF_FLYING)) return TRUE;

    if (ch->cur_form != 0 && form_data[ch->cur_form].can_fly == TRUE)
	return TRUE;
    if (ch->cur_form != 0 && form_data[ch->cur_form].can_fly == FALSE)
	return FALSE;

    if (IS_FLAG4(ch, AFF_CAN_FLY) )
	return TRUE;

    if (is_wearing_artifact(ch, RELIC_WIND_BOOTS) != NULL )
	return TRUE;
 
    return FALSE;
}

bool is_default_flying (CHAR_DATA *ch)
{
    if (ch->riding != NULL
	&& is_flying(ch->riding) )
	return TRUE;

    if (IS_FLAG4(ch, AFF_LANDED)) return FALSE;

    if (IS_SET(ch->affected_by, AFF_FLYING)) return TRUE;

    if (ch->cur_form != 0 && form_data[ch->cur_form].can_fly)
	return TRUE;
    if (ch->cur_form != 0 && !form_data[ch->cur_form].can_fly)
	return FALSE;

    if (IS_FLAG4(ch, AFF_CAN_FLY) )
	return TRUE;

    return FALSE;
}

bool can_fly (CHAR_DATA *ch)
{
    if (ch->riding != NULL && can_fly(ch->riding)) return TRUE;

    if (ch->secondaryrace == RACE_DRAGON) return TRUE;

    if (IS_FLAG4(ch, AFF_WINGS)) return TRUE;

    if (IS_SET(ch->flag2, AFF_WING)) return TRUE;

    if (IS_SET(ch->affected_by, AFF_FLYING)) return TRUE;

    if (ch->cur_form != 0
	&& form_data[ch->cur_form].can_fly) return TRUE;
    if (ch->cur_form != 0
	&& !form_data[ch->cur_form].can_fly) return FALSE;

    if (IS_FLAG4(ch, AFF_CAN_FLY) )
	return TRUE;

    if (is_wearing_artifact(ch, RELIC_WIND_BOOTS) != NULL )
	return TRUE;

    return FALSE;
}


bool can_pass_wall (CHAR_DATA *ch, EXIT_DATA *pexit, int dir)
{
    if (IS_SET(pexit->exit_info, EX_PRISMATIC_WALL)
	&& ch->secondaryrace == 0)
    {
	act("$n is sucked through the prismatic wall.", ch, NULL, NULL, TO_ROOM);
	send_to_char("You walk through the shimmering prismatic wall.\n\r", ch);
	return TRUE;
    }

    else if (IS_SET(pexit->exit_info, EX_PRISMATIC_WALL)
	&& ch->secondaryrace != 0)
    {
	act("$n is sucked through the prismatic wall.", ch, NULL, NULL, TO_ROOM);
	send_to_char("You walk through the prismatic wall.\n\r", ch);
	switch (dice(1, 5))
	{
 	    case 1:
		char_from_room(ch);
		char_to_room(ch, get_rand_room(ch),0);
		act("$n appears in a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
		do_look(ch, "auto");
		return FALSE;
		break;
	    case 2:
		set_form(ch, FRM_FROG);
		send_to_char("Something feels different.\n\r", ch);
		return TRUE;
		break;
	    case 3:
		send_to_char("Your body is ripped apart.  You die.\n\r", ch);
		act("$n's body is ripped apart.", ch, NULL, NULL, TO_ROOM);
		destroy_char(ch, ch, TRUE);
		return FALSE;
		break;
	    case 4:
        	ADD_TIMER(ch, TIMER_ON_HASH, dice(4, 3));
		send_to_char("The world starts to spin.\n\r",ch);
		return TRUE;
		break;
	    case 5:
		send_to_char("Ouch!  You get electrified.\n\r", ch);
		act("$n gets an electric shock from the wall.", ch, NULL, NULL, TO_ROOM);
		damage_old(ch, ch, dice(6, 50), 0, DAM_LIGHTNING, FALSE);
		return FALSE;
		break;
        }
	return FALSE;
    }

    if (IS_SET(pexit->exit_info, EX_WARDING)
	&& (ch->secondaryrace == RACE_VAMPIRE
	||  ch->secondaryrace == RACE_DAEMON)
	&& ch->alignment < 300 )
    {
	send_to_char("The exit is warded by goodness.  You may not pass.\n\r", ch);
	return FALSE;
    }

    if (IS_SET(pexit->exit_info, EX_CALTROP_WALL) && is_flying(ch) )
    {
	send_to_char("You fly over the wall of caltrops.\n\r", ch);
	act("$n flies over the wall of caltrops.", ch, NULL, NULL, TO_ROOM);
	return TRUE;
    }

    if (IS_SET(pexit->exit_info, EX_CALTROP_WALL) && dir != -1
	&& !is_flying(ch) )
    {
	if (dice(1, 55) > get_curr_stat(ch, STAT_STR)
		&& get_size(ch) < SIZE_HUGE )
	{
	    send_to_char("You struggle against the wall and injure yourself.\n\r", ch);
	    act("$n struggles against the wall and injure $mself.\n\r", ch, NULL, NULL, TO_ROOM);
	    WAIT_STATE(ch, 6);
	    damage_old(ch, ch, dice(6, 10), 0, DAM_PIERCE, FALSE);
	    return FALSE;
	}
	else
	{
	    send_to_char("You smash through the wall of caltrops.\n\r", ch);
	    act("$n smashes through the wall of caltrops.", ch, NULL, NULL, TO_ROOM);
	    make_wall(ch->in_room, dir, 0);
	    return TRUE;
	}
    }

    if (IS_SET(pexit->exit_info, EX_WATER_WALL)
	&& !IS_NOVIERE(ch) )
    {
	send_to_char("A towering wall of water blocks your path.\n\r", ch);
	act("$n's path is blocked by the wall of water.", ch, NULL, NULL, TO_ROOM);
	return FALSE;
    }

    if (IS_SET(pexit->exit_info, EX_IRON_WALL)
	&& ch->tertiaryrace == TERTIARY_LICH
	&& dir != -1)
    {
	send_to_char("You touch the iron wall and it disintegrates into rust.\n\r", ch);
	act("$n touches the iron wall and it disintegrates into rust.", ch, NULL, NULL, TO_ROOM);
	make_wall(ch->in_room, dir, 0);
	return FALSE;
    }

    if (IS_SET(pexit->exit_info, EX_IRON_WALL)
	&& !IS_AFFECTED(ch, AFF_PASS_DOOR) )
    {
	send_to_char("A towering wall of iron blocks your path.\n\r", ch);
	act("$n's path is blocked by the wall of iron.", ch, NULL, NULL, TO_ROOM);
	return FALSE;
    }

    if (IS_SET(pexit->exit_info, EX_MUSHROOM_WALL)
	&& !IS_AFFECTED(ch, AFF_PASS_DOOR) )
    {
	send_to_char("The mushrooms block your path.\n\r", ch);
	return FALSE;
    }

    if (IS_SET(pexit->exit_info, EX_ICE_WALL)
	&& !IS_AFFECTED(ch, AFF_PASS_DOOR)
	&& ch->power[DISC_DAEM_GELU] < 5 )
    {
	send_to_char("A huge wall of ice blocks your way.\n\r", ch);
	return FALSE;
    }

    if (IS_SET(pexit->exit_info, EX_FIRE_WALL)
	&& ch->secondaryrace == 0)
    {
	act("$n bursts through the wall of fire.", ch, NULL, NULL, TO_ROOM);
	send_to_char("You jump through the flames and are unaffected.\n\r", ch);
	return TRUE;
    }

    else if (IS_SET(pexit->exit_info, EX_FIRE_WALL)
	&& ch->secondaryrace != 0)
    {
	act("$n bursts through the wall of fire.", ch, NULL, NULL, TO_ROOM);
	send_to_char("You jump through the flames.\n\r", ch);
	damage_old(ch, ch, dice(6, 50), skill_lookup("wall of fire"), DAM_FIRE, TRUE);
	return TRUE;
    }

    if (IS_SET(pexit->exit_info, EX_SWORD_WALL)
	&& ch->secondaryrace == 0)
    {
	act("$n bursts through the wall of swords.", ch, NULL, NULL, TO_ROOM);
	send_to_char("You jump through the swords and are unaffected.\n\r", ch);
	return TRUE;
    }

    else if (IS_SET(pexit->exit_info, EX_SWORD_WALL)
	&& ch->secondaryrace != 0)
    {
	act("$n jumps through the wall of swords.", ch, NULL, NULL, TO_ROOM);
	send_to_char("You jump through the swords.\n\r", ch);
	send_to_char("Aaaaaaaaarghhhhhhh! That hurt!\n\r", ch);
	damage_old(ch, ch, dice(6, 70), 0, DAM_SLASH, FALSE);
	return TRUE;
    }

    if (IS_SET(pexit->exit_info, EX_ASH_WALL) )
    {
	send_to_char("You scream in agony as the wall of ash rips apart your life force.\n\r", ch);
	act("$n screams in agony as the wall of ash rips $s life force apart.", ch, NULL, NULL, TO_ROOM);
	ch->hit /= 2;
	ch->move /= 2;
	ch->psp /= 2;
	return TRUE;
    }

    return TRUE;
}

char * get_move_message(CHAR_DATA *ch)
{
    if (is_item(ch)) return "rolls";

    if (is_flying(ch)) return "flies";

    if (ch->cur_form != 0 && form_data[ch->cur_form].move_word[0] != '\0')
	return form_data[ch->cur_form].move_word;

    if (IS_NPC(ch))
    {
	if (is_name("rabbit", GET_NAME(ch) )) return "hops";
	if (is_name("frog", GET_NAME(ch) )) return "hops";
	if (is_name("horse", GET_NAME(ch) )) return "trots";
	if (is_name("rat", GET_NAME(ch) )) return "scurries";
	if (is_name("cat", GET_NAME(ch) )) return "pounces";
	if (is_name("mouse", GET_NAME(ch) )) return "scurries";
	if (is_name("wolf", GET_NAME(ch) )) return "bounds";
	if (is_name("bat", GET_NAME(ch) )) return "flaps";
	if (is_name("boar", GET_NAME(ch) )) return "charges";

	if (lost_one_leg(ch)) return "hops";
	if (lost_both_legs(ch)) return "drags $mself";

        if (ch->in_room->sector_type == SECT_WATER_SWIM
	    || ch->in_room->sector_type == SECT_WATER_NOSWIM
	    || ch->in_room->sector_type == SECT_UNDERWATER) return "swims";
        return "walks";
    }

    if (ch->in_room->sector_type == SECT_WATER_SWIM
	|| ch->in_room->sector_type == SECT_WATER_NOSWIM
	|| ch->in_room->sector_type == SECT_UNDERWATER) return "swims";

    if (lost_one_leg(ch)) return "hops";
    if (lost_both_legs(ch)) return "drags $mself";

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 15)
 	return "staggers";

    if (IS_FLAG4(ch, AFF_AUTOWALKING)) return "runs";
    return "walks";
}


void move_char( CHAR_DATA *ch, int door, bool follow )
{
    CHAR_DATA *fch;
    CHAR_DATA *vch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room = NULL;
    ROOM_INDEX_DATA *to_room = NULL;
    EXIT_DATA *pexit = NULL;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    crash("move_char 1");

    if (ch->auto_look_line != -1)
	return;

    if (ch->riding != NULL && !IS_NPC(ch->riding))
	return;

    crash("start of move_char routine");

    if ( door < 0 || door > 5 )
    {
	bug( "Do_move: bad door %d.", door );
	return;
    }

    in_room = ch->in_room;

    if (is_item(ch) && ch->sw_obj->carried_by !=NULL)
    {
	stc("Not while carried!\n\r", ch);
	return;
    }

    if (is_item(ch)
	&& !IS_SET(ch->sw_obj->wear_flags, ITEM_TAKE))
    {
	stc("You are too heavy to move.\n\r", ch);
	return;
    }

    if (is_item(ch)
	&& IS_SET(ch->sw_obj->extra_flags3, ITEM_CANNOT_MOVE))
    {
	stc("You have no legs and cannot move.\n\r", ch);
	return;
    }
	
    if (is_item(ch) && ch->sw_obj->in_obj != NULL)
    {
	stcf(ch, "But you are inside %s.\n\r", 
	    ch->sw_obj->in_obj->short_descr);
	return;
    }

    if (is_item(ch) && ch->sw_obj->on != NULL)
    {
	stcf(ch, "You must get down off %s first.\n\r", 
	    ch->sw_obj->on->short_descr);
	return;
    }

    if (IS_FLAG4(ch, AFF_ENTWINED)
	|| IS_FLAG4(ch, AFF_SMOTHERED) )
    {
	send_to_char("The plants are entwining you, you cannot move.\n\r", ch);
	return;
    }

    if (IS_FLAG5(ch, AFF5_WEB_STRONG))
    {
	stc("You cannot move with all these sticky webs on.\n\r", ch);
	act("$n struggles against the webs restraining $m.", ch, NULL, NULL, TO_ROOM);
	return;
    }
    if (IS_SET(ch->flag4, AFF_SENESCHIA))
    {
	send_to_char("But you are a pile of leaves!\n\r", ch);
	return;
    }

    if (lost_both_legs(ch))
    {
	stc("You cannot move with no legs.\n\r", ch);
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "What?  And leave your beloved master?\n\r", ch );
	return;
    }

    if ( IS_NEWFLAG(ch, AFF_HOLD)
	&& !is_flying(ch) )
    {
	send_to_char("Your feet are held fast!\n\r", ch);
	return;
    }

    if ( !TIME_UP(ch, TIMER_MESMERISED) )
    {
	send_to_char("But you want to stay and watch all the pretty colours.\n\r", ch);
	return;
    }


/* altitude shit.  and lots of it.  not by RT but by Chris cause it actually
   does something other than clogging up the system with urine */

    crash("move_char routine - past checks");

  if (can_fly(ch))
  {
    if (door == DIR_UP && in_room->exit[door] == NULL)
    {
	if (IS_INSIDE(ch))
	{
	    send_to_char("There is no exit in that direction.\n\r",ch);
	    return;
	}

	if (ch->altitude >= 10)
 	{
	    send_to_char("You cannot fly any higher.\n\r",ch);
	    return;
	}

	if (ch->altitude == 0)
	    send_to_char("You leap into the air.\n\r",ch);
	else send_to_char("You soar higher.\n\r",ch);

	REMOVE_BIT(ch->flag4, AFF_LANDED);
	ch->altitude++;
	do_look(ch, "auto");
	return;
    }

    else if (ch->altitude > 0 && door == DIR_DOWN)
    {
	send_to_char("You dive downwards.\n\r",ch);
	ch->altitude--;
	do_look(ch, "auto");
	if (ch->altitude == 0) do_land(ch, "");
	return;
    }

    else if (ch->altitude != 0)
    {
	char buf[MAX_STRING_LENGTH];
	int alt;

/* rush fix */
	stc("You cannot fly in that direction.\n\r",ch);
	return;

	if ( ( pexit   = in_room->exit[door] ) == NULL
	||   ( to_room = pexit->u1.to_room   ) == NULL)
	{
	    send_to_char("You cannot fly in that direction.\n\r",ch);
	    return;
	}

	for (alt = 0; alt < ch->altitude; alt++)
	{
	    if ( to_room->exit[DIR_UP] == NULL)
	    {
		to_room = NULL;
		break;
	    }
	    if ((to_room = to_room->exit[DIR_UP]->u1.to_room) == NULL)
	    	break;
	}

	if (to_room != NULL
	    && to_room->sector_type != SECT_AIR)
	{
	    send_to_char("There is a ",ch);
	    switch( to_room->sector_type )
	    {
		case SECT_FIELD    : send_to_char("hedge", ch); break;
		case SECT_FOREST   : send_to_char("tree", ch); break;
		case SECT_HILLS    : send_to_char("hill", ch); break;
		case SECT_MOUNTAIN : send_to_char("mountain", ch); break;
		case SECT_DESERT   : send_to_char("sand dune", ch); break;
		default		   : send_to_char("building", ch); break;
	    }
	    send_to_char(" blocking your way.\n\r",ch);
	    return;
	}
	else if (to_room != NULL)
	{
	    buf[0] = '\0';
	    sprintf(buf, "You fly %s.\n\r", dir_name[door]);
	    send_to_char(buf, ch);

	    char_from_room( ch );
	    char_to_room( ch, to_room, ch->altitude );
	    ch->altitude -= alt;
	    do_look(ch, "auto");
	    if (ch->altitude == 0) do_land(ch, "");
	    return;
	}

	to_room = pexit->u1.to_room;
	for (alt = 0; alt < ch->altitude; alt++)
	{
	    if (to_room->exit[DIR_UP] == NULL 
		|| to_room->exit[DIR_UP]->u1.to_room)
		break;
	    to_room = to_room->exit[DIR_UP]->u1.to_room;
	}

	buf[0] = '\0';
	sprintf(buf, "You fly %s.\n\r", dir_name[door]);
	send_to_char(buf, ch);

	char_from_room( ch );
	char_to_room( ch, to_room, ch->altitude );
	ch->altitude -= alt;
	do_look(ch, "auto");
	if (ch->altitude == 0) do_land(ch, "");
	return;
    }

  }

    if  ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||   !can_see_room(ch,pexit->u1.to_room))
    {
	/*  panic!  This is blemming impossible.  Trace down to first room
	with exit in the right direction and then trace across and check
	if can fly there.  Then move to that room and trace back up to the
	highest room and adjust altitude to compensate for room levels.
	Not very easy really.  Yuck! */

	char buf[MAX_STRING_LENGTH];
	int alt = 0;

	if (door == DIR_UP || door == DIR_DOWN || TRUE)
	{
	    stc("There is no exit in that direction.\n\r",ch);
	    return;
	}

	for (to_room = ch->in_room; to_room != NULL; )
	{
	    if ( ( pexit   = to_room->exit[door] ) != NULL
	    &&   ( to_room = pexit->u1.to_room   ) != NULL)
	    {
		for ( ; to_room != NULL; )
		{
		    if ((to_room->exit[DIR_UP] == NULL
			|| to_room->exit[DIR_UP]->u1.to_room == NULL)
			&& can_fly(ch)
			&& !ROOM_INSIDE(to_room))
		    {
			buf[0] = '\0';
			sprintf(buf, "You fly %s.\n\r", dir_name[door]);
			act("$n flies %s.", ch, NULL, NULL, TO_ROOM);
			send_to_char(buf, ch);

			char_from_room( ch );
			char_to_room( ch, to_room, ch->altitude );
			ch->altitude += alt;
			REMOVE_BIT(ch->flag4, AFF_LANDED);
			do_look(ch, "auto");
			return;
	    	    }
		    if (to_room->exit[DIR_UP] == NULL)
		    {
			to_room = NULL;
			continue;
		    }
		    to_room = to_room->exit[DIR_UP]->u1.to_room;
		    alt -= 1;
		}
	    }
	    if (to_room->exit[DIR_DOWN] == NULL)
	    {
		to_room = NULL;
		continue;
	    }
	    to_room = to_room->exit[DIR_DOWN]->u1.to_room;
	    alt += 1;
	}

	send_to_char( "There is no exit in that direction.\n\r", ch );
	return;

    }
/*    well thats it for altitude here
       BTW, RT doesn't just stand for Russ Taylor, but Right Twat as well */

    crash("move_char routine - past altitude");

    if (IS_SET(pexit->exit_info, EX_CLOSED)
    &&  ((!IS_AFFECTED(ch, AFF_PASS_DOOR) && !IS_INSUBSTANTIAL(ch))
    || IS_SET(pexit->exit_info,EX_NOPASS))
    &&   !IS_TRUSTED(ch,ANGEL))
    {
	act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	return;
    }

    if ( !IS_NPC(ch) )
    {
	int move;

	if ( door == 4
	&&   to_room->sector_type == SECT_AIR )
	{
	    if ( !can_fly(ch) )
	    {
		send_to_char( "You can't fly.\n\r", ch );
		return;
	    }
	}

	if (get_size( ch ) >= SIZE_GIANT
		&& ROOM_INSIDE(to_room))
	{
	    send_to_char("The exit in that direction is too small for you to enter.\n\r", ch);
	    return;
	}

	if (ch->secondaryrace == RACE_VAMPIRE
	    && (to_room->sector_type == SECT_WATER_SWIM || 
		to_room->sector_type == SECT_WATER_NOSWIM)
	    && ch->cur_form != FRM_MIST)
	{
	    send_to_char("Vampires cannot cross running water.\n\r", ch);
	    return;
	}

        if (get_curr_stat(ch, STAT_STR) > 23
            && (IS_FLAG4(ch, AFF_ENTWINED) || IS_FLAG4(ch, AFF_SMOTHERED) ) )
        {
            send_to_char("You tear and rend through the plants restricting you.\n\r", ch);
            act("$n rips through the plants restricting $m.", ch, NULL, NULL,TO_ROOM);
            REMOVE_BIT(ch->flag4, AFF_SMOTHERED);
            REMOVE_BIT(ch->flag4, AFF_ENTWINED);
	} 

	if (  to_room->sector_type == SECT_WATER_NOSWIM
	&&    !is_flying(ch)
	&&    ch->cur_form != FRM_MIST
	&&    ch->cur_form != FRM_FROG
	&&    ch->cur_form != FRM_DOLPHIN
	&&    !IS_SET(ch->flag4, AFF_WALK_ON_WATER) )
	{
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    if (IS_IMMORTAL(ch))
		found = TRUE;

	    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
	    {
		send_to_char( "You need a boat to go there.\n\r", ch );
		return;
	    }
	}


	move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)]
	     ;

	move /= 2;  /* i.e. the average */

	if (ch->secondaryrace == RACE_VAMPIRE
	     && ch->power[DISC_VAMP_CELE] > 3)
	{
	    move -= 1;
	    if (move < 1) move = 1;
	}

	/* conditional effects */
	if (is_flying(ch) || IS_AFFECTED(ch, AFF_HASTE) )
	    move /= 2;

	if (IS_AFFECTED(ch,AFF_SLOW))
	    move *= 2;

	if (IS_NEWFLAG(ch,AFF_SHADOW_PLANE))
	    move = 0;

	if (!TIME_UP(ch, TIMER_ON_SPEED))
	    move = 0;

 	if (lost_one_leg(ch)) move *= 3;

	if ( ch->riding == NULL)
	{
	    if (ch->move < move)
	    {
		send_to_char( "You are too exhausted.\n\r", ch );
		return;
	    }
	}
	else
	{
	    if (ch->riding->move < move)
	    {
		act("$N is too exhausted to move.",ch, NULL, ch->riding,TO_CHAR);
		send_to_char("You are too exhausted to move.\n\r",ch->riding);
		return;
	    }
	}

	WAIT_STATE( ch, 1 );
	if (ch->riding == NULL)
                ch->move -= move;
	else
		ch->riding->move -= move;
    


    }

    if (!can_pass_wall (ch, pexit, door)) return;

    /* Ok, they have actually moved at this point in time */

    if ( in_room->sector_type != SECT_UNDERWATER
    &&   to_room->sector_type == SECT_UNDERWATER )
    {
	if ( !IS_NEWFLAG(ch, AFF_WATERBREATHING) && !IS_IMMORTAL(ch))
	{
	    send_to_char( "You hold your breath and dive under the surface.\n\r", ch );
	}
    }

    in_room->track[2] = in_room->track[1];
    in_room->track[1] = in_room->track[0];
    WRITE_STR(in_room->track[0], GET_PROPER_NAME(ch));    
    
    in_room->trackdir[2] = in_room->trackdir[1];
    in_room->trackdir[1] = in_room->trackdir[0];
    WRITE_STR(in_room->trackdir[0], dir_name[door]);

    /* I bet it doesnt work, but hell, thats life. */

    set_stance(ch, 0);
    REMOVE_BIT(ch->flag4, AFF_INVULNERABLE);

    switch(door)
    {
	case DIR_NORTH : sprintf(buf2, "the south"); break;
	case DIR_SOUTH : sprintf(buf2, "the north"); break;
	case DIR_WEST  : sprintf(buf2, "the east"); break;
	case DIR_EAST  : sprintf(buf2, "the west"); break;
	case DIR_DOWN  : sprintf(buf2, "above"); break;
	case DIR_UP    : sprintf(buf2, "below"); break;
	default        : sprintf(buf2, "the small box in the corner"); break;
    }

    crash("well here i am in move_char");

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    &&   ch->invis_level < LEVEL_HERO
    &&	ch->riden_by == NULL)
    {
	sprintf(buf,"%s%s %s $T%s%s.",GET_NAME(ch),GET_EXTENSION(ch),
		get_move_message(ch)
		,((ch->riding != NULL) ? ", riding on " : "")
		,((ch->riding != NULL) ? GET_NAME(ch->riding) : ""));
	act( buf, ch, NULL, dir_name[door], TO_ROOM );
    }

    if ( IS_AFFECTED(ch, AFF_SNEAK)
    &&   ch->invis_level < LEVEL_HERO
    &&	ch->riden_by == NULL)
    {
	for (vch = ch->in_room->people ; vch != NULL ; vch = vch->next_in_room)
  	{
	    if (vch != ch
	 	&& vch->power[DISC_VAMP_AUSP] >= 3
		&& vch->power[DISC_VAMP_AUSP] > ch->power[DISC_VAMP_OBFU])
	        act("You hear $N sneaking out of the room.", vch, NULL, ch, TO_CHAR);
	}
    }

    char_from_room( ch );
    char_to_room( ch, to_room, ch->altitude );

    crash("ok... move_char, they have just moved");

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    &&   ch->invis_level < LEVEL_HERO
    && ch->riden_by == NULL)
    {
	sprintf(buf,"%s%s %s in from %s%s%s.",GET_NAME(ch),GET_EXTENSION(ch)
		,get_move_message(ch), buf2
		,(ch->riding != NULL) ? ", riding on " : ""
		,(ch->riding != NULL) ? GET_NAME(ch->riding) : "");
	act( buf, ch, NULL, NULL, TO_ROOM );
    }

    if ( IS_AFFECTED(ch, AFF_SNEAK)
    &&   ch->invis_level < LEVEL_HERO
    &&	ch->riden_by == NULL)
    {
	for (vch = ch->in_room->people ; vch != NULL ; vch = vch->next_in_room)
  	{
	    if (vch != ch
	 	&& vch->power[DISC_VAMP_AUSP] > 2
		&& vch->power[DISC_VAMP_AUSP] > ch->power[DISC_VAMP_OBFU])
	        act("You hear $N sneaking into the room.", vch, NULL, ch, TO_CHAR);
	}
    }

    check_fall(ch);

    crash("ok... move_char just after check_fall");

    do_look( ch, "auto" );
    if (ch->riding != NULL) do_look(ch->riding, "auto");
    if (ch->riden_by != NULL) do_look(ch->riden_by, "auto");

    check_trigger_room(ch, TRIG_ENTER_ROOM, NULL);

    /* the alarm spell BBBBBBBBBBBRRRRRRRRRRRRRRRRRRRRIIIIIIIIINNNNNNGGGGGG */

    if (to_room->alarm != NULL && !IS_NPC(ch) )
    {
	char buff[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list ; d != NULL ; d = d->next )
	{
	    if (d->connected == CON_PLAYING
		&& d->character != NULL
		&& d->character != ch
		&& !str_cmp(GET_PROPER_NAME(d->character), ch->in_room->alarm) )
	    {
		sprintf(buff, "*** ALARM *** %s has entered %s.\7\7\7\7\7\n\r",
		    GET_PROPER_NAME(ch), ch->in_room->name );
	 	stc(buff, d->character);
		if (dice(1, UMIN(d->character->psi_level,5)) == 1) ch->in_room->alarm = NULL;
	    }
	}
    }


    if (in_room == to_room) /* no circular follows */
	return;

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;

	if (fch->altitude != ch->altitude) continue;

	if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM) 
	&&   fch->position < POS_STANDING
	&&   fch->position > POS_SLEEPING)
	    do_stand(fch,"");

	if ( fch->master == ch && fch->position == POS_STANDING 
	&&   can_see_room(fch,to_room))
	{
	    act( "You follow $N.", fch, NULL, ch, TO_CHAR );
	    move_char( fch, door, TRUE );
	}
    }
    crash("end of move_char");

    return;
}

bool can_go_up(CHAR_DATA *ch)
{
    if (ch->in_room->exit[DIR_UP] != NULL) return TRUE;

    if (ch->altitude == 10) return FALSE;

    if (ch->altitude > 0) return TRUE;

    if (IS_OUTSIDE(ch)) return TRUE;

    return FALSE;
}

void check_fall(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *to_room;
    int getskill;
    int fell = 0;
    bool disp = FALSE;

    crash("check fall.. oh shit... thats whats doing it!");

    getskill = get_skill(ch, gsn_tumbling);
    if (getskill > 80) getskill = 80;

    while (ch->in_room->sector_type == SECT_AIR
		&& !IS_NPC(ch)
		&& ch->level < 112
		&& !is_flying(ch)
		&& ch->in_room->exit[5] != NULL
		&& (!IS_AFFECTED(ch, AFF_PASS_DOOR) 
			|| IS_SET(ch->in_room->exit[5]->exit_info, EX_CLOSED)))
    {
	crash("check fall - infinite loop?");

	if (disp)
	{
	    do_look(ch,"auto");
	}
	fell += 1;
	act( "$n plummets downwards.", ch, NULL, NULL, TO_ROOM );
	to_room = ch->in_room->exit[5]->u1.to_room;
	char_from_room( ch );
	char_to_room( ch, to_room, ch->altitude );

	act("$n falls from the sky.....",ch,NULL,NULL,TO_ROOM);
	disp = TRUE;
    }

    if (fell > 0)
    {
    if (ch->in_room->sector_type == SECT_WATER_SWIM)
    {
	int dam;

	act(".....and splashes into the water.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You splash into the water.\n\r",ch);
	
	dam = (get_carry_weight(ch) + 200) * fell + dice (1,200);
	if (getskill > 60) dam /= 2;
	do_look(ch,"auto");

	if (ch->power[DISC_VAMP_SATY] >= 2)
	{
	    dam = 0;
	    stc("You land unharmed.\n\r", ch);
	}

	damage(ch, ch, dam/6, 0, 0, FALSE);
    }                     
    else if (ch->in_room->sector_type == SECT_WATER_NOSWIM)
    {
	int dam;

	act(".....and splashes into the water.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You splash into the water.\n\r",ch);
	
	dam = (get_carry_weight(ch) + 200) * fell + dice (1,200);
	do_look(ch,"auto");
	if (getskill > 60) dam /= 2;
	if (ch->power[DISC_VAMP_SATY] >= 2)
	{
	    dam = 0;
	    stc("You land unharmed.\n\r", ch);
	}

	damage(ch, ch, dam/10, 0, 0, FALSE);

	if (ch->in_room->exit[5] != NULL
		&& ch->in_room->exit[5]->u1.to_room->sector_type == SECT_UNDERWATER
		&& fell > 2)
	{
		to_room = ch->in_room->exit[5]->u1.to_room;
		act("$n plummets downwards disappearing into the depths.",ch,NULL,NULL,TO_ROOM);
		char_from_room( ch );
		char_to_room( ch, to_room, ch->altitude );
		act("$n splashes in from above.",ch,NULL,NULL,TO_ROOM);
		send_to_char("You plummet downwards under the waves into the depths.",ch);
	}

    }                     
    else
    {
	int dam;

	act(".....and hits the ground with a sickening thud.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You hit the ground with a sickening thud.\n\r",ch);
	
	dam = (get_carry_weight(ch) + 200) * fell + dice (1,200);
	if ((getskill > 30 && fell == 1)
		|| ch->power[DISC_VAMP_SATY] >= 2)
	{
	    send_to_char("You manage to land on your feet and avoid all damage.\n\r", ch);
 	    dam = 0;
	}
	else if (get_skill(ch, gsn_tumbling ) > 30)
	{
	    send_to_char("You manage to land on your feet.\n\r", ch);
	    dam *= 100;
	    dam /= (100 + getskill);
	    dam = UMAX(20, dam);
	}

	do_look(ch,"auto");
	damage(ch, ch, dam, 0, 0, FALSE);
    }
    }
}


void do_north( CHAR_DATA *ch, char *argument )
{
    if (ch->riding != NULL && !IS_NPC(ch->riding))
    {
	send_to_char("You'll have to dismount first.\n\r",ch);
	return;
    }

    move_char( ch, DIR_NORTH, FALSE );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    if (ch->riding != NULL && !IS_NPC(ch->riding))
    {
	send_to_char("You'll have to dismount first.\n\r",ch);
	return;
    }

    move_char( ch, DIR_EAST, FALSE );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    if (ch->riding != NULL && !IS_NPC(ch->riding))
    {
	send_to_char("You'll have to dismount first.\n\r",ch);
	return;
    }

    move_char( ch, DIR_SOUTH, FALSE );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    if (ch->riding != NULL && !IS_NPC(ch->riding))
    {
	send_to_char("You'll have to dismount first.\n\r",ch);
	return;
    }

    move_char( ch, DIR_WEST, FALSE );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    if (ch->riding != NULL && !IS_NPC(ch->riding))
    {
	send_to_char("You'll have to dismount first.\n\r",ch);
	return;
    }

    move_char( ch, DIR_UP, FALSE );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    if (ch->riding != NULL && !IS_NPC(ch->riding))
    {
	send_to_char("You'll have to dismount first.\n\r",ch);
	return;
    }

    move_char( ch, DIR_DOWN, FALSE );
    return;
}



int find_door( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else
    {
	for ( door = 0; door <= 5; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) )
		return door;
	}
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return -1;
    }

    return door;
}

void make_unconcious(CHAR_DATA *ch, int tim)
{
    if (ch->position != POS_SLEEPING)
    	act("$n slumps to the floor unconcious.", ch, NULL, NULL,TO_ROOM);
    ch->position = POS_SLEEPING;
    SET_TIMER(ch, TIMER_UNCONCIOUS, tim);
}

void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* open portal */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1], EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1], EX_CLOSED))
	    {
		send_to_char("It's already open.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1], EX_LOCKED))
	    {
		send_to_char("It's locked.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1], EX_CLOSED);
	    act("You open $p.",ch,obj,NULL,TO_CHAR);
	    act("$n opens $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER)
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	act("You open $p.",ch,obj,NULL,TO_CHAR);
	act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	act( "You open the $d.", ch, NULL, pexit->keyword, TO_CHAR );

	/* open the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
	    {
		if (rch->altitude != ch->altitude) continue;
		act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	    }
	}
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{

	    if (!IS_SET(obj->value[1],EX_ISDOOR)
	    ||   IS_SET(obj->value[1],EX_NOCLOSE))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's already closed.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_CLOSED);
	    act("You close $p.",ch,obj,NULL,TO_CHAR);
	    act("$n closes $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }

	SET_BIT(obj->value[1], CONT_CLOSED);
	act("You close $p.",ch,obj,NULL,TO_CHAR);
	act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit   = ch->in_room->exit[door];
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* close the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
	    {
		if (rch->altitude != ch->altitude) continue;
		act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	    }
	}
    }

    return;
}



bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return TRUE;
    }

    return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR)
	    ||  IS_SET(obj->value[1],EX_NOCLOSE))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }
	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK))
	    {
		send_to_char("It can't be locked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already locked.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_LOCKED);
	    act("You lock $p.",ch,obj,NULL,TO_CHAR);
	    act("$n locks $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	act("You lock $p.",ch,obj,NULL,TO_CHAR);
	act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit   = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("It can't be unlocked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already unlocked.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("You unlock $p.",ch,obj,NULL,TO_CHAR);
	    act("$n unlocks $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	act("You unlock $p.",ch,obj,NULL,TO_CHAR);
	act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* unlock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if (gch->altitude != ch->altitude) continue;
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	    act( "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }

    if ( !IS_NPC(ch) && number_percent( ) > get_skill(ch,gsn_pick_lock))
    {
	send_to_char( "You failed.\n\r", ch);
	check_improve(ch,gsn_pick_lock,FALSE,2);
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {   
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("It can't be unlocked.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_PICKPROOF))
	    {
		send_to_char("You failed.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
	    check_improve(ch,gsn_pick_lock,TRUE,2);
	    return;
	}

	
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
	act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
	check_improve(ch,gsn_pick_lock,TRUE,2);
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 && !IS_IMMORTAL(ch))
	    { send_to_char( "It can't be picked.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "Click...the door opens!\n\r", ch );
	act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	check_improve(ch,gsn_pick_lock,TRUE,2);

	/* pick the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}




void do_stand( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (argument[0] != '\0')
    {
	if (ch->position == POS_FIGHTING)
	{
	    send_to_char("Maybe you should finish fighting first?\n\r",ch);
	    return;
	}
	if (BODY_PART(ch, BODY_LEFT_LEG)
	    && BODY_PART(ch, BODY_RIGHT_LEG) )
	{
	    send_to_char("You cannot stand, you haven't got any legs.\n\r", ch);
	    return;
	}

	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	if (obj->item_type != ITEM_FURNITURE
	||  (!IS_SET(obj->value[2],STAND_AT)
	&&   !IS_SET(obj->value[2],STAND_ON)
	&&   !IS_SET(obj->value[2],STAND_IN)))
	{
	    send_to_char("You can't seem to find a place to stand.\n\r",ch);
	    return;
	}
	if (ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act_new("There's no room to stand on $p.",
		ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
	}
	ch->on = obj;
    }
    
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }
	
	if (obj == NULL)
	{
	    send_to_char( "You wake and stand up.\n\r", ch );
	    act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	   act_new("You wake and stand at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	   act("$n wakes and stands at $p.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act_new("You wake and stand on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    act("$n wakes and stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	else 
	{
	    act_new("You wake and stand in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    act("$n wakes and stands in $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_STANDING;
	do_look(ch,"auto");
	break;

    case POS_RESTING: case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char( "You stand up.\n\r", ch );
	    act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	    act("You stand at $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands at $p.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act("You stand on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	else
	{
	    act("You stand in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (IS_AFFECTED(ch, AFF_SLEEP))
    {
	send_to_char("You are under a magical slumber and cannot wake up.\n\r",ch);
	return;
    }

    if (ch->position == POS_FIGHTING)
    {
	send_to_char("You are already fighting!\n\r",ch);
	return;
    }

    /* okay, now that we know we can rest, find an object to rest on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)
    {
	if (!IS_SET(obj->item_type,ITEM_FURNITURE) 
	||  (!IS_SET(obj->value[2],REST_ON)
	&&   !IS_SET(obj->value[2],REST_IN)
	&&   !IS_SET(obj->value[2],REST_AT)))
	{
	    send_to_char("You can't rest on that.\n\r",ch);
	    return;
	}

	if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
	}
	
	ch->on = obj;
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	if (IS_AFFECTED(ch,AFF_SLEEP))
	{
	    send_to_char("You are affected by a magical slumber and cannot wake up.\n\r",ch);
	    return;
	}

	if (obj == NULL)
	{
	    send_to_char( "You wake up and start resting.\n\r", ch );
	    act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],REST_AT))
	{
	    act_new("You wake up and rest at $p.",
		    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
	    act("$n wakes up and rests at $p.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],REST_ON))
	{
	    act_new("You wake up and rest on $p.",
		    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
	    act("$n wakes up and rests on $p.",ch,obj,NULL,TO_ROOM);
	}
	else
	{
	    act_new("You wake up and rest in $p.",
		    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
	    act("$n wakes up and rests in $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_STANDING:
	if (obj == NULL)
	{
	    send_to_char( "You rest.\n\r", ch );
	    act( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM );
	}
	else if (IS_SET(obj->value[2],REST_AT))
	{
	    act("You sit down at $p and rest.",ch,obj,NULL,TO_CHAR);
	    act("$n sits down at $p and rests.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],REST_ON))
	{
	    act("You sit on $p and rest.",ch,obj,NULL,TO_CHAR);
	    act("$n sits on $p and rests.",ch,obj,NULL,TO_ROOM);
	}
	else
	{
	    act("You rest in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_RESTING;
	break;

    case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char("You rest.\n\r",ch);
	    act("$n rests.",ch,NULL,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],REST_AT))
	{
	    act("You rest at $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests at $p.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],REST_ON))
	{
	    act("You rest on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests on $p.",ch,obj,NULL,TO_ROOM);
	}
	else
	{
	    act("You rest in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_RESTING;
	break;
    }


    return;
}


void do_sit (CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (ch->position == POS_FIGHTING)
    {
	send_to_char("Maybe you should finish this fight first?\n\r",ch);
	return;
    }

    /* okay, now that we know we can sit, find an object to sit on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)                                                              
    {
	if (!IS_SET(obj->item_type,ITEM_FURNITURE)
	||  (!IS_SET(obj->value[2],SIT_ON)
	&&   !IS_SET(obj->value[2],SIT_IN)
	&&   !IS_SET(obj->value[2],SIT_AT)))
	{
	    send_to_char("You can't sit on that.\n\r",ch);
	    return;
	}

	if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
	}

	ch->on = obj;
    }
    switch (ch->position)
    {
	case POS_SLEEPING:
	    if (IS_AFFECTED(ch,AFF_SLEEP))
	    {
		send_to_char("You can't wake up!\n\r",ch);
		return;
	    }

	    if (obj == NULL)
	    {
		send_to_char( "You wake and sit up.\n\r", ch );
		act( "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM );
	    }
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act_new("You wake and sit at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
		act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act_new("You wake and sit on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
		act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else
	    {
		act_new("You wake and sit in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
		act("$n wakes and sits in $p.",ch,obj,NULL,TO_ROOM);
	    }

	    ch->position = POS_SITTING;
	    break;
	case POS_RESTING:
	    if (obj == NULL)
		send_to_char("You stop resting.\n\r",ch);
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("You sit at $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits at $p.",ch,obj,NULL,TO_ROOM);
	    }

	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("You sit on $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    ch->position = POS_SITTING;
	    break;
	case POS_SITTING:
	    send_to_char("You are already sitting down.\n\r",ch);
	    break;
	case POS_STANDING:
	    if (obj == NULL)
	    {
		send_to_char("You sit down.\n\r",ch);
		act("$n sits down on the ground.",ch,NULL,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("You sit down at $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits down at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("You sit on $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else
	    {
		act("You sit down in $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits down in $p.",ch,obj,NULL,TO_ROOM);
	    }
	    ch->position = POS_SITTING;
	    break;
    }
    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING: 
	if (argument[0] == '\0' && ch->on == NULL)
	{
	    send_to_char( "You go to sleep.\n\r", ch );
	    act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
	    ch->position = POS_SLEEPING;
	}
	else  /* find an object and sleep on it */
	{
	    if (argument[0] == '\0')
		obj = ch->on;
	    else
		obj = get_obj_list( ch, argument,  ch->in_room->contents );

	    if (obj == NULL)
	    {
		send_to_char("You don't see that here.\n\r",ch);
		return;
	    }
	    if (obj->item_type != ITEM_FURNITURE
	    ||  (!IS_SET(obj->value[2],SLEEP_ON) 
	    &&   !IS_SET(obj->value[2],SLEEP_IN)
	    &&   !IS_SET(obj->value[2],SLEEP_AT)))
	    {
		send_to_char("You can't sleep on that!\n\r",ch);
		return;
	    }

	    if (ch->on != obj && count_users(obj) >= obj->value[0])
	    {
		act_new("There is no room on $p for you.",
		    ch,obj,NULL,TO_CHAR,POS_DEAD);
		return;
	    }

	    ch->on = obj;
	    if (IS_SET(obj->value[2],SLEEP_AT))
	    {
		act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR);
		act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SLEEP_ON))
	    {
		act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR);
		act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else
	    {
		act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR);
		act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM);
	    }
	    ch->position = POS_SLEEPING;
	}
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  return; }

    act_new( "$n wakes you.", ch, NULL, victim, TO_VICT,POS_SLEEPING );
    do_stand(victim,"");
    return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    send_to_char( "You attempt to move silently.\n\r", ch );
    affect_strip( ch, gsn_sneak );
    WAIT_STATE(ch, 10);

    if (IS_AFFECTED(ch,AFF_SNEAK))
	return;

    if ( number_percent( ) < get_skill(ch,gsn_sneak))
    {
	check_improve(ch,gsn_sneak,TRUE,3);
	af.where     = TO_AFFECTS;
	af.type      = gsn_sneak;
	af.level     = ch->level; 
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );
    }
    else
	check_improve(ch,gsn_sneak,FALSE,3);

    return;
}



void do_hide( CHAR_DATA *ch, char *argument )
{
    if (weather_info.sunlight == SUN_LIGHT
	&& IS_OUTSIDE(ch))
    {
	stc("You cannot hide in the daylight.\n\r", ch);
	return;
    }

    send_to_char( "You attempt to hide in the shadows.\n\r", ch );
    WAIT_STATE(ch, 12);

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( number_percent( ) < get_skill(ch,gsn_hide))
    {
	SET_BIT(ch->affected_by, AFF_HIDE);
    }
    else
	check_improve(ch,gsn_hide,FALSE,4);

    return;
}

void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, gsn_invis                        );
    affect_strip ( ch, gsn_mass_invis                   );
    affect_strip ( ch, gsn_sneak                        );
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE            );
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE       );
    REMOVE_BIT   ( ch->flag2, AFF_IMPROVED_INVIS        );
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK           );
    if (str_cmp(argument, "nomessage")) send_to_char( "You turn visible.\n\r", ch );
    return;
}



void do_kaipsi( CHAR_DATA *ch, char *argument )
{
    int i,j;
    bool found=FALSE;
    char buf[MAX_STRING_LENGTH];

    stc("Removed due to new psionicist class.\n\r", ch);
    stc("Will be reimplimented soon to allow wild talents.\n\r", ch);
    return;

    if (!can_use_psionics(ch) )
    {
	send_to_char("You cannot use psionics.\n\r", ch);
	return;
    }
 
    if (ch->qp < 10)
    {
	send_to_char("You need 10 kai points to progress in psionics.\n\r", ch);
	return;
    }
    ch->qp -= 10;
    ch->max_psp += (ch->psi_potential);
    send_to_char("Controlling your kai flow, you increase your psionic potential.\n\r", ch);
    if (dice(1,4) == 1) ch->psi_level +=1;

    for (i = 0; psi_table[i].name != NULL; i++)
    {
	if (!ch->psi_learnt[i])
	    found = TRUE;
    }

    if (!found)
    {
	send_to_char("You can't learn any more abilities.\n\r",ch);
	return;
    }

    found = FALSE;
    while (!found)
    {
	crash("kaipsi? - infinite loop?");

	j = dice(1, i)-1;
	if (!ch->psi_learnt[j])
	{
	    ch->psi_learnt[j] = TRUE;
	    found = TRUE;
	    sprintf(buf, "#2You have gained the power of %s.\n\r",
	 	psi_table[j].name);
	    send_to_char(buf, ch);
	}
    }
}
int max_vamp_disc_level(CHAR_DATA *ch)
{
    int gen;
    gen = 14-ch->generation;

    if (gen < 4) return 10;
    if (gen == 4) return 9;
    if (gen == 5) return 8;
    if (gen == 6) return 7;
    if (gen == 7) return 6;

    return 5;
}

void do_train( CHAR_DATA *ch, char *argument )
{
    int cost, found, loop, value, pow;
    int ss = 0;
    char arg[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2);
    cost = 1;
    value = 1;

    if (arg2[0] != '\0')
	 value = is_number( arg2 ) ? atoi( arg2 ) : 1;

    if (value < 1)
	value = 1;

    if (value > 100)
	value = 100;

    found = 0;
    for (loop = 1; loop < MAX_DISCIPLINES; loop++)
    {
	if ( !str_prefix(arg, discipline[loop])
	     && ch->power[loop] < 10 && strlen(arg)>2
	     && ch->power[loop] > -1) found = 1;
    }

    if ( !str_cmp(arg, "hp" ) )
	cost = 1;

    else if ( !str_cmp(arg, "faith" ) )
	cost = 1;

    else if ( !str_cmp(arg, "education" ) )
	cost = 1;

    else if ( !str_cmp(arg, "vitality" ) )
	cost = 1;

    else if ( found == 1 )
	cost = 1;
    else
    {
	send_to_char("You can train:\n\r",ch);
	for (loop = 1; loop < MAX_DISCIPLINES ; loop++)
	{
	    if ( str_cmp("", discipline[loop])
		 && ch->power[loop] < 10
		 && ch->power[loop] > -1
		 && loop != DISC_DRAG_CONJ)
	    {
		send_to_char("   ",ch);
		send_to_char(discipline[loop], ch);
		send_to_char("\n\r",ch);
	    }
	}

	send_to_char("    ",ch);
	send_to_char("hp, vitality, education, or faith.\n\r",ch);
	sprintf(buf,"\n\rCurrent exp:  %ld\n\r",ch->currentexp);
	send_to_char(buf, ch);
	return;
    }

    if (!str_cmp("hp",arg))
    {
	ss = get_curr_stat(ch, STAT_CON);
	if (value > 1)
	{
	    for (value = value; value > 1; value--)
	    {
		if (value > 1) 
		{
		if ( ch->max_hit < ch->currentexp + ss && value!=1)
		{
		    ch->currentexp -= (ch->max_hit) - ss;
		    ch->pcdata->perm_hit += 1;
		    ch->max_hit += 1;
		    ch->hit +=1;   
		}
		}

	    }
	}

	if ( ch->max_hit > ch->currentexp+ss && value==1)
	{
	    send_to_char( "You don't have enough experience.\n\r", ch );
	    return;
	}
 
	ch->currentexp -= ch->max_hit - ss;
	ch->pcdata->perm_hit += 1;
	ch->max_hit += 1;
	ch->hit +=1;
	if (value == 1)
	    act( "Your durability increases.",ch,NULL,NULL,TO_CHAR);
	return;
    }

    if (!str_cmp("faith",arg))
    {
	int cst = 0, mult = 1000 * ch->faith + 1000, cnt = 0;
	ss = get_curr_stat(ch, STAT_WIS);
	for (; value > 0; value--)
	{
	    cst += mult;
	    mult += 1000;
	    cnt += 1;
	}

	if ( cst > ch->currentexp+ss )
	{
	    send_to_char( "You don't have enough exp.\n\r", ch );
	    return;
	}    
	ch->currentexp -= cst - ss;
	ch->faith += cnt;
	act( "Your faith in your god grows stronger.",ch,NULL,NULL,TO_CHAR);
	return;
    }

    if (!str_cmp("education",arg))
    {
	int cst = 0, cnt = 0, mult;
	if (IS_NPC(ch)) return;
	mult = ch->pcdata->education * 25000 + 25000;
	for (; value > 0; value--)
	{
	    cst += mult;
	    mult += 50000;
	    cnt += 1;
	}

	if ( cst > ch->currentexp )
	{
	    send_to_char( "You don't have enough exp.\n\r", ch );
	    return;
	}    
	ch->currentexp -= cst;
	ch->pcdata->education += cnt;
	act( "You improve your knowledge.",ch,NULL,NULL,TO_CHAR);
	return;
    }

    if (!str_cmp("vitality",arg))
    {
	ss = get_curr_stat(ch, STAT_STR);
	if (value > 1)
	{
	    for (value = value; value > 1; value--)
	    {
		if (value > 0)
		{
		    if ( ch->max_move < ch->currentexp + ss && value!=1)
		    {
			ch->currentexp -= ch->max_move -ss;
	 		ch->pcdata->perm_move += 1;
			ch->max_move += 1;
			ch->move +=1;
		    }
		}
	    }
	}
	if ( ch->max_move > ch->currentexp +ss )
	{
	    send_to_char( "You don't have enough exp.\n\r", ch );
	    return;
	}

	ch->currentexp -= ch->max_move - ss;
	ch->pcdata->perm_move += 1;
	ch->max_move += 1;
	ch->move += 1;
	act( "Your vitality increases!",ch,NULL,NULL,TO_CHAR);
	return;
    }

    for (loop = 1; loop < MAX_DISCIPLINES; loop++)
    {
	if ( !str_prefix(arg, discipline[loop])
	    && ch->power[loop] > -1 )
	{
	    if (ch->pcdata->disc_research == -1)
	    {
	    	stc("You haven't started researching a discipline yet.\n\r", ch);
	 	return;
	    }

	    if (ch->pcdata->disc_points != 666)
	    {
	    	stc("You haven't finished researching the discipline yet.\n\r", ch);
	 	return;
	    }

	    if (loop != ch->pcdata->disc_research)
	    {
		stc("You have not researched that discipline.\n\r", ch);
		return;
	    }

	    if (loop == DISC_DRAG_CONJ)
	    {
		stc("This is not a discipline which you are able to train.\n\r",ch);
		return;
	    }

	    ch->power[loop]+=1;
	    ch->pcdata->disc_research = -1;
	    ch->pcdata->disc_points = 0;
	    send_to_char("Your mastery of ",ch);
	    send_to_char(discipline[loop],ch);
	    send_to_char(" increases.\n\r",ch);

    /*** DO ADVANCEMENTS IN DISCIPLINE ***/

    if ( loop == DISC_DRAG_AWE )
    {
	if ( ch->power[DISC_DRAG_AWE] == 7 )
	    ADD_INNATE( ch, skill_lookup("power word stun"), 1, 72);
	return;
    }

    /* HERO CRUSADE */
    if ( loop == DISC_HERO_CRUS )
    {
	pow = ch->power[DISC_HERO_CRUS];

    }

    /* DAEMON CORRUPTION */
    if (loop == DISC_DAEM_CORR)
    {
	if (ch->power[DISC_DAEM_CORR] == 1)
	{
	    send_to_char("A shimmering image of a spider appears above you.\n\r", ch);
	    send_to_char("Lolth utters the words 'avern osa'.\n\r", ch);
	    send_to_char("You are now resistant to all poisons.\n\r", ch);
	}
	if (ch->power[DISC_DAEM_CORR] == 3)
	{
	    send_to_char("A shimmering image of a spider appears above you.\n\r", ch);
	    send_to_char("Lolth utters the words 'avern asmoer'.\n\r", ch);
	    send_to_char("You are now immune to all poisons.\n\r", ch);
	}
    }

    /* LUPINAL HAWK */
    if (loop == DISC_LUPI_HAWK)
    {
	if (ch->power[loop] == 1)
	{
	    send_to_char("You have gained the ability to see in the dark.\n\r", ch);
	    SET_BIT(ch->affected_by, AFF_INFRARED);
	}
    }

    /* LUPINAL COBRA */
    if (loop == DISC_LUPI_COBR)
    {
	if (ch->power[DISC_LUPI_COBR] == 3)
	    send_to_char("You have gained the ability to mesmerise your opponents.\n\r", ch);
	else if (ch->power[DISC_LUPI_COBR] == 5)
	    send_to_char("You are now lightning fast in combat.\n\r", ch);
	else if (ch->power[DISC_LUPI_COBR] == 1)
	{
	    send_to_char("You feel resistant to poison.\n\r", ch);
 	}
	else if (ch->power[DISC_LUPI_COBR] == 6)
	{
	    send_to_char("You are now immune to poison.\n\r", ch);
	}
	else if (ch->power[DISC_LUPI_COBR] == 4)
	{
	    send_to_char("Your fangs start to drip with toxic venom.\n\r", ch);
	    return;
	}
    }

    /* ELADRIN PHYSICA */
    if (loop == DISC_ELAD_PHYS)
    {
	switch(ch->power[DISC_ELAD_PHYS])
	{
	    case 1 :
		send_to_char("You feel much stronger.\n\r", ch);
		break;
	    case 2 : 
		send_to_char("You feel much more agile.\n\r", ch);
		ch->perm_stat[STAT_DEX] += 3;
		break;
	    case 3 :
		send_to_char("You now have the blessing of Nyssa, leader of the Noviere.\n\r", ch);
		SET_BIT(ch->flag4, AFF_WALK_ON_WATER);
		break;
	    case 4 :
		send_to_char("You now have the blessing of Sharazan, chief Shiere.\n\r", ch);
		break;
	    case 5 :
		send_to_char("You now have the blessing of Doasin, princess of Ghaele.\n\r", ch);
		break;
	    case 6 :
		send_to_char("You now have the blessing of Lorien, lord of Tulani.\n\r", ch);
		break;
	    default: break;		
        }
    }

    /* DAEMON HELLFIRE */
    if (loop == DISC_DAEM_HELL)
    {
	if (ch->power[DISC_DAEM_HELL] == 1)
	{
	    send_to_char("You feel more resistant to flame.\n\r", ch);
	}
	else if (ch->power[DISC_DAEM_HELL] == 2)
	{
	    send_to_char("You feel totally resistant to flame.\n\r", ch);
	}
	else send_to_char("You have gained a new power in the hellfire discipline.\n\r", ch);
    }


    /* ELADRIN ANSORCIA */

    if (loop == DISC_ELAD_ANSO)
    {
	send_to_char("You feel more resistant against magic.\n\r", ch);
    }


    /* DAEMONIC GELUGE LEVELS 1 to 10 */
    if (loop == DISC_DAEM_GELU)
    {
	pow = ch->power[DISC_DAEM_GELU];

	if (pow == 1)
	{
	    send_to_char("You feel more resistant to cold.\n\r", ch);
	}

	if (pow == 4)
	{
	    send_to_char("You feel invulnerable to cold.\n\r", ch);
	}

	if (pow == 5)
	    send_to_char("Your skin takes on an icy hardness.\n\r", ch);
    }


    /* DAEMONIC ATTACK LEVELS 1 to 10 */
    if (loop == DISC_DAEM_ATTA)
    {
	pow = ch->power[DISC_DAEM_ATTA];

	if (pow == 2)
	{
	    SET_BIT(ch->flag2, AFF_CLAW);
	    send_to_char("You grow a pair of wicked claws.\n\r", ch);
	    act("$n grows a pair of wicked claws.\n\r", ch, NULL, NULL, TO_ROOM);
	}
	if (pow == 3)
	{
	    SET_BIT(ch->flag2, AFF_TAIL);
	    send_to_char("Your spine extends into a long tail.\n\r", ch);
	    act("$n's spine extends into a long tail.\n\r", ch, NULL, NULL, TO_ROOM);
	}
	if (pow == 4)
	{
	    horn(ch);	
	}
	if (pow == 5)
	{
	    SET_BIT(ch->flag2, AFF_WING);
            SET_BIT(ch->flag4, AFF_WINGS);
            REMOVE_BIT(ch->flag4, AFF_LANDED);
	    send_to_char("A pair of leathery wings grow out of your back.\n\r", ch);
	    act("A pair of leathery wings grow out of $n's back.\n\r", ch, NULL, NULL, TO_ROOM);
	}
	if (pow == 7)
	{
	    SET_BIT(ch->flag4, AFF_STEEL_CLAWS);
	    send_to_char("Your claws harden to steel.\n\r", ch);
	}
	if (pow == 8) SET_BIT(ch->flag4, AFF_TAIL_STINGER);

    }


    /* WARLOCK MEDITATION */

    if (loop == DISC_WARL_THEA)
	send_to_char("You master the magical energies around you.\n\r", ch);

    /* VAMPIRES... MUHAHAHAHHAHAHAA */
        switch( loop )
	{
	    case DISC_VAMP_CELE: /* celerity */
		    send_to_char("You feel your reactions speeding up.\n\r", ch);
		    break;
	    case DISC_VAMP_POTE: /* potence */
		    break;
	    case DISC_VAMP_OBFU: /* obfuscate */
		    break;
	    case DISC_VAMP_AUSP: /* auspex */
		    break;
	    case DISC_VAMP_FORT: /* fortitude */
		    break;
	    case DISC_VAMP_THAU: /* thaumaturgy */
		    break;
	    case DISC_VAMP_PROT: /* protean */
		    stc("You feel in touch with the beast within.\n\r", ch);
		    break;
	    case DISC_VAMP_DOMI: /* dominate */
		    break;
	    default: break;
        }

	if (loop == DISC_LUPI_LUPI)
	{
	    switch(ch->power[loop])
	    {
		case 1 : SET_BIT(ch->flag2, AFF_CLAW);
			break;
		case 2 : SET_BIT(ch->flag2, AFF_BITE);
			break;
	    }
	}
    

    /*** END DO ADVANCEMENTS ***/
	    return;
    }
    }
    return;
}


void do_truesight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;
 
    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Your truesight dims and fades.\n\r", ch );
	change_upkeep(ch, -4);
    }
    else
    {
	if (change_upkeep(ch, 4))
	{
	    SET_BIT(ch->act, PLR_HOLYLIGHT);
	    send_to_char( "Your senses increase to an incredible degree.\n\r", ch );
	}
	else
	{
	    send_to_char( "You cannot risk burning this much blood.\n\r", ch );
	    return;
	}
    }

    return;
}

	

void do_shadow_breath(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim, *vch, *vch_next;   

    argument = one_argument( argument, arg );

    if (ch->dragtype != DRAGON_SHADOW)
    {
	stc("Only shadow dragons can use this ability.\n\r", ch);
	return;
    }

    if ( ch->breaths == 0 )
    {
	send_to_char( "You are out of breath.\n\r", ch );
	return;
    }
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Whom do you wish do breathe on?\n\r", ch );
	return;
    }

    if (ch->level <120) WAIT_STATE(ch, 20);
    ch->breaths -= 1;

    if (!strcmp("all",arg))
    {
	if (ch->power[DISC_DRAG_FIRE] < 5) 
	{
	   send_to_char("You have not yet attained enough power to do that.\n\r",ch);
	   return;
	}
	act("$n inhales, and sends forth a massive blast of darkness.",ch,NULL,NULL,TO_ROOM);
	act("You draw in a deep breath, and breathes darkness into the room.",ch,NULL,NULL,TO_CHAR);
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;
		if (vch->altitude < ch->altitude - 1
		&& vch->altitude > ch->altitude + 1) continue;
		if (vch != ch)
		{
			set_fighting(ch, vch);
			vch->hit = vch->hit * 5 / 6;
			if ( ch->power[DISC_DRAG_FIRE] > 1 )  vch->move = vch->move / 2;
			if ( ch->power[DISC_DRAG_FIRE] > 3 )  vch->hit = vch->hit / 2;
		}
	}
	return;
    }


    
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }           


    send_to_char("You inhale sharply, and issue forth a massive blast of darkness.\n\r",ch);
    act("$n draws in a mighty breath, and sends forth a huge blast of darkness towards $N.",ch, NULL, victim, TO_NOTVICT);
    act("$N inhales sharply, and sends forth a blast of darkness towards you.",victim, NULL, ch, TO_CHAR);
    send_to_char("You feel drained of all your energy.\n\r",victim);    
    victim->hit = victim->hit * 5 / 6;
    if ( ch->power[DISC_DRAG_FIRE] > 1 )  victim->move = victim->move / 3;
    if ( ch->power[DISC_DRAG_FIRE] > 3 )  victim->hit = victim->hit / 3;
    set_fighting(ch, victim);
}

void do_disciplines(CHAR_DATA *ch, char *argument)    
{
    char buf[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    int loop;
    int percent = 0, i;
    int indent = 0;

    if (IS_NPC(ch)) return;
    if (ch->secondaryrace != RACE_LUPINE)
        sprintf(buf, "---===[ Disciplines (%d known, %d max level) ]===---",disciplines_known(ch), max_vamp_disc_level(ch) );
    else
        sprintf(buf, "---===[ Guardians (%d known, %d max level) ]===---",disciplines_known(ch), max_vamp_disc_level(ch) );

    cent_to_char(buf, ch);
    send_to_char("\n\r", ch);

    for (loop = 0; loop < MAX_DISCIPLINES; loop++)
    {
	if (discipline[loop][0] != '\0'
	&& strlen(discipline[loop]) != 0
	&& ch->power[loop] >= 0 )
	{
	    sprintf(buf2, discipline[loop]);
	    buf2[0] = UPPER(buf2[0]);

	    sprintf(buf,"     %-14s: %-2d",
		buf2,
		ch->power[loop]);
	    send_to_char(buf,ch);
	    indent++;
	    if (indent == 3)
	    {
		stc("\n\r", ch);
		indent = 0;
	    }
	}
    }
    if (indent != 0) stc("\n\r", ch);

    /* This tells them if they are currently researching */
    if (ch->pcdata->disc_research < 0)
    {
	send_to_char("\n\r", ch);
	divide6_to_char(ch);
	return;
    }

    if (ch->power[ch->pcdata->disc_research] < 0)
    {
	send_to_char("\n\r", ch);
	divide6_to_char(ch);
	return;
    }

    if (ch->pcdata->disc_points == 666)
    {
	stc("\n\r", ch);
	sprintf(buf, "You have finished researching %s.",
	    discipline[ch->pcdata->disc_research]);
	cent_to_char(buf, ch);
	stc("\n\r", ch);
	divide6_to_char(ch);
	return;
    }

    if (ch->pcdata->disc_research < MAX_DISCIPLINES)
    {
	send_to_char("\n\r", ch);
	sprintf(buf, "You are currently researching %s.",
	    discipline[ch->pcdata->disc_research]);
    	cent_to_char(buf, ch);
    }

    percent = ch->pcdata->disc_points * 40 / disc_points_needed(ch);

    sprintf(buf, "-[#1");

    for( i = 0 ; i < percent ; i++)
	strcat(buf, "*");

    for( i = percent ; i < 40 ; i++)
	strcat(buf, " ");

    strcat(buf, "#n]-");

    cent_to_char(buf, ch);
}

void do_breakcharm(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) return;

    if ( !IS_SET(ch->affected_by, AFF_CHARM) && !IS_NEWFLAG(ch, AFF_FEAR))
    {
	send_to_char("You are not charmed.\n\r", ch);
	return;
    }
  
    WAIT_STATE(ch, 12); 

    if ( dice (10,11) > ch->level )
    {
	send_to_char("The spell is too powerful for you to overcome.\n\r",ch);
	return;
    }

    ch->master = NULL;
    affect_strip(ch, skill_lookup("charm person") );
    affect_strip(ch, skill_lookup("fear") );
    send_to_char("You break the charm and regain your free will.\n\r",ch);
    if (ch->pcdata->conditioned_by != NULL && ch->generation > 7)
    {
	ch->pcdata->conditioned_by = NULL;
	stc("You are no longer conditioned.\n\r", ch);
    }
}    

void do_kiss(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	send_to_char("Whom do you wish to kiss?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, argument ) ) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    if (ch->faith == 0)
    {
	send_to_char("You do not have enough faith in your god.\n\r", ch);
	return;
    }

    ch->faith -= 1;

    if ( dice (10,6) > ch->level )
    {
	send_to_char("You failed to subvert your victim.\n\r",ch);
	return;
    }
    victim->alignment -= dice(3,5);
    if (victim->alignment < -1000)
	victim->alignment = -1000;

    WAIT_STATE(ch, 12);	
    send_to_char("You plant your evil suggestions in your victim's mind.\n\r",ch);
    send_to_char("Satanic thoughts brew in your subconcious.\n\r",victim); 
}    

void do_lure(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
 
    if (argument[0] == '\0')
    {
	send_to_char("Whom do you wish to lure?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, argument ) ) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch, victim)) return;
    if (sanity_resist(ch, victim, 5)) return;

    if (ch->faith == 0)
    {
	send_to_char("You do not have enough faith in your vile gods.\n\r", ch);
	return;
    }

    ch->faith -= 1;

    victim->faith -= dice(1, 3);
    if (victim->faith < 0)
	victim->faith = 0;

    act("$n cackles insanely.", ch, NULL, NULL, TO_ROOM);

    WAIT_STATE(ch, 8);	
    send_to_char("You plant your vile suggestions in your victim's mind.\n\r",ch);
    send_to_char("You suddenly have a disturbing lack of faith.\n\r",victim);  
}

void do_regenerate(CHAR_DATA *ch, char *argument)
{
    int gen;

    gen = ch->generation / 3 + 4;

    if (IS_NEWFLAG(ch, AFF_NOREGEN))
    {
	send_to_char("Your body will not let you regenerate.\n\r",ch);
	return;
    }

    if (ch->blood < 4)
    {
	send_to_char("You need blood in order to regenerate.\n\r",ch);
	return;
    }

    if (ch->position == POS_FIGHTING)
    {
	send_to_char("You cannot regenerate in combat.\n\r",ch);
	return;
    }    

    if (ch->hit == ch->max_hit &&
	ch->move == ch->max_move)
    {
	send_to_char("You have fully regenerated yourself.\n\r",ch);
    }


    if (ch->hit < ch->max_hit && ch->blood > 3)
    {
	ch->hit+=dice(gen, 20);
	if (ch->hit > ch->max_hit)
	{
	   send_to_char ("You are fully healed and may not regenerate further.\n\r", ch);
	   ch->hit = ch->max_hit;
	}
	else
	{
	   send_to_char ("Your wounds stop bleeding and heal up.\n\r", ch);
	}
	ch->blood -= 4;
    }
    if (ch->move < ch->max_move && ch->blood > 3)
    {
	ch->move+=dice(gen, 20);
	if (ch->move > ch->max_move)
	{
		ch->move = ch->max_move;
	}
	else
	{
		send_to_char("You feel refreshed.\n\r", ch);
	}
	ch->blood -= 4;
    }
    WAIT_STATE(ch, 8);	    
}


void do_inferno(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch, *vch_next;
    
    if (ch->power[DISC_DAEM_HELL] <3)
    {
	send_to_char("You need more experience in the Hellfire discipline.\n\r", ch);
	return;
    }

    if ( ch->hit > 0 )
    {
	send_to_char("This is only used as a last resort.\n\r",ch);
	return;
    }

    WAIT_STATE(ch, 16);
    act("$n explodes, sending forth a massive fireballs in all directions.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You explode in a huge blast of abyssal flame.\n\r",ch);
    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	    vch_next = vch->next_in_room;
	    if (vch->altitude != ch->altitude) continue;
	    if (vch != ch)
	    {
		damage_old(vch, ch, ch->power[DISC_DAEM_HELL] * 1500,75, DAM_FIRE, TRUE);
		damage_old(ch, ch, ch->power[DISC_DAEM_HELL] * 1500,75, DAM_FIRE, TRUE);
		fire_effect(vch, ch->level, 2000, TARGET_CHAR);
	    }
    }
    ch->hit = -10;
    destroy_char(ch, ch, FALSE);
}   




void do_dark_vision(CHAR_DATA *ch, char *argument)
{
    if (IS_AFFECTED(ch, AFF_DARK_VISION))
    {
	REMOVE_BIT(ch->affected_by, AFF_DARK_VISION);
	change_upkeep(ch, -1);
	send_to_char("The fire in your eyes fades.\n\r",ch);
	act("The glow in $n's eyes fades.",ch,NULL,NULL,TO_ROOM);
    }
    else
    {
	if (ch->blood < 5)
	{
	    send_to_char("Your first priority is blood....\n\r",ch);
	    return;
	}
	ch->blood -= 4;
	SET_BIT(ch->affected_by, AFF_DARK_VISION);
	change_upkeep(ch, 1);
	send_to_char("Your senses increase to incredible proportions.\n\r",ch);
	act("$n's eyes start to glow with a faint red light.",ch,NULL,NULL,TO_ROOM);
    }
}


void do_squish(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;   

    argument = one_argument( argument, arg );

    if (lost_both_legs(ch))
    {
	stc("You cannot squish somebody if you have no legs.\n\r", ch);
	return;
    }
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Who do you want to squish?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }           

    if (!form_data[victim->cur_form].squishable)
    {
	send_to_char("You can't squish them, they are too big!\n\r",ch);
	return;
    }

    if (form_data[ch->cur_form].squishable)
    {
	send_to_char("You are too small to squish things!\n\r",ch);
	return;
    }

    set_form(victim, get_normal_form(victim));
    victim->hit = -10;

    if (player_kill(ch, victim)) player_kill_update(ch, victim, FALSE);

    send_to_char("You stamp your foot on their little head, smashing it to bits.\n\r",ch);
    act("$N stamps his foot on your head, smashing your skull and spilling\n\ryour brains onto the floor.", victim, NULL, ch, TO_CHAR);
    act("$N stamps his foot on $n's head, smashing $s skull and spilling\n\ra disgusting stream of brains out onto the floor.", victim, NULL, ch, TO_NOTVICT);
    damage_old(ch, victim, 2000, 0, 0, FALSE);
}

void do_morphia(CHAR_DATA *ch, char *argument)
{
   if (ch->power[DISC_ELAD_MORP] < 1)
   {
	send_to_char("You have not yet gained mastery of that discipline.\n\r",ch);
	return;
   }

   if ( argument[0] == '\0' )
   {
	send_to_char("What do you wish to change into?\n\r",ch);
	return;
   }

   if (!str_prefix(argument, "back"))
   {
	set_form(ch, get_normal_form(ch));
	act("$n resumes $s normal form.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You change back into an Eladrin.\n\r",ch);
	return;
   }

   if (!str_prefix(argument, "rabbit"))
   {
	set_form(ch, FRM_RABBIT);
	act("There is a flash of light and $n turns into a bunny rabbit.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You change into a little white rabbit.\n\r",ch);
	return;
   }

   if (!str_prefix(argument, "dolphin") && ch->power[DISC_ELAD_MORP]>1)
   {
	set_form(ch, FRM_DOLPHIN);
	act("$n assumes the form of a dolphin, made purely of water.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You assume the form of a dolphin.\n\r",ch);
	return;
   }
	
   if (!str_prefix(argument, "whirlwind") && ch->power[DISC_ELAD_MORP]>2)
   {
	set_form(ch, FRM_WHIRLWIND);
	act("$n starts to spin around at an alarming rate.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You whirl your body around rapidly.\n\r",ch);
	return;
   }
	
   if (!str_prefix(argument, "pillar of flame") && ch->power[DISC_ELAD_MORP]>3)
   {
	set_form(ch, 5);
	act("$n bursts into flames, becoming a pillar of fire.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You turn into a roaring pillar of fire.\n\r",ch);
	return;
   }
	
   if (!str_prefix(argument, "ball of light") && ch->power[DISC_ELAD_MORP]>4)
   {
	set_form(ch, 7);
	act("$n's body dissipates into a glowing ball of light.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You turn into a glowing ball of energy.\n\r",ch);
	return;
   }

   if (!str_prefix(argument, "list"))
   {
   send_to_char("You can transform yourself into a:\n\r",ch);
   send_to_char("      Rabbit",ch);
   if (ch->power[DISC_ELAD_MORP]>1)
	send_to_char(", Dolphin",ch);
   if (ch->power[DISC_ELAD_MORP]>2)
	send_to_char(", Whirlwind",ch);
   if (ch->power[DISC_ELAD_MORP]>3)
	send_to_char(", Pillar of Flame",ch);
   if (ch->power[DISC_ELAD_MORP]>4)
	send_to_char(", Ball of Light",ch);
   if (ch->cur_form != 0)
	send_to_char(", Back into an Eladrin",ch);
   send_to_char(".\n\r",ch);
   return;
   }

   send_to_char("Syntax:  change <form|list>\n\r",ch);
}
	
void do_reset(CHAR_DATA *ch, char *argument)
{
    if (ch->cur_form == 0)
    {
	send_to_char("You are already form 0.\n\r", ch);
	return;
    }
    set_form(ch, get_normal_form(ch));
    send_to_char("You are now human again.\n\r", ch);
    act("$n polymorphs into a human.", ch, NULL, NULL, TO_ROOM);
}

void do_change(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );
    
    if ((ch->in_room->sector_type == SECT_UNDERWATER
	|| ch->in_room->sector_type == SECT_WATER_NOSWIM	
	|| ch->in_room->sector_type == SECT_WATER_SWIM)
	&& ch->level != 120)	
    {
	send_to_char("You may not change forms above water.\n\r", ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "What do you wish to change into?\n\r", ch );
	return;
    }

    if ( ch->blood < 30 )
    {
	send_to_char("You lack the blood to change your form.\n\r",ch);
	return;
    }

    if ( ch->cur_form >3 && ch->cur_form != FRM_CRIMSON)
    {
	send_to_char("Your current form does not allow you to change.\n\r", ch);
	return;
    }

    if ( str_prefix(arg,"human")
	&& str_prefix(arg,"bat")
	&& str_prefix(arg,"back")
	&& str_prefix(arg,"wolf")
	&& str_prefix(arg,"mist")
	&& str_prefix(arg,"crimson death") )
    {
	send_to_char("You cannot change into that.\n\r",ch);
	return;
    }


    if ( !str_prefix(arg,"human") || !str_prefix(arg, "back") )
    {
	if (ch->cur_form == 0)
        {
	    send_to_char("You are already in that form.\n\r", ch);
	    return;
	}
	send_to_char("You return to your humanoid form.\n\r", ch);
	act("$n returns to $s humanoid form.",ch, NULL, NULL, TO_ROOM);
	if (ch->cur_form == FRM_CRIMSON) change_upkeep(ch, -5);
	set_form(ch, 0);
	return;
	change_upkeep(ch, -3);
	ch->blood -= 10;
    }


    if (ch->cur_form != 0)
    {
	send_to_char("You can only change into this from human form.\n\r",ch);
	return;
    }

    ch->blood -= 30;

    if ( !str_prefix(arg, "crimson death") )
    {
	if(ch->tertiaryrace != TERTIARY_CRIMSON)
	{
	    send_to_char("You cannot change into that.\n\r", ch);
	    return;
	}
	if (ch->cur_form == FRM_CRIMSON)
	{
	    send_to_char("You are already in that form.\n\r", ch);
	    return;
	}

	change_upkeep(ch, 8);
	set_form(ch, FRM_CRIMSON);
	send_to_char("You dissipate into a thin crimson mist.\n\r", ch);
	act("$n dissipates into a crimson mist.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if ( !str_prefix(arg,"wolf") )
    {
	if (ch->power[DISC_VAMP_PROT] <4)
	{
	    send_to_char("You are not skilled enough at the Protean discipline.\n\r", ch);
	    return;
	}
	send_to_char("You drop to your knees and howl with pain as you change into a dire wolf.\n\r", ch);
	act("$n drops to $s knees and screams in agony as $e turns into a wolf.",ch, NULL, NULL, TO_ROOM);
	change_upkeep(ch, 3);
	set_form(ch, FRM_WOLF);
	return;
    }

    if ( !str_prefix(arg,"bat") )
    {
	if (ch->power[DISC_VAMP_PROT] <4)
	{
	    send_to_char("You are not skilled enough at the Protean discipline.\n\r", ch);
	    return;
	}
	send_to_char("Your begin the painful transformation into a bat.\n\r", ch);
	act("$n slowly changes into a bat and flaps around.",ch, NULL, NULL, TO_ROOM);
	change_upkeep(ch, 3);
	set_form(ch, FRM_BAT);
	return;
    }

    if ( !str_prefix(arg,"mist") )
    {
	if (ch->power[DISC_VAMP_PROT] < 5)
	{
	    send_to_char("You are not skilled enough at the Protean discipline.\n\r", ch);
	    return;
	}
	if (ch->cur_form == FRM_MIST && !TIME_UP(ch, TIMER_WRENCHED) )
	{
	    send_to_char("You have been wrenched from the negative plane and may not do that.\n\r", ch);
	    return;
 	}
	send_to_char("You turn into an incorporeal mist.\n\r", ch);
	act("$n dissipates and turns into a vaporous mist.",ch, NULL, NULL, TO_ROOM);
	change_upkeep(ch, 3);
	set_form(ch, FRM_MIST);
	return;
    }

}



/*
 *
 * Thaumaturgy
 *
 */


void do_thaumaone(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int gen;
 
    if (argument[0] == '\0')
    {
	send_to_char("Whose blood do you wish to taste?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    { 
	send_to_char( "They aren't here.\n\r",ch ); 
	return; 
    }

    WAIT_STATE(ch, 8);

    if (victim->secondaryrace == RACE_VAMPIRE)
	gen = 14 - victim->generation;
    else gen = victim->generation;

    sprintf(buf, "You deduce your victim has %d blood points, and is generation %d.\n\r",victim->blood, gen);
    send_to_char(buf, ch);
}

void do_thaumatwo(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	send_to_char("Who do you wish to frenzy?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    { 
	send_to_char( "They aren't here.\n\r",ch ); 
	return; 
    }

    if (victim->secondaryrace!=3)
    {
	send_to_char("They are not kindred.\n\r",ch);
	return;
    }

    if (victim->generation > ch->generation
	|| victim->level > 111)
    {
        act("$n touches $N on the shoulder.", ch, NULL, victim, TO_NOTVICT);
        act("You touch $N on $S shoulder, and nothing seems to happen.",ch,NULL,victim,TO_CHAR);
	act("$N touches you on the shoulder.", victim, NULL, ch, TO_CHAR);
	WAIT_STATE(ch, 12);
	return;
    }

    victim->blood -= 10;
    act("$n touches $N on the shoulder.", ch, NULL, victim, TO_NOTVICT);
    act("You touch $N on $S shoulder, and send him into a frenzy of excitement.",ch,NULL,victim,TO_CHAR);
    act("$N touches you on the shoulder.  You feel a frenzy sweep over you.",victim,NULL,ch,TO_CHAR);
    WAIT_STATE(ch, 12);
//    do_berserk(victim, "");
}

void do_thaumathree(CHAR_DATA *ch, char *argument)
{
    SET_BIT( ch->flag2, AFF_BLOOD_POTENCE );

    send_to_char("You feel the potency of your blood increase.\n\r",ch);

}

void do_thaumafour(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	send_to_char("Whose blood do you wish to steal?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    { 
	send_to_char( "They aren't here.\n\r",ch ); 
	return; 
    }

    WAIT_STATE(ch, 12);

    if (victim->level > ch->level + ch->power[DISC_VAMP_THAU] * 2
	|| victim->level > 111)
    {
	send_to_char("You concentrate but you cannot seem to steal their blood.\n\r", ch);
	return;
    }

    if (victim->secondaryrace == RACE_VAMPIRE
	&& victim->generation > ch->generation)
    {
	send_to_char("You cannot affect a more powerful vampire than yourself.\n\r", ch);	
	return;
    }

    send_to_char("You feel your blood seeping out of your body.\n\r",victim);

    victim->blood -= 20;
    if (victim->blood < 0)
	victim->blood = 0;
    ch->blood += 10;
    if (ch->blood > max_blood_pool(ch))
	ch->blood = max_blood_pool(ch);
    send_to_char("Your drain their blood into your own body.\n\r",ch);

    if (IS_NPC(victim))
	set_fighting(ch, victim);
}

void do_thaumafive(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	send_to_char("Whose blood do you wish to boil?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    { 
	send_to_char( "They aren't here.\n\r",ch ); 
	return; 
    }

    if (victim->level > 111)
    {
	send_to_char("They seem unaffected.\n\r", ch);
	return;
    }

    if (victim->secondaryrace!=3)
    {
	send_to_char("You may only perform this on other kindred.\n\r",ch);
	return;
    }

    WAIT_STATE(ch, 12);

    send_to_char("Your blood boils and burns inside you.\n\r",victim);
    damage_old(ch, victim, dice(10, 10), 0, DAM_FIRE, FALSE);
    victim->blood -= 10;
    if (victim->blood < 0)
	victim->blood = 0;
}

void do_spike(CHAR_DATA *ch, char *argument)
{
    if (ch->rage == 0)
    {
	stc("You are not raged enough to do this yet.\n\r", ch);
	return;
    }

    if (ch->fighting == NULL)
    {
	stc("You may only do this during combat.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);
    act("A spike grows from under your skin and shoots towards $N.", ch, NULL, ch->fighting, TO_CHAR);
    act("A spike grows from under $n's skin and shoots towards $N.", ch, NULL, ch->fighting, TO_NOTVICT);
    act("A spike grows from under $n's skin and shoots towards you.", ch, NULL, ch->fighting, TO_VICT);

    if (dice(1, 10) > 5) /* Missed ? */
    {
	room_message(ch->in_room, "The spike misses, falls to the floor and disintegrates.");
	return;
    }

    stc("The spike hits you in the chest.\n\r",ch->fighting);
    act("The spike hits $n in the chest.", ch->fighting, NULL, NULL, TO_ROOM);
    if (dice(1, 10) == 1) do_say(ch->fighting, "Ouch!");
    damage_old(ch, ch->fighting, dice(3, 20), 0, DAM_OTHER,FALSE);
}


void do_moonstrike(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (IS_DAY())
    {
	stc("You cannot call a moonstrike in the daytime.\n\r", ch);
	return;
    }

    if (IS_INSIDE(ch))
    {
	stc("You cannot call a moonstrike while inside.\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    WAIT_STATE(ch, 12);
    room_message(ch->in_room, "The entire room is bathed in moonlight.");

    stc("A beam of moonlight shoots down and bathes you in its iridescent glow.\n\r", victim);
    act("A beam of moonlight shoots down and bathes $n.", victim, NULL, NULL, TO_ROOM);
    if (victim->secondaryrace == RACE_LUPINE
	|| victim->secondaryrace == RACE_VAMPIRE) return;

    stc("As your skin burns you scream in agony.\n\r", victim);

    damage_old(ch, victim, dice(6, 30), 0, DAM_OTHER,FALSE);
}


void do_feed(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    { 
	send_to_char( "They aren't here.\n\r",ch ); 
	return; 
    }

    if (!IS_SET(victim->flag2, AFF_ALLOW_VAMP))
    {
	stc("They refuse to drink your blood.\n\r", ch);
	return;
    }

    if (ch->blood < 5)
    {
	send_to_char("You do not have enough blood.\n\r",ch);
	return;
    }

    WAIT_STATE(ch, 6);

    ch->blood -= 5;
    victim->blood += 5;

    act("You open up your wrist and feed $N blood.",ch,NULL,victim,TO_CHAR);
    act("$n feeds you blood from $s wrist.",ch,NULL,victim,TO_VICT);
    act("$n feeds $N blood from $s wrist.",ch,NULL,victim,TO_NOTVICT);

    if (victim->secondaryrace == 0
	&& IS_EVIL(victim)
	&& IS_SET(victim->flag2, AFF_ALLOW_VAMP)
	&& ch->qp > 9
	&& victim->qp > 9
	&& TIMER(ch, TIMER_TAINT) == 0)
    {
	stc("Your skin pales and cools.\n\r", victim);
	taint_transform(victim, RACE_VAMPIRE);
	ch->qp -= 10;
	victim->qp -= 10;
	victim->alignment -= 30;
	if (ch->power[DISC_VAMP_POTE] > 1)
            victim->power[DISC_VAMP_POTE] = ch->power[DISC_VAMP_POTE] / 2;
	if (ch->power[DISC_VAMP_FORT] > 1)
            victim->power[DISC_VAMP_FORT] = ch->power[DISC_VAMP_FORT] / 2;
	if (ch->power[DISC_VAMP_CELE] > 1)
            victim->power[DISC_VAMP_CELE] = ch->power[DISC_VAMP_CELE] / 2;
	if (ch->power[DISC_VAMP_ANIM] > 1)
            victim->power[DISC_VAMP_ANIM] = ch->power[DISC_VAMP_ANIM] / 2;
	if (ch->power[DISC_VAMP_OBFU] > 1)
            victim->power[DISC_VAMP_OBFU] = ch->power[DISC_VAMP_OBFU] / 2;
	if (ch->power[DISC_VAMP_AUSP] > 1)
            victim->power[DISC_VAMP_AUSP] = ch->power[DISC_VAMP_AUSP] / 2;
	if (ch->power[DISC_VAMP_PRES] > 1)
            victim->power[DISC_VAMP_PRES] = ch->power[DISC_VAMP_PRES] / 2;
	if (ch->power[DISC_VAMP_DOMI] > 1)
            victim->power[DISC_VAMP_DOMI] = ch->power[DISC_VAMP_DOMI] / 2;
    }

}

void do_cloak(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->flag5, AFF5_CLOAKING))
    {
	send_to_char("You step out of the shadows.\n\r",ch);
	REMOVE_BIT(ch->affected_by, AFF_HIDE);
	REMOVE_BIT(ch->affected_by, AFF_SNEAK);
	REMOVE_BIT(ch->flag5, AFF5_CLOAKING);
        WAIT_STATE(ch, 8);
	return;
    }
 
    send_to_char("You slide into the shadows.\n\r",ch);
    SET_BIT(ch->affected_by, AFF_HIDE);
    SET_BIT(ch->affected_by, AFF_SNEAK);
    SET_BIT(ch->flag5, AFF5_CLOAKING);
    WAIT_STATE(ch, 8);
}

/* claw and fang commands */

void declaw(CHAR_DATA *ch)
{
    if (!IS_SET(ch->flag2, AFF_CLAW)) return;

    send_to_char("You retract your claws.\n\r",ch);
    act("$n retracts $s claws.",ch, NULL, NULL, TO_ROOM);
    REMOVE_BIT(ch->flag2, AFF_CLAW);
}

void dehorn(CHAR_DATA *ch)
{
    if (!IS_SET(ch->flag4, AFF_HORN)) return;

    send_to_char("Your horns burrow back into your skull.\n\r",ch);
    act("$n's horns retract into $s skull.",ch, NULL, NULL, TO_ROOM);
    REMOVE_BIT(ch->flag4, AFF_HORN);
}

void do_horns(CHAR_DATA *ch, char *argument)
{
    WAIT_STATE(ch, 8);
    if (IS_SET(ch->flag4, AFF_HORN)) dehorn(ch);
    else horn(ch);
}

void detail(CHAR_DATA *ch)
{
    if (!IS_SET(ch->flag2, AFF_TAIL)) return;

    send_to_char("Your tail retracts into the base of your spine.\n\r",ch);
    act("$n's tail retracts into $s body.",ch, NULL, NULL, TO_ROOM);
    REMOVE_BIT(ch->flag2, AFF_TAIL);
}

void do_tail(CHAR_DATA *ch, char *argument)
{
    WAIT_STATE(ch, 12);
    if (IS_SET(ch->flag2, AFF_TAIL)) detail(ch);
    else tail(ch);
}

void tail(CHAR_DATA *ch)
{
    OBJ_DATA *obj;
  
    if (IS_SET(ch->flag2, AFF_TAIL)) return;

    send_to_char("A long tail extends from your spine.\n\r",ch);
    act("A long spiny tail extends from $n's spine.",ch, NULL, NULL, TO_ROOM);

    if ( (obj = get_eq_char(ch, WEAR_LEGS)) != NULL) 
    { 
        act("Your tail shreds $p.", ch, obj, NULL, TO_CHAR); 
        act("$n's tail shreds $p.", ch, obj, NULL, TO_ROOM); 
        extract_obj(obj); 
    } 

    SET_BIT(ch->flag2, AFF_TAIL);
}

void horn(CHAR_DATA *ch)
{
    OBJ_DATA *obj;
  
    if (IS_SET(ch->flag4, AFF_HORN)) return;

    /* Only certain demons may use their horns */
    if (ch->power[DISC_DAEM_ATTA] < 4) return;

    send_to_char("Sharp horns grow out of your skull.\n\r",ch);
    act("Sharp horns grow out of $n's skull.",ch, NULL, NULL, TO_ROOM);

    if ( (obj = get_eq_char(ch, WEAR_HEAD)) != NULL) 
    { 
        act("Your horns shred $p.", ch, obj, NULL, TO_CHAR); 
        act("$n's horns shred $p.", ch, obj, NULL, TO_ROOM); 
        extract_obj(obj); 
    } 

    SET_BIT(ch->flag4, AFF_HORN);
}

void defang(CHAR_DATA *ch)
{
    if (!IS_SET(ch->flag2, AFF_BITE)) return;

    send_to_char("Your fangs slide back into your gums.\n\r",ch);
    act("$n retracts $s fangs back into $s gums.",ch, NULL, NULL, TO_ROOM);
    REMOVE_BIT(ch->flag2, AFF_BITE);
}

void do_claw(CHAR_DATA *ch, char *argument)
{
    if (ch->cur_form != 0 && !form_data[ch->cur_form].can_use_claws )
    {
	send_to_char("You cannot use your claws in this form.\n\r", ch);
	return;
    }

   if (IS_NEWFLAG(ch, AFF_CLAW))
   {
	if (ch->rage >0)
	{    
	    send_to_char("You cannot retract your claws while the beast is within you.\n\r", ch);
	    return;
	}
	send_to_char("You retract your claws.\n\r",ch);
	act("$n retracts $s claws.",ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->flag2, AFF_CLAW);
   }
   else
   {
	send_to_char("Wicked claws extend from your fingers.\n\r",ch);
	act("Long sharp claws extend from $n's fingers.",ch, NULL, NULL, TO_ROOM);
	remove_obj( ch, WEAR_RIGHT_HAND, TRUE );
	remove_obj( ch, WEAR_LEFT_HAND, TRUE );
	remove_obj( ch, WEAR_THIRD_HAND, TRUE );
	remove_obj( ch, WEAR_FOURTH_HAND, TRUE );
	SET_BIT(ch->flag2, AFF_CLAW);
   }
}

void do_fang(CHAR_DATA *ch, char *argument)
{
    if (ch->cur_form != 0 && !form_data[ch->cur_form].can_use_claws )
    {
	send_to_char("You cannot use your fangs in this form.\n\r", ch);
	return;
    }

   if (IS_NEWFLAG(ch, AFF_BITE))
   {
	if (ch->rage >0)
	{    
	    send_to_char("You cannot retract your fangs while the beast is within you.\n\r", ch);
	    return;
	}
	send_to_char("Your fangs slide back into your gums.\n\r",ch);
	act("$n retracts $s fangs back into $s gums.",ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->flag2, AFF_BITE);
   }
   else
   {
	send_to_char("Sharp fangs slide out of your gums.\n\r",ch);
	act("Sharp fangs slide out of $n's gums.",ch, NULL, NULL, TO_ROOM);
	SET_BIT(ch->flag2, AFF_BITE);
   }
}

void do_water_shield(CHAR_DATA *ch, char *argument)
{
   if (ch->generation != 3 && ch->generation != 4 )
   {
	send_to_char("Only the Noviere may use this ability.\n\r",ch);
	return;
   }

   if (IS_NEWFLAG(ch, AFF_WATER_SHIELD))
   {
	send_to_char("Your watery shield vanishes.\n\r",ch);
	act("$n's watery shield dissipates in a cloud of steam.",ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->flag2, AFF_WATER_SHIELD);
   }
   else
   {
	send_to_char("You surround yourself with a shield of water.\n\r",ch);
	act("$n surrounds $mself with a shimmering shield of water.",ch, NULL, NULL, TO_ROOM);
	SET_BIT(ch->flag2, AFF_WATER_SHIELD);
   }
}

void do_shield(CHAR_DATA *ch, char *argument)
{
   if (IS_NEWFLAG(ch, AFF_SHIELD_AURA))
   {
	send_to_char("You stop shielding your aura.\n\r",ch);
	change_upkeep( ch, -2);
	REMOVE_BIT(ch->flag2, AFF_SHIELD_AURA);
   }
   else
   {
	send_to_char("You shield your aura from those around you.\n\r",ch);
	change_upkeep( ch, 2);
	SET_BIT(ch->flag2, AFF_SHIELD_AURA);
   }
}

int count_eladrin(ROOM_INDEX_DATA *room)
{
    int eladrin = 0;
    CHAR_DATA *rch;

    for (rch = room->people ; rch != NULL ; rch = rch->next)
    {
	if (rch->secondaryrace == RACE_ELADRIN
	    && rch->tertiaryrace == 0) eladrin++;
    }

    return eladrin;
}


void do_melody(CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;

    if (argument[0] == '\0')
    {
	send_to_char("\n\rYou can sing the following songs:\n\r",ch);
	send_to_char("      March of the Brave\n\r",ch);
	if (ch->power[DISC_ELAD_MELO] >= 2)
		send_to_char("      Maiden of Lothariel\n\r",ch);
	if (ch->power[DISC_ELAD_MELO] >= 3)
		send_to_char("      Lament of Prince Asila\n\r",ch);
	if (ch->power[DISC_ELAD_MELO] >= 4)
		send_to_char("      Soothing the soul\n\r",ch);
	if (ch->power[DISC_ELAD_MELO] >= 5)
		send_to_char("      Vengence of the Elves\n\r",ch);
	if (ch->power[DISC_ELAD_MELO] >= 7)
		send_to_char("      Ambush at Salius\n\r",ch);
	if (ch->power[DISC_ELAD_MELO] >= 9)
		send_to_char("      Becoming of the Land\n\r",ch);
	if (ch->power[DISC_ELAD_MELO] >= 10)
		send_to_char("      Storm over Arborea\n\r",ch);
	return;
    }

    if (!str_prefix(argument, "march of the brave" ))
    {
	if (is_affected(ch, skill_lookup("bless")))
	{
	    send_to_char("You have already been blessed.\n\r",ch);
	    return;
	}
	act("$n sings a loud marching chant.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The music floods your soul with courage and valor.\n\r", ch);

 	if (!IS_NPC(ch) && dice(1, 10) > ch->pcdata->faeriepath[2])
	    gain_elad_path(ch, 2);

	WAIT_STATE(ch, 12);
	af.where        = TO_AFFECTS;
	af.type         = skill_lookup("bless");
	af.level        = ch->level;
	af.duration     = ch->generation+6;
	af.location     = APPLY_HITROLL;
	af.modifier     = ch->generation+5;
	af.bitvector    = 0;
	affect_to_char(ch, &af);

	af.location     = APPLY_DAMROLL;
	af.modifier     = ch->generation+5;
	af.name 	= "march of the brave";
	af.end_message  = "You feel less brave";
	affect_name_to_char(ch, &af);
	return;
    }

    if (ch->power[DISC_ELAD_MELO] > 1 && !str_prefix(argument, "maiden of lothariel" ))
    {
	WAIT_STATE(ch, 22);
	send_to_char("The music floods your soul with the essence of Lothariel.\n\r", ch);
	act("$n serenades you with a beautiful lullaby.",ch,NULL,NULL,TO_ROOM);

 	if (!IS_NPC(ch) && dice(1, 20) > ch->pcdata->faeriepath[2])
	    gain_elad_path(ch, 2);

	ch->hit += dice (ch->generation , 10) + 20;
	if (ch->hit > ch->max_hit) ch->hit = ch->max_hit;
	return;
    }

    if (ch->power[DISC_ELAD_MELO] > 2 && !str_prefix(argument, "lament of prince asila" ))
    {
	send_to_char("The music infuses you with a great vitality, you feel better.\n\r", ch);
	act("$n sings a beautiful lullaby.",ch,NULL,NULL,TO_ROOM);

 	if (!IS_NPC(ch) && dice(1, 35) > ch->pcdata->faeriepath[2])
	    gain_elad_path(ch, 2);

	affect_strip(ch, gsn_poison);
	affect_strip(ch, gsn_plague);
	affect_strip(ch, gsn_blood_spew);
	affect_strip(ch, gsn_mummy_rot);
	WAIT_STATE(ch, 12);
	return;
    }

    if (ch->power[DISC_ELAD_MELO] >= 5 && !str_prefix(argument, "vengeance of the elves" ))
    {
	CHAR_DATA *rch;
	bool fiend = FALSE;

        if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
        {
            stc("You may not use this song while within a safe room.\r\n", ch);
            return;
        }

	WAIT_STATE(ch, 12);
	act("$n starts to sing in elvish.", ch, NULL, NULL, TO_ROOM);
	stc("You start to sing a beautiful melody in elvish.\n\r", ch);

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
	    if (rch->altitude != ch->altitude) continue;
	    if (rch == ch) continue;

	    if (rch->secondaryrace == RACE_DAEMON &&
		check_immune(rch, DAM_SOUND) != IS_IMMUNE)
	    {
		fiend = TRUE;	 	
		act("$n's beautiful singing wrenches your tainted soul, crippling you.",ch,NULL,rch,TO_VICT);
		act("$n tries to block $s ears with the pain.", rch, NULL, NULL, TO_ROOM);
		if (dice(1, 3) == 1)
	 	{
		    stc("You fall to the floor in agony.\n\r", rch);
		    rch->position = POS_SITTING;
		}
		WAIT_STATE(rch, 15);
	    }
	}
	if (fiend)
	   stc("Your beautiful singing cripples all the abyssial fiends around you.\n\r",ch);
	else
	   stc("There was nobody in the room to affect.\n\r", ch);
	
 	if (!IS_NPC(ch) && dice(1, 55) > ch->pcdata->faeriepath[2])
	    gain_elad_path(ch, 2);

	return;
    }

    if (ch->power[DISC_ELAD_MELO] >= 4 && !str_prefix(argument, "soothing of the soul"))
    {
        if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
        {
            stc("You may not use this song while within a safe room.\r\n", ch);
            return;
        }

	WAIT_STATE(ch, 12);

	stc("Your singing soothes your soul.\n\r", ch);
	act("$n sings a quiet soothing song.", ch, NULL, NULL, TO_ROOM);

	unwerewolf(ch);
	defang(ch);
	declaw(ch);
	detail(ch);
	dehorn(ch);
	ch->beast = 0;
	REMOVE_BIT(ch->flag5, AFF5_BEAST);
	ch->rage = 0;
	affect_strip(ch, gsn_fear);

 	if (!IS_NPC(ch) && dice(1, 45) > ch->pcdata->faeriepath[2])
	    gain_elad_path(ch, 2);

	return;
    }

    if (ch->power[DISC_ELAD_MELO] >= 9 && !str_prefix(argument, "becoming of the land"))
    {
	WAIT_STATE(ch, 12);

	if (count_eladrin(ch->in_room) < 2)
	{
	    stc("There must be two eladrin in the room to sing this song.\n\r", ch);
	    return;
	}

 	act("$n starts to sing a chilling melody.", ch, NULL, NULL, TO_ROOM);
	act("$n suddenly vanishes from sight.", ch, NULL, NULL, TO_ROOM);

  	char_from_room(ch);
	char_to_room(ch, get_rand_room_by_sect(SECT_FOREST), 0);
	
	do_look(ch, "auto");
	stc("The song has transported you elsewhere.\n\r", ch);

 	if (!IS_NPC(ch) && dice(1, 95) > ch->pcdata->faeriepath[2])
	    gain_elad_path(ch, 2);

	return;
    }

    if (ch->power[DISC_ELAD_MELO] >= 7 && !str_prefix(argument, "ambush at salius" ))
    {
	CHAR_DATA *rch;
	AFFECT_DATA af;
	bool fiend = FALSE;

        if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
        {
            stc("You may not use this song while within a safe room.\r\n", ch);
            return;
        }

	WAIT_STATE(ch, 12);

	act("$n sings a haunting melody.", ch, NULL, NULL, TO_ROOM);
	stc("You start to sing a haunting melody.\n\r", ch);

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
	    if (rch->altitude != ch->altitude) continue;
	    if (rch == ch) continue;

	    if (rch->alignment < -500 && rch->level < ch->level + 10)
	    {
	        if (!IS_NEWFLAG(rch, AFF_FEAR) && dice(1, 3) == 1 ) 
	        {
		    af.where = TO_NEWFLAGS;
		    af.type  = skill_lookup("fear");
		    af.level = ch->level;
		    af.level = 1;
		    af.location = APPLY_NONE;
		    af.modifier = 0;
		    af.duration = 2;
		    af.bitvector = AFF_FEAR;
		    af.name = "ambush at salius";
		    af.end_message = "You feel less afraid";
		    affect_name_to_char( rch, &af );
		    fiend = TRUE;
		    act("$n's melody makes you flee in panic.",ch,NULL,rch,TO_VICT);
	        }
	        else
	        {
		    damage_old(ch, rch, dice(ch->generation, 30), 0, DAM_OTHER,TRUE);
		    act("$n's song bursts your eardrums, the pain is unbearable.",ch,NULL,rch,TO_VICT);
		    fiend = TRUE;
	        }
	    }
	}
	if (fiend)
	    stc("Your beautiful singing cripples all the evil around you.\n\r",ch);
	else stc("Nothing seemed to happen.\n\r", ch);

 	if (!IS_NPC(ch) && dice(1, 75) > ch->pcdata->faeriepath[2])
	    gain_elad_path(ch, 2);

	return;
    }

    do_melody(ch, "");
    return;
}

void do_artificia(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    if (ch->power[DISC_ELAD_ARTI] < 1)
    {
	send_to_char("You have not yet gained mastery of this discipline\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("You can make the following items:\n\r",ch);
	send_to_char("    Circlet of Elvenkind   -  5 faith\n\r",ch);

	if (ch->power[DISC_ELAD_ARTI] >= 2)
	    send_to_char("    Boots of Silence       -  10 faith\n\r",ch);

	if (ch->power[DISC_ELAD_ARTI] >= 3)
	    send_to_char("    Cloak of Invisibility  -  15 faith\n\r",ch);

	if (ch->power[DISC_ELAD_ARTI] >= 5)
	    send_to_char("    Elven Chainmail        -  20 faith\n\r",ch);

	if (ch->power[DISC_ELAD_ARTI] >= 7)
	    send_to_char("    Ring of Deception      -  25 faith\n\r",ch);

	if (ch->power[DISC_ELAD_ARTI] >= 8)
	    send_to_char("    Sword of the Elflord   -  30 faith\n\r",ch);

	if (ch->power[DISC_ELAD_ARTI] >= 9)
	    send_to_char("    Shield of Oak Leaves   -  40 faith\n\r",ch);
	return;
    }

    if (!str_prefix(argument, "circlet of elvenkind"))
    {
	if (ch->faith < 5)
	{
	    send_to_char("You do not have enough faith to create this item.\n\r",ch);
	    return;
	}
	ch->faith -= 5;
	obj = create_object( get_obj_index(32), 0 );
	obj_to_char(obj, ch);
	send_to_char("You pray to the gods, and your faith is rewarded with an ancient relic.\n\r", ch);
	act("$n gets down on $s knees and starts to pray and chant in ancient elvish.", ch, NULL, NULL, TO_ROOM);
	act("A glowing circlet slowly descends from the heavens.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (!str_prefix(argument, "boots of silence") && ch->power[DISC_ELAD_ARTI] >= 2)
    {
	if (ch->faith < 10)
	{
	    send_to_char("You do not have enough faith to create this item.\n\r",ch);
	    return;
	}
	ch->faith -= 10;
	obj = create_object( get_obj_index(33), 0 );
	obj_to_char(obj, ch);
	send_to_char("You pray to the gods, and your faith is rewarded with an ancient relic.\n\r", ch);
	act("$n gets down on $s knees and starts to pray and chant in ancient elvish.", ch, NULL, NULL, TO_ROOM);
	act("An ancient relic slowly descends from the heavens.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (!str_prefix(argument, "cloak of invisibility") && ch->power[DISC_ELAD_ARTI] >= 3)
    {
	if (ch->faith < 15)
	{
	    send_to_char("You do not have enough faith to create this item.\n\r",ch);
	    return;
	}
	ch->faith -= 15;
	obj = create_object( get_obj_index(34), 0 );
	obj_to_char(obj, ch);
	send_to_char("You pray to the gods, and your faith is rewarded with an ancient relic.\n\r", ch);
	act("$n gets down on $s knees and starts to pray and chant in ancient elvish.", ch, NULL, NULL, TO_ROOM);
	act("A glowing relic slowly descends from the heavens.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (!str_prefix(argument, "elven chainmail") && ch->power[DISC_ELAD_ARTI] >= 5)
    {
	if (ch->faith < 20)
	{
	    send_to_char("You do not have enough faith to create this item.\n\r",ch);
	    return;
	}
	ch->faith -= 20;
	obj = create_object( get_obj_index(35), 0 );
	obj_to_char(obj, ch);
	send_to_char("You pray to the gods, and your faith is rewarded with an ancient relic.\n\r", ch);
	act("$n gets down on $s knees and starts to pray and chant in ancient elvish.", ch, NULL, NULL, TO_ROOM);
	act("A glowing relic slowly descends from the heavens.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (!str_prefix(argument, "ring of deception") && ch->power[DISC_ELAD_ARTI] >= 7)
    {
	if (ch->faith < 25)
	{
	    send_to_char("You do not have enough faith to create this item.\n\r",ch);
	    return;
	}
	ch->faith -= 25;
	obj = create_object( get_obj_index(36), 0 );
	obj_to_char(obj, ch);
	send_to_char("You pray to the gods, and your faith is rewarded with an ancient relic.\n\r", ch);
	act("$n gets down on $s knees and starts to pray and chant in ancient elvish.", ch, NULL, NULL, TO_ROOM);
	act("A glowing ring slowly descends from the heavens.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (!str_prefix(argument, "sword of the elflord") && ch->power[DISC_ELAD_ARTI] >= 8)
    {
	if (ch->faith < 30)
	{
	    send_to_char("You do not have enough faith to create this item.\n\r",ch);
	    return;
	}
	ch->faith -= 30;
	obj = create_object( get_obj_index(37), 0 );
	obj_to_char(obj, ch);
	send_to_char("You pray to the gods, and your faith is rewarded with an ancient relic.\n\r", ch);
	act("$n gets down on $s knees and starts to pray and chant in ancient elvish.", ch, NULL, NULL, TO_ROOM);
	act("A glowing sword slowly descends from the heavens.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (!str_prefix(argument, "shield of oak leaves") && ch->power[DISC_ELAD_ARTI] >= 9)
    {
	if (ch->faith < 40)
	{
	    send_to_char("You do not have enough faith to create this item.\n\r",ch);
	    return;
	}
	ch->faith -= 40;
	obj = create_object( get_obj_index(38), 0 );
	obj_to_char(obj, ch);
	send_to_char("You pray to the gods, and your faith is rewarded with an ancient relic.\n\r", ch);
	act("$n gets down on $s knees and starts to pray and chant in ancient elvish.", ch, NULL, NULL, TO_ROOM);
	act("A glowing shield slowly descends from the heavens.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    do_artificia(ch, "");
}

void do_fear(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *ich;
    AFFECT_DATA af;

    WAIT_STATE(ch, 12);

    send_to_char("Your very presence radiates fear to all those around you.\n\r", ch);
    act("You panic as the very presence of $n fills you with awe.", ch, NULL, NULL,TO_ROOM);

    for (ich = ch->in_room->people; ich!=NULL; ich = ich->next_in_room)
    {
	if (ich->altitude < ch->altitude - 1
	&& ich->altitude > ch->altitude + 1) continue;

	if (ich == ch) continue;
	if (is_safe(ich, ch)) continue;

	switch( ch->power[DISC_DRAG_AWE] )
	{
	    case 1 : if (ich->level < 10)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_FEAR;
			affect_to_char(ich, &af);
		     }
		     break;
	    case 2 : if (ich->level < 20 || (!IS_NPC(ich) && ich->secondaryrace == 0))
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_FEAR;
			affect_to_char(ich, &af);
		     }
		     break;
	    case 3: if (ich->level < 30)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_FEAR;
			affect_to_char(ich, &af);
		     }
		     break;
	    case 4 : if (ich->level < 20)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("hold person");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_HOLD;
			affect_to_char(ich, &af);
		     }
		     else if (ich->level < 40)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_FEAR;
			affect_to_char(ich, &af);
		     } 
		     else if (ich->level < 60)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_SHIVER;
			affect_to_char(ich, &af);
		     }
		     break;
	    case 5 : if (ich->level < 10 && IS_NPC(ich))
		     {
			act("Fear strikes your heart as $N rears $s colossal body.",ich,NULL,ch,TO_CHAR);
			damage_old(ch, ich, 10000, skill_lookup("fear"), DAM_OTHER, FALSE);
		     }
		     else if (ich->level < 40)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("hold person");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_HOLD;
			affect_to_char(ich, &af);
		     }
		     else if (ich->level < 60)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_FEAR;
			affect_to_char(ich, &af);
		     } 
		     else if (ich->level < 80)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_SHIVER;
			affect_to_char(ich, &af);
		     }
		     break;
	    default: bug("Bug, discipline outside bounds: Dragonfear",0);
		     break;
	}
    }
}

void do_sphinx_roar(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *ich;
    AFFECT_DATA af;
    char buf[100];

    WAIT_STATE(ch, 12);

    if (!TIME_UP(ch, TIMER_SPHINX_ROAR))
    {
	sprintf(buf, "You cannot use this ability for another %d ticks.\n\r",
		TIMER(ch, TIMER_SPHINX_ROAR) );
	stc(buf, ch);
	return;
    }

    SET_TIMER(ch, TIMER_SPHINX_ROAR, dice(2, 4) );
    send_to_char("You issue forth a mighty roar.\n\r", ch);
    act("$n issues forth a mighty roar.", ch, NULL, NULL,TO_ROOM);

    for (ich = ch->in_room->people; ich!=NULL; ich = ich->next_in_room)
    {
	if (ich->altitude < ch->altitude - 1
	&& ich->altitude > ch->altitude + 1) continue;

	if (ich == ch) continue;
	if (is_safe(ich, ch)) continue;

	switch( ch->power[DISC_VAMP_SATY] )
	{
	    case 6 : if (ich->level < 10)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_FEAR;
			affect_to_char(ich, &af);
		     }
		     break;
	    case 7 : if (ich->level < 20 || (!IS_NPC(ich) && ich->secondaryrace == 0))
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_FEAR;
			affect_to_char(ich, &af);
		     }
		     break;
	    case 8 : if (ich->level < 30)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_FEAR;
			affect_to_char(ich, &af);
		     }
		     break;
	    case 9 : if (ich->level < 20)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("hold person");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_HOLD;
			affect_to_char(ich, &af);
		     }
		     else if (ich->level < 40)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_FEAR;
			affect_to_char(ich, &af);
		     } 
		     else if (ich->level < 60)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_SHIVER;
			affect_to_char(ich, &af);
		     }
		     break;
	    case 10 : if (ich->level < 10 && IS_NPC(ich))
		     {
			act("Fear strikes your heart as $N gives a mighty roar.",ich,NULL,ch,TO_CHAR);
			damage_old(ch, ich, 10000, skill_lookup("fear"), DAM_OTHER, FALSE);
		     }
		     else if (ich->level < 40)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("hold person");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_HOLD;
			affect_to_char(ich, &af);
		     }
		     else if (ich->level < 60)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_FEAR;
			affect_to_char(ich, &af);
		     } 
		     else if (ich->level < 80)
		     {
			af.where = TO_NEWFLAGS;
			af.type  = skill_lookup("fear");
			af.level = ch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = -2;
			af.bitvector = AFF_SHIVER;
			affect_to_char(ich, &af);
		     }
		     break;
	    default: bug("Bug, discipline outside bounds: Sphinxroar",0);
		     break;
	}
    }
}

void do_shadowsight(CHAR_DATA *ch, char *argument)
{
    if (ch->power[DISC_VAMP_AUSP] < 3)
    {
	send_to_char("You have not yet gained mastery of this ability.\n\r",ch);
	return;
    }

    if (IS_NEWFLAG(ch, AFF_SHADOW_SIGHT))
    {
	send_to_char("You no longer see into the demiplane of shadows.\n\r",ch);
	REMOVE_BIT(ch->flag2,AFF_SHADOW_SIGHT);
	change_upkeep(ch, -5);
	return;
    }

    SET_BIT(ch->flag2, AFF_SHADOW_SIGHT);
    send_to_char("You now have an increased awareness of the shadows around you.\n\r",ch);
    change_upkeep(ch, 5);
}
void do_shadowwalk(CHAR_DATA *ch, char *argument)
{
    if (ch->power[DISC_VAMP_OBFU] < 4)
    {
	send_to_char("You have not yet gained mastery of this ability.\n\r",ch);
	return;
    }

    if (IS_NEWFLAG(ch, AFF_SHADOW_PLANE))
    {
	send_to_char("You reform from the shadows and emerge into the room.\n\r",ch);
	REMOVE_BIT(ch->flag2,AFF_SHADOW_PLANE);
	act("$n reforms from the shadows and emerges into the room.",ch,NULL,NULL,TO_ROOM);
	do_look(ch,"");
	change_upkeep(ch, -5);
	return;
    }

    if (ch->cur_form == FRM_MIST && !TIME_UP(ch, TIMER_WRENCHED) )
    {
        send_to_char("You have been wrenched from the negative plane and may not do that.\n\r", ch);
	return;
    }

    act("$n slides into the shadows, and disappears from view.",ch,NULL,NULL,TO_ROOM);
    SET_BIT(ch->flag2, AFF_SHADOW_PLANE);
    send_to_char("You slide into the shadows, as you shift your plane of existance.\n\r",ch);
    do_look(ch,"");
    change_upkeep(ch, 5);
}

char * const style_name[8] =
{
    "Cat", "Puma", "Pixie", "Hornet", "Boar", "Gorgon", "Wyvern", "Owl"
};

void do_style(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int loop, match,exp;

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if (arg[0] == '\0' || str_prefix(arg2, "train"))
    {
	send_to_char("Syntax:  style <style>\n\r",ch);
	send_to_char("Syntax:  style <style> train\n\r",ch);
	send_to_char("Styles:  Cat, Puma, Pixie, Hornet, Boar, Gorgon, Wyvern, or Owl.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "list") || !str_prefix(arg,"summary"))
    {
	divide4_to_char(ch);
	cent_to_char("Your Style Skills Are",ch);
	divide3_to_char(ch);
	for (loop = 0; loop < 8; loop++)
	{
	    sprintf(buf, "    %-30s  :  %d\n\r",style_name[loop],ch->style_skill[loop]);
	    send_to_char(buf,ch);
	}
	divide2_to_char(ch);
	sprintf(buf,"You currently have %ld experience.",ch->currentexp);
	cent_to_char(buf,ch);
	divide4_to_char(ch);
	return;
    }

    match = -1;

    for (loop = 0; loop < 8; loop++)
    {
	if (!str_prefix(arg, style_name[loop]))
		match = loop;
    }

    if (match < 0)
    {
	send_to_char("That is not a valid fighting style.\n\r",ch);
	send_to_char("Styles:  Cat, Puma, Pixie, Hornet, Boar, Gorgon, Wyvern, or Owl.\n\r",ch);
	return;
    }

    if (!str_prefix(arg2,"train") && arg2[0]!='\0')
    {
	if (ch->style_skill[match] == 20)
	{
	    send_to_char("You sad sad sad sad sad little tit!  You really trained this to max?\n\r",ch);
	    return;
	}
	exp = 1000;
	for (loop = 1; loop <= ch->style_skill[match]; loop++)
	{
	    exp = exp*2;
	}
	if (ch->currentexp < exp)
	{
	    send_to_char("You do not have enough experience to improve that fighting style.\n\r",ch);
	    return;
	}
	ch->currentexp -= exp;
	ch->style_skill[match] += 1;
	send_to_char("Your skill in the ",ch);
	send_to_char(style_name[match],ch);
	send_to_char(" improves.\n\r",ch);
	return;
    }
    divide3_to_char(ch);
    sprintf(buf,"Your skill in the %s fighting style is : %d\n",
	style_name[match], ch->style_skill[match]);
    cent_to_char(buf,ch);
	exp = 1000;
	for (loop = 1; loop <= ch->style_skill[match]; loop++)
	{
	    exp = exp*2;
	}
    sprintf(buf,"You need %d exp to improve it further.",exp);
    cent_to_char(buf,ch);
    sprintf(buf,"You currently have %ld exp.",ch->currentexp);
    cent_to_char(buf,ch);
    divide3_to_char(ch);
}

void do_mask(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;
    char buf[MAX_STRING_LENGTH];

    if (ch->power[DISC_VAMP_OBFU] < 5)
    {
	send_to_char("You have not yet gained mastery of this discipline.\n\r",ch);
	return;
    }

    if (ch->cur_form != 0)
    {
	send_to_char("You canoot mask while in another form.\n\r", ch);
	return;
    }

    if (IS_NEWFLAG(ch,AFF_SHAPE_SHIFTED))
    {
	send_to_char("You cannot use this ability while shape shifted.\n\r",ch);
	return;
    }

    if (argument[0]=='\0' || !str_cmp(argument,"self"))
    {
	if (!IS_NEWFLAG(ch, AFF_MASKED))
	{
	    send_to_char("Your form blurs and shimmers slightly.\n\r",ch);
	    act("$n's form blurs and shimmers slightly.",ch,NULL,NULL,TO_ROOM);
	    return;
	}

	sprintf(buf,"%s's form blurs and shifts to resemble %s.",
		ch->name,
		ch->pcdata->switchname);
	set_pc_name(ch,ch->pcdata->switchname);
	act(buf,ch,NULL,NULL,TO_ROOM);
	REMOVE_BIT(ch->flag2,AFF_MASKED);
	send_to_char("Your form blurs and shifts back to normal.\n\r",ch);
	look_name(ch);
	change_upkeep(ch, -4);
    }

    if ((vch = get_char_room(ch,argument))==NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_NPC(vch))
    {
	send_to_char("You can only mask as players.\n\r", ch);
	return;
    }


	if (vch != ch)
	{
	    if (!str_prefix(argument,vch->name))
	    {
		SET_BIT(ch->flag2, AFF_MASKED);
		sprintf(buf, "%s's form blurs and shifts to resemble %s.",
			ch->name,
			GET_NAME(vch));
		act(buf,ch,NULL,NULL,TO_ROOM);
		set_pc_name(ch, GET_NAME(vch));
		act("Your form blurs and shifts to resemble $n.",ch,NULL,NULL,TO_CHAR);
	 	set_form(ch, 0);
		change_upkeep(ch, 4);
		set_enter_room_text(ch, GET_EXTENSION(vch));
		return;
	    }
	}
}

void do_escape(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (ch->hit > 0)
    {
	send_to_char("You have to be mortally wounded for your deity to rescue you.\n\r",ch);
	return;
    }

    if (ch->in_room->vnum == ROOM_VNUM_TEMPLE)
    {
	send_to_char("You have already escaped from battle.\n\r", ch);
	return;
    }

    do_goto(ch, "3001");
    WAIT_STATE(ch, 8);
    sprintf(buf, "%s has escaped defenseless from a fight.",ch->name);
    informational(buf, -1, 0);
}

void do_beg(CHAR_DATA *ch, char *argument)
/* REMEMBER TO UPDATE VALUE 5 - NUMBER OF TIMES BEGGED
	AND TO ACTUALLY USE THIS AS A RANDOM ELEMENT 
	
      THE NEWB VARIABLE SHOWS THAT NICE OUTCOMES SHOULD ALWAYS  
       BE PUT IN THE FIRST 2 OUT OF 6 SLOTS, TO HELP NEWBIES    */

{
    ROOM_INDEX_DATA *location=NULL;
    int newb=0;

    if (argument[0] == '\0')
    {
	send_to_char("You can beg for:   safety or health\n\r",ch);
	return;
    }
    if (ch->value[5] < 8) newb = 2;
    else newb = 6;

    if (!str_prefix(argument,"safety"))
    {
	ch->value[5] += 1;
	switch (dice(1,newb))
	{
	case 1 : location = get_room_index( prime_realm, ROOM_VNUM_START );
		 if (!IS_SET(ch->in_room->room_flags,ROOM_NO_RECALL) ||
			IS_AFFECTED(ch, AFF_CURSE))
		 {
		     send_to_char("A green aura starts to form around you, but quickly dissipates.\n\r",ch);
		     act("A green aura starts to form around $n, but quickly dissipates.",ch,NULL,NULL,TO_ROOM);
		     return;
		 }
	case 2 : if (ch->fighting!=NULL)
			stop_fighting(ch,TRUE);
		 location = get_room_index( prime_realm, 3001 );
		 act("A green aura surrounds $n, and $e vanishes from sight.",ch,NULL,NULL,TO_ROOM);
		 char_from_room( ch );
		 char_to_room( ch, location, 0 );
		 act("$n appears in a flash of brilliant green light.",ch,NULL,NULL,TO_ROOM);
		 do_look(ch,"auto");
		 break;
	case 3 : send_to_char("You are struck down by a lightning bolt from the heavens.\n\r",ch);
		 act("$n is struck down by a lighting bolt from the heavens.",ch,NULL,NULL,TO_ROOM);
		 damage(ch, ch, 300, 0, DAM_LIGHTNING, FALSE);
		 if (ch->hit < 30) ch->hit = 30;
		 send_to_char("You are crippled as the electricity pulses through your body.\n\r",ch);
		 break;
	}
    }
    else if (!str_prefix(argument,"health"))
    {
	ch->value[5] += 2;
	switch(dice(1,newb))
	{
	case 1:
	send_to_char("The gods smile on you this once, granting you health.\n\r", ch);
	ch->hit = UMIN(ch->hit + dice(1, 50), ch->max_hit);
	break;
	case 2:
	case 3:
	case 4:
	send_to_char("Your prayers are unanswered.\n\r", ch);
	break;
	case 5:
	send_to_char("A chill wind blows through the room.  You feel drained.\n\r", ch);
	damage_old(ch, ch, dice(10, 10), 0, DAM_NEGATIVE, FALSE);
	break;
	case 6:
	send_to_char("A chill wind blows through the room.  You feel totally drained.\n\r", ch);
	damage_old(ch, ch, dice(20, 20), 0, DAM_NEGATIVE, FALSE);
	break;
        }
    }
    else
	do_beg(ch,"");
}

void do_tutor(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int eduneed;
    int chosen = -1,i;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (IS_NPC(ch)) return;

    if (!str_cmp(arg1, "restrict"))
    {
	if (IS_SET(ch->flag4, AFF_NO_TUTOR) )
	{
	    send_to_char("You are already restricting your learning.\n\r", ch);
	    return;
	}
	send_to_char("You now cannot be tutored by anybody.\n\r", ch);
	SET_BIT(ch->flag4, AFF_NO_TUTOR);
	return;
    }

    if (!str_cmp(arg1, "allow"))
    {
	if (!IS_SET(ch->flag4, AFF_NO_TUTOR) )
	{
	    send_to_char("You are already allowing people to tutor you.\n\r", ch);
	    return;
	}
	send_to_char("You can now be tutored.\n\r", ch);
	REMOVE_BIT(ch->flag4, AFF_NO_TUTOR);
	return;
    }

    if (arg1[0]=='\0' || arg2[0]=='\0')
    {
	send_to_char("Syntax : tutor <victim> <discipline>\n\r",ch);
	send_to_char("         tutor [restrict allow]\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg1)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("You may only teach disciplines to players.\n\r",ch);
	return;
    }

    if (ch->secondaryrace != victim->secondaryrace
	&& ch->level != 120)
    {
	send_to_char("You are not of the same species.\n\r", ch);
	return;
    }

    for (i = 1; i < MAX_DISCIPLINES ; i++)
    {
	if (!str_prefix(arg2, discipline[i]))
		chosen = i;
    }

    if (chosen == -1)
    {
	send_to_char("You know no such discipline.\n\r",ch);
	return;
    }

    if (ch->power[chosen] < 3)
    {
	send_to_char("You must attain level three in a discipline before you can teach it.\n\r", ch);
	return;
    }

    eduneed = disciplines_known(victim) - 2;
    if (eduneed < 1) eduneed = 1;

    reset_learnable_disciplines(victim, victim->secondaryrace);

    if (victim->power[chosen] == -2 && ch->level < 120)
    {
	send_to_char("They cannot learn that discipline.\n\r", ch);
	return;
    }

    if (ch->pcdata->education < eduneed)
    {
	stcf(ch, "You need %d more edu points to tutor them anything.\n\r", eduneed);
	return;
    }

    if (victim->pcdata->education < eduneed * 3)
    {
	act("$N cannot learn any more disciplines.", ch, NULL, victim, TO_CHAR);
	sprintf(buf, "$n tries to teach you, but you need %d education points.", eduneed);
	act(buf, ch, NULL, victim, TO_VICT);
	return;
    }

    if (IS_SET(victim->flag4, AFF_NO_TUTOR))
    {
	act("$n trys to teach you a new discipline.", ch, NULL,victim, TO_VICT);
	do_say(victim, "I don't want to learn it!");
	return;
    }	

    WAIT_STATE(ch, 8);
    
    if (victim->power[chosen] > -1)
    {
	sprintf(buf, "$n is already trained in %s.", discipline[chosen]);
	act(buf, victim, NULL, ch, TO_VICT);
	sprintf(buf, "$n tries to tutor you in %s, but you already know it.", discipline[chosen]);
        act(buf,ch, NULL, victim, TO_VICT);
 	return;   
    }


    WAIT_STATE(victim, 16); 
    victim->power[chosen] = 0;
    victim->pcdata->education -= eduneed * 3;
    ch->pcdata->education -= eduneed;
    sprintf(buf, "You tutor $n in the discipline of %s.", discipline[chosen]);
    act(buf, victim, NULL, ch, TO_VICT);
    sprintf(buf, "$n tutors you in the discipline of %s.", discipline[chosen]);
    act(buf, ch, NULL, victim, TO_VICT);
}

/* random room generation procedure */
ROOM_INDEX_DATA  *get_random_room(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *room;

    for ( ; ; )
    {
        room = get_room_index( char_realm(ch), number_range( 0, 65535 ) );
        if ( room != NULL )
        if ( can_see_room(ch,room)
        &&   !IS_SET(room->room_flags, ROOM_PRIVATE)
        &&   !IS_SET(room->room_flags, ROOM_SOLITARY) 
	&&   !IS_SET(room->room_flags, ROOM_SAFE) )
            break;
    }

    return room;
}

ROOM_INDEX_DATA  *get_rand_room()
{
    ROOM_INDEX_DATA *room;

    for ( ; ; )
    {
        room = get_room_index( prime_realm, number_range( 0, 65535 ) );
        if ( room != NULL )
        if ( !IS_SET(room->room_flags, ROOM_PRIVATE)
        &&   !IS_SET(room->room_flags, ROOM_SOLITARY) 
	&&   !IS_SET(room->room_flags, ROOM_SAFE) 
	&&  room->vnum > 300)
            break;
    }

    return room;
}

ROOM_INDEX_DATA  *get_treemeld_room()
{
    ROOM_INDEX_DATA *room;

    for ( ; ; )
    {
        room = get_room_index( get_realm("Arborea"), number_range( 0, 65535 ) );
        if ( room != NULL
	    && room->sector_type == SECT_FOREST ) break;
    }

    return room;
}

ROOM_INDEX_DATA  *get_rand_room_by_sect( int sect )
{
    ROOM_INDEX_DATA *room;

    for ( ; ; )
    {
        room = get_room_index( NULL, number_range( 0, 65535 ) );
        if ( room != NULL
	    && room->sector_type == sect ) break;
    }

    return room;
}

void do_enter( CHAR_DATA *ch, char *argument)
{    
    ROOM_INDEX_DATA *location; 

    if ( ch->fighting != NULL ) 
	return;

    if ( IS_NEWFLAG(ch, AFF_HOLD) )
    {
	send_to_char("Your feet are held fast!\n\r",ch);
	return;
    }

    /* nifty portal stuff */
    if (argument[0] != '\0')
    {
        ROOM_INDEX_DATA *old_room;
	OBJ_DATA *portal;
	CHAR_DATA *fch, *fch_next;

        old_room = ch->in_room;

	portal = get_obj_list( ch, argument,  ch->in_room->contents );
	
	if (portal == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}

	if (portal->item_type != ITEM_PORTAL 
        ||  (IS_SET(portal->value[1],EX_CLOSED) && !IS_TRUSTED(ch,ANGEL)))
	{
	    send_to_char("You can't seem to find a way in.\n\r",ch);
	    return;
	}

	if (!IS_TRUSTED(ch,ANGEL) && !IS_SET(portal->value[2],GATE_NOCURSE)
	&&  (IS_AFFECTED(ch,AFF_CURSE) 
	||   IS_SET(old_room->room_flags,ROOM_NO_RECALL)))
	{
	    send_to_char("Something prevents you from leaving...\n\r",ch);
	    return;
	}

	if (IS_SET(portal->value[2],GATE_RANDOM) || portal->value[3] == -1)
	{
	    location = get_random_room(ch);
	    portal->value[3] = location->vnum; /* for record keeping :) */
	}
	else if (IS_SET(portal->value[2],GATE_BUGGY) && (number_percent() < 5))
	    location = get_random_room(ch);
	else
	    location = get_room_index(portal->in_room->area->realm, portal->value[3]);

	if (location == NULL
	||  location == old_room
	||  !can_see_room(ch,location) 
	||  (room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR)))
	{
	   act("$p doesn't seem to go anywhere.",ch,portal,NULL,TO_CHAR);
	   return;
	}

	act("$n steps into $p.",ch,portal,NULL,TO_ROOM);
	
	if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
	    act("You enter $p.",ch,portal,NULL,TO_CHAR);
	else
	    act("You walk through $p and find yourself somewhere else...",
	        ch,portal,NULL,TO_CHAR); 

	char_from_room(ch);
	char_to_room(ch, location, ch->altitude);

	if (IS_SET(portal->value[2],GATE_GOWITH)) /* take the gate along */
	{
	    obj_from_room(portal);
	    obj_to_room(portal,location, 0);
	}

	if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
	    act("$n has arrived.",ch,portal,NULL,TO_ROOM);
	else
	    act("$n has arrived through $p.",ch,portal,NULL,TO_ROOM);

	do_look(ch,"auto");

	/* charges */
	if (portal->value[0] > 0)
	{
	    portal->value[0]--;
	    if (portal->value[0] == 0)
		portal->value[0] = -1;
	}

	/* protect against circular follows */
	if (old_room == location)
	    return;

    	for ( fch = old_room->people; fch != NULL; fch = fch_next )
    	{
            fch_next = fch->next_in_room;
	    if (fch->altitude != ch->altitude) continue;



            if (portal == NULL || portal->value[0] == -1) 
	    /* no following through dead portals */
                continue;
 
            if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
            &&   fch->position < POS_STANDING)
            	do_stand(fch,"");

            if ( fch->master == ch && fch->position == POS_STANDING)
            {
            	act( "You follow $N.", fch, NULL, ch, TO_CHAR );
		do_enter(fch,argument);
            }
    	}

 	if (portal != NULL && portal->value[0] == -1)
	{
	    act("$p fades out of existence.",ch,portal,NULL,TO_CHAR);
	    if (ch->in_room == old_room)
		act("$p fades out of existence.",ch,portal,NULL,TO_ROOM);
	    else if (old_room->people != NULL)
	    {
		act("$p fades out of existence.", 
		    old_room->people,portal,NULL,TO_CHAR);
		act("$p fades out of existence.",
		    old_room->people,portal,NULL,TO_ROOM);
	    }
	    extract_obj(portal);
	}
	return;
    }

    send_to_char("Nope, can't do it.\n\r",ch);
    return;
}
