/******************************************************************
 * Act_Dark -- (C) 1996, by Adam Powell                           *
 * Misc. functions coded by AdAm PoWeLl                           *
 ******************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hell.h"
#include "magic.h"
 
/* command procedures needed */

DECLARE_DO_FUN(do_get_obj_world );
DECLARE_DO_FUN(do_berserk );
DECLARE_DO_FUN(do_claw );
DECLARE_DO_FUN(do_fang );


void do_bandage(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	send_to_char("Who do you wish to bandage?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim->hit > -1 || victim->position > POS_INCAP)
    {
	send_to_char("They aren't dying.\n\r", ch);
	return;
    }

    if (victim->secondaryrace == RACE_VAMPIRE)
    {
	send_to_char("But they aren't bleeding.\n\r", ch);
	return;
    }

    act("$n bends down and bandages $N's wounds.", ch, NULL, victim,TO_NOTVICT);
    act("You bend down and bandage $N's wounds.", ch, NULL, victim,TO_CHAR);
    act_new("$n has bandaged your wounds.", ch, NULL, victim,TO_VICT,POS_DEAD);
    victim->hit = 1;
    victim->position = POS_RESTING;
    WAIT_STATE(ch, 12);
}

ROOM_INDEX_DATA * locate_obj (OBJ_DATA *obj)
{
    if (obj->on != NULL)
	return locate_obj(obj->on);

    if (obj->in_obj != NULL)
        return locate_obj(obj->in_obj);

    if (obj->carried_by != NULL)
	return obj->carried_by->in_room;

    return obj->in_room;
}


void do_mix(CHAR_DATA *ch, char *argument) 
{
    OBJ_DATA *obj;
    OBJ_DATA *pot1 = NULL;
    OBJ_DATA *pot2 = NULL;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int potlevel1, potlevel2, randpower, i;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("Syntax : mix <potion 1> <potion 2>\n\r", ch);
	return;
    }

    for ( obj = ch->carrying ; obj != NULL ; obj = obj->next_content)
    {
	if ( can_see_obj (ch, obj) && is_name (arg1, obj->name))
	pot1 = obj;
    }

    if (pot1 == NULL)
    {
	send_to_char("You have not got that item in your inventory.\n\r", ch);
	return;
    }

    for ( obj = ch->carrying ; obj != NULL ; obj = obj->next_content)
    {
	if ( can_see_obj (ch, obj) && is_name (arg2, obj->name)
		&& pot1 != pot2)
	pot2 = obj;
    }

    if (pot2 == NULL)
    {
	send_to_char("You do not have that item in your inventory.\n\r", ch);
	return;
    }

    if (pot1->item_type != ITEM_POTION
	|| pot2->item_type != ITEM_POTION)
    {
	send_to_char("You can only mix together potions.\n\r", ch);
	return;
    }

    potlevel1 = pot1->value[0]/10; 
    potlevel2 = pot2->value[0]/10;
    for ( i=1 ; i < 5 ; i++ )
    {
	if (pot1->value[i] != 0)
	    potlevel1 += skill_table[pot1->value[i]].level;
	if (pot2->value[i] != 0)
	    potlevel2 += skill_table[pot2->value[i]].level;
    }

    sprintf(buf, "You pour %s into %s and shake it violently.\n\r", pot1->short_descr, pot2->short_descr);
    send_to_char(buf, ch);
    sprintf(buf, "$p pours %s into %s and shakes it violently.\n\r", pot1->short_descr, pot2->short_descr);
    act(buf, ch, NULL, NULL, TO_ROOM);

    if (potlevel1 > potlevel2) randpower = potlevel1 + potlevel2;
    else randpower = (potlevel2 * 2) + potlevel1;
    randpower += ch->generation;
    randpower += ch->power[DISC_WARL_BREW] * 3;

    WAIT_STATE(ch, 8);

    if (randpower < 20)
    {
	send_to_char("The potion liquid bubbles and evaporates.\n\r", ch);
	act("The potion liquid bubbles and evaporates.", ch, NULL, NULL, TO_ROOM);
	extract_obj(pot1);
	extract_obj(pot2);
	return;
    }

    if (randpower < 40 && dice (1,4) == 1)
    {
	send_to_char("The potion explodes in a ball of flame, incinerating you.\n\r", ch);
	act("$n's potion explodes in a ball of flame, incinerating $m.", ch, NULL, NULL, TO_ROOM);
	damage_old(ch, ch, 100, 0, DAM_FIRE, FALSE);
	extract_obj(pot1);
	extract_obj(pot2);
	return;
    }

    if (dice(1, 3) == 1)
    {
        pot2->value[0] = ((pot1->value[0] + pot2->value[0]) / 2);
	pot2->value[2] = pot1->value[1];
	pot2->value[3] = 0;
	pot2->value[4] = 0;
	extract_obj(pot1);
   	return;
    }

    pot2->item_type = ITEM_SPEC_POTION;
    pot2->value[0] = ((pot1->value[0] + pot2->value[0]) / 2);
    pot2->value[1] = randpower / 10 + dice(1, 3);

    extract_obj(pot1);
}

void do_receptacle (CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;

    if (IS_FLAG3(ch, AFF_RECEPTACLE) )
    {
	send_to_char("The receptacle power is already in operation.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);

    af.where     = TO_FLAG3;
    af.level     = ch->generation * 10;
    af.duration  = ch->generation + 3;
    af.location  = 0;
    af.type      = -1;
    af.modifier  = 0;
    af.bitvector = AFF_RECEPTACLE;
    affect_join( ch, &af );
    send_to_char( "You explode with brilliant magical energy.\n\r", ch);
    act("$n's body explodes with a brilliant radiance of magical energy.",ch,NULL,NULL,TO_ROOM);
}

void do_track (CHAR_DATA *ch, char *argument)
{
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *in_room;
    
    in_room = ch->in_room;
    
    if (in_room->sector_type == SECT_UNDERWATER)
    {
	send_to_char("You cannot follow any trails underwater.\n\r", ch);
	return;
    }
    
    if (in_room->sector_type == SECT_AIR)
    {
	send_to_char("You are flying in the air, there are no trails to follow.\n\r", ch);
	return;
    }
    
    if (in_room->sector_type == SECT_WATER_NOSWIM)
    {
	send_to_char("The water is obscuring all possible trails.\n\r", ch);
	return;
    }
    
    if (in_room->sector_type == SECT_WATER_SWIM
	&& ch->generation < 10)
    {
	send_to_char("The water has obscured all possible tracks.\n\r", ch);
	return;
    }

    if (in_room->track[0] != NULL
	&& in_room->trackdir[0] != NULL
	&& in_room->track[0] != ch->pcdata->switchname)
    {
	sprintf(buf, "You detect the trail of %s leading %s.\n\r",
	    in_room->track[0], in_room->trackdir[0]);
	send_to_char(buf, ch);
	found = TRUE;
    }       
    
    if (in_room->track[1] != NULL
	&& in_room->trackdir[1] != NULL
	&& ch->generation > 5
	&& in_room->track[1] != ch->pcdata->switchname)
    {
	sprintf(buf, "You detect the feint trail of %s leading %s.\n\r",
	    in_room->track[1], in_room->trackdir[1]);
	send_to_char(buf, ch);
	found = TRUE;
    }
    
    if (in_room->track[2] != NULL
	&& in_room->trackdir[2] != NULL
	&& ch->generation > 8
	&& in_room->track[2] != ch->pcdata->switchname)
    {
	sprintf(buf, "You detect the very feint trail of %s leading %s.\n\r",
	    in_room->track[2], in_room->trackdir[2]);
	send_to_char(buf, ch);
	found = TRUE;
    }

    if (!found)
	send_to_char("You detect no trails from this place.\n\r", ch);
}

void do_condition (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    
    if (IS_NPC(ch)) return;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to condition?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim) )
    {
	send_to_char("You find your victim somewhat unreceptive to your charms.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);
    
    if (!str_cmp(ch->pcdata->conditioned_by, GET_PC_NAME(ch)) )
    {
	send_to_char("You don't want to do that.", ch);
	return;
    }

    if (ch->generation - victim->generation < 0 )
    {
	send_to_char("Their mind is too powerful for you to overcome.\n\r", ch);
	return;
    }

    victim->pcdata->conditioned_by = str_dup(GET_PC_NAME(ch));
    act("$n stares deeply into $N's eyes.", ch, NULL, victim, TO_NOTVICT);
    act("You stare deeply into $N's eyes and condition $S mind.", ch, NULL, victim, TO_CHAR);
    act("$n stares deeply into your eyes, you feel somewhat dizzy.", ch, NULL, victim, TO_VICT);
}

void do_karma (CHAR_DATA *ch, char *argument)
{
   if (IS_AFFECTED(ch, AFF_CURSE) )
   {
       send_to_char("Something is preventing you from using this ability.\n\r",ch);
       return;
   }
   
   if (!str_prefix(argument, "off") && argument[0] != '\0')
   {
       if (!IS_FLAG3(ch, AFF_KARMA) )
       {
	   send_to_char("You are not using your karma.\n\r",ch);
	   return;
       }
       else
       {
	   send_to_char("You stop draining your psychic energy.\n\r",ch);
	   REMOVE_BIT(ch->flag3, AFF_KARMA);
	   return;
       }
   }
   
   if (IS_FLAG3(ch, AFF_KARMA) )
   {
       send_to_char("You are already using your karma.\n\r", ch);
       return;
   }
   else if (ch->psp < (30 - ch->generation) )
   {
	send_to_char("You do not have enough psionic energy to start the process.\n\r", ch);
	return;
   }
   else
   {
       SET_BIT(ch->flag3, AFF_KARMA);
       send_to_char("You start to drain your psychic energy.\n\r", ch);
   }
}

void do_flip (CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    OBJ_DATA *obj = NULL;
    OBJ_DATA *foundobj = NULL;
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    AFFECT_DATA af;

    argument = one_argument( argument, arg1);
    argument = one_argument( argument, arg2);
    argument = one_argument( argument, arg3);
    
    if (ch->power[DISC_WARL_BIND] == 0)    
    {
	send_to_char("You need more experience in the Binding discipline.\n\r",ch);
	return;
    }

    if (arg1[0] == '\0')
    {
	 send_to_char("Syntax : flip <tome> <target>\n\r", ch);
	 return;
    }

    for ( obj = ch->carrying ; obj != NULL ; obj = obj->next_content)
    {
	if ( can_see_obj (ch, obj) && is_name (arg1, obj->name))
	foundobj = obj;
    }

    if (foundobj == NULL)
    {
	send_to_char("You have not got that item in your inventory.\n\r", ch);
	return;
    }

    if (foundobj->item_type != ITEM_TOME)
    {
	send_to_char("That item is not a spellbook.\n\r", ch);
	return;
    }

    if (foundobj->value[1] == 0)
    {
	send_to_char("You flip through but can't seem to find any spells.\n\r", ch);
	return;
    }

    if (foundobj->value[0] < 3)
    {
	send_to_char("The magic of this tome is too weak to summon.\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {   
	send_to_char("Syntax : flip <tome> <target>\n\r", ch);
	return;
    }

    act("You flip through $p.", ch, foundobj, NULL, TO_CHAR);
    act("$n flips through $p.",ch, foundobj, NULL, TO_ROOM);

    WAIT_STATE(ch, 12);

    if (IS_SET(foundobj->value[4], TOME_SPEED) )
       spell_haste(skill_lookup("haste"), foundobj->value[0], ch, victim, TARGET_CHAR);
    
    if (IS_SET(foundobj->value[4], TOME_PLAGUE) && foundobj->value[3] < 10 )
       spell_plague(skill_lookup("plague"), foundobj->value[0], ch, victim, TARGET_CHAR);
    else if (IS_SET(foundobj->value[4], TOME_PLAGUE) )    
    {
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next        = vch->next;
	    if ( vch->in_room == NULL )
		continue;
	    if ( vch->in_room == ch->in_room && vch != ch)
		spell_plague(skill_lookup("plague"), foundobj->value[0], ch, vch, TARGET_CHAR);
	}
    }
    
    if (IS_SET(foundobj->value[4], TOME_SHROUD) )
       spell_sanctuary(skill_lookup("sanctuary"), foundobj->value[0], ch, victim, TARGET_CHAR);
    if (IS_SET(foundobj->value[4], TOME_ARMOR) )
       spell_armor(skill_lookup("armor"), foundobj->value[0], ch, victim, TARGET_CHAR);


    if (IS_SET(foundobj->value[4], TOME_VENOM) )
    {  
	if ( saves_spell( ch->level, victim,DAM_POISON) )
	    send_to_char("A cloud of green gas envelops you, but it disperses.\n\r",victim);
	else
	{
	    af.where     = TO_AFFECTS;
	    af.type      = gsn_poison;
	    af.level     = foundobj->value[0];
	    af.duration  = (foundobj->value[0]/3);
	    af.location  = APPLY_STR;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( victim, &af );
	    send_to_char( "A cloud of green gas envelops you.  You feel very sick.\n\r", victim );
	    act("A cloud of green gas envelops $n.",victim,NULL,NULL,TO_ROOM);
	}
    }

    if (IS_SET(foundobj->value[4], TOME_BLAST) )
    {
	int dam;

	dam = foundobj->value[3];

	if (foundobj->value[3] < 5)
	{
	    act("An explosion of sound issues forth from $p.", ch, foundobj, NULL, TO_CHAR);
	    act("An explosion of sound issues forth from $p.", ch, foundobj, NULL, TO_ROOM);
	    damage_old(ch, victim, dam, skill_lookup("star storm"), DAM_SOUND, FALSE);
	}
	else
	{
	    act("A HUGE explosion of sound issues forth from $p.", ch, foundobj, NULL, TO_CHAR);
	    act("A HUGE explosion of sound issues forth from $p.", ch, foundobj, NULL, TO_ROOM);
	    damage_old(ch, victim, dam * 2, skill_lookup("star storm"), DAM_SOUND, FALSE);
	}
    }
    if (IS_SET(foundobj->value[4], TOME_SELFHEAL) )
    {
	int heal;
	heal = foundobj->value[2];
	send_to_char("Your wounds heal and close up.\n\r", ch);
	act("$n's wounds heal and close up.", ch, NULL, NULL, TO_ROOM);
	ch->hit += heal;
	if (ch->hit > ch->max_hit) ch->hit = ch->max_hit;
    }
    if (IS_SET(foundobj->value[4], TOME_HEAL) )
    {
	int heal;
	heal = foundobj->value[2];
	if (ch != victim)
	    act("$N's wounds heal and close up.", ch, NULL, victim, TO_CHAR);
	act("$N's wounds heal and close up.", ch, NULL, victim, TO_NOTVICT);
	send_to_char("Your wounds heal and close up.\n\r", victim);
	victim->hit += heal;
	if (victim->hit > victim->max_hit) victim->hit = victim->max_hit;
    }

    if (IS_SET(foundobj->value[4], TOME_FLAMEBALL) )
    {
	int dam;
	dam = foundobj->value[3];
	act("A huge blast of flame explodes from $p.", ch, foundobj, NULL, TO_CHAR);
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next        = vch->next;
	    if ( vch->in_room == NULL )
		continue;
	    if ( vch->in_room == ch->in_room && vch != ch)
	    {
		send_to_char("A huge ball of flame roars towards you.\n\r", vch);
		if (vch == victim) damage_old(ch, vch, dam*2, skill_lookup("fireball"), DAM_FIRE, TRUE);
		else damage_old(ch, vch, dam,skill_lookup("fireball"), DAM_FIRE, TRUE);
	    }
	}
    }
    if (IS_SET(foundobj->value[4], TOME_SUNBEAM) )
    {
	int dam;
	dam = foundobj->value[3];
	send_to_char("You call sunlight down from the heavens.\n\r", ch);
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next        = vch->next;
	    if ( vch->in_room == NULL )
		continue;
	    if ( vch->in_room == ch->in_room
		 && vch->secondaryrace == RACE_VAMPIRE)
	    {
		send_to_char("A dazzling beam of sunlight tears apart your flesh.\n\r", vch);
		if (vch == victim) damage_old(ch, vch, dam*2,skill_lookup("fireball"), DAM_LIGHT, TRUE);
		else damage_old(ch, vch, dam, skill_lookup("sunbeam"), DAM_LIGHT, TRUE);
	    }
	}
    }
}

bool is_undead(CHAR_DATA *ch)
{
    if (ch->secondaryrace == RACE_VAMPIRE
	|| ch->secondaryrace == RACE_DAEMON
	|| ch->tertiaryrace == TERTIARY_LICH)
    	    return TRUE;

    if (IS_SET(ch->act, ACT_UNDEAD)) return TRUE;

    if (!IS_NPC(ch) ) return FALSE;

    if (is_name("zombie", ch->name)) return TRUE;
    if (is_name("skeleton", ch->name)) return TRUE;
    if (is_name("ghoul", ch->name)) return TRUE;
    if (is_name("vampire", ch->name)) return TRUE;
    if (is_name("mummy", ch->name)) return TRUE;
    if (is_name("spectre", ch->name)) return TRUE;
    if (is_name("wraith", ch->name)) return TRUE;
    if (is_name("lich", ch->name)) return TRUE;
    if (is_name("wight", ch->name)) return TRUE;
    if (is_name("ghast", ch->name)) return TRUE;

    return FALSE;
}

void do_turn (CHAR_DATA *ch, char *argument)
{
    int maxlevel;
    OBJ_DATA *obj;
    AFFECT_DATA af;
    CHAR_DATA *vch, *vch_next;

    maxlevel = ch->generation * 2 + 3+(ch->power[DISC_HERO_EXOR] * 3);
    if (ch->tertiaryrace == TERTIARY_AASIMON) maxlevel *= 2;

    send_to_char("You attempt to turn the undead.\n\r", ch);
    act("$n holds $s hands aloft and attempts to banish the undead.", ch, NULL, NULL, TO_ROOM);

    if ((!TIME_UP(ch, TIMER_CANT_TURN) || IS_EVIL(ch) ) && ch->level < 112)
    {
	send_to_char("Your gods seem to have forsaken you.\n\r", ch);
	return;
    }

    act("Bright sunlight fills the room momentarily.", ch, NULL, NULL, TO_CHAR);
    act("Bright sunlight fills the room momentarily.", ch, NULL, NULL, TO_ROOM);

    SET_TIMER(ch, TIMER_CANT_TURN, dice(1, 4) );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next        = vch->next;
	if (vch == ch) continue;

	if ( vch->in_room == NULL )
	    continue;

	if ( vch->in_room != ch->in_room)
	    continue;

	if (!TIME_UP(vch, TIMER_CANT_BE_TURNED))
	{
	    send_to_char("You dont feel afraid.\n\r", vch);
	    continue;
	}

	if ( vch->in_room == ch->in_room
	    && is_undead(vch)
	    && IS_NPC(vch)
	    && vch->level < maxlevel / 2)
	{
	    send_to_char("You disintegrate into a fine powder.\n\r", vch);
	    act("$n disintegrates into a fine powder.", vch, NULL, NULL, TO_ROOM);
	    obj = create_object(get_obj_index(67), 0);
	    obj_to_room(obj, vch->in_room, vch->altitude);
	    destroy_char(ch, vch, FALSE);
	}

	else if ( vch->in_room == ch->in_room
	    && is_undead(vch)
	    && vch->level < maxlevel)
	{
    	    af.where     = TO_NEWFLAGS;
    	    af.type      = skill_lookup("fear");
    	    af.level     = ch->level;
    	    af.duration  = 1;
    	    if (ch->power[DISC_HERO_EXOR] > 4) af.duration +=1;
    	    af.location  = APPLY_NONE;
    	    af.modifier  = 0;
    	    af.bitvector = AFF_FEAR;
    	    affect_to_char( vch, &af );
    	    act( "$n runs away in panic and terror.", vch, NULL, NULL, TO_ROOM );
	    SET_TIMER(vch, TIMER_CANT_BE_TURNED, 20-ch->power[DISC_HERO_EXOR]);
    	    exec_fear(vch);
	}
    }
    WAIT_STATE(ch, 12);

}




void do_tome (CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj = NULL;
    OBJ_DATA *foundobj = NULL;
    int faithneed;    

    argument = one_argument( argument, arg1);
    argument = one_argument( argument, arg2);
    argument = one_argument( argument, arg3);
    
    if (ch->power[DISC_WARL_BIND] == 0)    
    {
	send_to_char("You need more experience in the Binding discipline.\n\r",ch);
	return;
    }

    if (arg1[0] == '\0')
    {
	send_to_char("Syntax : tome create                    :   create a tome.\n\r", ch);
	send_to_char("         tome <name> read               :   read a tome.\n\r", ch);
	send_to_char("         tome <name> improve            :   improve a tome's spell level.\n\r", ch);
	send_to_char("         tome <name> healing            :   improve a tome's healing power.\n\r", ch);
	send_to_char("         tome <name> attack             :   improve a tome's attack power.\n\r", ch);
	send_to_char("         tome <name> scribe <spell>     :   add a spell.\n\r", ch);
	send_to_char("         tome <name> rename <new name>  :   rename a tome.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "create") )
    {
	if (ch->qp == 0)
	{
	    send_to_char("You need kai to create a spellbook.\n\r", ch);
	    return;
	}
	ch->qp -= 1;
	send_to_char("Wisps of intangible magical energy swirl into the form of a spellbook.\n\r", ch);
	act("Wisps of magical energy swirl around $n and then form into a spellbook.", ch, NULL, NULL, TO_ROOM);
	do_anything(ch, 8);
	obj = create_object( get_obj_index(65), 0);
	obj_to_char(obj, ch);
	return;
    }

    for ( obj = ch->carrying ; obj != NULL ; obj = obj->next_content)
    {
	if ( can_see_obj (ch, obj) && is_name (arg1, obj->name))
	foundobj = obj;
    }

    if (foundobj == NULL)
    {
	send_to_char("You have not got that item in your inventory.\n\r", ch);
	return;
    }

    if (foundobj->item_type != ITEM_TOME)
    {
	send_to_char("That item is not a spellbook.\n\r", ch);
	return;
    }

    if (arg2[0] == '\0')
    {
	send_to_char("Syntax : tome create                    :   create a tome.\n\r", ch);
	send_to_char("         tome <name> read               :   read a tome.\n\r", ch);
	send_to_char("         tome <name> improve            :   improve a tome's spell level.\n\r", ch);
	send_to_char("         tome <name> healing            :   improve a tome's healing power.\n\r", ch);
	send_to_char("         tome <name> attack             :   improve a tome's attack power.\n\r", ch);
	send_to_char("         tome <name> scribe <spell>     :   add a spell.\n\r", ch);
	send_to_char("         tome <name> rename <new name>  :   rename a tome.\n\r", ch);
	return;
    }

    
    if (!str_prefix(arg2, "improve") )
    {
	if (ch->power[DISC_WARL_BIND] < 2)
	{
	    send_to_char("You need to have reached level two in Binding to improve a spellbook.\n\r", ch);
	    return;
	}
	
	faithneed = foundobj->value[0] / 12;

	if (ch->qp < faithneed)
	{
	    sprintf(buf, "You need %d kai to improve the level of %s.",
		faithneed, foundobj->short_descr);
	    send_to_char(buf, ch);
	    return;
	}

	ch->qp -= faithneed;
	foundobj->value[0] += 1;

	if (dice(1, 10) == 2)
	{
	    do_anything(ch, ch->power[DISC_WARL_BIND]);
	    foundobj->value[0] += ch->power[DISC_WARL_BIND];
	}

	WAIT_STATE(ch, 8);
	act("Using your magical knowledge, you rewrite the spells on $p.", ch, foundobj, NULL, TO_CHAR);
	return;
    }
    
    if (!str_prefix(arg2, "attacking") )
    {
	if (ch->power[DISC_WARL_BIND] < 4)
	{
	    send_to_char("You need to have reached level four in Binding to improve attack.\n\r", ch);
	    return;
	}
	
	faithneed = foundobj->value[3] / 8;
	if (ch->qp < faithneed)
	{
	    sprintf(buf, "You need %d kai to improve the attack power of %s.",
		faithneed, foundobj->short_descr);
	    send_to_char(buf, ch);
	    return;
	}
	ch->qp -= faithneed;
	foundobj->value[3] += dice(1, 3);

	if (dice(1, 10) == 2)
	{
	    do_anything(ch, ch->power[DISC_WARL_BIND]);
	    foundobj->value[3] += ch->power[DISC_WARL_BIND];
	}

	act("You improve the attacking power of $p.", ch, foundobj, NULL, TO_CHAR);
	WAIT_STATE(ch, 8);
	return;
    }
    
    if (!str_prefix(arg2, "healing") )
    {
	if (ch->power[DISC_WARL_BIND] < 3)
	{
	    send_to_char("You need to have reached level three in Binding to improve healing.\n\r", ch);
	    return;
	}
	
	faithneed = foundobj->value[2] / 7;
	if (ch->qp < faithneed)
	{
	    sprintf(buf, "You need %d kai to improve the healing power of %s.",
		faithneed, foundobj->short_descr);
	    send_to_char(buf, ch);
	    return;
	}
	ch->qp -= faithneed;

	foundobj->value[2] += dice(1, 3);
	if (dice(1, 10) == 2)
	{
	    do_anything(ch, ch->power[DISC_WARL_BIND]);
	    foundobj->value[2] += ch->power[DISC_WARL_BIND];
	}
	act("You improve the healing power of $p.", ch, foundobj, NULL, TO_CHAR);
	WAIT_STATE(ch, 8);
	return;
    }

    if (!str_prefix(arg2, "rename") )
    {
	if (ch->qp < 1)
	{
	    send_to_char("It costs a kai point to rename a tome.\n\r", ch);
	    return;
	}
	
	if (arg3[0] == '\0')
	{
	    act("What do you wish to rename $p to?", ch, foundobj, NULL, TO_CHAR);
	    return;
	}

	ch->qp -= 1;

	act("You have successfully renamed $p.", ch, foundobj, NULL, TO_CHAR);

	free_string(foundobj->short_descr);
	foundobj->short_descr =str_dup(arg3);
	free_string(foundobj->name);
	foundobj->name=str_dup(arg3);
	
	foundobj->value[0] -= 1;
	if (foundobj->value[0] < 1) foundobj->value[0] = 1;
	return;
    }

    if (!str_prefix(arg2, "scribe"))
    {
	faithneed = foundobj->value[1] + 1;
	
	if (ch->power[DISC_WARL_BIND] < 3)
	{
	    send_to_char("You need to have reached level three in Binding to scribe spells.\n\r", ch);
	    return;
	}
	
	if (arg3[0] == '\0')
	{
	    send_to_char("You may scribe one of the following : speed, heal, selfheal, armor\n\r", ch);
	    send_to_char(" shroud, plague, flameball, blast, sunbeam, venom\n\r.", ch);
	    return;
	}

	if (ch->qp < faithneed)
	{
	    sprintf(buf, "It will cost %d kai to add another spell.",
	       faithneed);
	    send_to_char(buf, ch);
	    return;
	}

	if (!str_prefix(arg3, "flameball") )
	{
	    if (IS_SET(foundobj->value[4], TOME_FLAMEBALL) )
	    {
		send_to_char("This spellbook already contains the flameball spell.\n\r", ch);
		return;
	    }
	    ch->qp -= faithneed;
	    do_anything (ch, dice (1, 6));
	    SET_BIT(foundobj->value[4], TOME_FLAMEBALL);
	    foundobj->value[1] += 1;
	    act("You scribe the runes of flameball into $p.", ch, foundobj, NULL, TO_CHAR);
	    return;
	}
	
	if (!str_prefix(arg3, "speed") )
	{
	    if (IS_SET(foundobj->value[4], TOME_SPEED) )
	    {
		send_to_char("This spellbook already contains the speed spell.\n\r", ch);
		return;
	    }
	    ch->qp -= faithneed;
	    do_anything (ch, dice (1, 6));
	    SET_BIT(foundobj->value[4], TOME_SPEED);
	    foundobj->value[1] += 1;
	    act("You scribe the runes of speed into $p.", ch, foundobj, NULL, TO_CHAR);
	    return;
	}
     
	if (!str_prefix(arg3, "heal") )
	{
	    if (IS_SET(foundobj->value[4], TOME_HEAL) )
	    {
		send_to_char("This spellbook already contains the heal spell.\n\r", ch);
		return;
	    }
	    ch->qp -= faithneed;
	    do_anything (ch, dice (1, 6));
	    SET_BIT(foundobj->value[4], TOME_HEAL);
	    foundobj->value[1] += 1;
	    act("You scribe the runes of heal into $p.", ch, foundobj, NULL, TO_CHAR);
	    return;
	}
     
	if (!str_prefix(arg3, "selfheal") )
	{
	    if (IS_SET(foundobj->value[4], TOME_SELFHEAL) )
	    {
		send_to_char("This spellbook already contains the selfheal spell.\n\r", ch);
		return;
	    }
	    ch->qp -= faithneed;
	    do_anything (ch, dice (1, 6));
	    SET_BIT(foundobj->value[4], TOME_SELFHEAL);
	    foundobj->value[1] += 1;
	    act("You scribe the runes of extra healing into $p.", ch, foundobj, NULL, TO_CHAR);
	    return;
	}
     
	if (!str_prefix(arg3, "armor") )
	{
	    if (IS_SET(foundobj->value[4], TOME_ARMOR) )
	    {
		send_to_char("This spellbook already contains the armor spell.\n\r", ch);
		return;
	    }
	    ch->qp -= faithneed;
	    do_anything (ch, dice (1, 6));
	    SET_BIT(foundobj->value[4], TOME_ARMOR);
	    foundobj->value[1] += 1;
	    act("You scribe the runes of armor into $p.", ch, foundobj, NULL, TO_CHAR);
	    return;
	}
     
	if (!str_prefix(arg3, "shroud") )
	{
	    if (IS_SET(foundobj->value[4], TOME_SHROUD) )
	    {
		send_to_char("This spellbook already contains the shroud spell.\n\r", ch);
		return;
	    }
	    ch->qp -= faithneed;
	    do_anything (ch, dice (1, 6));
	    SET_BIT(foundobj->value[4], TOME_SHROUD);
	    foundobj->value[1] += 1;
	    act("You scribe the runes of shroud into $p.", ch, foundobj, NULL, TO_CHAR);
	    return;
	}
     
	if (!str_prefix(arg3, "plague") )
	{
	    if (IS_SET(foundobj->value[4], TOME_PLAGUE) )
	    {
		send_to_char("This spellbook already contains the plague spell.\n\r", ch);
		return;
	    }
	    ch->qp -= faithneed;
	    do_anything (ch, dice (1, 6));
	    SET_BIT(foundobj->value[4], TOME_PLAGUE);
	    foundobj->value[1] += 1;
	    act("You scribe the runes of plague into $p.", ch, foundobj, NULL, TO_CHAR);
	    return;
	}
     
	if (!str_prefix(arg3, "blast") )
	{
	    if (IS_SET(foundobj->value[4], TOME_BLAST) )
	    {
		send_to_char("This spellbook already contains the blast spell.\n\r", ch);
		return;
	    }
	    ch->qp -= faithneed;
	    do_anything (ch, dice (1, 6));
	    SET_BIT(foundobj->value[4], TOME_BLAST);
	    foundobj->value[1] += 1;
	    act("You scribe the runes of blast into $p.", ch, foundobj, NULL, TO_CHAR);
	    return;
	}
     
	if (!str_prefix(arg3, "sunbeam") )
	{
	    if (IS_SET(foundobj->value[4], TOME_SUNBEAM) )
	    {
		send_to_char("This spellbook already contains the sunbeam spell.\n\r", ch);
		return;
	    }
	    ch->qp -= faithneed;
	    do_anything (ch, dice (1, 6));
	    SET_BIT(foundobj->value[4], TOME_SUNBEAM);
	    foundobj->value[1] += 1;
	    act("You scribe the runes of sunbeam into $p.", ch, foundobj, NULL, TO_CHAR);
	    return;
	}
     
	if (!str_prefix(arg3, "venom") )
	{
	    if (IS_SET(foundobj->value[4], TOME_VENOM) )
	    {
		send_to_char("This spellbook already contains the venom spell.\n\r", ch);
		return;
	    }
	    ch->qp -= faithneed;
	    do_anything (ch, dice (1, 6));
	    SET_BIT(foundobj->value[4], TOME_VENOM);
	    foundobj->value[1] += 1;
	    act("You scribe the runes of venom into $p.", ch, foundobj, NULL, TO_CHAR);
	    return;
	}
     
	if (!str_prefix(arg3, "shatter") )
	{
	    if (IS_SET(foundobj->value[4], TOME_SHATTER) )
	    {
		send_to_char("This spellbook already contains the shatter spell.\n\r", ch);
		return;
	    }
	    ch->qp -= faithneed;
	    do_anything (ch, dice (1, 6));
	    SET_BIT(foundobj->value[4], TOME_SHATTER);
	    foundobj->value[1] += 1;
	    act("You scribe the runes of shatter into $p.", ch, foundobj, NULL, TO_CHAR);
	    return;
	}
	send_to_char("You may scribe one of the following : speed, heal, selfheal, armor\n\r", ch);
	send_to_char(" shroud, plague, flameball, blast, sunbeam, venom.\n\r.", ch);
	return;
    }
    
    if (!str_prefix(arg2, "read") )
    {
	bool foundspell=FALSE;

	act("You read the $p.", ch, foundobj, NULL, TO_CHAR);
	sprintf(buf, "The level of the tome is %d.\n\r", foundobj->value[0]);
	send_to_char(buf, ch);
	sprintf(buf, "Attack power has been improved to level %d.\n\r", foundobj->value[3]);
	send_to_char(buf, ch);
	sprintf(buf, "Healing power has been improved to level %d.\n\r", foundobj->value[2]);
	send_to_char(buf, ch);
	if (IS_SET(foundobj->value[4], TOME_FLAMEBALL) )
	{
	    send_to_char("It contains the spell 'Flameball'.\n\r", ch);
	    foundspell = TRUE;
	}
	if (IS_SET(foundobj->value[4], TOME_HEAL) )
	{
	    send_to_char("It contains the spell 'Healing'.\n\r", ch);
	    foundspell = TRUE;
	}
	if (IS_SET(foundobj->value[4], TOME_SELFHEAL) )
	{
	    send_to_char("It contains the spell 'Extra Healing'.\n\r", ch);
	    foundspell = TRUE;
	}
	if (IS_SET(foundobj->value[4], TOME_SPEED) )
	{
	    send_to_char("It contains the spell 'Speed'.\n\r", ch);
	    foundspell = TRUE;
	}
	if (IS_SET(foundobj->value[4], TOME_SHATTER) )
	{
	    send_to_char("It contains the spell 'Shatter'.\n\r", ch);
	    foundspell = TRUE;
	}
	if (IS_SET(foundobj->value[4], TOME_VENOM) )
	{
	    send_to_char("It contains the spell 'Venom'.\n\r", ch);
	    foundspell = TRUE;
	}
	if (IS_SET(foundobj->value[4], TOME_SUNBEAM) )
	{
	    send_to_char("It contains the spell 'Sunbeam'.\n\r", ch);
	    foundspell = TRUE;
	}
	if (IS_SET(foundobj->value[4], TOME_SHROUD) )
	{
	    send_to_char("It contains the spell 'Shroud'.\n\r", ch);
	    foundspell = TRUE;
	}
	if (IS_SET(foundobj->value[4], TOME_ARMOR) )
	{
	    send_to_char("It contains the spell 'Armor'.\n\r", ch);
	    foundspell = TRUE;
	}
	if (IS_SET(foundobj->value[4], TOME_PLAGUE) )
	{
	    send_to_char("It contains the spell 'Plague'.\n\r", ch);
	    foundspell = TRUE;
	}
	if (IS_SET(foundobj->value[4], TOME_BLAST) )
	{
	    send_to_char("It contains the spell 'Blast'.\n\r", ch);
	    foundspell = TRUE;
	}
	sprintf(buf, "Adding an extra spell will cost %d kai.\n\r", foundobj->value[1] + 1);
	send_to_char(buf, ch);
	
	if (!foundspell)
	    send_to_char("No spells have yet been scribed into this tome.\n\r", ch);
	return;
    }    
    do_tome(ch, "");
}



void do_mesmerise (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Whom do you wish to mesmerise?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
 
    if (victim == ch)
    {
	send_to_char("You can't stare at yourself.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_MESMERISE) )
    {
	send_to_char("Your head is still spinning from the last attempt.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_FLYING) )
    {
        send_to_char("Your victim must be touching the ground.\n\r", ch);
        return;
    }

    if ( IS_NEWFLAG(victim, AFF_HOLD)
	|| !TIME_UP(victim, TIMER_MESMERISED) )
    {
        send_to_char("They are already staring blankly into space.\n\r",ch);
        return;
    }
    
    act("$n's eyes start to swirl hypnotically.", ch, NULL, NULL, TO_ROOM);
    
     if ( victim->generation - ch->power[DISC_LUPI_COBR] + dice (1, 6) > ch->generation)
     {
	act("$N shakes off your attempt to mesmerise $M.", ch, NULL, victim, TO_CHAR);
	act("You shake off $n's attempt to mesmerise you.", ch, NULL, victim, TO_VICT);
	return;
     }

     SET_TIMER(ch, TIMER_MESMERISE, 4);
     SET_TIMER(victim, TIMER_MESMERISED, ch->generation / 10 +1);
     send_to_char( "You like to stare at the pretty colours....\n\r", victim );
     act("$N stares blankly into space.", ch, NULL, victim, TO_CHAR);
     WAIT_STATE(ch, 10);
}


void do_destroy (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );
    
    if (ch->power[DISC_DAEM_HELL] < 5)
    {
	send_to_char("Only true masters of the hellfire discipline may use this ability.\n\r", ch);
	return;
    }


    if ( arg[0] == '\0' )
    {
	send_to_char( "Which daemon do you wish to destroy?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char("You cannot destroy yourself.\n\r",ch);
	return;
    }

    if ( victim->secondaryrace != ch->secondaryrace && ch->level!=120)
    {
	send_to_char("You may only destroy daemons.\n\r",ch);
	return;
    }

    if ( victim->generation >= ch->generation)
    {
	send_to_char("They are too powerful to destroy.\n\r",ch);
	return;
    }
    
    if (ch->generation < 12)
    {
	send_to_char("Only Pit Fiends or above may use the destroy ability.\n\r", ch);
	return;
    }

    if ( !is_same_clan(ch,victim) )
    {
	send_to_char("They are not in your clan.\n\r", ch);
	return;
    }

    ch->alignment -= 30;
    if (ch->alignment < -1000) ch->alignment = -1000;

    act("You breathe out a huge gout of abyssal flame and incinerate $N.", ch, NULL, victim, TO_CHAR);
    act("$n breathes out a huge gout of abyssal flame and incinerates you.", ch, NULL, victim, TO_VICT);
    act("$n breathes out a huge gout of flame and incinerates $N.", ch, NULL, victim, TO_NOTVICT);
    send_to_char("You have been KILLED!", victim);
    act("$n drops dead.", victim, NULL, NULL, TO_ROOM);
    victim->hit = -10;
    victim->move /= 3;
}


void do_kaiprayer (CHAR_DATA *ch, char *argument)        
{
    if (ch->qp == 0)
    {
	send_to_char("You have no kai, and may not attempt this kind of prayer.\n\r", ch);
	return;
    }
    
    if (ch->value[5] == 0)
    {
	send_to_char("There is no need, the gods have deemed you worthy.\n\r", ch);
	return;
    }

    ch->qp -= 1;
    ch->value[5] = 0;

    send_to_char("You get down on your knees and mumble a few apologies to the gods.\n\r", ch);
    act("$n mumbles a few apologies to the gods.", ch, NULL, NULL, TO_ROOM);
}

void do_sip (CHAR_DATA *ch, char *argument)        
{
    OBJ_DATA *obj = NULL, *foundobj = NULL;
    char buf[MAX_STRING_LENGTH];

    if (ch->power[DISC_WARL_BREW] < 4)
    {
	send_to_char("You are not experienced enough in the brewing discipline.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Which potion do you wish to take a sip of?\n\r", ch);
	return;
    }

    for ( obj = ch->carrying ; obj != NULL ; obj = obj->next_content)
    {
	if ( can_see_obj (ch, obj) && is_name (argument, obj->name))
	foundobj = obj;
    }

    if (foundobj == NULL)
    {
	send_to_char("You have not got that item in your inventory.\n\r", ch);
	return;
    }

    if (foundobj->item_type != ITEM_POTION)
    {
	send_to_char("You may only sip from potions.\n\r", ch);
	return;
    }
    
    send_to_char("You take a tiny sip from the potion.\n\r", ch);    
    act ("$n takes a sip from $p.", ch, foundobj, NULL, TO_ROOM);

    if (ch->power[DISC_WARL_BREW] == 4 && dice (1, 3) == 1 && foundobj->value[1] > 0)
    {
	send_to_char("Oops.... drunk a little bit too much!\n\r", ch);
	obj_cast_spell( foundobj->value[1], foundobj->value[0], ch, ch, NULL );
	foundobj->value[1] = 0;
    }

    WAIT_STATE(ch, 8);
    do_anything (ch, ch->power[DISC_WARL_BREW]);
    send_to_char("Mmmmm, tasty.  You deduce the following information about the potion.\n\r", ch);
    sprintf( buf, "Level %d spells of:", foundobj->value[0] );
    send_to_char( buf, ch );

    if ( foundobj->value[1] >= 0 && foundobj->value[1] < MAX_SKILL )
    {
	send_to_char( " '", ch );
	send_to_char( skill_table[foundobj->value[1]].name, ch );
	send_to_char( "'", ch );
    }

    if ( foundobj->value[2] >= 0 && foundobj->value[2] < MAX_SKILL )
    {
	send_to_char( " '", ch );
	send_to_char( skill_table[foundobj->value[2]].name, ch );
	send_to_char( "'", ch );
    }

    if ( foundobj->value[3] >= 0 && foundobj->value[3] < MAX_SKILL )
    {
	send_to_char( " '", ch );
	send_to_char( skill_table[foundobj->value[3]].name, ch );
	send_to_char( "'", ch );
    }

    if (foundobj->value[4] >= 0 && foundobj->value[4] < MAX_SKILL)
    {
	send_to_char(" '",ch);
	send_to_char(skill_table[foundobj->value[4]].name,ch);
	send_to_char("'",ch);
    }

    send_to_char( ".\n\r", ch );
}

void do_brew(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj = NULL;
    int i, found=0;
    OBJ_DATA *component = NULL;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    bool cauld = FALSE;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    
    SS(0);

    if (ch->power[DISC_WARL_BREW] < 1)
    {
	send_to_char("You have not yet mastered this discipline.\n\r", ch);
	return;
    }

    if (!str_cmp(arg1, "beer"))
    {
        component = get_obj_by_type(ch, ITEM_EMPTY_VIAL);

        if ( component == NULL && ch->level < 112 )
        {
	    send_to_char("You need to be holding an empty potion bottle to brew a potion.\n\r", ch);
	    return;
        }
	stc("You brew a bottle of beer.\n\r", ch);
	act("$n brews a bottle of beer.", ch, NULL, NULL, TO_ROOM);
	if (component != NULL) extract_obj(component);
	obj = create_object( get_obj_index(48), 0);
	obj->item_type = ITEM_SPEC_POTION;
	obj->value[1] = POTION_BEER;
	obj_to_char(obj, ch);
	return;
    }

    SS(1);

    if (arg1[0] == '\0')
    {
	send_to_char("What spell do you wish to brew?\n\r", ch);
	return;
    }
   
    for (i=0 ; i<MAX_SKILL ; i++)
    {
	if (skill_table[i].name != NULL 
	    && !str_prefix(arg1, skill_table[i].name) && found == 0) 
	    found = i;
    }

    SS(2);

    if (found == 0 || skill_table[found].realm == 0)
    {
	send_to_char("No such spell exists.\n\r", ch);
	return;
    }

    if (skill_table[found].spell_fun == spell_null
    || ch->pcdata->learned[found] == 0)
    {
	send_to_char("You don't know any spells of that name.\n\r", ch);
	return;
    }

    SS(3);

    if (ch->alignment > -500 && skill_table[found].realm == 5 && ch->level <112)
    {
	send_to_char("You could never create such an evil brew.\n\r", ch);
	return;
    }

    if (skill_table[found].target != TAR_CHAR_DEFENSIVE)
    {
	send_to_char("You may only brew defensive spells.\n\r", ch);
	return;
    }

    if (skill_table[found].level > ch->power[DISC_WARL_BREW] )
    {
	stc("You need to be better at brewing to brew this spell.\n\r",ch);
	return;
    }

    SS(4);

    if (!str_prefix(arg2, "into") && arg2[0] != '\0')
    {
	if (arg3[0] == '\0')
	{
	    send_to_char("What do you wish to brew the spell into?\n\r", ch);
	    return;
	}
	if (ch->power[DISC_WARL_BREW] < 3)
	{
	    send_to_char("You need to be more experienced at the brewing discipline.\n\r", ch);
	    return;
	}
	
	cauld = TRUE;
    }

    component = get_obj_by_type(ch, ITEM_EMPTY_VIAL);

    if ( component==NULL && ch->level < 112 && !cauld )
    {
	send_to_char("You need to be holding an empty potion bottle to brew a potion.\n\r", ch);
	return;
    }

    if (cauld)
    {
	obj = get_obj_list( ch, arg3, ch->in_room->contents );
	
	if ( obj == NULL )
	{
	    act( "I see no $T here.", ch, NULL, arg3, TO_CHAR );
	    return;
	}
	
	if (obj->item_type != ITEM_CAULDRON)
	{
	    send_to_char("That is not a cauldron.\n\r", ch);
	    return;
	}
	
	if (obj->value[1] != 0)
	{
	    send_to_char("That cauldron is not empty.\n\r", ch);
	    return;
	}

	if (dice(1, ch->power[DISC_WARL_BREW] * 4) == 1 )
	{
	    send_to_char("You failed to brew the mixture.\n\r", ch);
	    return;
	}

        SS(5);

	if (dice(1, ch->power[DISC_WARL_BREW] * 3) == 1 
	    && ch->power[DISC_WARL_BREW] < 5)
	{
	    send_to_char("The cauldron explodes in a ball of flame!\n\r", ch);
	    act("$n's cauldron explodes in a ball of flame.", ch, NULL, NULL, TO_ROOM);
	    damage_old(ch, ch, 50, skill_lookup("fireball"), DAM_FIRE, TRUE);
	    extract_obj(obj);
	    return;
	}

	if ( (component == NULL
	    || (skill_table[found].realm == 1
		&& str_cmp(component->short_descr,"a glow worm" ) )
	    || (skill_table[found].realm == 2
		&& str_cmp(component->short_descr,"some seaweed" ) )
	    || (skill_table[found].realm == 3
		&& str_cmp(component->short_descr,"some ambergris" ) )
	    || (skill_table[found].realm == 4
		&& str_cmp(component->short_descr,"some powdered aconite" ) )
	    || (skill_table[found].realm == 5
		&& str_cmp(component->short_descr,"a black lotus" ) ) ) 
	    && ch->level != 120 )

	{
	    send_to_char("You are not holding the correct material component.\n\r", ch);
	    return;
	}
	    
	if (component != NULL) extract_obj(component);

	switch(skill_table[found].realm)
	{
	case 1 :
	act("$p starts to fill with a reddish liquid.", ch, obj, NULL, TO_CHAR);
	act("$n fills $p with a reddish liquid.", ch, obj, NULL, TO_ROOM);
	break;        
	
	case 2 :
	act("$p starts to fill with a blue liquid.", ch, obj, NULL, TO_CHAR);
	act("$n fills $p with a blue liquid.", ch, obj, NULL, TO_ROOM);
	break;        

	case 3 :
	act("$p starts to fill with a yellow liquid.", ch, obj, NULL, TO_CHAR);
	act("$n fills $p with a yellow liquid.", ch, obj, NULL, TO_ROOM);
	break;        
	
	case 4 :
	act("$p starts to fill with a greeny liquid.", ch, obj, NULL, TO_CHAR);
	act("$n fills $p with a greeny liquid.", ch, obj, NULL, TO_ROOM);
	break;        
	
	case 5 :
	act("$p starts to fill with a thick black slime.", ch, obj, NULL, TO_CHAR);
	act("$n fills $p with a thick black slime.", ch, obj, NULL, TO_ROOM);
	break;        

	case 6 :
	act("$p starts to fill with a thick white gloop.", ch, obj, NULL, TO_CHAR);
	act("$n fills $p with thick white gloop.", ch, obj, NULL, TO_ROOM);
	break;
        
	default : break;
	}
	
	obj->value[1] = ch->power[DISC_WARL_BREW] + dice (1, 10) + 6;
	obj->value[2] = found;
	if (ch->power[DISC_WARL_BREW] > 3)
	{
	    obj->value[0] = ch->level + 20;
	}

	if (ch->power[DISC_WARL_BREW] > 5) obj->value[3] = found;
		
	WAIT_STATE(ch, 8);
	do_anything(ch, dice(1, 6) );
	return;
    }
    
    
    SS(6);

    send_to_char("The empty vial begins to shudder and pulsate.\n\r", ch);
    if (component != NULL) extract_obj(component);
    
    if (dice(1, ch->power[DISC_WARL_BREW] * 3) == 1 )
    {
	send_to_char("The vial explodes in a ball of flame!\n\r", ch);
	act("$n's potion bottle explodes in a ball of flame.", ch, NULL, NULL, TO_ROOM);
	damage_old(ch, ch, 30, skill_lookup("fireball"), DAM_FIRE, TRUE);
	return;
    }

    if (dice(1, ch->power[DISC_WARL_BREW] * 5) == 1 )
    {
	send_to_char("You failed to brew the potion.\n\r", ch);
	return;
    }

    SS(7);

    switch(skill_table[found].realm)
    {
	case 1 :
	send_to_char("   ...it starts to fill with a reddish liquid.\n\r", ch);
	obj = create_object( get_obj_index(48), 0);
	act("$n has created a potion of dragon ale!", ch, NULL, NULL, TO_ROOM);
	break;

	case 2 :
	send_to_char("   ...it starts to fill with a blue liquid.\n\r", ch);
	obj = create_object( get_obj_index(49), 0);
	act("$n has created an elixir of the moon!", ch, NULL, NULL, TO_ROOM);
	break;
	
	case 3 :
	send_to_char("   ...it starts to fill with a yellow liquid.\n\r", ch);
	obj = create_object( get_obj_index(47), 0);
	act("$n has created a potion of brimstone broth!", ch, NULL, NULL, TO_ROOM);
	break;

	case 4 :
	send_to_char("   ...it starts to fill with a greenish liquid.\n\r", ch);
	obj = create_object( get_obj_index(46), 0);
	act("$n has created a potion of serpent slime!", ch, NULL, NULL, TO_ROOM);
	break;

	case 5 :
	send_to_char("   ...it starts to fill with a black liquid.\n\r", ch);
	obj = create_object( get_obj_index(50), 0);
	act("$n has created a potion of voodoo juice!", ch, NULL, NULL, TO_ROOM);
	break;
    
	case 6 :
	send_to_char("   ...it starts to fill with a white gloop.\n\r", ch);
	obj = create_object( get_obj_index(49), 0);
	act("$n has created a potion of moon elixir!", ch, NULL, NULL, TO_ROOM);
	break;
    
	default :
	break;  
    }
    WAIT_STATE(ch, 8);

    SS(8);

    if (dice(1, 10 * ch->power[DISC_WARL_BREW]) == 13)
    {
	obj->item_type = ITEM_SPEC_POTION;
	obj->value[0] = 100;
	obj->value[1] = POTION_GAIN_BREW;
	return;
    }

    obj->value[1] = found;
    obj->value[0] = 10;
 
    if (ch->power[DISC_WARL_BREW] >1)
	obj->value[0] = ch->level;
  
    if (ch->power[DISC_WARL_BREW] >2)
    {
	if (dice(1, 2) == 1)
	    send_to_char("   ...it starts to leak foul-smelling fumes.\n\r", ch);
	obj->value[2] = found;
    }

    SS(9);

    if (ch->power[DISC_WARL_BREW] >3)
    {
	obj->value[0] = ch->level + 20;
	if (dice (1, 6) == 1)
	{
	    obj->value[3] = 40;
	    send_to_char("   ...it starts to glow with a white light.\n\r", ch);
	    SET_BIT(obj->extra_flags, ITEM_GLOW);
	}
	if (dice (1, 6) == 2)
	{
	    obj->value[4] = 3;
	    send_to_char("   ...it starts to bubble and smoke.\n\r", ch);
	}
    }

    if (ch->power[DISC_WARL_BREW] >4 && skill_table[found].level > 5)
    {
	if (dice (1, 6) == 1)
	{
	    obj->value[3] = 11;
	}
	if (dice (1, 6) == 2)
	{
	    obj->value[3] = 84;
	}
	if (dice (1, 6) == 3)
	{
	    send_to_char("   ...it starts to shake violently.\n\r", ch);
	    obj->value[4] = 85;
	}
	if (dice (1, 6) == 3)
	{
	    obj->value[4] = 120;
	    SET_BIT(obj->extra_flags, ITEM_HUM);
	    send_to_char("   ...it starts to hum evilly.\n\r", ch);
	}
    }
    SS(10);
    do_anything(ch, dice(1, 5) );
    obj_to_char(obj, ch);
}
 
void do_pen(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj, *foundobj = NULL;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    int i, found=0;

    if (ch->power[DISC_WARL_SCRI] < 1)
    {
	send_to_char("You have not yet mastered this discipline.\n\r", ch);
	return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
	send_to_char("What spell do you wish to scribe, and on what?\n\r", ch);
	return;
    }

    for (i=0 ; i<MAX_SKILL ; i++)
    {
	if (skill_table[i].name != NULL
	    && !str_prefix(arg1, skill_table[i].name) && found == 0)
	    found =i;
    }

    if (found == 0 || skill_table[found].realm == 0)
    {
	send_to_char("No such spell exists.\n\r", ch);
	return;
    }

    if (skill_table[found].spell_fun == spell_null
    || ch->pcdata->learned[found] == 0)
    {
	send_to_char("You dont know any spells of that name.\n\r", ch);
	return;
    }

    if (ch->alignment > -500 && skill_table[found].realm == 5 && ch->level <112)
    {
	send_to_char("You could never pen such an evil scroll.\n\r", ch);
	return;
    }

    if (arg2[0] == 0)
    {
	send_to_char("On which scroll do you wish to write?\n\r", ch);
	return;
    }

    for ( obj = ch->carrying ; obj != NULL ; obj = obj->next_content)
    {
	if ( can_see_obj (ch, obj) && is_name (arg2, obj->name))
	foundobj = obj;
    }

    if (foundobj == NULL)
    {
	send_to_char("You have not got that item in your inventory.\n\r", ch);
	return;
    }

    if (foundobj->item_type != ITEM_SCROLL)
    {
	send_to_char("You may only pen spells upon scrolls.\n\r", ch);
	return;
    }

    if (foundobj->value[1] != -1
	&& foundobj->value[2] !=-1
	&& foundobj->value[3] !=-1
	&& foundobj->value[4] !=-1)
    {
	send_to_char("There is no room left on this scroll for more spells\n\r", ch);
	return;
    }

    if (ch->faith < (skill_table[found].level * 3) +1 )
    {
	sprintf(buf, "You need to expend %d faith to pen this spell.\n\r", (skill_table[found].level*20) +1);
	send_to_char(buf, ch);
	return;
    }

    ch->faith -= skill_table[found].level * 3 +1;

    if (ch->power[DISC_WARL_SCRI] * 30 < dice (1, 100) )
    {
	send_to_char("You have failed to pen the spell, luckily the scroll is not ruined.\n\r", ch);
	return;
    }

    if (ch->power[DISC_WARL_SCRI] * 40 < dice (1, 100) )
    {
	send_to_char("The scroll catches fire and begins to burn.\n\r", ch);
	foundobj->short_descr = str_dup("a piece of burning paper");
	foundobj->name = str_dup("burning paper");
	foundobj->description = str_dup("A piece of paper is burning on the floor.");
	foundobj->item_type = ITEM_TRASH;
	foundobj->timer = 1;
	SET_BIT(foundobj->extra_flags, ITEM_FLAMING);
	return;
    }

    if (dice(1, ch->power[DISC_WARL_SCRI] * 6) == 1 )
    {
	send_to_char("The scroll explodes, sending bolts of electricity everywhere!\n\r", ch);
	act("$n's scroll explodes, sending bolts of electricity everywhere.", ch, NULL, NULL, TO_ROOM);
	damage_old(ch, ch, 50, skill_lookup("lightning bolt"), DAM_LIGHTNING, TRUE);
	extract_obj(foundobj);
	return;
    }

    if (dice(1, ch->power[DISC_WARL_SCRI] * 4) == 1 )
    {
	send_to_char("The scroll explodes in a ball of flame!\n\r", ch);
	act("$n's scroll explodes in a ball of flame.", ch, NULL, NULL, TO_ROOM);
	damage_old(ch, ch, 30, skill_lookup("fireball"), DAM_FIRE, TRUE);
	extract_obj(foundobj);
	return;
    }

    act("$n starts writing magical symbols on a scroll.", ch, NULL, NULL, TO_ROOM);
    if (skill_table[found].realm == 1)
    {    
	foundobj->short_descr = str_dup("a blood-red scroll");
	foundobj->name = str_dup("blood red scroll");
	foundobj->description = str_dup("A piece of blood-red parchment lies on the floor.");
    }
    if (skill_table[found].realm == 2)
    {    
	foundobj->short_descr = str_dup("a midnight-blue scroll");
	foundobj->name = str_dup("midnight blue scroll");
	foundobj->description = str_dup("A piece of midnight-blue parchment lies on the floor.");
    }
    if (skill_table[found].realm == 3)
    {    
	foundobj->short_descr = str_dup("a light-blue scroll");
	foundobj->name = str_dup("light blue scroll");
	foundobj->description = str_dup("A piece of light-blue parchment lies on the floor.");
    }
    if (skill_table[found].realm == 4)
    {    
	foundobj->short_descr = str_dup("a muddy scroll");
	foundobj->name = str_dup("muddy scroll");
	foundobj->description = str_dup("A piece of muddy paper lies on the floor.");
    }
    if (skill_table[found].realm == 5)
    {    
	foundobj->short_descr = str_dup("a blackened scroll");
	foundobj->name = str_dup("blackened scroll");
	foundobj->description = str_dup("A dark black scroll lies on the floor.");
    }

    if (ch->power[DISC_WARL_SCRI] == 3)
    {
	send_to_char("The scroll glows with a feint illumination.\n\r", ch);
	foundobj->value[0] +=5;
    }
    if (ch->power[DISC_WARL_SCRI] == 4)
    {
	send_to_char("The scroll glows with a silvery light.\n\r", ch);
	foundobj->value[0] +=10;
    }
    if (ch->power[DISC_WARL_SCRI] == 5)
    {
	send_to_char("The scroll blazes a brilliant gold.\n\r", ch);
	SET_BIT(foundobj->extra_flags, ITEM_GLOW);
	foundobj->value[0] +=15;
    }
    WAIT_STATE(ch, 8);
    if (foundobj->value[1] == -1)
    {
	sprintf(buf, "You scribe the spell of %s upon the scroll.\n\r",
	    skill_table[found].name);
	send_to_char(buf, ch);
	foundobj->value[1] = found;
	return;
    }
    if (foundobj->value[2] == -1)
    {
	sprintf(buf, "You scribe the spell of %s upon the scroll.\n\r",
	    skill_table[found].name);
	send_to_char(buf, ch);
	foundobj->value[2] = found;
	return;
    }
    if (foundobj->value[3] == -1)
    {
	sprintf(buf, "You scribe the spell of %s upon the scroll.\n\r",
	    skill_table[found].name);
	send_to_char(buf, ch);
	foundobj->value[3] = found;
	return;
    }
    if (foundobj->value[4] == -1)
    {
	sprintf(buf, "You scribe the spell of %s upon the scroll.\n\r",
	    skill_table[found].name);
	send_to_char(buf, ch);
	foundobj->value[4] = found;
	return;
    }
    do_anything (ch, ch->generation /2);
}



void do_enchant(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj = NULL, *component = NULL;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    int i, found=0;

    if (ch->power[DISC_WARL_ENCH] < 1)
    {
	send_to_char("You have not yet mastered this discipline.\n\r", ch);
	return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("Syntax : enchant <spell> [wand staff]\n\r", ch);
	return;
    }

    for (i=0 ; i<MAX_SKILL ; i++)
    {
	if (skill_table[i].name != NULL
	    && !str_prefix(arg1, skill_table[i].name) && found == 0)
	    found =i;
    }

    if (found == 0 || skill_table[found].realm == 0)
    {
	send_to_char("No such spell exists.\n\r", ch);
	return;
    }

    if (skill_table[found].spell_fun == spell_null
    || ch->pcdata->learned[found] == 0)
    {
	send_to_char("You dont know any spells of that name.\n\r", ch);
	return;
    }

    if (skill_table[found].realm == REALM_LIFE)
    {
	stc("You cannot trap good magic within a receptacle.\n\r", ch);	
	return;
    }

    if (str_cmp(arg2, "staff") && str_cmp(arg2, "wand") )
    {
	send_to_char("What do you wish to enchant, a wand or a staff?\n\r", ch);
	return;
    }

    if (ch->alignment > -500 && skill_table[found].realm == 5 && ch->level <112)
    {
	send_to_char("You could not commit such an evil act.\n\r", ch);
	return;
    }

    if (!str_cmp(arg2, "staff") )
    {
	component = get_obj_by_name(ch, "an oaken branch");

	if (component == NULL && ch->level < 112)
	{
	    send_to_char("You need to be holding an oaken branch to create a staff.\n\r", ch);
	    return;
	}
    }

    if (!str_cmp(arg2, "wand") )
    {
	component = get_obj_by_name(ch, "a wooden stick");

	if (component == NULL && ch->level < 112)
	{
	    send_to_char("You need to be holding a wooden stick to create a wand.\n\r", ch);
	    return;
	}
    }

    /* Ha! Thats all 10 levels sorted then isnt it */
    if (ch->power[DISC_WARL_ENCH] < skill_table[found].level)
    {
	stc("You are not experienced enough in enchanting for this spell.\n\r", ch);
	return;
    }

    if (ch->faith < skill_table[found].level + 1 )
    {
	sprintf(buf, "You need to expend %d faith to create this item.\n\r", (skill_table[found].level * 3) +1);
	send_to_char(buf, ch);
	return;
    }

    ch->faith -= (skill_table[found].level + 1);
    
    extract_obj(component);

    if (!str_cmp(arg2, "wand") )
    {
    send_to_char("The stick begins to shudder and vibrate.\n\r", ch);
    WAIT_STATE(ch, 8);
    if (dice(1, ch->power[DISC_WARL_ENCH] * 5) == 1 )
    {
	send_to_char("   ...it explodes, spraying acid everywhere!\n\r", ch);
	act("$n's wand explodes, spraying acid everywhere.", ch, NULL, NULL, TO_ROOM);
	damage_old(ch, ch, ch->generation * 50, skill_lookup("acid blast"), DAM_ACID, TRUE);
	return;
    }
    if (dice(1, ch->power[DISC_WARL_ENCH] * 3) == 1 )
    {
	send_to_char("   ...it explodes in a ball of flame!\n\r", ch);
	act("$n's wand explodes in a ball of flame.", ch, NULL, NULL, TO_ROOM);
	damage_old(ch, ch, ch->generation * 30, skill_lookup("fireball"), DAM_FIRE, TRUE);
	return;
    }
    
    switch(skill_table[found].realm)
    {
	case 1 :
	send_to_char("   ...it bursts into flames.\n\r", ch);
	obj = create_object( get_obj_index(55), 0);
	act("$n has created a wand of elemental fire!", ch, NULL, NULL, TO_ROOM);
	break;

	case 2 :
	send_to_char("   ...it starts to drip with water.\n\r", ch);
	obj = create_object( get_obj_index(56), 0);
	act("$n has created a wand of elemental water!", ch, NULL, NULL, TO_ROOM);
	break;
	
	case 3 :
	send_to_char("   ...it starts to glow brightly.\n\r", ch);
	obj = create_object( get_obj_index(57), 0);
	act("$n has created a wand of elemental air!", ch, NULL, NULL, TO_ROOM);
	break;

	case 4 :
	send_to_char("   ...it is now covered with thick brown mud.\n\r", ch);
	obj = create_object( get_obj_index(58), 0);
	act("$n has created a wand of elemental earth!", ch, NULL, NULL, TO_ROOM);
	break;

	case 5 :
	send_to_char("   ...it starts to blacken and char before your eyes.\n\r", ch);
	obj = create_object( get_obj_index(59), 0);
	act("$n has created a wand of elemental evil!", ch, NULL, NULL, TO_ROOM);
	break;
    
	default :
	break;  
    }
    }
    if (!str_cmp(arg2, "staff") )
    {
    send_to_char("The branch begins to shudder and vibrate.\n\r", ch);

    if (dice(1, ch->power[DISC_WARL_ENCH] * 4) == 1 )
    {
	send_to_char("   ...it explodes, spraying acid everywhere!\n\r", ch);
	act("$n's staff explodes, spraying acid everywhere.", ch, NULL, NULL, TO_ROOM);
	damage_old(ch, ch, ch->generation * 50, 1, DAM_ACID, TRUE);
	return;
    }
    if (dice(1, ch->power[DISC_WARL_ENCH] * 4) == 1 )
    {
	send_to_char("   ...it explodes in a ball of flame!\n\r", ch);
	act("$n's staff explodes in a ball of flame.", ch, NULL, NULL, TO_ROOM);
	damage_old(ch, ch, ch->generation * 30, 75, DAM_FIRE, TRUE);
	return;
    }
    
    switch(skill_table[found].realm)
    {
	case 1 :
	send_to_char("   ...it bursts into flames.\n\r", ch);
	obj = create_object( get_obj_index(60), 0);
	act("$n has created a staff of elemental fire!", ch, NULL, NULL, TO_ROOM);
	break;

	case 2 :
	send_to_char("   ...it starts to drip with water.\n\r", ch);
	obj = create_object( get_obj_index(61), 0);
	act("$n has created a staff of elemental water!", ch, NULL, NULL, TO_ROOM);
	break;
	
	case 3 :
	send_to_char("   ...it starts to glow brightly.\n\r", ch);
	obj = create_object( get_obj_index(62), 0);
	act("$n has created a staff of elemental air!", ch, NULL, NULL, TO_ROOM);
	break;

	case 4 :
	send_to_char("   ...it is now covered with thick brown mud.\n\r", ch);
	obj = create_object( get_obj_index(63), 0);
	act("$n has created a staff of elemental earth!", ch, NULL, NULL, TO_ROOM);
	break;

	case 5 :
	send_to_char("   ...it starts to blacken and char before your eyes.\n\r", ch);
	obj = create_object( get_obj_index(64), 0);
	act("$n has created a staff of death!", ch, NULL, NULL, TO_ROOM);
	break;
    
	default :
	break;  
    }
    }
    obj->value[3] = found;
    SET_BIT(obj->extra_flags, ITEM_MAGIC);
    if (ch->power[DISC_WARL_ENCH] >= 3)
    {
	send_to_char("   ...it starts to shudder uncontrollably.\n\r", ch);
	obj->value[1] = 5;
    }
    if (dice(1,7) < ch->power[DISC_WARL_ENCH])
    {
	send_to_char("   ...it starts to spark with electricity.\n\r", ch);
	obj->value[2] = 5;
    }
    if (ch->power[DISC_WARL_ENCH] >= 5 && dice(1,4) == 1)
    {
	send_to_char("   ...snaking tendrils of fire swirl around it.\n\r", ch);
	SET_BIT(obj->extra_flags, ITEM_FLAMING);
	obj->value[1] = 10;
    }
    if (ch->power[DISC_WARL_ENCH] >= 7 && dice(1,3) == 1)
    {
	send_to_char("   ...it gives off a cloud of purple smoke.\n\r", ch);
	SET_BIT(obj->extra_flags, ITEM_RELIC);
	obj->value[2] = 8;
    }
    if (obj->value[2] > obj->value[1])
	obj->value[2] = obj->value[1];
	obj_to_char(obj, ch);
}

void do_recharge(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj = NULL, *foundobj = NULL;
    int charge;

    if (argument[0] == '\0')
    {
	send_to_char("What item do you wish to recharge?\n\r", ch);
	return;
    }

    for ( obj = ch->carrying ; obj != NULL ; obj = obj->next_content)
    {
	if ( can_see_obj (ch, obj) && is_name (argument, obj->name))
	foundobj = obj;
    }

    if (foundobj == NULL)
    {
	send_to_char("You have not got that item in your inventory.\n\r", ch);
	return;
    }

    if (foundobj->item_type != ITEM_WAND
      && foundobj->item_type != ITEM_STAFF )
    {
	send_to_char("You may only recharge wands and staves.\n\r", ch);
	return;
    }

    charge = (foundobj->level * 10) + (foundobj->value[2] * 20);

    if (dice(1, ch->generation + ch->power[DISC_WARL_ENCH]) == 1)
    {
	send_to_char("You failed to recharge the item.\n\r", ch);
	return;    
    }
    if (dice(1, ch->generation * 2 + ch->power[DISC_WARL_ENCH] *2) == 1)
    {
	send_to_char("You suffer a mental backlash.\n\r", ch);
	return;    
    }
    WAIT_STATE(ch, 8);
    act("You lay your hands on $p and store your power within.", ch, foundobj, NULL, TO_CHAR);
    act("$n lays $s hands on $p, which begins to glow.", ch, foundobj, NULL, TO_ROOM);

    if (dice(1, ch->power[DISC_WARL_ENCH] * 5) == 1 )
    {
	send_to_char("   ...it explodes, with an ear-splitting BANG!\n\r", ch);
	act("$n creates a huge explosion, with an ear-splitting BANG!.", ch, NULL, NULL, TO_ROOM);
	damage_old(ch, ch, (ch->generation + ch->power[DISC_WARL_ENCH]) * 50, 1, DAM_SOUND, TRUE);
	extract_obj(foundobj);
	return;
    }

    if (dice(1,30 - ch->generation ) == 1)
    {    
	send_to_char("   ...it shudders violently.\n\r", ch);
	foundobj->value[1] +=2;
	foundobj->value[2] +=2;
    }

    if (ch->power[DISC_WARL_ENCH] == 3)
	foundobj->value[2] += dice (1,3);

    if (dice(1, ch->power[DISC_WARL_ENCH] *3) > 8)
    {
	send_to_char("   ...a cloud of yellow gas bursts from the tip.\n\r", ch);
	foundobj->value[2] += dice (2, 3);
    }
    if (dice(1, ch->power[DISC_WARL_ENCH] *4) > 14)
    {
	send_to_char("   ...it grows by a few centimeters.\n\r", ch);
	foundobj->value[1] += dice (1, 4);
	foundobj->value[2] += dice (3, 3);
    }
    if (dice(1, ch->power[DISC_WARL_ENCH] *5) > 24)
    {
	send_to_char("   ..it is enveloped in a shimmering aura.\n\r", ch);
	foundobj->value[1] += dice (1, 3);
	foundobj->value[2] += dice (4, 3);
    }
    do_anything (ch, ch->power[DISC_WARL_ENCH]);
    if (foundobj->value[2] > foundobj->value[1])
	foundobj->value[2] = foundobj->value[1];
}

void do_tolerate(CHAR_DATA *ch, char *argument)
{
    int expneed;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];

    expneed = (ch->value[0] + 1) * 500;

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0')
    {
	sprintf(buf, "You have currently attained %d%% tolerance against silver.\n\r", ch->value[0]);
	send_to_char(buf, ch);
	if (ch->value[0] < 80)
	    send_to_char("Use 'tolerate improve' to increase it further.\n\r", ch);
	return;
    }


    if (!str_prefix(arg1, "improve") )
    {
	if (ch->value[0] == 80)
	{
	    send_to_char("You cannot build up any more resistance to silver.\n\r", ch);
	    return;
	}
	 
	if (ch->currentexp < expneed)
	{
	    sprintf(buf,"You need %d experience to increase your tolerance further.\n\r", expneed);
	    send_to_char(buf, ch);
	    return;
	}

	ch->currentexp -= expneed;

	send_to_char("You condition your body against the poisonous touch of silver.\n\r", ch);
	ch->value[0] += 1;
	return;
    }

    do_tolerate(ch, "");
}

void do_sunlight(CHAR_DATA *ch, char *argument)
{
    int expneed;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];

    expneed = (ch->value[6] + 1) * 500;

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0')
    {
	sprintf(buf, "You have currently attained %d%% resistance against the sun.\n\r", ch->value[6]);
	send_to_char(buf, ch);
	if (ch->value[6] < 50)
	    send_to_char("Use 'sunlight improve' to increase it further.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "improve") )
    {
	if (ch->value[6] == 50)
	{ 
	    send_to_char("You cannot build up any more resistance to sunlight.\n\r", ch);
	    return;
	}

	if (ch->currentexp < expneed)
	{ 
	    sprintf(buf,"You need %d experience to increase your tolerance further.\n\r", expneed);
	    send_to_char(buf, ch);
	    return;
	}

	ch->currentexp -= expneed;

	send_to_char("You condition yourself against the deadly touch of the sun.\n\r", ch);
	ch->value[6] += 1;
	return;    
    }
    do_sunlight(ch, "");
}

void do_rage (CHAR_DATA *ch, char *argument)
{
    int maxrage, temp;

    if (ch->beast < 0) ch->beast = 0;
    if (ch->beast > 100) ch->beast = 100;

    maxrage = 40 + ch->beast / 2 + (ch->generation * 4);

    if (ch->rage > maxrage - 7)
    {
	send_to_char("Your beast is totally in control of you, you cannot rage any further.\n\r", ch);
	return;
    }
    
    send_to_char("You unleash your beast and go into a frenzied rage.\n\r", ch);
    act("$n screams with torment as $e goes into a frenzied rage.", ch, NULL, NULL, TO_ROOM);
    temp = ch->rage;
    
    ch->rage += (ch->beast / 5) + dice (1, 4) + 6;
/* berserk is evil -- whois
    if (temp == 0)
    {
	if (dice(1,3) == 1) do_berserk(ch, "");
	send_to_char("You plunge into a mad attacking frenzy.\n\r", ch);
    }
 */ 
    WAIT_STATE(ch, 8);
    
    if (ch->rage > maxrage)
    {
	send_to_char("You have reached the peak of your aggression.\n\r", ch);
	ch->rage = maxrage;
    }

    if (ch->secondaryrace == RACE_LUPINE
	&& ch->tertiaryrace == 0)
	werewolf(ch);
}

void do_calm (CHAR_DATA *ch, char *argument)
{
    WAIT_STATE(ch, 8);

    if (ch->beast >= 100) ch->beast = 100;
    if (ch->beast <= 0) ch->beast = 0;

    ch->rage -= (40 - (ch->beast / 3) +dice (1, 10));
    if (ch->rage <= 0)
    {
	ch->rage = 0;
	send_to_char("You have calmed out of your rage completely.\n\r", ch);
	unwerewolf(ch);
    }
    else send_to_char("You hold your breath and try to calm your frenzy.\n\r", ch);
}

void do_boost(CHAR_DATA *ch, char *argument)
{
    int max = 0;
    int off = 0;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];


    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    
    if ( (!str_prefix(arg1, "strength") 
	&& !str_prefix(arg1, "agility")
	&& !str_prefix(arg1, "resilience") )
	|| arg1 == '\0')
    {
	send_to_char("Which physical statistic do you wish to boost?\n\r", ch);
	send_to_char("  Syntax : boost <strength agility resilience> [max] [off]\n\r", ch);
	return;
    }

    if (!str_cmp(arg2, "max") && ch->generation < 7)
    {
	send_to_char("You are not of sufficient generation to maximise your stats.\n\r", ch);
	return;
    }

    if (!str_cmp(arg2, "off") )  off = 1;    

    if (!str_cmp(arg2, "max") )  max = 1;

    if (!str_prefix(arg1, "strength") )
    {
	if (off == 1)
	{
	   ch->perm_stat[STAT_STR] -= ch->value[7];
	   ch->value[7] = 0;
	   if (IS_FLAG3(ch, AFF_STR_BOOST) ) 
	   {
	       change_upkeep(ch, -12);           
	       REMOVE_BIT(ch->flag3, AFF_STR_BOOST);
	   }
	   else change_upkeep(ch, -4);
	   send_to_char("Your rush of adrenalin ceases, you feel weaker.\n\r", ch);           
	   return;
	}

	if (ch->value[7] != 0)
	{
	    send_to_char("Your strength is already boosted.\n\r", ch);
	    return;
	}
	if (max == 0)
	{
	    ch->value[7] = (ch->generation/3) + 1;
	    ch->perm_stat[STAT_STR] += ch->value[7];
	    change_upkeep(ch, 4);
	    send_to_char("You feel your blood pumping through your veins like fire.\n\r", ch);
	    return;
	}
	ch->value[7] = (ch->generation);
	ch->perm_stat[STAT_STR] += ch->value[7];
	change_upkeep(ch, 12);
	SET_BIT(ch->flag3, AFF_STR_BOOST);
	send_to_char("You feel your blood pumping through your veins like fire.\n\r", ch);
	send_to_char("You feel incredibly strong.\n\r", ch);
	return;
    }
    
    
    if (!str_prefix(arg1, "agility") )
    {
	if (off == 1)
	{
	   ch->perm_stat[STAT_DEX] -= ch->value[8];
	   ch->value[8] = 0;
	   if (IS_FLAG3(ch, AFF_DEX_BOOST) ) 
	   {
	       change_upkeep(ch, -12);           
	       REMOVE_BIT(ch->flag3, AFF_DEX_BOOST);
	   }
	   else change_upkeep(ch, -4);
	   send_to_char("Your rush of adrenalin ceases, you feel less agile.\n\r", ch);           
	   return;
	}
	if (ch->value[8] != 0)
	{
	    send_to_char("Your agility is already boosted.\n\r", ch);
	    return;
	}
	if (max == 0)
	{
	    ch->value[8] = (ch->generation/3) + 1;
	    ch->perm_stat[STAT_DEX] += ch->value[8];
	    change_upkeep(ch, 4);
	    send_to_char("You feel your blood pumping through your veins like fire.\n\r", ch);
	    return;
	}
	ch->value[8] = (ch->generation);
	ch->perm_stat[STAT_DEX] += ch->value[8];
	change_upkeep(ch, 12);
	SET_BIT(ch->flag3, AFF_DEX_BOOST);
	send_to_char("You feel your blood pumping through your veins like fire.\n\r", ch);
	send_to_char("You feel incredibly agile.\n\r", ch);
	return;
    }    
    
    if (!str_prefix(arg1, "resilience") )
    {
	if (off == 1)
	{
	   ch->perm_stat[STAT_CON] -= ch->value[9];
	   ch->value[9] = 0;
	   if (IS_FLAG3(ch, AFF_CON_BOOST) ) 
	   {
	       change_upkeep(ch, -12);           
	       REMOVE_BIT(ch->flag3, AFF_CON_BOOST);
	   }
	   else change_upkeep(ch, -4);
	   send_to_char("Your rush of adrenalin ceases, you feel less tough.\n\r", ch);           
	   return;
	}
	if (ch->value[9] != 0)
	{
	    send_to_char("Your resilience is already boosted.\n\r", ch);
	    return;
	}
	if (max == 0)
	{
	    ch->value[9] = (ch->generation/3) + 1;
	    ch->perm_stat[STAT_CON] += ch->value[9];
	    change_upkeep(ch, 4);
	    send_to_char("You feel your blood pumping through your veins like fire.\n\r", ch);
	    return;
	}
	ch->value[9] = (ch->generation);
	ch->perm_stat[STAT_CON] += ch->value[9];
	change_upkeep(ch, 12);
	SET_BIT(ch->flag3, AFF_CON_BOOST);
	send_to_char("You feel your blood pumping through your veins like fire.\n\r", ch);
	send_to_char("You feel incredibly tough.\n\r", ch);
	return;
    }    
    do_boost(ch, "");
}

void do_overload(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj = NULL, *foundobj = NULL;
    int dam;
    CHAR_DATA *gch;

    if (ch->power[DISC_WARL_ENCH] < 5)
    {
	send_to_char("You are not experienced enough in the enchanting discipline.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("What item do you wish to overload?\n\r", ch);
	return;
    }

    for ( obj = ch->carrying ; obj != NULL ; obj = obj->next_content)
    {
	if ( can_see_obj (ch, obj) && is_name (argument, obj->name))
	foundobj = obj;
    }

    if (foundobj == NULL)
    {
	send_to_char("You have not got that item in your inventory.\n\r", ch);
	return;
    }

    if (foundobj->item_type != ITEM_WAND
      && foundobj->item_type != ITEM_STAFF )
    {
	send_to_char("You may only overload wands and staves.\n\r", ch);
	return;
    }

    dam = 4;
    dam *= ch->generation;
    dam *= foundobj->value[2];
    WAIT_STATE(ch, 8);
    if (dice(1, 10) > ch->generation);    
    {
	send_to_char("It will not explode, maybe you should try again.\n\r", ch);
	return;
    }
    
    act("$p starts to vibrate rapidly...", ch, foundobj, NULL, TO_CHAR);    
    act("$n holds $p aloft, it explodes!", ch, foundobj, NULL, TO_ROOM);    
    
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if (ch->altitude != gch->altitude) continue;
	damage_old(ch, gch, dam, 1, DAM_ACID, TRUE);
	damage_old(ch, gch, dam, 75, DAM_FIRE, TRUE);
	send_to_char("There is an almighty blast of acid and flames!\n\r", gch);
    }
    extract_obj(foundobj);
}


void do_anything(CHAR_DATA *ch, int level)
{
/*  LOADS OF RANDOM EFFECTS    
       LEVEL SHOULD BE BETWEEN 1 AND 10.
	   FEEL FREE TO ADD MORE WEIRD AFFECTS!!!! */
    
    int any;
    CHAR_DATA *victim;    

    if (level > 10) level = 10;
    if (level < 1) level = 1;
    
    if (dice (1, 10) == 1) level += 2;
    if (dice (1, 10) == 1) level -= 1;
    if (dice (1, 50) == 1) level = 10-level;
    if (dice (1, 4) == 1) level += dice (1, level/2);
    if (dice (1, 4) == 1) level -= dice (1, level/2);
    
    if (level > 10) level = 10;
    if (level < 1) level = 1;
    if (dice (1, 10) != 4) return;
    
    any = dice (level, 10);
    
    switch (any)
    {
	case 1 :
	send_to_char("Lots of little pink butterflies fly out of your hair.\n\r", ch);
	act("Lots of little pink butterflies fly out of $n's hair.", ch, NULL, NULL, TO_ROOM);
	break;

	case 2 :
	send_to_char("A small rabbit appears out of thin air.\n\r", ch);
	act("$n has conjured a small white rabbit out of thin air.", ch, NULL, NULL, TO_ROOM);
	victim = create_mobile( get_mob_index( 16 ) );
	char_to_room(victim, ch->in_room, ch->altitude);
	break;

	case 3 :
	send_to_char("A small raincloud hovers over your head and soaks you to the skin.\n\r", ch);
	act("A little raincloud hovers above $n's head and soaks $m to the skin.", ch, NULL, NULL, TO_ROOM);
	ch->max_hit += 1;
	break;

	case 4 :
	send_to_char("The sky turns purple for a few seconds, you hear a malevolent laugh.\n\r", ch);
	act("The sky turns purple for a few seconds, $n laughs malevolently.", ch, NULL, NULL, TO_ROOM);
	break;

	case 5 :
	send_to_char("There is a sudden stillness in the air, until...\n\r", ch);
	act("There is a sudden stillness in the air, until...", ch, NULL, NULL, TO_ROOM);
	do_anything(ch, dice(3, 3) );
	break;

	case 6 :
	send_to_char("Your skin turns a bright blue, then fades.\n\r", ch);
	act("$n's skin turns a bright blue, then it fades.", ch, NULL, NULL, TO_ROOM);
	break;

	case 33 :
	send_to_char("A wandering leprechaun throws a bag of glittering dust over you.\n\r", ch);
	act("A wandering leprechaun throws a bag of glittering dust over $n.", ch, NULL, NULL, TO_ROOM);
	break;        

	case 34 :
	send_to_char("You feel soooooo beautiful today!\n\r", ch);
	act("Wow! Just look at $n, $e looks soooo beautiful!", ch, NULL, NULL, TO_ROOM);
	ch->perm_stat[STAT_CHA] += 1;
	break;

	case 37 :
	send_to_char("A wandering goblin throws a bag of dark powder over you.\n\r", ch);
	act("A wandering goblin throws a bag of dark powder over $n.", ch, NULL, NULL, TO_ROOM);
	break;

	case 40 :
	send_to_char("Smoke starts to leak out of your boots.\n\r", ch);
	act("Smoke starts to leak out of $n's boots.", ch, NULL, NULL, TO_ROOM);
	break;

	case 41 :
	send_to_char("You feel very strong, your muscles bulge!\n\r", ch);
	act("$n looks very strong, $s muscles are bulging.", ch, NULL, NULL, TO_ROOM);
	ch->perm_stat[STAT_STR] += 1;
	break;

	case 56 :
	send_to_char("Light glints off the end of your weapon.\n\r", ch);
	act("Light glints of the end of $n's weapon.", ch, NULL, NULL, TO_ROOM);
	ch->xhitroll += 5;
	break;

	case 57 :
	send_to_char("A feather falls out of the sky onto your nose.\n\r", ch);
	act("A small feather falls out of the sky onto $n's nose.", ch, NULL, NULL, TO_ROOM);
	break;

	case 58 :
	send_to_char("A bolt of electricity streaks down from the sky and turns you into a frog.\n\r", ch);
	act("A bolt of lightning streaks down and turns $n into a frog.", ch, NULL, NULL, TO_ROOM);
	ch->form = FRM_FROG;
	break;

	case 59 :
	send_to_char("You suddenly have a profound thought!\n\r", ch);
	ch->max_psp += 1;
	ch->psp += 1;
	break;

	case 60 :
	send_to_char("A passing rodent bites your leg, you feel weaker.\n\r", ch);
	act("A passing rodent bites $n's leg, $e looks weak with the poison.", ch, NULL, NULL, TO_ROOM);
	ch->perm_stat[STAT_STR] -= 1;
	break;

	case 61 :
	send_to_char("Suddenly everybody around you gets bigger, your nose starts to twitch.\n\r", ch);
	act("There is a puff of smoke and $n turns into a small rabbit.", ch, NULL, NULL, TO_ROOM);
	ch->form = FRM_RABBIT;
	break;

	case 77 :
	send_to_char("Your nose turns into a turnip.\n\r", ch);
	act("How weird, $n's nose has just turned into a turnip.", ch, NULL, NULL, TO_ROOM);
	ch->perm_stat[STAT_CHA] -= 3;
	break;

	case 90 :
	send_to_char("You drop dead.\n\r", ch);
	act("$n drops dead.\n\r", ch, NULL, NULL, TO_ROOM);
	ch->hit = -10;
	break;
    }
}


