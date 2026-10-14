/*
 * HELL, formerly known as BTL/Darkheart, by Adam Powell and Chris Mellish,
 *  1996 to 1998.  If you reproduce this code in any way please give credit
 *  to the writers.  Thanks.
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "hell.h"
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN(interpret        );
DECLARE_DO_FUN(do_dream         );
DECLARE_DO_FUN(do_stand         );
DECLARE_DO_FUN(do_say           );
DECLARE_DO_FUN(do_emote         );
DECLARE_DO_FUN(do_berserk       );
DECLARE_DO_FUN(do_bash          );
DECLARE_DO_FUN(do_trip          );
DECLARE_DO_FUN(do_dirt          );
DECLARE_DO_FUN(do_flee          );
DECLARE_DO_FUN(do_kick          );
DECLARE_DO_FUN(do_disarm        );
DECLARE_DO_FUN(do_get           );
DECLARE_DO_FUN(do_yell          );
DECLARE_DO_FUN(do_sacrifice     );
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_wear          );
DECLARE_DO_FUN(do_crush         );
DECLARE_DO_FUN(do_quit          );


/*
 * Local functions.
 */
void	check_quest_kill args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
void    exec_fear       args( ( CHAR_DATA *ch ) );
void    do_bite_atk     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    do_claw_atk     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    do_wing_atk     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    do_horn_atk     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    do_tail         args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    check_assist    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_dodge     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    check_killer    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_shield_block     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    dam_message     args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, bool immune ) );
void    death_cry       args( ( CHAR_DATA *ch, bool corpse ) );
int     xp_compute      args( ( CHAR_DATA *gch, CHAR_DATA *victim, 
			    int total_levels ) );
bool    is_safe         args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    make_corpse     args( ( CHAR_DATA *ch ) );
void    one_hit         args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void    mob_hit         args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void    disarm          args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	petrify		args( (CHAR_DATA *ch, CHAR_DATA *victim) );
void	turn_to_stone	args( (CHAR_DATA *ch) );
void    wipe_spells	args( (CHAR_DATA *victim) );

char *  const   direc_name        []              =
{
    "north", "east", "south", "west"
};

char *  const   direc2_name        []              =
{
    "south", "west", "north", "east"
};


/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *room;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;
    PROG_DATA *prog, *prg;
    OBJ_DATA *obj, *obj_next;
    int sn, dam, i;
    
    crash("violence update has started");

    /* Extract objects that have been damaged */
    for ( obj = object_list ; obj != NULL ; obj = obj_next )
    {
	obj_next = obj->next;

	if (!IS_OBJ_STAT3(obj, ITEM_DESTROYED)) continue;
	extract_obj(obj);
    }

    sn = skill_lookup("conflaguration");

    for ( prog = prog_list; prog != NULL; prog = prg )
    {
	crash("run mobprogs");
	prg = prog->next;
	prog->frozen += 1;
	if (prog->frozen == 1000)
	{
	    terminate_prog( prog, TRUE );
	    continue;
	}
	if (prog->pause == 0 && prog->frozen < 2)
	    run_prog( prog );
	else
	    prog->pause -= 1;
    }

    crash("weird weather in violence update.");

    /* 33 frog 112 fish */
    if (weird_weather_type == WEIRD_WEATHER_FROGS)
    {
	if (dice(1, 3) == 1) gecho("You hear croaking from the skies.\n\r");

	for (i = 0 ; i < dice(3, 8) ; i++)
	{
	    room = get_rand_room();
	    victim = create_mobile(get_mob_index(33));
	    char_to_room(victim, room, 0);
	    room_message(room, "A frog falls from the sky.");
	}
    }


    if (weird_weather_type == WEIRD_WEATHER_FISH)
    {
	for (i = 0 ; i < dice(3, 8) ; i++)
	{
	    room = get_rand_room();
	    obj = create_object(get_obj_index(112), 1);
	    obj_to_room(obj, room, 0);
	    obj->timer = 20;
	    room_message(room, "A fish falls from the sky onto your head.  OUCH!");
	}
    }

    crash("Its autoquit for LD players.  Serve them right.");

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if (IS_FLAG5(ch, AFF5_SWITCHED)) continue;

	if (!IS_NPC(ch) && ch->desc == NULL)
	{
	    stc("Link dead huh?   Bye.\n\r", ch);
	    do_quit(ch, "");
	}
    }
	
    crash("Bloody zolo and his blinking has crashed it!");

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

 	if (ch->blinkykill == NULL) continue;

	if (IS_SET(ch->flag5, AFF5_BLINK_1ST_RD))
	{
	    REMOVE_BIT(ch->flag5, AFF5_BLINK_1ST_RD);
	    SET_BIT(ch->flag5, AFF5_BLINK_2ND_RD);
	}
	else if (IS_SET(ch->flag5, AFF5_BLINK_2ND_RD))
	{
	    int dist = 3;

	    REMOVE_BIT(ch->flag5, AFF5_BLINK_2ND_RD);
	    REMOVE_BIT(ch->flag5, AFF5_BLINK_1ST_RD);

	    /* Has the victim left? */
	    if (ch->blinkykill->in_room != ch->in_room
		|| ch->blinkykill->altitude != ch->altitude)
	    {
		ch->blinkykill = NULL;
	 	act("$n pops back into existence.", ch, NULL, NULL, TO_ROOM);
		stc("You pop back into existence.\n\r", ch);
		stc("Your victim is no longer in the room.\n\r", ch);
		break;
	    }

	    dist = dice(1, 3) + 1;

	    act("$n pops back into existence next to $N.", ch, NULL,ch->blinkykill, TO_NOTVICT);
	    act("You pop back into existence next to $N.", ch, NULL, ch->blinkykill, TO_CHAR);
	    act("$n pops back into existence next to you.", ch, NULL, ch->blinkykill, TO_VICT);
	    set_fighting(ch, ch->blinkykill);
	    ch->blinkykill->distance = dist;
	    ch->distance = dist;
	    do_say(ch, "Muhahahahahaha");
	    ch->blinkykill = NULL;

	}
    }


/*    crash("Super mobs!  Love 'em. - artificial intellegence routines");

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	
    }*/

    crash("ok this is the beast thang.");

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	CHAR_DATA *dch; /* That means dead character btw */
	CHAR_DATA *dch_next;

 	ch_next = ch->next;

	if (ch->in_room == NULL)
	{
	    char buf[MAX_STRING_LENGTH];
	    sprintf(buf, "%s is in a NULL room..... and its going to crash.", GET_NAME(ch));
	    char_to_room(ch, get_room_index( prime_realm, 3054 ), 0);
	    gecho(buf);
	    continue;
	}

	crash("autolooking");

	if (ch->auto_look_line != -1)
	    do_look(ch, "%autolooking");

	crash("beast autostand");

        /* This is autostand stuff. for fighting */
        if (ch->fighting != NULL
	    && (ch->position == POS_SITTING || ch->position == POS_RESTING) )
 	    do_stand(ch, "");

        if (!IS_FLAG5(ch, AFF5_BEAST)) continue;

	crash("beast autogrowl");

	if (dice(1, 3) == 1) switch(dice(1, 7))
	{
	    case 1 : interpret(ch, "growl"); break;
	    case 2 : interpret(ch, "snarl"); break;
	    case 3 : interpret(ch, "kitty"); break;
	    case 4 : interpret(ch, "explode"); break;
	    case 5 : interpret(ch, "peer"); break;
	    case 6 : interpret(ch, "peer"); break;
	    case 7 : interpret(ch, "grin"); break;
	}

	if (ch->position == POS_SLEEPING)
	    do_dream(ch, "monsters");

	if (ch->position == POS_RESTING)
	    do_stand(ch, "");

	crash("beast autoattack");

	for (dch = ch->in_room->people ; dch != NULL ; dch = dch_next )
	{
	    dch_next = dch->next_in_room;

	    if (is_item(dch)) continue;
	    if (is_null_room(dch)) continue;

	    if (dch != ch && dice(1, 4) == 1)
	    {
		stc("You lust for BLOOOOOOD!!!!\n\r", ch);
		do_say(ch, "DIE!");
		act("You lunge at $N.", ch, NULL, dch, TO_CHAR);
		act("$n lunges at $N.", ch, NULL, dch, TO_NOTVICT);
		act("$n lunges at you.", ch, NULL, dch, TO_VICT);
		set_fighting(ch, dch);
		break;
	    }
	}
    }

    crash("Check who is embracing who");

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	/* Check who is embracing who and shit like that */
	if (ch->sucking != NULL)
	{
	    if (ch->fighting != NULL
		|| ch->in_room != ch->sucking->in_room
		|| ch->altitude != ch->sucking->altitude)
	    {
		REMOVE_BIT(ch->sucking->flag4, AFF_BEING_SUCKED);
		ch->sucking = NULL;
	    }
	}

        if (weird_weather_type == WEIRD_WEATHER_TORNADO
	    && weird_weather_timer == 3
	    && !is_item(ch))
	    stc("The winds tear around you, knocking you about.\n\r", ch);
	
	if (ch->hit > 0 && ch->position == POS_STUNNED)
	{
	    ch->position = POS_RESTING;
	    send_to_char("You slump to the floor.\n\r", ch);
	    act("$n slumps to the floor.", ch, NULL, NULL, TO_ROOM);
	}

	if (ch->visionary != NULL && ch->vision > 0)
	{
	    show_vision(ch);
	}

        if (ch->rage > 0 && dice(1, 6) == 1)
        {
            int downv;
	    crash("rage");
            downv = dice(1, 4);

            if (ch->rage > downv)
            {
                ch->rage -= downv;
            }
            else
            {
                ch->rage = 0;
	        stc("You feel calmer.\n\r", ch);
	        unwerewolf(ch);
            }
        }

	update_pos(ch);
   
	ch_next = ch->next;
	if (IS_AFFECTED(ch, AFF_REGENERATION) && ch->hit < ch->max_hit )
	    ch->hit += 1;

	if (ch->secondaryrace == RACE_LUPINE
	    && IS_NIGHT()
	    && dice(1, 2) == 1
	    && ch->power[DISC_LUPI_LUNA] >= 3)
	    ch->hit = UMIN(ch->hit+dice(2, 2)+1, ch->max_hit);
	
	else if (ch->secondaryrace == RACE_LUPINE
	    && dice(1,2) == 1
	    && ch->hit < ch->max_hit)
	     ch->hit += 1;

	if (ch->sucking != NULL )
	{
	    int loss;
	    crash("embracing damage");

	    loss = 8 + dice(1, 4);

	    /* Animalism 6 doing the do */
	    if (IS_FLAG5(ch->sucking, AFF5_SUCCULENCE)) loss *= 2;

	    /* Satyralis 1 doing the do as well */
	    if (IS_FLAG5(ch, AFF5_MAW)) loss *= 2;

	    if ((IS_FLAG5(ch->sucking, AFF5_AIR_BLOOD) || ch->sucking->level == 120)
		&& ch->blood > 10 && ch->sucking->blood > 20)
	    {
		act_new("Blood is sucked up from your throat and pumps into $N's neck.", ch, NULL, ch->sucking, TO_CHAR, POS_DEAD);
		act("Blood is sucked up from $n's throat and pumps into $N's neck.", ch, NULL, ch->sucking, TO_ROOM);
		act("Blood is sucked up from $n's throat and pumps into your neck.", ch, NULL, ch->sucking, TO_VICT);
	        if (ch->sucking->blood < 30)
	        {
		    send_to_char("You feel that they are close to death.\n\r", ch->sucking);
		    send_to_char("You feel on the verge of death.\n\r", ch);
		    act("$N looks pale and feint from bloodloss.", ch->sucking, NULL, ch, TO_NOTVICT);
	        }
		ch->blood -= loss;
		ch->sucking->blood += loss;
	        if (ch->sucking->blood > max_blood_pool(ch->sucking) )
	        { 
		    ch->sucking->blood = max_blood_pool(ch->sucking);
		    stc("Your blood lust is sated.\n\r", ch->sucking);
	        }
	    }
	    else {
	    act("$N's life blood pumps into your veins.", ch, NULL, ch->sucking,TO_CHAR);
	    send_to_char("You feel weaker as the blood is drained from your body.\n\r", ch->sucking);
	    act("Blood pumps from $N's jugular into $n's mouth.", ch,NULL, ch->sucking, TO_NOTVICT);

	    if (ch->sucking->blood < 30)
	    {
		send_to_char("You feel that they are close to death.\n\r", ch);
		send_to_char("You feel on the verge of death.\n\r", ch->sucking);
		act("$N looks pale and feint from bloodloss.", ch, NULL, ch->sucking, TO_NOTVICT);
	    }
	    ch->blood += loss;

	    if (ch->blood > max_blood_pool(ch) )
	    {
		ch->blood = max_blood_pool(ch);
		stc("Your blood lust is sated.\n\r", ch);
	    }

	    ch->sucking->blood -= loss;
	    if (ch->sucking->blood < 1)
	    {
		send_to_char("The last trickles of blood leave your body.\n\r",ch->sucking);
		send_to_char("You sink into a deep black sleep...\n\r", ch->sucking);
		act("You draw the last drops of blood from $N's body, and $E slumps to the floor.", ch, NULL, ch->sucking, TO_CHAR);
		WAIT_STATE(ch->sucking, 20);
		if (ch->secondaryrace == RACE_VAMPIRE
		    && ch->sucking->secondaryrace == RACE_VAMPIRE
		    && !IS_NPC(ch) && !IS_NPC(ch->sucking)
		    && ch->generation < ch->sucking->generation)
		{
		    ch->generation += 1;
		    SET_TIMER(ch, TIMER_VAMPIRE_GROW, 3000);
		    ch->sucking->generation -= 1;
		    send_to_char("Your blood has lost its potency.\n\r", ch->sucking);
		    sprintf(buf, "-->%s has been diablerised by %s.", 
			ch->sucking->name, GET_NAME(ch));
		    informational(buf, RACE_VAMPIRE, 0);
		    sprintf(buf, "Congratulations, you are now generation %d.\n\r",
			14 - ch->generation);
	   	    send_to_char(buf, ch);
		    look_name(ch);
		    look_name(ch->sucking);
		}
		ch->sucking->hit = -10;
		update_pos(ch->sucking);
	        ch->sucking->blood = max_blood_pool(ch);
		REMOVE_BIT(ch->sucking->flag4, AFF_BEING_SUCKED);
		raw_kill(ch->sucking);
	        ch->sucking = NULL;
	        if (IS_SET(ch->flag2, AFF_BITE))
		{
	            stc("You retract your fangs.\n\r", ch);
		    act("$n retracts $s fangs.", ch, NULL, NULL, TO_ROOM);
		    REMOVE_BIT(ch->flag2, AFF_BITE);
		}

	    }
	    }
	}


        if (!IS_NPC(ch) && !is_item(ch) &&
            dice (1, 500) + 10 < ch->pcdata->condition[COND_DRUNK] )
        {
          act("$n vomits all over the floor.", ch, NULL, NULL, TO_ROOM);
          stc("You vomit all over the floor.\n\r", ch);
	}

	if (ch->autoroute)
	{
	    char buf[MAX_STRING_LENGTH];
	    crash("autoroute");
	    switch(ch->route[ch->routepos++])
	    {
	    case '\0': ch->autoroute = FALSE;
		      ch->routepos = 0;
		      break;
	    case 'n' : interpret(ch, "north");
		       break;
	    case 's' : interpret(ch, "south");
		       break;
	    case 'e' : interpret(ch, "east");
		       break;
	    case 'w' : interpret(ch, "west");
		       break;
	    case 'u' : interpret(ch, "up");
		       break;
	    case 'd' : interpret(ch, "down");
		       break;
	    case 'S' : interpret(ch, "sleep");
		       break;
	    case 'p' : /* Pause */
		       break;
	    case 'W' : interpret(ch, "wake");
		       break;
	    case 'O' : sprintf(buf,"open %c",ch->route[ch->routepos++]);
		       interpret(ch, buf);
		       break;
	    case 'C' : sprintf(buf,"close %c",ch->route[ch->routepos++]);
		       interpret(ch, buf);
		       break;
	    case 'U' : sprintf(buf,"unlock %c",ch->route[ch->routepos++]);
		       interpret(ch, buf);
		       break;
	    case 'L' : sprintf(buf,"lock %c",ch->route[ch->routepos++]);
		       interpret(ch, buf);
		       break;
	    case 'T' : ch->autoroute = FALSE;
		       break;
	    case 'R' : ch->routepos = 0;
		       break;
	    case 'X' : ch->autoroute = FALSE;
		       ch->routepos = 0;
		       free_string(ch->route);
		       ch->route = NULL;
		       break;
	    default  : bug("Bad route command",0);
	    }
	}



	if ( IS_NEWFLAG(ch, AFF_LAUGHTER))
	{
	    switch(dice(1,6))
	    {
		case 1 : interpret(ch, "giggle");
			 break;
		case 2 : interpret(ch, "laugh");
			 break;
		case 3 : interpret(ch, "chuckle");
			 break;
		case 4 : interpret(ch, "smirk");
			 break;
		case 5 : interpret(ch, "grin");
			 break;
		case 6 : interpret(ch, "rofl");
			 break;
	    }
	}



	crash("do the fear spell");

	/* Some manticores are immune to fear */
	if (ch->power[DISC_VAMP_SATY] >= 7)
	{
	    affect_strip(ch, gsn_fear);
	    REMOVE_BIT(ch->flag2, AFF_FEAR);
  	}

	if ( IS_NEWFLAG(ch, AFF_FEAR) && is_item(ch))
	{
	    switch(ch->position)
	    {
		case POS_SLEEPING : rand_to_char(ch,
	"You have nightmares about vampires sucking your blood.\n\r",
	"You have nightmares about spiders crawling over your bed.\n\r",
	"You have a nightmare about a scorpion under your pillow.\n\r",
	"You have nightmares about monsters under your bed.\n\r",
	"You clench your stomach as you have nightmares about the fabled Rott Worm.\n\r");
			send_to_char("You SCREAM and wake up, sweat running down your forehead.\n\r",ch);
			ch->position = POS_RESTING;
			break;
		case POS_RESTING :
		case POS_SITTING :
			send_to_char("You stand up, screaming in fright as the monsters loom around you.\n\r",ch);
		case POS_FIGHTING :
		case POS_STANDING :
			send_to_char("As the monsters close in around you, you scream for your life and run.\n\r",ch);
		    exec_fear(ch);
	    }
	}

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	{
	    check_trigger(ch, victim, TRIG_FIGHTING, "");
	}
	else
	    stop_fighting( ch, FALSE );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;
    }

    /* mini loops.  if the mob can be killed, add a new loop here */
    crash("violence update - stasis");

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if (is_item(ch)) continue;

	if (IS_FLAG4(ch, AFF_STASIS))
	{
	    if (ch->hit == ch->max_hit)
	    {
		ch->hit -= ch->max_hit / 20;
		update_pos(ch);
		send_to_char("You start to melt.\n\r", ch);
		if (ch->hit < 1) ch->hit = 1;
	    }
	    else if (ch->hit > ch->max_hit * 3 / 4)
	    {
		ch->hit -= ch->max_hit / 20;
		update_pos(ch);
		send_to_char("Your flesh starts to turn into slush.\n\r", ch);
		if (ch->hit < 1) ch->hit = 1;
	    }
	    else if (ch->hit > ch->max_hit / 2)
	    {
		ch->hit -= ch->max_hit / 20;
		update_pos(ch);
		send_to_char("Your skin is melting and dropping to the floor.\n\r", ch);
		if (ch->hit < 1) ch->hit = 1;
	    }
	    else if (ch->hit > 2)
	    {
		ch->hit -= ch->max_hit / 20;
		update_pos(ch);
		send_to_char("You have nearly melted into slime.\n\r", ch);
		if (ch->hit < 1) ch->hit = 1;
	    }
	    if (ch->hit ==1)
	    {
		REMOVE_BIT(ch->flag4, AFF_STASIS);
		send_to_char("You slump to the floor.\n\r", ch);
		SET_TIMER(ch, TIMER_UNCONCIOUS, 1);
		ch->position = POS_RESTING;
	    }
        }
    }

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

  	if (is_null_room(ch)) continue;
	if (is_item(ch)) continue;

	if (ch->secondaryrace != 0 && ch->alignment < -300
	 	&& !IS_NPC(ch))
	{
	    OBJ_DATA *obj_next;

            crash("violence update - callrocks");

	    for ( obj = ch->in_room->contents ; obj != NULL ; obj=obj->next)
	    {
		obj_next = obj->next_content;
		if (IS_OBJ_STAT2(obj, ITEM_ATTACK_NON_ELAD) && dice(1, 3) == 1)
		{
		    act("$p jumps into the air and slams into $n.", ch, obj, NULL, TO_ROOM);
		    act("$p jumps into the air and slams into you.", ch, obj, NULL, TO_CHAR);
		    damage_old(ch, ch, 50, 0, DAM_BASH, FALSE);
		}
	    }
	}
    }
 
    crash("violence update - smother and entangle");

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;
	
	if (is_null_room(ch)) continue;
	if (is_item(ch)) continue;

	if (IS_FLAG4(ch, AFF_SMOTHERED) )
	{
	    if (dice(1, 30) == 1)
	    {
	        send_to_char("The plants release their grip.\n\r", ch);
	 	REMOVE_BIT(ch->flag4, AFF_SMOTHERED);
		act("The plants release their grip on $n.", ch, NULL, NULL, TO_ROOM);
	    }
	    else
	    {
	        send_to_char("The plants choke and strangle you.  You gasp for air.\n\r", ch);
	        act("$n struggles as the plants choke and strangle $m.", ch, NULL, NULL, TO_ROOM);
	        damage_old(ch,ch,40, 0, DAM_OTHER, FALSE);
	    }
	}
    }

    crash("stinking cloud");

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if (is_item(ch)) continue;
	if (ch->in_room == NULL) continue;

	if (!RTIME_UP(ch->in_room, RTIMER_STINKING_CLOUD)
		&& ch->secondaryrace == 0)
	    send_to_char("You choke and gag from the poisonous green cloud.\n\r", ch);
	else if (!RTIME_UP(ch->in_room, RTIMER_STINKING_CLOUD))
	{
	    send_to_char("You choke and gag from the poisonous green cloud.\n\r", ch);
	    spell_poison(skill_lookup("poison"), 20, ch, ch, TAR_IGNORE);
	}
    }	    

    crash("swarms has crashed it!");

    /* The swarms and what the little buggers do */

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if (ch->in_room == NULL) continue;
	if (is_item(ch)) continue;

        if (!RTIME_UP(ch->in_room, RTIMER_SWARM_BEES))
        {
	    crash("bee swarms has crashed it!");
            stc("The bees buzz around you, trying to sting you.\n\r", ch);
            if (dice(1, 30) > get_curr_stat(ch, STAT_DEX))
            {
                if (ch->power[DISC_VAMP_ANIM] > 0
                    || ch->secondaryrace == RACE_ELADRIN)
                    stc("The bees seem to leave you alone.\n\r", ch);
                else
                {
                    stc("Ouch!  You got stung.\n\r", ch);
                    if (ch->hit > 10) ch->hit--;  
                    spell_poison(skill_lookup("poison"), 28, ch, ch, TARGET_CHAR);
                }
            }
        }

        if (!RTIME_UP(ch->in_room, RTIMER_SWARM_BATS))
        {
            stc("The bats flap around you, trying to bite you.\n\r", ch);
            if (dice(1, 30) > get_curr_stat(ch, STAT_DEX))
            {
	        crash("bat swarms has crashed it!");
                if (ch->power[DISC_VAMP_ANIM] > 0
                    || ch->secondaryrace == RACE_ELADRIN)
                    stc("The bats seem to leave you alone.\n\r", ch);
                else
                {
                    stc("Ouch!  You got bitten.\n\r", ch);
                    if (ch->hit > 20) ch->hit -= 10;  
                    spell_poison(skill_lookup("poison"), 28, ch, ch, TARGET_CHAR);
                }
            }
        }

        if (!RTIME_UP(ch->in_room, RTIMER_SWARM_RATS))
        {
            stc("The rats scurry around you, trying to bite you.\n\r", ch);
            if (dice(1, 30) > get_curr_stat(ch, STAT_DEX))
            {
	        crash("rat swarms has crashed it!");
                if (ch->power[DISC_VAMP_ANIM] > 0
                    || ch->secondaryrace == RACE_ELADRIN)
                    stc("The rats seem to leave you alone.\n\r", ch);
                else
                {
                    stc("Ouch!  You got bitten.\n\r", ch);
                    if (ch->hit > 20) ch->hit -= 10;  
                    spell_poison(skill_lookup("plague"), 28, ch, ch, TARGET_CHAR);
                }
            }
        }
    }	    

    crash("violence update - vampires and sunlight");

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if (ch->in_room == NULL) continue;

	/* Ill stick discord in here for a laugh! */
        if (!RTIME_UP(ch->in_room, RTIMER_DISCORD))
	    stc("You hear banging and crashing noises all around you.\n\r", ch);

	if (  IS_OUTSIDE(ch) 
	   && ch->secondaryrace == RACE_VAMPIRE
	   && !IS_TAINTED(ch)
	   && weather_info.sky == SKY_CLOUDLESS
	   && (weather_info.sunlight == SUN_LIGHT 
	   || weather_info.sunlight == SUN_RISE)
	   && !IS_FLAG4(ch, AFF_PHASING)
	   && !is_item(ch)
	   && ch->hit > 0
	   && dice (1, 15) > ch->generation
	   && ch->level != 120)
	{
	    crash("odd, its got past the if statement in vamps and sunl");
	    send_to_char("The sunlight sears your flesh, you scream in agony.\n\r", ch);
	    act("$n screams in agony as the sunlight sears $s flesh.", ch, NULL, NULL, TO_ROOM);
	    if (ch->hit == 1)
	    {
		crash("actually disintegrate");
		send_to_char("The sunlight rips your flesh away, you disintegrate.\n\r", ch);
		act("$n disintegrates into a pile of fine powder as the sunlight hits $m.", ch, NULL, NULL, TO_ROOM);	
		ch->hit = -10;
		obj = create_object( get_obj_index(67), 0);
		obj_to_room(obj, ch->in_room, ch->altitude);
		if (!check_if_crimson(ch))
		    destroy_char(ch, ch, FALSE);
	    }
	    dam = ch->hit/12 + 20;
	    dam *= 100-ch->value[6];
	    dam /= 100;
	    ch->hit = UMAX(1, ch->hit-(50+dam) );
	}
    }

    crash("violence update - conflaguration");

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

 	if (is_item(ch)) continue;
	if (ch->in_room == NULL) continue;

	if ( IS_AFFECTED(ch, AFF_CONFLAGURATION) )
	{
	   send_to_char("You are scorched by the fires of hell.\n\r",ch); 
	   act("$n screams as the fires of hell envelop $m.", ch, NULL, NULL, TO_ROOM);
	   damage_old(ch, ch, dice(1, 30), 0, DAM_FIRE, FALSE);
	   if (ch->hit == -10)
	   { 
	       act("The fires around $n fade away.",ch, NULL, NULL, TO_ROOM);
	       affect_strip(ch, gsn_conflaguration);
	   }

	}
    }

    crash("violence update - underwater");

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

 	/* I hate it when this crashes it! */
	if ( ch->in_room == NULL ) continue;
	if (is_item(ch)) continue;

	if ( ch->in_room->sector_type == SECT_UNDERWATER &&
	    !can_breathe_underwater(ch) )
	    {
		switch (++ch->value[10])
		{
		case 4 : send_to_char("#4You try to hold your breath.\n\r",ch);
			 act("#4$n tries to hold $s breath.", ch,NULL,NULL,TO_ROOM);
			 break;
		case 8 : send_to_char("#4You gasp and choke for air.\n\r",ch);
			 act("#4$n gasps and chokes for air.", ch,NULL,NULL,TO_ROOM);
			 break;
		case 10 : send_to_char("#4Your lungs are filling with water.\n\r",ch);
			 act("#4$n's lungs fill with water.", ch,NULL,NULL,TO_ROOM);
			 break;
		case 12 : send_to_char("#4You drown, conciousness fades...\n\r",ch);
			 act("#4$n drowns.", ch,NULL,NULL,TO_ROOM);
			 destroy_char(ch, ch, TRUE);
			 break;
		}
	    }
	    else if (ch->value[10] > 0 && !can_breathe_underwater(ch) )
	    {
		ch->value[10]--;
		if (ch->value[10] < 0) ch->value[10] = 0;
		if (ch->value[10] > 8)
		    stc("You gasp and choke for air.\n\r", ch);
		else if (ch->value[10] > 6)
		    stc("You cough water out of your lungs.\n\r", ch);
	  	else if (ch->value[10] > 3)
		    stc("You take deep breaths.\n\r", ch);
	    }
    }

    crash("violence update - lack of blood");

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if (is_item(ch)) continue;
	if (is_null_room(ch)) continue;

	if (ch->secondaryrace == 3 && ch->hit > 0)
	{
	    if (ch->blood <= 1 )
	    {
		if (ch->blood == 1 && !IS_NPC(ch))
			send_to_char("Your thirst for blood grips you, you must drink soon...\n\r",ch);
		send_to_char("You are dying from lack of blood.\n\r",ch);
		damage_old(ch, ch, dice(4, 20), 0, DAM_FIRE, FALSE);
		if (ch->hit < 0)
		{
		    destroy_char(ch, ch, TRUE);
		    ch->level = 20;
		    ch->blood = max_blood_pool(ch);
		    send_to_char("You have died from lack of blood.\n\r",ch);
		}
	    }
	}
    }

    crash("Well, thats all folks - for violence update, that is!");

    return;
}

bool is_null_room(CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH];

    if (ch->in_room != NULL) return FALSE;
    sprintf(buf, "%s is currently situated in a NULL room.  Bad.\n\r",
	GET_PROPER_NAME(ch) );
    bug(buf, 0);
    gecho(buf);
    
    return TRUE;
}

bool check_if_crimson (CHAR_DATA *ch)
{
    crash("check if crimson routine");

    if (ch->level < 80) return FALSE;
    if (ch->tertiaryrace != 0) return FALSE;

    ch->tertiaryrace = TERTIARY_CRIMSON;
    send_to_char("As your body reforms, you find that you are no longer a vampire.\n\r", ch);
    send_to_char("Your body has turned to mist, and your blood hangs suspended within\n\r", ch);
    send_to_char("your incorporeal form, giving you a crimson look.\n\r", ch);
    send_to_char("You feel much more powerful, and your thirst for blood is now more\n\r", ch);
    send_to_char("than ever before.  Welcome to the Afterlife.\n\r", ch);

    WAIT_STATE(ch, 20);
    ch->hit += 500;
    ch->max_hit += 500;
    set_form(ch, FRM_CRIMSON);
    return TRUE;
}




void exec_fear(CHAR_DATA *ch)
{
   int door;
   ROOM_INDEX_DATA *was_in;
   ROOM_INDEX_DATA *now_in;
   bool found;

   crash("exec_fear routine");

   found = FALSE;

   rand_to_char(ch,
	"The monsters slash DEMOLISHES you.",
	"The nightmares attack DEVASTATES you.",
	"The beasts bite mauls you.",
	"The Rott Worm DEVASTATES you as it bursts from your stomach.",
	"The catoblepas *** EVISCERATES *** you.");

   send_to_char("\n\rThe really did HURT.\n\rYou flee from combat.\n\r",ch);

   for (door = 0; door < 6; door++)
   {
	if (ch->in_room->exit[door] != NULL)
		found = TRUE;
   }

   if (!found)
   {
	send_to_char("You cannot escape!\n\r",ch);   
	return;
   }

   found = FALSE;

   while (found == FALSE)
   {
	door = dice(1, 6) - 1;
	if (ch->in_room->exit[door] != NULL)
	    found = TRUE;
   }

   stop_fighting( ch, TRUE );
   was_in = ch->in_room;
   move_char( ch, door, FALSE );
   if (ch->in_room == was_in ) return;
   now_in = ch->in_room;
   ch->in_room = was_in;
   act("$n screams and runs out the room!", ch, NULL, NULL, TO_ROOM);
   ch->in_room = now_in;
   if (!IS_NPC(ch))
	send_to_char("You scream and run away in terror!\n\r",ch);
}

int calc_thaco(CHAR_DATA *ch)
{
    int thac0, thac0_00, thac0_32;

    crash("calc_thac0 routine");

    if (IS_NPC(ch))
    {
	thac0_00 = 20;
	thac0_32 = -4;
	if (IS_SET(ch->act, ACT_WARRIOR))
		thac0_32 = -10;
	else if (IS_SET(ch->act, ACT_THIEF))
		thac0_32 = -4;
	else if (IS_SET(ch->act, ACT_CLERIC))
		thac0_32 = 2;
	else if (IS_SET(ch->act, ACT_MAGE))
		thac0_32 = 6;
    }
    else
    {
	thac0_00 = 20;
	thac0_32 = -10;
    }

    thac0 = interpolate(ch->level, thac0_00, thac0_32);

    if (thac0 < 0)
	thac0 = thac0/2;

    if (thac0 < 5)
	thac0 = -5 + (thac0 + 5) / 2;

     return thac0;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    crash("check_assist routine");

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;
	if (rch->altitude != ch->altitude) continue;
	if (IS_AWAKE(rch) && rch->fighting == NULL)
	{

	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch) 
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	    &&  rch->level + 6 > victim->level)
	    {
		do_emote(rch,"screams and attacks!");
		set_fighting(rch, victim);
		continue;
	    }

	    /* PCs next */
	    if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))
	    {
		if ( ( (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
		||     IS_AFFECTED(rch,AFF_CHARM)) 
		&&   is_same_group(ch,rch) 
		&&   !is_safe(rch, victim))
		    set_fighting(rch, victim);
		
		continue;
	    }
	
	    /* now check the NPC cases */
	    
	    if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
	
	    {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

		||   (IS_NPC(rch) && rch->group && rch->group == ch->group)

		||   (IS_NPC(rch) && rch->race == ch->race 
		   && IS_SET(rch->off_flags,ASSIST_RACE))

		||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
		   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
		     ||  (IS_EVIL(rch)    && IS_EVIL(ch))
		     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch)))) 

		||   (rch->pIndexData == ch->pIndexData 
		   && IS_SET(rch->off_flags,ASSIST_VNUM)))

		{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;
		
		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)
		    {
			do_emote(rch,"screams a war cry and joins the fray!");
			set_fighting(rch, target);
		    }
		}       
	    }
	}
    }
}


void petrify(CHAR_DATA *ch, CHAR_DATA *victim)
{
    OBJ_DATA *obj = NULL;

    crash("petrify routine");

    act("You catch a glimpse of $n's eyes.", ch, NULL, victim, TO_VICT);
    act("You stare intently at $N.", ch, NULL, victim, TO_CHAR);

    if (ch->hit < victim->hit || dice(1, 10) < 10)
    {
	send_to_char("You failed to petrify them.\n\r", ch);
	return;
    }

    if (IS_NPC(ch) && !IS_NPC(victim)) victim->pcdata->mob_killed += 1;

    if ( ( obj = get_obj_by_type( victim, ITEM_MAGIC_MIRROR ) ) != NULL )
    {
	act("You brandish $p.", victim, obj, NULL, TO_CHAR);
	act("$N brandishes $p.", ch, obj, victim, TO_CHAR);
	turn_to_stone(ch);
	return;
    }   

    turn_to_stone(victim);
}

void turn_to_stone(CHAR_DATA *ch)
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];

    crash("turn_to_stone routine");

    obj = create_object( get_obj_index(OBJ_VNUM_STATUE), 0);
    obj_to_room(obj, ch->in_room, ch->altitude);
    act("$n starts to petrify, and turns into a statue.", ch, NULL, NULL, TO_ROOM);

    send_to_char("Your muscles start to stiffen and harden, until you turn to stone.\n\r", ch);

    sprintf(buf, obj->short_descr, GET_PROPER_NAME(ch) );
    free_string(obj->short_descr);
    obj->short_descr = str_dup(buf);
        
    sprintf(buf, obj->name, GET_PROPER_NAME(ch) );
    free_string(obj->name);
    obj->name = str_dup(buf);

    sprintf(buf, obj->description, GET_PROPER_NAME(ch) );
    free_string(obj->description);
    obj->description = str_dup(buf);
    obj->timer = 254;

    destroy_char(ch, ch, FALSE);
}


void check_quest_kill(CHAR_DATA *ch, CHAR_DATA *victim)
{
    crash("check_quest_kill routine");

    if (ch == NULL || victim == NULL || !IS_FLAG4(ch, AFF_ON_A_QUEST) 
	|| IS_NPC(ch)) return;
 
    crash("Its actually doing check_quest_kill for a NPC");

    if (ch->pcdata->quest_mob_1 != NULL
	&& !str_cmp(ch->pcdata->quest_mob_1, victim->short_descr) )
    {
	ch->pcdata->quest_mob_1 = NULL;
	REMOVE_BIT(ch->pcdata->quest_status, QUEST_MOB_1);
	act("Beams of energy shoot from $N into $n.", ch, NULL, victim, TO_NOTVICT);
	act("Beams of energy shoot from you into $n.", ch, NULL, victim, TO_VICT);
	act("Beams of energy shoot from $N into you.", ch, NULL, victim, TO_CHAR);
    }
    else if (ch->pcdata->quest_mob_2 != NULL
	&& !str_cmp(ch->pcdata->quest_mob_2, victim->short_descr) )
    {
	ch->pcdata->quest_mob_2 = NULL;
	REMOVE_BIT(ch->pcdata->quest_status, QUEST_MOB_2);
	act("Beams of energy shoot from $N into $n.", ch, NULL, victim, TO_NOTVICT);
	act("Beams of energy shoot from you into $n.", ch, NULL, victim, TO_VICT);
	act("Beams of energy shoot from $N into you.", ch, NULL, victim, TO_CHAR);
    }
    else if (ch->pcdata->quest_mob_3 != NULL
	&& !str_cmp(ch->pcdata->quest_mob_3, victim->short_descr) )
    {
	ch->pcdata->quest_mob_3 = NULL;
	REMOVE_BIT(ch->pcdata->quest_status, QUEST_MOB_3);
	act("Beams of energy shoot from $N into $n.", ch, NULL, victim, TO_NOTVICT);
	act("Beams of energy shoot from you into $n.", ch, NULL, victim, TO_VICT);
	act("Beams of energy shoot from $N into you.", ch, NULL, victim, TO_CHAR);
    }
}


/*
 * Inflict damage from a hit.
 */
bool damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,
	    bool show) 
{
    OBJ_DATA *corpse;
    bool immune;

    crash("bool damage routine");

    if ( victim->position == POS_DEAD )
	return FALSE;

    if (is_wearing_artifact(victim, ARTIFACT_JANDOR) != NULL)
	return FALSE;

    if (IS_FLAG4(victim, AFF_INVULNERABLE))
	return FALSE;

    /* aura induced damage reduction */

    if (victim->secondaryrace==5 && victim->value[0]>3)
    {
	if (victim->value[0]==4)
		dam = (dam * 3) / 4;
	if (victim->value[0]==5)
		dam = dam / 2;
    }

    /* damage reduction */
    if ( dam > 35)
	dam = (dam - 35)/2 + 35;
    if ( dam > 80)
	dam = (dam - 80)/2 + 80; 


    if (ch->secondaryrace == RACE_LUPINE
	&& (time_info.day > 33 || time_info.day < 2) )
        dam *= 2;

    if (ch->secondaryrace == RACE_LUPINE
	&& (time_info.day == 18 || time_info.day == 17) )
        dam /= 2;


   
    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return FALSE;

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
		set_fighting( victim, ch );
	    if (victim->timer <= 4)
		victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }

    /*
     * Damage modifiers.
     */

    if ( dam > 1 && !IS_NPC(victim) 
    &&   victim->pcdata->condition[COND_DRUNK]  > 10 )
	dam = dam / 2;

    if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;

    if ( dam > 1 && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
    ||               (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )))
	dam -= dam / 4;

    immune = FALSE;

    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):     
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_ETHERIAL))
    {
	immune = TRUE;
	dam = 0;
    }

    if (IS_FLAG4(victim, AFF_STASIS))
    {
	immune = TRUE;
	dam = 0;
	stop_fighting(victim, TRUE);
    }

    if (show)
	dam_message( ch, victim, dam, dt, immune );

    if (dam == 0)
	return FALSE;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "#1$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "#1You are mortally wounded, and will die soon, if not aided.\n\r",
	    victim );
	break;

    case POS_INCAP:
	act( "#1$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "#1You are incapacitated and will slowly die, if not aided.\n\r",
	    victim );
	break;

    case POS_STUNNED:
	act( "#3$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("#3You are stunned, but will probably recover.\n\r",
	    victim );
	break;

    case POS_DEAD:
	death_move(ch, victim);
        death_cry(victim, TRUE);
	act( "#1$n drops to the floor DEAD!!", victim, 0, 0, TO_ROOM );
	send_to_char( "#1You have been KILLED!!\n\r\n\r", victim );
	stop_fighting(victim, TRUE);
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "That really did HURT!\n\r", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "#1You sure are BLEEDING!\n\r", victim );
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if (victim->position == POS_DEAD && !IS_NPC(victim) && 
	(victim->secondaryrace!=0 || victim->hit + victim->illus_dam > -10))
    {
	stop_fighting(victim, TRUE);
	victim->hit = -10 - victim->illus_dam;
	if (ch->power[DISC_WARL_THEA] < 3) wipe_spells(victim);
	return TRUE;
    }
    else
    {
	update_pos(victim);
    }

    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    if (IS_FLAG4(ch, AFF_STASIS)) stop_fighting(victim, FALSE);
    if (IS_FLAG4(victim, AFF_STASIS)) stop_fighting(victim, FALSE);

    /*
     * Payoff for killing things.
     */

    if ( victim->position == POS_DEAD && victim->secondaryrace==0)
    {
	check_quest_kill(ch, victim);
	
	crash("geri from the spice girls has crashed it");

	if (IS_SPEC(victim, SPEC_EVERLIVING))
	{
	    victim->hit = victim->max_hit;
	    victim->move = victim->max_move;
	    stop_fighting(victim, TRUE);
	    update_pos(victim);
	    act("#2$n gets back to $s feet, and all $s wounds heal up.", victim, NULL, NULL, TO_ROOM);
	    return TRUE;
	}

	crash("mel b from the spice girls has crashed it");

	if (IS_SPEC(victim, SPEC_SPLIT))
	{
	    CHAR_DATA *mob = NULL;

	    victim->max_hit /= 2;
	    if (victim->max_hit > 30)
	    {
	    	clone_mobile( victim, mob );
	    	if (mob == NULL) return TRUE; 
		mob->max_hit = victim->max_hit;
	    	victim->hit = victim->max_hit;
	    	mob->hit = mob->max_hit;
	    	victim->move = victim->max_move;
	    	mob->move = mob->max_move;
		char_to_room(mob, ch->in_room, ch->altitude);
	    	set_fighting(mob, ch);
	    	set_fighting(victim, ch);
	    	update_pos(victim);
	    	update_pos(mob);
	    	act("#2$n splits in half and both pieces continue fighting.", victim, NULL, NULL, TO_ROOM);
	    	return TRUE;
	    }
	}

	group_gain( ch, victim ); 
	crash("mel c from the spice girls has crashed it");

	if (!IS_NPC(ch) && victim->level > ch->best_mob_killed)
	{
	    ch->best_mob_killed = victim->level;
	    check_player_table(ch);
	}

	if (IS_NPC(victim) && !IS_NPC(ch) )
	    ch->pcdata->mob_kills += 1;	

	if (IS_SPEC(victim, SPEC_FREEZE)) WAIT_STATE(ch, 18);

	crash("victoria from the spice girls has crashed it");

	if ( !IS_NPC(victim) )
	{
	    if (IS_NPC(ch))
		victim->pcdata->mob_killed += 1;
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		ch->in_room->vnum );
	    log_string( log_buf );
	}

	crash("emma from the spice girls has crashed it");

	sprintf( log_buf, "%s got toasted by %s at %s [room %d]",
	    (IS_NPC(victim) ? victim->short_descr : victim->name),
	    (IS_NPC(ch) ? ch->short_descr : ch->name),
	    ch->in_room->name, ch->in_room->vnum);

	if (IS_NPC(victim))
	    wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
	else
	    wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);
	raw_kill( victim );
	/* dump the flags */
	if (ch != victim && !IS_NPC(ch) && !is_same_clan(ch,victim))
	{
	    if (IS_SET(victim->act,PLR_KILLER))
		REMOVE_BIT(victim->act,PLR_KILLER);
	    else
		REMOVE_BIT(victim->act,PLR_THIEF);
	}
	 
 	crash("yes id like a slice of toast");

	if (!IS_NPC(ch)
	&&  (corpse = get_obj_list(ch,"corpse",ch->in_room->contents)) != NULL
	&&  corpse->item_type == ITEM_CORPSE_NPC && can_see_obj(ch,corpse))
	{
	    OBJ_DATA *coins;
	
	    bug("here", 0);

	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents );
	       
	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
		do_get( ch, "all corpse" );
				 
	    bug("here", 0);
	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
		corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
		if ((coins = get_obj_list(ch,"gcash",corpse->contains))
		     != NULL)
		    do_get(ch, "all.gcash corpse");

	    bug("here", 0);
	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
	      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
		return TRUE;  /* leave if corpse has treasure */
	      else
		do_sacrifice( ch, "corpse" );

	    bug("here", 0);
	}
	return TRUE;
    }

    crash("and some buttered scones");

    if ( victim == ch )
	return TRUE;


    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    crash("chupa chups lollies are great");

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
	do_flee( victim, "" );

    tail_chain( );
    return TRUE;
}

void wipe_spells(CHAR_DATA *victim)
{
    int i,j;

    if (IS_NPC(victim)) return;

    crash("maaaaaaaaan!  i didnt know that the wipe_spells function was broken!");

    for (i = 1 ; i < 11 ; i++)
    {
	for (j = 0 ; j < 10 ; j++)
 	{
	    victim->pcdata->memorize[i][j] = -1;
	}
    }

    send_to_char("All your memorized spells have been wiped from your mind.\n\r", victim);
}

bool damage_illusionary(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int 
				dam_type, bool show) 
{
    bool immune;

    if ( victim->position == POS_DEAD )
	return FALSE;

    if (ch->level == 120)
	return FALSE;

    /* aura induced damage reduction */

    if (victim->secondaryrace==5 && victim->value[0]>3)
    {
	if (victim->value[0]==4)
		dam = (dam * 3) / 4;
	if (victim->value[0]==5)
		dam = dam / 2;
    }

    /* damage reduction */
    if ( dam > 35)
	dam = (dam - 35)/2 + 35;
    if ( dam > 80)
	dam = (dam - 80)/2 + 80; 



   
    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return FALSE;

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
		set_fighting( victim, ch );
	    if (victim->timer <= 4)
		victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Invisible attacking
     */
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }

    /*
     * Damage modifiers.
     */

    if ( dam > 1 && !IS_NPC(victim) 
    &&   victim->pcdata->condition[COND_DRUNK]  > 10 )
	dam = 9 * dam / 10;

    if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;

    if ( dam > 1 && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
    ||               (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )))
	dam -= dam / 4;

    immune = FALSE;

    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):     
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_ETHERIAL))
    {
	immune = TRUE;
	dam = 0;
    }

    if (show)
	dam_message( ch, victim, dam, dt, immune );

    if (dam == 0)
	return FALSE;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    victim->illus_dam += dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "You are mortally wounded, and will die soon, if not aided.\n\r",
	    victim );
	break;

    case POS_INCAP:
	act( "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "You are incapacitated and will slowly die, if not aided.\n\r",
	    victim );
	break;

    case POS_STUNNED:
	act( "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("You are stunned, but will probably recover.\n\r",
	    victim );
	break;

    case POS_DEAD:
	act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	send_to_char( "You have been KILLED!!\n\r\n\r", victim );
	stop_fighting(ch, TRUE);
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "That really did HURT!\n\r", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "#1You sure are BLEEDING!\n\r", victim );
	break;
    }

    update_pos(victim);

    /*
     * Sleep spells and extremely wounded folks.
     */
    if (victim->position == POS_DEAD && !IS_NPC(victim) && victim->secondaryrace!=0)
    {
	stop_fighting(victim, TRUE);
	if (ch->power[DISC_WARL_THEA] < 3) wipe_spells(victim);
	victim->hit = -10;
    }
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    if (IS_FLAG4(ch, AFF_STASIS)) stop_fighting(victim, FALSE);
    if (IS_FLAG4(victim, AFF_STASIS)) stop_fighting(victim, FALSE);

    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
	do_flee( victim, "" );

    tail_chain( );
    return TRUE;
}






/*
 * Inflict damage from a hit.
 */
bool damage_old( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int 
dam_type, bool show ) {

    OBJ_DATA *corpse;
    bool immune;


    if ( victim->position == POS_DEAD )
	return FALSE;

    if (is_wearing_artifact(victim, ARTIFACT_JANDOR) != NULL)
	return FALSE;

    if (IS_FLAG4(victim, AFF_INVULNERABLE))
	return FALSE;

    /* aura induced damage reduction */

    if (victim->secondaryrace==5 && victim->value[0]>3)
    {
	if (victim->value[0]==4)
		dam = (dam * 3) / 4;
	if (victim->value[0]==5)
		dam = dam / 2;
    }

    
    /* damage reduction */
    if ( dam > 35)
	dam = (dam - 35)/2 + 35; 
    if ( dam > 80)
	dam = (dam - 80)/2 + 80; 


    if (ch->secondaryrace == RACE_LUPINE
        && (time_info.day > 33 || time_info.day < 2) )
        dam *= 2;
    
    if (ch->secondaryrace == RACE_LUPINE
        && (time_info.day == 18 || time_info.day == 17) )
        dam /= 2; 

   
    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return FALSE;

/*      if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
		set_fighting( victim, ch );
	    if (victim->timer <= 4)
		victim->position = POS_FIGHTING;
	}*/

	if ( victim->position > POS_STUNNED )
	{
	    /*if ( ch->fighting == NULL )
		set_fighting( ch, victim );*/

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if ( IS_NPC(ch)         
	    &&   IS_NPC(victim)
	    &&   IS_AFFECTED(victim, AFF_CHARM)
	    &&   victim->master != NULL
	    &&   victim->master->in_room == ch->in_room
	    &&   number_bits( 3 ) == 0 )
	    {
		stop_fighting( ch, FALSE );
		
		set_fighting( ch, victim->master);
		return FALSE;
	    }
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }

    if ((IS_FLAG4(victim, AFF_ENTWINED)
	|| IS_FLAG4(victim, AFF_SMOTHERED))
	&& dam_type == DAM_FIRE)
    {
	send_to_char("The flame burns through the plants, they back away.\n\r", ch);
	act("The plants constricting $n back away at the flames.", ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->flag4, AFF_ENTWINED);
	REMOVE_BIT(ch->flag4, AFF_SMOTHERED);
    }

    /*
     * Damage modifiers.
     */

    if ( dam > 1 && !IS_NPC(victim)
    &&   victim->pcdata->condition[COND_DRUNK]  > 10 )
	dam = 9 * dam / 10;
 
    if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;
 
    if ( dam > 1 && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
    ||               (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )))
	dam -= dam / 4;

    immune = FALSE;


    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):     
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_ETHERIAL))
    {
	immune = TRUE;
	dam = 0;
    }

    if (IS_FLAG4(victim, AFF_STASIS))
    {
	immune = TRUE;
 	dam = 0;
	stop_fighting(victim, TRUE);
    }

    if (show)
	dam_message( ch, victim, dam, dt, immune );

    if (dam == 0)
	return FALSE;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "You are mortally wounded, and will die soon, if not aided.\n\r",
	    victim );
	break;

    case POS_INCAP:
	act( "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "You are incapacitated and will slowly die, if not aided.\n\r",
	    victim );
	break;

    case POS_STUNNED:
	act( "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("You are stunned, but will probably recover.\n\r",
	    victim );
	break;

    case POS_DEAD:
	act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	send_to_char( "You have been KILLED!!\n\r\n\r", victim );
	stop_fighting(ch, TRUE);
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "That really did HURT!\n\r", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "#1You sure are BLEEDING!\n\r", victim );
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if (victim->position == POS_DEAD && !IS_NPC(victim) && 
	(victim->secondaryrace!=0 || victim->hit + victim->illus_dam > -10))
    {
	stop_fighting(victim, TRUE);
	victim->hit = -10 - victim->illus_dam;
	return TRUE;
    }
    else
    {
	update_pos(victim);
    }

    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */

    if ( victim->position == POS_DEAD && victim->secondaryrace==0)
    {
	check_quest_kill(ch, victim);

	crash("doodalaly eh!");

	if (IS_SPEC(victim, SPEC_EVERLIVING))
	{
	    victim->hit = victim->max_hit;
	    victim->move = victim->max_move;
	    stop_fighting(victim, TRUE);
	    update_pos(victim);
	    act("$n gets back to $s feet, and all $s wounds heal up.", victim, NULL, NULL, TO_ROOM);
	    return TRUE;
	}
	if (IS_SPEC(victim, SPEC_SPLIT))
	{
	    CHAR_DATA *mob = NULL;

	    victim->max_hit /= 2;
	    if (victim->max_hit > 30)
	    {
	        clone_mobile( victim, mob );
	        if (mob == NULL) return TRUE; 
		mob->max_hit = victim->max_hit;
	    	victim->hit = victim->max_hit;
	    	mob->hit = mob->max_hit;
	    	victim->move = victim->max_move;
	    	mob->move = mob->max_move;
		char_to_room(mob, ch->in_room, ch->altitude);
	    	set_fighting(mob, ch);
	    	set_fighting(victim, ch);
	    	update_pos(victim);
	    	update_pos(mob);
	    	act("#2$n splits in half and both pieces continue fighting.", victim, NULL, NULL, TO_ROOM);
	    	return TRUE;
	    }
	}

	crash("my feet have turned into sausages");


	group_gain( ch, victim );
	if (victim->level > ch->best_mob_killed)
	{
	    ch->best_mob_killed = victim->level;
	    check_player_table(ch);
	}
	if (IS_NPC(victim) && !IS_NPC(ch) )
	    ch->pcdata->mob_kills += 1;	


	if (IS_SPEC(victim, SPEC_FREEZE)) WAIT_STATE(ch, 18);

	crash("my hovercraft is full of eels");

	if ( !IS_NPC(victim) )
	{    
	    if (IS_NPC(ch))
	        victim->pcdata->mob_killed += 1;
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string( log_buf );
	}
	
	sprintf( log_buf, "%s got crapped upon by %s at %s [room %d]",
	    (IS_NPC(victim) ? victim->short_descr : victim->name),
	    (IS_NPC(ch) ? ch->short_descr : ch->name),
	    ch->in_room->name, ch->in_room->vnum);

	if (IS_NPC(victim))
	    wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
	else
	    wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);
     
	crash("my hovercraft is full of rodents");

	raw_kill( victim );
	/* dump the flags */
	if (ch != victim && !IS_NPC(ch) && !is_same_clan(ch,victim))
	{
	    if (IS_SET(victim->act,PLR_KILLER))
		REMOVE_BIT(victim->act,PLR_KILLER);
	    else
		REMOVE_BIT(victim->act,PLR_THIEF);
	}
	    
	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents );
	 
	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
		do_get( ch, "all corpse" );
	     
	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
		corpse && corpse->contains  && /* exists and not empty */  
		!IS_SET(ch->act,PLR_AUTOLOOT))  
	      do_get(ch, "gold corpse");
      
	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
	      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
		return TRUE;  /* leave if corpse has treasure */
	      else
		do_sacrifice( ch, "corpse" );
	}

	return TRUE;
    }

    if ( victim == ch )
	return TRUE;

	crash("what?  no! sir, that will just not do");

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
	do_flee( victim, "" );

    tail_chain( );
    return TRUE;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL)
	return FALSE;
 
    /* This stops link dead people being killed or attacking */
    if (ch->desc == NULL && !IS_NPC(ch)) return TRUE;

    if (victim->desc == NULL && !IS_NPC(victim))
    {
	stc("You cannot attack a link-dead player.\n\r", ch);
	return TRUE;
    }

    /* Beckoned animals and animalism */
    if (IS_NPC(ch) && victim->power[DISC_VAMP_ANIM] > 0
	&& IS_SET(ch->flag4, AFF_BECKONED) )
    {
	stc("You cannot harm such a beautiful person.\n\r", ch);
	act("$N growls at you but backs away.", ch, NULL, victim, TO_CHAR);
	return TRUE;
    }

    /* Lurrrrrrrve */
    if (ch->marriage_status == MARRY_PROPOSING
	&& ch->married_to[0] != '\0')
    {
	if (!str_cmp(ch->married_to, GET_NAME(victim)))
	{
	    stc("You would never do that to a loved one.\n\r", ch);
	    interpret(ch, "smile");
	    return TRUE;
	}
    }
    /* killing mobiles */
    if (IS_NPC(victim))
    {

	/* safe room? */
	if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	{
	    send_to_char("Not in this room.\n\r",ch);
	    return TRUE;
	}

	if (!RTIME_UP(victim->in_room, RTIMER_GHOST_LIGHT))
	{
	    act("$n raises $s weapon, but stops in $s tracks.", ch, NULL, NULL, TO_ROOM);
	    act("$n's skin starts to turn pale.", ch, NULL, NULL, TO_ROOM);
	    stc("The white vapours thicken and stop you attacking.\n\r", ch);
	    return TRUE;
	}

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	||  IS_SET(victim->act,ACT_IS_CHANGER))
	{
	    send_to_char("The powers disapprove of your actions.\n\r",ch);
	    return TRUE;
	}

	if (!IS_NPC(ch))
	{
	    /* mortals and immortals */ 
	    if (!IS_NPC(victim)
		&& (victim->secondaryrace == 0 || IS_TAINTED(victim) ) )
	    {
		stc("You cannot kill mortals.\n\r", ch);
		return TRUE;
	    }

	    if (!IS_NPC(victim)
		&& (ch->secondaryrace == 0 || IS_TAINTED(ch) ) )
	    {
		stc("You cannot player kill while mortal.\n\r", ch);
		return TRUE;
	    }

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim,ACT_PET) && ch != victim->master)
	    {
		send_to_char("You don't own that monster.\n\r",ch);
		return TRUE;
	    }
	}
    }
    /* killing players */
    else
    {
	/* NPC doing the killing */
	if (IS_NPC(ch))
	{
	    /* safe room check */
	    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	    {
		send_to_char("The gods do not permit fighting in this room.\n\r",ch);
		return TRUE;
	    }

	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	    &&  ch->master->fighting != victim)
	    {
		send_to_char("Players are your friends!\n\r",ch);
		return TRUE;
	    }
	}
	/* player doing the killing */
	else
	{
	    /* safe room check */
	    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	    {
		send_to_char("The gods do not permit fighting in this room.\n\r",ch);
		return TRUE;
	    }
	    return FALSE;
	}
    }
    return FALSE;
}
 
bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;

    if (victim == ch && area)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL && !area)
	return FALSE;

    /* killing mobiles */
    if (IS_NPC(victim))
    {
	/* safe room? */
	if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	    return TRUE;

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	||  IS_SET(victim->act,ACT_IS_CHANGER))
	    return TRUE;

	if (!IS_NPC(ch))
	{
	    /* no pets */
	    if (IS_SET(victim->act,ACT_PET))
		return TRUE;

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim,AFF_CHARM) && (area || ch != victim->master))
		return TRUE;

	    /* legal kill? -- cannot hit mob fighting non-group member */
	    if (victim->fighting != NULL && !is_same_group(ch,victim->fighting))
		return TRUE;
	}
	else
	{
	    /* area effect spells do not hit other mobs */
	    if (area && !is_same_group(victim,ch->fighting))
		return TRUE;
	}
    }
    /* killing players */
    else
    {
	if (area && IS_IMMORTAL(victim) && victim->level > LEVEL_IMMORTAL)
	    return TRUE;

	/* NPC doing the killing */
	if (IS_NPC(ch))
	{
	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	    &&  ch->master->fighting != victim)
		return TRUE;
	
	    /* safe room? */
	    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
		return TRUE;

	    /* legal kill? -- mobs only hit players grouped with opponent*/
	    if (ch->fighting != NULL && !is_same_group(ch->fighting,victim))
		return TRUE;
	}

	/* player doing the killing */
	else
	{
	    if (!is_clan(ch))
		return TRUE;

	    if (IS_SET(victim->act,PLR_KILLER) || IS_SET(victim->act,PLR_THIEF))
		return FALSE;

	    if (!is_clan(victim))
		return TRUE;

	    if (ch->level > victim->level + 8)
		return TRUE;
	}

    }
    return FALSE;
}

/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit <= -11 )
    {
	victim->position = POS_DEAD;
	return;
    }

	 if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int stpos = 10;

    if ( ch->fighting != NULL )
	return;

    if ( ch == victim ) return; /* SaNiTy? */

    if (is_item(ch) || is_item(victim)) return;

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, gsn_sleep );

    stpos = des_dist( ch );
    if (des_dist( victim ) > stpos) stpos = des_dist( victim );

    if (ch->fighting != NULL) stpos = 10;
    if (victim->fighting == ch) stpos = victim->distance;

    ch->fighting = victim;
    ch->position = POS_FIGHTING;
    ch->distance = stpos;
    ch->initiative[0] = 0;
    ch->initiative[1] = 0;
    ch->initiative[2] = 0;
    ch->initiative[3] = 0;
    ch->initiative[4] = 0;

    if (IS_FLAG4(victim, AFF_ENTWINED)
	|| IS_FLAG4(victim, AFF_SMOTHERED) )
    {
	act("The plants surrounding $N are cut away.", ch, NULL, victim,TO_ROOM);
	send_to_char("The plants surrounding you are cut away.", victim);
        REMOVE_BIT(victim->flag4, AFF_ENTWINED);
        REMOVE_BIT(victim->flag4, AFF_SMOTHERED);
    }
    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    affect_strip(ch, gsn_air_barrier);

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch == ch || ( fBoth && fch->fighting == ch ) )
	{
	    fch->fighting       = NULL;
	    fch->position       = IS_NPC(fch) ? fch->default_pos : POS_STANDING;
	    update_pos( fch );
	}
    }

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;
    if (ch->made_from != NULL)
    {
	act("$n clatters to the ground, unmoving.", ch, NULL, NULL, TO_ROOM);
	obj_to_room( ch->made_from, ch->in_room, 0 );
	ch->made_from->condition = 1;
	ch->made_from->animated = NULL;
	ch->made_from = NULL;
	return;
    }
    ch->in_room->gore += get_size(ch) + 1;
    if (IS_NPC(ch)) ch->in_room->gore += ch->level / 15;

    if (ch->cur_form != get_normal_form(ch))
    {
    	set_form(ch, get_normal_form(ch));
    	act("$n's dead body blurs and reverts back to $s real form.",ch,NULL,NULL,TO_ROOM);
    }

    if (IS_SPEC(ch, SPEC_FREEZE))
    {
	corpse=create_object(get_obj_index(96), 0);
	obj_to_room(corpse, ch->in_room, ch->altitude);
	corpse->timer = 150;
	act("$n melts into snow.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    /* Stop multiple raise deading and zombifying for xp/practice */
    if (IS_FLAG4(ch, AFF_NO_EXP) )
    {
	act("$n disintegrates as $e hits the floor.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if ( IS_NPC(ch) )
    {
	name            = ch->short_descr;
	corpse          = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer   = number_range( 3, 6 );
	corpse->cost = 0;
	corpse->corpse = ch->pIndexData;
    }
    else
    {
	name            = ch->name;
	corpse          = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer   = number_range( 25, 40 );
	REMOVE_BIT(ch->act,PLR_CANLOOT);
	if (!is_clan(ch))
	    corpse->owner = str_dup(ch->name);
	else
	{
	    corpse->owner = NULL;
	}
		
	corpse->cost = 0;
    }

    /* put money into corpse */
    if (ch->copper > 0)
    {
	OBJ_DATA *cash;

	if (ch->copper > 30000) ch->copper = 30000;
	cash = create_object(get_obj_index(108), 0);
	WRITE_STR(cash->name, "copper pieces gcash");
	sprintf(buf, "%ld copper piece%s", ch->copper, (ch->copper > 1) ? "s" : "");
	WRITE_STR(cash->short_descr, buf);
	sprintf(buf, "You see %ld copper piece%s here.", ch->copper, (ch->copper > 1) ? "s" : "");
	WRITE_STR(cash->description, buf);
	obj_to_obj( cash, corpse );
	cash->value[0] = ch->copper;
	cash->value[1] = CASH_COPPER;
	ch->copper = 0;	
    }
    if (ch->silver > 0)
    {
	OBJ_DATA *cash;
	if (ch->silver > 30000) ch->silver = 30000;
	cash = create_object(get_obj_index(108), 0);
	WRITE_STR(cash->name, "silver coins gcash");
	sprintf(buf, "%ld silver coin%s", ch->silver, (ch->silver > 1) ? "s" : "");
	WRITE_STR(cash->short_descr, buf);
	sprintf(buf, "You see %ld silver coin%s here.", ch->silver, (ch->silver > 1) ? "s" : "");
	WRITE_STR(cash->description, buf);
	obj_to_obj( cash, corpse );
	cash->value[0] = ch->silver;
	cash->value[1] = CASH_SILVER;
	ch->silver = 0;	
    }
    if (ch->gold > 0)
    {
	OBJ_DATA *cash;
	if (ch->gold > 30000) ch->gold = 30000;
	cash = create_object(get_obj_index(108), 0);
	WRITE_STR(cash->name, "gold coins gcash");
	sprintf(buf, "%ld gold coin%s", ch->gold, (ch->gold > 1) ? "s" : "");
	WRITE_STR(cash->short_descr, buf);
	sprintf(buf, "You see %ld gold coin%s here.", ch->gold, (ch->gold > 1) ? "s" : "");
	WRITE_STR(cash->description, buf);
	obj_to_obj( cash, corpse );
	cash->value[0] = ch->gold;
	cash->value[1] = CASH_GOLD;
	ch->gold = 0;	
    }
    if (ch->emeralds > 0)
    {
	OBJ_DATA *cash;
	if (ch->emeralds > 30000) ch->emeralds = 30000;
	cash = create_object(get_obj_index(108), 0);
	WRITE_STR(cash->name, "emeralds gcash");
	sprintf(buf, "%ld emerald%s", ch->emeralds, (ch->emeralds > 1) ? "s" : "");
	WRITE_STR(cash->short_descr, buf);
	sprintf(buf, "You see %ld emerald%s here.", ch->emeralds, (ch->emeralds > 1) ? "s" : "");
	WRITE_STR(cash->description, buf);
	obj_to_obj( cash, corpse );
	cash->value[0] = ch->emeralds;
	cash->value[1] = CASH_EMERALDS;
	ch->emeralds = 0;	
    }
    if (ch->sapphires > 0)
    {
	OBJ_DATA *cash;
	if (ch->sapphires > 30000) ch->sapphires = 30000;
	cash = create_object(get_obj_index(108), 0);
	WRITE_STR(cash->name, "sapphires gcash");
	sprintf(buf, "%ld sapphire%s", ch->sapphires, (ch->sapphires > 1) ? "s" : "");
	WRITE_STR(cash->short_descr, buf);
	sprintf(buf, "You see %ld sapphire%s here.", ch->sapphires, (ch->sapphires > 1) ? "s" : "");
	WRITE_STR(cash->description, buf);
	obj_to_obj( cash, corpse );
	cash->value[0] = ch->sapphires;
	cash->value[1] = CASH_SAPPHIRES;
	ch->sapphires = 0;	
    }
    if (ch->diamonds > 0)
    {
	OBJ_DATA *cash;
	if (ch->diamonds > 30000) ch->diamonds = 30000;
	cash = create_object(get_obj_index(108), 0);
	WRITE_STR(cash->name, "diamonds gcash");
	sprintf(buf, "%ld diamond%s", ch->diamonds, (ch->diamonds > 1) ? "s" : "");
	WRITE_STR(cash->short_descr, buf);
	sprintf(buf, "You see %ld diamond%s here.", ch->diamonds, (ch->diamonds > 1) ? "s" : "");
	WRITE_STR(cash->description, buf);
	obj_to_obj( cash, corpse );
	cash->value[0] = ch->diamonds;
	cash->value[1] = CASH_DIAMONDS;
	ch->diamonds = 0;	
    }

    corpse->size = get_size(ch);
    corpse->level = ch->level;

    free_string(corpse->voodoo_name);
    corpse->voodoo_name = str_dup(GET_PROPER_NAME(ch));

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	bool floating = FALSE;

	obj_next = obj->next_content;

	/* On death, these happen */

	if (IS_OBJ_STAT2(obj, ITEM_LOYAL))
	{
	    REMOVE_BIT(obj->extra_flags2, ITEM_TELEPORTS); /* sanity */
	    continue;
	}
 	else if (IS_OBJ_STAT2(obj, ITEM_TELEPORTS))
	{
	    REMOVE_BIT(obj->extra_flags2, ITEM_LOYAL); /* sanity */
	    obj_from_char(obj);
	    obj_to_room(obj, get_rand_room(), 0);
	    continue;
	}

	if (obj->wear_loc == WEAR_FLOAT)
	    floating = TRUE;

	obj_from_char( obj );

	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500,1000);

	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000,2500);

	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH) && !floating)
	{
	    obj->timer = number_range(5,10);
	    REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
	}

	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    extract_obj( obj );

	else if (floating)
	{
	    if (IS_OBJ_STAT(obj,ITEM_ROT_DEATH)) /* get rid of it! */
	    { 
		if (obj->contains != NULL)
		{
		    OBJ_DATA *in, *in_next;

		    act("$p evaporates,scattering its contents.",
			ch,obj,NULL,TO_ROOM);
		    for (in = obj->contains; in != NULL; in = in_next)
		    {
			in_next = in->next_content;
			obj_from_obj(in);
			obj_to_room(in,ch->in_room, ch->altitude);
		    }
		 }
		 else
		    act("$p evaporates.",
			ch,obj,NULL,TO_ROOM);
		 extract_obj(obj);
	    }
	    else
	    {
		act("$p falls to the floor.",ch,obj,NULL,TO_ROOM);
		obj_to_room(obj,ch->in_room, ch->altitude);
	    }
	}
	else obj_to_obj( obj, corpse );
    }

    obj_to_room( corpse, ch->in_room, ch->altitude );
    return;
}


void death_move (CHAR_DATA *ch, CHAR_DATA *victim)
{
    OBJ_DATA *wield;
    char to_char[MAX_STRING_LENGTH];
    char to_vict[MAX_STRING_LENGTH];
    char to_room[MAX_STRING_LENGTH];

    wield = get_obj_by_type( ch, ITEM_WEAPON );

    if (wield == NULL && (IS_SET(ch->flag2, AFF_BITE) 
	|| IS_SET(ch->flag2, AFF_CLAW) ) )
    {
	if(!IS_SET(ch->flag2, AFF_CLAW) )
	{
	    sprintf(to_char, "You sink your fangs into $N's neck and rip out $S throat.");
	    sprintf(to_vict, "$n sinks $s fangs into your neck and rips out your throat.");
	    sprintf(to_room, "$n sinks $s fangs into $N's neck and rips out $S throat.");
	}
   	else if (dice(1, 2) == 1 && IS_SET(ch->flag2, AFF_CLAW) )
	{
	    sprintf(to_char, "You thrust your claws through $N's eyes and tear off $S face.");
	    sprintf(to_vict, "$n thrusts $s claws into your eyes and tears off your face.");
	    sprintf(to_room, "$n thrusts $s claws into $N's eyes and tears off $S face.");
	}
	else if (IS_SET(ch->flag2, AFF_CLAW) )
	{
	    sprintf(to_char, "Your claws dig straight into $N's chest, ripping out $S heart.");
	    sprintf(to_vict, "$n's claws dig straight into your chest, ripping out your heart.");
	    sprintf(to_room, "$n digs $s claws into $N's chest, ripping out $S heart.");
	}
    }

    else if (wield == NULL)
    {
	if (dice(1, 2) == 1)
	{
	    sprintf(to_char, "You plunge your fist into $N's neck and rip out their throat.");
	    sprintf(to_vict, "$n plunges $s fist into your neck and rips out your throat.");
	    sprintf(to_room, "$n plunges $s fist into $N's neck and rips out $S throat.");
	}
	else
	{
	    sprintf(to_char, "You plunge your hand into $N's chest and rip out $s heart.");
	    sprintf(to_vict, "$n plunges $s hand into your chest and rips out your heart.");
	    sprintf(to_room, "$n plunges $s hand into $N's chest and rips out $S heart.");
	}
    }
	    
    else switch (wield->value[0])
    {
        case(WEAPON_SWORD):
	if (dice(1, 2) == 1)
	{
	sprintf(to_char, "You ram your sword into $N's chest, impaling $M.");
	sprintf(to_vict, "$n rams $s sword into your chest, impaling you through the heart.");
	sprintf(to_room, "$n rams $s sword into $N's chest, impaling $M.");
	}
	else
	{
	sprintf(to_char, "You plunge your sword into $N's neck and firmly twist it.");
	sprintf(to_vict, "$n plunges $s sword into your neck and twists it around.");
	sprintf(to_room, "$n plunges $s sword into $N's neck and twists it around.");
        }
	break;

        case(WEAPON_AXE):
	if (dice(1, 2) == 1)
	{
	sprintf(to_char, "You bring down your axe on $N's head, splitting it in two.");
	sprintf(to_vict, "$n brings down $s axe on your head, splitting it in two.");
	sprintf(to_room, "$n brings down $s axe on $N's head, splitting it in two.");
	}
	else
	{
	sprintf(to_char, "You swing your axe in a mighty arc and decapitate $N.");
	sprintf(to_vict, "$n swings $s axe in a mighty arc and cuts your head off.");
	sprintf(to_room, "$n swings $s axe in a mighty arc and decapitates $N.");
        }
	break;

        case(WEAPON_WHIP):
	{
	sprintf(to_char, "Your whip wraps around $N's neck and strangles $M.");
	sprintf(to_vict, "$n's whip wraps around your neck and chokes you to death.");
	sprintf(to_room, "$n's whip wraps around $N's neck and chokes $M to death.");
	}
	break;

        case(WEAPON_POLEARM):
	{
	sprintf(to_char, "Your weapon impales $N, who slides down the shaft twitching.");
	sprintf(to_vict, "$n's polearm goes through your groin, and out through the back of your neck.");
	sprintf(to_room, "$n's polearm impales $N, who slides down it's shaft, twitching.");
	}
	break;

        case(WEAPON_MACE):
	sprintf(to_char, "You smash your mace into $N's head and break $S skull.");
	sprintf(to_vict, "$n's mace smashes into your face and breaks your skull.");
	sprintf(to_room, "$n swings $s weapon into $N's face and breaks $S skull.");
	break;

        case(WEAPON_DAGGER):
	if (dice(1, 2) == 1)
	{
	sprintf(to_char, "You thrust your dagger into $N's neck and twist violently.");
	sprintf(to_vict, "$n thrusts $s dagger into your neck and twists violently.");
	sprintf(to_room, "$n thrusts $s dagger into $N's neck and twists violently.");
	}
	else
	{
	sprintf(to_char, "You plunge your dagger into $N's eye and pierce $S brain.");
	sprintf(to_vict, "$n plunges $s dagger into your eye, piercing your brain.");
	sprintf(to_room, "$n plunges $s dagger into $N's eye, piercing $S brain.");
        }
	break;

        case(WEAPON_SPEAR):
	if (dice(1, 2) == 1)
	{
	sprintf(to_char, "You impale your spear through $N's stomach.");
	sprintf(to_vict, "$n thrusts $s spear directly through your stomach.");
	sprintf(to_room, "$n thrusts $s spear through $N's stomach and out the other side.");
	}
	else
	{
	sprintf(to_char, "Your spear stakes $N right through the heart, blood sprays everywhere.");
	sprintf(to_vict, "$n stakes $s spear through your heart.");
	sprintf(to_room, "$n stakes $s spear directly through $N's heart.");
        }
	break;

	default : return;
        }

	if (ch->secondaryrace == RACE_DRAGON && dice(1,6) == 1)
	{
	   sprintf(to_char, "Your tail shoots between $N's legs, impaling $M.");
	   sprintf(to_vict, "$n's tail rips your whole body apart from between your legs.");
	   sprintf(to_room, "$n's tail rips $N's whole body apart from between $S legs.");
	}	

 	act(to_char, ch, NULL, victim, TO_CHAR);
 	act_new(to_vict, ch, NULL, victim, TO_VICT, POS_DEAD);
 	act(to_room, ch, NULL, victim, TO_ROOM);
}


/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch, bool corpse )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";
    


    if (corpse
	&& !IS_SPEC(ch, SPEC_FREEZE)) switch ( number_bits(4))
    {
    case  0: msg  = "$n hits the ground ... DEAD.";                     break;
    case  1: 
	if (ch->material == 0)
	{
	    msg  = "$n splatters blood on your armor.";         
	    break;
	}
    case  2:                                                    
	if (IS_SET(ch->parts,PART_GUTS))
	{
	    msg = "$n spills $s guts all over the floor.";
	    vnum = OBJ_VNUM_GUTS;
	}
	break;
    case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    msg  = "$n's severed head plops on the ground.";
	    vnum = OBJ_VNUM_SEVERED_HEAD;                               
	}
	break;
    case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    msg  = "$n's heart is torn from $s chest.";
	    vnum = OBJ_VNUM_TORN_HEART;                         
	}
	break;
    case  5: 
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "$n's arm is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_ARM;                         
	}
	break;
    case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "$n's leg is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_LEG;                         
	}
	break;
    case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    msg = "$n's head is shattered, and $s brains splash all over you.";
	    vnum = OBJ_VNUM_BRAINS;
	}
    }

    if (corpse) act( msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum != 0 && corpse)
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name            = IS_NPC(ch) ? ch->short_descr : ch->name;
	obj             = create_object( get_obj_index( vnum ), 0 );

 	if (!IS_NPC(ch)) SET_BIT(obj->extra_flags2, ITEM_VOO_BODY);
	if (!IS_NPC(ch) && vnum == OBJ_VNUM_BRAINS)
	    SET_BIT(obj->extra_flags2, ITEM_VOO_HEAD);

	obj->timer      = number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	free_string( obj->voodoo_name );
	obj->voodoo_name = str_dup( GET_PROPER_NAME(ch) );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room, ch->altitude );
    }

    if ( IS_NPC(ch) )
	msg = "You hear an agonising death cry in the distance.";
    else
	msg = "You hear someone's agonising death cry in the distance.";

    if ((was_in_room = ch->in_room) == NULL) return;

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}

void make_blood( CHAR_DATA *ch )
{
    int vnum;

    if (IS_SPEC(ch, SPEC_FREEZE)) return;

    vnum = OBJ_VNUM_BLOOD;

    if ( vnum != 0 )
    {
	OBJ_DATA *obj;

	obj             = create_object( get_obj_index( vnum ), 0 );
	obj->timer      = number_range( 1, 4 );

	obj_to_room( obj, ch->in_room, ch->altitude );
	act("Blood sprays everywhere, drenching the floor.", ch,NULL,NULL,TO_ROOM);
    }

}

void raw_kill( CHAR_DATA *victim )
{
    int i;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *owner;

    close_progs( victim );
    taint_untransform(victim);
 
    REMOVE_BIT(victim->flag4, AFF_ENTWINED);
    REMOVE_BIT(victim->flag4, AFF_SMOTHERED);

    victim->currentexp = 0;
    if (victim->faith > 1)
    {
	stc("You have lost faith in the gods.\n\r", victim);
	victim->faith /= 2;
    }

    stop_fighting( victim, TRUE );
    if (dice(1,4)==2 || victim->level>60)
	make_blood( victim );

    /* Crud on owners of a familiar */
    if (IS_FLAG5(victim, AFF5_FAMILIAR))
    {
	for (owner = char_list ; owner != NULL ; owner = owner->next)
	{
	    if (owner->pet == victim)
	    {
	 	stc("You feel a pain in your chest as your familiar dies.\n\r", owner);
		stc("You collapse to the ground.\n\r", owner);
		act("$n roars in pain and collapses to the ground.", owner, NULL, NULL, TO_ROOM);
		owner->hit = -10;
		update_pos(owner);
		wipe_spells(owner);
	    }
	}
    }

    if (victim->secondaryrace == 0 && victim->alignment < -666 && victim->level > 40
	&& !IS_NPC(victim))	
    {
        send_to_char("As you die your mortal soul is wrenched from your body, you do not reform\n\r",victim);
        send_to_char("as usual, but your body melts into the wretched form of a lowly deamon.\n\r", victim);
        send_to_char("You have been punished for your sins.  Welcome to eternal torment.\n\r",victim);
        WAIT_STATE(victim, 16);
        victim->secondaryrace = RACE_DAEMON;
        victim->tertiaryrace = 0;
        victim->generation = 1;
	SET_BIT(victim->act, ACT_UNDEAD);
        victim->hit += 50;
	victim->max_hit += 50;
	set_learnable_disciplines(victim, RACE_DAEMON);
	victim->power[DISC_DAEM_HELL] = 0;
	victim->power[DISC_DAEM_ATTA] = 0;
	sprintf(buf, "-->%s has been transformed into a daemon by the gods.", GET_NAME(victim) );
	informational(buf, 0, 0);
        SET_BIT(victim->imm_flags, IMM_IRON);
	victim->clan = CLAN_DAEM_OUTCAST;
        set_form(victim, 0);
        victim->perm_stat[STAT_INT] -= 4;
	victim->pcdata->damcap = 200;
	set_form(victim, FRM_LEMURE);
        if (victim->fire < 1)
        {
            victim->fire = 1;
            gn_add(victim, 10);
        }
 
        if (victim->entropy < 1)
        {
            victim->entropy = 1;
            gn_add(victim, 50);
        }

    }

    else make_corpse( victim );

	if (victim->riden_by != NULL)
	{
	    send_to_char("You scramble to your feet as your mount dies.\n\r",victim->riden_by);
	    victim->riden_by->riding = NULL;
	    victim->riden_by = NULL;
	}

	if (victim->riding != NULL)
 	{
	    victim->riding->riden_by = NULL;
	    victim->riding = NULL;
	}

    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	return;
    }

    extract_char( victim, FALSE );
    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by = race_table[victim->race].aff;
    for (i = 0; i < 4; i++)
	victim->armor[i]= 100;
    victim->position    = POS_RESTING;
    victim->hit         = UMAX( 1, victim->hit  );
    victim->move        = UMAX( 1, victim->move );
    victim->psp         = UMAX( 1, victim->psp  );
    victim->pcdata->condition[COND_HUNGER] = 60;
    victim->pcdata->condition[COND_THIRST] = 60;
    save_char_obj( victim );
    return;
}

void consume_kill( CHAR_DATA *victim, bool blood )
{
    int i;
    OBJ_DATA *obj, *obj_next;

    close_progs( victim );

    stop_fighting( victim, TRUE );
    if (dice(1,4)==2 || victim->level>60)
    {
	if (blood) make_blood( victim );
    }
	
    /* Ok, for consume kills (no corpse) the artifacts teleport */
    for (obj = victim->carrying ; obj != NULL ; obj = obj_next )
    {
	obj_next = obj->next_content;
	
	if (IS_OBJ_STAT(obj, ITEM_ARTIFACT)
	    || IS_OBJ_STAT(obj, ITEM_RELIC))
  	{
	    obj_from_char(obj);
	    obj_to_room(obj, get_rand_room(), 0);
	    continue;
	}
    }

    death_cry( victim, FALSE );
    
    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	return;
    }

    extract_char( victim, FALSE );
    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by = race_table[victim->race].aff;
    for (i = 0; i < 4; i++)
	victim->armor[i]= 100;
    victim->position    = POS_RESTING;
    victim->hit         = UMAX( 1, victim->hit  );
    victim->move        = UMAX( 1, victim->move );
/*  save_char_obj( victim ); we're stable enough to not need this :) */
    return;
}


void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;

    crash("Group gain routine");

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( victim == ch )
	return;

    SS(0);
    
    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if (gch->altitude != ch->altitude) continue;
	if ( is_same_group( gch, ch ) )
	{
	    members++;
	    group_levels += IS_NPC(gch) ? gch->level / 2 : gch->level;
	}
    }
    SS(1);

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = ch->level ;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;
    SS(2);

    if (ch->level <= victim->level)
    {
        ch->hit += 1;
        ch->max_hit += 1;
        if (!IS_NPC(ch)) ch->pcdata->perm_hit += 1;
    }

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if (gch->altitude != ch->altitude) continue;

	if ( !is_same_group( gch, ch ) || IS_NPC(gch))
	    continue;

	xp = xp_compute( gch, victim, group_levels );  
    SS(3);

	if ( xp >= 0) 
	{
	     sprintf( buf, "You receive %d experience points.\n\r", xp );
	}
	else
	{
	     sprintf( buf, "You LOSE %d experience points!!!\n\r", -xp );
	} /* end xp + or - */

	if (ch->level < 100) send_to_char( buf, gch );
	gain_exp( gch, xp );
    SS(4);

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
    	    &&   ch->level != 120  )
	    {
		act( "$p glows red hot and burns you.", ch, obj, NULL, TO_CHAR );
		act( "$p glows red hot and burns $n.",   ch, obj, NULL, TO_ROOM );
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room, ch->altitude );
	    }
	}
    }
    SS(5);

    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels )
{
    int xp = 0;
    int diff;
    int divider;

    crash("Xp_compute thingy");

    /* First is the alignment thingy */
    if (gch->alignment > 900 && IS_GOOD(victim)) gch->alignment -= dice(2, 10);
    else if (gch->alignment > 800 && IS_GOOD(victim)) gch->alignment -= dice(2, 9);
    else if (gch->alignment > 700 && IS_GOOD(victim)) gch->alignment -= dice(2, 8);
    else if (gch->alignment > 600 && IS_GOOD(victim)) gch->alignment -= dice(2, 6);
    else if IS_GOOD(victim) gch->alignment -= dice(1, 4);

    if (victim->alignment < -666) gch->alignment += 1;

    /* Better limit the alignment somehow */
    if (gch->alignment > 1000) gch->alignment = 1000;
    if (gch->alignment < -1000) gch->alignment = -1000;

    diff = victim->level - gch->level + 5;

    if (diff > 39) xp = diff * 40;
    else if (diff > 29) xp = diff * 35;
    else if (diff > 9) xp = diff * 30;
    else if (diff >= 0) xp = 100 + (diff * 20);
    else if (diff >= -10) xp = 100 + (diff * 5);
    else if (diff >= -20) xp = 90 + (diff * 4);
    else return 0;
    /* Low level characters */
    if (gch->level < 12) xp *= 2;

    /* Player kills */
    if (!IS_NPC(victim)) xp *= 3;

    /* Good killing evil */
    if (IS_GOOD(gch) && IS_EVIL(victim)) xp = xp * 3 / 2;

    /* Evil killing good */
    if (IS_EVIL(gch) && IS_GOOD(victim)) xp = xp * 3 / 2;

    /* Good killing good */
    if (IS_GOOD(gch) && IS_GOOD(victim)) xp = xp / 6;

    if (gch->secondaryrace == RACE_ELADRIN
	&& gch->in_room->sector_type == SECT_FOREST)
    {
	if (IS_GOOD(gch) && IS_EVIL(victim)) xp = xp * 3 / 2;
    }

    /* randomize the rewards */
    xp = number_range (xp * 5/6, xp * 7/6);

    /* Sort out the grouping thing */
    if (total_levels > gch->level && gch->level > 0)
    { 
	divider = total_levels / gch->level;
	if (divider < 1) divider = 1;
	if (divider > 6) divider = 6;
	xp /= divider;
    }	

    /* work out beast */
    if (victim->alignment > 300 && dice(1, 700) < gch->beast + 10
	&& gch->secondaryrace != 0)
    {
	send_to_char("You feel your humanity slipping away.\n\r", gch);
	gch->beast += 1;
	
	if (gch->beast >= 100) gch->beast = 100;

	/* Lupine autopromote is based on the beast */
  	if (gch->beast % 9 == 0 && gch->secondaryrace == RACE_LUPINE
		&& gch->tertiaryrace == 0 && gch->generation < 12)
	{
	    stc("You are now more respected among lupine society.\n\r", gch);
	    gch->generation++;
	    gch->pcdata->damcap += 15;
	}
    }

    /* No exp for zombies and shit like that. */
    if (IS_FLAG4(victim, AFF_NO_EXP)) xp = 0;

    if (xp > 3000) xp = 3000 + xp / 10;

    if (IS_TAINTED(gch) && !IS_NPC(gch))
    {
	stc("Your tainted blood has made you healthier.\n\r", gch);
	gch->hit++;
	gch->pcdata->perm_hit++;
	gch->max_hit++;
	gch->move++;
	gch->pcdata->perm_move++;
	gch->max_move++;
	xp /= 2;
    }

    xp = xp * 2;
 
    /* Discipline points */
    if (!IS_NPC(gch) && xp > 500 && gch->level >= 10
	&& gch->pcdata->disc_points != 666
	&& gch->pcdata->disc_research != -1)
    {
	stc("You gained a discipline point.\n\r", gch);
	gain_disc_points(gch, 1);
    }
    return xp;
}

void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune )
{
    char buf1[256], buf2[256], buf3[256], buf[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;

    if (ch == NULL || victim == NULL)
	return;

    buf[0] = '\0';

	 if ( dam ==   0 ) { vs = "miss";       vp = "misses";          }
    else if ( dam <=   4 ) { vs = "bruise";     vp = "bruises";         }
    else if ( dam <=   6 ) { vs = "scratch";    vp = "scratches";       }
    else if ( dam <=   8 ) { vs = "graze";      vp = "grazes";          }
    else if ( dam <=  12 ) { vs = "hit";        vp = "hits";            }
    else if ( dam <=  16 ) { vs = "injure";     vp = "injures";         }
    else if ( dam <=  20 ) { vs = "wound";      vp = "wounds";          }
    else if ( dam <=  24 ) { vs = "maul";       vp = "mauls";           }
    else if ( dam <=  28 ) { vs = "decimate";   vp = "decimates";       }
    else if ( dam <=  32 ) { vs = "devastate";  vp = "devastates";      }
    else if ( dam <=  36 ) { vs = "maim";       vp = "maims";           }
    else if ( dam <=  42 ) { vs = "MUTILATE";   vp = "MUTILATES";       }
    else if ( dam <=  48 ) { vs = "DISEMBOWEL"; vp = "DISEMBOWELS";     }
    else if ( dam <=  54 ) { vs = "DISMEMBER";  vp = "DISMEMBERS";      }
    else if ( dam <=  60 ) { vs = "MASSACRE";   vp = "MASSACRES";       }
    else if ( dam <=  68 ) { vs = "MANGLE";     vp = "MANGLES";         }
    else if ( dam <=  76 ) { vs = "> CRUSH <";
			     vp = "> CRUSHES <";                        }
    else if ( dam <=  86 ) { vs = "+- DESECRATE -+";
			     vp = "+- DESECRATES -+";                   }
    else if ( dam <=  100 ) { vs = ">- DEMOLISH -<";
			     vp = ">- DEMOLISHES -<";                   }
    else if ( dam <=  110 ) { vs = "*** DEVASTATE ***";
			     vp = "*** DEVASTATES ***";                 }
    else if ( dam <=  130) { vs = "*** EVISCERATE ***";
			     vp = "*** EVISCERATES ***";                }
    else if ( dam <= 160)  { vs = "=== OBLITERATE ===";
			     vp = "=== OBLITERATES ===";                }
    else if ( dam <= 200)  { vs = ">>> ANNIHILATE <<<";
			     vp = ">>> ANNIHILATES <<<";                }
    else if ( dam <= 240)  { vs = "<<< ERADICATE >>>";
			     vp = "<<< ERADICATES >>>";                 }
    else if ( dam <= 280)  { vs = ">-> LIQUEFY <-<";
			     vp = ">-> LIQUEFIES <-<";                  }
    else if ( dam <= 320)  { vs = "ooo PULVERISE ooo";
			     vp = "ooo PULVERISES ooo";                 }
    else if ( dam <= 360)  { vs = "<*<*< NUKE >*>*>";
			     vp = "<*<*< NUKES >*>*>";                  }
    else if ( dam <= 400)  { vs = "++++ PLASMERISE ++++";
			     vp = "++++ PLASMERISES ++++";              }
    else if ( dam <= 500)  { vs = "*****TOTALLY DESTROY*****";
			     vp = "*****TOTALLY DESTROYS*****";              }
    else                   { vs = "*******UTTERLY OBLITERATE*******";
			     vp = "*******UTTERLY OBLITERATES*******";         }




    punct   = (dam <= 50) ? '.' : '!';

    if ( dt == TYPE_HIT )
    {
	if (ch  == victim)
	{
	    sprintf( buf1, "$n %s $mself%c",vp,punct);
	    sprintf( buf2, "You %s yourself%c",vs,punct);
	}
	else
	{
	    sprintf( buf1, "$n %s $N%c",  vp, punct );
	    sprintf( buf2, "You %s $N%c", vs, punct );
	    sprintf( buf3, "$n %s you%c", vp, punct );
	}
    }
    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack      = skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&& dt < TYPE_HIT + MAX_DAMAGE_MESSAGE) 
	    attack      = attack_table[dt - TYPE_HIT].noun;
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].name;
	}
        if (!str_cmp(attack, "punch")
	    && IS_SET(ch->flag2, AFF_CLAW) )
	{
	    sprintf(buf, "claw");
	    attack = buf;
	}	    
        else if (!str_cmp(attack, "punch")
	    && ch->cur_form == FRM_CRIMSON )
	{
	    sprintf(buf, "blood drain");
	    attack = buf;
	}	    
        else if (!str_cmp(attack, "punch")
	    && ch->cur_form == FRM_WHIRLWIND )
	{
	    sprintf(buf, "blast of air");
	    attack = buf;
	}	    
        else if (!str_cmp(attack, "punch")
	    && ch->cur_form == FRM_FLAME )
	{
	    sprintf(buf, "flaming punch");
	    attack = buf;
	}	    
        else if (!str_cmp(attack, "punch")
	    && ch->cur_form == FRM_LIGHT )
	{
	    sprintf(buf, "beam of light");
	    attack = buf;
	}	    
	if (IS_SET(ch->in_room->room_flags, ROOM_ETHERIAL))
	{
	    if (ch == victim)
	    {
		sprintf(buf1,"$n's %s passes harmlessly through $mself.",attack);
		sprintf(buf2,"Luckily, your %s passes harmlessly through you.",attack);
	    }
	    else
	    {
		sprintf(buf1,"#2$n's %s passes harmlessly through $N.",attack);
		sprintf(buf2,"#2Your %s passes harmlessly through $N.",attack);
		sprintf(buf3,"#2$n's %s passes harmlessly through you.",attack);
	    }
	}
	else if (immune)
	{
	    if (ch == victim)
	    {
		sprintf(buf1,"#2$n is unaffected by $s own %s.",attack);
		sprintf(buf2,"#2Luckily, you are immune to that.");
	    } 
	    else
	    {
		sprintf(buf1,"#2$N is unaffected by $n's %s!",attack);
		sprintf(buf2,"#2$N is unaffected by your %s!",attack);
		sprintf(buf3,"#2$n's %s is powerless against you.",attack);
	    }
	}
	else
	{
	    if (ch == victim)
	    {
		sprintf( buf1, "$n's %s %s $m%c",attack,vp,punct);
		sprintf( buf2, "Your %s %s you%c",attack,vp,punct);
	    }
	    else
	    {
		sprintf( buf1, "$n's %s %s $N%c",  attack, vp, punct );
		sprintf( buf2, "Your %s %s $N%c",  attack, vp, punct );
		sprintf( buf3, "$n's %s %s you%c", attack, vp, punct );
	    }
	}
    }

    if (ch == victim)
    {
	act(buf1,ch,NULL,NULL,TO_ROOM);
	act(buf2,ch,NULL,NULL,TO_CHAR);
    }
    else
    {
	act( buf1, ch, NULL, victim, TO_NOTVICT );
	act( buf2, ch, NULL, victim, TO_CHAR );
	act( buf3, ch, NULL, victim, TO_VICT );
    }

    return;
}

void dam_message_new( CHAR_DATA *ch, CHAR_DATA *victim,int dam, char *dt ) 
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    char punct;

    if (ch == NULL || victim == NULL)
	return;

	 if ( dam ==   0 ) { vs = "miss";       vp = "misses";          }
    else if ( dam <=   4 ) { vs = "bruise";     vp = "bruises";         }
    else if ( dam <=   6 ) { vs = "scratch";    vp = "scratches";       }
    else if ( dam <=   8 ) { vs = "graze";      vp = "grazes";          }
    else if ( dam <=  12 ) { vs = "hit";        vp = "hits";            }
    else if ( dam <=  16 ) { vs = "injure";     vp = "injures";         }
    else if ( dam <=  20 ) { vs = "wound";      vp = "wounds";          }
    else if ( dam <=  24 ) { vs = "maul";       vp = "mauls";           }
    else if ( dam <=  28 ) { vs = "decimate";   vp = "decimates";       }
    else if ( dam <=  32 ) { vs = "devastate";  vp = "devastates";      }
    else if ( dam <=  36 ) { vs = "maim";       vp = "maims";           }
    else if ( dam <=  42 ) { vs = "MUTILATE";   vp = "MUTILATES";       }
    else if ( dam <=  48 ) { vs = "DISEMBOWEL"; vp = "DISEMBOWELS";     }
    else if ( dam <=  54 ) { vs = "DISMEMBER";  vp = "DISMEMBERS";      }
    else if ( dam <=  60 ) { vs = "MASSACRE";   vp = "MASSACRES";       }
    else if ( dam <=  68 ) { vs = "MANGLE";     vp = "MANGLES";         }
    else if ( dam <=  76 ) { vs = "> CRUSH <";
			     vp = "> CRUSHES <";                        }
    else if ( dam <=  86 ) { vs = "+- DESECRATE -+";
			     vp = "+- DESECRATES -+";                   }
    else if ( dam <=  100 ) { vs = ">- DEMOLISH -<";
			     vp = ">- DEMOLISHES -<";                   }
    else if ( dam <=  110 ) { vs = "*** DEVASTATE ***";
			     vp = "*** DEVASTATES ***";                 }
    else if ( dam <=  130) { vs = "*** EVISCERATE ***";
			     vp = "*** EVISCERATES ***";                }
    else if ( dam <= 160)  { vs = "=== OBLITERATE ===";
			     vp = "=== OBLITERATES ===";                }
    else if ( dam <= 200)  { vs = ">>> ANNIHILATE <<<";
			     vp = ">>> ANNIHILATES <<<";                }
    else if ( dam <= 240)  { vs = "<<< ERADICATE >>>";
			     vp = "<<< ERADICATES >>>";                 }
    else if ( dam <= 280)  { vs = ">-> LIQUEFY <-<";
			     vp = ">-> LIQUEFIES <-<";                  }
    else if ( dam <= 320)  { vs = "ooo PULVERISE ooo";
			     vp = "ooo PULVERISES ooo";                 }
    else if ( dam <= 360)  { vs = "<*<*< NUKE >*>*>";
			     vp = "<*<*< NUKES >*>*>";                  }
    else if ( dam <= 400)  { vs = "++++ PLASMERISE ++++";
			     vp = "++++ PLASMERISES ++++";              }
    else if ( dam <= 500)  { vs = "*****TOTALLY DESTROY*****";
			     vp = "*****TOTALLY DESTROYS*****";         }
    else                   { vs = "*******UTTERLY OBLITERATE*******";
			     vp = "*******UTTERLY OBLITERATES*******";         }




    punct   = (dam <= 50) ? '.' : '!';

	    if (ch == victim)
	    {
		sprintf( buf1, "$n's %s %s $m%c",dt,vp,punct);
		sprintf( buf2, "Your %s %s you%c",dt,vp,punct);
	    }
	    else
	    {
		sprintf( buf1, "$n's %s %s $N%c",  dt, vp, punct );
		sprintf( buf2, "Your %s %s $N%c",  dt, vp, punct );
		sprintf( buf3, "$n's %s %s you%c", dt, vp, punct );
	    }

    if (ch == victim)
    {
	act(buf1,ch,NULL,NULL,TO_ROOM);
	act(buf2,ch,NULL,NULL,TO_CHAR);
    }
    else
    {
	act( buf1, ch, NULL, victim, TO_NOTVICT );
	act( buf2, ch, NULL, victim, TO_CHAR );
	act( buf3, ch, NULL, victim, TO_VICT );
    }

    return;
}


void custom_dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam, char *dt, char *orig ) 
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    char punct;

    if (ch == NULL || victim == NULL)
	return;

	 if ( dam ==   0 ) { vs = "miss";       vp = "misses";          }
    else if ( dam <=   4 ) { vs = "bruise";     vp = "bruises";         }
    else if ( dam <=   6 ) { vs = "scratch";    vp = "scratches";       }
    else if ( dam <=   8 ) { vs = "graze";      vp = "grazes";          }
    else if ( dam <=  12 ) { vs = "hit";        vp = "hits";            }
    else if ( dam <=  16 ) { vs = "injure";     vp = "injures";         }
    else if ( dam <=  20 ) { vs = "wound";      vp = "wounds";          }
    else if ( dam <=  24 ) { vs = "maul";       vp = "mauls";           }
    else if ( dam <=  28 ) { vs = "decimate";   vp = "decimates";       }
    else if ( dam <=  32 ) { vs = "devastate";  vp = "devastates";      }
    else if ( dam <=  36 ) { vs = "maim";       vp = "maims";           }
    else if ( dam <=  42 ) { vs = "MUTILATE";   vp = "MUTILATES";       }
    else if ( dam <=  48 ) { vs = "DISEMBOWEL"; vp = "DISEMBOWELS";     }
    else if ( dam <=  54 ) { vs = "DISMEMBER";  vp = "DISMEMBERS";      }
    else if ( dam <=  60 ) { vs = "MASSACRE";   vp = "MASSACRES";       }
    else if ( dam <=  68 ) { vs = "MANGLE";     vp = "MANGLES";         }
    else if ( dam <=  76 ) { vs = "> CRUSH <";
			     vp = "> CRUSHES <";                        }
    else if ( dam <=  86 ) { vs = "+- DESECRATE -+";
			     vp = "+- DESECRATES -+";                   }
    else if ( dam <=  100 ) { vs = ">- DEMOLISH -<";
			     vp = ">- DEMOLISHES -<";                   }
    else if ( dam <=  110 ) { vs = "*** DEVASTATE ***";
			     vp = "*** DEVASTATES ***";                 }
    else if ( dam <=  130) { vs = "*** EVISCERATE ***";
			     vp = "*** EVISCERATES ***";                }
    else if ( dam <= 160)  { vs = "=== OBLITERATE ===";
			     vp = "=== OBLITERATES ===";                }
    else if ( dam <= 200)  { vs = ">>> ANNIHILATE <<<";
			     vp = ">>> ANNIHILATES <<<";                }
    else if ( dam <= 240)  { vs = "<<< ERADICATE >>>";
			     vp = "<<< ERADICATES >>>";                 }
    else if ( dam <= 280)  { vs = ">-> LIQUEFY <-<";
			     vp = ">-> LIQUEFIES <-<";                  }
    else if ( dam <= 320)  { vs = "ooo PULVERISE ooo";
			     vp = "ooo PULVERISES ooo";                 }
    else if ( dam <= 360)  { vs = "<*<*< NUKE >*>*>";
			     vp = "<*<*< NUKES >*>*>";                  }
    else if ( dam <= 400)  { vs = "++++ PLASMERISE ++++";
			     vp = "++++ PLASMERISES ++++";              }
    else if ( dam <= 500)  { vs = "*****TOTALLY DESTROY*****";
			     vp = "*****TOTALLY DESTROYS*****";         }
    else                   { vs = "*******UTTERLY OBLITERATE*******";
			     vp = "*******UTTERLY OBLITERATES*******";         }




    punct   = (dam <= 50) ? '.' : '!';

	    if (ch == victim)
	    {
		if (orig != NULL)
		{
		    sprintf(buf1, "%s %s %s $n%c", orig, dt, vp, punct);
		    sprintf(buf2, "%s %s %s you%c", orig, dt, vp, punct);
		}
		else
		{
		    sprintf( buf1, "$n's %s %s $m%c",dt,vp,punct);
		    sprintf( buf2, "Your %s %s you%c",dt,vp,punct);
		}
	    }
	    else
	    {
		sprintf( buf1, "$n's %s %s $N%c",  dt, vp, punct );
		sprintf( buf2, "Your %s %s $N%c",  dt, vp, punct );
		sprintf( buf3, "$n's %s %s you%c", dt, vp, punct );
	    }

    if (ch == victim)
    {
	act(buf1,ch,NULL,NULL,TO_ROOM);
	act(buf2,ch,NULL,NULL,TO_CHAR);
    }
    else
    {
	act( buf1, ch, NULL, victim, TO_NOTVICT );
	act( buf2, ch, NULL, victim, TO_CHAR );
	act( buf3, ch, NULL, victim, TO_VICT );
    }

    return;
}



void do_pounce( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim = NULL;
    EXIT_DATA *pExit = NULL;
    sh_int door= 0;

    one_argument( argument, arg );

    if (!in_were_form(ch))
    {
	stc("You can only do this in werewolf form.\n\r", ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pounce on whom?\n\r", ch );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You cannot pounce while fighting!\n\r", ch );
	return;
    }
 
    if (ch->riding != NULL)
    {
	send_to_char("You must dismount first.\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, arg) ) != NULL)
    {
	send_to_char("You haven't got a long enough run up.\n\r", ch);
	return;
    }

    for (door=0;door<4;door++)
    {
	if ((pExit = ch->in_room->exit[door]) != NULL
	&& !IS_SET(pExit->exit_info, EX_CLOSED) )
	{
	    if ( (victim = get_char_exit(ch,pExit->u1.to_room, arg) )!=NULL) break;
	}
    }

    if (victim == NULL) 
    {
	send_to_char("There is nobody of that name in the vicinity.\n\r",ch);
	return;
    }


    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   ch->in_room == ch->master->in_room )
    {
        send_to_char( "What?  And leave your beloved master?\n\r", ch );
        return;
    }
   
    if ( IS_NEWFLAG(ch, AFF_HOLD) )
    {
        send_to_char("Your feet are held fast!\n\r", ch);
        return;
    }

    if ( !TIME_UP(ch, TIMER_MESMERISED) )
    {
        send_to_char("But you want to stay and watch all the pretty colours.\n\r", ch);
        return;
    }

    if ( !is_room_owner(ch,victim->in_room) && room_is_private(victim->in_room ) )
    {
        send_to_char( "That room is private right now.\n\r", ch );
        return;
    }

    if ( victim == ch )
	return;

    sprintf(buf, "$n springs up into the air and pounces $T.");
    act(buf, ch, NULL, direc_name[door], TO_ROOM);
    act("You spring up into the air and pounce on $N.", ch, NULL, victim,TO_CHAR);

    if (!can_pass_wall(ch, pExit, -1)) return;

    char_from_room( ch );
    char_to_room( ch, victim->in_room, victim->altitude );
    do_look(ch, "auto");

    sprintf(buf, "$n leaps in from the %s and pounces on $N.",direc2_name[door]);
    act(buf, ch, NULL, victim, TO_NOTVICT);

    sprintf(buf, "$n leaps in from the %s and pounces on you.",direc2_name[door]);
    act(buf, ch, NULL, victim, TO_VICT);

    if (!can_kill(ch, victim)) return;

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    set_fighting(ch, victim);
    ch->distance = ch->des_dist;
    victim->distance = 0;
    return;
}

bool can_kill(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
	return FALSE;
    }

    if (victim->position < POS_STUNNED)
    {
	send_to_char("They are already mortally wounded.\n\r", ch);
	return FALSE;
    }

    if ( !IS_NPC(victim) && ch->level < 120)
    {
	if ( victim->secondaryrace == 0 && !IS_NPC(ch) )
	{
	    send_to_char( "You are not allowed to kill mortals.\n\r", ch );
	    return FALSE;
	}
    }

    if (is_wearing_artifact(victim, ARTIFACT_JANDOR) != NULL
	|| IS_FLAG4(victim, AFF_INVULNERABLE))
    {
	act("Your blows are powerless against $N.", ch, NULL, victim, TO_CHAR);
	act("$n tries to attack you, but you are protected.", ch, NULL, victim, TO_VICT);
	act("$n tries to kill $N but is stopped by an invisible shield.", ch, NULL, victim, TO_NOTVICT);
	return FALSE;
    }

    if ( is_flying(victim)
	&& !is_flying(ch)
	&& victim->in_room != NULL
	&& victim->in_room->sector_type != SECT_INSIDE)
    {
	send_to_char("They are flying too high for you to reach.\n\r",ch);
	act("$n tries to attack you, but you are flying too high.",ch,NULL, victim, TO_VICT);
	return FALSE;
    }

    if (victim->cur_form == FRM_MIST)
    {
	act("$S is insubstantial.",ch,NULL,victim,TO_CHAR);
	return FALSE;
    }

    if (ch->cur_form == FRM_MIST)
    {
	send_to_char("But you are insubstantial.\n\r",ch);
	return FALSE;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return FALSE;
    }

    if ( is_safe( ch, victim ) )
	return FALSE;

    return TRUE;
}


void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (victim == ch->pet)
    {
	stc("You wouldnt kill your own pet!\n\r", ch);
	return;
    }

    if ((victim->pet != NULL
	&& IS_FLAG5(victim->pet, AFF5_FAMILIAR_MOUSE))
	|| IS_FLAG5(victim, AFF5_FAMILIAR_MOUSE) )
    {
	stc("They quickly scurry out the way.\n\r", ch);
	return;
    }	

    if ( ch->fighting != NULL && ch->fighting != victim && victim->fighting == ch)
    {
	act("You redirect your assault on $N.", ch, NULL, victim, TO_CHAR);
	act("$n redirects $s assault on $N.", ch, NULL, victim, TO_NOTVICT);
	act("$n redirects $s assault on you.", ch, NULL, victim, TO_VICT);
	ch->fighting = victim;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    if (!can_kill(ch, victim)) return;

    if (get_size(ch) + 1 < get_size(victim))
    {
	act("You leap upon $N.", ch, NULL, victim, TO_CHAR);
	act("$n leaps upon $N.", ch, NULL, victim, TO_NOTVICT);
	act("$n leaps upon you.", ch, NULL, victim, TO_VICT);
    }
    else if (get_size(ch) > get_size(victim) + 1)
    {
	act("You step forward to crush $N.", ch, NULL, victim, TO_CHAR);
	act("$n steps forward to crush $N.", ch, NULL, victim, TO_NOTVICT);
	act("$n steps forward to crush you.", ch, NULL, victim, TO_VICT);
    }
    else
    {
	act("You rush forward and engage $N.", ch, NULL, victim, TO_CHAR);
	act("$n rushes forward to engage $N.", ch, NULL, victim, TO_NOTVICT);
	act("$n rushes forward to attack you.", ch, NULL, victim, TO_VICT);
    }


    if (ch->power[DISC_ELAD_PHYS] > 5
	&& ch->fighting == victim
	&& victim->position == POS_FIGHTING)
    {
	act("Lightning arcs from your body into $N.", ch, NULL, victim, TO_CHAR);
	act("A bolt of lightning arcs from $n's body into yours.", ch, NULL, victim, TO_VICT);
	act("A bolt of lightning arcs from $n's body into $N.", ch, NULL, victim, TO_NOTVICT);
	damage_old(ch, victim, 100 + (ch->generation * 20), skill_lookup("lightning bolt"), DAM_LIGHTNING, TRUE);
    }

    if (ch->secondaryrace == RACE_LUPINE && ch->power[DISC_LUPI_BOAR] > 3
	&& get_size(ch) >= get_size(victim) )
    {
/*	if (ch->power[DISC_LUPI_BOAR] < 5)
	{
	    act("You charge into your opponent, momentarily stunning $M.", ch, NULL, victim, TO_CHAR);
	    act("$n charges into you, momentarily stunning you.", ch, NULL, victim, TO_VICT);
	    act("$n charges into $N, momentarily stunning $M.", ch, NULL, victim, TO_NOTVICT);
	    one_hit(ch, victim, TYPE_UNDEFINED);
	}
	else
	{     
	    act("You charge into your opponent, knocking $M off $S feet.", ch, NULL, victim, TO_CHAR);
	    act("$n charges into you, knocking you off your feet.", ch, NULL, victim, TO_VICT);
	    act("$n charges into $N, $M off $S feet.", ch, NULL, victim, TO_NOTVICT);
	    one_hit(ch, victim, TYPE_UNDEFINED);
	    one_hit(ch, victim, TYPE_UNDEFINED);  
	}*/
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    set_fighting(ch, victim);
    return;
}

void do_tear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if (!in_were_form(ch))
    {
	stc("You can only do this in werewolf form.\n\r", ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You try to tear your own arms off, but to no avail.\n\r", ch );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    if (!can_kill(ch, victim) ) return;

    if (get_size(victim) > get_size(ch))
	send_to_char("Their arms are to big to rip off.\n\r", ch);

    else if (ch->power[DISC_LUPI_URSI] > 3 && dice(1, 100 - get_curr_stat(ch, STAT_STR) * 2) <= 2)
    {
	if (lose_body_part(victim, BODY_LEFT_ARM))
	{
	    act("$n grabs $N's left arm and rips it clean out of the socket.", ch, NULL, victim, TO_NOTVICT);
	    act("You grab $N's left arm and rip it out of its socket.", ch, NULL, victim, TO_CHAR);
	    act("$n grabs your left arm and rips it clean out of the socket.", ch, NULL, victim, TO_VICT);
	}
	else if (lose_body_part(victim, BODY_RIGHT_ARM))
	{
	    act("$n grabs $N's right arm and rips it clean out of the socket.", ch, NULL, victim, TO_NOTVICT);
	    act("You grab $N's right arm and rip it out of its socket.", ch, NULL, victim, TO_CHAR);
	    act("$n grabs your right arm and rips it clean out of the socket.", ch, NULL, victim, TO_VICT);
	}
	else if (lose_body_part(victim, BODY_THIRD_ARM))
	{
	    act("$n grabs $N's third arm and rips it clean out of the socket.", ch, NULL, victim, TO_NOTVICT);
	    act("You grab $N's third arm and rip it out of its socket.", ch, NULL, victim, TO_CHAR);
	    act("$n grabs your third arm and rips it clean out of the socket.", ch, NULL, victim, TO_VICT);
	}
	else if (lose_body_part(victim, BODY_FOURTH_ARM))
	{
	    act("$n grabs $N's fourth arm and rips it clean out of the socket.", ch, NULL, victim, TO_NOTVICT);
	    act("You grab $N's fourth arm and rip it out of its socket.", ch, NULL, victim, TO_CHAR);
	    act("$n grabs your fourth arm and rips it clean out of the socket.", ch, NULL, victim, TO_VICT);
	}
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    set_fighting(ch, victim);
    ch->distance = 0;
    victim->distance = 0;
    return;
}

void do_rampage( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int p;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    if (!can_kill(ch, victim)) return;

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
    for (p = 0 ; p < ch->power[DISC_LUPI_LEON] ; p++)
    {
//	if (dice(1, 4) != 1)
//	    one_hit(ch, victim, TYPE_UNDEFINED);
    }
//    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

void do_crush( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int dam;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Crush whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You cannot crush while fighting.\n\r", ch );
	return;
    }

    if (!is_flying(ch))
    {
	send_to_char("You need to be flying to crush a victim.\n\r", ch);
	return;
    }

    if (is_flying(victim))
    {
	send_to_char("You cannot crush a flying victim.\n\r", ch);
	return;
    }

    if (!can_kill(ch, victim)) return;

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
    act("$n drops from the sky onto $N, crushing $M.", ch, NULL, victim, TO_NOTVICT);
    act("You drop from the sky onto $N, crushing $M.", ch, NULL, victim, TO_CHAR);
    act("$n drops from the sky onto you.", ch, NULL, victim, TO_VICT);
    SET_BIT(ch->flag4, AFF_LANDED);

    /* work out crush damage - its a lot! */
    dam = 45;
    dam *= ch->generation;
    dam *= get_size(ch) + 2;

    damage_old(ch, victim, dam, 0, DAM_BASH, FALSE);
    set_fighting(ch, victim);
    ch->distance = 0;
    victim->distance = 0;
    return;
}


void do_backstab( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

	return;

    one_argument( argument, arg );

    if (arg[0] == '\0')
    {
	send_to_char("Backstab whom?\n\r",ch);
	return;
    }

    if (ch->fighting != NULL)
    {
	send_to_char("You're facing the wrong end.\n\r",ch);
	return;
    }
    

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( !can_kill( ch, victim ) )
      return;

    if ( count_obj_by_type( ch, ITEM_WEAPON ) < 1)
    {
	send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit / 3)
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_backstab].beats );
    if ( number_percent( ) < get_skill(ch,gsn_backstab)
    || ( get_skill(ch,gsn_backstab) >= 2 && !IS_AWAKE(victim) ) )
    {
	check_improve(ch,gsn_backstab,TRUE,1);
//	multi_hit( ch, victim, gsn_backstab );
    }
    else
    {
	check_improve(ch,gsn_backstab,FALSE,1);
	damage( ch, victim, 0, gsn_backstab,DAM_NONE,TRUE);
    }

    return;
}



void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt,dir;

    if ( ( victim = ch->fighting ) == NULL )
    {
	if ( ch->position == POS_FIGHTING )
	    ch->position = POS_STANDING;
	stc("You flee in a random direction!\n\r", ch);
	WAIT_STATE(ch, 2);
	move_char(ch, dice(1, 6)-1, FALSE);
	return;
    }

    WAIT_STATE(ch, 6);

    if (  IS_NEWFLAG(ch, AFF_HOLD) )
    {
	send_to_char("You try to flee, but you are paralysed.",ch);
	return;
    }

    if (ch->rage > 0 && !IS_NPC(ch) )
    {
	send_to_char("Your beast is in control of you and will not let you flee.\n\r", ch);
	return;
    }

    dir = -1;
    if (argument[0]!='\0')
    {
	if (!str_prefix(argument,"north"))
		dir = 0;
	else if (!str_prefix(argument,"east"))
		dir = 1;
	else if (!str_prefix(argument,"south"))
		dir = 2;
	else if (!str_prefix(argument,"west"))
		dir = 3;
	else if (!str_prefix(argument,"down"))
		dir = 4;
	else if (!str_prefix(argument,"up"))
		dir = 5;
	else
	    send_to_char("Thats not a direction!  You panic!\n\r",ch);
    }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if (dir >= 0)
		door = dir;
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	||   number_range(0,ch->daze) != 0
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	move_char( ch, door, FALSE );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n has fled!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC(ch) )
	{
	    send_to_char( "You flee from combat!\n\r", ch );
	}

	stop_fighting( ch, TRUE );
	return;
    }

    send_to_char( "PANIC! You couldn't escape!\n\r", ch );
    return;
}



void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( number_percent( ) > get_skill(ch,gsn_rescue))
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
    act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,gsn_rescue,TRUE,1);

    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );

    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}


void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    if (!IS_GRANTED(ch, CMD_CONTROL) && ch->level!=120)
    {
	send_to_char("You do not have access to this command.\n\r",ch);
	return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level >= get_trust(ch) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
    act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
    act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    raw_kill( victim );
    return;
}

void player_kill_update (CHAR_DATA *ch, CHAR_DATA *victim, bool combat)
{
    add_pkill(ch, victim, combat);
    check_player_table(ch);
    check_player_table(victim);
}

bool player_kill (CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (IS_NPC(ch) || IS_NPC(victim) ) return FALSE;

    if (ch == victim) return FALSE; /* just in case.... */

    if (ch->clan == victim->clan) return FALSE;

    if (ch->secondaryrace == victim->secondaryrace) return FALSE;

    return TRUE;
}


/**************** DO DECAPITATION ROUTINES *********************/

void do_decap(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *wield;
    
    wield = get_obj_by_type( ch, ITEM_WEAPON );

    if (wield == NULL && !IS_NEWFLAG(ch, AFF_CLAW) )
    {
	send_to_char("You must be wielding a weapon to decapitate someone.\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Who do you wish to decapitate?\n\r", ch );
	return;
    }
    
    if ( IS_ROOM_STAT(ch->in_room, ROOM_SAFE) )
    {
	stc("Not in this room.\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }              

    if (IS_NPC(victim) || victim->secondaryrace==0)
    {
	send_to_char("You can't decapitate them.\n\r",ch);
	return;
    }

    if (victim->hit > 0)
    {
	send_to_char("You may only decapitate mortally wounded players.\n\r",ch);
	return;
    }

    if (ch->secondaryrace == RACE_LUPINE && 
	!IS_NEWFLAG(ch, AFF_CLAW) )
    {
	send_to_char("You must have your claws out to decapitate somebody.\n\r", ch);
	return;
    }

    if (ch->secondaryrace != RACE_LUPINE && victim->secondaryrace == RACE_LUPINE 
	&& wield != NULL && !IS_WEAPON_STAT(wield, WEAPON_SILVER))
    {
	send_to_char("You hack at their neck, but your weapon won't cut through their flesh.\n\r",ch);
	return;
    }

    if (player_kill(ch, victim)) player_kill_update(ch, victim, TRUE);

    if (victim->level>1 && victim->level < 112)
	victim->level -= 1;

    act_new("You have been decapitated by $N.",victim,NULL,ch,TO_CHAR,POS_DEAD);
    if (!IS_NEWFLAG(ch, AFF_CLAW))
    {
        act("You bring down your weapon on $N's head, decapitating $M.",ch,NULL,victim,TO_CHAR);
        act("$n brings $s weapon down on $N's head, decapitating $M.",ch,NULL,victim,TO_NOTVICT);
    }
    else
    {
        act("You tear $N's head from $S body with your claws.",ch,NULL,victim,TO_CHAR);
        act("$n rips $N's head from $S body with $s claws.",ch,NULL,victim,TO_NOTVICT);
    }

	sprintf( log_buf, "%s got decapitated by %s at %s [room %d]",
	    (IS_NPC(victim) ? victim->short_descr : victim->name),
	    (IS_NPC(ch) ? ch->short_descr : ch->name),
	    ch->in_room->name, ch->in_room->vnum);

    sprintf(buf, "--> %s has been beheaded by %s.",
	victim->name, ch->name);

    informational(buf, victim->race, 0);
    informational(buf, ch->race, 0);

    death_cry(ch, FALSE);

    /* Make people more evilller */
    if (ch->alignment > 300
	&& ch->secondaryrace == victim->secondaryrace)
    {
	stc("You feel that was not a good idea.\n\r", ch);
	ch->alignment = UMAX(-1000, ch->alignment - 300);
    }

    /* Help the eladrin autopromote */
    if (ch->secondaryrace == RACE_ELADRIN
	&& ch->tertiaryrace == 0
	&& IS_EVIL(victim) )
	gain_elad_path(ch, 0);

    /* Check for drow transformation
	Must be outside and raining/lightning  */
    if (ch->secondaryrace == RACE_ELADRIN
	&& ch->tertiaryrace == 0
	&& victim->tertiaryrace == 0
	&& victim->secondaryrace == RACE_ELADRIN
	&& IS_GOOD(victim)
	&& (weather_info.sky == SKY_LIGHTNING ||
	    weather_info.sky == SKY_RAINING)
	&& IS_OUTSIDE(ch) )
    {
	ch->tertiaryrace = TERTIARY_DROW;
	ch->clan = CLAN_DROW_OUTCAST;
	ch->max_hit += 50;
	ch->max_move += 50;
	ch->perm_stat[STAT_STR] += 2;
	ch->perm_stat[STAT_DEX] += 2;
	ch->alignment = -666;
	WAIT_STATE(ch, 8);
        sprintf(buf, "--> %s has become a drow.",
	    GET_PROPER_NAME(ch));

	act("$n is struck down by a lightning bolt.", ch, NULL, NULL, TO_ROOM);
	stc("As you deal the death blow, a lightning bolt strikes you in the chest.\n\rYour flesh blackens and crisps.\n\r", ch);
        informational(buf, -1, 0);
	player_kill_update(ch, victim, TRUE);
    }	

    /* Spray that magical dragons blood everywhere */

    if (victim->secondaryrace == RACE_DRAGON
	&& victim->dragtype != 0
	&& !IS_AIR(ch->in_room))
    {
	OBJ_DATA *obj;
	int onum = 0;

	if (victim->dragtype == DRAGON_RED) onum = 102;
	else if (victim->dragtype == DRAGON_BLUE) onum = 104;
	else if (victim->dragtype == DRAGON_WHITE) onum = 100;
	else if (victim->dragtype == DRAGON_BLACK) onum = 101;
	else if (victim->dragtype == DRAGON_SHADOW) onum = 111;
	else if (victim->dragtype == DRAGON_SILVER) onum = 105;
	else if (victim->dragtype == DRAGON_GOLD) onum = 106;
	else if (victim->dragtype == DRAGON_GREEN) onum = 103;

	if (onum != 0)
	{
	    obj = create_object(get_obj_index(onum), 0);
	    obj_to_room(obj, ch->in_room, ch->altitude);
	    obj->value[0] = victim->dragtype;
	    obj->timer = 2;
	    act("A pool of $n's blood flows all over the floor.", victim, NULL, NULL, TO_ROOM);
	    stc("A pool of your blood flows everywhere.  You feel weaker.\n\r", victim);
	    victim->generation--;
	    if (victim->generation < 1) victim->generation = 1;
	}
    }


    if (ch->secondaryrace == RACE_DAEMON
	/*&& victim->secondaryrace == RACE_ELADRIN*/	
	&& victim->secondaryrace != RACE_VAMPIRE
	&& victim->tertiaryrace != TERTIARY_DROW
        && victim->tertiaryrace != TERTIARY_LICH
	&& !IS_NPC(victim)
	&& victim->alignment > 666
	&& victim->generation >= ch->generation - 1)
    {
	send_to_char("You wrench away your victim's soul and add it to your pool.\n\r", ch);
	send_to_char("You feel a part of your soul has been torn away.\n\r", victim);
	ch->pcdata->souls++;
	if (ch->pcdata->souls > souls_needed(ch))
	    ch->pcdata->souls = souls_needed(ch);
    }

    wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);
    
    group_gain(ch, victim);
    make_corpse(victim);
    victim->hit = 1;

    /* Makes a head and puts the victim in the head */
    obj = create_object(get_obj_index(12), 0);
    obj->timer = dice(3, 4);
    SET_BIT(obj->extra_flags2, ITEM_VOO_HEAD);
    SET_BIT(obj->extra_flags3, ITEM_DESTROY_CHAR_IF_SACCED);
    SET_BIT(obj->extra_flags3, ITEM_CANNOT_RETURN);
    SET_BIT(obj->extra_flags3, ITEM_CANNOT_MOVE);
     
    sprintf(buf, obj->short_descr, GET_PROPER_NAME(victim) );
    free_string(obj->short_descr);
    obj->short_descr = str_dup(buf);
        
    sprintf(buf, obj->name, GET_PROPER_NAME(victim) );
    free_string(obj->name);
    obj->name = str_dup(buf);

    free_string(obj->voodoo_name);
    obj->voodoo_name = str_dup(GET_PROPER_NAME(victim));
        
    obj_to_room(obj, ch->in_room, 0);
    char_to_obj(victim, obj);
}
	
void destroy_char(CHAR_DATA *ch, CHAR_DATA *victim, bool crp)
{
    OBJ_DATA *corpse;
    
    check_quest_kill(ch, victim);

    crash("Gurgle's noisy nose");

    if (!IS_NPC(ch))
    {
	if (victim->level>1 && victim->level < 112)
		victim->level -= 1;
    }
	group_gain(ch, victim);

	if (!crp)
	{
	    consume_kill(victim, TRUE);
	    return;
	}

	raw_kill( victim );
     
    crash("Gurgle's nose hair");

	if (!IS_NPC(ch)
	&&  (corpse = get_obj_list(ch,"corpse",ch->in_room->contents)) != NULL
	&&  corpse->item_type == ITEM_CORPSE_NPC && can_see_obj(ch,corpse))
	{
	    OBJ_DATA *coins;

	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents );
	       
	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
		do_get( ch, "all corpse" );
				 
	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
		corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
		if ((coins = get_obj_list(ch,"gcash",corpse->contains))
		     != NULL)
		    do_get(ch, "all.gcash corpse");

	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
	      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
		return;  /* leave if corpse has treasure */
	      else
		do_sacrifice( ch, "corpse" );
	}
        crash("Jander smell badly");
     
	return;
}
	
