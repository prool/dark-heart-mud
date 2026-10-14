/******************************************************************
 * Hero -- (C) 1996, by Adam Powell                               *
 * All functions coded by AdAm PoWeLl (Hero and Aasimon powers)   *
 ******************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hell.h"
#include "magic.h"
#include "tables.h"
#include "lookup.h"

DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_save		);
DECLARE_DO_FUN(do_drop		);
DECLARE_DO_FUN(do_group		);
DECLARE_DO_FUN(do_treasure	);

int	count_heroes	args( (ROOM_INDEX_DATA *room) );

void do_reimburse(CHAR_DATA *ch, char *argument) 
{
    int a;
    CHAR_DATA *victim;
    char buf[200];

    if ( ( victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim) || IS_FLAG6(victim, AFF6_REIMBURSEDTWICE) ) 
    {
	stc("No, they don't deserve it.\n\r", ch);
	return;
    }

    a = 200+dice(1, 200);

    victim->version = 8;
    victim->pcdata->perm_hit = a;
    victim->max_hit = a;
    victim->hit = a;
    victim->move = a;
    victim->qp = dice(1, 10);
    victim->faith = dice(1, 20); 
    SET_BIT(victim->flag6, AFF6_REIMBURSEDTWICE);
    sprintf(buf, "This stuff is for %s. HANDS OFF!", GET_NAME(victim) );
    do_say(ch, buf);
    do_treasure(ch, "weapon");
    do_treasure(ch, "ring");
    do_treasure(ch, "ring");
    do_drop(ch, "all");
    do_say(ch, "Have a nice day!");
    save_char_obj(victim);
}

bool is_clan_leader(CHAR_DATA *ch)
{ 
    if (IS_CREATOR(ch)) return TRUE;
    if (IS_NPC(ch)) return FALSE;
    if (!str_cmp(clan_data[ch->clan].leader, GET_PROPER_NAME(ch))) return TRUE;
    return FALSE;
}

void do_clanleader(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to make a clan leader?\n\r", ch);
	return;
    }

    if ( (victim = get_char_world(ch, argument) ) == NULL)
    {
	stc("No people of that name found.\n\r", ch);
	return;
    }

    if (IS_NPC(victim) )
    {
	stc("You can only make mortals clan leaders.\n\r", ch);
	return;
    }

    if (victim->clan == CLAN_NONE)
    {
	stc("They are not in a clan.\n\r", ch);
	return;
    }	

    if (ch != victim) stcf(ch, "%s is now the leader of %s.\n\r",
	GET_PROPER_NAME(victim), clan_table[victim->clan].name);
    WRITE_STR(clan_data[victim->clan].leader, GET_PROPER_NAME(victim) );
    stcf(victim, "You are now the leader of clan %s.\n\r",
	clan_table[victim->clan].name);
    do_save(victim, "");
}

void do_clanlist(CHAR_DATA *ch, char *argument)
{
    int i;
    char clanname[100];
    bool matching = FALSE, found = FALSE;
    char buf[100];
 
    if (argument[0] != '\0')
	matching = TRUE;  

    sprintf(buf, "Clans of %s", NAME_OF_MUD);
    ctc(buf, ch);
    ctc("_-_-_-_-_-_-_-_-_-_", ch);
    stc("\n\r", ch);

    for (i = 0 ; clan_table[i].name != NULL ; i++)
    {
	if (i == CLAN_LUPI_ROGUE
	    || i == CLAN_DAEM_OUTCAST
	    || i == CLAN_DROW_OUTCAST
	    || i == CLAN_DROW_FIRST
	    || i == CLAN_DROW_SECOND
	    || i == CLAN_DROW_THIRD
	    || i == CLAN_DROW_FOURTH
	    || i == CLAN_DROW_FIFTH) continue;

	if (matching && str_prefix(argument, clan_table[i].name) )continue; 
	if (matching) found = TRUE;

	if (clan_table[i].name[0] != '\0')
	{
	    sprintf(clanname, "%s", clan_table[i].name);
	    clanname[0] = UPPER(clanname[0]);

	    stcf(ch, "%s, at war with %s, and led by %s.\n\r",
		clanname, clan_fighting(i), 
		!str_cmp(clan_data[i].leader, "<none>") ? "nobody" : 
		    clan_data[i].leader);
	}
    }
    if (matching && !found)
	stc("No clans of that name found.\n\r", ch);

}

void do_clandesc(CHAR_DATA *ch, char *argument)
{
    if (ch->clan == CLAN_NONE)
    {
	stc("You are not in a clan.\n\r", ch);
	return;
    }

    if (!is_clan_leader(ch))
    {
	stc("Only the leader of a clan may set its description.\n\r", ch);
	return;
    }

    if (!IS_ROOM_STAT(ch->in_room, ROOM_SAFE) && ch->level != 120)
    {
	stc("Editing the clan description can only be done in a safe room.\n\r", ch);
	return;
    }

    do_edit(ch, &clan_data[ch->clan].description, "save", "Editing clan description");
}

void do_clanwar(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int clan;

    if (ch->clan == CLAN_NONE)
    {
	stc("You are not in a clan.\n\r", ch);
	return;
    }

    if (!is_clan_leader(ch))
    {
	stc("Only the leader of a clan may declare war.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("What clan do you wish to make your feudal enemy?\n\r", ch);
	stc("Also type 'clanwar everybody' or 'clanwar noone'\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "everybody"))
    {
	stc("You have declared war on everybody.\n\r", ch);
	clan_data[ch->clan].fighting = 0;
	save_clan_leaders();
	return;
    }
    else if (!str_prefix(argument, "noone"))
    {
	stc("Your clan has declared peace.\n\r", ch);
	clan_data[ch->clan].fighting = -1;
	save_clan_leaders();
	return;
    }

    if ((clan = clan_lookup(argument)) == CLAN_NONE)
    {
	stc("That is not a proper clan.\n\r", ch);
	return;
    }

    if (clan == ch->clan)
    {
	stc("You cannot declare war on yourself!\n\r", ch);
	return;
    }

    if ( clan == clan_data[ch->clan].ally )
    {
	stc("You cannot declare war on your allies.\n\r", ch);
	return;
    }

    clan_data[ch->clan].fighting = clan;

    for ( victim = char_list ; victim != NULL ; victim = victim->next )
    {
	if ( victim->clan == ch->clan )
	    stcf(victim, "Your clan has now declared war upon %s.\n\r", 
		clan_table[clan].name);
	else if ( victim->clan == clan )
	    stcf(victim, "%s has declared war upon your clan.\n\r", 
		clan_table[ch->clan].name);
    }

    save_clan_leaders();
}

void do_clanally(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int clan;

    if (ch->clan == CLAN_NONE)
    {
	stc("You are not in a clan.\n\r", ch);
	return;
    }

    if (!is_clan_leader(ch))
    {
	stc("Only the leader of a clan may declare an ally.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("What clan do you wish to make your ally?\n\r", ch);
	stc("Also type 'clanally everybody' or 'clanally noone'\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "everybody"))
    {
	stc("You have declared alliegance with everybody.\n\r", ch);
	clan_data[ch->clan].ally = 0;
	save_clan_leaders();
	return;
    }
    else if (!str_prefix(argument, "noone"))
    {
	stc("Your clan now has no allies.\n\r", ch);
	clan_data[ch->clan].ally = -1;
	save_clan_leaders();
	return;
    }

    if ((clan = clan_lookup(argument)) == CLAN_NONE)
    {
	stc("That is not a proper clan.\n\r", ch);
	return;
    }

    if (clan == ch->clan)
    {
	stc("You are already allies with yourself!\n\r", ch);
	return;
    }

    if ( clan == clan_data[ch->clan].fighting )
    {
	stc("You cannot declare alliegance with your enemies.\n\r", ch);
	return;
    }

    clan_data[ch->clan].ally = clan;

    for ( victim = char_list ; victim != NULL ; victim = victim->next )
    {
	if ( victim->clan == ch->clan )
	    stcf(victim, "Your clan has now declared alliegance with %s.\n\r", 
		clan_table[clan].name);
	else if ( victim->clan == clan )
	    stcf(victim, "%s has declared alliegance with your clan.\n\r", 
		clan_table[ch->clan].name);
    }

    save_clan_leaders();
}

void do_successor(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if (ch->clan == CLAN_NONE)
    {
	stc("You aren't in a clan.\n\r", ch);
	return;
    }

    if (!is_clan_leader(ch))
    {
	stc("Only the clan leader can use this ability.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("Who do you wish to make your successor?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room(ch, argument) ) == NULL )
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (victim->clan != ch->clan)
    {
	stc("They aren't in your clan.\n\r", ch);
	return;
    }	

    sprintf(buf, "%s, you are the new clan leader.", GET_PROPER_NAME(victim) );
    do_say(ch, buf);
  
    
    if (ch->sex == SEX_FEMALE) do_say(victim, "Thankyou my lady, I will not fail you.");
    else do_say(victim, "Thankyou my lord, I will not fail you.");

    WRITE_STR(clan_data[ch->clan].leader, GET_PROPER_NAME(victim) );
    save_clan_leaders();
}


void do_fire(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim = NULL;
    OBJ_DATA *bow = NULL;
    OBJ_DATA *arrow = NULL;
    OBJ_DATA *quiver = NULL;
    int dir, die;
    ROOM_INDEX_DATA *room; 
    int door;
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pExit;
    char * randhit;
    int chance, dam;

    if (IS_NEWFLAG(ch, AFF_CLAW))
    {
	stc("You cannot fire a bow with claws extended.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("Syntax : fire {direction creature}\n\r", ch);
	return;
    }

    if ( ( bow = get_obj_by_type(ch, ITEM_BOW) ) == NULL)
    {
	stc("You aren't carrying a bow.\n\r", ch);
	return;
    }

    chance = number_percent();
    if (dice(1, 3) == 1) chance /= 2;
    chance -= bow->value[1]; /* Some bows are very accurate */

    WAIT_STATE(ch, 12);

    /* Not strong enough to pull the bow back? */
    if ( bow->value[3] != 0
	  && get_curr_stat(ch, STAT_STR) < bow->value[3] )
    {
	act("You aren't strong enough to pull $p back.", ch, bow, NULL, TO_CHAR);
	act("$n strains to pull $p back, but is too weak.", ch, bow, NULL, TO_ROOM);
	return;
    }

    /* Dont need a quiver in some cases! */
    if (bow->value[2] == 0) /* value 2 is spellbow */
    {
        for (quiver = ch->carrying ; quiver != NULL ; quiver = quiver->next_content)
	    if (quiver->item_type == ITEM_QUIVER) break;

        if (quiver == NULL)
        {
	    stc("You have no quiver to get arrows from.\n\r", ch);
	    return;
        }

        /* Get the arrow from the quiver */
	if ((arrow = quiver->contains) == NULL)
	{
	    act("$p contains no arrows or bolts.", ch, quiver, NULL, TO_CHAR);
	    return;
	}

	if (arrow->item_type != ITEM_ARROW)
  	{
	    act("You draw out $p, however it is not an arrow.", ch, arrow, NULL, TO_CHAR);
	    obj_from_obj(arrow);
	    obj_to_char(arrow, ch);
	    return;
	}
	
	obj_from_obj(arrow);
  	obj_to_char(arrow, ch);
	/* That was just for act statements.... the arrow will move eventually */
    }

    /* Set randhit... quite funny really */
    die = dice(1, 6);
    
    if (die == 1) randhit = "back";
    else if (die == 2) randhit = "left arm";
    else if (die == 3) randhit = "right arm";
    else if (die == 4) randhit = "torso";
    else if (die == 5) randhit = "chest";
    else randhit = "leg";
 

    /* Check for a creature first! */
    for (door = 0 ; door < 6; door++)
    {
        if ((pExit = ch->in_room->exit[door]) != NULL
        && !IS_SET(pExit->exit_info, EX_CLOSED))
        {
            if ( (victim = get_char_exit(ch,pExit->u1.to_room, argument) ) != NULL)
	    {
		if ( chance > get_skill(ch, gsn_archery))
		{
		    sprintf(buf, "%s", dir_name[door]);
		    do_fire(ch, buf);
		    check_improve(ch, gsn_archery, FALSE, 2);  
		    return;
		}

		if (arrow != NULL)
		{
	  	    stcf(ch, "You pull back %s and fire %s %s.\n\r",
		        bow->short_descr, arrow->short_descr, dir_name[door]);
		    sprintf(buf, "$n pulls back $p and fires %s %s.",
			arrow->short_descr, dir_name[door]);
		    act(buf, ch, bow, NULL, TO_ROOM);

		    sprintf(buf, "$p comes flying in from the %s and hits $n in the %s.",
		        dir_name[revdir(door)], randhit);
		    act(buf, victim, arrow, NULL, TO_ROOM);

		    sprintf(buf, "%s comes flying in from the %s and hits you in the %s.\n\r",
		        arrow->short_descr, dir_name[revdir(door)], randhit);
		    stc(buf, victim);
		}
		else 
		{
	  	    stcf(ch, "You pull back %s and fire a blast of energy %s.\n\r",
		        bow->short_descr, dir_name[door]);
		    sprintf(buf, "$n pulls back $p and fires a blast of energy %s.",
			dir_name[door]);
		    act(buf, ch, bow, NULL, TO_ROOM);

		    sprintf(buf, "A bolt of energy comes flying in from the %s and hits $n.",
		        dir_name[revdir(door)]);
		    act(buf, victim, NULL, NULL, TO_ROOM);

		    sprintf(buf, "A bolt of energy comes flying in from the %s and hits you.",
		        dir_name[revdir(door)]);
		    act(buf, victim, NULL, NULL, TO_CHAR);
		}

	        break;
	    }
	}
    }

    if (victim == NULL) /* Check for creatures in room next */
    {
	if ( ( victim = get_char_room(ch, argument) ) != NULL )
   	{
	    if (!is_flying(victim)
		&& !is_flying(ch) )
	    {
		act("$N too near to shoot.", ch, NULL, victim, TO_CHAR);
	 	return;
	    }

	    if (arrow != NULL)
 	    {
	        stcf(ch, "You pull back %s and fire %s at %s.\n\r",
	             bow->short_descr, arrow->short_descr, GET_NAME(victim));
	        sprintf(buf, "%s pulls back %s and fires %s at you.\n\r",
	             GET_NAME(ch), bow->short_descr, arrow->short_descr);
		stc(buf, victim);
			
 	        sprintf(buf, "$n pulls back $p and fires %s at $N.",
		    arrow->short_descr);
	        act(buf, ch, bow, victim, TO_NOTVICT);
	    }
	    else
 	    {
	        stcf(ch, "You pull back %s and fire a bolt of energy at %s.\n\r",
	             bow->short_descr, GET_NAME(victim));
	
	        sprintf(buf, "%s pulls back %s and fires a bolt of energy at you.\n\r",
	             GET_NAME(ch), bow->short_descr);
	 	stc(buf, victim);

 	        sprintf(buf, "$n pulls back $p and fires a bolt of energy at $N.");
	        act(buf, ch, bow, victim, TO_NOTVICT);
	    }
	}
    }

    if (victim == NULL) /* Check for exits next! */
    {
	int while_counter = 0;

	if ((dir = get_direction(argument)) >= 0)
	{
	    if (ch->in_room->exit[dir] == NULL)
	    {
	 	stc("You cannot fire in that direction.\n\r", ch);
		return;
	    }

	    room = ch->in_room->exit[dir]->u1.to_room;

	    while (while_counter++, room->exit[dir] != NULL)
	    {
		/* Max distance, and stops infinite looping */
		if (while_counter >= get_size(ch) * 2) break;
	
		if (dice(1, 3) == 1) 
		{
		    victim = get_rand_person_in_room(room);
		    if (victim != NULL) break;
		}
		else
		{
		    victim = NULL;
		    if (arrow)
		        sprintf(buf, "%s whizzes past your head.", arrow->short_descr);
		    else sprintf(buf, "A bolt of energy whizzes past your head.");
		    room_message(room, buf);
		}

		room = room->exit[dir]->u1.to_room;
	    }

	    if (dice(1, 2) == 1) check_improve(ch, gsn_archery, FALSE, 2);  

	    if (victim == NULL)
	    {
		if (arrow != NULL)
		{
	  	    stcf(ch, "You pull back %s and fire %s %s.\n\r",
		        bow->short_descr, arrow->short_descr, dir_name[dir]);

		    sprintf(buf, "$n pulls back $p and fires %s %s.",
			arrow->short_descr, dir_name[dir]);
		    act(buf, ch, bow, NULL, TO_ROOM);

		    sprintf(buf, "%s comes flying in from the %s and lands on the floor.",
		        arrow->short_descr, dir_name[revdir(dir)]);
		}
		else 
		{
	  	    stcf(ch, "You pull back %s and fire a blast of energy %s.\n\r",
		        bow->short_descr, dir_name[dir]);
		    sprintf(buf, "$n pulls back $p and fires a blast of energy %s.",
			dir_name[dir]);
		    act(buf, ch, bow, NULL, TO_ROOM);

		    sprintf(buf, "A glowing arrow comes flying in from the %s and lands harmlessly on the floor.",
			dir_name[revdir(dir)]);
		}

		if (arrow != NULL) 
		{
		    obj_from_char(arrow);
		    obj_to_room(arrow, room, 0);
		}
		room_message(room, buf);
		return;
	    }
	    else /* the arrow hits the guy.  i love this code! */
	    {
		if (arrow != NULL)
		{
	  	    stcf(ch, "You pull back %s and fire %s %s.\n\r",
		        bow->short_descr, arrow->short_descr, dir_name[dir]);
		    sprintf(buf, "$n pulls back $p and fires %s %s.",
			arrow->short_descr, dir_name[dir]);
		    act(buf, ch, bow, NULL, TO_ROOM);

		    sprintf(buf, "%s comes flying in from the %s and hits $n in the %s.",
		        arrow->short_descr, dir_name[revdir(dir)], randhit);
		    act(buf, victim, arrow, NULL, TO_ROOM);

		    sprintf(buf, "%s comes flying in from the %s and hits you in the %s.",
		        arrow->short_descr, dir_name[revdir(dir)], randhit);
		    act(buf, victim, NULL, NULL, TO_CHAR);
		}
		else 
		{
	  	    stcf(ch, "You pull back %s and fire a blast of energy %s.\n\r",
		        bow->short_descr, dir_name[dir]);
		    sprintf(buf, "$n pulls back $p and fires a blast of energy %s.",
			dir_name[dir]);
		    act(buf, ch, bow, NULL, TO_ROOM);

		    sprintf(buf, "A bolt of energy comes flying in from the %s and hits $n.",
		        dir_name[revdir(dir)]);
		    act(buf, victim, NULL, NULL, TO_ROOM);

		    sprintf(buf, "A bolt of energy comes flying in from the %s and hits you.",
		        dir_name[revdir(dir)]);
		    act(buf, victim, NULL, NULL, TO_CHAR);
		}
  	    }
	}
    }

    /* At this point the arrow is still on the character who fired
	but all act statements apart from damage and spell effects are done */

    if (victim == ch)
    {
	stc("You cannot fire arrows at yourself.\n\r", ch);
	return;
    }

    if (victim == NULL)
    {
	stc("Who are you going to fire at?\n\r", ch);
	return;
    }

    if (IS_INSUBSTANTIAL(victim) && arrow != NULL)
    {
	act("$p drops through $n's insubstantial body.", ch, arrow, NULL, TO_ROOM);
	act("$p drops through your insubstantial body.", ch, arrow, NULL, TO_CHAR);
	if (!in_same_room(ch, victim))
	    act("$p passes straight through $N and clatters to the ground.", ch, arrow, victim, TO_CHAR);
	obj_from_char(arrow);
	obj_to_room(arrow, victim->in_room, victim->altitude);
	return;
    }

    /* Spellbow */
    if (arrow == NULL)
    {
	obj_cast_spell(bow->value[2], bow->level, ch, victim, bow);
	
	/* Will the bow lose its magic forever? */
	if (dice(1, 50) == 1)
   	{
	    bow->value[2] = 0;
	    act("$p fizzles and sparks.", ch, bow, NULL, TO_CHAR);
	}

	return;
    }

    dam = dice(arrow->value[1], arrow->value[2]);
    dam += get_curr_stat(ch, STAT_STR);
    deal_damage(ch, victim, dam);
    damage_obj(arrow, dam);
    obj_from_char(arrow);
    obj_to_room(arrow, victim->in_room, victim->altitude);

    if (arrow->value[4] != 0)
	check_funky_weapon(ch, victim, arrow);

    if (arrow->value[5] != 0)
	obj_cast_spell(arrow->value[5], arrow->level, ch, victim, arrow); 
}

int revdir(int direction)
{
    if (direction == DIR_NORTH) return DIR_SOUTH;
    if (direction == DIR_SOUTH) return DIR_NORTH;
    if (direction == DIR_EAST) return DIR_WEST;
    if (direction == DIR_WEST) return DIR_EAST;
    if (direction == DIR_UP) return DIR_DOWN;
    if (direction == DIR_DOWN) return DIR_UP;
    return -1;
}

CHAR_DATA * get_rand_person_in_room (ROOM_INDEX_DATA *room)
{
    CHAR_DATA *victim;
    int count = 0, looper;

    if (room == NULL) return NULL;
    if (room->people == NULL) return NULL;

    for (victim = room->people ; victim != NULL ; victim = victim->next_in_room)
	count++;

    victim = room->people;

    if (count == 1) return victim;

    for ( looper = 0 ; looper < dice(1, count) - 1 ; looper++)
	victim = victim->next_in_room;

    return victim;
}

void do_exorcise(CHAR_DATA *ch, char *argument)
{
    act("$n starts chanting words softly.", ch, NULL, NULL, TO_ROOM);
    stc("You attempt to exorcise the evil spirits.\n\r", ch);
    ch->move /= 2;
    obj_exorcise(ch);
}

void do_heal(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (IS_NIGHT())
    {
	stc("You cannot use the heal power at night.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_HEAL))
    {
	stcf(ch, "You cannot use this power for another %d ticks.\n\r",
		TIMER(ch, TIMER_HEAL) );
	return;
    }

    if (ch->power[DISC_HERO_VIVI] < 10 || argument[0] == '\0')
    {
        restore_char(ch);
        SET_TIMER(ch, TIMER_HEAL, dice(6, 4) );
	stc("A beam of brilliant white light shoots down from the sky.\n\r", ch);
	stc("You have been restored by the gods.\n\r", ch);
	act("A brilliant beam of white light shoots down and engulfs $n.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if ( ( victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("Syntax : heal <character>\n\r", ch);
	stc("	      or just heal to heal self\n\r", ch);
	return;
    }

    stc("A beam of brilliant white light shoots down from the sky.\n\r", victim);
    stc("You have been restored by the gods.\n\r", victim);
    act("A brilliant beam of white light shoots down and engulfs $n.", victim, NULL, NULL, TO_ROOM);
    restore_char(victim);
        SET_TIMER(ch, TIMER_HEAL, dice(4, 4) );

    if (IS_EVIL(victim))
    {
	stc("You have healed an evil person.  The gods look unfavourably on you.\n\r", ch);
	ch->qp--;
	return;
    }
}

void do_truefaith(CHAR_DATA *ch, char *argument)
{
    int cost;

    cost = ch->true_faith + 10;

    if (ch->faith < cost)
    {
	stcf(ch, "You need %d more faith point%c.\n\r", cost-ch->faith,
	cost-ch->faith == 1 ? "" : "s");
	return;
    }

    ch->true_faith++;
    ch->faith -= cost;

    stc("You get down on your knees.\n\r", ch);
    act("$n gets down on $s knees.", ch, NULL, NULL, TO_ROOM);
    ch->position = POS_RESTING;
    stc("You have gained a point of true faith.\n\r", ch);
}

void do_youth(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (!TIME_UP(ch, TIMER_YOUTH))
    {
	stc("You must wait a while to use this power again.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("Whose age do you wish to restore?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (!can_age(victim))
    {
	stc("Age is no worry to them.\n\r", ch);
	return;
    }

    if (IS_EVIL(victim) || IS_NPC(victim) )
    {
	stc("You cannot affect such a creature.\n\r",ch);
	return;
    }

    if (IS_RESISTING(victim) && victim != ch)
    {
	stc("They must lower their resistance first.\n\r", ch);
	return;
    }

    if (victim->pcdata->extra_age < -3)
    {
	stc("You cannot make them any younger.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);
    SET_TIMER(ch, TIMER_YOUTH, dice(4, 3) );

    victim->pcdata->extra_age -= dice(2, 2);

    if (ch != victim)
    {
        act("$n lays $s hands upon $N.", ch, NULL, victim, TO_ROOM);
        act("You lay your hands upon $N.", ch, NULL, victim, TO_CHAR);
        act("$n lays $s hands upon you.", ch, NULL, victim, TO_VICT);
    }
    act("$n looks younger.", victim, NULL, NULL, TO_ROOM);
    stc("You feel younger.\n\r", victim);
}

void do_dispel (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
  
    if (argument[0] == '\0')
    {
	stc("Dispel whose magical affects?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 10);
    act("You lay your hands upon $N.", ch, NULL, victim, TO_CHAR); 
    act("$n lays $s hands upon $N.", ch, NULL, victim, TO_NOTVICT); 
    act("$n lays $s hands upon you.", ch, NULL, victim, TO_VICT);
 

}

void do_call (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *horse;

    if (!TIME_UP(ch, TIMER_CAN_CALL_WAR_HORSE))
    {
	send_to_char("You call out but nothing arrives.\n\r", ch);
	WAIT_STATE(ch, 10);
	return;
    }

    do_say(ch, "A horse, a horse, my kingdom for a horse.");

    horse = create_mobile(get_mob_index(31));
    act("A fine war horse gallops up to $n.", ch, NULL, NULL, TO_ROOM);
    send_to_char("A large war horse gallops up to you.\n\r", ch);
    char_to_room(horse, ch->in_room, ch->altitude);
    add_follower(horse, ch);
    SET_TIMER(ch, TIMER_CAN_CALL_WAR_HORSE, dice(2, 8) );
    SET_TIMER(horse, TIMER_WAR_HORSE_GO, dice(2, 6) );
    do_group(ch, "war horse");
}


void do_seneschia(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->flag4, AFF_SENESCHIA))
    {
	send_to_char("You reform out of the leaves into your normal form.\n\r", ch);
	act("$n forms out of a pile of leaves lying on the forest floor.", ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->flag4, AFF_SENESCHIA);
	WAIT_STATE(ch, 12);
	return;
    }

    if (ch->in_room->sector_type != SECT_FOREST)
    {
	send_to_char("You may only use this ability in the forest.\n\r", ch);
	return;
    }

    send_to_char("You become a pile of leaves and fall to the floor.\n\r", ch);
    act("$n transforms into a pile of leaves and falls to the forest floor.", ch, NULL, NULL, TO_ROOM);
    WAIT_STATE(ch, 12);
    SET_BIT(ch->flag4, AFF_SENESCHIA);
}


void do_sharpen (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj, *wield = NULL;

    if (argument[0] == '\0')
    {
	send_to_char("What do you wish to sharpen?\n\r",ch);
	return;
    }

    for ( obj = ch->carrying ; obj != NULL ; obj = obj->next_content)
    {
	if ( can_see_obj (ch, obj) && is_name(argument, obj->name))
	wield = obj;
    }

    if (wield == NULL)
    {
	send_to_char("You have not got that item in your inventory.\n\r", ch);
	return;
    }

    if (str_cmp(wield->short_descr, "a wooden stick")
	&& str_cmp(wield->short_descr, "an oaken branch") )
    {
	send_to_char("That item cannot be sharpened into a stake.\n\r", ch);
	return;
    }
    act("You sharpen $p into a wooden stake.", ch, wield, NULL, TO_CHAR);
    act("$n sharpens $p into a wooden stake.", ch, wield, NULL, TO_ROOM);
    WAIT_STATE(ch, 12);
    extract_obj(wield);
    obj = create_object( get_obj_index(92), 0);
    obj_to_char(obj, ch);
    SET_BIT(obj->extra_flags2, ITEM_STAKE);
}

void do_tree_walk (CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *room = NULL;

    if (!IS_FLAG4(ch, AFF_TREE_MELD) )
    {
	send_to_char("You may only treewalk while melded with a tree.\n\r",ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_NO_ARBOREA) )
    {
	send_to_char("The forest will not let you do that any more.\n\r",ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_TREE_WALK) )
    {
	send_to_char("You can only do this once every 6 hours.\n\r", ch);
	return;
    }

    room = get_treemeld_room(ch);
    while (room->sector_type != SECT_FOREST)
    {
        room = get_treemeld_room(ch);
    }
    
    SET_TIMER(ch, TIMER_TREE_WALK, 6);

    ch->value[1] = room->vnum;
    send_to_char("You slide down the roots and emerge in another tree.\n\r", ch);

}


void do_smother (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (ch->in_room->sector_type != SECT_FOREST)
    {
	send_to_char("You may only use this ability in the forest.\n\r",ch);
	return;
    }

    if (argument == '\0')
    {
	send_to_char("Who do you wish the plants to smother?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	send_to_char("The forest tells you 'Who do you wish me to smother?'\n\r", ch);
	return;
    }

    if (IS_SET(victim->flag4, AFF_SMOTHERED)
	|| IS_SET(victim->flag4, AFF_ENTWINED) )
    {
	send_to_char("The forest tells you 'I can't do any more than that!'\n\r", ch);
	return;
    }

    if (victim->fighting != NULL)
    {
	send_to_char("The forest tells you 'Wait till they stop fighting.'\n\r", ch);
	return;
    }

    if (victim->level > ch->level)
    {
	send_to_char("The forest tells you 'They are too powerful to affect.'\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);

    if (victim->power[DISC_VAMP_CELE] > 3)
    {
	send_to_char("The forest tells you 'They are moving too quickly!'\n\r", ch);
	send_to_char("Vines and creepers move towards you, but you dodge out of their way.\n\r",victim);
 	return;
    }

    SET_BIT(victim->flag4, AFF_SMOTHERED);
    send_to_char("The plants and vegetation start to smother you.\n\r", victim);
    act("$n whispers something, and the plants start to smother and choke $N.", ch, NULL, victim,TO_NOTVICT);
    act("Plants and vines start to smother and choke $N.", ch, NULL, victim, TO_CHAR);
}

void do_entwine (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (ch->in_room->sector_type != SECT_FOREST)
    {
	send_to_char("You may only use this ability in the forest.\n\r",ch);
	return;
    }

    if (argument == '\0')
    {
	send_to_char("Who do you wish the plants to entwine?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	send_to_char("The forest tells you 'Who do you wish me to entwine?'\n\r", ch);
	return;
    }

    if (IS_SET(victim->flag4, AFF_SMOTHERED)
	|| IS_SET(victim->flag4, AFF_ENTWINED) )
    {
	send_to_char("The forest tells you 'I can't do any more than that!'\n\r", ch);
	return;
    }

    if (victim->fighting != NULL)
    {
	send_to_char("The forest tells you 'Wait till they stop fighting.'\n\r", ch);
	return;
    }

    if (victim->level > ch->level)
    {
	send_to_char("The forest tells you 'They are too powerful to affect.'\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);

    if (victim->power[DISC_VAMP_CELE] > 3)
    {
	send_to_char("The forest tells you 'They are moving too quickly!'\n\r", ch);
	send_to_char("Vines and creepers move towards you, but you dodge out of their way.\n\r",victim);
 	return;
    }

    SET_BIT(victim->flag4, AFF_ENTWINED);
    send_to_char("The plants and vegetation start to entwine you.  You cannot move.\n\r", victim);
    act("$n whispers something, and the plants start to entwine $N.", ch, NULL, victim,TO_NOTVICT);
    act("Plants and vines start to hold and entwine $N.", ch, NULL, victim, TO_CHAR);
}

void do_wrench (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA * victim;

    if (!TIME_UP(ch, TIMER_WRENCH))
    {
	send_to_char("You may only use this power once per day.\n\r", ch);
    	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Who do you wish to wrench out of the negative plane?\n\r", ch);
	return;
    }

    if (ch->alignment < -300)
    {
	send_to_char("You have displeased the gods and may not use this power.\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, argument) )== NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    SET_TIMER(ch, TIMER_WRENCH, 24);

    send_to_char("You start to perform the ritual.\n\r", ch);
    WAIT_STATE(ch, 12);

    act("$n starts to chant in strange ancient languages.", ch, NULL, NULL, TO_ROOM);

    if (victim->secondaryrace != RACE_VAMPIRE)
	send_to_char("Nothing seems to have happened.\n\r", ch);
    else
    {
        if (victim->level > ch->level + ch->generation)
	{
	    send_to_char("They are too powerful for you to affect currently.\n\r", ch);
	    return;
	}
	SET_TIMER(victim, TIMER_WRENCHED, ch->water+ch->generation);
	act("$N screams with agony as $E is wrenched out of the negative plane.", ch, NULL, victim, TO_CHAR);
	act("$N screams with agony as $E is wrenched out of the negative plane.", ch, NULL, victim, TO_NOTVICT);
	act("You scream with agony as you are wrenched out of the negative plane.", ch, NULL, victim, TO_VICT);
	return;
	if (victim->cur_form == FRM_MIST)
	{
	    act("$N's incorporeal form solidifies once more.", ch, NULL, victim, TO_CHAR);    
	    act("$N's incorporeal form solidifies once more.", ch, NULL, victim, TO_NOTVICT); 
	    act("You return to humanoid form.", ch, NULL, victim, TO_VICT);
 	    set_form(victim, 0);
	}
 	if (IS_SET(victim->flag2, AFF_SHADOW_PLANE) )
        {
	    act("$N's body reforms from the shadows.", ch, NULL, victim, TO_CHAR);
	    act("$N's body reforms from the shadows.", ch, NULL, victim, TO_NOTVICT);
	    act("You leave the shadowplane.", ch, NULL, victim, TO_VICT);
	    if (victim->secondaryrace == RACE_VAMPIRE) change_upkeep(victim, -5);
	    REMOVE_BIT(victim->flag2, AFF_SHADOW_PLANE);
	}
    }
}

void do_layhands (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int heal;
    char buf[MAX_STRING_LENGTH];

    if (!TIME_UP(ch, TIMER_LAYONHANDS))
    {
	sprintf(buf, "You must wait %d hours to use this power again.\n\r", TIMER(ch, TIMER_LAYONHANDS) );
	send_to_char(buf, ch);
    	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Who do you wish to lay on hands?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, argument) )== NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim==ch)
    {
	send_to_char("You cannot use this power on yourself.\n\r", ch);
	return;
    }

    heal = ch->power[DISC_HERO_VIVI] + 3;
    heal *= ch->generation * 10;
    heal = UMAX(10, heal);

    if (ch->power[DISC_HERO_VIVI] > 5) heal *= 2;

    if (victim->secondaryrace == RACE_VAMPIRE)
    {
	send_to_char("You cannot use this power to heal a vampire.\n\r", ch);
	return;
    }

    if (victim->hit > victim->max_hit - 10)
    {
	send_to_char("They are not in desperate need of healing.\n\r", ch);
	return;
    }

    act("$n lays $s hands on you, healing your wounds.", ch, NULL, victim, TO_VICT);
    act("You lay your hands on $N and heal $S wounds.", ch, NULL, victim, TO_CHAR);
    act("$n lays $s hands on $N, healing their wounds.", ch, NULL, victim, TO_NOTVICT);

    victim->hit = UMIN(victim->max_hit, victim->hit + heal);

    if (is_affected(victim, gsn_plague) && ch->power[DISC_HERO_VIVI] >= 3)
    {
	affect_strip(victim, gsn_plague);
	stc("You feel the plague leave your system.\n\r", victim);
    }

    if (is_affected(victim, gsn_curse) && ch->power[DISC_HERO_VIVI] >= 3)
    {
	affect_strip(victim, gsn_curse);
	stc("You feel the curse upon you has been lifted.\n\r", victim);
    }

    if (is_affected(victim, gsn_blindness) && ch->power[DISC_HERO_VIVI] >= 2)
    {
	affect_strip(victim, gsn_blindness);
	stc("You can see again.\n\r", victim);
    }

    if (is_affected(victim, gsn_poison) && ch->power[DISC_HERO_VIVI] >= 3)
    {
	affect_strip(victim, gsn_poison);
	stc("The poison leaves your system.\n\r", victim);
    }

    WAIT_STATE(ch,10);

    if (dice(1,10) == 1 && victim->alignment > 300)
    {
	send_to_char("That was a good and charitable act.\n\r", ch);
	ch->alignment = UMIN(1000, ch->alignment + 1);
    }

    if (victim->alignment < -500)
    {
	send_to_char("You feel wary of helping the unpure at heart.\n\r", ch);
	ch->alignment = UMAX(300, ch->alignment -1);
    }

    if (ch->power[DISC_HERO_VIVI] > 1)
        SET_TIMER(ch, TIMER_LAYONHANDS, 5 - (ch->generation)/4);

    else SET_TIMER(ch, TIMER_LAYONHANDS, 7 - (ch->generation)/4);

}

void do_materials (CHAR_DATA *ch, char *argument)
{
    int i, rel=0;
    char buf[MAX_STRING_LENGTH];

    if (!str_prefix(argument, "flame")) rel = 1;
    else if (!str_prefix(argument, "air")) rel = 3;
    else if (!str_prefix(argument, "water")) rel = 2;
    else if (!str_prefix(argument, "earth")) rel = 4;
    else if (!str_prefix(argument, "entropy")) rel = 5;
    if (argument[0]=='\0' || rel==0)
    {
	send_to_char("Syntax : materials <flame water air earth entropy>\n\r", ch);
	return;
    }

    send_to_char("The current material components are :\n\r", ch);
    for (i=1 ; i<MAX_SKILL ; i++)
    {
	if (skill_table[i].realm == rel
	    && skill_table[i].material_comp[0] != '\0')
	{
	    sprintf(buf, "The spell '%s' requires %s, at %d%% instability.\n\r", 
		    skill_table[i].name,
		    skill_table[i].material_comp,
		    skill_table[i].per_used);
	    send_to_char(buf, ch);
	}
    }
}

void do_glyph (CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (ch->in_room->sector_type == SECT_AIR)
    {
	send_to_char("You cannot use this ability in the air.\n\r", ch);
	return;
    }

    if (arg1[0] == '\0')
    {
	send_to_char("Syntax : glyph [protection", ch);
	if (ch->power[DISC_HERO_SIGI] > 4)
	    send_to_char(" warding", ch);
	if (ch->power[DISC_HERO_SIGI] > 6)
	    send_to_char(" swords", ch);
	if (ch->power[DISC_HERO_SIGI] > 8)
	    send_to_char(" prisms", ch);
	if (ch->power[DISC_HERO_SIGI] > 9)
	    send_to_char(" invulnerablility", ch);
	send_to_char("] [exit]\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "protection")
	&& ch->power[DISC_HERO_SIGI] > 1)
    {
	if (ch->faith < 2)
	{
	    send_to_char("It costs two faith points to draw a protection glyph.\n\r", ch);    
	    return;
	}
	ch->faith -= 2;
	ADD_RTIMER(ch->in_room, RTIMER_GLYPH_PROTECTION, dice(6, 12) );
	send_to_char("You draw a protection glyph on the floor.\n\r", ch);
	act("$n draws a protection glyph on the floor.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (!str_prefix(arg1, "invulnerable")
	&& ch->power[DISC_HERO_SIGI] > 9)
    {
	if (ch->faith < 10)
	{
	    send_to_char("It costs ten faith points to draw a invulnerability glyph.\n\r", ch);    
	    return;
	}
	ch->faith -= 10;
	SET_BIT(ch->flag4, AFF_INVULNERABLE);
	send_to_char("You draw a protection glyph on the floor beneath your feet.\n\r", ch);
	act("$n draws a protection glyph on the floor beneath $s feet.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (!str_prefix(arg1, "warding")
	&& ch->power[DISC_HERO_SIGI] > 4)
    {
	int dir;

	if (arg2[0] == '\0')
	{
	    send_to_char("Syntax : glyph warding <direction>\n\r", ch);
	    return;
	}

	if ( ( dir = get_direction(arg2) ) < 0)
	{
	    send_to_char("Syntax : glyph warding <direction>\n\r", ch);
	    return;
	}

	if (dir > 3)
	{
	    send_to_char("You may not place a glyph up or down.\n\r", ch);
	    return;
	}
	
	if (ch->in_room->exit[dir] == NULL)
	{
	    send_to_char("There is no exit in that direction.\n\r", ch);
	    return;
	}

	if (ch->faith < 6)
	{
	    send_to_char("It costs six faith points to draw a warding glyph.\n\r", ch);    
	    return;
	}
	ch->faith -= 6;

	send_to_char("You draw a protection glyph on the floor.\n\r", ch);
	act("$n draws a protection glyph on the floor.", ch, NULL, NULL, TO_ROOM);
	SET_BIT(ch->in_room->exit[dir]->exit_info, EX_WARDING);
	WAIT_STATE(ch, 8);
	return;
    }


    if (!str_prefix(arg1, "swords")
	&& ch->power[DISC_HERO_SIGI] > 6)
    {
	int dir;

	if (arg2[0] == '\0')
	{
	    send_to_char("Syntax : glyph swords <direction>\n\r", ch);
	    return;
	}

	if ( ( dir = get_direction(arg2) ) < 0)
	{
	    send_to_char("Syntax : glyph swords <direction>\n\r", ch);
	    return;
	}

	if (dir > 3)
	{
	    send_to_char("You may not place a glyph up or down.\n\r", ch);
	    return;
	}
	
	if (ch->in_room->exit[dir] == NULL)
	{
	    send_to_char("There is no exit in that direction.\n\r", ch);
	    return;
	}

	if (ch->faith < 10)
	{
	    send_to_char("It costs ten faith points to make a wall of swords.\n\r", ch);    
	    return;
	}
	ch->faith -= 10;

	send_to_char("You draw a glyph on the floor, it grows into a wall of swords.\n\r", ch);
	act("$n draws a glyph on the floor.", ch, NULL, NULL, TO_ROOM);
	act("The glyph grows into a wall of swords.", ch, NULL, NULL, TO_ROOM);
	make_wall(ch->in_room, dir, EX_SWORD_WALL);
	WAIT_STATE(ch, 8);
	return;
    }

    if (!str_prefix(arg1, "prisms")
	&& ch->power[DISC_HERO_SIGI] > 8)
    {
	int dir;

	if (arg2[0] == '\0')
	{
	    send_to_char("Syntax : glyph prisms <direction>\n\r", ch);
	    return;
	}

	if ( ( dir = get_direction(arg2) ) < 0)
	{
	    send_to_char("Syntax : glyph prisms <direction>\n\r", ch);
	    return;
	}

	if (dir > 3)
	{
	    send_to_char("You may not place a glyph up or down.\n\r", ch);
	    return;
	}
	
	if (ch->in_room->exit[dir] == NULL)
	{
	    send_to_char("There is no exit in that direction.\n\r", ch);
	    return;
	}

	if (ch->faith < 12)
	{	    
	    send_to_char("It costs twelve faith points to make a wall of prisms.\n\r", ch);    
	    return;
	}
	ch->faith -= 12;

	send_to_char("You draw a glyph on the floor, it grows into a wall of prisms.\n\r", ch);
	act("$n draws a glyph on the floor.", ch, NULL, NULL, TO_ROOM);
	act("The glyph grows into a shimmering wall of prisms.", ch, NULL, NULL, TO_ROOM);
	make_wall(ch->in_room, dir, EX_PRISMATIC_WALL);
	WAIT_STATE(ch, 10);
	return;
    }

    do_glyph(ch, "");
}


void do_sigil (CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *wield = NULL;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    
    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("Syntax : sigil <weapon> <mah ayn flam ort jig>\n\r", ch);
	return;
    }
    
    for ( obj = ch->carrying ; obj != NULL ; obj = obj->next_content)
    {
	if ( can_see_obj (ch, obj) && is_name (arg1, obj->name))
	wield = obj;
    }

    if (wield == NULL)
    {
	send_to_char("You have not got that item in your inventory.\n\r", ch);
	return;
    }

    if (wield->item_type != ITEM_WEAPON)
    {
	send_to_char("You may only draw a sigil on a weapon.\n\r", ch);
	return;
    }

    if (!str_cmp(arg2, "mah") ) 
    {
	if (IS_SET(wield->value[4], WEAPON_RUNE_FORCE_BOLT) )
	{
	     send_to_char("The Mah rune is already drawn on this weapon.\n\r", ch);
	     return;
	}
	if (ch->faith < 1)
	{
	     send_to_char("It costs one faith point to draw the Mah rune correctly.\n\r", ch);
	     return;
	}
	ch->faith -= 1;
	SET_BIT(wield->value[4], WEAPON_RUNE_FORCE_BOLT);
	act("You draw the Mah rune upon $p.", ch, wield, NULL, TO_CHAR);
	act("$n draws a Mah rune upon $s weapon.",ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (!str_cmp(arg2, "ayn") ) 
    {
	if (ch->power[DISC_HERO_SIGI] < 3)
	{
	    send_to_char("You must be level three in Sigil to draw the Ayn rune.\n\r", ch);
	    return;
	}
	if (IS_SET(wield->value[4], WEAPON_RUNE_SMITE_EVIL) )
	{
	     send_to_char("The Ayn rune is already drawn on this weapon.\n\r", ch);
	     return;
	}
	if (ch->faith < 2)
	{
	     send_to_char("It costs two faith points to draw the Ayn rune correctly.\n\r", ch);
	     return;
	}
	ch->faith -= 2;
	SET_BIT(wield->value[4], WEAPON_RUNE_SMITE_EVIL);
	act("You draw the Ayn rune upon $p.", ch, wield, NULL, TO_CHAR);
	act("$n draws the Ayn rune upon $s weapon.",ch, NULL, NULL, TO_ROOM);
	return;
    }
    if (!str_cmp(arg2, "flam") ) 
    {
	if (ch->power[DISC_HERO_SIGI] < 4)
	{
	    send_to_char("You must be level four in Sigil to draw the Flam rune.\n\r", ch);
	    return;
	}
	if (IS_SET(wield->value[4], WEAPON_RUNE_BLAZE) )
	{
	     send_to_char("The Flam rune is already drawn on this weapon.\n\r", ch);
	     return;
	}
	if (ch->faith < 4)
	{
	     send_to_char("It costs four faith points to draw the Flam rune correctly.\n\r", ch);
	     return;
	}
	ch->faith -= 4;
	SET_BIT(wield->value[4], WEAPON_RUNE_BLAZE);
	act("You draw the Flam rune upon $p.", ch, wield, NULL, TO_CHAR);
	act("$n draws the Flam rune upon $s weapon.",ch, NULL, NULL, TO_ROOM);
	return;
    }
    if (!str_cmp(arg2, "ort") ) 
    {
	if (ch->power[DISC_HERO_SIGI] < 6)
	{
	    send_to_char("You must be level six in Sigil to draw the Ort rune.\n\r", ch);
	    return;
	}
	if (IS_SET(wield->value[4], WEAPON_RUNE_LIGHTNING) )
	{
	     send_to_char("The Ort rune is already drawn on this weapon.\n\r", ch);
	     return;
	}
	if (ch->faith < 8)
	{
	     send_to_char("It costs eight faith points to draw the Ort rune correctly.\n\r", ch);
	     return;
	}
	ch->faith -= 8;
	SET_BIT(wield->value[4], WEAPON_RUNE_LIGHTNING);
	act("You draw the Ort rune upon $p.", ch, wield, NULL, TO_CHAR);
	act("$n draws the Ort rune upon $s weapon.",ch, NULL, NULL, TO_ROOM);
	return;
    }
    if (!str_cmp(arg2, "jig") ) 
    {
	if (ch->power[DISC_HERO_SIGI] < 8)
	{
	    send_to_char("You must be level eight in Sigil to draw the Jig rune.\n\r", ch);
	    return;
	}
	if (IS_SET(wield->value[4], WEAPON_RUNE_DANCING) )
	{
	     send_to_char("The Jig rune is already drawn on this weapon.\n\r", ch);
	     return;
	}
	if (ch->faith < 12)
	{
	     send_to_char("It costs twelve faith points to draw the Jig rune correctly.\n\r", ch);
	     return;
	}
	ch->faith -= 12;
	SET_BIT(wield->value[4], WEAPON_RUNE_DANCING);
	act("You draw the Jig rune upon $p.", ch, wield, NULL, TO_CHAR);
	act("$n draws the Jig rune upon $s weapon.",ch, NULL, NULL, TO_ROOM);
	return;
    }
}

int count_heroes ( ROOM_INDEX_DATA *room )
{
    CHAR_DATA *fch;
    int count = 0;

    for (fch = room->people; fch != NULL; fch = fch->next_in_room)
        if (fch->secondaryrace == RACE_HERO) count++;

    return count++;
}

void do_bless (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim = NULL;
    OBJ_DATA *obj = NULL;

    if (argument[0] == '\0')
    {
	send_to_char("Syntax : bless {character item}\n\r", ch);
	return;
    }

    if (ch->move < 30)
    {
	send_to_char("You are too exhausted to bless things.\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument)) == NULL)
    {
	if ( (obj = get_obj_carry(ch, argument, ch)) == NULL)
	{
	    do_bless(ch, "");
	    return;
	}
    }	        

    ch->move -= 30;

    WAIT_STATE(ch, 8);

    if (victim != NULL)
	spell_bless(skill_lookup("bless"), ch->power[DISC_HERO_CRUS] * 10,
		    ch, victim, TARGET_CHAR);
    else
	spell_bless(skill_lookup("bless"), ch->power[DISC_HERO_CRUS] * 10,
		    ch, obj, TARGET_OBJ);

}



void do_wall (CHAR_DATA *ch, char *argument)
{
    int dir;

    if (count_heroes(ch->in_room) < 2)
    {
	send_to_char("There is insufficient power within this room.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Syntax : wall <direction>\n\r", ch);
	return;
    }

    if ( (dir = get_direction(argument)) < 0)
    {
	do_wall(ch, "");
	return;
    }

    if (ch->in_room->exit[dir] == NULL)
    {
	send_to_char("There isn't an exit in that direction.\n\r", ch);
	return;
    }

    SET_BIT(ch->in_room->exit[dir]->exit_info, EX_CALTROP_WALL);
    WAIT_STATE(ch, 8);
    send_to_char("You set caltrops into the ground.\n\r", ch);
    act("$n sets caltrops into the ground.", ch, NULL, NULL, TO_ROOM);
}
