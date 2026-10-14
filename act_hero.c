/******************************************************************
 * Act_Hero -- (C) 1996, by Adam Powell                           *
 * All functions coded by AdAm PoWeLl (Hero and Aasimon powers)   *
 ******************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "magic.h"

DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_group		);

int	count_heroes	args( (ROOM_INDEX_DATA *room) );

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

    heal = ch->power[DISC_HERO_FAIT];
    heal *= ch->generation * 10;
    heal = UMAX(10, heal);

    if (ch->power[DISC_HERO_CRUS] > 1) heal *= 2;

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

    if (ch->power[DISC_HERO_CRUS] > 1)
        SET_TIMER(ch, TIMER_LAYONHANDS, 15 - (ch->generation)/2);

    else SET_TIMER(ch, TIMER_LAYONHANDS, 20 - (ch->generation)/2);

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
	send_to_char("You draw a holy glyph on the floor.\n\r", ch);
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

	if (ch->faith < 9)
	{	    
	    send_to_char("It costs nine faith points to make a wall of prisms.\n\r", ch);    
	    return;
	}
	ch->faith -= 9;

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
