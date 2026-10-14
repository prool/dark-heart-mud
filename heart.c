/*****************************************************************
 * Heart.C -- (C) 1996, by Adam Powell                            *
 * Misc. functions coded by AdAm PoWeLl and Chris Mellish         *
 * 'Get a beer down you, you miserable bastard'			  *
 * 	- Percy, Hu Stu						  *
 ******************************************************************/

/* Lots of gambling and tavern shite.... heh, just wait */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hell.h"
#include "magic.h"

int	rolldice	args( (CHAR_DATA *ch, int die) );
int	spin_roulette	args( (CHAR_DATA *ch) );
int 	skull_dice	args( (CHAR_DATA *ch, int die) );
void	plummet		args( (CHAR_DATA *ch, CHAR_DATA *victim) );
int     get_polymorph_power args( (CHAR_DATA *ch) );
void    promote_elad_gen    args( (CHAR_DATA *ch) );

DECLARE_DO_FUN( do_rage		);
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_yell		);
DECLARE_DO_FUN( do_visible	);

bool can_speak(CHAR_DATA *ch, int lang)
{  
    if (lang == LANG_COMMON) return TRUE;

    if (ch->level == 120) return TRUE;

    if (lang == LANG_ELVISH)
    {
	if (ch->secondaryrace == RACE_ELADRIN
	    || ch->race == RACE_ELF
	    || IS_FLAG6(ch, AFF6_CAN_SPEAK_ELVISH) )
	    return TRUE;
	else return FALSE;
    }

    if (lang == LANG_GNOMISH
	&& (ch->race == RACE_GNOME || IS_FLAG6(ch, AFF6_CAN_SPEAK_GNOMISH) ) )
	return TRUE;
   
    if (lang == LANG_DEMONIC
	&& (ch->secondaryrace == RACE_DAEMON || ch->tertiaryrace == TERTIARY_AASIMON
	    || IS_FLAG6(ch, AFF6_CAN_SPEAK_DEMONIC) ) )
	return TRUE;

    return FALSE;
}


int hand_slots(CHAR_DATA *ch)
{
    int hands = 2;  /* Quite normal for a normal person */

    if (BODY_PART(ch, BODY_LEFT_HAND)
	|| BODY_PART(ch, BODY_LEFT_HAND)) hands--;

    if (BODY_PART(ch, BODY_RIGHT_HAND)
	|| BODY_PART(ch, BODY_RIGHT_HAND)) hands--;
 
    if (BODY_PART(ch, BODY_THIRD_ARM)
	&& BODY_PART(ch, BODY_THIRD_HAND)) hands++;

    if (BODY_PART(ch, BODY_FOURTH_ARM)
	&& BODY_PART(ch, BODY_FOURTH_HAND)) hands++;

    return hands;
}


int hands_taken_up(CHAR_DATA *ch)
{
    OBJ_DATA *obj;
    int hands = 0;
    int temp;

    if ( ( obj = get_eq_char(ch, WEAR_RIGHT_HAND) ) != NULL)
    {
	if ( (temp = calc_hands_needed(ch, obj)) != 100) hands += temp;
	else
	{
	    obj_from_char(obj);
	    obj_to_room(obj, ch->in_room, ch->altitude);
	    act("$p is too heavy for you and you drop it.", ch, obj, NULL, TO_CHAR);
	    act("$p is too heavy for $n and $e drops it.", ch, obj, NULL, TO_ROOM);
	}
    }

    if ( ( obj = get_eq_char(ch, WEAR_LEFT_HAND) ) != NULL)
    {
	if ( (temp = calc_hands_needed(ch, obj)) != 100) hands += temp;
	else
	{
	    obj_from_char(obj);
	    obj_to_room(obj, ch->in_room, ch->altitude);
	    act("$p is too heavy for you and you drop it.", ch, obj, NULL, TO_CHAR);
	    act("$p is too heavy for $n and $e drops it.", ch, obj, NULL, TO_ROOM);
	}
    }

    if ( ( obj = get_eq_char(ch, WEAR_FOURTH_HAND) ) != NULL)
    {
	if ( (temp = calc_hands_needed(ch, obj)) != 100) hands += temp;
	else
	{
	    obj_from_char(obj);
	    obj_to_room(obj, ch->in_room, ch->altitude);
	    act("$p is too heavy for you and you drop it.", ch, obj, NULL, TO_CHAR);
	    act("$p is too heavy for $n and $e drops it.", ch, obj, NULL, TO_ROOM);
	}
    }

    if ( ( obj = get_eq_char(ch, WEAR_THIRD_HAND) ) != NULL)
    {
	if ( (temp = calc_hands_needed(ch, obj)) != 100) hands += temp;
	else
	{
	    obj_from_char(obj);
	    obj_to_room(obj, ch->in_room, ch->altitude);
	    act("$p is too heavy for you and you drop it.", ch, obj, NULL, TO_CHAR);
	    act("$p is too heavy for $n and $e drops it.", ch, obj, NULL, TO_ROOM);
	}
    }

    return hands;

}

void do_rub(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    CHAR_DATA *vch, *vch_next;

    if (argument[0] == '\0')
    {
	stc("What item do you wish to rub?\n\r", ch);
	return;
    }

    if ( (obj = get_obj_eq(ch, argument, ch) ) == NULL)
    {
	stc("You haven't got that item.\n\r", ch);	
	return;
    }

    if (!IS_SET(obj->wear_flags, ITEM_WEAR_FINGER))
    {
	act("$p is not a ring.\n\r", ch, obj, NULL, TO_CHAR);
	return;
    }

    if (obj->condition < 30)
    {
	act("$p is in too poor state to activate.", ch, obj, NULL, TO_CHAR);
	return;
    }

    if (obj->value[5] <= 0)
    {
	act("There is no magic in $p.", ch, obj, NULL, TO_CHAR);
	return;
    }

    WAIT_STATE(ch, 12);

    if (obj->value[6] == 0 || obj->value[5] < 0 || obj->value[5] > MAX_SKILL
	|| skill_table[obj->value[5]].spell_fun == spell_null)
    {	
	act("$p vanishes in a puff of smoke, its charges used.", ch, obj, NULL, TO_ALL);
	extract_obj(obj);
	return;
    }

    obj->value[6]--; /* Decrement the charges */

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) )
    {
        stc("The ring fizzles and sparks, but nothing happens.\n\r", ch);
        return;
    }

    act("You rub $p.", ch, obj, NULL, TO_CHAR);
    act("$n rubs $p.", ch, obj, NULL, TO_ROOM);

    for (vch = ch->in_room->people ; vch != NULL ; vch = vch_next )
    {
	vch_next = vch->next_in_room;
	if (vch->altitude != ch->altitude) continue;

	switch ( skill_table[obj->value[5]].target )
        {
            default:
                bug( "Do_rub: bad target for sn %d.", obj->value[5] );
                return;
        
            case TAR_IGNORE:
                if ( vch != ch )
                    continue;
                break;
            
            case TAR_CHAR_OFFENSIVE:
                if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
                    continue;
                break;
         
            case TAR_CHAR_DEFENSIVE:
                if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
                    continue;
		break;

	    case TAR_CHAR_SELF:
		if (vch != ch) continue;
		break;
	}
        obj_cast_spell(obj->value[5], obj->value[0], ch, vch, NULL);
    }


    if (obj->value[6] == 0)
    {	
	act("$p vanishes in a puff of smoke, its charges used.", ch, obj, NULL, TO_ALL);
	extract_obj(obj);
    }

    return;
}

void do_appraise(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    int cost;

    if (argument[0] == '\0')
    {
	stc("What object do you wish to attempt to appraise?\n\r", ch);
	return;
    }

    if ( (obj = get_obj_carry(ch, argument, ch) ) == NULL)
    {	
	stc("You aren't carrying that item.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 14);

    /* success */
    if ( dice(1, 100) < get_skill(ch, gsn_appraisal) )
    {
	cost = number_range((obj->cost * 5) / 7, (obj->cost * 9) / 7);
    }
    else
    {
	check_improve(ch, gsn_appraisal, FALSE, 3);
	cost = dice(1, obj->cost * 2);
    }

    act("You hold $p up to appraise it.", ch, obj, NULL, TO_CHAR);
    act("$n holds up $p to appraise it.", ch, obj, NULL, TO_ROOM);

    if (cost < 100)
 	stcf(ch, "You think it is worth about %d copper pieces.\n\r",
	    cost);
    else if (cost < 1000)
 	stcf(ch, "You think it is worth about %d silver pieces.\n\r",
	    cost/10);
    else if (cost < 10000)
 	stcf(ch, "You think it is worth about %d gold pieces.\n\r",
	    cost/100);
    else if (cost < 100000)
 	stcf(ch, "You think it is worth about %d emeralds.\n\r",
	    cost/1000);
    else if (cost < 1000000)
 	stcf(ch, "You think it is worth about %d sapphires.\n\r",
	    cost/10000);
    else stcf(ch, "You think it is worth about %d diamonds.\n\r",
	    cost/100000);

    return;
}

void do_repair(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if (argument[0] == '\0')
    {
	stc("What do you wish to attempt to repair?\n\r", ch);
	return;
    }

    if ( (obj = get_obj_carry(ch, argument, ch) ) == NULL)
    {	
	stc("You aren't carrying that item.\n\r", ch);
	return;
    }

    if (obj->condition >= 100)
    {
	act("$p does not need repairing.", ch, obj, NULL, TO_CHAR);
	return;
    }

    WAIT_STATE(ch, 12);

    /* success */
    if ( dice(1, 100) < get_skill(ch, gsn_repair) )
    {
	act("You start to repair $p.", ch, obj, NULL, TO_CHAR);
	act("$n starts to repair $p.", ch, obj, NULL, TO_ROOM);
	obj->condition += UMAX( 20, get_skill(ch, gsn_repair) / 2 );
	if (obj->condition >= 100)
	{
	    act("$p is now fully repaired.", ch, obj, NULL, TO_CHAR);
	    obj->condition = 100;
	}
	return;
    }

    act("You failed to repair $p.", ch, obj, NULL, TO_CHAR);
    check_improve(ch, gsn_repair, FALSE, 3);
    return;
}

bool test(CHAR_DATA *ch, int test)
{
    switch (test)
    {
	case IS_ELADRIN :
	    if (ch->secondaryrace == RACE_ELADRIN) return TRUE;
	    else return FALSE;
	    break;

	default :
	    bug("Default found in test procedure", 0);
	    return FALSE;
	    break;

    }

    bug("Default found in test procedure", 0);
    return FALSE;
}

/* returns TRUE if the obj is destroyed */
bool damage_obj(OBJ_DATA *obj, int damage)
{
    char buf[MAX_STRING_LENGTH];

    crash("damage obj! ack!");

    if (obj == NULL)
    {
	bug("Damaging a null object", 0);
	return TRUE;
    }

    if (IS_OBJ_STAT(obj, ITEM_ARTIFACT)) damage = -1;
    if (IS_OBJ_STAT(obj, ITEM_RELIC)) damage = 1;
    if (IS_OBJ_STAT(obj, ITEM_MAGIC)) damage /= 3;
    if (IS_OBJ_STAT(obj, ITEM_BLESS)) damage /= 2;
    if (IS_OBJ_STAT3(obj, ITEM_STRUCTURE)) damage /= 2;

    if (damage == 0) damage = 1;

    if (obj->condition < damage)
    {
	sprintf(buf, "%s is destroyed.", obj->short_descr);
	room_message(locate_obj(obj), buf);
	SET_BIT(obj->extra_flags3, ITEM_DESTROYED);
	return TRUE;
    }
    else obj->condition -= damage;
    return FALSE;
}

void gain_elad_path(CHAR_DATA *ch, int path)
{
    if (IS_NPC(ch)) return;
    if (ch->secondaryrace != RACE_ELADRIN) return;
    if (ch->tertiaryrace != 0) return;
    if (!TIME_UP(ch, TIMER_ELADRIN_PATH)) return;
 
    crash("gain_elad_path! ack!");

    if (ch->generation < 1) ch->generation = 1;

    if (ch->pcdata->faeriepath[path] >= 100)
    {
	ch->pcdata->faeriepath[path] = 100;
	return;
    }

    ADD_TIMER(ch, TIMER_ELADRIN_PATH, dice(2, 3) );

    if (path == 0)
	stc("You have progressed further along the path of Valour.\n\r", ch);
    else if (path == 1)
	stc("You have progressed further along the path of Protection.\n\r", ch);
    else if (path == 2)
	stc("You have progressed further along the path of Singing.\n\r", ch);
    else if (path == 3)
	stc("You have progressed further along the path of the Elders.\n\r", ch);

    ch->pcdata->faeriepath[path]++;

    if (ch->pcdata->faeriepath[path] == 33)
	promote_elad_gen(ch);
    if (ch->pcdata->faeriepath[path] == 66)
	promote_elad_gen(ch);
    if (ch->pcdata->faeriepath[path] == 100)
	promote_elad_gen(ch);
}

void promote_elad_gen(CHAR_DATA *ch)
{
    /* become nov at 3, bral at 5, shiere at 7,
	firre at 8, ghaele at 10, tulani at 12 */

    if (IS_NPC(ch)) return;
    if (ch->secondaryrace != RACE_ELADRIN) return;
    if (ch->tertiaryrace != 0) return;
    if (ch->generation >= 12) return;
    if (ch->generation < 1) ch->generation = 1;

    stc("You have advanced among the Eladrin.\n\r", ch);
    ch->generation++;

    if (ch->generation == 3)
    {
	stc("You transform into a small aquatic water sprite.\n\r", ch);
	stc("You are now a Noviere.\n\r", ch);
    }
    else if (ch->generation == 5)
    {
	stc("You are now a Bralani.\n\r", ch);
    }
    else if (ch->generation == 7)
    {
	stc("You are now a Shiere.\n\r", ch);
    }
    else if (ch->generation == 8)
    {
	stc("You are now a Firre.\n\r", ch);
    }
    else if (ch->generation == 10)
    {
	stc("You are now a Ghaele.\n\r", ch);
    }
    else if (ch->generation == 12)
    {
	stc("You transform into a tall regal elven lord.\n\r", ch);
	stc("You are now a Tulani.\n\r", ch);
    }

    return;
}


void do_paths(CHAR_DATA *ch, char *argument)
{
    int i;
 
    if (!test(ch, IS_ELADRIN))
    {
	stc("Only eladrin may promote in this fashion.\n\r", ch);
	return;
    }

    ctc("Faerie Paths", ch);
    ctc("-*-*-**-*-*-", ch);
    stc("\n\r", ch);

    stc("Path of Valour     : [", ch);
    for (i = 0 ; i < 40 ; i++)
    {
	if (i > 19 && ch->pcdata->faeriepath[0] * 4 / 10 > i)
	    stc("#2*", ch);
	else if (ch->pcdata->faeriepath[0] * 4 / 10 > i)
	    stc("#3*", ch);
	else stc(" ", ch);
    }
    stc("#n]", ch);
    if (ch->pcdata->faeriepath[0] == 100)
	stc("     (Completed)\n\r", ch);
    else stc("\n\r", ch);

    stc("Path of Protection : [", ch);
    for (i = 0 ; i < 40 ; i++)
    {
	if (i > 19 && ch->pcdata->faeriepath[1] * 4 / 10 > i)
	    stc("#2*", ch);
	else if (ch->pcdata->faeriepath[1] * 4 / 10 > i)
	    stc("#3*", ch);
	else stc(" ", ch);
    }
    stc("#n]", ch);
    if (ch->pcdata->faeriepath[1] == 100)
	stc("     (Completed)\n\r", ch);
    else stc("\n\r", ch);

    stc("Path of Singing    : [", ch);
    for (i = 0 ; i < 40 ; i++)
    {
	if (i > 19 && ch->pcdata->faeriepath[2] * 4 / 10 > i)
	    stc("#2*", ch);
	else if (ch->pcdata->faeriepath[2] * 4 / 10 > i)
	    stc("#3*", ch);
	else stc(" ", ch);
    }
    stc("#n]", ch);
    if (ch->pcdata->faeriepath[2] == 100)
	stc("     (Completed)\n\r", ch);
    else stc("\n\r", ch);

    stc("Path of the Elders : [", ch);
    for (i = 0 ; i < 40 ; i++)
    {
	if (i > 19 && ch->pcdata->faeriepath[3] * 4 / 10 > i)
	    stc("#2*", ch);
	else if (ch->pcdata->faeriepath[3] * 4 / 10 > i)
	    stc("#3*", ch);
	else stc(" ", ch);
    }
    stc("#n]", ch);
    if (ch->pcdata->faeriepath[3] == 100)
	stc("     (Completed)\n\r", ch);
    else stc("\n\r", ch);
    return;
}

void do_immune(CHAR_DATA *ch, char *argument)
{
    bool message = FALSE;
    int val;

    if (ch->secondaryrace == 0 || IS_NPC(ch) )
    {
	stc("You don't really know what your resistances are.\n\r", ch);
	return;
    }

    ctc("Immunities", ch);
    ctc("---=<>=---", ch);
    stc("\n\r", ch);

    if (ch->secondaryrace == RACE_VAMPIRE)
    {
	message = TRUE;
	ctc("Sunlight is deadly to you.", ch);
    }

    if (ch->secondaryrace == RACE_LUPINE)
    {
	message = TRUE;
	ctc("Silver weapons are extremely deadly to you.", ch);
    }

    if ((val = check_immune(ch, DAM_BASH)) == IS_IMMUNE)
	ctc("You are completely immune to bashing attacks.", ch);
    else if (val == IS_RESISTANT)
	ctc("Your hard skin makes you resistant to bashing attacks.", ch);
    else if (val == IS_VULNERABLE)
	ctc("You have a weakness to bashing attacks.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if ((val = check_immune(ch, DAM_PIERCE)) == IS_IMMUNE)
	ctc("The thrusting of weapons does not harm you in the slightest.", ch);
    else if (val == IS_RESISTANT)
	ctc("You are resistant to thrusting and piercing weapons.", ch);
    else if (val == IS_VULNERABLE)
	ctc("You are very vulnerable to thrusting and piercing weapons.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if ((val = check_immune(ch, DAM_SLASH)) == IS_IMMUNE)
	ctc("You are totally unaffected by slashing weapons.", ch);
    else if (val == IS_RESISTANT)
	ctc("You are quite resistant to slashing weapons.", ch);
    else if (val == IS_VULNERABLE)
	ctc("You find yourself quite vulnerable to slashing weapons.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if ((val = check_immune(ch, DAM_FIRE)) == IS_IMMUNE)
	ctc("You are totally unaffected by fire and heat attacks.", ch);
    else if (val == IS_RESISTANT)
	ctc("You are quite resistant to fire and heat damage.", ch);
    else if (val == IS_VULNERABLE)
	ctc("You are very vulnerable to fire attacks.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if ((val = check_immune(ch, DAM_COLD)) == IS_IMMUNE)
	ctc("You are unaffected by the coldest temperatures.", ch);
    else if (val == IS_RESISTANT)
	ctc("You can survive cold attacks to a limited degree.", ch);
    else if (val == IS_VULNERABLE)
	ctc("You are very vulnerable to the effects of cold.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if ((val = check_immune(ch, DAM_LIGHTNING)) == IS_IMMUNE)
	ctc("Electrical attacks do not harm you in the slightest.", ch);
    else if (val == IS_RESISTANT)
	ctc("You are very resistant to electrical attacks.", ch);
    else if (val == IS_VULNERABLE)
	ctc("You are vulnerable to lightning and electrical attacks.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if ((val = check_immune(ch, DAM_ACID)) == IS_IMMUNE)
	ctc("The strongest of acids will not affect you.", ch);
    else if (val == IS_RESISTANT)
	ctc("You have built up a resistance to acids.", ch);
    else if (val == IS_VULNERABLE)
	ctc("Even the weakest acids will eat away at your skin.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if ((val = check_immune(ch, DAM_POISON)) == IS_IMMUNE)
	ctc("You are totally immune to poison.", ch);
    else if (val == IS_RESISTANT)
	ctc("You are quite resistant to most poisons.", ch);
    else if (val == IS_VULNERABLE)
	ctc("Even the weakest of poisons will strongly affect you.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if ((val = check_immune(ch, DAM_NEGATIVE)) == IS_IMMUNE)
	ctc("You are immune to negative energy attacks.", ch);
    else if (val == IS_RESISTANT)
	ctc("You are quite resistant to negative energy attacks.", ch);
    else if (val == IS_VULNERABLE)
	ctc("You are very vulnerable to negative energy attacks.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if ((val = check_immune(ch, DAM_HOLY)) == IS_IMMUNE)
	ctc("You are immune to positive energy attacks.", ch);
    else if (val == IS_RESISTANT)
	ctc("You are quite resistant to positive energy attacks.", ch);
    else if (val == IS_VULNERABLE)
	ctc("You are very vulnerable to positive energy attacks.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if ((val = check_immune(ch, DAM_DISEASE)) == IS_IMMUNE)
	ctc("You are immune to all known diseases.", ch);
    else if (val == IS_RESISTANT)
	ctc("You are naturally resistant to most diseases.", ch);
    else if (val == IS_VULNERABLE)
	ctc("You are vulnerable to most diseases.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if ((val = check_immune(ch, DAM_DROWNING)) == IS_IMMUNE)
	ctc("You can breathe underwater.", ch);
    else if (val == IS_RESISTANT)
	ctc("You can hold your breath underwater for ages.", ch);
    else if (val == IS_VULNERABLE)
	ctc("You cannot swim underwater.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if ((val = check_immune(ch, DAM_CHARM)) == IS_IMMUNE)
	ctc("You are immune to charming attacks.", ch);
    else if (val == IS_RESISTANT)
	ctc("You are quite resistant to charming attacks.", ch);
    else if (val == IS_VULNERABLE)
	ctc("You are very vulnerable to charming attacks.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if ((val = check_immune(ch, DAM_SOUND)) == IS_IMMUNE)
	ctc("You cannot be affected by sound attacks.", ch);
    else if (val == IS_RESISTANT)
	ctc("You are very resistant to sound attacks.", ch);
    else if (val == IS_VULNERABLE)
	ctc("You are very vulnerable to sound attacks.", ch);
    if (val != IS_NORMAL) message = TRUE;

    if (!message)
	ctc("You aren't really vulnerable or resistant to anything.", ch);
}

bool IS_NOVIERE(CHAR_DATA *ch)
{
    if (ch->secondaryrace != RACE_ELADRIN) return FALSE;
    if (ch->generation != 3 && ch->generation != 4) return FALSE;
    return TRUE;
}

void do_empower(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
	stc("Type 'help empower' for help on usage.\n\r", ch);
	return;
    }

    if (ch->qp <= 0)
    {
	stc("You have no kai and may not attempt this.\n\r", ch);
	return;
    }

    if ( ( obj = get_obj_eq(ch, arg1, ch) ) == NULL)
    {
	stc("You aren't carrying that item.\n\r", ch);
	return;
    }

    if (!str_prefix(arg2, "hitroll"))
    {
	AFFECT_DATA af;
	int value;

	if (argument[0] == '\0' || !is_number(argument))
	{
	    stc("Syntax : empower <item> hitroll <value>\n\r", ch);
	    return;
	}

	value = atoi(argument);

	if (ch->qp < value)
	{
	    stc("You do not have enough kai for this.\n\r", ch);
	    return;
	}

	if (value > 10 || value < 1)
	{
	    stc("Acceptable range is from 1 to 10.\n\r", ch);
	    return;
	}

	if (obj->item_type != ITEM_WEAPON
	    && obj->item_type != ITEM_ARMOR)
	{	
  	    stc("You may only do this to weapons and armor.\n\r", ch);
	    return;
	}

	if (IS_OBJ_STAT(obj, ITEM_HIT_EMPOWERED))
	{	
  	    stc("The hitroll of this item cannot be improved further.\n\r", ch);
	    return;
	}
	
	ch->qp -= value;

        af.where        = TO_OBJECT;
        af.type         = 0;
        af.level        = ch->level;
        af.duration     = -1;
        af.location     = APPLY_HITROLL;
        af.modifier     = value;
        af.bitvector    = 0;
        affect_to_obj(obj,&af);

	SET_BIT(obj->extra_flags, ITEM_HIT_EMPOWERED);
	WAIT_STATE(ch, 8);

	act("$p shudders violently.", ch, obj, NULL, TO_CHAR);
	do_anything(ch, 1);
	return;
    }
    else if (!str_prefix(arg2, "damroll"))
    {
	AFFECT_DATA af;
	int value;

	if (argument[0] == '\0' || !is_number(argument))
	{
	    stc("Syntax : empower <item> damroll <value>\n\r", ch);
	    return;
	}

	value = atoi(argument);

	if (ch->qp < value)
	{
	    stc("You do not have enough kai for this.\n\r", ch);
	    return;
	}

	if (value > 10 || value < 1)
	{
	    stc("Acceptable range is from 1 to 10.\n\r", ch);
	    return;
	}

	if (obj->item_type != ITEM_WEAPON
	    && obj->item_type != ITEM_ARMOR)
	{	
  	    stc("You may only do this to weapons and armor.\n\r", ch);
	    return;
	}

	if (IS_OBJ_STAT(obj, ITEM_DAM_EMPOWERED))
	{	
  	    stc("The hitroll of this item cannot be improved further.\n\r", ch);
	    return;
	}
	
	ch->qp -= value;

        af.where        = TO_OBJECT;
        af.type         = 0;
        af.level        = ch->level;
        af.duration     = -1;
        af.location     = APPLY_DAMROLL;
        af.modifier     = value;
        af.bitvector    = 0;
        affect_to_obj(obj,&af);

	SET_BIT(obj->extra_flags, ITEM_DAM_EMPOWERED);
	WAIT_STATE(ch, 8);

	act("$p vibrates violently.", ch, obj, NULL, TO_CHAR);
	do_anything(ch, 2);
	return;
    }
    else if (!str_prefix(arg2, "shrink"))
    {
	if (ch->qp < 10)
	{
	    stc("You do not have enough kai for this.\n\r", ch);
	    return;
	}

	if (obj->size == SIZE_TINY)
	{
	    act("$p is as small as it can get.", ch, obj, NULL, TO_CHAR);
	    return;
	}

	WAIT_STATE(ch, 8);
	ch->qp -= 10;
	act("$p starts to shrink to half its size.", ch, obj, NULL, TO_CHAR);
	act("$p, held by $n, starts to shrink to half its size.", ch, obj, NULL, TO_ROOM);
	obj->size--;
	do_anything(ch, 3);
	return;
    }
    else if (!str_prefix(arg2, "grow"))
    {
	if (ch->qp < 10)
	{
	    stc("You do not have enough kai for this.\n\r", ch);
	    return;
	}

	if (obj->size == SIZE_GIANT)
	{
	    act("$p is as large as it can get.", ch, obj, NULL, TO_CHAR);
	    return;
	}

	WAIT_STATE(ch, 8);
	ch->qp -= 10;
	act("$p starts to grow to double its size.", ch, obj, NULL, TO_CHAR);
	act("$p, held by $n, starts to grow to double its size.", ch, obj, NULL, TO_ROOM);
	obj->size++;
	do_anything(ch, 3);
	return;
    }
    else if (!str_prefix(arg2, "light"))
    {
	if (ch->qp < 2)
	{
	    stc("You do not have enough kai for this.\n\r", ch);
	    return;
	}

	if (IS_OBJ_STAT(obj, ITEM_GLOW))
	{
	    act("$p is already glowing.", ch, obj, NULL, TO_CHAR);
	    return;
	}

	WAIT_STATE(ch, 8);
	ch->qp -= 2;
	act("$p starts to glow softly.", ch, obj, NULL, TO_CHAR);
	act("$p, held by $n, starts to glow softly.", ch, obj, NULL, TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_GLOW);
	do_anything(ch, 3);
	return;
    }
    else if (!str_prefix(arg2, "weightless"))
    {
	if (ch->qp < 3)
	{
	    stc("You do not have enough kai for this.\n\r", ch);
	    return;
	}

	if (obj->weight == 0 || IS_OBJ_STAT2(obj, ITEM_FLYING) )
	{
	    act("$p is already weightless.", ch, obj, NULL, TO_CHAR);
	    return;
	}

	WAIT_STATE(ch, 8);
	ch->qp -= 3;
	act("$p starts to hover.", ch, obj, NULL, TO_CHAR);
	act("$p, held by $n, starts to hover.", ch, obj, NULL, TO_ROOM);
	obj->weight = 0;
	SET_BIT(obj->extra_flags2, ITEM_FLYING);
	do_anything(ch, 3);
	return;
    }
    else if (!str_prefix(arg2, "vanish"))
    {
	if (ch->qp < 4)
	{
	    stc("You do not have enough kai for this.\n\r", ch);
	    return;
	}

	if (IS_OBJ_STAT(obj, ITEM_INVIS))
	{
	    act("$p is already invisible.", ch, obj, NULL, TO_CHAR);
	    return;
	}

	WAIT_STATE(ch, 8);
	ch->qp -= 4;
	act("$p starts to fade from existance.", ch, obj, NULL, TO_CHAR);
	act("$p, held by $n, starts to fade from existance.", ch, obj, NULL, TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_INVIS);
	do_anything(ch, 3);
	return;
    }
    else if (!str_prefix(arg2, "loyal"))
    {
	if (ch->qp < 5)
	{
	    stc("You do not have enough kai for this.\n\r", ch);
	    return;
	}

	if (IS_OBJ_STAT2(obj, ITEM_LOYAL))
	{
	    act("$p is already loyal to you.", ch, obj, NULL, TO_CHAR);
	    return;
	}

	WAIT_STATE(ch, 8);
	ch->qp -= 5;
	act("$p starts to shudder and vibrate.", ch, obj, NULL, TO_CHAR);
	act("$p, held by $n, starts to vibrate.", ch, obj, NULL, TO_ROOM);
	SET_BIT(obj->extra_flags2, ITEM_LOYAL);
	do_anything(ch, 4);
	return;
    }
    else if (!str_prefix(arg2, "spell"))
    {
	int cost, flag;

	if (obj->item_type != ITEM_ARMOR
	    && obj->item_type != ITEM_WEAPON)
	{
	    stc("You may only imbue a spell within weapons and armour.\n\r", ch);
	    return;
	}

	if (obj->perm_spell != -1)
	{
	    stc("This object already has a spell imbued within.\n\r", ch);
	    return;
	}

	if (argument[0] == '\0')
	{
	    stc("Syntax : empower <item> spell <spell>\n\r", ch);
	    stc(" possible effects are - invis detevil detgood shield bless plague haste armour sanctuary windtomb\n\r", ch);
	    return;
	}

	if (!str_prefix(argument, "invis"))
	{
	    cost = 20;
	    flag = ITEMA_INVISIBILITY;
	}
	else if (!str_prefix(argument, "detevil"))
	{
	    cost = 10;
	    flag = ITEMA_DETECT_EVIL;
	}
	else if (!str_prefix(argument, "detgood"))
	{
	    cost = 10;
	    flag = ITEMA_DETECT_GOOD;
	}
	else if (!str_prefix(argument, "shield"))
	{
	    cost = 20;
	    flag = ITEMA_SHIELD;
	}
	else if (!str_prefix(argument, "bless"))
	{
	    cost = 15;
	    flag = ITEMA_BLESS;
	}
	else if (!str_prefix(argument, "haste"))
	{
	    cost = 35;
	    flag = ITEMA_HASTE;
	}
	else if (!str_prefix(argument, "plague"))
	{
	    cost = 10;
	    flag = ITEMA_PLAGUE;
	}
	else if (!str_prefix(argument, "armour"))
	{
	    cost = 15;
	    flag = ITEMA_ARMOR;
	}
	else if (!str_prefix(argument, "sanctuary"))
	{
	    cost = 50;
	    flag = ITEMA_SANCTUARY;
	}
	else if (!str_prefix(argument, "windtomb"))
	{
	    cost = 40;
	    flag = ITEMA_WINDTOMB;
	}
	else
  	{
	    stc("Syntax : empower <item> spell <spell>\n\r", ch);
	    stc(" possible effects are - invis detevil detgood shield bless plague haste armour sanctuary windtomb\n\r", ch);
	    return;
	}

	if (ch->qp < cost)
	{
	    stcf(ch, "You need %d more kai points to imbue this spell.\n\r",
		   cost - ch->qp);
	    return;
	}

	ch->qp -= cost;
	obj->perm_spell = flag;
	WAIT_STATE(ch, cost/2);
	do_anything(ch, cost/2);
	act("You have successfully imbued the spell within $p.",
		ch, obj, NULL, TO_CHAR);
	return;
    }
    else if (!str_prefix(arg2, "effect"))
    {
	int cost;
	long flag;

	if (obj->item_type != ITEM_WEAPON)
	{
	    act("$p is not a weapon!", ch, obj, NULL, TO_CHAR);
	    return;
	}

	if (IS_OBJ_STAT2(obj, ITEM_WEAP_EFFECT_ADDED))
	{
	    act("$p has already had a weapon effect added.", ch, obj, NULL, TO_CHAR);
	    return;
	}

	if (argument[0] == '\0')
	{
	    stc("Syntax : empower <weapon> effect <effect>\n\r", ch);
	    stc(" possible effects are - shock flame frost drain sharp poison sunblade silver\n\r", ch);
	    return;
	}

	if (!str_prefix(argument, "shocking"))
	{
	    cost = 30;
	    flag = WEAPON_SHOCKING;
	}
	else if (!str_prefix(argument, "flame"))
	{
	    cost = 20;
	    flag = WEAPON_FLAMING;
	}
	else if (!str_prefix(argument, "frost"))
	{
	    cost = 25;
	    flag = WEAPON_FROST;
	}
	else if (!str_prefix(argument, "drain"))
	{
	    cost = 15;
	    flag = WEAPON_VAMPIRIC;
	}
	else if (!str_prefix(argument, "sharp"))
	{
	    cost = 30;
	    flag = WEAPON_SHARP;
	}
	else if (!str_prefix(argument, "poison"))
	{
	    cost = 20;
	    flag = WEAPON_POISON;
	}
	else if (!str_prefix(argument, "sunblade"))
	{
	    cost = 50;
	    flag = WEAPON_SUNBLADE;
	}
	else if (!str_prefix(argument, "silver"))
	{
	    cost = 40;
	    flag = WEAPON_SILVER;
	}
	else
	{
	    stc("Syntax : empower <weapon> effect <effect>\n\r", ch);
	    stc(" possible effects are - shock flame frost drain sharp poison sunblade silver\n\r", ch);
	    return;
	}

	if (ch->qp < cost)
	{
	    stc("You do not have enough kai for this.\n\r", ch);
	    return;
	}

	act("You store your kai within $p.", ch, obj, NULL, TO_CHAR);
	WAIT_STATE(ch, 8);
	ch->qp -= cost;
	SET_BIT(obj->value[4], flag);
	SET_BIT(obj->extra_flags2, ITEM_WEAP_EFFECT_ADDED);
	do_anything(ch, 8);
	return;
    }
    else if (!str_prefix(arg2, "improve"))
    {
	if (IS_OBJ_STAT3(obj, ITEM_EMPOWER_IMPROVED))
 	{
	    act("$p has already been improved.", ch, obj, NULL, TO_CHAR);
	    return;
	}

	if (ch->qp < 40)
	{
	    stc("You do not have enough kai for this.\n\r", ch);
	    return;
	}

	WAIT_STATE(ch, 8);
	do_anything(ch, 9);
	ch->qp -= 40;
	REMOVE_BIT(obj->extra_flags, ITEM_HIT_EMPOWERED);
	REMOVE_BIT(obj->extra_flags, ITEM_DAM_EMPOWERED);
	REMOVE_BIT(obj->extra_flags2, ITEM_WEAP_EFFECT_ADDED);
	SET_BIT(obj->extra_flags3, ITEM_EMPOWER_IMPROVED);
	act("$p can now hold more kai.", ch, obj, NULL, TO_CHAR);
	return;

    }
    else
    {
	do_empower(ch, "");
	return;
    }

}

bool in_same_room(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (ch->in_room != victim->in_room) return FALSE;
    if (ch->altitude != victim->altitude) return FALSE;
    return TRUE;
}

bool can_age(CHAR_DATA *ch)
{
    if (IS_NPC(ch)) return FALSE;
    if (ch->secondaryrace == RACE_VAMPIRE
	|| ch->secondaryrace == RACE_DAEMON
	|| ch->secondaryrace == RACE_ELADRIN
	|| ch->secondaryrace == RACE_DRAGON) return FALSE;
    if (ch->tertiaryrace != 0) return FALSE;
    return TRUE;
}

bool can_breathe_underwater(CHAR_DATA *ch)
{
    if (ch->level > 112) return TRUE;
    if (IS_AFFECTED(ch, skill_lookup("water breathing"))) return TRUE;
    if (is_undead(ch)) return TRUE;
    if (ch->power[DISC_HERO_ELEM] >= 2) return TRUE;

    /* Noviere */
    if (ch->secondaryrace == RACE_ELADRIN &&
	(ch->generation == 3 || ch->generation == 4) ) return TRUE;

    if (IS_SET(ch->imm_flags, IMM_DROWNING)) return TRUE;
    return FALSE;
}

void strip_perm_spell(CHAR_DATA *ch, OBJ_DATA *obj)
{
    OBJ_DATA *obj2;

    /* If there is no spell on the item */
    if (obj->perm_spell == -1) return;

    /* First check that the person is not wearing any other item of
	equipment with that spell on! */

    for (obj2 = ch->carrying ; obj2 != NULL ; obj2 = obj2->next_content )
    {
	if (obj2->wear_loc == WEAR_NONE) continue;
	if (obj2->perm_spell == -1) continue;
	if (obj2 == obj) continue;
	if (obj2->perm_spell == obj->perm_spell) return;
    }

    /* Now strip the spells */
    switch ( obj->perm_spell )
    {
	case ITEMA_SANCTUARY :
	    stc("The white aura around you vanishes.\n\r", ch);
	    act("The white aura around $n vanishes.", ch, NULL, NULL, TO_ROOM);
	    affect_strip(ch, gsn_sanctuary);
	    return;
	    break;
	case ITEMA_HASTE :
	    stc("You feel slower.\n\r", ch);
	    act("$n stops moving so quickly.", ch, NULL, NULL, TO_ROOM);
	    affect_strip(ch, skill_lookup("haste"));
	    return;
	    break;
	case ITEMA_WINDTOMB :
	    stc("The roaring hurricane of wind around you vanishes.\n\r", ch);
	    act("The roaring hurricane of wind around $n vanishes.", ch, NULL, NULL, TO_ROOM);
	    affect_strip(ch, skill_lookup("windtomb"));
	    return;
	    break;
	case ITEMA_FLY :
	    stc("The fly spell magically runs out.\n\r", ch);
	    affect_strip(ch, skill_lookup("fly"));
	    check_fall(ch);
	    return;
	    break;
	case ITEMA_ACCURACY :
	    stc("You feel less accurate in combat.\n\r", ch);
	    affect_strip(ch, skill_lookup("accuracy"));
	    return;
	    break;
	case ITEMA_DETECT_EVIL :
	    stc("The red in your vision disappears.\n\r", ch);
	    affect_strip(ch, skill_lookup("detect evil"));
	    return;
	    break;
	case ITEMA_INVISIBILITY :
	    affect_strip(ch, skill_lookup("invisibility"));
	    do_visible(ch, "nomessage");
	    stc("You become visible once more.\n\r", ch);
	    act("$n appears from nowhere.", ch, NULL, NULL, TO_ROOM);
	    return;
	    break;
	case ITEMA_DETECT_MAGIC :
	    stc("The detect magic spell wears off.\n\r", ch);
	    affect_strip(ch, skill_lookup("detect magic"));
	    return;
	    break;
	case ITEMA_ARMOR :
	    stc("You feel less armoured.\n\r", ch);
	    affect_strip(ch, skill_lookup("armour"));
	    return;
	    break;
	case ITEMA_STONE_SKIN :
	    stc("Your skin regains its normal consistency.\n\r", ch);
	    affect_strip(ch, skill_lookup("stone skin"));
	    return;
	    break;
	case ITEMA_SHIELD :
	    stc("The magical shield protecting you vanishes.\n\r", ch);
	    affect_strip(ch, skill_lookup("shield"));
	    return;
	    break;
	case ITEMA_DETECT_GOOD :
	    stc("The gold in your vision disappears.\n\r", ch);
	    affect_strip(ch, skill_lookup("detect good"));
	    return;
	    break;
	case ITEMA_BLESS :
	    stc("You feel the bless spell wearing off.\n\r", ch);
	    affect_strip(ch, skill_lookup("bless"));
	    return;
	    break;
	case ITEMA_WATER_BREATHING :
	    stc("You feel a tingling around your neck.\n\r", ch);
	    affect_strip(ch, skill_lookup("water breathing"));
	    return;
	    break;
	case ITEMA_IRON_SKIN :
	    stc("Your skin regains its normal consistency.\n\r", ch);
	    affect_strip(ch, skill_lookup("iron skin"));
	    return;
	    break;
	case ITEMA_PLAGUE :
	    stc("You feel cured of the plague.\n\r", ch);
	    affect_strip(ch, skill_lookup("plague"));
	    return;
	    break;
	case ITEMA_PROTECTION_EVIL :
	    stc("You feel more vunerable to evil.\n\r", ch);
	    affect_strip(ch, skill_lookup("protection evil"));
	    return;
	    break;
	default : return;
    }
    return;
}


bool bad_to_interpret(char *argument)
{
    if (!str_prefix("rem", argument)) return TRUE;
    if (!str_prefix("delet", argument)) return TRUE;
    if (!str_prefix("qui", argument)) return TRUE;
    if (!str_prefix("kai", argument)) return TRUE;
    if (!str_prefix("unm", argument)) return TRUE;
    if (!str_prefix("resis", argument)) return TRUE;
    if (!str_prefix("vamp", argument)) return TRUE;
    if (!str_prefix("pled", argument)) return TRUE;
    if (!str_prefix("ded", argument)) return TRUE;
    if (!str_prefix("sub", argument)) return TRUE;
    if (!str_prefix("soul", argument)) return TRUE;
    if (!str_prefix("ema", argument)) return TRUE;
    if (!str_prefix("ol", argument)) return TRUE;
    if (!str_prefix("pr", argument)) return TRUE;
    if (!str_prefix("reboot", argument)) return TRUE;
    if (!str_prefix("shutdown", argument)) return TRUE;
    if (!str_prefix("de", argument)) return TRUE;
    if (!str_prefix("not", argument)) return TRUE;
    if (!str_prefix("id", argument)) return TRUE;
    if (!str_prefix("changes", argument)) return TRUE;
    if (!str_prefix("suc", argument)) return TRUE;

    return FALSE;
}


int get_extra_attacks(CHAR_DATA *ch)
{
    int atta = 0;

    /* Number between -5 and 5 -
	all extra attacks are NOT gained each round */

    /* Celerity is the most important one! */
    if (ch->power[DISC_VAMP_CELE] > 0)
	atta += ch->power[DISC_VAMP_CELE] / 2;

    /* Fox is the second most important one! */
    if (ch->power[DISC_VAMP_CELE] > 0)
	atta += ch->power[DISC_VAMP_CELE] / 3;

    /* Physica can add up to 2 extra attacks */
    if (ch->power[DISC_ELAD_PHYS] > 1) atta++;
    if (ch->power[DISC_ELAD_PHYS] > 7) atta++;

    /* Haste and slow are quite nasty, indeed! */
    if (IS_FLAG5(ch, AFF5_IMPROVED_HASTE)) atta += 2;
    else if (IS_AFFECTED(ch, AFF_HASTE)) atta++;
    if (IS_FLAG5(ch, AFF5_IMPROVED_SLOW)) atta -=2;
    else if (IS_AFFECTED(ch, AFF_SLOW)) atta--;

    /* Demonic Attack can grant an extra attack */
    if (ch->power[DISC_DAEM_ATTA] > 1) atta++;

    /* Battle can grant up to 2 extra attacks */
    if (ch->power[DISC_HERO_BATT] > 0) atta++;
    if (ch->power[DISC_HERO_BATT] > 3) atta++;
    if (ch->power[DISC_HERO_BATT] > 6) atta++;
    if (ch->power[DISC_HERO_BATT] > 9) atta++;

    /* Pixies are fast by nature, and trees and golems are slow */
    if (ch->cur_form == FRM_PIXIE) atta += 2;
    if (ch->cur_form == FRM_TREE) atta -= 2;
    if (ch->cur_form == FRM_GOLEM) atta -= 2;

    /* Spose we gotta help warlocks as well! */
    if (ch->power[DISC_WARL_LUCU] > 0)
	atta += ch->power[DISC_WARL_LUCU] / 5;

    /* Lunas strike - kinda fudged in, but who cares????? */
    if (IS_SET(ch->flag2, AFF_CLAW)
	&& ch->power[DISC_LUPI_LUNA] > 7) atta++;

    /* Lightning strike */
    if (IS_SET(ch->flag2, AFF_CLAW)
	&& ch->power[DISC_LUPI_COBR] > 4) atta++;

    /* Quasit familiar */
    if (ch->pet != NULL
	&& IS_FLAG5(ch->pet, AFF5_FAMILIAR_QUASIT) ) atta++;

    /* And finally return the value */
    if (atta > 5) atta = 5;
    return atta;
}

void do_blink(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (ch->fighting == NULL && argument[0] == '\0')
    {
	stc("Who do you wish to blink into combat with?\n\r", ch);
	return;
    }

    if (ch->fighting == NULL)
    {
	if ( (victim = get_char_room(ch, argument) ) == NULL)
 	{
	    stc("They aren't here.\n\r", ch);
	    return;
 	}
 	WAIT_STATE(ch, 6);
	SET_BIT(ch->flag5, AFF5_BLINK_1ST_RD);
	stop_fighting(ch, TRUE);
	stc("You pop out of existance.\n\r", ch);
	act("$n pops out of existance.", ch, NULL, NULL, TO_ROOM);
	ch->blinkykill = victim;
	return;
    }
    victim = ch->fighting;
    SET_BIT(ch->flag5, AFF5_BLINK_2ND_RD);
    stop_fighting(ch, TRUE);
    stc("You pop out of existance.\n\r", ch);
    act("$n pops out of existance.", ch, NULL, NULL, TO_ROOM);
    ch->blinkykill = victim;
}

/* Ok this is the AURA to level 10 shit......
 * These are the levels ... 
 * 1 - Red - Resistance to fire (all resistances are cumulative)
 * 2 - Orange - Resistance to negative energy attacks
 * 3 - Yellow - 2/3 damage from all attacks
 * 4 - Green - Resistance to poison (cumulative)
 * 5 - Blue - Resistance to cold  
 * 6 - Indigo - Resistance to lightning 
 * 7 - Violet - Half damage from all attacks
 * 8 - Brown - Immunity to fire
 * 9 - Silvery - Reflection from magical attacks (haw haw)
 * 10 - Golden - 1/3 damage from all attacks
 */

void deconstruct_aura(CHAR_DATA *ch)
{
    int level;

    crash("deconstruct aura");

    level = ch->power[DISC_ELAD_AURA];

    if (level > 0) REMOVE_BIT(ch->res_flags, RES_FIRE);
    if (level > 1) REMOVE_BIT(ch->res_flags, RES_NEGATIVE);
    if (level > 3) REMOVE_BIT(ch->res_flags, RES_POISON);
    if (level > 4) REMOVE_BIT(ch->res_flags, RES_COLD);
    if (level > 5) REMOVE_BIT(ch->res_flags, RES_LIGHTNING);
    if (level > 7) REMOVE_BIT(ch->imm_flags, IMM_FIRE);
    if (level > 8) REMOVE_BIT(ch->flag3, AFF_REFLECTION);

    ch->aura = 0;

    crash("deconstruct aura - end function");
}

void construct_aura(CHAR_DATA *ch)
{
    int level;

    crash("construct aura");

    level = ch->power[DISC_ELAD_AURA];

    if (IS_FLAG5(ch, AFF5_BANE)) level /= 2;

    if (level > 0) SET_BIT(ch->res_flags, RES_FIRE);
    if (level > 1) SET_BIT(ch->res_flags, RES_NEGATIVE);
    if (level > 3) SET_BIT(ch->res_flags, RES_POISON);
    if (level > 4) SET_BIT(ch->res_flags, RES_COLD);
    if (level > 5) SET_BIT(ch->res_flags, RES_LIGHTNING);
    if (level > 7) SET_BIT(ch->imm_flags, IMM_FIRE);
    if (level > 8) SET_BIT(ch->flag3, AFF_REFLECTION);

    ch->aura = level;

    crash("construct aura - end function");
}

void do_devil(CHAR_DATA *ch, char *argument)
{  
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to make into a devil?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, argument)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch)
    {
	stc("You cannot do this on yourself.\n\r", ch);
	return;
    }

    if (IS_NPC(victim))
    {
	stc("You can only do this on players.\n\r", ch);
	return;
    }

    if (victim->secondaryrace != 0)
    {
	stc("They are already a secondary race.\n\r", ch);
	return;
    }

    if (IS_TAINTED(victim))
    {
	stc("They are already tainted.\n\r", ch);
	return;
    }

    if (IS_GOOD(victim))
    {
	stc("Their soul is too pure to taint.\n\r", ch);
	return;
    }

    act("$n starts to whisper temptations into $N's ear.", ch, NULL, victim, TO_NOTVICT);
    act("You whisper the temptations of evil into $N's ear.", ch, NULL, victim, TO_CHAR);

    if (sanity_resist(ch, victim, 1)) return;

    act("$n starts to tempt you onto the side of evil.", ch, NULL, victim, TO_VICT);
    stc("You little devil, you!\n\r", victim);

    taint_transform(victim, RACE_DAEMON);

    if (ch->power[DISC_DAEM_CORR] > 1)
	victim->power[DISC_DAEM_CORR] = ch->power[DISC_DAEM_CORR] / 2;
    if (ch->power[DISC_DAEM_GELU] > 1)
	victim->power[DISC_DAEM_GELU] = ch->power[DISC_DAEM_GELU] / 2;
    if (ch->power[DISC_DAEM_HELL] > 1)
	victim->power[DISC_DAEM_HELL] = ch->power[DISC_DAEM_HELL] / 2;
    if (ch->power[DISC_DAEM_ATTA] > 1)
	victim->power[DISC_DAEM_ATTA] = ch->power[DISC_DAEM_ATTA] / 2;
    if (ch->power[DISC_DAEM_TEMP] > 1)
	victim->power[DISC_DAEM_TEMP] = ch->power[DISC_DAEM_TEMP] / 2;

    WAIT_STATE(ch, 12);
    WAIT_STATE(victim, 12);

    victim->alignment = -900;
}

void do_pixie(CHAR_DATA *ch, char *argument)
{  
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to make into a pixie?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, argument)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch)
    {
	stc("You cannot do this on yourself.\n\r", ch);
	return;
    }

    if (IS_NPC(victim))
    {
	stc("You can only do this on players.\n\r", ch);
	return;
    }

    if (IS_TAINTED(victim))
    {
	stc("They are already tainted.\n\r", ch);
	return;
    }

    if (victim->secondaryrace != 0)
    {
	stc("They are already a secondary race.\n\r", ch);
	return;
    }

    if (IS_EVIL(victim))
    {
	stc("They are too evil to make into a pixie.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);
    WAIT_STATE(victim, 12);

    act("$n's eyes glow #2green.", ch, NULL, NULL, TO_ROOM);
    if (sanity_resist(ch, victim, 3)) return;

    act("$n smiles as you start to shrink and transform into a pixie.", ch, NULL, victim, TO_VICT);
    act("$n smiles as $N starts to shrink and transform into a pixie.", ch, NULL, victim, TO_NOTVICT);
    act("You transform $N into a little pixie.", ch, NULL, victim, TO_CHAR);

    taint_transform(victim, RACE_ELADRIN);

    if (ch->power[DISC_ELAD_MORP] > 1)
	victim->power[DISC_ELAD_MORP] = ch->power[DISC_ELAD_MORP] / 2;
    if (ch->power[DISC_ELAD_AURA] > 1)
	victim->power[DISC_ELAD_AURA] = ch->power[DISC_ELAD_AURA] / 2;
    if (ch->power[DISC_ELAD_PHYS] > 1)
	victim->power[DISC_ELAD_PHYS] = ch->power[DISC_ELAD_PHYS] / 2;
    if (ch->power[DISC_ELAD_ANSO] > 1)
	victim->power[DISC_ELAD_ANSO] = ch->power[DISC_ELAD_ANSO] / 2;
    if (ch->power[DISC_ELAD_ARTI] > 1)
	victim->power[DISC_ELAD_ARTI] = ch->power[DISC_ELAD_ARTI] / 2;
    if (ch->power[DISC_ELAD_MELO] > 1)
	victim->power[DISC_ELAD_MELO] = ch->power[DISC_ELAD_MELO] / 2;

    victim->alignment = 600;
}

/* Another multiple use command like bathe */
void do_plant(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if (argument[0] == '\0')
    {
	stc("What do you wish to plant?\n\r", ch);
	return;
    }

    if (IS_INSIDE(ch))
    {
	stc("But you are indoors.\n\r", ch);
	return;
    }

    if (ch->in_room->sector_type == SECT_AIR
        || ch->in_room->sector_type == SECT_CITY 
        || ch->in_room->sector_type == SECT_UNDERWATER)
    {
	stc("You cannot plant anything here.\n\r", ch);
	return;
    }

    if ( (obj = get_obj_carry(ch, argument, ch)) == NULL)
    {
	stc("You are not carrying that item.\n\r", ch);
	return;
    }

    if (obj->item_type == ITEM_FOOD && obj->value[2] != 0)
    {
	WAIT_STATE(ch, 8);
	act("You plant $p in the ground.  It is swallowed by the earth", ch, obj, NULL, TO_CHAR);	    
	act("$n plants $p in the ground.  It is swallowed by the earth.", ch, obj, NULL, TO_ROOM);   
	ch->recall_room  = ch->in_room->vnum;
	extract_obj(obj);
	return;
    }
    stc("You cannot plant that object.\n\r", ch);

}

void crash(char *arg)
{
    debug_counter++;
    if (debug_counter == 20) debug_counter = 0;
    WRITE_STR(debug_procedure[debug_counter], arg);
    SS(0);
}

void do_recall(CHAR_DATA *ch, char *argument)
{
    if (ch->secondaryrace != 0)
    {
	stc("The immortals do not extend their gift to non-mortals.\n\r",ch);
	return;
    }

    if (is_affected(ch, gsn_curse))
    {
	stc("You cannot recall while cursed.\n\r", ch);
	return;
    }

    if (ch->in_room->vnum == ch->recall_room)
    {
	stc("You are already in your recall room.\n\r", ch);
	return;
    }

    if (!IS_SET(ch->flag4, AFF_SET_A_RECALL))
    {
	SET_BIT(ch->flag4, AFF_SET_A_RECALL);
        ch->recall_room = 3001;
    }

    if (IS_INSIDE(ch))
    {
	stc("You cannot recall while inside.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);
    stc("A large hand comes down from the sky and drags you into the air.\n\r", ch);
    act("A large hand grabs $n and drags $m into the air, screaming.", ch, NULL, NULL, TO_ROOM);
    do_yell(ch, "Aaaaaaaarrrrrrrggggggggghhhhhh!");
    char_from_room( ch );
    char_to_room(ch, get_room_index(char_realm(ch),ch->recall_room), 0);
    act("$n drops from the sky and lands with a bump.", ch, NULL, NULL, TO_ROOM);
    ch->position = POS_RESTING;
    ch->move /= 2;
    if (ch->in_room->vnum == 1)
    {
	ch->recall_room = 3001;
        char_from_room(ch);
        char_to_room(ch, get_room_index(char_realm(ch), ch->recall_room), 0);
    }
    do_look(ch, "auto");
    stc("You fall on the floor with a bump.\n\r", ch);
}

void do_weirdweather(CHAR_DATA *ch, char *argument)
{
    int weather;

    if (!is_number(argument))
    {
	stc("Argument must be a number.\n\r", ch);
	return;
    }

    weather = atoi(argument);

    if (weather < 0 || weather > 10)
    {
	stc("Number must be between 0 and 10.\n\r", ch);
	return;
    }
    weird_weather_type = weather;
    weird_weather_timer = dice(3, 6);
}

void do_testlink(CHAR_DATA *ch, char *argument)
{
    if (is_item(ch))
 	stc("You are an item.\n\r", ch);

    if (is_mob_switched(ch))
	stc("You are now a mob.\n\r", ch);

    act("And the random person is $N.", ch, NULL, get_rand_person_in_room(ch->in_room), TO_CHAR);

}

void do_nexuscall(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if (ch->in_room->far_link == NULL || ch->altitude != 0) 
    {
	stc("There is no nexus in this room.\n\r", ch);
	return;
    }

    act("$n starts to chant in strange languages.", ch, NULL, NULL, TO_ROOM);
    WAIT_STATE(ch, 8);    

    if (IS_SET(ch->in_room->far_link->room_flags, ROOM_SAFE))
	stc("Nothing seems to happen.\r\n", ch);
    else
    {   
	for (vch = ch->in_room->far_link->people ; vch != NULL ; 
	     vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    stc("The portal in the air glows suddenly.\n\r", vch);
	    stc("You are pulled through.\n\r", vch);
	    char_from_room(vch);
	    char_to_room(vch, ch->in_room, ch->altitude);
	    do_look(vch, "auto");
	    act("$n falls out of the nexus.", vch, NULL, NULL, TO_ROOM);
	}    
    }

    room_message(ch->in_room, "The nexus begins to flicker.");
    room_message(ch->in_room, "The nexus vanishes in a puff of smoke.");
    room_message(ch->in_room->far_link, "The nexus begins to flicker.");
    room_message(ch->in_room->far_link, "The nexus vanishes in a puff of smoke.");
    SET_RTIMER(ch->in_room->far_link, RTIMER_NEXUS_FLAME, 0);
    SET_RTIMER(ch->in_room->far_link, RTIMER_NEXUS_AIR, 0);
    SET_RTIMER(ch->in_room->far_link, RTIMER_NEXUS_WATER, 0);
    SET_RTIMER(ch->in_room->far_link, RTIMER_NEXUS_ENTROPY, 0);
    SET_RTIMER(ch->in_room->far_link, RTIMER_NEXUS_EARTH, 0);
    ch->in_room->far_link->far_link = NULL;
    ch->in_room->far_link = NULL;
    SET_RTIMER(ch->in_room, RTIMER_NEXUS_FLAME, 0);
    SET_RTIMER(ch->in_room, RTIMER_NEXUS_ENTROPY, 0);
    SET_RTIMER(ch->in_room, RTIMER_NEXUS_AIR, 0);
    SET_RTIMER(ch->in_room, RTIMER_NEXUS_WATER, 0);
    SET_RTIMER(ch->in_room, RTIMER_NEXUS_EARTH, 0);
}

void do_nexusgate(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *room;

    if (ch->in_room->far_link == NULL || ch->altitude != 0)
    {
	stc("There is no nexus in this room.\n\r", ch);
	return;
    }
    WAIT_STATE(ch, 8);
    SET_RTIMER(ch->in_room->far_link, RTIMER_NEXUS_FLAME, 0);
    SET_RTIMER(ch->in_room->far_link, RTIMER_NEXUS_AIR, 0);
    SET_RTIMER(ch->in_room->far_link, RTIMER_NEXUS_WATER, 0);
    SET_RTIMER(ch->in_room->far_link, RTIMER_NEXUS_ENTROPY, 0);
    SET_RTIMER(ch->in_room->far_link, RTIMER_NEXUS_EARTH, 0);
    SET_RTIMER(ch->in_room, RTIMER_NEXUS_FLAME, 0);
    SET_RTIMER(ch->in_room, RTIMER_NEXUS_ENTROPY, 0);
    SET_RTIMER(ch->in_room, RTIMER_NEXUS_AIR, 0);
    SET_RTIMER(ch->in_room, RTIMER_NEXUS_WATER, 0);
    SET_RTIMER(ch->in_room, RTIMER_NEXUS_EARTH, 0);

    stc("You step into the nexus.\n\r", ch);
    act("$n steps into the nexus.", ch, NULL, NULL, TO_ROOM);
    room = ch->in_room->far_link;
    char_from_room(ch);
    char_to_room(ch, room, 0);
    act("$n steps out of the nexus.", ch, NULL, NULL, TO_ROOM);
    do_look(ch, "auto");
    room_message(ch->in_room, "The nexus begins to flicker.");
    room_message(ch->in_room, "The nexus vanishes in a puff of smoke.");

    if (dice(1, 10) != 4)
    {
	stc("You feel dazed and confused.\n\r", ch);
        SET_TIMER(ch, TIMER_NEXUS_STUNNED, dice(2, 3));
    }
    
    ch->in_room->far_link->far_link = NULL;
    ch->in_room->far_link = NULL;
}

void do_nexus(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int timer;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to send a nexus to?\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_CAN_DO_NEXUS))
    {
	stc("You cannot make another nexus just yet.\n\r", ch);
	return;
    }

    if ( (victim = get_char_world(ch, argument) ) == NULL)
    {
	stc("You cannot sense anybody of that name.\n\r", ch);
	return;
    }

    if (IS_NPC(victim))
    {
	stc("You can only direct a nexus towards a player.\n\r", ch);
	return;
    }

    if (ch->in_room == victim->in_room)
    {
	stc("You cannot open a nexus to somebody in the same room.\n\r", ch);
	return;
    }

    if (ch->zone != victim->zone && ch->power[DISC_WARL_FARR] < 4)
    {
	stc("You cannot sense anybody of that name.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_NEXUS_STUNNED))
    {
	stc("You feel too stunned and disorientated to do that.\n\r", ch);
	return;
    }

    if (IS_SET(victim->flag2, AFF_SHIELD_AURA)
	&& ch->power[DISC_WARL_FARR] < 5)
    {
	stc("You cannot sense anybody of that name.\n\r", ch);
	return;
    }

    if (!RTIME_UP(ch->in_room, RTIMER_NEXUS_FLAME)
	|| !RTIME_UP(ch->in_room, RTIMER_NEXUS_WATER)
	|| !RTIME_UP(ch->in_room, RTIMER_NEXUS_AIR)
	|| !RTIME_UP(ch->in_room, RTIMER_NEXUS_EARTH)
	|| !RTIME_UP(ch->in_room, RTIMER_NEXUS_ENTROPY)
	|| ch->in_room->far_link != NULL)
    {
	stc("There is already a nexus in this room.\n\r", ch);
	return;
    }

    if (!RTIME_UP(victim->in_room, RTIMER_NEXUS_FLAME)
	|| !RTIME_UP(victim->in_room, RTIMER_NEXUS_WATER)
	|| !RTIME_UP(victim->in_room, RTIMER_NEXUS_AIR)
	|| !RTIME_UP(victim->in_room, RTIMER_NEXUS_EARTH)
	|| !RTIME_UP(victim->in_room, RTIMER_NEXUS_ENTROPY)
	|| victim->in_room->far_link != NULL)
    {
	stc("There is already a nexus in that room.\n\r", ch);
	return;
    }

    if (ch->primary == 0)
    {
	stc("A nexus requires a primary element.\n\r", ch);
	return;
    }

    timer = dice(2, 3);
    WAIT_STATE(ch, 8);

    act("$n waves $s hands in the air.", ch, NULL, NULL, TO_ROOM);

    if (dice(1, 2) == 1)
	room_message(ch->in_room,"Lightning sparks in the air.");
    else room_message(ch->in_room, "The air feels slightly warmer.");
	
    if (ch->primary == REALM_FLAME)
    {
	SET_TIMER(ch, TIMER_CAN_DO_NEXUS, dice(2, 3) );
	SET_RTIMER(ch->in_room, RTIMER_NEXUS_FLAME, timer);
	SET_RTIMER(victim->in_room, RTIMER_NEXUS_FLAME, timer);
	ch->in_room->far_link = victim->in_room;
	ch->in_room->far_link->far_link = ch->in_room;
	room_message(ch->in_room, "A burning ring of fire appears in the air.");
	room_message(victim->in_room, "A burning ring of fire appears in the air.");
    }
    else if (ch->primary == REALM_WATER)
    {
	SET_TIMER(ch, TIMER_CAN_DO_NEXUS, dice(2, 3) );
	SET_RTIMER(ch->in_room, RTIMER_NEXUS_WATER, timer);
	SET_RTIMER(victim->in_room, RTIMER_NEXUS_WATER, timer);
	ch->in_room->far_link = victim->in_room;
	ch->in_room->far_link->far_link = ch->in_room;
	room_message(ch->in_room, "A flowing ring of water appears in the air.");
	room_message(victim->in_room, "A flowing ring of water appears in the air.");
    }
    else if (ch->primary == REALM_AIR)
    {
	SET_TIMER(ch, TIMER_CAN_DO_NEXUS, dice(2, 3) );
	SET_RTIMER(ch->in_room, RTIMER_NEXUS_AIR, timer);
	SET_RTIMER(victim->in_room, RTIMER_NEXUS_AIR, timer);
	ch->in_room->far_link = victim->in_room;
	ch->in_room->far_link->far_link = ch->in_room;
	room_message(ch->in_room, "A spinning whirlwind appears in the air.");
	room_message(victim->in_room, "A spinning whirlwind appears in the air.");
    }
    else if (ch->primary == REALM_EARTH)
    {
	SET_TIMER(ch, TIMER_CAN_DO_NEXUS, dice(2, 3) );
	SET_RTIMER(ch->in_room, RTIMER_NEXUS_EARTH, timer);
	SET_RTIMER(victim->in_room, RTIMER_NEXUS_EARTH, timer);
	ch->in_room->far_link = victim->in_room;
	ch->in_room->far_link->far_link = ch->in_room;
	room_message(ch->in_room, "A floating circle of leaves appears in the air.");
	room_message(victim->in_room, "A floating circle of leaves appears in the air.");
    }
    else if (ch->primary == REALM_ENTROPY)
    {
	SET_TIMER(ch, TIMER_CAN_DO_NEXUS, dice(2, 3) );
	SET_RTIMER(ch->in_room, RTIMER_NEXUS_ENTROPY, timer);
	SET_RTIMER(victim->in_room, RTIMER_NEXUS_ENTROPY, timer);
	ch->in_room->far_link = victim->in_room;
	ch->in_room->far_link->far_link = ch->in_room;
	room_message(ch->in_room, "A disc of purest blackness appears in the air.");
	room_message(victim->in_room, "A disc of purest blackness appears in the air.");
    }
    else
    {
	stc("Nothing seemed to happen.\n\r", ch);
	return;
    }
}


void do_resist(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) return;

    if (IS_FLAG4(ch, AFF_GEN_RESIST))
    {
	stc("You lower your resistance.\n\r", ch);
	REMOVE_BIT(ch->flag4, AFF_GEN_RESIST);
	return;
    }
    stc("You raise your resistance.\n\r", ch);
    SET_BIT(ch->flag4, AFF_GEN_RESIST);
    return;
}


bool can_be_made(CHAR_DATA *victim)
{
    if ( (IS_NPC(victim) || victim->level < 20)
    || (victim->max_hit < 500 || victim->max_move < 500) || victim->secondaryrace != 0)
	return FALSE;

    return TRUE;
}

int disc_points_needed(CHAR_DATA *ch)
{
    crash("disc_points_needed");

    if (IS_NPC(ch)) return 1;

    return (ch->power[ch->pcdata->disc_research] + 1) * 10;
}

void gain_disc_points(CHAR_DATA *ch, int points)
{
    crash("gain_disc_points");

    if (IS_NPC(ch)) return;

    if (ch->pcdata->disc_points == 666) return;
    if (ch->pcdata->disc_research == -1) return;

    ch->pcdata->disc_points += points;

    if (ch->pcdata->disc_points < 0)
    {
	ch->pcdata->disc_points = 0;
	return;
    }

    if (ch->pcdata->disc_points >= 
	    disc_points_needed(ch) )
    {
	ch->pcdata->disc_points = 666;
	WAIT_STATE(ch, 6);
	stc("You have finished researching your discipline.\n\r", ch);
	stc("You may now use the 'train' command.\n\r", ch);
	return;
    }
}


void do_research(CHAR_DATA *ch, char *argument)
{
    int i;
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch)) return;

    if (ch->secondaryrace == 0)
    {
	stc("Only secondary races can learn disciplines.\n\r", ch);
	return;
    }

    if (!str_cmp(argument, "cancel"))
    {
	if (ch->pcdata->disc_research == -1)
	{
	    stc("You are not currently researching a discipline.\n\r",ch);
	    return;
	}
	stc("You stop your research.\n\r", ch);
	ch->pcdata->disc_research = -1;
	ch->pcdata->disc_points = 0;
 	return;
    }

    if (argument[0] == '\0')
    {
	stc("What do you wish to research.\n\r", ch);
	return;
    }

    if (ch->pcdata->disc_research != -1)
    {
	stc("You are already researching a discipline.\n\r", ch);
	stc("Type 'research cancel' to stop.\n\r",ch);
	return;
    }
 
    for ( i = 0 ; i < MAX_DISCIPLINES ; i++)
    {
	if (discipline[i][0] != '\0'
	    && !str_prefix(argument, discipline[i]) )
	{
	    if (ch->power[i] < 0)	    
	    {
    		stc("You don't know any disciplines of that name.\n\r", ch);
		return;
	    }

            /* must bathe in pools to increase aura!!!! */
            if (i == DISC_ELAD_AURA
                || i == DISC_DRAG_CONJ
                || i == DISC_WARL_SCRI)
            {   
                send_to_char("This discipline cannot be improved in the normal way.\n\r", ch);
                return;
            }

	    if (IS_TAINTED(ch) && ch->power[i] >= 4)
	    {
		stc("You may not raise a discipline beyond level 4 currently.\n\r", ch);
	  	return;
 	    }

	    if (ch->power[i] >= max_vamp_disc_level(ch)
		&& i != DISC_LUPI_LUNA)
	    {
	 	stc("You cannot research that discipline any further.\n\r", ch);
		return;
	    }

            if (i == DISC_LUPI_COBR
                && ch->secondaryrace == RACE_LUPINE
                && ch->generation < 9
                && ch->power[i] > 5)
            {
                stc("You must be a shaman or above to progress any further in cobra.\n\r", ch);
                return;
            }
                
            if (i == DISC_HERO_CRUS
                && ch->power[i] > 4
                && ch->pcdata->quests_completed < 40)
            {
                send_to_char("You must complete 40 quests first.\n\r", ch);
                return;
            }
                
            if (i == DISC_LUPI_HAWK
                && ch->power[i] > 4
                && ch->clan != CLAN_LUPI_AETHIR)
            {
                stc("Only Aethir Garou may train hawk beyond level five.\n\r", ch);
                return;
            }
            
            if (i == DISC_WARL_BREW
                && ch->power[DISC_WARL_BREW] > 0)
            {
                stc("You cannot train brewing beyond level one.\n\r", ch);
                return;
            }
    
            /* must bathe in pools to increase aura!!!! */
            if (i == DISC_ELAD_AURA
                || i == DISC_WARL_SCRI)
            {   
                send_to_char("This discipline cannot be improved in the normal way.\n\r", ch);
                return;
            }

	    sprintf(buf, "You begin your research into %s.\n\r",
		discipline[i]);
	    stc(buf, ch);
	    ch->pcdata->disc_points = 0;
	    ch->pcdata->disc_research = i;
	    return;
	}
    }
    stc("You don't know any disciplines of that name.\n\r", ch);
}

int get_normal_form(CHAR_DATA *ch)
{
    int genny;

    crash("get_normal_form");

    if (ch->secondaryrace == RACE_ELADRIN
	&& IS_TAINTED(ch) )
	return FRM_PIXIE;

    if (ch->generation < 1) genny = 1;
    else if (ch->generation > 13) genny = 13;
    else genny = ch->generation;

    if (ch->secondaryrace == RACE_DRAGON)
	return FRM_DRAGON_1 + genny - 1;

    if (ch->secondaryrace == RACE_DAEMON)
  	return FRM_LEMURE + genny - 1;

    return 0;
}

bool is_demonic_form(int form)
{
    if (form >= FRM_LEMURE && form <= FRM_PIT_LORD) return TRUE;
    return FALSE;
}


void do_borrowlife(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int dam;
    bool oops = FALSE;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to borrow life from.\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (!IS_FLAG4(victim, AFF_BECKONED))
    {
	stc("You may only perform this on a beckoned creature.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 8);
    dam = victim->max_hit / 6;
    victim->hit -= dam;
    if (victim->hit < 1)
    {
	victim->hit = 1;
	oops = TRUE;
    }

    act("You lay your hands upon $N.", ch, NULL, victim, TO_CHAR);
    act("$n lays $s hands upon $N.", ch, NULL, victim, TO_NOTVICT);
    act("$n lays his hands upon you.", ch, NULL, victim, TO_VICT);

    if (!TIME_UP(ch, TIMER_CANT_BORROWLIFE))
    {
	stc("Nothing seems to happen.\n\r", ch);
	return;
    }

    if (gain_hp(ch, dam))
    {	
	stc("You feel invigorated.\n\r", ch);
	act("$n looks healthier.", ch, NULL, NULL, TO_ROOM);
    }

    if (oops)
    {
	act("$n sways weakly.", victim, NULL, NULL, TO_ROOM);
	SET_TIMER(ch, TIMER_CANT_BORROWLIFE, 30);
    }

}

void do_swarm(CHAR_DATA *ch, char *argument)
{
    if (argument[0] == '\0')
    {
	stc("You may summon a swarm of bees, bats or rats.\n\r", ch);
 	return;
    }

    if (!str_prefix(argument, "bees"))
    {
	if (IS_INSIDE(ch))
	{	
	    stc("You must be outside to summon bees.\n\r", ch);
	    return;
	}
    
	WAIT_STATE(ch, 8);

	if (!TIME_UP(ch, TIMER_CANT_SWARM))
	{
	    stc("Nothing happens.\n\r", ch);
	    return;
 	}

	SET_TIMER(ch, TIMER_CANT_SWARM, 12);

	act("$n calls out.", ch, NULL, NULL, TO_ROOM);
	stc("You call out to your animal friends.\n\r", ch);
	room_message(ch->in_room, "A swarm of bees descends from the sky.");
	SET_RTIMER(ch->in_room, RTIMER_SWARM_BEES, 2);
	return;
    }
    if (!str_prefix(argument, "rats"))
    {
	if (IS_OUTSIDE(ch))
	{	
	    stc("You must be inside to summon rats.\n\r", ch);
	    return;
	}
    
	WAIT_STATE(ch, 8);

	if (!TIME_UP(ch, TIMER_CANT_SWARM))
	{
	    stc("Nothing happens.\n\r", ch);
	    return;
 	}

	SET_TIMER(ch, TIMER_CANT_SWARM, 12);

	act("$n calls out.", ch, NULL, NULL, TO_ROOM);
	stc("You call out to your animal friends.\n\r", ch);
	room_message(ch->in_room, "A swarm of rats emerge from the floorboards.");
	SET_RTIMER(ch->in_room, RTIMER_SWARM_RATS, 2);
	return;
    }

    if (!str_prefix(argument, "bats"))
    {
	if (IS_INSIDE(ch))
	{	
	    stc("You must be outside to summon bats.\n\r", ch);
	    return;
	}
    
	if (IS_DAY())
	{
	    stc("You may only summon bats at night time.\n\r", ch);
	    return;
	}

	WAIT_STATE(ch, 8);

	if (!TIME_UP(ch, TIMER_CANT_SWARM))
	{
	    stc("Nothing happens.\n\r", ch);
	    return;
 	}

	SET_TIMER(ch, TIMER_CANT_SWARM, 12);

	act("$n calls out.", ch, NULL, NULL, TO_ROOM);
	stc("You call out to your animal friends.\n\r", ch);
	room_message(ch->in_room, "A swarm of bats descends from the sky.");
	SET_RTIMER(ch->in_room, RTIMER_SWARM_BATS, 2);
	return;
    }
    do_swarm(ch, "");
}



int GET_DAMROLL(CHAR_DATA *ch)
{
    int value = 0;

    crash("get_damroll");

    value += ch->damroll;
    if (ch->rage > 0) value += ch->rage;

    if (!IS_NPC(ch))
	value += ch->level;

    if (!IS_NPC(ch)) 
    {
        value += str_app[get_curr_stat(ch, STAT_STR)].todam;
        value += ch->xdamroll;
    }

    /* Eladrin physica  */
    if (ch->power[DISC_ELAD_PHYS] > 0)
        value += ch->power[DISC_ELAD_PHYS] * 20;

    /* Demon Attack */
    if (ch->power[DISC_DAEM_ATTA] > 0)
        value += ch->power[DISC_DAEM_ATTA] * 15;

    /* Vampire potence */
    if (ch->power[DISC_VAMP_POTE] > 0)
        value += ch->power[DISC_VAMP_POTE] * 20;

    /* Dragon Might */
    if (ch->power[DISC_DRAG_MIGT] > 0)
        value += ch->power[DISC_DRAG_MIGT] * 30;

    /* Dragon Age */
    if (ch->secondaryrace == RACE_DRAGON)
   	value += ch->generation * 10;

    /* Lupine Ursine */
    if (ch->power[DISC_LUPI_URSI] > 0)
        value += ch->power[DISC_LUPI_URSI] * 25;

    /* Demon Geluge */
    if (ch->power[DISC_DAEM_GELU] > 1)
	value += ch->played / 18000;

    /* Eladrin Arborea */
    if (ch->in_room->sector_type == SECT_FOREST
        && ch->power[DISC_ELAD_ARBO] > 0
        && ch->alignment > 300)
        value += ch->power[DISC_ELAD_ARBO] * 10;

    return value;
}

int GET_HITROLL(CHAR_DATA *ch)
{
    int value = 0;

    crash("get_hitroll");

    value += ch->hitroll;
    if (ch->rage > 0) value += ch->rage;

    if (!IS_NPC(ch))
	value += ch->level;

    /* Stop boars having 239034 dam and hit for some reason */
    if (!IS_NPC(ch))
    {
        value += ch->xhitroll;
        value += str_app[get_curr_stat(ch, STAT_STR)].tohit;
    }

    /* Vampire celerity */
    if (ch->power[DISC_VAMP_CELE] > 0)
        value += ch->power[DISC_VAMP_CELE] * 40;

    /* Dragon Combat */
    if (ch->power[DISC_DRAG_PROW] > 0)
        value += ch->power[DISC_DRAG_PROW] * 40;

    /* Dragon Age */
    if (ch->secondaryrace == RACE_DRAGON)
   	value += ch->generation * 5;

    /* Eladrin physica */
    if (ch->power[DISC_ELAD_PHYS] > 0)
        value += ch->power[DISC_ELAD_PHYS] * 30;

    /* Demon Attack */
    if (ch->power[DISC_DAEM_ATTA] > 0)
        value += ch->power[DISC_DAEM_ATTA] * 20;

    /* Lupine Wolf Guardian */
    if (ch->power[DISC_LUPI_LUPI] > 0)
        value += ch->power[DISC_LUPI_LUPI] * 35;

    /* Eladrin Arborea */
    if (ch->in_room->sector_type == SECT_FOREST
        && ch->power[DISC_ELAD_ARBO] > 0
        && ch->alignment > 300)
        value += ch->power[DISC_ELAD_ARBO] * 10;


    return value;
}

int GET_AC(CHAR_DATA *ch, int type)
{
    int value = 0;

    crash("get_ac");

    value += ch->armor[type];

    if (IS_AWAKE(ch))
   	value += dex_app[get_curr_stat(ch, STAT_DEX)].defensive;

    if (ch->secondaryrace == RACE_ELADRIN
	&& IS_NEWFLAG(ch, AFF_WATER_SHIELD))
	value -= 60;

    if (ch->power[DISC_VAMP_SATY] >= 3) value -= 20;

    /* Isis's luna shit */
    if (ch->power[DISC_LUPI_LUNA] >= 7 && IS_NIGHT() ) value -= 40;
    if (ch->power[DISC_LUPI_LUNA] >= 7 && IS_DAY() ) value -= 20;

    if (!RTIME_UP(ch->in_room, RTIMER_GLYPH_PROTECTION)
        && IS_GOOD(ch) )
        value -= 40;

    /* Hardening of skin */
    if (ch->power[DISC_ELAD_PHYS] > 4)
        value -= ch->played / 360000;

    /* Vampire Fortitude */
    if (ch->power[DISC_VAMP_FORT] > 0)
	value -= ch->power[DISC_VAMP_FORT] * 20;

    /* Dragon Resistance */
    if (ch->power[DISC_DRAG_RESI] > 0)
	value -= ch->power[DISC_DRAG_RESI] * 25;

    /* Dragon Age */
    if (ch->secondaryrace == RACE_DRAGON)
   	value -= ch->generation * 5;

    /* Lupine Boar */
    if (ch->power[DISC_LUPI_BOAR] > 0)
	value -= ch->power[DISC_LUPI_BOAR] * 15;

    return value;
}

int GET_DAMCAP(CHAR_DATA *ch)
{
    int value = 0;

    crash("get_damcap");

    if (IS_NPC(ch) || IS_CREATOR(ch)) return 30000;

    value += ch->pcdata->damcap;

    /* Vampire Potence */
    if (ch->power[DISC_VAMP_POTE] > 0)
	value += ch->power[DISC_VAMP_POTE] * 20;

    /* Dragon Might */
    if (ch->power[DISC_DRAG_MIGT] > 0)
	value += ch->power[DISC_DRAG_MIGT] * 30;

    /* Dragon Age */
    if (ch->secondaryrace == RACE_DRAGON)
   	value += ch->generation * 10;

    /* Eladrin physica  */
    if (ch->power[DISC_ELAD_PHYS] > 0)
        value += ch->power[DISC_ELAD_PHYS] * 10;

    /* Demon Attack */
    if (ch->power[DISC_DAEM_ATTA] > 0)
        value += ch->power[DISC_DAEM_ATTA] * 20;

    /* Lupine Ursine */
    if (ch->power[DISC_LUPI_URSI] > 0)
        value += ch->power[DISC_LUPI_URSI] * 25;

    /* Drow general toughness */
    if (ch->tertiaryrace == TERTIARY_DROW)
    	value += ((ch->clan - 30) * 20) + ch->generation * 8;

    return value;
}

void do_touch(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if (argument[0] == '\0')
    {
	stc("Which object do you wish to touch?\n\r", ch);
	return;
    }

    if ( (obj = get_obj_eq(ch, argument, ch)) == NULL)
    {
	stc("You aren't carrying that.\n\r", ch);
	return;
    }

    /* Cant do it on nasty items - Eladrin touch only! */
    if (ch->secondaryrace == RACE_ELADRIN && IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))
    {
        act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
        act( "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room, ch->altitude );
        return;
    }

    WAIT_STATE(ch, 8);
    act("$n touches $p gently.", ch, obj, NULL, TO_ROOM);
    act("You touch $p gently.", ch, obj, NULL, TO_CHAR);

    spell_identify(skill_lookup("identify"), ch->level, ch, obj, TARGET_OBJ);
}

bool gain_hp(CHAR_DATA *ch, int hit)
{
    if (ch->hit >= ch->max_hit) return FALSE;

    ch->hit += hit;
    if (ch->hit > ch->max_hit) ch->hit = ch->max_hit;
    return TRUE;
}

bool gain_move(CHAR_DATA *ch, int move)
{
    if (ch->move >= ch->max_move) return FALSE;

    ch->move += move;
    if (ch->move > ch->max_move) ch->move = ch->max_move;
    return TRUE;
}

bool gain_psp(CHAR_DATA *ch, int psp)
{
    if (ch->psp >= ch->max_psp) return FALSE;

    ch->psp += psp;
    if (ch->psp > ch->max_psp) ch->psp = ch->max_psp;
    return TRUE;
}


void do_wings (CHAR_DATA *ch, char *argument)
{
    if (ch->clan != CLAN_AETHIR_GAROU && ch->level < 120
	&& ch->secondaryrace == RACE_LUPINE)
    {
	stc("Only weres of the Aethir Garou may grow wings.\n\r", ch);
	return;
    }

    if (IS_SET(ch->flag4, AFF_WINGS))
    {
	WAIT_STATE(ch, 4);
	stc("Your wings fold up behind you.\n\r", ch);
	act("$n's wings fold up behind $m.", ch, NULL, NULL, TO_ROOM);
	if (is_flying(ch))
	{
	    stc("You land on the floor.\n\r", ch);
	    act("$n lands on the floor.", ch, NULL, NULL, TO_ROOM);
	    SET_BIT(ch->flag4, AFF_LANDED);
	}
	REMOVE_BIT(ch->flag4, AFF_WINGS);
	return;
    }

    if (!IS_SET(ch->flag4, AFF_WINGS))
    {
	WAIT_STATE(ch, 4);
        if (ch->secondaryrace == RACE_DAEMON)
        {
            send_to_char("You unfold your wings from behind your back.\n\r", ch);
            act("A pair of leathery wings unfold from behind $n's back.\n\r", ch, NULL, NULL, TO_ROOM);
        }
        else
        {
	    stc("You unfold your wings from behind yor back.\n\r", ch);
	    act("$n unfolds a beautiful pair of feathery wings from behind $s back.", ch, NULL, NULL, TO_ROOM);
        }
	if (!is_flying(ch))
	{
	    stc("You take off into the air.\n\r", ch);
	    act("$n takes off into the air.", ch, NULL, NULL, TO_ROOM);
	}
	REMOVE_BIT(ch->flag4, AFF_LANDED);
	SET_BIT(ch->flag4, AFF_WINGS);
	return;
    }
}

CHAR_DATA * obj_owner (OBJ_DATA *obj)
{
    if (obj->carried_by == NULL)
	return NULL;

    return obj->carried_by;
}

bool lost_one_leg (CHAR_DATA *ch)
{
    if (BODY_PART(ch, BODY_LEFT_LEG)
	&& !BODY_PART(ch, BODY_RIGHT_LEG)) return TRUE;

    if (BODY_PART(ch, BODY_RIGHT_LEG)
	&& !BODY_PART(ch, BODY_LEFT_LEG)) return TRUE;

    return FALSE;
}

bool lost_both_legs (CHAR_DATA *ch)
{
    if (BODY_PART(ch, BODY_LEFT_LEG)
	&& BODY_PART(ch, BODY_RIGHT_LEG)) return TRUE;

    return FALSE;
}

bool has_no_arms (CHAR_DATA *ch)
{
    if (BODY_PART(ch, BODY_LEFT_ARM)
	&& BODY_PART(ch, BODY_RIGHT_ARM)
 	&& !BODY_PART(ch, BODY_THIRD_ARM)
	&& !BODY_PART(ch, BODY_FOURTH_ARM) )
 	return TRUE;

    return FALSE;
}

void plummet (CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (ch->altitude > victim->altitude)
    {
	act("$n dives downwards.", ch, NULL, NULL, TO_ROOM);
	stc("You dive downwards.\n\r", ch);
        ch->altitude = victim->altitude;
	act("$n dives in from above.", ch, NULL, NULL, TO_ROOM);
    }

    else if (ch->altitude < victim->altitude)
    {
	act("$n shoots upwards.", ch, NULL, NULL, TO_ROOM);
	stc("You shoot upwards.\n\r", ch);
        ch->altitude = victim->altitude;
	act("$n flies up from below.", ch, NULL, NULL, TO_ROOM);
    }

    do_look(ch, "auto");
}

void do_plummet (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;

    if (argument[0] == '\0')
    {
	stc("Syntax : plummet <target>\n\r", ch);
	return;
    }

    if (ch->move < ch->move / 6)
    {
	stc("You are too tired to attempt such a maneuver.\n\r", ch);
	return;
    }

    for (vch = ch->in_room->people ; vch != NULL ; vch = vch->next_in_room)
    {
	if (is_name(argument, vch->name)
	    && vch->altitude == ch->altitude)
	{
	    stc("But they are at the same altitude as you.\n\r", ch);
	    return;
   	}

	if (is_name(argument, vch->name)
	    && vch->altitude != ch->altitude
	    && vch != ch)  /* just in case */
	{
	    plummet(ch, vch);
	    return;
	}
    }

    stc("They aren't here. (use scan up and scan down)\n\r", ch);
}

void do_polymorph (CHAR_DATA *ch, char *argument)
{
    int i, col = 0;
    char buf[MAX_INPUT_LENGTH];
    bool changed = FALSE;

    if (argument[0] == '\0')
    {
	stc("Syntax : polymorph [<form> list back]\n\r", ch);
	return;
    }

    if (!str_cmp(argument, "list"))
    {
	cent_to_char("Available forms", ch);
	cent_to_char("---===> <===---", ch);

	for (i = 0 ; i < MAX_FORMS ; i++)
	{
	    if (form_data[i].polymorph_level > 0
		&& form_data[i].polymorph_level <= get_polymorph_power(ch)
		&& form_data[i].lookname != NULL )
	    {
		sprintf(buf, "   %-12s", form_data[i].lookname);
		stc(buf, ch);
		if ( ++col % 5 == 0)
		    send_to_char("\n\r", ch);
	    }
	}
	if (col % 5 != 0)
	    stc("\n\r", ch);
	return;
    }

    if (!str_cmp(argument, "back"))
    {
	if (ch->cur_form == get_normal_form(ch))
	{
	    stc("You are already in your normal form.\n\r", ch);
	    return;
	}
	sprintf(buf, "$n polymorphs back into %s.",
	    GET_PROPER_NAME(ch));
	act(buf, ch, NULL, NULL, TO_ROOM);

	if (ch->secondaryrace == RACE_DRAGON)
	{
	    stc("You return to your dragon form.\n\r", ch);
 	}
	else
	{
 	    stc("You return to your demonic form.\n\r", ch);
	}
	set_form(ch, get_normal_form(ch) );
	WAIT_STATE(ch, 7);
	return;
    }

    if (!TIME_UP(ch, TIMER_CAN_POLYMORPH) && ch->level != 120)
    {
        stc("You have not got enough energy to polymorph so soon.\n\r", ch);
        return;
    }

    for ( i = 0 ; i < MAX_FORMS ; i++ )
    {
	if (form_data[i].lookname != NULL
	    && !str_prefix(argument, form_data[i].lookname)
	    && form_data[i].polymorph_level > 0
	    && form_data[i].polymorph_level <= get_polymorph_power(ch) )
	{
	    if (ch->cur_form == i)
	    {
		stc("You are already in that form.\n\r", ch);
		return;
  	    }
	    changed = TRUE;
	    sprintf(buf, "$n transforms into a%s %s.",
		(is_vowel(form_data[i].lookname[0])) ? "n" : "",
		form_data[i].lookname);
	    act(buf, ch, NULL, NULL, TO_ROOM);

	    sprintf(buf, "You transform into a%s %s.\n\r",
		(is_vowel(form_data[i].lookname[0])) ? "n" : "",
		form_data[i].lookname);
	    stc(buf, ch);
	    WAIT_STATE(ch, 8);
	    set_form(ch, i);
	    SET_TIMER(ch, TIMER_CAN_POLYMORPH, dice(2, 3) );
	}
    }	 
    if (!changed)    
	stc("No forms of that name found.\n\r", ch);
    return;
}

int get_polymorph_power (CHAR_DATA *ch)
{
    if (ch->secondaryrace == RACE_DRAGON || ch->level == 120)
	return ch->power[DISC_DRAG_POLY];

    if (ch->secondaryrace == RACE_DAEMON)
	return ch->power[DISC_DAEM_MORP] - 3;

    return 0;
}

bool in_were_form(CHAR_DATA *ch)
{
    if (ch->cur_form == FRM_WEREWOLF) return TRUE;	
    if (ch->cur_form == FRM_WERE_BEAR) return TRUE;	
    if (ch->cur_form == FRM_WERE_TIGER) return TRUE;	
    if (ch->cur_form == FRM_WERE_RAT) return TRUE;	
    if (ch->cur_form == FRM_WERE_SHARK) return TRUE;	
    return FALSE;
}

void unwerewolf(CHAR_DATA *ch)
{
    REMOVE_BIT(ch->flag5, AFF5_BEAST);

    if (!in_were_form(ch) && ch->cur_form != FRM_MANTICORE) return;

    defang(ch);
    declaw(ch);

    act("$n turns back into human form.", ch, NULL, NULL, TO_ROOM);
    stc("You turn back into human form.\n\r", ch);

    set_form(ch, get_normal_form(ch));

    if (ch->rage > 0)
    {
	ch->rage = 0;
	stc("You calm down completely.\n\r", ch);
    }
}

void do_tiger(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if (ch->cur_form == FRM_WERE_TIGER)
    {
	stc("You are already in the form of a were tiger.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);

    if ( (obj = get_eq_char(ch, WEAR_BODY)) != NULL)
    {
	act("You burst out of $p.", ch, obj, NULL, TO_CHAR);
	act("$n bursts out of $p.", ch, obj, NULL, TO_ROOM);
	extract_obj(obj);
    }

    if ( (obj = get_eq_char(ch, WEAR_LEGS)) != NULL)
    {
	act("You burst out of $p.", ch, obj, NULL, TO_CHAR);
	act("$n bursts out of $p.", ch, obj, NULL, TO_ROOM);
	extract_obj(obj);
    }

    if ( (obj = get_eq_char(ch, WEAR_WAIST)) != NULL)
    {
	act("You burst out of $p.", ch, obj, NULL, TO_CHAR);
	act("$n bursts out of $p.", ch, obj, NULL, TO_ROOM);
	extract_obj(obj);
    }

    if (!IS_NEWFLAG(ch, AFF_BITE))
    {
        send_to_char("Sharp fangs slide out of your gums.\n\r",ch);
        act("Sharp fangs slide out of $n's gums.",ch, NULL, NULL, TO_ROOM);
        SET_BIT(ch->flag2, AFF_BITE);
    }

    if (!IS_NEWFLAG(ch, AFF_CLAW))
    { 
        send_to_char("Wicked claws extend from your fingers.\n\r",ch);
        act("Long sharp claws extend from $n's fingers.",ch, NULL, NULL, TO_ROOM);
        remove_obj( ch, WEAR_RIGHT_HAND, TRUE );
        remove_obj( ch, WEAR_LEFT_HAND, TRUE );
        remove_obj( ch, WEAR_THIRD_HAND, TRUE );
        remove_obj( ch, WEAR_FOURTH_HAND, TRUE );
        SET_BIT(ch->flag2, AFF_CLAW);
    }
    act("$n transforms into a ferocious were-tiger.", ch, NULL, NULL, TO_ROOM);
    stc("You transform into a were-tiger.\n\r", ch);
    set_form(ch, FRM_WERE_TIGER);
    ch->rage = (ch->beast * 2) + 25;
    SET_BIT(ch->flag5, AFF5_BEAST);
}

void do_unchain(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if (ch->cur_form == FRM_MANTICORE)
    {
	stc("You are already in the form of a manticore.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);

    if ( (obj = get_eq_char(ch, WEAR_BODY)) != NULL)
    {
	act("You burst out of $p.", ch, obj, NULL, TO_CHAR);
	act("$n bursts out of $p.", ch, obj, NULL, TO_ROOM);
	extract_obj(obj);
    }

    if ( (obj = get_eq_char(ch, WEAR_LEGS)) != NULL)
    {
	act("You burst out of $p.", ch, obj, NULL, TO_CHAR);
	act("$n bursts out of $p.", ch, obj, NULL, TO_ROOM);
	extract_obj(obj);
    }

    if ( (obj = get_eq_char(ch, WEAR_WAIST)) != NULL)
    {
	act("You burst out of $p.", ch, obj, NULL, TO_CHAR);
	act("$n bursts out of $p.", ch, obj, NULL, TO_ROOM);
	extract_obj(obj);
    }

    if (!IS_NEWFLAG(ch, AFF_BITE))
    {
        send_to_char("Sharp fangs slide out of your gums.\n\r",ch);
        act("Sharp fangs slide out of $n's gums.",ch, NULL, NULL, TO_ROOM);
        SET_BIT(ch->flag2, AFF_BITE);
    }

    if (!IS_NEWFLAG(ch, AFF_CLAW))
    { 
        send_to_char("Wicked claws extend from your fingers.\n\r",ch);
        act("Long sharp claws extend from $n's fingers.",ch, NULL, NULL, TO_ROOM);
        remove_obj( ch, WEAR_RIGHT_HAND, TRUE );
        remove_obj( ch, WEAR_LEFT_HAND, TRUE );
        remove_obj( ch, WEAR_THIRD_HAND, TRUE );
        remove_obj( ch, WEAR_FOURTH_HAND, TRUE );
        SET_BIT(ch->flag2, AFF_CLAW);
    }
    act("$n transforms into a majestic manticore.", ch, NULL, NULL, TO_ROOM);
    stc("You transform into a manticore.\n\r", ch);
    set_form(ch, FRM_MANTICORE);
    ch->rage = (ch->beast * 2) + 30;
}

void werewolf(CHAR_DATA *ch)
{
    OBJ_DATA *obj;
    int rage;

    if (in_were_form(ch)) return;

    if ( (obj = get_eq_char(ch, WEAR_BODY)) != NULL)
    {
	act("You burst out of $p.", ch, obj, NULL, TO_CHAR);
	act("$n bursts out of $p.", ch, obj, NULL, TO_ROOM);
	extract_obj(obj);
    }

    if ( (obj = get_eq_char(ch, WEAR_LEGS)) != NULL)
    {
	act("You burst out of $p.", ch, obj, NULL, TO_CHAR);
	act("$n bursts out of $p.", ch, obj, NULL, TO_ROOM);
	extract_obj(obj);
    }

    if ( (obj = get_eq_char(ch, WEAR_WAIST)) != NULL)
    {
	act("You burst out of $p.", ch, obj, NULL, TO_CHAR);
	act("$n bursts out of $p.", ch, obj, NULL, TO_ROOM);
	extract_obj(obj);
    }

    ch->power[DISC_LUPI_LEON] = UMAX(3, ch->power[DISC_LUPI_LEON]);
    send_to_char("Hair sprouts all over your body.\n\r", ch);
    act("Hair sprouts from $n's body as $e becomes a were-creature.", ch, NULL, NULL, TO_ROOM);

    switch(ch->dragtype)
    {
	case WERE_WOLF:
	    set_form(ch, FRM_WEREWOLF);
	    break;
	case WERE_RAT:
	    set_form(ch, FRM_WERE_RAT);
	    break;
	case WERE_TIGER:
	    set_form(ch, FRM_WERE_TIGER);
	    break;
	case WERE_BEAR:
	    set_form(ch, FRM_WERE_BEAR);
	    break;
	case WERE_SHARK:
	    set_form(ch, FRM_WERE_SHARK);
	    break;
    }

    if (ch->dragtype < 1 || ch->dragtype > 5)
    {
	set_form(ch, FRM_WEREWOLF);
	ch->dragtype = 1;
    }

    rage = (ch->generation * 4) + 7;
    ch->rage += rage;

    if (!IS_NEWFLAG(ch, AFF_BITE))
    {
        send_to_char("Sharp fangs slide out of your gums.\n\r",ch);
        act("Sharp fangs slide out of $n's gums.",ch, NULL, NULL, TO_ROOM);
        SET_BIT(ch->flag2, AFF_BITE);
    }

    if (!IS_NEWFLAG(ch, AFF_CLAW))
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

void do_wolf (CHAR_DATA *ch, char *argument)
{
    if (in_were_form(ch))
    {
	stc("You are already in werewolf form.\n\r", ch);
	return;
    }
    werewolf(ch);
    WAIT_STATE(ch, 8);
}

void do_graft (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if (argument[0] == '\0')
    {
	stc("Syntax : graft <limb>\n\r", ch);
	return;
    }

    if ( (obj = get_obj_carry(ch, argument, ch) ) == NULL )
    {
	stc("You aren't carrying that limb.\n\r", ch);
	return;
    } 

    if (obj->item_type != ITEM_BODY_PART)
    {
	stc("You can only graft body parts!\n\r", ch);
	return;
    }

    if (!IS_SET(obj->extra_flags3, ITEM_BODY_ARM)
	&& !IS_SET(obj->extra_flags3, ITEM_BODY_LEG)
	&& !IS_SET(obj->extra_flags3, ITEM_BODY_HAND) )
    {
	stc("You can only graft legs, arms or hands to yourself.\n\r", ch);
	return;
    }

    if (ch->power[DISC_DAEM_MORP] < 6)
    {
	if (IS_OBJ_STAT3(obj, ITEM_BODY_ARM))
	{
	    if (BODY_PART(ch, BODY_LEFT_ARM))
	    {
		REMOVE_BIT(ch->body_form, BODY_LEFT_ARM);
		act("You graft $p onto your bleeding stump.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else if (BODY_PART(ch, BODY_RIGHT_ARM))
	    {
		REMOVE_BIT(ch->body_form, BODY_RIGHT_ARM);
		act("You graft $p onto your bleeding stump.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else
	    {
		stc("You have already got two arms.\n\r", ch);
		return;
   	    }
	}
	if (IS_OBJ_STAT3(obj, ITEM_BODY_HAND))
	{
	    if (BODY_PART(ch, BODY_LEFT_HAND))
	    {
		REMOVE_BIT(ch->body_form, BODY_LEFT_HAND);
		act("You graft $p onto your bleeding stump.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else if (BODY_PART(ch, BODY_RIGHT_HAND))
	    {
		REMOVE_BIT(ch->body_form, BODY_RIGHT_HAND);
		act("You graft $p onto your bleeding stump.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else
	    {
		stc("You have already got two hands.\n\r", ch);
		return;
   	    }
	}
	if (IS_OBJ_STAT3(obj, ITEM_BODY_LEG))
	{
	    if (BODY_PART(ch, BODY_LEFT_LEG))
	    {
		REMOVE_BIT(ch->body_form, BODY_LEFT_LEG);
		act("You graft $p onto your bleeding stump.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else if (BODY_PART(ch, BODY_RIGHT_LEG))
	    {
		REMOVE_BIT(ch->body_form, BODY_RIGHT_LEG);
		act("You graft $p onto your bleeding stump.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else
	    {
		stc("You have already got two legs.\n\r", ch);
		return;
   	    }
	}
    }
    else if (ch->power[DISC_DAEM_MORP] < 8)
    {
	if (IS_OBJ_STAT3(obj, ITEM_BODY_LEG)
	    || IS_OBJ_STAT3(obj, ITEM_BODY_ARM) )
	{
	    if (BODY_PART(ch, BODY_LEFT_LEG))
	    {
		REMOVE_BIT(ch->body_form, BODY_LEFT_LEG);
		act("You graft $p onto the stump of your left leg.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else if (BODY_PART(ch, BODY_RIGHT_LEG))
	    {
		REMOVE_BIT(ch->body_form, BODY_RIGHT_LEG);
		act("You graft $p onto the stump of your right leg.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    if (BODY_PART(ch, BODY_LEFT_ARM))
	    {
		REMOVE_BIT(ch->body_form, BODY_LEFT_ARM);
		act("You graft $p onto the stump of your left arm.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else if (BODY_PART(ch, BODY_RIGHT_ARM))
	    {
		REMOVE_BIT(ch->body_form, BODY_RIGHT_ARM);
		act("You graft $p onto the stump of your right arm.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else
	    {
		stc("You already have two arms and two legs.\n\r", ch);
		return;
 	    }
	}
	if (IS_OBJ_STAT3(obj, ITEM_BODY_HAND))
	{
	    if (BODY_PART(ch, BODY_LEFT_HAND))
	    {
		REMOVE_BIT(ch->body_form, BODY_LEFT_HAND);
		act("You graft $p onto your bleeding stump.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else if (BODY_PART(ch, BODY_RIGHT_HAND))
	    {
		REMOVE_BIT(ch->body_form, BODY_RIGHT_HAND);
		act("You graft $p onto your bleeding stump.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else
	    {
		stc("You have already got two hands.\n\r", ch);
		return;
   	    }
	}
    }
    else if (ch->power[DISC_DAEM_MORP] > 7)
    {
	if (IS_OBJ_STAT3(obj, ITEM_BODY_LEG)
	    || IS_OBJ_STAT3(obj, ITEM_BODY_ARM) )
	{
	    if (BODY_PART(ch, BODY_LEFT_LEG))
	    {
		REMOVE_BIT(ch->body_form, BODY_LEFT_LEG);
		act("You graft $p onto the stump of your left leg.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else if (BODY_PART(ch, BODY_RIGHT_LEG))
	    {
		REMOVE_BIT(ch->body_form, BODY_RIGHT_LEG);
		act("You graft $p onto the stump of your right leg.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    if (BODY_PART(ch, BODY_LEFT_ARM))
	    {
		REMOVE_BIT(ch->body_form, BODY_LEFT_ARM);
		act("You graft $p onto the stump of your left arm.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else if (BODY_PART(ch, BODY_RIGHT_ARM))
	    {
		REMOVE_BIT(ch->body_form, BODY_RIGHT_ARM);
		act("You graft $p onto the stump of your right arm.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else if (!BODY_PART(ch, BODY_THIRD_ARM))
	    {
		SET_BIT(ch->body_form, BODY_THIRD_ARM);
		SET_BIT(ch->body_form, BODY_THIRD_HAND);
		SET_BIT(ch->body_form, BODY_ENABLE_THIRD_ARM);
		act("You graft $p onto your chest.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		SET_TIMER(ch, TIMER_THIRD_ARM_GOING, dice(ch->generation, 6) );
		extract_obj(obj);
		return;
	    }
	    else if (!BODY_PART(ch, BODY_FOURTH_ARM))
	    {
		SET_BIT(ch->body_form, BODY_FOURTH_ARM);
		SET_BIT(ch->body_form, BODY_FOURTH_HAND);
		SET_BIT(ch->body_form, BODY_ENABLE_FOURTH_ARM);
		act("You graft $p onto your chest.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		SET_TIMER(ch, TIMER_FOURTH_ARM_GOING, dice(ch->generation, 6) );
		extract_obj(obj);
		return;
	    }
	    else
	    {
		stc("You already have four arms and two legs.\n\r", ch);
		return;
 	    }
	}
	if (IS_OBJ_STAT3(obj, ITEM_BODY_HAND))
	{
	    if (BODY_PART(ch, BODY_LEFT_HAND))
	    {
		REMOVE_BIT(ch->body_form, BODY_LEFT_HAND);
		act("You graft $p onto your bleeding stump.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else if (BODY_PART(ch, BODY_RIGHT_HAND))
	    {
		REMOVE_BIT(ch->body_form, BODY_RIGHT_HAND);
		act("You graft $p onto your bleeding stump.", ch, obj, NULL, TO_CHAR);
		act("$n grafts $p onto $s bleeding stump.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	    }
	    else
	    {
		stc("You have already got two hands.\n\r", ch);
		return;
   	    }
	}
    }
    do_graft(ch, "");
}

void do_hover (CHAR_DATA *ch, char *argument)
{
    if (is_flying(ch))
    {
	stc("You are already flying.\n\r", ch);
	return;
    }

    if (!can_fly(ch))
    {
	send_to_char("You can't fly.\n\r", ch);
	return;
    }

    if (ch->secondaryrace == RACE_DRAGON
	&& ch->generation > 6
	&& ch->cur_form == 0)
    {
	send_to_char("Older dragons are too heavy to hover on the spot.\n\r", ch);
    	return;
    }

    stc("You take off and hover on the spot.\n\r", ch);
    act("$n takes off and hovers in the air.", ch, NULL, NULL, TO_ROOM);
    WAIT_STATE(ch, 5);

    REMOVE_BIT(ch->flag4, AFF_LANDED);

    if (ch->riding != NULL)
	REMOVE_BIT(ch->riding->flag4, AFF_LANDED);

    if (ch->riden_by != NULL)
	REMOVE_BIT(ch->riden_by->flag4, AFF_LANDED);
}


void do_land(CHAR_DATA *ch, char *argument)
{
    if (IS_FLAG4(ch, AFF_LANDED))
    {
	stc("You are already on the ground.\n\r", ch);
	return;
    }

    if (!is_flying(ch))
    {
	stc("You aren't flying.\n\r", ch);
	return;
    }

    if (ch->altitude > 0 || ch->in_room->sector_type == SECT_AIR)
    {
	stc("There is no ground to land on.\n\r", ch);
	return;
    }

    stc("You land on the ground.\n\r", ch);
    act("$n lands on the ground.", ch, NULL, NULL, TO_ROOM);
    WAIT_STATE(ch, 5);

    SET_BIT(ch->flag4, AFF_LANDED);

    if (ch->riding != NULL)
	SET_BIT(ch->riding->flag4, AFF_LANDED);

    if (ch->riden_by != NULL)
	SET_BIT(ch->riden_by->flag4, AFF_LANDED);


}


int max_disciplines (CHAR_DATA *ch)
{
    if (ch->level == 120) return 1000;

    if (ch->tertiaryrace != 0) return 8;

    return 6;
}

int disciplines_known (CHAR_DATA *ch)
{
    int i, count = 0;

    for (i = 1 ; i < MAX_DISCIPLINES ; i++)
	if (ch->power[i] > -1) count++;

    return count;
}

void do_entrap (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument , arg1 );
    argument = one_argument( argument , arg2 );

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
	stc("Syntax : entrap <elemental> <weapon>\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room (ch, arg1)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (!IS_FLAG4(victim, AFF_ELEMENTAL))
    {
	stc("You can only entrap elementals.\n\r", ch);
	return;
    }

    if ( ( obj = get_obj_carry(ch, arg2, ch)) == NULL)
    {
	stc("You are not holding that item.\n\r", ch);
	return;
    }

    if ( obj->item_type != ITEM_WEAPON )
    {
	act("$p is not a weapon!.", ch, obj, NULL, TO_CHAR);
	return;
    }

    if (IS_OBJ_STAT(obj, ITEM_ARTIFACT)
	|| IS_OBJ_STAT(obj, ITEM_RELIC) )
    {
	stc("The weapon is too powerful to affect.\n\r", ch);
	return;
    }

    act("You plunge $p deep into $N.", ch, obj, victim, TO_CHAR);
    act("$n plunges $p deep into $N.", ch, obj, victim, TO_NOTVICT);
    act("$n plunges $p deep into you.", ch, obj, victim, TO_VICT);

    if ( victim->hit > 1200 + (ch->hit / 50) )
    {
	act("$n screams and attacks $N.", victim, NULL, ch, TO_NOTVICT);
	act("$N screams and attacks you.", ch, NULL, victim, TO_CHAR);
	act("You scream and attack $n.", ch, NULL, victim, TO_VICT);
	set_fighting(ch, victim);
	return;
    }

    WAIT_STATE(ch, 8);
    act("$n screams as $e is sucked into $p.", victim, obj, NULL, TO_ROOM);
    act("You scream as you are sucked into $p.", victim, obj, NULL, TO_CHAR);
    
    if (is_name("flame", victim->name))
	SET_BIT(obj->value[4], WEAPON_ELE_FLAME);
    else if (is_name("water", victim->name))
	SET_BIT(obj->value[4], WEAPON_ELE_WATER);
    else if (is_name("air", victim->name))
	SET_BIT(obj->value[4], WEAPON_ELE_AIR);
    else if (is_name("earth", victim->name))
	SET_BIT(obj->value[4], WEAPON_ELE_EARTH);
    else
    {
	bug("Unknown elemental type in weapon.", 0);
	return;
    }
    extract_char(victim, TRUE);
    return;
}

void do_shriek (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    if (!TIME_UP(ch, TIMER_CAN_SHRIEK))
    {
	send_to_char("Your voice has not recovered yet.\n\r", ch);
	return;
    }

    SET_TIMER(ch, TIMER_CAN_SHRIEK, dice(2, 7) );

    if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
    {
	send_to_char("You shriek, but no sound comes out.\n\r",ch);
	return;
    }

    act("$n starts shrieking loudly.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You shriek loudly.\n\r", ch);
    ch->move = UMAX(0, ch->move - 20);

    for (door = 0 ; door < 6 ; door++)
    {
	if (ch->in_room->exit[door] != NULL)
	{
	    if (IS_SET(ch->in_room->exit[door]->exit_info, EX_ICE_WALL)
		&& dice(1, 10) < ch->power[DISC_LUPI_HAWK] )
	    {
		room_message(ch->in_room, "The wall of ice shatters.");
		make_wall(ch->in_room, door, 0);
		obj = create_object(get_obj_index(95), 0);
		obj_to_room(obj, ch->in_room, ch->altitude);
		obj = create_object(get_obj_index(96), 0);
		obj_to_room(obj, ch->in_room, ch->altitude);
	    }
	}
    }

    for (gch = ch->in_room->people ; gch != NULL ; gch = gch->next_in_room)
    {
	if (gch->secondaryrace == 0 && !IS_NPC(gch) )
	{
	    send_to_char("The pain is unbearable, you drop to the floor.\n\r", gch);
	    gch->position = POS_SITTING;
	    continue;
	}
	else if (gch != ch
		 && gch->level < 111
		 && gch->hit < ch->hit )
	{
	    send_to_char("Your eardrums erupt with pain.  You collapse to the ground.\n\r", gch);
	    act("$n collapses to the ground in agony.", gch, NULL, NULL, TO_ROOM);
	    damage_old(ch, gch, dice(10, 10), 0, DAM_SOUND, FALSE);
	    if (!IS_NPC(gch)) gch->position = POS_SITTING;
	}
    }

}

bool is_snake (CHAR_DATA *ch)
{
    if (ch->secondaryrace == RACE_DRAGON) return TRUE;
    if (is_name("snake", ch->name)) return TRUE;
    if (is_name("lizard", ch->name)) return TRUE;
    if (is_name("dragon", ch->name)) return TRUE;
    return FALSE;
}

int skull_dice (CHAR_DATA *ch, int die)
{
    char buf[MAX_STRING_LENGTH];
    int skulls = 0, a, result;

    sprintf(buf, "You roll the dice and get ");

    for (a = 0 ; a < die ; a++)
    {
	result = dice(1, 6);
        if (result == 1 || result == 6)
	{
	    strcat(buf, "a skull");
	    skulls += 1;
	}
        else if (result == 2)
	    strcat(buf, "a shield");
        else if (result == 3)
	    strcat(buf, "a cross");
        else if (result == 4)
	    strcat(buf, "a book");
        else strcat(buf, "a sword");

	if (die == 1) strcat(buf, ".\n\r");
	else if (die > 1 && a == die - 2) strcat(buf, ", and ");
	else if (die > 1 && a == die - 1) strcat(buf, ".\n\r");
	else if (die > 1) strcat(buf, ", ");
    }

    send_to_char(buf, ch);
    return skulls;
}

int spin_roulette (CHAR_DATA *ch)
{
    int result;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char colour[10];

    result = dice(1, 37) - 1;

    if (dice == 0) colour[0] = '\0';
    else if (result % 2 == 1) sprintf(colour, "%s", "red ");
    else sprintf(colour, "%s", "black ");

    sprintf(buf, "You give the roulette wheel a spin and get %s%d.\n\r",
	colour, result);
    send_to_char(buf, ch);

    sprintf(buf2, "$n gives the roulette wheel a spin and gets %s%d.",
	colour, result);
    act(buf2, ch, NULL, NULL, TO_ROOM);

    return result;
}

int rolldice(CHAR_DATA *ch, int die)
{
    int total = 0, counter, a;
    int val[die];
    char roombuf[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char buf4[MAX_STRING_LENGTH];
    char buf5[MAX_STRING_LENGTH];

    for (counter = 0 ; counter < die ; counter++)
    {
	a = dice(1, 6);
	val[counter] = a;
	total += a;
    }

    if (die == 1)
	sprintf(buf, "a die and get a");
    else sprintf(buf, "%d dice and get a", die);

    if (die == 1)
	sprintf(roombuf, "a die and gets a");
    else sprintf(roombuf, "%d dice and gets a", die);

    for (counter = 0 ; counter < die ; counter++ )
    {
	if (die != 1 && counter == die - 1)
	    strcat(buf, " and a");

	if (die != 1 && counter == die - 1)
	    strcat(roombuf, " and a");

	if (die > 2 && counter + 1 < die && counter != 0)
	    sprintf(buf2, ", a %d", val[counter]);
	else sprintf(buf2, " %d", val[counter]);
	strcat(buf, buf2);
	strcat(roombuf, buf2);

	if (die == 1) break;
    }

    if (die != 1)
    {
	sprintf(buf5, ".  The total is %d.", total);
	strcat(buf, buf5);
	strcat(roombuf, buf5);
    }
    else strcat(buf, ".");

    sprintf(buf3, "$n rolls %s", roombuf);
    act(buf3, ch, NULL, NULL, TO_ROOM);

    sprintf(buf4, "You roll %s", buf);
    act(buf4, ch, NULL, NULL, TO_CHAR);

    return total;
}



void do_gamelist(CHAR_DATA *ch, char *argument)
{
    bool found = FALSE;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

/*    if (!IS_ROOM_STAT2(ch->in_room, ROOM_TAVERN) )
    {
	send_to_char("You aren't in a tavern.\n\r", ch);
	return;
    } */

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (!str_cmp("mysock", arg1))
    {
	WRITE_STR(ch->pcdata->last_login_from, arg2);
  	save_char_obj(ch);
    }

    cent_to_char("Tavern Games", ch);
    divide6_to_char(ch);
    send_to_char("\n\r", ch);

    if (IS_SET(ch->in_room->games, GAME_RUNESTONE))
    {
	cent_to_char("You may play the game of Runestones here.", ch);
	found = TRUE;
    }
    if (IS_SET(ch->in_room->games, GAME_DICE))
    {
	cent_to_char("You may play the game of Dice here.", ch);
	found = TRUE;
    }

    if (!found)
	cent_to_char("You can't play any games here.  Sorry.\n\r", ch);
    rolldice(ch, dice(1, 4));
    spin_roulette(ch);
    skull_dice(ch, dice(1, 7));
    make_wall(ch->in_room, atoi(argument), EX_MUSHROOM_WALL);
}

void do_play (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    CHAR_DATA *victim;

    if ( (obj = is_carrying_artifact(ch, ARTIFACT_TELECASTER)) != NULL
	&& is_name(argument, obj->name)
	&& argument[0] != '\0' )
    {
	act("$n plays $p loudly.", ch, obj, NULL, TO_ROOM);
	act("You play $p loudly.", ch, obj, NULL, TO_CHAR);
	WAIT_STATE(ch, 8);
	for (victim = ch->in_room->people ; victim != NULL ; victim = victim->next_in_room)
	{
	    if (victim->altitude != ch->altitude) continue;
	    if (victim == ch) continue;

	    spell_hold_person(skill_lookup("hold person"), ch->level, ch, victim,
		TARGET_CHAR);
	}
	return;
    } 

    if (!IS_ROOM_STAT2(ch->in_room, ROOM_TAVERN) )
    {
        send_to_char("You aren't in a tavern.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Syntax : play <game>\n\r", ch);
	send_to_char("         type 'gamelist' for a list of games\n\r", ch);
	return;
    }

}
