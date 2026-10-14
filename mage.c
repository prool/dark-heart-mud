/*****************************************************************************
*  Adam Powell - Mage.C    This should keep the French happy!		     *
*****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hell.h"
#include "recycle.h"
#include "magic.h"

/* Functions for this file only */
void 	check_char_function	args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void    check_exorcise_function args( (CHAR_DATA *ch, OBJ_DATA *obj) );

/* These are the SANITY routines.  Its quite easy to lose your head,
    you know what I mean.  When a characters sanity runs low, they are
    open to all attacks */

void do_sanity(CHAR_DATA *ch, char *argument)
{
    stcf(ch, "You have %d willpower points remaining, out of a possible %d.\n\r",
	get_sanity(ch), get_max_sanity(ch) );

    if (IS_RESISTING(ch))
	stc("You will try and resist all incoming attacks.\n\r", ch);

    else
    {
	stc("You are not resisting incoming attacks.\n\r", ch);
 	stc("Type 'resist' to do so.\n\r", ch);
    }
}

int sanity_regain_rate(CHAR_DATA *ch)
{
    int rate = 4;

    crash("sanity regain rate");

    if (ch->secondaryrace == RACE_VAMPIRE) rate += ch->generation;
    if (ch->power[DISC_DRAG_RESI] > 0) rate += ch->power[DISC_DRAG_RESI] + 2;
    if (ch->power[DISC_LUPI_FOX] > 0) rate += ch->power[DISC_LUPI_FOX] + 1;
    if (ch->power[DISC_ELAD_ANSO] > 0) rate += (ch->power[DISC_ELAD_ANSO] + 3);
    if (ch->power[DISC_DAEM_IMUN] > 0) rate += (ch->power[DISC_DAEM_IMUN]);
    if (ch->power[DISC_WARL_WILL] > 0) rate += (ch->power[DISC_WARL_WILL] * 2);

    return rate;
}

/* This procedure is called every tick.  :) */
void regain_sanity(CHAR_DATA *ch)
{
    if (IS_NPC(ch)) return;

    crash("actually regaining the sanity");

    ch->pcdata->sanity += sanity_regain_rate(ch);
    if (ch->pcdata->sanity >= get_max_sanity(ch))
	ch->pcdata->sanity = get_max_sanity(ch);
}

int get_max_sanity(CHAR_DATA *ch)
{
    int max = 100;

    crash("getting the max sanity");

    if (IS_NPC(ch)) return 0;

    if (ch->secondaryrace == RACE_VAMPIRE)
        max += (ch->generation * 10);

    if (ch->power[DISC_DRAG_RESI] > 0) max += (ch->power[DISC_DRAG_RESI] * 10);
    if (ch->power[DISC_ELAD_ANSO] > 0) max += (ch->power[DISC_ELAD_ANSO] * 15);
    if (ch->power[DISC_LUPI_FOX] > 0) max += (ch->power[DISC_LUPI_FOX] * 12);
    if (ch->power[DISC_DAEM_IMUN] > 0) max += (ch->power[DISC_DAEM_IMUN] * 7);
    if (ch->power[DISC_WARL_WILL] > 0) max += (ch->power[DISC_WARL_WILL] * 17);

    return max; /* For now! */
}

int get_sanity(CHAR_DATA *ch)
{
    if (IS_NPC(ch)) return 0;

    crash("getting the sanity");

    /* Bit of a sanity check here.... ho ho ho */
    if (ch->pcdata->sanity > get_max_sanity(ch))
	ch->pcdata->sanity = get_max_sanity(ch);

    return ch->pcdata->sanity;
}

bool sanity_resist(CHAR_DATA *ch, CHAR_DATA *victim, int damage)
{
    if (IS_NPC(victim) || IS_NPC(ch) ) return FALSE;

    crash("sanity resist");

    /* If they dont have their defences up, they can still lose their
	innate resistance points */
    if (!IS_RESISTING(victim))
    {
	victim->pcdata->sanity--;
	if (victim->pcdata->sanity <= 0) victim->pcdata->sanity = 0;
	stc("You feel your undefended mind being invaded.\n\r", victim);
	return FALSE;
    }

    /* Warlock willpower discipline - Naaasty */
    if (ch->power[DISC_WARL_WILL] > 0)
	damage += ch->power[DISC_WARL_WILL];

    if (ch->power[DISC_VAMP_DOMI] >= 10) /* INFLUENCE!!!! */
    {
	damage *= 3;
	damage /= 2;
    }

    if (victim->power[DISC_VAMP_DOMI] >= 9) /* self control */
    {
	damage -= 2;
	if (damage < 1) damage = 1;
    }

    victim->pcdata->sanity -= damage;

    if (victim->pcdata->sanity <= 0)
    {
	victim->pcdata->sanity = 0;
	stc("You try to resist, but you do not have enough mental power left.\n\r", victim);
	return FALSE;
    }

    stc("You burn off some sanity points to resist the attack.\n\r", victim);   
    stc("They manage to resist the attack.\n\r", ch);
    return TRUE;
}


bool is_same_realm(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (ch->realm != victim->realm) return FALSE;
    return TRUE;
}

void obj_exorcise(CHAR_DATA *ch)
{
    crash("obj exorcise");

    check_exorcise_function(ch, ch->carrying);
}

/* This is the recursive routine... hopefully */
void check_exorcise_function(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (obj == NULL) return;

    crash("check_exorcise_function");

    if (obj->sw_char != NULL && ch != obj->sw_char)
    {
 	char_from_obj(obj->sw_char);
	act("You are exorcised from $p.", obj->sw_char, obj, NULL, TO_CHAR);
	act("$n is exorcised from $p.", obj->sw_char, obj, NULL, TO_ROOM);
    }

    if (obj->contains != NULL)
	check_exorcise_function(ch, obj->contains);

    if (obj->next_content != NULL)
	check_exorcise_function(ch, obj->next_content);
}



void do_ostats(CHAR_DATA *ch, char *argument)
{
    if (ch->sw_obj == NULL)
    {
	stc("You are not in an object.\n\r", ch);
	return;
    }

    if (ch->sw_obj != NULL)
  	stcf(ch, "You are switched with %s.\n\r", ch->sw_obj->short_descr);
    if (ch->sw_obj->sw_char != NULL)
  	stcf(ch, "%s has %s in it.\n\r", ch->sw_obj->short_descr,
	    GET_PROPER_NAME(ch->sw_obj->sw_char));
}

void do_oswitch(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if (IS_NPC(ch) || ch->desc == NULL) return;

    if (is_mob_switched(ch))
    {
	stc("Not while switched.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("What object do you wish to switch with?\n\r", ch);
	return;
    }

    if ( ( obj = get_obj_world(ch, argument) ) == NULL)
    {
	stc("No such object found.\n\r", ch);
	return;
    }

    act("You switch with $p.", ch, obj, NULL, TO_CHAR);
    char_to_obj(ch, obj);
}

void do_oreturn(CHAR_DATA *ch, char *argument)
{
    stc("You return from the object.\n\r", ch);
    char_from_obj(ch);
}

/* This calls the recursive routine for checking moved sw_items */
void check_char_sw_obj(CHAR_DATA *ch)
{
    crash("check_char_sw_obj");

    check_char_function(ch, ch->carrying);
}

/* This is the recursive routine... hopefully */
void check_char_function(CHAR_DATA *ch, OBJ_DATA *obj)
{
    crash("check_char_function");

    if (obj == NULL) return;

    if (obj->sw_char != NULL && ch != obj->sw_char)
	move_char_to_obj(obj->sw_char);

    if (obj->contains != NULL)
	check_char_function(ch, obj->contains);

    if (obj->next_content != NULL)
	check_char_function(ch, obj->next_content);
}

void move_char_to_obj(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *room;

    crash("move_char_to_obj");

    if (ch->sw_obj == NULL) return;

    if (ch->in_room == (room = locate_obj(ch->sw_obj))) return;

    char_from_room(ch);
    char_to_room(ch, room, ch->sw_obj->altitude);
}

void move_obj_to_char(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *room;

    crash("move_obj_to_char");

    if (ch->sw_obj == NULL) return;

    if ((room = ch->in_room) == locate_obj(ch->sw_obj)) return;

    obj_from_room(ch->sw_obj);
    obj_to_room(ch->sw_obj, room, ch->altitude);
}

bool is_item(CHAR_DATA *ch)
{
    if (IS_NPC(ch)) return FALSE;

    if (ch->sw_obj != NULL) return TRUE;
    return FALSE;
}

bool char_from_obj(CHAR_DATA *ch)
{
    crash("char_from_obj");

    if (ch->sw_obj == NULL) return FALSE;
    REMOVE_BIT(ch->sw_obj->extra_flags3, ITEM_INHABITED);
    stop_fighting(ch, TRUE);

    ch->sw_obj->sw_char = NULL;

    /* This does all the riding shit */
    if (ch->riding != NULL)
	ch->riding->riden_by = NULL;
    if (ch->riden_by != NULL)
	ch->riden_by->riding = NULL;
    ch->riding = NULL;
    ch->riden_by = NULL;

    /* Blow up the item */
    if (IS_OBJ_STAT3(ch->sw_obj, ITEM_DESTROY_CHAR_RETURN)) extract_obj(ch->sw_obj);

    ch->sw_obj = NULL;
    REMOVE_BIT(ch->flag6, AFF6_OBJ_SWITCHED);

    set_pc_name(ch, GET_PROPER_NAME(ch));
    if (get_normal_form(ch) != 0)    
	set_form(ch, get_normal_form(ch));

    return TRUE;
}

bool char_to_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
    crash("char_to_obj");

    if (ch == NULL || obj == NULL) return FALSE;
    if (IS_NPC(ch)) return FALSE;

    stop_fighting(ch, TRUE);

    if (obj->carried_by == ch)
    {
	stc("You cannot switch with an object you are carrying.\n\r", ch);
	return FALSE;
    }

    if (ch->sw_obj != NULL)
    {
	stc("You are already inhabiting an object.\n\r", ch);
	return FALSE;
    }

    if (obj->sw_char != NULL)
    {
	stc("That object is already inhabited.\n\r", ch);
	return FALSE;
    }

    set_form(ch, 0);
    set_pc_name(ch, obj->short_descr);
    SET_BIT(ch->flag6, AFF6_OBJ_SWITCHED);
    SET_BIT(obj->extra_flags3, ITEM_INHABITED);

    ch->sw_obj = obj;
    obj->sw_char = ch;

    /* This moves the char if they are not in the same place */
    move_char_to_obj(ch);

    return TRUE;
}


void do_enhance(CHAR_DATA *ch, char *argument)
{
    if (ch->pet == NULL || !IS_SET(ch->pet->flag5, AFF5_FAMILIAR) )
    {
	stc("You do not have a familiar.\n\r", ch);	
	return;
    }

    if (!in_same_room(ch, ch->pet))
    {
	stc("Your familiar is not here.\n\r", ch);
	return;
    }

    if (ch->qp < 1)
    {
	stc("You do not have enough kai to do this.\n\r", ch);
	return;
    }

    if (ch->pet->power[DISC_WARL_CONT] < 0)
	ch->pet->power[DISC_WARL_CONT] = 0;

    if (argument[0] == '\0')
    {
	stc("Syntax : enhance [armour magres hitpoints]\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "magres"))
    {
	if (ch->pet->power[DISC_WARL_CONT] == 10)
	{
	    stc("You cannot raise their magic resistance any higher.\n\r", ch);
	    return;
	}
	ch->qp--;
	ch->pet->power[DISC_WARL_CONT]++;
	stcf(ch, "Your pets magic resistance has been raised to %d%%.\n\r",
		ch->pet->power[DISC_WARL_CONT] * 10);
	return;
    }

    else if (!str_prefix(argument, "armour"))
    {
	int a;

	ch->qp--;
	for (a = 0 ; a < 4 ; a++)
	    ch->pet->armor[a] -= (10 + dice(1, 4));
	stcf(ch, "Your pets armour rating has been reduced to %d%%.\n\r",
		ch->pet->armor[0]);
	return;
    }

    else if (!str_prefix(argument, "hitpoints"))
    {
	ch->qp--;

	ch->pet->max_hit += 5 * dice(1,ch->power[DISC_WARL_THEA]);
	restore_char(ch->pet);
	stcf(ch, "Your pets hit points have been increased to %d.\n\r",
		ch->pet->max_hit);
	return;
    }

    do_enhance(ch, "");
    return;
}

void do_callpet(CHAR_DATA *ch, char *argument)
{
    if (ch->pet == NULL || !IS_SET(ch->pet->flag5, AFF5_FAMILIAR) )
    {
	stc("You do not own a familiar.\n\r", ch);
	return;
    }

    if (in_same_room(ch, ch->pet))
    {
	act("But $N is already here!", ch, NULL, ch->pet, TO_CHAR);
	return;
    }

    WAIT_STATE(ch, 6);
    stc("You whistle loudly.\n\r", ch);
    act("$n whistles loudly.", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch->pet);
    char_to_room(ch->pet, ch->in_room, ch->altitude);
    act("$n walks calmly into the room.", ch->pet, NULL, NULL, TO_ROOM);
    stc("You have been summoned by your master.\n\r", ch->pet);
}

void do_familiar(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *pet;

    if (IS_NPC(ch)) return;
 
    if (ch->pet != NULL)
    {
	stc("You cannot have more than one pet at a time.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("Syntax : familiar [cat hawk mouse weasel quasit]\n\r", ch);
	return;
    }

    /* Imp 11, Cat 12 */
    if (!str_prefix(argument, "cat"))
    {
        pet = create_mobile( get_mob_index( 12 ) );
        char_to_room(pet, ch->in_room, ch->altitude);
        act("$n has summoned a cat for $s companion.",ch,NULL,NULL,TO_ROOM);
	SET_BIT(pet->flag5, AFF5_FAMILIAR_CAT);
        send_to_char("A cute black cat arrives and rubs up against your leg.\n\r", ch);
        add_follower( pet, ch );
	pet->size = SIZE_SMALL;
    }
    else if (!str_prefix(argument, "quasit"))
    {
	if (IS_GOOD(ch))
	{
	    stc("Good wizards may not summon a quasit.\n\r", ch);
	    return;
	}
        pet = create_mobile( get_mob_index( 11 ) );
        char_to_room(pet, ch->in_room, ch->altitude);
        act("$n has summoned a quasit for $s companion.",ch,NULL,NULL,TO_ROOM);
	SET_BIT(pet->flag5, AFF5_FAMILIAR_QUASIT);
        send_to_char("A nasty little quasit arrives and perches on your shoulder.\n\r", ch);
        add_follower( pet, ch );
	pet->size = SIZE_SMALL;
    }
    else if (!str_prefix(argument, "mouse"))
    {
        pet = create_mobile( get_mob_index( 35 ) );
        char_to_room(pet, ch->in_room, ch->altitude);
        act("$n has summoned a mouse for $s companion.",ch,NULL,NULL,TO_ROOM);
	SET_BIT(pet->flag5, AFF5_FAMILIAR_MOUSE);
        send_to_char("A small mouse scurries from nowhere.\n\r", ch);
        add_follower( pet, ch );
	pet->size = SIZE_TINY;
    }
    else if (!str_prefix(argument, "weasel"))
    {
        pet = create_mobile( get_mob_index( 34 ) );
        char_to_room(pet, ch->in_room, ch->altitude);
        act("$n has summoned a weasel for $s companion.",ch,NULL,NULL,TO_ROOM);
	SET_BIT(pet->flag5, AFF5_FAMILIAR_WEASEL);
        send_to_char("A little weasel appears and curls around your feet.\n\r", ch);
        add_follower( pet, ch );
	pet->size = SIZE_SMALL;
    }
    else if (!str_prefix(argument, "hawk"))
    {
        pet = create_mobile( get_mob_index( 36 ) );
        char_to_room(pet, ch->in_room, ch->altitude);
        act("$n has summoned a hawk for $s companion.",ch,NULL,NULL,TO_ROOM);
	SET_BIT(pet->flag5, AFF5_FAMILIAR_HAWK);
        send_to_char("A beautiful hawk arrives and perches on your shoulder.\n\r", ch);
        add_follower( pet, ch );
	pet->size = SIZE_SMALL;
    }
    else
    {
	do_familiar(ch, "");
	return;
    }

    WAIT_STATE(ch, 12);
    /*SET_BIT(pet->act, ACT_PET);*/
    SET_BIT(pet->affected_by, AFF_CHARM);
    SET_BIT(pet->flag5, AFF5_FAMILIAR);
    pet->leader = ch;
    ch->pet = pet;

    sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
            pet->description, GET_PROPER_NAME(ch) );
    free_string( pet->description );
    pet->description = str_dup( buf );


}


