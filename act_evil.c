/******************************************************************
 * Act_Evil -- (C) 1996, by Adam Powell (More racial abilities)   *
 *                       and Chris Mellish                        *
 ******************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hell.h"
#include "magic.h"

DECLARE_DO_FUN( do_look         );
DECLARE_DO_FUN( do_say          );
DECLARE_DO_FUN( do_think        );


bool IS_AIR (ROOM_INDEX_DATA *room)
{
    if (room->sector_type == SECT_AIR) return TRUE;

    return FALSE;
}

void do_bathe (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    obj = get_obj_list(ch, argument, ch->in_room->contents);

    if (IS_NPC(ch)) return;

    if (argument[0] == '\0')
    {
	send_to_char("Bathe in what?\n\r", ch);
	return;
    }

    if (obj == NULL)
    {
	send_to_char("There is nothing here to bathe in.\n\r", ch);
	return;
    }

    if (obj->item_type == ITEM_FOUNTAIN)
    {
	act("You bathe in $p.", ch, obj, NULL, TO_CHAR);
	act("$n bathes in $p.", ch, obj, NULL, TO_ROOM);
	WAIT_STATE(ch, 4);
	return;
    }

    if (obj->item_type == ITEM_RAINBOW_POOL)
    {
	if (obj->sw_char != NULL
	  	&& obj->sw_char->secondaryrace == RACE_DAEMON)
  	{
	    act("You bathe in $p.", ch, obj, NULL, TO_CHAR);
	    act("$n bathes in $p.", ch, obj, NULL, TO_ROOM);
	    act("You grow a huge mouth and swallow $n whole, start chewing, and then spit $m out.", ch, obj, obj->sw_char, TO_VICT);
	    act("$p grows a huge mouth, swallows you whole, starts chewing, and then spits you out.", ch, obj, obj->sw_char, TO_CHAR);
	    act("$p grows a huge mouth and swallows $n whole, starts chewing, and then spits $m out.", ch, obj, obj->sw_char, TO_NOTVICT);
	    ch->hit = 1;
	    return;	
	}
	   
	if (ch->secondaryrace != RACE_ELADRIN || ch->power[DISC_ELAD_AURA] < 0)
	{
	    act("You bathe in $p.", ch, obj, NULL, TO_CHAR);
	    act("$n bathes in $p.", ch, obj, NULL, TO_ROOM);
	    room_message(ch->in_room, "The pool drains away into the ground.");
	    restore_char(ch);
	    extract_obj(obj);
	    WAIT_STATE(ch, 4);
	    return;
	}
	else
	{
	    act("You bathe in $p.", ch, obj, NULL, TO_CHAR);
	    act("$n bathes in $p.", ch, obj, NULL, TO_ROOM);
	    WAIT_STATE(ch, 8);
	    send_to_char("Your aura flickers and shimmers.\n\r", ch);
	    act("$n's aura flickers and shimmers.", ch, NULL, NULL, TO_ROOM);
	    ch->power[DISC_ELAD_AURA] += 1;
	    if (ch->power[DISC_ELAD_AURA] > 10)
		ch->power[DISC_ELAD_AURA] = 10;
	    extract_obj(obj);
	    return;
	}
    }

    /* put all other bathable things before dragons blood...
	its way too complicated. */

    if (obj->item_type != ITEM_DRAGON_BLOOD)
    {
	send_to_char("You can't bathe in that.\n\r", ch);
	return;
    }

    if (ch->secondaryrace == RACE_VAMPIRE)
    {
	send_to_char("Dragon's blood is deadly poison to vampires.\n\r", ch);
	return;
    }

    if (ch->secondaryrace == RACE_DRAGON
	&& ch->dragtype == obj->value[0])
    {
        act("You bathe in $p.", ch, obj, NULL, TO_CHAR);
        act("$n bathes in $p.", ch, obj, NULL, TO_ROOM);
	ch->hit = ch->max_hit;
 	ch->move = ch->max_move;
	send_to_char("You feel fully refreshed.\n\r", ch);
	return;
    }	

    if (ch->secondaryrace == RACE_DRAGON
	&& ch->dragtype != obj->value[0])
    {
        act("You bathe in $p.", ch, obj, NULL, TO_CHAR);
        act("$n bathes in $p.", ch, obj, NULL, TO_ROOM);
	ch->hit = 1;
 	ch->move = 1;
	send_to_char("You feel totally drained.\n\r", ch);
	return;
    }	

    if (ch->secondaryrace != 0 || !can_be_made(ch))
    {
	send_to_char("Bathing in dragons' blood might be fatal.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);
    act("You bathe in $p.", ch, obj, NULL, TO_CHAR);
    act("$n bathes in $p.", ch, obj, NULL, TO_ROOM);

    if (obj->value[0] == DRAGON_RED)
	act("$n starts to transform into a baby red dragon.",ch, NULL, NULL, TO_ROOM);
    else if (obj->value[0] == DRAGON_WHITE)
	act("$n starts to transform into a baby white dragon.",ch, NULL, NULL, TO_ROOM);
    else if (obj->value[0] == DRAGON_BLACK)
	act("$n starts to transform into a baby black dragon.",ch, NULL, NULL, TO_ROOM);
    else if (obj->value[0] == DRAGON_GREEN)
	act("$n starts to transform into a baby green dragon.",ch, NULL, NULL, TO_ROOM);
    else if (obj->value[0] == DRAGON_BLUE)
	act("$n starts to transform into a baby blue dragon.",ch, NULL, NULL, TO_ROOM);
    else if (obj->value[0] == DRAGON_SILVER)
	act("$n starts to transform into a baby silver dragon.",ch, NULL, NULL, TO_ROOM);
    else if (obj->value[0] == DRAGON_GOLD)
	act("$n starts to transform into a baby gold dragon.",ch, NULL, NULL, TO_ROOM);
    else if (obj->value[0] == DRAGON_SHADOW)
	act("$n starts to transform into a baby shadow dragon.",ch, NULL,NULL, TO_ROOM);
    else
    {
	bug ("Transform into unknown dragon type.", 0);
	return;
    }

    send_to_char("You begin a painful transformation, the exertion on your body is too great\n\rYou pass out.\n\r", ch);
    ch->position = POS_SLEEPING;
    set_learnable_disciplines(ch, RACE_DRAGON);
    ch->pcdata->condition[COND_HUNGER] = 100;
    ch->pcdata->condition[COND_THIRST] = 100;
    ch->tertiaryrace = 0;
    ch->clan = 0;
    SET_TIMER(ch, TIMER_DRAGON_GROW, 1200);
    if (ch->fire < 1)
    {
	ch->fire = 1;
        gn_add(ch, 10);
	ch->pcdata->max_memorize[1]++;
    }
    if (ch->entropy < 1)
    {
        ch->entropy = 1;
        gn_add(ch, 50);
	ch->pcdata->max_memorize[1]++;
    }
    if (obj->value[0] < DRAGON_SILVER) ch->alignment = -900;
    else ch->alignment = 500;

    ch->generation = 1;
    ch->secondaryrace = RACE_DRAGON;
    ch->dragtype = obj->value[0]; /* value 0 is type of blood. */
    set_form( ch, 0 );
    save_char_obj(ch);
    extract_obj(obj);
    return;
}

char * dragon_colour (sh_int dragtype)
{
    if (dragtype == DRAGON_RED) return "#1Red#n";
    else if (dragtype == DRAGON_WHITE) return "#7White#n";
    else if (dragtype == DRAGON_BLACK) return "Black";
    else if (dragtype == DRAGON_GREEN) return "#2Green#n";
    else if (dragtype == DRAGON_BLUE) return "#4Blue#n";
    else if (dragtype == DRAGON_SILVER) return "#6Silver#n";
    else if (dragtype == DRAGON_GOLD) return "#3Gold#n";
    else if (dragtype == DRAGON_SHADOW) return "Shadow";
    return "Bug, please inform Jander";
}

char * dragon_colour2 (sh_int dragtype)
{
    if (dragtype == DRAGON_RED) return "red";
    else if (dragtype == DRAGON_WHITE) return "white";
    else if (dragtype == DRAGON_BLACK) return "black";
    else if (dragtype == DRAGON_GREEN) return "green";
    else if (dragtype == DRAGON_BLUE) return "blue";
    else if (dragtype == DRAGON_SILVER) return "silver";
    else if (dragtype == DRAGON_GOLD) return "gold";
    else if (dragtype == DRAGON_SHADOW) return "shadow";
    return "Bug, please inform Jander";
}

void do_feather (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if (argument[0] == '\0')
    {
	send_to_char("Syntax : feather [copper", ch);
	if (ch->power[DISC_LUPI_HAWK] > 3)
	    send_to_char(" silver", ch);
	if (ch->power[DISC_LUPI_HAWK] > 6)
	    send_to_char(" golden", ch);
	if (ch->power[DISC_LUPI_HAWK] > 9)
	    send_to_char(" platinum", ch);
	send_to_char("]\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_CAN_FEATHER))
    {
	send_to_char("You cannot create another feather yet.\n\r", ch);
	WAIT_STATE(ch, 2);
	return;
    }

    if (!str_prefix(argument, "copper"))
    {
	SET_TIMER(ch, TIMER_CAN_FEATHER, 3);
	obj = create_object(get_obj_index(99), ch->level);
	send_to_char("You pluck a hair, which transforms into a beautiful copper feather.\n\r", ch);
	act("$n plucks a hair out of $s head, which transforms into a feather.", ch, NULL, NULL, TO_ROOM);
	obj->item_type = ITEM_FEATHER;    /* just in case */
	obj->value[0] = FEATHER_COPPER;
	free_string(obj->short_descr);
	obj->short_descr = str_dup("a copper feather");
	free_string(obj->name);
	obj->name = str_dup("copper feather");
	obj_to_char(obj, ch);
	WAIT_STATE(ch, 8);	
	return;
    }
    else if (!str_prefix(argument, "silver"))
    {
	SET_TIMER(ch, TIMER_CAN_FEATHER, 6);
	obj = create_object(get_obj_index(99), ch->level);
	send_to_char("You pluck a hair, which transforms into a beautiful silver feather.\n\r", ch);
	act("$n plucks a hair out of $s head, which transforms into a feather.", ch, NULL, NULL, TO_ROOM);
	obj->item_type = ITEM_FEATHER;    /* just in case */
	obj->value[0] = FEATHER_SILVER;
	obj_to_char(obj, ch);
	free_string(obj->short_descr);
	obj->short_descr = str_dup("a silver feather");
	free_string(obj->name);
	obj->name = str_dup("silver feather");
	WAIT_STATE(ch, 8);	
	return;
    }
    else if (!str_prefix(argument, "golden"))
    {
	SET_TIMER(ch, TIMER_CAN_FEATHER, 9);
	obj = create_object(get_obj_index(99), ch->level);
	send_to_char("You pluck a hair, which transforms into a beautiful golden feather.\n\r", ch);
	act("$n plucks a hair out of $s head, which transforms into a feather.", ch, NULL, NULL, TO_ROOM);
	obj->item_type = ITEM_FEATHER;    /* just in case */
	obj->value[0] = FEATHER_GOLDEN;
	obj_to_char(obj, ch);
	free_string(obj->short_descr);
	obj->short_descr = str_dup("a golden feather");
	free_string(obj->name);
	obj->name = str_dup("golden feather");
	WAIT_STATE(ch, 8);	
	return;
    }
    else if (!str_prefix(argument, "platinum"))
    {
	SET_TIMER(ch, TIMER_CAN_FEATHER, 12);
	obj = create_object(get_obj_index(99), ch->level);
	send_to_char("You pluck a hair, which transforms into a beautiful platinum feather.\n\r", ch);
	act("$n plucks a hair out of $s head, which transforms into a feather.", ch, NULL, NULL, TO_ROOM);
	obj->item_type = ITEM_FEATHER;    /* just in case */
	obj->value[0] = FEATHER_PLATINUM;
	free_string(obj->short_descr);
	obj->short_descr = str_dup("a platinum feather");
	free_string(obj->name);
	obj->name = str_dup("platinum feather");
	obj_to_char(obj, ch);
	WAIT_STATE(ch, 8);	
	return;
    }

    do_feather(ch, "");
    return;
}



void do_spit (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    if (argument[0] == '\0')
    {
	send_to_char("Who do you wish to spit venom at?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_CAN_SPIT_VENOM))
    {
	send_to_char("Your throat is dry.\n\r", ch);
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        stc("You may not spit venom while within a safe room.\r\n", ch);
        return;
    }

    act("You spit venom at $N.", ch, NULL, victim, TO_CHAR);
    act("$n spits venom at you.", ch, NULL, victim, TO_VICT);
    act("$n spits venom at $N.", ch, NULL, victim, TO_NOTVICT);

    SET_TIMER(ch, TIMER_CAN_SPIT_VENOM, dice(1, 4) );

    if (IS_INSUBSTANTIAL(victim) )
    {
	send_to_char("The venom passes straight through you.", victim);
	act("The venom passes straight through $n.", victim, NULL, NULL, TO_ROOM);
	return;
    }

    if (IS_NPC(victim)) set_fighting(ch, victim);

    if ( (get_curr_stat(ch, STAT_DEX) > 18 || dice(1, 10) < 2)
	&& dice(1, 10) > 3)
    {
	act("$n is blinded.", victim, NULL, NULL, TO_ROOM);
	send_to_char("The venom hits your eyes, you are blinded.\n\r", victim);
	 
        af.where        = TO_AFFECTS;
        af.type         = skill_lookup("blindness");
        af.level        = ch->level;
        af.duration     = dice(1, 2);
        af.location     = APPLY_HITROLL;
        af.modifier     = -20;
        af.bitvector    = AFF_BLIND;
	affect_to_char(victim, &af);
	return;
    }
    else
    {
	send_to_char("The venom sprays over your body.\n\r", victim);
        if ( saves_spell( ch->level, victim,DAM_POISON)
            || IS_SET(victim->imm_flags, IMM_POISON) )
	{
    	    send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
            return;
	}

        af.where        = TO_AFFECTS;
        af.type         = skill_lookup("poison");
        af.level        = ch->level;
        af.duration     = dice(1, 4);
        af.location     = APPLY_STR;
        af.modifier     = -2;
        af.bitvector    = AFF_POISON;
	affect_to_char(victim, &af);
	send_to_char("You feel very sick.\n\r", victim);
	return;
    }
}

void do_expell (CHAR_DATA *ch, char *argument)
{
    if (ch->breaths == 0)
    {
	stc("You are out of breath.\n\r", ch);
	return;
    }

    stc("You expell all the tiredness within you outwards with a blast of air.\n\r", ch);
    act("$n breaths out a mighty blast of air.", ch, NULL, NULL, TO_ROOM);
    ch->move = UMIN(ch->max_move, ch->move * 3 / 2);
    ch->breaths -= 1;
    if (!TIME_UP(ch, TIMER_CAN_POLYMORPH))
    {
	SET_TIMER(ch, TIMER_CAN_POLYMORPH, 0);
	stc("You are ready to polymorph again.\n\r", ch);
    }
    WAIT_STATE(ch, 8);
}

 
void do_breathe (CHAR_DATA *ch, char *argument)
{
    int level;
    bool all = FALSE;
    CHAR_DATA *victim = NULL;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];

    level = ch->power[DISC_DRAG_FIRE];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (ch->breaths == 0)
    {
	send_to_char("You are out of breath.\n\r", ch);
	return;
    }

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("Syntax : breathe <type> {victim all}\n\r", ch);
	send_to_char("          type being fire, gas, lightning, frost or acid\n\r", ch);
	return;
    }

    if (ch->dragtype == 0)
    {
	send_to_char("You do not have a dragon colour.  Please inform Jander.\n\r", ch);
	return;
    }

    if (!str_cmp(arg2, "all")) all = TRUE;

    else if ( (victim = get_char_room(ch, arg2)) == NULL )
    {	
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "frost"))
    {
	if (ch->dragtype != DRAGON_WHITE
		&& level < 6)
	{
	    send_to_char("You don't know how to breathe frost yet.\n\r", ch);
	    return;
	}
	if (ch->dragtype == DRAGON_RED)
 	{
	    send_to_char("Red dragons cannot breathe frost.\n\r", ch);
	    return;
	}

	ch->breaths -= 1;
	WAIT_STATE(ch, 12);
	frost_breath(ch, victim, all);
	return;
    }
    if (!str_prefix(arg1, "lightning"))
    {
	if (ch->dragtype != DRAGON_BLUE
		&& level < 7)
	{
	    send_to_char("You don't know how to breathe lightning yet.\n\r", ch);
	    return;
	}
	if (ch->dragtype == DRAGON_GREEN)
 	{
	    send_to_char("Green dragons cannot breathe lightning.\n\r", ch);
	    return;
	}
	ch->breaths -= 1;
	WAIT_STATE(ch, 12);

	lightning_breath(ch, victim, all);
	return;
    }
    if (!str_prefix(arg1, "gas"))
    {
	if (ch->dragtype != DRAGON_GREEN
		&& level < 8)
	{
	    send_to_char("You don't know how to breathe gas yet.\n\r", ch);
	    return;
	}
	if (ch->dragtype == DRAGON_BLUE)
 	{
	    send_to_char("Blue dragons cannot breathe gas.\n\r", ch);
	    return;
	}
	ch->breaths -= 1;
	WAIT_STATE(ch, 12);
	gas_breath(ch, victim, all);
	return;
    }
    if (!str_prefix(arg1, "acid"))
    {
	if (ch->dragtype != DRAGON_BLACK
		&& level < 9)
	{
	    send_to_char("You don't know how to breathe acid yet.\n\r", ch);
	    return;
	}
	if (ch->dragtype == DRAGON_SILVER)
 	{
	    send_to_char("Silver dragons cannot breathe acid.\n\r", ch);
	    return;
	}
	ch->breaths -= 1;
	WAIT_STATE(ch, 12);

	acid_breath(ch, victim, all);
	return;
    }
    if (!str_prefix(arg1, "fire"))
    {
	if (ch->dragtype != DRAGON_RED
		&& level < 10)
	{
	    send_to_char("You don't know how to breathe fire yet.\n\r", ch);
	    return;
	}
	if (ch->dragtype == DRAGON_WHITE)
 	{
	    send_to_char("White dragons cannot breathe fire.\n\r", ch);
	    return;
	}
	ch->breaths -= 1;
	WAIT_STATE(ch, 12);

	fire_breath(ch, victim, all);
	return;
    }

    do_breathe(ch, "");
}

void do_artifacts (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;

    cent_to_char("Artifacts in the game", ch);
    cent_to_char("- -----==><==------ -", ch);
    send_to_char("\n\r", ch);

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if (!IS_SET(obj->extra_flags, ITEM_ARTIFACT)) continue;

	found = TRUE;

	if (obj->carried_by != NULL && can_see_obj(ch, obj)
	    && ( (IS_SET(obj->extra_flags2, ITEM_KNOW_OWNER)
		|| ch->power[DISC_HERO_CRUS] > 6
		|| is_wearing_artifact(ch, ARTIFACT_EYE_SENSUS) != NULL ) )
		&& can_see(ch, obj->carried_by) )
	    sprintf(buf, "%s (Owned by %s)", obj->short_descr, GET_PROPER_NAME(obj->carried_by) );
	else if (obj->carried_by != NULL)
	    sprintf(buf, "%s (Owned)", obj->short_descr);
	else
	    sprintf(buf, "%s", obj->short_descr);

	cent_to_char(buf, ch);
    }

    if (!found)
	cent_to_char("There are no artifacts in the game.", ch);
}

void do_voohunt (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;

    cent_to_char("Voodoo dolls in the game", ch);
    cent_to_char("- - -----==><==----- - -", ch);
    send_to_char("\n\r", ch);

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if (obj->item_type != ITEM_VOODOO_DOLL) continue;
	if (!can_see_obj(ch, obj)) continue;

	found = TRUE;

	if (obj->carried_by != NULL && can_see(ch, obj->carried_by) )
	    sprintf(buf, "%s (Owned by %s)", obj->short_descr, GET_PROPER_NAME(obj->carried_by) );
	else
	    sprintf(buf, "%s", obj->short_descr);

	cent_to_char(buf, ch);
    }

    if (!found)
	cent_to_char("There are no voodoo dolls in the game.", ch);
}

void do_relics (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;

    cent_to_char("Relics in the game", ch);
    cent_to_char("- ----==><==---- -", ch);
    send_to_char("\n\r", ch);

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if (!IS_SET(obj->extra_flags, ITEM_RELIC)) continue;
	if (!can_see_obj(ch, obj)) continue;
	found = TRUE;

	if (obj->carried_by != NULL && can_see(ch, obj->carried_by)
	    && ( IS_SET(obj->extra_flags2, ITEM_KNOW_OWNER)
		|| is_wearing_artifact(ch, ARTIFACT_EYE_SENSUS) != NULL ) )
	    sprintf(buf, "%s (Owned by %s)", obj->short_descr, GET_PROPER_NAME(obj->carried_by) );
	else if (obj->carried_by != NULL)
	    sprintf(buf, "%s (Owned)", obj->short_descr);
	else
	    sprintf(buf, "%s", obj->short_descr);

	cent_to_char(buf, ch);
    }

    if (!found)
	cent_to_char("There are no relics in the game.", ch);
}


OBJ_DATA * is_wearing_artifact(CHAR_DATA *ch, int artifact)
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	if (obj->artifact == artifact
		&& obj->wear_loc != WEAR_NONE) return obj;

    return NULL;
}

OBJ_DATA * is_carrying_artifact(CHAR_DATA *ch, int artifact)
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	if (obj->artifact == artifact) return obj;

    return NULL;
}


void do_diagnose (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    bool injury = FALSE;
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	send_to_char("Whose injuries do you wish to diagnose?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument)) == NULL )
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    sprintf(buf, "Diagnosis of %s's injuries.", GET_NAME(victim) );
    cent_to_char(buf, ch);
    divide6_to_char(ch);
    send_to_char("\n\r", ch);
 

    if (BODY_PART(victim, BODY_RIGHT_LEG)
	&& !BODY_PART(victim, BODY_LEFT_LEG) )
    {
	act("$N is missing $S right leg.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }

    else if (!BODY_PART(victim, BODY_RIGHT_LEG)
	&& BODY_PART(victim, BODY_LEFT_LEG) )
    {
	act("$N is missing $S left leg.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }

    else if (BODY_PART(victim, BODY_RIGHT_LEG)
	&& BODY_PART(victim, BODY_LEFT_LEG) )
    {
	act("$N is missing both $S legs!",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }


    /* Arms code, complex cos of hands, bloody useless things anyway. */


    if (BODY_PART(victim, BODY_LEFT_ARM)
	&& BODY_PART(victim, BODY_RIGHT_ARM) )
    {
	act("$N is missing both $S arms!",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }

    else if (!BODY_PART(victim, BODY_LEFT_ARM)
	&& !BODY_PART(victim, BODY_RIGHT_ARM)
	&& BODY_PART(victim, BODY_RIGHT_HAND)
	&& BODY_PART(victim, BODY_LEFT_HAND))
    {
	act("$N is missing both $S hands!",  ch, NULL, victim, TO_CHAR);
    	injury = TRUE;
    }


    else if (BODY_PART(victim, BODY_LEFT_ARM)
	&& !BODY_PART(victim, BODY_RIGHT_ARM)
	&& BODY_PART(victim, BODY_RIGHT_HAND))
    {
	act("$N is missing $S left arm and $S right hand.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }

    else if (!BODY_PART(victim, BODY_LEFT_ARM)
	&& BODY_PART(victim, BODY_RIGHT_ARM)
	&& !BODY_PART(victim, BODY_RIGHT_HAND))
    {
	act("$N is missing $S right arm and $S left hand.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }

    else if (!BODY_PART(victim, BODY_LEFT_ARM)
	&& BODY_PART(victim, BODY_RIGHT_ARM)
	&& BODY_PART(victim, BODY_RIGHT_HAND))
    {
	act("$N is missing $S right arm.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }

    else if (!BODY_PART(victim, BODY_RIGHT_ARM)
	&& BODY_PART(victim, BODY_LEFT_ARM)
	&& BODY_PART(victim, BODY_LEFT_HAND))
    {
	act("$N is missing $S left arm.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }

    else if (!BODY_PART(victim, BODY_LEFT_ARM)
	&& !BODY_PART(victim, BODY_RIGHT_ARM)
	&& BODY_PART(victim, BODY_RIGHT_HAND))
    {
	act("$N is missing $S right hand.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }

    else if (!BODY_PART(victim, BODY_LEFT_ARM)
	&& !BODY_PART(victim, BODY_RIGHT_ARM)
	&& BODY_PART(victim, BODY_LEFT_HAND))
    {
	act("$N is missing $S left hand.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }


    /* Eye and ear code */

    if (BODY_PART(victim, BODY_LEFT_EAR)
	&& BODY_PART(victim, BODY_RIGHT_EAR) )
    {
	act("$N has had both ears cut off.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }

    else if (BODY_PART(victim, BODY_LEFT_EAR)
	&& !BODY_PART(victim, BODY_RIGHT_EAR) )
    {
	act("$N has had $S left ear cut off.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }

    else if (!BODY_PART(victim, BODY_LEFT_EAR)
	&& BODY_PART(victim, BODY_RIGHT_EAR) )
    {
	act("$N has had $S right ear cut off.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }

    if (BODY_PART(victim, BODY_LEFT_EYE)
	&& BODY_PART(victim, BODY_RIGHT_EYE) )
    {
	act("Both $N's eyes are missing.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }


    else if (BODY_PART(victim, BODY_LEFT_EYE)
	&& !BODY_PART(victim, BODY_RIGHT_EYE) )
    {
	act("$N's left eye is missing.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }

    else if (!BODY_PART(victim, BODY_LEFT_EYE)
	&& BODY_PART(victim, BODY_RIGHT_EYE) )
    {
	act("$N's right eye is missing.",  ch, NULL, victim, TO_CHAR);
	injury = TRUE;
    }

    if (!injury)
	act("$N has no apparent injuries.", ch, NULL, victim, TO_CHAR);

}

void do_memorize (CHAR_DATA *ch, char *argument)
{
    int i, found = -1, level, realm, spells = 0;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
	send_to_char("NPC's do not need to memorize spells.\n\r", ch);
	return;
    }

    for (i = 1 ; i < 11 ; i++)
    {
	if (ch->pcdata->max_memorize[i] > 9)
	    ch->pcdata->max_memorize[i] = 9; 
    }

    if (argument[0] == '\0')
    {
	send_to_char("Syntax : memorize <spell> or memorize list\n\r", ch);
	return;
    }	

    if (ch->secondaryrace == 0)
    {
	send_to_char("You cannot memorize spells yet.\n\r", ch);
	return;
    }

    if (!str_cmp(argument, "list"))
    {
        int wrap = 0, i, j;
        bool foundspell = FALSE;
    
        cent_to_char("Usual Memorization", ch);
        cent_to_char("----- ------------", ch);
        send_to_char("\n\r", ch);
        for (i = 1 ; i < 11 ; i++)
        {
            for (j = 0 ; j < 10 ; j++)
            {
                if (wrap == 3)
                {
                    wrap = 0;
                    send_to_char("\n\r", ch);
                }
                if (ch->pcdata->last_memorized[i][j] != -1)
                {
                    sprintf(buf, "%-26s",skill_table[ch->pcdata->last_memorized[i][j]].name);
                    send_to_char(buf, ch);
                    foundspell = TRUE;
                    wrap += 1;  
                }
            }
        }
        if (!foundspell)
            cent_to_char("You have no spells ready to memorize.\n\r", ch);
         
        else if (wrap != 0) send_to_char("\n\r", ch);
        return;
    }


    for (i=0 ; i<MAX_SKILL ; i++)
    {
        if (skill_table[i].name != NULL
            && !str_prefix(argument, skill_table[i].name) && found == -1)
            found = i;
    }
    
    if (found == -1 || skill_table[found].realm == 0)
    {
        send_to_char("You don't know any spells of that name.\n\r", ch);
        return;
    }
    
    if (skill_table[found].spell_fun == spell_null
    || ch->pcdata->learned[found] == 0)
    {
        send_to_char("You don't know any spells of that name.\n\r", ch);
        return;
    }

    level = skill_table[found].level;
    realm = skill_table[found].realm;

    if (realm == 1 && !can_cast_flame(ch) )
    {
	send_to_char("You cannot memorize fire spells.\n\r", ch);
	return;
    }

    if (realm == 2 && !can_cast_water(ch) )
    {
	send_to_char("You cannot memorize water spells.\n\r", ch);
	return;
    }

    if (realm == 3 && !can_cast_air(ch) )
    {
	send_to_char("You cannot memorize air spells.\n\r", ch);
	return;
    }

    if (realm == 4 && !can_cast_earth(ch) )
    {
	send_to_char("You cannot memorize earth spells.\n\r", ch);
	return;
    }

    if (realm == 5 && !can_cast_entropy(ch) )
    {
	send_to_char("You cannot memorize entropy spells.\n\r", ch);
	return;
    }

    for (i = 0 ; i < 11 ; i++)
    {
	if (ch->pcdata->last_memorized[level][i] != -1)
	    spells++;
    }

    if (spells == ch->pcdata->max_memorize[level])
    {
	send_to_char("You can't memorize any more spells of that level.\n\r", ch);
	return;
    }

    for (i = 0 ; i < 11 ; i++)
    {
	if (ch->pcdata->last_memorized[level][i] == -1)
	{
	    ch->pcdata->last_memorized[level][i] = found;
	    sprintf(buf, "You memorize the spell of %s.\n\r", skill_table[found].name);
	    send_to_char(buf, ch);
	    return;
	}
    }
    
    bug("Cannot find a free slot in memorize table.", 0);
    return;
}

void do_unmemorize(CHAR_DATA *ch, char *argument)
{
    int i, j;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
	send_to_char("You have no spells to unmemorize.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Syntax : unmemorize <spell> or unmemorize all\n\r", ch);
	         stc("	        or unmemorize blank to clear your mind of spells.\n\r", ch);	
	return;
    }

    if (!str_cmp("all", argument) ) 
    {
	for (i = 1 ; i < 11 ; i++)
	{
	    for (j = 0 ; j < 11 ; j++)
		ch->pcdata->last_memorized[i][j] = -1;
	}	 	
	send_to_char("You will now not memorize any spells when you next rest.\n\r", ch);
	return;
    }

    if (!str_cmp("blank", argument) ) 
    {
	for (i = 1 ; i < 11 ; i++)
	{
	    for (j = 0 ; j < 11 ; j++)
		ch->pcdata->memorize[i][j] = -1;
	}	 	
	send_to_char("You blank your mind of all your current memorized spells.\n\r", ch);
	return;
    }

    for (i = 1 ; i < 11 ; i++)
    {
	for (j = 0 ; j < 11 ; j++)
	{
	    if (ch->pcdata->last_memorized[i][j] == -1) continue;

    	    if (!str_prefix(argument, skill_table[ch->pcdata->last_memorized[i][j]].name))
	    {
	 	sprintf(buf, "You unmemorize the spell of %s.\n\r", skill_table[ch->pcdata->last_memorized[i][j]].name);
		ch->pcdata->last_memorized[i][j] = -1;
		send_to_char(buf, ch);
	 	return;
 	    }
	}
    }

    send_to_char("You have not got any spells of that name ready for memorization.\n\r", ch);
    return;
}


bool can_cast_flame(CHAR_DATA *ch)
{
    if (IS_NPC(ch)) return TRUE;

    if (is_wearing_artifact(ch, ARTIFACT_FIRE_OPAL) != NULL) return TRUE;

    if (ch->level > 111) return TRUE;

    if (ch->secondaryrace == RACE_VAMPIRE
	&& ch->power[DISC_VAMP_THAU] > 7) return TRUE;

    if (ch->secondaryrace == RACE_DRAGON && 
	(ch->dragtype == DRAGON_RED || ch->dragtype == DRAGON_BLUE || ch->dragtype == DRAGON_GOLD))
	return TRUE;

    if (ch->secondaryrace == 0) return FALSE;

    if (ch->secondaryrace == RACE_DAEMON
	|| ch->secondaryrace == RACE_WARLOCK
	|| ch->secondaryrace == RACE_DEATH_KNIGHT) return TRUE;

    return FALSE;
}

bool can_cast_water(CHAR_DATA *ch)
{
    if (IS_NPC(ch)) return TRUE;

    if (ch->level > 111) return TRUE;

    if (ch->secondaryrace == RACE_DRAGON && 
	(ch->dragtype == DRAGON_WHITE || ch->dragtype == DRAGON_SILVER))
	return TRUE;

    if (ch->secondaryrace == 0) return FALSE;

    if (ch->secondaryrace == RACE_ELADRIN
	|| ch->secondaryrace == RACE_WARLOCK
	|| ch->secondaryrace == RACE_HERO) return TRUE;

    return FALSE;
}

bool can_cast_life(CHAR_DATA *ch)
{
    if (IS_NPC(ch)) return TRUE;

    if (ch->level > 111) return TRUE;

    if (ch->secondaryrace == RACE_DRAGON && 
	(ch->dragtype == DRAGON_GOLD || ch->dragtype == DRAGON_SILVER))
	return TRUE;

    if (ch->secondaryrace == 0) return FALSE;

    if (ch->secondaryrace == RACE_HERO) return TRUE;

    return FALSE;
}

bool can_cast_earth(CHAR_DATA *ch)
{
    if (IS_NPC(ch)) return TRUE;

    if (ch->secondaryrace == RACE_VAMPIRE
	&& ch->power[DISC_VAMP_THAU] > 5) return TRUE;

    if (is_wearing_artifact(ch, ARTIFACT_PTERMIAN_SCARAB) != NULL) return TRUE;

    if (ch->level > 111) return TRUE;

    if (ch->secondaryrace == RACE_DRAGON && 
	(ch->dragtype == DRAGON_GREEN || ch->dragtype == DRAGON_RED))
	return TRUE;

    if (ch->secondaryrace == 0) return FALSE;

    if (ch->secondaryrace == RACE_LUPINE
	&& ch->generation >= 9) return TRUE;

    if (ch->tertiaryrace == TERTIARY_AASIMON) return TRUE;

    if (ch->secondaryrace == RACE_ELADRIN
	|| ch->secondaryrace == RACE_WARLOCK) return TRUE;

    return FALSE;
}

bool can_cast_air(CHAR_DATA *ch)
{
    if (IS_NPC(ch)) return TRUE;

    if (ch->level > 111) return TRUE;

    if (ch->secondaryrace == RACE_VAMPIRE
	&& ch->power[DISC_VAMP_THAU] > 3) return TRUE;

    if (ch->secondaryrace == RACE_DRAGON && 
	(ch->dragtype == DRAGON_GREEN || ch->dragtype == DRAGON_BLUE || ch->dragtype == DRAGON_WHITE))
	return TRUE;

    if (ch->secondaryrace == 0) return FALSE;

    if (ch->tertiaryrace == TERTIARY_DROW) return TRUE;

    if (ch->secondaryrace == RACE_LUPINE
	&& ch->generation >= 9) return TRUE;

    if (ch->secondaryrace == RACE_HERO
	|| ch->secondaryrace == RACE_WARLOCK) return TRUE;

    return FALSE;
}

bool can_cast_entropy(CHAR_DATA *ch)
{
    if (IS_NPC(ch)) return TRUE;

    if (ch->secondaryrace == RACE_DRAGON && ch->dragtype == DRAGON_SHADOW)
	return TRUE;

    if (is_wearing_artifact(ch, ARTIFACT_NECKLACE_BONES) != NULL) return TRUE;

    if (ch->level > 111) return TRUE;

    if (ch->secondaryrace == 0) return FALSE;

    if (ch->tertiaryrace == TERTIARY_DROW) return TRUE;

    if (ch->secondaryrace == RACE_WARLOCK
	|| ch->secondaryrace == RACE_DEATH_KNIGHT
	|| ch->secondaryrace == RACE_VAMPIRE
	|| ch->secondaryrace == RACE_DAEMON) return TRUE;

    return FALSE;
}


/* Gets a mob suitable for a person to kill, randomly */

MOB_INDEX_DATA *get_random_mob(CHAR_DATA *ch)
{
    MOB_INDEX_DATA *mob;
    
    for ( ; ; )
    {
        mob = get_mob_index( number_range( 0, 65535 ) );
	
        if ( mob != NULL
	     && !IS_SET(mob->imm_flags, IMM_WEAPON)
	     && !IS_SET(mob->imm_flags, IMM_MAGIC)
	     && mob->vnum > 500
	     && mob->level < ch->level + 3 ) break;
    }
        
    return mob;
}


/* Gets a pickupable item, randomly in the game */

OBJ_INDEX_DATA *get_random_item( )
{
    OBJ_INDEX_DATA *obj;
    
    for ( ; ; )
    {
        obj = get_obj_index( number_range( 0, 65535 ) );
	
        if ( obj != NULL
	     && obj->vnum > 500
	     && obj->item_type != ITEM_MONEY
	     && !IS_SET(obj->extra_flags, ITEM_MELT_DROP)
	     && !IS_SET(obj->extra_flags, ITEM_NODROP)
	     && CAN_WEAR(obj, ITEM_TAKE) ) break;
    }
        
    return obj;
}

void do_quest(CHAR_DATA *ch, char *argument)
{
    bool finquest;
    char buf[MAX_STRING_LENGTH];

	console("quest 1");
    if (IS_NPC(ch) || ch->pcdata == NULL) return;
	console("quest 2");

    if (ch->pcdata->quests_completed < 0)
	ch->pcdata->quests_completed = 0;
	console("quest 3");

    if (ch->pcdata->quests_completed > 255)
	ch->pcdata->quests_completed = 0;
	console("quest 4");

    if (argument[0] == '\0')
    {
	send_to_char("Syntax : quest [progress start number complete abort]\n\r", ch);
	return;
    }
	console("quest 5");

    if (!str_prefix(argument, "abort"))
    {
	if (!IS_FLAG4(ch, AFF_ON_A_QUEST))
	{
	    send_to_char("You are not currently on a quest.\n\r", ch);
	    return;
  	}
	ch->qp -= 1;
	do_say(ch, "This quest sucks!  I want a new one!");
	stc("Quest aborted.\n\r", ch);
	WAIT_STATE(ch, 8);
	initialise_quest(ch);
	return;
    }
	console("quest 6");

    if (!str_prefix(argument, "progress"))
    {
	if (!IS_FLAG4(ch, AFF_ON_A_QUEST))
	{
	    send_to_char("You are not currently on a quest.\n\r", ch);
	    return;
  	}

	WAIT_STATE(ch, 6);
	do_say(ch, "How am I faring with my quest?");
	act("$n begins talking in strange languages with an unseen entity.", ch, NULL, NULL, TO_ROOM);
	send_to_char("A voice in your mind answers...\n\r", ch);
	finquest = TRUE;

        if (IS_QUEST(ch, QUEST_ITEM_1))
        {
	    sprintf(buf, "You still have to retrieve %s.\n\r", ch->pcdata->quest_item_1);
	    send_to_char(buf, ch);
	    finquest = FALSE;
        }
        if (IS_QUEST(ch, QUEST_ITEM_2))
        {
	    sprintf(buf, "You still have to retrieve %s.\n\r", ch->pcdata->quest_item_2);
	    send_to_char(buf, ch);
	    finquest = FALSE;
        }
        if (IS_QUEST(ch, QUEST_ITEM_3))
        {
	    sprintf(buf, "You still have to retrieve %s.\n\r", ch->pcdata->quest_item_3);
	    send_to_char(buf, ch);
	    finquest = FALSE;
        }

        if (IS_QUEST(ch, QUEST_MOB_1))
        {
	    sprintf(buf, "You still have to kill %s.\n\r", ch->pcdata->quest_mob_1);
	    send_to_char(buf, ch);
	    finquest = FALSE;
        }
        if (IS_QUEST(ch, QUEST_MOB_2))
        {
	    sprintf(buf, "You still have to kill %s.\n\r", ch->pcdata->quest_mob_2);
	    send_to_char(buf, ch);
	    finquest = FALSE;
        }
        if (IS_QUEST(ch, QUEST_MOB_3))
        {
	    sprintf(buf, "You still have to kill %s.\n\r", ch->pcdata->quest_mob_3);
	    send_to_char(buf, ch);
	    finquest = FALSE;
        }

        if (IS_QUEST(ch, QUEST_ROOM_1))
        {
	    sprintf(buf, "You still have to visit %s.\n\r", ch->pcdata->quest_room_1);
	    send_to_char(buf, ch);
	    finquest = FALSE;
        }
        if (IS_QUEST(ch, QUEST_ROOM_2))
        {
	    sprintf(buf, "You still have to visit %s.\n\r", ch->pcdata->quest_room_2);
	    send_to_char(buf, ch);
	    finquest = FALSE;
        }
        if (IS_QUEST(ch, QUEST_ROOM_3))
        {
	    sprintf(buf, "You still have to visit %s.\n\r", ch->pcdata->quest_room_3);
	    send_to_char(buf, ch);
	    finquest = FALSE;
        }
 	if (finquest)
	{
	    send_to_char("'You have completed the current quest.'\n\r", ch);
	    return;
	}
	return;
    }

	console("quest 7");
    if (!str_prefix(argument, "complete"))
    {
	do_say(ch, "I have completed the quest.");
	WAIT_STATE(ch, 6);

        if (!IS_ROOM_STAT2(ch->in_room, ROOM_QUEST_CENTRE))
        {
	    send_to_char("You get no response.\n\r", ch);
	    return;
        }

	if (!IS_FLAG4(ch, AFF_ON_A_QUEST))
	{
	    room_message(ch->in_room, "Something says 'But you aren't on a quest.'");	
	    return;
	}

	if (ch->pcdata->quest_status != 0)
	{
	    room_message(ch->in_room, "Something says 'No you haven't!'");	
	    return;
	}

	room_message(ch->in_room, "Something says 'Congratulations...'");
	ch->qp += 2+dice(1,2);
	ch->hit += ch->max_hit / 2;
	act("White vapours flow from the surrounding air into your body.", ch, NULL, NULL, TO_CHAR);
	act("White vapours flow from the surrounding air into $n's body.", ch, NULL, NULL, TO_ROOM);
	send_to_char("You feel much more powerful.\n\r", ch);
	gain_exp(ch, 500);
	stc("You gained 500 experience points.\n\r", ch);
	REMOVE_BIT(ch->flag4, AFF_ON_A_QUEST);
	ch->pcdata->quests_completed += 1;
	if (ch->pcdata->quests_completed > 10 && dice(1, 3) == 1)
	{
	    send_to_char("An emerald appears in your hand out of nowhere.\n\r", ch);
	    ch->emeralds += 1;
	}
	else if (ch->pcdata->quests_completed > 30 && dice(1, 5) == 1)
	{
	    send_to_char("A sapphire appears in your hand out of nowhere.\n\r", ch);
	    ch->sapphires += 1;
	}
	else if (ch->pcdata->quests_completed > 10 && dice(1, 8) == 1)
	{
	    send_to_char("A couple of sapphires appear in your hand out of nowhere.\n\r", ch);
	    ch->sapphires += 3;
	}

	return;
    }

	console("quest 8");
    if (!str_prefix(argument, "number"))
    {
	console("Chris");
	do_say(ch,"How many quests have I completed?");
	console("Mellish");
	if (ch->pcdata->quests_completed == 1)
	    sprintf(buf, "Something tells you 'You have completed only one quest.'\n\r");
	else sprintf(buf, "Something tells you 'You have completed %d quests.'\n\r",
		ch->pcdata->quests_completed);
	console("Loves");
	
	send_to_char(buf, ch);
	console("Benny");
	return;
    }
	console("quest 9");

    if (!str_prefix(argument, "start"))
    {
	do_say(ch, "I accept your quest.");

        if (!IS_ROOM_STAT2(ch->in_room, ROOM_QUEST_CENTRE))
        {
	    send_to_char("You get no response.\n\r", ch);
	    return;
        }
	if (IS_FLAG4(ch, AFF_ON_A_QUEST))
	{
	    send_to_char("Something tells you 'You are currently on a quest.'\n\r", ch);
	    return;
 	}

	send_to_char("Something tells you 'Good luck....'\n\r", ch);
	WAIT_STATE(ch, 8);
	initialise_quest(ch);
	return;
    }

	console("quest 10");
    do_quest(ch, "");
	console("quest 11");
}

void initialise_quest(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *room;
    int diff = 1;
    int qc = 1;
    char buf[MAX_STRING_LENGTH];

    crash("Initialise quest - place 1");

    if (IS_NPC(ch))
    {
	bug("Initialising quest for NPC.", 0);
	return;
    }

    qc = ch->pcdata->quests_completed;
    buf[0] = '\0';

    if (qc > 20) diff = 2;
    if (qc > 50) diff = 3;
    if (qc > 100) diff = 4;
    if (qc > 200) diff = 5;

    SET_BIT(ch->flag4, AFF_ON_A_QUEST);
    crash("Initialise quest - place 2");

    REMOVE_BIT(ch->pcdata->quest_status, QUEST_ROOM_1);
    REMOVE_BIT(ch->pcdata->quest_status, QUEST_ROOM_2);
    REMOVE_BIT(ch->pcdata->quest_status, QUEST_ROOM_3);
    REMOVE_BIT(ch->pcdata->quest_status, QUEST_MOB_1);
    REMOVE_BIT(ch->pcdata->quest_status, QUEST_MOB_2);
    REMOVE_BIT(ch->pcdata->quest_status, QUEST_MOB_3);
    REMOVE_BIT(ch->pcdata->quest_status, QUEST_ITEM_1);
    REMOVE_BIT(ch->pcdata->quest_status, QUEST_ITEM_2);
    REMOVE_BIT(ch->pcdata->quest_status, QUEST_ITEM_3);
 
    ch->pcdata->quest_room_1 = NULL;
    ch->pcdata->quest_room_2 = NULL;
    ch->pcdata->quest_room_3 = NULL;
    ch->pcdata->quest_item_1 = NULL;
    ch->pcdata->quest_item_2 = NULL;
    ch->pcdata->quest_item_3 = NULL;
    ch->pcdata->quest_mob_1 = NULL;
    ch->pcdata->quest_mob_2 = NULL;
    ch->pcdata->quest_mob_3 = NULL;

    crash("Initialise quest - place 3");
    if (dice(1,2) <= diff)
    {
	SET_BIT(ch->pcdata->quest_status, QUEST_ROOM_1);
	free_string(ch->pcdata->quest_room_1);

	if (ch->secondaryrace != RACE_ELADRIN)
	    room = get_rand_room();
	else room = get_treemeld_room();

	sprintf(buf, "%s", room->name);
	ch->pcdata->quest_room_1 = str_dup(buf);
    }
    if (dice(1,4) <= diff)
    {
	SET_BIT(ch->pcdata->quest_status, QUEST_ROOM_2);
	free_string(ch->pcdata->quest_room_2);

	if (ch->secondaryrace != RACE_ELADRIN)
	    room = get_rand_room();
	else room = get_treemeld_room();
    crash("Initialise quest - place 4");

	sprintf(buf, "%s", room->name);
	ch->pcdata->quest_room_2 = str_dup(buf);
    }
    if (dice(1,6) <= diff)
    {
	SET_BIT(ch->pcdata->quest_status, QUEST_ROOM_3);
	free_string(ch->pcdata->quest_room_3);

	if (ch->secondaryrace != RACE_ELADRIN)
	    room = get_rand_room();
	else room = get_treemeld_room();
    crash("Initialise quest - place 5");

	sprintf(buf, "%s", room->name);
	ch->pcdata->quest_room_3 = str_dup(buf);
    }
    if (dice(1,2) <= diff)
    {
	MOB_INDEX_DATA *mob;

	SET_BIT(ch->pcdata->quest_status, QUEST_MOB_1);
	free_string(ch->pcdata->quest_mob_1);

	mob = get_random_mob(ch);

    crash("Initialise quest - place 6");
	sprintf(buf, "%s", mob->short_descr);
	ch->pcdata->quest_mob_1 = str_dup(buf);
    }
    if (dice(1,3) <= diff)
    {
	MOB_INDEX_DATA *mob;

	SET_BIT(ch->pcdata->quest_status, QUEST_MOB_2);
	free_string(ch->pcdata->quest_mob_2);

	mob = get_random_mob(ch);

	sprintf(buf, "%s", mob->short_descr);
	ch->pcdata->quest_mob_2 = str_dup(buf);
    }
    if (dice(1,5) <= diff)
    {
	MOB_INDEX_DATA *mob;

	SET_BIT(ch->pcdata->quest_status, QUEST_MOB_3);
	free_string(ch->pcdata->quest_mob_3);

	mob = get_random_mob(ch);
    crash("Initialise quest - place 7");

	sprintf(buf, "%s", mob->short_descr);
	ch->pcdata->quest_mob_3 = str_dup(buf);
    }
    if (1 <= diff)
    {
	OBJ_INDEX_DATA *obj;

	SET_BIT(ch->pcdata->quest_status, QUEST_ITEM_1);
	free_string(ch->pcdata->quest_item_1);

	obj = get_random_item();
    crash("Initialise quest - place 8");

	sprintf(buf, "%s", obj->short_descr);
	ch->pcdata->quest_item_1 = str_dup(buf);
    }
    if (dice(1,3) <= diff)
    {
	OBJ_INDEX_DATA *obj;

	SET_BIT(ch->pcdata->quest_status, QUEST_ITEM_2);
	free_string(ch->pcdata->quest_item_2);

	obj = get_random_item();

    crash("Initialise quest - place 9");
	sprintf(buf, "%s", obj->short_descr);
	ch->pcdata->quest_item_2 = str_dup(buf);
    }
    if (dice(1,7) <= diff)
    {
	OBJ_INDEX_DATA *obj;

	SET_BIT(ch->pcdata->quest_status, QUEST_ITEM_3);
	free_string(ch->pcdata->quest_item_3);

	obj = get_random_item();
    crash("Initialise quest - place 10");

	sprintf(buf, "%s", obj->short_descr);
	ch->pcdata->quest_item_3 = str_dup(buf);
    }
}

void room_message(ROOM_INDEX_DATA *room, char *message)
{
    CHAR_DATA *rch;
    crash("roommessage");

    if ( (rch = room->people) == NULL) return;

    act(message, rch, NULL, NULL, TO_ROOM);
    act(message, rch, NULL, NULL, TO_CHAR);
}

void put_wall(ROOM_INDEX_DATA *room, int dir, int wall)
{
    int timer;

    timer = dice(2, 2);

    switch (dir)
    {
	case DIR_NORTH :
	    SET_RTIMER(room, RTIMER_WALL_NORTH, timer);
	    SET_RTIMER(room->exit[dir]->u1.to_room, RTIMER_WALL_SOUTH, timer);
	break;
	case DIR_EAST :
	    SET_RTIMER(room, RTIMER_WALL_EAST, timer);
	    SET_RTIMER(room->exit[dir]->u1.to_room, RTIMER_WALL_WEST, timer);
	break;
	case DIR_SOUTH :
	    SET_RTIMER(room, RTIMER_WALL_SOUTH, timer);
	    SET_RTIMER(room->exit[dir]->u1.to_room, RTIMER_WALL_NORTH, timer);
	break;
	case DIR_WEST :
	    SET_RTIMER(room, RTIMER_WALL_WEST, timer);
	    SET_RTIMER(room->exit[dir]->u1.to_room, RTIMER_WALL_EAST, timer);
	break;
	case DIR_UP :
	    SET_RTIMER(room, RTIMER_WALL_UP, timer);
	    SET_RTIMER(room->exit[dir]->u1.to_room, RTIMER_WALL_DOWN, timer);
	break;
	case DIR_DOWN :
	    SET_RTIMER(room, RTIMER_WALL_DOWN, timer);
	    SET_RTIMER(room->exit[dir]->u1.to_room, RTIMER_WALL_UP, timer);
	break;
    }	
    room->exit[dir]->exit_info = wall;

}

void make_wall(ROOM_INDEX_DATA *room, int dir, int wall)
{
    int was;
    bool makewall = TRUE;

    if (dir < 0 || dir > 5)
    {
	bug("Invalid direction (create wall)", 0);
	return;
    }

    if (room->exit[dir] == NULL)
    {
	bug ("Cant make wall!!!!", 0);
	return;
    }

    was = room->exit[dir]->exit_info;

    if (was == EX_ICE_WALL && wall == EX_FIRE_WALL)
    {
        room_message(room, "The wall of ice bursts into flames.");
        room_message(room->exit[dir]->u1.to_room, "The wall of ice bursts into flames.");
    }

    if (was == EX_FIRE_WALL && wall == EX_ICE_WALL)
    {
        room_message(room, "The wall of flames flickers blue for a second.");
        room_message(room->exit[dir]->u1.to_room, "The wall of flames flickers blue for a second.");
	makewall = FALSE;
    }

    if (makewall) put_wall(room, dir, wall);

    if (room->exit[dir]->u1.to_room == NULL)
    {
	bug("Exit doesnt lead anywhere (create wall).", 0);
	return;
    }

    if (room->exit[dir]->u1.to_room->exit[revdir(dir)] == NULL)
    {
	bug("Reverse Exit doesnt lead anywhere (create wall).", 0);
	return;
    }

    if (makewall) put_wall(room->exit[dir]->u1.to_room, revdir(dir), wall);
}

void do_frostbreath (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
 
    if (!TIME_UP(ch, TIMER_CAN_BREATHE_FROST))
    {
	send_to_char("You are out of breath.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Who do you wish to breathe on?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);
    SET_TIMER(ch, TIMER_CAN_BREATHE_FROST, 9 - ch->generation / 2);
    frost_breath(ch, victim, FALSE);
}

void do_summon (CHAR_DATA *ch, char *argument)
{
    int door;

    if (ch->in_room->sector_type == SECT_AIR)
    {
	send_to_char("This ability cannot be used in the air.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_HELLFIRE_SUMMON)
	&& ch->level < 120)
    {
	send_to_char("This ability can only be used once every 12 hours.\n\r", ch);
	return;
    }

    for (door = 0 ; door < 4 ; door++)
    {
	if (ch->in_room->exit[door] != NULL)
	    make_wall(ch->in_room, door, EX_FIRE_WALL);
    }

    SET_TIMER(ch, TIMER_HELLFIRE_SUMMON, 12);
}

void do_entomb (CHAR_DATA *ch, char *argument)
{
    int door;

    if (ch->in_room->sector_type == SECT_AIR)
    {
	send_to_char("This ability cannot be used in the air.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_ENTOMB)
	&& ch->level < 120)
    {
	send_to_char("This ability can only be used once every 12 hours.\n\r", ch);
	return;
    }

    for (door = 0 ; door < 4 ; door++)
    {
	if (ch->in_room->exit[door] != NULL)
	    make_wall(ch->in_room, door, EX_ICE_WALL);
    }

    ch->hit = ch->max_hit;
    ch->move = ch->max_move;
    SET_TIMER(ch, TIMER_ENTOMB, 12);

    act("$n calls forth sheets of ice, blocking your path in every direction.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You call forth sheets of ice to entrap your prey.\n\r", ch);
    act("$n melts into a puddle of slime on the floor, which drains away.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You teleport to safety.\n\r", ch);

    char_from_room(ch);
    char_to_room(ch, get_room_index(prime_realm, 666),0);
    act("$n forms out of the ground in front of you.", ch, NULL, NULL, TO_ROOM);
    do_look(ch, "auto");
    WAIT_STATE(ch, 20);
}

void 
do_snowman (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    char buf[MAX_STRING_LENGTH];

    if (has_no_arms(ch))
    {
	stc("You have no arms with which to make a snowman.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_MAKE_SNOWMAN) && ch->level < 120)
    {
	send_to_char("You aren't ready to make another snowman yet.\n\r", ch);
	return;
    }

    mob = create_mobile(get_mob_index(29));

    sprintf(buf, mob->short_descr, GET_PROPER_NAME(ch) );
    free_string(mob->short_descr);
    mob->short_descr = str_dup(buf);
        
    sprintf(buf, mob->name,GET_PROPER_NAME(ch) );
    free_string(mob->name);
    mob->name = str_dup(buf);

    sprintf(buf, "%s%s is standing here.\n\r",GET_PROPER_NAME(ch), GET_EXTENSION(ch) );
    free_string(mob->long_descr);
    mob->long_descr = str_dup(buf);

    free_string(mob->description);
    mob->description = str_dup(ch->description);

    mob->sex = ch->sex;

    char_to_room(mob, ch->in_room, ch->altitude);

    WAIT_STATE(ch, 12);
    SET_TIMER(ch, TIMER_MAKE_SNOWMAN, dice(2,4));
    send_to_char("You fashion a likeness of yourself out of snow.\n\r", ch);
    act("$n fashions a likeness of $mself out of snow.", ch, NULL, NULL, TO_ROOM);
}

void do_stasis (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_FLAG4(ch, AFF_STASIS))
    {
	REMOVE_BIT(ch->flag4, AFF_STASIS);
	send_to_char("You leave your stasis.\n\r", ch);
	SET_TIMER(ch, TIMER_CAN_ENTER_STASIS, 6);
	return;
    }

    if (!TIME_UP(ch, TIMER_CAN_ENTER_STASIS))
    {
	sprintf(buf, "You must wait %d hours before you enter stasis once more.\n\r", TIMER(ch, TIMER_CAN_ENTER_STASIS));
	send_to_char(buf, ch);
	return;
    }
	
    send_to_char("Your body freezes and you enter a state of stasis.\n\r", ch);
    act("$n stops moving and freezes like a statue.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->flag4, AFF_STASIS);
    WAIT_STATE(ch, 12);
}    

void do_gust( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *victim;
    CHAR_DATA       *mount;
    ROOM_INDEX_DATA *to_room = NULL;
    EXIT_DATA       *pexit;
    EXIT_DATA       *pexit_rev;
    char            buf       [MAX_INPUT_LENGTH];
    char            direction [MAX_INPUT_LENGTH];
    char            arg1      [MAX_INPUT_LENGTH];
    char            arg2      [MAX_INPUT_LENGTH];
    int             door;
    int             rev_dir;
    int             dam;
     
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!TIME_UP(ch, TIMER_CAN_GUST) && ch->level < 120)
    {
	send_to_char("You are out of breath.\n\r", ch);
	return;
    }
     
    if ( arg1[0] == '\0' )
    {
        send_to_char("Who do you wish to blow a gust of wind at?\n\r", ch);
        return;
    }
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char("They are not here.\n\r", ch);
        return;
    }
    
    if ( victim == ch )
    {
        send_to_char( "Not on yourself!\n\r", ch );
        return;
    }
     
    if ( is_safe( ch, victim ) )
      return;
     
    if ( (mount = victim->riden_by) != NULL)
    {
        send_to_char("But they have someone on their back!\n\r",ch);
        return;
    }
    else if ( (mount = victim->riding) != NULL)
    {
        send_to_char("But they are riding!\n\r",ch);
        return;
    }

    if ( get_size(victim) > get_size(ch))
    {
        send_to_char("They are too large to affect.\n\r",ch);
        return;
    }
    
    WAIT_STATE( ch, 12 );
    if ( !IS_NPC(ch) && number_percent( )/2 > ch->power[DISC_DAEM_GELU] + ch->generation)
    {
            send_to_char("You huff and puff but they do not budge.\n\r",ch);
	    act("$n blows a gust of cold air into the room.", ch, NULL, NULL, TO_ROOM);
            return;
    }
    SET_TIMER(ch, TIMER_CAN_GUST, dice(1,3)+1);    
    act("You breathe a gust of cold air at $N.", ch, NULL, victim, TO_CHAR);
    act("$n breathes a gust of cold air at $N.", ch, NULL, victim, TO_NOTVICT);
    act("$n breathes a gust of cold air at you.", ch, NULL, victim, TO_VICT);

    rev_dir = 0;
    if ( arg2[0] == '\0' )
        door = number_range(0,3);
    else
    {
        if      ( !str_prefix( arg2, "north" ) )
            door = 0;
        else if ( !str_prefix( arg2, "east" ) )
            door = 1;
        else if ( !str_prefix( arg2, "south" ) )
            door = 2;
        else if ( !str_prefix( arg2, "west" ) )
            door = 3;
        else
        {
            send_to_char("You can only blow people north, south, east or west.\n\r", ch);
            return;
        }
    }
     
    if (door == 0) {sprintf(direction,"north");rev_dir = 2;}
    if (door == 1) {sprintf(direction,"east");rev_dir = 3;}
    if (door == 2) {sprintf(direction,"south");rev_dir = 0;}
    if (door == 3) {sprintf(direction,"west");rev_dir = 1;}
     
    if (( pexit = ch->in_room->exit[door]) == NULL)
    {
        sprintf(buf,"$N flies into the %s wall.", direction);
        act(buf,ch,NULL,victim,TO_NOTVICT);
        sprintf(buf,"$N flies into the %s wall.", direction);
        act(buf,ch,NULL,victim,TO_CHAR);
        sprintf(buf,"You smash into the %s wall.", direction);
        act(buf,ch,NULL,victim,TO_VICT);
        dam = dice(get_size(victim)*3+1, ch->power[DISC_DAEM_GELU] * 3);
	if (!IS_INSUBSTANTIAL(victim))
	    damage_old(ch, victim, dam, 0, DAM_OTHER, FALSE);
        return;
    }
    
    pexit = victim->in_room->exit[door];
    if (IS_SET(pexit->exit_info, EX_CLOSED) &&
        !IS_AFFECTED(victim,AFF_PASS_DOOR ) )
    {
        if (IS_SET(pexit->exit_info, EX_LOCKED))
            REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        if (IS_SET(pexit->exit_info, EX_CLOSED))
            REMOVE_BIT(pexit->exit_info, EX_CLOSED);
        sprintf(buf,"$N is blown into the air and flies %s.", direction);
        act(buf,ch,NULL,victim,TO_NOTVICT);
        sprintf(buf,"$N is blown into the air and flies %s.", direction);
        act(buf,ch,NULL,victim,TO_CHAR);
        sprintf(buf,"You are blown into the air and fly %s.", direction);
        act(buf,ch,NULL,victim,TO_VICT);
        sprintf(buf,"There is a loud crash as $n smashes through the $d.");
        act(buf,victim,NULL,pexit->keyword,TO_ROOM);
     
        if ( ( to_room   = pexit->u1.to_room               ) != NULL
        &&   ( pexit_rev = to_room->exit[rev_dir] ) != NULL
        &&   pexit_rev->u1.to_room == ch->in_room
        &&   pexit_rev->keyword != NULL )
        {
            if (IS_SET(pexit_rev->exit_info, EX_LOCKED))
                REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
            if (IS_SET(pexit_rev->exit_info, EX_CLOSED))
                REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
            if (door == 0) sprintf(direction,"south");
            if (door == 1) sprintf(direction,"west");
            if (door == 2) sprintf(direction,"north");
            if (door == 3) sprintf(direction,"east");
            char_from_room(victim);
            char_to_room(victim,to_room, victim->altitude);
            sprintf(buf,"$n comes smashing in through the %s $d.", direction);
            act(buf,victim,NULL,pexit->keyword,TO_ROOM);
            dam = dice(get_size(victim)*3+1, ch->power[DISC_DAEM_GELU] * 7);
	    if (!IS_INSUBSTANTIAL(victim))
	        damage_old(ch, victim, dam, 0, DAM_OTHER, FALSE);
        }
    }
    else
    {
        sprintf(buf,"$N flies %s.", direction);
        act(buf,ch,NULL,victim,TO_NOTVICT);
        sprintf(buf,"$N flies %s.", direction);   
        act(buf,ch,NULL,victim,TO_CHAR);
        sprintf(buf,"You fly %s.", direction);  
        act(buf,ch,NULL,victim,TO_VICT);
        if (door == 0) sprintf(direction,"south");
        if (door == 1) sprintf(direction,"west");
        if (door == 2) sprintf(direction,"north");
        if (door == 3) sprintf(direction,"east");
	if (!can_pass_wall(victim, pexit, door)) return;
        char_from_room(victim);
        char_to_room(victim,pexit->u1.to_room, victim->altitude);
        sprintf(buf,"$n comes flying in from the %s.", direction);
        act(buf,victim,NULL,NULL,TO_ROOM);
        dam = dice(get_size(victim)*3+1, ch->power[DISC_DAEM_GELU] * 5);
	if (!IS_INSUBSTANTIAL(victim))
	    damage_old(ch, victim, dam, 0, DAM_OTHER, FALSE);
    }
    return;
}

void do_hurl( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *victim;
    CHAR_DATA       *mount;
    ROOM_INDEX_DATA *to_room = NULL;
    EXIT_DATA       *pexit;
    EXIT_DATA       *pexit_rev;
    char            buf       [MAX_INPUT_LENGTH];
    char            direction [MAX_INPUT_LENGTH];
    char            arg1      [MAX_INPUT_LENGTH];
    char            arg2      [MAX_INPUT_LENGTH];
    int             door;
    int             rev_dir;
    int             dam;
     
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (has_no_arms(ch))
    {
	stc("You cannot hurl if you have no arms.\n\r", ch);
	return;
    }

    if (lost_both_legs(ch))
    {
	stc("You cannot stand up.\n\r", ch);
	return;
    }

    if ( arg1[0] == '\0' )
    {
        send_to_char("Who do you wish to hurl?\n\r", ch);
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char("They are not here.\n\r", ch);
        return;
    }

    if (IS_INSUBSTANTIAL(ch))
    {
	stc("You cannot do this while in mist form.\n\r", ch);
	return;
    }
     
    if (IS_INSUBSTANTIAL(victim) )
    {
	send_to_char("You cannot pick up a character in mist form.\n\r", ch);
	return;
    }

    if ( victim == ch )
    {
        send_to_char( "You cannot throw yourself!\n\r", ch );
        return;
    }
     
    if (is_wearing_artifact(victim, ARTIFACT_LODERING) != NULL)
    {
	act("$N's feet do not leave the ground.", ch, NULL, victim, TO_CHAR);
	act("$n tries to pick up $N, but $E is too heavy.", ch, NULL, victim, TO_NOTVICT);
	act("$n tries to pick you up, but your feet do not leave the ground.", ch, NULL, victim, TO_VICT);
	send_to_char("The lodering glows with a soft green light.\n\r", victim);
	return;
    }

    if ( is_safe( ch, victim ) )
      return;
     
    if ( (mount = victim->riden_by) != NULL)
    {
        send_to_char("But they have someone on their back!\n\r",ch);
        return;
    }
    else if ( (mount = victim->riding) != NULL)
    {
        send_to_char("But they are riding!\n\r",ch);
        return;
    }

    if ( get_size(victim) > get_size(ch))
    {
        send_to_char("They are too heavy for you to pick up.\n\r",ch);
	act("$n tries to pick up $N, but $E is too heavy.", ch, NULL, victim, TO_NOTVICT);
	act("$n tries to pick you up, but you are too heavy.", ch, NULL, victim, TO_VICT);
        return;
    }
    
    WAIT_STATE( ch, 12 );

    if ( !IS_NPC(ch) && number_percent( )/2 > get_curr_stat(ch, STAT_STR) )
    {
        send_to_char("They are slightly too heavy for you to pick up.\n\r",ch);
	act("$n tries to pick up $N, but $E is too heavy.", ch, NULL, victim, TO_NOTVICT);
	act("$n tries to pick you up, but you are too heavy.", ch, NULL, victim, TO_VICT);
        return;
    }

    rev_dir = 0;
    if ( arg2[0] == '\0' )
        door = number_range(0,3);
    else
    {
        if      ( !str_prefix( arg2, "north" ) )
            door = 0;
        else if ( !str_prefix( arg2, "east" ) )
            door = 1;
        else if ( !str_prefix( arg2, "south" ) )
            door = 2;
        else if ( !str_prefix( arg2, "west" ) )
            door = 3;
        else
        {
            send_to_char("You can only hurl people north, south, east or west.\n\r", ch);
            return;
        }
    }

    if (lost_one_leg(ch))
    {
	stc("You fall to the floor.\n\r", ch);
	ch->position = POS_SITTING;
    }
     
    if (door == 0) {sprintf(direction,"north");rev_dir = 2;}
    if (door == 1) {sprintf(direction,"east");rev_dir = 3;}
    if (door == 2) {sprintf(direction,"south");rev_dir = 0;}
    if (door == 3) {sprintf(direction,"west");rev_dir = 1;}
     
    if (( pexit = ch->in_room->exit[door]) == NULL)
    {
        sprintf(buf,"$n picks $N up and throws $M into the %s wall.", direction);
        act(buf,ch,NULL,victim,TO_NOTVICT);
        sprintf(buf,"You pick $N up and throw $M into the %s wall.", direction);
        act(buf,ch,NULL,victim,TO_CHAR);
        sprintf(buf,"$n picks you up and throws you into the %s wall.", direction);
        act(buf,ch,NULL,victim,TO_VICT);
        dam = dice(get_size(victim)*3+1, get_curr_stat(ch, STAT_STR) );
	if (!IS_INSUBSTANTIAL(victim))
	    damage_old(ch, victim, dam, 0, DAM_OTHER, FALSE);
        return;
    }
    
    pexit = victim->in_room->exit[door];
    if (IS_SET(pexit->exit_info, EX_CLOSED) &&
        !IS_AFFECTED(victim,AFF_PASS_DOOR ) )
    {
        if (IS_SET(pexit->exit_info, EX_LOCKED))
            REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        if (IS_SET(pexit->exit_info, EX_CLOSED))
            REMOVE_BIT(pexit->exit_info, EX_CLOSED);
        sprintf(buf,"$n picks $N up and throws $M %s.", direction);
        act(buf,ch,NULL,victim,TO_NOTVICT);
        sprintf(buf,"You pick $N up and throw $M %s.", direction);
        act(buf,ch,NULL,victim,TO_CHAR);
        sprintf(buf,"$n picks you up and throws you %s.", direction);
        act(buf,ch,NULL,victim,TO_VICT);
        sprintf(buf,"There is a loud crash as $n smashes through the $d.");
        act(buf,victim,NULL,pexit->keyword,TO_ROOM);
     
        if ( ( to_room   = pexit->u1.to_room               ) != NULL
        &&   ( pexit_rev = to_room->exit[rev_dir] ) != NULL
        &&   pexit_rev->u1.to_room == ch->in_room
        &&   pexit_rev->keyword != NULL )
        {
            if (IS_SET(pexit_rev->exit_info, EX_LOCKED))
                REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
            if (IS_SET(pexit_rev->exit_info, EX_CLOSED))
                REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
            if (door == 0) sprintf(direction,"south");
            if (door == 1) sprintf(direction,"west");
            if (door == 2) sprintf(direction,"north");
            if (door == 3) sprintf(direction,"east");
            char_from_room(victim);
            char_to_room(victim,to_room, victim->altitude);
            sprintf(buf,"$n comes smashing in through the %s $d.", direction);
            act(buf,victim,NULL,pexit->keyword,TO_ROOM);
            dam = dice(get_size(victim)*3+1, ch->power[DISC_DAEM_GELU] * 7);
        }
    }
    else
    {
        sprintf(buf,"$n picks $N up and throws $M %s.", direction);
        act(buf,ch,NULL,victim,TO_NOTVICT);
        sprintf(buf,"You pick $N up and throw $M %s.", direction);   
        act(buf,ch,NULL,victim,TO_CHAR);
        sprintf(buf,"$n picks you up and throws you %s.", direction);  
        act(buf,ch,NULL,victim,TO_VICT);
        if (door == 0) sprintf(direction,"south");
        if (door == 1) sprintf(direction,"west");
        if (door == 2) sprintf(direction,"north");
        if (door == 3) sprintf(direction,"east");
	if (!can_pass_wall(victim, pexit, door)) return;
        char_from_room(victim);
        char_to_room(victim,pexit->u1.to_room, victim->altitude);
        sprintf(buf,"$n comes flying in from the %s.", direction);
        act(buf,victim,NULL,NULL,TO_ROOM);
        dam = dice(get_size(victim)*3+1, ch->power[DISC_DAEM_GELU] * 5);
    }
    return;
}


void do_shard(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];

    if (!TIME_UP(ch, TIMER_CAN_CREATE_SHARD))
    {
	send_to_char("You failed to create an ice dagger.\n\r", ch);
	return;
    }

    SET_TIMER(ch, TIMER_CAN_CREATE_SHARD, 6);
    WAIT_STATE(ch, 8);

    obj=create_object(get_obj_index(94), 0);
    obj_to_char(obj, ch);

    obj->value[1] = ch->generation;
    obj->value[2] = ch->power[DISC_DAEM_GELU];
    send_to_char("Water droplets from your skin freeze into an icicle.\n\r", ch);
    act("Water dripping from $n's skin forms into an icicle.", ch, NULL, NULL, TO_ROOM);
    obj->size=get_size(ch);

    sprintf(buf, obj->short_descr, GET_PROPER_NAME(ch) );
    free_string(obj->short_descr);
    obj->short_descr = str_dup(buf);
 
    sprintf(buf, obj->name, GET_PROPER_NAME(ch) );
    free_string(obj->name);
    obj->name = str_dup(buf);
    SET_BIT(obj->extra_flags2, ITEM_ICE);
}

void do_itemhide(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    CHAR_DATA *mob;

    if (argument[0] == '\0')
    {
	send_to_char("Which item do you wish to hide?\n\r", ch);
	return;
    }

    if ( (obj = get_obj_carry(ch, argument, ch)) == NULL)
    {
	send_to_char("You are not carrying that item.\n\r", ch);
	return;
    }

    if (dice(1, 100) == 1)
    {
	mob = create_mobile(get_mob_index(VNUM_HELL_DEMON));
	char_to_room(mob, ch->in_room, ch->altitude);
	act("$p explodes and a hell demon appears.", ch,obj, NULL, TO_ROOM);
	act("$p explodes and a hell demon appears.", ch,obj, NULL, TO_CHAR);
	extract_obj(obj);
	set_fighting(mob, ch);
	return;
    }

    if (IS_OBJ_STAT2(obj, ITEM_ITEMHIDE))
    {
	act("$p is already affected.", ch, obj, NULL, TO_CHAR);
	return;
    }

    act("$p fades out of existence.", ch, obj, NULL, TO_CHAR);
    act("$p, carried by $n fades out of existence.", ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, 14);
    SET_BIT(obj->extra_flags2, ITEM_ITEMHIDE);
}


void do_flicker(CHAR_DATA *ch, char *argument)
{
    if (IS_FLAG4(ch, AFF_FLICKER))
    {
	send_to_char("You stop flickering in and out of existence.\n\r", ch);
	REMOVE_BIT(ch->flag4, AFF_FLICKER);
	change_upkeep(ch, -6);
	return;
    }
    WAIT_STATE(ch, 8);
    SET_BIT(ch->flag4, AFF_FLICKER);
    send_to_char("Your body starts to flicker on and off.\n\r", ch);
    change_upkeep(ch, 6);
}


void do_immolate(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    CHAR_DATA *mob;

    if (argument[0] == '\0')
    {
	send_to_char("Which item do you wish to immolate?\n\r", ch);
	return;
    }

    if ( (obj = get_obj_carry(ch, argument, ch)) == NULL)
    {
	send_to_char("You are not carrying that item.\n\r", ch);
	return;
    }

    if (obj->item_type != ITEM_WEAPON)
    {
	send_to_char("That is not a weapon!\n\r", ch);
	return;
    }

    if (dice(1, 100) == 1)
    {
	mob = create_mobile(get_mob_index(VNUM_HELL_DEMON));
	char_to_room(mob, ch->in_room, ch->altitude);
	act("$p explodes and a hell demon appears.", ch,obj, NULL, TO_ROOM);
	act("$p explodes and a hell demon appears.", ch,obj, NULL, TO_CHAR);
	extract_obj(obj);
	set_fighting(mob, ch);
	return;
    }

    if (!str_cmp(obj->material, "wood"))
    {
	act("$p ignites and burns to a crisp.", ch, obj , NULL, TO_CHAR);
	act("$p, carried by $n ignites and burns to a crisp.", ch, obj , NULL, TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (IS_WEAPON_STAT(obj, WEAPON_FLAMING))
    {
	act("$p is already flaming.", ch, obj, NULL, TO_CHAR);
	return;
    }

    act("$p bursts into flames.", ch, obj, NULL, TO_CHAR);
    act("$p, carried by $n bursts into flames.", ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, 8);
    SET_BIT(obj->value[4], WEAPON_FLAMING);
}

void do_fly(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (ch->cur_form != FRM_HAWK)
    {
	send_to_char("You must be in hawk form to fly.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Who do you wish to fly to?\n\r", ch);
	return;
    }

    if (ch->move < ch->max_move)
    {
	send_to_char("You must be at maximum vitality to use this power.\n\r", ch);
	return;
    }

    if (ch->in_room->sector_type == SECT_INSIDE)
    {
	send_to_char("*OUCH* You fly into the ceiling.\n\r",ch);
	WAIT_STATE(ch, 4);
	return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL)
    {
	send_to_char("Nobody of that name exists.\n\r",ch);
	return;
    }

    if (ch->in_room == victim->in_room)
    {
	send_to_char("You are already here.\n\r", ch);
	return;
    }

    if (victim->in_room == NULL)
    {
	send_to_char("You do not know where to fly to.\n\r", ch);
	return;
    }

    if (victim->in_room->sector_type == SECT_INSIDE)
    {
	send_to_char("You cannot fly to somebody who is inside a building.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 8);
    send_to_char("You soar upwards into the air.\n\r", ch);
    act("$n soars up into the air and is gone.", ch, NULL, NULL, TO_ROOM);
    REMOVE_BIT(ch->flag4, AFF_LANDED);
    char_from_room(ch);
    char_to_room(ch, victim->in_room, victim->altitude);
    act("You land before $N and change back into your human form.", ch, NULL, victim, TO_CHAR);
    act("$n swoops down and lands before you.", ch, NULL, victim, TO_VICT);
    act("$n swoops down and lands before $N.", ch, NULL, victim, TO_NOTVICT);
    set_form(ch, get_normal_form(ch));
    ch->move = ch->move / 2;
}

void do_hawkform (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (ch->cur_form == FRM_HAWK)
    {
	send_to_char("You change back into human form.\n\r", ch);
	act("$n changes back into humanoid form.", ch, NULL, NULL, TO_ROOM);
	set_form(ch, get_normal_form(ch));
	return;
    }

    if (!TIME_UP(ch, TIMER_CAN_CHANGE_HAWK))
    {
	sprintf(buf, "You must wait %d hours to do this again.\n\r", TIMER(ch, TIMER_CAN_CHANGE_HAWK));
	send_to_char(buf, ch);
	return;
    }

    SET_TIMER(ch, TIMER_CAN_CHANGE_HAWK, 18-ch->generation);
    set_form(ch, FRM_HAWK);
    act("$n starts to grow wings, and transforms into a hawk.", ch, NULL, NULL, TO_ROOM);
    send_to_char("Huge wings sprout from your back, and you shrink until you have become a hawk.\n\r", ch);
}


void do_phase (CHAR_DATA *ch, char *argument)
{
    if (IS_FLAG4(ch, AFF_PHASING))
    {
	send_to_char("You stop phasing and return to normal.\n\r", ch);
	REMOVE_BIT(ch->flag4, AFF_PHASING);
	change_upkeep(ch, -4);
	return;
    }
    SET_BIT(ch->flag4, AFF_PHASING);
    send_to_char("The molecules in your body shift out of phase.\n\r", ch);
    change_upkeep(ch, 4);
}


void do_stake (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *wield, *obj;
    CHAR_DATA *victim;
    int i;

    if (argument[0] == '\0')
    {
	send_to_char("Who do you wish to stake?\n\r", ch);
	return;
    }

    wield = get_obj_by_stat2(ch, ITEM_STAKE);

    if (wield == NULL)
    {
	send_to_char("You are not holding a stake.\n\r", ch);
	return;
    }

    if (!IS_OBJ_STAT2(wield, ITEM_STAKE))
    {
	send_to_char("You are not holding a stake.\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim->secondaryrace != RACE_VAMPIRE)
    {
	send_to_char("But you can only stake a vampire.\n\r", ch);
	return;
    }

    if (IS_INSUBSTANTIAL(victim))
    {
	send_to_char("But they are in mist form.\n\r", ch);
	return;
    }

    if (IS_IMMORTAL(victim))
    {
	send_to_char("The stake melts on contact.\n\r", ch);
	extract_obj(wield);
	return;
    }

    if (victim->position > POS_MORTAL)
    {
	send_to_char("You can only do this to a mortally wounded vampire.\n\r", ch);	
	return;
    }

    send_to_char("A stake is driven into your heart.  You crumble to dust.\n\r", victim);
    act("$n plunges a stake deep into $N's heart, and $N crumbles into dust.", ch, NULL, victim, TO_NOTVICT);
    act("You plunge a stake into $N's heart, and $E crumbles into dust.", ch, NULL, victim, TO_CHAR);

    if (victim->level > 16) victim->level = 16;
    WAIT_STATE(ch, 12);
    WAIT_STATE(victim, 12);
    for ( i = 0 ; i < MAX_DISCIPLINES ; i++ )
	victim->power[i] = -2;
    victim->secondaryrace = 0;
    victim->tertiaryrace = 0;
    declaw(victim);
    defang(victim);
    set_form(victim, 0);
    victim->clan = 0;
    obj = create_object(get_obj_index(67), 0);
    obj_to_room(obj, victim->in_room, victim->altitude);
    destroy_char(ch, victim , FALSE);
}

void do_callrock (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if (argument[0] == '\0')
    {
	send_to_char("Who do you wish to call rocks against?\n\r", ch);
	return;
    }

    if (!IS_OUTSIDE(ch) && ch->in_room->sector_type != SECT_FOREST)
    { 
	send_to_char("You must be outside to call rocks.\n\r", ch);
	return;
    }
    
    if (!TIME_UP(ch, TIMER_CAN_CALL_ROCKS) )
    {
	send_to_char("You call out but nothing comes.\n\r", ch);
	return;
    }

    act("$n cries out and soon boulders and rocks roll into the room to help.", ch, NULL, NULL,TO_ROOM);
    send_to_char("Rocks and boulders roll into the room to help.\n\r",ch);

    obj = create_object(get_obj_index(89+dice(1, 2)), 0);
    obj_to_room(obj, ch->in_room, ch->altitude);
    obj->timer = ch->generation / 3 + 1;
    SET_BIT(obj->extra_flags2, ITEM_ATTACK_NON_ELAD);
    SET_TIMER(ch, TIMER_CAN_CALL_ROCKS, 3);

    if (ch->generation > 5)
    {
        obj = create_object(get_obj_index(89+dice(1, 2)), 0);
        obj_to_room(obj, ch->in_room, ch->altitude);
        obj->timer = ch->generation / 3 + 1;
	SET_BIT(obj->extra_flags2, ITEM_ATTACK_NON_ELAD);
    }

    if (ch->generation > 10)
    {
        obj = create_object(get_obj_index(89+dice(1, 2)), 0);
        obj_to_room(obj, ch->in_room, ch->altitude);
        obj->timer = ch->generation / 3 + 1;
	SET_BIT(obj->extra_flags2, ITEM_ATTACK_NON_ELAD);
    }
}

void do_preserve (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if (argument[0] == '\0')
    {
	send_to_char("Which object do you wish to preserve?\n\r", ch);
	return;
    }

    if ((obj = get_obj_carry(ch, argument,ch) ) == NULL)
    {
	send_to_char("You do not have that item in your inventory.\n\r",ch);
	return;
    }

    if (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && ch->alignment > 300)
    {
	send_to_char("You cannot affect such an evil item.\n\r", ch);
	return;
    }

    if (IS_OBJ_STAT(obj, ITEM_ARTIFACT)
	|| IS_OBJ_STAT(obj, ITEM_RELIC) )
    {
	send_to_char("You cannot affect such a powerful item.\n\r", ch);
	return;
    }

    if (obj->item_type == ITEM_WEAPON
	&& obj->timer != -1)
    {
	act("$p disappears in a cloud of purple smoke.", ch, obj, NULL, TO_CHAR);
	extract_obj(obj);
	return;
    }

    act("You have succesfully preserved $p.", ch, obj, NULL, TO_CHAR);
    act("$n lays $s hands on $p.", ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, 12);
    obj->timer = -1;
}

void do_seed(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if (argument[0] == '\0')
    {
	send_to_char("Which item do you wish to plant with Deamonseed?\n\r", ch);
	return;
    }

    if ((obj = get_obj_carry(ch, argument,ch) ) == NULL)
    {
	send_to_char("You do not have that item in your inventory.\n\r",ch);
	return;
    }
  
    if (IS_OBJ_STAT2(obj, ITEM_DAEMONSEED) )
    {
	send_to_char("That item is already set to explode.\n\r", ch);
	return;
    }

    if (IS_OBJ_STAT(obj, ITEM_BLESS)
	|| IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) )
    {
	send_to_char("You cannot affect such a holy item.\n\r", ch);
	return;
    }

    if (IS_OBJ_STAT(obj, ITEM_ARTIFACT)
	|| IS_OBJ_STAT(obj, ITEM_RELIC) )
    {
	send_to_char("You cannot affect such a powerful item.\n\r", ch);
	return;
    }

    act("$p starts to shudder and vibrate in your hands.", ch, obj, NULL, TO_CHAR);
    act("$n lays $s hands on $p.", ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, 12);
    SET_BIT(obj->extra_flags2, ITEM_DAEMONSEED);
    obj->timer = ch->generation / 2 + dice(1,3);
}


void do_forage(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    int chance;

    if (IS_NPC(ch))
    {
	send_to_char("You fail to find anything interesting.\n\r", ch);
	return;
    }

    chance = get_skill(ch, gsn_foraging);

    if (ch->in_room->sector_type == SECT_INSIDE)
    {
	send_to_char("You cannot forage while inside.\n\r", ch);
	return;
    }

    if (ch->in_room->sector_type == SECT_CITY)
    {
	send_to_char("You cannot forage while in a city.\n\r", ch);
	return;
    }

    if (ch->in_room->sector_type == SECT_AIR)
    {
	send_to_char("You cannot forage while flying in the air.\n\r",ch);
	return;
    }

    if (ch->in_room->sector_type == SECT_DESERT)
    {
	send_to_char("You cannot see anything for miles around.\n\r", ch);
	return;
    }

    if (ch->in_room->sector_type == SECT_WATER_SWIM
	|| ch->in_room->sector_type == SECT_WATER_NOSWIM
	|| ch->in_room->sector_type == SECT_UNDERWATER)
    {
	if (!IS_NPC(ch) && ch->pcdata->voodoo_points > 5
	    && ch->in_room->sector_type == SECT_WATER_SWIM)
	{
	    send_to_char("You find some wormwart floating by.\n\r", ch);
	    act("$n takes some wormwart $e sees floating by.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(12004), 0);
	    obj_to_char(obj, ch);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_WORMWART);
	    return;
	}
	else if (!IS_NPC(ch) && ch->pcdata->voodoo_points > 5
	    && ch->in_room->sector_type == SECT_UNDERWATER)
	{
	    send_to_char("You pluck some scullyweed from the sea bed.\n\r", ch);
	    act("$n plucks some scullyweed from the sea bed.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(12005), 0);
	    obj_to_char(obj, ch);
	    obj->timer = dice (3, 20);
	    SET_BIT(obj->extra_flags2, ITEM_DAEMONSEED);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_SCULLYWEED);
	    return;
	}
	send_to_char("You cannot forage while in the water.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch,TIMER_FORAGE)&& ch->level < 120)
    {
	send_to_char("You fail to find anything interesting.\n\r", ch);
	return;
    }

    if (dice(1, 100) > chance)
    {
	send_to_char("You fail to find anything interesting.\n\r", ch);
	SET_TIMER(ch, TIMER_FORAGE, 6);
	check_improve(ch, gsn_foraging, FALSE, 0);
	return;
    }

    WAIT_STATE(ch, 12);
    SET_TIMER(ch, TIMER_FORAGE, 4);

    switch (dice(1,17) )
    {
	case 1 :
	    send_to_char("You find some wild berries growing nearby.\n\r", ch);
	    act("$n scavenges around and finds some wild berries to eat.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(78), 0);
	    obj_to_char(obj, ch);
	    return;
	case 2 :
	    send_to_char("You kill and cook a nearby rabbit.\n\r", ch);
	    act("$n goes into the bushes and returns with a rabbit to eat.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(79), 0);
	    obj_to_char(obj, ch);
	    return;
	case 3 :
	    send_to_char("You pick some wild fruits growing nearby.\n\r", ch);
	    act("$n scavenges around and finds some wild fruits to eat.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(80), 0);
	    obj_to_char(obj, ch);
	    return;
	case 4 :
	    send_to_char("You kill and cook a passing deer for food.\n\r", ch);
	    act("$n hunts and kills a passing deer, and for food.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(82), 0);
	    obj_to_char(obj, ch);
	    obj = create_object(get_obj_index(82), 0);
	    obj_to_char(obj, ch);
	    obj = create_object(get_obj_index(82), 0);
	    obj_to_char(obj, ch);
	    return;
	case 5 :
	    send_to_char("You find some pebbles on the floor.\n\r", ch);
	    act("$n picks up some pebbles lying on the floor.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(83), 0);
	    obj_to_char(obj, ch);
	    return;
	case 6 :
	    send_to_char("You find a small firefly buzzing around a nearby tree.\n\r", ch);
	    act("$n finds a small firefly on a nearby tree.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(84), 0);
	    obj_to_char(obj, ch);
	    return;
	case 7 :
	    send_to_char("You find some poppy seeds on the floor.\n\r", ch);
	    act("$n scavenges around and finds some poppy seeds.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(85), 0);
	    obj_to_char(obj, ch);
	    return;
	case 8 :
	    send_to_char("You find some sand lying on the floor.\n\r", ch);
	    act("$n scavenges around and finds some sand on the floor.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(86), 0);
	    obj_to_char(obj, ch);
	    return;
	case 9 :
	    send_to_char("You find some juicy leaves on a nearby plant.\n\r", ch);
	    act("$n scavenges around and finds some juicy leaves to eat.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(81), 0);
	    obj_to_char(obj, ch);
	    obj = create_object(get_obj_index(81), 0);
	    obj_to_char(obj, ch);
	    return;
	case 10 :
	    send_to_char("You break off a stick from a nearby tree.\n\r", ch);
	    act("$n breaks off a stick from a nearby tree.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(52), 0);
	    obj_to_char(obj, ch);
	    return;
	case 11 :
	    send_to_char("You break off a branch from a nearby tree.\n\r", ch);
	    act("$n breaks off a branch from a nearby tree.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(53), 0);
	    obj_to_char(obj, ch);
	    return;
	case 12 :
	    if (ch->in_room->sector_type != SECT_FOREST)
	    {
		send_to_char("You fail to find anything interesting.\n\r", ch);
		return;
	    }
	    send_to_char("You pick up a few cloves of bajuju from the forest floor.\n\r", ch);
	    act("$n picks up a few cloves of bajuju from the forest floor.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(12002), 0);
	    obj_to_char(obj, ch);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_BAJUJU);
	    obj = create_object(get_obj_index(12002), 0);
	    obj_to_char(obj, ch);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_BAJUJU);
	    return;
	case 13 :
	    if (ch->in_room->sector_type != SECT_HILLS)
	    {
		send_to_char("You fail to find anything interesting.\n\r", ch);
		return;
	    }
	    send_to_char("You find a single muldaleaf growing on the grass.\n\r", ch);
	    act("$n picks a single muldaleaf from the floor.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(12003), 0);
	    obj_to_char(obj, ch);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_MULDALEAF);
	    return;
	case 14 :
	    if (ch->in_room->sector_type != SECT_FIELD)
	    {
		send_to_char("You fail to find anything interesting.\n\r", ch);
		return;
	    }
	    send_to_char("You pick up a few strange herbs growing in the field.\n\r", ch);
	    act("$n picks up a few strange herbs from the floor.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(12006), 0);
	    obj_to_char(obj, ch);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_FROGBREATH);
	    obj = create_object(get_obj_index(12006), 0);
	    obj_to_char(obj, ch);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_FROGBREATH);
	    obj = create_object(get_obj_index(12006), 0);
	    obj_to_char(obj, ch);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_FROGBREATH);
	    return;
	case 15 :
	    if (ch->in_room->sector_type != SECT_HILLS)
	    {
		send_to_char("You fail to find anything interesting.\n\r", ch);
		return;
	    }
	    send_to_char("You scrape some nice looking fungi from a nearby plant.\n\r", ch);
	    act("$n scrapes some nice looking fungi from a nearby plant.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(12007), 0);
	    obj_to_char(obj, ch);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_TILLIFREEN);
	    obj = create_object(get_obj_index(12007), 0);
	    obj_to_char(obj, ch);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_TILLIFREEN);
	    return;
	case 16 :
	    if (ch->in_room->sector_type != SECT_FOREST)
	    {
		send_to_char("You fail to find anything interesting.\n\r", ch);
		return;
	    }
	    send_to_char("You cut some plant tendrils from a nearby tree.\n\r", ch);
	    act("$n cuts some nearby plant tendrils from a tree.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(12008), 0);
	    obj_to_char(obj, ch);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_PAPACOCO);
	    obj = create_object(get_obj_index(12008), 0);
	    obj_to_char(obj, ch);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_PAPACOCO);
	    return;
	case 17 :
	    if (ch->in_room->sector_type != SECT_FOREST)
	    {
		send_to_char("You fail to find anything interesting.\n\r", ch);
		return;
	    }
	    send_to_char("You find some strange yellow ooze on the floor.\n\r", ch);
	    act("$n gets some strange yellow ooze that $e saw on the floor.", ch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(12009), 0);
	    obj_to_char(obj, ch);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_GLOIN);
	    obj = create_object(get_obj_index(12009), 0);
	    obj_to_char(obj, ch);
	    SET_BIT(obj->extra_flags2, ITEM_INGRED_GLOIN);
	    return;
	
        default : return;
    }
}


void do_moon(CHAR_DATA *ch, char *argument)
{
    int day, percent_full, percent_across, i, j;
    char buf[MAX_STRING_LENGTH];
/*    static const sh_int triangle[] = { 1,3,5,7,9,11,13,15,17,19};
    static const sh_int squar[] = {14,14,14,14,14,14,14,14,14,14};
    static const sh_int oval[] = {3,11,17,21,26,26,21,17,11,3}; */
    static const sh_int phase_length[] = {3,9,13,15,17,17,15,13,9,3};


/*  if (!str_prefix(argument, "triangle") && argument[0] != '\0')
    {
        for (i = 0; i < 10; i++)
	{
	    phase_length[i] = triangle[i];
	}
    }
    else if (!str_prefix(argument, "square") && argument[0] != '\0')
    {
        for (i=0; i<10; i++)
	    phase_length[i] = squar[i];
    }
    else if (!str_prefix(argument, "oval") && argument[0] != '\0')
    {
        for (i=0; i<10; i++)
	    phase_length[i] = oval[i];
    }*/

    buf[0] = '\0';



    if ( !IS_OUTSIDE(ch) )
    {
        send_to_char( "You can't see the moon indoors.\n\r", ch );
        return;
    }

    if (time_info.hour > 5 && time_info.hour < 19)
    {
	send_to_char("But its daylight, you can't see the moon.\n\r", ch);
	return;
    }

    for ( i=0; i < 10; i++)
    {
	if (time_info.day > 17)
	{
	    day = time_info.day - 17;
	    percent_full = day * 100 / 17;
	    for (j=phase_length[i];j>0;j--)
	    {
	        percent_across = j * 100 /phase_length[i];
		if (percent_across>percent_full)
		    strcat(buf, ".");
		else
		    strcat(buf, "O");
	    }
        }
	else
	{
	    day = time_info.day;
	    percent_full = (17-day) * 100 / 17;
	    for (j=0;j<phase_length[i];j++)
	    {
	        percent_across = j * 100 / phase_length[i];
		if (percent_across>percent_full)
		    strcat(buf, ".");
		else
		    strcat(buf, "O");
	    }
	}
	cent_to_char(buf, ch);
	buf[0] ='\0';
    }
}


void do_transform(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj = NULL;
    char buf[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
    {
        send_to_char("You can transform into the following items:\n\r",ch);
        send_to_char("    A Daemonic Helm\n\r",ch);
        if (ch->power[DISC_DAEM_MORP] > 2)
            send_to_char("    Black Steel Greaves\n\r",ch);
        if (ch->power[DISC_DAEM_MORP] > 4)
	{
            send_to_char("    A Black Opera Cloak\n\r",ch);
            send_to_char("    Daemonic Plate Armor\n\r",ch);
	}
        if (ch->power[DISC_DAEM_MORP] > 6)
	{
            send_to_char("    Ring of Cacophony\n\r",ch);
            send_to_char("    Blood-Ruby Sword\n\r",ch);
	}
        if (ch->power[DISC_DAEM_MORP] > 8)
            send_to_char("    Shield of Bone\n\r",ch);
        return;
    }

    if (IS_NPC(ch) )
    {
	send_to_char("Not while switched as a mob.\n\r", ch);
	return;
    }

    if (!str_cmp(argument, "back") )
    {
	if (!is_item(ch))
	{
	    stc("You are not in item form at the moment.\n\r", ch);
	    return;
	}
	
	if (IS_OBJ_STAT3(ch->sw_obj, ITEM_CANNOT_RETURN))
        {
	    send_to_char("Some mysterious force prevents you from transforming back.\n\r", ch);
	    return;
	}

	sprintf(buf, "$p suddenly transforms back into %s.", GET_PROPER_NAME(ch));
	act(buf,ch,ch->sw_obj, NULL, TO_ROOM);
    	send_to_char("You transform back into your normal form.\n\r", ch);
	char_from_obj(ch);
	return;
    }

    if (is_item(ch))
    {
	stc("You have already transformed into an item.\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "helm")
	&& ch->power[DISC_DAEM_MORP] > 0)
	obj = create_object( get_obj_index(69), 0 );
    else if (!str_prefix(argument, "greaves")
	 && ch->power[DISC_DAEM_MORP] > 2)
	obj = create_object( get_obj_index(70), 0 );
    else if (!str_prefix(argument, "cloak")
	&& ch->power[DISC_DAEM_MORP] > 4)
	obj = create_object( get_obj_index(71), 0 );
    else if (!str_prefix(argument, "armor")
	&& ch->power[DISC_DAEM_MORP] > 4)
	obj = create_object( get_obj_index(72), 0 );
    else if (!str_prefix(argument, "ring")
	&& ch->power[DISC_DAEM_MORP] > 6)
	obj = create_object( get_obj_index(73), 0 );
    else if (!str_prefix(argument, "sword")
	&& ch->power[DISC_DAEM_MORP] > 6)
	obj = create_object( get_obj_index(74), 0 );
    else if (!str_prefix(argument, "shield")
	&& ch->power[DISC_DAEM_MORP] > 8)
	obj = create_object( get_obj_index(75), 0 );
    else
    {
	do_transform(ch, "");
	return;
    }	

    SET_TIMER(ch, TIMER_DAEMONIC_TRANSFORM , 24);
    obj_to_room(obj, ch->in_room, ch->altitude);
    SET_BIT(obj->extra_flags3, ITEM_DESTROY_CHAR_RETURN);
    act("You transform yourself into $p.", ch, obj, NULL, TO_CHAR);
    act("$n transforms into $p and falls to the ground.", ch, obj, NULL,TO_ROOM);
    char_to_obj(ch, obj);
}

/*
void do_inhabit(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj = NULL;

    if (ch->cur_form != FRM_MIST)
    {
	send_to_char("You must be in mist form to attempt to inhabit an item.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_VAMP_INHABIT)&& ch->level != 120 )
    {
	send_to_char("You can only use this ability once in every 24 hours.\n\r", ch);
	return;
    }

    if (IS_NPC(ch) )
    {
	send_to_char("Not while in the body of another.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
        send_to_char("Inhabit which object?\n\r", ch);
        return;
    }

    obj = get_obj_list( ch, argument, ch->in_room->contents );
    if ( obj == NULL )
    {
        send_to_char("No objects of that name exist.\n\r", ch);
        return;
    }

    if (obj->sw_char != NULL)
    {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    if (obj->carried_by != NULL
        && obj->carried_by->pc_carried != NULL)
    {
        send_to_char("Some mysterious force prevents you.\n\r", ch);
        return;
    }

    if (obj->carried_by == ch)
    {
        obj_from_char(obj);
        obj_to_room(obj, ch->in_room, ch->altitude);
    }

    if (obj->in_obj != NULL || obj->on != NULL)
    {
        send_to_char("Some mysterious force prevents you.\n\r", ch);
        return;
    }

    change_upkeep (ch,2);
 
    SET_TIMER (ch, TIMER_VAMP_INHABIT, 24);

    act("You inhabit $p.", ch, obj, NULL, TO_CHAR);
    act("$n the vampiric mist flows into $p.", ch, obj, NULL,TO_ROOM);

    obj->destroy_on_char_return = FALSE;
    obj->can_return = TRUE;
     
    obj_switch(ch, obj);
}

void do_inhabit_return(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (!IS_ITEM(ch) )
    {
	send_to_char("You aren't inhabiting an item.\n\r", ch);
	return;
    }

    if (!ch->sw_item->can_return)
    {
	send_to_char("Something is preventing you leaving the object./n/r", ch);
	return;
    }

    sprintf(buf,"A vaporous white mist flows out of $p and forms into %s.", GET_PROPER_NAME(ch));
    act(buf, ch,ch->sw_item, NULL, TO_ROOM);

    act("You reform out of $p.", ch, ch->sw_item, NULL, TO_CHAR);

    change_upkeep(ch, -2);
    obj_return(ch, ch->sw_item);
}
*/

void do_mass(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *gch;

    if (!IS_NIGHT())
    {
	send_to_char("You may only use this ability at night.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_VAMPCALL) && ch->level != 120)
    {
	send_to_char("This ability may only be attempted once per night.\n\r", ch);
	return;
    }

    SET_TIMER(ch, TIMER_VAMPCALL, 12);
    send_to_char("You cry out, attempting to call the creatures of the night.\n\r", ch);
    act("$n raises $s head to the sky and cries out.", ch, NULL, NULL,TO_ROOM);

    for (gch = char_list ; gch != NULL ; gch = gch->next)
    {
 	if (IS_FLAG4(gch, AFF_BECKONED)
	    && is_same_realm(ch, gch))	
 	{
	    char_from_room(gch);
	    char_to_room(gch, ch->in_room, ch->altitude);
	    act("$n arrives suddenly.", gch, NULL, NULL, TO_ROOM);
	    gch->master = NULL;
	    add_follower(gch, ch);
	    gch->leader = ch;
	}
    }
}

void do_beckon(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int i;
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("Syntax : beckon <rats wolves bats>\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_VAMPCALL) && ch->level != 120)
    {
	send_to_char("This ability may only be attempted once per night.\n\r", ch);
	return;
    }

    if (!IS_NIGHT())
    {
	send_to_char("You may only use this ability at night.\n\r", ch);
	return;
    }

/* 20 direwolf 21 wolf 22 vampbat 23 bat 24 direrat 25 rat */

    send_to_char("You cry out, attempting to call the creatures of the night.\n\r", ch);
    act("$n raises $s head to the sky and cries out.", ch, NULL, NULL,TO_ROOM);

    if (!IS_OUTSIDE(ch))
    {
	send_to_char("You must be outside to beckon animals.\n\r", ch);
	return;
    }

    if ((dice(1,ch->generation) == 1  || ch->alignment > -100)
	&& ch->level < 120)
    {
	send_to_char("Nothing appears to have arrived.\n\r", ch);
	SET_TIMER(ch, TIMER_VAMPCALL, 12);
	return;
    }

    if (!str_prefix(argument, "rats") )
    {
	if (ch->generation < 6)
	{
	    for (i=0 ; i < dice(2,3) ; i++)
	    {
	        SET_TIMER(ch, TIMER_VAMPCALL, 12);
		victim = create_mobile ( get_mob_index( 25 ) );
		char_to_room(victim, ch->in_room, ch->altitude);
		act("A small black rat scurries up to $n.", ch, NULL, NULL,TO_ROOM);		
		send_to_char("A small black rat scurries up to you.\n\r",ch);
		add_follower(victim, ch);
		victim->leader = ch;
		SET_BIT(victim->flag4, AFF_BECKONED);
	    }
	}
	else
	{
	    for (i=0 ; i < dice(1,3) ; i++)
	    {
	        SET_TIMER(ch, TIMER_VAMPCALL, 12);
		victim = create_mobile ( get_mob_index( 24 ) );
		char_to_room(victim, ch->in_room, ch->altitude);
		act("A huge dire rat scurries up to $n.", ch,NULL,NULL,TO_ROOM);	
		send_to_char("A huge dire rat arrives to do your bidding.\n\r",ch);
		add_follower(victim, ch);
		victim->leader = ch;
		SET_BIT(victim->flag4, AFF_BECKONED);
	    }
	}
    return;
    }

    if (!str_prefix(argument, "wolf")
	|| !str_prefix(argument, "wolves") )
    {
	if (dice(1,3) == 1)
	{
	    bool got=FALSE,found=TRUE;
	    while (!got && found)
    	    {   
    		found = FALSE;
    		for (d = descriptor_list; d != NULL; d = d->next )
    		{
        	    if (d->connected == CON_PLAYING
        	    &&  d->character != ch
        	    &&  d->character->in_room != NULL
        	    &&  can_see( ch, d->character )
        	    &&  d->character->secondaryrace == RACE_LUPINE
        	    &&  !got )
        	    {
            		victim = d->character;
            		found = TRUE;
    	    		send_to_char("You have accidentally beckoned a were-creature.\n\r",ch);
            		if ( victim->fighting != NULL ) stop_fighting( victim, TRUE ); 
    	    		char_from_room( victim );
            		char_to_room( victim, ch->in_room, ch->altitude );
			act("$n has beckoned you to $m.", ch, NULL, victim, TO_VICT);
    	    		act( "$n arrives through the dark blackness of night.", victim,NULL, NULL, TO_ROOM );
    	    		do_look( victim, "auto" );
	    		return;
		    }
    		}	
	    }
	}
	if (ch->generation < 8)
	{
	    for (i=0 ; i < dice(1,3) ; i++)
	    {
	        SET_TIMER(ch, TIMER_VAMPCALL, 12);
		victim = create_mobile ( get_mob_index( 21 ) );
		char_to_room(victim, ch->in_room, ch->altitude);
		act("A large black wolf bounds up to $n.", ch, NULL,NULL,TO_ROOM);		
		send_to_char("A ferocious black wolf bounds up to you.\n\r",ch);
		add_follower(victim, ch);
		victim->leader = ch;
		SET_BIT(victim->flag4, AFF_BECKONED);
	    }
	}
	else
	{
	    for (i=0 ; i < 1 ; i++)
	    {
	        SET_TIMER(ch, TIMER_VAMPCALL, 12);
		victim = create_mobile ( get_mob_index( 20 ) );
		char_to_room(victim, ch->in_room, ch->altitude);
		act("A huge dire wolf arrives.", ch, NULL,NULL,TO_ROOM);
		send_to_char("A huge dire wolf arrives to do your bidding.\n\r",ch);
		victim->leader = ch;
		add_follower(victim, ch);
		SET_BIT(victim->flag4, AFF_BECKONED);
	    }
	}
    return;
    }


    if (!str_prefix(argument, "bats") )
    {
	if (ch->generation < 7)
	{
	    for (i=0 ; i < dice(2,2) ; i++)
	    {
	        SET_TIMER(ch, TIMER_VAMPCALL, 12);
		victim = create_mobile ( get_mob_index( 22 ) );
		char_to_room(victim, ch->in_room, ch->altitude);
		act("A small black bat flies overhead.", ch, NULL,NULL,TO_ROOM);		
		send_to_char("A small black bat flies up to you.\n\r",ch);
		add_follower(victim, ch);
		victim->leader = ch;
		SET_BIT(victim->flag4, AFF_BECKONED);
	    }
	}
	else
	{
	    for (i=0 ; i < dice(1,2) ; i++)
	    {
	        SET_TIMER(ch, TIMER_VAMPCALL, 12);
		victim = create_mobile ( get_mob_index( 23 ) );
		char_to_room(victim, ch->in_room, ch->altitude);
		act("A vampire bat flies up to $n.", ch,NULL,NULL,TO_ROOM);
		send_to_char("A vampire bat arrives to do your bidding.\n\r",ch);
		add_follower(victim, ch);
		victim->leader = ch;
		SET_BIT(victim->flag4, AFF_BECKONED);
	    }
	}
    return;
    }

    do_beckon(ch, "");

}
 
const char * elderpower [9] =
{
    "None", "Immunity", "Battle Mastery", "Charisma", "Sense", 
"Psionics", "Station"
    , "Death Gaze", "Flight"
};


void do_elder(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool foundelder = FALSE;
    int i;
    int eld = 0;
    int expneed;

    if (IS_NPC(ch)) return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 ); 
    
    if (ch->tertiaryrace == 0)
    {
	send_to_char("You are not advanced enough to use elder powers.\n\r", ch);
	return;
    }

    if (arg1[0] == '\0')
    {
	send_to_char("Syntax : elder list\n\r", ch);
	send_to_char("Syntax : elder improve <power>\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "improve") )
    {
	if (arg2[0] == '\0')
	{
	    send_to_char("Which elder power do you wish to improve?\n\r", ch);
	    return;
	}
	
	for(i = 1 ; i < 9 ; i++)
	{
	    if(!str_prefix(arg2, elderpower[i]) )    
	    {
		if (ch->elder[i] == -1)
		{
		    send_to_char("You do not know that power.\n\r", ch);
		    return;
		}
		else eld = i;
	    }
	}

	if (eld == 0)
	{
	    send_to_char("You do not know that power.\n\r", ch);
	    return;
	}

	expneed = 5000 + (5000 * ch->elder[eld]);

	if (ch->elder[eld] == 100);
	{
	    sprintf("You have already achieved 100% at %s.\n\r", elderpower[eld]);
	    send_to_char(buf, ch);
	    return;
	}

	if (ch->currentexp < expneed)
	{
	    sprintf(buf, "You need %d experience points to advance at %s.\n\r", expneed, elderpower[eld]);
	    send_to_char(buf, ch);
	    return;
	}
	
	
	ch->currentexp -= expneed;
	ch->elder[eld] += 1;

	switch(eld)
	{
	    case ELDER_INVULNERABILITY :
	    break;

	    case ELDER_DAMCAP :
	    ch->pcdata->damcap += 2;
	    send_to_char("You feel your combat prowess increase.\n\r", ch);
	    break;

	    default :
	    break;
	}
	return;
    }


    if (!str_prefix(arg1, "list") )    
    {    
	for (i = 1 ; i < 9 ; i++)
	{
	    if (ch->elder[i] != -1)
	    {
		sprintf(buf, "%s, currently at %d%%\n\r", elderpower[i], ch->elder[i]);
		send_to_char(buf, ch);
		foundelder = TRUE;
	    }
	}
	if (!foundelder)
	    send_to_char("You do not know any elder powers.\n\r", ch);
	return;
    }

    
    do_elder(ch, "");
}


void do_kaielder(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    bool notelder = FALSE;
    int i, rand;

    if (IS_NPC(ch)) return;

    if (ch->tertiaryrace == 0)
    {
	send_to_char("You are not advanced enough to use elder powers.\n\r", ch);
	return;
    }
    
    for (i = 1; i < 9; i++)
	if (ch->elder[i] == -1) notelder = TRUE;
    
    if (!notelder)
    {
	send_to_char("You already know all the elder powers available.\n\r", ch);
	return;
    }

    if (ch->qp < 10)
    {
	send_to_char("You need ten kai points to gain an elder power.\n\r", ch);
	return;
    }

    ch->qp -= 10;

    for(;;)
    {
	rand = dice(1,8);
	if (ch->elder[rand] == -1)
	{
	    ch->elder[rand] = 0;
	    sprintf(buf, "You have been granted the power of %s.\n\r", elderpower[rand]);
	    send_to_char(buf, ch);
	    do_anything(ch, 5);
	    return;
	}
    }
}


void do_tempt(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;    
    CHAR_DATA *victim_next;    

    argument = one_argument( argument, arg1 );
    /* useful only if we wanted action to be one word -- whois
    argument = one_argument( argument, arg2 ); */

    if (arg1[0] == '\0')
    {
	send_to_char("Syntax : tempt <victim> <action>\n\r",ch);
	return;
    }
   
    sprintf(buf,"%s", argument);

    if (!str_cmp(arg1, "all") && ch->power[DISC_DAEM_TEMP] >= 8)
    {
        
	WAIT_STATE(ch, 8);
	if (argument[0] == '\0')
	{
	    stc("Tempt all to do what?\n\r", ch);
	    return;
	}

	for (victim = ch->in_room->people ; victim != NULL ; 
		victim = victim_next)
	{
	    WAIT_STATE(ch, 8);
	    victim_next = victim->next_in_room;

	    if (victim == ch) continue;

	    if (is_safe(victim, ch)) continue;

            sprintf(buf2, "I think %s wants to %s", GET_NAME(victim), buf);
            do_think(ch, buf2);

            if (sanity_resist(ch, victim, ch->power[DISC_DAEM_TEMP])) return;

            if (victim->level > 111 || victim->power[DISC_DAEM_DISC] >= 7)
    	    {
	  	send_to_char("They shake off your suggestion.\n\r", ch);
		continue;
    	    }
 
            else if (victim->level > ch->level + (ch->power[DISC_DAEM_TEMP] * 2) )
            {
	        send_to_char("You fail to plant your suggestion.\n\r", ch);
	        return;
            }

            if (bad_to_interpret(buf))
            {
	        send_to_char("The powers acknowledge your actions.\n\r",ch);
	        return;
            }

            interpret(victim, buf);
	}
	return;
    }

    if ((victim = get_char_room(ch, arg1)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (is_safe(victim, ch)) return;

    if ( arg1[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tempt whom to do what?\n\r", ch );
	return;
    }

    if (ch->power[DISC_DAEM_TEMP] < 7)
    {
        sprintf(buf2, "I think %s wants to %s", GET_NAME(victim), buf);
        do_say(ch, buf2);
    }
    else
    {
        sprintf(buf2, "I think %s wants to %s", GET_NAME(victim), buf);
        do_think(ch, buf2);
    }

    if (sanity_resist(ch, victim, ch->power[DISC_VAMP_DOMI])) return;

    if (victim->level > 111 || victim->power[DISC_DAEM_DISC] >= 7)
    {
	send_to_char("They shake off your suggestion.\n\r", ch);
	WAIT_STATE(ch, 12);
	return;
    }

    else if (victim->level > ch->level + (ch->power[DISC_VAMP_DOMI] * 2) )
    {
	send_to_char("You fail to plant your suggestion.\n\r", ch);
	return;
    }

    if (bad_to_interpret(buf))
    {
	send_to_char("The powers acknowledge your actions.\n\r",ch);
	return;
    }

    WAIT_STATE(ch, 12);

    interpret(victim, buf);
}
void do_command(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;    
    CHAR_DATA *victim_next;    

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 ); 

    if (arg1[0] == '\0')
    {
	send_to_char("Syntax : command <victim> <message>\n\r",ch);
	return;
    }

    if (!str_cmp(arg1, "all") && ch->power[DISC_VAMP_DOMI] >= 8)
    {
        sprintf(buf,"%s",arg2);
	WAIT_STATE(ch, 8);
	if (arg2[0] == '\0')
	{
	    stc("Command all to do what?\n\r", ch);
	    return;
	}

	for (victim = ch->in_room->people ; victim != NULL ; 
		victim = victim_next)
	{
	    WAIT_STATE(ch, 8);
	    victim_next = victim->next_in_room;

	    if (victim == ch) continue;

	    if (is_safe(victim, ch)) continue;

            sprintf(buf2, "I think %s wants to %s", GET_NAME(victim), buf);
            do_think(ch, buf2);

            if (sanity_resist(ch, victim, ch->power[DISC_VAMP_DOMI])) return;

            if (victim->level > 111 || victim->power[DISC_DAEM_DISC] >= 7)
    	    {
	  	send_to_char("They shake off your suggestion.\n\r", ch);
		continue;
    	    }
 
 	    if (!IS_NPC(victim)
	 	&& victim->secondaryrace == RACE_VAMPIRE
		&& victim->generation > ch->generation
	   	&& (victim->pcdata->conditioned_by == NULL
	   	|| str_cmp(victim->pcdata->conditioned_by, GET_PC_NAME(ch)) ) )
            {
	         send_to_char("You cannot dominate a vampire of stronger blood.\n\r", ch);
	         return;
	    }

            if (!IS_NPC(victim)
   	        && victim->pcdata->conditioned_by != NULL
	        && !str_cmp(victim->pcdata->conditioned_by, GET_PC_NAME(ch)) )
 		send_to_char("You find it easy to dominate the mind of your conditioned victim.\n\r", ch);
 
            else if (victim->level > ch->level + (ch->power[DISC_VAMP_DOMI] * 2) )
            {
	        send_to_char("You fail to plant your suggestion.\n\r", ch);
	        return;
            }

            if (bad_to_interpret(buf))
            {
	        send_to_char("The powers acknowledge your actions.\n\r",ch);
	        return;
            }

            interpret(victim, buf);
	}
	return;
    }

    if ((victim = get_char_room(ch, arg1)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (is_safe(victim, ch)) return;

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Command whom to do what?\n\r", ch );
	return;
    }

    sprintf(buf,"%s",arg2);

    if (ch->power[DISC_VAMP_DOMI] < 3)
    {
        sprintf(buf2, "I think %s wants to %s", GET_NAME(victim), buf);
        do_say(ch, buf2);
    }
    else if (ch->power[DISC_VAMP_DOMI] < 6)
    {
        sprintf(buf2, "I think %s wants to %s", GET_NAME(victim), buf);
        do_think(ch, buf2);
    }
    else
    {
        sprintf(buf2, "You command %s to '%s'\n\r", GET_NAME(victim), buf);
	stc(buf2, ch);
    }

    if (sanity_resist(ch, victim, ch->power[DISC_VAMP_DOMI])) return;

    if (victim->level > 111 || victim->power[DISC_DAEM_DISC] >= 7)
    {
	send_to_char("They shake off your suggestion.\n\r", ch);
	WAIT_STATE(ch, 12);
	return;
    }
    
    if (!IS_NPC(victim)
	&& victim->secondaryrace == RACE_VAMPIRE
	&& victim->generation > ch->generation
	&& (victim->pcdata->conditioned_by == NULL
	|| str_cmp(victim->pcdata->conditioned_by, GET_PC_NAME(ch)) ) )
    {
	send_to_char("You cannot dominate a vampire of stronger blood.\n\r", ch);
	return;
    }

    if (!IS_NPC(victim)
	&& victim->pcdata->conditioned_by != NULL
	&& !str_cmp(victim->pcdata->conditioned_by, GET_PC_NAME(ch)) )
	send_to_char("You find it easy to dominate the mind of your conditioned victim.\n\r", ch);

    else if (victim->level > ch->level + (ch->power[DISC_VAMP_DOMI] * 2) )
    {
	send_to_char("You fail to plant your suggestion.\n\r", ch);
	
	return;
    }

    if (bad_to_interpret(buf))
    {
	send_to_char("The powers acknowledge your actions.\n\r",ch);
	return;
    }

    WAIT_STATE(ch, 12);

    interpret(victim, buf);
}

void do_vamp_mesmerise(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;    

    argument = one_argument( argument, arg1 );

    if (arg1[0] == '\0')
    {
	send_to_char("Syntax : mesmerise <victim> <message>\n\r",ch);
	return;
    }

    if ((victim = get_char_room(ch, arg1)) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if ( arg1[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Command whom to do what?\n\r", ch );
	return;
    }

    if (is_safe(victim, ch)) return;

    sprintf(buf,"%s",argument);

    if (ch->power[DISC_VAMP_DOMI] < 3)
    {
        sprintf(buf2, "I think %s wants to %s", GET_NAME(victim), buf);
        do_say(ch, buf2);
    }
    else if (ch->power[DISC_VAMP_DOMI] < 6)
    {
        sprintf(buf2, "I think %s wants to %s", GET_NAME(victim), buf);
        do_think(ch, buf2);
    }
    else
    {
        sprintf(buf2, "You command %s to '%s'.\n\r", GET_NAME(victim), buf);
	stc(buf2, ch);
    }

    if (sanity_resist(ch, victim, ch->power[DISC_VAMP_DOMI])) return;

    if (victim->level > 111 || victim->power[DISC_DAEM_DISC] >= 7)
    {
	send_to_char("They shake off your suggestion.\n\r", ch);
	WAIT_STATE(ch, 12);
	return;
    }
    
    if (!IS_NPC(victim)
	&& victim->secondaryrace == RACE_VAMPIRE
	&& victim->generation > ch->generation
	&& (victim->pcdata->conditioned_by == NULL
	|| str_cmp(victim->pcdata->conditioned_by, GET_PC_NAME(ch)) ) )
    {
	send_to_char("You cannot dominate a vampire of stronger blood.\n\r", ch);
	return;
    }

    if (!IS_NPC(victim)
	&& victim->pcdata->conditioned_by != NULL
	&& !str_cmp(victim->pcdata->conditioned_by, GET_PC_NAME(ch)) )
	send_to_char("You find it easy to dominate the mind of your conditioned victim.\n\r", ch);

    else if (victim->level > ch->level + (ch->power[DISC_VAMP_DOMI] * 2) )
    {
	send_to_char("You fail to plant your suggestion.\n\r", ch);
	return;
    }

    if (victim->pet != NULL
	&& IS_FLAG5(victim->pet, AFF5_FAMILIAR_WEASEL))
    {
	stc("You cannot seem to influence their mind.\n\r", ch);
	return;
    }

    if (bad_to_interpret(arg2))
    {
	send_to_char("The powers acknowledge your actions.\n\r",ch);
	return;
    }

    WAIT_STATE(ch, 12);

    interpret(victim, buf);
}

void do_subvert (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    if (ch->power[DISC_WARL_CONT] < 5)
    {
	send_to_char("You are not experienced enough at the Control discipline.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Subvert who?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }
    
    if (!IS_NPC(victim) || !IS_AFFECTED(victim, AFF_CHARM) )
    {
	send_to_char("Only charmed monsters can be subverted.\n\r", ch);
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM)
    ||   ch->level < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( victim->level - (ch->power[DISC_WARL_CONT] * 4), victim,DAM_CHARM) )
    {    
	act("$N shrugs off your suggestion.",ch, NULL, victim, TO_CHAR);
	return;
    }    
    
    if (victim->master == ch)
    {
	act("$N is already under your control.", ch, NULL, victim, TO_CHAR);
	return;
    }

    WAIT_STATE(ch, 10);
    act("You manage to subvert $N to follow you instead.", ch, NULL, victim, TO_CHAR);
    act("$n glares at $N intently.", ch, NULL, victim, TO_NOTVICT);
    act("$n is your new master, you feel compelled to follow.", ch, NULL, victim, TO_VICT);

    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;
    af.where     = TO_AFFECTS;
    af.type      = gsn_charm_person;
    af.level     = ch->level;
    af.duration  = number_fuzzy( ch->power[DISC_WARL_CONT] * 4 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
}

void do_archkai (CHAR_DATA *ch, char *argument)
{
    int imprealm = 0;
    
    if (IS_NPC(ch)) return;

    if (ch->secondaryrace != RACE_WARLOCK)
    {
	send_to_char("Only a Warlock may surpass the ninth circle of spell ability.\n\r", ch);
	return;
    }

    if (!str_cmp(argument, "flame"))
    {
	if (ch->fire < 9)
	{
	    send_to_char("You have not progressed to the ninth circle in flame.\n\r", ch);
	    return;
	}
	else if (ch->fire == 10)
	{
	    send_to_char("You are already an archmage at flame.\n\r", ch);
	    return;
	}
	imprealm = 1;
    }
    if (!str_cmp(argument, "water"))
    {
	if (ch->water < 9)
	{
	    send_to_char("You have not progressed to the ninth circle in water.\n\r", ch);
	    return;
	}
	else if (ch->water == 10)
	{
	    send_to_char("You are already an archmage at water.\n\r", ch);
	    return;
	}
	imprealm = 2;
    }
    if (!str_cmp(argument, "air"))
    {
	if (ch->air < 9)
	{
	    send_to_char("You have not progressed to the ninth circle in air.\n\r", ch);
	    return;
	}
	else if (ch->air == 10)
	{
	    send_to_char("You are already an archmage at air.\n\r", ch);
	    return;
	}
	imprealm = 3;
    }
    if (!str_cmp(argument, "earth"))
    {
	if (ch->earth < 9)
	{
	    send_to_char("You have not progressed to the ninth circle in earth.\n\r", ch);
	    return;
	}
	else if (ch->earth == 10)
	{
	    send_to_char("You are already an archmage at earth.\n\r", ch);
	    return;
	}
	imprealm = 4;
    }
    if (!str_cmp(argument, "entropy"))
    {
	if (ch->entropy < 9)
	{
	    send_to_char("You have not progressed to the ninth circle in entropy.\n\r", ch);
	    return;
	}
	else if (ch->entropy == 10)
	{
	    send_to_char("You are already an archmage at entropy.\n\r", ch);
	    return;
	}
	imprealm = 5;
    }
    if (!str_cmp(argument, "lifemagic"))
    {
	if (ch->lifemagic < 9)
	{
	    send_to_char("You have not progressed to the ninth circle in life magic.\n\r", ch);
	    return;
	}
	else if (ch->lifemagic == 10)
	{
	    send_to_char("You are already an archmage at life magic.\n\r", ch);
	    return;
	}
	imprealm = 5;
    }
    if (imprealm == 0)
    {
	send_to_char("Which realm do you wish to become an archmage in?\n\r", ch);
	return;
    }
    
    if (ch->qp < 10)
    {
	send_to_char("You need ten points of kai to become an archmage.\n\r", ch);
	return;
    }

    ch->qp -= 10;
    WAIT_STATE(ch, 8);
    switch(imprealm)
    {
	case 1 :
	send_to_char("The fires within you burn stronger than ever.\n\r", ch);
	ch->fire = 10;
	gn_add(ch, 19);
	break;
	case 2 :
	send_to_char("The waters of Lothariel immerse you for eternity.\n\r", ch);
	ch->water = 10;
	gn_add(ch, 29);
	break;
	case 3 :
	send_to_char("Your soul will be free forever.\n\r", ch);
	ch->air = 10;
	gn_add(ch, 39);
	break;
	case 4 :
	send_to_char("You pledge your very being to the Mother Earth.\n\r", ch);
	ch->earth = 10;
	gn_add(ch, 49);
	break;
	case 5 :
	send_to_char("The powers have acknowledged your actions.\n\r", ch);
	ch->entropy = 10;
	gn_add(ch, 59);
	break;

	case 6 :
	send_to_char("The powers have acknowledged your actions.\n\r", ch);
	ch->lifemagic = 10;
	gn_add(ch, 69);
	break;

	default : break;
    }
    send_to_char("Welcome to a new world, Archmage.\n\r", ch);
    ch->generation++;
    if (ch->generation > 12) ch->generation = 12;
    ch->pcdata->max_memorize[10]++;
}

void do_transport (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg1);
    argument = one_argument( argument, arg2);

    if (ch->power[DISC_WARL_FARR] < 4)
    {
	send_to_char("You need more experience at the Far Reaching discipline.\n\r", ch);
	return;
    }
    
    if (arg1[0] == '\0')
    {
	send_to_char("Which item do you wish to transport?\n\r", ch);
	return;
    }
    
    if (arg2[0] == '\0')
    {
	send_to_char("Which character do you wish to transport the item to?\n\r", ch);
	return;
    }
    
    if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )
    {
	send_to_char( "You cannot sense anybody of this name.\n\r", ch);
	return;
    }
    
    WAIT_STATE(ch, 4);

    obj_from_char( obj );
    obj_to_char( obj, victim );
  
    if (ch->primary == 1)
    {
	act("$p ignites and burns to a crisp.", ch, obj, NULL, TO_CHAR);
	act("$p appears in a blast of flames.", victim, obj, NULL, TO_CHAR);
    }
    else if (ch->primary == 2)
    {
	act("$p melts into slime, which then evaporates.", ch, obj, NULL, TO_CHAR);
	act("Slime bubbles in your hands, forming into $p.", victim, obj, NULL, TO_CHAR);
    }
    else if (ch->primary == 3)
    {
	act("$p floats away on the breeze.", ch, obj, NULL, TO_CHAR);
	act("$p floats down from the sky into your hand.", victim, obj, NULL, TO_CHAR);
    }
    else if (ch->primary == 4)
    {
	act("$p jumps out of your hand and rolls away.", ch, obj, NULL, TO_CHAR);
	act("$p rolls up to you and leaps in your hand.", victim, obj, NULL, TO_CHAR);
    }
    else
    {
	act("A small imp swoops down and carries $p away.", ch, obj, NULL, TO_CHAR);
	act("A small imp swoops down, cackles insanely, and gives you $p.", victim, obj, NULL, TO_CHAR);
    }
}

void do_roar (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *ich;
    AFFECT_DATA af;

    send_to_char("Roar is temporarily disabled.\n\r", ch);
    return;    

    WAIT_STATE(ch, 12);

    if (ch->power[DISC_DRAG_AWE] > 0)
    {
    for (ich = ch->in_room->people; ich!=NULL; ich = ich->next_in_room)
    {
	if (ch->altitude != ich->altitude) continue;
	if (ich->level < 7
	    && get_size(ich) < SIZE_MEDIUM
	    && !IS_NEWFLAG(ich, AFF_FEAR) )
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
    }
    }
 
    if (ch->power[DISC_DRAG_AWE] > 1)
    {
	for (ich = ch->in_room->people; ich!=NULL; ich = ich->next_in_room)
	{
	    if (ch->altitude != ich->altitude) continue;
	    if (!IS_NEWFLAG(ich, AFF_MAGIC_DEAF) 
		&& !saves_spell( ch->generation * 8, ich, DAM_NEGATIVE ) )
	    {
		af.where = TO_NEWFLAGS;
		af.type  = skill_lookup("fireball");
		af.level = ch->level;
		af.duration = ch->generation + 3;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = AFF_MAGIC_DEAF;
		affect_to_char(ich, &af);
	    }
	}
	if (!IS_NEWFLAG(ich, AFF_INSPIRATION) 
	    && !saves_spell( ch->generation * 8, ich, DAM_NEGATIVE ) )
	{
	    af.where = TO_FLAG3;
	    af.type  = 0;
	    af.level = ch->level;
	    af.duration = ch->generation + 3;
	    af.location = APPLY_HITROLL;
	    af.modifier = ch->generation + (ch->power[DISC_DRAG_AWE] * 4);
	    af.bitvector = AFF_INSPIRATION;
	    affect_to_char(ch, &af);
	    af.location = APPLY_DAMROLL;
	    af.modifier = ch->generation + (ch->power[DISC_DRAG_AWE] * 3);
	    affect_to_char(ch, &af);    
	}
    }
    
    if (ch->power[DISC_DRAG_AWE] > 2)
    {
    
    }
    
    if (ch->power[DISC_DRAG_AWE] > 3)
    {
    
    }
    
    if (ch->power[DISC_DRAG_AWE] > 4)
    {
    
    }
}
