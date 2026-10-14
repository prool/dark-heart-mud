/*
 * HELL, formerly known as BTL/Darkheart, by Adam Powell and Chris Mellish,
 *  1996 to 1998.  If you reproduce this code in any way please give credit
 *  to the writers.  Thanks.
 */



#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "hell.h"
#include "trade.h"

/* command procedures needed */
DECLARE_DO_FUN(do_say           );
DECLARE_DO_FUN(do_tell          );
DECLARE_DO_FUN(do_berserk       );
DECLARE_DO_FUN(do_shout         );
DECLARE_DO_FUN(do_gecho         );
DECLARE_DO_FUN(do_quit          );
DECLARE_DO_FUN(do_psioff        );


/* Set these globals up */
int 	weird_weather_type = 0;
int 	weird_weather_timer = 0;

/*
 * Local functions.
 */
int     hit_gain        args( ( CHAR_DATA *ch ) );
int     psp_gain        args( ( CHAR_DATA *ch ) );
int     move_gain       args( ( CHAR_DATA *ch ) );
void    mobile_update   args( ( void ) );
void    weather_update  args( ( void ) );
void    char_update     args( ( void ) );
void    obj_update      args( ( void ) );
void    room_update     args( ( void ) );
void    aggr_update     args( ( void ) );
void    add_drag_innate args( ( CHAR_DATA *ch ) );
void    gain_conjunction args(( CHAR_DATA *ch ) );

/* used for saving */

int     save_number = 0;

/* secondary races */
/* Name, min level */
const struct secondaryrace_type secndrace [8] =
{
    {   "  None   ",                0      },
    {   " Dragon  ",          30      },
    {   " Daemon  ",          20      },
    {   " Vampire ",          20      },
    {   " Lupine  ",          20      },
    {   " Eladrin ",          20      },
    {   " Warlock ",          15      },
    {   "  *Hero* ",          15      }
};


const char * wall[MAX_WALL+1] = 
{
    "","wall of ice", "wall of fire", "wall of swords",
	"prismatic wall", "wall or iron", "wall of mushrooms",
	"wall of caltrops", "wall of ash", "wall of water"
};


/*
 * Advancement stuff.
 */
void advance_level( CHAR_DATA *ch, bool hide )
{
    char buf[MAX_STRING_LENGTH];

    WRITE_STR(debug_last_command, "advance_level");

    if (ch->secondaryrace == 0 && ch->alignment > 400
	&& ch->level > 40)
    {
	crash("making a hero at level 40");
	ch->secondaryrace = RACE_HERO;
	ch->tertiaryrace = 0;
	set_learnable_disciplines(ch, RACE_HERO);
	ch->clan = CLAN_HERO_SUN;
	ch->generation = 1;
	ch->hit += 100;
	ch->max_hit += 100;
	ch->faith += 3;
	ch->qp += 1;
	send_to_char("You have been rewarded for your goodness.  You have joined the ranks of heroes.\n\r", ch);
	send_to_char("You are now a member of the Sun Warriors, do not fail them.\n\r", ch);
	sprintf(buf, "-->%s has been granted the status of Hero by the gods.",GET_NAME(ch) );
        informational(buf, 0, 0);
    }

    ch->pcdata->last_level = 
	( ch->played + (int) (current_time - ch->logon) ) / 3600;

    check_player_table(ch);
    save_char_obj(ch);
    return;
}   


void add_drag_innate(CHAR_DATA *ch)
{
    int genm, genr;

    WRITE_STR(debug_last_command, "add_drag_innate");

    switch (ch->generation)
    {
	case  1 : genm =  3; genr =  8; break;
	case  2 : genm =  3; genr = 10; break;
	case  3 : genm =  3; genr = 12; break;
	case  4 : genm =  4; genr = 12; break;
	case  5 : genm =  6; genr = 12; break;
	case  6 : genm =  2; genr =  8; break;
	case  7 : genm =  1; genr =  8; break;
	case  8 : genm =  1; genr = 10; break;
	case  9 : genm =  1; genr = 12; break;
	case 10 : genm =  2; genr = 16; break;
	case 11 : genm =  1; genr = 16; break;
	case 12 : genm =  1; genr = 16; break;
	default : genm =  1; genr = 24; break;
    }

    if (ch->dragtype == DRAGON_RED)
    {
	if (ch->generation == 2)
	    ADD_INNATE(ch, skill_lookup("burning hands"), 3, 10)

	if (ch->generation == 3)
	    ADD_INNATE(ch, skill_lookup("continual light"), 3, 12)

	if (ch->generation == 5)
	    ADD_INNATE(ch, skill_lookup("fireball"), 6, 12)

	if (ch->generation == 7)
	    ADD_INNATE(ch, skill_lookup("flame strike"), 1, 8)

	if (ch->generation == 9)
	    ADD_INNATE(ch, skill_lookup("wall of fire"), 1, 12)

	if (ch->generation == 10)
	    ADD_INNATE(ch, skill_lookup("conflaguration"), 2, 16)

	if (ch->generation == 12)
	    ADD_INNATE(ch, skill_lookup("summon inferno"), 1, 16)

	return;
    }

    if (ch->dragtype == DRAGON_BLACK)
    {
	if (ch->generation == 2)
	    ADD_INNATE(ch, skill_lookup("armour"), 3, 10);
	if (ch->generation == 2)
	    ADD_INNATE(ch, skill_lookup("glitterdust"), genm, genr);
	if (ch->generation == 4)
	    ADD_INNATE(ch, skill_lookup("dispel magic"), genm, genr);
	if (ch->generation == 6)
	    ADD_INNATE(ch, skill_lookup("slow"), genm, genr);
	if (ch->generation == 7)
	    ADD_INNATE(ch, skill_lookup("earthquake"), genm, genr);
	if (ch->generation == 9)
	    ADD_INNATE(ch, skill_lookup("stinking cloud"), genm, genr);
	if (ch->generation == 11)
	    ADD_INNATE(ch, skill_lookup("iron skin"), genm, genr);
	if (ch->generation == 12)
	    ADD_INNATE(ch, skill_lookup("wall of iron"), genm, genr);

	return;
    }

    if (ch->dragtype == DRAGON_WHITE)
    {
	if (ch->generation == 1)
	    ADD_INNATE(ch, skill_lookup("chill touch"), genm, genr);
	if (ch->generation == 7)
	    ADD_INNATE(ch, skill_lookup("dispel evil"), genm, genr);
	if (ch->generation == 8)
	    ADD_INNATE(ch, skill_lookup("sanctuary"), genm, genr);
	if (ch->generation == 3)
	    ADD_INNATE(ch, skill_lookup("cause serious"), genm, genr);
	if (ch->generation == 1)
	    ADD_INNATE(ch, skill_lookup("detect magic"), genm, genr);
	if (ch->generation == 2)
	    ADD_INNATE(ch, skill_lookup("detect good"), genm, genr);
	if (ch->generation == 4)
	    ADD_INNATE(ch, skill_lookup("haste"), genm, genr);
	if (ch->generation == 12)
	    ADD_INNATE(ch, skill_lookup("winds of caina"), genm, genr);
	return;
    }

    if (ch->dragtype == DRAGON_BLUE)
    {
	if (ch->generation == 4)
	    ADD_INNATE(ch, skill_lookup("lightning bolt"), genm, genr);
	if (ch->generation == 10)
	    ADD_INNATE(ch, skill_lookup("chain lightning"), genm, genr);
	if (ch->generation == 9)
	    ADD_INNATE(ch, skill_lookup("windtomb"), genm, genr);
	if (ch->generation == 11)
	    ADD_INNATE(ch, skill_lookup("storm herald"), genm, genr);
	if (ch->generation == 12)
	    ADD_INNATE(ch, skill_lookup("time twister"), 1, 24);
	if (ch->generation == 4)
	    ADD_INNATE(ch, skill_lookup("phantasmal force"), genm, genr);
	if (ch->generation == 8)
	    ADD_INNATE(ch, skill_lookup("control weather"), genm, genr);
	if (ch->generation == 9)
	    ADD_INNATE(ch, skill_lookup("call lightning"), genm, genr);
	return;
    }

    if (ch->dragtype == DRAGON_SHADOW)
    {
	if (ch->generation == 8)
	    ADD_INNATE(ch, skill_lookup("animate dead"), genm, genr);
	if (ch->generation == 4)
	    ADD_INNATE(ch, skill_lookup("blindness"), genm, genr);
	if (ch->generation == 10)
	    ADD_INNATE(ch, skill_lookup("pentagram"), 1, 48);
	if (ch->generation == 10)
	    ADD_INNATE(ch, skill_lookup("bind demon"), 1, 48);
	if (ch->generation == 8)
	    ADD_INNATE(ch, skill_lookup("black tenticals"), 1, 36);
	if (ch->generation == 5)
	    ADD_INNATE(ch, skill_lookup("harm"), genm, genr);
	if (ch->generation == 1)
	    ADD_INNATE(ch, skill_lookup("cause light"), genm, genr);
	if (ch->generation == 2)
	    ADD_INNATE(ch, skill_lookup("cause serious"), genm, genr);
	if (ch->generation == 8)
	    ADD_INNATE(ch, skill_lookup("energy drain"), genm, genr);
	if (ch->generation == 11)
	    ADD_INNATE(ch, skill_lookup("shadow blade"), genm, genr);
	if (ch->generation == 12)
	    ADD_INNATE(ch, skill_lookup("blood spew"), 1, 24);

	return;
    }

    if (ch->dragtype == DRAGON_GOLD)
    {
	if (ch->generation == 6)
	    ADD_INNATE(ch, skill_lookup("cry of anger"), genm, genr);
	if (ch->generation == 11)
	    ADD_INNATE(ch, skill_lookup("summon inferno"), genm, genr);
	if (ch->generation == 8)
	    ADD_INNATE(ch, skill_lookup("wall of fire"), genm, genr);
	if (ch->generation == 7)
	    ADD_INNATE(ch, skill_lookup("sunbeam"), genm, genr);
	if (ch->generation == 1)
	    ADD_INNATE(ch, skill_lookup("burning hands"), genm, genr);
	if (ch->generation == 3)
	    ADD_INNATE(ch, skill_lookup("faerie fire"), genm, genr);
	if (ch->generation == 2)
	    ADD_INNATE(ch, skill_lookup("glitterdust"), genm, genr);
	if (ch->generation == 5)
	    ADD_INNATE(ch, skill_lookup("hold person"), genm, genr);
	return;
    }

/*    if (ch->dragtype == DRAGON_SILVER)
    {
	if (ch->generation == )
	    ADD_INNATE(ch, skill_lookup(""), genm, genr);
	return;
    }*/

    if (ch->dragtype == DRAGON_GREEN)
    {
	if (ch->generation == 7)
	    ADD_INNATE(ch, skill_lookup("stinking cloud"), genm, genr);
	if (ch->generation == 4)
	    ADD_INNATE(ch, skill_lookup("poison"), genm, genr);
	if (ch->generation == 11)
	    ADD_INNATE(ch, skill_lookup("fear"), genm, genr);
	if (ch->generation == 6)
	    ADD_INNATE(ch, skill_lookup("sleep"), genm, genr);
	if (ch->generation == 9)
	    ADD_INNATE(ch, skill_lookup("gate"), genm, genr);
	if (ch->generation == 12)
	    ADD_INNATE(ch, skill_lookup("frog"), genm, genr);
	if (ch->generation == 5)
	    ADD_INNATE(ch, skill_lookup("slow"), genm, genr);
	if (ch->generation == 6)
	    ADD_INNATE(ch, skill_lookup("stone skin"), genm, genr);
	if (ch->generation == 4)
	    ADD_INNATE(ch, skill_lookup("faerie fog"), genm, genr);
	return;
    }

    /* Do the rest of the dragons as well :) */

}


void gain_exp( CHAR_DATA *ch, int gain )
{
    int lmt = ch->level * 500;
    char buf[MAX_INPUT_LENGTH];

    WRITE_STR(debug_last_command, "gain_exp routine");

    if (ch->level > 99) return;

    if (ch->level < 2)
	lmt *= 2;
   
    crash("gain_exp 1");

    console("GAIN_EXP: ch=%s  gain=%d  lmt=%d",
	GET_PC_NAME(ch), gain, lmt);

    if (gain > lmt)
	gain = lmt * (dice(1,10) + 96) / 100;

    crash("gain_exp 2");

    ch->currentexp += gain;
    if ( IS_NPC(ch) || ch->level >= LEVEL_HERO )
	return;
    
    gain = gain/ch->level/2;

    crash("gain_exp 3");

    ch->exp = UMAX( 1000, ch->exp + gain );
    while ( ch->level < 100 && ch->exp >= 
	1000 * (ch->level+1))
    {
	ch->level += 1;
	sprintf(buf,"%s has attained level %d!",ch->name,ch->level);
	log_string(buf);
	sprintf(buf,"%s has attained level %d!",ch->name,ch->level);
	wiznet(buf,ch,NULL,WIZ_LEVELS,0,0);
	advance_level(ch,FALSE);
    }
    
    crash("gain_exp 4");

    while (ch->level > 1 && ch->exp < 1000 * (ch->level - 1) )
    {
	ch->level -=1;
	sprintf(buf,"%s has dropped to level %d!",ch->name,ch->level);
	log_string(buf);
	sprintf(buf,"%s has dropped to level %d!",ch->name,ch->level);
	wiznet(buf,ch,NULL,WIZ_LEVELS,0,0);
    } 
    
    crash("gain_exp 5");

    save_char_obj(ch);
    
    crash("gain_exp 6");

    return;
}



/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    WRITE_STR(debug_last_command, "hit_gain routine");

    if (is_null_room(ch))
	return 0;

    if ( IS_NPC(ch) )
    {
	gain =  5 + ch->level + dice(1, 4);
	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 4;

	switch(ch->position)
	{
	    default :           gain /= 2;                      break;
	    case POS_SLEEPING:  gain *= 3;	                break;
	    case POS_RESTING:                                   break;
	    case POS_FIGHTING:  gain /= 3;                      break;
	}

	
    }
    else
    {
	gain = UMAX(3,get_curr_stat(ch,STAT_CON) - 3 + ch->level/2); 
	number = number_percent();

   	/* Modified for extra gainingness */
	if (number < get_skill(ch,gsn_fast_healing) + 5)
	{
	    gain += number * gain / 100;
	    if (ch->hit < ch->max_hit)
		check_improve(ch,gsn_fast_healing,FALSE,8);
	}
	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 4;

	switch ( ch->position )
	{
	    default:            gain /= 3;                      break;
	    case POS_SLEEPING:  gain *= 3; 			break;
	    case POS_RESTING:   gain /= 2;                      break;
	    case POS_FIGHTING:  gain /= 6;                      break;
	}

	if ( ch->pcdata->condition[COND_HUNGER] < 20 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] < 20 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_HUNGER] < 10 )
	    gain /= 3;

    }

    gain = gain * ch->in_room->heal_rate / 100;
    
    if (!RTIME_UP(ch->in_room, RTIMER_LIFE_VORTEX))
    {
	send_to_char("Blue light floods out of the vortex into you.\n\r", ch);
	gain *= 5;
    }

    if (!RTIME_UP(ch->in_room, RTIMER_DEATH_VORTEX))
    {
	send_to_char("Black light floods out of the vortex into you.\n\r", ch);
	gain = 0;
    }
 
    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 100;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;
    
    if (IS_FLAG3(ch, AFF_PESTILENCE))
	gain /= 12;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
	gain /=2 ;

    if (ch->secondaryrace == RACE_LUPINE)
	gain *= 3;

    if (ch->trades[TRADE_SURVIVAL] >= 1 && ch->position == POS_SLEEPING)
	gain = gain * 3 / 2;

    return UMIN(gain, ch->max_hit - ch->hit);
}

int psp_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    WRITE_STR(debug_last_command, "psp_gain routine");

    if (is_null_room(ch))
	return 0;

    if ( IS_NPC(ch) )
    {
	gain = 5 + ch->level;
	switch (ch->position)
	{
	    default:            gain /= 2;              break;
	    case POS_SLEEPING:  gain = 3 * gain/2;      break;
	    case POS_RESTING:                           break;
	    case POS_FIGHTING:  gain /= 3;              break;
	}
    }
    else
    {
	gain = (get_curr_stat(ch,STAT_WIS) 
	      + get_curr_stat(ch,STAT_INT)*2 + ch->psi_potential*2) / 2;
	number = number_percent();
	if (number < get_skill(ch,gsn_meditation))
	{
	    gain += number * gain / 100;
/*          if (ch->psp < ch->max_psp)
		check_improve(ch,gsn_meditation,TRUE,8);*/
	}

	switch ( ch->position )
	{
	    default:            gain /= 4;                      break;
	    case POS_SLEEPING:                                  break;
	    case POS_RESTING:   gain /= 2;                      break;
	    case POS_FIGHTING:  gain /= 6;                      break;
	}

	if ( ch->pcdata->condition[COND_HUNGER] < 20 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] < 20 )
	    gain /= 2;

    }

    if (is_wearing_artifact(ch, ARTIFACT_CROWN_OF_AGES) != NULL) gain *= 3;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[4] / 100;

    if (weird_weather_type == WEIRD_WEATHER_LIGHT)
    	gain *= 3;

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
	gain /=2 ;

    if (ch->trades[TRADE_SURVIVAL] >= 1 && ch->position == POS_SLEEPING)
	gain = gain * 3 / 2;

    return UMIN(gain, ch->max_psp - ch->psp);
}



int move_gain( CHAR_DATA *ch )
{
    int gain;

    WRITE_STR(debug_last_command, "move_gain routine");

    if (ch->in_room == NULL || !TIME_UP(ch, TIMER_ON_SPEED))
	return 0;

    if ( IS_NPC(ch) )
    {
	gain = ch->level;
    }
    else
    {
	gain = UMAX( 15, ch->level );

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_stat(ch,STAT_DEX);          break;
	case POS_RESTING:  gain += get_curr_stat(ch,STAT_DEX) / 2;      break;
	}

	if ( ch->pcdata->condition[COND_HUNGER] < 20 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] < 20 )
	    gain /= 2;
    }

    gain = gain * ch->in_room->heal_rate/100;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 100;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
	gain /=2 ;

    if (ch->trades[TRADE_SURVIVAL] >= 1 && ch->position == POS_SLEEPING)
	gain = gain * 3 / 2;

    return UMIN(gain, ch->max_move - ch->move);
}



void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;

    WRITE_STR(debug_last_command, "gain_condition routine");

    if ( value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
	return;

    condition                           = ch->pcdata->condition[iCond];
    if (condition == -1)
	return;
    if (dice (1, 2) == 1)	
        ch->pcdata->condition[iCond]        = URANGE( 0, condition +value, 48 );

    if (condition < 20 && ch->pcdata->condition[iCond] >= 20)
    {
	switch ( iCond )
	{
	case COND_HUNGER:
	    send_to_char("You are no longer hungry.\n\r",ch);
	    break;

	case COND_THIRST:
	    send_to_char("You are no longer thirsty.\n\r",ch);
	    break;
	}
    }


    if ( ch->pcdata->condition[iCond] < 20 
	&& ch->pcdata->condition[iCond] > 5)
    {
	switch ( iCond )
	{
	case COND_HUNGER:
	    send_to_char( "You are hungry.\n\r",  ch );
	    break;

	case COND_THIRST:
	    send_to_char( "You are thirsty.\n\r", ch );
	    break;
	}
    }

    if ( ch->pcdata->condition[iCond] < 6)
    {
	switch ( iCond )
	{
	case COND_HUNGER:
	    send_to_char( "You are dying of hunger.\n\r",  ch );
	    break;

	case COND_THIRST:
	    send_to_char( "You are dying of thirst.\n\r", ch );
	    break;
	}
    }

    if ( ch->pcdata->condition[iCond] < 1)
    {
	switch ( iCond )
	{
	case COND_DRUNK:
	    if ( condition != 0 )
		send_to_char( "You are sober.\n\r", ch );
	    break;
	}
    }

    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    char buf[200];
    int door;
    int counter = 0;

    WRITE_STR(debug_last_command, "mobile_update routine");

    /* Examine all mobs. */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

 	counter += 1;
	if (counter == 1000)
	{
	    bug("crashing out of mobile_update - overflow", 0);
	    crash("mob update overflow");	
	    return; 
	}

	if ( !IS_NPC(ch) || ch->in_room == NULL || IS_AFFECTED(ch,AFF_CHARM))
	    continue;

	if (ch->in_room->area->empty && !IS_SET(ch->act,ACT_UPDATE_ALWAYS))
	    continue;

	/* That's all for sleeping / busy monster, and empty zones */
	if ( ch->position != POS_STANDING )
	    continue;

	sprintf(buf, "mobile update - scavenge (%s in room %d)",	
	    GET_NAME(ch), ch->in_room->vnum);
	crash(buf);

	/* Scavenge */
	if ( IS_SET(ch->act, ACT_SCAVENGER)
	&&   ch->in_room->contents != NULL
	&&   number_bits( 6 ) == 0 )
	{
	    OBJ_DATA *obj = NULL;
	    OBJ_DATA *obj_best = NULL;
	    int max;

	    max         = 1;

	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
	    {
		if ( CAN_WEAR(obj, ITEM_TAKE) && can_loot(ch, obj)
		     && obj->cost > max  && obj->cost > 0)
		{
		    obj_best    = obj;
		    max         = obj->cost;
		}
	    }

	    if ( obj_best != NULL )
	    {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act( "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
	    }
	}

	sprintf(buf, "mobile update - wander (%s in room %d)",	
	    GET_NAME(ch), ch->in_room->vnum);
	crash(buf);

	/* Wander */
	if ( !IS_SET(ch->act, ACT_SENTINEL) 
	&& number_bits(3) == 0
	&& ( door = number_bits( 5 ) ) <= 5
	&& ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
	&& ( !IS_SET(ch->act, ACT_STAY_AREA)
	||   pexit->u1.to_room->area == ch->in_room->area ) 
	&& ( !IS_SET(ch->act, ACT_OUTDOORS)
	||   !IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)) 
	&& ( !IS_SET(ch->act, ACT_INDOORS)
	||   IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)))
	{
	    crash("A mob is wandering.");
	    move_char( ch, door, FALSE );
	    crash("A mob has wandered.");
	}
    }

    return;
}


/* do conjunction discipline auto-gain */

void gain_conjunction ( CHAR_DATA *ch )
{
    ch->power[DISC_DRAG_CONJ] += 1;

    WRITE_STR(debug_last_command, "gain_conjunction routine");

    act("A bright moon beam bathes $n in light, surrounding $m with an aura.", ch, NULL, NULL, TO_ROOM);
    stc("A bright moon beam bathes you in light, surrounding you with an aura.\n\r",ch);
    if (ch->power[DISC_DRAG_CONJ] == 1)
    {
	stc("You feel more protected by the gods.\n\r",ch);
	return;
    }
    if (ch->power[DISC_DRAG_CONJ] == 2)
    {
	stc("You feel the power of the gods pulsing through your veins.\n\r",ch);
	return;
    }
    if (ch->power[DISC_DRAG_CONJ] == 3)
    {
	stc("You feel more at ease.\n\r",ch);
	return;
    }
    if (ch->power[DISC_DRAG_CONJ] == 4)
    {
	ADD_INNATE( ch, skill_lookup("consultation"), 1, 24);
	stc("You can now consult with the gods.\n\r",ch);
	return;
    }
    if (ch->power[DISC_DRAG_CONJ] == 5)
    {
	stc("A quiet voice whispers in the wind 'Behold'.\n\r",ch);
	return;
    }
    if (ch->power[DISC_DRAG_CONJ] == 6)
    {
	ADD_INNATE( ch, skill_lookup("power word kill"), 1, 24);
	stc("You now command life.\n\r",ch);
	return;
    }
    if (ch->power[DISC_DRAG_CONJ] == 7)
    {
	stc("Your body tingles slightly as your new regenerative powers start.\n\r",ch);
	return;
    }
    if (ch->power[DISC_DRAG_CONJ] == 8)
    {
	stc("You grow two new arms.\n\r",ch);
    	REMOVE_BIT(ch->body_form, BODY_LEFT_ARM);                               
    	REMOVE_BIT(ch->body_form, BODY_LEFT_HAND);                              
    	REMOVE_BIT(ch->body_form, BODY_RIGHT_ARM);                              
    	REMOVE_BIT(ch->body_form, BODY_RIGHT_HAND);                             
    	SET_BIT(ch->body_form, BODY_ENABLE_THIRD_ARM);                          
    	SET_BIT(ch->body_form, BODY_ENABLE_FOURTH_ARM);                         
    	SET_BIT(ch->body_form, BODY_THIRD_ARM);                                 
    	SET_BIT(ch->body_form, BODY_THIRD_HAND);                        
    	SET_BIT(ch->body_form, BODY_FOURTH_ARM); 
       	SET_BIT(ch->body_form, BODY_FOURTH_HAND);                               

	return;
    }
    if (ch->power[DISC_DRAG_CONJ] == 9)
    {
	stc("A soft voice whispers on the wind 'In Articulo Mortis'.\n\r",ch);
	return;
    }
    if (ch->power[DISC_DRAG_CONJ] == 10)
    {
	stc("You feel the supreme power of the gods flowing through you.\n\r",ch);
	return;
    }
}



/*
 * Update the weather.
 */
void weather_update( void )
{
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int diff;

    buf[0] = '\0';

    WRITE_STR(debug_last_command, "weather_update routine");

    /* Initialising the weird weather things */
    if (weird_weather_type == 0 && dice(1, 1000) == 1)
    {
	weird_weather_type = dice(1, MAX_WEIRD_WEATHER);
	if (weird_weather_type == WEIRD_WEATHER_FOG)
	    gecho("A blanket of fog descends from the sky.\n\r");
	if (weird_weather_type == WEIRD_WEATHER_FISH)
	    gecho("Small fish begin to rain from the sky.\n\r");
	if (weird_weather_type == WEIRD_WEATHER_HURRICANE)
	    gecho("Strong winds begin to buffet you around.\n\r");
	
	weird_weather_timer = dice(3, 6);
    }

    weird_weather_timer--;
    if (weird_weather_timer <= 0)
    {
	if (weird_weather_type == WEIRD_WEATHER_FOG)
	    gecho("The fog clears.\n\r");
	if (weird_weather_type == WEIRD_WEATHER_HURRICANE)
	    gecho("The strong winds die down.\n\r");
	weird_weather_type = 0;
	weird_weather_timer = 0;
    }

    if (weird_weather_type == WEIRD_WEATHER_TORNADO)
    {
	if ( weird_weather_timer == 6)
	    gecho("A slight breeze blows in from the east.\n\r");
	if ( weird_weather_timer == 5)
	    gecho("The winds start to pick up.\n\r");
	if ( weird_weather_timer == 4)
	    gecho("The winds start to get stronger.\n\r");
	if ( weird_weather_timer == 3)
	    gecho("The fierce winds blow around you, nearly knocking you down.\n\r");
	if ( weird_weather_timer == 2)
	{
	    gecho("A huge tornado blows around you.\n\r");
	    for (ch = char_list ; ch != NULL ; ch = ch->next)
	    {
		if (IS_INSIDE(ch) || IS_NPC(ch)) continue;
	 	stc("You get sucked into the air......\n\r", ch);
		stc("You get battered and buffeted by the tornado.\n\r", ch);
		stc("You eventually fall to the ground in a new place.\n\r", ch);
		ch->position = POS_RESTING;
		update_pos(ch);
		char_from_room(ch);
		char_to_room(ch, get_rand_room(), 0);
	    }
	    weird_weather_timer = 0;
	    weird_weather_type = 0;
	}
    }

    time_info.min += 10;
    if (time_info.min >= 60)
    {
	time_info.min -= 60;
    	switch ( ++time_info.hour )
    	{
    	case  5:
		weather_info.sunlight = SUN_LIGHT;
		strcat( buf, "The day has begun.\n\r" );
		break;
	
   	case  6:
		weather_info.sunlight = SUN_RISE;
		strcat( buf, "The sun rises in the east.\n\r" );
		if (weird_weather_type == WEIRD_WEATHER_SUNRISE)
		{
		    CHAR_DATA *ch;
  		    for ( ch = char_list; ch != NULL; ch = ch->next )
		    {
		        stc("Brilliant rays of light infuse your body with goodness.\n\r", ch);
		        restore_char(ch);
		    }
		}
		break;

    	case 19:
		weather_info.sunlight = SUN_SET;
		strcat( buf, "The sun slowly disappears in the west.\n\r" );
		crash("gaining voodoo points");

		for ( d = descriptor_list; d != NULL; d = d->next )
    		{
        	    if ( d->connected == CON_PLAYING
			&& !IS_NPC(d->character)
			&& IS_EVIL(d->character)
			&& TIME_UP(d->character, TIMER_CAN_GAIN_VOODOO) )
            	{
			int voogain;

			voogain = voodoo_level(d->character) + 1 - dice(1,6);
			if (voogain == 1)
			    sprintf(buf2,"You gained one voodoo point for that day.\n\r");
			else if (voogain > 1)
			    sprintf(buf2,"You gained %d voodoo points for that day.\n\r", voogain);
		 	if (voogain > 0)
			{
			    send_to_char(buf2, d->character);
			    SET_TIMER(d->character, TIMER_CAN_GAIN_VOODOO, 3);
			    d->character->pcdata->voodoo_points += voogain;
			    voogain = 0;
			}
		    }
		}
		break;

    	case 20:
		weather_info.sunlight = SUN_DARK;
		strcat( buf, "Night sweeps over the land.\n\r" );
		if (time_info.day > 33
			|| time_info.day < 2)
		{
		     for ( d = descriptor_list; d != NULL; d = d->next )
    		     {
        		 if ( d->connected == CON_PLAYING && d->character != NULL)
        		 {
   			     send_to_char("It is a full moon tonight.\n\r",d->character);
			     if (d->character->secondaryrace == RACE_DRAGON)
			     {
				if (dice(1, d->character->power[DISC_DRAG_CONJ] * 3 + 1) == 1)
				{
				    gain_conjunction( d->character );
				}
			     }
			     if (d->character->secondaryrace == RACE_ELADRIN
				 && dice(1, 5) == 1)	 
			    	 gain_elad_path(d->character, 3);
			     if (d->character->secondaryrace == RACE_LUPINE)
			     {
				d->character->hit = d->character->max_hit;	
				d->character->move = d->character->max_move;	
				d->character->psp = d->character->max_psp;
			  	send_to_char("You feel fully invigorated.\n\r", d->character);
				if (dice (1, 2) == 1 && in_were_form(d->character))
				    do_shout(d->character,"Aaaaarrrrrrrooooooooooowwwwwwwwwwwhhhhhhhhh");
				else if (in_were_form(d->character))
				    do_shout(d->character, "Wwwwwwowoooooowoowooooohhhhhhhhh");
				werewolf(d->character);
			     }
	
		             if (IS_FLAG3(d->character, AFF_LYCANTHROPY)
			        && d->character->secondaryrace == 0
			        && !IS_NPC(d->character)
				&& IS_OUTSIDE(d->character) )
 		             {	
 				 WRITE_STR(debug_last_command, "make werewolf");
			         REMOVE_BIT(d->character->flag3, AFF_LYCANTHROPY);
			         d->character->secondaryrace = 4;
    			         d->character->generation = 1;
				 SET_BIT(d->character->affected_by, AFF_DARK_VISION);
    			         set_learnable_disciplines(d->character, RACE_LUPINE);
        	                 d->character->power[DISC_LUPI_LEON] = 0;
        	          	 d->character->power[DISC_LUPI_LUPI] = 0;
				 d->character->dragtype = dice(1, 4);
        	          	 d->character->pcdata->damcap = 100;
        	          	 set_form(d->character,  0);
        	          	 d->character->hit = d->character->max_hit;
			  	 d->character->clan = CLAN_LUPI_ROGUE;
			  	 WAIT_STATE(d->character, 20);
			   	 send_to_char("You scream in agony as you are bathed in the light of the full moon.\n\r",d->character);
			  	 send_to_char("Hair bursts through your skin and you transform into a werewolf.\n\r",d->character);
			         act("$n screams in agony as $e painfully transforms into a werewolf.", d->character, NULL, NULL,TO_ROOM);
				 werewolf(d->character);
			    }
		        }
        	    }
        	}	
    	break;

    	case 24:
		time_info.hour = 0;
		time_info.day++;
		break;
    	}
    }

    if ( time_info.day   >= 35 )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= 17 )
    {
	time_info.month = 0;
	time_info.year++;
    }

    /*
     * Weather change.
     */
    if ( time_info.month >= 9 && time_info.month <= 16 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change    = UMAX(weather_info.change, -12);
    weather_info.change    = UMIN(weather_info.change,  12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
    weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

    switch ( weather_info.sky )
    {
    default: 
	bug( "Weather_update: bad sky %d.", weather_info.sky );
	weather_info.sky = SKY_CLOUDLESS;
	break;

    case SKY_CLOUDLESS:
	if ( weather_info.mmhg <  990
	|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The sky is getting cloudy.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_CLOUDY:
	if ( weather_info.mmhg <  970
	|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "It starts to rain heavily.\n\r" );
	    weather_info.sky = SKY_RAINING;
	}

	if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "The clouds disappear.\n\r" );
	    weather_info.sky = SKY_CLOUDLESS;
	}
	break;

    case SKY_RAINING:
	if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "Lightning flashes in the sky.\n\r" );
	    weather_info.sky = SKY_LIGHTNING;
	}

	if ( weather_info.mmhg > 1030
	|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The rain stops.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_LIGHTNING:
	if ( weather_info.mmhg > 1010
	|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The lightning has stopped.\n\r" );
	    weather_info.sky = SKY_RAINING;
	    break;
	}
	break;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character) )
		send_to_char( buf, d->character );
	}
    }

    return;
}

int is_wall(EXIT_DATA *ex)
{
    crash("is_wall procedure");

    if (IS_SET(ex->exit_info, EX_ICE_WALL)) return 1;
    if (IS_SET(ex->exit_info, EX_FIRE_WALL)) return 2;
    if (IS_SET(ex->exit_info, EX_SWORD_WALL)) return 3;
    if (IS_SET(ex->exit_info, EX_PRISMATIC_WALL)) return 4;
    if (IS_SET(ex->exit_info, EX_IRON_WALL)) return 5;
    if (IS_SET(ex->exit_info, EX_MUSHROOM_WALL)) return 6;
    if (IS_SET(ex->exit_info, EX_CALTROP_WALL)) return 7;
    if (IS_SET(ex->exit_info, EX_ASH_WALL)) return 8;
    if (IS_SET(ex->exit_info, EX_WATER_WALL)) return 9;
    return 0;
}


/*
 * Update all rooooooms, like gore, poison clouds etc....
 */
void room_update( void )
{
    int i, door;
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *room;
    OBJ_DATA *obj;

    WRITE_STR(debug_last_command, "room_update");

    /* Puts the magical rainbow pools in, for Eladrin Aura 
	 Ha! They should be so lucky!  */
    if (dice(1, 16) == 1)
    {
	room = get_treemeld_room();
	obj = create_object( get_obj_index(107), 0);
	obj->timer = dice(2, 3);
	obj_to_room(obj, room, 0);
	room_message(room, "A rainbow pool bubbles up from the ground.");
    }

    if (dice(1, 30) == 1)
    {
	room = get_rand_room_by_sect( SECT_MOUNTAIN );
	obj = create_object( get_obj_index(109), 0);
	obj->timer = dice(2, 10) + 4;
	obj_to_room(obj, room, 0);
	room_message(room, "A book falls from the sky.");
    }

    /* This bit puts a random mushroom into the game */
    if (dice(1, 13) == 1)
    {
	room = get_treemeld_room();
	obj = create_object( get_obj_index(97), 0);
	obj->value[2] = dice(1, MAX_MUSHROOM_EFFECT);
	obj_to_room(obj, room, 0);
	room_message(room, "A mushroom sprouts from the ground.");
    }

    WRITE_STR(debug_last_command, "room_update - timers");

    for ( room = room_list; room != NULL; room = room->next_room)
    {
        crash("getting rid of walls in update.c");

        if (RTIMER(room, RTIMER_WALL_NORTH) == 1
	    && room->exit[DIR_NORTH] != NULL
	    && is_wall(room->exit[DIR_NORTH])!= 0)
        {
	    sprintf(buf, "The %s crumbles into dust.", wall[is_wall(room->exit[DIR_NORTH])]);
	    room_message(room, buf);
	    make_wall(room, DIR_NORTH, 0);
        }

        if (RTIMER(room, RTIMER_WALL_SOUTH) == 1
	    && room->exit[DIR_SOUTH] != NULL
	    && is_wall(room->exit[DIR_SOUTH])!= 0)
        {
	    sprintf(buf, "The %s crumbles into dust.", wall[is_wall(room->exit[DIR_SOUTH])]);
	    room_message(room, buf);
	    make_wall(room, DIR_SOUTH, 0);
        }

        if (RTIMER(room, RTIMER_WALL_EAST) == 1
	    && room->exit[DIR_EAST] != NULL
	    && is_wall(room->exit[DIR_EAST])!= 0)
        {
	    sprintf(buf, "The %s crumbles into dust.", wall[is_wall(room->exit[DIR_EAST])]);
	    room_message(room, buf);
	    make_wall(room, DIR_EAST, 0);
        }

        if (RTIMER(room, RTIMER_WALL_WEST) == 1
	    && room->exit[DIR_WEST] != NULL
	    && is_wall(room->exit[DIR_WEST])!= 0)
        {
	    sprintf(buf, "The %s crumbles into dust.", wall[is_wall(room->exit[DIR_WEST])]);
	    room_message(room, buf);
	    make_wall(room, DIR_WEST, 0);
        }

        if (RTIMER(room, RTIMER_WALL_UP) == 1
	    && room->exit[DIR_UP] != NULL
	    && is_wall(room->exit[DIR_UP])!= 0)
        {
	    sprintf(buf, "The %s crumbles into dust.", wall[is_wall(room->exit[DIR_UP])]);
	    room_message(room, buf);
	    make_wall(room, DIR_UP, 0);
        }

        if (RTIMER(room, RTIMER_WALL_DOWN) == 1
	    && room->exit[DIR_DOWN] != NULL
	    && is_wall(room->exit[DIR_DOWN])!= 0)
        {
	    sprintf(buf, "The %s crumbles into dust.", wall[is_wall(room->exit[DIR_DOWN])]);
	    room_message(room, buf);
	    make_wall(room, DIR_DOWN, 0);
        }




	if (RTIMER(room, RTIMER_STINKING_CLOUD) == 1)
	    room_message(room, "The poisonous vapours dissipate and clear.");

	if (RTIMER(room, RTIMER_HIDE_ROOM) == 1)
	    room_message(room, "The shroud leaves the room.");

	if (RTIMER(room, RTIMER_GHOST_LIGHT) == 2)
	    room_message(room, "The vapourous ghosts start howling insanely.");

	if (RTIMER(room, RTIMER_GHOST_LIGHT) == 3)
	    room_message(room, "The vapourous ghosts start moaning.");

	if (RTIMER(room, RTIMER_GHOST_LIGHT) == 1)
	    room_message(room, "The vapourous ghosts dissipate and vanish.");

	if (RTIMER(room, RTIMER_GLYPH_PROTECTION) == 1)
	    room_message(room, "The glyph of protection flares and vanishes.");

	if (RTIMER(room, RTIMER_SWARM_BEES) == 1)
	    room_message(room, "The bees fly away into the sky.");

	if (RTIMER(room, RTIMER_DISCORD) == 1)
	    room_message(room, "The banging and crashing stops.");

	if (RTIMER(room, RTIMER_SWARM_BATS) == 1)
	    room_message(room, "The bats flap away into the night.");

	if (RTIMER(room, RTIMER_SWARM_RATS) == 1)
	    room_message(room, "The rats scurry away into the floorboards.");

  	/* Nexus vanishing */
	if (RTIMER(room, RTIMER_NEXUS_FLAME) == 1)
	{
	    room_message(room, "The ring of fire vanishes in a cloud of smoke.");
	    if (room->far_link != NULL)
		room->far_link->far_link = NULL;
	    room->far_link = NULL;
	}
	if (RTIMER(room, RTIMER_NEXUS_WATER) == 1)
	{
	    room_message(room, "The ring of water vanishes in a cloud of smoke.");
	    if (room->far_link != NULL)
		room->far_link->far_link = NULL;
	    room->far_link = NULL;
	}
	if (RTIMER(room, RTIMER_NEXUS_AIR) == 1)
	{
	    room_message(room, "The spinning whirlwind vanishes in a cloud of smoke.");
	    if (room->far_link != NULL)
		room->far_link->far_link = NULL;
	    room->far_link = NULL;
	}
	if (RTIMER(room, RTIMER_NEXUS_EARTH) == 1)
	{
	    room_message(room, "The floating leaves vanish in a cloud of smoke.");
	    if (room->far_link != NULL)
		room->far_link->far_link = NULL;
	    room->far_link = NULL;
	}
	if (RTIMER(room, RTIMER_NEXUS_ENTROPY) == 1)
	{
	    room_message(room, "The circle of blackness vanishes in a cloud of smoke.");
	    if (room->far_link != NULL)
		room->far_link->far_link = NULL;
	    room->far_link = NULL;
	}

        WRITE_STR(debug_last_command, "moving room timers");

	/* The life vortex random movement routine */
	if (RTIMER(room, RTIMER_LIFE_VORTEX) == 1)
	{
	    bool moved = FALSE;
	    if (dice(1, 6) == 1)
		room_message(room, "The vortex blinks out of existence.");
	    else
	    {
		for (door = 0 ; door < 6 ; door++ )
		{
		    if (room->exit[door] != NULL)
		    {
			room_message(room, "The vortex spins off into the distance.");
			ADD_RTIMER(room->exit[door]->u1.to_room, RTIMER_LIFE_VORTEX,dice(1, 5)+3 );
			moved = TRUE;
			break;
		    }
		}
		if (!moved) room_message(room, "The vortex blinks out of existence.");
	    }
	}

	if (RTIMER(room, RTIMER_DEATH_VORTEX) == 1)
	{
	    bool moved = FALSE;
	    if (dice(1, 6) == 1)
		room_message(room, "The vortex blinks out of existence.");
	    else
	    {
		for (door = 0 ; door < 6 ; door++ )
		{
		    if (room->exit[door] != NULL)
		    {
			room_message(room, "The vortex spins off into the distance.");
			ADD_RTIMER(room->exit[door]->u1.to_room, RTIMER_DEATH_VORTEX,dice(1, 5)+3 );
			moved = TRUE;
			break;
		    }
		}
		if (!moved) room_message(room, "The vortex blinks out of existence.");
	    }
	}

	for (i = 0 ; i < MAX_RTIMER ; i++)
	    room->tick_timer[i] = UMAX(room->tick_timer[i] - 1, 0);

	room->gore--;
	if (room->gore < 0) room->gore = 0;
    }

    crash("reached the end of room_update ok");
}

/*
 * Update all chars, including mobs.
 */
void char_update( void )
{   
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_quit;
    OBJ_DATA *obj;

    crash("char_update routine");

    ch_quit     = NULL;

    /* update save counter */
    save_number++;

    if (save_number > 29)
	save_number = 0;

    crash("char_update routine - pre builder");

    if (rb_timer > -1)
    {
	rb_timer -= 1;
	act("$n rushes around putting the house together.", rb_mob, NULL, NULL, TO_ROOM);
	if (rb_timer == 0)
	{
	    WRITE_STR(rb_room->description, "You are in a small, but neat house, which has just been finished.\n\r");
	    act("$n has finally finished the house, and wanders back to his office.", rb_mob, NULL, NULL, TO_ROOM);
	    char_from_room( rb_mob );
	    char_to_room( rb_mob, get_room_index( prime_realm, 20001 ), 0);
	    act("$n wanders into the room looking exhausted.", rb_mob, NULL, NULL, TO_ROOM);
	    rb_timer = -1;
	}
    }

    crash("char_update routine - done builder");

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	AFFECT_DATA *paf, af;
	AFFECT_DATA *paf_next;
	int i, j, k;
	char buf[MAX_STRING_LENGTH];	
  	int gain = 3;

	ch_next = ch->next;

	/* This should sort out any weird crashes.... ???? */
	if (is_null_room(ch)) continue;

	if (ch->trade_researching != -1)
	{
	    if (--ch->trade_time <= 0)
		gain_trade(ch);		    
	}

	crash("Put all that nice willpower back");
	if (!IS_NPC(ch)) regain_sanity(ch);

	crash("Getting your breath back");

        if (ch->power[DISC_DRAG_FIRE] >= 1
	    && ch->breaths < (ch->generation / 2) + 1 
	    && dice (1,2) == 1) ch->breaths += 1;    

	crash("hero continual bless spells");

	/* Level 3 true faith  -  autobless(curse) */

	if (ch->power[DISC_HERO_TRUE] >= 5
	    && !IS_EVIL(ch)
	    && !is_affected(ch, skill_lookup("protection evil") ) )
	{
            af.where     = TO_AFFECTS;
            af.type      = skill_lookup("protection evil");
            af.level     = ch->level;
            af.duration  = -1;
            af.modifier  = 0;
            af.location  = APPLY_NONE;
            af.bitvector = 0;
            send_to_char( "You feel safe from evil.\n\r", ch );
            affect_join( ch, &af );
	}

	if (ch->power[DISC_HERO_TRUE] >= 3
	    && !IS_EVIL(ch)
	    && !is_affected(ch, skill_lookup("bless") ) )
	{
            af.where     = TO_AFFECTS;
            af.type      = skill_lookup("bless");
            af.level     = ch->level;
            af.duration  = -1;
            af.modifier  = ch->level / (11 - ch->water);
            af.location  = APPLY_HITROLL;
            af.bitvector = 0;
            send_to_char( "You feel favoured by the gods.\n\r", ch );
            affect_join( ch, &af );
	    affect_strip(ch, skill_lookup("curse") );
	}
	else if (ch->power[DISC_HERO_TRUE] >= 3
	    && IS_EVIL(ch)
	    && !is_affected(ch, gsn_curse) )
	{
            af.where     = TO_AFFECTS;
            af.type      = skill_lookup("curse");
            af.level     = ch->level;
            af.duration  = -1;
            af.modifier  = - (ch->level / (11 - ch->water));
            af.location  = APPLY_HITROLL;
            af.bitvector = 0;
            send_to_char( "The gods are angered at you.\n\r", ch );
            affect_join( ch, &af );
	    affect_strip(ch, skill_lookup("bless") );
	}

        WRITE_STR(debug_last_command, "innate abils (char_update)");

	if ( GOT_INNATE( ch ) )
	{
	    int sn;

	    for (sn = 0; sn < MAX_SKILL; sn++)
	    {
	    	if (ch->pcdata->innate_timer[sn] <= 0) continue;
		ch->pcdata->innate_timer[sn] -= 1;
	    	if (ch->pcdata->innate_timer[sn] <= 0
			&& ch->pcdata->innate[sn] < ch->pcdata->innate_max[sn])
		{
		    ch->pcdata->innate[sn] += 1;
		    sprintf(buf, "You can now cast %s again.\n\r",skill_table[sn].name);
		    if (ch->pcdata->innate[sn] < ch->pcdata->innate_max[sn])
			ch->pcdata->innate_timer[sn] = ch->pcdata->innate_regain[sn];
		    else ch->pcdata->innate_timer[sn] = 0;
		    stc(buf, ch);
		}
	    }
	}

	/* Lets do some wacky weather */
  	if (weird_weather_type == WEIRD_WEATHER_LIGHT)
	{
	    if (dice(1, 4) == 1)
		stc("Beams of multi-coloured light rain from the sky.\n\r", ch);
	    /* The rest of this is in psp_gain */
	}


        WRITE_STR(debug_last_command, "spell regeneration (char_update)");

        /* Ok, this lets all those demons regen from blooood */
 	if (ch->secondaryrace == RACE_DAEMON) gain_hp(ch, ch->in_room->gore);

	if ( ch->timer > 30 )
	    ch_quit = ch;

	if (is_carrying_artifact(ch, ARTIFACT_STAFF_WIZARDRY) != NULL) gain = 8;
	if (IS_FLAG4(ch, AFF_PSI_RECALL)) gain += dice(1, 3);


	if (!IS_NPC(ch) && ch->position == POS_SLEEPING)
	{
	    bool found;
	    for (k = 1 ; k < dice(2, gain) ; k++)
	    {
		found = FALSE;
	        for (i=1 ; i < 11 ; i++ )
	        {
		    for (j = 0 ; j < 10 ; j++)
		    {
		        if (!found
			    && ch->pcdata->last_memorized[i][j] != -1
			    && ch->pcdata->memorize[i][j] == -1
			    && dice(1, 2) == 1)
			{
			    ch->pcdata->memorize[i][j] =
				ch->pcdata->last_memorized[i][j];
			    sprintf(buf, "You may now cast the spell of %s.\n\r", skill_table[ch->pcdata->memorize[i][j]].name);
			    send_to_char(buf, ch);
			    found = TRUE;
			}
		    }
		}
	    }
	}

	crash("weird weather in char update");
  	
	if (weird_weather_type == WEIRD_WEATHER_HURRICANE && !is_item(ch))
	{
	    if(dice(1, 3) == 1)
	    {
	  	stc("The fierce hurricane blows you around.\n\r", ch);
		move_char(ch, dice(1, 4), FALSE);
		stc("You fall flat on your face.\n\r", ch);
		ch->position = POS_SITTING;
		update_pos(ch);
	    }
	}

        WRITE_STR(debug_last_command, "limb regeneration (char_update)");

	if (((ch->secondaryrace == RACE_VAMPIRE
	    || ch->secondaryrace == RACE_LUPINE)
	    && ch->cur_form == 0
	    && !is_item(ch)
	    && ch->body_form != 0)
	    ||	(ch->position == POS_SLEEPING && ch->power[DISC_DRAG_CONJ] >= 3)
	    ||	(ch->power[DISC_DRAG_CONJ] >= 7)
	    || IS_FLAG6(ch, AFF6_REGROWTH))
	{
	    if (BODY_PART(ch, BODY_LEFT_ARM))
	    {
		stc("Your left arm grows back.\n\r", ch);
	 	act("$n's left arm grows back.", ch, NULL, NULL, TO_ROOM);
		REMOVE_BIT(ch->body_form, BODY_LEFT_ARM);
	    }
	    else if (BODY_PART(ch, BODY_RIGHT_ARM))
	    {
		stc("Your right arm grows back.\n\r", ch);
	 	act("$n's right arm grows back.", ch, NULL, NULL, TO_ROOM);
		REMOVE_BIT(ch->body_form, BODY_RIGHT_ARM);
	    }
	    else if (BODY_PART(ch, BODY_RIGHT_LEG))
	    {
		stc("Your right leg grows back.\n\r", ch);
	 	act("$n's right leg grows back.", ch, NULL, NULL, TO_ROOM);
		REMOVE_BIT(ch->body_form, BODY_RIGHT_LEG);
	    }
	    else if (BODY_PART(ch, BODY_LEFT_LEG) )
	    {
		stc("Your left leg grows back.\n\r", ch);
	 	act("$n's left leg grows back.", ch, NULL, NULL, TO_ROOM);
		REMOVE_BIT(ch->body_form, BODY_LEFT_LEG);
	    }
	    else if (BODY_PART(ch, BODY_RIGHT_HAND))
	    {
		stc("Your right hand grows back.\n\r", ch);
	 	act("$n's right hand grows back.", ch, NULL, NULL, TO_ROOM);
		REMOVE_BIT(ch->body_form, BODY_RIGHT_HAND);
	    }
	    else if (BODY_PART(ch, BODY_LEFT_HAND))
	    {
		stc("Your left hand grows back.\n\r", ch);
	 	act("$n's left hand grows back.", ch, NULL, NULL, TO_ROOM);
		REMOVE_BIT(ch->body_form, BODY_LEFT_HAND);
	    }
	}

	if (ch->psi_level > 0)
	    psi_slowticker(ch);

	if (ch->tick_timer[TIMER_WAR_HORSE_GO] == 1 && IS_NPC(ch) )
	{
	    send_to_char("You gallop off into the distance.\n\r", ch);
	    act("$n gallops off into the distance.", ch, NULL, NULL, TO_ROOM);
	    extract_char(ch, TRUE);
	    continue;
	}

	if (ch->tick_timer[TIMER_TAINT] == 1)
   	{
	    if (ch->secondaryrace == RACE_VAMPIRE)
	  	stc("Your skin warms up again.\n\r", ch);	 	 	
	    else if (ch->secondaryrace == RACE_DAEMON)
	  	stc("You are no longer a devil.\n\r", ch);	 	 	
	    else if (ch->secondaryrace == RACE_WARLOCK)
	  	stc("You forget how to cast spells.\n\r", ch);	 	 	
	    else if (ch->secondaryrace == RACE_ELADRIN)
	  	stc("You turn back into an elf.\n\r", ch);	 	 	
	    taint_untransform(ch);
	}

	if (ch->tick_timer[TIMER_VAMPIRE_GROW] <= 1
	    && !IS_NPC(ch)
	    && ch->secondaryrace == RACE_VAMPIRE
	    && ch->generation < 12)
        {
	    SET_TIMER(ch, TIMER_VAMPIRE_GROW, 3000+ch->generation * 200);
	    ch->generation += 1;
	    ch->pcdata->damcap += ch->generation + 20;
 	    ch->xhitroll += dice(1, 8);
 	    ch->xdamroll += dice(1, 8);
	    stc("Your blood bubbles and boils with the force of Caine.\n\r", ch);
	}

	if (ch->tick_timer[TIMER_DRAGON_GROW] <= 1
	    && !IS_NPC(ch)
	    && ch->secondaryrace == RACE_DRAGON
	    && ch->generation < 12)
	{
	    SET_TIMER(ch, TIMER_DRAGON_GROW, 2000+ch->generation * 200);
	    ch->generation += 1;
	    act("$n starts to grow before your eyes.", ch, NULL, NULL, TO_ROOM);
	    stc("You start to grow and mature.\n\r", ch);

	    /* Add the new innate abilities */
 	    add_drag_innate(ch);

	    /* Adding all those nice extra attacks. */
	    switch(ch->generation)
	    {
	        case 2:
	        SET_BIT(ch->flag2, AFF_CLAW);
                send_to_char("You grow a pair of wicked claws.\n\r", ch);
                act("$n grows a pair of wicked claws.\n\r", ch, NULL, NULL, TO_ROOM);
	        break;

	        case 3:
                SET_BIT(ch->flag2, AFF_BITE);
                send_to_char("You grow a set of sharp teeth.\n\r", ch);
                act("$n grows a set of sharp teeth.\n\r", ch, NULL, NULL, TO_ROOM);
	        break;

	        case 5:
                SET_BIT(ch->flag2, AFF_TAIL);
                send_to_char("Your spine extends into a long tail.\n\r", ch);
                act("$n's spine extends into a long tail.\n\r", ch, NULL, NULL, TO_ROOM);
	        break;

	        case 6:
                SET_BIT(ch->flag2, AFF_WING);
                send_to_char("A pair of leathery wings grow out of your back.\n\r",ch);
                act("A pair of leathery wings grow out of $n's back.\n\r", ch, NULL, NULL, TO_ROOM);
	        break;

	        case 8:
                SET_BIT(ch->flag4, AFF_STEEL_CLAWS);
                send_to_char("Your claws harden to steel.\n\r", ch);   
	        break;
	    }
	    set_form(ch, get_normal_form(ch));
	}

        WRITE_STR(debug_last_command, "ticky timers (char_update)");

 	if (ch->tick_timer[TIMER_CAN_CALL_WAR_HORSE] == 1)
	    send_to_char("You may now call your war horse again.\n\r", ch);

 	if (ch->tick_timer[TIMER_CAN_POLYMORPH] == 1)
	    send_to_char("You may now polymorph again.\n\r", ch);

 	if (ch->tick_timer[TIMER_MAKE_SNOWMAN] == 1)
	    send_to_char("You are ready to make another snowman now.\n\r", ch);

 	if (ch->tick_timer[TIMER_SPHINX_ROAR] == 1)
	    send_to_char("You are ready to roar again.\n\r", ch);

 	if (ch->tick_timer[TIMER_NEWBIE_IMM] == 1)
	    send_to_char("You are no longer protected by the gods.\n\r", ch);

 	if (ch->tick_timer[TIMER_CAN_DO_NEXUS] == 1)
	    send_to_char("You may now create another nexus.\n\r", ch);

 	if (ch->tick_timer[TIMER_TREE] == 1 && ch->cur_form == FRM_TREE)
	{
	    send_to_char("You start to change back into humanoid form.\n\r", ch);
	    act("$n starts to change back into a humanoid form.",ch,NULL,NULL,TO_ROOM);
	    set_form(ch, get_normal_form(ch) );
	}

 	if (ch->tick_timer[TIMER_CAN_SHRIEK] == 1)
	    send_to_char("Your voice has recovered.\n\r", ch);

 	if (ch->tick_timer[TIMER_CAN_CALL_ROCKS] == 1)
	    send_to_char("You may now call rocks again.\n\r", ch);

 	if (ch->tick_timer[TIMER_HELLFIRE_SUMMON] == 1)
	    send_to_char("You may now summon hellfire once more.\n\r", ch);

 	if (ch->tick_timer[TIMER_ENTOMB] == 1)
	    send_to_char("You may now use the entomb ability again.\n\r", ch);

 	if (ch->tick_timer[TIMER_CAN_BREATHE_FROST] == 1)
	    send_to_char("You are ready to breathe frost again.\n\r", ch);

 	if (ch->tick_timer[TIMER_CAN_FEATHER] == 1)
	    send_to_char("You are ready to make another feather.\n\r", ch);

        WRITE_STR(debug_last_command, "growing arms (char_update)");

 	/* code to see if arms burst through stomach or not */
	    if (ch->tick_timer[TIMER_THIRD_ARM_GROWING] == 1)
	    {
	 	send_to_char("A third arm bursts out of your stomach.\n\r", ch);
		act("An arm bursts out of $n's stomach, spraying blood everywhere.", ch, NULL, NULL, TO_ROOM);
		ch->in_room->gore++;
		SET_BIT(ch->body_form, BODY_ENABLE_THIRD_ARM);
		SET_BIT(ch->body_form, BODY_THIRD_ARM);
		SET_BIT(ch->body_form, BODY_THIRD_HAND);
		SET_TIMER(ch, TIMER_THIRD_ARM_GOING, dice(3,10));
	    }

	    if (ch->tick_timer[TIMER_FOURTH_ARM_GROWING] == 1)
	    {
	 	send_to_char("Another arm bursts out of your stomach.\n\r", ch);
		act("An arm bursts out of $n's stomach, spraying blood everywhere.", ch, NULL, NULL, TO_ROOM);
		ch->in_room->gore++;
		SET_BIT(ch->body_form, BODY_ENABLE_FOURTH_ARM);
		SET_BIT(ch->body_form, BODY_FOURTH_ARM);
		SET_BIT(ch->body_form, BODY_FOURTH_HAND);
		SET_TIMER(ch, TIMER_FOURTH_ARM_GOING, dice(3, 6) );
	    }

	    if (ch->tick_timer[TIMER_THIRD_ARM_GOING] == 1)
	    {
		send_to_char("You feel immense pain in your third arm.\n\r", ch);
		send_to_char(" ... it claws its way back into your stomach.\n\r", ch);
		act("$n's third arm claws its way back into $s stomach.", ch, NULL, NULL, TO_ROOM);
		REMOVE_BIT(ch->body_form, BODY_ENABLE_THIRD_ARM);
		REMOVE_BIT(ch->body_form, BODY_THIRD_ARM);
		REMOVE_BIT(ch->body_form, BODY_THIRD_HAND);
		if ((obj = get_eq_char(ch, WEAR_THIRD_HAND) ) != NULL)
		    lose_obj(ch, obj);
	    }
	    if (ch->tick_timer[TIMER_FOURTH_ARM_GOING] == 1)
	    {
		send_to_char("You feel immense pain in your fourth arm.\n\r", ch);
		send_to_char("... it claws its way back into your stomach.\n\r", ch);
		act("$n's fourth arm claws its way back into $s stomach.", ch, NULL, NULL, TO_ROOM);
		REMOVE_BIT(ch->body_form, BODY_FOURTH_ARM);
		REMOVE_BIT(ch->body_form, BODY_FOURTH_HAND);
		REMOVE_BIT(ch->body_form, BODY_ENABLE_FOURTH_ARM);
		if ((obj = get_eq_char(ch, WEAR_FOURTH_HAND)) != NULL)
		    lose_obj(ch, obj);
	    }


	for (i = 0; i < MAX_TIMER; i++)
	    if (ch->tick_timer[i] > 0) ch->tick_timer[i] -= 1;

	if (IS_FLAG3(ch, AFF_MUMMY_ROT) && is_item(ch))
	{
	    int loss;

	    rand_to_char(ch, "The skin peels from your face.\n\r",
		"Your flesh begins to boil.\n\r",
		"Your hair starts to fall out.\n\r",
		"Your face bubbles and blisters.\n\r",
		"You start to cough up blood and mucus.\n\r");
	    loss = dice(1, 6);
	    ch->max_hit -= loss;
	    if (!IS_NPC(ch))
	    {
		ch->pcdata->perm_hit -= loss;
		if (ch->pcdata->perm_hit < 10) ch->pcdata->perm_hit = 10;
  	    }

	    act("$n's skin starts to rot and decay.", ch, NULL,NULL,TO_ROOM);
	    if (ch->max_hit < 10) ch->max_hit = 10;
	    if (ch->hit > ch->max_hit) ch->hit = ch->max_hit;
	    if (dice(1,20) == 1) ch->perm_stat[STAT_CHA] -= 1;
	    if (ch->perm_stat[STAT_CHA] < 3) ch->perm_stat[STAT_CHA] = 3;
	}

	if (IS_FLAG3(ch, AFF_BLOOD_SPEW) && is_item(ch))
	{
	    int loss;
	    OBJ_DATA *obj;
	    rand_to_char(ch, "Blood trickles out of your nose.\n\r",
		"You puke blood all over the floor.\n\r",
		"Blood flows out of your ears.\n\r",
		"Your blood drains out of your skin, all over the floor.\n\r",
		"You start to cough up blood and vomit.\n\r");
	    act("$n coughs up blood all over the floor.", ch, NULL, NULL,TO_ROOM);
	    loss = dice(1, 10);
	    if (!IS_NPC(ch))
	    {
	 	ch->pcdata->perm_hit -= loss;
		if (ch->pcdata->perm_hit < 10) ch->pcdata->perm_hit = 10;
	    }
	    ch->max_hit -= loss;
	    if (ch->max_hit < 10) ch->max_hit = 10;
	    if (ch->hit > ch->max_hit) ch->hit = ch->max_hit;
	    obj = create_object(get_obj_index(OBJ_VNUM_BLOOD), 0);
	    obj->timer = dice(3,3);
	    obj_to_room(obj, ch->in_room, ch->altitude);
	}
	
	if (ch->secondaryrace == RACE_VAMPIRE
	    && !IS_NPC(ch) && ch->desc != NULL
	    && !is_item(ch)
	    && !IS_FLAG4(ch, AFF_STASIS)
	    && !IS_FLAG2(ch, AFF_EARTH_MELD)
	    && ch->level < 120)
	{
	    ch->blood -= ch->upkeep / 2;
	    
	    if (ch->blood <= 10)
	    {
		send_to_char("You are dying from lack of blood.\n\r",ch);
		damage_old(ch, ch, dice(4, 20), 0, DAM_FIRE, FALSE);
		if (ch->blood < 0) ch->blood = 0;
	    }
	}
	    
	if ( ch->position >= POS_STUNNED )
	{
	    /* check to see if we need to go home */
	    if (IS_NPC(ch) && ch->zone != NULL && ch->zone != ch->in_room->area
	    && ch->desc == NULL &&  ch->fighting == NULL 
	    && !IS_AFFECTED(ch,AFF_CHARM) && number_percent() < 5)
	    {
		act("$n wanders on home.",ch,NULL,NULL,TO_ROOM);
		extract_char(ch,TRUE);
		continue;
	    }

	    if (ch->secondaryrace != RACE_VAMPIRE
		&& ch->tertiaryrace != TERTIARY_LICH)
	    {
	        if ( ch->hit  < ch->max_hit )
		    ch->hit  += hit_gain(ch);
	        else
		    ch->hit = ch->max_hit;

	        if ( ch->psp < ch->max_psp )
		    ch->psp += psp_gain(ch);

	        if ( ch->move < ch->max_move )
		    ch->move += move_gain(ch);
	        else
		    ch->move = ch->max_move;
	    }
	}

        WRITE_STR(debug_last_command, "natural regen (char_update)");

	if ( ch->position < POS_SLEEPING && ch->secondaryrace!=0)
	    update_pos( ch );

	if (ch->secondaryrace!=0 && ch->hit<1)
	{
	    ch->hit+=3;
	    if (ch->hit>0)
	    {
		send_to_char("You clamber back up to your feet.\n\r",ch);
		act("$n clambers back to $s feet.",ch,NULL,NULL,TO_ROOM);
	    }
	    if (ch->hit>-4 && ch->hit<1)
	    {
		send_to_char("Your wounds start to close over and stop bleeding.\n\r",ch);
		act("$n's wounds start to close over and stop bleeding.",ch,NULL,NULL,TO_ROOM);
	    }
	    update_pos(ch);
	    if (ch->hit > 0 && ch->position == POS_STUNNED)
		ch->position = POS_RESTING;
	}

	if ( !IS_NPC(ch) && ch->level < LEVEL_IMMORTAL )
	{
	    OBJ_DATA *obj;

	    if ( ( obj = get_obj_by_type( ch, ITEM_LIGHT ) ) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room != NULL )
		{
		    --ch->in_room->light;
		    act( "$p goes out.", ch, obj, NULL, TO_ROOM );
		    act( "$p flickers and goes out.", ch, obj, NULL, TO_CHAR );
		    extract_obj( obj );
		}
		else if ( obj->value[2] <= 5 && ch->in_room != NULL)
		    act("$p flickers.",ch,obj,NULL,TO_CHAR);
	    }

	    if (IS_IMMORTAL(ch))
		ch->timer = 0;

	    if ( ch->fighting != NULL && ch->timer > 15) ch->timer = 15;

	    if ( ++ch->timer >= 20 && ch->timer < 24)
	    {
		stcf(ch, "Your connection will time out in %d ticks.\n\r",
			24 - ch->timer);
	    }
	    if ( ch->timer >= 24 )
	    {
		if ( ch->was_in_room == NULL && ch->in_room != NULL )
		{
		    ch->was_in_room = ch->in_room;
		    if ( ch->fighting != NULL )
			stop_fighting( ch, TRUE );
		    if ( ch->visionary == NULL )
		    {
			char log_buf[200];
			DESCRIPTOR_DATA *d;
			DESCRIPTOR_DATA *d_next;
			CHAR_DATA *tch;
			int id;

			if (ch->riding != NULL)
			{
			    ch->riding->riden_by = NULL;
			    ch->riding = NULL;
			}
			if (ch->riden_by != NULL)
			{
			    ch->riden_by->riding = NULL;
			    ch->riden_by = NULL;
			}

    			if (ch->shadow_victim != NULL)
    			{
    			    ch->shadow_victim->shadow_master = NULL;
    			    ch->shadow_victim = NULL;
    			    REMOVE_BIT(ch->flag4, AFF_SHADOW_HIDE);
    			}

    			if (ch->shadow_master != NULL)
    			{
    			    ch->shadow_master->shadow_victim = NULL;
    			    REMOVE_BIT(ch->shadow_master->flag4, AFF_SHADOW_HIDE);
			    ch->shadow_master = NULL;
		    	}

			obj_exorcise( ch );

			sprintf( log_buf, "Connection timed out for %s.", GET_PROPER_NAME( ch ) );
			log_string( log_buf );
			wiznet("$N is returned to the real world by the mists.", ch, NULL, WIZ_LOGINS, 0, get_trust(ch) );
                                            
		    	act( "$n is surrounded by mist, and slowly vanishes from sight.",
			    ch, NULL, NULL, TO_ROOM );
		    	send_to_char( "You are surrounded by mist, and transported back to reality.\n\r", ch );
			do_psioff( ch, "all" );
		    	if (ch->registerred == 1) save_char_obj( ch );
		    	id = ch->id;
			d = ch->desc;
			extract_char( ch, TRUE );
			if ( d != NULL )
			    close_socket( d );
			for (d = descriptor_list; d != NULL; d = d_next)
			{
			    d_next = d->next;
			    tch = d->original ? d->original : d->character;
			    if (tch && tch->id == id)
			    {
				extract_char(tch, TRUE);
				close_socket(d);
			    }
			}
			continue;
		    }
		}
	    }

	    gain_condition( ch, COND_DRUNK,  -1 );

	if (ch->secondaryrace != RACE_VAMPIRE && ch->level > 2
	    && ch->secondaryrace != RACE_DAEMON && 1 == 0) /* Temp fix */
	{
	    gain_condition( ch, COND_FULL, get_size(ch) > SIZE_MEDIUM ? -4 : -2 );
	    gain_condition( ch, COND_THIRST, -1 );
	    gain_condition( ch, COND_HUNGER, get_size(ch) > SIZE_MEDIUM ? -2 : -1);
	}
	}
        WRITE_STR(debug_last_command, "fading spells (char_update)");

	for ( paf = ch->affected; paf != NULL; paf = paf_next )
	{
	    paf_next    = paf->next;
	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  /* spell strength fades with time */
	    }
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			if (paf->end_message != NULL)
			    stc(paf->end_message, ch);
			else send_to_char( skill_table[paf->type].msg_off, ch );
			send_to_char( "\n\r", ch );
		    }
		}
	  
		affect_remove( ch, paf );
	    }
	}

	/*
	 * Careful with the damages here,
	 *   MUST NOT refer to ch after damage taken,
	 *   as it may be lethal damage (on NPC).
	 */
        WRITE_STR(debug_last_command, "plague and poison (char_update)");

	if ( (is_affected(ch, gsn_plague) )
	   && ch != NULL)
	{
	    AFFECT_DATA *af, plague;
	    CHAR_DATA *vch;
	    int dam;

	    if (ch->hit < 0)
	    {
		REMOVE_BIT(ch->affected_by, AFF_PLAGUE);
	    }			

	    if (is_null_room(ch))
		return;
	    
	    act("$n writhes in agony as plague sores erupt from $s skin.",
		ch,NULL,NULL,TO_ROOM);
	    send_to_char("You writhe in agony from the plague.\n\r",ch);
	    for ( af = ch->affected; af != NULL; af = af->next )
	    {
		if (af->type == gsn_plague)
		    break;
	    }
	
	    if (af == NULL)
	    {
		REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
		return;
	    }
	
	    if (af->level == 1)
		return;
	
	    plague.where                = TO_AFFECTS;
	    plague.type                 = gsn_plague;
	    plague.level                = af->level - 1; 
	    plague.duration     = number_range(1,2 * plague.level);
	    plague.location             = APPLY_STR;
	    plague.modifier     = -5;
	    plague.bitvector    = AFF_PLAGUE;
	
	    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	    {
		if (vch->altitude != ch->altitude) continue;
		if (!saves_spell(plague.level - 2,vch,DAM_DISEASE) 
		&&  !IS_IMMORTAL(vch)
		&&  !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(4) == 0)
		{
		    send_to_char("You feel hot and feverish.\n\r",vch);
		    act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
		    affect_join(vch,&plague);
		}
	    }

	    dam = UMIN(ch->level,af->level/4+1);
	    ch->move -= dam;
	    damage_old( ch, ch, dam, gsn_plague,DAM_DISEASE,FALSE);
	}
	else if ( IS_AFFECTED(ch, AFF_POISON) && ch != NULL
	     &&   !IS_AFFECTED(ch,AFF_SLOW))

	{
	    AFFECT_DATA *poison;

	    poison = affect_find(ch->affected,gsn_poison);

	    if (poison != NULL)
	    {
		act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "You shiver and suffer.\n\r", ch );
		damage_old(ch,ch,poison->level/10 + 1,gsn_poison,
		    DAM_POISON,FALSE);
	    }
	}

	else if ( ch->position == POS_INCAP && number_range(0,1) == 0 && ch->secondaryrace == 0)
	{
	    damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE);
	}
	else if ( ch->position == POS_MORTAL && ch->secondaryrace==0)
	{
	    damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE);
	}
    }

    WRITE_STR(debug_last_command, "autosave (char_update)");

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if (is_null_room(ch)) continue;
	if (IS_NPC(ch)) continue;

	if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number
	    && ch->registerred == 0)
	{
	    sprintf(buf, "Autosaving %s.", GET_PROPER_NAME(ch));
	    bug(buf, 0);
            WRITE_STR(debug_last_command, buf);
	    save_char_obj(ch);
	}

	if (ch == ch_quit)
 	{
	    DESCRIPTOR_DATA *d;
	    for ( d = descriptor_list; d != NULL; d = d->next )
	    {
		if ( d == ch->desc )
		{
		    crash("player gone idle");
		    stc("Hello?  Are you there?\n\r", d->character);
		    stc("Hmm... obviously not.  Oh well.\n\r", d->character);
		    save_char_obj(ch);
	 	    do_quit(ch, "");
		}
	    }
	}
    }
    return;
}




/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf, *paf_next;

    WRITE_STR(debug_last_command, "obj_update)");

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	CHAR_DATA *rch;
	char *message;

	obj_next = obj->next;

	/* go through affects and decrement */
	for ( paf = obj->affected; paf != NULL; paf = paf_next )
	{
	    paf_next    = paf->next;
	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  /* spell strength fades with time */
	    }
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_obj )
		    {
			if (obj->carried_by != NULL)
			{
			    rch = obj->carried_by;
			    act(skill_table[paf->type].msg_obj,
				rch,obj,NULL,TO_CHAR);
			}
			if (obj->in_room != NULL 
			&& obj->in_room->people != NULL)
			{
			    rch = obj->in_room->people;
			    act(skill_table[paf->type].msg_obj,
				rch,obj,NULL,TO_ALL);
			}
		    }
		}

		affect_remove_obj( obj, paf );
	    }
	}


	if ( obj->timer <= 0 || --obj->timer > 0 )
	    continue;

	switch ( obj->item_type )
	{
	default:              message = "$p crumbles into dust.";  break;
	case ITEM_FOUNTAIN:   message = "$p dries up.";         break;
	case ITEM_CORPSE_NPC: message = "$p decays into dust."; break;
	case ITEM_CORPSE_PC:  message = "$p decays into dust."; break;
	case ITEM_FOOD:       message = "$p decomposes.";       break;
	case ITEM_POTION:     message = "$p has evaporated from disuse.";       
								break;
	case ITEM_PORTAL:     message = "$p fades out of existence."; break;
	case ITEM_CONTAINER: 
	    if (CAN_WEAR(obj,ITEM_WEAR_FLOAT))
		if (obj->contains)
		    message = 
		"$p flickers and vanishes, spilling its contents on the floor.";
		else
		    message = "$p flickers and vanishes.";
	    else
		message = "$p crumbles into dust.";
	    break;
	case ITEM_DRAGON_BLOOD:  message = "$p drains away into the ground."; break;
	case ITEM_RAINBOW_POOL:  message = "$p soaks away into the ground."; break;
	}

	if ( obj->carried_by != NULL
	    && !IS_OBJ_STAT2(obj, ITEM_DAEMONSEED)  )
	{
	    if (IS_NPC(obj->carried_by) 
	    &&  obj->carried_by->pShop != NULL)
		obj->carried_by->silver += obj->cost/5;
	    else
	    {
		act( message, obj->carried_by, obj, NULL, TO_CHAR );
		if ( obj->wear_loc == WEAR_FLOAT)
		    act(message,obj->carried_by,obj,NULL,TO_ROOM);
	    }
	}
	else if ( obj->in_room != NULL
	&&      ( rch = obj->in_room->people ) != NULL
	&&	 !IS_OBJ_STAT2(obj, ITEM_DAEMONSEED) )
	{
	    if (! (obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
		   && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))
	    {
		act( message, rch, obj, NULL, TO_ROOM );
		act( message, rch, obj, NULL, TO_CHAR );
	    }
	}

	if ((obj->item_type == ITEM_CORPSE_PC || obj->wear_loc == WEAR_FLOAT)
	&&  obj->contains)
	{   /* save the contents */
	    OBJ_DATA *t_obj, *next_obj;

	    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
	    {
		next_obj = t_obj->next_content;
		obj_from_obj(t_obj);

		if (obj->in_obj) /* in another object */
		    obj_to_obj(t_obj,obj->in_obj);

		else if (obj->carried_by)  /* carried */
		    if (obj->wear_loc == WEAR_FLOAT)
			if (obj->carried_by->in_room == NULL)
			    extract_obj(t_obj);
			else
			    obj_to_room(t_obj,obj->carried_by->in_room, obj->altitude);
		    else
			obj_to_char(t_obj,obj->carried_by);

		else if (obj->in_room == NULL)  /* destroy it */
		    extract_obj(t_obj);

		else /* to a room */
		    obj_to_room(t_obj,obj->in_room, obj->altitude);
	    }
	}

	if (IS_OBJ_STAT2(obj, ITEM_DAEMONSEED)
	    && obj != NULL
	    && obj->in_obj == NULL
	    && (locate_obj(obj))->people )
	{
	    char buf[MAX_STRING_LENGTH];
	    CHAR_DATA *vch;

	    sprintf(buf, "%s suddenly explodes in a ball of flame, incinerating you!\n\r",obj->short_descr);
	    buf[0] = UPPER(buf[0]);
	    if ((locate_obj(obj))->people == NULL) break;
	    for ( vch = (locate_obj(obj))->people; vch != NULL;vch=vch->next_in_room )
    	    {
		if (vch->altitude != obj->altitude) continue;
		if (vch->secondaryrace == 0) continue;

                damage_old(vch,vch,obj->level+ dice(12, 50), 0,DAM_FIRE,FALSE); 
		send_to_char(buf, vch);
 	    }
	}

	extract_obj( obj );
    }

    return;
}



/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update( void )
{
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;

    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;
	if ( IS_NPC(wch)
	||   wch->level >= LEVEL_IMMORTAL
	||   wch->in_room == NULL 
	||   wch->in_room->area->empty)
	    continue;

	for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
	{
	    int count;

	    ch_next     = ch->next_in_room;

	    if (ch->altitude != wch->altitude) continue;
	    if ( !IS_NPC(ch)
	    ||   !IS_SET(ch->act, ACT_AGGRESSIVE)
	    ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	    ||   IS_AFFECTED(ch,AFF_CALM)
	    ||   ch->fighting != NULL
	    ||   IS_AFFECTED(ch, AFF_CHARM)
	    ||   !IS_AWAKE(ch)
	    ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	    ||   !can_see( ch, wch ) 
	    ||   number_bits(1) == 0)
		continue;

	    /*
	     * Ok we have a 'wch' player character and a 'ch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     */
	    count       = 0;
	    victim      = NULL;
	    for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	    {
		vch_next = vch->next_in_room;
	    	if (ch->altitude != wch->altitude) continue;

		if ( !IS_NPC(vch)
		&&   vch->level < LEVEL_IMMORTAL
		&&   ch->level >= vch->level - 5 
		&&   ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
		&&   can_see( ch, vch ) )
		{
		    if ( number_range( 0, count ) == 0 )
			victim = vch;
		    count++;
		}
	    }

	    if ( victim == NULL )
		continue;
/* berserk is evil -- whois
	    if (dice(1, 3) == 2)
		do_berserk(ch, "");
	    else
 */
		set_fighting( ch, victim );
	}
    }

    return;
}



/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler( void )
{
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_violence;
    static  int     pulse_second;
    static  int     pulse_point;

    if ( --pulse_area     <= 0 )
    {
	pulse_area      = PULSE_AREA;
	/* number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 ); */
	area_update     ( );
    }

    if ( --pulse_mobile   <= 0 )
    {
	pulse_mobile    = PULSE_MOBILE;
	mobile_update   ( );
    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence  = PULSE_VIOLENCE;
	violence_update ( );
    }

    if ( --pulse_second <= 0 )
    {
	pulse_second  = PULSE_PER_SECOND;
	second_update ( );
    }

    if ( --pulse_point    <= 0 )
    {
	wiznet("Tick!",NULL,NULL,WIZ_TICKS,0,0);
	pulse_point     = number_range( PULSE_TICK / 2, 3 * PULSE_TICK / 2 );
	weather_update  ( );
	char_update     ( );
	obj_update      ( );
	room_update	( );
    }

    aggr_update( );
    tail_chain( );
    return;
}

void second_update( )
{
    CHAR_DATA *ch, *victim, *chnext;

    crash("second update");

    for (ch = char_list; ch != NULL; ch = chnext)
    {
	chnext = ch->next;

	psi_fastticker( ch );

	if (ch->power[DISC_DRAG_CONJ] >= 3 && ch->position == POS_SLEEPING)
	{
	   ch->hit += 10;
	   if (ch->hit > ch->max_hit) ch->hit = ch->max_hit;
	   ch->move += 10;
	   if (ch->move > ch->max_move) ch->move = ch->max_move;
	}

	if (ch->power[DISC_DRAG_CONJ] >= 7)
        {
	    if (ch->hit > 1)
	    {
	   	ch->hit += dice(1,5);
	   	if (ch->hit > ch->max_hit) ch->hit = ch->max_hit;
	   	ch->move += dice(1,5);
	   	if (ch->move > ch->max_move) ch->move = ch->max_move;
	    } else
	    {
		ch->hit += 1;
		update_pos( ch );
	    }
	}

        crash("second update 2");
       

 	if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
	    continue;

	if (IS_AWAKE(ch) && ch->in_room == victim->in_room
		&& ch->altitude == victim->altitude)
	    char_attack(ch);
	else
	    stop_fighting(ch, FALSE);

	if ((victim = ch->fighting) == NULL)
	    continue;
       
        crash("second update 3");

	check_group_combat(ch, victim);

    }

    crash("second update 4");

    if (count_down > -1) 
    { 
        if (count_down == 0) 
        { 
            char buf[80]; 
            sprintf(buf, "#1--#3-#7>#3 Time to %s", count_down_to); 
            informational(buf, -1, -1); 
        } 
        else if (count_down < 11
	|| ((count_down / 10 * 10) == count_down && count_down <= 60))
        { 
            char buf[80]; 
            sprintf(buf, "#1--#3-#7>#3 %d seconds to %s",count_down,count_down_to) ;
           informational(buf, -1, -1); 
        } 
	else if ((count_down / 60 * 60) == count_down)
	{
            char buf[80]; 
            sprintf(buf, "#1--#3-#7>#3 %d minutes to %s",count_down/60,count_down_to) ;
           informational(buf, -1, -1); 
	}
	count_down -= 1;
    }  
    crash("second update 5");
}

void do_necromancy(CHAR_DATA *ch, char *argument)
{ 
    OBJ_DATA *obj;
    bool found;
    int j, k;

    if (IS_NPC(ch)) return;
  
    if (argument[0] == '\0')
    {
	stc("What corpse do you wish to use necromancy on?\n\r", ch);
	return;
    }

    stc("not implemented yet.\n\r", ch);
/*
    for (k = 1 ; k < dice(2, gain) ; k++)
    {
	found = FALSE;
        for (i=1 ; i < 11 ; i++ )
        {
	    for (j = 0 ; j < 10 ; j++)
	    {
	        if (!found
		    && ch->pcdata->last_memorized[i][j] != -1
		    && ch->pcdata->memorize[i][j] == -1
		    && dice(1, 2) == 1)
		{
		    ch->pcdata->memorize[i][j] =
			ch->pcdata->last_memorized[i][j];
		    sprintf(buf, "You may now cast the spell of %s.\n\r", skill_table[ch->pcdata->memorize[i][j]].name);
		    send_to_char(buf, ch);
		    found = TRUE;
		}
	    }
	}
    }
*/
}

